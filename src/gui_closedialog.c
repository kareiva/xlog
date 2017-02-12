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

/* gui_closedialog.c - creation and destruction of the close dialog 
 *
 * create a warning dialog if a log is changed and we want to close it.
 */
 
#include <gtk/gtk.h>
#include <stdio.h>

#include "gui_closedialog.h"
#include "support.h"
#include "log.h"
#include "gui_utils.h"
#include "main.h"
#include "dxcc.h"
#include "cfg.h"

extern GtkWidget *mainwindow, *scorewindow;
extern GtkWidget *mainnotebook;
extern GList *logwindowlist;
extern programstatetype programstate;
extern preferencestype preferences;

void
on_menu_close_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *closedialog, *label, *image, *vbox;
	logtype *logwindow;
	gint page, response;

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
	if (page >= 0)
	{
		logwindow = g_list_nth_data (logwindowlist, page);
		if (logwindow->logchanged)
		{
			closedialog = gtk_dialog_new_with_buttons (_("xlog - closing"),
				GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
			vbox = gtk_vbox_new (TRUE, 0);
			gtk_container_add
				(GTK_CONTAINER (GTK_DIALOG (closedialog)->vbox), vbox);
			image = gtk_image_new_from_stock
				("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
			gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
			label = gtk_label_new
	(_("This log has not been saved, do you want to close it anyway?"));
			gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
			gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
			gtk_misc_set_padding (GTK_MISC (label), 20, 20);
			gtk_widget_show_all (closedialog);
			response = gtk_dialog_run (GTK_DIALOG(closedialog));
			if (response == GTK_RESPONSE_OK)
			{
				logwindowlist = g_list_remove (logwindowlist, logwindow);
				g_free (logwindow->logname);
				g_free (logwindow->filename);
				g_free (logwindow);
				gtk_notebook_remove_page (GTK_NOTEBOOK (mainnotebook), page);
				set_tabs_menu ();
				programstate.logwindows--;
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
			}
			gtk_widget_destroy (closedialog);
		}
		else
		{
			logwindowlist = g_list_remove (logwindowlist, logwindow);
			g_free (logwindow->logname);
			g_free (logwindow->filename);
			g_free (logwindow);
			gtk_notebook_remove_page (GTK_NOTEBOOK (mainnotebook), page);
			set_tabs_menu ();
			programstate.logwindows--;
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
		}
	}
}
