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
 * dxcc.h
 */
#ifndef DXCC_H
#define DXCC_H

/* struct for dxcc information from cty.dat */
typedef struct
{
	gchar *countryname;
	guchar cq;              /* guchar max=255 */
	guchar itu;
	guchar continent;
	gint latitude;
	gint longitude;
	gshort timezone;
	gchar *px;
	gchar *exceptions;
	guint worked;
	guint confirmed;
}
dxcc_data;

/* struct for dxcc information from area.dat */
typedef struct
{
	gchar *countryname;
	guchar cq;              /* guchar max=255 */
	guchar itu;
	gchar *continent;
	gint latitude;
	gint longitude;
	gshort timezone;
	gchar *px;
}
area_data;

struct info
{
	guint country;
	guint cq;
	guint itu;
	guint continent;
};

void cleanup_dxcc (void);
void cleanup_area (void);
gint readctyversion (void);
gint readctydata (void);
gint readareadata (void);
void updatedxccframe (gchar * item, gboolean byprefix, gint st, gint zone, gint cont, guint iota);
void update_dxccscoring (void);
void update_wacscoring (void);
void update_wasscoring (void);
void update_wazscoring (void);
void update_iotascoring (void);
void update_locscoring (void);
void fill_scoring_arrays (void);
struct info lookupcountry_by_callsign (gchar * callsign);
struct info lookupcountry_by_prefix (gchar * px);

void hash_inc(GHashTable *hash_table, const gchar *key);
void hash_dec(GHashTable *hash_table, const gchar *key);
void iota_new_qso(guint iota, gint f, gboolean qslconfirmed);
void iota_del_qso(guint iota, gint f, gboolean qslconfirmed);
void loc_new_qso(const gchar *locator, gint f, gboolean qslconfirmed);
void loc_del_qso(const gchar *locator, gint f, gboolean qslconfirmed);
gchar *loc_norm(const gchar *locator);

#endif	/* DXCC_H */
