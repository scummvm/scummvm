/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2005-2006 John Willis (Portions of the GP2X Backend)
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

#include "gp2x-mem.h"

void InitRam (void)
{
	if(!gp2x_dev)
	{
		gp2x_dev = open("/dev/mem", O_RDWR);
		gp2x_ram = (unsigned short *)mmap(0, 0x10000, 3, 1, gp2x_dev, 0x03000000);
		gp2x_memregs = (unsigned short *)mmap(0, 0x10000, 3, 1, gp2x_dev, 0xc0000000);
	}
}

void CloseRam (void)
{
	if(gp2x_dev) close(gp2x_dev);
}

/*
****** [BEGIN] Squidge's MMU hack code ******
*/

int myuname(char *buffer)
{
	asm volatile ("swi #0x90007a");
}

void DecodeCoarse (unsigned int indx, unsigned int sa)
{
	unsigned int cpt[256];
	unsigned int temp;
	unsigned int i = 0;
    unsigned int wb = 0;

    sa &= 0xfffffc00;
    indx *= 1048576;

    //printf ("  > %08X\n", sa);
    //printf ("%d\n",
    lseek (gp2x_dev, sa, SEEK_SET);
    memset (cpt, 0, 256*4);
    temp = read (gp2x_dev, cpt, 256*4);
    //printf ("%d\n", temp);
    if (temp != 256*4)
    {
        printf ("  # Bad read\n");
        return;
    }

    //printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);

    for (i = 0; i < 256; i ++)
    {
        if (cpt[i])
        {
            switch (cpt[i] & 3)
            {
                case 0:
                    //printf ("  -- [%08X] Invalid --\n", cpt[i]);
                    break;
                case 1:
			/*
			  printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
			  cpt[i] & 0xFFFF0000, (cpt[i] & 0xFFFF0000) | 0xFFFF,
			  (cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3,
			  (cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1);
			*/
                    break;
                case 2:
			//a=((cpt[i] & 0xff000000)>>24);
			/*printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
			  cpt[i] & 0xfffff000, (cpt[i] & 0xfffff000) | 0xFFF,
			  (cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3,
			  (cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1);
			*/
			// This is where we look for any virtual addresses that map to physical address 0x03000000 and
			// alter the cachable and bufferable bits...
			if (((cpt[i] & 0xff000000) == 0x02000000) )
			{
				//printf("SOUND c and b bits not set, overwriting\n");
				if((cpt[i] & 12)==0) {
					cpt[i] |= 0xFFC;
					wb++;
				}
			}
			//if ((a>=0x31 && a<=0x36) && ((cpt[i] & 12)==0))
			if (((cpt[i] & 0xff000000) == 0x03000000) )
			{
				//printf("SDL   c and b bits not set, overwriting\n");
				if((cpt[i] & 12)==0) {
					cpt[i] |= 0xFFC;
					wb++;
				}
			}
			break;
                case 3:
                    //printf ("  -- [%08X/%d] Unsupported --\n", cpt[i],cpt[i] & 3);
                    break;
                default:
                    //printf ("  -- [%08X/%d] Unknown --\n", cpt[i], cpt[i] & 3);
                    break;
            }
        }
        indx += 4096;
    }
    //printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
    if (wb)
    {
        //printf("Hacking cpt\n");
        lseek (gp2x_dev, sa, SEEK_SET);
        temp = write (gp2x_dev, cpt, 256*4);
        printf("%d bytes written, %s\n", temp, temp == 1024 ? "yay!" : "oh fooble :(!");
    }
}

void dumppgtable (unsigned int ttb)
{
    unsigned int pgtable[4096];
    char *desctypes[] = {"Invalid", "Coarse", "Section", "Fine"};

    memset (pgtable, 0, 4096*4);
    lseek (gp2x_dev, ttb, SEEK_SET);
    read (gp2x_dev, pgtable, 4096*4);

    int i;
    for (i = 0; i < 4096; i ++)
    {
        int temp;

        if (pgtable[i])
        {
		//printf ("Indx: %d VA: %08X Type: %d [%s] \n", i, i * 1048576, pgtable[i] & 3, desctypes[pgtable[i]&3]);
            switch (pgtable[i]&3)
            {
                case 0:
                    //printf (" -- Invalid --\n");
                    break;
                case 1:
                    DecodeCoarse(i, pgtable[i]);
                    break;
                case 2:
                    temp = pgtable[i] & 0xFFF00000;
                    //printf ("  PA: %08X - %08X A: %d D: %d C: %d B: %d\n", temp, temp | 0xFFFFF,
                    //        (pgtable[i] >> 10) & 3, (pgtable[i] >> 5) & 15, (pgtable[i] >> 3) & 1,
                    //        (pgtable[i] >> 2) & 1);

                    break;
                case 3:
                    printf ("  -- Unsupported! --\n");
                    break;
            }
        }
    }
}

