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
		_controlCharacterSet.charSpacing = 0;

		_linkCharacterSet.addr = _skyDisk->loadFile(60521, NULL);
		_linkCharacterSet.charHeight = 12;
		_linkCharacterSet.charSpacing = 1;

		patchLINCCharset();
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

void SkyText::patchChar(byte *charSetPtr, int width, int height, int c, const uint16 *data) {
	byte *ptr = charSetPtr + (CHAR_SET_HEADER + (height << 2) * c);

	charSetPtr[c] = width;

	for (int i = 0; i < height; i++) {
		ptr[i * 4 + 0] = ptr[i * 4 + 2] = (data[i] & 0xFF00) >> 8;
		ptr[i * 4 + 1] = ptr[i * 4 + 3] = data[i] & 0x00FF;
	}
}

void SkyText::patchLINCCharset() {
	// The LINC terminal charset looks strange in some cases. This
	// function attempts to patch up the worst blemishes.

	byte *charSetPtr = _controlCharacterSet.addr;
	int charHeight = _controlCharacterSet.charHeight;

	// In v0.0288, decrease the character spacing is too wide. Decrease
	// the width for every character by one, except for space which needs
	// to be one pixel wider than before.

	if (SkyState::_systemVars.gameVersion == 288) {
		for (int i = 1; i < CHAR_SET_HEADER; i++)
			charSetPtr[i]--;
		charSetPtr[0]++;
	}

	// NOTE: I have only tested this part of the code with v0.0372

	// Several characters are different in this charset than in the other
	// two. This is particularly noticeable when using a non-English
	// version.

	const uint16 slash[] = {
		0x0000, 0x0000, 0x0000, 0x0800, 0x1000, 0x1000,
		0x2000,	0x2000, 0x4000, 0x0000, 0x0000, 0x0000
	};

	const uint16 lt[] = {
		0x0000, 0x0000, 0x0800, 0x1000, 0x2000, 0x4000,
		0x2000, 0x1000, 0x0800, 0x0000, 0x0000, 0x0000
	};

	const uint16 gt[] = {
		0x0000, 0x0000, 0x4000, 0x2000, 0x1000, 0x0800,
		0x1000, 0x2000, 0x4000, 0x0000, 0x0000, 0x0000
	};

	const uint16 a_umlaut[] = {
        0x0000, 0x0000, 0x2800, 0x0000, 0x3000, 0x0800,
		0x3800, 0x4800, 0x3800,	0x0000,	0x0000,	0x0000
	};

	const uint16 o_umlaut[] = {
		0x0000,	0x0000,	0x4800,	0x0000,	0x3000,	0x4800,
		0x4800,	0x4800,	0x3000,	0x0000,	0x0000,	0x0000
	};

	const uint16 u_umlaut[] = {
		0x0000,	0x0000,	0x4800,	0x0000,	0x4800,	0x4800,
		0x4800,	0x4800,	0x3000,	0x0000,	0x0000,	0x0000
	};

	const uint16 A_umlaut[] = {
		0x0000, 0x4800, 0x0000, 0x3000, 0x4800, 0x4800,
		0x7800, 0x4800, 0x4800, 0x0000, 0x0000, 0x0000
	};

	const uint16 O_umlaut[] = {
		0x0000,	0x4800,	0x0000,	0x3000,	0x4800, 0x4800,
		0x4800,	0x4800,	0x3000,	0x0000,	0x0000,	0x0000
	};

	const uint16 U_umlaut[] = {
		0x0000, 0x4800, 0x0000, 0x4800, 0x4800, 0x4800,
		0x4800, 0x4800, 0x3000, 0x0000,	0x0000,	0x0000
	};

	const uint16 normal_j[] = {
		0x0000, 0x0000, 0x0000, 0x0800, 0x0000, 0x0800,
		0x0800,	0x0800, 0x0800, 0x4800, 0x3000, 0x0000
	};

	const uint16 german_sz[] = {
		0x0000, 0x0000, 0x2000, 0x5000, 0x5000, 0x4800,
		0x4800, 0x4800, 0x5000, 0x0000, 0x0000, 0x0000
	};

	const uint16 normal_1[] = {
		0x0000, 0x0000, 0x0000, 0x1000, 0x7000, 0x1000,
		0x1000, 0x1000, 0x7c00, 0x0000, 0x0000, 0x0000
	};

	patchChar(charSetPtr, 5, charHeight,  3, u_umlaut);
	patchChar(charSetPtr, 5, charHeight,  8, german_sz);
	patchChar(charSetPtr, 5, charHeight,  9, o_umlaut);
	patchChar(charSetPtr, 5, charHeight, 93, U_umlaut);
	patchChar(charSetPtr, 5, charHeight, 74, normal_j);
	patchChar(charSetPtr, 6, charHeight, 17, normal_1);
	if (SkyState::_systemVars.gameVersion <= 303) {
		patchChar(charSetPtr, 5, charHeight, 10, a_umlaut);
	} else {
		patchChar(charSetPtr, 5, charHeight, 94, A_umlaut);
		patchChar(charSetPtr, 5, charHeight, 95, O_umlaut);

		// Used by, for instance, the BRIEFING.DOC file in all (?) languages
		patchChar(charSetPtr, 5, charHeight, 96, lt);
		patchChar(charSetPtr, 5, charHeight, 97, gt);
		patchChar(charSetPtr, 5, charHeight, 98, slash);
	}
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
	uint16* msgData = (uint16 *)SkyState::fetchCompact(textInfoId);
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

	uint8 textChar;
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

	textChar = (uint8)*curPos++;
	_dtLetters++;

	// work around bug #778105 (line width exceeded)
	char *tmpPtr = strstr(textPtr, "MUND-BEATMUNG!");
	if (tmpPtr)
		strcpy(tmpPtr, "MUND BEATMUNG!");

	while (textChar >= 0x20) {
		if ((_curCharSet == 1) && (textChar >= 0x80))
			textChar = 0x20;

		textChar -= 0x20;
		if (textChar == 0) {
			lastSpace = curPos; //keep track of last space
			*(uint32 *)centerTblPtr = TO_LE_32(lineWidth);
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

		textChar = (uint8)*curPos++;
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
	((struct dataFileHeader *)curDest)->s_width = _dtLineWidth;
	((struct dataFileHeader *)curDest)->s_height = (uint16)(_charHeight * _dtLines);
	((struct dataFileHeader *)curDest)->s_sp_size = (uint16)(_dtLineWidth * _charHeight * _dtLines);
	((struct dataFileHeader *)curDest)->s_offset_x = 0;
	((struct dataFileHeader *)curDest)->s_offset_y = 0;
	
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

		textChar = (uint8)*curPos++;
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

void SkyText::makeGameCharacter(uint8 textChar, uint8 *charSetPtr, uint8 *&dest, uint8 color) {

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

	dataFileHeader *header = (dataFileHeader *)sprData;
	sprData += sizeof(dataFileHeader);
	for (uint16 cnt = 0; cnt < header->s_sp_size; cnt++)
		if (sprData[cnt] >= 241) sprData[cnt] = newCol;
}

void SkyText::initHuffTree() {
	switch (SkyState::_systemVars.gameVersion) {
	case 109:
		_huffTree = _huffTree_00109;
		break;
	case 267:
		_huffTree = _huffTree_00267;
		break;
	case 288:
		_huffTree = _huffTree_00288;
		break;
	case 303:
		_huffTree = _huffTree_00303;
		break;
	case 331:
		_huffTree = _huffTree_00331;
		break;
	case 348:
		_huffTree = _huffTree_00348;
		break;
	case 365:
		_huffTree = _huffTree_00365;
		break;
	case 368:
		_huffTree = _huffTree_00368;
		break;
	case 372:
		_huffTree = _huffTree_00372;
		break;
	default:
		error("Unknown game version %d", SkyState::_systemVars.gameVersion);
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

