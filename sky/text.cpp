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

#include "stdafx.h"
#include "common/scummsys.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "sky/text.h"
#include "sky/logic.h"

#define FIRST_TEXT_SEC	77
#define	FIRST_TEXT_BUFFER	274
#define NO_OF_TEXT_SECTIONS	8	// 8 sections per language
#define	CHAR_SET_FILE	60150
#define MAX_SPEECH_SECTION	7 
#define CHAR_SET_HEADER	128
#define	MAX_NO_LINES	10

SkyText::SkyText(SkyDisk *skyDisk) {
	_skyDisk = skyDisk;

	initHuffTree();

	_mainCharacterSet.addr = _skyDisk->loadFile(CHAR_SET_FILE, NULL);
	_mainCharacterSet.charHeight = MAIN_CHAR_HEIGHT;
	_mainCharacterSet.charSpacing = 0;
	
	fnSetFont(0);

	if (!SkyState::isDemo()) {
		_controlCharacterSet.addr = _skyDisk->loadFile(60520, NULL);
		_controlCharacterSet.charHeight = 12;
		_controlCharacterSet.charSpacing = 1;
		
		_linkCharacterSet.addr = _skyDisk->loadFile(60521, NULL);
		_linkCharacterSet.charHeight = 12;
		_linkCharacterSet.charSpacing = 0;
	} else {
		_controlCharacterSet.addr = NULL;
		_linkCharacterSet.addr = NULL;
	}

	if (SkyState::isCDVersion()) {
		_preAfterTableArea = _skyDisk->loadFile(60522, NULL);
	} else _preAfterTableArea = NULL;
}

SkyText::~SkyText(void) {

	if (_controlCharacterSet.addr) free(_controlCharacterSet.addr);
	if (_linkCharacterSet.addr) free(_linkCharacterSet.addr);
	if (_preAfterTableArea) free(_preAfterTableArea);
}

void SkyText::fnSetFont(uint32 fontNr) { 

	struct charSet *newCharSet;

	switch (fontNr) {
		case 0:
			newCharSet = &_mainCharacterSet;
			break;
		case 1:
			newCharSet = &_controlCharacterSet;
			break;
		case 2:
			newCharSet = &_linkCharacterSet;
			break;
		default:
			error("Tried to set invalid font (%d)", fontNr);
	}

	_curCharSet = fontNr;
	_characterSet = newCharSet->addr;
	_charHeight = (byte)newCharSet->charHeight;
	_dtCharSpacing = newCharSet->charSpacing;
}

void SkyText::fnTextModule(uint32 textInfoId, uint32 textNo) {

	fnSetFont(1);
	uint16* msgData = (uint16*)SkyState::fetchCompact(textInfoId);
	lowTextManager_t textId = lowTextManager(textNo, msgData[1], msgData[2], 209, false);
	SkyLogic::_scriptVariables[RESULT] = textId.compactNum;
	Compact *textCompact = SkyState::fetchCompact(textId.compactNum);
	textCompact->xcood = msgData[3];
	textCompact->ycood = msgData[4];
	fnSetFont(0);
}

void SkyText::getText(uint32 textNr) { //load text #"textNr" into textBuffer
	uint32 sectionNo = (textNr & 0x0F000) >> 12;
	
	if (SkyState::_itemList[FIRST_TEXT_SEC + sectionNo] == (void **)NULL) { //check if already loaded
		debug(5, "Loading Text item(s) for Section %d", (sectionNo>>2));
		
		uint32 fileNo = sectionNo + ((SkyState::_systemVars.language * NO_OF_TEXT_SECTIONS) + 60600);
		SkyState::_itemList[FIRST_TEXT_SEC + sectionNo] = (void **)_skyDisk->loadFile((uint16)fileNo, NULL);
	}
	_textItemPtr = (uint8 *)SkyState::_itemList[FIRST_TEXT_SEC + sectionNo];
	
	uint32 offset = 0; 
	uint32 nr32MsgBlocks = (textNr & 0x0fe0);
	uint32 skipBytes; 
	byte *blockPtr;
	bool bitSeven; 
	
	if (nr32MsgBlocks) { 
		blockPtr = (byte *)(_textItemPtr + 4);
		nr32MsgBlocks >>= 5;
		do {
			offset += READ_LE_UINT16(blockPtr);
			blockPtr += 2;
		} while (--nr32MsgBlocks); 
	}
       
	uint32 remItems = textNr; 
	textNr &= 0x1f;
	if (textNr) {
	
		remItems &= 0x0fe0;
		remItems += READ_LE_UINT16(_textItemPtr);
		blockPtr = _textItemPtr + remItems; 
		
		do {
			skipBytes = *blockPtr++;
			bitSeven = (bool)((skipBytes >> (7)) & 0x1);
			skipBytes &= ~(1UL << 7);
	
			if (bitSeven) 
				skipBytes <<= 3;
		 
			offset += skipBytes;
	
		} while (--textNr);
	}

	uint32 numBits = offset;
	offset >>= 2;
	offset += READ_LE_UINT16(_textItemPtr + 2); 
	_textItemPtr += offset;

	//bit pointer: 0->8, 1->6, 2->4 ...
	numBits &= 3;
	numBits ^= 3; 
	numBits++;
	numBits <<= 1;

	_inputValue = *_textItemPtr++;
	char *dest = (char *)_textBuffer;
	char textChar;
	_shiftBits = (uint8) numBits; 

	do {
		textChar = getTextChar();
		*dest++ = textChar;	
	} while(textChar);
}

void SkyText::fnPointerText(uint32 pointedId, uint16 mouseX, uint16 mouseY) {

	Compact *ptrComp = SkyState::fetchCompact(pointedId);
	lowTextManager_t text = lowTextManager(ptrComp->cursorText, TEXT_MOUSE_WIDTH, L_CURSOR, 242, false);
	SkyLogic::_scriptVariables[CURSOR_ID] = text.compactNum;
	if (SkyLogic::_scriptVariables[MENU]) {
		_mouseOfsY = TOP_LEFT_Y - 2;
		if (mouseX < 150) _mouseOfsX = TOP_LEFT_X + 24;
		else _mouseOfsX = TOP_LEFT_X  - 8 - _lowTextWidth;
	} else {
        _mouseOfsY = TOP_LEFT_Y - 10;
		if (mouseX < 150) _mouseOfsX = TOP_LEFT_X + 13;
		else _mouseOfsX = TOP_LEFT_X - 8 - _lowTextWidth;
	}
	Compact *textCompact = SkyState::fetchCompact(text.compactNum);
	logicCursor(textCompact, mouseX, mouseY);
}

void SkyText::logicCursor(Compact *textCompact, uint16 mouseX, uint16 mouseY) {

	textCompact->xcood = (uint16)(mouseX + _mouseOfsX);
	textCompact->ycood = (uint16)(mouseY + _mouseOfsY);
	if (textCompact->ycood < TOP_LEFT_Y) textCompact->ycood = TOP_LEFT_Y;
}

bool SkyText::getTBit() {
	
	if (_shiftBits) {
		(_shiftBits)--;
	} else {
		_inputValue = *_textItemPtr++;
		_shiftBits = 7;
	} 
	
	return (bool)(((_inputValue) >> (_shiftBits)) & 1);
}

displayText_t SkyText::displayText(uint8 *dest, bool centre, uint16 pixelWidth, uint8 color) {
	//Render text in _textBuffer in buffer *dest
	return displayText(this->_textBuffer, dest, centre, pixelWidth, color);
}

