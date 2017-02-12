/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 Andy Stewart <kb1oiq@arrl.net>
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

/* gui_awards_waz.c */

#include <gtk/gtk.h>
#include "gui_awards_waz.h"
#include "gui_warningdialog.h"
#include "support.h"
#include "xlog_enum.h"
#include "awards_enum.h"
#include "cfg.h"

extern GtkWidget *mainwindow, *scorewindow;
extern preferencestype preferences;
extern gushort waz_w[MAX_ZONES][MAX_BANDS + 1];
extern gushort waz_c[MAX_ZONES][MAX_BANDS + 1];

void on_awards_waz_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (!gtk_widget_get_visible(scorewindow))
		warningdialog (_("xlog - WAZ award"),
			_("Please enable the scoring window"), "gtk-dialog-warning");
	if (preferences.awardswaz == 0)
	{
		warningdialog (_("xlog - WAZ award"),
                        _("You will need to enable WAZ scoring from the \"windows and dialogs\" dialog"), "gtk-dialog-warning");
		return;
	}

	GtkWidget *dialog = gtk_dialog_new_with_buttons (_("xlog - WAZ award"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	gtk_dialog_add_button (GTK_DIALOG (dialog),	GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_widget_set_size_request (dialog, 450, 500);

	GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	/* 2 extra columns, zone and 'all bands */
	GtkListStore *model = gtk_list_store_new (MAX_BANDS + 1,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	g_object_unref (G_OBJECT (model));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), treeview);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes
		("zone", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	column = gtk_tree_view_column_new_with_attributes
		("0.137", renderer, "text", BAND_2190 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_2190]);
	column = gtk_tree_view_column_new_with_attributes
		("0.501", renderer, "text", BAND_560 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_560]);
	column = gtk_tree_view_column_new_with_attributes
		("1.8", renderer, "text", BAND_160 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_160]);
	column = gtk_tree_view_column_new_with_attributes
		("3.5", renderer, "text", BAND_80 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_80]);
	column = gtk_tree_view_column_new_with_attributes
		("5.3", renderer, "text", BAND_60 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_60]);
	column = gtk_tree_view_column_new_with_attributes
		("7", renderer, "text", BAND_40 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_40]);
	column = gtk_tree_view_column_new_with_attributes
		("10", renderer, "text", BAND_30 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_30]);
	column = gtk_tree_view_column_new_with_attributes
		("14", renderer, "text", BAND_20 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_20]);
	column = gtk_tree_view_column_new_with_attributes
		("18", renderer, "text", BAND_17 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_17]);
	column = gtk_tree_view_column_new_with_attributes
		("21", renderer, "text", BAND_15 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_15]);
	column = gtk_tree_view_column_new_with_attributes
		("24", renderer, "text", BAND_12 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_12]);
	column = gtk_tree_view_column_new_with_attributes
		("28", renderer, "text", BAND_10 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_10]);
	column = gtk_tree_view_column_new_with_attributes
		("50", renderer, "text", BAND_6 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_6]);
	column = gtk_tree_view_column_new_with_attributes
		("70", renderer, "text", BAND_4 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_4]);
	column = gtk_tree_view_column_new_with_attributes
		("144", renderer, "text", BAND_2 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_2]);
	column = gtk_tree_view_column_new_with_attributes
		("222", renderer, "text", BAND_125CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_125CM]);
	column = gtk_tree_view_column_new_with_attributes
		("420", renderer, "text", BAND_70CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_70CM]);
	column = gtk_tree_view_column_new_with_attributes
		("902", renderer, "text", BAND_33CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_33CM]);
	column = gtk_tree_view_column_new_with_attributes
		("1240", renderer, "text", BAND_23CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_23CM]);
	column = gtk_tree_view_column_new_with_attributes
		("2300", renderer, "text", BAND_13CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_13CM]);
	column = gtk_tree_view_column_new_with_attributes
		("3300", renderer, "text", BAND_9CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_9CM]);
	column = gtk_tree_view_column_new_with_attributes
		("5650", renderer, "text", BAND_6CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_6CM]);
	column = gtk_tree_view_column_new_with_attributes
		("10000", renderer, "text", BAND_3CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_3CM]);
	column = gtk_tree_view_column_new_with_attributes
		("24000", renderer, "text", BAND_12HMM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_12HMM]);
	column = gtk_tree_view_column_new_with_attributes
		("47000", renderer, "text", BAND_6MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_6MM]);
	column = gtk_tree_view_column_new_with_attributes
		("75500", renderer, "text", BAND_4MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_4MM]);
	column = gtk_tree_view_column_new_with_attributes
		("120000", renderer, "text", BAND_2HMM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_2HMM]);
	column = gtk_tree_view_column_new_with_attributes
		("142000", renderer, "text", BAND_2MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_2MM]);
	column = gtk_tree_view_column_new_with_attributes
		("241000", renderer, "text", BAND_1MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_1MM]);
	column = gtk_tree_view_column_new_with_attributes
		("300000", renderer, "text", BAND_SUBMM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
	gtk_tree_view_column_set_visible
                (column, preferences.scoringbands[BAND_SUBMM]);

	GtkTreeIter iter;
	gint i, j;
	for (i = 0; i < MAX_ZONES; i++)
	{
		gchar strzone[8];

		sprintf(strzone, "%d", i+1);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, strzone, -1);
		for (j = 0; j < MAX_BANDS; j++)
		{
			if (waz_c[i][j] > 0)
			{
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, j+1, "0", -1);
			}
			else if (waz_w[i][j] > 0)
			{
				gchar *str = g_strdup_printf ("%d", waz_w[i][j]);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, j+1, str, -1);
				g_free (str);
			}
		}
	}
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}
