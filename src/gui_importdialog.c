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

/* gui_importlogdialog.c - creation and destruction of the importlog dialog 
 */

#include <gtk/gtk.h>
#include <string.h>
#include <glib/gstdio.h>

#include "gui_mergedialog.h"
#include "support.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "gui_utils.h"
#include "main.h"
#include "dxcc.h"

extern GtkWidget *mainwindow, *scorewindow;
extern GtkWidget *mainnotebook;
extern preferencestype preferences;
extern programstatetype programstate;
extern GList *logwindowlist;

GtkWidget *importassistant;
static GtkWidget *previewscrolledwindow;

/* 
 * in case we open a non-xlog file, we have to know the xlog filename
 */
static gchar *find_logname (const gchar *selected)
{
	gchar *basen, *dirn, **split, *l;

	basen = g_path_get_basename (selected);
	dirn = g_path_get_dirname (selected);

	if (g_strrstr(basen, "."))
	{
		split = g_strsplit (basen, ".", 0);
		l = g_strconcat (dirn, G_DIR_SEPARATOR_S, split[0], ".xlog", NULL);
		g_strfreev (split);
	}
	else
	{
		l = g_strconcat (dirn, G_DIR_SEPARATOR_S, basen, ".xlog", NULL);
	}
	g_free (basen);
	g_free (dirn);
	return l;
}

/* callback for the preview widget, read the first 10 lines */
static void
preview_update (GtkFileChooser *chooser, gpointer data)
{
	GtkWidget *preview;
	gchar *filename, buf[80];
	gboolean have_preview = FALSE;
	gint i;
	FILE *in;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	preview = GTK_WIDGET (data);
	filename = gtk_file_chooser_get_preview_filename (chooser);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(preview));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_delete (buffer, &start, &end);

	in = g_fopen (filename, "r");
	if (in)
	{
		have_preview = TRUE;
		for (i = 0; i < 10; i++)
		{
			if (fgets (buf, 80, in) == NULL) 
				break;
			else if (g_utf8_validate (buf, -1, NULL))
				gtk_text_buffer_insert (buffer, &start, buf, -1);
		}
	}
	gtk_widget_set_size_request (previewscrolledwindow, 200, -1);
	gtk_file_chooser_set_preview_widget_active (chooser, have_preview);
}

static void
on_importlogbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *fileselection, *logentry, *preview;
	gchar *sellog;
	gint response;

	fileselection = gtk_file_chooser_dialog_new (_("xlog - select a log for import"),
    	GTK_WINDOW(mainwindow), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, 
		GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_modal (GTK_WINDOW(fileselection), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(fileselection), TRUE);

	gtk_file_chooser_set_current_folder
		(GTK_FILE_CHOOSER (fileselection), preferences.savedir);

	/* preview widget */
	previewscrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (previewscrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	preview = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW(preview), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(preview), FALSE);
	gtk_container_add (GTK_CONTAINER (previewscrolledwindow), preview);
	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (fileselection),
		previewscrolledwindow);
	gtk_widget_set_size_request (previewscrolledwindow, 0, -1);
	gtk_widget_show_all (previewscrolledwindow);
	g_signal_connect
		(fileselection, "update-preview", G_CALLBACK (preview_update), preview);

	gtk_widget_show_all (fileselection);
	response = gtk_dialog_run (GTK_DIALOG(fileselection));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		sellog = gtk_file_chooser_get_filename 
			(GTK_FILE_CHOOSER (fileselection));
		logentry = lookup_widget (importassistant, "logentry");
		gtk_entry_set_text (GTK_ENTRY(logentry), sellog);
		g_free (sellog);
	}
	gtk_widget_destroy (fileselection);
}

static void
on_wizard_cancel_clicked (GtkAssistant *assistant, gpointer user_data)
{
	gtk_widget_destroy (GTK_WIDGET(assistant));
	gtk_widget_set_sensitive (mainwindow, TRUE);
}

