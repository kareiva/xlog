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
#include <math.h>

#include "gui_gtkprintdialog.h"
#include "cfg.h"
#include "log.h"
#include "support.h"
#include "utils.h"

extern GtkWidget *mainwindow;
extern GtkWidget *mainnotebook;
extern GList *logwindowlist;
extern preferencestype preferences;

typedef struct 
{
	gdouble font_height;
	gdouble font_width;
	gdouble top_margin;
	gdouble left_margin;
	gdouble page_height;
	gdouble page_width;
	guint lines_per_page;
	guint num_pages;
} PrintData;
GtkPageSetup *print_page_setup;
GtkPrintSettings *print_settings;
static const char *printtype = "sans 12";
static GtkWidget *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10,
	*pawards, *p11, *p12, *p13, *p14, *p15, *p16, *p17, *p18;
static gboolean boolnr, booldate, boolutc, boolend, boolcall, boolmhz,
	boolmode, booltx, boolrx, boolawards, boolqslout, boolqslin, boolpower,
	boolname, boolqth, boollocator, boolfree1, boolfree2, boolremarks;

static void
end_print (GtkPrintOperation *operation, GtkPrintContext *context,
	gpointer user_data)
{
	PrintData *data = (PrintData *)user_data;

	g_free (data);
}

static void
begin_print (GtkPrintOperation *operation, GtkPrintContext *context,
	gpointer user_data)
{
	PrintData *data = (PrintData *)user_data;
	PangoFontDescription *desc;
	PangoFontMetrics *metrics;
	PangoLayout *layout;
	logtype *logwindow;

	if (!print_settings)
		print_settings = gtk_print_settings_new ();
	if (!print_page_setup)
		print_page_setup = gtk_page_setup_new ();
	gtk_print_operation_set_print_settings (operation, print_settings);
	gtk_print_operation_set_default_page_setup (operation, print_page_setup);

	logwindow = g_list_nth_data (logwindowlist,
		gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook)));

	GtkPageOrientation orientation =
		gtk_page_setup_get_orientation (print_page_setup);
	switch (orientation) {
		case GTK_PAGE_ORIENTATION_PORTRAIT:
		case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
		data->lines_per_page = 50;
		break;
		case GTK_PAGE_ORIENTATION_LANDSCAPE:
		case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
		data->lines_per_page = 30;
		default:
		break;
	}

	data->num_pages = floor (1 + logwindow->qsos/data->lines_per_page);
	gtk_print_operation_set_n_pages (operation, data->num_pages);

	desc = pango_font_description_from_string (printtype);
	data->font_height =
		pango_units_to_double (pango_font_description_get_size (desc));

	layout = gtk_print_context_create_pango_layout (context);
	metrics = pango_context_get_metrics (pango_layout_get_context (layout),
		desc, pango_language_get_default ());
	data->font_width =
		pango_font_metrics_get_approximate_char_width (metrics)
		/ PANGO_SCALE;

	pango_font_description_free (desc);
	pango_font_metrics_unref (metrics);
	g_object_unref (layout);

	data->top_margin =
		gtk_page_setup_get_top_margin (print_page_setup, GTK_UNIT_POINTS);
	data->left_margin =
		gtk_page_setup_get_left_margin (print_page_setup, GTK_UNIT_POINTS);
	data->page_height =
		gtk_page_setup_get_page_height (print_page_setup, GTK_UNIT_POINTS);
	data->page_width =
		gtk_page_setup_get_page_width (print_page_setup, GTK_UNIT_POINTS);
}

