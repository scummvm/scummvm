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
 * GP2X: Hardware Stuff.
 * Thanks to Rlyeh, Snaff, Squidge, Hermes, PS2Reality and RobBrown
 * for there help with us all getting to grips with this.
 *
 */

#include "gp2x-common.h"

#include "gp2x-hw.h"
#include "gp2x-mem.h"

// Linux includes to let us goof about with the system in a 'standard' way.
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/time.h>
#include <unistd.h>

namespace GP2X_HW {

enum {
	VOLUME_NOCHG			= 0,
	VOLUME_DOWN				= 1,
	VOLUME_UP				= 2,
	VOLUME_CHANGE_RATE		= 8,
	VOLUME_MIN				= 0,
	VOLUME_INITIAL			= 70,
	VOLUME_MAX				= 100
};

int volumeLevel = VOLUME_INITIAL;

/* system registers */
static struct
{
	unsigned short SYSCLKENREG,SYSCSETREG,FPLLVSETREG,DUALINT920,DUALINT940,DUALCTRL940;
}
system_reg;

static unsigned short dispclockdiv;

static volatile unsigned short *MEM_REG;

#define SYS_CLK_FREQ 7372800

void deviceInit() {
	// Open devices
	if (!gp2x_dev[0])  gp2x_dev[0] = open("/dev/mixer", O_RDWR);
	if (!gp2x_dev[1])  gp2x_dev[1] = open("/dev/batt",   O_RDONLY);
	if (!gp2x_dev[2])  gp2x_dev[2] = open("/dev/mem",   O_RDWR);
}

void deviceDeinit() {
	// Close devices
	{
		int i;
		for (i=0;i<8;i++)
		{
			if (gp2x_dev[i])
			{
				close(gp2x_dev[i]);
			}
		}
	}

	MEM_REG[0x91c>>1] = system_reg.SYSCSETREG;
	MEM_REG[0x910>>1] = system_reg.FPLLVSETREG;
	MEM_REG[0x3B40>>1] = system_reg.DUALINT920;
	MEM_REG[0x3B42>>1] = system_reg.DUALINT940;
	MEM_REG[0x3B48>>1] = system_reg.DUALCTRL940;
	MEM_REG[0x904>>1] = system_reg.SYSCLKENREG;
	MEM_REG[0x924>>1] = dispclockdiv;

	unpatchMMU();
}

void mixerMoveVolume(int direction) {
    if (volumeLevel <= 10) {
        if (direction == VOLUME_UP)   volumeLevel += VOLUME_CHANGE_RATE/2;
        if (direction == VOLUME_DOWN) volumeLevel -= VOLUME_CHANGE_RATE/2;
    } else {
        if(direction == VOLUME_UP)   volumeLevel += VOLUME_CHANGE_RATE;
        if(direction == VOLUME_DOWN) volumeLevel -= VOLUME_CHANGE_RATE;
    }

    if (volumeLevel < VOLUME_MIN) volumeLevel = VOLUME_MIN;
    if (volumeLevel > VOLUME_MAX) volumeLevel = VOLUME_MAX;

    unsigned long soundDev = open("/dev/mixer", O_RDWR);

    if(soundDev) {
        int vol = ((volumeLevel << 8) | volumeLevel);
        ioctl(soundDev, SOUND_MIXER_WRITE_PCM, &vol);
        close(soundDev);
    }
}

void setCpuspeed(unsigned int mhz)
{
	set_FCLK(mhz);
	set_DCLK_Div(0);
	set_920_Div(0);
}

int getBattLevel() {
    int devbatt;
    unsigned short currentval=0;
    devbatt = open("/dev/batt", O_RDONLY);
    read (devbatt, &currentval, 2);
    close (devbatt);
    return (currentval);
}

void set_display_clock_div(unsigned div)
{
	div=((div & 63) | 64)<<8;
	MEM_REG[0x924>>1]=(MEM_REG[0x924>>1] & ~(255<<8)) | div;
}


void set_FCLK(unsigned MHZ)
{
	unsigned v;
	unsigned mdiv,pdiv=3,scale=0;
	MHZ*=1000000;
	mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
	mdiv=((mdiv-8)<<8) & 0xff00;
	pdiv=((pdiv-2)<<2) & 0xfc;
	scale&=3;
	v=mdiv | pdiv | scale;
	MEM_REG[0x910>>1]=v;
}


void set_920_Div(unsigned short div)
{
	unsigned short v;
	v = MEM_REG[0x91c>>1] & (~0x3);
	MEM_REG[0x91c>>1] = (div & 0x7) | v;
}


void set_DCLK_Div( unsigned short div )
{
	unsigned short v;
	v = (unsigned short)( MEM_REG[0x91c>>1] & (~(0x7 << 6)) );
	MEM_REG[0x91c>>1] = ((div & 0x7) << 6) | v;
}


void Disable_940(void)
{
	MEM_REG[0x3B42>>1];
	MEM_REG[0x3B42>>1]=0;
	MEM_REG[0x3B46>>1]=0xffff;
	MEM_REG[0x3B48>>1]|= (1 << 7);
	MEM_REG[0x904>>1]&=0xfffe;
}

void gp2x_video_wait_vsync(void)
{
	MEM_REG[0x2846>>1]=(MEM_REG[0x2846>>1] | 0x20) & ~2;
	while (!(MEM_REG[0x2846>>1] & 2));
}

} /* namespace GP2X_HW */

