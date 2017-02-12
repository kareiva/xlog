/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
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
 * xlog_enum.c - enumeration functions
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 

#include "xlog_enum.h"

/* translate enumerated band to a meaningful string */
gchar *band_enum2char (guint band_enum)
{
	switch (band_enum)
	{
		case BAND_2190: return g_strdup ("0.136");
		case BAND_560: return g_strdup ("0.501");
  		case BAND_160: return g_strdup ("1.8");
		case BAND_80: return g_strdup ("3.5");
		case BAND_60: return g_strdup ("5.2");
		case BAND_40: return g_strdup ("7");
		case BAND_30: return g_strdup ("10");
		case BAND_20: return g_strdup ("14");
		case BAND_17: return g_strdup ("18");
		case BAND_15: return g_strdup ("21");
		case BAND_12: return g_strdup ("24");
		case BAND_10: return g_strdup ("28");
		case BAND_6: return g_strdup ("50");
		case BAND_4: return g_strdup ("70");
		case BAND_2: return g_strdup ("144");
		case BAND_125CM: return g_strdup ("222");
		case BAND_70CM: return g_strdup ("420");
		case BAND_33CM: return g_strdup ("902");
		case BAND_23CM: return g_strdup ("1240");
		case BAND_13CM: return g_strdup ("2300");
		case BAND_9CM: return g_strdup ("3300");
		case BAND_6CM: return g_strdup ("5650");
		case BAND_3CM: return g_strdup ("10000");
		case BAND_12HMM: return g_strdup ("24000");
		case BAND_6MM: return g_strdup ("47000");
		case BAND_4MM: return g_strdup ("75500");
		case BAND_2HMM: return g_strdup ("120000");
		case BAND_2MM: return g_strdup ("142000");
		case BAND_1MM: return g_strdup ("241000");
		case BAND_SUBMM: return g_strdup ("300000");
	}
	return NULL;
}

gchar *band_enum2cabrillochar (guint band_enum)
{
	switch (band_enum)
	{
		case BAND_2190: return g_strdup ("136");
		case BAND_560: return g_strdup ("501");
  		case BAND_160: return g_strdup ("1800");
		case BAND_80: return g_strdup ("3500");
		case BAND_60: return g_strdup ("5200");
		case BAND_40: return g_strdup ("7000");
		case BAND_30: return g_strdup ("10000");
		case BAND_20: return g_strdup ("14000");
		case BAND_17: return g_strdup ("18000");
		case BAND_15: return g_strdup ("21000");
		case BAND_12: return g_strdup ("24000");
		case BAND_10: return g_strdup ("28000");
		case BAND_6: return g_strdup ("50");
		case BAND_4: return g_strdup ("70");
		case BAND_2: return g_strdup ("144");
		case BAND_125CM: return g_strdup ("222");
		case BAND_70CM: return g_strdup ("432");
		case BAND_33CM: return g_strdup ("902");
		case BAND_23CM: return g_strdup ("1.2G");
		case BAND_13CM: return g_strdup ("2.3G");
		case BAND_9CM: return g_strdup ("3.4G");
		case BAND_6CM: return g_strdup ("5.7G");
		case BAND_3CM: return g_strdup ("10G");
		case BAND_12HMM: return g_strdup ("24G");
		case BAND_6MM: return g_strdup ("47G");
		case BAND_4MM: return g_strdup ("75G");
		case BAND_2HMM: return g_strdup ("119G");
		case BAND_2MM: return g_strdup ("142G");
		case BAND_1MM: return g_strdup ("241G");
		case BAND_SUBMM: return g_strdup ("300G");
	}
	return NULL;
}

