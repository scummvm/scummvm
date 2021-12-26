/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

	//  Basic rectangle drawing function
	virtual void fillRect(Rect16 r, uint8 color);
	//  Complement mode drawing function
	virtual void invertRect(Rect16 r, uint8 color);

	//  REM: General rectangular display blitting function
	//  REM: Special-case blitting functions

	//  General-purpose blitting functions to transfer data
	//  from svga to offscreen buffer and vice versa
	//  (dispRect is the rectangle on the SVGA page, pixPtr
	//  is the buffer to transfer the pixels to/from, and
	//  pixMod is the line modulus of the buffer.)

	virtual void readPixels(Rect16 &dispRect, uint8 *pixPtr, uint16 pixMod);
	virtual void writePixels(Rect16 dispRect, uint8 *pixPtr, uint16 pixMod);
	virtual void writeTransPixels(Rect16 dispRect, uint8 *pixPtr, uint16 pixMod);
	virtual void writeColorPixels(Rect16 dispRect, uint8 *pixPtr, uint16 pixMod, uint8 color);
	virtual void writeComplementPixels(Rect16 dispRect, uint8 *pixPtr, uint16 pixMod, uint8 color);
};

} // end of namespace Saga2

#endif
