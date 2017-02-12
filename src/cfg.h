/*

   xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2012 - 2013 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2001 - 2010 Joop Stakenborg <pg4i@amsat.org>

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
 * cfg.h
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

/* preferences saved to ~/.xlog/xlog.cfg */
typedef struct
{
	gint x;
	gint y;
	gint width;
	gint height;
	gdouble latitude;
	gint NS;
	gdouble longitude;
	gint EW;
	gint units;           /* kilometers or nautical miles */
	gchar *modes;
	gchar *bands;
	gint bandseditbox;    /* optionmenu or entry for bands? */
	gint modeseditbox;    /* optionmenu or entry modes? */
	gint bandoptionmenu;  /* position of optionmenu */
	gint modeoptionmenu;  /* position of optionmenu */
	gint hamlib;  /* hamlib enabled? 0=no, 1=yes, 2=display freq on statusbar */
	gint rigid;      /* what type of radio do you use? */
	gchar *device;   /* which serial port does it use? */
	gchar *rigconf;  /* string for rig_set_conf */
	gint round;      /* how many digits to round to */
	gint polltime;   /* poll every 'polltime' milliseconds, 0 = no polling */
	gint clock;      /* clock on the statusbar? */
	gchar *logfont;
	gint autosave;
	gchar *savedir;
	gchar *backupdir;
	gint saving;    /* 1= autosave, 2=save with click on toolbar */
	gint backup;    /* 1= standard, 2=safe */
	gchar *logstoload;
	gint logorder;  /* 0=by last modification data, 1=alphabetically */
	gchar *locator;
	gchar *freefield1;
	gchar *freefield2;
	gchar *callsign;
	gchar *defaultmhz;
	gchar *defaultmode;
	gchar *defaulttxrst;
	gchar *defaultrxrst;
	gchar *defaultawards;
	gchar *defaultpower;
	gchar *defaultfreefield1;
	gchar *defaultfreefield2;
	gchar *defaultremarks;
	gint *b4columns2;
	gint *logcwidths2;
	gint typeaheadfind;  /* do we want type ahead find */
	gint remoteadding;   /* add remote data to log or qso frame */
	gint viewtoolbar;    /* do we want to see the toolbar button */
	gint viewb4;         /* do we want to see the worked before dialog */
	gint b4x;
	gint b4y;
	gint b4width;
	gint b4height;
	gint saveasadif;
	gint saveascabrillo;
	gint *saveastsv2;
	gint tsvcalc;
	gint tsvsortbydxcc;
	gint tsvgroupbycallsign;
	gint handlebarpos;
	gchar *cwf1;
	gchar *cwf2;
	gchar *cwf3;
	gchar *cwf4;
	gchar *cwf5;
	gchar *cwf6;
	gchar *cwf7;
	gchar *cwf8;
	gchar *cwf9;
	gchar *cwf10;
	gchar *cwf11;
	gchar *cwf12;
	gchar *cwcq;
	gchar *cwsp;
	gint cwspeed;
	gint fcc;
	gint viewscoring;
	gint scorex;
	gint scorey;
	gint scorewidth;
	gint scoreheight;
	gint *scoringbands;
	gint distqrb;
	gint awardswac;  /* WAC visible in the scoring window */
	gint awardswas;  /* WAS visible in the scoring window */
	gint awardswaz;  /* WAZ visible in the scoring window */
	gint awardsiota; /* IOTA visible in the scoring window */
	gint awardsloc;  /* Locator visible in the scoring window */
	gchar *openurl;
	gchar *initlastmsg;
	gint areyousure; /* Display "Are You Sure" dialog on exit */
}
preferencestype;

void loadpreferences (void);
void savepreferences (void);

#endif	/* PREFERENCES_H */