gint
colwidth (gpointer arg, gint column, gint firstqso, gpointer user_data)
{
	PrintData *data = (PrintData *)user_data;
	gint i, lastqso, width = 0;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	gchar *pathstr, *it;

	lastqso = firstqso - data->lines_per_page;
	if (lastqso < 0) lastqso = 0;

	/* make sure the header fits */
	if (column == NR)
		width = g_utf8_strlen ("Nr", -1);
	else if (column == DATE)
		width = g_utf8_strlen ("Date", -1);
	else if (column == GMT)
		width = g_utf8_strlen ("UTC", -1);
	else if (column == GMTEND)
		width = g_utf8_strlen ("END", -1);
	else if (column == CALL)
		width = g_utf8_strlen ("Callsign", -1);
	else if (column == BAND)
		width = g_utf8_strlen ("MHz", -1);
	else if (column == MODE)
		width = g_utf8_strlen ("Mode", -1);
	else if (column == RST)
		width = g_utf8_strlen ("TX", -1);
	else if (column == MYRST)
		width = g_utf8_strlen ("RX", -1);
	else if (column == AWARDS)
		width = g_utf8_strlen ("Awards", -1);
	else if (column == QSLOUT)
		width = g_utf8_strlen ("Out", -1);
	else if (column == QSLIN)
		width = g_utf8_strlen ("In", -1);
	else if (column == POWER)
		width = g_utf8_strlen ("Pwr", -1);
	else if (column == NAME)
		width = g_utf8_strlen ("Name", -1);
	else if (column == QTH)
		width = g_utf8_strlen ("QTH", -1);
	else if (column == LOCATOR)
		width = g_utf8_strlen ("Locator", -1);
	else if (column == U1)
		width = g_utf8_strlen (preferences.freefield1, -1);
	else if (column == U2)
		width = g_utf8_strlen (preferences.freefield2, -1);
	else if (column == REMARKS)
		width = g_utf8_strlen ("Remarks", -1);
	else
		width = 4;

	logtype *logwindow = (logtype *) arg;
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(logwindow->treeview));
	for (i = firstqso; i > lastqso; i--)
	{
		pathstr = g_strdup_printf("%d", i);
		path = gtk_tree_path_new_from_string (pathstr);
		if (path)
		{
			if (gtk_tree_model_get_iter (model, &iter, path))
			{
				gtk_tree_model_get (model, &iter, column, &it, -1);
				width = g_utf8_strlen (it, -1) > width
					? g_utf8_strlen (it, -1) : width;
			}
			else
				g_warning ("gui_gtkprintdialog->colwidth->GtkTreeIter");
		}
		else
			g_warning ("gui_gtkprintdialog->colwidth->GtkTreePath");
		g_free (pathstr);
	}
	if (column == CALL)
		return (width + 4) * data->font_width;
	else
		return (width + 3) * data->font_width;
}

gboolean
column_visible (gpointer arg, gint nr)
{
	logtype *logwindow = (logtype *) arg;
	GtkTreeViewColumn *column =
		gtk_tree_view_get_column (GTK_TREE_VIEW(logwindow->treeview), nr);
	return gtk_tree_view_column_get_visible (column);
}  


