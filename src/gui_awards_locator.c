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

/* gui_awards_locator.c */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <hamlib/rotator.h>
#include "gui_awards_locator.h"
#include "gui_warningdialog.h"
#include "support.h"
#include "xlog_enum.h"
#include "awards_enum.h"
#include "cfg.h"

extern GtkWidget *mainwindow, *scorewindow;
extern preferencestype preferences;
extern GHashTable *loc_w[MAX_BANDS + 1];
extern GHashTable *loc_c[MAX_BANDS + 1];
static GdkPixmap *pm = NULL;
GtkWidget *locatormap = NULL;

static void locator_gh_collect (gpointer key, gpointer value, gpointer user_data)
{
	guint *locator_list = (guint*)user_data;
	guint locator;

	locator = locator_to_num(key);
	while (*locator_list != NOT_A_LOCATOR)
	{
		if (*locator_list == locator)
			return;
		locator_list++;
	}
	*locator_list = locator;
}

static int num_compar(const void *a, const void *b)
{
	return *(guint*)a - *(guint*)b;
}

/* create a boring locator scoring list */
void on_awards_locator_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (!gtk_widget_get_visible(scorewindow))
		warningdialog (_("xlog - locator list"),
			_("Please enable the scoring window"), "gtk-dialog-warning");
	if (preferences.awardsloc == 0)
	{
		warningdialog (_("xlog - locator list"),
                        _("You will need to enable locator scoring from the \"windows and dialogs\" dialog"), "gtk-dialog-warning");
		return;
	}

	GtkWidget *dialog = gtk_dialog_new_with_buttons (_("xlog - locator list"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	gtk_dialog_add_button (GTK_DIALOG (dialog),	GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_widget_set_size_request (dialog, 450, 500);

	GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	/* 2 extra columns, iota and 'all bands */
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
		("iota", renderer, "text", 0, NULL);
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
	gint i, j, nmemb=0;
	static guint locator_list[1000];

	for (i=0; i<1000; i++)
		locator_list[i] = NOT_A_LOCATOR;

	for (j = 0; j < MAX_BANDS; j++)
	{
		g_hash_table_foreach (loc_w[j], &locator_gh_collect, (gpointer)locator_list);
	}

	for (i=0; i<1000 && locator_list[i]!=NOT_A_LOCATOR; i++)
		nmemb++;

	qsort(locator_list, nmemb, sizeof(guint), &num_compar);

	for (i=0; i<nmemb; i++)
	{
		gchar *locatorstr = num_to_locator(locator_list[i]);
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, g_ascii_strup(locatorstr, -1), -1);
		for (j = 0; j < MAX_BANDS; j++)
		{
			gpointer p_locator_c = g_hash_table_lookup (loc_c[j], g_ascii_strup(locatorstr, -1));
			if (p_locator_c && (GPOINTER_TO_INT(p_locator_c) > 0))
			{
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, j+1, "X", -1);
			}
			else
			{
				gpointer p_locator_w = g_hash_table_lookup (loc_w[j], g_ascii_strup(locatorstr, -1));
			
				if (p_locator_w && (GPOINTER_TO_INT(p_locator_w) > 0))
				{
					gchar *str = g_strdup_printf ("%d", GPOINTER_TO_INT(p_locator_w));
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, j+1, str, -1);
					g_free (str);
				}
			}
		}
	}

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}

