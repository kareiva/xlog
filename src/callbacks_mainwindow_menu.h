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
 * callbacks_mainwindow_menu.h
 */

void on_menu_log_activate (GtkAction *action, gpointer user_data);
void on_menu_pagesetup_activate (GtkAction *action, gpointer user_data);
void on_menu_exit_activate (GtkMenuItem * menuitem, gpointer user_data);
void on_menu_save_activate (GtkMenuItem * menuitem, gpointer user_data);
void on_clearframe_activate (GtkMenuItem * menuitem, gpointer user_data);
void on_addclickall_activate (GtkMenuItem * menuitem, gpointer user_data);
void on_clickall_activate (GtkMenuItem * menuitem, gpointer user_data);
void on_view_toolbar_activate (GtkAction *action, gpointer user_data);
void on_sort_log_activate (GtkMenuItem *menuitem, gpointer user_data);
