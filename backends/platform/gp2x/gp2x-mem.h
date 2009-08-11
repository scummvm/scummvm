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
 * GP2X: Memory Stuff.
 *
 */

#ifndef GP2X_MEM_H
#define GP2X_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

// Use Squidge's MMU patch rather then myown (his is neater).
// The effect if not that great but cacheing the upper RAM is no bad thing (tm) ;).

//extern void InitRam (void);
//extern void CloseRam (void);
// Set ARM920t clock frequency
extern void SetClock (unsigned c);
extern void patchMMU (void);
extern void unpatchMMU (void);

#define SYS_CLK_FREQ 7372800

static          unsigned long   gp2x_dev[8]={0,0,0,0,0,0,0,0};//, gp2x_ticks_per_second;
static volatile unsigned short *gp2x_ram, *gp2x_memregs;

#ifdef __cplusplus
    }
#endif

#endif //GP2X_MEM_H
