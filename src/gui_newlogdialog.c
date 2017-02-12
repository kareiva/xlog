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

/* gui_newlogdialog.c - creation and destruction of the newlog dialog 
 *
 * here we create a new log, if an existing logname is chosen, we open it.
 * If the logname is correct we fire up the logeditor, so we can choose
 * the fields which should be present in the log.
 */

#include <gtk/gtk.h>
#include <string.h>

#include "gui_newlogdialog.h"
#include "gui_logeditordialog.h"
#include "gui_warningdialog.h"
#include "callbacks_mainwindow_menu.h"
#include "support.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "gui_utils.h"
#include "main.h"

extern GList *logwindowlist;
extern GtkWidget *mainnotebook;
extern GtkWidget *mainwindow;
extern preferencestype preferences;
extern programstatetype programstate;
extern GtkUIManager *ui_manager;

/* check for correct characters */
static void
on_newlogentry_changed (GtkEditable * editable, gpointer user_data)
{
	gchar *entry, *end, *j;
	gint len, pos;
	
	entry = gtk_editable_get_chars (editable, 0, -1);
	if ((len = strlen(entry)) > 0)
	{
		end = entry + strlen (entry);
		for (j = entry; j < end; ++j)
		{
			switch (*j)
			{
				case '0' ... '9':
				case 'a' ... 'z':
				case 'A' ... 'Z':
				case '-':
				case '_':
					break;
				default:
					gdk_beep ();
					pos = gtk_editable_get_position (editable);
					gtk_editable_delete_text (editable, pos, pos+1);
					break;
			}
		}
	}
}

void
on_menu_new_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *newlogdialog, *vbox, *label, *newlogentry, *clearmenu;
	gchar *entry, *filename, *xlogbackupfile;
	logtype *logwindow = NULL;
	gint i, response;
	LOGDB *lp;
	gboolean logopen = FALSE, logexist = FALSE;

	newlogdialog = gtk_dialog_new_with_buttons (_("xlog - new log"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (newlogdialog)->vbox), vbox);
	label = gtk_label_new
(_("Enter a name for this log (a-z, A-Z, 0-9, dash(-) and underscore(_))"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	newlogentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox), newlogentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (newlogentry), 30);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_entry_set_activates_default (GTK_ENTRY (newlogentry), TRUE);

	g_signal_connect ((gpointer) newlogentry, "changed",
		G_CALLBACK (on_newlogentry_changed), NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (newlogdialog),
					 GTK_RESPONSE_OK);

	gtk_widget_show_all (newlogdialog);
	response = gtk_dialog_run (GTK_DIALOG (newlogdialog));
	programstate.utf8error = FALSE;
	if (response == GTK_RESPONSE_OK)
	{
		entry = gtk_editable_get_chars (GTK_EDITABLE (newlogentry), 0, -1);

		if (strlen (entry) > 0)
		{
			for (i = 0; i < g_list_length (logwindowlist); i++)
			{		/* see if the log is already open */
				logwindow = g_list_nth_data (logwindowlist, i);
				if (g_ascii_strcasecmp (entry, logwindow->logname) == 0)
				{
					gtk_notebook_set_current_page (GTK_NOTEBOOK(mainnotebook), i);
					set_qsoframe (logwindow);
					logopen = TRUE;
					update_statusbar (_("A log with the same name is already open"));
					break;
				}
			}
			if (!logopen)
			{		/* check if the log exists and open it */
				filename = g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S, entry, ".xlog", NULL);
				if ((g_file_test (filename, G_FILE_TEST_EXISTS)))
				{
					logexist = TRUE;
					lp = log_file_open (filename, TYPE_FLOG);
					if (lp)
					{
						logwindow =	openlogwindow (lp, entry, programstate.logwindows++);
						log_file_qso_foreach (lp, fillin_list, logwindow);
						log_file_close (lp);
						logwindow -> filename = g_strdup (filename);
						logwindowlist =	g_list_append (logwindowlist, logwindow);
						gtk_notebook_set_current_page(GTK_NOTEBOOK (mainnotebook), programstate.logwindows - 1);
						set_qsoframe (logwindow);
						set_tabs_menu ();
						xlogbackupfile = g_strconcat (filename, ".backup", NULL);
						savelog (logwindow, xlogbackupfile, TYPE_FLOG, 1, logwindow->qsos);
						g_free (xlogbackupfile);
						update_statusbar (_("A log with this name already exists"));
					}
					g_free (filename);
				}
			}
			if ((!logopen) && (!logexist))
			{		/* create a new log */
				filename = g_strconcat (preferences.savedir, G_DIR_SEPARATOR_S, entry, ".xlog", NULL);
				lp = log_file_create (filename, TYPE_FLOG, qso_field_nr, qso_fields, qso_widths);
				if (lp)
				{
					logwindow =	openlogwindow (lp, entry, programstate.logwindows++);
					log_file_close (lp);
					logwindow->filename = g_strdup (filename);
					logwindowlist =	g_list_append (logwindowlist, logwindow);
					gtk_notebook_set_current_page (GTK_NOTEBOOK(mainnotebook), programstate.logwindows - 1);
					set_qsoframe (logwindow);
					set_tabs_menu ();
					set_font (preferences.logfont);
				}	
				g_free (filename);
			}
			g_free (entry);
		}
		else
		{
			gtk_widget_destroy (newlogdialog);
			return;
		}
		clearmenu = gtk_ui_manager_get_widget
			(ui_manager, "/MainMenu/EditMenu/Clear All");
		g_signal_emit_by_name (G_OBJECT (clearmenu), "activate");
		gtk_widget_destroy (newlogdialog);

		if (logexist && programstate.utf8error) 
			warningdialog (_("xlog - open log"), 
				_("There were some errors converting from your locale to UTF8, "
				"which is used by GTK+ internally. It is best if you start xlog "
				"from a terminal and see what the errors are. Please check your "
				"language settings and your log fields!"), "gtk-dialog-warning");

		/* now fire up the logeditor */
		if ((!logopen) && (!logexist))
		{
			on_menu_logeditor_activate (NULL, NULL);
		}
	} /* cancel clicked */
	else
		gtk_widget_destroy (newlogdialog);
}
