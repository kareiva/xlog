/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2008 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2016 Andy Stewart <kb1oiq@arrl.net>

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
#include <stdlib.h>
#include <string.h>
#include <hamlib/rig.h>

#include "hamlib-utils.h"
#include "cfg.h"
#include "utils.h"
#include "support.h"
#include "main.h"
#include "gui_warningdialog.h"

#include "../data/pixmaps/s0.xpm"
#include "../data/pixmaps/s1.xpm"
#include "../data/pixmaps/s2.xpm"
#include "../data/pixmaps/s3.xpm"
#include "../data/pixmaps/s4.xpm"
#include "../data/pixmaps/s5.xpm"
#include "../data/pixmaps/s6.xpm"
#include "../data/pixmaps/s7.xpm"
#include "../data/pixmaps/s8.xpm"
#include "../data/pixmaps/s9.xpm"

struct rig_id {
	const gint modelnr;
	const gchar *modelname;
};

typedef struct
{
	gchar **xpmdata;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
}
typeLevel;

typeLevel S[] = {
	{s0_xpm, NULL, NULL},
	{s1_xpm, NULL, NULL},
	{s2_xpm, NULL, NULL},
	{s3_xpm, NULL, NULL},
	{s4_xpm, NULL, NULL},
	{s5_xpm, NULL, NULL},
	{s6_xpm, NULL, NULL},
	{s7_xpm, NULL, NULL},
	{s8_xpm, NULL, NULL},
	{s9_xpm, NULL, NULL},
	{NULL, NULL, NULL}
};
extern preferencestype preferences;
extern programstatetype programstate;
extern GtkWidget *mainwindow;
gint svalue[10];
gint hamlibtimer = -1;
RIG *myrig;
GdkPixmap *pixmap = NULL;
GList *rigs = NULL;
long long setfreq = 0;

/* load xpm data from s-meter pixmaps */
void
loadsmeter (GtkWidget * widget)
{
	gint i = 0;

	while (S[i].xpmdata)
		{
			S[i].pixmap = gdk_pixmap_create_from_xpm_d (widget->window,
							&S[i].mask, NULL,
							S[i].xpmdata);
			i++;
		}
}

/* copy a s-meter pixmap to pixmap depending on the value of smax */
static void
draw_smeter (gint value)
{
	GtkWidget *drawingarea;

	drawingarea = lookup_widget (mainwindow, "smeterdrawingarea");
	/* clear background */
	gdk_draw_rectangle (pixmap, drawingarea->style->white_gc, TRUE, 0, 0,
		drawingarea->allocation.width, drawingarea->allocation.height);
	gdk_draw_drawable (pixmap, drawingarea->style->fg_gc[GTK_STATE_NORMAL], 
		S[value].pixmap, 0, 0, 0, 0, 
		drawingarea->allocation.width, drawingarea->allocation.height);
	gtk_widget_queue_draw_area (drawingarea, 0, 0, 
		drawingarea->allocation.width, drawingarea->allocation.height);
	/* see expose_event further down */
}

/* stop timer if used and close rig */
void
stop_hamlib (void)
{
	if (hamlibtimer != -1)
		g_source_remove (hamlibtimer);
	hamlibtimer = -1;
	rig_close ((RIG *) myrig);
	rig_cleanup ((RIG *) myrig);
}

/* configure the hamlib port [TODO: add more string checks] */
static gint set_conf(RIG *my_rig, gchar *parms)
{
	gchar *p, *q, *n, *conf_parms;
	gint ret;

	if (! strchr(parms, '=')) return -1;

	conf_parms = g_strdup (parms);
	p = conf_parms;
	while (p && *p != '\0') 
		{
			q = strchr(p, '=');
			if (q) *q++ = '\0';
			n = strchr(q, ',');
			if (n) *n++ = '\0';
 
			ret = rig_set_conf(my_rig, rig_token_lookup(my_rig, p), q);
			if (ret != RIG_OK) return ret;
			p = n;
		}
		return RIG_OK;
}

