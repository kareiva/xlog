/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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

#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "logfile.h"

#ifdef ENABLE_NLS
#       include <libintl.h>
#       undef _
#       define _(String) dgettext (PACKAGE, String)
#       ifdef gettext_noop
#               define N_(String) gettext_noop (String)
#       else
#               define N_(String) (String)
#       endif
#else
#       define _(String) (String)
#       define N_(String) (String)
#endif

static gint flog_open (LOGDB *);
static void flog_close (LOGDB *);
static gint flog_create (LOGDB *);
static gint flog_qso_append (LOGDB *, const qso_t *);
static gint flog_qso_foreach
	(LOGDB *, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);

const struct log_ops flog_ops = {
	.open = flog_open,
	.close = flog_close,
	.create = flog_create,
	.qso_append = flog_qso_append,
	.qso_foreach = flog_qso_foreach,
	.type = TYPE_FLOG,
	.name = "Flog",
	.extension = ".xlog",
};

/* open for read */
gint
flog_open (LOGDB * handle)
{
	FILE *fp;
	gint column_nr = 0, field_width = 0, in_space_state = 0;
	gchar *p, *result, *field_name, header_line[FLOG_MAX_LINE_SIZE];

	fp = g_fopen (handle->path, "r");
	if (!fp)
		return -1;
	handle->priv = (gpointer) fp;

	result = fgets (header_line, FLOG_MAX_LINE_SIZE, fp);
	
	/* error handling */
	if (strlen (header_line) < 80)
	{
		g_warning (_("The header of log %s is corrupted."), handle->path);
		g_warning (_("It's length is smaller than 80."));
		g_warning (_("This does not look like an xlog file."));
		return 1;
	}
	if (strncmp (header_line, "DATE ", 5))
	{
		g_warning (_("The header of log %s is corrupted."), handle->path);
		g_warning (_("It does not start with DATE."));
		g_warning (_("This does not look like an xlog file."));
		return 1;
	}

	field_name = header_line;
	for (p = header_line; *p; p++)
	{
		field_width++;
		if (in_space_state)
		{
			if (*p != ' ')
			{
				/* start of a new column */
				in_space_state = 0;

				/* 
				 * add previous column, 
				 * now we know the column width
				 */
				handle->column_fields[column_nr] =
					parse_field_name (field_name);
				handle->column_widths[column_nr] = field_width;
				column_nr++;

				field_width = 1;
				field_name = p;
			}
		}
		else if (*p == ' ')
		{
			/* end of field name */
			in_space_state = 1;
			*p = '\0';
		}
		/* last field */
		if (*p == '\n')
		{
			*p = '\0';
		}
	}
	handle->column_fields[column_nr] = parse_field_name (field_name);
	handle->column_widths[column_nr] = MAX_VARIABLE_LEN;
	handle->column_nr = column_nr + 1;

	return 0;
}

/* create and open for read */
gint
flog_create (LOGDB * handle)
{
	FILE *fp;
	gint i;

	fp = g_fopen (handle->path, "w");
	if (!fp)
		return -1;

	handle->priv = (gpointer) fp;

	/* write header line */
	for (i = 0; i < handle->column_nr - 1; i++)
		{
			fprintf (fp, "%-*s", handle->column_widths[i],
				 strfield (handle->column_fields[i]));
		}
	/* last field can be variable, omit trailing spaces */
	fprintf (fp, "%s\n",
		 strfield (handle->column_fields[handle->column_nr - 1]));

	return 0;
}

void
flog_close (LOGDB * handle)
{
	FILE *fp = (FILE *) handle->priv;
	fclose (fp);
}

gint
flog_qso_append (LOGDB * handle, const qso_t * q)
{
	FILE *fp = (gpointer) handle->priv;
	gint i;

	for (i = 0; i < handle->column_nr - 1; i++)
		{
			gint field = handle->column_fields[i];
			fprintf (fp, "%-*s", handle->column_widths[i], q[field]);
		}
	/* last field is variable length, omit trailing spaces */
	fprintf (fp, "%s\n", q[handle->column_fields[handle->column_nr - 1]]);

	return 0;
}

gint
flog_qso_foreach (LOGDB * handle, gint (*fn) (LOGDB *, qso_t *, gpointer arg),
			gpointer arg)
{
	FILE *fp = (gpointer) handle->priv;
	gint i, width, ret;
	qso_t *q;
	gchar buffer[MAX_VARIABLE_LEN], *result;


	while (!feof (fp))
	{
		q = g_new0 (qso_t, QSO_FIELDS);

		for (i = 0; i < handle->column_nr - 1; i++)
		{
			width = handle->column_widths[i];
			result = fgets (buffer, width, fp);
			if (strlen (buffer) != width - 1 && i < handle->column_nr - 2)
				return 0;		/* broken file */
			q[handle->column_fields[i]] = g_strdup (g_strstrip (buffer));
		}
		/* last field */
		result = fgets (buffer, MAX_VARIABLE_LEN, fp);
		width = strlen (buffer);
		/* chop off EOL */
		if (width > 0 && buffer[width - 1] == '\n')
			buffer[width - 1] = '\0';
		q[handle->column_fields[handle->column_nr - 1]] =
			g_strdup (g_strstrip (buffer));

		/*
		 * populate fields not present in log file
		 */
		for (i = 0; i < QSO_FIELDS; i++)
			if (!q[i])
				q[i] = g_strdup ("");

		ret = (*fn) (handle, q, arg);
		if (ret) return ret;
	}
	return 0;
}
