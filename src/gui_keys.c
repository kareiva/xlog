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

/* gui_keys.c - creation and destruction of the keys dialog
 *
 */

#include <gtk/gtk.h>
#include <string.h>
#include <glib/gstdio.h>

#include "support.h"
#include "utils.h"
#include "gui_keys.h"

extern GtkWidget *mainwindow;

static void open_help (gchar *filename)
{
	GtkWidget *keyswindow, *scrolledwindow, *keyswindowvbox, *keystreeview;
	gchar *buf, fbuf[80], *helpfile, *title;
	const gchar *encoding;
	FILE *in;
	gint l;
	GError *err;
	gsize utf8_len, read_len;
	gboolean retry;
	GtkListStore *model;
	GtkCellRenderer *renderer, *brenderer;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	GdkPixbuf *keyswindow_icon_pixbuf;

	title = g_strdup_printf ("xlog - %s", filename);
	keyswindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (keyswindow), title);
	keyswindow_icon_pixbuf = create_pixbuf ("xlog.png");
	if (keyswindow_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (keyswindow), keyswindow_icon_pixbuf);
		g_object_unref (keyswindow_icon_pixbuf);
	}
	g_free (title);
	gtk_widget_set_size_request (keyswindow, 500, 300);

	keyswindowvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (keyswindow), keyswindowvbox);
	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (keyswindowvbox), scrolledwindow, TRUE, TRUE, 0);

	model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	keystreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	g_object_unref (G_OBJECT (model));
	gtk_container_add (GTK_CONTAINER (scrolledwindow), keystreeview);

	brenderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (brenderer), "weight", "bold", NULL);
	column = gtk_tree_view_column_new_with_attributes	
		(_("Key"), brenderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (keystreeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		(_("Usage"), renderer, "text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (keystreeview), column);

#ifdef G_OS_WIN32
	helpfile = g_strdup_printf ("%s", filename);
#else
	helpfile = g_strdup_printf ("%s%s%s", XLOG_DOCDIR, G_DIR_SEPARATOR_S, 
		filename);
#endif
	buf = g_new0 (gchar, 100);
	g_get_charset (&encoding);
	in = g_fopen (helpfile, "r");
	if (in)
	{
		do 
		{
			if (fgets (fbuf, 80, in) == NULL) 
				break;
			else
			{
				l = strlen (fbuf);
				do
				{
					err = NULL;
					retry = FALSE;
					buf = g_convert_with_fallback
						(fbuf, l, "UTF-8", encoding, "?", &read_len, &utf8_len, &err);

					if (err != NULL)
					{
						if (err->code == G_CONVERT_ERROR_ILLEGAL_SEQUENCE)
						{
							/* Make our best bet by removing the erroneous char.
							   This will work for casual 8-bit strings with non-standard chars. */
							memmove (fbuf + read_len, fbuf + read_len + 1, l - read_len -1);
							l--;
							retry = TRUE;
						}
						g_error_free (err);
					}
				} while (retry);

				if (!buf)
					buf = g_strdup ("\n");

				if (buf[0] == '#')
					continue;
				else
				{
					gchar **split = g_strsplit (buf, ":", 2);
					gtk_list_store_append (GTK_LIST_STORE (model), &iter);
					if (split[0] && g_utf8_strlen (split[0], -1) > 0)
					{
						split[0] = my_strreplace (split[0], "\n", "");
						gtk_list_store_set
							(GTK_LIST_STORE (model), &iter, 0, split[0], -1);
					}
					if (split[1] && g_utf8_strlen (split[1], -1) > 0)
					{
						split[1] = my_strreplace (split[1], "\n", "");
						gtk_list_store_set
							(GTK_LIST_STORE (model), &iter, 1, split[1], -1);
					}
					g_strfreev (split);
				}
			}
		} 
		while (!feof (in));

		fclose (in);
	}
	g_free (buf);
	g_free (helpfile);
	gtk_widget_show_all (keyswindow);
}

void
on_helpkeys_activate (GtkMenuItem *menuitem,	gpointer user_data)
{
	open_help ("KEYS");
}