static void
draw_page (GtkPrintOperation *operation, GtkPrintContext *context,
	gint page_nr, gpointer user_data)
{
	logtype *logwindow;
	cairo_t *cr;
	PangoLayout *layout;
	PangoFontDescription *desc;
	gint i, j, left, linecount, first, last, top = 20;
	gint wnr = 0, wdate = 0, wgmt = 0, wgmtend = 0, wcall = 0,
		wband = 0, wmode = 0, wrst = 0, wmyrst = 0, wawards = 0,
		wqslout = 0, wqslin = 0, wpower = 0, wname = 0, wqth = 0,
		wlocator = 0, wu1 = 0, wu2 = 0, wremarks = 0;
	gboolean gmtendok = FALSE, awardsok = FALSE, qsloutok = FALSE,
		qslinok = FALSE, powerok = FALSE,nameok = FALSE, qthok = FALSE,
		locatorok = FALSE, u1ok = FALSE, u2ok = FALSE, remarksok = FALSE;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	gchar *pathstr, **item;


	PrintData *data = (PrintData *)user_data;
	cr = gtk_print_context_get_cairo_context (context);

	logwindow = g_list_nth_data (logwindowlist,
		gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook)));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (logwindow->treeview));
	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		linecount = 0;
		left = data->left_margin;
		top = data->top_margin;

		layout = gtk_print_context_create_pango_layout (context);
		desc = pango_font_description_from_string (printtype);
		pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
		pango_layout_set_font_description (layout, desc);

		item = g_new0 (gchar *, QSO_FIELDS);

		/* calculate first and last QSO for this page,
		 * NB: page_nr starts at zero */
		first = logwindow->qsos - (page_nr * data->lines_per_page) -1;
		last = first - data->lines_per_page;
		if (last < 0) last = -1;

		/* print the header and calculate column width for next 50 QSO's */
		if (boolnr) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "Nr", -1);
		pango_cairo_show_layout (cr, layout);
		wnr = colwidth (logwindow, NR, first, data);
		left = left + wnr; }

		if (booldate) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "Date", -1);
		pango_cairo_show_layout (cr, layout);
		wdate = colwidth (logwindow, DATE, first, data);
		left = left + wdate; }

		
		if (boolutc) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "UTC", -1);
		pango_cairo_show_layout (cr, layout);
		wgmt = colwidth (logwindow, GMT, first, data);
		left = left + wgmt; }

		gmtendok = column_visible (logwindow, GMTEND);
		if (gmtendok)
		{
			if (boolend) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "END", -1);
			pango_cairo_show_layout (cr, layout);
			wgmtend = colwidth (logwindow, GMTEND, first, data);
			left = left + wgmtend; }
		}

		if (boolcall) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "Callsign", -1);
		pango_cairo_show_layout (cr, layout);
		wcall = colwidth (logwindow, CALL, first, data);
		left = left + wcall; }

		if (boolmhz) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "MHz", -1);
		pango_cairo_show_layout (cr, layout);
		wband = colwidth (logwindow, BAND, first, data);
		left = left + wband; }

		if (boolmode) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "Mode", -1);
		pango_cairo_show_layout (cr, layout);
		wmode = colwidth (logwindow, MODE, first, data);
		left = left + wmode; }

		if (booltx) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "TX", -1);
		pango_cairo_show_layout (cr, layout);
		wrst = colwidth (logwindow, RST, first, data);
		left = left + wrst; }

		if (boolrx) {
		cairo_move_to (cr, left, top);
		pango_layout_set_text (layout, "RX", -1);
		pango_cairo_show_layout (cr, layout);
		wmyrst = colwidth (logwindow, MYRST, first, data);
		left = left + wmyrst; }

		awardsok = column_visible (logwindow, AWARDS);
		if (awardsok)
		{
			if (boolawards) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "Awards", -1);
			pango_cairo_show_layout (cr, layout);
			wawards = colwidth (logwindow, AWARDS, first, data);
			left = left + wawards; }
		}

		qsloutok = column_visible (logwindow, QSLOUT);
		if (qsloutok)
		{
			if (boolqslout) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "Out", -1);
			pango_cairo_show_layout (cr, layout);
			wqslout = colwidth (logwindow, QSLOUT, first, data);
			left = left + wqslout; }
		}

		qslinok = column_visible (logwindow, QSLIN);
		if (qslinok)
		{
			if (boolqslin) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "In", -1);
			pango_cairo_show_layout (cr, layout);
			wqslin = colwidth (logwindow, QSLIN, first, data);
			left = left + wqslin; }
		}

		powerok = column_visible (logwindow, POWER);
		if (powerok)
		{
			if (boolpower) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "Pwr", -1);
			pango_cairo_show_layout (cr, layout);
			wpower = colwidth (logwindow, POWER, first, data);
			left = left + wpower; }
		}

		nameok = column_visible (logwindow, NAME);
		if (nameok)
		{
			if (boolname) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "Name", -1);
			pango_cairo_show_layout (cr, layout);
			wname = colwidth (logwindow, NAME, first, data);
			left = left + wname; }
		}

		qthok = column_visible (logwindow, QTH);
		if (qthok)
		{
			if (boolqth) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "QTH", -1);
			pango_cairo_show_layout (cr, layout);
			wqth = colwidth (logwindow, QTH, first, data);
			left = left + wqth; }
		}

		locatorok = column_visible (logwindow, LOCATOR);
		if (locatorok)
		{
			if (boollocator) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, "Locator", -1);
			pango_cairo_show_layout (cr, layout);
			wlocator = colwidth (logwindow, LOCATOR, first, data);
			left = left + wlocator; }
		}

		u1ok = column_visible (logwindow, U1);
		if (u1ok)
		{
			if (boolfree1) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, preferences.freefield1, -1);
			pango_cairo_show_layout (cr, layout);
			wu1 = colwidth (logwindow, U1, first, data);
			left = left + wu1; }
		}

		u2ok = column_visible (logwindow, U2);
		if (u2ok)
		{
			if (boolfree2) {
			cairo_move_to (cr, left, top);
			pango_layout_set_text (layout, preferences.freefield2, -1);
			pango_cairo_show_layout (cr, layout);
			wu2 = colwidth (logwindow, U2, first, data);
			left = left + wu2; }
		}

		remarksok = column_visible (logwindow, REMARKS);
		if (remarksok)
		{
			if (boolremarks) {
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, "Remarks", -1);
			pango_cairo_show_layout (cr, layout);
			wremarks = colwidth (logwindow, REMARKS, first, data);
			left = left + wremarks; }
		}

		pango_font_description_set_weight (desc, PANGO_WEIGHT_NORMAL);
		pango_layout_set_font_description (layout, desc);

		linecount = pango_layout_get_line_count (layout);
		top = top + (0.5 + linecount) * data->font_height;
		left = data->left_margin;
		cairo_set_source_rgb (cr, 0, 0, 0);
		cairo_set_line_width (cr, 1);
		cairo_move_to (cr, left, top);
		cairo_line_to (cr, data->page_width, top);
		cairo_stroke (cr);
		linecount = pango_layout_get_line_count (layout);
		top = top + linecount * data->font_height;

		/* print QSO's */
		for (i = first; i > last; i--)
		{
			pathstr = g_strdup_printf("%d", i);
			path = gtk_tree_path_new_from_string (pathstr);
			if (path) {
			if (gtk_tree_model_get_iter (model, &iter, path)) {

			for (j = 0; j < QSO_FIELDS; j++)
				gtk_tree_model_get (model, &iter, j, &item[j], -1);

			left = data->left_margin;

			if (boolnr) {
			cairo_move_to (cr, left, top);
	  		pango_layout_set_text (layout, item[NR], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wnr; }

			if (booldate) {
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, item[DATE], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wdate; }

			if (boolutc) {
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, item[GMT], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wgmt; }

			if (gmtendok)
			{
				if (boolend) {
				cairo_move_to (cr, left, top);
		  		pango_layout_set_text (layout, item[GMTEND], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wgmtend; }
			}

			if (boolcall) {
			pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
			pango_layout_set_font_description (layout, desc);
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, item[CALL], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wcall;
			pango_font_description_set_weight (desc, PANGO_WEIGHT_NORMAL);
			pango_layout_set_font_description (layout, desc); }

			if (boolmhz) {
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, item[BAND], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wband; }

			if (boolmode) {
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, item[MODE], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wmode; }

			if (booltx) {
			cairo_move_to (cr, left, top);
		  	pango_layout_set_text (layout, item[RST], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wrst; }

			if (boolrx) {
			cairo_move_to (cr, left, top);
	  		pango_layout_set_text (layout, item[MYRST], -1);
			pango_cairo_show_layout (cr, layout);
			left = left + wmyrst; }

			if (awardsok)
			{
				if (boolawards) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[AWARDS], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wawards; }
			}

			if (qslinok)
			{
				if (boolqslin) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[QSLIN], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wqslin; }
			}

			if (qsloutok)
			{
				if (boolqslout) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[QSLOUT], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wqslout; }
			}

			if (powerok)
			{
				if (boolpower) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[POWER], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wpower; }
			}

			if (nameok)
			{
				if (boolname) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[NAME], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wname; }
			}

			if (qthok)
			{
				if (boolqth) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[QTH], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wqth; }
			}

			if (locatorok)
			{
				if (boollocator) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[LOCATOR], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wlocator; }
			}

			if (u1ok)
			{
				if (boolfree1) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[U1], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wu1; }
			}

			if (u2ok)
			{
				if (boolfree2) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[U2], -1);
				pango_cairo_show_layout (cr, layout);
				left = left + wu2; }
			}

			if (remarksok)
			{
				if (boolremarks) {
				cairo_move_to (cr, left, top);
			  	pango_layout_set_text (layout, item[REMARKS], -1);
				pango_cairo_show_layout (cr, layout); }
			}

			linecount = pango_layout_get_line_count (layout);
			top = top + linecount * data->font_height;

			} else g_warning ("gui_gtkprintdialog->draw_page->GtkTreePath");
			} else g_warning ("gui_gtkprintdialog->draw_page->GtkTreeIter");

		}
		left = data->left_margin;
		top = data->page_height - 1.5 * data->font_height;
		cairo_set_source_rgb (cr, 0, 0, 0);
		cairo_set_line_width (cr, 1);
		cairo_move_to (cr, left, top);
		cairo_line_to (cr, data->page_width, top);
		cairo_stroke (cr);

		pango_font_description_set_style (desc, PANGO_STYLE_ITALIC);
		pango_layout_set_font_description (layout, desc);
		left = data->left_margin;
		top = data->page_height - 1 * data->font_height;
		cairo_move_to (cr, left, top);
		gchar *temp = g_strdup_printf
			(_("Log \"%s\" printed on %s by %s version %s"),
			logwindow->logname, xloggetdate(), PACKAGE, VERSION);
	  	pango_layout_set_text (layout, temp, -1);
	  	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
	  	pango_layout_set_width
	  		(layout, pango_units_from_double (data->page_width));
		pango_cairo_show_layout (cr, layout);

		g_free (temp);
		pango_font_description_free (desc);
		g_object_unref (layout);
	}
}

