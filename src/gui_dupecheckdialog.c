/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2015        Andy Stewart <kb1oiq@arrl.net>
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
 * gui_dupecheckdialog.c - creation and destruction of the dupecheck dialog
 *
 * a dialog is created where you can select which logs to dupecheck and if
 * you want to ignore bands and/or modes used. Either a list of dupes is
 * displayed or a dialog which tells you there are no dupes. When you select
 * a QSO in the dupelist, the corresponding QSO in the log is selected.
 */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "gui_dupecheckdialog.h"
#include "support.h"
#include "log.h"
#include "xlog_enum.h"
#include "main.h"
#include "gui_warningdialog.h"

extern programstatetype programstate;
extern GtkWidget *mainnotebook;
extern GtkWidget *mainwindow;
extern GList *logwindowlist;

/* global used for dupecheck abort */
gboolean breakit;
GtkWidget *dupecheckdialog;

static void
on_dupelisttreeview_select_row (GtkTreeSelection * selection,
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
			if (qsofound)	break;
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
on_dupecheckcancelbutton_clicked (GtkButton *button, gpointer user_data)
{
	breakit = TRUE;
	while (gtk_events_pending ()) gtk_main_iteration ();
	gtk_widget_destroy (dupecheckdialog);
}

void
on_menu_dupecheck_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkWidget *dupecheckthisradiobutton,
		*dupecheckallradiobutton, *dupecheckbuttonmode, *dupecheckbuttonband,
		*vbox, *dupechecklabel, *dupecheckframe, *dupechecklogs_vbox,
		*dupecheckitems_label, *dupecheckinclude_frame, *dupecheckinclude_hbox,
		*dupecheckstatusbar_hbox, *dupecheckstatusbar, *dupecheckprogressbar,
		*dupelistdialog, *dupelistdialogvbox, *dupelistlabel,
		*dupelisthseparator, *dupelistscrolledwindow, *dupelisttreeview,
		*cancel_button;
	GdkPixbuf *dupelistdialog_icon_pixbuf;
	GList *entirelist = NULL;
	GList *uniqlist = NULL;
	GList *dupelist = NULL;
	GSList *duperadiobutton_group = NULL;
	logtype *logwindow;
	gint i, j, qso_search_num, num_uniq_dupes, savedpage, page, pages, row = 0, dupecheckresponse,
	  enumband, enummode, dupe_already_found, uniq;
	gchar *nr, *date, *gmt, *callsign, *band,
	  *mode, *temp = NULL, *str, *prog_str = NULL;
	gchar *temp_dupe = NULL;
	gchar **dupe = NULL;
	gboolean dupefound = FALSE, valid = FALSE;
	GtkTreeIter dupeiter, iter;
	GtkTreeModel *logmodel, *dupemodel;
	GtkListStore *dupestore;
	GtkCellRenderer *duperenderer;
	GtkTreeViewColumn *dupecolumn;
	GObject *selection;

	breakit = FALSE;
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook));
	if (page >= 0)
	{
		dupecheckdialog = gtk_dialog_new_with_buttons (_("xlog - dupecheck"),
			GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
		cancel_button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
		gtk_dialog_add_action_widget
			(GTK_DIALOG (dupecheckdialog), cancel_button, GTK_RESPONSE_CANCEL);
		vbox = gtk_vbox_new (FALSE, 10);
		gtk_container_add (GTK_CONTAINER
			(GTK_DIALOG (dupecheckdialog)->vbox), vbox);
		dupechecklabel = gtk_label_new (_("Select logs for dupe checking"));
		gtk_box_pack_start
			(GTK_BOX (vbox), dupechecklabel, FALSE, FALSE, 0);
		dupecheckframe = gtk_frame_new (NULL);
		gtk_box_pack_start
			(GTK_BOX (vbox), dupecheckframe, FALSE, FALSE, 0);
		dupechecklogs_vbox = gtk_vbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER (dupecheckframe), dupechecklogs_vbox);
		dupecheckthisradiobutton =
			gtk_radio_button_new_with_label (NULL, _("Check this log"));
		gtk_box_pack_start (GTK_BOX (dupechecklogs_vbox),
			dupecheckthisradiobutton, FALSE, FALSE, 0);
		duperadiobutton_group = gtk_radio_button_get_group
			(GTK_RADIO_BUTTON (dupecheckthisradiobutton));
		dupecheckallradiobutton = gtk_radio_button_new_with_label
			(duperadiobutton_group, _("Check all logs"));
		gtk_box_pack_start (GTK_BOX (dupechecklogs_vbox),
			dupecheckallradiobutton, FALSE, FALSE, 0);
		dupecheckitems_label = gtk_label_new (_("Items to exclude"));
		gtk_box_pack_start (GTK_BOX (vbox),	dupecheckitems_label,
			FALSE, FALSE, 0);
		dupecheckinclude_frame = gtk_frame_new (NULL);
		gtk_box_pack_start (GTK_BOX (vbox),	dupecheckinclude_frame,
			FALSE, FALSE, 0);
		dupecheckinclude_hbox = gtk_hbox_new (TRUE, 0);
		gtk_container_add
			(GTK_CONTAINER (dupecheckinclude_frame), dupecheckinclude_hbox);
		dupecheckbuttonmode = gtk_check_button_new_with_label (_("Mode"));
		gtk_box_pack_start (GTK_BOX (dupecheckinclude_hbox),
			dupecheckbuttonmode, FALSE, FALSE, 0);
		dupecheckbuttonband = gtk_check_button_new_with_label (_("Band"));
		gtk_box_pack_start (GTK_BOX (dupecheckinclude_hbox),
			dupecheckbuttonband, FALSE, FALSE, 0);
		dupecheckstatusbar_hbox = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vbox),
			dupecheckstatusbar_hbox, FALSE, FALSE, 0);
		dupecheckstatusbar = gtk_statusbar_new ();
		gtk_box_pack_start (GTK_BOX (dupecheckstatusbar_hbox),
			dupecheckstatusbar, TRUE, TRUE, 0);
		gtk_statusbar_set_has_resize_grip
			(GTK_STATUSBAR (dupecheckstatusbar), FALSE);
		dupecheckprogressbar = gtk_progress_bar_new ();
		gtk_box_pack_start (GTK_BOX (dupecheckstatusbar_hbox),
			dupecheckprogressbar, TRUE, TRUE, 0);

		if (programstate.dupecheck == 0)
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (dupecheckthisradiobutton), TRUE);
		else
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (dupecheckallradiobutton), TRUE);

		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (dupecheckbuttonmode), programstate.notdupecheckmode);
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (dupecheckbuttonband), programstate.notdupecheckband);
		gtk_dialog_set_default_response (GTK_DIALOG (dupecheckdialog),
						 GTK_RESPONSE_OK);
		g_signal_connect ((gpointer) cancel_button, "clicked",
                    G_CALLBACK (on_dupecheckcancelbutton_clicked),
                    NULL);
		g_signal_connect ((gpointer) dupecheckdialog, "delete_event",
                    G_CALLBACK (on_dupecheckcancelbutton_clicked),
                    NULL);

		gtk_widget_show_all (dupecheckdialog);
		dupecheckresponse = gtk_dialog_run (GTK_DIALOG(dupecheckdialog));
		if (dupecheckresponse == GTK_RESPONSE_OK)
		{
			dupe = g_new0 (gchar *, 7);
			for (i = 0; i < 7; i++)
				dupe[i] = g_new0 (gchar, 100);

			gtk_statusbar_pop (GTK_STATUSBAR (dupecheckstatusbar), 1);
			gtk_statusbar_push (GTK_STATUSBAR (dupecheckstatusbar), 1,
					    _("Searching..."));

			if (gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (dupecheckthisradiobutton)))
				programstate.dupecheck = 0;
			else
				programstate.dupecheck = 1;
			programstate.notdupecheckmode = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(dupecheckbuttonmode));
			programstate.notdupecheckband = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(dupecheckbuttonband));

			/* all or this page */
			if (programstate.dupecheck == 0)
			{
			        /* get here if only checking this page */
				page = gtk_notebook_get_current_page
					(GTK_NOTEBOOK(mainnotebook));
				pages = page + 1;
			}
			else
			{
			        /* get here if checking all pages */
				page = 0;
				pages = g_list_length (logwindowlist);
			}

			/* create the dupelist window, don't make this a dialog because we want */
			/* to be able to switch focus to the main window                        */
			dupelistdialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title
				(GTK_WINDOW (dupelistdialog), _("xlog - dupe results"));
			gtk_window_set_default_size (GTK_WINDOW (dupelistdialog), 600, 300);
			dupelistdialog_icon_pixbuf = create_pixbuf ("xlog.png");
			if (dupelistdialog_icon_pixbuf)
				{
					gtk_window_set_icon (GTK_WINDOW (dupelistdialog),
						dupelistdialog_icon_pixbuf);
					g_object_unref (dupelistdialog_icon_pixbuf);
				}
			dupelistdialogvbox = gtk_vbox_new (FALSE, 0);
			gtk_container_add
				(GTK_CONTAINER (dupelistdialog), dupelistdialogvbox);

			dupelistlabel = gtk_label_new
				(_("Select an entry to highlight the corresponding entry in the main window"));
			gtk_box_pack_start
				(GTK_BOX (dupelistdialogvbox), dupelistlabel, FALSE, FALSE, 0);
			dupelisthseparator = gtk_hseparator_new ();
			gtk_box_pack_start (GTK_BOX (dupelistdialogvbox),
				dupelisthseparator, FALSE, FALSE, 0);

			/* create a treeview */
			dupelistscrolledwindow = gtk_scrolled_window_new (NULL, NULL);
			gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				(dupelistscrolledwindow),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			gtk_box_pack_start (GTK_BOX (dupelistdialogvbox),
				dupelistscrolledwindow, TRUE, TRUE, 0);
			dupestore = gtk_list_store_new (7, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING);
			dupelisttreeview = gtk_tree_view_new_with_model
				(GTK_TREE_MODEL (dupestore));
			gtk_tree_sortable_set_sort_column_id
				(GTK_TREE_SORTABLE (dupestore), 4, GTK_SORT_ASCENDING);
			g_object_unref (G_OBJECT (dupestore));
			gtk_container_add (GTK_CONTAINER (dupelistscrolledwindow),
				dupelisttreeview);

			/* add callback for selecting a row */
			selection = G_OBJECT (gtk_tree_view_get_selection
				(GTK_TREE_VIEW (dupelisttreeview)));
			gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
				GTK_SELECTION_SINGLE);
			g_signal_connect (selection, "changed",
				G_CALLBACK (on_dupelisttreeview_select_row), NULL);

			/* add columns */
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("LOG", duperenderer, "text", 0, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("NR", duperenderer, "text", 1, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("DATE", duperenderer, "text", 2, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("UTC", duperenderer, "text", 3, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("CALL", duperenderer, "text", 4, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_column_set_sort_column_id (dupecolumn, 4);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("BAND", duperenderer, "text", 5, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);
			duperenderer = gtk_cell_renderer_text_new ();
			dupecolumn = gtk_tree_view_column_new_with_attributes
				("MODE", duperenderer, "text", 6, NULL);
			gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (dupecolumn),
				GTK_TREE_VIEW_COLUMN_GROW_ONLY);
			gtk_tree_view_append_column
				(GTK_TREE_VIEW (dupelisttreeview), dupecolumn);

			dupelisthseparator = gtk_hseparator_new ();
			gtk_box_pack_start (GTK_BOX (dupelistdialogvbox),
				dupelisthseparator, FALSE, FALSE, 0);
			dupelistlabel = gtk_label_new
				(_("This window will not update when you modify QSO's"));
			gtk_box_pack_start
				(GTK_BOX (dupelistdialogvbox), dupelistlabel, FALSE, FALSE, 0);

			dupemodel = gtk_tree_view_get_model (GTK_TREE_VIEW(dupelisttreeview));

			savedpage = page;
			qso_search_num = 0;
			while (page < pages) {
		        /* make a string consisting of callsign[/band][/mode] and store in a list */
			  /* printf("DEBUG: making lists...\n"); */
			  while (gtk_events_pending ()) gtk_main_iteration ();
			  if (breakit) return;

			  logwindow = g_list_nth_data (logwindowlist, page);
			  logmodel = gtk_tree_view_get_model(GTK_TREE_VIEW (logwindow->treeview));

			  valid = gtk_tree_model_get_iter_first (logmodel, &iter);

			  while (valid) {
			    /* printf("DEBUG: Searching QSO %0d\n", qso_search_num++); */

			    if (row++ % 20 == 0) {
			      gtk_progress_bar_pulse (GTK_PROGRESS_BAR (dupecheckprogressbar));
			      while (gtk_events_pending ()) gtk_main_iteration ();
			      if (breakit) return;
			    }

			    gtk_tree_model_get (logmodel, &iter, CALL, &callsign, -1);
			    temp = g_strdup (callsign);
			    if (!programstate.notdupecheckband) {
			      gtk_tree_model_get (logmodel, &iter, BAND, &band, -1);
			      enumband = freq2enum (band);
			      str = g_strdup_printf ("/%d", enumband);
			      temp = g_strconcat (temp, str, NULL);
			      g_free (str);
			    }

			    if (!programstate.notdupecheckmode) {
			      gtk_tree_model_get (logmodel, &iter, MODE, &mode, -1);
			      enummode = mode2enum (mode);
			      str = g_strdup_printf ("/%d", enummode);
			      temp = g_strconcat (temp, str, NULL);
			      g_free (str);
			    }

			    /*                                                                   */
			    /* Create several lists for dupe detection                           */
			    /* entirelist: a list of callsign[/band][/mode] for every QSO        */
			    /* uniqlist: a list of unique callsign[/band][/mode]                 */
			    /*    the uniqlist is only used to figure out which ones are dupes   */
			    /* dupelist: a list of duplicate callsign[/band][/mode] one per dupe */
			    /*                                                                   */

			    entirelist = g_list_append (entirelist, temp);
					
			    uniq = 1;
			    for (i=0; i<g_list_length(uniqlist); i++) {
			      if (g_ascii_strcasecmp(g_list_nth_data(uniqlist,i), temp) == 0) {
				/* dupe found */
				uniq = 0;
				dupe_already_found = 0;
				for (j=0; j<g_list_length(dupelist); j++) {
				  if (g_ascii_strcasecmp(g_list_nth_data(dupelist,j), temp) == 0) {
				    dupe_already_found = 1;
				  }
				}
				if (dupe_already_found == 0) {
				  /* first time finding this dupe...add it to the list */
				  dupefound = TRUE;
				  dupelist = g_list_append(dupelist,temp);
				}
			      }
			    }
			    if (uniq == 1) {
			      /* this is a uniq QSO...add it to the list */
			      uniqlist = g_list_append(uniqlist,temp);
			    }
			    valid = gtk_tree_model_iter_next (logmodel, &iter);
			  }
			  page++;
			}

			/*                                                                   */
			/* Now that all necessary lists have been created, the algorithm is: */
			/* foreach (dupe in dupelist) {                                      */
			/*    foreach (page to search) {                                     */
			/*        foreach (entry on that page) {                             */
			/*            compare the entry to the dupe                          */
			/*            when found, add it to the GUI window                   */
                        /*        }                                                          */
                        /*     }                                                             */
			/* }                                                                 */

			/* printf("DEBUG: finding DUPES...\n"); */
			row = 0;
			num_uniq_dupes = g_list_length(dupelist);
			for (i=0; i<num_uniq_dupes; i++) {
			  temp_dupe = g_list_nth_data(dupelist,i);

			  prog_str = g_strdup_printf("Dup %0d/%0d...", i, num_uniq_dupes);
			  gtk_statusbar_pop (GTK_STATUSBAR (dupecheckstatusbar), 1);
			  gtk_statusbar_push (GTK_STATUSBAR (dupecheckstatusbar), 1,
					      _(prog_str));

			  if (row++ % 5 == 0) {
			    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (dupecheckprogressbar));
			    while (gtk_events_pending ()) gtk_main_iteration ();
			    if (breakit) return;
			  }

			  page = savedpage;
			  while (page < pages) {

			    while (gtk_events_pending ()) gtk_main_iteration ();
			    if (breakit) return;

			    logwindow = g_list_nth_data (logwindowlist, page);
			    logmodel = gtk_tree_view_get_model(GTK_TREE_VIEW (logwindow->treeview));
			    valid = gtk_tree_model_get_iter_first (logmodel, &iter);

			    while (valid) {

			      gtk_tree_model_get (logmodel, &iter, CALL, &callsign, -1);
			      temp = g_strdup (callsign);

			      if (!programstate.notdupecheckband) {
				gtk_tree_model_get (logmodel, &iter, BAND, &band, -1);
				enumband = freq2enum (band);
				str = g_strdup_printf ("/%d", enumband);
				temp = g_strconcat (temp, str, NULL);
				g_free (str);
			      }

			      if (!programstate.notdupecheckmode) {
				gtk_tree_model_get (logmodel, &iter, MODE, &mode, -1);
				enummode = mode2enum (mode);
				str = g_strdup_printf ("/%d", enummode);
				temp = g_strconcat (temp, str, NULL);
				g_free (str);
			      }
			      
			      /* compare dupe entry to log entry */
			      if (g_ascii_strcasecmp(temp_dupe, temp) == 0) {

				/* found a dupe - get the original log data */
				/* and add it to the GUI window             */

				dupe[0] = g_strdup (logwindow->logname);

				gtk_tree_model_get(logmodel, &iter, NR, &nr, -1);
				dupe[1] = g_strdup (nr);

				gtk_tree_model_get(logmodel, &iter, DATE, &date, -1);
				dupe[2] = g_strdup (date);

				gtk_tree_model_get(logmodel, &iter, GMT, &gmt, -1);
				dupe[3] = g_strdup (gmt);

				gtk_tree_model_get(logmodel, &iter, CALL, &callsign, -1);
				dupe[4] = g_strdup (callsign);

				gtk_tree_model_get(logmodel, &iter, BAND, &band, -1);
				dupe[5] = g_strdup (band);

				gtk_tree_model_get(logmodel, &iter, MODE, &mode, -1);
				dupe[6] = g_strdup (mode);

				gtk_list_store_append (GTK_LIST_STORE (dupemodel), &dupeiter);

				gtk_list_store_set (GTK_LIST_STORE (dupemodel),
						    &dupeiter, 0, dupe[0], 1, dupe[1], 2,
						    dupe[2], 3, dupe[3], 4, dupe[4], 5,
						    dupe[5], 6, dupe[6], -1);

			      } /* if (g_ascii_strcasecmp(temp_dupe, temp) == 0) */

			    valid = gtk_tree_model_iter_next (logmodel, &iter);

			    } /* while (valid) */

			    page++;

			  } /* while (page < pages) */

			} /* foreach dupe in the dupelist */

			g_list_free (dupelist);
			g_list_free (uniqlist);
			g_list_free (entirelist);

			if (temp) g_free (temp);
			if (prog_str) g_free (prog_str);

			for (i = 0; i < 7; i++)	g_free (dupe[i]);
			g_free (dupe);
		
			gtk_widget_destroy (dupecheckdialog);
			
			if (dupefound) {
			  gtk_widget_show_all (dupelistdialog);
			}
			else {
			  /* no dupe found, show a message */
			  /* printf("DEBUG: No dupes found.\n"); */
			  warningdialog (_("xlog - dupe results"),
					 _("No dupes found!"), "gtk-info");
			}

		} /* GTK_RESPONSE_OK */
		
	} /* if (page >= 0) */
}
