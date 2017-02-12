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
 * Specifications from http://www.adif.org/adif227.htm
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <locale.h>
#include <glib/gstdio.h>

#include "logfile.h"
#include "../cfg.h"
#include "../utils.h"
#include "../xlog_enum.h"

#ifndef HAVE_STRPTIME
#include "../strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

extern preferencestype preferences;

/*
 * fields to be stored in the adif file
 */
static const gint adif_fields[] = { DATE, GMT, CALL, BAND, MODE, RST, MYRST,
	QSLOUT, QSLIN, REMARKS, POWER, GMTEND, NAME, QTH, LOCATOR, U1, U2 };
static const gint adif_widths[]__attribute__((unused)) = { 15, 8, 15, 15, 8, 8, 8, 8, 8, 80, 8, 8, 8,
	8, 6, 8, 8 };
static const gint adif_field_nr = 17;

static LOGDB *adif_handle;
static gint (*adif_fn)(LOGDB*, qso_t*, gpointer);
static gpointer adif_arg;
static qso_t q[QSO_FIELDS];
static gint adif_field = -1;

static gint adif_open(LOGDB*);
static void adif_close(LOGDB*);
static gint adif_create(LOGDB*);
static gint adif_qso_append(LOGDB*, const qso_t*);
static gint adif_qso_foreach(LOGDB*, gint (*fn)(LOGDB*, qso_t*, gpointer), gpointer);
static const gchar *xlog2adif_name(gint);

const struct log_ops adif2_ops = {
	.open =		adif_open,
	.close =	adif_close,
	.create =	adif_create,
	.qso_append =	adif_qso_append,
	.qso_foreach =	adif_qso_foreach,
	.type =		TYPE_ADIF2,
	.name =		"ADIF2",
	.extension =	".adi",
};

static gint adif_qso_foreach
(LOGDB *handle,  gint (*fn)(LOGDB*, qso_t*, gpointer arg), gpointer arg)
{
	gchar line [1024], sdate[16], *buffer, *p, *tmp;
	gchar **adifline, **adifitem, **adifid;
	gint ret, items, i=0;
	struct tm timestruct;
	FILE *fp = (FILE *) handle->priv;
	adif_handle = handle;
	adif_fn = fn;
	adif_arg = arg;

	memset (q, 0, sizeof (q));
	buffer = g_strdup ("");
	while (!feof (fp))
	{
		if (!fgets (line, 1023, fp)) break;

		if (line[0] != '\n') /* ignore empty lines */
			buffer = g_strconcat (buffer, line, NULL);
		else continue;

		if (g_strrstr (line, "<EOH>") || g_strrstr (line, "<eoh>")) /* read past header */
			buffer = g_strdup ("");

		if (g_strrstr (line, "<EOR>") || g_strrstr (line, "<eor>")) /* read past one record */
		{
			/* let's do this simple, check if we have multiple '<' */
			buffer = my_strreplace (buffer, "\n", "");
			adifline = g_strsplit (buffer, "<", 0);
			for (items = 0;; items++)
				if (!adifline || adifline[items] == NULL) break;

			if (items > 2) /* we have a valid adif line*/
			{
				for (i = 0; i < items; i++)
				if (g_strrstr (adifline[i], ">")
						&& g_strrstr (adifline[i], ":"))
				/* valid adif item */
				{
					adifitem = g_strsplit (adifline[i], ">", 2);
					/* adifitem [0] contains 'id' and string length */
					adifid = g_strsplit (adifitem[0], ":", -1);
					/* that's all we need, now fill in the fields */
					if (!strcasecmp (adifid[0], "CALL"))
					{
						adif_field = CALL;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "QSO_DATE"))
					{
						sscanf(adifitem[1], "%4d%2d%2d", &timestruct.tm_year,
							&timestruct.tm_mon,	&timestruct.tm_mday);
						timestruct.tm_year -= 1900;
						timestruct.tm_mon--;
						strftime (sdate, 16, "%d %b %Y", &timestruct);
						adif_field = DATE;
						q[adif_field] = g_strdup (sdate);
					}
					else if (!strcasecmp (adifid[0], "TIME_ON"))
					{
						adif_field = GMT;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "TIME_OFF"))
					{
						adif_field = GMTEND;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "BAND"))
					{
						/* prefer FREQ over BAND */
						if (!q[BAND])
						{
							adif_field = BAND;
							guint enumband = meters2enum (adifitem[1]);
							q[adif_field] = band_enum2char (enumband);
						}
					}
					else if (!strcasecmp (adifid[0], "FREQ"))
					{
						/* prefer FREQ over BAND */
						if (q[BAND])
						{
							g_free(q[BAND]);
							q[BAND] = NULL;
						}
						adif_field = BAND;
						if ((p = g_strrstr(adifitem[1], ",")))
							*p = '.';
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "MODE"))
					{
						adif_field = MODE;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "TX_PWR"))
					{
						adif_field = POWER;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "RST_SENT"))
					{
						adif_field = RST;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "RST_RCVD"))
					{
						adif_field = MYRST;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "QSL_SENT"))
					{
						adif_field = QSLOUT;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "QSL_RCVD"))
					{
						adif_field = QSLIN;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "QSL_VIA"))
					{
						if (!strcasecmp (preferences.freefield1, "QSL_VIA"))
						{
							adif_field = U1;
							q[adif_field] =
								g_strndup (adifitem[1], atoi(adifid[1]));
						}
						else if (!strcasecmp (preferences.freefield2, "QSL_VIA"))
						{
							adif_field = U2;
							q[adif_field] =
								g_strndup (adifitem[1], atoi(adifid[1]));
						}
					}
					else if (!strcasecmp (adifid[0], "NAME"))
					{
						adif_field = NAME;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "QTH"))
					{
						adif_field = QTH;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "GRIDSQUARE"))
					{
						adif_field = LOCATOR;
						q[adif_field] =
							g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "COMMENT"))
					{
						adif_field = REMARKS;
						/* append comment to remarks */
						if (q[REMARKS])
						{
							tmp = g_strndup (adifitem[1], atoi(adifid[1]));
							q[adif_field] =
								g_strconcat (q[adif_field], ", ", tmp, NULL);
							g_free (tmp);
						}
						else
							q[adif_field] =
								g_strndup (adifitem[1], atoi(adifid[1]));
					}
					else if (!strcasecmp (adifid[0], "NOTES"))
					{
						adif_field = REMARKS;
						/* append notes to remarks */
						if (q[REMARKS])
						{
							tmp = g_strndup (adifitem[1], atoi(adifid[1]));
							q[adif_field] =
								g_strconcat (q[adif_field], ", ", tmp, NULL);
							g_free (tmp);
						}
						else
							q[adif_field] =
								g_strndup (adifitem[1], atoi(adifid[1]));

					}
					g_strfreev (adifid);
					g_strfreev (adifitem);
				}
			}
		g_strfreev (adifline);
		buffer = g_strdup ("");
		}
		if (adif_field != -1)
		{
			/* fill in empty fields */
			for (i = 0; i < adif_field_nr; i++)
				if ( !q[adif_fields[i]] ) q[adif_fields[i]] = g_strdup ("");
			ret = (*adif_fn)(adif_handle, q, adif_arg);
			adif_field = -1;
			memset(q, 0, sizeof(q));
			if (ret) return ret;
		}
	}
	g_free (buffer);
	return 0;
}

