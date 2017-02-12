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

/* gui_dxcclistdialog.c - creation of the dxcclist
 *
 */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "gui_dxcclist.h"
#include "support.h"
#include "dxcc.h"
#include "main.h"
#include "awards_enum.h"

extern GPtrArray *dxcc;
extern programstatetype programstate;

/* sort function for the latitude, longitude and timezone columns */
static gint
sort_column (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	gchar *item_a = NULL, *item_b = NULL;
	double da, db;

	gtk_tree_model_get (model, a, GPOINTER_TO_INT(user_data), &item_a, -1);
	gtk_tree_model_get (model, b, GPOINTER_TO_INT(user_data), &item_b, -1);

	da = strtod (item_a, NULL);
	db = strtod (item_b, NULL);

	return da-db;
}

void
on_menu_dxcclist_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dxccwindow, *dxccscrolledwindow, *dxcctreeview, *dxccwindowvbox;
	GdkPixbuf *dxccwindow_icon_pixbuf;
	GtkTreeViewColumn *column;
	GtkListStore *model;
	GtkCellRenderer *renderer, *brenderer;
	GtkTreeIter iter;
	gint i;
	gchar *lat, *lon, *tz;

	dxccwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (dxccwindow), _("xlog - DXCC list"));
	dxccwindow_icon_pixbuf = create_pixbuf ("xlog.png");
	if (dxccwindow_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (dxccwindow), dxccwindow_icon_pixbuf);
		g_object_unref (dxccwindow_icon_pixbuf);
	}
	dxccwindowvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (dxccwindow), dxccwindowvbox);
	dxccscrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(dxccscrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (dxccwindowvbox), dxccscrolledwindow, TRUE, TRUE, 0);
	gtk_box_reorder_child (GTK_BOX (dxccwindowvbox), dxccscrolledwindow, 0);
	model = gtk_list_store_new (10,
		G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, 
		G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	dxcctreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	g_object_unref (G_OBJECT (model));
	gtk_container_add (GTK_CONTAINER (dxccscrolledwindow), dxcctreeview);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Nr"), renderer, "text", 0, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);

	brenderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (brenderer), "weight", "bold", NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Prefix"), brenderer, "text", 1, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Country"), renderer, "text", 2, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Continent"), renderer, "text", 3, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 3);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("CQ", renderer, "text", 4, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 4);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("ITU", renderer, "text", 5, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 5);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Latitude"), renderer, "text", 6, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (model), 6, sort_column, GINT_TO_POINTER (6), NULL);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Longitude"), renderer, "text", 7, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 7);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (model), 7, sort_column, GINT_TO_POINTER (7), NULL);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Timezone"), renderer, "text", 8, NULL);
	gtk_tree_view_column_set_sort_column_id (column, 8);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dxcctreeview), column);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (model), 8, sort_column, GINT_TO_POINTER (8), NULL);

	gtk_widget_set_size_request (dxccwindow, 750, 400);

	lat = g_strdup ("");
	lon = g_strdup ("");
	tz = g_strdup ("");
	for (i = 1; i < programstate.countries; i++)
	{
		dxcc_data *d = g_ptr_array_index (dxcc, i);
		lat = g_strdup_printf ("%3.2f", (double)d->latitude/100);
		lon = g_strdup_printf ("%3.2f", -1 * (double)d->longitude/100);
		tz = g_strdup_printf ("%2.1f", (double)d->timezone/10);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			0, i, 1, d->px, 2, d->countryname, 3, enum_to_cont(d->continent), 4, d->cq,
			5, d->itu, 6, lat, 7, lon, 8, tz, -1);
	}
	g_free (lon);
	g_free (lat);
	g_free (tz);

	gtk_widget_show_all (dxccwindow);
}
