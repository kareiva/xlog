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
 * gui_setupdialog.c - creation and destruction if the setupdialog
 *
 * when xlog is started for the very first time we display a message where 
 * the logs are saved.
 */

#include <gtk/gtk.h>

#include "gui_setupdialog.h"
#include "support.h"

extern GtkWidget *mainwindow;
extern gchar *xlogdir;

void show_setupdialog (void)
{
	GtkWidget *setupdialog, *setuplabel;
	GdkPixbuf *setupdialog_icon_pixbuf;
	gint response;
	gchar *labeltext;

	setupdialog = gtk_dialog_new_with_buttons (_("xlog - setup"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	setupdialog_icon_pixbuf = create_pixbuf ("xlog.png");
	if (setupdialog_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (setupdialog), setupdialog_icon_pixbuf);
		g_object_unref (setupdialog_icon_pixbuf);
	}
	labeltext = g_strdup_printf
			(_("Xlog has created %s, this is the directory where your "
					"log will be saved. You can change the default path "
					"for your logs in the preferences dialog.\n\n"
					"Now select Log -> New from the menu to create a log.\n\n"
					"Before you start logging, you should read the handbook "
					"by selecting Help -> Manual from the menu!"),
					xlogdir);
	setuplabel = gtk_label_new (labeltext);
	g_free (labeltext);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (setupdialog)->vbox), setuplabel);
	gtk_label_set_line_wrap (GTK_LABEL (setuplabel), TRUE);
	gtk_misc_set_padding (GTK_MISC (setuplabel), 10, 10);
	gtk_widget_show_all (setupdialog);
	response = gtk_dialog_run (GTK_DIALOG(setupdialog));
 	gtk_widget_destroy (setupdialog);
}