displayText_t SkyText::displayText(char *textPtr, uint8 *dest, bool centre, uint16 pixelWidth, uint8 color) {
	
	//Render text pointed to by *textPtr in buffer *dest

	char textChar;
	char *curPos = textPtr;
	char *lastSpace = curPos;
	byte *centerTblPtr = _centreTable;
	uint16 lineWidth = 0;  
	
	_dtCol = color;
	_dtLineWidth = pixelWidth;  
	_dtLines = 0;
	_dtLetters = 1;
	_dtData = dest;
	_dtText = textPtr;
	_dtCentre = centre;

	textChar = *curPos++;
	_dtLetters++;

	while (textChar >= 0x20) {

		textChar -= 0x20;
		if (textChar == 0) {
			lastSpace = curPos; //keep track of last space
			*(uint32*)centerTblPtr = TO_LE_32(lineWidth);
		}
		
		lineWidth += *(_characterSet+textChar);	//add character width
		lineWidth += (uint16)_dtCharSpacing;	//include character spacing
	
		if (pixelWidth <= lineWidth) {
	
			if (*(lastSpace-1) == 10)
				error("line width exceeded!");

			*(lastSpace-1) = 10;
			lineWidth = 0;
			_dtLines++;
			centerTblPtr += 4;	//get next space in centering table
			curPos = lastSpace;	//go back for new count
		}

		textChar = *curPos++;
		_dtLetters++;
	}

	_dtLastWidth = lineWidth;	//save width of last line
	*(uint32 *)centerTblPtr = TO_LE_32(lineWidth);	//and update centering table
	_dtLines++;

	if (_dtLines > MAX_NO_LINES)
		error("Maximum no. of lines exceeded!");

	_dtLineSize = pixelWidth * _charHeight;
	uint32 numBytes = (_dtLineSize * _dtLines) + sizeof(struct dataFileHeader) + 4;	

	if (_dtData == NULL)
		_dtData = (byte *)malloc(numBytes);

	uint8 *curDest = _dtData; 

	uint32 bytesToClear = numBytes; //no of bytes to clear
	bytesToClear -= sizeof(struct dataFileHeader);	//don't touch the header.
	memset(curDest + sizeof(struct dataFileHeader), 0, bytesToClear);
	curPos += bytesToClear;

	//make the header
	((struct dataFileHeader *)curDest)->s_width = TO_LE_16(_dtLineWidth);
	((struct dataFileHeader *)curDest)->s_height = TO_LE_16((uint16)(_charHeight * _dtLines));
	((struct dataFileHeader *)curDest)->s_sp_size = TO_LE_16((uint16)(_dtLineWidth * _charHeight * _dtLines));
	((struct dataFileHeader *)curDest)->s_offset_x = TO_LE_16(0);
	((struct dataFileHeader *)curDest)->s_offset_y = TO_LE_16(0);
	
	//reset position
	curPos = textPtr;

	curDest += sizeof(struct dataFileHeader);	//point to where pixels start
	byte *prevDest = curDest;
	centerTblPtr = _centreTable;

	do {
		if (_dtCentre) {
		
			uint32 width = _dtLineWidth;
			width -= READ_LE_UINT32(centerTblPtr); 
			centerTblPtr += 4;
			width >>=1;
			curDest += width;
		}

		textChar = *curPos++;
		while (textChar >= 0x20) {
			textChar -= 0x20;
			makeGameCharacter(textChar, _characterSet, curDest, color);
			textChar = *curPos++;
		}

		curDest = prevDest;	//start of last line
		curDest += _dtLineSize;	//start of next
		prevDest = curDest;

	} while (textChar >= 10);
	
	struct displayText_t ret;
	ret.textData = _dtData;
	ret.textWidth = _dtLastWidth;	
	return ret;
}

void SkyText::makeGameCharacter(char textChar, uint8 *charSetPtr, uint8 *&dest, uint8 color) {

	bool maskBit, dataBit;	
	uint8 charWidth = (uint8)((*(charSetPtr + textChar)) + 1 - _dtCharSpacing);
	uint16 data, mask; 
	byte *charSpritePtr = charSetPtr + (CHAR_SET_HEADER + ((_charHeight << 2) * textChar));
	byte *startPos = dest;
	byte *curPos = startPos;

	for (int i = 0; i < _charHeight; i++) {

		byte *prevPos = curPos;
	
		data = READ_BE_UINT16(charSpritePtr);
		mask = READ_BE_UINT16(charSpritePtr + 2);
		charSpritePtr += 4;
		
		for (int j = 0; j < charWidth; j++) {
	
			maskBit = (mask & 0x8000) != 0; //check mask
			mask <<= 1;
			dataBit = (data & 0x8000) != 0; //check data
			data <<= 1;

			if (maskBit) 
				if (dataBit) 
					*curPos = color;
				else
					//black edge
					*curPos = 240; 

			curPos++;
		}

		//advance a line
		curPos = prevPos;
		curPos += _dtLineWidth;
	}
	
	//update position
	dest = startPos + charWidth + _dtCharSpacing*2 - 1; 

}

lowTextManager_t SkyText::lowTextManager(uint32 textNum, uint16 width, uint16 logicNum, uint8 color, bool centre) {

	getText(textNum);

	struct displayText_t textInfo = displayText(NULL, centre, width, color);
	
	_lowTextWidth = textInfo.textWidth;
	byte *textData = textInfo.textData;

	uint32 compactNum = FIRST_TEXT_COMPACT;

	Compact *cpt = SkyState::fetchCompact(compactNum);

	while (cpt->status != 0) { 
		compactNum++;
		cpt = SkyState::fetchCompact(compactNum);
	}

	cpt->flag = (uint16)(compactNum - FIRST_TEXT_COMPACT) + FIRST_TEXT_BUFFER;

	byte *oldText = (byte *)SkyState::_itemList[cpt->flag];
	SkyState::_itemList[cpt->flag] = (void **)textData; 

	if (oldText != NULL)
		free (oldText);

	cpt->logic = logicNum; 
	cpt->status = ST_LOGIC | ST_FOREGROUND | ST_RECREATE;
	cpt->screen = (uint16) SkyLogic::_scriptVariables[SCREEN];

	struct lowTextManager_t ret;
	ret.textData = _dtData;
	ret.compactNum = (uint16)compactNum;

	return ret;
}

void SkyText::changeTextSpriteColour(uint8 *sprData, uint8 newCol) {

	dataFileHeader *header = (dataFileHeader*)sprData;
	sprData += sizeof(dataFileHeader);
	for (uint16 cnt = 0; cnt < header->s_sp_size; cnt++)
		if (sprData[cnt] >= 241) sprData[cnt] = newCol;
}

