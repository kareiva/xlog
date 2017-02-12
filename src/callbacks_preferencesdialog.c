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

/* callbacks_preferencesdialog.c
 *
 * callbacks for the preferences dialog, both for the widgets and preferences. 
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <hamlib/rig.h>

#include "callbacks_preferencesdialog.h"
#include "gui_utils.h"
#include "support.h"
#include "cfg.h"
#include "log.h"
#include "utils.h"
#include "hamlib-utils.h"


extern GtkWidget *mainwindow;
extern GtkWidget *keyerwindow;
extern preferencestype preferences;
extern GtkWidget *preferencesdialog;
extern gint clocktimer, savetimer;
extern GList *logwindowlist;

/* return value for setting widgets */
gint
whichhamlibwidgets (gboolean frequency, gboolean smeter)
{
	if (frequency && smeter)
		return (4);
	else if (frequency && (!smeter))
		return (2);
	else if ((!frequency) && smeter)
		return (3);
	else
		return (1);
}

/* check save value, if autosave value larger than 0 start timer */
void
set_autosave (gint value, gint saving)
{
	if (saving != 1 && preferences.saving == 1)
	{
		g_source_remove (savetimer);
		savetimer = -1;
	}
	else if (saving == 1 && preferences.saving != 1)
	{
		if (value > 0)
			savetimer =
		g_timeout_add (value * 60 * 1000, (GSourceFunc) autosave, NULL);
	}
	else if (value != preferences.autosave)
	{			/* new value, stop and start timer */
		g_source_remove (savetimer);
		if (value > 0)
			savetimer =
		g_timeout_add (value * 60 * 1000, (GSourceFunc) autosave, NULL);
	}
	preferences.autosave = value;
	preferences.saving = saving;
}

/* set savepath if changed */
void
set_path (gchar * pathstr)
{
	if (strlen (pathstr) > 0)
	{
		if (g_ascii_strcasecmp (preferences.savedir, pathstr))
			preferences.savedir = g_strdup (pathstr);
	}
}

/* set backuppath if changed */
void
set_backuppath (gchar * pathstr)
{
	if (strlen (pathstr) > 0)
	{
		if (g_ascii_strcasecmp (preferences.backupdir, pathstr))
			preferences.backupdir = g_strdup (pathstr);
	}
}

/* set logs to load if changed */
void
set_logstoload (gchar * logs)
{
	gchar **logsplit;

	if (g_ascii_strcasecmp (logs, preferences.logstoload) != 0)
	{
		if (strlen (logs) == 0)
			logs = g_strdup ("*");
		logsplit = g_strsplit (logs, ",", -1);	/* delete spaces */
		deletespaces (logsplit);
		preferences.logstoload = g_strjoinv (",", logsplit);
		g_strfreev (logsplit);
	}
}

/* set QTH locator */
void
set_qthlocator (gchar * locator)
{
	if (g_ascii_strcasecmp (locator, preferences.locator) != 0)
	{
		if (strlen (locator) == 0)
			locator = g_strdup ("AA00AA");
		preferences.locator = g_strdup (locator);
	}
}

/* set callsign */
void
set_callsign (gchar * callsign)
{
	if (g_ascii_strcasecmp (callsign, preferences.callsign) != 0)
	{
		if (strlen (callsign) == 0)
			callsign = g_strdup ("N0CALL");
		preferences.callsign = g_strdup (callsign);
	}
}

/* enable/disable clock */
void
set_clock (gboolean on)
{
	GtkWidget *clockhandlebox;

	clockhandlebox = lookup_widget (mainwindow, "clockhandlebox");
	if (on)
	{			/* yes, we want clock */
		if (preferences.clock == 0)
			clocktimer = g_timeout_add (1000, (GSourceFunc) updateclock, NULL);
		gtk_widget_show (clockhandlebox);
		preferences.clock = 1;
	}
	else
	{			/* no clock */
		if (preferences.clock > 0)
			g_source_remove (clocktimer);
		clocktimer = -1;
		gtk_widget_hide (clockhandlebox);
		preferences.clock = 0;
	}
}

/* check if hamlib has changed */
gboolean hamlib_changed (gint hamlibwidgets, gint rigid, gchar *device, 
	gint polltime, gchar *rigconf)
{

	if (preferences.hamlib != hamlibwidgets)
		return TRUE;
	else if (g_ascii_strcasecmp (preferences.rigconf, rigconf))
		return TRUE;
	else if (preferences.rigid != rigid)
		return TRUE;
	else if (g_ascii_strcasecmp (preferences.device, device))
		return TRUE;
	else if (preferences.polltime != polltime)
		return TRUE;
	else 
	/* FALSE is returned when nothing is changed */
		return FALSE;
}

/* toggle sensistivity of backup entry and button */
void
on_backupradiobutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data)
{
	gboolean state;
	GtkWidget *backupentry, *backupbutton;

	backupentry = lookup_widget (preferencesdialog, "backupentry");
	backupbutton = lookup_widget (preferencesdialog, "backupbutton");
	state = gtk_toggle_button_get_active (togglebutton);
	if (state)
	{
		gtk_widget_set_sensitive(backupentry, TRUE);
		gtk_widget_set_sensitive(backupbutton, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(backupentry, FALSE);
		gtk_widget_set_sensitive(backupbutton, FALSE);
	}
}

/* toggle sensistivity of band widgets in preferences dialog */
void
on_bandsradiobutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data)
{
	GtkWidget *bandslabel, *bandsentry;

	bandslabel = lookup_widget (preferencesdialog, "bandslabel");
	bandsentry = lookup_widget (preferencesdialog, "bandsentry");
	switch (atoi (user_data))
	{
	case 1:
		gtk_widget_set_sensitive (bandslabel, 0);
		gtk_widget_set_sensitive (bandsentry, 0);
		break;
	case 2:
		gtk_widget_set_sensitive (bandslabel, 1);
		gtk_widget_set_sensitive (bandsentry, 1);
		break;
	}
}

