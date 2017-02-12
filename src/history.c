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
 * history.c - private functions for handling search history.
 */

#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "history.h"
#include "support.h"

extern GtkWidget *mainwindow;
extern gchar *xlogdir;
GList *searchhistory;

/*
 * Recall history and copy into the appropriate GList.
 */
void
loadhistory (void)
{
	gchar *historyfile, history[1024], **histsplit = NULL;
	FILE *fp;

	historyfile = g_strconcat (xlogdir, G_DIR_SEPARATOR_S, "history", NULL);
	fp = g_fopen (historyfile, "r");
	if (fp != NULL)
		{
			while (!feof (fp))
	{
		if (fscanf (fp, "%s", history) == EOF)
			break;
		histsplit = g_strsplit (history, ":", -1);
		if (!g_ascii_strcasecmp (histsplit[0], "se"))
			{
				g_strdelimit (histsplit[1], "_", ' ');
				searchhistory =
		g_list_append (searchhistory, g_strdup (histsplit[1]));
			}
		g_strfreev (histsplit);
	}
			fclose (fp);
		}
	g_free (historyfile);
}

/*
 * Save history to ~/.xlog/history
 */
void
savehistory (void)
{
	gchar *historyfile;
	FILE *fp;
	guint i, n;
	gchar *search;

	historyfile = g_strconcat (xlogdir, G_DIR_SEPARATOR_S, "history", NULL);
	if ((g_list_length (searchhistory) > 0))
		{
			fp = g_fopen (historyfile, "w");
			if (fp != NULL)
	{
		if ((n = g_list_length (searchhistory)) > 0)
			{
				for (i = 0; i < n; i++)
		{
			search = g_list_nth_data (searchhistory, i);
			g_strdelimit (search, " ", '_');
			fprintf (fp, "se:%s\n", search);
			g_free (search);
		}
			}
		fclose (fp);
	}
		}
	g_free (historyfile);
}