static void
custom_widget_apply (GtkPrintOperation *operation, GtkWidget *widget, gpointer user_data)
{
	boolnr = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p1));
	booldate = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p2));
	boolutc = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p3));
	boolend = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p4));
	boolcall = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p5));
	boolmhz = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p6));
	boolmode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p7));
	booltx = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p8));
	boolrx = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p9));
	boolawards = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pawards));
	boolqslout = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p10));
	boolqslin = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p11));
	boolpower = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p12));
	boolname = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p13));
	boolqth = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p14));
	boollocator = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p15));
	boolfree1 = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p16));
	boolfree2 = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p17));
	boolremarks = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(p18));
}

/*
 * This callback creates a "custom" widget that gets inserted into the
 * print operation dialog.
 */
static GtkWidget *
create_custom_widget (GtkPrintOperation *operation, PrintData *data)
{
	GtkWidget *pvbox, *columnshbox, *columnsvbox1, *columnsvbox2;
	logtype *logwindow;
	GtkTreeViewColumn *column;

	logwindow = g_list_nth_data (logwindowlist,
		gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook)));

	pvbox = gtk_vbox_new (FALSE, 0);
	columnshbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (pvbox), columnshbox);
	columnsvbox1 = gtk_vbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (columnshbox), columnsvbox1, FALSE, FALSE, 0);
	p1 = gtk_check_button_new_with_label (_("QSO Number"));
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p1, FALSE, FALSE, 0);
	p2 = gtk_check_button_new_with_label (_("Date"));
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p2, FALSE, FALSE, 0);
	p3 = gtk_check_button_new_with_label ("UTC");
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p3, FALSE, FALSE, 0);
	p4 = gtk_check_button_new_with_label (_("UTC - end"));
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p4, FALSE, FALSE, 0);
	p5 = gtk_check_button_new_with_label (_("Call"));
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p5, FALSE, FALSE, 0);
	p6 = gtk_check_button_new_with_label (_("Frequency"));
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p6, FALSE, FALSE, 0);
	p7 = gtk_check_button_new_with_label (_("Mode"));
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p7, FALSE, FALSE, 0);
	p8 = gtk_check_button_new_with_label ("TX(RST)");
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p8, FALSE, FALSE, 0);
	p9 = gtk_check_button_new_with_label ("RX(RST)");
	gtk_box_pack_start (GTK_BOX (columnsvbox1), p9, FALSE, FALSE, 0);
	columnsvbox2 = gtk_vbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (columnshbox), columnsvbox2, FALSE, FALSE, 0);
	pawards = gtk_check_button_new_with_label (_("Awards"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), pawards, FALSE, FALSE, 0);
	p10 = gtk_check_button_new_with_label (_("Qsl Out"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p10, FALSE, FALSE, 0);
	p11 = gtk_check_button_new_with_label (_("Qsl In"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p11, FALSE, FALSE, 0);
	p12 = gtk_check_button_new_with_label (_("Power"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p12, FALSE, FALSE, 0);
	p13 = gtk_check_button_new_with_label (_("Name"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p13, FALSE, FALSE, 0);
	p14 = gtk_check_button_new_with_label ("QTH");
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p14, FALSE, FALSE, 0);
	p15 = gtk_check_button_new_with_label (_("Locator"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p15, FALSE, FALSE, 0);
	p16 = gtk_check_button_new_with_label (_("Freefield1"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p16, FALSE, FALSE, 0);
	p17 = gtk_check_button_new_with_label (_("Freefield2"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p17, FALSE, FALSE, 0);
	p18 = gtk_check_button_new_with_label (_("Remarks"));
	gtk_box_pack_start (GTK_BOX (columnsvbox2), p18, FALSE, FALSE, 0);
	gtk_widget_show_all (pvbox);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), NR);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p1), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p1), FALSE);
		gtk_widget_set_sensitive (p1, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), DATE);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p2), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p2), FALSE);
		gtk_widget_set_sensitive (p2, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), GMT);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p3), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p3), FALSE);
		gtk_widget_set_sensitive (p3, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), GMTEND);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p4), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p4), FALSE);
		gtk_widget_set_sensitive (p4, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), CALL);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p5), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p5), FALSE);
		gtk_widget_set_sensitive (p5, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), BAND);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p6), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p6), FALSE);
		gtk_widget_set_sensitive (p6, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), MODE);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p7), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p7), FALSE);
		gtk_widget_set_sensitive (p7, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), RST);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p8), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p8), FALSE);
		gtk_widget_set_sensitive (p8, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), MYRST);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p9), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p9), FALSE);
		gtk_widget_set_sensitive (p9, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), AWARDS);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pawards), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pawards), FALSE);
		gtk_widget_set_sensitive (pawards, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), QSLOUT);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p10), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p10), FALSE);
		gtk_widget_set_sensitive (p10, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), QSLIN);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p11), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p11), FALSE);
		gtk_widget_set_sensitive (p11, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), POWER);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p12), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p12), FALSE);
		gtk_widget_set_sensitive (p12, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), NAME);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p13), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p13), FALSE);
		gtk_widget_set_sensitive (p13, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), QTH);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p14), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p14), FALSE);
		gtk_widget_set_sensitive (p14, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), LOCATOR);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p15), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p15), FALSE);
		gtk_widget_set_sensitive (p15, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), U1);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p16), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p16), FALSE);
		gtk_widget_set_sensitive (p16, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), U2);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p17), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p17), FALSE);
		gtk_widget_set_sensitive (p17, 0);
	}
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (logwindow->treeview), REMARKS);
	if (gtk_tree_view_column_get_visible (GTK_TREE_VIEW_COLUMN(column)))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p18), TRUE);
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(p18), FALSE);
		gtk_widget_set_sensitive (p18, 0);
	}
	return pvbox;
}

