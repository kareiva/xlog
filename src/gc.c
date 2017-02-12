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
 * gc.c - distance calculations
 */


#include <gtk/gtk.h>
#include <hamlib/rotator.h>

#include "support.h"
#include "utils.h"
#include "gc.h"

/* 
 * distance function for lat/lon coordinates
 * 
 * description: calculates distance between coordinates
 * 
 * arguments: units, latitude and longitude, latitude and longitude of the
 *            dx location
 * returns: empty string on error, string with short and long path on succes
 * 
 * note: longitude = E/W, latitude = N/S, N and E are positive
 * 
 */
gchar *
gcircle (gint units, gdouble latmy, gdouble lonmy, gdouble latdx, gdouble londx)
{
	gdouble distsp, bsp, distlp, blp;
	gchar *result, *unitsstr;
	gint retcode;

	result = g_strdup ("");

	if (units == 1)
		unitsstr = g_strdup ("km");
	else
		unitsstr = g_strdup ("m");

	/* data in cty.dat uses a negative number for eastern longitudes */
#ifdef G_OS_WIN32
	retcode = myqrb (lonmy, latmy, (-1. * londx), latdx, &distsp, &bsp);
#else
	retcode = qrb (lonmy, latmy, (-1. * londx), latdx, &distsp, &bsp);
#endif
	if (retcode == RIG_OK)
	{
		distlp = distance_long_path (distsp);
		blp = azimuth_long_path (bsp);
		if (units == 0)
		{
			distsp = distsp / 1.609;
			distlp = distlp / 1.609;
		}
		result = g_strdup_printf
(_("\nShort Path: %03.0f deg, %.0f %s\nLong Path: %03.0f deg, %.0f %s\n"), 
		bsp, distsp, unitsstr, blp, distlp, unitsstr);
	}

	g_free (unitsstr);
	return (result);
}