/* open rig and report errors, start timer when polling */
gboolean
start_hamlib (gint rigid, gchar *device, gint debugmode, gint timervalue)
{
	gint retcode;
	gchar *temp;

	rig_set_debug (debugmode);
	myrig = rig_init (rigid);
	if (strlen(preferences.rigconf) > 0)
	{
		retcode = set_conf(myrig, preferences.rigconf);
		if (retcode != RIG_OK) 
		{
 			temp = g_strdup_printf
				(_("Hamlib config parameter error: %s"), rigerror (retcode));
			update_statusbar (temp);
		}
	}
	strncpy (myrig->state.rigport.pathname, device, FILPATHLEN);
	retcode = rig_open (myrig);
	if (retcode != RIG_OK)
	{
		temp = g_strdup_printf
			(_("An error occured while opening port %s: %s"),
				myrig->state.rigport.pathname, rigerror (retcode));
		update_statusbar (temp);
		return FALSE;
	}
	else
	{
		if (timervalue == 0)
			hamlibtimer = -1;
		else
			hamlibtimer = g_timeout_add
				(timervalue, (GSourceFunc)poll_riginfo, NULL);
	}
	return TRUE;
}

/* Append a new entry in the driver list. It is called by rig_list_foreach */
static gint
riglist_make_list (const struct rig_caps *caps, gpointer data)
{
	rigs = g_list_append (rigs, (gpointer) caps);
	return 1;		/* !=0, we want them all ! */
}

/* Return a list with pointers of available drivers from hamlib */
GList *
rig_get_list (void)
{
	gint status;

	// Bringing up the "Select a Radio" GUI would work the first time,
	// but would crash if done twice in succession.  In the file
	// callbacks_preferencesdialog.c, g_list_free() was being called on 
	// this handle, which seems correct, but the list was never being
	// re-initialized.  This line appears to fix the problem.
	// AndyS 31-july-2016
	rigs = NULL;  // start with an empty list

	rig_load_all_backends ();
	status = rig_list_foreach (riglist_make_list, NULL);
	return rigs;
}

/* return string with mode */
gchar *
rigmode (gint mode)
{
	gchar *rigmode;

	switch (mode)
	{
	case RIG_MODE_AM:
	case RIG_MODE_AMS:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("AM");
		else
			rigmode = g_strdup ("A3E");
		break;
	case RIG_MODE_CW:
	case RIG_MODE_CWR:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("CW");
		else
			rigmode = g_strdup ("A1A");
		break;
	case RIG_MODE_USB:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("USB");
		else
			rigmode = g_strdup ("J3E");
		break;
	case RIG_MODE_LSB:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("LSB");
		else
			rigmode = g_strdup ("J3E");
		break;
	case RIG_MODE_SSB:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("SSB");
		else
			rigmode = g_strdup ("J3E");
		break;
	case RIG_MODE_RTTY:
	case RIG_MODE_RTTYR:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("RTTY");
		else
			rigmode = g_strdup ("F1B");
		break;
	case RIG_MODE_FM:
	case RIG_MODE_WFM:
		if (preferences.fcc == 0)
			rigmode = g_strdup ("FM");
		else
			rigmode = g_strdup ("F3E");
		break;
	default:
		rigmode = g_strdup ("UNKNOWN");
		break;
	}
	return (rigmode);
}

static void
hamlib_error (gint code)
{
	gchar *message;

	message = g_strdup_printf (_("Hamlib error %d: %s"), code,
					 rigerror (code));
	update_statusbar (message);
	g_free (message);

	if (code == -1)
	{
		warningdialog ("xlog - hamlib",
		_("There was an error while communicating with your rig, "
		"hamlib has stopped"), "gtk-dialog-warning");
		stop_hamlib();
		preferences.hamlib = 0;
	}
}

