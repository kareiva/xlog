/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 Andy Stewart KB1OIQ <kb1oiq@arrl.net>
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
#include <ctype.h>
#include <time.h>

#include "logfile.h"

/*
 * fields to be stored in the flog file
 */
const gint qso_fields[] =
	{ DATE, GMT, CALL, BAND, MODE, RST, MYRST, AWARDS, QSLOUT, QSLIN, REMARKS };
const gint qso_widths[] = { 15, 8, 15, 15, 8, 15, 15, 15, 8, 8, 80 };
const gint qso_field_nr = 11;

extern const struct log_ops flog_ops;
extern const struct log_ops twlog_ops;
extern const struct log_ops adif2_ops;
extern const struct log_ops cabrillo3_ops;
extern const struct log_ops editest_ops;
extern const struct log_ops trlog_ops;
extern const struct log_ops edi_ops;
extern const struct log_ops oh1aa_ops;
extern const struct log_ops labels_ops;

LOGDB *
log_file_open (const gchar * path, enum log_file_type type)
{
	LOGDB *handle;

	handle = (LOGDB *) g_malloc (sizeof (LOGDB));
	if (!handle)
		return NULL;
	handle->type = type;
	switch (type)
	{
		case TYPE_FLOG:
			handle->ops = &flog_ops;
			break;
		case TYPE_TWLOG:
			handle->ops = &twlog_ops;
			break;
		case TYPE_ADIF2:
			handle->ops = &adif2_ops;
			break;
		case TYPE_CABRILLO3:
			handle->ops = &cabrillo3_ops;
			break;
		case TYPE_EDITEST:
			handle->ops = &editest_ops;
			break;
		case TYPE_EDI:
			handle->ops = &edi_ops;
			break;
		case TYPE_TRLOG:
			handle->ops = &trlog_ops;
			break;
		case TYPE_OH1AA:
			handle->ops = &oh1aa_ops;
			break;
		case TYPE_LABELS:
			handle->ops = &labels_ops;
			break;
		default:
			/* unsupported log file type */
			g_free (handle);
			return NULL;
	}
	handle->path = g_strdup (path);
	handle->column_nr = 0;
	if (handle->ops->open (handle))
	{
		g_free (handle->path);
		g_free (handle);
		handle = NULL;
	}
	return handle;
}

LOGDB *
log_file_create (const gchar * path, enum log_file_type type,
		 gint columns, const gint column_fields[],
		 const gint column_widths[])
{
	LOGDB *handle;

	handle = (LOGDB *) g_malloc (sizeof (LOGDB));
	if (!handle)
		return NULL;
	handle->type = type;
	switch (type)
	{
		case TYPE_FLOG:
			handle->ops = &flog_ops;
			break;
		case TYPE_TWLOG:
			handle->ops = &twlog_ops;
			break;
		case TYPE_ADIF2:
			handle->ops = &adif2_ops;
			break;
		case TYPE_CABRILLO3:
			handle->ops = &cabrillo3_ops;
			break;
		case TYPE_EDITEST:
			handle->ops = &editest_ops;
			break;
		case TYPE_EDI:
			handle->ops = &edi_ops;
			break;
		case TYPE_TRLOG:
			handle->ops = &trlog_ops;
			break;
		case TYPE_OH1AA:
			handle->ops = &oh1aa_ops;
			break;
		case TYPE_LABELS:
			handle->ops = &labels_ops;
			break;
		default:
			/* unsupported log file type */
			g_free (handle);
			return NULL;
	}
	handle->path = g_strdup (path);
	handle->column_nr = columns;
	memcpy (handle->column_fields, column_fields, columns * sizeof (gint));
	memcpy (handle->column_widths, column_widths, columns * sizeof (gint));

	if (handle->ops->create (handle))
	{
		g_free (handle->path);
		g_free (handle);
		handle = NULL;
	}
	return handle;
}

void
log_file_close (LOGDB * handle)
{
	handle->ops->close (handle);
	g_free (handle->path);
	g_free (handle);
}

gint
log_file_qso_append (LOGDB * handle, const qso_t * q)
{
	return handle->ops->qso_append (handle, q);
}

