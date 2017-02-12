/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2016 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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

#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <hamlib/rig.h>
#include <hamlib/rotator.h>

#include "gui_utils.h"
#include "gui_fontselectiondialog.h"
#include "gui_pathselectiondialog.h"
#include "support.h"
#include "cfg.h"
#include "callbacks_preferencesdialog.h"
#include "gui_preferencesdialog.h"
#include "utils.h"
#include "main.h"
#include "hamlib-utils.h"

GtkWidget *preferencesdialog;
extern preferencestype preferences;
extern GtkWidget *mainwindow;
extern programstatetype programstate;

static void latlon_changed (GtkWidget *widget, gpointer data);

static void
qra_changed (GtkEntry *entry, gpointer data)
{
	gint retcode;
	gdouble lat,lon;
	gchar *temp;
	GtkWidget *qthlocatorentry, *latentry, *longentry, *EWcombo, *NScombo;

	qthlocatorentry = lookup_widget (preferencesdialog, "qthlocatorentry");
	latentry = lookup_widget (preferencesdialog, "latentry");
	longentry = lookup_widget (preferencesdialog, "longentry");
	EWcombo = lookup_widget (preferencesdialog, "EWcombo");
	NScombo = lookup_widget (preferencesdialog, "NScombo");

	retcode = locator2longlat (&lon, &lat,
		gtk_entry_get_text (GTK_ENTRY (qthlocatorentry)));

	if (retcode == RIG_OK)
	{

		/* block signal emissions for lat/lon widgets */
		g_signal_handlers_block_by_func
			(GTK_OBJECT (latentry), latlon_changed, data);
		g_signal_handlers_block_by_func
			(GTK_OBJECT (longentry), latlon_changed, data);
		g_signal_handlers_block_by_func
			(GTK_OBJECT (EWcombo), latlon_changed, data);
		g_signal_handlers_block_by_func
			(GTK_OBJECT (NScombo), latlon_changed, data);

		/* update widgets */
		if (lat < 0.00)
		{
			lat = -1.0 * lat;
			gtk_combo_box_set_active (GTK_COMBO_BOX (NScombo), 1);
		}
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (NScombo), 0);

		if (lon < 0.00)
		{
			lon = -1.0 * lon;
			gtk_combo_box_set_active (GTK_COMBO_BOX (EWcombo), 1);
		}
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (EWcombo), 0);

		temp = g_strdup_printf ("%g", lat);
		gtk_entry_set_text (GTK_ENTRY (latentry), temp);
		temp = g_strdup_printf ("%g", lon);
		gtk_entry_set_text (GTK_ENTRY (longentry), temp);

		/* make sure text is upper case */
		temp = g_ascii_strup (gtk_entry_get_text (GTK_ENTRY (qthlocatorentry)), -1);
		gtk_entry_set_text (GTK_ENTRY (qthlocatorentry), temp);
		g_free (temp);

		/* unblock signal emissions */
		g_signal_handlers_unblock_by_func
			(GTK_OBJECT (latentry), latlon_changed, data);
		g_signal_handlers_unblock_by_func
			(GTK_OBJECT (longentry), latlon_changed, data);
		g_signal_handlers_unblock_by_func
			(GTK_OBJECT (EWcombo), latlon_changed, data);
		g_signal_handlers_unblock_by_func
			(GTK_OBJECT (NScombo), latlon_changed, data);
	}

}

static void
latlon_changed (GtkWidget *widget, gpointer data)
{
	gchar *locator;
	gdouble lat, lon;
	gint retcode;
	GtkWidget *qthlocatorentry, *latentry, *longentry, *EWcombo, *NScombo;

	locator = g_try_malloc (7);
	qthlocatorentry = lookup_widget (preferencesdialog, "qthlocatorentry");
	latentry = lookup_widget (preferencesdialog, "latentry");
	longentry = lookup_widget (preferencesdialog, "longentry");
	EWcombo = lookup_widget (preferencesdialog, "EWcombo");
	NScombo = lookup_widget (preferencesdialog, "NScombo");
	
	lat = atof(gtk_editable_get_chars (GTK_EDITABLE (latentry), 0, -1));
	lon = atof(gtk_editable_get_chars (GTK_EDITABLE (longentry), 0, -1));
	if (gtk_combo_box_get_active (GTK_COMBO_BOX (NScombo)))
	{
		/* index 1 => South */
		lat = -lat;
	}
	if (gtk_combo_box_get_active (GTK_COMBO_BOX (EWcombo)))
	{
		/* index 1 => West */
		lon = -lon;
	}

	retcode = longlat2locator (lon, lat, locator, 3);
	if (retcode == RIG_OK)
	{
		g_signal_handlers_block_by_func
			(GTK_OBJECT (qthlocatorentry), qra_changed, data);
		gtk_entry_set_text (GTK_ENTRY (qthlocatorentry), locator);
		g_signal_handlers_unblock_by_func
			(GTK_OBJECT (qthlocatorentry), qra_changed, data);
	}
}

