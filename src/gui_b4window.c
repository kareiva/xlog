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

/* gui_b4window.c - creation and handling of the b4window
 *
 * this dialog will show a list of stations who match the characters entered
 * in the callsign field of the QSO frame. The number of columns displayed here
 * are assigned in the "dialogs dialog". Size and position of this dialog is
 * remembered when it is hidden. When a row in the dialog is selected, the
 * corresponding QSO in the log is selected.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "gui_b4window.h"
#include "support.h"
#include "log.h"
#include "utils.h"
#include "callbacks_mainwindow_qsoframe.h"
#include "xlog_enum.h"
#include "dxcc.h"

extern preferencestype preferences;
extern GList *logwindowlist;
extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;
extern GtkUIManager *ui_manager;

GtkWidget *b4window;

void
on_menu_worked_activate (GtkAction *action, gpointer user_data)
{
	gboolean status;

	status = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	if (status)
	{
		gtk_widget_show (b4window);
		gdk_window_move_resize (b4window->window,
			preferences.b4x, preferences.b4y, preferences.b4width, preferences.b4height);
	}
	else
	{
		gtk_window_get_size (GTK_WINDOW(b4window), &preferences.b4width, &preferences.b4height);
		gtk_window_get_position (GTK_WINDOW(b4window), &preferences.b4x, &preferences.b4y);
		gtk_widget_hide (b4window);
	}
}

static void
on_b4treeview_select_row (GtkTreeSelection * selection, gpointer user_data)
{
	gchar *nr, *lognr, *logn;
	guint i;
	logtype *logwindow = NULL;
	gboolean qsofound = FALSE, valid = FALSE;
	GtkTreeModel *model, *logmodel = NULL;
	GtkTreeIter iter, logiter;
	GtkTreeSelection *logselection;
	GtkTreePath *logpath;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		/* make sure the list is not re-ordered */
		gtk_tree_model_get (model, &iter, 0, &logn, -1);
		gtk_tree_model_get (model, &iter, NR + 1, &nr, -1);
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logwindow = g_list_nth_data (logwindowlist, i);
			logmodel = gtk_tree_view_get_model (GTK_TREE_VIEW(logwindow->treeview));
			valid = gtk_tree_model_get_iter_first (logmodel, &logiter);
			while (valid)
			{
				gtk_tree_model_get (logmodel, &logiter, NR, &lognr, -1);
				if ((g_ascii_strcasecmp (nr, lognr) == 0)
					&& (g_ascii_strcasecmp(logn, logwindow->logname) == 0))
				{
					qsofound = TRUE;
					break;
				}
				valid = gtk_tree_model_iter_next (logmodel, &logiter);
			}
			if (qsofound)
				break;
		}
		if (qsofound)
		{
			guint st, zone, cont, iota;
			GtkWidget *callentry = lookup_widget (mainwindow, "callentry"); 	 
			GtkWidget *awardsentry = lookup_widget (mainwindow, "awardsentry"); 	 
			g_signal_handlers_block_by_func (GTK_OBJECT (callentry), on_callentry_changed, user_data);
			g_signal_handlers_block_by_func (GTK_OBJECT (awardsentry), on_awardsentry_changed, user_data);
			gtk_notebook_set_current_page (GTK_NOTEBOOK(mainnotebook), i);
			logselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(logwindow-> treeview));
			logpath = gtk_tree_model_get_path (logmodel, &logiter);
			gtk_tree_view_set_cursor (GTK_TREE_VIEW(logwindow->treeview), logpath, NULL, FALSE);
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(logwindow->treeview), logpath, NULL, TRUE, 0.0, 0.0);
			gtk_tree_path_free (logpath);
			gchar *aw = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
			gchar *result = valid_awards_entry (aw, &st, &zone, &cont, &iota);
			if (!result)
			{
				gchar *call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
				updatedxccframe (call, FALSE, st, zone, cont, iota);
			}
			else
			g_free (result);
			g_signal_handlers_unblock_by_func (GTK_OBJECT (callentry), on_callentry_changed, user_data);
			g_signal_handlers_unblock_by_func (GTK_OBJECT (awardsentry), on_awardsentry_changed, user_data);
		}
	}

}