gint
log_file_qso_foreach (LOGDB * handle,
	gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg)
{
	return handle->ops->qso_foreach (handle, fn, arg);
}

#if 0
/* this is for live surgery */
gint
log_file_add_column (LOGDB * handle, gint pos, gint field, gint width)
{
	g_array_insert_val (handle->column_widths, pos, width);
	g_array_insert_val (handle->column_fields, pos, field);
	return handle->ops->add_column (handle, pos, field, width);
}

gint
log_file_delete_column (LOGDB * handle, gint pos)
{
	g_array_remove_index (handle->column_widths, pos);
	g_array_remove_index (handle->column_fields, pos);
	return handle->ops->delete_column (handle, pos);
}
#endif


/*
 * flog fields names
 */
static const gchar *field_strings[] = {
	"NR", "DATE", "GMT", "GMTEND", "CALL", "BAND", "MODE", "RST", "MYRST", "AWARDS",
	"QSLOUT", "QSLIN", "POWER", "NAME", "QTH", "LOCATOR", "U1", "U2", "REMARKS"
};

static const gchar *column_strings[] = {
	"NR", "DATE", "UTC", "UTCEND", "CALL", "BAND", "MODE", "RST", "MYRST", "AWARDS",
	"QSLOUT", "QSLIN", "POWER", "NAME", "QTH", "LOCATOR", "U1", "U2", "REMARKS"
};

static const gint field_widths[] = {
	8, 15, 8, 8, 15, 15, 8, 15, 15, 30, 8, 8, 8, 30, 30, 8, 30, 30, 80
};

gint
parse_field_name (const gchar * s)
{
	gint i;

	for (i = 0; i < QSO_FIELDS; i++)
	{
		if (!strcmp (s, field_strings[i]))
			return i;
	}
	return -1;
}

gint
parse_column_name (const gchar * s)
{
	gint i;

	for (i = 0; i < QSO_FIELDS; i++)
	{
		if (!strcmp (s, column_strings[i]))
			return i;
	}
	return -1;
}

gint
parse_field_width (const gint field)
{
	/* better know what you're doing */
	if (field >= QSO_FIELDS || field < 0)
		return 0;

	return field_widths[field];
}

const gchar *
strfield (gint field)
{
	/* better know what you're doing */
	if (field >= QSO_FIELDS || field < 0)
		return NULL;

	return field_strings[field];
}

/* starting at xlog-1.5 we use UTC in logwindow */
const gchar *
strcolumn (gint field)
{
	/* better know what you're doing */
	if (field >= QSO_FIELDS || field < 0)
		return NULL;

	return column_strings[field];
}

size_t strftime (char *s, size_t max, const char *format,
	 const struct tm * tm);

gint
scan_month (const gchar * s)
{
	int i;
	static gchar month_abv[12][6] =
		{ "", "", "", "", "", "", "", "", "", "", "", "" };
	static struct tm timestruct =
		{ 0, 0, 0, 1, 0, 0, 0, 0, 0 };

	/*
	 * get locale month abreviations
	 */
	if (strlen(month_abv[0]) == 0)
	{
		for (i = 0; i < 12; i++)
		{
			timestruct.tm_mon = i;
			strftime (month_abv[i], sizeof(month_abv[i]), "%b", &timestruct);
		}
	}
	for (i = 0; i < 12; i++)
	{
		if (!strncasecmp (s, month_abv[i], 3))
		return i + 1;
	}

	/* fall back to english abreviations */
	switch (tolower (s[0]))
	{
		case 'j':
			if (s[1] == 'a')
				return 1;
			else if (s[2] == 'n')
				return 6;
			else
				return 7;
		case 'f':
			return 2;
		case 'm':
			if (s[2] == 'r')
				return 3;
			else
				return 5;
		case 'a':
			if (s[2] == 'r')
				return 4;
			else
				return 8;
		case 's':
			return 9;
		case 'o':
			return 10;
		case 'n':
			return 11;
		case 'd':
			return 12;
		default:
			return 0;
	}
}
