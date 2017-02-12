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
 * callbacks_mainwindow_qsoframe.c - callbacks for the QSO frame, where all
 * the entry widgets are ...
 */

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>

#if HAVE_SYS_SHM_H
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <errno.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <hamlib/rig.h>

#include "callbacks_mainwindow_qsoframe.h"
#include "support.h"
#include "dxcc.h"
#include "cfg.h"
#include "xlog_enum.h"
#include "utils.h"
#include "gui_utils.h"
#include "log.h"
#include "wwl.h"
#include "gui_netkeyer.h"
#include "main.h"
#include "hamlib-utils.h"

/* defines and variables for shared memory support */
#define KEY 6146
#define SHMSIZE	40
void *shareCall;

extern GtkWidget *mainwindow, *b4window, *keyerwindow;
extern gint callid;
extern programstatetype programstate;
extern GList *logwindowlist;
extern preferencestype preferences;
extern GList *logwindowlist;

/* QSO FRAME */
/* get current date and fill in the dateentry */
void
on_datebutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry;
	gchar *nowdate;

	nowdate = xloggetdate ();
	entry = lookup_widget (mainwindow, "dateentry");
	gtk_entry_set_text (GTK_ENTRY (entry), nowdate);
	g_free (nowdate);
}

/* get current time and fill in the gmtentry */
void
on_gmtbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry;
	gchar *nowtime;

	entry = lookup_widget (mainwindow, "gmtentry");
	nowtime = xloggettime ();
	gtk_entry_set_text (GTK_ENTRY (entry), nowtime);
	g_free (nowtime);
}

void
on_endbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry;
	gchar *nowtime;

	entry = lookup_widget (mainwindow, "endentry");
	nowtime = xloggettime ();
	gtk_entry_set_text (GTK_ENTRY (entry), nowtime);
	g_free (nowtime);
}

/* convert callsign to uppercase */
void
on_callentry_insert_text (GtkEditable * editable, gchar * new_text,
	gint new_text_length, gpointer position, gpointer user_data)
{
	gint i;
	gchar *result = g_new (gchar, new_text_length);

	for (i = 0; i < new_text_length; i++)
result[i] = islower (new_text[i]) ? toupper (new_text[i]) : new_text[i];

/* insert the new callsign and block insert_text, so we don't call this twice */
	g_signal_handlers_block_by_func
		(GTK_OBJECT (editable),	on_callentry_insert_text, user_data);
	gtk_editable_insert_text (editable, result, new_text_length, position);
	g_signal_handlers_unblock_by_func
		(GTK_OBJECT (editable), on_callentry_insert_text, user_data);
	g_signal_stop_emission_by_name (GTK_OBJECT (editable), "insert_text");
	g_free (result);
}

/* convert awards entry to uppercase */
void
on_awardsentry_insert_text (GtkEditable * editable, gchar * new_text,
	gint new_text_length, gpointer position, gpointer user_data)
{
	gint i;
	gchar *result = g_new (gchar, new_text_length);

	for (i = 0; i < new_text_length; i++)
result[i] = islower (new_text[i]) ? toupper (new_text[i]) : new_text[i];

	g_signal_handlers_block_by_func
		(GTK_OBJECT (editable),	on_awardsentry_insert_text, user_data);
	gtk_editable_insert_text (editable, result, new_text_length, position);
	g_signal_handlers_unblock_by_func
		(GTK_OBJECT (editable), on_awardsentry_insert_text, user_data);
	g_signal_stop_emission_by_name (GTK_OBJECT (editable), "insert_text");
	g_free (result);
}

void
on_callbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry;

	entry = lookup_widget (mainwindow, "callentry");
	gchar *call = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (strlen(call) > 2)
	{
		if (g_strrstr (preferences.openurl, "<call>"))
		{
			gchar *link = g_strdup (preferences.openurl);
			link = my_strreplace (link, "<call>", call);
			open_url (link);
			g_free (link);
		}
	}
	g_free (call);
}

