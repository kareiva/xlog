/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
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
 * trlog.c - support for TRLOG(TLF) flat file format
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
const gint trlog_fields[] = { BAND, MODE, DATE, GMT, RST /* exch */, 
	CALL, RST, MYRST, MYRST /* exch */, REMARKS };
const gint trlog_widths[] = { 3, 3, 9, 5, 5, 14, 4, 4, 4, 80 };
const gint trlog_field_nr = 10;

static gint trlog_open (LOGDB *);
static void trlog_close (LOGDB *);
static gint trlog_create (LOGDB *);
static gint trlog_qso_append (LOGDB *, const qso_t *);
static gint trlog_qso_foreach (LOGDB *, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);

const struct log_ops trlog_ops = {
	.open = trlog_open,
	.close = trlog_close,
	.create = trlog_create,
	.qso_append = trlog_qso_append,
	.qso_foreach = trlog_qso_foreach,
	.type = TYPE_TRLOG,
	.name = "TRLog",
	.extension = ".log",
};

/*
 * open for read
 */
gint
trlog_open (LOGDB * handle)
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

/*
 * open for write
 */
gint
trlog_create (LOGDB * handle)
{
	FILE *fp;

	fp = g_fopen (handle->path, "w");
	if (!fp) return -1;
	handle->priv = (gpointer) fp;
	return 0;
}

void
trlog_close (LOGDB * handle)
{
	FILE *fp = (FILE *) handle->priv;
	fclose (fp);
}

/* 
 * The save method is unsupported and fully broken,
 * esp. the exchanges
 */
gint
trlog_qso_append (LOGDB * handle, const qso_t * q)
{
	FILE *fp = (FILE *) handle->priv;
	gchar *exch = "", *rxexch = "";
	gchar gmt[6];
	
	if (q[GMT][2] == ':')
		strcpy(gmt, q[GMT]);
	else
	{
		gmt[0] = q[GMT][0];
		gmt[1] = q[GMT][1];
		gmt[2] = ':';
		gmt[3] = q[GMT][2];
		gmt[4] = q[GMT][3];
		gmt[5] = '\0';
	}

// 80CW	21-Feb-04 23:41 0053	VY2LZ					599	599	PEI					 PEI			3	

	fprintf (fp, "%3s%-3s%9s %5s %-4s	 %-15s%-5s%-5s%-7s\n",
		q[BAND], q[MODE], q[DATE], gmt, exch, q[CALL], q[RST], q[MYRST], rxexch);
	return 0;
}

#define MAXROWLEN 100

gint trlog_qso_foreach 
(LOGDB * handle, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	FILE *fp = (FILE *) handle->priv;
	gint i, ret;
	qso_t q[QSO_FIELDS];
	gchar *field, *end, buffer[MAXROWLEN+1], *d, *res = NULL, buf[20], *remark,
		**split, rembuffer[MAXROWLEN+1];
	const gint *widths = trlog_widths;
	struct tm tm_trlog;

	/* set environment for trlog import */
	setlocale (LC_TIME, "C");

	while (!feof (fp))
	{
		remark = g_strdup ("");
		if (!fgets (buffer, MAXROWLEN - 1, fp)) break;

		/* sanity check */
		if (strlen(buffer) < 80) continue;
		/* skip header of trlog */
		if ((buffer[1] == 'B') || (buffer[1] == '-')) continue;

		/* check of there is a comment on the next line and read it */
		if (fgets (rembuffer, MAXROWLEN - 1, fp))
		{
			if (rembuffer[0] == ';')
			{
				if  (g_strrstr (rembuffer, ":"))
				{
					split = g_strsplit (rembuffer, ":", 2);
					remark = g_strdup (g_strstrip(split[1]));
					g_strfreev (split);
				}
				else
				{
					split = g_strsplit (rembuffer, ";", 2);
					remark = g_strdup (g_strstrip(split[1]));
					g_strfreev (split);
				}
			}
			/* go back */
			fseek (fp, -81, SEEK_CUR);
		}

		memset (q, 0, sizeof (q));
		q[trlog_fields[9]] = g_strdup (remark);
		g_free (remark);
		field = buffer;

		/* comment line */
		if (buffer[0] == ';')
			continue;

		/* insert a space between band and mode */
		memmove (buffer+4, buffer+3, MAXROWLEN-4);
		buffer[3] = ' ';

		for (i = 0; i < trlog_field_nr - 1; i++)
		{

// 40CW  21-Aug-04 11:04 0001  OH1XX          599  599                OH       3   
			end = field + widths[i];
			*end = '\0';

			/* remove the ':' in the GMT field */
			if (trlog_fields[i] == GMT && field[2] == ':')
			{
				memmove(field+2, field+3, 2);
				field[4] = ' ';
			}

			/* exchange fields are appended to RST and MYRST fields */
			if (i == 8)
				q[trlog_fields[8]] = g_strdup_printf ("%s%s", q[trlog_fields[8]], g_strstrip (field));
			else if (i == 6)
				q[trlog_fields[6]] = g_strdup_printf ("%s%s", g_strstrip (field), q[trlog_fields[6]]);
			else if (i == 0)
			{ /* convert band to frequency */
				if (!strcmp(field, "160"))
					q[trlog_fields[i]] = g_strdup ("1.8");
				else if (!strcmp(field, " 80"))
					q[trlog_fields[i]] = g_strdup ("3.5");
				else if (!strcmp(field, " 60"))
					q[trlog_fields[i]] = g_strdup ("5.3");
				else if (!strcmp(field, " 40"))
					q[trlog_fields[i]] = g_strdup ("7");
				else if (!strcmp(field, " 20"))
					q[trlog_fields[i]] = g_strdup ("14");
				else if (!strcmp(field, " 15"))
					q[trlog_fields[i]] = g_strdup ("21");
				else if (!strcmp(field, " 10"))
					q[trlog_fields[i]] = g_strdup ("28");
				else if (!strcmp(field, " 30"))
					q[trlog_fields[i]] = g_strdup ("10");
				else if (!strcmp(field, " 17"))
					q[trlog_fields[i]] = g_strdup ("18");
				else if (!strcmp(field, " 12"))
					q[trlog_fields[i]] = g_strdup ("24");
				else if (!strcmp(field, " 6"))
					q[trlog_fields[i]] = g_strdup ("50");
				else
					q[trlog_fields[i]] = g_strdup (field);
			}
			else if (i == 2)
			{ /* convert date 21-Feb-04 -> 21 Feb 2004 */
				field[2] = ' ';
				field[6] = '\0';
				if (field[7] == '0' || field[7] == '1')
					d = g_strdup_printf ("%s 20%s", field, field + 7);
				else
					d = g_strdup_printf ("%s 19%s", field, field + 7);
				/* convert string to current locale */
				res = strptime (d, "%d %b %Y", &tm_trlog);
				if (res != NULL)
				{
					setlocale (LC_TIME, "");
					strftime (buf, 20, "%d %b %Y", &tm_trlog);
					q[trlog_fields[i]] = g_strdup (buf);
					setlocale (LC_TIME, "C");
				}
				else
					q[trlog_fields[i]] = g_strdup (d);
				g_free (d);
			}
			else
				q[trlog_fields[i]] = g_strdup (g_strstrip (field));
			field = end + 1;
		}
		/* restore locale */
		setlocale (LC_TIME, "");
		ret = (*fn) (handle, q, arg);
		if (ret) return ret;
	}
	setlocale (LC_TIME, "");
	return 0;
}
