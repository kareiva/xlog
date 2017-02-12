/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2008 Joop Stakenborg <pg4i@amsat.org>

   This file is part of xlog.

   Xlog is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Xlog is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with xlog.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
 * main.c - start of gtk main loop and initialization
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <string.h>
#include <hamlib/rig.h>
#include <glib/gstdio.h>

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#include "gui_b4window.h"
#include "gui_scorewindow.h"
#include "gui_mainwindow.h"
#include "gui_setupdialog.h"
#include "gui_warningdialog.h"
#include "gui_utils.h"
#include "support.h"
#include "callbacks_mainwindow_menu.h"
#include "callbacks_mainwindow_qsoframe.h"
#include "callbacks_mainwindow.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "dxcc.h"
#include "remote.h"
#include "history.h"
#include "main.h"
#include "hamlib-utils.h"

GtkWidget *mainwindow, *mainnotebook;
gchar *xlogdir;
gint remotetimer = -1, clocktimer = -1, savetimer = -1, sockettimer = -1;
gchar **qso = NULL;
programstatetype programstate;
glong msgid;
GList *logwindowlist = NULL;
GdkColormap *colormap;
GIOChannel *channel;

extern GtkWidget *b4window, *scorewindow;
extern preferencestype preferences;
extern remotetype remote;
extern gint server_sockfd;
extern GtkUIManager *ui_manager;
extern GtkPrintSettings *print_settings;
extern GtkPageSetup *print_page_setup;

/* command line options */
static void
parsecommandline (int argc, char *argv[])
{
	gint p;
	gchar *versionstr;

	while ((p = getopt (argc, argv, "hv")) != -1)
	{
		switch (p)
		{
			case ':':
			case '?':
			case 'h':
				g_print ("Usage: %s [option] <log1.xlog> <log2.xlog>... \n",
					PACKAGE);
				g_print ("	-h	Display this help and exit\n");
				g_print ("	-v	Output version information and exit\n");
				exit (0);
			case 'v':
				versionstr = g_strdup_printf (_("%s version %s\n"),
					g_path_get_basename (argv[0]), VERSION);
				g_print ("%s", versionstr);
				g_free (versionstr);
				exit (0);
		}
	}
}

/* see if we can create the ~/.xlog directory */
static gboolean
xlogdircheck (void)
{
	GString *packagedir = g_string_new ("");
	struct stat statdir;

	g_string_printf (packagedir, "%s.", G_DIR_SEPARATOR_S);
	g_string_append (packagedir, PACKAGE);
	xlogdir = g_strconcat (g_get_home_dir (), packagedir->str, NULL);
	if (g_stat (xlogdir, &statdir) == -1)
		{
			if (g_mkdir (xlogdir, S_IRUSR | S_IWUSR | S_IXUSR) == -1)
				g_error (_("Creating ~%s directory."), packagedir->str);
			else
				show_setupdialog();
		}
	else if (!S_ISDIR (statdir.st_mode))
		g_error (_("~%s is not a directory."), packagedir->str);
	g_string_free (packagedir, TRUE);
	return TRUE;
}

/* defaults for program state */
static void
setdefaultstate (void)
{
	programstate.qsos = 0;
	programstate.controlkey = FALSE;
	programstate.rigfrequency = 0;
	programstate.rigmode = 0;
	programstate.rigrst = g_strdup ("0");
	programstate.rigpower = 0;
	programstate.scounter = 0;
	programstate.hlcounter = 0;
	programstate.tx = FALSE;
	programstate.statustimer = FALSE;
	programstate.shmid = -1;
	programstate.logwindows = 0;
	programstate.searchstr = g_strdup("");
	programstate.importremark = g_strdup("");
	programstate.dupecheck = 0;
	programstate.notdupecheckmode = FALSE;
	programstate.notdupecheckband = FALSE;
	programstate.px = g_strdup("");
	programstate.warning_nologopen = FALSE;
}