/* draw locator scoring in a world map */
static void
draw_locator_scoring (GtkWidget *da, GdkPixmap *pixmap, gint band)
{
	gint i, j, nmembw=0, nmembc=0;
	GdkColor color;

	/* set up colors */
	GdkGC *gc1 = gdk_gc_new (pixmap);
	color.red = 50000;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color (gc1, &color);

	GdkGC *gc2 = gdk_gc_new (pixmap);
	color.red = 0;
	color.green = 50000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color (gc2, &color);
  
	GdkGC *gc3 = gdk_gc_new (pixmap);
	color.red = 0;
	color.green = 0;
	color.blue = 50000;
	gdk_gc_set_rgb_fg_color (gc3, &color);

	/* collect scoring from worked and confirmed arrays */
	static guint locatorw_list[32400];
	static guint locatorc_list[32400];

	for (i=0; i<32400; i++)
		locatorw_list[i] = NOT_A_LOCATOR;
	for (i=0; i<32400; i++)
		locatorc_list[i] = NOT_A_LOCATOR;

	g_hash_table_foreach (loc_w[band], &locator_gh_collect, (gpointer)locatorw_list);
	g_hash_table_foreach (loc_c[band], &locator_gh_collect, (gpointer)locatorc_list);

	for (i=0; i<32400 && locatorw_list[i]!=NOT_A_LOCATOR; i++)
		nmembw++;
	for (i=0; i<32400 && locatorc_list[i]!=NOT_A_LOCATOR; i++)
		nmembc++;

	qsort(locatorw_list, nmembw, sizeof(guint), &num_compar);
	qsort(locatorc_list, nmembc, sizeof(guint), &num_compar);


	gdouble w = da->allocation.width;
	gdouble h = da->allocation.height;
	gdouble checksize_w = w / 180;
	gdouble checksize_h = h / 180;

	/* backing pixmap */
#ifdef G_OS_WIN32
	gchar *map_location = g_strconcat
		("maps", G_DIR_SEPARATOR_S, "worldmap.jpg", NULL);
#else
	gchar *map_location = g_strconcat (XLOG_DATADIR, G_DIR_SEPARATOR_S,
		"maps", G_DIR_SEPARATOR_S, "worldmap.jpg", NULL);
#endif
	GdkPixbuf *background = gdk_pixbuf_new_from_file (map_location, NULL);
	g_free (map_location);
	if (background)
	{
		GdkPixbuf *buf =  gdk_pixbuf_scale_simple
			(background, w, h, GDK_INTERP_BILINEAR);
		gdk_draw_pixbuf (pm, da->style->black_gc, buf,
			0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
		g_object_unref (buf);
		g_object_unref (background);
	}

	/* AA-RR grid */
	PangoLayout *layout;
	gchar *str = g_new0(gchar, 2);
	for (i=1; i<19; i++) for (j=1; j<19; j++)
	{
		str[0]= i+64;
		str[1]= j+64;
		layout = gtk_widget_create_pango_layout (da, str);
		gdk_draw_layout (pixmap, gc3,
			(i-0.7)*10*checksize_w,
			h-(j-0.2)*10*checksize_h,
			layout);
		g_object_unref(G_OBJECT(layout));
	}
	g_free (str);

	/* red (worked) and green (confirmed) squares */
	gint y, x, x1, y1;
	for (i=0; i<nmembw; i++)
	{
		x = locatorw_list[i]/10000;
		locatorw_list[i] = locatorw_list[i] - x * 10000;
		y = locatorw_list[i]/100;
		locatorw_list[i] = locatorw_list[i] - y * 100;
		x1 = locatorw_list[i]/10;
		locatorw_list[i] = locatorw_list[i] - x1 * 10;
		y1 = locatorw_list[i];
		gdk_draw_rectangle (pixmap, gc1, FALSE,
			(10*(x-1) + x1) * checksize_w,
			h - ((10*(y-1) + y1+1) * checksize_h),
			checksize_w, checksize_h);
	}
	for (i=0; i<nmembc; i++)
	{
		x = locatorc_list[i]/10000;
		locatorc_list[i] = locatorc_list[i] - x * 10000;
		y = locatorc_list[i]/100;
		locatorc_list[i] = locatorc_list[i] - y * 100;
		x1 = locatorc_list[i]/10;
		locatorc_list[i] = locatorc_list[i] - x1 * 10;
		y1 = locatorc_list[i];
		gdk_draw_rectangle (pixmap, gc2, FALSE,
			(10*(x-1) + x1) * checksize_w,
			h - ((10*(y-1) + y1+1) * checksize_h),
			checksize_w, checksize_h);
	}

	g_object_unref (gc1);
	g_object_unref (gc2);
	g_object_unref (gc3);
}

static gboolean
map_configure (GtkWidget *da, GdkEventConfigure *event, gpointer data)
{
	/* find out which band has been selected */
	gint band = gtk_combo_box_get_active (data);
	gint j = 0;
	gint i;
	for (i = 0; i < MAX_BANDS; i++)
	{
		if (preferences.scoringbands[i] == 1)
		if (band == j++)
			break;
	}
	if (pm)
	{
		g_object_unref (pm);
		pm = NULL;
	}
	pm = gdk_pixmap_new(da->window, da->allocation.width, da->allocation.height, -1);
	gdk_draw_rectangle(pm, da->style->white_gc,	TRUE, 0, 0,
		da->allocation.width, da->allocation.height);
	draw_locator_scoring (da, pm, i);
	gdk_draw_drawable(da->window, da->style->fg_gc[gtk_widget_get_state(da)],
		pm,	da->allocation.x, da->allocation.y,
		da->allocation.x, da->allocation.y,
		da->allocation.width, da->allocation.height);
	return TRUE;
}

static gboolean
map_expose (GtkWidget *da, GdkEventExpose *event, gpointer data)
{
	if (pm) gdk_draw_drawable
		(da->window, da->style->fg_gc[gtk_widget_get_state(da)], pm,
		event->area.x, event->area.y, event->area.x, event->area.y,
		event->area.width, event->area.height);
	return FALSE;
}

static void combochanged (GtkComboBox *widget, gpointer user_data)
{
	/* draw the new map */
	GtkWidget *da = GTK_WIDGET(user_data);
	map_configure (da, NULL, widget);
}

static gboolean
map_delete (GtkWidget * widget, GdkEvent *event, gpointer user_data)
{
	gtk_widget_destroy (widget);
	locatormap = NULL;
	return FALSE;
}

static void closeclicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy (user_data);
	locatormap = NULL;
}

