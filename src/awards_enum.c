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
 * awards_enum.c
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

#include "awards_enum.h"

guint cont_to_enum (gchar *str)
{
	if (strlen (str) < 2) return 99;
	switch (str[0])
	{
		case 'N': return str[1]=='A'?CONTINENT_NA:99;
		case 'E': return str[1]=='U'?CONTINENT_EU:99;
		case 'S': return str[1]=='A'?CONTINENT_SA:99;
		case 'O': return str[1]=='C'?CONTINENT_OC:99;
		case 'A':
		{
			switch (str[1])
			{
				case 'S': return CONTINENT_AS;
				case 'F': return CONTINENT_AF;
			}
			break;
		}
	}
	return 99;
}

gchar *enum_to_cont (guint cont)
{
	switch (cont)
	{
		case CONTINENT_NA: return g_strdup("NA");
		case CONTINENT_SA: return g_strdup("SA");
		case CONTINENT_OC: return g_strdup("OC");
		case CONTINENT_AS: return g_strdup("AS");
		case CONTINENT_EU: return g_strdup("EU");
		case CONTINENT_AF: return g_strdup("AF");
	}
	return g_strdup("--");
}

guint state_to_enum (gchar *str)
{
	if (strlen (str) < 2) return 99;
	switch (str[0])
	{
		case 'A':
		{
			switch (str[1])
			{
				case 'L': return STATE_AL;
				case 'K': return STATE_AK;
				case 'Z': return STATE_AZ;
				case 'R': return STATE_AR;
			}
			break;
		}
		case 'C':
		{
			switch (str[1])
			{
				case 'A': return STATE_CA;
				case 'O': return STATE_CO;
				case 'T': return STATE_CT;
			}
			break;
		}
		case 'D':
		{
			return STATE_DE;
		}
		case 'F':
		{
			return STATE_FL;
		}
		case 'G':
		{
			return STATE_GA;
		}
		case 'H':
		{
			return STATE_HI;
		}
		case 'I':
		{
			switch (str[1])
			{
				case 'L': return STATE_IL;
				case 'N': return STATE_IN;
				case 'A': return STATE_IA;
				case 'D': return STATE_ID;
			}
			break;
		}
		case 'K':
		{
			switch (str[1])
			{
				case 'S': return STATE_KS;
				case 'Y': return STATE_KY;
			}
			break;
		}
		case 'L':
		{
			return STATE_LA;
		}
		case 'M':
		{
			switch (str[1])
			{
				case 'E': return STATE_ME;
				case 'D': return STATE_MD;
				case 'A': return STATE_MA;
				case 'I': return STATE_MI;
				case 'N': return STATE_MN;
				case 'S': return STATE_MS;
				case 'O': return STATE_MO;
				case 'T': return STATE_MT;
			}
			break;
		}
		case 'N':
		{
			switch (str[1])
			{
				case 'E': return STATE_NE;
				case 'V': return STATE_NV;
				case 'H': return STATE_NH;
				case 'J': return STATE_NJ;
				case 'M': return STATE_NM;
				case 'Y': return STATE_NY;
				case 'C': return STATE_NC;
				case 'D': return STATE_ND;
			}
			break;
		}
		case 'O':
		{
			switch (str[1])
			{
				case 'H': return STATE_OH;
				case 'K': return STATE_OK;
				case 'R': return STATE_OR;
			}
			break;
		}
		case 'P':
		{
			return STATE_PA;
		}
		case 'R':
		{
			return STATE_RI;
		}
		case 'S':
		{
			switch (str[1])
			{
				case 'C': return STATE_SC;
				case 'D': return STATE_SD;
			}
			break;
		}
		case 'T':
		{
			switch (str[1])
			{
				case 'N': return STATE_TN;
				case 'X': return STATE_TX;
			}
			break;
		}
		case 'U':
		{
			return STATE_UT;
		}
		case 'V':
		{
			switch (str[1])
			{
				case 'T': return STATE_VT;
				case 'A': return STATE_VA;
			}
			break;
		}
		case 'W':
		{
			switch (str[1])
			{
				case 'A': return STATE_WA;
				case 'V': return STATE_WV;
				case 'I': return STATE_WI;
				case 'Y': return STATE_WY;
			}
			break;
		}
	}
	return 99;
}


