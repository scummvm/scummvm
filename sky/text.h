/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKYTEXT_H
#define SKYTEXT_H

#include "sky/disk.h"

class SkyText {
public:
	SkyText(SkyDisk *skyDisk, uint32 gameVersion);
	void getText(uint32 textNr, void **itemList, uint16 language);

protected:
	bool getTBit();
	void fnSetFont(uint32 fontNr);
	virtual char getTextChar() = 0;

	SkyDisk *_skyDisk;
	uint32	_gameVersion;
	uint8	_inputValue;
	uint8	_shiftBits;
	uint8	*_textItemPtr;

	struct charSet {
		uint8 *addr;
		uint32 charHeight;
		uint32 thirdVal;
	} _mainCharacterSet, _linkCharacterSet, _controlCharacterSet;	
	
	uint32	_curCharSet;
	uint32	_characterSet;
	uint32	_charHeight;
	uint8	*_preAfterTableArea;

	uint8 _textBuffer[1024];

	uint8	*_mouseTextData;	//space for the mouse text
	uint32	_dtLineWidth;	//width of line in pixels
	uint32	_dtLines;	//no of lines to do
	uint32	_dtLineSize;	//size of one line in bytes
	uint8	*_dtData;	//address of textdata
	uint32	_dtLetters;	//no of chars in message
	uint8	*_dtText;	//pointer to text
	uint32	_dtCharSpacing;	//character seperation adjustment
	uint32	_dtWidth;	//width of chars in last line (for editing (?))
	uint32	_dtCentre;	//set for centre text
};

class SkyText_v00267 : public SkyText {
public:
	SkyText_v00267(SkyDisk *skyDisk, uint32 gameVersion) : SkyText(skyDisk, gameVersion) {};
protected:
	char getTextChar();
};

class SkyText_v00288 : public SkyText {
public:
	SkyText_v00288(SkyDisk *skyDisk, uint32 gameVersion) : SkyText(skyDisk, gameVersion) {};
protected:
	char getTextChar();
};

class SkyText_v00372 : public SkyText {
public:
	SkyText_v00372(SkyDisk *skyDisk, uint32 gameVersion) : SkyText(skyDisk, gameVersion) {};
protected:
	char getTextChar();
};

#endif