/* get power level from the rig */
void
get_powerlevel (void)
{
	gint retcode, status;
	value_t val;
	GtkWidget *powerhbox;

	powerhbox = lookup_widget (mainwindow, "powerhbox");
	if (gtk_widget_get_visible (powerhbox))
	{
		retcode =	rig_get_level (myrig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &val);
		if (retcode == RIG_OK)
		{
			status = rig_power2mW (myrig, &programstate.rigpower, val.f, programstate.rigfrequency, programstate.rigmode);
		}
		else if (retcode != -RIG_ENAVAIL)
			hamlib_error (retcode);
	}
}

/* get mode */
void
get_mode (void)
{
	gint retcode;
	rmode_t rmode;
	pbwidth_t width;

	retcode = rig_get_mode (myrig, RIG_VFO_CURR, &rmode, &width);
	if (retcode == RIG_OK)
	{
		programstate.rigmode = rmode;
	}
	else if (retcode != -RIG_ENAVAIL)
		hamlib_error (retcode);
}

/* get freq */
void
get_frequency (void)
{
	gint retcode;
	freq_t freq;
	GString *digits = g_string_new ("");
	GtkWidget *frequencylabel;
	gchar *temp;

	retcode = rig_get_freq (myrig, RIG_VFO_CURR, &freq);
	if (retcode == RIG_OK)
	{
		programstate.rigfrequency = freq;
		if ((preferences.hamlib == 2) || (preferences.hamlib == 4))
		{
			digits = convert_frequency ();
			digits = g_string_append (digits, " MHz");
			frequencylabel =
				lookup_widget (mainwindow, "frequencylabel");
			temp = g_strdup_printf ("<b>%s</b>", digits->str);
			gtk_label_set_markup (GTK_LABEL (frequencylabel), temp);
			g_free (temp);
			g_string_free (digits, TRUE);
		}
	}
	else if (retcode != -RIG_ENAVAIL)
		hamlib_error (retcode);
}

void
set_frequency (long long freq)
{
	if (preferences.polltime == 0)
	{
		rig_set_freq (myrig, RIG_VFO_CURR, freq);
		programstate.rigfrequency = freq;
	}
	else
		setfreq = freq;
}

/* are we transmitting or receiving ? */
void
get_ptt (void)
{
	gint retcode;
	ptt_t ptt;

	retcode = rig_get_ptt (myrig, RIG_VFO_CURR, &ptt);
	if (retcode == RIG_OK)
	{
		if (ptt == RIG_PTT_OFF)
			programstate.tx = FALSE;
		else
			programstate.tx = TRUE;
	}
	else if (retcode != -RIG_ENAVAIL)
		hamlib_error (retcode);
}

/* get s-meter value */
void
get_smeter (void)
{
	gint retcode, strength, spoint, i, smax = 0;

	retcode = rig_get_strength (myrig, RIG_VFO_CURR, &strength);
	if (retcode == RIG_OK)
	{
		if (strength >= 0)
			spoint = 9;
		else
			spoint = (gint) floor ((strength + 54) / 6);

		svalue[programstate.scounter] = spoint;
		programstate.scounter++;
		if (programstate.scounter == 3)
			programstate.scounter = 0;

		/* find maximum of s-meter during last 3 measurements */
		for (i = 0; i < 3; i++)
			if (svalue[i] > smax)
				smax = svalue[i];

		if ((preferences.hamlib == 3) || (preferences.hamlib == 4))
			draw_smeter (smax);

		if ((programstate.rigmode == RIG_MODE_CW)
				|| (programstate.rigmode == RIG_MODE_RTTY))
			programstate.rigrst = g_strdup_printf ("5%d9", smax);
		else
			programstate.rigrst = g_strdup_printf ("5%d", smax);
	}
	else if (retcode != -RIG_ENAVAIL)
		hamlib_error (retcode);
}

/* used by 'click all menu' when not polling */
void
get_riginfo (void)
{
	get_frequency ();
	get_mode ();
	get_ptt ();
	if (!programstate.tx)
		get_smeter ();
	else if ((preferences.hamlib == 3) || (preferences.hamlib == 4))
		draw_smeter (0);
	get_powerlevel ();
}

