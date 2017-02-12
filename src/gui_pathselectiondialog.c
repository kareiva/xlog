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
 * gui_pathselectiondialog.c - creation and destruction of the path selection dialog
 *
 * this dialog used by the preferences dialog. You can select a path where your logs
 * are saved. If OK is clicked, the entry in the preferences dialog is updated. The
 * actual path is set when OK is clicked in the preferences dialog.
 */

#include <gtk/gtk.h>

#include "gui_pathselectiondialog.h"
#include "support.h"

extern GtkWidget *preferencesdialog, *mainwindow;

/* fill in entries in the preferences dialog, depending on the directory
   which is selected here */
void
on_pathbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *pathselection, *pathentry, *backupentry;
	gint response, which;
	gchar *selpath;

	which = GPOINTER_TO_INT (user_data);
	pathselection = gtk_file_chooser_dialog_new (_("xlog - select a directory"),
    	GTK_WINDOW(mainwindow), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, 
		GTK_RESPONSE_OK, NULL);

	gtk_window_set_modal (GTK_WINDOW(pathselection), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(pathselection), TRUE);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pathselection), 
		g_get_home_dir ());
	gtk_widget_show_all (pathselection);
	response = gtk_dialog_run (GTK_DIALOG(pathselection));
	if (response == GTK_RESPONSE_OK)
	{
		selpath = gtk_file_chooser_get_filename 
			(GTK_FILE_CHOOSER (pathselection));
		if (which == 1)
		{
			pathentry = lookup_widget (preferencesdialog, "pathentry");
			gtk_entry_set_text (GTK_ENTRY (pathentry), selpath);
		}
		else
		{
			backupentry = lookup_widget (preferencesdialog, "backupentry");
			gtk_entry_set_text (GTK_ENTRY (backupentry), selpath);
		}
		g_free (selpath);
	}
	gtk_widget_destroy (pathselection);
}