void
on_menu_preferences_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *preferencesnotebook, *preferenceslabel1,
		*preferenceslabel3, *preferenceslabel4,	*preferenceslabel5;
	GtkWidget *vboxpage1, *clockframe, *clockcheckbutton, *areyousurecheckbutton, *clockvbox,
		*typeaheadfindcheckbutton, *distqrbcheckbutton,
		*modesloggingframe, *modesframevbox, *modesframe,
		*modeseditvbox,	*bandsloggingframe, *bandsframevbox, *bandsframe,
		*bandseditvbox, *modesentry, *bandsentry, *modesradiobutton1,
		*modesradiobutton2, *bandsradiobutton1,	*bandsradiobutton2,
		*modeslabel, *bandslabel, *bandoptionmenu, *modeoptionmenu, *bandentry,
		*bandlabel, *modelabel,	*modeentry, *fontframe, *fontvbox, *fontseparator,
		*fonthbox, *fontentry, *fontbutton, *fontlabel;
	GtkWidget *vboxpage3, *callsignframe, *callsignhbox, *callsignlabel,
		*locationframe, *locationvbox, *labelloc, *lathbox, *locatorhbox,
		*locatorlabel, *unitsframe, *unitshbox, *labeldist, *latentry,
		*longentry, *NScombo, *EWcombo, *unitscombo, *callsignentry,
		*qthlocatorentry;
	GtkWidget *vboxpage4;
	GtkWidget *hamlibvbox, *radiohbox, *radioentry, *devicehbox, *devicelabel,
		*hamlibseparator, *configurelabel, *pollingframe, *pollinghbox,
		*pollinglabel, *secondslabel, *digitshbox,  *digitslabel, *characterslabel,
		*fcccheckbutton;
	GtkWidget *vboxpage5, *logsframe, *logsvbox, *pathlabel, *pathhbox,
		*pathbutton, *logshseparator, *loadlabel, *savingframe, *savingvbox,
		*autosavehbox, *autosavelabel, *minuteslabel, *pathentry, *logsentry,
		*saveradiobutton, *autosaveradiobutton,	*autosaveframe, *backupframe,
		*backupvbox, *backupradiobutton1, *backupradiobutton2, *backupbutton,
		*backuphbox, *backupentry, *autosavecombo;
	GtkWidget *remotedataframe, *remotedatavbox, *remotedatalabel,
		*remotedataseparator, *remoteradiobutton1,
		*remoteradiobutton2;

	GSList *modesradiobutton_group = NULL, *bandsradiobutton_group = NULL,
		*remoteradiobutton_group = NULL, *backupradiobutton_group = NULL,
		*saveradiobutton_group = NULL;

	gchar *temp, *font;
	gboolean toggletrue;
	gint autosavevalue, saving, response;
	GtkWidget *hamlibcheckbutton, *radiobutton, *hamlibframe, *devicecombo,
		*frequencycheckbutton, *digitscombo, *smetercheckbutton,
		*pollingcheckbutton, *rigsetconfentry, *pollcombo;
	gchar *radio = NULL, *device = NULL, *digits = NULL, *pollstr = NULL,
		*rigconf = NULL;
	gboolean hamlibyes, frequencyyesno, smeteryesno, pollingyes, fccyes,
		hamlibresult;
	gint hamlibwidgets = 0;

	GDir *dev_serial_dir = NULL;
	const gchar *serial_file_dir = "/dev/serial/by-id";
	const gchar *serial_file_name = NULL;
	const gchar *absolute_serial_file_name = NULL;

	GDir *dev_dir = NULL;
	const gchar *dev_file_dir = "/dev";
	const gchar *dev_file_name = NULL;
	const gchar *absolute_dev_file_name = NULL;

	/**************************/
	/* creation of the dialog */
	/**************************/

	preferencesdialog = gtk_dialog_new_with_buttons (_("xlog - preferences"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	preferencesnotebook = gtk_notebook_new ();
	gtk_container_add (GTK_CONTAINER
		(GTK_DIALOG (preferencesdialog)->vbox), preferencesnotebook);

	/* page 1, clock, bands and modes */
	vboxpage1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (preferencesnotebook), vboxpage1);
	clockframe = gtk_frame_new (_("General"));
	gtk_box_pack_start (GTK_BOX (vboxpage1), clockframe, FALSE, FALSE, 0);
	clockvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (clockframe), clockvbox);
	clockcheckbutton = gtk_check_button_new_with_label
		(_("Enable clock on statusbar"));
	gtk_box_pack_start (GTK_BOX (clockvbox), clockcheckbutton, FALSE, FALSE, 0);
	typeaheadfindcheckbutton = gtk_check_button_new_with_label
		(_("Enable type and find"));
	gtk_box_pack_start
		(GTK_BOX (clockvbox), typeaheadfindcheckbutton, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text (typeaheadfindcheckbutton, _(
		"Search all the logs for a callsign match and when found, fill "
		"in name, QTH, locator and remarks like in the previous QSO when "
		"these fields are empty"));
	distqrbcheckbutton = gtk_check_button_new_with_label
		(_("Use free fields (when present) to add distance and azimuth to the log"));
	gtk_label_set_line_wrap (GTK_LABEL (GTK_BIN (distqrbcheckbutton)->child), TRUE);
	gtk_box_pack_start
		(GTK_BOX (clockvbox), distqrbcheckbutton, FALSE, FALSE, 0);

	areyousurecheckbutton = gtk_check_button_new_with_label
		(_("Confirm exit via dialog box"));
	gtk_box_pack_start (GTK_BOX (clockvbox), areyousurecheckbutton, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text (areyousurecheckbutton, _(" Display the \"Are you sure?\" dialog when exiting "));

	preferenceslabel1 = gtk_label_new (_("General"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (preferencesnotebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (preferencesnotebook), 0),
		preferenceslabel1);

	modesloggingframe = gtk_frame_new (_("Modes"));
	gtk_box_pack_start
		(GTK_BOX (vboxpage1), modesloggingframe, FALSE, FALSE, 0);
	modesframevbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (modesloggingframe), modesframevbox);
	modesradiobutton1 = gtk_radio_button_new_with_label
		(NULL, _("Use Editbox"));
	gtk_box_pack_start
		(GTK_BOX (modesframevbox), modesradiobutton1, FALSE, FALSE, 0);
	modesradiobutton_group = gtk_radio_button_get_group
		(GTK_RADIO_BUTTON (modesradiobutton1));
	modesradiobutton2 = gtk_radio_button_new_with_label
		(modesradiobutton_group, _("Use Optionmenu"));
	gtk_box_pack_start
		(GTK_BOX (modesframevbox), modesradiobutton2, FALSE, FALSE, 0);
	modesframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (modesframevbox), modesframe, FALSE, FALSE, 0);
	modeseditvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (modesframe), modeseditvbox);
	modeslabel = gtk_label_new (_("Comma separated list of modes"));
	gtk_box_pack_start (GTK_BOX (modeseditvbox), modeslabel, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (modeslabel), GTK_JUSTIFY_CENTER);
	modesentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (modeseditvbox), modesentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (modesentry), 512);
	gtk_entry_set_activates_default (GTK_ENTRY (modesentry), TRUE);

	bandsloggingframe = gtk_frame_new (_("Bands"));
	gtk_box_pack_start
		(GTK_BOX (vboxpage1), bandsloggingframe, FALSE, FALSE, 0);
	bandsframevbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (bandsloggingframe), bandsframevbox);
	bandsradiobutton1 = gtk_radio_button_new_with_label
		(NULL, _("Use Editbox"));
	gtk_box_pack_start
		(GTK_BOX (bandsframevbox), bandsradiobutton1, FALSE, FALSE, 0);
	bandsradiobutton_group = gtk_radio_button_get_group
		(GTK_RADIO_BUTTON (bandsradiobutton1));
	bandsradiobutton2 = gtk_radio_button_new_with_label
		(bandsradiobutton_group, _("Use Optionmenu"));
	gtk_box_pack_start
		(GTK_BOX (bandsframevbox), bandsradiobutton2, FALSE, FALSE, 0);
	bandsframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (bandsframevbox), bandsframe, FALSE, FALSE, 0);
	bandseditvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (bandsframe), bandseditvbox);
	bandslabel = gtk_label_new (_("Comma separated list of bands"));
	gtk_box_pack_start (GTK_BOX (bandseditvbox), bandslabel, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (bandslabel), GTK_JUSTIFY_CENTER);
	bandsentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (bandseditvbox), bandsentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (bandsentry), 200);
	gtk_entry_set_activates_default (GTK_ENTRY (bandsentry), TRUE);

	remotedataframe = gtk_frame_new (_("Remote data"));
	gtk_box_pack_start (GTK_BOX (vboxpage1), remotedataframe, FALSE, FALSE, 0);
	remotedatavbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (remotedataframe), remotedatavbox);
	remotedatalabel = gtk_label_new
		(_("When receiving data from another application (gmfsk, ktrack):"));
	gtk_box_pack_start
		(GTK_BOX (remotedatavbox), remotedatalabel, FALSE, FALSE, 0);
	gtk_label_set_line_wrap (GTK_LABEL (remotedatalabel), TRUE);
	remotedataseparator = gtk_hseparator_new ();
	gtk_box_pack_start
		(GTK_BOX (remotedatavbox), remotedataseparator, FALSE, FALSE, 10);
	remoteradiobutton1 = gtk_radio_button_new_with_label
		(NULL, _("Add data directly to the log"));
	gtk_box_pack_start
		(GTK_BOX (remotedatavbox), remoteradiobutton1, FALSE, FALSE, 0);
	remoteradiobutton_group = gtk_radio_button_get_group 
		(GTK_RADIO_BUTTON (remoteradiobutton1));
	remoteradiobutton2 = gtk_radio_button_new_with_label
		(remoteradiobutton_group, _("Add data to the QSO frame"));
	gtk_box_pack_start
		(GTK_BOX (remotedatavbox), remoteradiobutton2, FALSE, FALSE, 0);

	/* page 2, information */
	vboxpage3 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (preferencesnotebook), vboxpage3);

	callsignframe = gtk_frame_new (_("Callsign"));
	gtk_box_pack_start (GTK_BOX (vboxpage3), callsignframe, FALSE, FALSE, 0);
	callsignhbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (callsignframe), callsignhbox);
	callsignlabel = gtk_label_new (_("Your Callsign"));
	gtk_box_pack_start (GTK_BOX (callsignhbox), callsignlabel, FALSE, FALSE, 0);
	callsignentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (callsignhbox), callsignentry, TRUE, TRUE, 10);
	gtk_entry_set_max_length (GTK_ENTRY (callsignentry), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (callsignentry), TRUE);

	locationframe = gtk_frame_new (_("Location"));
	gtk_box_pack_start (GTK_BOX (vboxpage3), locationframe, FALSE, FALSE, 0);
	locationvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (locationframe), locationvbox);
	labelloc = gtk_label_new (_("Your location (ddd.mm)"));
	gtk_box_pack_start (GTK_BOX (locationvbox), labelloc, FALSE, FALSE, 0);
	lathbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (locationvbox), lathbox, FALSE, FALSE, 0);
	latentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (lathbox), latentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (latentry), 8);
	gtk_entry_set_activates_default (GTK_ENTRY (latentry), TRUE);
	NScombo = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (lathbox), NScombo, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (NScombo), _("N"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (NScombo), _("S"));
	longentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (lathbox), longentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (longentry), 8);
	gtk_entry_set_activates_default (GTK_ENTRY (longentry), TRUE);
	EWcombo = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (lathbox), EWcombo, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (EWcombo), _("E"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (EWcombo), _("W"));
	locatorhbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (locationvbox), locatorhbox, FALSE, FALSE, 0);
	locatorlabel = gtk_label_new (_("QTH locator"));
	gtk_box_pack_start (GTK_BOX (locatorhbox), locatorlabel, FALSE, FALSE, 0);
	qthlocatorentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (locatorhbox), qthlocatorentry, TRUE, TRUE, 10);
	gtk_entry_set_max_length (GTK_ENTRY (qthlocatorentry), 8);
	gtk_entry_set_activates_default (GTK_ENTRY (qthlocatorentry), TRUE);
	unitsframe = gtk_frame_new (_("Units"));
	gtk_box_pack_start (GTK_BOX (vboxpage3), unitsframe, FALSE, FALSE, 0);
	unitshbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (unitsframe), unitshbox);
	labeldist = gtk_label_new (_("Display distance in"));
	gtk_box_pack_start (GTK_BOX (unitshbox), labeldist, FALSE, FALSE, 0);
	unitscombo = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (unitshbox), unitscombo, TRUE, TRUE, 10);
	gtk_combo_box_append_text (GTK_COMBO_BOX (unitscombo), _("Kilometers"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (unitscombo), _("Miles"));
	GtkWidget *lookupframe = gtk_frame_new (_("Lookup"));
	gtk_box_pack_start (GTK_BOX (vboxpage3), lookupframe, FALSE, FALSE, 0);
	GtkWidget *lookupvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (lookupframe), lookupvbox);
	GtkWidget *labellookup = gtk_label_new
