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

#include "dm/dm.h"
#include "dm/gfx.h"

namespace DM {

struct TextScroller {
	// Placeholder, empty for now
}; // @ Text_Scroller

#define kDMFontLetterWidth 5
#define kDMFontLetterHeight 6

class TextMan {
	DMEngine *_vm;
	int16 _messageAreaCursorColumn; // @ G0359_i_MessageAreaCursorColumn
	int16 _messageAreaCursorRow; // @ G0358_i_MessageAreaCursorRow
	int32 _messageAreaRowExpirationTime[4]; // @ G0360_al_MessageAreaRowExpirationTime
	byte *_bitmapMessageAreaNewRow; // @ G0356_puc_Bitmap_MessageAreaNewRow

	// for scrolling 'em messages
	bool _isScrolling;
	int64 _startedScrollingAt;
	byte *_messageAreaCopy;
public:
	TextScroller _textScroller;

	explicit TextMan(DMEngine *vm);
	~TextMan();

	void printTextToBitmap(byte *destBitmap, uint16 destByteWidth, int16 destX, int16 destY,
							   Color textColor, Color bgColor, const char *text, uint16 destHeight); // @ F0040_TEXT_Print
	void printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char *text); // @ F0053_TEXT_PrintToLogicalScreen
	void printToViewport(int16 posX, int16 posY, Color textColor, const char *text, Color bgColor = kDMColorDarkestGray); // @ F0052_TEXT_PrintToViewport
	void printWithTrailingSpaces(byte *destBitmap, int16 destByteWidth, int16 destX, int16 destY, Color textColor, Color bgColor,
									 const char *text, int16 strLenght, int16 destHeight); // @ F0041_TEXT_PrintWithTrailingSpaces
	void printLineFeed(); // @ F0051_TEXT_MESSAGEAREA_PrintLineFeed
	void printMessage(Color color, const char *string, bool printWithScroll = true); // @ F0047_TEXT_MESSAGEAREA_PrintMessage
	void createNewRow(); // @ F0045_TEXT_MESSAGEAREA_CreateNewRow
	void printString(Color color, const char* string);// @ F0046_TEXT_MESSAGEAREA_PrintString
	void initialize(); // @ F0054_TEXT_Initialize
	void moveCursor(int16 column, int16 row); // @ F0042_TEXT_MESSAGEAREA_MoveCursor
	void clearExpiredRows(); // @ F0044_TEXT_MESSAGEAREA_ClearExpiredRows
	void printEndGameString(int16 x, int16 y, Color textColor, const char *text); // @ F0443_STARTEND_EndgamePrintString
	bool isTextScrolling(TextScroller *scroller, bool waitEndOfScrolling) { return false; } // @ F0561_SCROLLER_IsTextScrolling
	void setScrollerCommand(TextScroller *scroller, int16 command) { } // @ F0560_SCROLLER_SetCommand
	void clearAllRows(); // @ F0043_TEXT_MESSAGEAREA_ClearAllRows

	void updateMessageArea();
};

}
#endif
