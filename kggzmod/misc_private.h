/*
    This file is part of the kggzmod library.
    Copyright (c) 2005 - 2007 Josef Spillner <josef@ggzgamingzone.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KGGZMOD_MISC_PRIVATE_H
#define KGGZMOD_MISC_PRIVATE_H

#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

namespace KGGZMod
{

// The following function is a variant of libggz's ggz_read_fd()
// All the historic Unix compatibility has been removed
// The return code has been C++-ified
// All GGZ debug/error handling code has been removed

bool readfiledescriptor(int sock, int *recvfd)
{
	struct msghdr msg;
	struct iovec iov[1];
	ssize_t	n;
        char dummy;

	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

        /* We're just sending a fd, so it's a dummy byte */
	iov[0].iov_base = &dummy;
	iov[0].iov_len = 1;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if ( (n = recvmsg(sock, &msg, 0)) < 0) {
		//ggz_debug(GGZ_SOCKET_DEBUG, "Error reading fd msg.");
		return false;
	}

        if (n == 0) {
		//ggz_debug(GGZ_SOCKET_DEBUG, "Warning: fd is closed.");
	        return false;
	}

        if ( (cmptr = CMSG_FIRSTHDR(&msg)) == NULL
	     || cmptr->cmsg_len != CMSG_LEN(sizeof(int))) {
		//ggz_debug(GGZ_SOCKET_DEBUG, "Bad cmsg.");
		return false;
	}

	if (cmptr->cmsg_level != SOL_SOCKET) {
		//ggz_debug(GGZ_SOCKET_DEBUG, "Bad cmsg.");
		return false;
	}

	if (cmptr->cmsg_type != SCM_RIGHTS) {
		//ggz_debug(GGZ_SOCKET_DEBUG, "Bad cmsg.");
		return false;
	}

	/* Everything is good */
	*recvfd = *((int *) CMSG_DATA(cmptr));
	
        return true;
}

}

#endif

