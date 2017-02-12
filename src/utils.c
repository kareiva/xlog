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
 * utils.c - assorted utilities
 */

#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <hamlib/rig.h>

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#include <sys/msg.h>
#endif

#if HAVE_SYS_SHM_H
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include "log.h"
#include "utils.h"
#include "cfg.h"
#include "support.h"
#include "history.h"
#include "dxcc.h"
#include "gui_netkeyer.h"
#include "remote.h"
#include "callbacks_mainwindow_qsoframe.h"
#include "main.h"
#include "hamlib-utils.h"
#ifdef G_OS_WIN32
#include "math.h"
#endif
#include "awards_enum.h"

#ifdef G_OS_WIN32
#define RADIAN  (180.0 / M_PI)
#define ARC_IN_KM 111.2
#endif

gint statusbartimer;

extern GtkWidget *mainwindow, *keyerwindow, *b4window, *scorewindow;
extern preferencestype preferences;
extern programstatetype programstate;
extern remotetype remote;
extern gchar **qso;
extern gchar *xlogdir;
extern gint remotetimer, clocktimer, savetimer, sockettimer;
extern glong msgid;
extern void *shareCall;
extern GList *logwindowlist;
extern GtkUIManager *ui_manager;

#ifndef HAVE_INDEX
#ifdef HAVE_STRCHR
# define index(a,b) strchr(a,b)
# define rindex(a,b) strrchr(a,b)
#endif
#endif

#ifdef G_OS_WIN32
int win32_fork (char *prog) 
{
    PROCESS_INFORMATION proc_info; 
    STARTUPINFO start_info; 
    int ret;

    ZeroMemory(&proc_info, sizeof proc_info);
    ZeroMemory(&start_info, sizeof start_info);
    start_info.cb = sizeof start_info; 

    ret = CreateProcess(NULL, 
			prog,          /* command line */
			NULL,          /* process security attributes  */
			NULL,          /* primary thread security attributes */ 
			FALSE,         /* handles are inherited?  */
			0,             /* creation flags  */
			(LPVOID) NULL, /* NULL => use parent's environment  */
			NULL,          /* use parent's current directory  */
			&start_info,   /* receives STARTUPINFO */ 
			&proc_info);   /* receives PROCESS_INFORMATION  */

    return ret;
}
#endif