/*
 * open for read
 */
gint adif_open(LOGDB *handle)
{
	FILE *fp;
	const gint xlog_fields [] = {DATE, GMT, GMTEND, CALL, BAND, MODE, RST, MYRST,
		QSLOUT, QSLIN, POWER, NAME, QTH, LOCATOR, U1, U2, REMARKS};

	fp = g_fopen(handle->path, "r");
	if (!fp) return -1;
	handle->priv = (gpointer)fp;

	handle->column_nr = adif_field_nr;
	memcpy (handle->column_fields, xlog_fields, sizeof (xlog_fields));
	/* TODO: set and use handle->column_widths */

	return 0;
}

/*
 * open for write
 */
gint adif_create(LOGDB *handle)
{
	FILE *fp;
	time_t timet;
	gchar sdate[32];

	fp = g_fopen(handle->path, "w");
	if (!fp) return -1;
	handle->priv = (gpointer)fp;

	/* use C locale for date */
	setlocale (LC_TIME, "C");
	time(&timet);
	strftime (sdate, 32, "%d %b %Y %T", localtime(&timet));
	setlocale (LC_TIME, "");

	/* write header */
	fprintf(fp, "ADIF Export from " PACKAGE " Version " VERSION "\n"
				"Copyright (C) 2012 Andy Stewart <kb1oiq@arrl.net>\n"
				"Copyright (C) 2001-2010 Joop Stakenborg <pg4i@amsat.org>\n"
				"Internet:  http://savannah.nongnu.org/projects/Xlog\n\n"
				"Date of export: %s\n",
				sdate);


	if (preferences.callsign[0] != '\0') {
		fprintf(fp, "Callsign: %s\n", preferences.callsign);
	}
	if (preferences.locator[0] != '\0') {
		fprintf(fp, "Locator: %s\n", preferences.locator);
	}

	fprintf(fp, 		"\n<ADIF_VER:5>2.2.7\n"
				"<EOH>\n");
	return 0;
}

void adif_close(LOGDB *handle)
{
	FILE *fp = (FILE*)handle->priv;
	fclose(fp);
}

/*
 * 'field' is a constant defined by our scanner
 */
