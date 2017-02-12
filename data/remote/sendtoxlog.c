/*

   sendtoxlog - example for sending data to xlog from a second application
   Copyright (C) 2001-2005 Joop Stakenborg <pg4i@amsat.org>
  
   This program is free software: you can redistribute it and/or modify
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
 *
 * Compile with 'gcc -o sendtoxlog sendtoxlog.c'
 * See the provided README for more details.
 *
 */

/* definition of some strings to send over */
#define DEMO1	"program:sendtoxlog\1version:1\1date:29 Jul 2001\1time:2214\1call:wa0eir"
#define DEMO2	"program:sendtoxlog\1version:1\1date:30 Dec 2001\1time:2214\1endtime:2220\
\1call:pg4i\1mhz:14\1mode:cw\1tx:579\1rx:569\1name:joop\1qth:houten\1notes:this\nis DEMO2\
\1power:100W\1locator:JO22OB\1free1:testfree1\1free2:testfree2"
#define DEMO3	"program:sendtoxlog\1version:1\1call:f8cfe"
#define DEMO4	"program:sendtoxlog\1version:1\1date:26 Aug 2003\1time:2214\1endtime:2220\
\1call:pg4i\1mhz:HAMLIB\1mode:HAMLIB\1tx:HAMLIB\1rx:569\1name:joop\1qth:houten\1notes:this is DEMO4\
\1power:HAMLIB\1locator:JO22OB\1free1:testfree1\1free2:testfree2"
#define DEMO5	"program:sendtoxlog\1version:1\1call:pg4i\1notes:áóèúéà"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

extern int errno;

/* structure needed for the message queue */
typedef struct
{
	long mtype;				/* mtype should always be 88 */
	char mtext[1024];	/* mtext holds the message */
}
msgtype;

msgtype msgbuf;
int msgid;

/* this is the function which does the actual sending over the message queue */
static void
sendtoxlog (char *message)
{
	strcpy (msgbuf.mtext, message);
	if (msgsnd (msgid, (void *) &msgbuf, 1024, 0) == -1)
		fprintf (stderr, "msgsnd failed: %s\n", strerror (errno));
	else
		fprintf (stdout, "message sent OK\n");
}

int
main (int argc, char *argv[])
{

	/* first some command line parsing */
	if (argc == 1 || argc > 2)
		{
			fprintf (stdout, "Sendtoxlog, program which demonstrates how to send remote data to xlog.\n");
			fprintf (stdout, "	Usage: sendtoxlog [number], where number can be 1, 2, 3, 4 or 5 ...\n");
			fprintf (stdout, "		 sendtoxlog 1, demonstrates date time and callsign\n");
			fprintf (stdout, "		 sendtoxlog 2, demonstrates all entries\n");
			fprintf (stdout, "		 sendtoxlog 3, demonstrates only the callsign field\n");
			fprintf (stdout, "		 sendtoxlog 4, demonstrates hamlib capabilities\n");
			fprintf (stdout, "		 sendtoxlog 5, test language characters\n");
			exit (0);
		}
	if (strcmp (argv[1], "1") != 0 && strcmp (argv[1], "2") != 0
			&& strcmp (argv[1], "3") != 0 && strcmp (argv[1], "4") != 0
			&& strcmp (argv[1], "5") != 0)
		{
			fprintf (stdout, "Argument for sendtoxlog is '1', '2', '3', '4' or '5',");
			fprintf (stdout, "	type sendtoxlog for help.\n");
			exit (1);
		}

	/* create the message queue, you need to use 1238 for the key to talk to xlog */
	msgid = msgget ((key_t) 1238, 0666 | IPC_CREAT);
	if (msgid == -1)
		{
			fprintf (stderr, "msgget failed: %s\n", strerror (errno));
			exit (1);
		}
	/* you need to set mtype to 88 */
	msgbuf.mtype = 88;

	/* here we go */

	/* demo 1 */
	if (strcmp (argv[1], "1") == 0)
		sendtoxlog (DEMO1);
	/* demo 2 */
	if (strcmp (argv[1], "2") == 0)
		sendtoxlog (DEMO2);
	/* demo 3 */
	if (strcmp (argv[1], "3") == 0)
		sendtoxlog (DEMO3);
	/* demo 4 */
	if (strcmp (argv[1], "4") == 0)
		sendtoxlog (DEMO4);
	/* demo 5 */
	if (strcmp (argv[1], "5") == 0)
		sendtoxlog (DEMO5);

/* 
 * WARNING: do not remove the message queue with 
 * "msgctl(msgid, IPC_RMID, 0)", xlog will take care of it
 */
	exit (0);
}
