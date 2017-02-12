/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2015 Andy Stewart <kb1oiq@arrl.net>
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
 * cabrillo3.c - scanner for Cabrillo format - 3.0
 * Reports to Andy Stewart KB1OIQ
 * Specifications from http://www.kkn.net/~trey/cabrillo/
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <glib/gstdio.h>
#include <time.h>
#include <glib.h>
#include "logfile.h"
#include "../cfg.h"
#include "../utils.h"
#include "../main.h"
#include "../xlog_enum.h"

#ifndef HAVE_STRPTIME
#include "strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

extern preferencestype preferences;
extern programstatetype programstate;

/*
 * fields to be stored in the cabrillo file
 */
static const gint cabrillo_fields[] =
	{ BAND, MODE, DATE, GMT, RST, CALL, MYRST, REMARKS };

static const gint cabrillo3_widths[] = { 5, 2, 10, 4, 10, 13, 10 };
static const gint cabrillo3_ss_widths[] = { 5, 2, 10, 4, 13, 10, 13 };
static const gint cabrillo3_na_widths[] = { 5, 2, 10, 4, 20, 10, 21 };
static const gint cabrillo3_iota_widths[] = { 5, 2, 10, 4, 15, 13, 15 };
static const gint cabrillo3_field_nr = 7;

static gint cabrillo3_open (LOGDB *);
static void cabrillo3_close (LOGDB *);
static gint cabrillo3_create (LOGDB *);
static gint cabrillo3_qso_append (LOGDB *, const qso_t *);
static gint cabrillo3_qso_foreach (LOGDB *, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);

const struct log_ops cabrillo3_ops = {
	.open = cabrillo3_open,
	.close = cabrillo3_close,
	.create = cabrillo3_create,
	.qso_append = cabrillo3_qso_append,
	.qso_foreach = cabrillo3_qso_foreach,
	.type = TYPE_CABRILLO3,
	.name = "Cabrillo3",
	.extension = ".cbr",
};

/*
 * open for read
 */
