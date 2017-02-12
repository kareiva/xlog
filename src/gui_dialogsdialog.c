/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
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
 * gui_dialogsdialog.c - creation and destruction of the dialogs dialog
 *
 * show a dialog with 2 pages which will allow you to:
 * 1) select the fields in the "worked before" dialog. If the worked before
 *    dialog is active the fields are updated.
 * 2) set export options for save as ADIF and TSV (for glabels).
 */
#include <stdlib.h>
#include <gtk/gtk.h>

#include "gui_dialogsdialog.h"
#include "gui_scorewindow.h"
#include "support.h"
#include "cfg.h"
#include "xlog_enum.h"
#include "dxcc.h"

extern GtkWidget *mainwindow;
extern GtkWidget *b4window;
extern GtkWidget *scorewindow;
extern GtkWidget *locatormap;
extern preferencestype preferences;

void
on_menu_dialogs_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *dialogsdialog, *notebook, *vbox, *label,
		*frame, *framehbox, *leftvbox, *middlevbox, *rightvbox, *vbox2,
		*b40, *b41, *b42, *b43, *b44, *b45, *b46, *b47, *b48, *b49,
		*b410, *b411, *b412, *b413, *b414, *b415, *b416, *b417, *b418,
		*b419,
		*badif, *bcabrillo, *hsep,
		*bu1, *bu2, *bu3, *bu4, *bu5, *bu6, *bu7, *bu8, *bu9, *bu10,
		*bu11, *bu12, *bu13, *bu14, *bu15, *bu16, *bu17, *bu18,
		*b4treeview, *tsvhbox, *bucalc, *busortbydxcc,
		*groupbycallsignhbox, *groupbycallsignlabel, *groupbycallsignentry,
		*bb1, *bb2, *bb3, *bb4, *bb5, *bb6, *bb7, *bb8, *bb9, *bb10,
		*bb11, *bb12, *bb13, *bb14, *bb15, *bb16, *bb17, *bb18, *bb19,
		*bb20, *bb21, *bb22, *bb23, *bb24, *bb25, *bb26, *bb27, *bb28,
		*bb29, *bb30, *vbox3, *bwac, *bwas, *bwaz, *biota, *bloc,
		*countrytreeview, *dxcctreeview;
	gboolean check;
	gchar *temp;
	GtkTreeViewColumn *column;
	gint i, response;

	dialogsdialog = gtk_dialog_new_with_buttons (_("xlog - dialogs and windows"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	notebook = gtk_notebook_new ();
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (dialogsdialog)->vbox), notebook);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (notebook), vbox);
	label = gtk_label_new (_("Columns to show in the 'Worked Before' dialog"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 10);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

	frame = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	framehbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), framehbox);

	leftvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (framehbox), leftvbox, FALSE, FALSE, 0);
	b40 = gtk_check_button_new_with_label (_("Logname"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b40), preferences.b4columns2[0]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b40, FALSE, FALSE, 0);
	b41 = gtk_check_button_new_with_label (_("QSO Number"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b41), preferences.b4columns2[1]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b41, FALSE, FALSE, 0);
	b42 = gtk_check_button_new_with_label (_("Date"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b42), preferences.b4columns2[2]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b42, FALSE, FALSE, 0);
	b43 = gtk_check_button_new_with_label ("UTC");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b43), preferences.b4columns2[3]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b43, FALSE, FALSE, 0);
	b44 = gtk_check_button_new_with_label (_("UTC - end"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b44), preferences.b4columns2[4]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b44, FALSE, FALSE, 0);
	b45 = gtk_check_button_new_with_label (_("Call"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b45), preferences.b4columns2[5]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b45, FALSE, FALSE, 0);
	b46 = gtk_check_button_new_with_label (_("Frequency"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b46), preferences.b4columns2[6]);
	gtk_box_pack_start (GTK_BOX (leftvbox), b46, FALSE, FALSE, 0);

	middlevbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (framehbox), middlevbox, FALSE, FALSE, 0);
	b47 = gtk_check_button_new_with_label (_("Mode"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b47), preferences.b4columns2[7]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b47, FALSE, FALSE, 0);
	b48 = gtk_check_button_new_with_label ("TX(RST)");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b48), preferences.b4columns2[8]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b48, FALSE, FALSE, 0);
	b49 = gtk_check_button_new_with_label ("RX(RST)");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b49), preferences.b4columns2[9]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b49, FALSE, FALSE, 0);
	b410 = gtk_check_button_new_with_label (_("Awards"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b410), preferences.b4columns2[10]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b410, FALSE, FALSE, 0);
	b411 = gtk_check_button_new_with_label (_("Qsl Out"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b411), preferences.b4columns2[11]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b411, FALSE, FALSE, 0);
	b412 = gtk_check_button_new_with_label (_("Qsl In"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b412), preferences.b4columns2[12]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b412, FALSE, FALSE, 0);
	b413 = gtk_check_button_new_with_label (_("Power"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b413), preferences.b4columns2[13]);
	gtk_box_pack_start (GTK_BOX (middlevbox), b413, FALSE, FALSE, 0);

	rightvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (framehbox), rightvbox, FALSE, FALSE, 0);
	b414 = gtk_check_button_new_with_label (_("Name"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b414), preferences.b4columns2[14]);
	gtk_box_pack_start (GTK_BOX (rightvbox), b414, FALSE, FALSE, 0);
	b415 = gtk_check_button_new_with_label ("QTH");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b415), preferences.b4columns2[15]);
	gtk_box_pack_start (GTK_BOX (rightvbox), b415, FALSE, FALSE, 0);
	b416 = gtk_check_button_new_with_label (_("Locator"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b416), preferences.b4columns2[16]);
	gtk_box_pack_start (GTK_BOX (rightvbox), b416, FALSE, FALSE, 0);
	b417 = gtk_check_button_new_with_label ("");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b417), preferences.b4columns2[17]);
	gtk_box_pack_start (GTK_BOX (rightvbox), b417, FALSE, FALSE, 0);
	b418 = gtk_check_button_new_with_label ("");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b418), preferences.b4columns2[18]);
	gtk_box_pack_start (GTK_BOX (rightvbox), b418, FALSE, FALSE, 0);
	b419 = gtk_check_button_new_with_label (_("Remarks"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b419), preferences.b4columns2[19]);
	gtk_box_pack_start (GTK_BOX (rightvbox), b419, FALSE, FALSE, 0);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (notebook), vbox);

	label = gtk_label_new (_("Options for the export dialog"));
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 10);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

	frame = gtk_frame_new ("ADIF");
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	badif = gtk_check_button_new_with_label
		(_("When exporting to ADIF convert frequency to band"));
	if (preferences.saveasadif == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (badif), TRUE);
	gtk_container_add (GTK_CONTAINER (frame), badif);
	gtk_label_set_line_wrap	(GTK_LABEL (GTK_BIN (badif)->child), TRUE);

	frame = gtk_frame_new ("Cabrillo");
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	bcabrillo = gtk_check_button_new_with_label
		(_("When exporting to Cabrillo convert frequency to band"));
	if (preferences.saveascabrillo == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bcabrillo), TRUE);
	gtk_container_add (GTK_CONTAINER (frame), bcabrillo);
	gtk_label_set_line_wrap	(GTK_LABEL (GTK_BIN (bcabrillo)->child), TRUE);

	frame = gtk_frame_new ("TSV");
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox2);
	label = gtk_label_new
