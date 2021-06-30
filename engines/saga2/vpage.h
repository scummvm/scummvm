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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_VPAGE_H
#define SAGA2_VPAGE_H

namespace Saga2 {

class vDisplayPage {
public:
	virtual ~vDisplayPage() {}

	Point16         size;               // size in pixels

#if     ! defined( USEWINDOWS )
	//  inline function to set the bank if needed
	virtual void setWriteBank(uint16) {}
	virtual void setReadBank(uint16) {}
#endif

	//  Function to set and get pixel
	virtual void setPixel(int x, int y, uint8 color);
	virtual uint8 getPixel(int x, int y);

	//  Basic rectangle drawing function
	virtual void fillRect(Rect16 r, uint8 color);
	//  Complement mode drawing function
	virtual void invertRect(Rect16 r, uint8 color);

	virtual void hLine(int16 x, int16 y, int16 width, uint8 color);
	virtual void vLine(int16 x, int16 y, int16 height, uint8 color);

	//  REM: General rectangular display blitting function
	//  REM: Special-case blitting functions

	//  General-purpose blitting functions to transfer data
	//  from svga to offscreen buffer and vice versa
	//  (dispRect is the rectangle on the SVGA page, pixPtr
	//  is the buffer to transfer the pixels to/from, and
	//  pixMod is the line modulus of the buffer.)

	virtual void readPixels(Rect16 &dispRect, uint8 *pixPtr, uint16 pixMod);
	virtual void writePixels(Rect16 &dispRect, uint8 *pixPtr, uint16 pixMod);
	virtual void writeTransPixels(Rect16 &dispRect, uint8 *pixPtr, uint16 pixMod);
	virtual void writeColorPixels(Rect16 dispRect, uint8 *pixPtr, uint16 pixMod, uint8 color);
	virtual void writeComplementPixels(Rect16 dispRect, uint8 *pixPtr, uint16 pixMod, uint8 color);
};

extern Extent16     gDisplaySize;

#if     defined( USEWINDOWS )
// For access to WinPause/ResumeTransfer()'s
#include "WDisplay.h"
#endif

} // end of namespace Saga2

#endif