gint
cabrillo3_open (LOGDB * handle)
{
	FILE *fp;
	static const gint xlog_fields [] = {DATE, GMT, CALL, BAND, MODE, RST, MYRST, AWARDS, REMARKS};

	fp = g_fopen (handle->path, "r");
	if (!fp)
		return -1;
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
cabrillo3_create (LOGDB * handle)
{
	FILE *fp;

	fp = g_fopen (handle->path, "w");
	if (!fp)
		return -1;
	handle->priv = (gpointer) fp;

	/* write header */
	fprintf (fp, "START-OF-LOG: 3.0\n"
		"CREATED-BY: " PACKAGE " Version " VERSION "\n"
		"CONTEST: \n"
		"LOCATION: \n"
		"CALLSIGN: \n"
		"CATEGORY-ASSISTED: \n"
		"CATEGORY-BAND: \n"
		"CATEGORY-MODE: \n"
		"CATEGORY-OPERATOR: \n"
		"CATEGORY-POWER: \n"
		"CATEGORY-STATION: \n"
		"CATEGORY-TIME: \n"
		"CATEGORY-TRANSMITTER: \n"
		"CATEGORY-OVERLAY: \n"
		"CLAIMED-SCORE: \n"
		"CLUB: \n"
		"NAME: \n"
		"ADDRESS: \n"
		"ADDRESS-CITY: \n"
		"ADDRESS-STATE-PROVINCE: \n"
		"ADDRESS-POSTALCODE: \n"
		"ADDRESS-COUNTRY: \n"
		"EMAIL: \n"
		"OPERATORS: \n"
		"OFFTIME: \n"
		"SOAPBOX: \n"
		"SOAPBOX: \n");
	return 0;
}

void
cabrillo3_close (LOGDB * handle)
{
	FILE *fp = (FILE *) handle->priv;

	/* will fail silently if file was open read-only */
	fprintf (fp, "END-OF-LOG:\n");

	fclose (fp);
}

/*
 * append a qso. NOTE: preferences.callsign contains the operator's call.
 */
gint
cabrillo3_qso_append (LOGDB * handle, const qso_t * q)
{
	FILE *fp = (FILE *) handle->priv;
	gchar rst[16], exch[16] = "", my_rst[16], my_exch[16] = "", date[16];
	const gchar *mode;
	gint rst_len;
	gchar *q_mode = q[MODE] ? q[MODE] : "SSB";
	gchar *freq;
	gchar *res = NULL;
	struct tm tm_cab;

	/*
	 * there's no exchange fields in xlog. However, the exchange information
	 * may be piggybacked by the rst field. eg. "599ON".
	 */
	if (!strcmp (q_mode, "SSB") || !strcmp (q_mode, "USB")
			|| !strcmp (q_mode, "LSB") || !strcmp (q_mode, "FM"))
		rst_len = 2;
	else
		rst_len = 3;

	strncpy (rst, q[RST], rst_len);
	rst[rst_len] = '\0';
	strncpy (my_rst, q[MYRST], rst_len);
	my_rst[rst_len] = '\0';

	if (strlen (q[RST]) > rst_len)
		strcpy (exch, q[RST] + rst_len);
	if (strlen (q[MYRST]) > rst_len)
		strcpy (my_exch, q[MYRST] + rst_len);

	gint bandenum = freq2enum (q[BAND]);

	if (preferences.saveascabrillo == 1) {
	  freq = band_enum2cabrillochar (bandenum); /* Save just the band */
	} else {
	  /* Save the freq in khz for some bands */
	  /* punt for the other bands */
	  if ((bandenum >= BAND_160) && (bandenum <= BAND_10)) {
	    freq = freq2khz (q[BAND]);
	  } else {
	    freq = band_enum2cabrillochar (bandenum);
	  }
	}

	/* convert "dd <month> yyyy" (month in any language) to dd-mm-yyyy */
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

	/* translate mode, valid: PH, RY, FM, CW */
	if (!strcmp (q_mode, "USB") || !strcmp (q_mode, "LSB") || !strcmp (q_mode, "SSB"))
		{
			mode = "PH";
		}
	else if (!strcmp (q_mode, "RTTY"))
		{
			mode = "RY";
		}
	else if (!strcmp (q_mode, "FM") || !strcmp (q_mode, "CW"))
		{
			mode = q_mode;
		}

	// The PSK* modes are not present in the "official"(?) Cabrillo specification
	// http://www.kkn.net/~trey/cabrillo/qso-template.html
	// These translations are used in European contests (and maybe others???)
	// Added at the request of Alex (EW1LN) who showed descriptions of these
	// modes from 3 European contest sites.

	else if (!strcmp (q_mode, "PSK63"))
		{
			mode = "PM";
		}
	else if (!strcmp (q_mode, "PSK31"))
		{
			mode = "PS";
		}
	else if (!strcmp (q_mode, "PSK125"))
		{
			mode = "PO";
		}
	else
		mode = "  ";

	fprintf (fp, "QSO: %5s %-3s%-11s%-5s%-14s%-4s%-7s%-14s%-4s%-7s\n",
		freq, mode, date, q[GMT],
		preferences.callsign, rst, exch, q[CALL], my_rst, my_exch);
	g_free (freq);
	return 0;
}

#define MAXROWLEN 120

enum cbr_contest_type
{
	CBR_OTHER,
	CBR_SWEEPSTAKES,
	CBR_NA,
	CBR_IOTA
};

gint
cabrillo3_qso_foreach (LOGDB * handle,
	gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	FILE *fp = (FILE *) handle->priv;
	gint i, ret;
	qso_t q[QSO_FIELDS];
	gchar *field, *end, buffer[MAXROWLEN], buf[20], *res = NULL;
	enum cbr_contest_type contest_type = CBR_OTHER;
	const gint *widths = cabrillo3_widths;
	gint sent_call_len = 14;
	struct tm tm_cab;
	gboolean phone;
	gint awards_was = 0;

	while (!feof (fp))
	{
		if (!fgets (buffer, MAXROWLEN - 1, fp))
			break;

		phone = FALSE;
			/* valid record starts with "QSO: " */
		if (strncmp (buffer, "QSO: ", 5))
		{

			/* okay, this is not a QSO record. look at the Contest field
			* to deduce the log format
			*/
			if (!strncmp (buffer, "CONTEST: ", 9))
			{
				if (!strncmp (buffer + 9, "NA", 2))
				{
					contest_type = CBR_NA;
					widths = cabrillo3_na_widths;
					sent_call_len = 11;
					awards_was = 1;
				}
				else if (!strncmp (buffer + 9, "ARRL-SS", 7))
				{
					contest_type = CBR_SWEEPSTAKES;
					widths = cabrillo3_ss_widths;
					sent_call_len = 11;
					awards_was = 1;
				}
				else if (!strncmp (buffer + 9, "ARRL", 4))
				{
					awards_was = 1;
				}
				else if (!strncmp (buffer + 9, "RSGB-IOTA", 9))
				{
					contest_type = CBR_IOTA;
					widths = cabrillo3_iota_widths;
					sent_call_len = 14;
				}
			}
			else if (!strncmp (buffer, "END-OF-LOG:", 11))
			{
				break;
			}
			continue;
		}

		memset (q, 0, sizeof (q));
		field = buffer + 5;

		for (i = 0; i < cabrillo3_field_nr; i++)
		{
			/* hop the fifth field, this is the operator call sign */
			if (i == 4)
				field += sent_call_len;

			end = field + widths[i];
			*end = '\0';

			switch (i)
			{
			case 0:
				if (!strchr(field, 'G') && (
					!strncmp(field, " 1", 2) ||
					!strncmp(field, " 3", 2) ||
					!strncmp(field, " 7", 2) ||
					(!strncmp(field, "14", 2) && field[2]!='4') ||
					!strncmp(field, "21", 2) ||
					!strncmp(field, "28", 2)))
				{
					gint khz = atoi(field);
					if (khz%1000 == 0)
						q[cabrillo_fields[i]] =
							g_strdup_printf ("%d", khz/1000);
					else
						q[cabrillo_fields[i]] =
							g_strdup_printf ("%d.%03d", khz/1000, khz%1000);
				}
				else
				{
					q[cabrillo_fields[i]] = g_strdup (g_strstrip (field));
				}
				break;

			case 1:
				/* check the different mode strings and store mode */
				if (!strcmp (field, "PH"))
				{
					phone = TRUE;
					strcpy (buf, "SSB");
				}
				else if (!strcmp (field, "RY"))
					strcpy (buf, "RTTY");
				else
					strcpy (buf, field);
				q[cabrillo_fields[i]] = g_strdup (buf);
				break;

			case 2:
				/* reformat date 2007-01-23 -> 23 Jan 2007 */
				res = strptime (field, "%Y-%m-%d", &tm_cab);
				if (res != NULL)
				{
					setlocale (LC_TIME, "");
					strftime (buf, 20, "%d %b %Y", &tm_cab);
					setlocale (LC_TIME, "C");
				}
				else
					strcpy (buf, field);
				q[cabrillo_fields[i]] = g_strdup (buf);
				break;

			case 4:
			case 6:
				/* cut IOTA ref */
				if (contest_type == CBR_IOTA)
					field[8] = '\0';

				/* TODO: add prepending zero's if serial */
				if (phone)
				{
					strcpy (buf, field + 2);
					g_strstrip (buf);
					field[2] = '\0';
				}
				else
				{
					strcpy (buf, field + 3);
					g_strstrip (buf);
					field[3] = '\0';
				}

				q[cabrillo_fields[i]] =
					g_strdup_printf ("%s%s", field, buf);

				if (i == 6 && contest_type == CBR_IOTA &&
						strcmp(field+9, "------"))
				{
					field[9+6] = '\0';
					g_strstrip (field+9);
					q[AWARDS] = g_strdup_printf ("IOTA-%s", field+9);
				}
				else if (i == 6 && awards_was && isupper(field[4]) &&
						strcmp(field+4, "DX"))
				{
					g_strstrip (field+4);
					q[AWARDS] = g_strdup_printf ("WAS-%s", field+4);
				}
				/* TODO: WAZ for CONTEST:CQ-WW* ? */

				/* add the remark field if present */
				if ((i == 6) && (strlen(programstate.importremark) > 0))
				{
					q[cabrillo_fields[i + 1]] =
						g_strdup (programstate.importremark);
				}
				break;

			default:
				q[cabrillo_fields[i]] = g_strdup (g_strstrip (field));
			}

			field = end + 1;
		}

		ret = (*fn) (handle, q, arg);
		if (ret) return ret;
	}
	return 0;
}
