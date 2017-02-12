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
 * callbacks_mainwindow_list.c - callbacks for the list widget
 */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "callbacks_mainwindow_list.h"
#include "callbacks_mainwindow_qsoframe.h"
#include "support.h"
#include "cfg.h"
#include "xlog_enum.h"
#include "gui_utils.h"
#include "log.h"
#include "main.h"

extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;
extern preferencestype preferences;
extern GList *logwindowlist;

/* a row is selected, update the qso frame */
void
on_log_select_row (GtkTreeSelection *selection, gpointer user_data)
{
	GtkWidget *dateentry, *gmtentry, *endentry, *callentry, *rstentry,
		*myrstentry, *remtv, *bandentry, *modeentry, *outcheckbutton,
		*incheckbutton, *powerentry, *nameentry, *qthentry, *locatorentry,
		*unknownentry1, *unknownentry2, *modeoptionmenu, *bandoptionmenu,
		*qsoframe, *framelabel, *awardsentry;
	gchar *frametext, *entry, *temp;
	gint i, page, bandenum, modeenum;
	GtkTreeModel *model;
	GtkTreeIter iter;
	logtype *logw;
	GtkTextBuffer *b;

	dateentry = lookup_widget (mainwindow, "dateentry");
	gmtentry = lookup_widget (mainwindow, "gmtentry");
	endentry = lookup_widget (mainwindow, "endentry");
	callentry = lookup_widget (mainwindow, "callentry");
	bandentry = lookup_widget (mainwindow, "bandentry");
	modeentry = lookup_widget (mainwindow, "modeentry");
	rstentry = lookup_widget (mainwindow, "rstentry");
	myrstentry = lookup_widget (mainwindow, "myrstentry");
	awardsentry = lookup_widget (mainwindow, "awardsentry");
	remtv = lookup_widget (mainwindow, "remtv");
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
	outcheckbutton = lookup_widget (mainwindow, "outcheckbutton");
	incheckbutton = lookup_widget (mainwindow, "incheckbutton");
	powerentry = lookup_widget (mainwindow, "powerentry");
	nameentry = lookup_widget (mainwindow, "nameentry");
	qthentry = lookup_widget (mainwindow, "qthentry");
	locatorentry = lookup_widget (mainwindow, "locatorentry");
	unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
	unknownentry2 = lookup_widget (mainwindow, "unknownentry2");

	/* unselect selected QSO's in the other logs */
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		logw = g_list_nth_data (logwindowlist, i);
		if (page != i)
			gtk_tree_selection_unselect_all (gtk_tree_view_get_selection(GTK_TREE_VIEW (logw->treeview)));
	}

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, NR, &entry, -1);
		qsoframe = lookup_widget (mainwindow, "qsoframe");
		framelabel = gtk_frame_get_label_widget (GTK_FRAME(qsoframe));
		frametext = g_strdup_printf ("<b>QSO %s</b>", entry);
		gtk_label_set_markup (GTK_LABEL (framelabel), frametext);
		g_free (frametext);

		/* we don't need to convert to uppercase here */
		g_signal_handlers_block_by_func (GTK_OBJECT (awardsentry),
			on_awardsentry_insert_text, user_data);
		/* order matters, DXCC- string in the awards entry comes before callsign */
		gtk_tree_model_get (model, &iter, AWARDS, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (awardsentry), entry);
		g_signal_handlers_unblock_by_func (GTK_OBJECT (awardsentry),
			on_awardsentry_insert_text, user_data);

		gtk_tree_model_get (model, &iter, DATE, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (dateentry), entry);

		gtk_tree_model_get (model, &iter, GMT, &entry, -1);
		if (strlen(entry) > 4)
		{
			temp = g_strndup (entry, 4);
			gtk_entry_set_text (GTK_ENTRY (gmtentry), temp);
			g_free (temp);
		}
		else
		gtk_entry_set_text (GTK_ENTRY (gmtentry), entry);

		gtk_tree_model_get (model, &iter, GMTEND, &entry, -1);
		if (strlen(entry) > 4)
		{
			temp = g_strndup (entry, 4);
			gtk_entry_set_text (GTK_ENTRY (endentry), temp);
			g_free (temp);
		}
		else
		gtk_entry_set_text (GTK_ENTRY (endentry), entry);

		/* we don't need to convert to uppercase here */
		g_signal_handlers_block_by_func (GTK_OBJECT (callentry),
			on_callentry_insert_text, user_data);
		gtk_tree_model_get (model, &iter, CALL, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (callentry), entry);
		g_signal_handlers_unblock_by_func (GTK_OBJECT (callentry),
			on_callentry_insert_text, user_data);

		/* update bandoptionmenu and entry */
		gtk_tree_model_get (model, &iter, BAND, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (bandentry), entry);
		bandenum = freq2enum (entry);
		if (bandenum >= 0)
			activate_bandoption_by_enum
				(bandoptionmenu, preferences.bands, bandenum);

		/* update modeoptionmenu and entry */
		gtk_tree_model_get (model, &iter, MODE, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (modeentry), entry);
		modeenum = mode2enum (entry);
		if (modeenum >= 0)
			activate_modeoption_by_enum
				(modeoptionmenu, preferences.modes, modeenum);

		gtk_tree_model_get (model, &iter, POWER, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (powerentry), entry);
		gtk_tree_model_get (model, &iter, RST, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (rstentry), entry);
		gtk_tree_model_get (model, &iter, MYRST, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (myrstentry), entry);

		g_signal_handlers_block_by_func (GTK_OBJECT (awardsentry),
			on_callentry_insert_text, user_data);
		gtk_tree_model_get (model, &iter, AWARDS, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (awardsentry), entry);
		g_signal_handlers_unblock_by_func (GTK_OBJECT (awardsentry),
			on_callentry_insert_text, user_data);

		gtk_tree_model_get (model, &iter, QSLOUT, &entry, -1);
		if (!g_ascii_strcasecmp (entry, "x") || !g_ascii_strcasecmp (entry, "y"))
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outcheckbutton), TRUE);
		else
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outcheckbutton), FALSE);
		gtk_tree_model_get (model, &iter, QSLIN, &entry, -1);
		if (!g_ascii_strcasecmp (entry, "x") || !g_ascii_strcasecmp (entry, "y"))
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (incheckbutton), TRUE);
		else
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (incheckbutton), FALSE);
		gtk_tree_model_get (model, &iter, NAME, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (nameentry), entry);
		gtk_tree_model_get (model, &iter, QTH, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (qthentry), entry);
		gtk_tree_model_get (model, &iter, LOCATOR, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (locatorentry), entry);
		gtk_tree_model_get (model, &iter, U1, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (unknownentry1), entry);
		gtk_tree_model_get (model, &iter, U2, &entry, -1);
		gtk_entry_set_text (GTK_ENTRY (unknownentry2), entry);
		gtk_tree_model_get (model, &iter, REMARKS, &entry, -1);
		b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
		gtk_text_buffer_set_text (b, entry, -1);
	}
}
