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
 * gui_logeditoredialog.c - creation and destruction of the logeditor
 *
 * the logeditor is used to enable/disable columns in the log. We can also name
 * the 2 free fields here. When OK is clicked, the log is updated with the new
 * columns and likewise fields in the QSO frame are added or deleted.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "support.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "gui_logeditordialog.h"

extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;
extern GList *logwindowlist;
extern preferencestype preferences;

static gint
compare_fields (void *f1, void *f2, gpointer user_data)
{
	gint *p = f1, *q = f2;
	if (*p - *q == *p)
		return 0;
	else
		return (*p - *q);
}

/* update logfields and return number of columns
 * 
 * column: column number in the log 
 * logfields: array with visible columns
 * showhide: 1 = hide, 0 = show 
 */
static void
update_logfields (const gint column, gint logfields[], const gint showhide)
{
	gint i, j;

	/*
	 * Search for the column in the logfields array
	 */
	for (i = 0; i < QSO_FIELDS; i++)
	{
		if (logfields[i] == column)
			break;
	}
	/*
	 * If the column is not present, add it to the end of the array
	 * if 'Show' is selected
	 */
	if (i == QSO_FIELDS)
	{
		if (showhide == 0)
		{
			for (j = 0; j < QSO_FIELDS; j++)
				if (logfields[j] == 0)
				{
					logfields[j] = column;
					break;
				}
		}
	}

	/*
	 * If the column is present, remove from the array
	 * if 'Hide' is selected
	 */
	else
	{
		if (showhide == 1)
		{
			logfields[i] = 0;
			for (j = 0; j < QSO_FIELDS; j++)
			{
				if (logfields[j] == 0)
				{
					if (logfields[j + 1] == 0)
						break;
					/* Shift '0' to the right */
					logfields[j] = logfields[j + 1];
					logfields[j + 1] = 0;
				}
			}
		}
	}
}