static const HuffTree huffTree_00267[] = {
	{ 1, 20, 0 },
	{ 2, 7, 0 },
	{ 3, 6, 0 },
	{ 4, 5, 0 },
	{ 0, 0, (char)0x20 },
	{ 0, 0, (char)0x65 },
	{ 0, 0, (char)0x73 },
	{ 8, 15, 0 },
	{ 9, 12, 0 },
	{ 10, 11, 0 },
	{ 0, 0, (char)0x6d },
	{ 0, 0, (char)0x61 },
	{ 13, 14, 0 },
	{ 0, 0, (char)0x0 },
	{ 0, 0, (char)0x6e },
	{ 16, 19, 0 },
	{ 17, 18, 0 },
	{ 0, 0, (char)0x69 },
	{ 0, 0, (char)0x74 },
	{ 0, 0, (char)0x75 },
	{ 21, 44, 0 },
	{ 22, 31, 0 },
	{ 23, 28, 0 },
	{ 24, 27, 0 },
	{ 25, 26, 0 },
	{ 0, 0, (char)0x2e },
	{ 0, 0, (char)0x72 },
	{ 0, 0, (char)0x45 },
	{ 29, 30, 0 },
	{ 0, 0, (char)0x6f },
	{ 0, 0, (char)0x68 },
	{ 32, 39, 0 },
	{ 33, 36, 0 },
	{ 34, 35, 0 },
	{ 0, 0, (char)0x44 },
	{ 0, 0, (char)0x67 },
	{ 37, 38, 0 },
	{ 0, 0, (char)0x49 },
	{ 0, 0, (char)0x79 },
	{ 40, 43, 0 },
	{ 41, 42, 0 },
	{ 0, 0, (char)0x53 },
	{ 0, 0, (char)0x64 },
	{ 0, 0, (char)0x6c },
	{ 45, 64, 0 },
	{ 46, 53, 0 },
	{ 47, 50, 0 },
	{ 48, 49, 0 },
	{ 0, 0, (char)0x54 },
	{ 0, 0, (char)0x4e },
	{ 51, 52, 0 },
	{ 0, 0, (char)0x63 },
	{ 0, 0, (char)0x52 },
	{ 54, 59, 0 },
	{ 55, 58, 0 },
	{ 56, 57, 0 },
	{ 0, 0, (char)0x41 },
	{ 0, 0, (char)0x4f },
	{ 0, 0, (char)0x4c },
	{ 60, 63, 0 },
	{ 61, 62, 0 },
	{ 0, 0, (char)0x48 },
	{ 0, 0, (char)0x43 },
	{ 0, 0, (char)0x55 },
	{ 65, 84, 0 },
	{ 66, 75, 0 },
	{ 67, 72, 0 },
	{ 68, 71, 0 },
	{ 69, 70, 0 },
	{ 0, 0, (char)0x62 },
	{ 0, 0, (char)0x21 },
	{ 0, 0, (char)0x27 },
	{ 73, 74, 0 },
	{ 0, 0, (char)0x77 },
	{ 0, 0, (char)0x66 },
	{ 76, 81, 0 },
	{ 77, 80, 0 },
	{ 78, 79, 0 },
	{ 0, 0, (char)0x47 },
	{ 0, 0, (char)0x4d },
	{ 0, 0, (char)0x2c },
	{ 82, 83, 0 },
	{ 0, 0, (char)0x57 },
	{ 0, 0, (char)0x42 },
	{ 85, 98, 0 },
	{ 86, 93, 0 },
	{ 87, 90, 0 },
	{ 88, 89, 0 },
	{ 0, 0, (char)0x3f },
	{ 0, 0, (char)0x50 },
	{ 91, 92, 0 },
	{ 0, 0, (char)0x6b },
	{ 0, 0, (char)0x70 },
	{ 94, 97, 0 },
	{ 95, 96, 0 },
	{ 0, 0, (char)0x76 },
	{ 0, 0, (char)0x46 },
	{ 0, 0, (char)0x4b },
	{ 99, 110, 0 },
	{ 100, 105, 0 },
	{ 101, 104, 0 },
	{ 102, 103, 0 },
	{ 0, 0, (char)0x56 },
	{ 0, 0, (char)0x59 },
	{ 0, 0, (char)0x7a },
	{ 106, 109, 0 },
	{ 107, 108, 0 },
	{ 0, 0, (char)0x2d },
	{ 0, 0, (char)0x23 },
	{ 0, 0, (char)0x4a },
	{ 111, 124, 0 },
	{ 112, 117, 0 },
	{ 113, 116, 0 },
	{ 114, 115, 0 },
	{ 0, 0, (char)0x5a },
	{ 0, 0, (char)0x28 },
	{ 0, 0, (char)0x29 },
	{ 118, 121, 0 },
	{ 119, 120, 0 },
	{ 0, 0, (char)0x6a },
	{ 0, 0, (char)0x2b },
	{ 122, 123, 0 },
	{ 0, 0, (char)0x2a },
	{ 0, 0, (char)0x71 },
	{ 125, 138, 0 },
	{ 126, 131, 0 },
	{ 127, 130, 0 },
	{ 128, 129, 0 },
	{ 0, 0, (char)0x7d },
	{ 0, 0, (char)0x3a },
	{ 0, 0, (char)0x60 },
	{ 132, 135, 0 },
	{ 133, 134, 0 },
	{ 0, 0, (char)0x30 },
	{ 0, 0, (char)0x32 },
	{ 136, 137, 0 },
	{ 0, 0, (char)0x31 },
	{ 0, 0, (char)0x51 },
	{ 139, 150, 0 },
	{ 140, 145, 0 },
	{ 141, 144, 0 },
	{ 142, 143, 0 },
	{ 0, 0, (char)0x58 },
	{ 0, 0, (char)0x78 },
	{ 0, 0, (char)0x33 },
	{ 146, 149, 0 },
	{ 147, 148, 0 },
	{ 0, 0, (char)0x39 },
	{ 0, 0, (char)0x34 },
	{ 0, 0, (char)0x3c },
	{ 151, 160, 0 },
	{ 152, 157, 0 },
	{ 153, 156, 0 },
	{ 154, 155, 0 },
	{ 0, 0, (char)0x38 },
	{ 0, 0, (char)0x5d },
	{ 0, 0, (char)0x3d },
	{ 158, 159, 0 },
	{ 0, 0, (char)0x26 },
	{ 0, 0, (char)0x35 },
	{ 161, 168, 0 },
	{ 162, 165, 0 },
	{ 163, 164, 0 },
	{ 0, 0, (char)0x36 },
	{ 0, 0, (char)0x2f },
	{ 166, 167, 0 },
	{ 0, 0, (char)0x37 },
	{ 0, 0, (char)0x24 },
	{ 169, 174, 0 },
	{ 170, 173, 0 },
	{ 171, 172, 0 },
	{ 0, 0, (char)0x3e },
	{ 0, 0, (char)0x25 },
	{ 0, 0, (char)0x5e },
	{ 175, 180, 0 },
	{ 176, 179, 0 },
	{ 177, 178, 0 },
	{ 0, 0, (char)0x22 },
	{ 0, 0, (char)0x40 },
	{ 0, 0, (char)0x5b },
	{ 181, 184, 0 },
	{ 182, 183, 0 },
	{ 0, 0, (char)0x5f },
	{ 0, 0, (char)0x7b },
	{ 185, 186, 0 },
	{ 0, 0, (char)0x5c },
	{ 0, 0, (char)0x7c },
};