gchar *band_enum2bandchar (gint band_enum)
{
	switch (band_enum)
	{
		case BAND_2190: return g_strdup ("2190m");
		case BAND_560: return g_strdup ("560m");
  		case BAND_160: return g_strdup ("160m");
		case BAND_80: return g_strdup ("80m");
		case BAND_60: return g_strdup ("60m");
		case BAND_40: return g_strdup ("40m");
		case BAND_30: return g_strdup ("30m");
		case BAND_20: return g_strdup ("20m");
		case BAND_17: return g_strdup ("17m");
		case BAND_15: return g_strdup ("15m");
		case BAND_12: return g_strdup ("12m");
		case BAND_10: return g_strdup ("10m");
		case BAND_6: return g_strdup ("6m");
		case BAND_4: return g_strdup ("4m");
		case BAND_2: return g_strdup ("2m");
		case BAND_125CM: return g_strdup ("1.25m");
		case BAND_70CM: return g_strdup ("70cm");
		case BAND_33CM: return g_strdup ("33cm");
		case BAND_23CM: return g_strdup ("23cm");
		case BAND_13CM: return g_strdup ("13cm");
		case BAND_9CM: return g_strdup ("9cm");
		case BAND_6CM: return g_strdup ("6cm");
		case BAND_3CM: return g_strdup ("3cm");
		case BAND_12HMM: return g_strdup ("1.25cm");
		case BAND_6MM: return g_strdup ("6mm");
		case BAND_4MM: return g_strdup ("4mm");
		case BAND_2HMM: return g_strdup ("2.5mm");
		case BAND_2MM: return g_strdup ("2mm");
		case BAND_1MM: return g_strdup ("1mm");
		case BAND_SUBMM: return g_strdup ("SUBmm");
	}
	return g_strdup ("??");
}

/* return freq in kHz from a given frequency string */
gchar *freq2khz (gchar * str)
{
	gchar *p, *temp;
	gchar *pEnd;
	gchar temp_mhz[7];
	gdouble freq, freq1;

	temp  = g_strdup (str);
	p = g_strrstr (temp, ".");
	freq  = strtod (temp, &pEnd);

	if (p)
	{
	  freq1 = trunc(freq * 1000);
	  snprintf(temp_mhz, 6, "%d", (int)freq1);
	}
	else {
	  freq1 = freq * 1000;
	  snprintf(temp_mhz, 6, "%d", (int)freq1);
	}

	return g_strdup (temp_mhz);
}

/* return band as enum from a given frequency string */
gint
freq2enum (gchar * str)
{
	gchar *p, *temp, *temp0;
	guint fr;
	gdouble fr0;

	temp  = g_strdup (str);
	temp0 = g_strdup (str);

	p = g_strrstr (temp, ".");
	if (p)
	{
		*p = '\0';
		fr = atoi (temp);
	}
	else
		fr = atoi (temp);
	g_free (temp);

	switch (fr)
	{
		case 0:
		{
			fr0 = atof (temp0);
			g_free (temp0);

			if (trunc((fr0 * 100)) == 50)       /* Freq = 0.50xxxxx  = 560m */
				return BAND_560;
			else if (trunc((fr0 * 100)) == 13)  /* Freq = 0.13xxxxx  = 2190m */
				return BAND_2190;
			else 
				return -1;                   /* Freq = 0.??xxxxx */
		}
		case 1: return BAND_160;
		case 3: return BAND_80;
		case 5: return BAND_60;
		case 7: return BAND_40;
		case 10: return BAND_30;
		case 14: return BAND_20;
		case 18: return BAND_17;
		case 21: return BAND_15;
		case 24: return BAND_12;
		case 28 ... 29: return BAND_10;
		case 50 ... 54: return BAND_6;
		case 70 ... 71: return BAND_4;
		case 144 ... 148: return BAND_2;
		case 222 ... 225: return BAND_125CM;
		case 420 ... 450: return BAND_70CM;
		case 902 ... 928: return BAND_33CM;
		case 1240 ... 1325: return BAND_23CM;
		case 2300 ... 2450: return BAND_13CM;
		case 3300 ... 3500: return BAND_9CM;
		case 5650 ... 5925: return BAND_6CM;
		case 10000 ... 10500: return BAND_3CM;
		case 24000 ... 24250: return BAND_12HMM;
		case 47000 ... 47200: return BAND_6MM;
		case 75500 ... 81000: return BAND_4MM;
		case 119980 ... 120020: return BAND_2HMM;
		case 142000 ... 149000: return BAND_2MM;
		case 241000 ... 250000: return BAND_1MM;
		case 300000 ... 999999: return BAND_SUBMM;
	}
	return -1;
}

