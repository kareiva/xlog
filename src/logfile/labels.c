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
 * labels.c - support for glabels, export some fields in a TAB separated way
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
#include "logfile.h"
#include "../cfg.h"
#include "../wwl.h"

#ifndef HAVE_STRPTIME
#include "strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

/*
 * file fields
 */
static gint labels_open (LOGDB *);
static void labels_close (LOGDB *);
static gint labels_create (LOGDB *);
static gint labels_qso_append (LOGDB *, const qso_t *);
static gint labels_qso_foreach (LOGDB *,
	gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);

extern preferencestype preferences;

const struct log_ops labels_ops = {
	.open = labels_open,
	.close = labels_close,
	.create = labels_create,
	.qso_append = labels_qso_append,
	.qso_foreach = labels_qso_foreach,
	.type = TYPE_LABELS,
	.name = "Labels",
	.extension = ".labels",
};

typedef struct {
	FILE *fp;
	gint groupbycallsign;
	gchar *prev_call;
} labels_priv_t;
/*
 * open for read
 */
gint
labels_open (LOGDB * handle)
{
	labels_priv_t *priv;

	priv = g_new0(labels_priv_t, 1);
	if (!priv)
		return -1;

	priv->fp = g_fopen (handle->path, "r");
	if (!priv->fp) {
		g_free(priv);
		return -1;
	}
	handle->priv = (gpointer) priv;
	priv->groupbycallsign = preferences.tsvgroupbycallsign;

	return 0;
}

/*
 * open for write
 */
gint
labels_create (LOGDB * handle)
{
	labels_priv_t *priv;

	priv = g_new0(labels_priv_t, 1);
	if (!priv)
		return -1;

	priv->fp = g_fopen (handle->path, "w");
	if (!priv->fp) {
		g_free(priv);
		return -1;
	}
	handle->priv = (gpointer) priv;
	priv->groupbycallsign = preferences.tsvgroupbycallsign;
	return 0;
}

void
labels_close (LOGDB * handle)
{
	labels_priv_t *priv = (labels_priv_t*)handle->priv;
	if (priv->prev_call) {
		g_free(priv->prev_call);
		priv->prev_call = NULL;
	}
	fprintf (priv->fp, "\n");
	fclose (priv->fp);
	g_free(priv);
}

gint
labels_qso_append (LOGDB * handle, const qso_t * q)
{
	labels_priv_t *priv = (labels_priv_t*)handle->priv;
	FILE *fp = priv->fp;
	gint kms, miles, l, result;
	gchar *res = NULL;
	struct tm tm_cab;
	gchar date[16];

	if (priv->prev_call && (strcmp(q[CALL],priv->prev_call) || --priv->groupbycallsign <= 0)) {
		fprintf (fp, "\n");
		priv->groupbycallsign = preferences.tsvgroupbycallsign;
	}
	if (priv->prev_call) {
		g_free(priv->prev_call);
		priv->prev_call = NULL;
	}
	priv->prev_call = g_strdup(q[CALL]);

	if (preferences.saveastsv2[0] == 1)
	{
	  if (q[DATE])
	  {
	    /* convert "dd <month> yyyy" (month in any language) to ISO 8601 YYYY-MM-DD */
	    setlocale (LC_TIME, "");
	    res = strptime (q[DATE], "%d %b %Y", &tm_cab);
	    setlocale (LC_TIME, "C");
	    if (res != NULL)
	    {
	      setlocale (LC_TIME, "");
	      strftime (date, sizeof(date), "%Y-%m-%d", &tm_cab);
	      setlocale (LC_TIME, "C");
	    }
	    else
	    {
	      strcpy (date, q[DATE]);
	    }

	    fprintf (fp, "%s\t", date);
	  }
	  else
	  {
	    fprintf (fp, "\t");
	  }
	}
	if (preferences.saveastsv2[1] == 1)
	{
		if (q[GMT]) fprintf (fp, "%s\t", q[GMT]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[2] == 1)
	{
		if (q[GMTEND]) fprintf (fp, "%s\t", q[GMTEND]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[3] == 1)
	{
		if (q[CALL]) fprintf (fp, "%s\t", q[CALL]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[4] == 1)
	{
		if (q[BAND]) fprintf (fp, "%s\t", q[BAND]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[5] == 1)
	{
		if (q[MODE]) fprintf (fp, "%s\t", q[MODE]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[6] == 1)
	{
		if (q[RST]) fprintf (fp, "%s\t", q[RST]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[7] == 1)
	{
		if (q[MYRST]) fprintf (fp, "%s\t", q[MYRST]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[8] == 1)
	{
		if (q[AWARDS]) fprintf (fp, "%s\t", q[AWARDS]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[9] == 1)
	{
		if (q[QSLOUT]) fprintf (fp, "%s\t", q[QSLOUT]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[10] == 1)
	{
		if (q[QSLIN]) fprintf (fp, "%s\t", q[QSLIN]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[11] == 1)
	{
		if (q[POWER]) fprintf (fp, "%s\t", q[POWER]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[12] == 1)
	{
		if (q[NAME]) fprintf (fp, "%s\t", q[NAME]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[13] == 1)
	{
		if (q[QTH]) fprintf (fp, "%s\t", q[QTH]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[14] == 1)
	{
		if (q[LOCATOR])
		{
			if (preferences.tsvcalc == 0) 
				fprintf (fp, "%s\t", q[LOCATOR]);
			else
			{
				result =  locatordistance (preferences.locator, q[LOCATOR], &kms, &l);
				if (result == 0)
				{
					if (preferences.units == 1)
						fprintf (fp, "%s\t%d\t%d\t", q[LOCATOR], kms, l);
					else
					{
						miles = (gint) (kms / 1.609);
						fprintf (fp, "%s\t%d\t%d\t", q[LOCATOR], miles, l);
					}
				}
				else
					fprintf (fp, "%s\t%d\t%d\t", q[LOCATOR], 0, 0);
			}
 		}
		else 
		{
			if (preferences.tsvcalc == 0) 
				fprintf (fp, "\t");
			else
				fprintf (fp, "\t\t\t");
		}
	}
	if (preferences.saveastsv2[15] == 1)
	{
		if (q[U1]) fprintf (fp, "%s\t", q[U1]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[16] == 1)
	{
		if (q[U2]) fprintf (fp, "%s\t", q[U2]); else fprintf (fp, "\t");
	}
	if (preferences.saveastsv2[17] == 1)
	{
		if (q[REMARKS]) fprintf (fp, "%s\t", q[REMARKS]); else fprintf (fp, "\t");
	}
	return 0;
}

gint labels_qso_foreach 
(LOGDB * handle, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	return 1;
}