static const HuffTree huffTree_00288[] = {
	{ 1, 20, 0 },
	{ 2, 7, 0 },
	{ 3, 4, 0 },
	{ 0, 0, (char)0x20 },
	{ 5, 6, 0 },
	{ 0, 0, (char)0x65 },
	{ 0, 0, (char)0x73 },
	{ 8, 15, 0 },
	{ 9, 12, 0 },
	{ 10, 11, 0 },
	{ 0, 0, (char)0x6e },
	{ 0, 0, (char)0x69 },
	{ 13, 14, 0 },
	{ 0, 0, (char)0x74 },
	{ 0, 0, (char)0x61 },
	{ 16, 19, 0 },
	{ 17, 18, 0 },
	{ 0, 0, (char)0x2e },
	{ 0, 0, (char)0x0 },
	{ 0, 0, (char)0x45 },
	{ 21, 44, 0 },
	{ 22, 31, 0 },
	{ 23, 28, 0 },
	{ 24, 27, 0 },
	{ 25, 26, 0 },
	{ 0, 0, (char)0x72 },
	{ 0, 0, (char)0x6f },
	{ 0, 0, (char)0x75 },
	{ 29, 30, 0 },
	{ 0, 0, (char)0x6d },
	{ 0, 0, (char)0x68 },
	{ 32, 39, 0 },
	{ 33, 36, 0 },
	{ 34, 35, 0 },
	{ 0, 0, (char)0x49 },
	{ 0, 0, (char)0x6c },
	{ 37, 38, 0 },
	{ 0, 0, (char)0x53 },
	{ 0, 0, (char)0x52 },
	{ 40, 43, 0 },
	{ 41, 42, 0 },
	{ 0, 0, (char)0x64 },
	{ 0, 0, (char)0x4e },
	{ 0, 0, (char)0x54 },
	{ 45, 68, 0 },
	{ 46, 55, 0 },
	{ 47, 52, 0 },
	{ 48, 51, 0 },
	{ 49, 50, 0 },
	{ 0, 0, (char)0x63 },
	{ 0, 0, (char)0x41 },
	{ 0, 0, (char)0x4f },
	{ 53, 54, 0 },
	{ 0, 0, (char)0x44 },
	{ 0, 0, (char)0x4c },
	{ 56, 63, 0 },
	{ 57, 60, 0 },
	{ 58, 59, 0 },
	{ 0, 0, (char)0x43 },
	{ 0, 0, (char)0x67 },
	{ 61, 62, 0 },
	{ 0, 0, (char)0x55 },
	{ 0, 0, (char)0x27 },
	{ 64, 67, 0 },
	{ 65, 66, 0 },
	{ 0, 0, (char)0x79 },
	{ 0, 0, (char)0x48 },
	{ 0, 0, (char)0x70 },
	{ 69, 88, 0 },
	{ 70, 79, 0 },
	{ 71, 76, 0 },
	{ 72, 75, 0 },
	{ 73, 74, 0 },
	{ 0, 0, (char)0x21 },
	{ 0, 0, (char)0x4d },
	{ 0, 0, (char)0x62 },
	{ 77, 78, 0 },
	{ 0, 0, (char)0x76 },
	{ 0, 0, (char)0x2c },
	{ 80, 85, 0 },
	{ 81, 84, 0 },
	{ 82, 83, 0 },
	{ 0, 0, (char)0x50 },
	{ 0, 0, (char)0x66 },
	{ 0, 0, (char)0x3f },
	{ 86, 87, 0 },
	{ 0, 0, (char)0x47 },
	{ 0, 0, (char)0x42 },
	{ 89, 104, 0 },
	{ 90, 97, 0 },
	{ 91, 94, 0 },
	{ 92, 93, 0 },
	{ 0, 0, (char)0x77 },
	{ 0, 0, (char)0x57 },
	{ 95, 96, 0 },
	{ 0, 0, (char)0x46 },
	{ 0, 0, (char)0x56 },
	{ 98, 101, 0 },
	{ 99, 100, 0 },
	{ 0, 0, (char)0x6b },
	{ 0, 0, (char)0x7a },
	{ 102, 103, 0 },
	{ 0, 0, (char)0x4b },
	{ 0, 0, (char)0x2d },
	{ 105, 116, 0 },
	{ 106, 111, 0 },
	{ 107, 110, 0 },
	{ 108, 109, 0 },
	{ 0, 0, (char)0x4a },
	{ 0, 0, (char)0x2b },
	{ 0, 0, (char)0x71 },
	{ 112, 115, 0 },
	{ 113, 114, 0 },
	{ 0, 0, (char)0x59 },
	{ 0, 0, (char)0x6a },
	{ 0, 0, (char)0x5a },
	{ 117, 132, 0 },
	{ 118, 125, 0 },
	{ 119, 122, 0 },
	{ 120, 121, 0 },
	{ 0, 0, (char)0x23 },
	{ 0, 0, (char)0x51 },
	{ 123, 124, 0 },
	{ 0, 0, (char)0x28 },
	{ 0, 0, (char)0x29 },
	{ 126, 129, 0 },
	{ 127, 128, 0 },
	{ 0, 0, (char)0x3c },
	{ 0, 0, (char)0x78 },
	{ 130, 131, 0 },
	{ 0, 0, (char)0x3a },
	{ 0, 0, (char)0x2a },
	{ 133, 146, 0 },
	{ 134, 141, 0 },
	{ 135, 138, 0 },
	{ 136, 137, 0 },
	{ 0, 0, (char)0x58 },
	{ 0, 0, (char)0x7d },
	{ 139, 140, 0 },
	{ 0, 0, (char)0x3d },
	{ 0, 0, (char)0x60 },
	{ 142, 145, 0 },
	{ 143, 144, 0 },
	{ 0, 0, (char)0x5d },
	{ 0, 0, (char)0x30 },
	{ 0, 0, (char)0x32 },
	{ 147, 158, 0 },
	{ 148, 153, 0 },
	{ 149, 152, 0 },
	{ 150, 151, 0 },
	{ 0, 0, (char)0x31 },
	{ 0, 0, (char)0x33 },
	{ 0, 0, (char)0x39 },
	{ 154, 157, 0 },
	{ 155, 156, 0 },
	{ 0, 0, (char)0x2f },
	{ 0, 0, (char)0x34 },
	{ 0, 0, (char)0x5e },
	{ 159, 168, 0 },
	{ 160, 165, 0 },
	{ 161, 164, 0 },
	{ 162, 163, 0 },
	{ 0, 0, (char)0x38 },
	{ 0, 0, (char)0x3e },
	{ 0, 0, (char)0x26 },
	{ 166, 167, 0 },
	{ 0, 0, (char)0x35 },
	{ 0, 0, (char)0x40 },
	{ 169, 176, 0 },
	{ 170, 173, 0 },
	{ 171, 172, 0 },
	{ 0, 0, (char)0x36 },
	{ 0, 0, (char)0x5f },
	{ 174, 175, 0 },
	{ 0, 0, (char)0x5c },
	{ 0, 0, (char)0x37 },
	{ 177, 182, 0 },
	{ 178, 181, 0 },
	{ 179, 180, 0 },
	{ 0, 0, (char)0x5b },
	{ 0, 0, (char)0x24 },
	{ 0, 0, (char)0x7b },
	{ 183, 186, 0 },
	{ 184, 185, 0 },
	{ 0, 0, (char)0x25 },
	{ 0, 0, (char)0x9 },
	{ 187, 188, 0 },
	{ 0, 0, (char)0x22 },
	{ 0, 0, (char)0x7c },
};

