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
 * xlog_enum.h
 */

enum /* we have 29 amateur radio bands */
{
	BAND_2190,	/* 0.136 - 0.137 MHz experimental */
	BAND_560,       /* 0.501 - 0.504 MHz */
	BAND_160,
	BAND_80,
	BAND_60,	/* 5.2 - 5.5 MHz, some countries */
	BAND_40,
	BAND_30,
	BAND_20,
	BAND_17,
	BAND_15,
	BAND_12,
	BAND_10,
	BAND_6,
	BAND_4,
	BAND_2,
	BAND_125CM,     /* 220 - 225 MHz, region 2 */
	BAND_70CM,
	BAND_33CM,	/* 902 - 928 MHz, region 2 */
	BAND_23CM,
	BAND_13CM,
	BAND_9CM,
	BAND_6CM,
	BAND_3CM,
	BAND_12HMM,
	BAND_6MM,
	BAND_4MM,
	BAND_2HMM,
	BAND_2MM,
	BAND_1MM,
	BAND_SUBMM,
	MAX_BANDS	/* number of bands in the enumeration */
};

enum /* modes taken from ADIF 2.2.7 spec */
{
    MODE_AM,
    MODE_AMTORFEC,
    MODE_ASCI,
    MODE_ATV,
    MODE_CHIP64,
    MODE_CHIP128,
    MODE_CLO,
    MODE_CONTESTI,
    MODE_CW,
    MODE_DSTAR,
    MODE_DOMINO,
    MODE_DOMINOF,
    MODE_FAX,
    MODE_FM,
    MODE_FMHELL,
    MODE_FSK31,
    MODE_FSK441,
    MODE_GTOR,
    MODE_HELL,
    MODE_HELL80,
    MODE_HFSK,
    MODE_JT44,
    MODE_JT4A,
    MODE_JT4B,
    MODE_JT4C,
    MODE_JT4D,
    MODE_JT4E,
    MODE_JT4F,
    MODE_JT4G,
    MODE_JT9,
    MODE_JT9_1,
    MODE_JT9_2,
    MODE_JT9_5,
    MODE_JT9_10,
    MODE_JT9_30,
    MODE_JT65,
    MODE_JT65A,
    MODE_JT65B,
    MODE_JT65C,
    MODE_JT6M,
    MODE_MFSK8,
    MODE_MFSK16,
    MODE_MT63,
    MODE_OLIVIA,
    MODE_PAC,
    MODE_PAC2,
    MODE_PAC3,
    MODE_PAX,
    MODE_PAX2,
    MODE_PCW,
    MODE_PKT,
    MODE_PSK10,
    MODE_PSK31,
    MODE_PSK63,
    MODE_PSK63F,
    MODE_PSK125,
    MODE_PSKAM10,
    MODE_PSKAM31,
    MODE_PSKAM50,
    MODE_PSKFEC31,
    MODE_PSKHELL,
    MODE_Q15,
    MODE_QPSK31,
    MODE_QPSK63,
    MODE_QPSK125,
    MODE_ROS,
    MODE_RTTY,
    MODE_RTTYM,
    MODE_SSB,
    MODE_SSTV,
    MODE_THRB,
    MODE_THOR,
    MODE_THRBX,
    MODE_TOR,
    MODE_VOI,
    MODE_WINMOR,
    MODE_WSPR,
    MAX_MODES	/* number of modes in the enumeration */
};

gchar *band_enum2char (guint band_enum);
gchar *band_enum2cabrillochar (guint band_enum);
gint freq2enum (gchar * str);
gint hamlibfreq2enum (long long f);
gchar *band_enum2bandchar (gint band_enum);
gint meters2enum (gchar * str);
gchar *mode_enum2char (guint mode_enum);
gint reportlen(guint mode_enum);
gint mode2enum (gchar * str);
gchar *freq2khz (gchar *str);
