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

/* gui_helpdialog.c - creation and destruction of the help dialog
 *
 * dialog for displaying documentation, the document which is loaded
 * may have tags, depending on these tags the appearance is altered:
 *
 * '-' on first line: bold text
 * '!' on first line: blue text
 * '\t' (tab) on first line: italic text
 *
 * text between '{' and '}' are images to be loaded, they may have leading
 * whitespaces or text.
 */

#include <gtk/gtk.h>
#include <string.h>
#include <glib/gstdio.h>
#ifdef G_OS_WIN32
#include <stdlib.h>
#endif

#include "gui_helpdialog.h"
#include "support.h"
#include "utils.h"

extern GtkWidget *mainwindow;

static void open_help (gchar *filename)
{
	GtkWidget *helpdialog, *helptextview, *scrolledwindow, *ok_button;
	gchar *buf, fbuf[80], *helpfile, *title;
	const gchar *encoding;
	FILE *in;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gint response, l;
	GError *err;
	gsize utf8_len, read_len;
	gboolean retry;

	title = g_strdup_printf ("xlog - %s", filename);
	helpdialog = gtk_dialog_new_with_buttons (title,
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	ok_button = gtk_dialog_add_button (GTK_DIALOG (helpdialog),
		GTK_STOCK_OK, GTK_RESPONSE_OK);
	g_free (title);
	gtk_widget_set_size_request (helpdialog, 550, 300);

	scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (helpdialog)->vbox), scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	helptextview = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW(helptextview), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(helptextview), FALSE);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), helptextview);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(helptextview));
	gtk_text_buffer_get_start_iter (buffer, &iter);
#ifdef G_OS_WIN32
	helpfile = g_strdup_printf ("%s", filename);
#else
	helpfile = g_strdup_printf ("%s%s%s", XLOG_DOCDIR, G_DIR_SEPARATOR_S, 
		filename);
#endif
	gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag (buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag (buffer, "blue", "foreground", "blue", NULL);

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
					buf = g_convert_with_fallback(fbuf, l, "UTF-8", encoding, "?", &read_len, &utf8_len, &err);

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

				if (buf[0] == '-')
					gtk_text_buffer_insert_with_tags_by_name
						(buffer, &iter, buf, -1, "bold", NULL);
				else if (buf[0] == '!')
				{
					buf[0] = ' ';
					gtk_text_buffer_insert_with_tags_by_name
						(buffer, &iter, buf, -1, "blue", NULL);
				}
				else if (buf[0] == '\t')
					gtk_text_buffer_insert_with_tags_by_name
						(buffer, &iter, buf, -1, "italic", NULL);
				else
					gtk_text_buffer_insert (buffer, &iter, buf, -1);
			}
		} 
		while (!feof (in));

		fclose (in);
	}
	g_free (buf);
	g_free (helpfile);
	gtk_dialog_set_default_response (GTK_DIALOG (helpdialog),
					 GTK_RESPONSE_OK);

	gtk_widget_show_all (helpdialog);
	response = gtk_dialog_run (GTK_DIALOG(helpdialog));
	gtk_widget_destroy (helpdialog);
}

void
on_helpmanual_activate (GtkMenuItem *menuitem, gpointer user_data)
{
#ifdef G_OS_WIN32
	gchar *win32_dir = g_win32_get_package_installation_directory_of_module (NULL);
	gchar *command = g_strdup_printf ("iexplore.exe %s%smanual%soutput%shtml%sindex.html", win32_dir, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
	win32_fork (command);
	g_free (win32_dir);
	g_free (command);
#else
	gchar *command[] = {"xdg-open", NULL, NULL};
	command[1] = g_strconcat (XLOG_DOCDIR, G_DIR_SEPARATOR_S, "manual", G_DIR_SEPARATOR_S, "output", G_DIR_SEPARATOR_S, "html", G_DIR_SEPARATOR_S, "index.html", NULL);
	g_spawn_async (NULL, command, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
	g_free (command[1]);
#endif
}


void
on_helptodo_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	open_help ("TODO");
}

void
on_helpchangelog_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	open_help ("ChangeLog");
}

void
on_helpbugs_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	open_help ("BUGS");
}
