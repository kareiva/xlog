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
 * wwl.c - distance / bearing calculation and update of the locatorframe
 */

#include <string.h>
#include <gtk/gtk.h>
#include <hamlib/rotator.h>

#include "wwl.h"
#include "support.h"
#include "utils.h"
#include "cfg.h"

extern preferencestype preferences;
extern GtkWidget *scorewindow;


/* 
 * distance function for QRA locators
 * 
 * description: calculates distance between 2 locators
 * 
 * arguments: from and to, p and l are pointers to the distance in kilometers
 *            and azimuth
 * returns: 1 on error, 0 on succes
 * 
 * */
gint
locatordistance (gchar *my, gchar *dx, gint *dist, gint *bearing)
{
	gint retcode;
	gdouble latmy, lonmy, latdx, londx, d, b;
     
	retcode = locator2longlat (&lonmy, &latmy, my);
	if (retcode == RIG_OK)
	{
		retcode = locator2longlat (&londx, &latdx, dx);
		if (retcode == RIG_OK)
		{
#ifdef G_OS_WIN32
			retcode = myqrb (lonmy, latmy, londx, latdx, &d, &b);
#else
			retcode = qrb (lonmy, latmy, londx, latdx, &d, &b);
#endif
			if (retcode == RIG_OK)
			{
				*dist = d;
				*bearing = b;
				return 0;
			}
		}
	}
	return 1;
}

void
updatelocatorframe (gchar * locator)
{
	gchar *locatorlabeltext, *framelabeltext;
	GtkWidget *locatorframe, *framelabel, *locatorlabel;
	gint p, m, result, l;

	/* initialize */
	locatorframe = lookup_widget (scorewindow, "locatorframe");
	locatorlabel = lookup_widget (scorewindow, "locatorlabel");
	framelabel = gtk_frame_get_label_widget (GTK_FRAME(locatorframe));
	locatorlabeltext = g_strdup ("");
	framelabeltext = g_strdup ("");

	if (strlen (locator) >= 2)
	{
		result = locatordistance (preferences.locator, locator, &p, &l);
		if (result == 0)
		{
			if (preferences.units == 1)
				locatorlabeltext = g_strdup_printf
					("%d km, %d\xc2\xb0", p, l);
			else
			{
				m = (gint) (p / 1.609);
				locatorlabeltext = g_strdup_printf
					("%d m, %d\xc2\xb0", m, l);
			}
		}
		framelabeltext = g_strdup_printf ("<b>%s</b>", g_ascii_strup (locator, -1));
	}
	gtk_label_set_markup (GTK_LABEL (framelabel), framelabeltext);
	gtk_label_set_text (GTK_LABEL (locatorlabel), locatorlabeltext);
	g_free (framelabeltext);
	g_free (locatorlabeltext);
}
