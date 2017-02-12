/*

   client - example for sending data to xlog from a second application
   using sockets
  
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
 * Compile with 'gcc -o client client.c'
 * See the provided README for more details.

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char ch[1024];

/*  Create a socket for the client.  */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

/*  Name the socket, as agreed with the server.  */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons (7311);
    len = sizeof(address);

/*  Now connect our socket to the server's socket.  */

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if(result == -1) {
        perror("oops: Connect to server failed");
        exit(1);
    }

/*  We can now read/write via sockfd.  */

    strcpy(ch, "program:Marote\1call:K6EEP\1mhz:28.058\1mode:SSB\1rx:456\1name:Mike\1");
    len = sizeof ch;
    write(sockfd, &ch, len);
    close(sockfd);
    exit(0);
}