(_("When clicking the \"?\" next to the callsign you can use your "
	"internet browser to look it up. \"<call>\" in the lookup-URL will "
	"be replaced by the callsign"));
	gtk_label_set_line_wrap (GTK_LABEL (labellookup), TRUE);
	gtk_box_pack_start (GTK_BOX (lookupvbox), labellookup, FALSE, FALSE, 0);
	GtkWidget *lookupentry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (lookupentry), preferences.openurl);
	gtk_box_pack_start (GTK_BOX (lookupvbox), lookupentry, FALSE, FALSE, 0);
	GtkWidget *keyerframe = gtk_frame_new (_("Keyer"));
	gtk_box_pack_start (GTK_BOX (vboxpage3), keyerframe, FALSE, FALSE, 0);
	GtkWidget *keyervbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (keyerframe), keyervbox);
	GtkWidget *labelkeyer = gtk_label_new
(_("Initial exchange to use for \"~\" (see the manual for details)"));
	gtk_label_set_line_wrap (GTK_LABEL (labelkeyer), TRUE);
	gtk_box_pack_start (GTK_BOX (keyervbox), labelkeyer, FALSE, FALSE, 0);
	GtkWidget *lastmsgentry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (lastmsgentry), preferences.initlastmsg);
	gtk_box_pack_start (GTK_BOX (keyervbox), lastmsgentry, FALSE, FALSE, 0);

	preferenceslabel3 = gtk_label_new (_("Info"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (preferencesnotebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (preferencesnotebook), 1),
		preferenceslabel3);

	/* page 3, hamlib */
	vboxpage4 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (preferencesnotebook), vboxpage4);

	hamlibcheckbutton = gtk_check_button_new_with_label 
		(_("Enable hamlib support"));
	gtk_box_pack_start
		(GTK_BOX (vboxpage4), hamlibcheckbutton, FALSE, FALSE, 0);

	hamlibframe = gtk_frame_new (_("Settings"));
	gtk_box_pack_start (GTK_BOX (vboxpage4), hamlibframe, FALSE, FALSE, 0);
	hamlibvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (hamlibframe), hamlibvbox);

	radiohbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hamlibvbox), radiohbox, FALSE, FALSE, 0);

	radioentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (radiohbox), radioentry, TRUE, TRUE, 0);
	radiobutton = gtk_button_new_with_mnemonic (_("_Select a Radio"));
	gtk_box_pack_start (GTK_BOX (radiohbox), radiobutton, TRUE, TRUE, 0);
	gtk_editable_set_editable (GTK_EDITABLE (radioentry), FALSE);
	gtk_widget_set_can_focus (GTK_WIDGET (radioentry), FALSE);

	devicehbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hamlibvbox), devicehbox, FALSE, FALSE, 0);
	devicelabel = gtk_label_new (_("Attach to Device"));
	gtk_box_pack_start (GTK_BOX (devicehbox), devicelabel, TRUE, TRUE, 0);
	devicecombo = gtk_combo_box_entry_new_text ();
	gtk_box_pack_start (GTK_BOX (devicehbox), devicecombo, TRUE, TRUE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "localhost");
