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
 * callbacks_preferencesdialog.h
 */

void set_autosave (gint value, gint saving);
void set_path (gchar * pathstr);
void set_backuppath (gchar * pathstr);
void set_logstoload (gchar * logs);
void set_qthlocator (gchar * locator);
void set_callsign (gchar * callsign);
void set_clock (gboolean on);
gint whichhamlibwidgets (gboolean frequency, gboolean smeter);
gboolean hamlib_changed (gint hamlibwidgets, gint rigid, gchar *device, 
	gint polltime, gchar *rigconf);
void on_backupradiobutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data);
void on_bandsradiobutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data);
void on_modesradiobutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data);
void on_hamlibcheckbutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data);
void on_autosaveradiobutton_toggled(GtkToggleButton * togglebutton,
	gpointer user_data);
void on_pollingcheckbutton_toggled (GtkToggleButton *togglebutton,
	gpointer user_data);
void on_radiobutton_clicked (GtkButton * button, gpointer user_data);
