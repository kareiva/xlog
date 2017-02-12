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
 * cfg.c - load and save program settings in GKeyFile format
 */

#include <glib.h>
#include <stdlib.h>
#include "cfg.h"

preferencestype preferences;
extern gchar *xlogdir;

/*
 * Initial configuration 
 */
 void
config_create (void)
{
	preferences.x = 10;
	preferences.y = 30;
	preferences.width = 750;
	preferences.height = 540;
	preferences.latitude = 0.0;
	preferences.NS = 1;
	preferences.longitude = 0.0;
	preferences.EW = 1;
	preferences.units = 1;
	preferences.bands = g_strdup
("0.136,0.501,1.8,3.5,5.2,7,10,14,18,21,24,28,50,70,144,222,420,902,1240,2300,3300,5650,10000,24000,47000,75500,120000,142000,241000,300000");
	preferences.modes = g_strdup
("SSB,CW,FM,AM,PSK31,RTTY,HELL,AMTORFEC,ASCI,ATV,CHIP64,CHIP128,CLO,CONTESTI,DSTAR,DOMINO,DOMINOF,FAX,FMHELL,FSK31,FSK441,GTOR,HELL80,HFSK,JT44,JT4A,JT4B,JT4C,JT4D,JT4E,JT4F,JT4G,JT9,JT9-1,JT9-2,JT9-5,JT9-10,JT9-30,JT65,JT65A,JT65B,JT65C,JT6M,MFSK8,MFSK16,MT63,OLIVIA,PAC,PAC2,PAC3,PAX,PAX2,PCW,PKT,PSK10,PSK63,PSK63F,PSK125,PSKAM10,PSKAM31,PSKAM50,PSKFEC31,PSKHELL,Q15,QPSK31,QPSK63,QPSK125,ROS,RTTYM,SSTV,THRB,THOR,THRBX,TOR,VOI,WINMOR,WSPR");
	preferences.bandseditbox = 0;
	preferences.modeseditbox = 0;
	preferences.bandoptionmenu = 0;
	preferences.modeoptionmenu = 0;
	preferences.hamlib = 0;
	preferences.rigid = 1;
#ifdef G_OS_WIN32
	preferences.device = g_strdup ("com2");
#else
	preferences.device = g_strdup ("/dev/ttyS1");
#endif
	preferences.rigconf = g_strdup ("");
	preferences.round = 3;
	preferences.polltime = 0;
	preferences.clock = 0;
	preferences.logfont = g_strdup ("Sans 10");
	preferences.autosave = 5;
	preferences.savedir = g_strdup_printf ("%s%s.%s",
		g_get_home_dir (), G_DIR_SEPARATOR_S, PACKAGE);
	preferences.saving = 1;
	preferences.logstoload = g_strdup ("*");
	preferences.logorder = 0;
	preferences.locator = g_strdup ("AA00AA");
	preferences.freefield1 = g_strdup ("UNKNOWN");
	preferences.freefield2 = g_strdup ("UNKNOWN");
	preferences.callsign = g_strdup ("N0CALL");
	preferences.defaultmhz = g_strdup ("");
	preferences.defaultmode = g_strdup ("");
	preferences.defaulttxrst = g_strdup ("");
	preferences.defaultrxrst = g_strdup ("");
	preferences.defaultawards = g_strdup ("");
	preferences.defaultpower = g_strdup ("");
	preferences.defaultfreefield1 = g_strdup ("");
	preferences.defaultfreefield2 = g_strdup ("");
	preferences.defaultremarks = g_strdup ("");
	preferences.b4columns2 = g_new (gint, 20);
//	preferences.b4columns2={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	preferences.b4columns2[0]=1;
	preferences.b4columns2[1]=1;
	preferences.b4columns2[2]=1;
	preferences.b4columns2[3]=1;
	preferences.b4columns2[4]=0;
	preferences.b4columns2[5]=1;
	preferences.b4columns2[6]=1;
	preferences.b4columns2[7]=1;
	preferences.b4columns2[8]=0;
	preferences.b4columns2[9]=0;
	preferences.b4columns2[10]=0;
	preferences.b4columns2[11]=0;
	preferences.b4columns2[12]=0;
	preferences.b4columns2[13]=0;
	preferences.b4columns2[14]=0;
	preferences.b4columns2[15]=0;
	preferences.b4columns2[16]=0;
	preferences.b4columns2[17]=0;
	preferences.b4columns2[18]=0;
	preferences.b4columns2[19]=0;
	preferences.logcwidths2 = g_new (gint, 19);
	preferences.logcwidths2[0]=50;
	preferences.logcwidths2[1]=100;
	preferences.logcwidths2[2]=50;
	preferences.logcwidths2[3]=50;
	preferences.logcwidths2[4]=100;
	preferences.logcwidths2[5]=50;
	preferences.logcwidths2[6]=50;
	preferences.logcwidths2[7]=50;
	preferences.logcwidths2[8]=50;
	preferences.logcwidths2[9]=100;
	preferences.logcwidths2[10]=50;
	preferences.logcwidths2[11]=50;
	preferences.logcwidths2[12]=50;
	preferences.logcwidths2[13]=100;
	preferences.logcwidths2[14]=100;
	preferences.logcwidths2[15]=100;
	preferences.logcwidths2[16]=100;
	preferences.logcwidths2[17]=100;
	preferences.logcwidths2[18]=200;

	preferences.typeaheadfind = 1;
	preferences.remoteadding = 1;
	preferences.viewtoolbar = 1;
	preferences.viewb4 = 0;
	preferences.b4x = 10;
	preferences.b4y = 30;
	preferences.b4width = 500;
	preferences.b4height = 300;
	preferences.saveasadif = 0;
	preferences.saveascabrillo = 0;
	preferences.saveastsv2 = g_new (gint, 18);
	preferences.saveastsv2[0] = 1;
	preferences.saveastsv2[1] = 1;
	preferences.saveastsv2[2] = 0;
	preferences.saveastsv2[3] = 1;
	preferences.saveastsv2[4] = 1;
	preferences.saveastsv2[5] = 1;
	preferences.saveastsv2[6] = 1;
	preferences.saveastsv2[7] = 0;
	preferences.saveastsv2[8] = 0;
	preferences.saveastsv2[9] = 0;
	preferences.saveastsv2[10] = 0;
	preferences.saveastsv2[11] = 0;
	preferences.saveastsv2[12] = 0;
	preferences.saveastsv2[13] = 0;
	preferences.saveastsv2[14] = 0;
	preferences.saveastsv2[15] = 0;
	preferences.saveastsv2[16] = 0;
	preferences.saveastsv2[17] = 0;
	preferences.handlebarpos = 300;
	preferences.cwf1 = g_strdup ("cq de % % test");
	preferences.cwf2 = g_strdup ("@ de %");
	preferences.cwf3 = g_strdup ("@ ++5nn--#");
	preferences.cwf4 = g_strdup ("tu 73");
	preferences.cwf5 = g_strdup ("@");
	preferences.cwf6 = g_strdup ("%");
	preferences.cwf7 = g_strdup ("@ sri qso b4 gl");
	preferences.cwf8 = g_strdup ("agn");
	preferences.cwf9 = g_strdup ("?");
	preferences.cwf10 = g_strdup ("qrz?");
	preferences.cwf11 = g_strdup ("pse k");
	preferences.cwf12 = g_strdup ("++++test---- % %");
	preferences.cwcq = g_strdup ("tu %");
	preferences.cwsp = g_strdup ("tu 5nn#");
	preferences.cwspeed = 24;
	preferences.backupdir = g_strdup_printf ("%s%s.%s",
		g_get_home_dir (), G_DIR_SEPARATOR_S, PACKAGE);
	preferences.backup = 1;
	preferences.fcc = 0;
	preferences.tsvcalc = 0;
	preferences.tsvsortbydxcc = 0;
	preferences.tsvgroupbycallsign = 1;
	preferences.viewscoring = 1;
	preferences.scorex = 10;
	preferences.scorey = 300;
	preferences.scorewidth = 500;
	preferences.scoreheight = 300;
	preferences.scoringbands = g_new (gint, 30);
	preferences.scoringbands[0] = 0;
	preferences.scoringbands[1] = 0;
	preferences.scoringbands[2] = 1;
	preferences.scoringbands[3] = 1;
	preferences.scoringbands[4] = 0;
	preferences.scoringbands[5] = 1;
	preferences.scoringbands[6] = 1;
	preferences.scoringbands[7] = 1;
	preferences.scoringbands[8] = 1;
	preferences.scoringbands[9] = 1;
	preferences.scoringbands[10] = 1;
	preferences.scoringbands[11] = 1;
	preferences.scoringbands[12] = 0;
	preferences.scoringbands[13] = 0;
	preferences.scoringbands[14] = 0;
	preferences.scoringbands[15] = 0;
	preferences.scoringbands[16] = 0;
	preferences.scoringbands[17] = 0;
	preferences.scoringbands[18] = 0;
	preferences.scoringbands[19] = 0;
	preferences.scoringbands[20] = 0;
	preferences.scoringbands[21] = 0;
	preferences.scoringbands[22] = 0;
	preferences.scoringbands[23] = 0;
	preferences.scoringbands[24] = 0;
	preferences.scoringbands[25] = 0;
	preferences.scoringbands[26] = 0;
	preferences.scoringbands[27] = 0;
	preferences.scoringbands[28] = 0;
	preferences.scoringbands[29] = 0;
	preferences.distqrb = 0;
	preferences.awardswac = 0;
	preferences.awardswas = 0;
	preferences.awardswaz = 0;
	preferences.awardsiota = 0;
	preferences.awardsloc = 0;
	preferences.openurl = g_strdup ("http://www.qrz.com/db/<call>");
	preferences.areyousure = 1;
	preferences.initlastmsg = g_strdup ("");
}

