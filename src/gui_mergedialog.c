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

/* gui_mergelogdialog.c - creation and destruction of the mergelog dialog 
 *
 * This dialog shows 2 entries where log names should be filled in, clicking
 * on OK will open them in one page. After that, the user can decide to sort
 * the log or not.
 */

#include <gtk/gtk.h>
#include <string.h>

#include "gui_mergedialog.h"
#include "support.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "gui_utils.h"
#include "main.h"

extern GtkWidget *mainwindow;
extern preferencestype preferences;
extern programstatetype programstate;
extern GList *logwindowlist;
extern GtkWidget *mainnotebook;
GtkWidget *mergelogdialog;

static void
on_mergelogbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *fileselection, *log1entry, *log2entry;
	GtkFileFilter *filter;
	gchar *sellog;
	gint response;

	fileselection = gtk_file_chooser_dialog_new (_("xlog - select a log"),
    	GTK_WINDOW(mainwindow), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, 
		GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_current_folder
		(GTK_FILE_CHOOSER (fileselection), preferences.savedir);
	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.xlog");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fileselection), filter);

	gtk_widget_show_all (fileselection);
	response = gtk_dialog_run (GTK_DIALOG(fileselection));
	if (response == GTK_RESPONSE_ACCEPT)
	{
		sellog = gtk_file_chooser_get_filename 
			(GTK_FILE_CHOOSER (fileselection));
		if (GPOINTER_TO_INT(user_data) == 1)
		{
			log1entry = lookup_widget (mergelogdialog, "log1entry");
			gtk_entry_set_text (GTK_ENTRY(log1entry), sellog);
		}
		else
		{
			log2entry = lookup_widget (mergelogdialog, "log2entry");
			gtk_entry_set_text (GTK_ENTRY(log2entry), sellog);
		}
		g_free (sellog);
	}
	gtk_widget_destroy (fileselection);
}

void
on_menu_merge_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *vbox,
		*label1, *log1entry, *log1hbox, *log1button, *hsep,
		*label2, *log2entry, *log2hbox, *log2button;
	gint response;
	gchar *log1, *log2, *logn, *logn1, *logn2, *temp;
	LOGDB *lp1, *lp2;
	logtype *logw;

	mergelogdialog = gtk_dialog_new_with_buttons (_("xlog - merge logs"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		_("_Merge"), GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size (GTK_WINDOW (mergelogdialog), 350, 180);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (mergelogdialog)->vbox), vbox);
	label1 = gtk_label_new (_("Select first log for merging"));
	gtk_box_pack_start (GTK_BOX (vbox), label1, FALSE, FALSE, 0);
	log1hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), log1hbox, FALSE, FALSE, 0);
	log1entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (log1hbox), log1entry, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (log1entry), TRUE);
	log1button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_widget_set_size_request (log1button, 100, -1);
	gtk_box_pack_start (GTK_BOX (log1hbox), log1button, FALSE, FALSE, 0);
	hsep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 10);
	label2 = gtk_label_new (_("Select second log for merging"));
	gtk_box_pack_start (GTK_BOX (vbox), label2, FALSE, FALSE, 0);
	log2hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), log2hbox, FALSE, FALSE, 0);
	log2entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (log2hbox), log2entry, TRUE, TRUE, 0);
	gtk_entry_set_activates_default (GTK_ENTRY (log2entry), TRUE);
	log2button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_widget_set_size_request (log2button, 100, -1);
	gtk_box_pack_start (GTK_BOX (log2hbox), log2button, FALSE, FALSE, 0);

	g_signal_connect ((gpointer) log1button, "clicked",
		G_CALLBACK (on_mergelogbutton_clicked), GINT_TO_POINTER(1));
	g_signal_connect ((gpointer) log2button, "clicked",
		G_CALLBACK (on_mergelogbutton_clicked), GINT_TO_POINTER(2));

	GLADE_HOOKUP_OBJECT (mergelogdialog, log1entry, "log1entry");
	GLADE_HOOKUP_OBJECT (mergelogdialog, log2entry, "log2entry");

	gtk_dialog_set_default_response (GTK_DIALOG (mergelogdialog),
		GTK_RESPONSE_OK);
	gtk_widget_show_all (mergelogdialog);
	response = gtk_dialog_run (GTK_DIALOG (mergelogdialog));
	if (response == GTK_RESPONSE_OK)
	{
		log1 = gtk_editable_get_chars (GTK_EDITABLE (log1entry), 0, -1);
		log2 = gtk_editable_get_chars (GTK_EDITABLE (log2entry), 0, -1);
		if ( (strlen(log1) > 0) && (strlen(log1) > 0) )
		{
			programstate.qsos = 0;
			lp1 = log_file_open (log1, TYPE_FLOG);
			lp2 = log_file_open (log2, TYPE_FLOG);
			if (lp1 && lp2)
			{
				logn1 = logname (g_path_get_basename (log1));
				logn2 = logname (g_path_get_basename (log2));
				logn = g_strdup_printf ("%s+%s", logn1, logn2);
				logw =	openlogwindow (lp1, logn, programstate.logwindows++);
				g_free (logn2);
				g_free (logn1);
				log_file_qso_foreach (lp1, fillin_list, logw);
				log_file_qso_foreach (lp2, fillin_list, logw);
				log_file_close (lp1);
				log_file_close (lp2);
				logw->filename =
	g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S, logn, ".xlog", NULL);
				logw->logchanged = TRUE;
				g_free (logn);
				logwindowlist =	g_list_append (logwindowlist, logw);
				gtk_notebook_set_current_page 
					(GTK_NOTEBOOK(mainnotebook), programstate.logwindows - 1);
				set_qsoframe (logw);
				set_tabs_menu ();
				temp = g_strdup_printf ("<b>%s*</b>", logw->logname);
				gtk_label_set_markup (GTK_LABEL (logw->label), temp);
				temp = g_strdup_printf
	(_("%d QSO's merged, the log may need sorting"), programstate.qsos);
				update_statusbar (temp);
				g_free (temp);
			}
		}
		g_free (log2);
		g_free (log1);
	}
	gtk_widget_destroy (mergelogdialog);
}
