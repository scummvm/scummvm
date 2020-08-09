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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _DSMAIN_H
#define _DSMAIN_H

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <nds.h>
#include "osystem_ds.h"

namespace DS {

// Video
void 	displayMode8Bit();											// Switch to 8-bit mode5
void 	displayMode16Bit();										// Switch to 16-bit mode5

// Get address of current back buffer
u16 *	get16BitBackBuffer();

void 	setTalkPos(int x, int y);
void 	setTopScreenTarget(int x, int y);
void	set200PercentFixedScale(bool on);
void	setTopScreenZoom(int percentage);
void	setGamma(int gamma);

// Timers
void 	setTimerCallback(OSystem_DS::TimerProc proc, int interval);		// Setup a callback function at a regular interval
int 	getMillis(bool skipRecord = false);								// Return the current runtime in milliseconds
void 	doTimerCallback();												// Call callback function if required

// Events
void 	VBlankHandler();
Common::Point transformPoint(uint16 x, uint16 y, bool isOverlayShown);

// Sam and Max Stuff
void 	setCursorIcon(const u8 *icon, uint w, uint h, byte keycolor, int hotspotX, int hotspotY);
void	setShowCursor(bool enable);
void	setMouseCursorVisible(bool visible);
void	warpMouse(int penX, int penY, bool isOverlayShown);
void	updateMouse();

// Shake
void 	setShakePos(int shakeXOffset, int shakeYOffset);

// Options
void	setGameScreenSwap(bool enable);
void 	setUnscaledMode(bool enable);
bool    isCpuScalerEnabled();
void	setCpuScalerEnable(bool enable);

// Display
bool 	getIsDisplayMode8Bit();
void 	setGameSize(int width, int height);
int		getGameWidth();
int		getGameHeight();

// Fast RAM allocation (ITCM)
void	fastRamReset();
void*	fastRamAlloc(int size);


} // End of namespace DS



int cygprofile_getHBlanks();

#endif