void
open_url (const char *link)
{
#ifdef G_OS_WIN32
	gchar *command = g_strdup_printf ("iexplore.exe %s", link);
	win32_fork (command);
	g_free (command);
#else
	gchar *command[] = {"xdg-open", NULL, NULL};

	command[1] = g_strdup (link);
	g_spawn_async
		(NULL, command, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
	g_free (command[1]);
#endif
}

#ifdef G_OS_WIN32
int myqrb(double lon1, double lat1, double lon2, double lat2, double *distance, double *azimuth) {
	double delta_long, tmp, arc, az;

	/* bail if NULL pointers passed */
	if (!distance || !azimuth)
		return -RIG_EINVAL;

	if ((lat1 > 90.0 || lat1 < -90.0) || (lat2 > 90.0 || lat2 < -90.0))
		return -RIG_EINVAL;

	if ((lon1 > 180.0 || lon1 < -180.0) || (lon2 > 180.0 || lon2 < -180.0))
		return -RIG_EINVAL;

	/* Prevent ACOS() Domain Error */
	if (lat1 == 90.0)
		lat1 = 89.999999999;
	else if (lat1 == -90.0)
		lat1 = -89.999999999;

	if (lat2 == 90.0)
		lat2 = 89.999999999;
	else if (lat2 == -90.0)
		lat2 = -89.999999999;

	/* Convert variables to Radians */
	lat1	/= RADIAN;
	lon1	/= RADIAN;
	lat2	/= RADIAN;
	lon2	/= RADIAN;

	delta_long = lon2 - lon1;

	tmp = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(delta_long);

	if (tmp > .999999999999999) {
		/* Station points coincide, use an Omni! */
		*distance = 0.0;
		*azimuth = 0.0;
		return RIG_OK;
	}

	if (tmp < -.999999) {
		/*
		 * points are antipodal, it's straight down.
		 * Station is equal distance in all Azimuths.
		 * So take 180 Degrees of arc times 60 nm,
		 * and you get 10800 nm, or whatever units...
		 */
		*distance = 180.0 * ARC_IN_KM;
		*azimuth = 0.0;
		return RIG_OK;
	}

	arc = acos(tmp);

	/*
	 * One degree of arc is 60 Nautical miles
	 * at the surface of the earth, 111.2 km, or 69.1 sm
	 * This method is easier than the one in the handbook
	 */

	
	*distance = ARC_IN_KM * RADIAN * arc;

	/* Short Path */
	/* Change to azimuth computation by Dave Freese, W1HKJ */

	az = RADIAN * atan2(sin(lon2 - lon1) * cos(lat2),
			    (cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(lon2 - lon1)));

	az = fmod(360.0 + az, 360.0);
	if (az < 0.0)
		az += 360.0;
	else if (az >= 360.0)
		az -= 360.0;

	*azimuth = floor(az + 0.5);
	return RIG_OK;
}
#endif

/* update the menu items of an optionmenu */
void
makebandoptionmenu (gchar *bands)
{
	GtkWidget *bandhbox2, *bandoptionmenu, *old, *entry, *label;
	gchar **split;
	gint index = 0;

	entry = g_object_get_data (G_OBJECT (mainwindow), "bandentry");
	label = g_object_get_data (G_OBJECT (mainwindow), "mhzlabel");
	old = g_object_get_data (G_OBJECT (mainwindow), "bandoptionmenu");
	if (old) gtk_widget_destroy (old);

	bandoptionmenu = gtk_combo_box_new_text ();
	gtk_widget_show (bandoptionmenu);
	bandhbox2 = lookup_widget (mainwindow, "bandhbox2");
	gtk_box_pack_start (GTK_BOX (bandhbox2), bandoptionmenu, TRUE, TRUE, 0);

	split = g_strsplit (bands, ",", -1);
	for (;;)
	{
		if (split[index] == NULL)
			break;
		gtk_combo_box_append_text (GTK_COMBO_BOX(bandoptionmenu), split[index]);
		index++;
	}
	g_strfreev (split);
	gtk_combo_box_set_active (GTK_COMBO_BOX (bandoptionmenu),
		preferences.bandoptionmenu);
	GLADE_HOOKUP_OBJECT (mainwindow, bandoptionmenu, "bandoptionmenu");
	if (preferences.bandseditbox == 1)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);
	else
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), bandoptionmenu);
	g_signal_connect (G_OBJECT(bandoptionmenu), "mnemonic-activate",
		G_CALLBACK (bandoptionactivate), NULL);
}

void
makemodeoptionmenu (gchar *modes)
{
	GtkWidget *modehbox2, *modeoptionmenu, *old, *entry, *label;
	gchar **split;
	gint index = 0;

	entry = g_object_get_data (G_OBJECT (mainwindow), "modeentry");
	label = g_object_get_data (G_OBJECT (mainwindow), "modelabel");
	old = g_object_get_data (G_OBJECT (mainwindow), "modeoptionmenu");
	if (old) gtk_widget_destroy (old);

	modeoptionmenu = gtk_combo_box_new_text ();
	gtk_widget_show (modeoptionmenu);
	modehbox2 = lookup_widget (mainwindow, "modehbox2");
	gtk_box_pack_start (GTK_BOX (modehbox2), modeoptionmenu, TRUE, TRUE, 0);

	split = g_strsplit (modes, ",", -1);
	for (;;)
	{
		if (split[index] == NULL)
			break;
		gtk_combo_box_append_text (GTK_COMBO_BOX(modeoptionmenu), split[index]);
		index++;
	}
	g_strfreev (split);
	gtk_combo_box_set_active (GTK_COMBO_BOX (modeoptionmenu),
		preferences.modeoptionmenu);
	GLADE_HOOKUP_OBJECT (mainwindow, modeoptionmenu, "modeoptionmenu");
	if (preferences.modeseditbox == 1)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);
	else
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), modeoptionmenu);
	g_signal_connect (G_OBJECT(modeoptionmenu), "mnemonic-activate",
		G_CALLBACK (modeoptionactivate), NULL);
}