static const HuffTree huffTree_00303[] = {
	{ 1, 22, 0 },
	{ 2, 9, 0 },
	{ 3, 6, 0 },
	{ 4, 5, 0 },
	{ 0, 0, (char)0x20 },
	{ 0, 0, (char)0x65 },
	{ 7, 8, 0 },
	{ 0, 0, (char)0x74 },
	{ 0, 0, (char)0x73 },
	{ 10, 17, 0 },
	{ 11, 14, 0 },
	{ 12, 13, 0 },
	{ 0, 0, (char)0x6e },
	{ 0, 0, (char)0x61 },
	{ 15, 16, 0 },
	{ 0, 0, (char)0x69 },
	{ 0, 0, (char)0x2e },
	{ 18, 21, 0 },
	{ 19, 20, 0 },
	{ 0, 0, (char)0x6f },
	{ 0, 0, (char)0x0 },
	{ 0, 0, (char)0x45 },
	{ 23, 46, 0 },
	{ 24, 35, 0 },
	{ 25, 30, 0 },
	{ 26, 29, 0 },
	{ 27, 28, 0 },
	{ 0, 0, (char)0x72 },
	{ 0, 0, (char)0x75 },
	{ 0, 0, (char)0x68 },
	{ 31, 34, 0 },
	{ 32, 33, 0 },
	{ 0, 0, (char)0x49 },
	{ 0, 0, (char)0x6d },
	{ 0, 0, (char)0x6c },
	{ 36, 41, 0 },
	{ 37, 40, 0 },
	{ 38, 39, 0 },
	{ 0, 0, (char)0x54 },
	{ 0, 0, (char)0x53 },
	{ 0, 0, (char)0x52 },
	{ 42, 45, 0 },
	{ 43, 44, 0 },
	{ 0, 0, (char)0x64 },
	{ 0, 0, (char)0x4e },
	{ 0, 0, (char)0x41 },
	{ 47, 70, 0 },
	{ 48, 59, 0 },
	{ 49, 54, 0 },
	{ 50, 53, 0 },
	{ 51, 52, 0 },
	{ 0, 0, (char)0x63 },
	{ 0, 0, (char)0x4f },
	{ 0, 0, (char)0x44 },
	{ 55, 58, 0 },
	{ 56, 57, 0 },
	{ 0, 0, (char)0x4c },
	{ 0, 0, (char)0x43 },
	{ 0, 0, (char)0x79 },
	{ 60, 65, 0 },
	{ 61, 64, 0 },
	{ 62, 63, 0 },
	{ 0, 0, (char)0x67 },
	{ 0, 0, (char)0x27 },
	{ 0, 0, (char)0x55 },
	{ 66, 69, 0 },
	{ 67, 68, 0 },
	{ 0, 0, (char)0x48 },
	{ 0, 0, (char)0x21 },
	{ 0, 0, (char)0x70 },
	{ 71, 88, 0 },
	{ 72, 81, 0 },
	{ 73, 78, 0 },
	{ 74, 77, 0 },
	{ 75, 76, 0 },
	{ 0, 0, (char)0x4d },
	{ 0, 0, (char)0x62 },
	{ 0, 0, (char)0x3f },
	{ 79, 80, 0 },
	{ 0, 0, (char)0x76 },
	{ 0, 0, (char)0x66 },
	{ 82, 85, 0 },
	{ 83, 84, 0 },
	{ 0, 0, (char)0x50 },
	{ 0, 0, (char)0x2c },
	{ 86, 87, 0 },
	{ 0, 0, (char)0x77 },
	{ 0, 0, (char)0x47 },
	{ 89, 102, 0 },
	{ 90, 95, 0 },
	{ 91, 94, 0 },
	{ 92, 93, 0 },
	{ 0, 0, (char)0x42 },
	{ 0, 0, (char)0x57 },
	{ 0, 0, (char)0x6b },
	{ 96, 99, 0 },
	{ 97, 98, 0 },
	{ 0, 0, (char)0x46 },
	{ 0, 0, (char)0x56 },
	{ 100, 101, 0 },
	{ 0, 0, (char)0x59 },
	{ 0, 0, (char)0x4b },
	{ 103, 114, 0 },
	{ 104, 109, 0 },
	{ 105, 108, 0 },
	{ 106, 107, 0 },
	{ 0, 0, (char)0x2d },
	{ 0, 0, (char)0x7a },
	{ 0, 0, (char)0x4a },
	{ 110, 113, 0 },
	{ 111, 112, 0 },
	{ 0, 0, (char)0x71 },
	{ 0, 0, (char)0x2b },
	{ 0, 0, (char)0x6a },
	{ 115, 132, 0 },
	{ 116, 123, 0 },
	{ 117, 120, 0 },
	{ 118, 119, 0 },
	{ 0, 0, (char)0x5a },
	{ 0, 0, (char)0x23 },
	{ 121, 122, 0 },
	{ 0, 0, (char)0x51 },
	{ 0, 0, (char)0x78 },
	{ 124, 129, 0 },
	{ 125, 128, 0 },
	{ 126, 127, 0 },
	{ 0, 0, (char)0x3a },
	{ 0, 0, (char)0x29 },
	{ 0, 0, (char)0x28 },
	{ 130, 131, 0 },
	{ 0, 0, (char)0x3c },
	{ 0, 0, (char)0x58 },
	{ 133, 148, 0 },
	{ 134, 141, 0 },
	{ 135, 138, 0 },
	{ 136, 137, 0 },
	{ 0, 0, (char)0x2a },
	{ 0, 0, (char)0x60 },
	{ 139, 140, 0 },
	{ 0, 0, (char)0x7d },
	{ 0, 0, (char)0x3d },
	{ 142, 145, 0 },
	{ 143, 144, 0 },
	{ 0, 0, (char)0x32 },
	{ 0, 0, (char)0x30 },
	{ 146, 147, 0 },
	{ 0, 0, (char)0x5d },
	{ 0, 0, (char)0x31 },
	{ 149, 160, 0 },
	{ 150, 155, 0 },
	{ 151, 154, 0 },
	{ 152, 153, 0 },
	{ 0, 0, (char)0x7e },
	{ 0, 0, (char)0x33 },
	{ 0, 0, (char)0x7f },
	{ 156, 159, 0 },
	{ 157, 158, 0 },
	{ 0, 0, (char)0x39 },
	{ 0, 0, (char)0x34 },
	{ 0, 0, (char)0x2f },
	{ 161, 172, 0 },
	{ 162, 167, 0 },
	{ 163, 166, 0 },
	{ 164, 165, 0 },
	{ 0, 0, (char)0x38 },
	{ 0, 0, (char)0x5e },
	{ 0, 0, (char)0x26 },
	{ 168, 171, 0 },
	{ 169, 170, 0 },
	{ 0, 0, (char)0x35 },
	{ 0, 0, (char)0x36 },
	{ 0, 0, (char)0x3e },
	{ 173, 182, 0 },
	{ 174, 179, 0 },
	{ 175, 178, 0 },
	{ 176, 177, 0 },
	{ 0, 0, (char)0x40 },
	{ 0, 0, (char)0x37 },
	{ 0, 0, (char)0x5f },
	{ 180, 181, 0 },
	{ 0, 0, (char)0x5c },
	{ 0, 0, (char)0x24 },
	{ 183, 190, 0 },
	{ 184, 187, 0 },
	{ 185, 186, 0 },
	{ 0, 0, (char)0x5b },
	{ 0, 0, (char)0x80 },
	{ 188, 189, 0 },
	{ 0, 0, (char)0x81 },
	{ 0, 0, (char)0x22 },
	{ 191, 194, 0 },
	{ 192, 193, 0 },
	{ 0, 0, (char)0x25 },
	{ 0, 0, (char)0x82 },
	{ 195, 196, 0 },
	{ 0, 0, (char)0x7b },
	{ 197, 198, 0 },
	{ 0, 0, (char)0x9 },
	{ 0, 0, (char)0x7c },
};