/* poll the rig */
gint
poll_riginfo (void)
{
	if (setfreq != 0)
	{
		if (programstate.hlcounter == 0)
			programstate.hlcounter++;
		rig_set_freq (myrig, RIG_VFO_CURR, setfreq);
		programstate.rigfrequency = setfreq;
		setfreq = 0;
		return 1;
	}

	if (programstate.hlcounter == 0)
	{
		programstate.hlcounter++;
		get_frequency ();
		return 1;
	}
	if (programstate.hlcounter == 1)
	{
		programstate.hlcounter++;
		get_mode ();
		return 1;
	}
	if (programstate.hlcounter == 2)
	{
		programstate.hlcounter++;
		get_ptt ();
		return 1;
	}
	if (programstate.hlcounter == 3)
	{
		programstate.hlcounter++;
		if (!programstate.tx)
			get_smeter ();
		else if ((preferences.hamlib == 3)
			 || (preferences.hamlib == 4))
			draw_smeter (0);
		return 1;
	}
	if (programstate.hlcounter == 4)
	{
		programstate.hlcounter = 0;
		get_powerlevel ();
		return 1;
	}
	return 1;
}


/* set appearance of some widgets dependent on preferences.hamlib */
void
sethamlibwidgets (gint status, gboolean initsmeter)
{
	GtkWidget *mhzlabel, *mhzbutton, *bandoptionmenu, *bandentry,
		*frequencyhandlebox, *modelabel, *modebutton, *drawingarea,
		*modeoptionmenu, *modeentry, *rstlabel, *rstbutton, *smeterhandlebox,
		*powerlabel, *powerbutton, *powerhbox;

	mhzlabel = lookup_widget (mainwindow, "mhzlabel");
	mhzbutton = lookup_widget (mainwindow, "mhzbutton");
	modelabel = lookup_widget (mainwindow, "modelabel");
	modebutton = lookup_widget (mainwindow, "modebutton");
	rstlabel = lookup_widget (mainwindow, "rstlabel");
	rstbutton = lookup_widget (mainwindow, "rstbutton");
	powerlabel = lookup_widget (mainwindow, "powerlabel");
	powerbutton = lookup_widget (mainwindow, "powerbutton");
	powerhbox = lookup_widget (mainwindow, "powerhbox");
	bandoptionmenu = lookup_widget (mainwindow, "bandoptionmenu");
	bandentry = lookup_widget (mainwindow, "bandentry");
	modeoptionmenu = lookup_widget (mainwindow, "modeoptionmenu");
	modeentry = lookup_widget (mainwindow, "modeentry");
	frequencyhandlebox = lookup_widget (mainwindow, "frequencyhandlebox");
	smeterhandlebox = lookup_widget (mainwindow, "smeterhandlebox");
	drawingarea = lookup_widget (mainwindow, "smeterdrawingarea");

	if (status == 0)	/* hamlib disabled */
	{
		gtk_widget_show (mhzlabel);
		gtk_widget_hide (mhzbutton);
		gtk_widget_show (modelabel);
		gtk_widget_hide (modebutton);
		gtk_widget_show (rstlabel);
		gtk_widget_hide (rstbutton);
		if (gtk_widget_get_visible (powerhbox))
		{
			gtk_widget_show (powerlabel);
			gtk_widget_hide (powerbutton);
		}
		gtk_widget_hide (frequencyhandlebox);
		gtk_widget_hide (smeterhandlebox);
	}
	else if (status == 1)	/* hamlib enabled */
	{
		gtk_widget_hide (mhzlabel);
		gtk_widget_show (mhzbutton);
		gtk_widget_hide (modelabel);
		gtk_widget_show (modebutton);
		gtk_widget_hide (rstlabel);
		gtk_widget_show (rstbutton);
		if (gtk_widget_get_visible (powerhbox))
		{
			gtk_widget_hide (powerlabel);
			gtk_widget_show (powerbutton);
		}
		gtk_widget_hide (frequencyhandlebox);
		gtk_widget_hide (smeterhandlebox);
	}
	else if (status == 2)	/* hamlib enabled with frequency on statusbar */
	{
		gtk_widget_hide (mhzlabel);
		gtk_widget_show (mhzbutton);
		gtk_widget_hide (modelabel);
		gtk_widget_show (modebutton);
		gtk_widget_hide (rstlabel);
		gtk_widget_show (rstbutton);
		if (gtk_widget_get_visible (powerhbox))
		{
			gtk_widget_hide (powerlabel);
			gtk_widget_show (powerbutton);
		}
		gtk_widget_show (frequencyhandlebox);
		gtk_widget_hide (smeterhandlebox);
	}
	else if (status == 3)	/* hamlib enabled with s-meter on statusbar */
	{
		gtk_widget_hide (mhzlabel);
		gtk_widget_show (mhzbutton);
		gtk_widget_hide (modelabel);
		gtk_widget_show (modebutton);
		gtk_widget_hide (rstlabel);
		gtk_widget_show (rstbutton);
		if (gtk_widget_get_visible (powerhbox))
		{
			gtk_widget_hide (powerlabel);
			gtk_widget_show (powerbutton);
		}
		gtk_widget_hide (frequencyhandlebox);
		gtk_widget_show (smeterhandlebox);
		if (initsmeter)
		{
			drawingarea = lookup_widget (mainwindow, "smeterdrawingarea");
			loadsmeter (drawingarea);
		}
	}
	else if (status == 4)
	{			/* hamlib enabled with frequency and s-meter on statusbar */
		gtk_widget_hide (mhzlabel);
		gtk_widget_show (mhzbutton);
		gtk_widget_hide (modelabel);
		gtk_widget_show (modebutton);
		gtk_widget_hide (rstlabel);
		gtk_widget_show (rstbutton);
		if (gtk_widget_get_visible (powerhbox))
		{
			gtk_widget_hide (powerlabel);
			gtk_widget_show (powerbutton);
		}
		gtk_widget_show (frequencyhandlebox);
		gtk_widget_show (smeterhandlebox);
		if (initsmeter)
		{
			drawingarea = lookup_widget (mainwindow, "smeterdrawingarea");
			loadsmeter (drawingarea);
		}
	}
}