(_("Fields to export when saving as TSV (Tab Separated Value) for gLabels"));
	gtk_container_add (GTK_CONTAINER (vbox2), label);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	hsep = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (vbox2), hsep);
	tsvhbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (vbox2), tsvhbox);

	leftvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (tsvhbox), leftvbox, FALSE, FALSE, 0);
	bu1 = gtk_check_button_new_with_label (_("Date"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu1), preferences.saveastsv2[0]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bu1, FALSE, FALSE, 0);
	bu2 = gtk_check_button_new_with_label ("UTC");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu2), preferences.saveastsv2[1]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bu2, FALSE, FALSE, 0);
	bu3 = gtk_check_button_new_with_label (_("UTC - end"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu3), preferences.saveastsv2[2]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bu3, FALSE, FALSE, 0);
	bu4 = gtk_check_button_new_with_label (_("Call"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu4), preferences.saveastsv2[3]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bu4, FALSE, FALSE, 0);
	bu5 = gtk_check_button_new_with_label (_("Frequency"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu5), preferences.saveastsv2[4]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bu5, FALSE, FALSE, 0);
	bu6 = gtk_check_button_new_with_label (_("Mode"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu6), preferences.saveastsv2[5]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bu6, FALSE, FALSE, 0);

	middlevbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (tsvhbox), middlevbox, FALSE, FALSE, 0);
	bu7 = gtk_check_button_new_with_label ("TX(RST)");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu7), preferences.saveastsv2[6]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bu7, FALSE, FALSE, 0);
	bu8 = gtk_check_button_new_with_label ("RX(RST)");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu8), preferences.saveastsv2[7]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bu8, FALSE, FALSE, 0);
	bu9 = gtk_check_button_new_with_label (_("Awards"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu9), preferences.saveastsv2[8]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bu9, FALSE, FALSE, 0);
	bu10 = gtk_check_button_new_with_label (_("Qsl Out"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu10), preferences.saveastsv2[9]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bu10, FALSE, FALSE, 0);
	bu11 = gtk_check_button_new_with_label (_("Qsl In"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu11), preferences.saveastsv2[10]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bu11, FALSE, FALSE, 0);
	bu12 = gtk_check_button_new_with_label (_("Power"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu12), preferences.saveastsv2[11]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bu12, FALSE, FALSE, 0);

	rightvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (tsvhbox), rightvbox, FALSE, FALSE, 0);
	bu13 = gtk_check_button_new_with_label (_("Name"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu13), preferences.saveastsv2[12]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bu13, FALSE, FALSE, 0);
	bu14 = gtk_check_button_new_with_label ("QTH");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu14), preferences.saveastsv2[13]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bu14, FALSE, FALSE, 0);
	bu15 = gtk_check_button_new_with_label (_("Locator"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu15), preferences.saveastsv2[14]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bu15, FALSE, FALSE, 0);
	bu16 = gtk_check_button_new_with_label ("");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu16), preferences.saveastsv2[15]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bu16, FALSE, FALSE, 0);
	bu17 = gtk_check_button_new_with_label ("");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu17), preferences.saveastsv2[16]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bu17, FALSE, FALSE, 0);
	bu18 = gtk_check_button_new_with_label (_("Remarks"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bu18), preferences.saveastsv2[17]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bu18, FALSE, FALSE, 0);

	hsep = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (vbox2), hsep);
	bucalc = gtk_check_button_new_with_label
