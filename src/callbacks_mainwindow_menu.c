/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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
 * callbacks_mainwindow_menu.c - menu's which don't create a dialog
 */


#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <hamlib/rig.h>

#include "callbacks_mainwindow_menu.h"
#include "gui_savedialog.h"
#include "gui_utils.h"
#include "support.h"
#include "cfg.h"
#include "utils.h"
#include "log.h"
#include "hamlib-utils.h"
#include "xlog_enum.h"
#include "dxcc.h"

#ifndef HAVE_STRPTIME
#include "strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

extern GtkWidget *mainwindow, *keyerwindow, *scorewindow;
extern GtkWidget *mainnotebook;
extern preferencestype preferences;
extern GList *logwindowlist;
extern GtkUIManager *ui_manager;
extern GtkPageSetup *print_page_setup;
extern GtkPrintSettings *print_settings;

/* get the action name and use it to switch logs <alt>1, <alt>2, etc. */
void 
on_menu_log_activate (GtkAction *action, gpointer user_data)
{
	const gchar *name;
	gint lognr;

	name = gtk_action_get_name(action);
	lognr = atoi (name);
	if (lognr <= g_list_length (logwindowlist))
		gtk_notebook_set_current_page (GTK_NOTEBOOK (mainnotebook), lognr - 1);
}


void
on_menu_pagesetup_activate (GtkAction *action, gpointer user_data)
{
	GtkPageSetup *new_page_setup;

	if (print_settings == NULL)
		print_settings = gtk_print_settings_new ();
	new_page_setup = gtk_print_run_page_setup_dialog
		(GTK_WINDOW (mainwindow), print_page_setup, print_settings);
	if (print_page_setup)
		g_object_unref (print_page_setup);
	print_page_setup = new_page_setup;
}

void
on_menu_exit_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	gint i;
	logtype *logwindow;
	gboolean logchanged = FALSE;

	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		logwindow = g_list_nth_data (logwindowlist, i);
		if (logwindow->logchanged) logchanged = TRUE;
	}

	if (logchanged)
		create_savedialog ();
	else
	{
		if (preferences.areyousure > 0)
			create_exitdialog ();
		else
		{
			save_windowsize_and_cleanup ();
			gtk_main_quit ();
		}
	}
}

/* save current page to the log */
void
on_menu_save_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook));
	if (page >= 0)
	{
		logtype *logwindow = g_list_nth_data (logwindowlist, page);
		savelog (logwindow, logwindow->filename, TYPE_FLOG, 1 , logwindow->qsos);
		gchar *temp = g_strdup_printf (_("Log saved to %s"), logwindow->filename);
		update_statusbar (temp);
		logwindow->logchanged = FALSE;
		temp = g_strdup_printf ("<b>%s</b>", logwindow->logname);
		gtk_label_set_markup (GTK_LABEL (logwindow->label), temp);
		g_free (temp);
	}
}