/* removing leading and trailing whitespaces from an array of strings */
void
deletespaces (gchar ** split)
{
	gint index = 0;

	for (;;)
	{
		if (split[index] == NULL)
			break;
		g_strstrip (split[index]);
		index++;
	}
}

/* clear statusbar */
static gint
statusbar_timeout (gpointer data)
{
	GtkWidget *statusbar;

	statusbar = lookup_widget (mainwindow, "statusbar");
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, _("Ready."));
	g_source_remove (statusbartimer);
	programstate.statustimer = FALSE;
	return FALSE;
}

/* change the statusbar */
void
update_statusbar (gchar * string)
{
	GtkWidget *statusbar;

	statusbar = lookup_widget (mainwindow, "statusbar");
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, string);
	if (programstate.statustimer)
		g_source_remove (statusbartimer);
	statusbartimer = g_timeout_add (30000, statusbar_timeout, NULL);
	programstate.statustimer = TRUE;
}

/* free mem in state struct */
static void
freestate (void)
{
	g_free (programstate.rigrst);
	g_free (programstate.searchstr);
	g_free (programstate.px);
}

/* free mem in prefs struct */
static void
freeprefs (void)
{
	g_free (preferences.modes);
	g_free (preferences.bands);
	g_free (preferences.device);
	g_free (preferences.rigconf);
	g_free (preferences.logfont);
	g_free (preferences.savedir);
	g_free (preferences.backupdir);
	g_free (preferences.logstoload);
	g_free (preferences.locator);
	g_free (preferences.freefield1);
	g_free (preferences.freefield2);
	g_free (preferences.callsign);
	g_free (preferences.defaultmhz);
	g_free (preferences.defaultmode);
	g_free (preferences.defaulttxrst);
	g_free (preferences.defaultrxrst);
	g_free (preferences.defaultpower);
	g_free (preferences.defaultfreefield1);
	g_free (preferences.defaultfreefield2);
	g_free (preferences.defaultremarks);
	g_free (preferences.cwf1);
	g_free (preferences.cwf2);
	g_free (preferences.cwf3);
	g_free (preferences.cwf4);
	g_free (preferences.cwf5);
	g_free (preferences.cwf6);
	g_free (preferences.cwf7);
	g_free (preferences.cwf8);
	g_free (preferences.cwf9);
	g_free (preferences.cwf10);
	g_free (preferences.cwf11);
	g_free (preferences.cwf12);
	g_free (preferences.openurl);
	g_free (preferences.initlastmsg);
}

/* free mem in remote struct */
static void
freeremote (void)
{
	g_free (remote.program);
}