int hackpgtable (void)
{
    unsigned int oldc1, oldc2, oldc3, oldc4;
    unsigned int newc1 = 0xee120f10, newc2 = 0xe12fff1e;
    unsigned int ttb, ttx;
    int a=0;int try=0;
    // We need to execute a "MRC p15, 0, r0, c2, c0, 0", to get the pointer to the translation table base, but we can't
    // do this in user mode, so we have to patch the kernel to get it to run it for us in supervisor mode. We dothis
    // at the moment by overwriting the sys_newuname function and then calling it.

    lseek (gp2x_dev, 0x6ec00, SEEK_SET); // fixme: We should ask the kernel for this address rather than assuming it...
    read (gp2x_dev, &oldc1, 4);
    read (gp2x_dev, &oldc2, 4);
    read (gp2x_dev, &oldc3, 4);
    read (gp2x_dev, &oldc4, 4);

    printf ("0:%08X %08X - %08X %08X\n", oldc1, oldc2, newc1, newc2);



    printf ("point1 %d\n",a);
    do {
	    lseek (gp2x_dev, 0x6ec00, SEEK_SET);
	    a+=write (gp2x_dev, &newc1, 4);
	    a+=write (gp2x_dev, &newc2, 4);
	    SDL_Delay(200);
	    try++;
	    ttb = myuname(name);
	    printf ("2:%08X try %d\n", ttb,try);
    } while (ttb==0 && try<4);



    lseek (gp2x_dev, 0x6ec00, SEEK_SET);
    a+=write (gp2x_dev, &oldc1, 4);
    a+=write (gp2x_dev, &oldc2, 4);

    printf ("2:%08X %d\n", ttb,a);
    if (ttb!=0) {


	    //printf ("Restored contents\n");

	    // We now have the translation table base ! Walk the table looking for our allocation and hack it :)
	    dumppgtable(ttb);

	    // Now drain the write buffer and flush the tlb caches. Something else we can't do in user mode...
	    unsigned int tlbc1 = 0xe3a00000; // mov    r0, #0
	    unsigned int tlbc2 = 0xee070f9a; // mcr    15, 0, r0, cr7, cr10, 4
	    unsigned int tlbc3 = 0xee080f17; // mcr    15, 0, r0, cr8, cr7, 0
	    unsigned int tlbc4 = 0xe1a0f00e; // mov    pc, lr

	    lseek (gp2x_dev, 0x6ec00, SEEK_SET);
	    write (gp2x_dev, &tlbc1, 4);
	    write (gp2x_dev, &tlbc2, 4);
	    write (gp2x_dev, &tlbc3, 4);
	    write (gp2x_dev, &tlbc4, 4);

	    SDL_Delay(200);

	    ttx = myuname(name);

	    printf ("Return from uname: %08X\n", ttx);

	    lseek (gp2x_dev, 0x6ec00, SEEK_SET);
	    write (gp2x_dev, &oldc1, 4);
	    write (gp2x_dev, &oldc2, 4);
	    write (gp2x_dev, &oldc3, 4);
	    write (gp2x_dev, &oldc4, 4);
	    lseek (gp2x_dev, 0x0, SEEK_SET);
	    return 0;
    }

    lseek (gp2x_dev, 0x0, SEEK_SET);
    return 1;
    //printf ("Restored contents\n");

    //printf ("Pagetable after modification!\n");
    //printf ("-------------------------------\n");
    //dumppgtable(ttb);
}

/*
****** [END] Squidge's MMU hack code ******
*/

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

void MMUpatch (void)
{
	volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

	// Squidge's MMU hack
	hackpgtable();
}