void
on_menu_print_activate (GtkMenuItem * menuitem, gpointer user_data)
{
	GtkPrintOperation *operation;
	GError *error = NULL;
	PrintData *data;

	/* first check if there is something to print */
	if (gtk_notebook_get_current_page (GTK_NOTEBOOK(mainnotebook)) == -1)
		return;

	operation = gtk_print_operation_new ();
	data = g_new0 (PrintData, 1);

	g_signal_connect (G_OBJECT (operation), "begin-print", 
				G_CALLBACK (begin_print), data);
	g_signal_connect (G_OBJECT (operation), "draw-page", 
				G_CALLBACK (draw_page), data);
	g_signal_connect (G_OBJECT (operation), "end-print", 
				G_CALLBACK (end_print), data);
	g_signal_connect (operation, "create-custom-widget",
				G_CALLBACK (create_custom_widget), data);
	g_signal_connect (operation, "custom-widget-apply",
                          G_CALLBACK (custom_widget_apply), data);

	gtk_print_operation_set_custom_tab_label (operation, _("Columns"));

	gtk_print_operation_run (operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
		GTK_WINDOW (mainwindow), &error);

	g_object_unref (operation);
	if (error)
	{
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (GTK_WINDOW (mainwindow),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			"%s", error->message);
		g_error_free (error);
		g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show (dialog); 
	}

}
