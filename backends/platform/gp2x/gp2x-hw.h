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
 *
 */

#ifndef GP2X_HW_H
#define GP2X_HW_H

namespace GP2X_HW {

#define GP2X_MAXVOL 100 // Highest level permitted by GP2X's mixer
#define SYS_CLK_FREQ 7372800 // Clock Frequency

extern int volumeLevel;

extern void	deviceInit();
extern void	deviceDeinit();
extern void	mixerMoveVolume(int);
extern void setCpuspeed(unsigned int cpuspeed);
extern int getBattLevel();

extern void save_system_regs(void); /* save some registers */
extern void set_display_clock_div(unsigned div);
extern void set_FCLK(unsigned MHZ); /* adjust the clock frequency (in Mhz units) */
extern void set_920_Div(unsigned short div); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void set_DCLK_Div(unsigned short div); /* 0 to 7 divider (freq=FCLK/(1+div)) */
extern void Disable_940(void); /* 940t down */
extern void gp2x_video_wait_vsync(void);

} /* namespace GP2X_HW */

#endif //GP2X_HW_H
