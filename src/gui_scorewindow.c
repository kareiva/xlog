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

/* gui_scorewindow.c - creation of the scorelist
 *
 */

#include <gtk/gtk.h>
#include "support.h"
#include "cfg.h"
#include "gui_scorewindow.h"
#include "xlog_enum.h"
#include "dxcc.h"
#include "awards_enum.h"

extern GtkWidget *mainwindow;
extern GtkUIManager *ui_manager;
extern preferencestype preferences;

GtkWidget *scorewindow;

static gboolean
on_scorewindow_delete_event (GtkWidget * widget, GdkEvent * event,	gpointer user_data)
{
	GtkWidget *score;

	score = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/OptionMenu/Scoring");
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(score), FALSE);
	gtk_window_get_size (GTK_WINDOW(widget), &preferences.scorewidth, &preferences.scoreheight);
	gtk_window_get_position (GTK_WINDOW(widget), &preferences.scorex, &preferences.scorey);
	gtk_widget_hide (widget);
	return TRUE;		/* do not destroy */
}

static void
notebook_remove_awardpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook, const gchar *windowname)
{
	GtkWidget *scrolledwindow = lookup_widget (scorewindow, windowname);
	gtk_notebook_remove_page (GTK_NOTEBOOK(awardsnotebook),
		gtk_notebook_page_num (GTK_NOTEBOOK(awardsnotebook), scrolledwindow));
}

static void
notebook_append_awardpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook, const gchar *treeviewname, const gchar *tabname, const gchar *windowname)
{
	GtkWidget *tablabel, *awardtreeview, *scrolledwindow;
	GtkListStore *awardmodel;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeIter awarditer;

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	/* one extra field for 'all' (bands) in last column */
	awardmodel = gtk_list_store_new (MAX_BANDS + 1,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	awardtreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(awardmodel));
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (awardtreeview), -1);
	g_object_unref (G_OBJECT (awardmodel));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), awardtreeview);

	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new_with_attributes
		("0.136", renderer, "text", BAND_2190, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2190]);

	column = gtk_tree_view_column_new_with_attributes
		("0.501", renderer, "text", BAND_560, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_560]);

	column = gtk_tree_view_column_new_with_attributes
		("1.8", renderer, "text", BAND_160, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_160]);

	column = gtk_tree_view_column_new_with_attributes
		("3.5", renderer, "text", BAND_80, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_80]);

	column = gtk_tree_view_column_new_with_attributes
		("5.2", renderer, "text", BAND_60, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_60]);

	column = gtk_tree_view_column_new_with_attributes
		("7", renderer, "text", BAND_40, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_40]);

	column = gtk_tree_view_column_new_with_attributes
		("10", renderer, "text", BAND_30, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_30]);

	column = gtk_tree_view_column_new_with_attributes
		("14", renderer, "text", BAND_20, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_20]);

	column = gtk_tree_view_column_new_with_attributes
		("18", renderer, "text", BAND_17, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_17]);

	column = gtk_tree_view_column_new_with_attributes
		("21", renderer, "text", BAND_15, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_15]);

	column = gtk_tree_view_column_new_with_attributes
		("24", renderer, "text", BAND_12, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_12]);

	column = gtk_tree_view_column_new_with_attributes
		("28", renderer, "text", BAND_10, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_10]);

	column = gtk_tree_view_column_new_with_attributes
		("50", renderer, "text", BAND_6, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_6]);

	column = gtk_tree_view_column_new_with_attributes
		("70", renderer, "text", BAND_4, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_4]);

	column = gtk_tree_view_column_new_with_attributes
		("144", renderer, "text", BAND_2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2]);

	column = gtk_tree_view_column_new_with_attributes
		("222", renderer, "text", BAND_125CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_125CM]);

	column = gtk_tree_view_column_new_with_attributes
		("420", renderer, "text", BAND_70CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_70CM]);

	column = gtk_tree_view_column_new_with_attributes
		("902", renderer, "text", BAND_33CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_33CM]);

	column = gtk_tree_view_column_new_with_attributes
		("1240", renderer, "text", BAND_23CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_23CM]);

	column = gtk_tree_view_column_new_with_attributes
		("2300", renderer, "text", BAND_13CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_13CM]);

	column = gtk_tree_view_column_new_with_attributes
		("3300", renderer, "text", BAND_9CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_9CM]);

	column = gtk_tree_view_column_new_with_attributes
		("5650", renderer, "text", BAND_6CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_6CM]);

	column = gtk_tree_view_column_new_with_attributes
		("10000", renderer, "text", BAND_3CM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_3CM]);

	column = gtk_tree_view_column_new_with_attributes
		("24000", renderer, "text", BAND_12HMM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_12HMM]);

	column = gtk_tree_view_column_new_with_attributes
		("47000", renderer, "text", BAND_6MM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_6MM]);

	column = gtk_tree_view_column_new_with_attributes
		("75500", renderer, "text", BAND_4MM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_4MM]);

	column = gtk_tree_view_column_new_with_attributes
		("120000", renderer, "text", BAND_2HMM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2HMM]);

	column = gtk_tree_view_column_new_with_attributes
		("142000", renderer, "text", BAND_2MM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2MM]);

	column = gtk_tree_view_column_new_with_attributes
		("241000", renderer, "text", BAND_1MM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_1MM]);

	column = gtk_tree_view_column_new_with_attributes
		("300000", renderer, "text", BAND_SUBMM, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_SUBMM]);

	column = gtk_tree_view_column_new_with_attributes
		("All", renderer, "text", MAX_BANDS, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (awardtreeview), column);

	gtk_list_store_append (GTK_LIST_STORE (awardmodel), &awarditer);
	gtk_tree_selection_set_mode
		(gtk_tree_view_get_selection (GTK_TREE_VIEW (awardtreeview)),
		GTK_SELECTION_NONE );
	tablabel = gtk_label_new (tabname);
	gtk_notebook_append_page
		(GTK_NOTEBOOK(awardsnotebook), scrolledwindow, tablabel);

	GLADE_HOOKUP_OBJECT (scorewindow, awardtreeview, treeviewname);
	GLADE_HOOKUP_OBJECT (scorewindow, scrolledwindow, windowname);
	gtk_widget_show_all (scrolledwindow);
}