/* return band as enum from a given hamlib frequency string */
gint
hamlibfreq2enum (long long f)
{
	guint fr_small = f / 1000;   /* for 2190m and 560m */
	guint fr = f / 1000000;

	switch (fr)
	{
		case 0: 
		{
			switch (fr_small)
			{
				case 136 ... 137: return BAND_2190;
				case 501 ... 504: return BAND_560;
			}
			return -1;
		}
		case 1: return BAND_160;
		case 3: return BAND_80;
		case 5: return BAND_60;
		case 7: return BAND_40;
		case 10: return BAND_30;
		case 14: return BAND_20;
		case 18: return BAND_17;
		case 21: return BAND_15;
		case 24: return BAND_12;
		case 28 ... 29: return BAND_10;
		case 50 ... 54: return BAND_6;
		case 70 ... 71: return BAND_4;
		case 144 ... 148: return BAND_2;
		case 222 ... 225: return BAND_125CM;
		case 420 ... 450: return BAND_70CM;
		case 902 ... 928: return BAND_33CM;
		case 1240 ... 1325: return BAND_23CM;
		case 2300 ... 2450: return BAND_13CM;
		case 3300 ... 3500: return BAND_9CM;
		case 5650 ... 5925: return BAND_6CM;
		case 10000 ... 10500: return BAND_3CM;
		case 24000 ... 24250: return BAND_12HMM;
		case 47000 ... 47200: return BAND_6MM;
		case 75500 ... 81000: return BAND_4MM;
		case 119980 ... 120020: return BAND_2HMM;
		case 142000 ... 149000: return BAND_2MM;
		case 241000 ... 250000: return BAND_1MM;
		case 300000 ... 999999: return BAND_SUBMM;
	}
	return -1;
}

/* return band as enum from a given band string */
gint
meters2enum (gchar * str)
{
	gchar *p, *temp;
	guint m = -1, factor = 100;

	temp = g_strdup (str);
	/* find meters, centimeters, millimeters */
	p = g_strrstr (temp, "M");
	if (p) factor = 100;
	else
	{
		p = g_strrstr (temp, "m");
		if (p) factor = 100;
		else
		{
			p = g_strrstr (temp, "CM");
			if (p) factor = 10;
			else
			{
				p = g_strrstr (temp, "cm");
				if (p) factor = 10;
				else
				{
					p = g_strrstr (temp, "MM");
					if (p) factor = 1;
					else
					{
						p = g_strrstr (temp, "mm");
						if (p) factor = 1;
					}
				}
			}
		}
	}

	if (p)
	{
		*p = '\0';
		if (!g_strrstr (temp, "."))
			m = atoi (temp);
		else
		{
			/* 1.25m */
			if (factor == 100)
			{
				m = 1;
			}
			/* 1.25cm*/
			else
			{
				if (factor == 10)
					m = 1;
				/* 2.5mm */
				else
				{
					if (factor == 1)
						m = 3;
				}
			}
		}
	}
	g_free (temp);

	switch (factor)
	{
		case 100:
			switch (m)
			{
				case 2190: return 0;
				case 560: return 1;
				case 160: return 2;
				case 80: return 3;
				case 60: return 4;
				case 40: return 5;
				case 30: return 6;
				case 20: return 7;
				case 17: return 8;
				case 15: return 9;
				case 12: return 10;
				case 10: return 11;
				case 6: return 12;
				case 4: return 13;
				case 2: return 14;
				case 1: return 15;
			}
		break;
		case 10:
			switch (m)
			{
				case 125: return 15;
				case 70: return 16;
				case 33: return 17;
				case 23: return 18;
				case 13: return 19;
				case 9: return 20;
				case 6: return 21;
				case 3: return 22;
				case 1: return 23;
			}
		break;
		case 1:
			switch (m)
			{
				case 12: return 23;
				case 6: return 24;
				case 4: return 25;
				case 3: return 26;
				case 2: return 27;
				case 1: return 28;
			}
		break;
	}
	return -1;
}