static const gchar *xlog2adif_name(gint fld)
{
	switch (fld) {
		case DATE: return "QSO_DATE";
		case GMT: return "TIME_ON";
		case GMTEND: return "TIME_OFF";
		case CALL: return "CALL";
		case BAND:
		{
			if (preferences.saveasadif == 0)
				return "FREQ";
			else
				return "BAND";
		}
		case MODE: return "MODE";
		case POWER: return "TX_PWR";
		case RST: return "RST_SENT";
		case MYRST: return "RST_RCVD";
		case QSLOUT: return "QSL_SENT";
		case QSLIN: return "QSL_RCVD";
		case REMARKS: return "COMMENT";
		case NAME: return "NAME";
		case QTH: return "QTH";
		case LOCATOR: return "GRIDSQUARE";
		case U1: return preferences.freefield1;
		case U2: return preferences.freefield2;
		default: return "UNKNOWN";
	}
}

gint adif_qso_append(LOGDB *handle, const qso_t *q)
{
	FILE *fp = (FILE *)handle->priv;
	int i, mode;
	struct tm tm_a;
	gchar *result;

	mode = mode2enum (q[MODE]);
	if (mode == -1) {
	  printf("ERROR: Illegal ADIF mode in adif_qso_append() with %s, mode %0d, callsign %s\n", q[MODE], mode, q[CALL]);
	  return 1;
	}

	for (i = 0; i < handle->column_nr; i++)
	{
		gint qfield_len, number;
		gint fld = handle->column_fields[i];
		gchar *qfield, *serial = NULL, *endptr = NULL;

		if (!q[fld])
			continue;
		qfield_len = strlen(q[fld]);
		if (qfield_len == 0)
			continue;

		/* reshape date, convert from locale notation to yyyymmdd */
		if (fld == DATE)
		{
			gchar date[32];

			result = strptime (q[DATE], "%d %b %Y", &tm_a);
			if (result)
			{
				strftime (date, sizeof(date), "%Y%m%d", &tm_a);
				qfield = date;
				qfield_len = 8;
			}
			else
			{
				qfield = q[fld];
				qfield_len = strlen(qfield);
			}
		}
		else if (fld == BAND)
		{
			if (preferences.saveasadif == 0)
				qfield = q[fld];
			else
			{
				gint bandenum = freq2enum (q[fld]);
				qfield = band_enum2bandchar (bandenum);
				qfield_len = strlen(qfield);
			}
		}

		/* export all QSL-info's as 'Y' for 'y', 'Y' or 'x', 
		 * drop entry for 'n' or 'N' and 
		 * as 'I' (invalid) for all other values of nonzero length
		 */
		else if (fld == QSLOUT || fld == QSLIN)
		{
			qfield = q[fld];	
			if (qfield_len == 1 && (qfield[0] == 'Y' || qfield[0] == 'y' || qfield[0] == 'X' || qfield[0] == 'x'))
				qfield[0] = 'Y';
			else if (qfield_len == 1 && (qfield[0] == 'N' || qfield[0] == 'n'))
				continue;
			else
			{
				qfield[0] = 'I';
				qfield[1] = '\0';
				qfield_len = 1;
			}
		}

		else if (fld == RST || fld == MYRST)
		{
			if (mode == MODE_SSB || mode == MODE_AM || mode == MODE_FM)
			{
				serial = g_strdup (g_strstrip(q[fld] + 2));
				q[fld][2] = '\0';
				qfield = q[fld];
				qfield_len = 2;
			}
			else
			{
				serial = g_strdup (g_strstrip(q[fld] + 3));
				q[fld][3] = '\0';
				qfield = q[fld];
				qfield_len = 3;
			}
		}

		else if (fld == MODE)
		{
			gint mode_enum = mode2enum (q[fld]);
			qfield = mode_enum2char (mode_enum);
			qfield_len = strlen(qfield);
		}

		else
		{
			qfield = q[fld];
		}

		fprintf(fp, "<%s:%d>%s ", xlog2adif_name(fld), qfield_len, qfield);

/* export exchange fields when present...
 *
 * NOTE: only digits will be converted to the designated SRX/STX fields,
 * if there is an ascii character present, we use SRX_STRING/STX_STRING
 * TODO: use endptr to split exchanges like 001UT or 001/322
 * */
		if ((fld == RST) && serial && (strlen(serial) > 0))
		{
			number = strtol (serial, &endptr, 10);
			if (strlen (endptr) > 0)
				fprintf(fp, "<STX_STRING:%zd>%s ", strlen(serial), serial);
			else
				fprintf(fp, "<STX:%zd>%s ", strlen(serial), serial);
		}
		if ((fld == MYRST) && serial && (strlen(serial) > 0))
		{
			number = strtol (serial, &endptr, 10);
			if (strlen (endptr) > 0)
				fprintf(fp, "<SRX_STRING:%zd>%s ", strlen(serial), serial);
			else
				fprintf(fp, "<SRX:%zd>%s ", strlen(serial), serial);
		}

	}
	fprintf(fp, "\n<EOR>\n");

	return 0;
}