static gboolean
check_gtk_version (void)
{
	if ((gtk_major_version >= 2) && (gtk_minor_version >= 12))
		return FALSE;
	return TRUE;
}

/* sort logs by last modified date or by alphabet */
static gint sort_logs(gconstpointer a, gconstpointer b, gchar *savedir)
{
	gchar *log1 = g_strconcat
		(savedir, G_DIR_SEPARATOR_S, *(gchar **)a, NULL);
	gchar *log2 = g_strconcat
		(savedir, G_DIR_SEPARATOR_S, *(gchar **)b, NULL);

	gint diff;
	if (preferences.logorder == 0)
	{
		struct stat stat1, stat2;
		g_stat (log1, &stat1);
		g_stat (log2, &stat2);
		diff = stat1.st_mtime-stat2.st_mtime;
	}
	else
	{
		diff = strcmp (*(const char**)a, *(const char**)b);
	}

	g_free (log1);
	g_free (log2);
	return diff;
}


/* the fun starts here */
gint
main (int argc, char *argv[])
{
	GtkWidget *bandoptionmenu, *modeoptionmenu, *dateentry,
		*bandentry, *modeentry,	*clockhandlebox, *mhzlabel, *mhzbutton,
		*frequencylabel, *frequencyhandlebox, *modelabel, *modebutton,
		*rstlabel, *rstbutton, *smeterhandlebox, *smeterdrawingarea,
		*hpaned, *powerlabel, *powerbutton, *unknownlabel1,
		*unknownlabel2, *handlebox, *menuitem;
	gchar *temp;
	
	gint i, j, result;
	gboolean showmainwindow = TRUE, logsfromlist = FALSE, gtk_mismatch = FALSE;
	logtype *logwindow = NULL;
	LOGDB *lp = NULL;
	gboolean hamlibresult = FALSE;
	GtkTreePath *path;

	parsecommandline (argc, argv);

#ifdef G_OS_WIN32
	bindtextdomain (PACKAGE, "locale");
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);
#else
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, XLOG_LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);
#endif
#endif

	g_set_application_name ("xlog");

#ifdef G_OS_WIN32
	gchar *win32_dir = g_win32_get_package_installation_directory_of_module (NULL);
	g_chdir (win32_dir);
	g_free (win32_dir);
#endif

	gtk_init (&argc, &argv);
	gtk_mismatch = check_gtk_version ();
	if (gtk_mismatch)
	{
		warningdialog (_("xlog - startup"),
			_("xlog needs at least version 2.12 of the GTK+ libraries"), "gtk-dialog-error");
		exit (1);
	}
	setlocale(LC_NUMERIC, "C");

#ifdef G_OS_WIN32
	add_pixmap_directory ("pixmaps");
#else
	add_pixmap_directory
		(DATADIR G_DIR_SEPARATOR_S "pixmaps" G_DIR_SEPARATOR_S "xlog");
