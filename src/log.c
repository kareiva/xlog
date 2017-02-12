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
 * log.c - assorted utilities for maintaining the logs
 */

#include <gtk/gtk.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <glib/gstdio.h>

#ifndef G_OS_WIN32
#include <sys/wait.h>
#endif

#include "callbacks_mainwindow_list.h"
#include "gui_utils.h"
#include "utils.h"
#include "cfg.h"
#include "log.h"
#include "support.h"
#include "main.h"
#include "dxcc.h"

extern GtkWidget *mainnotebook;
extern programstatetype programstate;
extern preferencestype preferences;
extern gchar **qso;

/* backup a log */
void
backuplog (gchar * filename, gchar * backupfilename)
{
	gint ch;
	gchar *msg;
	FILE *in, *out;

	in = g_fopen (filename, "r");
	if (in)
	{
		out = g_fopen (backupfilename, "w");
		if (out)
		{
			while ((ch = getc (in)) != EOF)
				putc (ch, out);
			fclose (out);
		}
		else
		{
			msg = g_strdup_printf (_("Backup to %s failed: %s"),
				backupfilename, g_strerror (errno));
			g_warning ("%s", msg);
			g_free (msg);
		}
		fclose (in);
	}
}

/* extract name of the log from filename, returned string should be free'd */
gchar *
logname (gchar * filename)
{
	gchar *logname, *basen, **split;

	basen = g_path_get_basename (filename);
	split = g_strsplit (basen, ".", -1);
	logname = g_strdup (split[0]);
	g_free (basen);
	g_strfreev (split);
	return (logname);
}

/*
 * Prepend qsos to the log, we convert from locale to UTF-8 here. If for some
 * reason locale conversion fails we set a flag and replace the character
 * with a dot. If that fails, the field is emptied. The flag is used to display
 * a warning dialog. Only date, name, QTH, freefield1, freefield2 and the
 * remarks field are checked for locales.
 */