/* clear all the fields in the qso frame */
void
on_clearframe_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *dateentry, *gmtentry, *callentry, *rstentry, *myrstentry,
		*remtv, *modeentry, *bandentry, *incheckbutton, *outcheckbutton,
		*endentry, *powerentry, *nameentry, *qthentry, *locatorentry,
		*unknownentry1, *unknownentry2, *modeoptionmenu, *bandoptionmenu,
		*awardsentry, *qsoframe, *locatorframe, *datebutton, *framelabel;
	GtkTextBuffer *b;
	gchar *temp;

	dateentry = lookup_widget (mainwindow, "dateentry");
	gmtentry = lookup_widget (mainwindow, "gmtentry");
	endentry = lookup_widget (mainwindow, "endentry");
	callentry = lookup_widget (mainwindow, "callentry");
	bandentry = lookup_widget (mainwindow, "bandentry");
	modeentry = lookup_widget (mainwindow, "modeentry");
	rstentry = lookup_widget (mainwindow, "rstentry");
	myrstentry = lookup_widget (mainwindow, "myrstentry");
	remtv = lookup_widget (mainwindow, "remtv");
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
	incheckbutton = lookup_widget (mainwindow, "incheckbutton");
	outcheckbutton = lookup_widget (mainwindow, "outcheckbutton");
	awardsentry = lookup_widget (mainwindow, "awardsentry");
	powerentry = lookup_widget (mainwindow, "powerentry");
	nameentry = lookup_widget (mainwindow, "nameentry");
	qthentry = lookup_widget (mainwindow, "qthentry");
	locatorentry = lookup_widget (mainwindow, "locatorentry");
	unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
	unknownentry2 = lookup_widget (mainwindow, "unknownentry2");

	unselect_logs ();

	gtk_editable_delete_text (GTK_EDITABLE (dateentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (gmtentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (endentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (callentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (modeentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (bandentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (rstentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (myrstentry), 0, -1);
	b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
	gtk_text_buffer_set_text (b, "", 0);
	gtk_combo_box_set_active (GTK_COMBO_BOX(modeoptionmenu), 0);
	gtk_combo_box_set_active (GTK_COMBO_BOX(bandoptionmenu), 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outcheckbutton), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (incheckbutton), FALSE);
	gtk_editable_delete_text (GTK_EDITABLE (awardsentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (powerentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (nameentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (qthentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (locatorentry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (unknownentry1), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (unknownentry2), 0, -1);

	qsoframe = lookup_widget (mainwindow, "qsoframe");
	framelabel = gtk_frame_get_label_widget (GTK_FRAME(qsoframe));
	temp = g_strdup_printf ("<b>%s</b>", _("New QSO"));
	gtk_label_set_markup (GTK_LABEL (framelabel), temp);
	g_free (temp);

	locatorframe = lookup_widget (scorewindow, "locatorframe");
	if (gtk_widget_get_visible (locatorframe))
	{
		framelabel = gtk_frame_get_label_widget (GTK_FRAME(locatorframe));
		gtk_label_set_text (GTK_LABEL (framelabel), _("Locator"));
	}
	if (keyerwindow)
		gtk_widget_grab_focus (callentry);
	else
	{
		datebutton = lookup_widget (mainwindow, "datebutton");
		gtk_widget_grab_focus (datebutton);
	}

	/* Update (clear) Awards */
	guint st, zone, cont, iota;
	gchar *aw = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
	gchar *result = valid_awards_entry (aw,  &st, &zone, &cont, &iota);
	if (result)
	{
		updatedxccframe (result, TRUE, st, zone, cont, iota);
		g_free (result);
	}
	else
	{
		gchar *call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
		updatedxccframe (call, FALSE, st, zone, cont, iota);
		g_free (call);
	}
	g_free (aw);
}

/* click all buttons when hamlib enabled, fill in defaults */
void
on_clickall_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *datebutton, *gmtbutton, *endbutton, *endhbox,
		*myrstentry, *unknown1hbox, *unknown2hbox, *unknownentry1, *unknownentry2,
		*remarksvbox, *remtv, *bandentry, *modeentry, *rstentry, *powerhbox, 
		*powerentry, *endentry, *callentry, *count, *awardsentry,
		*bandoptionmenu, *modeoptionmenu;
	GtkTextBuffer *b;
	gint c, bandenum, modeenum;
	gchar *countstr, *lastmsg, *str;
	GtkWidget *mhzbutton, *modebutton, *rstbutton, *powerbutton;

	datebutton = lookup_widget (mainwindow, "datebutton");
	gmtbutton = lookup_widget (mainwindow, "gmtbutton");
	g_signal_emit_by_name (G_OBJECT (datebutton), "clicked");
	g_signal_emit_by_name (G_OBJECT (gmtbutton), "clicked");
	endhbox = lookup_widget (mainwindow, "endhbox");
	if (gtk_widget_get_visible (endhbox))
	{
		endentry = lookup_widget (mainwindow, "endentry");
		if (strlen(gtk_editable_get_chars (GTK_EDITABLE (endentry), 0, -1)) == 0)
		{
			endbutton = lookup_widget (mainwindow, "endbutton");
			g_signal_emit_by_name (G_OBJECT (endbutton), "clicked");
		}
	}

	bandentry = lookup_widget (mainwindow, "bandentry");
	modeentry = lookup_widget (mainwindow, "modeentry");
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
	rstentry = lookup_widget (mainwindow, "rstentry");
	powerhbox = lookup_widget (mainwindow, "powerhbox");
	powerentry = lookup_widget (mainwindow, "powerentry");

	if (preferences.hamlib > 0)
	{ /* when polling use state struct else retrieve info from
		the rig one by one */
		if (preferences.polltime == 0) get_riginfo();
		if (preferences.bandseditbox == 0 ||
	strlen(gtk_editable_get_chars (GTK_EDITABLE (bandentry), 0, -1)) == 0)
		{
			mhzbutton = lookup_widget (mainwindow, "mhzbutton");
			g_signal_emit_by_name (G_OBJECT (mhzbutton), "clicked");
		}
		if (preferences.modeseditbox == 0 ||
	strlen(gtk_editable_get_chars (GTK_EDITABLE (modeentry), 0, -1)) == 0)
		{
			modebutton = lookup_widget (mainwindow, "modebutton");
			g_signal_emit_by_name (G_OBJECT (modebutton), "clicked");
		}
		if (strlen(gtk_editable_get_chars (GTK_EDITABLE (rstentry), 0, -1)) == 0)
		{
			rstbutton = lookup_widget (mainwindow, "rstbutton");
			g_signal_emit_by_name (G_OBJECT (rstbutton), "clicked");
		}
		if (gtk_widget_get_visible (powerhbox))
		{
			if (strlen(gtk_editable_get_chars (GTK_EDITABLE (powerentry), 0, -1)) == 0)
			{
				powerbutton = lookup_widget (mainwindow, "powerbutton");
				g_signal_emit_by_name (G_OBJECT (powerbutton), "clicked");
			}
		}
	}
	else
	{
 		if (strlen (preferences.defaultmhz) > 0)
		{
 			if (strlen(gtk_editable_get_chars (GTK_EDITABLE (bandentry), 0, -1)) == 0)
 				gtk_entry_set_text (GTK_ENTRY (bandentry), preferences.defaultmhz);
			bandenum = freq2enum (preferences.defaultmhz);
			if (bandenum >= 0)
			activate_bandoption_by_enum
				(bandoptionmenu, preferences.bands, bandenum);
		}
 		if (strlen (preferences.defaultmode) > 0)
		{
 			if (strlen(gtk_editable_get_chars (GTK_EDITABLE (modeentry), 0, -1)) == 0)
 				gtk_entry_set_text (GTK_ENTRY (modeentry), preferences.defaultmode);
			modeenum = mode2enum (preferences.defaultmode);
			if (modeenum >= 0)
			activate_modeoption_by_enum
				(modeoptionmenu, preferences.modes, modeenum);
		}
		if (strlen (preferences.defaulttxrst) > 0)
			if (strlen(gtk_editable_get_chars (GTK_EDITABLE (rstentry), 0, -1)) == 0)
			{
				if (keyerwindow && g_strrstr (preferences.defaulttxrst, "#"))
				{
					count = lookup_widget (keyerwindow, "count");
					c = gtk_spin_button_get_value (GTK_SPIN_BUTTON(count));
					if (c < 10)
						countstr = g_strdup_printf ("00%d", c);
					else if (c < 100)
						countstr = g_strdup_printf ("0%d", c);
					else
						countstr = g_strdup_printf ("%d", c);
					str = my_strreplace (preferences.defaulttxrst, "#", countstr);
					g_free (countstr);
					gtk_entry_set_text (GTK_ENTRY (rstentry), str);
					g_free (str);
				}
				else if (keyerwindow && g_strrstr (preferences.defaulttxrst, "~"))
				{
					lastmsg = get_last_msg ();
					str = my_strreplace (preferences.defaulttxrst, "~", lastmsg);
					g_free (lastmsg);
					gtk_entry_set_text (GTK_ENTRY (rstentry), str);
					g_free (str);
				}
				else
					gtk_entry_set_text (GTK_ENTRY (rstentry), preferences.defaulttxrst);
			}
		if (gtk_widget_get_visible (powerhbox) && strlen (preferences.defaultpower) > 0)
		{
			if (strlen(gtk_editable_get_chars (GTK_EDITABLE (powerentry), 0, -1)) == 0)
				gtk_entry_set_text (GTK_ENTRY (powerentry), preferences.defaultpower);
		}
	}

	/* set non-hamlib defaults */
	if (strlen (preferences.defaultrxrst) > 0)
	{
		myrstentry = lookup_widget (mainwindow, "myrstentry");
		if (strlen(gtk_editable_get_chars (GTK_EDITABLE (myrstentry), 0, -1)) == 0)
			gtk_entry_set_text (GTK_ENTRY (myrstentry), preferences.defaultrxrst);
	}
	if (strlen (preferences.defaultawards) > 0)
	{
		awardsentry = lookup_widget (mainwindow, "awardsentry");
		if (strlen(gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1)) == 0)
			gtk_entry_set_text (GTK_ENTRY (awardsentry), preferences.defaultawards);
	}
	unknown1hbox = lookup_widget (mainwindow, "unknown1hbox");
	if (gtk_widget_get_visible (unknown1hbox)	&& strlen (preferences.defaultfreefield1) > 0)
	{
		unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
		if (strlen(gtk_editable_get_chars (GTK_EDITABLE (unknownentry1), 0, -1)) == 0)
			gtk_entry_set_text (GTK_ENTRY (unknownentry1), preferences.defaultfreefield1);
	}
	unknown2hbox = lookup_widget (mainwindow, "unknown2hbox");
	if (gtk_widget_get_visible (unknown2hbox)	&& strlen (preferences.defaultfreefield2) > 0)
	{
		unknownentry2 = lookup_widget (mainwindow, "unknownentry2");
		if (strlen(gtk_editable_get_chars (GTK_EDITABLE (unknownentry2), 0, -1)) == 0)
			gtk_entry_set_text (GTK_ENTRY (unknownentry2), preferences.defaultfreefield2);
	}
	remarksvbox = lookup_widget (mainwindow, "remarksvbox");
	if (gtk_widget_get_visible (remarksvbox) && strlen (preferences.defaultremarks) > 0)
	{
		remtv = lookup_widget (mainwindow, "remtv");
		b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
		if (gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(b)) == 0)
			gtk_text_buffer_set_text (b, preferences.defaultremarks, -1);
	}

	callentry = lookup_widget (mainwindow, "callentry");
	gtk_widget_grab_focus (callentry);
	gtk_editable_set_position (GTK_EDITABLE(callentry), -1);
}

void
on_addclickall_activate (GtkMenuItem * menuitem, gpointer user_data)
{
        GtkWidget *writemenu, *clickallmenu;

	// This part does the write (equivalent to Ctrl-W)
	writemenu = gtk_ui_manager_get_widget 
	  (ui_manager, "/MainMenu/EditMenu/Write"); 
	g_signal_emit_by_name (G_OBJECT (writemenu), "activate"); 

	// This part does the Click All (equivalent to Ctrl-K)
 	clickallmenu = gtk_ui_manager_get_widget 
 		(ui_manager, "/MainMenu/EditMenu/Click All"); 
 	g_signal_emit_by_name (G_OBJECT (clickallmenu), "activate"); 
}

/* show/hide the toolbar */
void
on_view_toolbar_activate (GtkAction *action, gpointer user_data)
{
	GtkWidget *handlebox;
	gboolean status;
	
	handlebox = lookup_widget (mainwindow, "handlebox");
	status = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	if (status) gtk_widget_show (handlebox);
		else gtk_widget_hide (handlebox);
}

/* sort function for the date column */
static gint
sort_by_date_time (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
  gchar *date_a, *date_b, *time_a, *time_b, *dt_a, *dt_b, *res = NULL;
  gint result = 0;
	struct tm tm_a, tm_b;
	char buf[255];

  gtk_tree_model_get (model, a, DATE, &date_a, -1);
  gtk_tree_model_get (model, a, GMT, &time_a, -1);
  gtk_tree_model_get (model, b, DATE, &date_b, -1);
  gtk_tree_model_get (model, b, GMT, &time_b, -1);

	/* uses the current locale */
	res = strptime (date_a, "%d %b %Y", &tm_a);
	if (res == NULL)
		g_warning (_("Could not read %s with your current locale"), date_a);
	res = strptime (date_b, "%d %b %Y", &tm_b);
	if (res == NULL)
		g_warning (_("Could not read %s with your current locale"), date_b);

	/* yyyydddhhmm, where ddd is day of the year */
	if (res != NULL)
	{
		strftime (buf, sizeof(buf), "%Y%j", &tm_a);
		dt_a = g_strdup_printf ("%s%s", buf, time_a);
		strftime (buf, sizeof(buf), "%Y%j", &tm_b);
		dt_b = g_strdup_printf ("%s%s", buf, time_b);
	}
	else
	{ /* fallback which does not sort very well */
		dt_a = g_strdup_printf ("%s%s", date_a, time_a);
		dt_b = g_strdup_printf ("%s%s", date_b, time_b);
	}
	result = strcmp (dt_a, dt_b);
	g_free (dt_a);
	g_free (dt_b);

  return -result;
}

void
on_sort_log_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	logtype *logw;
	gint page;
	GtkTreeViewColumn *column;
	GtkTreeModel *model;
	gchar *label;

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook));
	if (page >= 0)
	{ /* sort the log and make column unclickable */
		logw = g_list_nth_data (logwindowlist, page);
		column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), DATE);
		gtk_tree_view_column_set_sort_column_id (column, DATE);
		model = gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview));
		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (model), DATE, sort_by_date_time, NULL, NULL);
		gtk_tree_view_column_clicked (column);
		logw->logchanged = TRUE;
		label = g_strdup_printf ("<b>%s*</b>", logw->logname);
		gtk_label_set_markup (GTK_LABEL (logw->label), label);
		g_free (label);
		gtk_tree_view_column_set_clickable (column, FALSE);
		update_statusbar (_("Log sorted"));
	}
}