#ifdef G_OS_WIN32
	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "com1");
	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "com2");
#else
	dev_serial_dir = g_dir_open(serial_file_dir, 0, NULL);
	if (dev_serial_dir != NULL) {
	  while ((serial_file_name = g_dir_read_name(dev_serial_dir)) != NULL) {
	    absolute_serial_file_name = g_strdup_printf ("%s/%s", serial_file_dir, serial_file_name);
	    gtk_combo_box_append_text  (GTK_COMBO_BOX (devicecombo), absolute_serial_file_name);
	  }
	  g_dir_close(dev_serial_dir);
	}

	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "/dev/ttyS0");
	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "/dev/ttyS1");
	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "/dev/ttyS2");
	gtk_combo_box_append_text (GTK_COMBO_BOX (devicecombo), "/dev/ttyS3");

	dev_dir = g_dir_open(dev_file_dir, 0, NULL);
	if (dev_dir != NULL) {
	  while ((dev_file_name = g_dir_read_name(dev_dir)) != NULL) {
	    if (! g_ascii_strncasecmp(dev_file_name, "ttyUSB", 6)) {
	      absolute_dev_file_name = g_strdup_printf ("%s/%s", dev_file_dir, dev_file_name);
	      gtk_combo_box_append_text  (GTK_COMBO_BOX (devicecombo), absolute_dev_file_name);
	    }
	  }
	  g_dir_close(dev_dir);
	}

