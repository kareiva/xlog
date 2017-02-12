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
 * callbacks_mainwindow_qsoframe.h
 */

void on_datebutton_clicked (GtkButton * button, gpointer user_data);
void on_gmtbutton_clicked (GtkButton * button, gpointer user_data);
void on_callentry_insert_text (GtkEditable * editable, gchar * new_text, gint new_text_length, gpointer position, gpointer user_data);
void on_callentry_changed (GtkEditable * editable, gpointer user_data);
gboolean on_callentry_unfocus (GtkEntry *callentry, GdkEventFocus *event, gpointer user_data);
void on_callbutton_clicked (GtkButton * button, gpointer user_data);
void on_awardsentry_insert_text (GtkEditable * editable, gchar * new_text, gint new_text_length, gpointer position, gpointer user_data);
void on_awardsentry_changed (GtkEditable * editable, gpointer user_data);
gboolean on_qsoframeeventbox_button_press_event (GtkWidget * widget, GdkEventButton * event, gpointer user_data);
gboolean on_bandoptionmenu_key_press_event (GtkWidget * widget, GdkEventKey * event, gpointer user_data);
gboolean on_modeoptionmenu_key_press_event (GtkWidget * widget, GdkEventKey * event, gpointer user_data);
void on_locatorentry_changed (GtkEditable * editable, gpointer user_data);
void on_mhzbutton_clicked (GtkButton * button, gpointer user_data);
void on_modebutton_clicked (GtkButton * button, gpointer user_data);
void on_rstbutton_clicked (GtkButton * button, gpointer user_data);
void on_powerbutton_clicked (GtkButton * button, gpointer user_data);
void on_endbutton_clicked (GtkButton * button, gpointer user_data);
void on_powerbutton_clicked (GtkButton * button, gpointer user_data);
void tv_changed (GtkTextBuffer * buffer, gpointer user_data);
void entry_mnemonic_activate (GtkWidget * entry, gboolean arg1, gpointer user_data);
void tv_mnemonic_activate (GtkWidget * tv, gboolean arg1, gpointer user_data);
void bandoptionactivate (GtkWidget * bandoptionmenu, gboolean arg1, gpointer user_data);
void modeoptionactivate (GtkWidget * modeoptionmenu, gboolean arg1, gpointer user_data);