gint
fillin_list (LOGDB * handle, qso_t q[], gpointer arg)
{
	GtkTreeIter iter;
	GtkListStore *model;
	logtype *logw = (logtype *) arg;
	gchar *date = NULL, *name = NULL, *qth = NULL, *u1 = NULL,
		*u2 = NULL, *remarks = NULL;
	GError *error;
	gint i;
	GtkTreePath *path;

	programstate.qsos++;
	logw->qsos++;
	q[NR] = g_strdup_printf ("%d", logw->qsos);

	model = GTK_LIST_STORE
		(gtk_tree_view_get_model (GTK_TREE_VIEW (logw->treeview)));
	gtk_list_store_prepend (GTK_LIST_STORE (model), &iter);

	if (!q[DATE])
		date = g_strdup ("");
	else if (!g_utf8_validate (q[DATE], -1, NULL ))
	{
	  error = NULL;
		date = g_locale_to_utf8 (q[DATE], -1, NULL, NULL, &error);
		if (!date)
		{
			g_warning (_("Unable to convert '%s' to UTF-8: %s"), q[DATE], error->message);
			g_error_free (error);
			programstate.utf8error = TRUE;
			date = g_convert_with_fallback(q[DATE], strlen(q[DATE]), "UTF-8", "ISO-8859-1", ".", NULL, NULL, NULL);
			if (!date) date = g_strdup ("");
		}
 	}
	else date = g_strdup (q[DATE]);

	if (!q[GMT])
		q[GMT] = g_strdup ("");
	if (!q[GMTEND])
		q[GMTEND] = g_strdup ("");
	if (!q[CALL])
		q[CALL] = g_strdup ("");
	if (!q[BAND])
		q[BAND] = g_strdup ("");
	if (!q[MODE])
		q[MODE] = g_strdup ("");
	if (!q[RST])
		q[RST] = g_strdup ("");
	if (!q[MYRST])
		q[MYRST] = g_strdup ("");
	if (!q[AWARDS])
		q[AWARDS] = g_strdup ("");
	if (!q[QSLOUT])
		q[QSLOUT] = g_strdup ("");
	if (!q[QSLIN])
		q[QSLIN] = g_strdup ("");
	if (!q[POWER])
		q[POWER] = g_strdup ("");

	if (!q[NAME])
		name = g_strdup ("");
	else if (!g_utf8_validate (q[NAME], -1, NULL ))
	{
	  error = NULL;
		name = g_locale_to_utf8 (q[NAME], -1, NULL, NULL, &error);
		if (!name)
		{
			g_warning (_("Unable to convert '%s' to UTF-8: %s"), q[NAME], error->message);
			g_error_free (error);
			programstate.utf8error = TRUE;
			name = g_convert_with_fallback(q[NAME], strlen(q[NAME]), "UTF-8", "ISO-8859-1", ".", NULL, NULL, NULL);
			if (!name) name = g_strdup ("");
		}
 	}
	else name = g_strdup (q[NAME]);

	if (!q[QTH])
		qth = g_strdup ("");
	else if (!g_utf8_validate (q[QTH], -1, NULL ))
	{
	  error = NULL;
		qth = g_locale_to_utf8 (q[QTH], -1, NULL, NULL, &error);
		if (!qth)
		{
			g_warning (_("Unable to convert '%s' to UTF-8: %s"), q[QTH], error->message);
			g_error_free (error);
			programstate.utf8error = TRUE;
			qth = g_convert_with_fallback(q[QTH], strlen(q[QTH]), "UTF-8", "ISO-8859-1", ".", NULL, NULL, NULL);
			if (!qth) qth = g_strdup ("");
		}
 	}
	else qth = g_strdup (q[QTH]);

	if (!q[LOCATOR])
		q[LOCATOR] = g_strdup ("");

	if (!q[U1])
		u1 = g_strdup ("");
	else if (!g_utf8_validate (q[U1], -1, NULL ))
	{
	  error = NULL;
		u1 = g_locale_to_utf8 (q[U1], -1, NULL, NULL, &error);
		if (!u1)
		{
			g_warning (_("Unable to convert '%s' to UTF-8: %s"), q[U1], error->message);
			g_error_free (error);
			programstate.utf8error = TRUE;
			u1 = g_convert_with_fallback(q[U1], strlen(q[U1]), "UTF-8", "ISO-8859-1", ".", NULL, NULL, NULL);
			if (!u1) u1 = g_strdup ("");
		}
 	}
	else u1 = g_strdup (q[U1]);

	if (!q[U2])
		u2 = g_strdup ("");
	else if (!g_utf8_validate (q[U2], -1, NULL ))
	{
	  error = NULL;
		u2 = g_locale_to_utf8 (q[U2], -1, NULL, NULL, &error);
		if (!u2)
		{
			g_warning (_("Unable to convert '%s' to UTF-8: %s"), q[U2], error->message);
			g_error_free (error);
			programstate.utf8error = TRUE;
			u2 = g_convert_with_fallback(q[U2], strlen(q[U2]), "UTF-8", "ISO-8859-1", ".", NULL, NULL, NULL);
			if (!u2) u2 = g_strdup ("");
		}
 	}
	else u2 = g_strdup (q[U2]);

	if (!q[REMARKS])
		remarks = g_strdup ("");
	else if (!g_utf8_validate (q[REMARKS], -1, NULL ))
	{
	  error = NULL;
		remarks = g_locale_to_utf8 (q[REMARKS], -1, NULL, NULL, &error);
		if (!remarks)
		{
			g_warning (_("Unable to convert '%s' to UTF-8: %s"), q[REMARKS], error->message);
			g_error_free (error);
			programstate.utf8error = TRUE;
			remarks = g_convert_with_fallback(q[REMARKS], strlen(q[REMARKS]), "UTF-8", "ISO-8859-1", ".", NULL, NULL, NULL);
			if (!remarks) remarks = g_strdup ("");
		}
 	}
	else remarks = g_strdup (q[REMARKS]);

	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		NR, q[NR], DATE, date, GMT, q[GMT], GMTEND, q[GMTEND], CALL, q[CALL],
		BAND, q[BAND], MODE, q[MODE], RST, q[RST], MYRST, q[MYRST], AWARDS, q[AWARDS],
		QSLOUT, q[QSLOUT], QSLIN, q[QSLIN], POWER, q[POWER], NAME, name, QTH, qth,
		LOCATOR, q[LOCATOR], U1, u1, U2, u2, REMARKS, remarks, -1);
	if (!(logw->qsos % 1000))
	{
		while (gtk_events_pending ())
			gtk_main_iteration ();

		path = gtk_tree_path_new_from_string ("0");
		gtk_tree_view_scroll_to_cell
			(GTK_TREE_VIEW (logw->treeview), path, NULL, TRUE, 0.5, 0.0);
		gtk_tree_path_free (path);

		update_statusbar (_("Reading..."));
	}

	for (i = 0; i < QSO_FIELDS; i++)
		g_free (q[i]);

	if (date) g_free (date);
	if (remarks) g_free (remarks);
	if (name) g_free (name);
	if (qth) g_free (qth);
	if (u1) g_free (u1);
	if (u2) g_free (u2);
	return 0;
}

/* create a new struct for a log */
static logtype *
new_logwindow (void)
{
	logtype *newlog;
	gint i;

	newlog = g_new0 (struct logtype, 1);
	newlog->scrolledwindow = NULL;
	newlog->treeview = NULL;
	newlog->label = NULL;
	newlog->logname = NULL;
	newlog->filename = NULL;
	newlog->logchanged = FALSE;
	newlog->readonly = FALSE;
	newlog->qsos = 0;
	newlog->columns = 0;
	for (i = 0; i < QSO_FIELDS; i++)
		newlog->logfields[i] = 0;
	return (newlog);
}