/*
 * Load values from the configuration file and when failed load a fallback value
 * 
 * NOTE:
 * - when a value fails to load, a string will be NULL and an integer zero
 * - fallback values should be the same as in config_create above
 * 
 * because integers are zero when failed to load we add '1' when saving and
 * subtract '1' when loading, so zero values will be saved correctly
 */
static void config_load_from_keyfile (GKeyFile *file)
{
	gsize len = 0;

	preferences.x = g_key_file_get_integer (file, "mainwindow", "x", NULL);
	if (preferences.x == 0)
		preferences.x = 10;
	else
		preferences.x = preferences.x - 1;
	preferences.y = g_key_file_get_integer (file, "mainwindow", "y", NULL);
	if (preferences.y == 0)
		preferences.y = 30;
	else
		preferences.y = preferences.y - 1;
	preferences.width = g_key_file_get_integer (file, "mainwindow", "width", NULL);
	if (preferences.width == 0)
		preferences.width = 750;
	else
		preferences.width = preferences.width - 1;
	preferences.height = g_key_file_get_integer (file, "mainwindow", "height", NULL);
	if (preferences.height == 0)
		preferences.height = 540;
	else
		preferences.height = preferences.height - 1;

	preferences.scorex = g_key_file_get_integer (file, "scorewindow", "x", NULL);
	if (preferences.scorex == 0)
		preferences.scorex = 10;
	else
		preferences.scorex = preferences.scorex - 1;
	preferences.scorey = g_key_file_get_integer (file, "scorewindow", "y", NULL);
	if (preferences.scorey == 0)
		preferences.scorey = 300;
	else
		preferences.scorey = preferences.scorey - 1;
	preferences.scorewidth = g_key_file_get_integer (file, "scorewindow", "width", NULL);
	if (preferences.scorewidth == 0)
		preferences.scorewidth = 500;
	else
		preferences.scorewidth = preferences.scorewidth - 1;
	preferences.scoreheight = g_key_file_get_integer (file, "scorewindow", "height", NULL);
	if (preferences.scoreheight == 0)
		preferences.scoreheight = 300;
	else
		preferences.scoreheight = preferences.scoreheight - 1;

	preferences.b4x = g_key_file_get_integer (file, "b4window", "x", NULL);
	if (preferences.b4x == 0)
		preferences.b4x = 10;
	else
		preferences.b4x = preferences.b4x - 1;
	preferences.b4y = g_key_file_get_integer (file, "b4window", "y", NULL);
	if (preferences.b4y == 0)
		preferences.b4y = 30;
	else
		preferences.b4y = preferences.b4y - 1;
	preferences.b4width = g_key_file_get_integer (file, "b4window", "width", NULL);
	if (preferences.b4width == 0)
		preferences.b4width = 500;
	else
		preferences.b4width = preferences.b4width - 1;
	preferences.b4height = g_key_file_get_integer (file, "b4window", "height", NULL);
	if (preferences.b4height == 0)
		preferences.b4height = 300;
	else
		preferences.b4height = preferences.b4height - 1;

	preferences.b4columns2 = g_key_file_get_integer_list (file, "b4window", "columns", &len, NULL);
	if (!preferences.b4columns2)
	{
		preferences.b4columns2[0]=1;
		preferences.b4columns2[1]=1;
		preferences.b4columns2[2]=1;
		preferences.b4columns2[3]=1;
		preferences.b4columns2[4]=0;
		preferences.b4columns2[5]=1;
		preferences.b4columns2[6]=1;
		preferences.b4columns2[7]=1;
		preferences.b4columns2[8]=0;
		preferences.b4columns2[9]=0;
		preferences.b4columns2[10]=0;
		preferences.b4columns2[11]=0;
		preferences.b4columns2[12]=0;
		preferences.b4columns2[13]=0;
		preferences.b4columns2[14]=0;
		preferences.b4columns2[15]=0;
		preferences.b4columns2[16]=0;
		preferences.b4columns2[17]=0;
		preferences.b4columns2[18]=0;
		preferences.b4columns2[19]=0;
	}
	preferences.units = g_key_file_get_integer (file, "general", "units", NULL);
	if (preferences.units == 0)
		preferences.units = 1;
	else
		preferences.units = preferences.units - 1;
	preferences.bands = g_key_file_get_string (file, "general", "bands", NULL);
	if (!preferences.bands) preferences.bands = g_strdup
("0.136,0.501,1.8,3.5,5.2,7,10,14,18,21,24,28,50,70,144,222,420,902,1240,2300,3300,5650,10000,24000,47000,75500,120000,142000,241000,300000");
	preferences.modes = g_key_file_get_string (file, "general", "modes", NULL);
	if (!preferences.modes) preferences.modes = g_strdup
("SSB,CW,FM,AM,PSK31,RTTY,HELL,AMTORFEC,ASCI,ATV,CHIP64,CHIP128,CLO,CONTESTI,DSTAR,DOMINO,DOMINOF,FAX,FMHELL,FSK31,FSK441,GTOR,HELL80,HFSK,JT44,JT4A,JT4B,JT4C,JT4D,JT4E,JT4F,JT4G,JT9,JT9-1,JT9-2,JT9-5,JT9-10,JT9-30,JT65,JT65A,JT65B,JT65C,JT6M,MFSK8,MFSK16,MT63,OLIVIA,PAC,PAC2,PAC3,PAX,PAX2,PCW,PKT,PSK10,PSK63,PSK63F,PSK125,PSKAM10,PSKAM31,PSKAM50,PSKFEC31,PSKHELL,Q15,QPSK31,QPSK63,QPSK125,ROS,RTTYM,SSTV,THRB,THOR,THRBX,TOR,VOI,WINMOR,WSPR");
	preferences.typeaheadfind = g_key_file_get_integer (file, "general", "typeaheadfind", NULL);
	if (preferences.typeaheadfind == 0)
		preferences.typeaheadfind = 1;
	else
		preferences.typeaheadfind = preferences.typeaheadfind - 1;
	preferences.remoteadding = g_key_file_get_integer (file, "general", "remoteadding", NULL);
	if (preferences.remoteadding == 0)
		preferences.remoteadding = 1;
	else
		preferences.remoteadding = preferences.remoteadding - 1;
	preferences.distqrb = g_key_file_get_integer (file, "general", "distqrb", NULL);
	if (preferences.distqrb == 0)
		preferences.distqrb = 0;
	else
		preferences.distqrb = preferences.distqrb - 1;
	preferences.openurl = g_key_file_get_string (file, "general", "openurl", NULL);
	if (!preferences.openurl)
		preferences.openurl = g_strdup ("http://www.qrz.com/db/<call>");

	preferences.areyousure = g_key_file_get_integer (file, "general", "areyousure", NULL);
	if (preferences.areyousure == 0)
		preferences.areyousure = 1;
	else
		preferences.areyousure = preferences.areyousure - 1;
	preferences.viewscoring = g_key_file_get_integer (file, "appearance", "viewscoring", NULL);
	if (preferences.viewscoring == 0)
		preferences.viewscoring = 1;
	else
		preferences.viewscoring = preferences.viewscoring - 1;
	preferences.viewtoolbar = g_key_file_get_integer (file, "appearance", "viewtoolbar", NULL);
	if (preferences.viewtoolbar == 0)
		preferences.viewtoolbar = 1;
	else
		preferences.viewtoolbar = preferences.viewtoolbar - 1;
	preferences.viewb4 = g_key_file_get_integer (file, "appearance", "viewb4", NULL);
	if (preferences.viewb4 == 0)
		preferences.viewb4 = 0;
	else
		preferences.viewb4 = preferences.viewb4 - 1;
	preferences.bandseditbox = g_key_file_get_integer (file, "appearance", "bandseditbox", NULL);
	if (preferences.bandseditbox == 0)
		preferences.bandseditbox = 0;
	else
		preferences.bandseditbox = preferences.bandseditbox - 1;
	preferences.modeseditbox = g_key_file_get_integer (file, "appearance", "modeseditbox", NULL);
	if (preferences.modeseditbox == 0)
		preferences.modeseditbox = 0;
	else
		preferences.modeseditbox = preferences.modeseditbox - 1;
	preferences.bandoptionmenu = g_key_file_get_integer (file, "appearance", "bandoptionmenu", NULL);
	if (preferences.bandoptionmenu == 0)
		preferences.bandoptionmenu = 0;
	else
		preferences.bandoptionmenu = preferences.bandoptionmenu - 1;
	preferences.modeoptionmenu = g_key_file_get_integer (file, "appearance", "modeoptionmenu", NULL);
	if (preferences.modeoptionmenu == 0)
		preferences.modeoptionmenu = 0;
	else
		preferences.modeoptionmenu = preferences.modeoptionmenu - 1;
	preferences.handlebarpos = g_key_file_get_integer (file, "appearance", "handlebarpos", NULL);
	if (preferences.handlebarpos == 0)
		preferences.handlebarpos = 300;
	else
		preferences.handlebarpos = preferences.handlebarpos - 1;
	preferences.clock = g_key_file_get_integer (file, "appearance", "viewclock", NULL);
	if (preferences.clock == 0)
		preferences.clock = 0;
	else
		preferences.clock = preferences.clock - 1;
	preferences.callsign = g_key_file_get_string (file, "personal", "callsign", NULL);
	if (!preferences.callsign) preferences.callsign = g_strdup ("N0CALL");
	preferences.locator = g_key_file_get_string (file, "personal", "locator", NULL);
	if (!preferences.locator) preferences.locator = g_strdup ("AA00AA");
	preferences.latitude = g_key_file_get_double (file, "personal", "latitude", NULL);
	if (preferences.latitude == 0.0)
		preferences.latitude = 0.0;
	else
		preferences.latitude = preferences.latitude - 1.0;
	preferences.NS = g_key_file_get_integer (file, "personal", "NS", NULL);
	if (preferences.NS == 0)
		preferences.NS = 1;
	else
		preferences.NS = preferences.NS - 1;
	preferences.longitude = g_key_file_get_double (file, "personal", "longitude", NULL);
	if (preferences.longitude == 0.0)
		preferences.longitude = 0.0;
	else
		preferences.longitude = preferences.longitude - 1.0;
	preferences.EW = g_key_file_get_integer (file, "personal", "EW", NULL);
	if (preferences.EW == 0)
		preferences.EW = 1;
	else
		preferences.EW = preferences.EW - 1;

	preferences.hamlib = g_key_file_get_integer (file, "hamlib", "setting", NULL);
	if (preferences.hamlib == 0)
		preferences.hamlib = 0;
	else
		preferences.hamlib = preferences.hamlib - 1;
	preferences.rigid = g_key_file_get_integer (file, "hamlib", "rigid", NULL);
	if (preferences.rigid == 0)
		preferences.rigid = 1;
	else
		preferences.rigid = preferences.rigid - 1;
	preferences.device = g_key_file_get_string (file, "hamlib", "device", NULL);
#ifdef G_OS_WIN32
	if (!preferences.device) preferences.device = g_strdup ("com2");
#else
	if (!preferences.device) preferences.device = g_strdup ("/dev/ttyS1");
#endif
	preferences.rigconf = g_key_file_get_string (file, "hamlib", "rigconf", NULL);
	if (!preferences.rigconf) preferences.rigconf = g_strdup ("");
	preferences.round = g_key_file_get_integer (file, "hamlib", "round", NULL);
	if (preferences.round == 0)
		preferences.round = 3;
	else
		preferences.round = preferences.round - 1;
	preferences.polltime = g_key_file_get_integer (file, "hamlib", "polltime", NULL);
	if (preferences.polltime == 0)
		preferences.polltime = 0;
	else
		preferences.polltime = preferences.polltime - 1;
	preferences.fcc = g_key_file_get_integer (file, "hamlib", "fcc", NULL);
	if (preferences.fcc == 0)
		preferences.fcc = 0;
	else
		preferences.fcc = preferences.fcc - 1;

	preferences.defaultmhz = g_key_file_get_string (file, "defaults", "mhz", NULL);
	if (!preferences.defaultmhz) preferences.defaultmhz = g_strdup ("");
	preferences.defaultmode = g_key_file_get_string (file, "defaults", "mode", NULL);
	if (!preferences.defaultmode) preferences.defaultmode = g_strdup ("");
	preferences.defaulttxrst = g_key_file_get_string (file, "defaults", "txrst", NULL);
	if (!preferences.defaulttxrst) preferences.defaulttxrst = g_strdup ("");
	preferences.defaultrxrst = g_key_file_get_string (file, "defaults", "rxrst", NULL);
	if (!preferences.defaultrxrst) preferences.defaultrxrst = g_strdup ("");
	preferences.defaultawards = g_key_file_get_string (file, "defaults", "awards", NULL);
	if (!preferences.defaultawards) preferences.defaultawards = g_strdup ("");
	preferences.defaultpower = g_key_file_get_string (file, "defaults", "power", NULL);
	if (!preferences.defaultpower) preferences.defaultpower = g_strdup ("");
	preferences.defaultfreefield1 = g_key_file_get_string (file, "defaults", "freefield1", NULL);
	if (!preferences.defaultfreefield1) preferences.defaultfreefield1 = g_strdup ("");
	preferences.defaultfreefield2 = g_key_file_get_string (file, "defaults", "freefield2", NULL);
	if (!preferences.defaultfreefield2) preferences.defaultfreefield2 = g_strdup ("");
	preferences.defaultremarks = g_key_file_get_string (file, "defaults", "remarks", NULL);
	if (!preferences.defaultremarks) preferences.defaultremarks = g_strdup ("");

	preferences.cwf1 = g_key_file_get_string (file, "keyer", "f1", NULL);
	if (!preferences.cwf1) preferences.cwf1 = g_strdup ("cq de % % test");
	preferences.cwf2 = g_key_file_get_string (file, "keyer", "f2", NULL);
	if (!preferences.cwf2) preferences.cwf2 = g_strdup ("@ de %");
	preferences.cwf3 = g_key_file_get_string (file, "keyer", "f3", NULL);
	if (!preferences.cwf3) preferences.cwf3 = g_strdup ("@ ++5nn--#");
	preferences.cwf4 = g_key_file_get_string (file, "keyer", "f4", NULL);
	if (!preferences.cwf4) preferences.cwf4 = g_strdup ("tu 73");
	preferences.cwf5 = g_key_file_get_string (file, "keyer", "f5", NULL);
	if (!preferences.cwf5) preferences.cwf5 = g_strdup ("@");
	preferences.cwf6 = g_key_file_get_string (file, "keyer", "f6", NULL);
	if (!preferences.cwf6) preferences.cwf6 = g_strdup ("%");
	preferences.cwf7 = g_key_file_get_string (file, "keyer", "f7", NULL);
	if (!preferences.cwf7) preferences.cwf7 = g_strdup ("@ sri qso b4 gl");
	preferences.cwf8 = g_key_file_get_string (file, "keyer", "f8", NULL);
	if (!preferences.cwf8) preferences.cwf8 = g_strdup ("agn");
	preferences.cwf9 = g_key_file_get_string (file, "keyer", "f9", NULL);
	if (!preferences.cwf9) preferences.cwf9 = g_strdup ("?");
	preferences.cwf10 = g_key_file_get_string (file, "keyer", "f10", NULL);
	if (!preferences.cwf10) preferences.cwf10 = g_strdup ("qrz?");
	preferences.cwf11 = g_key_file_get_string (file, "keyer", "f11", NULL);
	if (!preferences.cwf11) preferences.cwf11 = g_strdup ("pse k");
	preferences.cwf12 = g_key_file_get_string (file, "keyer", "f12", NULL);
	if (!preferences.cwf12) preferences.cwf12 = g_strdup ("++++test---- % %");
	preferences.cwcq = g_key_file_get_string (file, "keyer", "cq", NULL);
	if (!preferences.cwcq) preferences.cwcq = g_strdup ("tu %");
	preferences.cwsp = g_key_file_get_string (file, "keyer", "sp", NULL);
	if (!preferences.cwsp) preferences.cwsp = g_strdup ("tu 5nn#");
	preferences.cwspeed = g_key_file_get_integer (file, "keyer", "speed", NULL);
	preferences.initlastmsg = g_key_file_get_string (file, "keyer", "initial_lastmessage", NULL);
	if (!preferences.initlastmsg) preferences.initlastmsg = g_strdup ("");

	preferences.scoringbands = g_key_file_get_integer_list (file, "scoring", "bands", &len, NULL);
	if (!preferences.scoringbands)
	{
		preferences.scoringbands[0] = 0;
		preferences.scoringbands[1] = 0;
		preferences.scoringbands[2] = 1;
		preferences.scoringbands[3] = 1;
		preferences.scoringbands[4] = 0;
		preferences.scoringbands[5] = 1;
		preferences.scoringbands[6] = 1;
		preferences.scoringbands[7] = 1;
		preferences.scoringbands[8] = 1;
		preferences.scoringbands[9] = 1;
		preferences.scoringbands[10] = 1;
		preferences.scoringbands[11] = 1;
		preferences.scoringbands[12] = 0;
		preferences.scoringbands[13] = 0;
		preferences.scoringbands[14] = 0;
		preferences.scoringbands[15] = 0;
		preferences.scoringbands[16] = 0;
		preferences.scoringbands[17] = 0;
		preferences.scoringbands[18] = 0;
		preferences.scoringbands[19] = 0;
		preferences.scoringbands[20] = 0;
		preferences.scoringbands[21] = 0;
		preferences.scoringbands[22] = 0;
		preferences.scoringbands[23] = 0;
		preferences.scoringbands[24] = 0;
		preferences.scoringbands[25] = 0;
		preferences.scoringbands[26] = 0;
		preferences.scoringbands[27] = 0;
		preferences.scoringbands[28] = 0;
		preferences.scoringbands[29] = 0;
	}
	preferences.awardswac = g_key_file_get_integer (file, "scoring", "wac", NULL);
	if (preferences.awardswac == 0)
		preferences.awardswac = 0;
	else
		preferences.awardswac = preferences.awardswac - 1;
	preferences.awardswas = g_key_file_get_integer (file, "scoring", "was", NULL);
	if (preferences.awardswas == 0)
		preferences.awardswas = 0;
	else
		preferences.awardswas = preferences.awardswas - 1;
	preferences.awardswaz = g_key_file_get_integer (file, "scoring", "waz", NULL);
	if (preferences.awardswaz == 0)
		preferences.awardswaz = 0;
	else
		preferences.awardswaz = preferences.awardswaz - 1;
	preferences.awardsiota = g_key_file_get_integer (file, "scoring", "iota", NULL);
	if (preferences.awardsiota == 0)
		preferences.awardsiota = 0;
	else
		preferences.awardsiota = preferences.awardsiota - 1;
	preferences.awardsloc = g_key_file_get_integer (file, "scoring", "loc", NULL);
	if (preferences.awardsloc == 0)
		preferences.awardsloc = 0;
	else
		preferences.awardsloc = preferences.awardsloc - 1;

	preferences.logstoload = g_key_file_get_string (file, "logs", "logstoload", NULL);
	if (!preferences.logstoload) preferences.logstoload = g_strdup("*");
	preferences.logorder = g_key_file_get_integer (file, "logs", "logorder", NULL);
	if (preferences.logorder == 0)
		preferences.logorder = 0;
	else
		preferences.logorder = preferences.logorder - 1;
	preferences.logfont = g_key_file_get_string (file, "logs", "font", NULL);
	if (!preferences.logfont) preferences.logfont = g_strdup("Sans 10");
	preferences.savedir = g_key_file_get_string (file, "logs", "savedir", NULL);
	if (!preferences.savedir) preferences.savedir = g_strdup_printf
		("%s%s.%s",	g_get_home_dir (), G_DIR_SEPARATOR_S, PACKAGE);
	preferences.autosave = g_key_file_get_integer (file, "logs", "autosave", NULL);
	if (preferences.autosave == 0)
		preferences.autosave = 5;
	else
		preferences.autosave = preferences.autosave - 1;
	preferences.saving = g_key_file_get_integer (file, "logs", "saving", NULL);
	if (preferences.saving == 0)
		preferences.saving = 1;
	else
		preferences.saving = preferences.saving - 1;
	preferences.backupdir = g_key_file_get_string (file, "logs", "backupdir", NULL);
	if (!preferences.backupdir) preferences.backupdir = g_strdup_printf
		("%s%s.%s",	g_get_home_dir (), G_DIR_SEPARATOR_S, PACKAGE);
	preferences.backup = g_key_file_get_integer (file, "logs", "backup", NULL);
	if (preferences.backup == 0)
		preferences.backup = 1;
	else
		preferences.backup = preferences.backup - 1;
	preferences.freefield1 = g_key_file_get_string (file, "logs", "freefield1", NULL);
	if (!preferences.freefield1) preferences.freefield1 = g_strdup("UNKNOWN");
	preferences.freefield2 = g_key_file_get_string (file, "logs", "freefield2", NULL);
	if (!preferences.freefield2) preferences.freefield2 = g_strdup("UNKNOWN");
	preferences.logcwidths2 = g_key_file_get_integer_list (file, "logs", "columnwidths", &len, NULL);
	if (!preferences.logcwidths2)
	{
		preferences.logcwidths2[0]=50;
		preferences.logcwidths2[1]=100;
		preferences.logcwidths2[2]=50;
		preferences.logcwidths2[3]=50;
		preferences.logcwidths2[4]=100;
		preferences.logcwidths2[5]=50;
		preferences.logcwidths2[6]=50;
		preferences.logcwidths2[7]=50;
		preferences.logcwidths2[8]=50;
		preferences.logcwidths2[9]=100;
		preferences.logcwidths2[10]=50;
		preferences.logcwidths2[11]=50;
		preferences.logcwidths2[12]=50;
		preferences.logcwidths2[13]=100;
		preferences.logcwidths2[14]=100;
		preferences.logcwidths2[15]=100;
		preferences.logcwidths2[16]=100;
		preferences.logcwidths2[17]=100;
		preferences.logcwidths2[18]=200;	
	}

	preferences.saveasadif = g_key_file_get_integer (file, "saveas", "adif", NULL);
	if (preferences.saveasadif == 0)
		preferences.saveasadif = 0;
	else
		preferences.saveasadif = preferences.saveasadif - 1;

	preferences.saveascabrillo = g_key_file_get_integer (file, "saveas", "cabrillo", NULL);
	if (preferences.saveascabrillo == 0)
		preferences.saveascabrillo = 0;
	else
		preferences.saveascabrillo = preferences.saveascabrillo - 1;

	preferences.saveastsv2 = g_key_file_get_integer_list (file, "saveas", "tsvcolumns", &len, NULL);
	if (!preferences.saveastsv2)
	{
		preferences.saveastsv2[0] = 1;
		preferences.saveastsv2[1] = 1;
		preferences.saveastsv2[2] = 0;
		preferences.saveastsv2[3] = 1;
		preferences.saveastsv2[4] = 1;
		preferences.saveastsv2[5] = 1;
		preferences.saveastsv2[6] = 1;
		preferences.saveastsv2[7] = 0;
		preferences.saveastsv2[8] = 0;
		preferences.saveastsv2[9] = 0;
		preferences.saveastsv2[10] = 0;
		preferences.saveastsv2[11] = 0;
		preferences.saveastsv2[12] = 0;
		preferences.saveastsv2[13] = 0;
		preferences.saveastsv2[14] = 0;
		preferences.saveastsv2[15] = 0;
		preferences.saveastsv2[16] = 0;
		preferences.saveastsv2[17] = 0;
	}
	preferences.tsvcalc = g_key_file_get_integer (file, "saveas", "tsvcalc", NULL);
	if (preferences.tsvcalc == 0)
		preferences.tsvcalc = 0;
	else
		preferences.tsvcalc = preferences.tsvcalc - 1;
	preferences.tsvsortbydxcc = g_key_file_get_integer (file, "saveas", "tsvsortbydxcc", NULL);
	if (preferences.tsvsortbydxcc == 0)
		preferences.tsvsortbydxcc = 0;
	else
		preferences.tsvsortbydxcc = preferences.tsvsortbydxcc - 1;
	preferences.tsvgroupbycallsign = g_key_file_get_integer (file, "saveas", "tsvgroupbycallsign", NULL);
	if (preferences.tsvgroupbycallsign == 0)
		preferences.tsvgroupbycallsign = 1;
	else
		preferences.tsvgroupbycallsign = preferences.tsvgroupbycallsign - 1;
}

