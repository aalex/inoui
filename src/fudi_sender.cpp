/*
 * This file is part of Spatosc.
 *
 * Copyright (c) 2000 Miller Puckette
 * Copyright (c) 2010 Society for Arts and Technologies <info@sat.qc.ca>
 *
 * Spatosc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Spatosc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Spatosc.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Copyright (c) 2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in the Pd distribution.  */

/*
 * g++ -Wall -o send_fudi fudi_translator.cpp
 */

#include "fudi_sender.h"
#include "unused.h"
#include <sys/types.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <errno.h>
#include <stdlib.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

namespace inoui
{

#define BUFSIZE 4096

FudiSender::FudiSender(std::string host, unsigned int port, bool isTcp) :
    host_(host),
    port_(port),
    protocol_(0)
{
    if (isTcp)
        protocol_ = SOCK_STREAM;
    else
        protocol_ = SOCK_DGRAM;
}

bool FudiSender::sendFudi(const std::string &message)
{
    int sockfd, portno, protocol;
    struct sockaddr_in server;
    struct hostent *hp;
    const char *hostname;
#ifdef WIN32
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#endif
    hostname = host_.c_str();
    protocol = protocol_;
    portno = (int) port_;
#ifdef WIN32
    if (WSAStartup(version, &nobby))
        sockerror((char *) "WSAstartup");
#endif

    sockfd = socket(AF_INET, protocol, 0);
    if (sockfd < 0)
    {
        sockerror((char *) "socket()");
        return false;
    }
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname);
    if (hp == 0)
    {
        fprintf(stderr, "%s: unknown host\n", hostname);
        x_closesocket(sockfd);
        return false;
    }
    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((unsigned short) portno);

    /* try to connect.  */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        sockerror((char *) "connect");
        x_closesocket(sockfd);
        return false;
    }
    /* now loop reading stdin and sending  it to socket */
    char buf[BUFSIZE];
    strncpy(buf, message.c_str(), BUFSIZE);
    char *bp;
    int nsent, nsend;
    nsend = strlen(buf);
    //printf("sending=%s     length=%d\n", buf, nsend);
    for (bp = buf, nsent = 0; nsent <= nsend;)
    {
        int res = send(sockfd, bp, nsend - nsent, 0);
        if (res < 0)
        {
            sockerror((char *) "send");
            break;
        }
        nsent += res;
        bp += res;
        // printf("nsent=%d / %d\n", nsent, nsend);
        if (nsent >= nsend)
            break;
    }
    x_closesocket(sockfd);
    return true;
}

void FudiSender::sockerror(char *s)
{
#ifdef WIN32
    int err = WSAGetLastError();
    if (err == 10054)
        return;
    else if (err == 10044)
    {
        fprintf(stderr, "Warning: you might not have TCP/IP \"networking\" turned on\n");
    }
#else
    int err = errno;
#endif
    fprintf(stderr, "%s: %s (%d)\n", s, strerror(err), err);
}

void FudiSender::x_closesocket(int fd)
{
#ifdef WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

#if 0
int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    FudiSender sender("localhost", 31337, true);
    sender.sendFudi("hello 1 2 3.14159;");
    return 0;
}
#endif

} // end namespace spatosc

