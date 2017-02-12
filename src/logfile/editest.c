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
 * editest.c - support for .exp Editest file format (CSV like)
 *  Reports to Stephane Fillod F8CFE
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include <ctype.h>
#include <glib/gstdio.h>

#include "logfile.h"

/* 
 * Editest is a french contest logging program.
 * Files can be generated for 2 kinds of contest. 
 * Number of fields is identical for both.
 *
 * For HF:
 *	DATE GMT CALL RST MYRST NR BAND PTS ZONE
 * For VHF:
 *	DATE GMT CALL RST MYRST LOCATOR BAND PTS KM
 *
 * Note: there's no MODE field!
 */

/*
 * file fields
 */
const gint editest_fields[] = { DATE, GMT, CALL, RST, MYRST, LOCATOR, BAND };
const gint editest_widths[] = { 9, 5, 14, 8, 8, 9, 9, 5 };
const gint editest_field_nr = 7;

static gint editest_open (LOGDB *);
static void editest_close (LOGDB *);
static gint editest_create (LOGDB *);
static gint editest_qso_append (LOGDB *, const qso_t *);
static gint editest_qso_foreach (LOGDB *,
				 gint (*fn) (LOGDB *, qso_t *, gpointer arg),
				 gpointer arg);

const struct log_ops editest_ops = {
  .open = editest_open,
  .close = editest_close,
  .create = editest_create,
  .qso_append = editest_qso_append,
  .qso_foreach = editest_qso_foreach,
  .type = TYPE_EDITEST,
  .name = "Editest",
  .extension = ".exp",
};

/*
 * open for read
 */
gint
editest_open (LOGDB * handle)
{
  FILE *fp;

  fp = g_fopen (handle->path, "r");
  if (!fp)
    return -1;
  handle->priv = (gpointer) fp;

  handle->column_nr = editest_field_nr;
  memcpy (handle->column_fields, editest_fields, sizeof (editest_fields));
  memcpy (handle->column_widths, editest_widths, sizeof (editest_widths));

  return 0;
}

/*
 * open for write
 */
gint
editest_create (LOGDB * handle)
{
  FILE *fp;

  fp = g_fopen (handle->path, "w");
  if (!fp)
    return -1;
  handle->priv = (gpointer) fp;
  return 0;
}

void
editest_close (LOGDB * handle)
{
  FILE *fp = (FILE *) handle->priv;
  fclose (fp);
}

/* 
 */
gint
editest_qso_append (LOGDB * handle, const qso_t * q)
{
  FILE *fp = (FILE *) handle->priv;
  gchar rst[16], exch[16] = "", my_rst[16], my_exch[16] = "";
  gchar date[16], locator[16];
  gchar *p;
  gint rst_len;

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
    strcpy (exch, q[RST] + rst_len);
  if (strlen (q[MYRST]) > rst_len)
    strcpy (my_exch, q[MYRST] + rst_len);

//const gint editest_fields[] = { DATE, GMT, CALL, RST, MYRST, LOCATOR, BAND };

  if (strlen (q[DATE]) == 11 && q[DATE][2] == ' ' && q[DATE][6] == ' ')
    sprintf (date, "%c%c-%02u-%s", q[DATE][0], q[DATE][1],
	     scan_month (q[DATE] + 3), q[DATE] + 9);

  if (q[NR] && (!q[LOCATOR] || strlen (q[LOCATOR]) == 0))
    sprintf (locator, "%s", q[NR]);
  else
    strcpy (locator, q[LOCATOR] ? q[LOCATOR] : "");

  /* 
   * .exp CSV-like format 
   *
   * If there's no locator data, maybe it's a HF log
   *
   * We're missing the points and zone/km column, 
   * hence the empty semicolon
   *
   * TODO: compute these from location set in the preferences
   */

  fprintf (fp, "%s;%s;%s;%s;%s;%s;%s;%s;%s;;\n",
	   date, q[GMT], q[CALL], rst, exch, my_rst, my_exch,
	   locator, q[BAND]);

  return 0;
}

static const gchar * band2freq(const gchar *field)
{

	gint band;
	const gchar*fr;

	band = atoi(field);

	switch (band)
	{
		case 160: fr = "1.8"; break;
		case 80: fr = "3.5"; break;
		case 40: fr = "7"; break;
		case 20: fr = "14"; break;
		case 15: fr = "21"; break;
		case 10: fr = "28"; break;
		default: fr = field;
	}
	return fr;
}

#define MAXROWLEN 120

gint
editest_qso_foreach (LOGDB * handle,
		     gint (*fn) (LOGDB *, qso_t *, gpointer arg),
		     gpointer arg)
{
  FILE *fp = (FILE *) handle->priv;
  gint ret, i;
  qso_t q[QSO_FIELDS];
  gchar *field;
  gchar buffer[MAXROWLEN];
  gint field_cnt = 0;
  gint concat;
  gchar *p, *qfield;

  while (!feof (fp))
    {
      if (!fgets (buffer, MAXROWLEN - 1, fp))
	break;

      memset (q, 0, sizeof (q));

      field = buffer;
      concat = 0;
      field_cnt = 0;
      for (p = buffer; *p && field_cnt < editest_field_nr; p++)
	{
	  if (*p == ';')
	    {
	      /* concat rst with exchange number */
	      if (!concat && (field_cnt == 3 || field_cnt == 4))
		{
		  *p = ' ';
		  concat = 1;
		  continue;
		}
	      concat = 0;
	      *p = '\0';
	      switch (editest_fields[field_cnt]) {
              case DATE:
		{
		  struct tm timestruct;
		  gchar sdate[16];

		  memset(&timestruct, 0, sizeof(timestruct));
		  sscanf (field, "%u-%u-%u",
			  &timestruct.tm_mday,
			  &timestruct.tm_mon,
			  &timestruct.tm_year);
		  if (timestruct.tm_year < 70)
		    timestruct.tm_year += 100;
		  timestruct.tm_mon--;
		  strftime (sdate, 16, "%d %b %Y", &timestruct);
		  qfield = g_strdup (sdate);
		  break;
		}
              case GMT:
		{
		  gchar gmt[16];

                  if (strlen(field) > 2 && field[2] == ':') {
                    memcpy(gmt, field, 2);
                    /* suppress the colon */
                    strncpy(gmt+2, field+3, 14);
                  } else {
                    strncpy(gmt, field, 16);
                  }
		  qfield = g_strdup (gmt);
		  break;
		}
              case BAND:
		qfield = g_strdup (band2freq(field));
		break;
              case LOCATOR:
		/* HF logs don't have locator, but useless NR */
		qfield = g_strdup ( isalpha(field[0]) ? field : "");
		break;
	      default:
		qfield = g_strdup (field);
	      }

	      q[editest_fields[field_cnt]] = qfield;
	      field = p + 1;
	      field_cnt++;
	    }
	}

      /* fill in empty fields */
      for (i = 0; i < editest_field_nr; i++)
	{
	  if (!q[editest_fields[i]])
	    q[editest_fields[i]] = g_strdup ("");
	}

      ret = (*fn) (handle, q, arg);
      if (ret)
	return ret;
    }
  return 0;
}
