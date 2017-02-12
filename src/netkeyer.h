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
 * netkeyer.h
 */

#define K_MESSAGE 1
#define K_SPEED 2
#define K_ABORT 4

/* structure used for keyer state */
typedef struct
{
	gboolean cqmode;
	gboolean shortnr;
}
keyerstatetype;

gint tonetkeyer (gint cw_op, gchar *cwmessage);
void netkeyer_close (void);
gint netkeyer_init (void);