static const HuffTree huffTree_00331[] = {
	{ 1, 20, 0 },
	{ 2, 7, 0 },
	{ 3, 4, 0 },
	{ 0, 0, (char)0x20 },
	{ 5, 6, 0 },
	{ 0, 0, (char)0x65 },
	{ 0, 0, (char)0x61 },
	{ 8, 15, 0 },
	{ 9, 12, 0 },
	{ 10, 11, 0 },
	{ 0, 0, (char)0x6f },
	{ 0, 0, (char)0x73 },
	{ 13, 14, 0 },
	{ 0, 0, (char)0x74 },
	{ 0, 0, (char)0x6e },
	{ 16, 19, 0 },
	{ 17, 18, 0 },
	{ 0, 0, (char)0x2e },
	{ 0, 0, (char)0x69 },
	{ 0, 0, (char)0x72 },
	{ 21, 44, 0 },
	{ 22, 31, 0 },
	{ 23, 28, 0 },
	{ 24, 27, 0 },
	{ 25, 26, 0 },
	{ 0, 0, (char)0x0 },
	{ 0, 0, (char)0x45 },
	{ 0, 0, (char)0x75 },
	{ 29, 30, 0 },
	{ 0, 0, (char)0x6d },
	{ 0, 0, (char)0x41 },
	{ 32, 39, 0 },
	{ 33, 36, 0 },
	{ 34, 35, 0 },
	{ 0, 0, (char)0x6c },
	{ 0, 0, (char)0x49 },
	{ 37, 38, 0 },
	{ 0, 0, (char)0x64 },
	{ 0, 0, (char)0x52 },
	{ 40, 43, 0 },
	{ 41, 42, 0 },
	{ 0, 0, (char)0x4e },
	{ 0, 0, (char)0x53 },
	{ 0, 0, (char)0x54 },
	{ 45, 68, 0 },
	{ 46, 55, 0 },
	{ 47, 52, 0 },
	{ 48, 51, 0 },
	{ 49, 50, 0 },
	{ 0, 0, (char)0x4f },
	{ 0, 0, (char)0x68 },
	{ 0, 0, (char)0x63 },
	{ 53, 54, 0 },
	{ 0, 0, (char)0x44 },
	{ 0, 0, (char)0x67 },
	{ 56, 63, 0 },
	{ 57, 60, 0 },
	{ 58, 59, 0 },
	{ 0, 0, (char)0x4c },
	{ 0, 0, (char)0x43 },
	{ 61, 62, 0 },
	{ 0, 0, (char)0x70 },
	{ 0, 0, (char)0x55 },
	{ 64, 67, 0 },
	{ 65, 66, 0 },
	{ 0, 0, (char)0x21 },
	{ 0, 0, (char)0x79 },
	{ 0, 0, (char)0x4d },
	{ 69, 88, 0 },
	{ 70, 79, 0 },
	{ 71, 76, 0 },
	{ 72, 75, 0 },
	{ 73, 74, 0 },
	{ 0, 0, (char)0x50 },
	{ 0, 0, (char)0x76 },
	{ 0, 0, (char)0x48 },
	{ 77, 78, 0 },
	{ 0, 0, (char)0x3f },
	{ 0, 0, (char)0x62 },
	{ 80, 85, 0 },
	{ 81, 84, 0 },
	{ 82, 83, 0 },
	{ 0, 0, (char)0x27 },
	{ 0, 0, (char)0x66 },
	{ 0, 0, (char)0x2c },
	{ 86, 87, 0 },
	{ 0, 0, (char)0x47 },
	{ 0, 0, (char)0x42 },
	{ 89, 108, 0 },
	{ 90, 99, 0 },
	{ 91, 96, 0 },
	{ 92, 95, 0 },
	{ 93, 94, 0 },
	{ 0, 0, (char)0x56 },
	{ 0, 0, (char)0x6b },
	{ 0, 0, (char)0x46 },
	{ 97, 98, 0 },
	{ 0, 0, (char)0x71 },
	{ 0, 0, (char)0x2a },
	{ 100, 105, 0 },
	{ 101, 104, 0 },
	{ 102, 103, 0 },
	{ 0, 0, (char)0x77 },
	{ 0, 0, (char)0x4b },
	{ 0, 0, (char)0x2d },
	{ 106, 107, 0 },
	{ 0, 0, (char)0x57 },
	{ 0, 0, (char)0x4a },
	{ 109, 126, 0 },
	{ 110, 117, 0 },
	{ 111, 114, 0 },
	{ 112, 113, 0 },
	{ 0, 0, (char)0x7a },
	{ 0, 0, (char)0x2b },
	{ 115, 116, 0 },
	{ 0, 0, (char)0x59 },
	{ 0, 0, (char)0x6a },
	{ 118, 123, 0 },
	{ 119, 122, 0 },
	{ 120, 121, 0 },
	{ 0, 0, (char)0x85 },
	{ 0, 0, (char)0x29 },
	{ 0, 0, (char)0x51 },
	{ 124, 125, 0 },
	{ 0, 0, (char)0x5a },
	{ 0, 0, (char)0x7e },
	{ 127, 148, 0 },
	{ 128, 137, 0 },
	{ 129, 134, 0 },
	{ 130, 133, 0 },
	{ 131, 132, 0 },
	{ 0, 0, (char)0x8b },
	{ 0, 0, (char)0x3c },
	{ 0, 0, (char)0x8a },
	{ 135, 136, 0 },
	{ 0, 0, (char)0x7f },
	{ 0, 0, (char)0x3a },
	{ 138, 143, 0 },
	{ 139, 142, 0 },
	{ 140, 141, 0 },
	{ 0, 0, (char)0x87 },
	{ 0, 0, (char)0x23 },
	{ 0, 0, (char)0x78 },
	{ 144, 147, 0 },
	{ 145, 146, 0 },
	{ 0, 0, (char)0x58 },
	{ 0, 0, (char)0x91 },
	{ 0, 0, (char)0x83 },
	{ 149, 168, 0 },
	{ 150, 159, 0 },
	{ 151, 156, 0 },
	{ 152, 155, 0 },
	{ 153, 154, 0 },
	{ 0, 0, (char)0x88 },
	{ 0, 0, (char)0x60 },
	{ 0, 0, (char)0x32 },
	{ 157, 158, 0 },
	{ 0, 0, (char)0x30 },
	{ 0, 0, (char)0x31 },
	{ 160, 165, 0 },
	{ 161, 164, 0 },
	{ 162, 163, 0 },
	{ 0, 0, (char)0x28 },
	{ 0, 0, (char)0x2f },
	{ 0, 0, (char)0x5d },
	{ 166, 167, 0 },
	{ 0, 0, (char)0x3d },
	{ 0, 0, (char)0x86 },
	{ 169, 184, 0 },
	{ 170, 177, 0 },
	{ 171, 174, 0 },
	{ 172, 173, 0 },
	{ 0, 0, (char)0x5e },
	{ 0, 0, (char)0x33 },
	{ 175, 176, 0 },
	{ 0, 0, (char)0x39 },
	{ 0, 0, (char)0x34 },
	{ 178, 181, 0 },
	{ 179, 180, 0 },
	{ 0, 0, (char)0x7d },
	{ 0, 0, (char)0x38 },
	{ 182, 183, 0 },
	{ 0, 0, (char)0x5c },
	{ 0, 0, (char)0x22 },
	{ 185, 198, 0 },
	{ 186, 193, 0 },
	{ 187, 190, 0 },
	{ 188, 189, 0 },
	{ 0, 0, (char)0x3e },
	{ 0, 0, (char)0x26 },
	{ 191, 192, 0 },
	{ 0, 0, (char)0x8d },
	{ 0, 0, (char)0x7b },
	{ 194, 197, 0 },
	{ 195, 196, 0 },
	{ 0, 0, (char)0x35 },
	{ 0, 0, (char)0x36 },
	{ 0, 0, (char)0x8f },
	{ 199, 210, 0 },
	{ 200, 205, 0 },
	{ 201, 204, 0 },
	{ 202, 203, 0 },
	{ 0, 0, (char)0x8e },
	{ 0, 0, (char)0x8c },
	{ 0, 0, (char)0x37 },
	{ 206, 209, 0 },
	{ 207, 208, 0 },
	{ 0, 0, (char)0x89 },
	{ 0, 0, (char)0x24 },
	{ 0, 0, (char)0x92 },
	{ 211, 218, 0 },
	{ 212, 215, 0 },
	{ 213, 214, 0 },
	{ 0, 0, (char)0x5b },
	{ 0, 0, (char)0x80 },
	{ 216, 217, 0 },
	{ 0, 0, (char)0x81 },
	{ 0, 0, (char)0x40 },
	{ 219, 222, 0 },
	{ 220, 221, 0 },
	{ 0, 0, (char)0x5f },
	{ 0, 0, (char)0x82 },
	{ 223, 224, 0 },
	{ 0, 0, (char)0x25 },
	{ 225, 226, 0 },
	{ 0, 0, (char)0x9 },
	{ 227, 228, 0 },
	{ 0, 0, (char)0x3b },
	{ 0, 0, (char)0x7c },
};