static gboolean
on_b4window_delete_event (GtkWidget * widget, GdkEvent * event,
				gpointer user_data)
{
	GtkWidget *worked;

	worked = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/OptionMenu/WorkedBefore");
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(worked), FALSE);
	gtk_window_get_size (GTK_WINDOW(widget), &preferences.b4width, &preferences.b4height);
	gtk_window_get_position (GTK_WINDOW(widget), &preferences.b4x, &preferences.b4y);
	gtk_widget_hide (widget);
	return TRUE;		/* do not destroy */
}

GtkWidget*
create_b4window (void)
{
	GtkWidget *b4scrolledwindow, *b4treeview, *b4windowvbox;
	GdkPixbuf *b4window_icon_pixbuf;
	GtkTreeViewColumn *column;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	GObject *selection;
	gint i, j;

	b4window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus (GTK_WINDOW(b4window), FALSE);
	gtk_window_set_title (GTK_WINDOW (b4window), _("xlog - worked before"));
	b4window_icon_pixbuf = create_pixbuf ("xlog-b4.png");
	if (b4window_icon_pixbuf)
		{
			gtk_window_set_icon (GTK_WINDOW (b4window), b4window_icon_pixbuf);
			g_object_unref (b4window_icon_pixbuf);
		}

	b4windowvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (b4window), b4windowvbox);

	g_signal_connect ((gpointer) b4window, "delete_event",
										G_CALLBACK (on_b4window_delete_event),
										NULL);

	b4scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(b4scrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (b4windowvbox), b4scrolledwindow, TRUE, TRUE, 0);
	gtk_box_reorder_child (GTK_BOX (b4windowvbox), b4scrolledwindow, 0);
	model = gtk_list_store_new (QSO_FIELDS + 1,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	b4treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model),
		CALL + 1, GTK_SORT_ASCENDING);
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (b4treeview), -1);
	g_object_unref (G_OBJECT (model));
	gtk_container_add (GTK_CONTAINER (b4scrolledwindow), b4treeview);

	/* add callback for selecting a row */
	selection = G_OBJECT (gtk_tree_view_get_selection(GTK_TREE_VIEW (b4treeview)));
	gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);
	g_signal_connect (selection, "changed", G_CALLBACK (on_b4treeview_select_row), NULL);

	/* one extra column compared to the log */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("LOG", renderer, "text", 0, NULL);
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY );
	gtk_tree_view_append_column (GTK_TREE_VIEW (b4treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("NR", renderer, "text", NR + 1, NULL);
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY );
	gtk_tree_view_append_column (GTK_TREE_VIEW (b4treeview), column);

	for (j = 1; j < QSO_FIELDS; j++)
	{
		renderer = gtk_cell_renderer_text_new ();
		if (j == U1)
			column = gtk_tree_view_column_new_with_attributes (preferences.freefield1, renderer, "text", j + 1, NULL);
		else if (j == U2)
			column = gtk_tree_view_column_new_with_attributes (preferences.freefield2, renderer, "text", j + 1, NULL);
		else
			column = gtk_tree_view_column_new_with_attributes (strcolumn(j), renderer, "text", j + 1, NULL);
		gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN(column), GTK_TREE_VIEW_COLUMN_GROW_ONLY);
		if (j == CALL)
			gtk_tree_view_column_set_sort_column_id (column, CALL + 1);
		gtk_tree_view_append_column (GTK_TREE_VIEW (b4treeview), column);
	}

	/* for lookup_widget */
	g_object_set_data_full (G_OBJECT (b4window), "b4treeview",
		g_object_ref (b4treeview), (GDestroyNotify) g_object_unref);

	for (i = 0; i < 20; i++)
	{
		column = gtk_tree_view_get_column (GTK_TREE_VIEW(b4treeview), i);
		gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(column), preferences.b4columns2[i]==1 ? TRUE : FALSE);
	}
	gtk_widget_show_all (b4windowvbox);
	return b4window;
}