static void
updateb4window (gchar *callsign)
{
	GtkWidget *b4treeview;
	guint i, j;
	logtype *logw;
	gchar *logcallsign, **b4 = NULL;
	gboolean valid = FALSE;
	GtkTreeIter iter, b4iter;
	GtkTreeModel *model;
	GtkListStore *b4model = NULL;
	GtkTreeViewColumn *column;

	if (b4window && gtk_widget_get_visible (b4window))
	{
		/* clear the list */
		b4treeview = lookup_widget (b4window, "b4treeview");
		b4model = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW (b4treeview)));
		gtk_list_store_clear (GTK_LIST_STORE (b4model));

		if (strlen(callsign) < 2) return; /* for responsiveness */

		b4 = g_new0 (gchar *, QSO_FIELDS + 1);
		for (j = 0; j < QSO_FIELDS + 1; j++)
			b4[j] = g_new0 (gchar, 100);

		/* search all logs, if there is a match display it */
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logw = g_list_nth_data (logwindowlist, i);
			model = gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview));
			valid = gtk_tree_model_get_iter_first (model, &iter);
			while (valid)
			{
				gtk_tree_model_get (model, &iter, CALL, &logcallsign, -1);
 				if (!g_ascii_strncasecmp (callsign, logcallsign, strlen (callsign)))
				{
					b4[0] = g_strdup (logw->logname);
					for (j = 0; j < QSO_FIELDS; j++)
					{
						column = gtk_tree_view_get_column
							(GTK_TREE_VIEW(logw->treeview), j);
						gtk_tree_model_get (model, &iter, j, &b4[j+1], -1);
					}
					gtk_list_store_prepend
						(GTK_LIST_STORE(b4model), &b4iter);
					for (j = 0; j < QSO_FIELDS + 1; j++)
					if (strlen(b4[j]) > 0) /* for responsiveness */
						gtk_list_store_set
							(GTK_LIST_STORE(b4model), &b4iter, j, b4[j], -1);
				}
				valid = gtk_tree_model_iter_next (model, &iter);
			}
		}

		for (j = 0; j < QSO_FIELDS + 1; j++)
			g_free (b4[j]);
		g_free (b4);
	}
}

void
on_callentry_changed (GtkEditable * editable, gpointer user_data)
{
	gchar *call;
	guint st, zone, cont, iota;
#if HAVE_SYS_SHM_H
	gchar *errorstr;
#endif

	call = gtk_editable_get_chars (GTK_EDITABLE (editable), 0, -1);

	GtkWidget *awardsentry = lookup_widget (mainwindow, "awardsentry");
	gchar *aw = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
	gchar *result = valid_awards_entry (aw, &st, &zone, &cont, &iota);
	if (!result)
		updatedxccframe (call, FALSE, st, zone, cont, iota);
	else
		g_free (result);
	g_free (aw);
	updateb4window (call);

	/* twpsk support, if no IPC ID yet, create one */
#if HAVE_SYS_SHM_H
	if (programstate.shmid == -1)
	{
		if ((programstate.shmid = shmget (KEY, SHMSIZE, IPC_CREAT | 0600)) < 0)
		{
			errorstr = g_strdup_printf
				(_("shmget failed: %s"), g_strerror (errno));
			update_statusbar (errorstr);
			g_free (errorstr);
		}
		if ((shareCall = (shmat (programstate.shmid, NULL, 0))) == (gchar *) - 1)
		{
			errorstr = g_strdup_printf
				(_("shmat failed: %s"), g_strerror (errno));
			update_statusbar (errorstr);
			g_free (errorstr);
			programstate.shmid = -1;
		}
	}
	if (programstate.shmid != -1)
		strncpy (shareCall, call, SHMSIZE);	/* put call in shm */
#endif

	g_free (call);
}

gboolean
	on_callentry_unfocus (GtkEntry *callentry, GdkEventFocus *event, 
		gpointer user_data)
{
	guint i;
	GtkWidget *remarksvbox, *remtv, *namehbox, *nameentry,
		*qthhbox, *qthentry, *locatorhbox, *locatorentry;
	gboolean valid = FALSE, found = FALSE;
	gchar *logcallsign, *logname, *logqth, *logloc, *logremarks, *entry;
	const char *call;
	GtkTextBuffer *b;
	logtype *logw;
	GtkTreeModel *model;
	GtkTreeIter iter;

	if (preferences.typeaheadfind == 1) {
	call = gtk_entry_get_text (callentry);
	if (strlen(call) > 2)
	{
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logw = g_list_nth_data (logwindowlist, i);
			model = gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview));
			valid = gtk_tree_model_get_iter_first (model, &iter);
			while (valid)
			{
				gtk_tree_model_get (model, &iter, CALL, &logcallsign, -1);
				if (!g_ascii_strcasecmp (call, logcallsign))
				{
					gtk_tree_model_get (model, &iter, NAME, &logname, -1);
					gtk_tree_model_get (model, &iter, QTH, &logqth, -1);
 					gtk_tree_model_get (model, &iter, LOCATOR, &logloc, -1);
					gtk_tree_model_get (model, &iter, REMARKS, &logremarks, -1);
					i = g_list_length (logwindowlist) - 1;
					found = TRUE;
					break;
				}
				valid = gtk_tree_model_iter_next (model, &iter);
			}
		}
		if (found)
		{
			namehbox = lookup_widget(mainwindow, "namehbox");
			if (gtk_widget_get_visible (namehbox))
			{
				nameentry = lookup_widget(mainwindow, "nameentry");
				entry = gtk_editable_get_chars (GTK_EDITABLE (nameentry), 0, -1);
				if (strlen (entry) == 0)
					gtk_entry_set_text (GTK_ENTRY (nameentry), logname);
				g_free (entry);
			}
			qthhbox = lookup_widget(mainwindow, "qthhbox");
			if (gtk_widget_get_visible (qthhbox))
			{
				qthentry = lookup_widget(mainwindow, "qthentry");
				entry = gtk_editable_get_chars (GTK_EDITABLE (qthentry), 0, -1);
				if (strlen (entry) == 0)
					gtk_entry_set_text (GTK_ENTRY (qthentry), logqth);
				g_free (entry);
			}
			locatorhbox = lookup_widget(mainwindow, "locatorhbox");
			if (gtk_widget_get_visible (locatorhbox))
			{
				locatorentry = lookup_widget(mainwindow, "locatorentry");
				entry = gtk_editable_get_chars (GTK_EDITABLE (locatorentry), 0, -1);
				if (strlen (entry) == 0)
					gtk_entry_set_text (GTK_ENTRY (locatorentry), logloc);
				g_free (entry);
			}
			remarksvbox = lookup_widget(mainwindow, "remarksvbox");
			if (strlen (preferences.defaultremarks) == 0
				&& gtk_widget_get_visible (remarksvbox))
			{
				remtv = lookup_widget(mainwindow, "remtv");
				b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
				if (gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER(b)) == 0)
					gtk_text_buffer_set_text (b, logremarks, -1);
			}
		}
	}}
	return FALSE;
}

