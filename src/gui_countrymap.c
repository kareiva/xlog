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

/* gui_countrymap.c */

#include <gtk/gtk.h>

#include "support.h"
#include "gui_countrymap.h"
#include "utils.h"
#include "dxcc.h"

GtkWidget *countrymap = NULL;
static GdkPixmap *pm = NULL;
static gchar *pxformaps = NULL;
static gint savew = -1, saveh = -1;
extern GtkWidget *mainwindow;
extern GPtrArray *dxcc;

static gboolean
map_delete (GtkWidget * widget, GdkEvent *event, gpointer user_data)
{
	countrymap = NULL;
	g_free (pxformaps);
	pxformaps = NULL;
	return FALSE;
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

static gboolean
map_configure (GtkWidget *da, GdkEventConfigure *event, gpointer data)
{
	if (pm)
	{
		g_object_unref (pm);
		pm = NULL;
	}
	pm = gdk_pixmap_new(da->window, da->allocation.width, da->allocation.height, -1);
	gdk_draw_rectangle(pm, da->style->white_gc,	TRUE, 0, 0,
		da->allocation.width, da->allocation.height);
	if (pxformaps)
	{
#ifdef G_OS_WIN32
		gchar *map_location = g_strconcat
		("maps", G_DIR_SEPARATOR_S, pxformaps, ".png", NULL);
#else
		gchar *map_location = g_strconcat (XLOG_DATADIR, G_DIR_SEPARATOR_S,
			"maps", G_DIR_SEPARATOR_S, pxformaps, ".png", NULL);
#endif
		GdkPixbuf *background = gdk_pixbuf_new_from_file (map_location, NULL);
		g_free (map_location);
		if (background)
		{
			gdouble w = da->allocation.width;
			gdouble h = da->allocation.height;
			GdkPixbuf *buf =  gdk_pixbuf_scale_simple
				(background, w, h, GDK_INTERP_BILINEAR);
			gdk_draw_pixbuf (pm, da->style->black_gc, buf,
				0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
			g_object_unref (buf);
			g_object_unref (background);
		}
	}
	gdk_draw_drawable(da->window, da->style->fg_gc[gtk_widget_get_state(da)],
		pm,	da->allocation.x, da->allocation.y,
		da->allocation.x, da->allocation.y,
		da->allocation.width, da->allocation.height);
	return TRUE;
}

void countrymap_refresh (gchar *px)
{
	if (!g_ascii_strcasecmp(px, pxformaps)) return;
	pxformaps = g_strdup (px);
#ifdef G_OS_WIN32
	gchar *map_location = g_strconcat
		("maps", G_DIR_SEPARATOR_S, px, ".png", NULL);
	map_location = my_strreplace (map_location, "/", "\\");
#else
	gchar *map_location = g_strconcat (XLOG_DATADIR, G_DIR_SEPARATOR_S,
		"maps", G_DIR_SEPARATOR_S, px, ".png", NULL);
#endif
	gint width, height;
	GdkPixbufFormat *f = gdk_pixbuf_get_file_info (map_location, &width, &height);
	if (f)
	{
		if (width != savew || height != saveh)
		{
			gdk_window_resize (countrymap->window, width, height);
			savew = width;
			saveh = height;
		}
		else
		{
			GtkWidget *da = lookup_widget (countrymap, "da");
			map_configure (da, NULL, NULL);
		}
	}
	g_free (map_location);
}

void on_countrymap_activate (GtkAction *action, gpointer user_data)
{
	if (countrymap)
	{
		gtk_window_present (GTK_WINDOW(countrymap));
		return;
	}
	countrymap = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus (GTK_WINDOW(countrymap), FALSE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(countrymap), TRUE);
	gtk_window_set_title (GTK_WINDOW (countrymap), _("xlog - country map"));
	GdkPixbuf *mapw_icon_pixbuf = create_pixbuf ("xlog-map.png");
	if (mapw_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (countrymap), mapw_icon_pixbuf);
		g_object_unref (mapw_icon_pixbuf);
	}
	g_signal_connect (countrymap, "delete_event",G_CALLBACK (map_delete), NULL);

	GtkWidget *da = gtk_drawing_area_new ();
	gtk_container_add (GTK_CONTAINER(countrymap), da);
	g_signal_connect (da, "expose_event", G_CALLBACK (map_expose), NULL);
	g_signal_connect (da, "configure_event", G_CALLBACK (map_configure), NULL);
	GLADE_HOOKUP_OBJECT (countrymap, da, "da");
	pxformaps = g_strdup ("");
        GtkWidget *awardsentry = lookup_widget (mainwindow, "awardsentry");
        gchar *str = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
        guint st, zone, cont, iota;
        gchar *result = valid_awards_entry (str, &st, &zone, &cont, &iota);
	gtk_widget_show_all (countrymap);
        if (result) countrymap_refresh (result);
        else
        {
                GtkWidget *callentry = lookup_widget (mainwindow, "callentry");
                gchar *str = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
                struct info lookup = lookupcountry_by_callsign (str);
                if (lookup.country > 0)
                {
                        dxcc_data *d = g_ptr_array_index (dxcc, lookup.country);
                        countrymap_refresh (d->px);
                }
        }
        g_free (result);
        g_free (str);
}