/* translate enumerated mode to a meaningful string */

gchar *mode_enum2char (guint mode_enum)
{
	switch (mode_enum)
	{
		case	MODE_AM:	   return g_strdup("AM");
		case	MODE_AMTORFEC:	   return g_strdup("AMTORFEC");
		case	MODE_ASCI:	   return g_strdup("ASCI");
		case	MODE_ATV:	   return g_strdup("ATV");
		case	MODE_CHIP64:	   return g_strdup("CHIP64");
		case	MODE_CHIP128:	   return g_strdup("CHIP128");
		case	MODE_CLO:	   return g_strdup("CLO");
		case	MODE_CONTESTI:	   return g_strdup("CONTESTI");
		case	MODE_CW:	   return g_strdup("CW");
		case	MODE_DSTAR:	   return g_strdup("DSTAR");
		case	MODE_DOMINO:	   return g_strdup("DOMINO");
		case	MODE_DOMINOF:	   return g_strdup("DOMINOF");
		case	MODE_FAX:	   return g_strdup("FAX");
		case	MODE_FM:	   return g_strdup("FM");
		case	MODE_FMHELL:	   return g_strdup("FMHELL");
		case	MODE_FSK31:	   return g_strdup("FSK31");
		case	MODE_FSK441:	   return g_strdup("FSK441");
		case	MODE_GTOR:	   return g_strdup("GTOR");
		case	MODE_HELL:	   return g_strdup("HELL");
		case	MODE_HELL80:	   return g_strdup("HELL80");
		case	MODE_HFSK:	   return g_strdup("HFSK");
		case	MODE_JT44:	   return g_strdup("JT44");
		case	MODE_JT4A:	   return g_strdup("JT4A");
		case	MODE_JT4B:	   return g_strdup("JT4B");
		case	MODE_JT4C:	   return g_strdup("JT4C");
		case	MODE_JT4D:	   return g_strdup("JT4D");
		case	MODE_JT4E:	   return g_strdup("JT4E");
		case	MODE_JT4F:	   return g_strdup("JT4F");
		case	MODE_JT4G:	   return g_strdup("JT4G");
		case	MODE_JT9:	   return g_strdup("JT9");
		case	MODE_JT9_1:	   return g_strdup("JT9-1");
		case	MODE_JT9_2:	   return g_strdup("JT9-2");
		case	MODE_JT9_5:	   return g_strdup("JT9-5");
		case	MODE_JT9_10:	   return g_strdup("JT9-10");
		case	MODE_JT9_30:	   return g_strdup("JT9-30");
		case	MODE_JT65:	   return g_strdup("JT65");
		case	MODE_JT65A:	   return g_strdup("JT65A");
		case	MODE_JT65B:	   return g_strdup("JT65B");
		case	MODE_JT65C:	   return g_strdup("JT65C");
		case	MODE_JT6M:	   return g_strdup("JT6M");
		case	MODE_MFSK8:	   return g_strdup("MFSK8");
		case	MODE_MFSK16:	   return g_strdup("MFSK16");
		case	MODE_MT63:	   return g_strdup("MT63");
		case	MODE_OLIVIA:	   return g_strdup("OLIVIA");
		case	MODE_PAC:	   return g_strdup("PAC");
		case	MODE_PAC2:	   return g_strdup("PAC2");
		case	MODE_PAC3:	   return g_strdup("PAC3");
		case	MODE_PAX:	   return g_strdup("PAX");
		case	MODE_PAX2:	   return g_strdup("PAX2");
		case	MODE_PCW:	   return g_strdup("PCW");
		case	MODE_PKT:	   return g_strdup("PKT");
		case	MODE_PSK10:	   return g_strdup("PSK10");
		case	MODE_PSK31:	   return g_strdup("PSK31");
		case	MODE_PSK63:	   return g_strdup("PSK63");
		case	MODE_PSK63F:	   return g_strdup("PSK63F");
		case	MODE_PSK125:	   return g_strdup("PSK125");
		case	MODE_PSKAM10:	   return g_strdup("PSKAM10");
		case	MODE_PSKAM31:	   return g_strdup("PSKAM31");
		case	MODE_PSKAM50:	   return g_strdup("PSKAM50");
		case	MODE_PSKFEC31:	   return g_strdup("PSKFEC31");
		case	MODE_PSKHELL:	   return g_strdup("PSKHELL");
		case	MODE_Q15:	   return g_strdup("Q15");
		case	MODE_QPSK31:	   return g_strdup("QPSK31");
		case	MODE_QPSK63:	   return g_strdup("QPSK63");
		case	MODE_QPSK125:	   return g_strdup("QPSK125");
		case	MODE_ROS:	   return g_strdup("ROS");
		case	MODE_RTTY:	   return g_strdup("RTTY");
		case	MODE_RTTYM:	   return g_strdup("RTTYM");
		case	MODE_SSB:	   return g_strdup("SSB");
		case	MODE_SSTV:	   return g_strdup("SSTV");
		case	MODE_THRB:	   return g_strdup("THRB");
		case	MODE_THOR:	   return g_strdup("THOR");
		case	MODE_THRBX:	   return g_strdup("THRBX");
		case	MODE_TOR:	   return g_strdup("TOR");
		case	MODE_VOI:	   return g_strdup("VOI");
		case	MODE_WINMOR:	   return g_strdup("WINMOR");
		case	MODE_WSPR:	   return g_strdup("WSPR");
	}
	return NULL;
}

