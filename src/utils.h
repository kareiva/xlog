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
 * utils.h
 */

#ifdef G_OS_WIN32
int win32_fork (char *prog);
int myqrb(double lon1, double lat1, double lon2, double lat2, double *distance, double *azimuth);
#endif
void open_url (const char *link);
void makebandoptionmenu (gchar *bands);
void makemodeoptionmenu (gchar *modes);
void deletespaces (gchar ** split);
void update_statusbar (gchar * string);
gboolean remove_prefixes (gchar * key, gpointer value, gpointer user_data);
void save_windowsize_and_cleanup (void);
gchar *xloggetdate (void);
gchar *xloggettime (void);
gchar *lookup_mode (gint index);
gchar *lookup_band (gint index);
gint updateclock (void);
gchar *color_parse (gchar * value);
gint autosave (void);
void set_qsoframe (gpointer arg);
gchar *my_strreplace(const char *str, const char *delimiter, const char *replacement);
gchar *valid_awards_entry (gchar *a, guint *st, guint *zone, guint *cont, guint *iota);
gboolean qslreceived(gchar *in);

/* used only for EDI export (so far) */
gint get_num_qsos_to_export(void);
void save_num_qsos_to_export(gint num);
