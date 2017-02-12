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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "support.h"
#include "netkeyer.h"
#include "gui_netkeyer.h"
#include "gui_utils.h"
#include "utils.h"
#include "cfg.h"
#include "hamlib-utils.h"
#include "main.h"

GtkWidget *keyerwindow = NULL;
extern preferencestype preferences;
extern programstatetype programstate;
extern GtkWidget *mainwindow;
keyerstatetype keyerstate;

void savekeyer (void)
{
	GtkWidget *f1entry, *f2entry, *f3entry, *f4entry, *f5entry,
		*f6entry, *f7entry, *f8entry, *f9entry, *f10entry, *f11entry,
		*f12entry, *cqentry, *spentry, *wpm;

	wpm = lookup_widget (keyerwindow, "wpm");
	f1entry = lookup_widget (keyerwindow, "f1entry");
	f2entry = lookup_widget (keyerwindow, "f2entry");
	f3entry = lookup_widget (keyerwindow, "f3entry");
	f4entry = lookup_widget (keyerwindow, "f4entry");
	f5entry = lookup_widget (keyerwindow, "f5entry");
	f6entry = lookup_widget (keyerwindow, "f6entry");
	f7entry = lookup_widget (keyerwindow, "f7entry");
	f8entry = lookup_widget (keyerwindow, "f8entry");
	f9entry = lookup_widget (keyerwindow, "f9entry");
	f10entry = lookup_widget (keyerwindow, "f10entry");
	f11entry = lookup_widget (keyerwindow, "f11entry");
	f12entry = lookup_widget (keyerwindow, "f12entry");
	cqentry = lookup_widget (keyerwindow, "cqentry");
	spentry = lookup_widget (keyerwindow, "spentry");

	preferences.cwspeed = gtk_spin_button_get_value (GTK_SPIN_BUTTON(wpm));
	preferences.cwf1 = gtk_editable_get_chars (GTK_EDITABLE (f1entry), 0, -1);
	preferences.cwf2 = gtk_editable_get_chars (GTK_EDITABLE (f2entry), 0, -1);
	preferences.cwf3 = gtk_editable_get_chars (GTK_EDITABLE (f3entry), 0, -1);
	preferences.cwf4 = gtk_editable_get_chars (GTK_EDITABLE (f4entry), 0, -1);
	preferences.cwf5 = gtk_editable_get_chars (GTK_EDITABLE (f5entry), 0, -1);
	preferences.cwf6 = gtk_editable_get_chars (GTK_EDITABLE (f6entry), 0, -1);
	preferences.cwf7 = gtk_editable_get_chars (GTK_EDITABLE (f7entry), 0, -1);
	preferences.cwf8 = gtk_editable_get_chars (GTK_EDITABLE (f8entry), 0, -1);
	preferences.cwf9 = gtk_editable_get_chars (GTK_EDITABLE (f9entry), 0, -1);
	preferences.cwf10 = gtk_editable_get_chars (GTK_EDITABLE (f10entry), 0, -1);
	preferences.cwf11 = gtk_editable_get_chars (GTK_EDITABLE (f11entry), 0, -1);
	preferences.cwf12 = gtk_editable_get_chars (GTK_EDITABLE (f12entry), 0, -1);
	preferences.cwcq = gtk_editable_get_chars (GTK_EDITABLE (cqentry), 0, -1);
	preferences.cwsp = gtk_editable_get_chars (GTK_EDITABLE (spentry), 0, -1);
}

/* delete keyer window and save settings, close connection to cwdaemon */
static gboolean
on_keyerwindow_delete_event (GtkWidget * widget, GdkEvent * event,
	gpointer user_data)
{
	savekeyer ();
	netkeyer_close ();
	keyerwindow = NULL;

	return FALSE;
}

