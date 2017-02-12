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

/*
 * twlog.c - support for WA0EIR Ted's twlog file format, as of v1.3
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "logfile.h"

/*
 * file fields
 */
const gint twlog_fields[] = { DATE, GMT, CALL, BAND, MODE, POWER,
  RST, MYRST, GMTEND, REMARKS
};
const gint twlog_widths[] = { 11, 11, 11, 7, 7, 7, 7, 7, 11, 80 };
const gint twlog_field_nr = 10;

static gint twlog_open (LOGDB *);
static void twlog_close (LOGDB *);
static gint twlog_create (LOGDB *);
static gint twlog_qso_append (LOGDB *, const qso_t *);
static gint twlog_qso_foreach (LOGDB *,
				gint (*fn) (LOGDB *, qso_t *, gpointer arg),
				gpointer arg);

const struct log_ops twlog_ops = {
	.open = twlog_open,
	.close = twlog_close,
	.create = twlog_create,
	.qso_append = twlog_qso_append,
	.qso_foreach = twlog_qso_foreach,
	.type = TYPE_TWLOG,
	.name = "Twlog",
	.extension = "",
};

/*
 * open for read
 */
gint
twlog_open (LOGDB * handle)
{
	FILE *fp;
	const gint xlog_fields [] = {DATE, GMT, GMTEND, CALL, BAND, MODE, RST, MYRST, POWER, REMARKS};

	fp = g_fopen (handle->path, "r");
	if (!fp)
		return -1;
	handle->priv = (gpointer) fp;

	/* set columns to be used in xlog */
	handle->column_nr = 10;
	memcpy (handle->column_fields, xlog_fields, sizeof (xlog_fields));
	/* TODO: set and use handle->column_widths */

	return 0;
}

/*
 * open for write
 */
gint
twlog_create (LOGDB * handle)
{
	FILE *fp;

	fp = g_fopen (handle->path, "w");
	if (!fp)
		return -1;
	handle->priv = (gpointer) fp;
	return 0;
}

void
twlog_close (LOGDB * handle)
{
	FILE *fp = (FILE *) handle->priv;
	fclose (fp);
}

/* 
 * each field 15 positions, remarks on a separate line, 80 wide
 */
gint
twlog_qso_append (LOGDB * handle, const qso_t * q)
{
	FILE *fp = (FILE *) handle->priv;

	fprintf (fp, "%-11s%-11s%-11s%-7s%-7s%-7s%-7s%-7s%-11s\n%-80s\n\n",
		q[DATE], q[GMT], q[CALL], q[BAND], q[MODE], q[POWER],
		q[RST], q[MYRST], q[GMTEND], q[REMARKS]);
	return 0;
}


static gchar *
find_field_end (gchar * p)
{
	while (*p && *p != ' ')
		p++;
	if (*p == ' ')
		while (*(p + 1) && *(p + 1) == ' ')
	p++;

	return p;
}

gint
twlog_qso_foreach (LOGDB * handle,
	gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	FILE *fp = (FILE *) handle->priv;
	gint i, ret, rlen;
	qso_t q[QSO_FIELDS];
	gchar buffer[81], *field, *end, *result;

	while (!feof (fp))
	{
	result = fgets (buffer, 13, fp);
	if (strlen (buffer) != 12 || feof (fp))
	{
		return 0;
	}
	memset (q, 0, sizeof (q));

	q[DATE] = g_strdup (g_strstrip (buffer));

	result = fgets (buffer, 80, fp);

	field = buffer;
	end = find_field_end (field);
	if (*end)
	*end++ = '\0';
	q[GMT] = g_strdup (g_strstrip (field));
	field = end;

	/* skip useless (optional) GMT keyword */
	if (!strncmp (field, "GMT", 3))
	{
		end = find_field_end (field);
		if (*end)
			*end++ = '\0';
		field = end;
	}

	/* from CALL to GMTEND */
	for (i = 2; i < 9; i++)
	{
		end = find_field_end (field);
		if (*end)
			*end++ = '\0';
		q[twlog_fields[i]] = g_strdup (g_strstrip (field));
		field = end;
	}
	/* ignore "GMT" keyword on last column */

	/* does not exist in twlog. TODO: select a default? */
	q[QSLOUT] = g_strdup ("");
	q[QSLIN] = g_strdup ("");

	/* line with remarks */
	result = fgets (buffer, 81, fp);

	rlen = strlen (buffer);
	q[REMARKS] = g_strdup (g_strstrip (buffer));

	if (rlen == 80 && buffer[79] != '\n')
	{
		/* old twlog format, with space padded remarks */
		result = fgets (buffer, 3, fp);
	}

	/* (supposed)empty line */
	result = fgets (buffer, 80, fp);

	for (i = 0; i < twlog_field_nr; i++)
	if (!q[twlog_fields[i]])
		q[twlog_fields[i]] = g_strdup ("");

	ret = (*fn) (handle, q, arg);
	if (ret)
	return ret;
	}
	return 0;
}