GString *convert_frequency (void)
{
	GString *digits = g_string_new ("");

	if (preferences.round == 0)
		g_string_printf (digits, "%Ld", programstate.rigfrequency);
	else
		g_string_printf (digits, "%Ld", (long long) rint (programstate.rigfrequency /
			pow (10, preferences.round)));
	g_string_insert_c (digits, (digits->len) - 6 + preferences.round, '.');
	g_strstrip (digits->str);
	g_strdelimit (digits->str, " ", '0');
	return digits;
}

/* load s-meter pixmaps when main window is displayed */
void
on_mainwindow_show (GtkWidget * widget, gpointer user_data)
{
	GtkWidget *drawingarea;

	if ((preferences.hamlib == 3) || (preferences.hamlib == 4))
		{
			drawingarea = lookup_widget (mainwindow, "smeterdrawingarea");
			loadsmeter (drawingarea);
			draw_smeter (0);
		}
}

/* create a new backing pixmap for the s-meter whenever the window is resized */
gboolean
on_smeterdrawingarea_configure_event (GtkWidget * widget,
	GdkEventConfigure * event, gpointer user_data)
{
	if ((preferences.hamlib == 3) || (preferences.hamlib == 4))
		{
			if (pixmap)	g_object_unref (pixmap);
			pixmap = gdk_pixmap_new (widget->window, 
				widget->allocation.width, widget->allocation.height, -1);
		}
	return FALSE;
}

/* copy the background pixmap to the drawing area for the s-meter */
gboolean
on_smeterdrawingarea_expose_event (GtkWidget * widget,
	GdkEventExpose * event, gpointer user_data)
{
	if ((preferences.hamlib == 3) || (preferences.hamlib == 4))
		gdk_draw_drawable (widget->window,
			widget->style->fg_gc[gtk_widget_get_state (widget)], pixmap,
			event->area.x, event->area.y, event->area.x,
			event->area.y, event->area.width, event->area.height);
	return FALSE;
}
