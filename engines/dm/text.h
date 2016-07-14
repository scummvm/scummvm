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
	int16 _g359_messageAreaCursorColumn; // @ G0359_i_MessageAreaCursorColumn
	int16 _g358_messageAreaCursorRow; // @ G0358_i_MessageAreaCursorRow
	int32 _g360_messageAreaRowExpirationTime[4]; // @ G0360_al_MessageAreaRowExpirationTime
	byte *_g356_bitmapMessageAreaNewRow; // @ G0356_puc_Bitmap_MessageAreaNewRow
public:
	explicit TextMan(DMEngine *vm);
	~TextMan();
	void f40_printTextToBitmap(byte *destBitmap, uint16 destByteWidth, int16 destX, int16 destY,
						   Color textColor, Color bgColor, const char *text, uint16 destHeight); // @ F0040_TEXT_Print
	void f53_printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char *text); // @ F0053_TEXT_PrintToLogicalScreen
	void f52_printToViewport(int16 posX, int16 posY, Color textColor, const char *text, Color bgColor = k12_ColorDarkestGray); // @ F0052_TEXT_PrintToViewport
	void f41_printWithTrailingSpaces(byte *destBitmap, int16 destByteWidth, int16 destX, int16 destY, Color textColor, Color bgColor,
								 const char *text, int16 strLenght, int16 destHeight); // @ F0041_TEXT_PrintWithTrailingSpaces
	void f51_messageAreaPrintLineFeed(); // @ F0051_TEXT_MESSAGEAREA_PrintLineFeed
	void f47_messageAreaPrintMessage(Color color, const char *string); // @ F0047_TEXT_MESSAGEAREA_PrintMessage
	void f45_messageAreaCreateNewRow(); // @ F0045_TEXT_MESSAGEAREA_CreateNewRow
	void f46_messageAreaPrintString(Color color, const char* string);// @ F0046_TEXT_MESSAGEAREA_PrintString
	void f54_textInitialize(); // @ F0054_TEXT_Initialize
	void f42_messageAreaMoveCursor(int16 column, int16 row); // @ F0042_TEXT_MESSAGEAREA_MoveCursor
	void f44_messageAreaClearExpiredRows(); // @ F0044_TEXT_MESSAGEAREA_ClearExpiredRows


};

}

#endif
