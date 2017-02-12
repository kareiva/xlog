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
 * awards_enum.h
 */

#ifndef AWARDS_ENUM_H
#define AWARDS_ENUM_H

enum /* continents */
{
	CONTINENT_NA,
	CONTINENT_SA,
	CONTINENT_OC,
	CONTINENT_AS,
	CONTINENT_EU,
	CONTINENT_AF,
	MAX_CONTINENTS
};

enum /* US states */
{
	STATE_AL,
	STATE_AK,
	STATE_AZ,
	STATE_AR,
	STATE_CA,
	STATE_CO,
	STATE_CT,
	STATE_DE,
	STATE_FL,
	STATE_GA,
	STATE_HI,
	STATE_ID,
	STATE_IL,
	STATE_IN,
	STATE_IA,
	STATE_KS,
	STATE_KY,
	STATE_LA,
	STATE_ME,
	STATE_MD,
	STATE_MA,
	STATE_MI,
	STATE_MN,
	STATE_MS,
	STATE_MO,
	STATE_MT,
	STATE_NE,
	STATE_NV,
	STATE_NH,
	STATE_NJ,
	STATE_NM,
	STATE_NY,
	STATE_NC,
	STATE_ND,
	STATE_OH,
	STATE_OK,
	STATE_OR,
	STATE_PA,
	STATE_RI,
	STATE_SC,
	STATE_SD,
	STATE_TN,
	STATE_TX,
	STATE_UT,
	STATE_VT,
	STATE_VA,
	STATE_WA,
	STATE_WV,
	STATE_WI,
	STATE_WY,
	MAX_STATES
};

#define MAX_ZONES 40

enum /* IOTA continents */
{
	IOTA_CONTINENT_AF,
	IOTA_CONTINENT_AN,
	IOTA_CONTINENT_AS,
	IOTA_CONTINENT_EU,
	IOTA_CONTINENT_NA,
	IOTA_CONTINENT_OC,
	IOTA_CONTINENT_SA,
	MAX_IOTA_CONTINENTS
};


#define NOT_AN_IOTA 9999
#define NOT_A_LOCATOR 181900

guint cont_to_enum (gchar *str);
gchar *enum_to_cont (guint cont);
guint state_to_enum (gchar *str);
gchar *enum_to_state (guint st);
guint iota_to_num (gchar *str);
gchar *num_to_iota (guint st);
gint locator_to_num (gchar *str);
gchar *num_to_locator (gint num);

/* DXCC, WAZ, WAC, WAS, IOTA, LOCATOR */
#define NB_AWARDS 6

#endif	/* AWARDS_ENUM_H */
