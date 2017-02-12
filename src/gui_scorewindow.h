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

/* gui_scorewindow.h */

#ifndef GUI_SCOREWINDOW_H
#define GUI_SCOREWINDOW_H

void on_scoring_activate (GtkAction *action, gpointer user_data);
GtkWidget *create_scorewindow (void);
void notebook_append_dxccpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_append_wacpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_append_waspage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_append_wazpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_append_iotapage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_append_locpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_remove_dxccpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_remove_wacpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_remove_waspage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_remove_wazpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_remove_iotapage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
void notebook_remove_locpage (GtkWidget *scorewindow, GtkWidget *awardsnotebook);
#endif	/* GUI_SCOREWINDOW_H */