/* reportlen: 2 for "59", 3 for "599", 0 if unknown mode */

gint reportlen(guint mode_enum)
{
	switch (mode_enum)
	{
		case MODE_SSB:
		case MODE_FM:
		case MODE_AM:
			return 2;

		case MODE_AMTORFEC:
		case MODE_ASCI:
		case MODE_ATV:
		case MODE_CHIP64:
		case MODE_CHIP128:
		case MODE_CLO:
		case MODE_CONTESTI:
		case MODE_CW:
		case MODE_DSTAR:
		case MODE_DOMINO:
		case MODE_DOMINOF:
		case MODE_FAX:
		case MODE_FMHELL:
		case MODE_FSK31:
		case MODE_FSK441:
		case MODE_GTOR:
		case MODE_HELL:
		case MODE_HELL80:
		case MODE_HFSK:
		case MODE_JT44:
		case MODE_JT4A:
		case MODE_JT4B:
		case MODE_JT4C:
		case MODE_JT4D:
		case MODE_JT4E:
		case MODE_JT4F:
		case MODE_JT4G:
		case MODE_JT9:
		case MODE_JT9_1:
		case MODE_JT9_2:
		case MODE_JT9_5:
		case MODE_JT9_10:
		case MODE_JT9_30:
		case MODE_JT65:
		case MODE_JT65A:
		case MODE_JT65B:
		case MODE_JT65C:
		case MODE_JT6M:
		case MODE_MFSK8:
		case MODE_MFSK16:
		case MODE_MT63:
		case MODE_OLIVIA:
		case MODE_PAC:
		case MODE_PAC2:
		case MODE_PAC3:
		case MODE_PAX:
		case MODE_PAX2:
		case MODE_PCW:
		case MODE_PKT:
		case MODE_PSK10:
		case MODE_PSK31:
		case MODE_PSK63:
		case MODE_PSK63F:
		case MODE_PSK125:
		case MODE_PSKAM10:
		case MODE_PSKAM31:
		case MODE_PSKAM50:
		case MODE_PSKFEC31:
		case MODE_PSKHELL:
		case MODE_Q15:
		case MODE_QPSK31:
		case MODE_QPSK63:
		case MODE_QPSK125:
		case MODE_ROS:
		case MODE_RTTY:
		case MODE_RTTYM:
		case MODE_SSTV:
		case MODE_THRB:
		case MODE_THOR:
		case MODE_THRBX:
		case MODE_TOR:
		case MODE_VOI:
		case MODE_WINMOR:
		case MODE_WSPR:
			return 3;
	}
	return 0;
}

