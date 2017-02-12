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

/* gui_openlogdialog.c - creation and destruction of the logopen dialog
 *
 * this dialog let's you select any of the supported fileformats
 * (twlog, xlog, editest, trlog (tlf), EDI, adif and cabrillo), checks
 * if a filename is selected and opens the log. If a file is already opened,
 * we jump to the page in the log.
 */

#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "gui_openlogdialog.h"
#include "gui_warningdialog.h"
#include "cfg.h"
#include "log.h"
#include "support.h"
#include "utils.h"
#include "gui_utils.h"
#include "main.h"
#include "dxcc.h"

extern programstatetype programstate;
extern preferencestype preferences;
extern GtkWidget *mainwindow, *scorewindow;
extern GtkWidget *mainnotebook;
extern GList *logwindowlist;

gboolean ro;

static void
on_readonly_toggled (GtkToggleButton * togglebutton, gpointer user_data)
{
	ro = gtk_toggle_button_get_active (togglebutton);
}

void
on_menu_open_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *fileselection, *readonlycheckbutton;
	GtkFileFilter *filter;
	gchar *logn, *xlogbackupfile, *temp, *selection;
	gchar *sellog;
	gboolean logexist = FALSE;
	gint i, response;
	logtype *logwindow;
	LOGDB *lp;
	GtkTreePath *path;

	fileselection = gtk_file_chooser_dialog_new (_("xlog - open log"),
    	GTK_WINDOW(mainwindow), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, 
		GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);

	gtk_window_set_modal (GTK_WINDOW(fileselection), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(fileselection), TRUE);

	selection = g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S, NULL);
	gtk_file_chooser_set_current_folder
		(GTK_FILE_CHOOSER (fileselection), selection);
	g_free (selection);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.xlog");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fileselection), filter);

	readonlycheckbutton =
		gtk_check_button_new_with_label (_("Open log read-only"));
	gtk_file_chooser_set_extra_widget
		(GTK_FILE_CHOOSER(fileselection), readonlycheckbutton);
	g_signal_connect (G_OBJECT (readonlycheckbutton), "toggled",
                        G_CALLBACK (on_readonly_toggled), NULL);

	gtk_widget_show_all (fileselection);
	response = gtk_dialog_run (GTK_DIALOG(fileselection));

	programstate.utf8error = FALSE;
	if (response == GTK_RESPONSE_ACCEPT)
	{
		/* unselect any selected QSO */
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logwindow = g_list_nth_data (logwindowlist, i);
			gtk_tree_selection_unselect_all
				(gtk_tree_view_get_selection
					(GTK_TREE_VIEW (logwindow->treeview)));
		}

		sellog = gtk_file_chooser_get_filename
			(GTK_FILE_CHOOSER (fileselection));

		/* destroy the dialog now we have all the information */
		gtk_widget_destroy (fileselection);

		/* check by filename and if log exist jump to the page in the notebook */
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logwindow = g_list_nth_data (logwindowlist, i);
			if (g_ascii_strcasecmp (sellog, logwindow->filename) == 0)
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK(mainnotebook), i);
				set_qsoframe (logwindow);
				logexist = TRUE;
				update_statusbar
					(_("A log with the same name is already open"));
				break;
			}
		}

		if (!logexist)
		{
			programstate.qsos = 0;
			lp = log_file_open (sellog, TYPE_FLOG);
			if (lp)
			{
				logn = logname (g_path_get_basename (sellog));
				logwindow = openlogwindow (lp, logn, programstate.logwindows++);
				gtk_notebook_set_current_page 
					(GTK_NOTEBOOK(mainnotebook), programstate.logwindows - 1);
				log_file_qso_foreach (lp, fillin_list, logwindow);
				log_file_close (lp);
				g_free (logn);
				if (ro)	logwindow->readonly = TRUE;
				lp->type = TYPE_FLOG;
				if (((lp->type) == TYPE_FLOG) && !logwindow->readonly)
				{
					if (preferences.backup == 1)
			xlogbackupfile = g_strconcat (sellog, ".backup", NULL);
					else
			xlogbackupfile = g_strconcat
				(preferences.backupdir, G_DIR_SEPARATOR_S, 
				g_path_get_basename(sellog), ".backup", NULL);
					unlink (xlogbackupfile);
					backuplog (sellog, xlogbackupfile);
					g_free (xlogbackupfile);
				}
				logwindow->filename = g_strdup (sellog);
				logwindowlist =	g_list_append (logwindowlist, logwindow);
				set_qsoframe (logwindow);
				set_tabs_menu ();

				if (gtk_widget_get_visible(scorewindow))
				{
					fill_scoring_arrays ();
					update_dxccscoring ();
				}

				path = gtk_tree_path_new_from_string ("0");
					gtk_tree_view_scroll_to_cell
			(GTK_TREE_VIEW (logwindow->treeview), path, NULL, TRUE, 0.5, 0.0);
				gtk_tree_path_free (path);

				set_font (preferences.logfont);
				temp = g_strdup_printf (_("%d QSO's loaded"), programstate.qsos);
				update_statusbar (temp);
				g_free (temp);
			}
		}
		g_free (sellog);
	}
	else
		gtk_widget_destroy (fileselection);

	if (programstate.utf8error) 
		warningdialog (_("xlog - open log"), 
			_("There were some errors converting from your locale to UTF8, "
			"which is used by GTK+ internally. It is best if you start xlog "
			"from a terminal and see what the errors are. Please check your "
			"language settings and your log fields!"), "gtk-dialog-warning");
}
