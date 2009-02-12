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

/* system registers */
static struct
{
	unsigned short SYSCLKENREG,SYSCSETREG,FPLLVSETREG,DUALINT920,DUALINT940,DUALCTRL940;
}
system_reg;

static unsigned short dispclockdiv;

static volatile unsigned short *MEM_REG;

#define SYS_CLK_FREQ 7372800

void GP2X_device_init() {
	// Open devices
	if (!gp2x_dev[0])  gp2x_dev[0] = open("/dev/mixer", O_RDWR);
	if (!gp2x_dev[1])  gp2x_dev[1] = open("/dev/batt",   O_RDONLY);
	if (!gp2x_dev[2])  gp2x_dev[2] = open("/dev/mem",   O_RDWR);
}

void GP2X_device_deinit() {
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

// Vairous mixer level fudges.
// TODO: Clean up and merge quick hacks.

void GP2X_mixer_set_volume(int L /*0..100*/, int R /*0..100*/) {

	/* Set an arbitrary percentage value for the hardware mixer volume.

	   Parameters:
	   L (0..100) - volume percentage for the left channel
	   R (0..100) - volume percentage for the right channel

	   Note:
	   - A higher percentage than 100 will distort your sound.
	*/

	unsigned char temp[4];

	if (L < 0) L = 0;
	if (L > GP2X_MAXVOL) L = GP2X_MAXVOL;
	if (R < 0) R = 0;
	if (R > GP2X_MAXVOL) R = GP2X_MAXVOL;

	temp[0]=(unsigned char)L;
	temp[1]=(unsigned char)R;
	temp[2]=temp[3]=0;

	//warning("GP2X_mixer_set_volume is about to set %d %d", L, R);
	ioctl(gp2x_dev[0], SOUND_MIXER_WRITE_PCM, temp);
}

int GP2X_mixer_get_volume() {
	int vol;
	ioctl(gp2x_dev[0], SOUND_MIXER_READ_PCM, &vol);
	//warning("GP2X_mixer_get_volume returned %d %d", (int)((vol & 0xff)), (int)((vol >> 8) & 0xff));
	return (int)((vol & 0xff));
}

void GP2X_mixer_move_volume(int UpDown) {
	// Raise volume 5% if 1 passed, lower 5% if 0.
	int curvol, newvol;
	ioctl(gp2x_dev[0], SOUND_MIXER_READ_PCM, &curvol);
	curvol = ((int)((curvol & 0xff)));
	newvol = ((int)((curvol & 0xff)));
	//warning("GP2X_mixer_move_volume got current volume @ %d", curvol);
	if (UpDown == 1) {
			newvol = (curvol + 5);
	} else if (UpDown == 0) {
			newvol = (curvol - 5);
	}
	//warning("GP2X_mixer_move_volume is about to set volume @ %d", newvol);
	GP2X_mixer_set_volume(newvol, newvol);
	return;
}

void GP2X_setCpuspeed(unsigned int mhz)
{
	set_FCLK(mhz);
	set_DCLK_Div(0);
	set_920_Div(0);
}

int GP2X_getBattLevel() {
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