static const HuffTree huffTree_00368[] = {
	{ 1, 20, 0 },
	{ 2, 7, 0 },
	{ 3, 4, 0 },
	{ 0, 0, ' ' },
	{ 5, 6, 0 },
	{ 0, 0, 'e' },
	{ 0, 0, 'a' },
	{ 8, 15, 0 },
	{ 9, 12, 0 },
	{ 10, 11, 0 },
	{ 0, 0, 'o' },
	{ 0, 0, 's' },
	{ 13, 14, 0 },
	{ 0, 0, 't' },
	{ 0, 0, 'n' },
	{ 16, 19, 0 },
	{ 17, 18, 0 },
	{ 0, 0, '.' },
	{ 0, 0, 'i' },
	{ 0, 0, 'r' },
	{ 21, 44, 0 },
	{ 22, 31, 0 },
	{ 23, 28, 0 },
	{ 24, 27, 0 },
	{ 25, 26, 0 },
	{ 0, 0, (char)0 },
	{ 0, 0, 'E' },
	{ 0, 0, 'u' },
	{ 29, 30, 0 },
	{ 0, 0, 'm' },
	{ 0, 0, 'A' },
	{ 32, 39, 0 },
	{ 33, 36, 0 },
	{ 34, 35, 0 },
	{ 0, 0, 'l' },
	{ 0, 0, 'I' },
	{ 37, 38, 0 },
	{ 0, 0, 'd' },
	{ 0, 0, 'R' },
	{ 40, 43, 0 },
	{ 41, 42, 0 },
	{ 0, 0, 'N' },
	{ 0, 0, 'S' },
	{ 0, 0, 'T' },
	{ 45, 68, 0 },
	{ 46, 55, 0 },
	{ 47, 52, 0 },
	{ 48, 51, 0 },
	{ 49, 50, 0 },
	{ 0, 0, 'O' },
	{ 0, 0, 'h' },
	{ 0, 0, 'c' },
	{ 53, 54, 0 },
	{ 0, 0, 'D' },
	{ 0, 0, 'g' },
	{ 56, 63, 0 },
	{ 57, 60, 0 },
	{ 58, 59, 0 },
	{ 0, 0, 'L' },
	{ 0, 0, 'C' },
	{ 61, 62, 0 },
	{ 0, 0, 'p' },
	{ 0, 0, 'U' },
	{ 64, 67, 0 },
	{ 65, 66, 0 },
	{ 0, 0, '!' },
	{ 0, 0, 'y' },
	{ 0, 0, 'M' },
	{ 69, 88, 0 },
	{ 70, 79, 0 },
	{ 71, 76, 0 },
	{ 72, 75, 0 },
	{ 73, 74, 0 },
	{ 0, 0, 'P' },
	{ 0, 0, 'v' },
	{ 0, 0, 'H' },
	{ 77, 78, 0 },
	{ 0, 0, '?' },
	{ 0, 0, 'b' },
	{ 80, 85, 0 },
	{ 81, 84, 0 },
	{ 82, 83, 0 },
	{ 0, 0, (char)39 },
	{ 0, 0, 'f' },
	{ 0, 0, ',' },
	{ 86, 87, 0 },
	{ 0, 0, 'G' },
	{ 0, 0, 'B' },
	{ 89, 108, 0 },
	{ 90, 99, 0 },
	{ 91, 96, 0 },
	{ 92, 95, 0 },
	{ 93, 94, 0 },
	{ 0, 0, 'V' },
	{ 0, 0, 'k' },
	{ 0, 0, 'F' },
	{ 97, 98, 0 },
	{ 0, 0, 'q' },
	{ 0, 0, 'w' },
	{ 100, 105, 0 },
	{ 101, 104, 0 },
	{ 102, 103, 0 },
	{ 0, 0, 'K' },
	{ 0, 0, '-' },
	{ 0, 0, 'W' },
	{ 106, 107, 0 },
	{ 0, 0, 'J' },
	{ 0, 0, '*' },
	{ 109, 128, 0 },
	{ 110, 117, 0 },
	{ 111, 114, 0 },
	{ 112, 113, 0 },
	{ 0, 0, 'z' },
	{ 0, 0, 'Y' },
	{ 115, 116, 0 },
	{ 0, 0, 'j' },
	{ 0, 0, '+' },
	{ 118, 123, 0 },
	{ 119, 122, 0 },
	{ 120, 121, 0 },
	{ 0, 0, 'Q' },
	{ 0, 0, (char)133 },
	{ 0, 0, ')' },
	{ 124, 127, 0 },
	{ 125, 126, 0 },
	{ 0, 0, 'Z' },
	{ 0, 0, (char)139 },
	{ 0, 0, '<' },
	{ 129, 150, 0 },
	{ 130, 139, 0 },
	{ 131, 136, 0 },
	{ 132, 135, 0 },
	{ 133, 134, 0 },
	{ 0, 0, (char)149 },
	{ 0, 0, (char)126 },
	{ 0, 0, (char)138 },
	{ 137, 138, 0 },
	{ 0, 0, (char)135 },
	{ 0, 0, ':' },
	{ 140, 145, 0 },
	{ 141, 144, 0 },
	{ 142, 143, 0 },
	{ 0, 0, (char)127 },
	{ 0, 0, ']' },
	{ 0, 0, 'x' },
	{ 146, 149, 0 },
	{ 147, 148, 0 },
	{ 0, 0, 'X' },
	{ 0, 0, 'X' },
	{ 0, 0, (char)145 },
	{ 151, 172, 0 },
	{ 152, 163, 0 },
	{ 153, 158, 0 },
	{ 154, 157, 0 },
	{ 155, 156, 0 },
	{ 0, 0, (char)136 },
	{ 0, 0, '`' },
	{ 0, 0, '2' },
	{ 159, 162, 0 },
	{ 160, 161, 0 },
	{ 0, 0, '0' },
	{ 0, 0, (char)131 },
	{ 0, 0, '1' },
	{ 164, 169, 0 },
	{ 165, 168, 0 },
	{ 166, 167, 0 },
	{ 0, 0, '/' },
	{ 0, 0, '(' },
	{ 0, 0, '=' },
	{ 170, 171, 0 },
	{ 0, 0, (char)134 },
	{ 0, 0, '^' },
	{ 173, 190, 0 },
	{ 174, 181, 0 },
	{ 175, 178, 0 },
	{ 176, 177, 0 },
	{ 0, 0, '3' },
	{ 0, 0, '9' },
	{ 179, 180, 0 },
	{ 0, 0, (char)152 },
	{ 0, 0, '4' },
	{ 182, 187, 0 },
	{ 183, 186, 0 },
	{ 184, 185, 0 },
	{ 0, 0, '}' },
	{ 0, 0, '8' },
	{ 0, 0, '\\' },
	{ 188, 189, 0 },
	{ 0, 0, '"' },
	{ 0, 0, (char)144 },
	{ 191, 206, 0 },
	{ 192, 199, 0 },
	{ 193, 196, 0 },
	{ 194, 195, 0 },
	{ 0, 0, '&' },
	{ 0, 0, (char)141 },
	{ 197, 198, 0 },
	{ 0, 0, '5' },
	{ 0, 0, '6' },
	{ 200, 203, 0 },
	{ 201, 202, 0 },
	{ 0, 0, (char)146 },
	{ 0, 0, (char)143 },
	{ 204, 205, 0 },
	{ 0, 0, (char)142 },
	{ 0, 0, (char)147 },
	{ 207, 220, 0 },
	{ 208, 213, 0 },
	{ 209, 212, 0 },
	{ 210, 211, 0 },
	{ 0, 0, (char)140 },
	{ 0, 0, '7' },
	{ 0, 0, (char)128 },
	{ 214, 217, 0 },
	{ 215, 216, 0 },
	{ 0, 0, (char)129 },
	{ 0, 0, (char)153 },
	{ 218, 219, 0 },
	{ 0, 0, '$' },
	{ 0, 0, '@' },
	{ 221, 230, 0 },
	{ 222, 227, 0 },
	{ 223, 226, 0 },
	{ 224, 225, 0 },
	{ 0, 0, '[' },
	{ 0, 0, (char)154 },
	{ 0, 0, '_' },
	{ 228, 229, 0 },
	{ 0, 0, '>' },
	{ 0, 0, (char)150 },
	{ 231, 236, 0 },
	{ 232, 235, 0 },
	{ 233, 234, 0 },
	{ 0, 0, (char)130 },
	{ 0, 0, '%' },
	{ 0, 0, (char)9 },
	{ 237, 240, 0 },
	{ 238, 239, 0 },
	{ 0, 0, (char)156 },
	{ 0, 0, (char)151 },
	{ 241, 242, 0 },
	{ 0, 0, '{' },
	{ 243, 244, 0 },
	{ 0, 0, (char)148 },
	{ 0, 0, '!' },
};