/* toggle sensistivity of modes widgets in preferences dialog */
void
on_modesradiobutton_toggled (GtkToggleButton * togglebutton,
					 gpointer user_data)
{
	GtkWidget *modeslabel, *modesentry;

	modeslabel = lookup_widget (preferencesdialog, "modeslabel");
	modesentry = lookup_widget (preferencesdialog, "modesentry");
	switch (atoi (user_data))
	{
	case 1:
		gtk_widget_set_sensitive (modeslabel, 0);
		gtk_widget_set_sensitive (modesentry, 0);
		break;
	case 2:
		gtk_widget_set_sensitive (modeslabel, 1);
		gtk_widget_set_sensitive (modesentry, 1);
		break;
	}

}


/* check autosave */
void
on_autosaveradiobutton_toggled (GtkToggleButton * togglebutton,
				gpointer user_data)
{
	gboolean state;
	GtkWidget *autosaveframe;

	state = gtk_toggle_button_get_active (togglebutton);
	autosaveframe = lookup_widget (preferencesdialog, "autosaveframe");
	gtk_widget_set_sensitive (autosaveframe, state);
}

/* check polling */
void
on_pollingcheckbutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data)
{
	gboolean state;
	GtkWidget *pollingframe;

	state = gtk_toggle_button_get_active (togglebutton);
	pollingframe = lookup_widget (preferencesdialog, "pollingframe");
	gtk_widget_set_sensitive (pollingframe, state);
}

/* toggle hamlib widgets in the preferences dialog */
void
on_hamlibcheckbutton_toggled (GtkToggleButton * togglebutton,
	gpointer user_data)
{
	GtkWidget *hamlibframe, *pollingframe, *pollingcheckbutton;
	gboolean state;

	hamlibframe = lookup_widget (preferencesdialog, "hamlibframe");
	state = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (hamlibframe, state);

	if (state)
	{
		pollingcheckbutton = lookup_widget (preferencesdialog,
			"pollingcheckbutton");
		pollingframe = lookup_widget (preferencesdialog, "pollingframe");
		state = gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (pollingcheckbutton));
		gtk_widget_set_sensitive (pollingframe, state);
	}
}

void
on_radiobutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *rigdialog, *riglistdialogvbox, *riglisttreeview,
		*rigscrolledwindow, *radioentry;
	gint i, response, numrigs, rigid;
	gchar *temp;
	GtkListStore *rigstore;
	GtkTreeModel *model;
	GtkTreeIter rigiter, selected;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *sel;
	GList *riglist;
	struct rig_caps *rcaps = NULL;

	rigdialog = gtk_dialog_new_with_buttons (_("xlog - select a radio"),
		GTK_WINDOW(mainwindow), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_widget_set_size_request (rigdialog, 400, 300);
	riglistdialogvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add
		(GTK_CONTAINER (GTK_DIALOG(rigdialog)->vbox), riglistdialogvbox);
	rigscrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
		(rigscrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    	gtk_box_pack_start (GTK_BOX (riglistdialogvbox),
		rigscrolledwindow, TRUE, TRUE, 0);
	gtk_box_reorder_child (GTK_BOX (riglistdialogvbox), rigscrolledwindow, 0);
	rigstore = gtk_list_store_new (3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
	riglisttreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (rigstore));
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		("Rigid", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (riglisttreeview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		(_("Model"), renderer, "text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (riglisttreeview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		(_("Manufacturer"), renderer, "text", 2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (riglisttreeview), column);

	gtk_tree_sortable_set_sort_column_id
		(GTK_TREE_SORTABLE (rigstore), 1, GTK_SORT_ASCENDING);
	g_object_unref (G_OBJECT (rigstore));
	gtk_container_add (GTK_CONTAINER (rigscrolledwindow), riglisttreeview);

	riglist = rig_get_list ();
	numrigs = g_list_length (riglist);
	for (i = 0; i < numrigs; i++)
	{
		rcaps = (struct rig_caps *) g_list_nth_data (riglist, i);
		gtk_list_store_append (rigstore, &rigiter);
		gtk_list_store_set (rigstore, &rigiter,
			0, rcaps->rig_model, 1, rcaps->model_name,  2, rcaps->mfg_name, -1);
	}
	gtk_widget_show_all (rigdialog);

	response = gtk_dialog_run (GTK_DIALOG(rigdialog));
	if (response == GTK_RESPONSE_OK)
	{
		sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (riglisttreeview));
		radioentry = lookup_widget (preferencesdialog, "radioentry");
		if (gtk_tree_selection_get_selected (sel, &model, &selected))
		{
			gtk_tree_model_get (model, &selected, 0, &rigid, -1);
			temp = g_strdup_printf ("%d", rigid);
			gtk_entry_set_text (GTK_ENTRY(radioentry), temp);
			g_free (temp);
		}
		else
			gtk_entry_set_text (GTK_ENTRY(radioentry), "1");
	}
	g_list_free (riglist);
	gtk_widget_destroy (rigdialog);
}
