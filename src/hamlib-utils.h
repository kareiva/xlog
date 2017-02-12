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

void stop_hamlib (void);
gboolean start_hamlib (gint rigid, gchar *device, gint debugmode, gint timervalue);
GList *riglist_get_list (void);
GList *rig_get_list (void);
gint get_rigid (gchar * rig);
gchar *rigmode (gint mode);
void get_powerlevel(void);
void get_mode(void);
void get_frequency(void);
void set_frequency (long long freq);
void get_ptt(void);
void get_smeter(void);
void get_riginfo (void);
gint poll_riginfo (void);
void sethamlibwidgets (gint status, gboolean initsmeter);
GString *convert_frequency (void);
gboolean on_smeterdrawingarea_configure_event (GtkWidget * widget, GdkEventConfigure * event, gpointer user_data);
gboolean on_smeterdrawingarea_expose_event (GtkWidget * widget, GdkEventExpose * event, gpointer user_data);
void on_mainwindow_show (GtkWidget * widget, gpointer user_data);