static const HuffTree huffTree_00372[] = {
	{ 1, 20, 0 },
	{ 2, 7, 0 },
	{ 3, 4, 0 },
	{ 0, 0, ' ' },
	{ 5, 6, 0 },
	{ 0, 0, 'e' },
	{ 0, 0, 'a' },
	{ 8, 15, 0 },
	{ 9, 12, 0 },
	{ 10, 11, 0 },
	{ 0, 0, 'o' },
	{ 0, 0, 's' },
	{ 13, 14, 0 },
	{ 0, 0, 't' },
	{ 0, 0, 'n' },
	{ 16, 19, 0 },
	{ 17, 18, 0 },
	{ 0, 0, '.' },
	{ 0, 0, 'i' },
	{ 0, 0, 'r' },
	{ 21, 44, 0 },
	{ 22, 31, 0 },
	{ 23, 28, 0 },
	{ 24, 27, 0 },
	{ 25, 26, 0 },
	{ 0, 0, (char)0 },
	{ 0, 0, 'E' },
	{ 0, 0, 'u' },
	{ 29, 30, 0 },
	{ 0, 0, 'm' },
	{ 0, 0, 'A' },
	{ 32, 39, 0 },
	{ 33, 36, 0 },
	{ 34, 35, 0 },
	{ 0, 0, 'l' },
	{ 0, 0, 'I' },
	{ 37, 38, 0 },
	{ 0, 0, 'd' },
	{ 0, 0, 'R' },
	{ 40, 43, 0 },
	{ 41, 42, 0 },
	{ 0, 0, 'N' },
	{ 0, 0, 'S' },
	{ 0, 0, 'T' },
	{ 45, 68, 0 },
	{ 46, 55, 0 },
	{ 47, 52, 0 },
	{ 48, 51, 0 },
	{ 49, 50, 0 },
	{ 0, 0, 'O' },
	{ 0, 0, 'h' },
	{ 0, 0, 'c' },
	{ 53, 54, 0 },
	{ 0, 0, 'D' },
	{ 0, 0, 'g' },
	{ 56, 63, 0 },
	{ 57, 60, 0 },
	{ 58, 59, 0 },
	{ 0, 0, 'L' },
	{ 0, 0, 'C' },
	{ 61, 62, 0 },
	{ 0, 0, 'p' },
	{ 0, 0, 'U' },
	{ 64, 67, 0 },
	{ 65, 66, 0 },
	{ 0, 0, '!' },
	{ 0, 0, 'y' },
	{ 0, 0, 'M' },
	{ 69, 88, 0 },
	{ 70, 79, 0 },
	{ 71, 76, 0 },
	{ 72, 75, 0 },
	{ 73, 74, 0 },
	{ 0, 0, 'P' },
	{ 0, 0, 'v' },
	{ 0, 0, 'H' },
	{ 77, 78, 0 },
	{ 0, 0, '?' },
	{ 0, 0, 'b' },
	{ 80, 85, 0 },
	{ 81, 84, 0 },
	{ 82, 83, 0 },
	{ 0, 0, (char)39 },
	{ 0, 0, 'f' },
	{ 0, 0, ',' },
	{ 86, 87, 0 },
	{ 0, 0, 'G' },
	{ 0, 0, 'B' },
	{ 89, 108, 0 },
	{ 90, 99, 0 },
	{ 91, 96, 0 },
	{ 92, 95, 0 },
	{ 93, 94, 0 },
	{ 0, 0, 'V' },
	{ 0, 0, 'k' },
	{ 0, 0, 'F' },
	{ 97, 98, 0 },
	{ 0, 0, 'q' },
	{ 0, 0, 'w' },
	{ 100, 105, 0 },
	{ 101, 104, 0 },
	{ 102, 103, 0 },
	{ 0, 0, 'K' },
	{ 0, 0, '-' },
	{ 0, 0, 'W' },
	{ 106, 107, 0 },
	{ 0, 0, 'J' },
	{ 0, 0, '*' },
	{ 109, 128, 0 },
	{ 110, 117, 0 },
	{ 111, 114, 0 },
	{ 112, 113, 0 },
	{ 0, 0, 'z' },
	{ 0, 0, 'Y' },
	{ 115, 116, 0 },
	{ 0, 0, 'j' },
	{ 0, 0, '+' },
	{ 118, 123, 0 },
	{ 119, 122, 0 },
	{ 120, 121, 0 },
	{ 0, 0, 'Q' },
	{ 0, 0, (char)133 },
	{ 0, 0, ')' },
	{ 124, 127, 0 },
	{ 125, 126, 0 },
	{ 0, 0, 'Z' },
	{ 0, 0, (char)139 },
	{ 0, 0, '<' },
	{ 129, 150, 0 },
	{ 130, 139, 0 },
	{ 131, 136, 0 },
	{ 132, 135, 0 },
	{ 133, 134, 0 },
	{ 0, 0, (char)149 },
	{ 0, 0, (char)126 },
	{ 0, 0, (char)138 },
	{ 137, 138, 0 },
	{ 0, 0, (char)135 },
	{ 0, 0, ':' },
	{ 140, 145, 0 },
	{ 141, 144, 0 },
	{ 142, 143, 0 },
	{ 0, 0, (char)127 },
	{ 0, 0, ']' },
	{ 0, 0, '#' },
	{ 146, 149, 0 },
	{ 147, 148, 0 },
	{ 0, 0, 'x' },
	{ 0, 0, 'X' },
	{ 0, 0, (char)145 },
	{ 151, 172, 0 },
	{ 152, 163, 0 },
	{ 153, 158, 0 },
	{ 154, 157, 0 },
	{ 155, 156, 0 },
	{ 0, 0, (char)136 },
	{ 0, 0, '`' },
	{ 0, 0, '2' },
	{ 159, 162, 0 },
	{ 160, 161, 0 },
	{ 0, 0, '0' },
	{ 0, 0, (char)131 },
	{ 0, 0, '1' },
	{ 164, 169, 0 },
	{ 165, 168, 0 },
	{ 166, 167, 0 },
	{ 0, 0, '/' },
	{ 0, 0, '(' },
	{ 0, 0, '=' },
	{ 170, 171, 0 },
	{ 0, 0, (char)134 },
	{ 0, 0, '^' },
	{ 173, 190, 0 },
	{ 174, 181, 0 },
	{ 175, 178, 0 },
	{ 176, 177, 0 },
	{ 0, 0, '3' },
	{ 0, 0, '9' },
	{ 179, 180, 0 },
	{ 0, 0, (char)152 },
	{ 0, 0, '4' },
	{ 182, 187, 0 },
	{ 183, 186, 0 },
	{ 184, 185, 0 },
	{ 0, 0, '}' },
	{ 0, 0, '8' },
	{ 0, 0, '\\' },
	{ 188, 189, 0 },
	{ 0, 0, '"' },
	{ 0, 0, (char)144 },
	{ 191, 206, 0 },
	{ 192, 199, 0 },
	{ 193, 196, 0 },
	{ 194, 195, 0 },
	{ 0, 0, '&' },
	{ 0, 0, (char)141 },
	{ 197, 198, 0 },
	{ 0, 0, '5' },
	{ 0, 0, '6' },
	{ 200, 203, 0 },
	{ 201, 202, 0 },
	{ 0, 0, (char)146 },
	{ 0, 0, (char)143 },
	{ 204, 205, 0 },
	{ 0, 0, (char)142 },
	{ 0, 0, (char)147 },
	{ 207, 220, 0 },
	{ 208, 213, 0 },
	{ 209, 212, 0 },
	{ 210, 211, 0 },
	{ 0, 0, (char)140 },
	{ 0, 0, '7' },
	{ 0, 0, (char)128 },
	{ 214, 217, 0 },
	{ 215, 216, 0 },
	{ 0, 0, (char)129 },
	{ 0, 0, (char)153 },
	{ 218, 219, 0 },
	{ 0, 0, '$' },
	{ 0, 0, '@' },
	{ 221, 230, 0 },
	{ 222, 227, 0 },
	{ 223, 226, 0 },
	{ 224, 225, 0 },
	{ 0, 0, '[' },
	{ 0, 0, (char)154 },
	{ 0, 0, '_' },
	{ 228, 229, 0 },
	{ 0, 0, '>' },
	{ 0, 0, (char)150 },
	{ 231, 236, 0 },
	{ 232, 235, 0 },
	{ 233, 234, 0 },
	{ 0, 0, (char)130 },
	{ 0, 0, '%' },
	{ 0, 0, (char)9 },
	{ 237, 240, 0 },
	{ 238, 239, 0 },
	{ 0, 0, (char)156 },
	{ 0, 0, (char)151 },
	{ 241, 242, 0 },
	{ 0, 0, '{' },
	{ 243, 244, 0 },
	{ 0, 0, (char)148 },
	{ 0, 0, '!' },
};

void SkyText::initHuffTree() {
	switch (SkyState::_systemVars.gameVersion) {
	case 267:
		_huffTree = huffTree_00267;
		break;
	case 288:
		_huffTree = huffTree_00288;
		break;
	case 303:
		_huffTree = huffTree_00303;
		break;
	case 331:
	case 348:
		_huffTree = huffTree_00331;
		break;
	case 365:
	case 368:
		_huffTree = huffTree_00368;
		break;
	case 372:
		_huffTree = huffTree_00372;
		break;
	default:
		error("Unknown game version");
	}
}

char SkyText::getTextChar() {
	int pos = 0;
	for (;;) {
		if (getTBit() == 0)
			pos = _huffTree[pos].lChild;
		else
			pos = _huffTree[pos].rChild;
		if (_huffTree[pos].lChild == 0 && _huffTree[pos].rChild == 0) {
			return _huffTree[pos].value;
		}
	}
}

