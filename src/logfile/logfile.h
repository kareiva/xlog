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

/*
 * logfile.h
 */

#ifndef _LOGFILE_H
#define _LOGFILE_H 1

#include <glib.h>

#define FLOG_MAX_LINE_SIZE 300
#define MAX_VARIABLE_LEN 512
#define MAX_COLUMN 32
#define QSO_FIELDS 19

/* fields for the qso array */
#define NR 0
#define DATE 1
#define GMT 2
#define GMTEND 3
#define CALL 4
#define BAND 5
#define MODE 6
#define RST 7
#define MYRST 8
#define AWARDS 9
#define QSLOUT 10
#define QSLIN 11
#define POWER 12
#define NAME 13
#define QTH 14
#define LOCATOR 15
#define U1 16
#define U2 17
#define REMARKS 18

extern const gint qso_fields[];
extern const gint qso_widths[];
extern const gint qso_field_nr;
typedef gchar *qso_t;

/* Forward struct references */

struct log_file;
typedef struct log_file LOGDB;

/* Keep enums in same order as menu in callbacks_menu.c */
enum log_file_type
{
	TYPE_FLOG,	/* flexible xlog format */
	TYPE_TWLOG,
	TYPE_ADIF2,     /* ADIF version 2 */
	TYPE_CABRILLO3, /* Cabrillo v3 */
	TYPE_EDITEST,	/* French contest logger */
	TYPE_TRLOG,
	TYPE_EDI,	/* Electronic Data Interchange, IARU Region 1 */
	TYPE_OH1AA,     /* OH1AA logbook */
	TYPE_LABELS,    /* Tab separated export of some fields for glabels */
};

/*
 * Here we go again, OOP in C. hi.
 */
struct log_ops
{
	gint (*open) (LOGDB *);
	void (*close) (LOGDB *);
	gint (*create) (LOGDB *);
	gint (*add_column) (LOGDB *, gint pos, gint field, gint width);
	gint (*delete_column) (LOGDB *, gint pos);
	gint (*qso_append) (LOGDB *, const qso_t *);
	gint (*qso_foreach) (LOGDB *, gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);
	enum log_file_type type;
	const char *name;
	const char *extension;
};

struct log_file
{
	enum log_file_type type;
	const struct log_ops *ops;
	gchar *path;
	gpointer priv;		/* for use by the log backend */
	gint column_nr;
	gint column_fields[MAX_COLUMN];
	gint column_widths[MAX_COLUMN];
};

LOGDB *log_file_open (const gchar * path, enum log_file_type type);
LOGDB *log_file_create (const gchar * path, enum log_file_type type,
		gint columns, const gint column_field[], const gint column_width[]);
void log_file_close (LOGDB *);
gint log_file_add_column (LOGDB *, gint pos, gint field, gint width);
gint log_file_delete_column (LOGDB *, gint pos);
gint log_file_qso_append (LOGDB *, const qso_t *);
gint log_file_qso_foreach (LOGDB *,
		gint (*fn) (LOGDB *, qso_t *, gpointer arg), gpointer arg);
/*
	qso_t* get_qso_from_nr(nr)
	update_qso(nr, qso_t)
	delete_qso(nr)
 */

gint parse_field_name (const gchar * s);
gint parse_column_name (const gchar * s);
gint parse_field_width (const gint field);
const gchar *strfield (gint field);
const gchar *strcolumn (gint field);
gint scan_month (const gchar * s);

#endif /* _LOGFILE_H */