#endif

	showmainwindow = xlogdircheck();
	loadpreferences ();

	print_settings = NULL;
	print_page_setup = NULL;

	loadhistory ();
	mainwindow = create_mainwindow ();
	if (!mainwindow) exit (1);
	setdefaultstate ();
	colormap = gdk_colormap_get_system ();

	/* defaults for remote data */
	remote.version = 0;
	remote.nr = 0;
	remote.program = g_strdup ("unknown");

	result = readctydata ();
	if (result)
		g_warning (_("Could not read dxcc table"));
	result = readareadata ();
	if (result)
		g_warning (_("Could not read area table"));

	qso = g_new0 (gchar *, QSO_FIELDS);
	qso[REMARKS] = g_new0 (gchar, 512);
	for (i = 0; i < QSO_FIELDS - 1; i++)
		qso[i] = g_new0 (gchar, 100);

	/* logging should always be in GMT, shouldn't it? */
	putenv ("TZ=GMT");
	tzset ();

	/* update the date field */
	temp = xloggetdate ();
	dateentry = lookup_widget (mainwindow, "dateentry");
	gtk_entry_set_text (GTK_ENTRY (dateentry), temp);
	gtk_widget_grab_focus (GTK_WIDGET (dateentry));

	/* read the logs */
	mainnotebook = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK(mainnotebook), TRUE);
	gtk_widget_show (mainnotebook);
	hpaned = lookup_widget (mainwindow, "hpaned");
	gtk_paned_pack2 (GTK_PANED (hpaned), mainnotebook, TRUE, TRUE);
	gtk_paned_set_position (GTK_PANED (hpaned), preferences.handlebarpos);

	if (!g_dir_open (preferences.savedir, 0, NULL))
		preferences.savedir = g_strdup (xlogdir);

	/* check for multiple *.xlog */
	GPtrArray *xlogs;
	if (argc == 1)
	{
		if (g_strrstr (preferences.logstoload, "*")||
			g_strrstr (preferences.logstoload, "?"))
		/* read all logs of type xlog with a wildcard used */
		xlogs = getxlogs (preferences.savedir, preferences.logstoload);
		else
		{ /* read logs from preferences.logstoload */
			gchar **loglist;
			loglist = g_strsplit (preferences.logstoload, ",", -1);
			xlogs = g_ptr_array_new ();
			for (i = 0;; i++)
			{
				if (!loglist[i]) break;
				g_strstrip (loglist[i]);
				g_ptr_array_add (xlogs, g_strdup(loglist[i]));
			}
			g_strfreev (loglist);
			logsfromlist = TRUE;
		}
		g_ptr_array_sort_with_data
			(xlogs, (GCompareDataFunc) sort_logs, preferences.savedir);
	}
	else /* open logs from the command line */
	{
		xlogs = g_ptr_array_new ();
		for (i = 1; i < argc; i++)
			g_ptr_array_add (xlogs, g_strdup(argv[i]));
	}


	programstate.utf8error = FALSE;
	j = 0;

	if (xlogs) {
	for (i = 0; i < xlogs->len; i++)
	{
		gchar *logname = g_ptr_array_index (xlogs, i);
		gchar *xlogfile;

		if (logsfromlist)
			xlogfile = g_strconcat
				(preferences.savedir, G_DIR_SEPARATOR_S, logname, ".xlog", NULL);
		else
		{
			if (argc == 1)
				xlogfile = g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S,
					logname, NULL);
			else
				xlogfile = g_strdup(logname);
		}

		lp = log_file_open (xlogfile, TYPE_FLOG);
		if (lp)
		{
			logwindow = openlogwindow (lp, logname, j);
			log_file_qso_foreach (lp, fillin_list, logwindow);
			log_file_close (lp);
			logwindow->filename = g_strdup (xlogfile);
			if (logwindow->qsos > 1000)
			{
				path = gtk_tree_path_new_from_string ("0");
				gtk_tree_view_scroll_to_cell
					(GTK_TREE_VIEW (logwindow->treeview), path, NULL, TRUE, 0.5, 0.0);
				gtk_tree_path_free (path);

			}
			logwindowlist = g_list_append (logwindowlist, logwindow);
			/* backup */
			gchar *xlogbackupfile;
			if (preferences.backup == 1)
				xlogbackupfile = g_strconcat (xlogfile, ".backup", NULL);

			else
				xlogbackupfile = g_strconcat
					(preferences.backupdir, G_DIR_SEPARATOR_S,
					g_path_get_basename(xlogfile), ".backup", NULL);
			backuplog (xlogfile, xlogbackupfile);
			g_free (xlogbackupfile);
			j++;
		}
		else
		{
			g_warning (_("Can not open log \"%s\""), xlogfile);
		}
		g_free (logname);
		g_free (xlogfile);
	} g_ptr_array_free (xlogs, TRUE);}
	
	programstate.logwindows = j;
	set_tabs_menu ();

	g_signal_connect (G_OBJECT (mainnotebook), "switch_page",
					G_CALLBACK (on_mainnotebook_switch_page), NULL);

	/* set the menu strings for the bandoption menu */
	makebandoptionmenu (preferences.bands);
	/* set the menu strings for the modeoption menu */
	makemodeoptionmenu (preferences.modes);

	/* create wkd B4 dialog in case we need it */
	b4window = create_b4window ();

	/* we have to set the labels of the unknown fields,
		wether they are visible or not */
	unknownlabel1 = lookup_widget (mainwindow, "unknownlabel1");
	gtk_label_set_text
		(GTK_LABEL (unknownlabel1), preferences.freefield1);
	unknownlabel2 = lookup_widget (mainwindow, "unknownlabel2");
	gtk_label_set_text
		(GTK_LABEL (unknownlabel2), preferences.freefield2);

	/* update the statusbar with some information */
	temp = g_strdup_printf (_("%d QSO's loaded"), programstate.qsos);
	update_statusbar (temp);

	/* either hide the editbox or the optionmenu */
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
	if (preferences.modeseditbox == 0)
	{
		modeentry = lookup_widget (mainwindow, "modeentry");
		gtk_widget_hide (modeentry);
	}
	else
		gtk_widget_hide (modeoptionmenu);
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
	if (preferences.bandseditbox == 0)
	{
		bandentry = lookup_widget (mainwindow, "bandentry");
		gtk_widget_hide (bandentry);
	}
	else
		gtk_widget_hide (bandoptionmenu);

	/* set clock appearance */
	clockhandlebox = lookup_widget (mainwindow, "clockhandlebox");
	if (preferences.clock == 0)
	{	/* no clock */
		gtk_widget_hide (clockhandlebox);
	}
	else
	{	/* clock on statusbar */
		gtk_widget_show (clockhandlebox);
		clocktimer = g_timeout_add (1000, (GSourceFunc) updateclock, NULL);
	}

	/* set up the message queue for remote data */
