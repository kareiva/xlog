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

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifndef G_OS_WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <errno.h>

#include "netkeyer.h"
#include "support.h"

#ifndef G_OS_WIN32
static gint socket_descriptor;
static struct sockaddr_in address;
#endif

/* initialize the UDP socket for cwdaemon */
gint
netkeyer_init (void) 
{
#ifndef G_OS_WIN32
	gint netkeyer_port = 6789;
	gchar netkeyer_hostaddress[16] = "127.0.0.1";
	struct hostent *hostbyname;

	hostbyname = gethostbyname (netkeyer_hostaddress);
	if (hostbyname == NULL) return 1;

	bzero (&address, sizeof (address));
	address.sin_family = AF_INET;
	memcpy (&address.sin_addr.s_addr, hostbyname->h_addr,
		 sizeof (address.sin_addr.s_addr));
	address.sin_port = htons (netkeyer_port);
	
	socket_descriptor = socket (AF_INET, SOCK_DGRAM, 0);
	if (socket_descriptor == -1) return 2;
#endif
	return 0;
}

/* close UDP socket */
void
netkeyer_close (void)
{
#ifndef G_OS_WIN32
	gint close_rc;

	close_rc = close (socket_descriptor);
	if (close_rc == -1)
		g_warning (_("closing of socket failed"));
#endif
}

/* use sendto to transmit a message over the socket, the only control messages
   we use here are speed and abort */
gint
tonetkeyer (gint cw_op, gchar *cwmessage) 
{
	ssize_t sendto_rc = 0;
#ifndef G_OS_WIN32
	gchar buf[80];
	gsize length = 0;
	gchar *upcasestr;

	if (cwmessage)
	{
		upcasestr = g_utf8_strup (cwmessage, -1);
		cwmessage = g_convert(upcasestr, -1, "ISO-8859-1", "UTF-8", NULL, &length, NULL);
		g_free (upcasestr);
	}
	switch (cw_op)
	{

		case K_MESSAGE:	// cw message
			g_snprintf(buf, 80, "%s", cwmessage);
		break;
		case K_SPEED:	// speed
			g_snprintf(buf, 80, "%c2%s", 27, cwmessage);
		break;
		case K_ABORT:	// message abort
			g_snprintf(buf, 80, "%c4", 27);
		break;
		default:
			buf[0] = '\0';
	}
	
	if (buf[0] != '\0')
	{
		sendto_rc = sendto (socket_descriptor, buf, sizeof (buf), 0,
				(struct sockaddr *) &address, sizeof (address));
	}
	else
		sendto_rc = -1;
	g_free (cwmessage);
#endif
	return sendto_rc;
}