/* return mode as enum from a given string */
gint mode2enum (gchar * str)
{
	if
	(
		!g_ascii_strcasecmp  (str, "AM")     ||
		!g_ascii_strncasecmp (str, "A3E", 3) ||
		!g_ascii_strncasecmp (str, "A3", 2)
	)
	return MODE_AM;

	if
	(
		!g_ascii_strcasecmp (str, "AMTORFEC")
	)
	return MODE_AMTORFEC;

	if
	(
		!g_ascii_strcasecmp (str, "ASCI") ||
		!g_ascii_strcasecmp (str, "ASCII")
	)
	return MODE_ASCI;

	if
	(
		!g_ascii_strcasecmp (str, "ATV") ||
		!g_ascii_strcasecmp (str, "A5")	 ||
		!g_ascii_strcasecmp (str, "C3F")
	)
	return MODE_ATV;

	if
	(
		!g_ascii_strcasecmp (str, "CHIP64")
	)
	return MODE_CHIP64;

	if
	(
		!g_ascii_strcasecmp (str, "CHIP128")
	)
	return MODE_CHIP128;

	if 
	(
		!g_ascii_strcasecmp (str, "CLO") ||
		!g_ascii_strcasecmp (str, "CLOVER")
	)
	return MODE_CLO;

	if
	(
		!g_ascii_strcasecmp (str, "CONTESTI")
	)
	return MODE_CONTESTI;

	if
	(
		!g_ascii_strcasecmp  (str, "CW")    ||
		!g_ascii_strncasecmp (str, "A1", 2) ||
		!g_ascii_strncasecmp (str, "A2", 2)
	)
	return MODE_CW;

	if
	(
		!g_ascii_strcasecmp (str, "DSTAR")
	)
	return MODE_DSTAR;

	if
	(
		!g_ascii_strcasecmp (str, "DOMINO")
	)
	return MODE_DOMINO;

	if
	(
		!g_ascii_strcasecmp (str, "DOMINOF")
	)
	return MODE_DOMINOF;

	if
	(
		!g_ascii_strcasecmp (str, "FAX")
	)
	return MODE_FAX;

	if
	(
		!g_ascii_strcasecmp  (str, "FM")	  ||
		!g_ascii_strncasecmp (str, "F3", 2)	  ||
		!g_ascii_strcasecmp  (str, "20K0F3E")
	)
	return MODE_FM;

	if
	(
		!g_ascii_strcasecmp (str, "FMHELL")	 ||
		!g_ascii_strcasecmp (str, "FM-HELL")
	)
	return MODE_FMHELL;

	if
	(
		!g_ascii_strcasecmp (str, "FSK31")
	)
	return MODE_FSK31;

	if
	(
		!g_ascii_strcasecmp (str, "FSK441")
	)
	return MODE_FSK441;

	if
	(
		!g_ascii_strcasecmp (str, "GTOR")
	)
	return MODE_GTOR;

	if
	(
		!g_ascii_strcasecmp (str, "HELL")	   ||
		!g_ascii_strcasecmp (str, "HELLSCHREIBER") ||
		!g_ascii_strcasecmp (str, "FELDHELL")
	)
	return MODE_HELL;

	if
	(
		!g_ascii_strcasecmp (str, "HELL80")
	)
	return MODE_HELL80;

	if
	(
		!g_ascii_strcasecmp (str, "HFSK")
	)
	return MODE_HFSK;

	if
	(
		!g_ascii_strcasecmp (str, "JT44")
	)
	return MODE_JT44;

	if
	(
		!g_ascii_strcasecmp (str, "JT4A")
	)
	return MODE_JT4A;

	if
	(
		!g_ascii_strcasecmp (str, "JT4B")
	)
	return MODE_JT4B;

	if
	(
		!g_ascii_strcasecmp (str, "JT4C")
	)
	return MODE_JT4C;

	if
	(
		!g_ascii_strcasecmp (str, "JT4D")
	)
	return MODE_JT4D;

	if
	(
		!g_ascii_strcasecmp (str, "JT4E")
	)
	return MODE_JT4E;

	if
	(
		!g_ascii_strcasecmp (str, "JT4F")
	)
	return MODE_JT4F;

	if
	(
		!g_ascii_strcasecmp (str, "JT4G")
	)
	return MODE_JT4G;

	if
	(
		!g_ascii_strcasecmp (str, "JT9")
	)
	return MODE_JT9;

	if
	(
		!g_ascii_strcasecmp (str, "JT9-1")
	)
	return MODE_JT9_1;

	if
	(
		!g_ascii_strcasecmp (str, "JT9-2")
	)
	return MODE_JT9_2;

	if
	(
		!g_ascii_strcasecmp (str, "JT9-5")
	)
	return MODE_JT9_5;

	if
	(
		!g_ascii_strcasecmp (str, "JT9-10")
	)
	return MODE_JT9_10;

	if
	(
		!g_ascii_strcasecmp (str, "JT9-30")
	)
	return MODE_JT9_30;

	if
	(
		!g_ascii_strcasecmp (str, "JT65")
	)
	return MODE_JT65;

	if
	(
		!g_ascii_strcasecmp (str, "JT65A")
	)
	return MODE_JT65A;

	if
	(
		!g_ascii_strcasecmp (str, "JT65B")
	)
	return MODE_JT65B;

	if
	(
		!g_ascii_strcasecmp (str, "JT65C")
	)
	return MODE_JT65C;

	if
	(
		!g_ascii_strcasecmp (str, "JT6M")
	)
	return MODE_JT6M;

	if
	(
		!g_ascii_strcasecmp (str, "MFSK8") ||
		!g_ascii_strcasecmp (str, "MFSK")
	)
	return MODE_MFSK8;

	if
	(
		!g_ascii_strcasecmp (str, "MFSK16")
	)
	return MODE_MFSK16;

	if
	(
		!g_ascii_strcasecmp (str, "MT63")
	)
	return MODE_MT63;

	if
	(
		!g_ascii_strcasecmp (str, "OLIVIA")
	)
	return MODE_OLIVIA;

	if
	(
		!g_ascii_strcasecmp (str, "PAC") ||
		!g_ascii_strcasecmp (str, "PACTOR")
	)
	return MODE_PAC;

	if
	(
		!g_ascii_strcasecmp (str, "PAC2")      ||
		!g_ascii_strcasecmp (str, "PACTOR II") ||
		!g_ascii_strcasecmp (str, "PACTORII")  ||
		!g_ascii_strcasecmp (str, "PACTOR 2")  ||
		!g_ascii_strcasecmp (str, "PACTOR2")
	)
	return MODE_PAC2;

	if
	(
		!g_ascii_strcasecmp (str, "PAC3")	||
		!g_ascii_strcasecmp (str, "PACTOR III") ||
		!g_ascii_strcasecmp (str, "PACTORIII")	||
		!g_ascii_strcasecmp (str, "PACTOR 3")	||
		!g_ascii_strcasecmp (str, "PACTOR3")
	)
	return MODE_PAC3;

	if
	(
		!g_ascii_strcasecmp (str, "PAX")
	)
	return MODE_PAX;

	if
	(
		!g_ascii_strcasecmp (str, "PAX2")
	)
	return MODE_PAX2;

	if
	(
		!g_ascii_strcasecmp (str, "PCW")
	)
	return MODE_PCW;

	if
	(
		!g_ascii_strcasecmp (str, "PKT") ||
		!g_ascii_strcasecmp (str, "PACKET")
	)
	return MODE_PKT;

	if
	(
		!g_ascii_strcasecmp (str, "PSK10")
	)
	return MODE_PSK10;

	if
	(
		!g_ascii_strcasecmp (str, "PSK")	||
		!g_ascii_strcasecmp (str, "PSK31")	||
		!g_ascii_strcasecmp (str, "QPSK")	||
		!g_ascii_strcasecmp (str, "BPSK")	||
		!g_ascii_strcasecmp (str, "BPSK31")
	)
	return MODE_PSK31;

	if
	(
		!g_ascii_strcasecmp (str, "PSK63") ||
		!g_ascii_strcasecmp (str, "BPSK63")
	)
	return MODE_PSK63;

	if
	(
		!g_ascii_strcasecmp (str, "PSK63F")	 ||
		!g_ascii_strcasecmp (str, "BPSK63F")
	)
	return MODE_PSK63F;

	if
	(
		!g_ascii_strcasecmp (str, "PSK125")
	)
	return MODE_PSK125;

	if
	(
		!g_ascii_strcasecmp (str, "PSKAM10")
	)
	return MODE_PSKAM10;

	if
	(
		!g_ascii_strcasecmp (str, "PSKAM31")
	)
	return MODE_PSKAM31;

	if
	(
		!g_ascii_strcasecmp (str, "PSKAM50")
	)
	return MODE_PSKAM50;

	if
	(
		!g_ascii_strcasecmp (str, "PSKFEC31")
	)
	return MODE_PSKFEC31;

	if
	(
		!g_ascii_strcasecmp (str, "PSKHELL")  ||
		!g_ascii_strcasecmp (str, "PSK-HELL")
	)
	return MODE_PSKHELL;

	if
	(
		!g_ascii_strcasecmp (str, "Q15") ||
		!g_ascii_strcasecmp (str, "Q15X25")
	)
	return MODE_Q15;

	if
	(
		!g_ascii_strcasecmp (str, "QPSK31")
	)
	return MODE_QPSK31;

	if
	(
		!g_ascii_strcasecmp (str, "QPSK63")
	)
	return MODE_QPSK63;

	if
	(
		!g_ascii_strcasecmp (str, "QPSK125")
	)
	return MODE_QPSK125;

	if
	(
		!g_ascii_strcasecmp (str, "ROS")
	)
	return MODE_ROS;

	if
	(
		!g_ascii_strcasecmp  (str, "RTTY")  ||
		!g_ascii_strncasecmp (str, "F1", 2) ||
		!g_ascii_strncasecmp (str, "F2", 2)
	)
	return MODE_RTTY;

	if
	(
		!g_ascii_strcasecmp (str, "RTTYM")
	)
	return MODE_RTTYM;

	if
	(
		!g_ascii_strcasecmp (str, "SSB") ||
		!g_ascii_strcasecmp (str, "USB") ||
		!g_ascii_strcasecmp (str, "LSB") ||
		!g_ascii_strcasecmp (str, "J3E") ||
		!g_ascii_strcasecmp (str, "A3J") ||
		!g_ascii_strcasecmp (str, "R3E") ||
		!g_ascii_strcasecmp (str, "H3E") ||
		!g_ascii_strcasecmp (str, "A3R") ||
		!g_ascii_strcasecmp (str, "PHONE") ||
		!g_ascii_strcasecmp (str, "VOICE") ||
		!g_ascii_strcasecmp (str, "A3H")
	)
	return MODE_SSB;

	if
	(
		!g_ascii_strcasecmp (str, "SSTV")
	)
	return MODE_SSTV;

	if
	(
		!g_ascii_strcasecmp (str, "THRB")  ||
		!g_ascii_strcasecmp (str, "THROB")
	)
	return MODE_THRB;

	if
	(
		!g_ascii_strcasecmp (str, "THRBX")
	)
	return MODE_THRBX;

	if
	(
		!g_ascii_strcasecmp (str, "THOR")
	)
	return MODE_THOR;

	if
	(
		!g_ascii_strcasecmp (str, "TOR")  ||
		!g_ascii_strcasecmp (str, "AMTOR")
	)
	return MODE_TOR;

	if
	(
		!g_ascii_strcasecmp (str, "VOI")
	)
	return MODE_VOI;

	if
	(
		!g_ascii_strcasecmp (str, "WINMOR")
	)
	return MODE_WINMOR;

	if
	(
		!g_ascii_strcasecmp (str, "WSPR")
	)
	return MODE_WSPR;

	return -1;
}