(_("Add calculated bearing and distance fields when the locator log field is used"));
	gtk_label_set_line_wrap	(GTK_LABEL (GTK_BIN (bucalc)->child), TRUE);
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON (bucalc), preferences.tsvcalc);
	gtk_box_pack_start (GTK_BOX (vbox2), bucalc, FALSE, FALSE, 0);

	hsep = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (vbox2), hsep);
	busortbydxcc = gtk_check_button_new_with_label (_("Sort by DXCC"));
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON (busortbydxcc), preferences.tsvsortbydxcc);
	gtk_box_pack_start (GTK_BOX (vbox2), busortbydxcc, FALSE, FALSE, 0);

	hsep = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (vbox2), hsep);
	groupbycallsignhbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (vbox2), groupbycallsignhbox);
	groupbycallsignlabel = gtk_label_new (_("Group by call-sign"));
	gtk_box_pack_start (GTK_BOX (groupbycallsignhbox), groupbycallsignlabel, FALSE, FALSE, 0);
	groupbycallsignentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (groupbycallsignhbox), groupbycallsignentry, TRUE, TRUE, 2);
	gtk_entry_set_max_length (GTK_ENTRY (groupbycallsignentry), 2);
	gtk_entry_set_activates_default (GTK_ENTRY (groupbycallsignentry), TRUE);
	temp = g_strdup_printf ("%d", preferences.tsvgroupbycallsign);
	gtk_entry_set_text (GTK_ENTRY (groupbycallsignentry), temp);
	g_free(temp);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (notebook), vbox);
	frame = gtk_frame_new (_("Bands"));
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox2);
	label = gtk_label_new (_("Bands to include in the scoring window"));
	gtk_container_add (GTK_CONTAINER (vbox2), label);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	hsep = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (vbox2), hsep);

	framehbox = gtk_hbox_new (TRUE	, 0);
	gtk_container_add (GTK_CONTAINER (vbox2), framehbox);

	leftvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (framehbox), leftvbox, FALSE, FALSE, 0);
	bb1 = gtk_check_button_new_with_label ("0.136 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb1),
		preferences.scoringbands[BAND_2190]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb1, FALSE, FALSE, 0);
	bb2 = gtk_check_button_new_with_label ("0.501 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb2),
		preferences.scoringbands[BAND_560]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb2, FALSE, FALSE, 0);
	bb3 = gtk_check_button_new_with_label ("1.8 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb3),
		preferences.scoringbands[BAND_160]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb3, FALSE, FALSE, 0);
	bb4 = gtk_check_button_new_with_label ("3.5 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb4),
		preferences.scoringbands[BAND_80]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb4, FALSE, FALSE, 0);
	bb5 = gtk_check_button_new_with_label ("5.2 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb5),
		preferences.scoringbands[BAND_60]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb5, FALSE, FALSE, 0);
	bb6 = gtk_check_button_new_with_label ("7 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb6),
		preferences.scoringbands[BAND_40]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb6, FALSE, FALSE, 0);
	bb7 = gtk_check_button_new_with_label ("10 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb7),
		preferences.scoringbands[BAND_30]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb7, FALSE, FALSE, 0);
	bb8 = gtk_check_button_new_with_label ("14 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb8),
		preferences.scoringbands[BAND_20]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb8, FALSE, FALSE, 0);
	bb9 = gtk_check_button_new_with_label ("18 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb9),
		preferences.scoringbands[BAND_17]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb9, FALSE, FALSE, 0);
	bb10 = gtk_check_button_new_with_label ("21 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb10),
		preferences.scoringbands[BAND_15]);
	gtk_box_pack_start (GTK_BOX (leftvbox), bb10, FALSE, FALSE, 0);

	middlevbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (framehbox), middlevbox, FALSE, FALSE, 0);
	bb11 = gtk_check_button_new_with_label ("24 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb11),
		preferences.scoringbands[BAND_12]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb11, FALSE, FALSE, 0);
	bb12 = gtk_check_button_new_with_label ("28 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb12),
		preferences.scoringbands[BAND_10]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb12, FALSE, FALSE, 0);
	bb13 = gtk_check_button_new_with_label ("50 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb13),
		preferences.scoringbands[BAND_6]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb13, FALSE, FALSE, 0);
	bb14 = gtk_check_button_new_with_label ("70 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb14),
		preferences.scoringbands[BAND_4]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb14, FALSE, FALSE, 0);
	bb15 = gtk_check_button_new_with_label ("144 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb15),
		preferences.scoringbands[BAND_2]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb15, FALSE, FALSE, 0);
	bb16 = gtk_check_button_new_with_label ("222 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb16),
		preferences.scoringbands[BAND_125CM]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb16, FALSE, FALSE, 0);
	bb17 = gtk_check_button_new_with_label ("420 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb17),
		preferences.scoringbands[BAND_70CM]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb17, FALSE, FALSE, 0);
	bb18 = gtk_check_button_new_with_label ("902 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb18),
		preferences.scoringbands[BAND_33CM]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb18, FALSE, FALSE, 0);
	bb19 = gtk_check_button_new_with_label ("1240 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb19),
		preferences.scoringbands[BAND_23CM]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb19, FALSE, FALSE, 0);
	bb20 = gtk_check_button_new_with_label ("2300 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb20),
		preferences.scoringbands[BAND_13CM]);
	gtk_box_pack_start (GTK_BOX (middlevbox), bb20, FALSE, FALSE, 0);

	rightvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (framehbox), rightvbox, FALSE, FALSE, 0);
	bb21 = gtk_check_button_new_with_label ("3300 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb21),
		preferences.scoringbands[BAND_9CM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb21, FALSE, FALSE, 0);
	bb22 = gtk_check_button_new_with_label ("5650 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb22),
		preferences.scoringbands[BAND_6CM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb22, FALSE, FALSE, 0);
	bb23 = gtk_check_button_new_with_label ("10000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb23),
		preferences.scoringbands[BAND_3CM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb23, FALSE, FALSE, 0);
	bb24 = gtk_check_button_new_with_label ("24000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb24),
		preferences.scoringbands[BAND_12HMM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb24, FALSE, FALSE, 0);
	bb25 = gtk_check_button_new_with_label ("47000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb25),
		preferences.scoringbands[BAND_6MM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb25, FALSE, FALSE, 0);
	bb26 = gtk_check_button_new_with_label ("75500 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb26),
		preferences.scoringbands[BAND_4MM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb26, FALSE, FALSE, 0);
	bb27 = gtk_check_button_new_with_label ("120000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb27),
		preferences.scoringbands[BAND_2HMM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb27, FALSE, FALSE, 0);
	bb28 = gtk_check_button_new_with_label ("142000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb28),
		preferences.scoringbands[BAND_2MM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb28, FALSE, FALSE, 0);
	bb29 = gtk_check_button_new_with_label ("241000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb29),
		preferences.scoringbands[BAND_1MM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb29, FALSE, FALSE, 0);
	bb30 = gtk_check_button_new_with_label ("300000 MHz");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bb30),
		preferences.scoringbands[BAND_SUBMM]);
	gtk_box_pack_start (GTK_BOX (rightvbox), bb30, FALSE, FALSE, 0);

	frame = gtk_frame_new (_("Awards"));
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox3);
	label = gtk_label_new (_("Awards to add to the scoring window"));
	gtk_container_add (GTK_CONTAINER (vbox3), label);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	hsep = gtk_hseparator_new ();
	gtk_container_add (GTK_CONTAINER (vbox3), hsep);
	bwac = gtk_check_button_new_with_label ("WAC (Worked All Continents)");
	gtk_box_pack_start (GTK_BOX (vbox3), bwac, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bwac), preferences.awardswac);
	bwas = gtk_check_button_new_with_label ("WAS (Worked All States)");
	gtk_box_pack_start (GTK_BOX (vbox3), bwas, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bwas), preferences.awardswas);
	bwaz = gtk_check_button_new_with_label ("WAZ (Worked All Zones)");
	gtk_box_pack_start (GTK_BOX (vbox3), bwaz, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bwaz), preferences.awardswaz);
	biota = gtk_check_button_new_with_label ("IOTA (Islands On The Air)");
	gtk_box_pack_start (GTK_BOX (vbox3), biota, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (biota), preferences.awardsiota);
	bloc = gtk_check_button_new_with_label ("Grid Locators");
	gtk_box_pack_start (GTK_BOX (vbox3), bloc, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bloc), preferences.awardsloc);

	label = gtk_label_new (_("Worked Before"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), label);
	label = gtk_label_new (_("Export"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), label);
	label = gtk_label_new (_("Scoring"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 2), label);

	gtk_button_set_label (GTK_BUTTON (b417), preferences.freefield1);
	gtk_button_set_label (GTK_BUTTON (b418), preferences.freefield2);
	gtk_button_set_label (GTK_BUTTON (bu16), preferences.freefield1);
	gtk_button_set_label (GTK_BUTTON (bu17), preferences.freefield2);

	gtk_dialog_set_default_response (GTK_DIALOG (dialogsdialog),
                                             GTK_RESPONSE_OK);

	gtk_widget_show_all (dialogsdialog);
	response = gtk_dialog_run (GTK_DIALOG(dialogsdialog));
	if (response == GTK_RESPONSE_OK)
	{
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b40));
		preferences.b4columns2[0] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b41));
		preferences.b4columns2[1] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b42));
		preferences.b4columns2[2] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b43));
		preferences.b4columns2[3] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b44));
		preferences.b4columns2[4] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b45));
		preferences.b4columns2[5] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b46));
		preferences.b4columns2[6] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b47));
		preferences.b4columns2[7] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b48));
		preferences.b4columns2[8] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b49));
		preferences.b4columns2[9] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b410));
		preferences.b4columns2[10] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b411));
		preferences.b4columns2[11] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b412));
		preferences.b4columns2[12] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b413));
		preferences.b4columns2[13] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b414));
		preferences.b4columns2[14] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b415));
		preferences.b4columns2[15] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b416));
		preferences.b4columns2[16] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b417));
		preferences.b4columns2[17] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b418));
		preferences.b4columns2[18] = (check ? 1 : 0);
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(b419));
		preferences.b4columns2[19] = (check ? 1 : 0);
		
		if (b4window)
		{
			b4treeview = lookup_widget (b4window, "b4treeview");
			for (i = 0; i <= 19; i++)
			{
				column = gtk_tree_view_get_column (GTK_TREE_VIEW(b4treeview), i);
				gtk_tree_view_column_set_visible
					(GTK_TREE_VIEW_COLUMN(column), preferences.b4columns2[i]);
			}
		}

		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(badif));
		preferences.saveasadif = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bcabrillo));
		preferences.saveascabrillo = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu1));
		preferences.saveastsv2[0] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu2));
		preferences.saveastsv2[1] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu3));
		preferences.saveastsv2[2] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu4));
		preferences.saveastsv2[3] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu5));
		preferences.saveastsv2[4] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu6));
		preferences.saveastsv2[5] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu7));
		preferences.saveastsv2[6] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu8));
		preferences.saveastsv2[7] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu9));
		preferences.saveastsv2[8] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu10));
		preferences.saveastsv2[9] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu11));
		preferences.saveastsv2[10] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu12));
		preferences.saveastsv2[11] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu13));
		preferences.saveastsv2[12] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu14));
		preferences.saveastsv2[13] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu15));
		preferences.saveastsv2[14] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu16));
		preferences.saveastsv2[15] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu17));
		preferences.saveastsv2[16] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bu18));
		preferences.saveastsv2[17] = check ? 1 : 0;

		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bucalc));
		if (check)
			preferences.tsvcalc = 1;
		else
			preferences.tsvcalc = 0;

		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(busortbydxcc));
		if (check)
			preferences.tsvsortbydxcc = 1;
		else
			preferences.tsvsortbydxcc = 0;

		temp = gtk_editable_get_chars (GTK_EDITABLE (groupbycallsignentry), 0, -1);
		preferences.tsvgroupbycallsign = atoi(temp);

		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb1));
		preferences.scoringbands[BAND_2190] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb2));
		preferences.scoringbands[BAND_560] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb3));
		preferences.scoringbands[BAND_160] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb4));
		preferences.scoringbands[BAND_80] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb5));
		preferences.scoringbands[BAND_60] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb6));
		preferences.scoringbands[BAND_40] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb7));
		preferences.scoringbands[BAND_30] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb8));
		preferences.scoringbands[BAND_20] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb9));
		preferences.scoringbands[BAND_17] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb10));
		preferences.scoringbands[BAND_15] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb11));
		preferences.scoringbands[BAND_12] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb12));
		preferences.scoringbands[BAND_10] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb13));
		preferences.scoringbands[BAND_6] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb14));
		preferences.scoringbands[BAND_4] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb15));
		preferences.scoringbands[BAND_2] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb16));
		preferences.scoringbands[BAND_125CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb17));
		preferences.scoringbands[BAND_70CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb18));
		preferences.scoringbands[BAND_33CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb19));
		preferences.scoringbands[BAND_23CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb20));
		preferences.scoringbands[BAND_13CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb21));
		preferences.scoringbands[BAND_9CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb22));
		preferences.scoringbands[BAND_6CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb23));
		preferences.scoringbands[BAND_3CM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb24));
		preferences.scoringbands[BAND_12HMM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb25));
		preferences.scoringbands[BAND_6MM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb26));
		preferences.scoringbands[BAND_4MM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb27));
		preferences.scoringbands[BAND_2HMM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb28));
		preferences.scoringbands[BAND_2MM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb29));
		preferences.scoringbands[BAND_1MM] = check ? 1 : 0;
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bb30));
		preferences.scoringbands[BAND_SUBMM] = check ? 1 : 0;

		/* update the scoring window */
		GtkWidget *awardsnotebook = lookup_widget (scorewindow, "awardsnotebook");
		notebook_remove_dxccpage (scorewindow, awardsnotebook);
		notebook_append_dxccpage (scorewindow, awardsnotebook);
		fill_scoring_arrays ();
		update_dxccscoring ();

		/* WAC */
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bwac));
		if (check)
		{
			if (preferences.awardswac == 1)
				notebook_remove_wacpage (scorewindow, awardsnotebook);
			notebook_append_wacpage (scorewindow, awardsnotebook);
			update_wacscoring ();
			preferences.awardswac = 1;
		}
		else
		{
			if (preferences.awardswac == 1)
				notebook_remove_wacpage (scorewindow, awardsnotebook);
			preferences.awardswac = 0;
		}

		/* WAS */
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bwas));
		if (check)
		{
			if (preferences.awardswas == 1)
				notebook_remove_waspage (scorewindow, awardsnotebook);
			notebook_append_waspage (scorewindow, awardsnotebook);
			update_wasscoring ();
			preferences.awardswas = 1;
		}
		else
		{
			if (preferences.awardswas == 1)
				notebook_remove_waspage (scorewindow, awardsnotebook);
			preferences.awardswas = 0;
		}

		/* WAZ */
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bwaz));
		if (check)
		{
			if (preferences.awardswaz == 1)
				notebook_remove_wazpage (scorewindow, awardsnotebook);
			notebook_append_wazpage (scorewindow, awardsnotebook);
			update_wazscoring ();
			preferences.awardswaz = 1;
		}
		else
		{
			if (preferences.awardswaz == 1)
				notebook_remove_wazpage (scorewindow, awardsnotebook);
			preferences.awardswaz = 0;
		}

		/* IOTA */
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(biota));
		if (check)
		{
			if (preferences.awardsiota == 1)
				notebook_remove_iotapage (scorewindow, awardsnotebook);
			notebook_append_iotapage (scorewindow, awardsnotebook);
			update_iotascoring ();
			preferences.awardsiota = 1;
		}
		else
		{
			if (preferences.awardsiota == 1)
				notebook_remove_iotapage (scorewindow, awardsnotebook);
			preferences.awardsiota = 0;
		}

		/* Grid locator */
		check = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(bloc));
		if (check)
		{
			if (preferences.awardsloc == 1)
				notebook_remove_locpage (scorewindow, awardsnotebook);
			notebook_append_locpage (scorewindow, awardsnotebook);
			update_locscoring ();
			preferences.awardsloc = 1;
		}
		else
		{
			if (preferences.awardsloc == 1)
			{
				notebook_remove_locpage (scorewindow, awardsnotebook);
				if (locatormap)
				{
					gtk_widget_destroy (locatormap);
					locatormap = NULL;
				}
			}
			preferences.awardsloc = 0;
		}

		countrytreeview = lookup_widget (scorewindow, "countrytreeview");
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_2190 + 1), preferences.scoringbands[BAND_2190]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_560 + 1), preferences.scoringbands[BAND_560]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_160 + 1), preferences.scoringbands[BAND_160]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_80 + 1), preferences.scoringbands[BAND_80]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_60 + 1), preferences.scoringbands[BAND_60]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_40 + 1), preferences.scoringbands[BAND_40]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_30 + 1), preferences.scoringbands[BAND_30]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_20 + 1), preferences.scoringbands[BAND_20]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_17 + 1), preferences.scoringbands[BAND_17]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_15 + 1), preferences.scoringbands[BAND_15]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_12 + 1), preferences.scoringbands[BAND_12]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_10 + 1), preferences.scoringbands[BAND_10]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_6 + 1), preferences.scoringbands[BAND_6]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_4 + 1), preferences.scoringbands[BAND_4]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_2 + 1), preferences.scoringbands[BAND_2]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_125CM + 1), preferences.scoringbands[BAND_125CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_70CM + 1), preferences.scoringbands[BAND_70CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_33CM + 1), preferences.scoringbands[BAND_33CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_23CM + 1), preferences.scoringbands[BAND_23CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_13CM + 1), preferences.scoringbands[BAND_13CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_9CM + 1), preferences.scoringbands[BAND_9CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_6CM + 1), preferences.scoringbands[BAND_6CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_3CM + 1), preferences.scoringbands[BAND_3CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_12HMM + 1), preferences.scoringbands[BAND_12HMM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_6MM + 1), preferences.scoringbands[BAND_6MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_4MM + 1), preferences.scoringbands[BAND_4MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_2HMM + 1), preferences.scoringbands[BAND_2HMM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_2MM + 1), preferences.scoringbands[BAND_2MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_1MM + 1), preferences.scoringbands[BAND_1MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(countrytreeview), BAND_SUBMM + 1), preferences.scoringbands[BAND_SUBMM]);

		dxcctreeview = lookup_widget (scorewindow, "dxcctreeview");
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_2190), preferences.scoringbands[BAND_2190]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_560), preferences.scoringbands[BAND_560]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_160), preferences.scoringbands[BAND_160]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_80), preferences.scoringbands[BAND_80]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_60), preferences.scoringbands[BAND_60]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_40), preferences.scoringbands[BAND_40]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_30), preferences.scoringbands[BAND_30]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_20), preferences.scoringbands[BAND_20]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_17), preferences.scoringbands[BAND_17]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_15), preferences.scoringbands[BAND_15]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_12), preferences.scoringbands[BAND_12]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_10), preferences.scoringbands[BAND_10]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_6), preferences.scoringbands[BAND_6]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_4), preferences.scoringbands[BAND_4]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_2), preferences.scoringbands[BAND_2]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_125CM), preferences.scoringbands[BAND_125CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_70CM), preferences.scoringbands[BAND_70CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_33CM), preferences.scoringbands[BAND_33CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_23CM), preferences.scoringbands[BAND_23CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_13CM), preferences.scoringbands[BAND_13CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_9CM), preferences.scoringbands[BAND_9CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_6CM), preferences.scoringbands[BAND_6CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_3CM), preferences.scoringbands[BAND_3CM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_12HMM), preferences.scoringbands[BAND_12HMM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_6MM), preferences.scoringbands[BAND_6MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_4MM), preferences.scoringbands[BAND_4MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_2HMM), preferences.scoringbands[BAND_2HMM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_2MM), preferences.scoringbands[BAND_2MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_1MM), preferences.scoringbands[BAND_1MM]);
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column
(GTK_TREE_VIEW(dxcctreeview), BAND_SUBMM), preferences.scoringbands[BAND_SUBMM]);

	}
	gtk_widget_destroy (dialogsdialog);
}
