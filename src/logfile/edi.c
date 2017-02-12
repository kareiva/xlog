/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2015 Andy Stewart <kb1oiq@arrl.net>
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
 * edi.c - scanner for EDI (Electronic Data Interchange) IARU Region 1
 *	from version 1.1 of 06-21-1995
 *
 * Reports to Stephane Fillod F8CFE
 * Specifications (in French) from http://www.ref-union.org
 *
 * Also, a specification was found here:
 * http://www.ari.it/index.php?option=com_content&view=article&id=451%3Astandard-format-for-e-contest-log&Itemid=150
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <ctype.h>
#include <glib/gstdio.h>

#include "logfile.h"
#include "../utils.h"
#include "../cfg.h"
#include "../wwl.h"

#ifndef HAVE_STRPTIME
#include "../strptime.h"
#define strptime(s,f,t) mystrptime(s,f,t)
#endif

extern preferencestype preferences;

/*
 * fields to be stored in the edi file
 *
 * BAND and POWER are extra fields, derived from the header
 */
static const gint edi_fields[] =
	{ DATE, GMT, CALL, MODE, RST, MYRST, LOCATOR, BAND, POWER };

static const gint edi_widths[] = { 6, 4, 14, 1, 3, 3, 6, 4, 3 };
static const gint edi_field_nr = 9;

static gint edi_open (LOGDB *);
static void edi_close (LOGDB *);
static gint edi_create (LOGDB *);
static gint edi_qso_append (LOGDB *, const qso_t *);
static gint edi_qso_foreach (LOGDB *,
	gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);

const struct log_ops edi_ops = {
	.open = edi_open,
	.close = edi_close,
	.create = edi_create,
	.qso_append = edi_qso_append,
	.qso_foreach = edi_qso_foreach,
	.type = TYPE_EDI,
	.name = "EDI",
	.extension = ".edi",
};

/*
 * open for read
 */
gint
edi_open (LOGDB * handle)
{
	FILE *fp;

	fp = g_fopen (handle->path, "r");
	if (!fp)
		return -1;
	handle->priv = (gpointer) fp;

	handle->column_nr = edi_field_nr;
	memcpy (handle->column_fields, edi_fields, sizeof (edi_fields));
	memcpy (handle->column_widths, edi_widths, sizeof (edi_widths));

	return 0;
}

/*
 * open for write
 */
gint
edi_create (LOGDB * handle)
{
	FILE *fp;
	gint num_qsos;

	fp = g_fopen (handle->path, "w");
	if (!fp)
		return -1;
	handle->priv = (gpointer) fp;

	/* write header */
	num_qsos = get_num_qsos_to_export();
	fprintf (fp, "[REG1TEST;1]\r\n"
		"TName=\r\n"
		"TDate=\r\n"
		"PCall=%s\r\n"
		"PWWLo=%s\r\n"
		"PExch=\r\n"
		"RName=\r\n"
		"RAdr1=\r\n"
		"RAdr2=\r\n"
		"RPoCo=\r\n"
		"RCity=\r\n"
		"RCoun=\r\n"
		"RHBBS=\r\n"
		"PBand=\r\n"
		"PSect=\r\n"
		"MOpe1=\r\n"
		"STXEq=\r\n"
		"SRXEq=\r\n"
		"SPowe=\r\n"
		"SAnte=\r\n"
		"CQSOs=\r\n"
		"CQSOP=\r\n"
		"CWWLs=\r\n"
		"CODXC=\r\n"
		"LSoft=" PACKAGE " version " VERSION "\r\n"
		"[Remarks]\r\n"
		"[QSORecords;%0d]\r\n",
		preferences.callsign,
		preferences.locator,
		num_qsos);
	return 0;
}

void
edi_close (LOGDB * handle)
{
	FILE *fp = (FILE *) handle->priv;

	fclose (fp);
}


gint
edi_qso_append (LOGDB * handle, const qso_t * q)
{
	FILE *fp = (FILE *) handle->priv;
	gchar rst[16], exch[16] = "", my_rst[16], my_exch[16] = "";
	gchar *p;
	gchar *temp;
	struct tm tm_a;
	gint rst_len;
	gint mode, kms, result = -1, l;

	/*
	 * there's no exchange fields in xlog. However, the exchange information
	 * may be piggybacked by the rst field. eg. "599 35".
	 */
	p = strchr (q[RST], ' ');
	rst_len = p ? p - q[RST] : strlen (q[RST]);

	strncpy (rst, q[RST], rst_len);
	rst[rst_len] = '\0';
	strncpy (my_rst, q[MYRST], rst_len);
	my_rst[rst_len] = '\0';
	if (strlen (q[RST]) > rst_len)
		strcpy (exch, q[RST] + rst_len + 1);
	if (strlen (q[MYRST]) > rst_len)
		strcpy (my_exch, q[MYRST] + rst_len + 1);

	/* YYMMDD */
	gchar date[32];

	temp = strptime (q[DATE], "%d %b %Y", &tm_a);
	if (temp) {
	  strftime (date, sizeof(date), "%y%m%d", &tm_a);
	}
	else {
	  date[0] = '\0';
	}

	if (!q[MODE] || !strcmp(q[MODE], "SSB") || !strcmp(q[MODE], "USB")) {
		mode = 1;
	} else if (!strcmp(q[MODE], "CW")) {
		mode = 2;
	} else if (!strcmp(q[MODE], "AM")) {
		mode = 5;
	} else if (!strcmp(q[MODE], "FM")) {
		mode = 6;
	} else if (!strcmp(q[MODE], "RTTY")) {
		mode = 7;
	} else if (!strcmp(q[MODE], "SSTV")) {
		mode = 8;
	} else if (!strcmp(q[MODE], "ATV")) {
		mode = 9;
	} else {
		mode = 0;
	}

	if (q[LOCATOR]) 
		result = locatordistance (preferences.locator, q[LOCATOR], &kms, &l);
	else 
		kms = 0;
	if (result != 0) kms = 0;

	/* 
	 * CSV-alike format 
	 *
	 * We're missing the points and zone/km column, 
	 * hence the empty semicolon
	 */
//020907;1404;PI4GN;1;59;001;59;005;;JO33II;515;;;;
	fprintf (fp, "%s;%s;%s;%d;%s;%s;%s;%s;;%s;%d;;;;\r\n",
		date, q[GMT], q[CALL], mode, rst, exch, my_rst, my_exch,
		q[LOCATOR] ? q[LOCATOR]:"", kms);

	return 0;
}

