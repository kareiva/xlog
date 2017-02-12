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

/* gui_searchdialog.c - creation and destruction of the search dialog
 *
 * we create a dialog where a string can be entered. If Find is clicked this
 * string is checked against the callsigns in all logs. If there is a partial
 * match, the QSO is selected. If there is no match, a warning is displayed.

 * The dialog will be kept open until Close is clicked.
 */
 
#include <gtk/gtk.h>
#include <string.h>

#include "gui_searchdialog.h"
#include "support.h"
#include "log.h"
#include "main.h"

extern programstatetype programstate;
extern GList *searchhistory;
extern GList *logwindowlist;
extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;

GtkWidget *searchdialog = NULL;

#define SEARCHHISTORY 10

static gchar *
compare_utf8 (const gchar *haystack, const gchar *needle)
{
	gchar *normalized_haystack = g_utf8_normalize
		(haystack, -1, G_NORMALIZE_DEFAULT_COMPOSE);
	gchar *normalized_needle = g_utf8_normalize
		(needle, -1, G_NORMALIZE_DEFAULT_COMPOSE);
	gchar *case_normalized_haystack = g_utf8_casefold (normalized_haystack, -1);
	gchar *case_normalized_needle = g_utf8_casefold (normalized_needle, -1);
	return strstr (case_normalized_haystack, case_normalized_needle);
}

static void
searchok (GtkButton *button, gpointer user_data)
{
	GtkWidget *searchresultdialog, *resultlabel, *searchcombo;
	gchar *searchstr, *data, *current, *nr, *labeltext, *column;
	gint i, j, len;
	gboolean valid, result = FALSE;
	GList *node;
	logtype *logwindow;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreePath *path;

	searchcombo = lookup_widget (searchdialog, "searchcombo");
	GtkWidget *callradiobutton = lookup_widget (searchdialog, "callradiobutton");
	GtkWidget *thisradiobutton = lookup_widget (searchdialog, "thisradiobutton");
	gboolean searchcall = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON(callradiobutton));
	gboolean thislog = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON(thisradiobutton));

	searchstr = gtk_editable_get_chars 
		(GTK_EDITABLE (GTK_BIN(searchcombo)->child), 0, -1);
	len = strlen (searchstr);

	if (len > 0) {
	node = g_list_find_custom (searchhistory, searchstr, (GCompareFunc) strcmp);
	if (!node)
	{
		searchhistory =	g_list_prepend (searchhistory, g_strdup(searchstr));
		gtk_combo_box_append_text (GTK_COMBO_BOX (searchcombo), searchstr);
	}
	if (g_list_length (searchhistory) > SEARCHHISTORY)
	{
		searchhistory = g_list_remove 
			(searchhistory, g_list_last (searchhistory)->data);
		gtk_combo_box_remove_text (GTK_COMBO_BOX (searchcombo), 0);
	}

	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		if (thislog)
		{
			gint page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
			logwindow = g_list_nth_data (logwindowlist, page);
			i = g_list_length (logwindowlist);
		}
		else
			logwindow = g_list_nth_data (logwindowlist, i);
		model = gtk_tree_view_get_model (GTK_TREE_VIEW(logwindow->treeview));
		valid = gtk_tree_model_get_iter_first (model, &iter);
		while (valid)
		{
			if (searchcall) {
			gtk_tree_model_get (model, &iter, CALL, &data, -1);
			if (compare_utf8 (data, searchstr))
			{
				/* construct an id for this search */
				current = g_strdup_printf ("%d", i);
				gtk_tree_model_get (model, &iter, NR, &nr, -1);
				current = g_strconcat (current, "/", nr, NULL);
				if (!g_strrstr (programstate.searchstr, current))
				{
					gtk_notebook_set_current_page
						(GTK_NOTEBOOK (mainnotebook), i);
					selection =	gtk_tree_view_get_selection
						(GTK_TREE_VIEW(logwindow->treeview));
					gtk_tree_selection_select_iter(selection, &iter);
					path = gtk_tree_model_get_path (model, &iter);
					gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW
						(logwindow->treeview), path, NULL, TRUE, 0.5, 0.0);
					gtk_tree_path_free (path);
					/* add id to the array */
					programstate.searchstr = g_strconcat
						(programstate.searchstr, current, ",", NULL);
					result = TRUE;
					break;
				}
				g_free (current);
			} }
			else {
			for (j = 1; j < QSO_FIELDS; j++)
			{
				GtkTreeViewColumn *col = gtk_tree_view_get_column
					(GTK_TREE_VIEW (logwindow->treeview), j);
				if (gtk_tree_view_column_get_visible (col))
				{
					gtk_tree_model_get (model, &iter, j, &data, -1);
					if (compare_utf8 (data, searchstr))
					{
						current = g_strdup_printf ("%d", i);
						gtk_tree_model_get (model, &iter, NR, &nr, -1);
						current = g_strconcat (current, "/", nr, NULL);
						column = g_strdup_printf ("%d", j);
						current = g_strconcat (current, "/", column, NULL);
						g_free (column);
						if (!g_strrstr (programstate.searchstr, current))
						{
							gtk_notebook_set_current_page
								(GTK_NOTEBOOK (mainnotebook), i);
							selection =	gtk_tree_view_get_selection
								(GTK_TREE_VIEW(logwindow->treeview));
							gtk_tree_selection_select_iter(selection, &iter);
							path = gtk_tree_model_get_path (model, &iter);
							gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW
								(logwindow->treeview), path, NULL, TRUE, 0.5, 0.0);
							gtk_tree_path_free (path);
							programstate.searchstr = g_strconcat
								(programstate.searchstr, current, ",", NULL);
							result = TRUE;
							break;
						}
						g_free (current);
					}
				}
			} 
			if (result) break;
			}
			valid = gtk_tree_model_iter_next (model, &iter);
		}
		if (result)	break;
	}

	if (!result)
	{
		programstate.searchstr = g_strdup ("");
		searchresultdialog = gtk_dialog_new_with_buttons
			(_("xlog - searchresult"),
			GTK_WINDOW(searchdialog), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
		labeltext =	g_strdup_printf (_("\'%s\' was not found"), searchstr);
		resultlabel = gtk_label_new (labeltext);
		g_free (labeltext);
		gtk_container_add (GTK_CONTAINER
			(GTK_DIALOG (searchresultdialog)->vbox), resultlabel);
		gtk_misc_set_padding (GTK_MISC (resultlabel), 30, 20);
		gtk_widget_show_all (searchresultdialog);
		gtk_dialog_run (GTK_DIALOG(searchresultdialog));
		gtk_editable_delete_text
			(GTK_EDITABLE (GTK_BIN(searchcombo)->child), 0, -1);
		gtk_widget_destroy (searchresultdialog);
		gtk_widget_grab_focus (searchcombo);
	} }
	g_free (searchstr);
}