/* Must be in same order as awards_enum.h */
static const gchar *usa_states[MAX_STATES] = {
	"AL",
	"AK",
	"AZ",
	"AR",
	"CA",
	"CO",
	"CT",
	"DE",
	"FL",
	"GA",
	"HI",
	"ID",
	"IL",
	"IN",
	"IA",
	"KS",
	"KY",
	"LA",
	"ME",
	"MD",
	"MA",
	"MI",
	"MN",
	"MS",
	"MO",
	"MT",
	"NE",
	"NV",
	"NH",
	"NJ",
	"NM",
	"NY",
	"NC",
	"ND",
	"OH",
	"OK",
	"OR",
	"PA",
	"RI",
	"SC",
	"SD",
	"TN",
	"TX",
	"UT",
	"VT",
	"VA",
	"WA",
	"WV",
	"WI",
	"WY"
};

gchar *enum_to_state (guint st)
{
	if (st < MAX_STATES)
		return g_strdup(usa_states[st]);

	return NULL;
}

guint iota_to_num (gchar *str)
{
	guint cont;

	if (!str || strlen (str) < 6 || str[2] != '-') return NOT_AN_IOTA;

	switch (str[0])
	{
		case 'A':
			switch (str[1])
			{
				case 'F': cont = IOTA_CONTINENT_AF; break;
				case 'N': cont = IOTA_CONTINENT_AN; break;
				case 'S': cont = IOTA_CONTINENT_AS; break;
				default: return NOT_AN_IOTA;
			}
			break;
		case 'E': cont = IOTA_CONTINENT_EU; break;
		case 'N': cont = IOTA_CONTINENT_NA; break;
		case 'O': cont = IOTA_CONTINENT_OC; break;
		case 'S': cont = IOTA_CONTINENT_SA; break;
		default: return NOT_AN_IOTA;
	}

	return cont*1000 + (atoi(str+3)%1000);
}

gchar *num_to_iota (guint num)
{
	const gchar *cont;

	if (num == NOT_AN_IOTA) return NULL;
	switch (num/1000)
	{
		case IOTA_CONTINENT_AF: cont = "AF"; break;
		case IOTA_CONTINENT_AN: cont = "AN"; break;
		case IOTA_CONTINENT_AS: cont = "AS"; break;
		case IOTA_CONTINENT_EU: cont = "EU"; break;
		case IOTA_CONTINENT_NA: cont = "NA"; break;
		case IOTA_CONTINENT_OC: cont = "OC"; break;
		case IOTA_CONTINENT_SA: cont = "SA"; break;
		default: return NULL;
	}
	return g_strdup_printf("%s-%03u", cont, num%1000);
}

/* locator runs from AA00 to RR99 and returns 010100 to 181899 */
gint locator_to_num (gchar *str)
{
	if (!str || strlen (str) < 4) return NOT_A_LOCATOR;

	gint first;
	if (g_ascii_islower(str[0]))
		first = str[0] - 96;
	else
		first = str[0] - 64;
	gint second;
	if (g_ascii_islower(str[1]))
		second = str[1] - 96;
	else
		second = str[1] - 64;
	
	return first*10000 + second*100 + (str[2]-48)*10 + str[3]-48;
}

gchar *num_to_locator (gint num)
{
	gchar *locator = g_new0 (gchar, 4);

	gint first = num/10000;
	num = num - first * 10000;
	gint second = num/100;
	num = num - second * 100;
	gint third = num/10;
	num = num - third * 10;

	locator[0] = first + 96;
	locator[1] = second + 96;
	locator[2] = third + 48;
	locator[3] = num + 48;

	return locator;
}
