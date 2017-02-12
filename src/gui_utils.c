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
 * gui_utils.c - utilities for the user interface
 */

#include <gtk/gtk.h>
#include <string.h>

#include "cfg.h"
#include "log.h"
#include "support.h"
#include "xlog_enum.h"

extern GtkUIManager *ui_manager;
extern GtkWidget *mainnotebook;
extern GList *logwindowlist;
extern preferencestype preferences;
/* configure "Tabs" menu */
void set_tabs_menu (void)
{
	gchar *menupath;
	GtkWidget *menuitem;
	GList *children, *cur;
	logtype *logw;
	gint i;

	for (i = 1; i < 11; i++)
	{
		menupath = g_strdup_printf ("/MainMenu/TabsMenu/%d", i);
		menuitem = gtk_ui_manager_get_widget (ui_manager, menupath);
		gtk_widget_hide (menuitem);
		g_free (menupath);
	}
	for (i = 0; i < g_list_length(logwindowlist); i++)
	{
		if (i < 10)
		{
			menupath = g_strdup_printf ("/MainMenu/TabsMenu/%d", i + 1);
			menuitem = gtk_ui_manager_get_widget (ui_manager, menupath);
			gtk_widget_show (menuitem);
			children = gtk_container_get_children (GTK_CONTAINER (menuitem));
			cur = children;
			logw = g_list_nth_data (logwindowlist, i);
			while (cur)
			{
				if (GTK_IS_LABEL (cur->data))
					gtk_label_set_text (GTK_LABEL (cur->data), logw->logname);
				cur = cur->next;
			}
			g_free (menupath);
		}
	}
}

void unselect_logs (void)
{
	gint i;
	logtype *logw;
	GtkTreeIter selected;
	GtkTreeModel *model;
	GtkTreeSelection *sel;

	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		logw = g_list_nth_data (logwindowlist, i);
		sel = gtk_tree_view_get_selection(GTK_TREE_VIEW (logw->treeview));
		if (gtk_tree_selection_get_selected	(sel, &model, &selected))
			gtk_tree_selection_unselect_all (sel);
	}
}

/* set the log font */
void
set_font (gchar * font)
{
	PangoFontDescription *font_description;
	gint i;
	logtype *logw;

	font_description = pango_font_description_from_string (font);
	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		logw = g_list_nth_data (logwindowlist, i);
		gtk_widget_modify_font (GTK_WIDGET(logw->treeview), font_description);
	}
	pango_font_description_free (font_description);
	preferences.logfont = g_strdup (font);
}

/* activate an item in a combobox by band enumeration */
void
activate_bandoption_by_enum (GtkWidget *combo, gchar *prefs, guint enumband)
{
	gchar **spl = NULL;
	guint index = 0, prefsband;

	spl = g_strsplit (prefs, ",", 0);
	for (;;)
	{
		if (!spl[index])
			break;
		prefsband = freq2enum (spl[index]);
		if (prefsband == enumband)
		{
			gtk_combo_box_set_active (GTK_COMBO_BOX(combo), index);
			g_strfreev (spl);
			return;
		}
		index++;
	}
	if (spl) g_strfreev (spl);
}

/* activate an item in the mode combobox */
void
activate_modeoption_by_enum (GtkWidget *combo, gchar *prefs, guint enummode)
{
	gchar **spl = NULL;
	guint index = 0, prefsmode;

	spl = g_strsplit (prefs, ",", 0);
	for (;;)
	{
		if (!spl[index])
			break;
		prefsmode = mode2enum (spl[index]);
		if (prefsmode == enummode)
		{
			gtk_combo_box_set_active (GTK_COMBO_BOX(combo), index);
			g_strfreev (spl);
			return;
		}
		index++;
	}
	if (spl) g_strfreev (spl);
}

/*
 * return the last exchange message in the log. If none found then return
 * the initial exchange message. In case the mode field is empty return
 * some fallbacks depending on the reportlength
 */
gchar*
get_last_msg (void)
{
	GtkTreeIter iter;
	gchar *rst, *mode, *suffix;
	gchar *res = preferences.initlastmsg;

	guint page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
	/* check if there is a log open */
	if (page > -1)
	{
	logtype *logw = g_list_nth_data (logwindowlist, page);
	GtkListStore *model = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview)));
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter))
	{
		gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, MYRST, &rst, -1);
		gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, MODE, &mode, -1);
		if (strlen(mode) > 0)
		{
			guint modeenum = mode2enum (mode);
			suffix = rst + reportlen (modeenum);
		}
		else
		{
			if (g_strrstr (rst, "599"))
				suffix = rst + 3;
			else if (g_strrstr (rst, "59"))
				suffix = rst + 2;
			else
				suffix = rst;
		}
		if (*suffix) res = suffix;
	}
	}
	return g_strdup (res);
}