/* cleanup of variables, used at exit */
void
save_windowsize_and_cleanup (void)
{
	gint i;
	GtkWidget *bandoptionmenu, *modeoptionmenu, *handlebox, *hpaned;

	/* free the tables array */
	cleanup_dxcc ();
	cleanup_area ();

	/* free the qso array */
	for (i = 0; i < QSO_FIELDS; i++)
		g_free (qso[i]);
	g_free (qso);

	g_list_free (logwindowlist);

	gtk_window_get_size (GTK_WINDOW(mainwindow),
		&preferences.width, &preferences.height);
	gtk_window_get_position (GTK_WINDOW(mainwindow),
		&preferences.x, &preferences.y);
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
	preferences.modeoptionmenu =
		gtk_combo_box_get_active (GTK_COMBO_BOX(modeoptionmenu));
	preferences.bandoptionmenu =
		gtk_combo_box_get_active (GTK_COMBO_BOX(bandoptionmenu));

	handlebox = lookup_widget (mainwindow, "handlebox");
	if (gtk_widget_get_visible(handlebox))
		preferences.viewtoolbar = 1;
	else
		preferences.viewtoolbar = 0;
	if (gtk_widget_get_visible(b4window))
	{
		preferences.viewb4 = 1;
		gtk_window_get_size (GTK_WINDOW(b4window),
			&preferences.b4width, &preferences.b4height);
		gtk_window_get_position (GTK_WINDOW(b4window),
			&preferences.b4x, &preferences.b4y);
	}
	else
	{
		preferences.viewb4 = 0;
		preferences.b4x = 10;
		preferences.b4y = 30;
		preferences.b4width = 500;
		preferences.b4height = 300;
	}
	if (gtk_widget_get_visible(scorewindow))
	{
		gtk_window_get_size (GTK_WINDOW(scorewindow),
			&preferences.scorewidth, &preferences.scoreheight);
		gtk_window_get_position (GTK_WINDOW(scorewindow),
			&preferences.scorex, &preferences.scorey);
		preferences.viewscoring = 1;
	}
	else
	{
		preferences.viewscoring = 0;
		preferences.scorex = 10;
		preferences.scorey = 300;
		preferences.scorewidth = 500;
		preferences.scoreheight = 300;
	}

	hpaned = lookup_widget (mainwindow, "hpaned");
	preferences.handlebarpos = gtk_paned_get_position (GTK_PANED(hpaned));

	if (keyerwindow) savekeyer ();
	savepreferences ();
	savehistory ();
	if (preferences.hamlib > 0)
		stop_hamlib ();

	g_free (xlogdir);
	if (remotetimer != -1)
		g_source_remove (remotetimer);
	if (savetimer != -1)
		g_source_remove (savetimer);
	if (clocktimer != -1)
		g_source_remove (clocktimer);
	if (sockettimer != -1)
		g_source_remove (sockettimer);

	/* remove message queue */
#if HAVE_SYS_IPC_H
	if (msgid != -1)
		msgctl (msgid, IPC_RMID, 0);
#endif

	/* detach shared mem and destroy it */
#if HAVE_SYS_SHM_H
	if (programstate.shmid != -1)
		{
			shmdt (shareCall);
			shmctl (programstate.shmid, IPC_RMID, NULL);
		}
#endif

	freestate ();
	freeprefs ();
	freeremote ();
}

/* get the current date, returned value has to be freed */
gchar *
xloggetdate (void)
{
	time_t current;
	struct tm *timestruct = NULL;
	gchar datenow[20], *date;
	GError *error;

	time (&current);
	timestruct = localtime (&current);
	strftime (datenow, sizeof(datenow), "%d %b %Y", timestruct);

	if (!g_utf8_validate (datenow, -1, NULL ))
	{
		date = g_locale_to_utf8 (datenow, -1, NULL, NULL, &error);
		if (!date)
		{
				g_warning (_("Unable to convert '%s' to UTF-8: %s"), datenow,
					error->message);
				g_error_free (error);
			}
 	}
	else date = g_strdup (datenow);

	return (date);
}

/* get the current time, returned value has to be freed */
gchar *
xloggettime (void)
{
	time_t current;
	struct tm *timestruct = NULL;
	gchar stimenow[20];

	time (&current);
	timestruct = localtime (&current);
	strftime (stimenow, sizeof(stimenow), "%H%M", timestruct);
	return (g_strdup (stimenow));
}

/* look up mode in a list of modes */
gchar *
lookup_mode (gint index)
{
	gchar **s, *mode;

	s = g_strsplit (preferences.modes, ",", -1);
	mode = g_strdup (s[index]);
	g_strfreev (s);
	return (mode);
}

/* look up band in a list of bands */
gchar *
lookup_band (gint index)
{
	gchar **s, *band;

	s = g_strsplit (preferences.bands, ",", -1);
	band = g_strdup (s[index]);
	g_strfreev (s);
	return (band);
}

/* clock which updates a label in the statusbar */
gint
updateclock (void)
{
	GtkWidget *clocklabel;
	GString *timestr = g_string_new ("");
	gchar *nowdate, *nowtime;

	nowdate = xloggetdate ();
	nowtime = xloggettime ();
	clocklabel = lookup_widget (mainwindow, "clocklabel");
	g_string_printf (timestr, "<b>%s", nowdate);
	timestr = g_string_append_c (timestr, ' ');
	timestr = g_string_append (timestr, nowtime);
	timestr = g_string_append (timestr, " UTC</b>");
	gtk_label_set_markup (GTK_LABEL (clocklabel), timestr->str);
	g_string_free (timestr, TRUE);
	g_free (nowdate);
	g_free (nowtime);
	return 1;
}

