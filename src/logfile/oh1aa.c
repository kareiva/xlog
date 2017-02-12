/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
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
 * oh1aa.c - support for oh1aa flat file format
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <glib.h>
#include <glib/gstdio.h>

#ifndef HAVE_STRPTIME
#include "../strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

#include "logfile.h"

/*
 * file fields
 */
const gint oh1aa_fields[] = { DATE, GMT, CALL, RST, MYRST, BAND, MODE, REMARKS };
const gint oh1aa_widths[] = { 6, 4, 12, 3, 3, 7, 4, 38 };
const gint oh1aa_field_nr = 8;

static gint oh1aa_open (LOGDB *);
static void oh1aa_close (LOGDB *);
static gint oh1aa_qso_foreach (LOGDB *, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);

const struct log_ops oh1aa_ops = {
	.open = oh1aa_open,
	.close = oh1aa_close,
	.qso_foreach = oh1aa_qso_foreach,
	.type = TYPE_OH1AA,
	.name = "oh1aa",
	.extension = ".log",
};

/*
 * open for read
 */
gint
oh1aa_open (LOGDB * handle)
{
	FILE *fp;
	const gint xlog_fields [] = {DATE, GMT, CALL, BAND, MODE, RST, MYRST, REMARKS};

	fp = g_fopen (handle->path, "r");
	if (!fp) return -1;
	handle->priv = (gpointer) fp;

	/* set columns to be used in xlog */
	handle->column_nr = 8;
	memcpy (handle->column_fields, xlog_fields, sizeof (xlog_fields));
	/* TODO: set and use handle->column_widths */
	return 0;
}

void
oh1aa_close (LOGDB * handle)
{
	FILE *fp = (FILE *) handle->priv;
	fclose (fp);
}


#define MAXROWLEN 80

gint oh1aa_qso_foreach 
(LOGDB * handle, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	FILE *fp = (FILE *) handle->priv;
	gint i, ret;
	qso_t q[QSO_FIELDS];
	gchar *field, *end, buffer[MAXROWLEN+1], *res = NULL, buf[20];
	const gint *widths = oh1aa_widths;
	struct tm tm_oh1aa;

	while (!feof (fp))
	{
		if (!fgets (buffer, MAXROWLEN - 1, fp)) break;

		memset (q, 0, sizeof (q));
		field = buffer;

//0502201751OK2BMA      59 59  28 MHzSSB pavel                                 0

		/* insert a space between date and time */
		memmove (buffer+7, buffer+6, MAXROWLEN-7);
		buffer[6] = ' ';
		/* insert a space between time and call */
		memmove (buffer+12, buffer+11, MAXROWLEN-12);
		buffer[11] = ' ';
		/* insert a space between call and myrst */
		memmove (buffer+24, buffer+23, MAXROWLEN-24);
		buffer[24] = ' ';
		/* insert a space between myrst and rst */
		memmove (buffer+28, buffer+27, MAXROWLEN-28);
		buffer[28] = ' ';
		/* insert a space between rst and band */
		memmove (buffer+32, buffer+31, MAXROWLEN-32);
		buffer[32] = ' ';
		/* insert a space between band and mode */
		memmove (buffer+40, buffer+39, MAXROWLEN-40);
		buffer[40] = ' ';
		/* insert a space between mode and remarks */
		memmove (buffer+45, buffer+44, MAXROWLEN-45);
		buffer[45] = ' ';

		for (i = 0; i < oh1aa_field_nr; i++)
		{
			end = field + widths[i];

			if (i == 5)
				*(end - 3) = '\0';
			else
				*end = '\0';

			if (i == 5 && !strncmp(field, "1.2", 3))
				field = g_strdup ("1296");

			if (i == 0)
			{
				/* set environment for oh1aa import */
				setlocale (LC_TIME, "C");
				res = strptime (field, "%y%m%d", &tm_oh1aa);
				/* back to current locale */
				if (res != NULL)
				{
					setlocale (LC_TIME, "");
					strftime (buf, 20, "%d %b %Y", &tm_oh1aa);
					q[oh1aa_fields[i]] = g_strdup (buf);
				}
				else
					q[oh1aa_fields[i]] = g_strdup (field);
			}
			else
				q[oh1aa_fields[i]] = g_strdup (g_strstrip (field));

			field = end + 1;
		}
		ret = (*fn) (handle, q, arg);
		if (ret) return ret;
	}
	return 0;
}