/* grab an entry and send it over the UDP socket */
void cw (GtkButton *button, gpointer user_data)
{
	GtkWidget *entry, *callentry, *count;
	gchar *entryname, *sendstr, *call, *countstr, *lastmsg;
	gint i, result = 0, fkey;

	fkey = GPOINTER_TO_INT(user_data);
	if (fkey < 13)
	{
		entryname = g_strdup_printf("f%dentry", fkey);
	}
	else if (fkey == 13)
	{
		entryname = g_strdup("cqentry");
	}
	else if (fkey == 14)
	{
		entryname = g_strdup("spentry");
	}
	else
		return;
	entry = lookup_widget (keyerwindow, entryname);
	callentry = lookup_widget (mainwindow, "callentry");
	count = lookup_widget (keyerwindow, "count");

	sendstr = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
	if (strlen (sendstr) > 0)
	{
		i = gtk_spin_button_get_value (GTK_SPIN_BUTTON(count));
		if (i < 10)
			countstr = g_strdup_printf ("00%d", i);
		else if (i < 100)
			countstr = g_strdup_printf ("0%d", i);
		else
			countstr = g_strdup_printf ("%d", i);
		if (keyerstate.shortnr)
		{
			countstr = my_strreplace (countstr, "0", "t");
			countstr = my_strreplace (countstr, "9", "n");
		}
		sendstr = my_strreplace (sendstr, "#", countstr);
		g_free (countstr);

		sendstr = my_strreplace (sendstr, "%", preferences.callsign);

		lastmsg = get_last_msg ();
		if (strlen (lastmsg) > 0)
		{
			sendstr = my_strreplace (sendstr, "~", lastmsg);
			g_free (lastmsg);
		}
		else
			sendstr = my_strreplace (sendstr, "~", "?");
		if (strlen (call) > 0)
		{
			sendstr = my_strreplace (sendstr, "@", call);
			g_free (call);
		}
		else
			sendstr = my_strreplace (sendstr, "@", "?");
		result = tonetkeyer (K_MESSAGE, sendstr);
		if (result == -1)
			update_statusbar (_("Send to keyer has failed"));
		g_free (sendstr);
	}
	g_free (entryname);
}

/* stop button clicked */
static void stop (GtkButton *button, gpointer user_data)
{
	gint result;

	result = tonetkeyer (K_ABORT, NULL);
}

/* catch keys when keyerwindow has focus, so we can use F1-F12, ESC and 
   PgUp, PgDn */
static gboolean
on_keyerwindow_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GtkWidget *stopbutton,
		*f1button, *f2button, *f3button, *f4button, *f5button, *f6button,
		*f7button, *f8button, *f9button, *f10button, *f11button, *f12button;

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
	case GDK_Up:
		if ((preferences.hamlib > 0) && (event->state & GDK_CONTROL_MASK))
		{
			if (preferences.polltime == 0)
				get_frequency ();
			set_frequency (programstate.rigfrequency + 50);
			return TRUE;
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
			return TRUE;
		}
		else
			return FALSE;
	break;
	default:
	break;
	}

	return FALSE;
}

/* handle changes to the wpm spin button */
static void
change_wpm (GtkSpinButton *wpm, gpointer user_data)
{
	gint cwspeed, result;
	gchar *value;

	cwspeed = gtk_spin_button_get_value (wpm);
	value = g_strdup_printf ("%d", cwspeed);
	result = tonetkeyer (K_SPEED, value);
	g_free (value);
}

/* send typed text */
static void
send_text (GtkTextBuffer * buffer, GtkTextIter * iter, gchar * text,
	gint len, gpointer user_data)
{
	gint result;

	result = tonetkeyer (K_MESSAGE, text);
	if (result < 0) update_statusbar (_("Send to keyer has failed"));
}

static void
mode_change(GtkComboBox *widget, gpointer user_data)
{
	gint active;

	active = gtk_combo_box_get_active (widget);
	if (active == 0)
		keyerstate.cqmode = TRUE;
	else
		keyerstate.cqmode = FALSE;
}

static void
on_shortnr_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	keyerstate.shortnr = gtk_toggle_button_get_active (togglebutton);
}