/* Put color in the right format for XParseColor, so it can be processed by gdk_color_parse */
gchar *
color_parse (gchar * value)
{
	gchar **valuesplit = NULL;
	gchar *rgbsyntax;

	valuesplit = g_strsplit (value, ",", 3);
	rgbsyntax =
		g_strconcat ("rgb:", valuesplit[0], "/", valuesplit[1], "/",
		 valuesplit[2], NULL);
	g_strfreev (valuesplit);
	return (rgbsyntax);
}



gint
autosave (void)
{
	gint i;
	logtype *logw;
	gchar *temp;
	gboolean message = FALSE;

	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		logw = g_list_nth_data (logwindowlist, i);
		if (logw->logchanged)
		{
			savelog (logw, logw->filename, TYPE_FLOG, 1, logw->qsos);
			logw->logchanged = FALSE;
			temp = g_strdup_printf ("<b>%s</b>", logw->logname);
			gtk_label_set_markup (GTK_LABEL (logw->label), temp);
			g_free (temp);
			message = TRUE;
		}
	}

	if (message)
	{
		temp = g_strdup_printf (_("Log(s) autosaved"));
		update_statusbar (temp);
		g_free (temp);
	}
	return 1;
}

/*
 * check visibility of fields in the loglist and show/hide the corresponding
 * widgets in the QSO frame
 */
void
set_qsoframe (gpointer arg)
{
	gint i, j;
	GtkWidget *endhbox, *awardshbox, *powerhbox, *namehbox, *locatorhbox, *qthhbox,
		*unknown1hbox, *unknown2hbox, *powerbutton, *powerlabel, *qslhbox,
		*locatorframe, *remarksvbox, *clearallmenu;
	logtype *logw = (logtype *) arg;

	endhbox = lookup_widget (mainwindow, "endhbox");
	awardshbox = lookup_widget (mainwindow, "awardshbox");
	powerhbox = lookup_widget (mainwindow, "powerhbox");
	qslhbox = lookup_widget (mainwindow, "qslhbox");
	namehbox = lookup_widget (mainwindow, "namehbox");
	qthhbox = lookup_widget (mainwindow, "qthhbox");
	locatorhbox = lookup_widget (mainwindow, "locatorhbox");
	locatorframe = lookup_widget (scorewindow, "locatorframe");
	unknown1hbox = lookup_widget (mainwindow, "unknown1hbox");
	unknown2hbox = lookup_widget (mainwindow, "unknown2hbox");
	remarksvbox = lookup_widget (mainwindow, "remarksvbox");

	if (logw) for (i = 0; i < QSO_FIELDS; i++)
	{
		for (j = 0; j < logw->columns; j++)
		{
		if (i == logw->logfields[j])
			break;
		}
		if (i == GMTEND)
		{
			if (j == logw->columns)
				gtk_widget_hide (endhbox);
			else
				gtk_widget_show (endhbox);
		}
		else if (i == AWARDS)
		{
			if (j == logw->columns)
				gtk_widget_hide (awardshbox);
			else
				gtk_widget_show (awardshbox);
		}
		else if (i == POWER)
		{
			powerbutton = lookup_widget (mainwindow, "powerbutton");
			powerlabel = lookup_widget (mainwindow, "powerlabel");
			if (j == logw->columns)
				gtk_widget_hide (powerhbox);
			else
			{
				gtk_widget_show (powerhbox);
				if (preferences.hamlib > 0)
				{
					gtk_widget_show (powerbutton);
					gtk_widget_hide (powerlabel);
				}
				else
				{
					gtk_widget_hide (powerbutton);
					gtk_widget_show (powerlabel);
				}
			}
		}
		else if (i == QSLOUT)
		{
			if (j == logw->columns)
				gtk_widget_hide (qslhbox);
			else
				gtk_widget_show (qslhbox);
		}
		else if (i == NAME)
		{
			if (j == logw->columns)
				gtk_widget_hide (namehbox);
			else
				gtk_widget_show (namehbox);
		}
		else if (i == QTH)
		{
			if (j == logw->columns)
				gtk_widget_hide (qthhbox);
			else
				gtk_widget_show (qthhbox);
		}
		else if (i == LOCATOR)
		{
			if (j == logw->columns)
			{
				gtk_widget_hide (locatorhbox);
				gtk_widget_hide (locatorframe);
			}
			else
			{
				gtk_widget_show (locatorhbox);
				gtk_widget_show (locatorframe);
			}
		}
		else if (i == U1)
		{
			if (j == logw->columns)
				gtk_widget_hide (unknown1hbox);
			else
				gtk_widget_show (unknown1hbox);
		}
		else if (i == U2)
		{
			if (j == logw->columns)
				gtk_widget_hide (unknown2hbox);
			else
				gtk_widget_show (unknown2hbox);
		}
		else if (i == REMARKS)
		{
			if (j == logw->columns)
				gtk_widget_hide (remarksvbox);
			else
				gtk_widget_show (remarksvbox);
		}
	}
	else
	{
		gtk_widget_hide (endhbox);
		gtk_widget_show (qslhbox);
		gtk_widget_hide (powerhbox);
		gtk_widget_hide (namehbox);
		gtk_widget_hide (qthhbox);
		gtk_widget_hide (locatorhbox);
		gtk_widget_hide (locatorframe);
		gtk_widget_hide (endhbox);
		gtk_widget_hide (unknown1hbox);
		gtk_widget_hide (unknown2hbox);
		gtk_widget_show (remarksvbox);
	}
	clearallmenu = gtk_ui_manager_get_widget
         (ui_manager, "/MainMenu/EditMenu/Clear All");
	g_signal_emit_by_name (G_OBJECT (clearallmenu), "activate");
}

