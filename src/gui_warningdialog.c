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
 * gui_warningdialog.c - creation and destruction of the warning dialog
 *
 * a dialog with a label and a title.
 */

#include <gtk/gtk.h>
#include "gui_warningdialog.h"
#include "support.h"

extern GtkWidget *mainwindow;

void warningdialog (gchar *title, gchar *warning, gchar *dialogimage)
{
	GtkWidget *warningdialog, *warninglabel, *vbox, *image, *ok_button;
	gint response;

	warningdialog = gtk_dialog_new_with_buttons (title,
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	ok_button = gtk_dialog_add_button (GTK_DIALOG (warningdialog),
		GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_widget_grab_focus (ok_button);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (warningdialog)->vbox), vbox);
	image = gtk_image_new_from_stock
		(dialogimage, GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
	warninglabel = gtk_label_new (warning);
	gtk_misc_set_padding (GTK_MISC (warninglabel), 10, 10);
	gtk_label_set_line_wrap (GTK_LABEL (warninglabel), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), warninglabel, FALSE, FALSE, 0);
	gtk_widget_show_all (warningdialog);
	response = gtk_dialog_run (GTK_DIALOG(warningdialog));
	gtk_widget_destroy (warningdialog);
}