static void
on_wizard_close_clicked (GtkAssistant *assistant, gpointer user_data)
{
	LOGDB *lp;
	logtype *logw;
	GtkTreePath *path;
	gchar *temp;

	GtkWidget *newlogradiobutton = lookup_widget
		(importassistant, "newlogradiobutton");
	GtkWidget *logentry = lookup_widget (importassistant, "logentry");
	GtkWidget *combobox = lookup_widget (importassistant, "combobox");
	GtkWidget *rementry = lookup_widget (importassistant, "rementry");

	gboolean newlog = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (newlogradiobutton));
	gchar *log = gtk_editable_get_chars (GTK_EDITABLE (logentry), 0, -1);
	gint filetype = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));
	programstate.importremark =
		gtk_editable_get_chars (GTK_EDITABLE (rementry), 0, -1);

	/* check by filename and if log exist jump to the page in the notebook */
	if (newlog)
	{
		gchar *xlogfile = find_logname (log);
		gint i;
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logw = g_list_nth_data (logwindowlist, i);
			if (g_ascii_strcasecmp (xlogfile, logw->filename) == 0)
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK(mainnotebook), i);
				set_qsoframe (logw);
				update_statusbar (_("A log with the same name is already open"));
			 	gtk_widget_destroy (GTK_WIDGET(assistant));
				gtk_widget_set_sensitive (mainwindow, TRUE);
				return;
			}
		} 
		g_free (xlogfile);
	}
	programstate.qsos = 0;
	lp = log_file_open (log, filetype + 1);
	if (lp)
	{
		if (newlog)
		{
			gchar *logn = logname (g_path_get_basename (log));
			logw =	openlogwindow (lp, logn, programstate.logwindows++);
			gtk_notebook_set_current_page 
				(GTK_NOTEBOOK(mainnotebook), programstate.logwindows - 1);
			log_file_qso_foreach (lp, fillin_list, logw);
			log_file_close (lp);
			g_free (logn);
			logw->filename = g_strconcat
				(preferences.savedir, G_DIR_SEPARATOR_S, logw->logname, ".xlog", NULL);
			logw->logchanged = TRUE;
			gchar *label = g_strdup_printf ("<b>%s*</b>", logw->logname);
			gtk_label_set_markup (GTK_LABEL (logw->label), label);
			g_free (label);
			logwindowlist =	g_list_append (logwindowlist, logw);
			gtk_notebook_set_current_page 
				(GTK_NOTEBOOK(mainnotebook), programstate.logwindows - 1);
			set_qsoframe (logw);
			set_tabs_menu ();

			if (gtk_widget_get_visible(scorewindow))
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
			}

			path = gtk_tree_path_new_from_string ("0");
			gtk_tree_view_scroll_to_cell
				(GTK_TREE_VIEW (logw->treeview), path, NULL, TRUE, 0.5, 0.0);
			gtk_tree_path_free (path);

			set_font (preferences.logfont);
			temp = g_strdup_printf (_("%d QSO's imported"), programstate.qsos);
			update_statusbar (temp);
			g_free (temp);
		}
		else
		{
			gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
			if (page >= 0)
			{
				logw = g_list_nth_data (logwindowlist, page);
				log_file_qso_foreach (lp, fillin_list, logw);

				/* we enable all columns */
				GtkTreeViewColumn *column;
				gint j;
				for (j = 0; j < QSO_FIELDS; j++)
				{
					column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), j);
					gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
					logw->logfields[j] = j+1;
				}
				logw->logfields[QSO_FIELDS] = 0;
				logw->columns = 17;
				logw->logchanged = TRUE;
				gchar *label = g_strdup_printf ("<b>%s*</b>", logw->logname);
				gtk_label_set_markup (GTK_LABEL (logw->label), label);
				g_free (label);
				log_file_close (lp);
				set_qsoframe (logw);
				path = gtk_tree_path_new_from_string ("0");
				gtk_tree_view_scroll_to_cell
					(GTK_TREE_VIEW (logw->treeview), path, NULL, TRUE, 0.5, 0.0);
				gtk_tree_path_free (path);
				temp = g_strdup_printf (_("%d QSO's imported"), programstate.qsos);
				update_statusbar (temp);
				g_free (temp);
			}
		}
	}
	g_free (log);

 	gtk_widget_destroy (GTK_WIDGET(assistant));
	gtk_widget_set_sensitive (mainwindow, TRUE);
}

static void
on_logentry_changed (GtkEditable *editable, gpointer user_data)
{
	GtkWidget *vbox = GTK_WIDGET(user_data);
	gchar *str = gtk_editable_get_chars (editable, 0, -1);
	if (strlen(str) > 0)
	  gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), vbox, TRUE);
	else
	  gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), vbox, FALSE);
	g_free (str);
}