#define MAXROWLEN 120

gint
edi_qso_foreach (LOGDB * handle,
	gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	FILE *fp = (FILE *) handle->priv;
	gint ret, i;
	qso_t q[QSO_FIELDS];
	gchar *field;
	gchar buffer[MAXROWLEN];
	gchar band[MAXROWLEN]="";
	gchar power[MAXROWLEN]="";
	gint field_cnt = 0;
	gint concat;
	gchar *p, *qfield;
	gint header = 1;

continue_loop:
	while (!feof (fp))
	{
	if (!fgets (buffer, MAXROWLEN - 1, fp))
		break;

	/* skip header */
	if (header)
	{
		if (!memcmp(buffer, "PBand=", 6))
		{
			/* strcpy and strip CR LF, stop before " [MG]Hz" */
			for (p = buffer+6, i=0; p[i] != '\0' && p[i] != 10 && p[i] != 13 && p[i] != ' ' && i<MAXROWLEN; i++)
				band[i] = p[i];
			band[i] = '\0';
			if (p[i+1] == 'G')
			{
				gchar *dpoint = strchr(band, ',');
				gint freq;
				freq = atoi(band)*1000;
				if (dpoint)
					freq += atoi(dpoint+1)*100;
				sprintf(band, "%u", freq);
			}
		}

		if (!memcmp(buffer, "SPowe=", 6))
		{
			/* strcpy and strip CR LF */
			for (p = buffer+6, i=0; p[i] != '\0' && p[i] != 10 && p[i] != 13; i++)
				power[i] = p[i];
			power[i] = '\0';
		}

		if (!memcmp(buffer, "[QSORecords;", 12))
			header = 0;
		continue;
	}

	memset (q, 0, sizeof (q));

	field = buffer;
	concat = 0;
	field_cnt = 0;
	for (p = buffer; *p && field_cnt < edi_field_nr-2; p++)
	{
		if (*p == ';')
		{
			/* concat rst with exchange number */
			if (!concat && (edi_fields[field_cnt] == RST || edi_fields[field_cnt] == MYRST))
			{
				*p = ' ';
				concat = 1;
				continue;
			}
			concat = 0;
			*p = '\0';

			if (edi_fields[field_cnt] == CALL && !strcmp(field, "ERROR"))
				goto continue_loop;

			if (edi_fields[field_cnt] == DATE)
			{
				struct tm timestruct;
				static gchar sdate[32];

				timestruct.tm_year = (field[0]-'0')*10 + (field[1]-'0');
				timestruct.tm_mon = (field[2]-'0')*10 + (field[3]-'0');
				timestruct.tm_mday = (field[4]-'0')*10 + (field[5]-'0');

				if (timestruct.tm_year < 70)
					timestruct.tm_year += 100;
				timestruct.tm_mon--;
				strftime (sdate, 31, "%d %b %Y", &timestruct);
				qfield = g_strdup (sdate);
			}
			else if (edi_fields[field_cnt] == MODE)
			{
				gchar *mode;
				gint mode_id;

				sscanf (field, "%i", &mode_id);
				switch(mode_id) {
					case 1: mode = "SSB"; break;
					case 2: mode = "CW"; break;
					case 5: mode = "AM"; break;
					case 6: mode = "FM"; break;
					case 7: mode = "RTTY"; break;
					case 8: mode = "SSTV"; break;
					case 9: mode = "ATV"; break;
					default: mode = "other";
				}
				qfield = g_strdup (mode);
			}
			else if (edi_fields[field_cnt] == MYRST)
			{
				qfield = g_strdup (field);
				/* eat up the next field */
				while (*++p != ';')
					if (!*p)
						break;
			}
			else
				qfield = g_strdup (field);

			q[edi_fields[field_cnt]] = qfield;
			field = p + 1;
			field_cnt++;
		}
	}

	q[BAND] = g_strdup (band);
	q[POWER] = g_strdup (power);

	/* fill in empty fields */
	for (i = 0; i < edi_field_nr; i++)
	{
		if (!q[edi_fields[i]])
			q[edi_fields[i]] = g_strdup ("");
	}

	ret = (*fn) (handle, q, arg);
	if (ret) return ret;
	}
	return 0;
}
