/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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
 * callbacks_mainwindow.c - callbacks for the main window
 *
 * these are callbacks not part of the menu, qsoframe, toolbar or list.
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <hamlib/rig.h>

#include "hamlib-utils.h"
#include "callbacks_mainwindow.h"
#include "callbacks_mainwindow_toolbar.h"
#include "callbacks_mainwindow_menu.h"
#include "gui_savedialog.h"
#include "support.h"
#include "gui_utils.h"
#include "utils.h"
#include "cfg.h"
#include "log.h"
#include "gui_netkeyer.h"
#include "netkeyer.h"
#include "main.h"

extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;
extern GtkWidget *keyerwindow;
extern preferencestype preferences;
extern keyerstatetype keyerstate;
extern programstatetype programstate;
extern GList *logwindowlist;
extern GtkUIManager *ui_manager;

/* capture the delete event and display a warning in case the log
 * has not been saved */
gboolean
on_mainwindow_delete_event (GtkWidget * widget, GdkEvent * event,
					gpointer user_data)
{
	gint i;
	logtype *logwindow;
	gboolean logchanged = FALSE;

	for (i = 0; i < g_list_length (logwindowlist); i++)
	{
		logwindow = g_list_nth_data (logwindowlist, i);
		if (logwindow->logchanged) logchanged = TRUE;
	}

	if (logchanged)
		create_savedialog ();
	else
	{
		if (preferences.areyousure > 0)
			create_exitdialog ();
		else
		{
			save_windowsize_and_cleanup ();
			gtk_main_quit ();
		}

	}
	return (TRUE);
}

/* switch between pages of the notebook */
void
on_mainnotebook_switch_page (GtkNotebook * notebook, GtkNotebookPage * page,
					 gint page_num, gpointer user_data)
{
	logtype *logw = NULL;

	while (page_num >= 0)
	{
		logw = g_list_nth_data (logwindowlist, page_num);
		if (logw) break;
		page_num--;
	}

	if (logw) 
	{
		set_qsoframe (logw);
	}
}

/* catch keypresses when keyer is active, don't use PgUp/PgDn here */
gboolean
on_mainwindow_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GtkWidget *callentry, *rstentry, *myrstentry, *count, *stopbutton, 
		*f1button, *f2button, *f3button, *f4button, *f5button, *f6button,
		*f7button, *f8button, *f9button, *f10button, *f11button, *f12button,
		*clickallmenu, *abutton;
	gchar *call, *countstr, *str;
	gint c;