gchar *
my_strreplace(const char *str, const char *delimiter, const char *replacement)
{
	gchar **split;
	gchar *ret;

	g_return_val_if_fail (str != NULL, NULL);
	g_return_val_if_fail (delimiter != NULL, NULL);
	g_return_val_if_fail (replacement != NULL, NULL);

	split = g_strsplit (str, delimiter, 0);
	ret = g_strjoinv (replacement, split);
	g_strfreev (split);

	return ret;
}

/* check if the awards entry contains a valid awards string */
gchar *
valid_awards_entry (gchar *a, guint *st, guint *zone, guint *cont, guint *iota)
{
	gchar **s = g_strsplit (a, ",", -1);
	gchar *p = NULL, *dup = NULL;
	guint i = 0;

	*st = 99;
	*zone = 99;
	*cont = 99;
	*iota = NOT_AN_IOTA;
	for (;;)
	{
		if (s[i] == NULL) break;
		p = index (s[i], '-');
		if (p)
		{
			if (!g_ascii_strncasecmp(s[i], "DXCC", 4))
				dup = g_strdup (p + 1);
			else if (!g_ascii_strncasecmp(s[i], "WAZ", 3))
				*zone = atoi (p + 1);
			else if (!g_ascii_strncasecmp(s[i], "WAC", 3))
				*cont = cont_to_enum (p + 1);
			else if (!g_ascii_strncasecmp(s[i], "WAS", 3))
				*st = state_to_enum (p + 1);
			else if (!g_ascii_strncasecmp(s[i], "IOTA", 4))
				*iota = iota_to_num (p + 1);
		}
		i++;
	}
	g_strfreev (s);
	return dup;
}

gboolean
qslreceived(gchar *in)
{
	if (in && (!g_ascii_strcasecmp (in, "x") || !g_ascii_strcasecmp (in, "y"))) return TRUE;
	return FALSE;
}

gint num_qsos_to_export;

gint
get_num_qsos_to_export(void)
{
  return num_qsos_to_export;
}

void 
save_num_qsos_to_export(gint num)
{
  num_qsos_to_export = num;
}
