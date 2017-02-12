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

/* gui_savedialog.c - creation and destruction of the davedialog 
 *
 * this dialog is shown when you exit xlog and one of the logs is not saved,
 * when you click OK, we exit xlog anyway.
 */
#include <gtk/gtk.h>

#include "support.h"
#include "utils.h"
#include "gui_savedialog.h"

extern GtkWidget *mainwindow;

void
create_savedialog (void)
{
	GtkWidget *savedialog, *vbox, *image, *label;
	gint response;

	savedialog = gtk_dialog_new_with_buttons (_("xlog - saving"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	vbox = gtk_vbox_new (TRUE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (savedialog)->vbox), vbox);
	image = gtk_image_new_from_stock
		("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
	label = gtk_label_new
(_("One of your logs has not been saved, do you want to quit anyway?"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_padding (GTK_MISC (label), 20, 20);
	gtk_dialog_set_default_response (GTK_DIALOG (savedialog),
					 GTK_RESPONSE_OK);
	gtk_widget_show_all (savedialog);
	response = gtk_dialog_run (GTK_DIALOG(savedialog));
	if (response == GTK_RESPONSE_OK)
	{
		save_windowsize_and_cleanup ();
		gtk_main_quit ();
	}
	gtk_widget_destroy (savedialog);
}

void
create_exitdialog (void)
{
	GtkWidget *exitdialog, *vbox, *image, *label;
	gint response;

	exitdialog = gtk_dialog_new_with_buttons (_("xlog - exiting"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	vbox = gtk_vbox_new (TRUE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (exitdialog)->vbox), vbox);
	image = gtk_image_new_from_stock
		("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
	label = gtk_label_new
(_("Are you sure you want to exit xlog?"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_padding (GTK_MISC (label), 20, 20);
	gtk_dialog_set_default_response (GTK_DIALOG (exitdialog),
					 GTK_RESPONSE_OK);
	gtk_widget_show_all (exitdialog);
	response = gtk_dialog_run (GTK_DIALOG(exitdialog));
	if (response == GTK_RESPONSE_OK)
	{
		save_windowsize_and_cleanup ();
		gtk_main_quit ();
	}
	gtk_widget_destroy (exitdialog);
}