void
notebook_append_dxccpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_append_awardpage (scorewindow, awardsnotebook, "dxcctreeview", "DXCC", "scrolledwindow1");
}

void
notebook_remove_dxccpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_remove_awardpage (scorewindow, awardsnotebook, "scrolledwindow1");
}

void
notebook_append_wacpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_append_awardpage (scorewindow, awardsnotebook, "wactreeview", "WAC", "scrolledwindow2");
}

void
notebook_remove_wacpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_remove_awardpage (scorewindow, awardsnotebook, "scrolledwindow2");
}

void
notebook_append_waspage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_append_awardpage (scorewindow, awardsnotebook, "wastreeview", "WAS", "scrolledwindow3");
}

void
notebook_remove_waspage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_remove_awardpage (scorewindow, awardsnotebook, "scrolledwindow3");
}

void
notebook_append_wazpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_append_awardpage (scorewindow, awardsnotebook, "waztreeview", "WAZ", "scrolledwindow4");
}

void
notebook_remove_wazpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_remove_awardpage (scorewindow, awardsnotebook, "scrolledwindow4");
}

void
notebook_append_iotapage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_append_awardpage (scorewindow, awardsnotebook, "iotatreeview", "IOTA", "scrolledwindow5");
}

void
notebook_remove_iotapage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_remove_awardpage (scorewindow, awardsnotebook, "scrolledwindow5");
}

void
notebook_append_locpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_append_awardpage (scorewindow, awardsnotebook, "loctreeview", "GRID", "scrolledwindow6");
}

void
notebook_remove_locpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook)
{
	notebook_remove_awardpage (scorewindow, awardsnotebook, "scrolledwindow6");
}

