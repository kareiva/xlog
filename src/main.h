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
 * main.h - program state
 */

/* structure used for program state */
typedef struct
{
	gint countries;         /* number of countries loaded */
	gint qsos;              /* number of qso's read from the logs */
	gboolean controlkey;    /* control key is pressed */
	long long rigfrequency; /* frequency read from the rig */
	guint rigmode;          /* mode read from the rig */
	gchar *rigrst;          /* signal strength read from rig */
	guint rigpower;         /* rf power */
	gint scounter;          /* counter for s-levels stored in array */
	gint hlcounter;         /* counter for hamlib */
	gboolean tx;            /* transmitting or receiving */
	gboolean statustimer;   /* 'ready' timer for the statusbar */
	gint shmid;             /* id for shared memory */
	gint logwindows;        /* number of logwindows */
	gchar *searchstr;       /* array with logs/qsos seached */
	gint dupecheck;         /* dupe check this log or all logs */
	gboolean notdupecheckmode;  /* exclude bands from dupecheck */
	gboolean notdupecheckband;  /* exclude modes from dupecheck */
	gboolean utf8error;     /* error in utf-8 conversion when reading the log */
	gchar *importremark;	/* remark added when importing from trlog or cabrillo */
	gchar *px;              /* prefix lookup used for countrymap */
	gboolean warning_nologopen;	/* No log open while receiving remote data warning dialog */
}
programstatetype;