void
on_awardsentry_changed (GtkEditable * editable, gpointer user_data)
{
	guint st, zone, cont, iota;

	gchar *aw = gtk_editable_get_chars (editable, 0, -1);
	gchar *result = valid_awards_entry (aw,  &st, &zone, &cont, &iota);
	if (result)
	{
		updatedxccframe (result, TRUE, st, zone, cont, iota);
		g_free (result);
	}
	else
	{
		GtkWidget *callentry = lookup_widget (mainwindow, "callentry");
		gchar *call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
		updatedxccframe (call, FALSE, st, zone, cont, iota);
		g_free (call);
	}
	g_free (aw);
}

/* check for polling, if it is enabled use the state struct,
	 otherwise get frequency from the rig when there is no
	 default entry */
void
on_mhzbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry, *bandoptionmenu;
	GString *digits = g_string_new ("");
	guint bandenum;

	entry = lookup_widget (mainwindow, "bandentry");
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");

	/* set optionmenu and entry to the default */
	if (strlen (preferences.defaultmhz) > 0)
	{
		gtk_entry_set_text (GTK_ENTRY (entry), preferences.defaultmhz);
		bandenum = freq2enum (preferences.defaultmhz);
		if (bandenum >= 0)
			activate_bandoption_by_enum
				(bandoptionmenu, preferences.bands, bandenum);
	}
	else /* there is no default */
	{
		if (programstate.rigfrequency != 0)
			digits = convert_frequency ();
		else
			g_string_printf (digits, "UNKNOWN");
		/* set entry and optionmenu */
		gtk_entry_set_text (GTK_ENTRY (entry), digits->str);
		bandenum = freq2enum (digits->str);
		if (bandenum >= 0)
			activate_bandoption_by_enum
				(bandoptionmenu, preferences.bands, bandenum);
		g_string_free (digits, TRUE);
	}
}

void
on_modebutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry, *modeoptionmenu;
	gchar *mode;
	guint modeenum;

	entry = lookup_widget (mainwindow, "modeentry");
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");

	/* use default */
	if (strlen (preferences.defaultmode) > 0)
	{
		gtk_entry_set_text (GTK_ENTRY (entry), preferences.defaultmode);
		modeenum = mode2enum (preferences.defaultmode);
		if (modeenum >= 0)
			activate_modeoption_by_enum
				(modeoptionmenu, preferences.modes, modeenum);
	}
	else	/* NO default */
	{
		mode = rigmode (programstate.rigmode);
		gtk_entry_set_text (GTK_ENTRY (entry), mode);
		modeenum = mode2enum (mode);
		if (modeenum >= 0)
			activate_modeoption_by_enum
				(modeoptionmenu, preferences.modes, modeenum);
		g_free (mode);
	}
}

/* fill in the rst field, if the keyerwindow is active and the default rst
   contains the '#' macro, substitute it with the counter */
void
on_rstbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *rstentry, *count;
	gint c;
	gchar *str, *countstr, *lastmsg;

	rstentry = lookup_widget (mainwindow, "rstentry");
	if (strlen (preferences.defaulttxrst) > 0)
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
	else if (programstate.rigrst != 0)
		gtk_entry_set_text (GTK_ENTRY (rstentry), programstate.rigrst);
	else
		gtk_entry_set_text (GTK_ENTRY (rstentry), "UNKNOWN");
}

