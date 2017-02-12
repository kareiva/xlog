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

/* gui_defaultsdialog.c - creation and destruction of the defaults dialog
 *
 * a dialog is created where you can fill in default values to use for 
 * "click all" for the mhz, mode, rxrst, txrst, power, freefield1, freefield2
 * and remarks log entries.
 */

#include <gtk/gtk.h>
#include <string.h>

#include "support.h"
#include "cfg.h"

extern GtkWidget *mainwindow;
extern preferencestype preferences;

void
on_menu_defaults_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *defaultsmhzentry, *defaultsmodeentry, *defaultstxrstentry,
		*defaultsrxrstentry, *defaultsawardsentry, *defaultspowerentry, *defaultsdialog,
		*defaultsfreefield1entry, *defaultsfreefield2entry, *defaultsremarksentry,
		*defaultsfreefield1label, *defaultsfreefield2label,
		*mainvbox, *defaultslabel, *frame, *framevbox, *hbox1, *mhzlabel,
		*hbox2, *modelabel, *hbox3, *txrstlabel, *hbox4, *rxrstlabel,
		*hbox9, *awardslabel, *hbox5, *powerlabel, *hbox6, *hbox7, *hbox8, *remarkslabel;
	gint response;

	defaultsdialog = gtk_dialog_new_with_buttons (_("xlog - defaults"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (defaultsdialog)->vbox), mainvbox);

	defaultslabel = gtk_label_new (_("Values to use for 'Click all' or Ctl+k"));
	gtk_box_pack_start (GTK_BOX (mainvbox), defaultslabel, FALSE, FALSE, 10);
	frame = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (mainvbox), frame, FALSE, FALSE, 0);
	framevbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), framevbox);
	hbox1 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox1, FALSE, FALSE, 0);

	mhzlabel = gtk_label_new ("MHz");
	gtk_box_pack_start (GTK_BOX (hbox1), mhzlabel, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (mhzlabel), GTK_JUSTIFY_CENTER);
	defaultsmhzentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox1), defaultsmhzentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsmhzentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsmhzentry), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsmhzentry), TRUE);
	hbox2 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox2, FALSE, FALSE, 0);
	modelabel = gtk_label_new (_("Mode"));
	gtk_box_pack_start (GTK_BOX (hbox2), modelabel, FALSE, FALSE, 0);
	defaultsmodeentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox2), defaultsmodeentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsmodeentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsmodeentry), 8);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsmodeentry), TRUE);
	hbox3 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox3, FALSE, FALSE, 0);
	txrstlabel = gtk_label_new ("TX(RST)");
	gtk_box_pack_start (GTK_BOX (hbox3), txrstlabel, FALSE, FALSE, 0);
	defaultstxrstentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox3), defaultstxrstentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultstxrstentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultstxrstentry), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultstxrstentry), TRUE);
	hbox4 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox4, FALSE, FALSE, 0);
	rxrstlabel = gtk_label_new ("RX(RST)");
	gtk_box_pack_start (GTK_BOX (hbox4), rxrstlabel, FALSE, FALSE, 0);
	defaultsrxrstentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox4), defaultsrxrstentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsrxrstentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsrxrstentry), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsrxrstentry), TRUE);
	hbox9 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox9, FALSE, FALSE, 0);
	awardslabel = gtk_label_new (_("Awards"));
	gtk_box_pack_start (GTK_BOX (hbox9), awardslabel, FALSE, FALSE, 0);
	defaultsawardsentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox9), defaultsawardsentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsawardsentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsawardsentry), 30);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsawardsentry), TRUE);
	hbox5 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox5, FALSE, FALSE, 0);
	powerlabel = gtk_label_new (_("Power"));
	gtk_box_pack_start (GTK_BOX (hbox5), powerlabel, FALSE, FALSE, 0);
	defaultspowerentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox5), defaultspowerentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultspowerentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultspowerentry), 8);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultspowerentry), TRUE);
	hbox6 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox6, FALSE, FALSE, 0);
	defaultsfreefield1label = gtk_label_new (_("Freefield1"));
	gtk_box_pack_start (GTK_BOX (hbox6), defaultsfreefield1label, FALSE, FALSE, 0);
	defaultsfreefield1entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox6), defaultsfreefield1entry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsfreefield1entry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsfreefield1entry), 30);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsfreefield1entry), TRUE);
	hbox7 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox7, FALSE, FALSE, 0);
	defaultsfreefield2label = gtk_label_new (_("Freefield2"));
	gtk_box_pack_start (GTK_BOX (hbox7), defaultsfreefield2label, FALSE, FALSE, 0);
	defaultsfreefield2entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox7), defaultsfreefield2entry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsfreefield2entry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsfreefield2entry), 30);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsfreefield2entry), TRUE);
	hbox8 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (framevbox), hbox8, FALSE, FALSE, 0);
	remarkslabel = gtk_label_new (_("Remarks"));
	gtk_box_pack_start (GTK_BOX (hbox8), remarkslabel, FALSE, FALSE, 0);
	defaultsremarksentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox8), defaultsremarksentry, FALSE, FALSE, 0);
	gtk_widget_set_size_request (defaultsremarksentry, 150, -1);
	gtk_entry_set_max_length (GTK_ENTRY (defaultsremarksentry), 80);
	gtk_entry_set_activates_default (GTK_ENTRY (defaultsremarksentry), TRUE);

	gtk_label_set_text (GTK_LABEL(defaultsfreefield1label), 
		preferences.freefield1);
	gtk_label_set_text (GTK_LABEL(defaultsfreefield2label), 
		preferences.freefield2);

	gtk_entry_set_text (GTK_ENTRY (defaultsmhzentry), preferences.defaultmhz);
	gtk_entry_set_text (GTK_ENTRY (defaultsmodeentry), preferences.defaultmode);
	gtk_entry_set_text (GTK_ENTRY (defaultstxrstentry),	preferences.defaulttxrst);
	gtk_entry_set_text (GTK_ENTRY (defaultsrxrstentry),	preferences.defaultrxrst);
	gtk_entry_set_text (GTK_ENTRY (defaultsawardsentry), preferences.defaultawards);
	gtk_entry_set_text (GTK_ENTRY (defaultspowerentry),	preferences.defaultpower);
	gtk_entry_set_text (GTK_ENTRY (defaultsfreefield1entry), preferences.defaultfreefield1);
	gtk_entry_set_text (GTK_ENTRY (defaultsfreefield2entry), preferences.defaultfreefield2);
	gtk_entry_set_text (GTK_ENTRY (defaultsremarksentry), preferences.defaultremarks);

	gtk_dialog_set_default_response (GTK_DIALOG (defaultsdialog),
                                             GTK_RESPONSE_OK);
	gtk_widget_show_all (defaultsdialog);
	response = gtk_dialog_run (GTK_DIALOG(defaultsdialog));
	if (response == GTK_RESPONSE_OK)
	{
		preferences.defaultmhz =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsmhzentry), 0, -1);
		preferences.defaultmode =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsmodeentry), 0, -1);
		preferences.defaulttxrst =
			gtk_editable_get_chars (GTK_EDITABLE (defaultstxrstentry), 0, -1);
		preferences.defaultrxrst =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsrxrstentry), 0, -1);
		preferences.defaultawards =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsawardsentry), 0, -1);
		preferences.defaultpower =
			gtk_editable_get_chars (GTK_EDITABLE (defaultspowerentry), 0, -1);
		preferences.defaultfreefield1 =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsfreefield1entry), 0, -1);
		preferences.defaultfreefield2 =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsfreefield2entry), 0, -1);
		preferences.defaultremarks =
			gtk_editable_get_chars (GTK_EDITABLE (defaultsremarksentry), 0, -1);
	}
	gtk_widget_destroy (defaultsdialog);
}