/* activate keyer window and initialize UDP socket */
void on_keyer_activate (GtkAction *action, gpointer user_data)
{
	GtkWidget *keyervbox, *hbox1, *hbox2, *hbox3, *hbox4, *hbox5, *hbox6,
		*hbox7, *hbox8,	*hbox9, *hsep,
		*f1button, *f1entry, *f6button, *f6entry,
		*f2button, *f2entry, *f7button, *f7entry, *f3button, *f3entry,
		*f8button, *f8entry, *f4button, *f4entry, *f9button, *f9entry,
		*f5button, *f5entry, *f10button, *f10entry, *f11button, *f11entry,
		*f12button, *f12entry,
		*cqlabel, *cqentry, *splabel, *spentry, *mode, *shortnr,
		*scrolledkeyerwindow,
		*keyertext, *wpm, *wpmcountlabel, *count, *stopbutton;
	GtkObject *wpm_adj, *count_adj;
	GdkPixbuf *keyer_icon_pixbuf;
	GtkTextBuffer *buffer;
	gint result = 0;
	gchar *speed;

	if (keyerwindow)
	{
		gtk_window_present (GTK_WINDOW(keyerwindow));
		return;
	}

	keyerwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (keyerwindow), _("xlog - keyer"));
	keyer_icon_pixbuf = create_pixbuf ("xlog-keyer.png");
	if (keyer_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (keyerwindow), keyer_icon_pixbuf);
		g_object_unref (keyer_icon_pixbuf);
	}

	keyervbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (keyerwindow), keyervbox);
	gtk_container_set_border_width (GTK_CONTAINER (keyervbox), 4);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox1, TRUE, TRUE, 0);
	f1button = gtk_button_new_with_label ("F1");
	gtk_box_pack_start (GTK_BOX (hbox1), f1button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f1button, 50, -1);
	f1entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox1), f1entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f1entry, 120, -1);
	f7button = gtk_button_new_with_label ("F7");
	gtk_box_pack_start (GTK_BOX (hbox1), f7button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f7button, 50, -1);
	f7entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox1), f7entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f7entry, 120, -1);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox2, TRUE, TRUE, 0);
	f2button = gtk_button_new_with_label ("F2");
	gtk_box_pack_start (GTK_BOX (hbox2), f2button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f2button, 50, -1);
	f2entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox2), f2entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f2entry, 120, -1);
	f8button = gtk_button_new_with_label ("F8");
	gtk_box_pack_start (GTK_BOX (hbox2), f8button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f8button, 50, -1);
	f8entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox2), f8entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f8entry, 120, -1);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox3, TRUE, TRUE, 0);
	f3button = gtk_button_new_with_label ("F3");
	gtk_box_pack_start (GTK_BOX (hbox3), f3button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f3button, 50, -1);
	f3entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox3), f3entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f3entry, 120, -1);
	f9button = gtk_button_new_with_label ("F9");
	gtk_box_pack_start (GTK_BOX (hbox3), f9button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f9button, 50, -1);
	f9entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox3), f9entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f9entry, 120, -1);

	hbox4 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox4, TRUE, TRUE, 0);
	f4button = gtk_button_new_with_label ("F4");
	gtk_box_pack_start (GTK_BOX (hbox4), f4button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f4button, 50, -1);
	f4entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox4), f4entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f4entry, 120, -1);
	f10button = gtk_button_new_with_label ("F10");
	gtk_box_pack_start (GTK_BOX (hbox4), f10button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f10button, 50, -1);
	f10entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox4), f10entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f10entry, 120, -1);

	hbox5 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox5, TRUE, TRUE, 0);
	f5button = gtk_button_new_with_label ("F5");
	gtk_box_pack_start (GTK_BOX (hbox5), f5button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f5button, 50, -1);
	f5entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox5), f5entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f5entry, 120, -1);
	f11button = gtk_button_new_with_label ("F11");
	gtk_box_pack_start (GTK_BOX (hbox5), f11button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f11button, 50, -1);
	f11entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox5), f11entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f11entry, 120, -1);

	hbox7 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox7, TRUE, TRUE, 0);
	f6button = gtk_button_new_with_label ("F6");
	gtk_box_pack_start (GTK_BOX (hbox7), f6button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f6button, 50, -1);
	f6entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox7), f6entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f6entry, 120, -1);
	f12button = gtk_button_new_with_label ("F12");
	gtk_box_pack_start (GTK_BOX (hbox7), f12button, FALSE, FALSE, 0);
	gtk_widget_set_size_request (f12button, 50, -1);
	f12entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox7), f12entry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (f12entry, 120, -1);

	hbox8 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox8, TRUE, TRUE, 0);
	cqlabel = gtk_label_new ("CQTU");
	gtk_box_pack_start (GTK_BOX (hbox8), cqlabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (cqlabel, 50, -1);
	cqentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox8), cqentry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (cqentry, 120, -1);
	splabel = gtk_label_new ("S&PTU");
	gtk_box_pack_start (GTK_BOX (hbox8), splabel, FALSE, FALSE, 0);
	gtk_widget_set_size_request (splabel, 50, -1);
	spentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox8), spentry, TRUE, TRUE, 0);
	gtk_widget_set_size_request (spentry, 120, -1);
	gtk_widget_set_tooltip_text
		(cqentry, _("Auto exchange message in CQ mode"));
	gtk_widget_set_tooltip_text
		(spentry, _("Auto exchange message in S&P mode"));

	hsep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (keyervbox), hsep, TRUE, TRUE, 3);

	hbox6 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox6, FALSE, FALSE, 0);
	wpm_adj = gtk_adjustment_new (10, 5, 60, 1, 2, 0);
	wpm = gtk_spin_button_new (GTK_ADJUSTMENT (wpm_adj), 1, 0);
	gtk_box_pack_start (GTK_BOX (hbox6), wpm, FALSE, FALSE, 0);
	wpmcountlabel = gtk_label_new ("<wpm|count>");
	gtk_box_pack_start (GTK_BOX (hbox6), wpmcountlabel, FALSE, FALSE, 0);
	count_adj = gtk_adjustment_new (1, 1, 10000, 1, 10, 0);
	count = gtk_spin_button_new (GTK_ADJUSTMENT (count_adj), 1, 0);
	gtk_box_pack_start (GTK_BOX (hbox6), count, FALSE, FALSE, 0);

	hsep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (keyervbox), hsep, TRUE, TRUE, 3);

	hbox9 = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (keyervbox), hbox9, TRUE, TRUE, 0);
	mode = gtk_combo_box_new_text ();
	gtk_box_pack_start (GTK_BOX (hbox9), mode, FALSE, FALSE, 0);
	gtk_combo_box_append_text (GTK_COMBO_BOX(mode), "CQ");
	gtk_combo_box_append_text (GTK_COMBO_BOX(mode), "S&P");
	gtk_combo_box_set_active (GTK_COMBO_BOX(mode), 0);
	stopbutton = gtk_button_new_from_stock (GTK_STOCK_STOP);
	gtk_box_pack_start (GTK_BOX (hbox9), stopbutton, FALSE, FALSE, 0);
	shortnr = gtk_check_button_new_with_label ("9=n, 0=t");
	gtk_box_pack_start (GTK_BOX (hbox9), shortnr, FALSE, FALSE, 0);

	hsep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (keyervbox), hsep, TRUE, TRUE, 3);

	scrolledkeyerwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_end (GTK_BOX (keyervbox), scrolledkeyerwindow, FALSE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledkeyerwindow),
		GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_set_shadow_type
		(GTK_SCROLLED_WINDOW (scrolledkeyerwindow), GTK_SHADOW_IN);

	keyertext = gtk_text_view_new ();
	gtk_container_add (GTK_CONTAINER (scrolledkeyerwindow), keyertext);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(keyertext), GTK_WRAP_CHAR);

	gtk_entry_set_max_length (GTK_ENTRY (f1entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f2entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f3entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f4entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f5entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f6entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f7entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f8entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f9entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f10entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f11entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (f12entry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (cqentry), 80);
	gtk_entry_set_max_length (GTK_ENTRY (spentry), 80);

	/* init keyer and set speed */
	result = netkeyer_init ();
	if (result == 1)
	{
		update_statusbar (_("gethostbyname for keyer failed"));
		return;
	}
	else if (result == 2)
	{
		update_statusbar (_("creation of socket for keyer failed"));
		return;
	}
	keyerstate.shortnr = FALSE;
	keyerstate.cqmode = TRUE;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (wpm), preferences.cwspeed);
	speed = g_strdup_printf ("%d", preferences.cwspeed);
	result = tonetkeyer (K_SPEED, speed);
	g_free (speed);

	/* set entries for function keys */
	gtk_entry_set_text (GTK_ENTRY(f1entry), preferences.cwf1);
	gtk_entry_set_text (GTK_ENTRY(f2entry), preferences.cwf2);
	gtk_entry_set_text (GTK_ENTRY(f3entry), preferences.cwf3);
	gtk_entry_set_text (GTK_ENTRY(f4entry), preferences.cwf4);
	gtk_entry_set_text (GTK_ENTRY(f5entry), preferences.cwf5);
	gtk_entry_set_text (GTK_ENTRY(f6entry), preferences.cwf6);
	gtk_entry_set_text (GTK_ENTRY(f7entry), preferences.cwf7);
	gtk_entry_set_text (GTK_ENTRY(f8entry), preferences.cwf8);
	gtk_entry_set_text (GTK_ENTRY(f9entry), preferences.cwf9);
	gtk_entry_set_text (GTK_ENTRY(f10entry), preferences.cwf10);
	gtk_entry_set_text (GTK_ENTRY(f11entry), preferences.cwf11);
	gtk_entry_set_text (GTK_ENTRY(f12entry), preferences.cwf12);
	gtk_entry_set_text (GTK_ENTRY(cqentry), preferences.cwcq);
	gtk_entry_set_text (GTK_ENTRY(spentry), preferences.cwsp);

	GLADE_HOOKUP_OBJECT (keyerwindow, f1entry, "f1entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f2entry, "f2entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f3entry, "f3entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f4entry, "f4entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f5entry, "f5entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f6entry, "f6entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f7entry, "f7entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f8entry, "f8entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f9entry, "f9entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f10entry, "f10entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f11entry, "f11entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f12entry, "f12entry");
	GLADE_HOOKUP_OBJECT (keyerwindow, f1button, "f1button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f2button, "f2button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f3button, "f3button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f4button, "f4button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f5button, "f5button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f6button, "f6button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f7button, "f7button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f8button, "f8button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f9button, "f9button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f10button, "f10button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f11button, "f11button");
	GLADE_HOOKUP_OBJECT (keyerwindow, f12button, "f12button");
	GLADE_HOOKUP_OBJECT (keyerwindow, cqentry, "cqentry");
	GLADE_HOOKUP_OBJECT (keyerwindow, spentry, "spentry");
	GLADE_HOOKUP_OBJECT (keyerwindow, wpm, "wpm");
	GLADE_HOOKUP_OBJECT (keyerwindow, count, "count");
	GLADE_HOOKUP_OBJECT (keyerwindow, cqlabel, "cqlabel");
	GLADE_HOOKUP_OBJECT (keyerwindow, splabel, "splabel");
	GLADE_HOOKUP_OBJECT (keyerwindow, stopbutton, "stopbutton");

	g_signal_connect ((gpointer) keyerwindow, "delete_event",
		G_CALLBACK (on_keyerwindow_delete_event), NULL);
	g_signal_connect ((gpointer) keyerwindow, "key_press_event",
		G_CALLBACK (on_keyerwindow_keypress), NULL);
	g_signal_connect (G_OBJECT (f1button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(1));
	g_signal_connect (G_OBJECT (f2button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(2));
	g_signal_connect (G_OBJECT (f3button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(3));
	g_signal_connect (G_OBJECT (f4button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(4));
	g_signal_connect (G_OBJECT (f5button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(5));
	g_signal_connect (G_OBJECT (f6button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(6));
	g_signal_connect (G_OBJECT (f7button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(7));
	g_signal_connect (G_OBJECT (f8button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(8));
	g_signal_connect (G_OBJECT (f9button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(9));
	g_signal_connect (G_OBJECT (f10button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(10));
	g_signal_connect (G_OBJECT (f11button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(11));
	g_signal_connect (G_OBJECT (f12button), "clicked", G_CALLBACK (cw), GINT_TO_POINTER(12));
	g_signal_connect (G_OBJECT (stopbutton), "clicked", G_CALLBACK (stop), NULL);
	g_signal_connect (G_OBJECT (wpm), "value_changed", G_CALLBACK (change_wpm), NULL);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(keyertext));
	g_signal_connect (G_OBJECT(buffer), "insert-text", G_CALLBACK (send_text), NULL);
	g_signal_connect (G_OBJECT(mode), "changed", G_CALLBACK (mode_change), NULL);
	g_signal_connect ((gpointer) shortnr, "toggled", G_CALLBACK (on_shortnr_toggled), NULL);

	gtk_widget_grab_focus (wpm);
	gtk_widget_show_all (keyerwindow);
//	gtk_window_present (GTK_WINDOW(mainwindow));
}
