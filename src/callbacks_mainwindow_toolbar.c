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
 * callbacks_mainwindow_toolbar.c - clickable toolbar
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <hamlib/rig.h>

#include "callbacks_mainwindow_toolbar.h"
#include "callbacks_mainwindow_menu.h"
#include "support.h"
#include "cfg.h"
#include "log.h"
#include "xlog_enum.h"
#include "awards_enum.h"
#include "utils.h"
#include "main.h"
#include "dxcc.h"
#include "wwl.h"
#include "hamlib-utils.h"

extern GtkWidget *mainwindow, *scorewindow, *keyerwindow, *locatormap;
extern GtkWidget *mainnotebook;
extern preferencestype preferences;
extern gchar **bandsplit;
extern gchar **modesplit;
extern gchar **qso;
extern GList *logwindowlist;
extern gushort dxcc_w[400][MAX_BANDS + 1];
extern gushort dxcc_c[400][MAX_BANDS + 1];
extern gushort wac_w[MAX_CONTINENTS][MAX_BANDS + 1];
extern gushort wac_c[MAX_CONTINENTS][MAX_BANDS + 1];
extern gushort was_w[MAX_STATES][MAX_BANDS + 1];
extern gushort was_c[MAX_STATES][MAX_BANDS + 1];
extern gushort waz_w[MAX_ZONES][MAX_BANDS + 1];
extern gushort waz_c[MAX_ZONES][MAX_BANDS + 1];

