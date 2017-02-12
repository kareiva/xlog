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

/* gui_exportdialog.c - creation and destruction of the exportdialog 
 *
 * this shows a savedialog with an optionmenu added for the supported filetypes:
 * adif v2, cabrillo v3, EDI, trlog (tlf) and tab separated value (TSV) for
 * glabels. There are also 2 entries which you can use to save part of the log.
 * We do a check if the first QSO number is lower than the last and display a
 * warning dialog in case we overwrite a previously created log.
 */
 

#include <gtk/gtk.h>
#include <stdlib.h>

#include "cfg.h"
#include "log.h"
#include "gui_exportdialog.h"
#include "support.h"
#include "utils.h"

extern GList *logwindowlist;
extern GtkWidget *mainnotebook;
extern GtkWidget *mainwindow;
extern preferencestype preferences;

void
on_menu_export_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	gint index, page, export_response, exportwarning_response;
	GtkWidget *exportdialog, *filemenu, *hbox, 
		*label, *qsolabel, *lowqsoentry, *highqsoentry, *dash,
		*exportwarningdialog, *exportwarninglabel;
	logtype *logwindow;
	gchar *temp, *selection;
	gchar *sellog;
	gint type;
	gint highqso, lowqso;


	page = gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook));
	if (page >= 0)
	{
		exportdialog = gtk_file_chooser_dialog_new (_("xlog - export"),
		  GTK_WINDOW(mainwindow), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, 
		  GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

		gtk_window_set_modal (GTK_WINDOW(exportdialog), TRUE);
		gtk_window_set_destroy_with_parent (GTK_WINDOW(exportdialog), TRUE);

		/* optionmenu for the file types */
		hbox = gtk_hbox_new (FALSE, 0);
		label = gtk_label_new (_("Type of log:"));
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
		filemenu = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (hbox), filemenu, FALSE, FALSE, 15);
		gtk_combo_box_append_text (GTK_COMBO_BOX (filemenu), "adif v2");
		gtk_combo_box_append_text (GTK_COMBO_BOX (filemenu), "cabrillo v3");
		gtk_combo_box_append_text (GTK_COMBO_BOX (filemenu), "EDI");
		gtk_combo_box_append_text (GTK_COMBO_BOX (filemenu), "TSV");
		gtk_combo_box_set_active (GTK_COMBO_BOX (filemenu), 0);

		qsolabel = gtk_label_new (_("Export QSO:"));
		gtk_box_pack_start (GTK_BOX (hbox), qsolabel, FALSE, FALSE, 15);
		lowqsoentry = gtk_entry_new ();
		gtk_widget_set_size_request (lowqsoentry, 60, -1);
		gtk_box_pack_start (GTK_BOX (hbox), lowqsoentry, FALSE, FALSE, 15);
		dash = gtk_label_new (_("to"));
		gtk_box_pack_start (GTK_BOX (hbox), dash, FALSE, FALSE, 15);
		highqsoentry = gtk_entry_new ();
		gtk_widget_set_size_request (highqsoentry, 60, -1);
		gtk_box_pack_start (GTK_BOX (hbox), highqsoentry, FALSE, FALSE, 15);
		
		logwindow = g_list_nth_data (logwindowlist, page);

		temp = g_strdup_printf ("1");
		gtk_entry_set_text (GTK_ENTRY (lowqsoentry), temp);
		temp = g_strdup_printf ("%d", logwindow->qsos);
		gtk_entry_set_text (GTK_ENTRY (highqsoentry), temp);
		g_free (temp);

		gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(exportdialog), hbox);
		gtk_widget_show_all (hbox);

		selection = g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S, NULL);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (exportdialog), selection);
		g_free (selection);
		gtk_dialog_set_default_response (GTK_DIALOG (exportdialog),
						 GTK_RESPONSE_OK);

		export_response = gtk_dialog_run (GTK_DIALOG (exportdialog));
		if (export_response == GTK_RESPONSE_OK)
		{
			index = gtk_combo_box_get_active (GTK_COMBO_BOX(filemenu));
			switch (index)
			{
			case 0:
				type = TYPE_ADIF2;
				break;
			case 1:
				type = TYPE_CABRILLO3;
				break;
			case 2:
				type = TYPE_EDI;
				break;
			case 3:
				type = TYPE_LABELS;
				break;
			default:
				type = 0;
			}

			sellog = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (exportdialog));
			temp = gtk_editable_get_chars (GTK_EDITABLE (lowqsoentry), 0, -1);
			lowqso = atoi (temp);
			temp = gtk_editable_get_chars (GTK_EDITABLE (highqsoentry), 0, -1);
			highqso = atoi (temp);
			logwindow = g_list_nth_data (logwindowlist,
				gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook)));
			if ((lowqso > highqso) ||( highqso > logwindow->qsos))
				update_statusbar(_("Cannot export log: invalid entry for first and/or last QSO"));
			else if (!g_file_test(sellog, G_FILE_TEST_EXISTS))
			{
				savelog (logwindow, sellog, type, lowqso, highqso);
				temp = g_strdup_printf (_("Log exported to %s"), sellog);
				update_statusbar (temp);
			}
			else
			{
				exportwarningdialog = gtk_dialog_new_with_buttons
					(_("xlog - exporting"),
					GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
				temp = g_strdup_printf
					(_("%s already exist, do want to overwrite it?"), sellog);
				exportwarninglabel = gtk_label_new (temp);
				gtk_container_add (GTK_CONTAINER
					(GTK_DIALOG (exportwarningdialog)->vbox), exportwarninglabel);
				gtk_label_set_line_wrap (GTK_LABEL (exportwarninglabel), TRUE);
				gtk_misc_set_padding (GTK_MISC (exportwarninglabel), 20, 20);
				gtk_widget_show_all (exportwarningdialog);
				exportwarning_response = gtk_dialog_run
					(GTK_DIALOG(exportwarningdialog));

				if (exportwarning_response == GTK_RESPONSE_OK)
				{
					logwindow = g_list_nth_data (logwindowlist,
						gtk_notebook_get_current_page(GTK_NOTEBOOK (mainnotebook)));
					if ((lowqso > highqso) || (highqso > logwindow->qsos))
					  update_statusbar (_("Cannot export log: invalid entry for first and/or last QSO"));
					else
					{
						savelog (logwindow, sellog, type, lowqso, highqso);
						temp = g_strdup_printf (_("Log exported to %s"), sellog);
						update_statusbar (temp);
					}
				}
				gtk_widget_destroy (exportwarningdialog);
			}
			g_free (sellog);
			g_free (temp);
		}
	gtk_widget_destroy (exportdialog);
	}
}