/* search changed, reset saved state */
static void
on_searchcombo_changed (GtkEditable * editable, gpointer user_data)
{
	programstate.searchstr = g_strdup ("");
}

static void
searchcancel (GtkButton *button, gpointer user_data)
{
	programstate.searchstr = g_strdup ("");
	gtk_widget_destroy (searchdialog);
	gtk_window_present (GTK_WINDOW(mainwindow));
	searchdialog = NULL;
}

void
on_menu_search_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *vbox, *searchcombo, *cancel_button, *ok_button;
	GdkPixbuf *dialog_icon_pixbuf;
	gint i, num;
	gchar *s;

	if (searchdialog)
	{
		gtk_window_present (GTK_WINDOW(searchdialog));
		return;
	}

	searchdialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (searchdialog), _("xlog - search"));
	dialog_icon_pixbuf = create_pixbuf ("xlog.png");
	if (dialog_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (searchdialog), dialog_icon_pixbuf);
		g_object_unref (dialog_icon_pixbuf);
	}
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG (searchdialog)->vbox), vbox);

	GtkWidget *hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);
	GtkWidget *thisradiobutton = gtk_radio_button_new_with_label
		(NULL, _("Search this log"));
	gtk_box_pack_start (GTK_BOX (hbox), thisradiobutton, FALSE, FALSE, 0);
	GSList *logsradiobutton_group = gtk_radio_button_get_group
		(GTK_RADIO_BUTTON (thisradiobutton));
	GtkWidget *alllogsradiobutton = gtk_radio_button_new_with_label
		(logsradiobutton_group, _("Search all logs"));
	gtk_box_pack_start (GTK_BOX (hbox), alllogsradiobutton, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (alllogsradiobutton), TRUE);

	GtkWidget *hsep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);
	GtkWidget *callradiobutton = gtk_radio_button_new_with_label
		(NULL, _("Search callsign field"));
	gtk_box_pack_start (GTK_BOX (hbox), callradiobutton, FALSE, FALSE, 0);
	GSList *fieldsradiobutton_group = gtk_radio_button_get_group
		(GTK_RADIO_BUTTON (callradiobutton));
	GtkWidget *allfieldsradiobutton = gtk_radio_button_new_with_label
		(fieldsradiobutton_group, _("Search all fields"));
	gtk_box_pack_start (GTK_BOX (hbox), allfieldsradiobutton, FALSE, FALSE, 0);

	searchcombo = gtk_combo_box_entry_new_text ();
	gtk_entry_set_activates_default 
	  (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (searchcombo))), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), searchcombo, FALSE, FALSE, 0);

	g_signal_connect ((gpointer) searchcombo, "changed",
		G_CALLBACK (on_searchcombo_changed), NULL);

	/* Store pointers to all widgets, for use by lookup_widget(). */
	GLADE_HOOKUP_OBJECT (searchdialog, searchcombo, "searchcombo");
	GLADE_HOOKUP_OBJECT (searchdialog, callradiobutton, "callradiobutton");
	GLADE_HOOKUP_OBJECT (searchdialog, thisradiobutton, "thisradiobutton");

	if (searchhistory)
	{
		num = g_list_length (searchhistory);
		for (i = 0; i < num; i++)
		{
			s = g_list_nth_data (searchhistory, i);
			gtk_combo_box_prepend_text (GTK_COMBO_BOX (searchcombo), s);
		}
	}

	cancel_button = gtk_dialog_add_button
		(GTK_DIALOG(searchdialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL);
	ok_button = gtk_dialog_add_button
		(GTK_DIALOG(searchdialog), GTK_STOCK_FIND, GTK_RESPONSE_OK);

	g_signal_connect (G_OBJECT (ok_button), "clicked",
		G_CALLBACK (searchok), NULL);
	g_signal_connect (G_OBJECT (cancel_button), "clicked",
		G_CALLBACK (searchcancel), NULL);
	g_signal_connect (searchdialog, "delete_event",G_CALLBACK (searchcancel), NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (searchdialog), GTK_RESPONSE_OK);

	/* don't use gtk_dialog_run, so we can keep on using xlog */
	gtk_widget_grab_focus (searchcombo);
	gtk_widget_show_all (searchdialog);
}