/* open a log and return a struct */
logtype *
openlogwindow (LOGDB * lp, gchar * name, gint page)
{
	logtype *logwindow;
	gint i, j;
	gchar *logn, *labelname;
	GtkCellRenderer *renderer, *brenderer;
	GtkTreeViewColumn *column;
	GObject *selection;
	GtkListStore *model;

	logwindow = new_logwindow ();
	logwindow->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (logwindow->scrolledwindow);
	gtk_container_add (GTK_CONTAINER (mainnotebook), logwindow->scrolledwindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(logwindow->scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	model = gtk_list_store_new (QSO_FIELDS,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING);
	logwindow->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (logwindow->treeview), -1);
	g_object_unref (G_OBJECT (model));

	/* add callback for selecting a row */
	selection = G_OBJECT (gtk_tree_view_get_selection(GTK_TREE_VIEW (logwindow->treeview)));
	gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);
	g_signal_connect (selection, "changed",	G_CALLBACK (on_log_select_row), NULL);

	logwindow->columns = lp->column_nr;
	/* save the active columns */
	for (j = 0; j < logwindow->columns; j++)
		logwindow->logfields[j] = lp->column_fields[j];

	/* NR column is the first one */
	renderer = gtk_cell_renderer_text_new ();
	brenderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (brenderer), "weight", "bold", NULL);
	column = gtk_tree_view_column_new_with_attributes ("NR", renderer, "text", NR, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (logwindow->treeview), column);
	if (preferences.logcwidths2[NR] == 0)
		gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
	else
		gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN(column), preferences.logcwidths2[NR]);

	/* see which fields are in the log and add a column or hide it */
	for (j = 1; j < QSO_FIELDS; j++)
	{
		for (i = 0; i < lp->column_nr; i++)
		{
			if (j == lp->column_fields[i])
				break;
		}
		renderer = gtk_cell_renderer_text_new ();
		if (j == U1)
			column = gtk_tree_view_column_new_with_attributes
				(preferences.freefield1, renderer, "text", j, NULL);
		else if (j == U2)
			column = gtk_tree_view_column_new_with_attributes
				(preferences.freefield2, renderer, "text", j, NULL);
		else if (j == CALL)
			column = gtk_tree_view_column_new_with_attributes
				(strcolumn (j), brenderer, "text", j, NULL);
		else
			column = gtk_tree_view_column_new_with_attributes
				(strcolumn (j), renderer, "text", j, NULL);

		gtk_tree_view_column_set_sizing
			(GTK_TREE_VIEW_COLUMN(column), GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_resizable
			(GTK_TREE_VIEW_COLUMN(column), TRUE);
		gtk_tree_view_column_set_fixed_width
			(GTK_TREE_VIEW_COLUMN(column), preferences.logcwidths2[j]);
		gtk_tree_view_append_column (GTK_TREE_VIEW(logwindow->treeview), column);

		if (i == lp->column_nr)
			gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), FALSE);
	}

	g_object_set(G_OBJECT(logwindow->treeview), "fixed-height-mode", TRUE, NULL);
	gtk_widget_show (logwindow->treeview);
	gtk_container_add (GTK_CONTAINER (logwindow->scrolledwindow), logwindow->treeview);

	/* change the page label */
	logn = logname (name);
	logwindow->label = gtk_label_new (NULL);
	labelname = g_strdup_printf ("<b>%s</b>", logn);
	gtk_label_set_markup (GTK_LABEL (logwindow->label), labelname);
	gtk_widget_show (logwindow->label);
	g_free (labelname);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (mainnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK(mainnotebook), page), logwindow->label);
	gtk_misc_set_padding (GTK_MISC (logwindow->label), 10, 0);
	logwindow->logname = g_strdup (logn);
	g_free (logn);

	return (logwindow);
}

/* close child process when finished */
#ifndef G_OS_WIN32
static gboolean
childcheck (void)
{
  gint status, childpid;

  childpid = waitpid (-1, &status, WNOHANG);
  return (WIFEXITED(status) == 0);
}
#endif

typedef gchar *item_t[QSO_FIELDS];
static int savelog_compar_groupbycall(const void *b, const void *a)
{
	const gchar **item_a, **item_b;
	item_a = (const gchar **)a;
	item_b = (const gchar **)b;

	return strcmp(item_a[CALL], item_b[CALL]);
}