#if HAVE_SYS_IPC_H
	msgid = msgget ((key_t) 1238, 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		temp = g_strdup_printf (_("Msgget failed: %s"), g_strerror (errno));
		update_statusbar (temp);
	}
	else /* check for a message twice a second */
		remotetimer = g_timeout_add (500, (GSourceFunc) remote_entry, NULL);
#endif
	/* Socket Server AF_INET Setup */
#ifndef G_OS_WIN32
	result = remote_socket_setup();
	if (result == -1)
	{
		temp = g_strdup_printf (_("Socket setup failed: %s"), g_strerror (errno));
		update_statusbar (temp);
		close (server_sockfd);
	}
	else /* check for a message on the socket */
	{
		channel = g_io_channel_unix_new (server_sockfd);
		sockettimer = g_io_add_watch (channel, G_IO_IN, socket_entry, NULL);
	}
#endif

	/* hamlib stuff */
	mhzlabel = lookup_widget (mainwindow, "mhzlabel");
	mhzbutton = lookup_widget (mainwindow, "mhzbutton");
	modelabel = lookup_widget (mainwindow, "modelabel");
	modebutton = lookup_widget (mainwindow, "modebutton");
	rstlabel = lookup_widget (mainwindow, "rstlabel");
	rstbutton = lookup_widget (mainwindow, "rstbutton");
	frequencylabel = lookup_widget (mainwindow, "frequencylabel");
	frequencyhandlebox = lookup_widget (mainwindow, "frequencyhandlebox");
	smeterhandlebox = lookup_widget (mainwindow, "smeterhandlebox");
	smeterdrawingarea = lookup_widget (mainwindow, "smeterdrawingarea");
	powerlabel = lookup_widget (mainwindow, "powerlabel");
	powerbutton = lookup_widget (mainwindow, "powerbutton");

	/* clickable buttons and events for hamlib */
	g_signal_connect (G_OBJECT (mhzbutton), "clicked",
					G_CALLBACK (on_mhzbutton_clicked), NULL);
	g_signal_connect (G_OBJECT (modebutton), "clicked",
					G_CALLBACK (on_modebutton_clicked), NULL);
	g_signal_connect (G_OBJECT (rstbutton), "clicked",
					G_CALLBACK (on_rstbutton_clicked), NULL);
	g_signal_connect (G_OBJECT (powerbutton), "clicked",
					G_CALLBACK (on_powerbutton_clicked), NULL);

	/* init rig if hamlib is enabled and show/hide some widgets */
	if (preferences.hamlib > 0)
		hamlibresult = start_hamlib (preferences.rigid, preferences.device,
			RIG_DEBUG_NONE, preferences.polltime);

	g_signal_connect (G_OBJECT (smeterdrawingarea), "configure_event",
		G_CALLBACK (on_smeterdrawingarea_configure_event), NULL);
	g_signal_connect (G_OBJECT (smeterdrawingarea), "expose_event",
		G_CALLBACK (on_smeterdrawingarea_expose_event),	NULL);
	g_signal_connect (G_OBJECT (mainwindow), "show",
		G_CALLBACK (on_mainwindow_show), NULL);
	sethamlibwidgets (preferences.hamlib, FALSE);

	if ((preferences.autosave > 0) && (preferences.saving == 1))
		savetimer = g_timeout_add (preferences.autosave * 60 * 1000,
				 (GSourceFunc) autosave, NULL);

	g_free (temp);

	gtk_widget_show (mainwindow);
	/* position the main window */
	gdk_window_move_resize (mainwindow->window, preferences.x,
		preferences.y, preferences.width, preferences.height);

	/* check all of the option menus and act upon */
	handlebox = lookup_widget (mainwindow, "handlebox");
	menuitem = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/OptionMenu/ShowToolbar");
	if (preferences.viewtoolbar == 1)
	{
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), TRUE);
		gtk_widget_show (handlebox);
	}
	else
	{
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), FALSE);
		gtk_widget_hide (handlebox);
	}

	menuitem = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/OptionMenu/WorkedBefore");
	if (preferences.viewb4 == 1)
	{
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), TRUE);
		gtk_widget_show (b4window);
		gdk_window_move_resize (b4window->window, preferences.b4x,
			preferences.b4y, preferences.b4width, preferences.b4height);
	}
	else
	{
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), FALSE);
		gtk_widget_hide (b4window);
	}

	/* activate scoring */
	scorewindow = create_scorewindow ();
	menuitem = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/OptionMenu/Scoring");
	if (preferences.viewscoring == 1)
	{
		fill_scoring_arrays ();
		update_dxccscoring ();
		if (preferences.awardswac == 1)
			update_wacscoring ();
		if (preferences.awardswas == 1)
			update_wasscoring ();
		if (preferences.awardswaz == 1)
			update_wazscoring ();
		if (preferences.awardsiota == 1)
			update_iotascoring ();
		if (preferences.awardsloc == 1)
			update_locscoring ();
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), TRUE);
		gtk_widget_show (scorewindow);
		gdk_window_move_resize (scorewindow->window, preferences.scorex,
			preferences.scorey, preferences.scorewidth, preferences.scoreheight);
	}
	else
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(menuitem), FALSE);

	/* last page gets focus */
	if (lp && i > 0)
	{
		logwindow = g_list_nth_data (logwindowlist, programstate.logwindows - 1);
		gtk_notebook_set_current_page
			(GTK_NOTEBOOK(mainnotebook), programstate.logwindows - 1);
		set_qsoframe (logwindow);
	}
	set_font (preferences.logfont);

	if (programstate.utf8error)
	{
		warningdialog (_("xlog - open log"),
			_("There were some errors converting from your locale to UTF8, "
			"which is used by GTK+ internally. It is best if you start xlog "
			"from a terminal and see what the errors are. Please check your "
			"language settings and your log fields!"), "gtk-dialog-warning");
	}
	if (j < i)
	{
		warningdialog (_("xlog - error"),
			_("There was an error while loading one of your logs, "
			"you may want to start xlog from a terminal "
			"to see what the errors are."), "gtk-dialog-error");
	}

	gtk_main ();
	return 0;
}
