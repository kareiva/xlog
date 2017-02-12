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

/* gui_saveasdialog.c - creation and destruction of the saveasdialog 
 *
 * this shows a savedialog with 2 entries which you can use to save part of the log.
 * We do a check if the first QSO number is lower than the last and display a
 * warning dialog in case we overwrite a previously created log.
 */
 

#include <gtk/gtk.h>
#include <stdlib.h>

#include "cfg.h"
#include "log.h"
#include "gui_saveasdialog.h"
#include "support.h"
#include "utils.h"

extern GList *logwindowlist;
extern GtkWidget *mainnotebook;
extern GtkWidget *mainwindow;
extern preferencestype preferences;

void
on_menu_saveas_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	gint page, saveas_response, savewarning_response;
	GtkWidget *saveasdialog, *hbox,  *qsolabel, *lowqsoentry, *highqsoentry, *dash,
		*savewarningdialog, *savewarninglabel;
	logtype *logwindow;
	gchar *temp, *selection;
	gchar *sellog;
	gint highqso, lowqso;


	page = gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook));
	if (page >= 0)
	{
		saveasdialog = gtk_file_chooser_dialog_new (_("xlog - save as"),
    	GTK_WINDOW(mainwindow), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, 
		GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

		gtk_window_set_modal (GTK_WINDOW(saveasdialog), TRUE);
		gtk_window_set_destroy_with_parent (GTK_WINDOW(saveasdialog), TRUE);

		/* optionmenu for the file types */
		hbox = gtk_hbox_new (FALSE, 0);
		qsolabel = gtk_label_new (_("Save QSO:"));
		gtk_box_pack_start (GTK_BOX (hbox), qsolabel, TRUE, TRUE, 0);
		lowqsoentry = gtk_entry_new ();
		gtk_entry_set_activates_default (GTK_ENTRY (lowqsoentry), TRUE);
		gtk_box_pack_start (GTK_BOX (hbox), lowqsoentry, TRUE, TRUE, 0);
		dash = gtk_label_new (_("to"));
		gtk_box_pack_start (GTK_BOX (hbox), dash, TRUE, TRUE, 0);
		highqsoentry = gtk_entry_new ();
		gtk_entry_set_activates_default (GTK_ENTRY (highqsoentry), TRUE);
		gtk_box_pack_start (GTK_BOX (hbox), highqsoentry, TRUE, TRUE, 0);
		
		logwindow = g_list_nth_data (logwindowlist, page);

		temp = g_strdup_printf ("1");
		gtk_entry_set_text (GTK_ENTRY (lowqsoentry), temp);
		temp = g_strdup_printf ("%d", logwindow->qsos);
		gtk_entry_set_text (GTK_ENTRY (highqsoentry), temp);
		g_free (temp);

		gtk_dialog_set_default_response (GTK_DIALOG (saveasdialog),
						 GTK_RESPONSE_OK);
		gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(saveasdialog), hbox);
		gtk_widget_show_all (hbox);

		selection = g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S, NULL);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (saveasdialog), selection);
		g_free (selection);
		saveas_response = gtk_dialog_run (GTK_DIALOG (saveasdialog));
		if (saveas_response == GTK_RESPONSE_OK)
		{
			sellog = gtk_file_chooser_get_filename
				(GTK_FILE_CHOOSER (saveasdialog));
			temp = gtk_editable_get_chars (GTK_EDITABLE (lowqsoentry), 0, -1);
			lowqso = atoi (temp);
			temp = gtk_editable_get_chars (GTK_EDITABLE (highqsoentry), 0, -1);
			highqso = atoi (temp);
			logwindow = g_list_nth_data (logwindowlist,
				gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook)));
			if (lowqso > highqso || highqso > logwindow->qsos)
update_statusbar(_("Cannot save log: invalid entry for first and/or last QSO"));
			else if (!g_file_test(sellog, G_FILE_TEST_EXISTS))
			{
				savelog (logwindow, sellog, TYPE_FLOG, lowqso, highqso);
				temp = g_strdup_printf (_("Log saved to %s"), sellog);
				update_statusbar (temp);
			}
			else
			{
				savewarningdialog = gtk_dialog_new_with_buttons
					(_("xlog - saving"),
					GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
				temp = g_strdup_printf
					(_("%s already exist, do want to overwrite it?"), sellog);
				savewarninglabel = gtk_label_new (temp);
				gtk_container_add (GTK_CONTAINER
					(GTK_DIALOG (savewarningdialog)->vbox), savewarninglabel);
				gtk_label_set_line_wrap (GTK_LABEL (savewarninglabel), TRUE);
				gtk_misc_set_padding (GTK_MISC (savewarninglabel), 20, 20);
				gtk_widget_show_all (savewarningdialog);
				savewarning_response = gtk_dialog_run
					(GTK_DIALOG(savewarningdialog));

				if (savewarning_response == GTK_RESPONSE_OK)
				{
					logwindow = g_list_nth_data (logwindowlist,
						gtk_notebook_get_current_page(GTK_NOTEBOOK (mainnotebook)));
					if (lowqso > highqso || highqso > logwindow->qsos)
update_statusbar (_("Cannot save log: invalid entry for first and/or last QSO"));
					else
					{
						savelog (logwindow, sellog, TYPE_FLOG, lowqso, highqso);
						temp = g_strdup_printf (_("Log saved to %s"), sellog);
						update_statusbar (temp);
					}
				}
				gtk_widget_destroy (savewarningdialog);
			}
			g_free (sellog);
			g_free (temp);
		}
	gtk_widget_destroy (saveasdialog);
	}
}