#endif
	gtk_entry_set_max_length (GTK_ENTRY (GTK_BIN(devicecombo)->child), 100);
	gtk_entry_set_activates_default
	  (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (devicecombo))), TRUE);

	hamlibseparator = gtk_hseparator_new ();
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), hamlibseparator, FALSE, FALSE, 10);
	configurelabel = gtk_label_new
		(_("Comma separated list of commands for configuring your hamlib port"));
	gtk_box_pack_start (GTK_BOX (hamlibvbox), configurelabel, FALSE, FALSE, 0);
	gtk_label_set_line_wrap (GTK_LABEL (configurelabel), TRUE);

	rigsetconfentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hamlibvbox), rigsetconfentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (rigsetconfentry), 80);
	gtk_widget_set_tooltip_text (rigsetconfentry, 
		_("example: rts_state=ON,timeout=600,serial_speed=19200"));
	gtk_entry_set_activates_default (GTK_ENTRY (rigsetconfentry), TRUE);

	hamlibseparator = gtk_hseparator_new ();
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), hamlibseparator, FALSE, FALSE, 10);
	fcccheckbutton = gtk_check_button_new_with_label
		(_("Use FCC emission designators for modes (e.g. A1A, J3E, etc.)"));
	gtk_label_set_line_wrap
                (GTK_LABEL (GTK_BIN (fcccheckbutton)->child), TRUE);
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), fcccheckbutton, FALSE, FALSE, 0);
	hamlibseparator = gtk_hseparator_new ();
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), hamlibseparator, FALSE, FALSE, 10);
	pollingcheckbutton = gtk_check_button_new_with_label
		(_("Enable polling"));
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), pollingcheckbutton, FALSE, FALSE, 0);

	pollingframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (hamlibvbox), pollingframe, FALSE, FALSE, 0);
	pollinghbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (pollingframe), pollinghbox);
	pollinglabel = gtk_label_new (_("Poll rig every"));
	gtk_box_pack_start (GTK_BOX (pollinghbox), pollinglabel, FALSE, FALSE, 0);
	pollcombo = gtk_combo_box_entry_new_text ();
	gtk_box_pack_start (GTK_BOX (pollinghbox), pollcombo, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "50");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "100");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "200");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "300");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "400");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "500");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "700");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "1000");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "2000");
	gtk_combo_box_append_text (GTK_COMBO_BOX (pollcombo), "3000");
	gtk_editable_set_editable
		(GTK_EDITABLE (GTK_BIN(pollcombo)->child), FALSE);
	gtk_widget_set_can_focus
		(GTK_WIDGET (GTK_BIN(pollcombo)->child), FALSE);
	gtk_widget_set_size_request (pollcombo, 100, -1);
	secondslabel = gtk_label_new (_("milliseconds"));
	gtk_box_pack_start (GTK_BOX (pollinghbox), secondslabel, FALSE, FALSE, 0);

	smetercheckbutton = gtk_check_button_new_with_label
		(_("Show S-meter on statusbar"));
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), smetercheckbutton, FALSE, FALSE, 0);
	frequencycheckbutton = gtk_check_button_new_with_label
		(_("Show frequency on statusbar"));
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), frequencycheckbutton, FALSE, FALSE, 0);

	hamlibseparator = gtk_hseparator_new ();
	gtk_box_pack_start
		(GTK_BOX (hamlibvbox), hamlibseparator, FALSE, FALSE, 10);
	digitshbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hamlibvbox), digitshbox, FALSE, FALSE, 0);
	digitslabel = gtk_label_new (_("Round frequency at digit"));
	gtk_box_pack_start (GTK_BOX (digitshbox), digitslabel, FALSE, FALSE, 0);
	digitscombo = gtk_combo_box_entry_new_text ();
	gtk_box_pack_start (GTK_BOX (digitshbox), digitscombo, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (digitscombo), "0");
	gtk_combo_box_append_text (GTK_COMBO_BOX (digitscombo), "1");
	gtk_combo_box_append_text (GTK_COMBO_BOX (digitscombo), "2");
	gtk_combo_box_append_text (GTK_COMBO_BOX (digitscombo), "3");
	gtk_combo_box_append_text (GTK_COMBO_BOX (digitscombo), "4");
	gtk_combo_box_append_text (GTK_COMBO_BOX (digitscombo), "5");
	gtk_editable_set_editable
		(GTK_EDITABLE (GTK_BIN(digitscombo)->child), FALSE);
	gtk_widget_set_can_focus
		(GTK_WIDGET (GTK_BIN(digitscombo)->child), FALSE);
	gtk_widget_set_size_request (digitscombo, 100, -1);
	characterslabel = gtk_label_new ("(0 = 1 Hz, 1 = 10 Hz, ...)");
	gtk_box_pack_start (GTK_BOX (digitshbox), characterslabel, FALSE, FALSE, 0);

	preferenceslabel4 = gtk_label_new ("Hamlib");
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (preferencesnotebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (preferencesnotebook), 2),
		preferenceslabel4);

	/* page 4, logs */
	vboxpage5 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (preferencesnotebook), vboxpage5);

	GtkWidget *reqlabel = gtk_label_new (_("Changing settings in the Logs section "
		"of this page requires a restart!"));
	gtk_box_pack_start (GTK_BOX (vboxpage5), reqlabel, FALSE, FALSE, 0);
	logsframe = gtk_frame_new (_("Logs"));
	gtk_box_pack_start (GTK_BOX (vboxpage5), logsframe, FALSE, FALSE, 0);
	logsvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (logsframe), logsvbox);
	pathlabel = gtk_label_new (_("Directory where the logs will be saved"));
	gtk_box_pack_start (GTK_BOX (logsvbox), pathlabel, FALSE, FALSE, 0);
	pathhbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (logsvbox), pathhbox, FALSE, FALSE, 0);
	pathentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (pathhbox), pathentry, TRUE, TRUE, 0);
	gtk_editable_set_editable (GTK_EDITABLE (pathentry), FALSE);
	gtk_widget_set_can_focus (pathentry, FALSE);
	pathbutton = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_widget_set_size_request (pathbutton, 100, -1);
	gtk_box_pack_start (GTK_BOX (pathhbox), pathbutton, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text
		(pathbutton, _("Click here to change the path"));

	logshseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (logsvbox), logshseparator, TRUE, TRUE, 10);

	loadlabel = gtk_label_new 
		(_("Comma separated list of logs to load at startup"));
	gtk_box_pack_start (GTK_BOX (logsvbox), loadlabel, FALSE, FALSE, 0);
	gtk_label_set_line_wrap (GTK_LABEL (loadlabel), TRUE);
	logsentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (logsvbox), logsentry, FALSE, FALSE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (logsentry), 90);
	gtk_entry_set_activates_default (GTK_ENTRY (logsentry), TRUE);

	logshseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (logsvbox), logshseparator, TRUE, TRUE, 10);

	GtkWidget *orderhbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (logsvbox), orderhbox, FALSE, FALSE, 0);
	GtkWidget *orderlabel = gtk_label_new 
		(_("Load logs by"));
	gtk_box_pack_start (GTK_BOX (orderhbox), orderlabel, FALSE, FALSE, 0);
	GtkWidget *ordercombo = gtk_combo_box_entry_new_text ();
	gtk_box_pack_start (GTK_BOX (orderhbox), ordercombo, FALSE, FALSE, 0);
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (ordercombo), _("Time of last modification"));
	gtk_combo_box_append_text (GTK_COMBO_BOX (ordercombo), _("Alphabet"));
	gtk_editable_set_editable
		(GTK_EDITABLE (GTK_BIN(ordercombo)->child), FALSE);
	gtk_widget_set_can_focus
		(GTK_WIDGET (GTK_BIN(ordercombo)->child), FALSE);

	savingframe = gtk_frame_new (_("Saving"));
	gtk_box_pack_start (GTK_BOX (vboxpage5), savingframe, FALSE, FALSE, 0);
	savingvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (savingframe), savingvbox);
	saveradiobutton = gtk_radio_button_new_with_label
		(NULL, _("Save with every log change"));
	gtk_box_pack_start
		(GTK_BOX (savingvbox), saveradiobutton, FALSE, FALSE, 0);
	saveradiobutton_group = gtk_radio_button_get_group
                (GTK_RADIO_BUTTON (saveradiobutton));
	autosaveradiobutton = gtk_radio_button_new_with_label
		(saveradiobutton_group, _("Enable autosave"));
	gtk_box_pack_start
		(GTK_BOX (savingvbox), autosaveradiobutton, FALSE, FALSE, 0);

	autosaveframe = gtk_frame_new (NULL);
	gtk_box_pack_start
		(GTK_BOX (savingvbox), autosaveframe, FALSE, FALSE, 0);
	autosavehbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (autosaveframe), autosavehbox);
	autosavelabel = gtk_label_new (_("Autosave logs every"));
	gtk_box_pack_start (GTK_BOX (autosavehbox), autosavelabel, FALSE, FALSE, 0);
	autosavecombo = gtk_combo_box_entry_new_text ();
	gtk_box_pack_start (GTK_BOX (autosavehbox), autosavecombo, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "0");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "5");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "10");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "20");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "30");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "40");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "50");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "60");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "90");
	gtk_combo_box_append_text (GTK_COMBO_BOX (autosavecombo), "120");
	gtk_editable_set_editable
		(GTK_EDITABLE (GTK_BIN(autosavecombo)->child), FALSE);
	gtk_widget_set_can_focus
		(GTK_WIDGET (GTK_BIN(autosavecombo)->child), FALSE);
	gtk_widget_set_size_request (autosavecombo, 100, -1);
	minuteslabel = gtk_label_new (_("minutes"));
	gtk_box_pack_start (GTK_BOX (autosavehbox), minuteslabel, FALSE, FALSE, 0);

	backupframe = gtk_frame_new (_("Backup"));
	gtk_box_pack_start
		(GTK_BOX (vboxpage5), backupframe, FALSE, FALSE, 0);
	backupvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (backupframe), backupvbox);
	backupradiobutton1 = gtk_radio_button_new_with_label
		(NULL, _("Use standard backup procedure"));
	gtk_box_pack_start
		(GTK_BOX (backupvbox), backupradiobutton1, FALSE, FALSE, 0);
	backupradiobutton_group = gtk_radio_button_get_group 
		(GTK_RADIO_BUTTON (backupradiobutton1));
	backupradiobutton2 = gtk_radio_button_new_with_label
		(backupradiobutton_group, _("Copy logs to a safe location"));
	gtk_box_pack_start
		(GTK_BOX (backupvbox), backupradiobutton2, FALSE, FALSE, 0);
	logshseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (backupvbox), logshseparator, TRUE, TRUE, 10);
	backuphbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (backupvbox), backuphbox, FALSE, FALSE, 0);
	backupentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (backuphbox), backupentry, TRUE, TRUE, 0);
	gtk_editable_set_editable (GTK_EDITABLE (backupentry), FALSE);
	gtk_widget_set_can_focus (backupentry, FALSE);
	backupbutton = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_widget_set_size_request (backupbutton, 100, -1);
	gtk_box_pack_start (GTK_BOX (backuphbox), backupbutton, FALSE, FALSE, 0);
	gtk_widget_set_tooltip_text
		(backupbutton, _("Click here to change the path"));

	fontframe = gtk_frame_new (_("Font"));
	gtk_box_pack_start (GTK_BOX (vboxpage5), fontframe, FALSE, FALSE, 0); 	 
	fontvbox = gtk_vbox_new (FALSE, 0); 	 
	gtk_container_add (GTK_CONTAINER (fontframe), fontvbox);
	fontlabel = gtk_label_new (_("Font to use for the log")); 	 
	gtk_box_pack_start (GTK_BOX (fontvbox), fontlabel, FALSE, FALSE, 0);
	fontseparator = gtk_hseparator_new (); 	 
	gtk_box_pack_start (GTK_BOX (fontvbox), fontseparator, FALSE, FALSE, 0);
	fonthbox = gtk_hbox_new (FALSE, 0); 	 
	gtk_box_pack_start (GTK_BOX (fontvbox), fonthbox, FALSE, FALSE, 0);
	fontentry = gtk_entry_new (); 	 
	gtk_box_pack_start (GTK_BOX (fonthbox), fontentry, TRUE, TRUE, 10); 	 
	gtk_editable_set_editable (GTK_EDITABLE (fontentry), FALSE); 	 
	gtk_widget_set_can_focus (fontentry, FALSE); 	 
	fontbutton = gtk_button_new_from_stock (GTK_STOCK_SELECT_FONT); 	 
	gtk_widget_set_size_request (fontbutton, 100, -1); 	 
	gtk_box_pack_start (GTK_BOX (fonthbox), fontbutton, FALSE, FALSE, 0);

	preferenceslabel5 = gtk_label_new (_("Logs"));
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (preferencesnotebook),
		gtk_notebook_get_nth_page (GTK_NOTEBOOK (preferencesnotebook), 3),
		preferenceslabel5);

	/**************************/
	/* settings of the dialog */
	/**************************/

	/* remote logging */
	if (preferences.remoteadding == 1)
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (remoteradiobutton1), TRUE);
	else
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (remoteradiobutton2), TRUE);

	/* type ahead find */
	if (preferences.typeaheadfind == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
			(typeaheadfindcheckbutton), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
			(typeaheadfindcheckbutton), FALSE);

	/* distance and azimuth */
	if (preferences.distqrb == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
			(distqrbcheckbutton), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
			(distqrbcheckbutton), FALSE);

	/* path for the logs */
	gtk_entry_set_text (GTK_ENTRY (pathentry), preferences.savedir);

	/* logs to load at startup */
	gtk_entry_set_text (GTK_ENTRY (logsentry), preferences.logstoload);
	gtk_combo_box_set_active (GTK_COMBO_BOX(ordercombo), preferences.logorder);

	/* callsign */
	gtk_entry_set_text (GTK_ENTRY (callsignentry), preferences.callsign);

	/* saving */
	temp = g_strdup_printf ("%d", preferences.autosave);
	gtk_entry_set_text (GTK_ENTRY (GTK_BIN(autosavecombo)->child), temp);
	if (preferences.saving == 1)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(autosaveradiobutton), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(saveradiobutton), TRUE);
		gtk_widget_set_sensitive(autosaveframe, FALSE);
	}

	/* backup */
	gtk_entry_set_text (GTK_ENTRY (backupentry), preferences.backupdir);
	if (preferences.backup == 1)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(backupradiobutton1), TRUE);
		gtk_widget_set_sensitive(backupentry, FALSE);
		gtk_widget_set_sensitive(backupbutton, FALSE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(backupradiobutton2), TRUE);
	}

	/* clock */
	if (preferences.clock == 0)
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (clockcheckbutton), FALSE);
	else
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (clockcheckbutton), TRUE);

	/* are you sure */
	if (preferences.areyousure == 1)
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (areyousurecheckbutton), TRUE);
	else
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (areyousurecheckbutton), FALSE);

	/* font */ 	 
	gtk_entry_set_text (GTK_ENTRY (fontentry), preferences.logfont);

	if (preferences.hamlib == 0)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (hamlibcheckbutton), FALSE);
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (frequencycheckbutton), FALSE);
		gtk_widget_set_sensitive (hamlibframe, 0);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (hamlibcheckbutton),	TRUE);
		gtk_widget_set_sensitive (hamlibframe, 1);
		if (preferences.hamlib == 2)
		{
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (frequencycheckbutton), TRUE);
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (smetercheckbutton),	FALSE);
		}
		else if (preferences.hamlib == 3)
		{
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (frequencycheckbutton), FALSE);
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (smetercheckbutton), TRUE);
		}
		else if (preferences.hamlib == 4)
		{
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (frequencycheckbutton), TRUE);
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (smetercheckbutton), TRUE);
		}
	}

	gtk_entry_set_text (GTK_ENTRY (GTK_BIN(devicecombo)->child), 
		preferences.device);
	temp = g_strdup_printf ("%d", preferences.rigid);
	gtk_entry_set_text (GTK_ENTRY (radioentry), temp);

	temp = g_strdup_printf ("%d", preferences.round);
	gtk_entry_set_text (GTK_ENTRY (GTK_BIN(digitscombo)->child), temp);
	gtk_entry_set_text (GTK_ENTRY (rigsetconfentry), preferences.rigconf);

	if (preferences.fcc == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fcccheckbutton), TRUE);

	/* polling */
	if (preferences.polltime == 0)
	{
		gtk_entry_set_text (GTK_ENTRY (GTK_BIN(pollcombo)->child), "300");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pollingcheckbutton), 
			FALSE);
		gtk_widget_set_sensitive (pollingframe, FALSE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pollingcheckbutton), 
			TRUE);
		temp = g_strdup_printf ("%d", preferences.polltime);
		gtk_entry_set_text (GTK_ENTRY (GTK_BIN(pollcombo)->child), temp);
	}

	/* location */
	temp = g_strdup_printf ("%3.2f", preferences.latitude);
	gtk_entry_set_text (GTK_ENTRY (latentry), temp);
	if (preferences.NS == 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (NScombo), 0);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX (NScombo), 1);
	temp = g_strdup_printf ("%3.2f", preferences.longitude);
	gtk_entry_set_text (GTK_ENTRY (longentry), temp);
	if (preferences.EW == 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (EWcombo), 0);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX (EWcombo), 1);
	if (preferences.units == 1)
		gtk_combo_box_set_active (GTK_COMBO_BOX (unitscombo), 0);
	else
		gtk_combo_box_set_active (GTK_COMBO_BOX (unitscombo), 1);
	g_free (temp);

	/* locator */
	gtk_entry_set_text (GTK_ENTRY (qthlocatorentry), preferences.locator);

	/* modes and bands */
	gtk_entry_set_text (GTK_ENTRY (modesentry), preferences.modes);
	gtk_entry_set_text (GTK_ENTRY (bandsentry), preferences.bands);
	/* set state of radio buttons and sensitivity of associated widgets */
	if (preferences.modeseditbox == 0)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (modesradiobutton2),	TRUE);
		gtk_widget_set_sensitive (modeslabel, 1);
		gtk_widget_set_sensitive (modesentry, 1);
	}
	else
	{
		gtk_toggle_button_set_active 
			(GTK_TOGGLE_BUTTON (modesradiobutton1),	TRUE);
		gtk_widget_set_sensitive (modeslabel, 0);
		gtk_widget_set_sensitive (modesentry, 0);
	}
	if (preferences.bandseditbox == 0)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (bandsradiobutton2),	TRUE);
		gtk_widget_set_sensitive (bandslabel, 1);
		gtk_widget_set_sensitive (bandsentry, 1);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (bandsradiobutton1),	TRUE);
		gtk_widget_set_sensitive (bandslabel, 0);
		gtk_widget_set_sensitive (bandsentry, 0);
	}

	/************************/
	/* signals and pointers */
	/************************/

	/* page 1 */
	GLADE_HOOKUP_OBJECT (preferencesdialog, fontentry, "fontentry"); 	 
	g_signal_connect ((gpointer) fontbutton, "clicked", 	 
		G_CALLBACK (on_fontbutton_clicked), NULL);
	GLADE_HOOKUP_OBJECT (preferencesdialog, modeslabel, "modeslabel");
	GLADE_HOOKUP_OBJECT (preferencesdialog, modesentry, "modesentry");
	GLADE_HOOKUP_OBJECT (preferencesdialog, bandslabel, "bandslabel");
	GLADE_HOOKUP_OBJECT (preferencesdialog, bandsentry, "bandsentry");
	g_signal_connect ((gpointer) modesradiobutton1, "toggled",
		G_CALLBACK (on_modesradiobutton_toggled), "1");
	g_signal_connect ((gpointer) modesradiobutton2, "toggled",
		G_CALLBACK (on_modesradiobutton_toggled), "2");
	g_signal_connect ((gpointer) bandsradiobutton1, "toggled",
		G_CALLBACK (on_bandsradiobutton_toggled), "1");
	g_signal_connect ((gpointer) bandsradiobutton2, "toggled",
		G_CALLBACK (on_bandsradiobutton_toggled), "2");

	/* page 2 */
	g_signal_connect (latentry, "changed", G_CALLBACK (latlon_changed), NULL);
	g_signal_connect (longentry, "changed", G_CALLBACK (latlon_changed), NULL);
	g_signal_connect (NScombo, "changed", G_CALLBACK (latlon_changed), NULL);
	g_signal_connect (EWcombo, "changed", G_CALLBACK (latlon_changed), NULL);
	g_signal_connect (qthlocatorentry, "changed", G_CALLBACK (qra_changed), NULL);
	GLADE_HOOKUP_OBJECT (preferencesdialog, qthlocatorentry, "qthlocatorentry");
	GLADE_HOOKUP_OBJECT (preferencesdialog, latentry, "latentry");
	GLADE_HOOKUP_OBJECT (preferencesdialog, longentry, "longentry");
	GLADE_HOOKUP_OBJECT (preferencesdialog, NScombo, "NScombo");
	GLADE_HOOKUP_OBJECT (preferencesdialog, EWcombo, "EWcombo");
	GLADE_HOOKUP_OBJECT (preferencesdialog, lookupentry, "lookupentry");

	/* page 3, hamlib */
	GLADE_HOOKUP_OBJECT (preferencesdialog, hamlibframe, "hamlibframe");
	GLADE_HOOKUP_OBJECT (preferencesdialog, pollingframe, "pollingframe");
	GLADE_HOOKUP_OBJECT (preferencesdialog, pollingcheckbutton,
		"pollingcheckbutton");
	GLADE_HOOKUP_OBJECT (preferencesdialog, frequencycheckbutton,
		"frequencycheckbutton");
	GLADE_HOOKUP_OBJECT (preferencesdialog, radioentry,	"radioentry");
	g_signal_connect ((gpointer) hamlibcheckbutton, "toggled",
		G_CALLBACK (on_hamlibcheckbutton_toggled), NULL);
	g_signal_connect ((gpointer) pollingcheckbutton, "toggled",
		G_CALLBACK (on_pollingcheckbutton_toggled), NULL);
	g_signal_connect (G_OBJECT (radiobutton), "clicked",
		G_CALLBACK (on_radiobutton_clicked), NULL);

	/* page 4 */
	GLADE_HOOKUP_OBJECT (preferencesdialog, pathentry, "pathentry");
	GLADE_HOOKUP_OBJECT (preferencesdialog, autosaveframe, "autosaveframe");
	GLADE_HOOKUP_OBJECT (preferencesdialog, backupentry, "backupentry");
	GLADE_HOOKUP_OBJECT (preferencesdialog, backupbutton, "backupbutton");
	g_signal_connect ((gpointer) pathbutton, "clicked",
		G_CALLBACK (on_pathbutton_clicked), GINT_TO_POINTER(1));
	g_signal_connect ((gpointer) autosaveradiobutton, "toggled",
		G_CALLBACK (on_autosaveradiobutton_toggled), NULL);
	g_signal_connect ((gpointer) backupbutton, "clicked",
		G_CALLBACK (on_pathbutton_clicked), GINT_TO_POINTER(2));
	g_signal_connect ((gpointer) backupradiobutton2, "toggled",
		G_CALLBACK (on_backupradiobutton_toggled), NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (preferencesdialog),
					 GTK_RESPONSE_OK);

	/******************/
	/* run the dialog */
	/******************/

	gtk_widget_show_all (preferencesdialog);
	response = gtk_dialog_run (GTK_DIALOG(preferencesdialog));

	if (response == GTK_RESPONSE_OK)
	{
		/* check autosave value */
		if (gtk_toggle_button_get_active 
				(GTK_TOGGLE_BUTTON (autosaveradiobutton)))
			saving = 1;
		else if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (saveradiobutton)))
			saving = 2;
		else
			saving = 0;
		temp = gtk_editable_get_chars
			(GTK_EDITABLE (GTK_BIN(autosavecombo)->child), 0, -1);
		autosavevalue = atoi (temp);
		set_autosave (autosavevalue, saving);

		/* check backup value */
		if (gtk_toggle_button_get_active 
				(GTK_TOGGLE_BUTTON (backupradiobutton1)))
			preferences.backup = 1;
		else
			preferences.backup = 2;

		/* get backup path for the logs */
		temp = gtk_editable_get_chars (GTK_EDITABLE (backupentry), 0, -1);
		set_backuppath (temp);
			
		/* are you sure */
		if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (areyousurecheckbutton)))
			preferences.areyousure = 1;
		else
			preferences.areyousure = 0;


		/* check type ahead find */
		if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (typeaheadfindcheckbutton)))
			preferences.typeaheadfind = 1;
		else
			preferences.typeaheadfind = 0;

		/* check distance and azimuth */
		if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (distqrbcheckbutton)))
			preferences.distqrb = 1;
		else
			preferences.distqrb = 0;

		/* remote logging */
		if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (remoteradiobutton1)))
			preferences.remoteadding = 1;
		else
			preferences.remoteadding = 2;

		/* font */
		font = gtk_editable_get_chars (GTK_EDITABLE (fontentry), 0, -1); 	 
		set_font (font); 	 
		g_free (font);

		/* get path for the logs */
		temp = gtk_editable_get_chars (GTK_EDITABLE (pathentry), 0, -1);
		set_path (temp);

		/* get logs to load */
		temp = gtk_editable_get_chars (GTK_EDITABLE (logsentry), 0, -1);
		set_logstoload (temp);
		preferences.logorder = gtk_combo_box_get_active (GTK_COMBO_BOX(ordercombo));

		/* locator */
		temp = gtk_editable_get_chars (GTK_EDITABLE (qthlocatorentry), 0, -1);
		set_qthlocator (temp);

		/* callsign */
		temp = gtk_editable_get_chars (GTK_EDITABLE (callsignentry), 0, -1);
		set_callsign (temp);

		/* set latitude, longitude and units */
		temp = gtk_editable_get_chars (GTK_EDITABLE (latentry), 0, -1);
		g_strstrip (temp);
		if (strlen (temp) == 0)
			preferences.latitude = 0.0;
		else
			preferences.latitude = atof (temp);
		temp = gtk_editable_get_chars (GTK_EDITABLE (longentry), 0, -1);
		g_strstrip (temp);
		if (strlen (temp) == 0)
			preferences.longitude = 0.0;
		else
			preferences.longitude = atof (temp);

		preferences.NS = gtk_combo_box_get_active (GTK_COMBO_BOX (NScombo));
		preferences.EW = gtk_combo_box_get_active (GTK_COMBO_BOX (EWcombo));
		preferences.units = gtk_combo_box_get_active 
			(GTK_COMBO_BOX (unitscombo))  == 1 ? 0 : 1;

		/* lookup URL */
		temp = gtk_editable_get_chars (GTK_EDITABLE (lookupentry), 0, -1);
		preferences.openurl = g_strdup (temp);

		/* lastmessage for the keyer */
		temp = gtk_editable_get_chars (GTK_EDITABLE (lastmsgentry), 0, -1);
		preferences.initlastmsg = g_strdup (temp);

		/* get the comma seperated list of modes and assign 
		 * to the optionmenu */
		temp = gtk_editable_get_chars (GTK_EDITABLE (modesentry), 0, -1);
		makemodeoptionmenu (temp);
		preferences.modes = g_strdup (temp);

		/* get the comma seperated list of bands and assign to the optionmenu */
		temp = gtk_editable_get_chars (GTK_EDITABLE (bandsentry), 0, -1);
		makebandoptionmenu (temp);
		preferences.bands = g_strdup (temp);

		/* set visibility of widgets in QSO frame depending on 
		 * state of toggle buttons */
		bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
		bandentry = lookup_widget (mainwindow, "bandentry");
		bandlabel = g_object_get_data (G_OBJECT (mainwindow), "mhzlabel");
		toggletrue = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(bandsradiobutton1));
		if (toggletrue)
		{
			preferences.bandseditbox = 1;
			gtk_label_set_mnemonic_widget (GTK_LABEL(bandlabel), bandentry);
			gtk_widget_hide (bandoptionmenu);
			gtk_widget_show (bandentry);
		}
		else
		{
			preferences.bandseditbox = 0;
			gtk_label_set_mnemonic_widget (GTK_LABEL(bandlabel), bandoptionmenu);
			gtk_widget_hide (bandentry);
			gtk_widget_show (bandoptionmenu);
		}

		modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
		modeentry = lookup_widget (mainwindow, "modeentry");
		modelabel = g_object_get_data (G_OBJECT (mainwindow), "modelabel");
		toggletrue = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(modesradiobutton1));
		if (toggletrue)
		{
			preferences.modeseditbox = 1;
			gtk_label_set_mnemonic_widget (GTK_LABEL(modelabel), modeentry);
			gtk_widget_hide (modeoptionmenu);
			gtk_widget_show (modeentry);
		}
		else
		{
			preferences.modeseditbox = 0;
			gtk_label_set_mnemonic_widget (GTK_LABEL(modelabel), modeoptionmenu);
			gtk_widget_hide (modeentry);
			gtk_widget_show (modeoptionmenu);
		}

		/* hamlib */
		hamlibyes = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (hamlibcheckbutton));
		frequencyyesno = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (frequencycheckbutton));
		smeteryesno = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (smetercheckbutton));
		pollingyes = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (pollingcheckbutton));
		fccyes = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (fcccheckbutton));
		radio = gtk_editable_get_chars (GTK_EDITABLE (radioentry), 0, -1);
		device = gtk_editable_get_chars
			(GTK_EDITABLE (GTK_BIN(devicecombo)->child), 0, -1);
		digits = gtk_editable_get_chars
			(GTK_EDITABLE (GTK_BIN(digitscombo)->child), 0, -1);
		if (pollingyes)
			pollstr = gtk_editable_get_chars 
				(GTK_EDITABLE (GTK_BIN(pollcombo)->child), 0, -1);
		else
			pollstr = g_strdup ("0");
		rigconf = gtk_editable_get_chars
			(GTK_EDITABLE (rigsetconfentry), 0, -1);

		if (hamlibyes)
			hamlibwidgets =	whichhamlibwidgets (frequencyyesno, smeteryesno);
		else
			hamlibwidgets = 0;

		if (smeteryesno)
			sethamlibwidgets (hamlibwidgets, TRUE);
		else
			sethamlibwidgets (hamlibwidgets, FALSE);

		preferences.round = atoi (digits);
		if (fccyes)
			preferences.fcc = 1; 
		else 
			preferences.fcc = 0;

		/* hamlib should be started or restarted because settings have changed */
		if (hamlib_changed
			(hamlibwidgets, atoi(radio), device, atoi(pollstr), rigconf))
		{
			if (preferences.hamlib != 0)
				stop_hamlib ();
			if (hamlibwidgets > 0)
			{
				preferences.rigconf = g_strdup (rigconf);
				hamlibresult = start_hamlib
					(atoi(radio), device, RIG_DEBUG_NONE, atoi (pollstr));
				preferences.rigid = atoi(radio);
				preferences.device = g_strdup (device);
				preferences.polltime = atoi (pollstr);
			}
			else
			{
				preferences.rigid = 1;
#ifdef G_OS_WIN32
				preferences.device = g_strdup ("com2");
#else
				preferences.device = g_strdup ("/dev/ttyS1");
#endif
				preferences.rigconf = g_strdup ("");
				preferences.polltime = 0;
			}
		}
		preferences.hamlib = hamlibwidgets;

		g_free (digits);
		g_free (pollstr);
		g_free (radio);
		g_free (device);
		g_free (rigconf);

		/* clock */
		toggletrue = gtk_toggle_button_get_active 
			(GTK_TOGGLE_BUTTON (clockcheckbutton));
		set_clock (toggletrue);

		g_free (temp);
	}
	gtk_widget_destroy (preferencesdialog);
}
