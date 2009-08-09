/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * GP2X: Memory tweaking stuff.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "backends/platform/gp2x/gp2x-mem.h"

void SetClock (unsigned c)
{
	unsigned v;
	unsigned mdiv,pdiv=3,scale=0;

	// Set ARM920t clock
	c *= 1000000;
	mdiv = (c*pdiv) / SYS_CLK_FREQ;
    mdiv = ((mdiv-8)<<8) & 0xff00;
	pdiv = ((pdiv-2)<<2) & 0xfc;
    scale &= 3;
    v = mdiv | pdiv | scale;
    gp2x_memregs[0x910>>1] = v;
}

void patchMMU (void)
{
	//volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

	printf ("Reconfiguring cached memory regions...\n");

	//hackpgtable();
	//printf ("Sucess...\n");

	system("/sbin/rmmod mmuhack");
	system("/sbin/insmod -f mmuhack.o");

	int mmufd = open("/dev/mmuhack", O_RDWR);

	if(mmufd < 0)
	{
		printf ("Upper memory uncached (attempt failed, access to upper memory will be slower)...\n");
	}
	else
	{
		printf ("Upper memory cached...\n");
		close(mmufd);
	}
}

void unpatchMMU (void)
{
	printf ("Restoreing cached memory regions...\n");
	system("/sbin/rmmod mmuhack");
}