#define CQ 13
#define SP 14
 
	if (keyerwindow)
	switch (event->keyval)
	{
	case GDK_F1:
		f1button = lookup_widget (keyerwindow, "f1button");
		gtk_widget_activate (f1button);
	break;
	case GDK_F2:
		f2button = lookup_widget (keyerwindow, "f2button");
		gtk_widget_activate (f2button);
	break;
	case GDK_F3:
		f3button = lookup_widget (keyerwindow, "f3button");
		gtk_widget_activate (f3button);
	break;
	case GDK_F4:
		f4button = lookup_widget (keyerwindow, "f4button");
		gtk_widget_activate (f4button);
	break;
	case GDK_F5:
		f5button = lookup_widget (keyerwindow, "f5button");
		gtk_widget_activate (f5button);
	break;
	case GDK_F6:
		f6button = lookup_widget (keyerwindow, "f6button");
		gtk_widget_activate (f6button);
	break;
	case GDK_F7:
		f7button = lookup_widget (keyerwindow, "f7button");
		gtk_widget_activate (f7button);
	break;
	case GDK_F8:
		f8button = lookup_widget (keyerwindow, "f8button");
		gtk_widget_activate (f8button);
	break;
	case GDK_F9:
		f9button = lookup_widget (keyerwindow, "f9button");
		gtk_widget_activate (f9button);
	break;
	case GDK_F10:
		f10button = lookup_widget (keyerwindow, "f10button");
		gtk_widget_activate (f10button);
	break;
	case GDK_F11:
		f11button = lookup_widget (keyerwindow, "f11button");
		gtk_widget_activate (f11button);
	break;
	case GDK_F12:
		f12button = lookup_widget (keyerwindow, "f12button");
		gtk_widget_activate (f12button);
	break;
	case GDK_Escape:
		stopbutton = lookup_widget (keyerwindow, "stopbutton");
		gtk_widget_activate (stopbutton);
	break;
	case GDK_Return:
		myrstentry = lookup_widget (mainwindow, "myrstentry");
		callentry = lookup_widget (mainwindow, "callentry");
		call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);

		if (!GTK_WIDGET_HAS_FOCUS (callentry) && (strlen (call) == 0))
		{ // Ubuntu bug #608718: do not log QSO without callsign
		  // Thanks John Nogatch for the patch.
			gtk_widget_grab_focus (callentry);
		}

		else if (gtk_widget_has_focus (myrstentry))
		{
			if (keyerstate.cqmode)
				cw (NULL, GINT_TO_POINTER(CQ));
			else
				cw (NULL, GINT_TO_POINTER(SP));
			clickallmenu = gtk_ui_manager_get_widget
				(ui_manager, "/MainMenu/EditMenu/Click All");
			g_signal_emit_by_name (G_OBJECT (clickallmenu), "activate");
			abutton = gtk_ui_manager_get_widget (ui_manager,
				"/MainMenu/EditMenu/Write");
			g_signal_emit_by_name (G_OBJECT (abutton), "activate");
		}
		else if (gtk_widget_has_focus (callentry))
		{
			rstentry = lookup_widget (mainwindow, "rstentry");
			if (strlen (call) > 0)
			{
				if (keyerstate.cqmode)
				{
					f3button = lookup_widget (keyerwindow, "f3button");
					gtk_widget_activate (f3button);
				}
				else
				{
					f6button = lookup_widget (keyerwindow, "f6button");
					gtk_widget_activate (f6button);
				}

				/* fill in defaults, check if we use a counter */
				if (strlen (preferences.defaulttxrst) > 0)
				{
					if (g_strrstr (preferences.defaulttxrst, "#"))
					{
						count = lookup_widget (keyerwindow, "count");
						c = gtk_spin_button_get_value (GTK_SPIN_BUTTON (count));
						if (c < 10)
							countstr = g_strdup_printf ("00%d", c);
						else if (c < 100)
							countstr = g_strdup_printf ("0%d", c);
						else
							countstr = g_strdup_printf ("%d", c);
						str = my_strreplace (preferences.defaulttxrst, "#",
							countstr);
						g_free (countstr);
						gtk_entry_set_text (GTK_ENTRY (rstentry), str);
						g_free (str);
					}
					else
						gtk_entry_set_text (GTK_ENTRY (rstentry),
							preferences.defaulttxrst);
				}
				if (strlen (preferences.defaultrxrst) > 0)
					gtk_entry_set_text (GTK_ENTRY (myrstentry),
						preferences.defaultrxrst);

				gtk_widget_grab_focus (myrstentry);
				gtk_editable_set_position (GTK_EDITABLE(myrstentry), -1);
			}
			else
			{
				if (keyerstate.cqmode)
				{	/* call CQ */ 
					f1button = lookup_widget (keyerwindow, "f1button");
					gtk_widget_activate (f1button);
				}
				else
				{
					f6button = lookup_widget (keyerwindow, "f6button");
					gtk_widget_activate (f6button);
				}
			}
		}
	break;
	case GDK_Up:
		if ((preferences.hamlib > 0) && (event->state & GDK_CONTROL_MASK))
		{
			if (preferences.polltime == 0)
				get_frequency ();
			set_frequency (programstate.rigfrequency + 50);
		}
		else
			return FALSE;
	break;
	case GDK_Down:
		if ((preferences.hamlib > 0) && (event->state & GDK_CONTROL_MASK))
		{
			if (preferences.polltime == 0)
				get_frequency ();
			set_frequency (programstate.rigfrequency - 50);
		}
		else
			return FALSE;
	break;
	default:
	return FALSE;
	break;
	}
	else
	switch (event->keyval)
	{
		case GDK_Up:
			if ((preferences.hamlib > 0) && (event->state & GDK_CONTROL_MASK))
			{
				if (preferences.polltime == 0)
					get_frequency ();
				set_frequency (programstate.rigfrequency + 50);
			}
			else
				return FALSE;
		break;
		case GDK_Down:
			if ((preferences.hamlib > 0) && (event->state & GDK_CONTROL_MASK))
			{
				if (preferences.polltime == 0)
					get_frequency ();
				set_frequency (programstate.rigfrequency - 50);
			}
			else
				return FALSE;
		break;
		case GDK_Return:
			callentry = lookup_widget (mainwindow, "callentry");
			if (gtk_widget_has_focus(callentry))
			{
				rstentry = lookup_widget (mainwindow, "rstentry");
				gtk_widget_grab_focus (rstentry);
			}
		break;
		default:
		return FALSE;
		break;
	}

	return TRUE;
}
