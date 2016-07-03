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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_TEXT_H
#define DM_TEXT_H

#include "dm.h"
#include "gfx.h"

namespace DM {

class TextMan {
	DMEngine *_vm;
public:
	explicit TextMan(DMEngine *vm);
	void f40_printTextToBitmap(byte *destBitmap, uint16 destByteWidth, uint16 destX, uint16 destY,
						   Color textColor, Color bgColor, const char *text, uint16 destHeight); // @ F0040_TEXT_Print
	void f53_printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char *text); // @ F0053_TEXT_PrintToLogicalScreen
	void f52_printToViewport(int16 posX, int16 posY, Color textColor, const char *text, Color bgColor = k12_ColorDarkestGray); // @ F0052_TEXT_PrintToViewport
	void f41_printWithTrailingSpaces(byte *destBitmap, int16 destByteWidth, int16 destX, int16 destY, Color textColor, Color bgColor,
								 const char *text, int16 strLenght, int16 destHeight); // @ F0041_TEXT_PrintWithTrailingSpaces
};

}

#endif
