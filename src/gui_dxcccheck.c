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

/* gui_dxcccheck.c - check for unknown countries
 *
 */

#include <gtk/gtk.h>

#include "support.h"
#include "log.h"
#include "dxcc.h"
#include "gui_warningdialog.h"

extern GtkWidget *mainnotebook;
extern GList *logwindowlist;

GtkWidget *dxcccheckdialog = NULL;

static gboolean
on_dxcccheckwindow_delete_event (GtkWidget * widget, GdkEvent * event,
	gpointer user_data)
{
	dxcccheckdialog = NULL;
	return FALSE;
}

static void
on_dxccchecktreeview_select_row (GtkTreeSelection * selection,
	gpointer user_data)
{
	gchar *nr, *lognr, *logn;
	guint i = 0;
	logtype *logwindow = NULL;
	gboolean qsofound = FALSE, valid = FALSE;
	GtkTreeModel *model, *logmodel = NULL;
	GtkTreeIter iter, logiter;
	GtkTreeSelection *logselection;
	GtkTreePath *logpath;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &logn, -1);
		gtk_tree_model_get (model, &iter, NR + 1, &nr, -1);
		for (i = 0; i < g_list_length (logwindowlist); i++)
		{
			logwindow = g_list_nth_data (logwindowlist, i);
			logmodel = gtk_tree_view_get_model
				(GTK_TREE_VIEW(logwindow->treeview));
			valid = gtk_tree_model_get_iter_first (logmodel, &logiter);
			while (valid)
			{
				gtk_tree_model_get (logmodel, &logiter, NR, &lognr, -1);
				if ((g_ascii_strcasecmp (nr, lognr) == 0) &&
					(g_ascii_strcasecmp (logn, logwindow->logname) == 0))
				{
					qsofound = TRUE;
					break;
				}
				valid = gtk_tree_model_iter_next (logmodel, &logiter);
			}
			if (qsofound) break;
		}
		if (qsofound)
		{
			gtk_notebook_set_current_page (GTK_NOTEBOOK(mainnotebook), i);
			logselection = gtk_tree_view_get_selection
				(GTK_TREE_VIEW(logwindow->treeview));
			gtk_tree_selection_select_iter (logselection, &logiter);
			logpath = gtk_tree_model_get_path (logmodel, &logiter);
			gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(logwindow->treeview),
				logpath, NULL, TRUE, 0.5, 0.0);
			gtk_tree_path_free (logpath);
		}
	}
}