/* add a new qso to the log */
void
on_abutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *dateentry, *gmtentry, *callentry, *rstentry, *myrstentry,
		*remtv, *bandentry, *modeentry, *bandoptionmenu,
		*modeoptionmenu, *incheckbutton, *outcheckbutton,
		*qsoframe, *endhbox, *endentry, *powerhbox, *count,
		*awardshbox, *awardsentry,
		*powerentry, *namehbox, *nameentry, *qthhbox, *qthentry,
		*locatorhbox, *locatorentry, *unknown1hbox, *unknownentry1,
		*unknown2hbox, *unknownentry2, *qslhbox, *remarksvbox, *framelabel;
	GtkTextBuffer *b;
	GtkTextIter start, end;
	gchar *temp, *label;
	gint bandindex, modeindex, i = 0, page, f, kms, l, result;
	guint st, zone, cont, iota;
	logtype *logw = NULL;
	GtkTreeIter iter, selected;
	GtkTreeModel *model;
	GtkTreePath *path;
	struct info lookup;

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
	if (page >= 0)
	{
		logw = g_list_nth_data (logwindowlist, page);

		if (logw->readonly)
		{
			update_statusbar (_("Can not add QSO, log is read-only"));
			return;
		}

		dateentry = lookup_widget (mainwindow, "dateentry");
		gmtentry = lookup_widget (mainwindow, "gmtentry");
		callentry = lookup_widget (mainwindow, "callentry");
		bandentry = lookup_widget (mainwindow, "bandentry");
		bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
		modeentry = lookup_widget (mainwindow, "modeentry");
		modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
		rstentry = lookup_widget (mainwindow, "rstentry");
		myrstentry = lookup_widget (mainwindow, "myrstentry");

		for (i = 0; i < QSO_FIELDS; i++) qso[i] = g_strdup ("");
		qso[NR] = g_strdup_printf ("%d", ++logw->qsos);
		qso[DATE] = gtk_editable_get_chars (GTK_EDITABLE (dateentry), 0, -1);
		qso[GMT] = gtk_editable_get_chars (GTK_EDITABLE (gmtentry), 0, -1);
		qso[CALL] = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
		if (preferences.bandseditbox == 0)
		{
			bandindex = gtk_combo_box_get_active (GTK_COMBO_BOX(bandoptionmenu));
			qso[BAND] = lookup_band (bandindex);
		}
		else
			qso[BAND] = gtk_editable_get_chars (GTK_EDITABLE (bandentry), 0, -1);
		if (preferences.modeseditbox == 0)
		{
			modeindex = gtk_combo_box_get_active (GTK_COMBO_BOX(modeoptionmenu));
			qso[MODE] = lookup_mode (modeindex);
		}
		else
			qso[MODE] = gtk_editable_get_chars (GTK_EDITABLE (modeentry), 0, -1);

		powerhbox = lookup_widget (mainwindow, "powerhbox");
		powerentry = lookup_widget (mainwindow, "powerentry");
		if (gtk_widget_get_visible (powerhbox))
		{
			qso[POWER] = gtk_editable_get_chars (GTK_EDITABLE (powerentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (powerentry), 0, -1);
		}

		qso[RST] = gtk_editable_get_chars (GTK_EDITABLE (rstentry), 0, -1);
		qso[MYRST] = gtk_editable_get_chars (GTK_EDITABLE (myrstentry), 0, -1);

		endhbox = lookup_widget (mainwindow, "endhbox");
		if (gtk_widget_get_visible (endhbox))
		{
			endentry = lookup_widget (mainwindow, "endentry");
			qso[GMTEND] = gtk_editable_get_chars (GTK_EDITABLE (endentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (endentry), 0, -1);
		}

		awardshbox = lookup_widget (mainwindow, "awardshbox");
		if (gtk_widget_get_visible (awardshbox))
		{
			awardsentry = lookup_widget (mainwindow, "awardsentry");
			qso[AWARDS] = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (awardsentry), 0, -1);
		}

		qslhbox = lookup_widget (mainwindow, "qslhbox");
		if (gtk_widget_get_visible (qslhbox))
		{
			incheckbutton = lookup_widget (mainwindow, "incheckbutton");
			outcheckbutton = lookup_widget (mainwindow, "outcheckbutton");
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (outcheckbutton)))
				qso[QSLOUT] = g_strdup ("X");
			else
				qso[QSLOUT] = g_strdup ("");
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (incheckbutton)))
				qso[QSLIN] = g_strdup ("X");
			else
				qso[QSLIN] = g_strdup ("");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outcheckbutton),	FALSE);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (incheckbutton), FALSE);
		}

		namehbox = lookup_widget (mainwindow, "namehbox");
		if (gtk_widget_get_visible (namehbox))
		{
			nameentry = lookup_widget (mainwindow, "nameentry");
			qso[NAME] =	gtk_editable_get_chars (GTK_EDITABLE (nameentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (nameentry), 0, -1);
		}

		qthhbox = lookup_widget (mainwindow, "qthhbox");
		if (gtk_widget_get_visible (qthhbox))
		{
			qthentry = lookup_widget (mainwindow, "qthentry");
			qso[QTH] = gtk_editable_get_chars (GTK_EDITABLE (qthentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (qthentry), 0, -1);
		}

		locatorhbox = lookup_widget (mainwindow, "locatorhbox");
		if (gtk_widget_get_visible (locatorhbox))
		{
			locatorentry = lookup_widget (mainwindow, "locatorentry");
			qso[LOCATOR] = gtk_editable_get_chars (GTK_EDITABLE (locatorentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (locatorentry), 0, -1);
		}

		unknown1hbox = lookup_widget (mainwindow, "unknown1hbox");
		if (gtk_widget_get_visible (unknown1hbox))
		{
			unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
			qso[U1] =	gtk_editable_get_chars (GTK_EDITABLE (unknownentry1), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (unknownentry1), 0, -1);
		}

		unknown2hbox = lookup_widget (mainwindow, "unknown2hbox");
		if (gtk_widget_get_visible (unknown2hbox))
		{
			unknownentry2 = lookup_widget (mainwindow, "unknownentry2");
			qso[U2] =	gtk_editable_get_chars (GTK_EDITABLE (unknownentry2), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (unknownentry2), 0, -1);
		}

		remarksvbox = lookup_widget (mainwindow, "remarksvbox");
		if (gtk_widget_get_visible (remarksvbox))
		{
			remtv = lookup_widget (mainwindow, "remtv");
			b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
			gtk_text_buffer_get_bounds (b, &start, &end);
			qso[REMARKS] = gtk_text_buffer_get_text (b, &start, &end, TRUE);
			g_strdelimit (qso[REMARKS], "\n", ' ');
			gtk_text_buffer_set_text (b, "", 0);
		}

		/* calculate distance and azimuth */
		if ((preferences.distqrb == 1) && gtk_widget_get_visible (locatorhbox))
		{
			if (strlen(qso[LOCATOR]) >= 2)
			{
				result = locatordistance (preferences.locator, qso[LOCATOR], &kms, &l);
				if (result == 0)
				{
					if (gtk_widget_get_visible (unknown1hbox))
					{
						if (preferences.units == 1)
							qso[U1] = g_strdup_printf ("%d km", kms);
						else
							qso[U1] = g_strdup_printf ("%d m", (gint) (kms/1.609));
					}
					if (gtk_widget_get_visible (unknown2hbox))
					{
						qso[U2] = g_strdup_printf ("%d deg", l);
					}
				}
			}

		}

		/* add the QSO */
		model = gtk_tree_view_get_model (GTK_TREE_VIEW(logw->treeview));
		gtk_list_store_prepend (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, NR, qso[NR],
			DATE, qso[DATE], GMT, qso[GMT], GMTEND, qso[GMTEND], CALL, qso[CALL],
			BAND, qso[BAND], MODE, qso[MODE], RST, qso[RST], MYRST, qso[MYRST], AWARDS, qso[AWARDS],
			QSLOUT, qso[QSLOUT], QSLIN, qso[QSLIN], POWER, qso[POWER], NAME, qso[NAME],
			QTH, qso[QTH], LOCATOR, qso[LOCATOR], U1, qso[U1], U2, qso[U2],
			REMARKS, qso[REMARKS], -1);

		/* scroll there */
		path = gtk_tree_path_new_from_string ("0");
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(logw->treeview), path, NULL, TRUE, 1.0, 0.0);
		gtk_tree_path_free (path);

		gtk_editable_delete_text (GTK_EDITABLE (gmtentry), 0, -1);
		gtk_editable_delete_text (GTK_EDITABLE (callentry), 0, -1);
		if (preferences.modeseditbox == 1)
			gtk_editable_delete_text (GTK_EDITABLE (modeentry), 0, -1);
		if (preferences.bandseditbox == 1)
		gtk_editable_delete_text (GTK_EDITABLE (bandentry), 0, -1);
		gtk_editable_delete_text (GTK_EDITABLE (rstentry), 0, -1);
		gtk_editable_delete_text (GTK_EDITABLE (myrstentry), 0, -1);

		/* unselect any selected QSO */
		if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW(logw->treeview)), &model, &selected))
			gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (logw->treeview)));

		if (gtk_widget_get_visible(scorewindow))
		{
			/* update worked/confirmed array */
			gchar *result = valid_awards_entry (qso[AWARDS], &st, &zone, &cont, &iota);
			if (result)
			{
				lookup = lookupcountry_by_prefix (result);
				g_free (result);
			}
			else
				lookup = lookupcountry_by_callsign (qso[CALL]);
			f = freq2enum (qso[BAND]);
			gboolean qslconfirmed = qslreceived (qso[QSLIN]);
			if (preferences.scoringbands[f] == 1)
			{
				if (lookup.country > 0)
				{
					dxcc_w[lookup.country][f]++;
					dxcc_w[lookup.country][MAX_BANDS]++;
					if (qslconfirmed)
					{
						dxcc_c[lookup.country][f]++;
						dxcc_c[lookup.country][MAX_BANDS]++;
					}
					if (dxcc_w[lookup.country][f] == 1 || dxcc_c[lookup.country][f] == 1)
						update_dxccscoring ();
				}
				if (cont != 99 || lookup.continent != 99)
				{
					if (cont == 99) cont = lookup.continent;
					wac_w[cont][f]++;
					wac_w[cont][MAX_BANDS]++;
					if (qslconfirmed)
					{
						wac_c[cont][f]++;
						wac_c[cont][MAX_BANDS]++;
					}
					if ((preferences.awardswac == 1) &&
						(wac_w[cont][f] == 1 || wac_c[cont][f] == 1))
						update_wacscoring ();
				}
				if (st != 99)
				{
					was_w[st][f]++;
					was_w[st][MAX_BANDS]++;
					if (qslconfirmed)
					{
						was_c[st][f]++;
						was_c[st][MAX_BANDS]++;
					}
					if ((preferences.awardswas == 1) &&
						(was_w[st][f] == 1 || was_c[st][f] == 1))
						update_wasscoring ();
				}
				if ((zone > 0 && zone < 99) || (lookup.cq > 0 && lookup.cq < 99))
				{
					if (zone == 99) zone = lookup.cq;
					waz_w[zone-1][f]++;
					waz_w[zone-1][MAX_BANDS]++;
					if (qslconfirmed)
					{
						waz_c[zone-1][f]++;
						waz_c[zone-1][MAX_BANDS]++;
					}
					if ((preferences.awardswaz == 1) &&
						(waz_w[zone-1][f] == 1 || waz_c[zone-1][f] == 1))
						update_wazscoring ();
				}
				if (iota != NOT_AN_IOTA)
				{
					iota_new_qso(iota, f, qslconfirmed);
					if ((preferences.awardsiota == 1))
						update_iotascoring ();
				}
				if (qso[LOCATOR] && (strlen(qso[LOCATOR]) > 0))
				{
					loc_new_qso(qso[LOCATOR], f, qslconfirmed);
					if ((preferences.awardsloc == 1))
						update_locscoring ();
				}
			}
		}
		else
		{
			if (locatormap)
			{
				if (qso[LOCATOR] && (strlen(qso[LOCATOR]) > 0))
				{
					gboolean qslconfirmed = qslreceived (qso[QSLIN]);
					f = freq2enum (qso[BAND]);
					loc_new_qso(qso[LOCATOR], f, qslconfirmed);
					if ((preferences.awardsloc == 1))
						update_locscoring ();
				}
			}
		}

		if (locatormap)
		{
			GtkWidget *da= lookup_widget (locatormap, "drawingarea");
			g_signal_emit_by_name(da, "configure_event");
		}

		if (preferences.saving == 2)
		{
			savelog (logw, logw->filename, TYPE_FLOG, 1, logw->qsos);
			logw->logchanged = FALSE;
			temp = g_strdup_printf (_("QSO %s added to %s log, log saved"), qso[NR], logw->logname);
		}
		else
		{
			temp = g_strdup_printf (_("QSO %s added to %s log"), qso[NR], logw->logname);
			logw->logchanged = TRUE;
			label = g_strdup_printf ("<b>%s*</b>", logw->logname);
			gtk_label_set_markup (GTK_LABEL (logw->label), label);
			g_free (label);
		}
		update_statusbar (temp);
		g_free (temp);

		qsoframe = lookup_widget (mainwindow, "qsoframe");
		framelabel = gtk_frame_get_label_widget (GTK_FRAME(qsoframe));
		temp = g_strdup_printf ("<b>%s</b>", _("New QSO"));
		gtk_label_set_markup (GTK_LABEL (framelabel), temp);

		gtk_widget_grab_focus (callentry);

		if (keyerwindow)
		{	/* increment counter for next QSO */
			count = lookup_widget (keyerwindow, "count");
			i = gtk_spin_button_get_value (GTK_SPIN_BUTTON (count));
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (count), i + 1);
		}
	}
}