/*
 * Load the configuration file 
 */
void
loadpreferences (void)
{
	gchar *path = g_strconcat (g_get_home_dir(), G_DIR_SEPARATOR_S, ".xlog/xlog.cfg", NULL);

	config_create ();
	if (g_file_test(path, G_FILE_TEST_EXISTS))
	{
		GKeyFile *file = g_key_file_new ();
		g_key_file_load_from_file (file, path, G_KEY_FILE_KEEP_COMMENTS, NULL);
		config_load_from_keyfile (file);
		g_key_file_free(file);
	}
	g_free(path);
}

/*
 * Save configuration file
 * 
 * NOTE: add 1 to integers and doubles, see text above
 * config_load_from_keyfile() for explanation....
 */
void
savepreferences (void)
{
	GKeyFile *file = g_key_file_new();
	gchar *path = g_strconcat (g_get_home_dir(), G_DIR_SEPARATOR_S, ".xlog/xlog.cfg", NULL);
	g_key_file_load_from_file(file, path, G_KEY_FILE_KEEP_COMMENTS, NULL);

	g_key_file_set_integer (file, "mainwindow", "x", preferences.x + 1);
	g_key_file_set_integer (file, "mainwindow", "y", preferences.y + 1);
	g_key_file_set_integer (file, "mainwindow", "width", preferences.width + 1);
	g_key_file_set_integer (file, "mainwindow", "height", preferences.height + 1);
	g_key_file_set_integer (file, "scorewindow", "x", preferences.scorex + 1);
	g_key_file_set_integer (file, "scorewindow", "y", preferences.scorey + 1);
	g_key_file_set_integer (file, "scorewindow", "width", preferences.scorewidth + 1);
	g_key_file_set_integer (file, "scorewindow", "height", preferences.scoreheight + 1);

	g_key_file_set_integer (file, "b4window", "x", preferences.b4x + 1);
	g_key_file_set_integer (file, "b4window", "y", preferences.b4y + 1);
	g_key_file_set_integer (file, "b4window", "width", preferences.b4width + 1);
	g_key_file_set_integer (file, "b4window", "height", preferences.b4height + 1);
	g_key_file_set_integer_list (file, "b4window", "columns", preferences.b4columns2, 20);

	g_key_file_set_integer (file, "general", "units", preferences.units + 1);
	g_key_file_set_string (file, "general", "bands", preferences.bands);
	g_key_file_set_string (file, "general", "modes", preferences.modes);
	g_key_file_set_integer (file, "general", "typeaheadfind", preferences.typeaheadfind + 1);
	g_key_file_set_integer (file, "general", "remoteadding", preferences.remoteadding + 1);
	g_key_file_set_integer (file, "general", "distqrb", preferences.distqrb + 1);
	g_key_file_set_string (file, "general", "openurl", preferences.openurl);
	g_key_file_set_integer (file, "general", "areyousure", preferences.areyousure + 1);

	g_key_file_set_integer (file, "appearance", "viewscoring", preferences.viewscoring + 1);
	g_key_file_set_integer (file, "appearance", "viewtoolbar", preferences.viewtoolbar + 1);
	g_key_file_set_integer (file, "appearance", "viewb4", preferences.viewb4 + 1);
	g_key_file_set_integer (file, "appearance", "bandseditbox", preferences.bandseditbox + 1);
	g_key_file_set_integer (file, "appearance", "modeseditbox", preferences.modeseditbox + 1);
	g_key_file_set_integer (file, "appearance", "bandoptionmenu", preferences.bandoptionmenu + 1);
	g_key_file_set_integer (file, "appearance", "modeoptionmenu", preferences.modeoptionmenu + 1);
	g_key_file_set_integer (file, "appearance", "handlebarpos", preferences.handlebarpos + 1);
	g_key_file_set_integer (file, "appearance", "viewclock", preferences.clock + 1);

	g_key_file_set_string (file, "personal", "callsign", preferences.callsign);
	g_key_file_set_string (file, "personal", "locator", preferences.locator);
	g_key_file_set_double (file, "personal", "latitude", preferences.latitude + 1.0);
	g_key_file_set_integer (file, "personal", "NS", preferences.NS + 1);
	g_key_file_set_double (file, "personal", "longitude", preferences.longitude + 1.0);
	g_key_file_set_integer (file, "personal", "EW", preferences.EW + 1);

	g_key_file_set_integer (file, "hamlib", "setting", preferences.hamlib + 1);
	g_key_file_set_integer (file, "hamlib", "rigid", preferences.rigid + 1);
	g_key_file_set_string (file, "hamlib", "device", preferences.device);
	g_key_file_set_string (file, "hamlib", "rigconf", preferences.rigconf);
	g_key_file_set_integer (file, "hamlib", "round", preferences.round + 1);
	g_key_file_set_integer (file, "hamlib", "polltime", preferences.polltime + 1);
	g_key_file_set_integer (file, "hamlib", "fcc", preferences.fcc + 1);

	g_key_file_set_string (file, "defaults", "mhz", preferences.defaultmhz);
	g_key_file_set_string (file, "defaults", "mode", preferences.defaultmode);
	g_key_file_set_string (file, "defaults", "txrst", preferences.defaulttxrst);
	g_key_file_set_string (file, "defaults", "rxrst", preferences.defaultrxrst);
	g_key_file_set_string (file, "defaults", "awards", preferences.defaultawards);
	g_key_file_set_string (file, "defaults", "power", preferences.defaultpower);
	g_key_file_set_string (file, "defaults", "freefield1", preferences.defaultfreefield1);
	g_key_file_set_string (file, "defaults", "freefield2", preferences.defaultfreefield2);
	g_key_file_set_string (file, "defaults", "remarks", preferences.defaultremarks);

	g_key_file_set_string (file, "keyer", "f1", preferences.cwf1);
	g_key_file_set_string (file, "keyer", "f2", preferences.cwf2);
	g_key_file_set_string (file, "keyer", "f3", preferences.cwf3);
	g_key_file_set_string (file, "keyer", "f4", preferences.cwf4);
	g_key_file_set_string (file, "keyer", "f5", preferences.cwf5);
	g_key_file_set_string (file, "keyer", "f6", preferences.cwf6);
	g_key_file_set_string (file, "keyer", "f7", preferences.cwf7);
	g_key_file_set_string (file, "keyer", "f8", preferences.cwf8);
	g_key_file_set_string (file, "keyer", "f9", preferences.cwf9);
	g_key_file_set_string (file, "keyer", "f10", preferences.cwf10);
	g_key_file_set_string (file, "keyer", "f11", preferences.cwf11);
	g_key_file_set_string (file, "keyer", "f12", preferences.cwf12);
	g_key_file_set_string (file, "keyer", "cq", preferences.cwcq);
	g_key_file_set_string (file, "keyer", "sp", preferences.cwsp);
	g_key_file_set_integer (file, "keyer", "speed", preferences.cwspeed);
	g_key_file_set_string (file, "keyer", "initial_lastmessage", preferences.initlastmsg);

	g_key_file_set_integer_list (file, "scoring", "bands", preferences.scoringbands, 30);
	g_key_file_set_integer (file, "scoring", "wac", preferences.awardswac + 1);
	g_key_file_set_integer (file, "scoring", "was", preferences.awardswas + 1);
	g_key_file_set_integer (file, "scoring", "waz", preferences.awardswaz + 1);
	g_key_file_set_integer (file, "scoring", "iota", preferences.awardsiota + 1);
	g_key_file_set_integer (file, "scoring", "loc", preferences.awardsloc + 1);

	g_key_file_set_string (file, "logs", "logstoload", preferences.logstoload);
	g_key_file_set_integer (file, "logs", "logorder", preferences.logorder + 1);
	g_key_file_set_string (file, "logs", "font", preferences.logfont);
	g_key_file_set_string (file, "logs", "savedir", preferences.savedir);
	g_key_file_set_integer (file, "logs", "autosave", preferences.autosave + 1);
	g_key_file_set_integer (file, "logs", "saving", preferences.saving + 1);
	g_key_file_set_string (file, "logs", "backupdir", preferences.backupdir);
	g_key_file_set_integer (file, "logs", "backup", preferences.backup + 1);
	g_key_file_set_string (file, "logs", "freefield1", preferences.freefield1);
	g_key_file_set_string (file, "logs", "freefield2", preferences.freefield2);
	g_key_file_set_integer_list (file, "logs", "columnwidths", preferences.logcwidths2, 19);

	g_key_file_set_integer (file, "saveas", "adif", preferences.saveasadif + 1);
	g_key_file_set_integer (file, "saveas", "cabrillo", preferences.saveascabrillo + 1);
	g_key_file_set_integer_list (file, "saveas", "tsvcolumns", preferences.saveastsv2, 18);
	g_key_file_set_integer (file, "saveas", "tsvcalc", preferences.tsvcalc + 1);
	g_key_file_set_integer (file, "saveas", "tsvsortbydxcc", preferences.tsvsortbydxcc + 1);
	g_key_file_set_integer (file, "saveas", "tsvgroupbycallsign", preferences.tsvgroupbycallsign + 1);

	gchar *buffer = g_key_file_to_data(file, NULL, NULL);
	g_key_file_free(file);
	GIOChannel *channel = g_io_channel_new_file (path, "w", NULL);
	g_io_channel_write_chars (channel, buffer, -1, NULL, NULL);
	g_io_channel_unref (channel);
	g_free (buffer);
	g_free (path);
}