void
on_menu_logeditor_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *logeditor,
		*endhbox, *awardshbox, *powerhbox, *qslhbox, *namehbox, *qthhbox,
		*locatorhbox,  *unknown1hbox, *unknown2hbox, *remarksvbox,
		*unknownlabel1, *unknownlabel2, *hbox, *vboxleft, *vboxright, *hboxtopleft,
		*hboxtopright, *fieldlabel, *showhidelabel, *widthlabel, *hsep, *vsep,
		*lehbox0, *lelabel0_1, *lelabel0_2, *lee0,
		*lehbox1, *lelabel1_1, *lelabel1_2, *lee1,
		*lehbox2, *lelabel2_1, *lelabel2_2, *lee2,
		*lehbox3, *lelabel3_1, *lecombo3, *lee3,
		*lehbox4, *lelabel4_1, *lelabel4_2, *lee4,
		*lehbox5, *lelabel5_1, *lelabel5_2, *lee5,
		*lehbox6, *lelabel6_1, *lelabel6_2, *lee6,
		*lehbox7, *lelabel7_1, *lelabel7_2, *lee7,
		*lehbox8, *lelabel8_1, *lelabel8_2, *lee8,
		*lehbox18, *lelabel18, *lecombo18, *lee18,
		*lehbox9_10, *lelabel9_10, *lecombo9_10, *lee9_10,
		*lehbox11, *lelabel11, *lecombo11, *lee11,
		*lehbox12, *lelabel12, *lecombo12, *lee12,
		*lehbox13, *lelabel13, *lecombo13, *lee13,
		*lehbox14, *lelabel14, *lecombo14, *lee14,
		*lehbox15, *leentry15, *lecombo15, *lee15,
		*lehbox16, *leentry16, *lecombo16, *lee16,
		*lehbox17, *lelabel17_1, *lecombo17, *lee17;
	logtype *logw;
	gchar *temp;
	gint i, j, index, page, response, columns, fields[QSO_FIELDS];
	GtkTreeViewColumn *column;

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));

	if (page >= 0)
	{
		logeditor = gtk_dialog_new_with_buttons (_("xlog - logeditor"),
			GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_container_add
			(GTK_CONTAINER (GTK_DIALOG (logeditor)->vbox), hbox);
		vboxleft = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), vboxleft, FALSE, FALSE, 0);
		vsep = gtk_vseparator_new ();
		gtk_box_pack_start (GTK_BOX (hbox), vsep, FALSE, FALSE, 0);
		vboxright = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), vboxright, FALSE, FALSE, 0);

		hboxtopleft = gtk_hbox_new (TRUE, 0);
		gtk_widget_set_size_request (hboxtopleft, -1, 50);
		gtk_box_pack_start (GTK_BOX (vboxleft), hboxtopleft, FALSE, FALSE, 0);
		fieldlabel = gtk_label_new (_("Fieldname"));
		gtk_box_pack_start (GTK_BOX (hboxtopleft), fieldlabel, FALSE, FALSE, 0);
		showhidelabel = gtk_label_new (_("Show / Hide\n-This log-"));
		gtk_box_pack_start (GTK_BOX (hboxtopleft), showhidelabel, FALSE, FALSE, 0);
		widthlabel = gtk_label_new (_("Width (Pixels)\n-All logs-"));
		gtk_box_pack_start (GTK_BOX (hboxtopleft), widthlabel, FALSE, FALSE, 0);

		hsep = gtk_hseparator_new ();
		gtk_box_pack_start (GTK_BOX (vboxleft), hsep, FALSE, FALSE, 10);

		lehbox0 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox0, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox0, -1, 30);
		lelabel0_1 = gtk_label_new (_("QSO Number"));
		gtk_box_pack_start (GTK_BOX (lehbox0), lelabel0_1, FALSE, FALSE, 0);
		lelabel0_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox0), lelabel0_2, FALSE, FALSE, 0);
		lee0 = gtk_entry_new ();
		gtk_widget_set_size_request (lee0, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox0), lee0, FALSE, FALSE, 0);
		logw = g_list_nth_data (logwindowlist, page);
		column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), NR);
		if (gtk_tree_view_column_get_visible (column))
		{
			temp = g_strdup_printf ("%d", preferences.logcwidths2[NR]);
			gtk_entry_set_text (GTK_ENTRY(lee0), temp);
		}
		else
			gtk_entry_set_text (GTK_ENTRY(lee0), "0");
		gtk_entry_set_activates_default (GTK_ENTRY (lee0), TRUE);
		lehbox1 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox1, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox1, -1, 30);
		lelabel1_1 = gtk_label_new (_("Date"));
		gtk_box_pack_start (GTK_BOX (lehbox1), lelabel1_1, FALSE, FALSE, 0);
		lelabel1_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox1), lelabel1_2, FALSE, FALSE, 0);
		lee1 = gtk_entry_new ();
		gtk_widget_set_size_request (lee1, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox1), lee1, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[DATE]);
		gtk_entry_set_text (GTK_ENTRY(lee1), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee1), TRUE);

		lehbox2 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox2, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox2, -1, 30);
		lelabel2_1 = gtk_label_new ("UTC");
		gtk_box_pack_start (GTK_BOX (lehbox2), lelabel2_1, FALSE, FALSE, 0);
		lelabel2_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox2), lelabel2_2, FALSE, FALSE, 0);
		lee2 = gtk_entry_new ();
		gtk_widget_set_size_request (lee2, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox2), lee2, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[GMT]);
		gtk_entry_set_text (GTK_ENTRY(lee2), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee2), TRUE);

		lehbox3 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox3, FALSE, FALSE, 0);
		lelabel3_1 = gtk_label_new (_("Endtime (UTC)"));
		gtk_box_pack_start (GTK_BOX (lehbox3), lelabel3_1, FALSE, FALSE, 0);
		lecombo3 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox3), lecombo3, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo3), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo3), _("Hide"));
		lee3 = gtk_entry_new ();
		gtk_widget_set_size_request (lee3, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox3), lee3, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[GMTEND]);
		gtk_entry_set_text (GTK_ENTRY(lee3), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee3), TRUE);

		lehbox4 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox4, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox4, -1, 30);
		lelabel4_1 = gtk_label_new (_("Call"));
		gtk_box_pack_start (GTK_BOX (lehbox4), lelabel4_1, FALSE, FALSE, 0);
		lelabel4_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox4), lelabel4_2, FALSE, FALSE, 0);
		lee4 = gtk_entry_new ();
		gtk_widget_set_size_request (lee4, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox4), lee4, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[CALL]);
		gtk_entry_set_text (GTK_ENTRY(lee4), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee4), TRUE);

		lehbox5 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox5, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox5, -1, 30);
		lelabel5_1 = gtk_label_new ("MHz");
		gtk_box_pack_start (GTK_BOX (lehbox5), lelabel5_1, FALSE, FALSE, 0);
		lelabel5_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox5), lelabel5_2, FALSE, FALSE, 0);
		lee5 = gtk_entry_new ();
		gtk_widget_set_size_request (lee5, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox5), lee5, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[BAND]);
		gtk_entry_set_text (GTK_ENTRY(lee5), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee5), TRUE);

		lehbox6 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox6, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox6, -1, 30);
		lelabel6_1 = gtk_label_new (_("Mode"));
		gtk_box_pack_start (GTK_BOX (lehbox6), lelabel6_1, FALSE, FALSE, 0);
		lelabel6_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox6), lelabel6_2, FALSE, FALSE, 0);
		lee6 = gtk_entry_new ();
		gtk_widget_set_size_request (lee6, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox6), lee6, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[MODE]);
		gtk_entry_set_text (GTK_ENTRY(lee6), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee6), TRUE);

		lehbox7 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox7, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox7, -1, 30);
		lelabel7_1 = gtk_label_new ("TX(RST)");
		gtk_box_pack_start (GTK_BOX (lehbox7), lelabel7_1, FALSE, FALSE, 0);
		lelabel7_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox7), lelabel7_2, FALSE, FALSE, 0);
		lee7 = gtk_entry_new ();
		gtk_widget_set_size_request (lee7, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox7), lee7, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[RST]);
		gtk_entry_set_text (GTK_ENTRY(lee7), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee7), TRUE);

		lehbox8 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxleft), lehbox8, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox8, -1, 30);
		lelabel8_1 = gtk_label_new ("RX(RST)");
		gtk_box_pack_start (GTK_BOX (lehbox8), lelabel8_1, FALSE, FALSE, 0);
		lelabel8_2 = gtk_label_new (_("Show"));
		gtk_box_pack_start (GTK_BOX (lehbox8), lelabel8_2, FALSE, FALSE, 0);
		lee8 = gtk_entry_new ();
		gtk_widget_set_size_request (lee8, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox8), lee8, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[MYRST]);
		gtk_entry_set_text (GTK_ENTRY(lee8), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee8), TRUE);

		hboxtopright = gtk_hbox_new (TRUE, 0);
		gtk_widget_set_size_request (hboxtopright, -1, 50);
		gtk_box_pack_start (GTK_BOX (vboxright), hboxtopright, FALSE, FALSE, 0);
		fieldlabel = gtk_label_new (_("Fieldname"));
		gtk_box_pack_start (GTK_BOX (hboxtopright), fieldlabel, FALSE, FALSE, 0);
		showhidelabel = gtk_label_new (_("Show / Hide\n-This log-"));
		gtk_box_pack_start (GTK_BOX (hboxtopright), showhidelabel, FALSE, FALSE, 0);
		widthlabel = gtk_label_new (_("Width (Pixels)\n-All logs-"));
		gtk_box_pack_start (GTK_BOX (hboxtopright), widthlabel, FALSE, FALSE, 0);

		hsep = gtk_hseparator_new ();
		gtk_box_pack_start (GTK_BOX (vboxright), hsep, FALSE, FALSE, 10);

		lehbox18 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox18, FALSE, FALSE, 0);
		gtk_widget_set_size_request (lehbox18, -1, 30);
		lelabel18 = gtk_label_new (_("Awards"));
		gtk_box_pack_start (GTK_BOX (lehbox18), lelabel18, FALSE, FALSE, 0);
		lecombo18 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox18), lecombo18, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo18), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo18), _("Hide"));
		lee18 = gtk_entry_new ();
		gtk_widget_set_size_request (lee18, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox18), lee18, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[AWARDS]);
		gtk_entry_set_text (GTK_ENTRY(lee18), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee18), TRUE);

		lehbox9_10 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox9_10, FALSE, FALSE, 0);
		lelabel9_10 = gtk_label_new (_("QSL out/in"));
		gtk_box_pack_start (GTK_BOX (lehbox9_10), lelabel9_10, FALSE, FALSE, 0);
		lecombo9_10 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox9_10), lecombo9_10, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo9_10), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo9_10), _("Hide"));
		lee9_10 = gtk_entry_new ();
		gtk_widget_set_size_request (lee9_10, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox9_10), lee9_10, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[QSLIN]);
		gtk_entry_set_text (GTK_ENTRY(lee9_10), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee9_10), TRUE);

		lehbox11 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox11, FALSE, FALSE, 0);
		lelabel11 = gtk_label_new (_("Power"));
		gtk_box_pack_start (GTK_BOX (lehbox11), lelabel11, FALSE, FALSE, 0);
		lecombo11 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox11), lecombo11, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo11), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo11), _("Hide"));
		lee11 = gtk_entry_new ();
		gtk_widget_set_size_request (lee11, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox11), lee11, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[POWER]);
		gtk_entry_set_text (GTK_ENTRY(lee11), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee11), TRUE);

		lehbox12 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox12, FALSE, FALSE, 0);
		lelabel12 = gtk_label_new (_("Name"));
		gtk_box_pack_start (GTK_BOX (lehbox12), lelabel12, FALSE, FALSE, 0);
		lecombo12 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox12), lecombo12, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo12), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo12), _("Hide"));
		lee12 = gtk_entry_new ();
		gtk_widget_set_size_request (lee12, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox12), lee12, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[NAME]);
		gtk_entry_set_text (GTK_ENTRY(lee12), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee12), TRUE);

		lehbox13 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox13, FALSE, FALSE, 0);
		lelabel13 = gtk_label_new ("QTH");
		gtk_box_pack_start (GTK_BOX (lehbox13), lelabel13, FALSE, FALSE, 0);
		lecombo13 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox13), lecombo13, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo13), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo13), _("Hide"));
		lee13 = gtk_entry_new ();
		gtk_widget_set_size_request (lee13, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox13), lee13, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[QTH]);
		gtk_entry_set_text (GTK_ENTRY(lee13), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee13), TRUE);

		lehbox14 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox14, FALSE, FALSE, 0);
		lelabel14 = gtk_label_new (_("QTH Locator"));
		gtk_box_pack_start (GTK_BOX (lehbox14), lelabel14, FALSE, FALSE, 0);
		lecombo14 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox14), lecombo14, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo14), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo14), _("Hide"));
		lee14 = gtk_entry_new ();
		gtk_widget_set_size_request (lee14, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox14), lee14, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[LOCATOR]);
		gtk_entry_set_text (GTK_ENTRY(lee14), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee14), TRUE);

		lehbox15 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox15, FALSE, FALSE, 0);
		leentry15 = gtk_entry_new ();
		gtk_widget_set_size_request (leentry15, 100, -1);
		gtk_box_pack_start (GTK_BOX (lehbox15), leentry15, FALSE, FALSE, 0);
		gtk_entry_set_activates_default (GTK_ENTRY (leentry15), TRUE);
		lecombo15 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox15), lecombo15, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo15), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo15), _("Hide"));
		lee15 = gtk_entry_new ();
		gtk_widget_set_size_request (lee15, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox15), lee15, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[U1]);
		gtk_entry_set_text (GTK_ENTRY(lee15), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee15), TRUE);

		lehbox16 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox16, FALSE, FALSE, 0);
		leentry16 = gtk_entry_new ();
		gtk_widget_set_size_request (leentry16, 100, -1);
		gtk_box_pack_start (GTK_BOX (lehbox16), leentry16, FALSE, FALSE, 0);
		gtk_entry_set_activates_default (GTK_ENTRY (leentry16), TRUE);
		lecombo16 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox16), lecombo16, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo16), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo16), _("Hide"));
		lee16 = gtk_entry_new ();
		gtk_widget_set_size_request (lee16, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox16), lee16, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[U2]);
		gtk_entry_set_text (GTK_ENTRY(lee16), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee16), TRUE);

		lehbox17 = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vboxright), lehbox17, FALSE, FALSE, 0);
		lelabel17_1 = gtk_label_new (_("Remarks"));
		gtk_box_pack_start (GTK_BOX (lehbox17), lelabel17_1, FALSE, FALSE, 0);
		lecombo17 = gtk_combo_box_new_text ();
		gtk_box_pack_start (GTK_BOX (lehbox17), lecombo17, FALSE, FALSE, 0);
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo17), _("Show"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (lecombo17), _("Hide"));
		lee17 = gtk_entry_new ();
		gtk_widget_set_size_request (lee17, 50, -1);
		gtk_box_pack_start (GTK_BOX (lehbox17), lee17, FALSE, FALSE, 0);
		temp = g_strdup_printf ("%d", preferences.logcwidths2[REMARKS]);
		gtk_entry_set_text (GTK_ENTRY(lee17), temp);
		gtk_entry_set_activates_default (GTK_ENTRY (lee17), TRUE);

		endhbox = lookup_widget (mainwindow, "endhbox");
		if (gtk_widget_get_visible (endhbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo3), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo3), 1);

		awardshbox = lookup_widget (mainwindow, "awardshbox");
		if (gtk_widget_get_visible (awardshbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo18), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo18), 1);

		qslhbox = lookup_widget (mainwindow, "qslhbox");
		if (gtk_widget_get_visible (qslhbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo9_10), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo9_10), 1);

		powerhbox = lookup_widget (mainwindow, "powerhbox");
		if (gtk_widget_get_visible (powerhbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo11), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo11), 1);

		namehbox = lookup_widget (mainwindow, "namehbox");
		if (gtk_widget_get_visible (namehbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo12), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo12), 1);

		qthhbox = lookup_widget (mainwindow, "qthhbox");
		if (gtk_widget_get_visible (qthhbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo13), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo13), 1);

		locatorhbox = lookup_widget (mainwindow, "locatorhbox");
		if (gtk_widget_get_visible (locatorhbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo14), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo14), 1);

		unknown1hbox = lookup_widget (mainwindow, "unknown1hbox");
		if (gtk_widget_get_visible (unknown1hbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo15), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo15), 1);

		unknown2hbox = lookup_widget (mainwindow, "unknown2hbox");
		if (gtk_widget_get_visible (unknown2hbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo16), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo16), 1);

		gtk_entry_set_text (GTK_ENTRY (leentry15), preferences.freefield1);
		gtk_entry_set_text (GTK_ENTRY (leentry16), preferences.freefield2);

		remarksvbox = lookup_widget (mainwindow, "remarksvbox");
		if (gtk_widget_get_visible (remarksvbox))
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo17), 0);
		else
			gtk_combo_box_set_active (GTK_COMBO_BOX (lecombo17), 1);
		gtk_dialog_set_default_response (GTK_DIALOG (logeditor),
						 GTK_RESPONSE_OK);

		gtk_widget_show_all (logeditor);
		response = gtk_dialog_run (GTK_DIALOG(logeditor));
		if (response == GTK_RESPONSE_OK)
		{

			/* fields is the array with visible columns */
			for (i = 0; i < logw->columns; i++)
				fields[i] = logw->logfields[i];
			for (i = logw->columns; i < QSO_FIELDS; i++)
				fields[i] = 0;

			/* show/hide columns and update the array */
			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo3));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), GMTEND);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (GMTEND, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo18));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), AWARDS);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (AWARDS, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo9_10));
			if (index == 1)
			{
				column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), QSLOUT);
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
				column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), QSLIN);
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			}
			else
			{
				column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), QSLOUT);
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
				column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), QSLIN);
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			}
			update_logfields (QSLOUT, fields, index);
			update_logfields (QSLIN, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo11));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), POWER);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (POWER, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo12));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), NAME);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (NAME, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo13));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), QTH);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (QTH, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo14));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), LOCATOR);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (LOCATOR, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo15));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), U1);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (U1, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo16));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), U2);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (U2, fields, index);

			index = gtk_combo_box_get_active (GTK_COMBO_BOX (lecombo17));
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), REMARKS);
			if (index == 1)
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
			else
				gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
			update_logfields (REMARKS, fields, index);

			/* how many columns do we have now */
			for (i = 0; i < QSO_FIELDS; i++)
			{
				if (fields[i] == 0)
				break;
			}
			columns = i;

			/* is the log changed? */
			for (i = 0; i < QSO_FIELDS; i++)
			{
				if (fields[i] != logw->logfields[i])
				{
					logw->logchanged = TRUE;
					temp = g_strdup_printf ("<b>%s*</b>", logw->logname);
					gtk_label_set_markup (GTK_LABEL (logw->label), temp);
					g_free (temp);
					break;
				}
			}

			/* sort the fields array */
			g_qsort_with_data (fields, QSO_FIELDS, sizeof (gint),
				(GCompareDataFunc) compare_fields, NULL);

			/*
			 * copy the array, ignore prepending zero's 
			 * which have been added by qsort 
			 */
			for (i = 0; i < QSO_FIELDS; i++)
				if (fields[i] != 0) break;
			for (j = 0; j < columns; j++)
				logw->logfields[j] = fields[j + i];

			/* fill up with zero's */
			for (j = columns; j < QSO_FIELDS; j++)
				logw->logfields[j] = 0;

			logw->columns = columns;
			set_qsoframe (logw);
			/* changing the name of the free fields applies to all logs */
			temp = gtk_editable_get_chars (GTK_EDITABLE (leentry15), 0, -1);
			if (strcasecmp (temp, preferences.freefield1) != 0)
			{
				if (strlen (temp) == 0)
					temp = g_strdup ("UNKNOWN");
				preferences.freefield1 = g_strdup (temp);
				unknownlabel1 = lookup_widget (mainwindow, "unknownlabel1");
				gtk_label_set_text (GTK_LABEL (unknownlabel1), temp);
				for (i = 0; i < g_list_length(logwindowlist); i++)
				{
					logw = g_list_nth_data (logwindowlist, i);
					column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), U1);
					gtk_tree_view_column_set_title (column,	preferences.freefield1);
				}
			}
			temp = gtk_editable_get_chars (GTK_EDITABLE (leentry16), 0, -1);
			if (strcasecmp (temp, preferences.freefield2) != 0)
			{
				if (strlen (temp) == 0)
					temp = g_strdup ("UNKNOWN");
				preferences.freefield2 = g_strdup (temp);
				unknownlabel2 = lookup_widget (mainwindow, "unknownlabel2");
				gtk_label_set_text (GTK_LABEL (unknownlabel2), temp);
				for (i = 0; i < g_list_length(logwindowlist); i++)
				{
					logw = g_list_nth_data (logwindowlist, i);
					column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), U2);
					gtk_tree_view_column_set_title (column,	preferences.freefield2);
				}
			}
			g_free (temp);

			/* now set column widths for all visible logs if changed */
			preferences.logcwidths2[NR] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee0), 0, -1));
			preferences.logcwidths2[DATE] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee1), 0, -1));
			preferences.logcwidths2[GMT] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee2), 0, -1));
			preferences.logcwidths2[GMTEND] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee3), 0, -1));
			preferences.logcwidths2[CALL] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee4), 0, -1));
			preferences.logcwidths2[BAND] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee5), 0, -1));
			preferences.logcwidths2[MODE] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee6), 0, -1));
			preferences.logcwidths2[RST] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee7), 0, -1));
			preferences.logcwidths2[MYRST] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee8), 0, -1));
			preferences.logcwidths2[AWARDS] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee18), 0, -1));
			preferences.logcwidths2[QSLIN] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee9_10), 0, -1));
			preferences.logcwidths2[QSLOUT] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee9_10), 0, -1));
			preferences.logcwidths2[POWER] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee11), 0, -1));
			preferences.logcwidths2[NAME] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee12), 0, -1));
			preferences.logcwidths2[QTH] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee13), 0, -1));
			preferences.logcwidths2[LOCATOR] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee14), 0, -1));
			preferences.logcwidths2[U1] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee15), 0, -1));
			preferences.logcwidths2[U2] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee16), 0, -1));
			preferences.logcwidths2[REMARKS] = atoi(gtk_editable_get_chars (GTK_EDITABLE (lee17), 0, -1));

			for (i = 0; i < g_list_length (logwindowlist); i++)
			{
				logw = g_list_nth_data (logwindowlist, i);
				for (j = 0; j < QSO_FIELDS; j++)
				{
					column = gtk_tree_view_get_column (GTK_TREE_VIEW(logw->treeview), j);
					if (j != NR)
					{
						if (preferences.logcwidths2[j] > 19)
							gtk_tree_view_column_set_fixed_width
								(GTK_TREE_VIEW_COLUMN(column), preferences.logcwidths2[j]);
						else
						{
							gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN(column), 20);
							preferences.logcwidths2[j] = 20;
						}
					}
					else
					{
						if (preferences.logcwidths2[j] == 0)
							gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
						else
						{
							gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), TRUE);
							if (preferences.logcwidths2[j] > 19)
								gtk_tree_view_column_set_fixed_width
									(GTK_TREE_VIEW_COLUMN(column), preferences.logcwidths2[j]);
							else
							{
								gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN(column), 20);
								preferences.logcwidths2[j] = 20;
							}
						}
					}
				}
			}
		}
		gtk_widget_destroy (logeditor);
	}
}