GtkWidget *
create_scorewindow (void)
{
	GdkPixbuf *scorewindow_icon_pixbuf;
	GtkWidget *hbox, *vboxleft, *locatorframe, *locatorvbox,
		*locatorframelabel, *locatorlabel, *dxccframe, *dxccvbox, *dxcchbox,
		*dxcclabel1, *dxcclabel3, *dxcclabel4, *dxcclabel5,
		*dxccframelabel, *contlabel, *itulabel, *cqlabel,
		*vboxright, *rightframe, *framevbox, *scrolledwindow, *countrytreeview,
		*rightsep, *awardsnotebook;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	gchar *temp;
	gint awardscount, i;

	scorewindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus (GTK_WINDOW(scorewindow), FALSE);
	gtk_window_set_title (GTK_WINDOW (scorewindow), _("xlog - scoring"));
	scorewindow_icon_pixbuf = create_pixbuf ("xlog-scoring.png");
	if (scorewindow_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (scorewindow), scorewindow_icon_pixbuf);
		g_object_unref (scorewindow_icon_pixbuf);
	}
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (scorewindow), hbox);
	vboxleft = gtk_vbox_new (FALSE, 0);
	gtk_widget_set_size_request (vboxleft, 250, -1);
	gtk_box_pack_start (GTK_BOX (hbox), vboxleft, FALSE, FALSE, 0);

	/* locator information frame */
	locatorframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vboxleft), locatorframe, TRUE, TRUE, 0);
	locatorvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (locatorframe), locatorvbox);
	locatorlabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (locatorvbox), locatorlabel, FALSE, FALSE, 0);
	temp = g_strdup_printf ("<b> %s </b>", _("Locator"));
	locatorframelabel = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (locatorframelabel), temp);
	gtk_frame_set_label_widget (GTK_FRAME (locatorframe), locatorframelabel);
	gtk_widget_set_size_request (locatorframelabel, 120, 10);

	/* dxcc information frame */
	dxccframe = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vboxleft), dxccframe, TRUE, TRUE, 0);
	dxccvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (dxccframe), dxccvbox);
	dxcclabel1 = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxccvbox), dxcclabel1, FALSE, FALSE, 0);

	dxcchbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (dxccvbox), dxcchbox, FALSE, FALSE, 0);
	contlabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxcchbox), contlabel, FALSE, FALSE, 0);
	itulabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxcchbox), itulabel, FALSE, FALSE, 0);
	cqlabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxcchbox), cqlabel, FALSE, FALSE, 0);

	dxcclabel3 = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxccvbox), dxcclabel3, FALSE, FALSE, 0);
	dxcclabel4 = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxccvbox), dxcclabel4, FALSE, FALSE, 0);
	dxcclabel5 = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (dxccvbox), dxcclabel5, FALSE, FALSE, 0);
	
	dxccframelabel = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (dxccframelabel), "<b>VERYBIGCALLSIGN</b>");
	gtk_frame_set_label_widget (GTK_FRAME (dxccframe), dxccframelabel);
	gtk_label_set_line_wrap (GTK_LABEL (dxcclabel1), TRUE);
	gtk_widget_set_size_request (dxcclabel1, -1, 60);
	gtk_label_set_justify (GTK_LABEL (dxcclabel1), GTK_JUSTIFY_CENTER);

	/* top */
	vboxright = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vboxright, TRUE, TRUE, 0);
	rightframe = gtk_frame_new ("");
	gtk_box_pack_start (GTK_BOX (vboxright), rightframe, TRUE, TRUE, 0);
	framevbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (rightframe), framevbox);
	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (framevbox), scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	/* one extra field for official prefix in first column */
	model = gtk_list_store_new (MAX_BANDS + 1,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	countrytreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (countrytreeview), -1);
	g_object_unref (G_OBJECT (model));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), countrytreeview);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		("", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 80);

	column = gtk_tree_view_column_new_with_attributes
		("0.136", renderer, "text", BAND_2190 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2190]);

	column = gtk_tree_view_column_new_with_attributes
		("0.501", renderer, "text", BAND_560 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_560]);

	column = gtk_tree_view_column_new_with_attributes
		("1.8", renderer, "text", BAND_160 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_160]);

	column = gtk_tree_view_column_new_with_attributes
		("3.5", renderer, "text", BAND_80 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_80]);

	column = gtk_tree_view_column_new_with_attributes
		("5.2", renderer, "text", BAND_60 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_60]);

	column = gtk_tree_view_column_new_with_attributes
		("7", renderer, "text", BAND_40 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_40]);

	column = gtk_tree_view_column_new_with_attributes
		("10", renderer, "text", BAND_30 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_30]);

	column = gtk_tree_view_column_new_with_attributes
		("14", renderer, "text", BAND_20 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_20]);

	column = gtk_tree_view_column_new_with_attributes
		("18", renderer, "text", BAND_17 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_17]);

	column = gtk_tree_view_column_new_with_attributes
		("21", renderer, "text", BAND_15 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_15]);

	column = gtk_tree_view_column_new_with_attributes
		("24", renderer, "text", BAND_12 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_12]);

	column = gtk_tree_view_column_new_with_attributes
		("28", renderer, "text", BAND_10 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_10]);

	column = gtk_tree_view_column_new_with_attributes
		("50", renderer, "text", BAND_6 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_6]);

	column = gtk_tree_view_column_new_with_attributes
		("70", renderer, "text", BAND_4 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_4]);

	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_column_new_with_attributes
		("144", renderer, "text", BAND_2 + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2]);

	column = gtk_tree_view_column_new_with_attributes
		("222", renderer, "text", BAND_125CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_125CM]);

	column = gtk_tree_view_column_new_with_attributes
		("420", renderer, "text", BAND_70CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_70CM]);

	column = gtk_tree_view_column_new_with_attributes
		("902", renderer, "text", BAND_33CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_33CM]);

	column = gtk_tree_view_column_new_with_attributes
		("1240", renderer, "text", BAND_23CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_23CM]);

	column = gtk_tree_view_column_new_with_attributes
		("2300", renderer, "text", BAND_13CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_13CM]);

	column = gtk_tree_view_column_new_with_attributes
		("3300", renderer, "text", BAND_9CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_9CM]);

	column = gtk_tree_view_column_new_with_attributes
		("5650", renderer, "text", BAND_6CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_6CM]);

	column = gtk_tree_view_column_new_with_attributes
		("10000", renderer, "text", BAND_3CM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_3CM]);

	column = gtk_tree_view_column_new_with_attributes
		("24000", renderer, "text", BAND_12HMM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_12HMM]);

	column = gtk_tree_view_column_new_with_attributes
		("47000", renderer, "text", BAND_6MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_6MM]);

	column = gtk_tree_view_column_new_with_attributes
		("75500", renderer, "text", BAND_4MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_4MM]);

	column = gtk_tree_view_column_new_with_attributes
		("120000", renderer, "text", BAND_2HMM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2HMM]);

	column = gtk_tree_view_column_new_with_attributes
		("142000", renderer, "text", BAND_2MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_2MM]);

	column = gtk_tree_view_column_new_with_attributes
		("241000", renderer, "text", BAND_1MM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_1MM]);

	column = gtk_tree_view_column_new_with_attributes
		("300000", renderer, "text", BAND_SUBMM + 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (countrytreeview), column);
	gtk_tree_view_column_set_visible
		(column, preferences.scoringbands[BAND_SUBMM]);

	awardscount = NB_AWARDS;

	for (i=0; i<awardscount; i++)
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);

	gtk_tree_selection_set_mode
		(gtk_tree_view_get_selection (GTK_TREE_VIEW (countrytreeview)),
		GTK_SELECTION_NONE );
	rightsep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (framevbox), rightsep, FALSE, FALSE, 0);

	/* bottom */
	awardsnotebook = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX (framevbox), awardsnotebook, TRUE, TRUE, 0);

	notebook_append_dxccpage (scorewindow, awardsnotebook);

	if (preferences.awardswac == 1)
		notebook_append_wacpage (scorewindow, awardsnotebook);

	if (preferences.awardswas == 1)
		notebook_append_waspage (scorewindow, awardsnotebook);

	if (preferences.awardswaz == 1)
		notebook_append_wazpage (scorewindow, awardsnotebook);

	if (preferences.awardsiota == 1)
		notebook_append_iotapage (scorewindow, awardsnotebook);

	if (preferences.awardsloc == 1)
		notebook_append_locpage (scorewindow, awardsnotebook);

	GLADE_HOOKUP_OBJECT (scorewindow, dxccframe, "dxccframe");
	GLADE_HOOKUP_OBJECT (scorewindow, dxcclabel1, "dxcclabel1");
	GLADE_HOOKUP_OBJECT (scorewindow, contlabel, "contlabel");
	GLADE_HOOKUP_OBJECT (scorewindow, itulabel, "itulabel");
	GLADE_HOOKUP_OBJECT (scorewindow, cqlabel, "cqlabel");
	GLADE_HOOKUP_OBJECT (scorewindow, dxcclabel3, "dxcclabel3");
	GLADE_HOOKUP_OBJECT (scorewindow, dxcclabel4, "dxcclabel4");
	GLADE_HOOKUP_OBJECT (scorewindow, dxcclabel5, "dxcclabel5");
	GLADE_HOOKUP_OBJECT (scorewindow, locatorframe, "locatorframe");
	GLADE_HOOKUP_OBJECT (scorewindow, locatorlabel, "locatorlabel");
	GLADE_HOOKUP_OBJECT (scorewindow, awardsnotebook, "awardsnotebook");
	GLADE_HOOKUP_OBJECT (scorewindow, countrytreeview, "countrytreeview");

	g_signal_connect ((gpointer) scorewindow, "delete_event",
		G_CALLBACK (on_scorewindow_delete_event), NULL);

	gtk_widget_show_all (hbox);
	return scorewindow;
}

void
on_scoring_activate (GtkAction *action, gpointer user_data)
{
	gboolean status;

	status = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	if (status)
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
		gtk_widget_show (scorewindow);
		gdk_window_move_resize (scorewindow->window,
			preferences.scorex, preferences.scorey, preferences.scorewidth, preferences.scoreheight);
	}
	else
	{
		gtk_window_get_size (GTK_WINDOW(scorewindow), &preferences.scorewidth, &preferences.scoreheight);
		gtk_window_get_position (GTK_WINDOW(scorewindow), &preferences.scorex, &preferences.scorey);
		gtk_widget_hide (scorewindow);
	}

}