void
on_menu_import_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	gtk_widget_set_sensitive (mainwindow, FALSE);
	importassistant = gtk_assistant_new ();
	gtk_window_set_title (GTK_WINDOW (importassistant), _("xlog - import"));
	gtk_window_set_transient_for (GTK_WINDOW(importassistant), GTK_WINDOW(mainwindow));
	GtkWidget *label = gtk_label_new
		(_(
		"These pages will help you import your log. "
		"Formats supported are ADIF, EDI ( Electronic Data Interchange) and cabrillo, "
		"together with editest, oh1aa, tlf, trlog and twlog native logbook formats."
		));
	gtk_assistant_append_page (GTK_ASSISTANT(importassistant), label);
	gtk_assistant_set_page_title (GTK_ASSISTANT (importassistant), label, _("Welcome"));
	gtk_assistant_set_page_type
		(GTK_ASSISTANT(importassistant), label, GTK_ASSISTANT_PAGE_INTRO);
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), label, TRUE);

	GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
	gtk_assistant_append_page (GTK_ASSISTANT (importassistant), vbox);
	gtk_assistant_set_page_title (GTK_ASSISTANT (importassistant), vbox, _("Step 1"));
	gtk_assistant_set_page_type
		(GTK_ASSISTANT (importassistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
	label = gtk_label_new (_(
		"Select a log by clicking on the \"Open\" button. "
		"The logtype can be chosen with the combobox."
		));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	GtkWidget *logentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), logentry, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT (importassistant, logentry, "logentry");
	g_signal_connect (logentry, "changed", G_CALLBACK (on_logentry_changed), vbox);
	GtkWidget *logbutton = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_box_pack_start (GTK_BOX (hbox), logbutton, FALSE, FALSE, 0);
	g_signal_connect ((gpointer) logbutton, "clicked",
		G_CALLBACK (on_importlogbutton_clicked), NULL);
	GtkWidget *combobox = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (hbox), combobox, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT (importassistant, combobox, "combobox");
	/* Keep entries in same order as enum log_file_type in logfile/logfile.h */
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "twlog");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "adif v2");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "cabrillo v3");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "editest");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "trlog");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "EDI");
	gtk_combo_box_append_text (GTK_COMBO_BOX (combobox), "oh1aa");
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 1);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), vbox, FALSE);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_assistant_append_page (GTK_ASSISTANT (importassistant), vbox);
	gtk_assistant_set_page_title (GTK_ASSISTANT (importassistant), vbox, _("Step 2"));
	gtk_assistant_set_page_type
		(GTK_ASSISTANT (importassistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
	label = gtk_label_new (_(
		"Select the type of import"
		));
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	GtkWidget *newlogradiobutton =
			gtk_radio_button_new_with_label (NULL, _("Import as new log"));
	gtk_box_pack_start (GTK_BOX (vbox), newlogradiobutton, FALSE, FALSE, 5);
	GLADE_HOOKUP_OBJECT (importassistant, newlogradiobutton, "newlogradiobutton");
	GSList *logradiobutton_group = gtk_radio_button_get_group
			(GTK_RADIO_BUTTON (newlogradiobutton));
	GtkWidget *thislogradiobutton = gtk_radio_button_new_with_label
		(logradiobutton_group, _("Import into current log"));
	gtk_box_pack_start (GTK_BOX (vbox), thislogradiobutton, FALSE, FALSE, 5);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), vbox, TRUE);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_assistant_append_page (GTK_ASSISTANT (importassistant), vbox);
	gtk_assistant_set_page_title (GTK_ASSISTANT (importassistant), vbox, _("Step 3"));
	gtk_assistant_set_page_type
		(GTK_ASSISTANT (importassistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
	label = gtk_label_new (_(
		"When importing from cabrillo add the following remark. "
		"Leave empty if you don't want to have a remark added."
		));
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	GtkWidget *rementry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox), rementry, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT (importassistant, rementry, "rementry");
	if (strlen(programstate.importremark) > 0)
		gtk_entry_set_text (GTK_ENTRY (rementry), programstate.importremark);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), vbox, TRUE);

	label = gtk_label_new
		(_(
		"Clicking apply will now import your log. "
		"In case you import into an existing log, you should use xlog's "
		"sorting capabilties to sort the log. You can also use the log-editor "
		"to hide columns."
		));
	gtk_assistant_append_page (GTK_ASSISTANT (importassistant), label);
	gtk_assistant_set_page_type
                (GTK_ASSISTANT (importassistant), label, GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_title (GTK_ASSISTANT (importassistant), label, _("End"));
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (importassistant), label, TRUE);

	g_signal_connect (importassistant, "close",
		G_CALLBACK (on_wizard_close_clicked), NULL);
	g_signal_connect (importassistant, "cancel",
		G_CALLBACK (on_wizard_cancel_clicked), NULL);

	gtk_window_set_position (GTK_WINDOW(importassistant), GTK_WIN_POS_CENTER);
	gtk_widget_show_all (importassistant);
}