/* delete a qso from the log */
void
on_dbutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *gmtbutton, *gmtentry, *endentry, *callentry, *bandentry,
		*modeentry, *rstentry, *myrstentry, *remtv, *powerentry,
		*nameentry, *qthentry, *unknownentry1, *unknownentry2,
		*locatorentry, *incheckbutton, *outcheckbutton, *awardsentry;
	GtkTextBuffer *b;
	gchar *qsonumber, *label, *call, *freq, *awards, *temp, *locator;
	gboolean qslin;
	gint i, page, f;
	guint st, zone, cont, iota;
	logtype *logw;
	GtkTreeIter selected, iter;
	GtkTreeModel *model;
	struct info lookup;

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));
	if (page >= 0)
	{
		logw = g_list_nth_data (logwindowlist, page);
		if (logw->readonly)
		{
			update_statusbar (_("Can not delete QSO, log is read-only"));
			return;
		}
		if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection
			(GTK_TREE_VIEW(logw->treeview)), &model, &selected))
		{
			gtk_tree_model_get (model, &selected, NR, &qsonumber, -1);
			temp = g_strdup_printf (_("QSO %s deleted, %s log renumbered"),
				qsonumber, logw->logname);
			update_statusbar (temp);

			gmtentry = lookup_widget (mainwindow, "gmtentry");
			endentry = lookup_widget (mainwindow, "endentry");
			callentry = lookup_widget (mainwindow, "callentry");
			bandentry = lookup_widget (mainwindow, "bandentry");
			modeentry = lookup_widget (mainwindow, "modeentry");
			rstentry = lookup_widget (mainwindow, "rstentry");
			myrstentry = lookup_widget (mainwindow, "myrstentry");
			awardsentry = lookup_widget (mainwindow, "awardsentry");
			remtv = lookup_widget (mainwindow, "remtv");
			incheckbutton = lookup_widget (mainwindow, "incheckbutton");
			outcheckbutton = lookup_widget (mainwindow, "outcheckbutton");
			powerentry = lookup_widget (mainwindow, "powerentry");
			nameentry = lookup_widget (mainwindow, "nameentry");
			qthentry = lookup_widget (mainwindow, "qthentry");
			locatorentry = lookup_widget (mainwindow, "locatorentry");
			unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
			unknownentry2 = lookup_widget (mainwindow, "unknownentry2");
			gtk_editable_delete_text (GTK_EDITABLE (gmtentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (endentry), 0, -1);

			call = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (callentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (modeentry), 0, -1);

			freq = gtk_editable_get_chars (GTK_EDITABLE (bandentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (bandentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (rstentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (myrstentry), 0, -1);

			awards = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (awardsentry), 0, -1);
			b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
			gtk_text_buffer_set_text (b, "", 0);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outcheckbutton), FALSE);
			qslin = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (incheckbutton));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (incheckbutton), FALSE);
			gtk_editable_delete_text (GTK_EDITABLE (powerentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (nameentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (qthentry), 0, -1);

			locator = gtk_editable_get_chars (GTK_EDITABLE (locatorentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (locatorentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (unknownentry1), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (unknownentry2), 0, -1);

			/* now remove the QSO from the log */
			gtk_list_store_remove (GTK_LIST_STORE (model), &selected);
			logw->qsos--;

			if (preferences.saving != 2)
			{
				logw->logchanged = TRUE;
				label = g_strdup_printf ("<b>%s*</b>", logw->logname);
				gtk_label_set_markup (GTK_LABEL (logw->label), label);
				g_free (label);
			}

			/* update the first column with the number of qso's */
			gtk_tree_model_get_iter_first (model, &iter);
			for (i = 0; i < logw->qsos; i++)
			{
				qsonumber = g_strdup_printf ("%d", logw->qsos - i);
				gtk_list_store_set (GTK_LIST_STORE(model), &iter, NR, qsonumber, -1);
				g_free (qsonumber);
				if (!(i % 50)) /* needed for responsiveness */
				{
					while (gtk_events_pending ())	gtk_main_iteration ();
				}
				gtk_tree_model_iter_next (model, &iter);
			}

			gmtbutton = lookup_widget (mainwindow, "gmtbutton");
			gtk_widget_grab_focus (gmtbutton);

			if (gtk_widget_get_visible(scorewindow))
			{
				/* update worked/confirmed array */
				gchar *result = valid_awards_entry (awards, &st, &zone, &cont, &iota);
				if (result)
				{
					lookup = lookupcountry_by_prefix (result);
					g_free (result);
				}
				else
				lookup = lookupcountry_by_callsign (call);

				f = freq2enum (freq);
				if (preferences.scoringbands[f] == 1)
				{
					if (lookup.country > 0)
					{
						dxcc_w[lookup.country][f]--;
						dxcc_w[lookup.country][MAX_BANDS]--;
						if (qslin)
						{
							dxcc_c[lookup.country][f]--;
							dxcc_c[lookup.country][MAX_BANDS]--;
						}
		/* check if this is the only country on this frequency */
						if (dxcc_w[lookup.country][f] == 0 ||
							dxcc_c[lookup.country][f] == 0)
							update_dxccscoring ();
					}
					if (cont != 99 || lookup.continent != 99)
					{
						if (cont == 99) cont = lookup.continent;
						wac_w[cont][f]--;
						wac_w[cont][MAX_BANDS]--;
						if (qslin)
						{
							wac_c[cont][f]--;
							wac_c[cont][MAX_BANDS]--;
						}
						if ((preferences.awardswac == 1) &&
							(wac_w[cont][f] == 0 || wac_c[cont][f] == 0))
							update_wacscoring ();
					}
					if (st != 99)
					{
						was_w[st][f]--;
						was_w[st][MAX_BANDS]--;
						if (qslin)
						{
							was_c[st][f]--;
							was_c[st][MAX_BANDS]--;
						}
						if ((preferences.awardswas == 1) &&
							(was_w[st][f] == 0 || was_c[st][f] == 0))
							update_wasscoring ();
					}
					if ((zone > 0 && zone < 99) || (lookup.cq > 0 && lookup.cq < 99))
					{
						if (zone == 99) zone = lookup.cq;
						waz_w[zone-1][f]--;
						waz_w[zone-1][MAX_BANDS]--;
						if (qslin)
						{
							waz_c[zone-1][f]--;
							waz_c[zone-1][MAX_BANDS]--;
						}
						if ((preferences.awardswaz == 1) &&
							(waz_w[zone-1][f] == 0 || waz_c[zone-1][f] == 0))
							update_wazscoring ();
					}
					if (iota != NOT_AN_IOTA)
					{
						iota_del_qso(iota, f, qslin);
						if ((preferences.awardsiota == 1))
							update_iotascoring ();
					}
					if (locator && (strlen(locator) > 0))
					{
						loc_del_qso(locator, f, qslin);
						if ((preferences.awardsloc == 1))
							update_locscoring ();
					}
				}
			}
			else
			{
				if (locatormap)
				{
					if (qso[LOCATOR] && (strlen(qso[LOCATOR]) > 0))
					{
						gboolean qslconfirmed = qslreceived (qso[QSLIN]);
						f = freq2enum (qso[BAND]);
						loc_new_qso(qso[LOCATOR], f, qslconfirmed);
						if ((preferences.awardsloc == 1))
							update_locscoring ();
					}
				}
			}
			if (locatormap)
			{
				GtkWidget *da= lookup_widget (locatormap, "drawingarea");
				g_signal_emit_by_name(da, "configure_event");
			}
			if (preferences.saving == 2)
			{
				savelog (logw, logw->filename, TYPE_FLOG, 1, logw->qsos);
				logw->logchanged = FALSE;
			}
		g_free (call);
		g_free (freq);
		g_free (awards);
		g_free (temp);
		g_free (locator);
		}
	}
}


/* update/modify a log entry */
void
on_ubutton_clicked (GtkButton * button, gpointer user_data)
{
	GtkWidget *dateentry, *gmtentry, *callentry, *rstentry, *myrstentry,
		*remtv, *bandentry, *modeentry, *incheckbutton,
		*outcheckbutton, *modeoptionmenu, *bandoptionmenu,
		*qsoframe, *endhbox, *endentry, *powerhbox, *awardshbox, *awardsentry,
		*powerentry, *namehbox, *nameentry, *qthhbox, *qthentry,
		*locatorhbox, *locatorentry, *unknown1hbox, *unknownentry1,
		*unknown2hbox, *unknownentry2, *qslhbox, *remarksvbox, *gmtbutton,
		*framelabel;
	GtkTextBuffer *b;
	GtkTextIter start, end;
	gint bandindex, modeindex, i = 0, page, fr, froption, result, kms, l;
	guint st, zone, cont, iota;
	gchar *temp, *label, *logcall, *logfreq, *logqslin, *logawards, *logtime,
		*logendtime, *loglocator;
	logtype *logw;
	GtkTreeIter selected;
	GtkTreeModel *model;
	struct info lookup;
	gboolean qslconfirmed;

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mainnotebook));

	if (page >= 0)
	{
		logw = g_list_nth_data (logwindowlist, page);
		if (logw->readonly)
		{
			update_statusbar (_("Can not update QSO, log is read-only"));
			return;
		}
		dateentry = lookup_widget (mainwindow, "dateentry");
		gmtentry = lookup_widget (mainwindow, "gmtentry");
		callentry = lookup_widget (mainwindow, "callentry");
		bandentry = lookup_widget (mainwindow, "bandentry");
		modeentry = lookup_widget (mainwindow, "modeentry");
		rstentry = lookup_widget (mainwindow, "rstentry");
		myrstentry = lookup_widget (mainwindow, "myrstentry");
		modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
		bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");

		for (i = 0; i < QSO_FIELDS; i++)
			qso[i] = g_strdup ("");

		/* now retrieve new logdata from the qsoframe, we need to keep the
		 * call, frequency, qslin, awards and locator from the log to update
		 * scoring and/or the locator map further down */

		if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection
			(GTK_TREE_VIEW(logw->treeview)), &model, &selected))
		{
			gtk_tree_model_get (model, &selected, NR, &qso[NR], -1);

			qso[DATE] = gtk_editable_get_chars (GTK_EDITABLE (dateentry), 0, -1);

			/* do not touch time if hours and seconds are equal */
			gtk_tree_model_get (model, &selected, GMT, &logtime, -1);
			qso[GMT] = gtk_editable_get_chars (GTK_EDITABLE (gmtentry), 0, -1);
			if (!g_ascii_strncasecmp (logtime, qso[GMT], 4))
				qso[GMT] = g_strdup (logtime);

			gtk_tree_model_get (model, &selected, CALL, &logcall, -1);
			qso[CALL] = gtk_editable_get_chars (GTK_EDITABLE (callentry), 0, -1);

			gtk_tree_model_get (model, &selected, BAND, &logfreq, -1);
			if (preferences.bandseditbox == 1)
			{
				qso[BAND] = gtk_editable_get_chars (GTK_EDITABLE (bandentry), 0, -1);
			}
			else
			{
				fr = freq2enum (logfreq);
				bandindex = gtk_combo_box_get_active (GTK_COMBO_BOX(bandoptionmenu));
				qso[BAND] = lookup_band (bandindex);
				/* do not touch freq in the log if it's the same as optionmenu */
				froption = freq2enum (qso[BAND]);
				if (fr == froption) qso[BAND] = g_strdup (logfreq);
			}

			if (preferences.modeseditbox == 1)
			{
				qso[MODE] = gtk_editable_get_chars (GTK_EDITABLE (modeentry), 0, -1);
			}
			else
			{
				modeindex = gtk_combo_box_get_active (GTK_COMBO_BOX(modeoptionmenu));
				qso[MODE] = lookup_mode (modeindex);
			}

			awardshbox = lookup_widget (mainwindow, "awardshbox");
			if (gtk_widget_get_visible (awardshbox))
			{
				gtk_tree_model_get (model, &selected, AWARDS, &logawards, -1);
				awardsentry = lookup_widget (mainwindow, "awardsentry");
				qso[AWARDS] = gtk_editable_get_chars (GTK_EDITABLE (awardsentry), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (awardsentry), 0, -1);
			}
			else
				logawards = g_strdup ("");

			powerhbox = lookup_widget (mainwindow, "powerhbox");
			if (gtk_widget_get_visible (powerhbox))
			{
				powerentry = lookup_widget (mainwindow, "powerentry");
				qso[POWER] = gtk_editable_get_chars (GTK_EDITABLE (powerentry), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (powerentry), 0, -1);
			}

			qso[RST] = gtk_editable_get_chars (GTK_EDITABLE (rstentry), 0, -1);
			qso[MYRST] = gtk_editable_get_chars (GTK_EDITABLE (myrstentry), 0, -1);

			endhbox = lookup_widget (mainwindow, "endhbox");
			if (gtk_widget_get_visible (endhbox))
			{
				gtk_tree_model_get (model, &selected, GMTEND, &logendtime, -1);
				endentry = lookup_widget (mainwindow, "endentry");
				qso[GMTEND] = gtk_editable_get_chars (GTK_EDITABLE (endentry), 0, -1);
				if (!g_ascii_strncasecmp (logendtime, qso[GMTEND], 4))
					qso[GMTEND] = g_strdup (logendtime);
				gtk_editable_delete_text (GTK_EDITABLE (endentry), 0, -1);
			}

			qslhbox = lookup_widget (mainwindow, "qslhbox");
			if (gtk_widget_get_visible (qslhbox))
			{
				gtk_tree_model_get (model, &selected, QSLIN, &logqslin, -1);
				incheckbutton = lookup_widget (mainwindow, "incheckbutton");
				outcheckbutton = lookup_widget (mainwindow, "outcheckbutton");
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (outcheckbutton)))
					qso[QSLOUT] = g_strdup ("X");
				else
					qso[QSLOUT] = g_strdup ("");
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (incheckbutton)))
					qso[QSLIN] = g_strdup ("X");
				else
					qso[QSLIN] = g_strdup ("");
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON	(outcheckbutton), FALSE);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (incheckbutton), FALSE);
			}
			else
				logqslin = g_strdup ("");

			namehbox = lookup_widget (mainwindow, "namehbox");
			if (gtk_widget_get_visible (namehbox))
			{
				nameentry = lookup_widget (mainwindow, "nameentry");
				qso[NAME] = gtk_editable_get_chars (GTK_EDITABLE (nameentry), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (nameentry), 0, -1);
			}

			qthhbox = lookup_widget (mainwindow, "qthhbox");
			if (gtk_widget_get_visible (qthhbox))
			{
				qthentry = lookup_widget (mainwindow, "qthentry");
				qso[QTH] = gtk_editable_get_chars (GTK_EDITABLE (qthentry), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (qthentry), 0, -1);
			}

			locatorhbox = lookup_widget (mainwindow, "locatorhbox");
			if (gtk_widget_get_visible (locatorhbox))
			{
				gtk_tree_model_get (model, &selected, LOCATOR, &loglocator, -1);
				locatorentry = lookup_widget (mainwindow, "locatorentry");
				qso[LOCATOR] = gtk_editable_get_chars (GTK_EDITABLE (locatorentry), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (locatorentry), 0, -1);
			}

			unknown1hbox = lookup_widget (mainwindow, "unknown1hbox");
			if (gtk_widget_get_visible (unknown1hbox))
			{
				unknownentry1 = lookup_widget (mainwindow, "unknownentry1");
				qso[U1] = gtk_editable_get_chars (GTK_EDITABLE (unknownentry1), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (unknownentry1), 0, -1);
			}

			unknown2hbox = lookup_widget (mainwindow, "unknown2hbox");
			if (gtk_widget_get_visible (unknown2hbox))
			{
				unknownentry2 = lookup_widget (mainwindow, "unknownentry2");
				qso[U2] = gtk_editable_get_chars (GTK_EDITABLE (unknownentry2), 0, -1);
				gtk_editable_delete_text (GTK_EDITABLE (unknownentry2), 0, -1);
			}

			remarksvbox = lookup_widget (mainwindow, "remarksvbox");
			if (gtk_widget_get_visible (remarksvbox))
			{
				remtv = lookup_widget (mainwindow, "remtv");
				b = gtk_text_view_get_buffer (GTK_TEXT_VIEW (remtv));
				gtk_text_buffer_get_bounds (b, &start, &end);
				qso[REMARKS] = gtk_text_buffer_get_text (b, &start, &end, TRUE);
				g_strdelimit (qso[REMARKS], "\n", ' ');
				gtk_text_buffer_set_text (b, "", 0);
			}

			/* calculate distance and azimuth */
			if ((preferences.distqrb == 1) && gtk_widget_get_visible (locatorhbox))
			{
				if (strlen(qso[LOCATOR]) >= 2)
				{
					result = locatordistance (preferences.locator, qso[LOCATOR], &kms, &l);
					if (result == 0)
					{
						if (gtk_widget_get_visible (unknown1hbox))
						{
							if (preferences.units == 1)
								qso[U1] = g_strdup_printf ("%d km", kms);
							else
								qso[U1] = g_strdup_printf ("%d m", (gint) (kms/1.609));
						}
						if (gtk_widget_get_visible (unknown2hbox))
						{
							qso[U2] = g_strdup_printf ("%d deg", l);
						}
					}
				}

			}

			/* change the selected QSO */
			gtk_list_store_set (GTK_LIST_STORE(model), &selected, NR, qso[NR],
				DATE, qso[DATE], GMT, qso[GMT], GMTEND, qso[GMTEND], CALL, qso[CALL],
				BAND, qso[BAND], MODE, qso[MODE], RST, qso[RST], MYRST, qso[MYRST], AWARDS, qso[AWARDS],
				QSLOUT, qso[QSLOUT], QSLIN, qso[QSLIN], POWER, qso[POWER], NAME, qso[NAME],
				QTH, qso[QTH], LOCATOR, qso[LOCATOR], U1, qso[U1], U2, qso[U2],
				REMARKS, qso[REMARKS], -1);

			gtk_tree_selection_unselect_all
				(gtk_tree_view_get_selection (GTK_TREE_VIEW (logw->treeview)));

			gtk_editable_delete_text (GTK_EDITABLE (gmtentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (callentry), 0, -1);
			if (preferences.modeseditbox == 1)
				gtk_editable_delete_text (GTK_EDITABLE (modeentry), 0, -1);
			if (preferences.bandseditbox == 1)
				gtk_editable_delete_text (GTK_EDITABLE (bandentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (rstentry), 0, -1);
			gtk_editable_delete_text (GTK_EDITABLE (myrstentry), 0, -1);

			qsoframe = lookup_widget (mainwindow, "qsoframe");
			framelabel = gtk_frame_get_label_widget (GTK_FRAME(qsoframe));
			temp = g_strdup_printf ("<b>%s</b>", _("New QSO"));
			gtk_label_set_markup (GTK_LABEL (framelabel), temp);

			if (gtk_widget_get_visible(scorewindow))
			{ /* first we delete, then we add */
				gchar *result = valid_awards_entry (logawards, &st, &zone, &cont, &iota);
				if (result)
				{
					lookup = lookupcountry_by_prefix (result);
					g_free (result);
				}
				else
				lookup = lookupcountry_by_callsign (logcall);

				fr = freq2enum (logfreq);
				qslconfirmed = qslreceived (logqslin);
				if (preferences.scoringbands[fr] == 1)
				{
					if (lookup.country > 0)
					{
						dxcc_w[lookup.country][fr]--;
						dxcc_w[lookup.country][MAX_BANDS]--;
						if (qslconfirmed)
						{
							dxcc_c[lookup.country][fr]--;
							dxcc_c[lookup.country][MAX_BANDS]--;
						}
					}
					if (cont != 99 || lookup.continent != 99)
					{
						if (cont == 99) cont = lookup.continent;
						wac_w[cont][fr]--;
						wac_w[cont][MAX_BANDS]--;
						if (qslconfirmed)
						{
							wac_c[cont][fr]--;
							wac_c[cont][MAX_BANDS]--;
						}
					}
					if (st != 99)
					{
						was_w[st][fr]--;
						was_w[st][MAX_BANDS]--;
						if (qslconfirmed)
						{
							was_c[st][fr]--;
							was_c[st][MAX_BANDS]--;
						}
					}
					if ((zone > 0 && zone < 99) || (lookup.cq > 0 && lookup.cq < 99))
					{
						if (zone == 99) zone = lookup.cq;
						waz_w[zone-1][fr]--;
						waz_w[zone-1][MAX_BANDS]--;
						if (qslconfirmed)
						{
							waz_c[zone-1][fr]--;
							waz_c[zone-1][MAX_BANDS]--;
						}
					}
					if (iota != NOT_AN_IOTA)
						iota_del_qso(iota, fr, qslconfirmed);
					if (qso[LOCATOR] && (strlen(qso[LOCATOR]) > 0))
						loc_del_qso(loglocator, fr, qslconfirmed);
				}

				result = valid_awards_entry (qso[AWARDS], &st, &zone, &cont, &iota);
				if (result)
				{
					lookup = lookupcountry_by_prefix (result);
					g_free (result);
				}
				else
				lookup = lookupcountry_by_callsign (qso[CALL]);

				fr = freq2enum (qso[BAND]);
				qslconfirmed = qslreceived (qso[QSLIN]);
				if (preferences.scoringbands[fr] == 1)
				{
					if (lookup.country > 0)
					{
						dxcc_w[lookup.country][fr]++;
						dxcc_w[lookup.country][MAX_BANDS]++;
						if (qslconfirmed)
						{
							dxcc_c[lookup.country][fr]++;
							dxcc_c[lookup.country][MAX_BANDS]++;
						}
					}
					if (cont != 99 || lookup.continent != 99)
					{
						if (cont == 99) cont = lookup.continent;
						wac_w[cont][fr]++;
						wac_w[cont][MAX_BANDS]++;
						if (qslconfirmed)
						{
							wac_c[cont][fr]++;
							wac_c[cont][MAX_BANDS]++;
						}
					}
					if (st != 99)
					{
						was_w[st][fr]++;
						was_w[st][MAX_BANDS]++;
						if (qslconfirmed)
						{
							was_c[st][fr]++;
							was_c[st][MAX_BANDS]++;
						}
					}
					if ((zone > 0 && zone < 99) || (lookup.cq > 0 && lookup.cq < 99))
					{
						if (zone == 99) zone = lookup.cq;
						waz_w[zone-1][fr]++;
						waz_w[zone-1][MAX_BANDS]++;
						if (qslconfirmed)
						{
							waz_c[zone-1][fr]++;
							waz_c[zone-1][MAX_BANDS]++;
						}
					}
					if (iota != NOT_AN_IOTA)
						iota_new_qso(iota, fr, qslconfirmed);
					if (qso[LOCATOR] && (strlen(qso[LOCATOR]) > 0))
						loc_new_qso(qso[LOCATOR], fr, qslconfirmed);
				}
				/* don't bother if this is a new one or if a
					country is deleted, just update scoring */
				update_dxccscoring ();
				if (preferences.awardswac == 1) update_wacscoring ();
				if (preferences.awardswas == 1) update_wasscoring ();
				if (preferences.awardswaz == 1) update_wazscoring ();
				if (preferences.awardsiota == 1) update_iotascoring ();
				if (preferences.awardsloc == 1) update_locscoring ();
			}
			else
			{
				if (locatormap)
				{
					if (qso[LOCATOR] && (strlen(qso[LOCATOR]) > 0))
					{
						gboolean qslconfirmed = qslreceived (logqslin);
						fr = freq2enum (logfreq);
						loc_del_qso(loglocator, fr, qslconfirmed);
						fr = freq2enum (qso[BAND]);
						qslconfirmed = qslreceived (qso[QSLIN]);
						loc_new_qso(qso[LOCATOR], fr, qslconfirmed);
						if ((preferences.awardsloc == 1))
							update_locscoring ();
					}
				}
			}
			if (locatormap)
			{
				GtkWidget *da= lookup_widget (locatormap, "drawingarea");
				g_signal_emit_by_name(da, "configure_event");
			}

			g_free (logcall);
			g_free (logfreq);
			g_free (logqslin);
			g_free (logawards);

			if (preferences.saving == 2)
			{
				savelog (logw, logw->filename, TYPE_FLOG, 1, logw->qsos);
				logw->logchanged = FALSE;
				temp = g_strdup_printf (_("QSO %s updated in %s log, log saved"),
					qso[NR], logw->logname);
			}
			else
			{
				logw->logchanged = TRUE;
				label = g_strdup_printf ("<b>%s*</b>", logw->logname);
				gtk_label_set_markup (GTK_LABEL (logw->label), label);
				g_free (label);
				temp = g_strdup_printf (_("QSO %s updated in %s log"),
					qso[NR], logw->logname);
			}
			update_statusbar (temp);
			g_free (temp);

			gmtbutton = lookup_widget (mainwindow, "gmtbutton");
			gtk_widget_grab_focus (gmtbutton);
		}
	}
}
