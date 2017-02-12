/*

   sendtoxlog - example for retrieving data from xlog
   Copyright (C) 2002 -2005 Joop Stakenborg <pg4i@amsat.org>
  
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
 * Compile with 'gcc -o fromxlog fromxlog.c'
 * See the provided README for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define KEY 6146		/* unique key */
#define SHMSIZE	40		/* size of shared memory */

int
main (void)
{
	int shmid = -1;
	void *pt;

	for (;;)
	{				/* allocate a shared memory segment */
		if ((shmid = shmget ((key_t) KEY, SHMSIZE, 0600)) < 0)
		{
			fprintf (stderr, "No logging program...\n");
			shmid = -1;
		}
		if (shmid != -1)
		{			/* attach	the	shared memory segment */
			if ((pt = (shmat (shmid, NULL, 0))) == (void *) -1)
			{
				perror ("fromlog - shmat");
				exit (1);
			}
			fprintf (stdout, "%s\n", (char *) pt);
			shmdt ((void *) pt);	/* detach	the	shared	memory	segment */
		}
		sleep (1);
	}
	exit (0);
}