extern GPtrArray *dxcc;
static int savelog_compar_sortbydxcc(const void *b, const void *a)
{
	gint rescmp;
	gchar **item_a, **item_b;
	item_a = (gchar **)a;
	item_b = (gchar **)b;

	struct info info_a, info_b;
	info_a = lookupcountry_by_callsign (item_a[CALL]);
	info_b = lookupcountry_by_callsign (item_b[CALL]);
	dxcc_data *d_a = g_ptr_array_index (dxcc, info_a.country);
	dxcc_data *d_b = g_ptr_array_index (dxcc, info_b.country);

	/* Sort by DXCC first, then group by call sign within same DXCC */
	rescmp = strcmp(d_b->px, d_a->px);
	if (rescmp != 0)
		return rescmp;
	return strcmp(item_a[CALL], item_b[CALL]);
}

/* saving of the log */
void
savelog (gpointer arg, gchar * logfile, gint type, gint first, gint last)
{
	LOGDB *lp;
	gint i, j, k, pid, exported;
	G_CONST_RETURN gchar *label;
	gchar *pathstr;
	item_t *sorteditems;
	gint fields[QSO_FIELDS], widths[QSO_FIELDS];
	logtype *logw = (logtype *) arg;
	GtkTreeViewColumn *column;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;

	/* how many columns do we have and what are the labels, skip first field */
	/* NOTE: unknown fields have a fixed label */
	for (i = 0; i < logw->columns; i++)
	{
		column = gtk_tree_view_get_column (GTK_TREE_VIEW (logw->treeview), logw->logfields[i]);
		label = gtk_tree_view_column_get_title (column);
		if (logw->logfields[i] == U1)
			fields[i] = U1;
		else if (logw->logfields[i] == U2)
			fields[i] = U2;
		else
			fields[i] = parse_column_name (label);
		widths[i] = parse_field_width (fields[i]);
	}

#ifndef G_OS_WIN32
	pid = fork (); /* use fork for log saving */
	if (pid == -1)
		g_warning(_("fork failed when saving log: %s"), g_strerror (errno));
#else
	pid = 0;
#endif

	if (pid == 0)
	{ /* this is the child */
	        exported = last - first + 1;
	        save_num_qsos_to_export(exported); /* saved for EDI export, nobody else needs it for now */
		lp = log_file_create (logfile, type, logw->columns, fields, widths);
		k = 0;
		if (lp)
		{
			model = gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview));
			/* create an array to be used for sorting */
			sorteditems = g_new0 (item_t, exported);
			/* go through the QSO's and store in the sort array */
			for (i = logw->qsos - first; i >= logw->qsos - last; i--)	
			{
				pathstr = g_strdup_printf ("%d", i);
				path = gtk_tree_path_new_from_string (pathstr);
				gtk_tree_model_get_iter (model, &iter, path);
				for (j = 0; j < logw->columns; j++)
				{
					gtk_tree_model_get (model, &iter, logw->logfields[j], &sorteditems[k][fields[j]], -1);
					if (fields[j] == DATE || fields[j] == NAME || fields[j] == QTH || fields[j] == U1 || fields[j] == U2 || fields[j] == REMARKS)
						sorteditems[k][fields[j]] = g_locale_from_utf8 (sorteditems[k][fields[j]], -1, NULL, NULL, NULL);
				}
				k++;
				gtk_tree_path_free (path);
				g_free (pathstr);
			}
	
			if (type == TYPE_LABELS)
			{
				if (preferences.tsvsortbydxcc)
					qsort(sorteditems, exported, sizeof(gchar*)*QSO_FIELDS, &savelog_compar_sortbydxcc);
				else if (preferences.tsvgroupbycallsign > 1)
					qsort(sorteditems, exported, sizeof(gchar*)*QSO_FIELDS, &savelog_compar_groupbycall);
			}
	
			/* QSO's have been sorted (or not), now save them */

			for (i = 0; i <= exported -1; i++)
				log_file_qso_append (lp, sorteditems[i]);
			/* free the sortarray */
			g_free(sorteditems);

			log_file_close (lp);
#ifndef G_OS_WIN32
			_exit (0);
#else
			return;
#endif
		}
	}
	/* parent running */
#ifndef G_OS_WIN32
	g_timeout_add (1000, (GSourceFunc) childcheck, NULL);
#endif
}


/* look for logs in dir */
GPtrArray*
getxlogs (gchar *path, gchar *patt)
{
	GError *error  = NULL;

 	GDir *dir = g_dir_open (path, 0, &error);
	gchar *pattern = g_strdup_printf ("%s.xlog", patt);
 	GPtrArray *arr = g_ptr_array_new ();
	if (!error)
	{
		const gchar *dirname = g_dir_read_name (dir);
		while (dirname)
		{
			if (g_pattern_match_simple (pattern, dirname))
				g_ptr_array_add (arr, g_strdup(dirname));
			dirname = g_dir_read_name (dir);
		}
		g_dir_close (dir);
	}
	g_free (pattern);
	return (arr);
}