static gchar *xy2locator (const gint x, const gint y, GdkWindow *win)
{
	gint w, h;
	gchar *locator = g_try_malloc (5);

	gdk_drawable_get_size (win, &w, &h);
	gdouble lat = ((gdouble)y/h-0.5)*-180;
	gdouble lon = ((gdouble)x/w-0.5)*360;
	gint retcode = longlat2locator (lon, lat, locator, 2);
	if (retcode == RIG_OK)
		return locator;
	return g_strdup ("");;
}

static gboolean
motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;
	GdkModifierType state;

	gdk_window_get_pointer (event->window, &x, &y, &state);
	gchar *temp = xy2locator (x, y, event->window);
    gtk_widget_set_tooltip_text (widget, temp);
    g_free (temp);
	
	return TRUE;
}

void on_awards_locatormap_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (!gtk_widget_get_visible(scorewindow))
		warningdialog (_("xlog - locator map"),
			_("Please enable the scoring window"), "gtk-dialog-warning");
	if (preferences.awardsloc == 0)
	{
		warningdialog (_("xlog - locator map"),
_("You will need to enable locator scoring from the \"windows and dialogs\" dialog"), "gtk-dialog-warning");
		return;
	}

	if (locatormap)
	{
		gtk_window_present (GTK_WINDOW(locatormap));
		return;
	}

	locatormap = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(locatormap), TRUE);
	gtk_window_set_title (GTK_WINDOW (locatormap), _("xlog - locator map"));
	gtk_widget_set_size_request (locatormap, 800, 500);
	g_signal_connect (locatormap, "delete_event",G_CALLBACK (map_delete), NULL);

	GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (locatormap), vbox);
	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new (_("Select band (MHz) for locator scoring"));
	gtk_container_add (GTK_CONTAINER(hbox), label);
	GtkWidget *combo = gtk_combo_box_new_text ();
	gtk_container_add (GTK_CONTAINER(hbox), combo);
	GtkWidget *sep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), sep, FALSE, TRUE, 0);
	GtkWidget *da = gtk_drawing_area_new ();
	gtk_box_pack_start (GTK_BOX (vbox), da, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT (locatormap, da, "drawingarea");
	g_signal_connect (da, "expose_event", G_CALLBACK (map_expose), NULL);
	g_signal_connect (da, "configure_event", G_CALLBACK (map_configure), combo);
	g_signal_connect (da, "motion_notify_event", G_CALLBACK (motion_notify_event), NULL);
    gtk_widget_set_events (da, GDK_EXPOSURE_MASK
                             | GDK_LEAVE_NOTIFY_MASK
                             | GDK_POINTER_MOTION_MASK
                             | GDK_POINTER_MOTION_HINT_MASK);


	/* fill in the combobox and find out what bands we want scoring for */
	gint i, j = 0; gchar *band;
	for (i = 0; i < MAX_BANDS; i++)
	{
		band = band_enum2char (i);
		if (preferences.scoringbands[i] == 1)
		{
			gtk_combo_box_append_text (GTK_COMBO_BOX(combo), band);
			j++;
		}
		g_free (band);
	}
	gtk_combo_box_append_text (GTK_COMBO_BOX(combo), _("All"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), j);
	g_signal_connect (G_OBJECT(combo), "changed", G_CALLBACK (combochanged), da);
	GtkWidget *okbutton = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_box_pack_start (GTK_BOX (vbox), okbutton, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT(okbutton), "clicked", G_CALLBACK (closeclicked), locatormap);
	gtk_widget_show_all (locatormap);
}
