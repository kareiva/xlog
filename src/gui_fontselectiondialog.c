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
 * gui_fontselectiondialog.c - creation and destruction of a font selection dialog
 *
 * this is used by the preferences dialog. We display a font selection dialog
 * and when OK is clicked we update the fields in the preferencesdialog and update
 * the example frame with the new font.
 */
 
#include <gtk/gtk.h>

#include "gui_utils.h"
#include "support.h"
#include "cfg.h"

extern GtkWidget *preferencesdialog;
extern preferencestype preferences;

void
on_fontbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *fontselectiondialog, *fontentry;
	GdkPixbuf *fontselectiondialog_icon_pixbuf;
	gchar *font;
	gint response;

	fontselectiondialog = gtk_font_selection_dialog_new
		(_("xlog - select Font"));
	gtk_window_set_modal (GTK_WINDOW(fontselectiondialog), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(fontselectiondialog), TRUE);

	fontselectiondialog_icon_pixbuf = create_pixbuf ("xlog.png");
	if (fontselectiondialog_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (fontselectiondialog),
			fontselectiondialog_icon_pixbuf);
		g_object_unref (fontselectiondialog_icon_pixbuf);
	}
	gtk_widget_destroy (GTK_FONT_SELECTION_DIALOG
		(fontselectiondialog)->apply_button);
	gtk_font_selection_dialog_set_preview_text (GTK_FONT_SELECTION_DIALOG
		(fontselectiondialog), _("How about this font?"));
	gtk_font_selection_dialog_set_font_name 
		(GTK_FONT_SELECTION_DIALOG(fontselectiondialog), preferences.logfont);
	gtk_widget_show_all (fontselectiondialog);
	response = gtk_dialog_run (GTK_DIALOG(fontselectiondialog));
	if (response == GTK_RESPONSE_OK)
	{
		font = gtk_font_selection_dialog_get_font_name
			(GTK_FONT_SELECTION_DIALOG (fontselectiondialog));
		fontentry = lookup_widget (preferencesdialog, "fontentry");
		gtk_entry_set_text (GTK_ENTRY (fontentry), font);
	}
	gtk_widget_destroy (fontselectiondialog);
	gtk_widget_set_sensitive (preferencesdialog, 1);
}
