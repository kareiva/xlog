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
 * callbacks_mainwindow.h
 */

gboolean on_mainwindow_delete_event (GtkWidget * widget, GdkEvent * event, gpointer user_data);
void on_mainnotebook_switch_page (GtkNotebook * notebook, GtkNotebookPage * page, gint page_num, gpointer user_data);
gboolean on_mainwindow_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data);