void
on_powerbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *entry;
	gchar *rigpower;

	entry = lookup_widget (mainwindow, "powerentry");
	if (strlen (preferences.defaultpower) > 0)
		gtk_entry_set_text (GTK_ENTRY (entry), preferences.defaultpower);
	else if (programstate.rigpower != 0)
		{
			rigpower = g_strdup_printf ("%d", programstate.rigpower / 1000);
			gtk_entry_set_text (GTK_ENTRY (entry), rigpower);
			g_free (rigpower);
		}
	else
		gtk_entry_set_text (GTK_ENTRY (entry), "UNKNOWN");
}


/* check locator and calculate distance and azimuth */
void
on_locatorentry_changed (GtkEditable * editable, gpointer user_data)
{
	gchar *locator;

	locator = gtk_editable_get_chars (GTK_EDITABLE (editable), 0, -1);
	updatelocatorframe (locator);
	g_free (locator);

	/* Update Locator Award */

	guint st, zone, cont, iota;
	GtkWidget *awardsentry = lookup_widget (mainwindow, "awardsentry");
	gchar *aw = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
	gchar *result = valid_awards_entry (aw,  &st, &zone, &cont, &iota);
	if (result)
	{
		updatedxccframe (result, TRUE, st, zone, cont, iota);
		g_free (result);
	}
	else
	{
		GtkWidget *callentry = lookup_widget (mainwindow, "callentry");
		gchar *call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
		updatedxccframe (call, FALSE, st, zone, cont, iota);
		g_free (call);
	}
	g_free (aw);
}

void
tv_changed (GtkTextBuffer *buffer, gpointer user_data)
{
	gint count;

	count = gtk_text_buffer_get_char_count (buffer);
	if (count > 512)
	{
		update_statusbar (_("Warning, remarks line to long!!"));
		gdk_beep ();
	}
}

/* entry get's focus because Alt+key is used */
void
entry_mnemonic_activate (GtkWidget *entry, gboolean arg1, gpointer user_data)
{
	gchar *temp;

	if (GPOINTER_TO_INT(user_data) == 1)
		temp = g_strdup (preferences.defaultmhz);
	else if (GPOINTER_TO_INT(user_data) == 2)
		temp = g_strdup (preferences.defaultmode);
	else if (GPOINTER_TO_INT(user_data) == 3)
		temp = g_strdup (preferences.defaulttxrst);
	else if (GPOINTER_TO_INT(user_data) == 4)
		temp = g_strdup (preferences.defaultrxrst);
	else if (GPOINTER_TO_INT(user_data) == 5)
		temp = g_strdup (preferences.defaultawards);
	else if (GPOINTER_TO_INT(user_data) == 6)
		temp = g_strdup (preferences.defaultpower);
	else if (GPOINTER_TO_INT(user_data) == 7)
		temp = g_strdup (preferences.defaultfreefield1);
	else if (GPOINTER_TO_INT(user_data) == 8)
		temp = g_strdup (preferences.defaultfreefield2);
	else
		temp = g_strdup ("?");
	if (g_ascii_strcasecmp (temp, "?"))
		gtk_entry_set_text (GTK_ENTRY(entry), temp);
	gtk_widget_grab_focus (entry);
	gtk_editable_set_position (GTK_EDITABLE(entry), -1);
	g_free (temp);
}

/* textview get's focus because Alt+key is used */
void
tv_mnemonic_activate (GtkWidget *tv, gboolean arg1, gpointer user_data)
{
	if (strlen (preferences.defaultremarks) > 0)
	{
		GtkTextBuffer *b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv));
		gtk_text_buffer_set_text (b, preferences.defaultremarks, -1);
		gtk_widget_grab_focus (tv);
	}
}

/* bandcombobox get's focus because Alt+m is used */
void
bandoptionactivate (GtkWidget *bandoptionmenu, gboolean arg1, gpointer user_data)
{
	guint bandenum;

	if (strlen (preferences.defaultmhz) > 0)
	{
		bandenum = freq2enum (preferences.defaultmhz);
		if (bandenum >= 0)
			activate_bandoption_by_enum
				(bandoptionmenu, preferences.bands, bandenum);
		gtk_widget_grab_focus (bandoptionmenu);
	}
}


/* modecombobox get's focus because Alt+o is used */
void
modeoptionactivate (GtkWidget *modeoptionmenu, gboolean arg1, gpointer user_data)
{
	guint modeenum;

	if (strlen (preferences.defaultmode) > 0)
	{
		modeenum = mode2enum (preferences.defaultmode);
		if (modeenum >= 0)
			activate_modeoption_by_enum
				(modeoptionmenu, preferences.modes, modeenum);
		gtk_widget_grab_focus (modeoptionmenu);
	}
}