void
on_menu_dxcccheck_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (dxcccheckdialog)
	{
		gtk_window_present (GTK_WINDOW(dxcccheckdialog));
		return;
	}

	gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook));
	if (page >= 0) {
	gboolean foundunknown = FALSE;
	dxcccheckdialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title
		(GTK_WINDOW (dxcccheckdialog), _("xlog - dxcc check results"));
	gtk_window_set_default_size (GTK_WINDOW (dxcccheckdialog), 600, 300);
	GdkPixbuf *dxcccheckdialog_icon_pixbuf = create_pixbuf ("xlog.png");
	if (dxcccheckdialog_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (dxcccheckdialog),
		dxcccheckdialog_icon_pixbuf);
		g_object_unref (dxcccheckdialog_icon_pixbuf);
	}
	GtkWidget *dxcccheckdialogvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (dxcccheckdialog), dxcccheckdialogvbox);

	GtkWidget *dxccchecklabel = gtk_label_new
		(_("Select an entry to highlight the corresponding entry in the main window"));
	gtk_box_pack_start (GTK_BOX (dxcccheckdialogvbox), dxccchecklabel, FALSE, TRUE, 0);
	GtkWidget *dxcccheckhseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (dxcccheckdialogvbox), dxcccheckhseparator, FALSE, FALSE, 0);

	/* create a treeview */
	GtkWidget *dxcccheckscrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
		(dxcccheckscrolledwindow),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (dxcccheckdialogvbox),
		dxcccheckscrolledwindow, TRUE, TRUE, 0);
	GtkListStore *dxcccheckstore = gtk_list_store_new
		(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *dxccchecktreeview = gtk_tree_view_new_with_model
		(GTK_TREE_MODEL (dxcccheckstore));
	gtk_tree_sortable_set_sort_column_id
		(GTK_TREE_SORTABLE (dxcccheckstore), 4, GTK_SORT_ASCENDING);
	g_object_unref (G_OBJECT (dxcccheckstore));
	gtk_container_add (GTK_CONTAINER (dxcccheckscrolledwindow),
		dxccchecktreeview);

	/* add callback for selecting a row */
	GObject *selection = G_OBJECT (gtk_tree_view_get_selection
		(GTK_TREE_VIEW (dxccchecktreeview)));
	gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
		GTK_SELECTION_SINGLE);
	g_signal_connect (selection, "changed",
		G_CALLBACK (on_dxccchecktreeview_select_row), NULL);

	/* add columns */
	GtkCellRenderer *dxcccheckrenderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("LOG", dxcccheckrenderer, "text", 0, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("NR", dxcccheckrenderer, "text", 1, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("DATE", dxcccheckrenderer, "text", 2, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("UTC", dxcccheckrenderer, "text", 3, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("CALL", dxcccheckrenderer, "text", 4, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_sort_column_id (dxcccheckcolumn, 4);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("BAND", dxcccheckrenderer, "text", 5, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("MODE", dxcccheckrenderer, "text", 6, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);
	dxcccheckrenderer = gtk_cell_renderer_text_new ();
	dxcccheckcolumn = gtk_tree_view_column_new_with_attributes
		("AWARDS", dxcccheckrenderer, "text", 7, NULL);
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dxcccheckcolumn),
		GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_append_column
		(GTK_TREE_VIEW (dxccchecktreeview), dxcccheckcolumn);

	dxcccheckhseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (dxcccheckdialogvbox), dxcccheckhseparator, FALSE, FALSE, 0);
	dxccchecklabel = gtk_label_new
		(_("This window will not update when you modify QSO's"));
	gtk_box_pack_start (GTK_BOX (dxcccheckdialogvbox), dxccchecklabel, FALSE, TRUE, 0);

	gint pages = g_list_length (logwindowlist);
	logtype *logwindow;
	GtkTreeModel *logmodel;
	GtkTreeIter iter, checkiter;
	gboolean valid = FALSE;
	gchar *logname = NULL, *nr = NULL, *date = NULL, *gmt = NULL,
		*callsign = NULL, *band = NULL, *mode = NULL, *award = NULL;
	struct info dxcc_info;
	for (page=0; page<pages; page++)
	{
		logwindow = g_list_nth_data (logwindowlist, page);
		logmodel = gtk_tree_view_get_model (GTK_TREE_VIEW (logwindow->treeview));
		valid = gtk_tree_model_get_iter_first (logmodel, &iter);
		while (valid)
		{
			gtk_tree_model_get (logmodel, &iter, CALL, &callsign, -1);
			dxcc_info = lookupcountry_by_callsign (callsign);
			if (dxcc_info.country == 0)
			{
				foundunknown = TRUE;
				logname = g_strdup (logwindow->logname);
				gtk_tree_model_get (logmodel, &iter, NR, &nr, -1);
				gtk_tree_model_get (logmodel, &iter, DATE, &date, -1);
				gtk_tree_model_get (logmodel, &iter, GMT, &gmt, -1);
				gtk_tree_model_get (logmodel, &iter, BAND, &band, -1);
				gtk_tree_model_get (logmodel, &iter, MODE, &mode, -1);
				gtk_tree_model_get (logmodel, &iter, AWARDS, &award, -1);
				dxcccheckstore = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW (dxccchecktreeview)));
				gtk_list_store_prepend (GTK_LIST_STORE (dxcccheckstore), &checkiter);
				gtk_list_store_set (GTK_LIST_STORE (dxcccheckstore),
					&checkiter, 0, logname, 1, nr, 2, date, 3, gmt,
					4, callsign, 5, band, 6, mode, 7, award, -1);
			}
			valid = gtk_tree_model_iter_next (logmodel, &iter);
		}
	}
	if (logname) g_free (logname);
	if (nr) g_free (nr);
	if (date) g_free (date);
	if (gmt) g_free (gmt);
	if (callsign) g_free (callsign);
	if (band) g_free (band);
	if (mode) g_free (mode);
	if (award) g_free (award);
	
	g_signal_connect ((gpointer) dxcccheckdialog, "delete_event",
		G_CALLBACK (on_dxcccheckwindow_delete_event), NULL);

	if (foundunknown)
		gtk_widget_show_all (dxcccheckdialog);
	else
		warningdialog (_("xlog - DXCC check results"),
		_("No unknown countries found!"), "gtk-info"); }
}
