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

SkyText::SkyText(SkyDisk *skyDisk, uint32 gameVersion, uint16 language) {
	_skyDisk = skyDisk;
	_language = language;
	_gameVersion = gameVersion;

	_mainCharacterSet.addr = _skyDisk->loadFile(CHAR_SET_FILE, NULL);
	_mainCharacterSet.charHeight = MAIN_CHAR_HEIGHT;
	_mainCharacterSet.charSpacing = 0;
	
	fnSetFont(0);
	
	if (!SkyState::isDemo(_gameVersion)) {
		_controlCharacterSet.addr = _skyDisk->loadFile(60520, NULL);
		_controlCharacterSet.charHeight = 12;
		_controlCharacterSet.charSpacing = 1;
		
		_linkCharacterSet.addr = _skyDisk->loadFile(60521, NULL);
		_linkCharacterSet.charHeight = 12;
		_linkCharacterSet.charSpacing = 0;
	}

	if (SkyState::isCDVersion(_gameVersion)) {
		_preAfterTableArea = _skyDisk->loadFile(60522, NULL);
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

void SkyText::getText(uint32 textNr) { //load text #"textNr" into textBuffer
	uint32 sectionNo = (textNr & 0x0F000) >> 10;
	
	if (SkyState::_itemList[FIRST_TEXT_SEC + sectionNo] == (void **)NULL) { //check if already loaded
		debug(5, "Loading Text item(s) for Section %d", (sectionNo>>2));
		
		uint32 fileNo = (sectionNo >> 2); 
		fileNo += ((_language * NO_OF_TEXT_SECTIONS) + 60600);
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
				warning("line width exceeded!");

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

	byte *curDest = dest;

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
					//FIXME: this is 240 in the original sources (with 1 commented out),
					//yet 240 appears to be white in most palettes.
					*curPos = 1; //240; 

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

	while (cpt->status != 0xFFFF) { //-1
		compactNum++;
		cpt = SkyState::fetchCompact(compactNum);
	}

	cpt->flag = (compactNum - FIRST_TEXT_COMPACT) + FIRST_TEXT_BUFFER;

	byte *oldText = (byte *)SkyState::_itemList[compactNum];
	SkyState::_itemList[compactNum] = (void **)textData; 

	if (oldText != NULL)
		free (oldText);

	cpt->logic = logicNum; 
	cpt->status = ST_LOGIC | ST_FOREGROUND | ST_RECREATE;
	cpt->screen = SkyLogic::_screen; 

	struct lowTextManager_t ret;
	ret.textData = _dtData;
	ret.compactNum = compactNum;

	return ret;

}

char SkyText_v00267::getTextChar() {

	//NOTE: The decoding code for the floppy versions (both demo and full version) is _completely_ different.
	//This code is based on disassembly of the v0.0267 binary.
	

	if (getTBit())
		goto ab73;
	if (getTBit())
		goto ab34;

	if (getTBit())
		goto ab31;
	if (getTBit())
		goto ab2e;

	return (char)0x20;

ab2e:  
	return (char)0x65;

ab31:  
	return (char)0x73;

ab34:  
	if (getTBit())
		goto ab5c;
	if (getTBit())
		goto ab4f;
	if (getTBit())
		goto ab4c;

	return (char)0x6d;

ab4c:  
	return (char)0x61;

ab4f:  
	if (getTBit())
		goto ab59;

	return (char)0x0;

ab59:  
	return (char)0x6e;

ab5c:  
	if (getTBit())
		goto ab70;
	if (getTBit())
		goto ab6d;
	
	return (char)0x69;

ab6d:  
	return (char)0x74;

ab70:  
	return (char)0x75;

ab73:  
	if (getTBit())
		goto abeb;
	if (getTBit())
		goto abac;
	if (getTBit())
		goto ab9f;
	if (getTBit())
		goto ab9c;
	if (getTBit())
		goto ab99;
	
	return (char)0x2e;

ab99:  
	return (char)0x72;

ab9c:  
	return (char)0x45;

ab9f:  
	if (getTBit())
		goto aba9;
	
	return (char)0x6f;

aba9:  
	return (char)0x68;

abac:  
	if (getTBit())
		goto abd4;
	if (getTBit())
		goto abc7;
	if (getTBit())
		goto abc4;

	return (char)0x44;

abc4:  
	return (char)0x67;

abc7:  
	if (getTBit())
		goto abd1;
	
	return (char)0x49;

abd1:  
	return (char)0x79;

abd4:  
	if (getTBit())
		goto abe8;
	if (getTBit())
		goto abe5;
	
	return (char)0x53;

abe5:  
	return (char)0x64;

abe8:  
	return (char)0x6c;

abeb:  
	if (getTBit())
		goto ac4f;
	if (getTBit())
		goto ac1a;
	if (getTBit())
		goto ac0d;
	if (getTBit())
		goto ac0a;
       
	return (char)0x54;

ac0a:  
	return (char)0x4e;

ac0d:  
       if (getTBit())
		goto ac17;

	return (char)0x63;

ac17:  
	return (char)0x52;

ac1a:  
	if (getTBit())
		goto ac38;
	if (getTBit())
		goto ac35;
	if (getTBit())
		goto ac32;
	
	return (char)0x41;

ac32:  
	return (char)0x4f;

ac35:  
	return (char)0x4c;

ac38:  
	if (getTBit())
		goto ac4c;
	if (getTBit())
		goto ac49;
	
	return (char)0x48;

ac49:  
	return (char)0x43;

ac4c:  
	return (char)0x55;

ac4f:  
	if (getTBit())
		goto acb3;
	if (getTBit())
		goto ac88;
	if (getTBit())
		goto ac7b;
	if (getTBit())
		goto ac78;
	if (getTBit())
		goto ac75;
	
	return (char)0x62;

ac75:  
	return (char)0x21;

ac78:  
	return (char)0x27;

ac7b:  
	if (getTBit())
		goto ac85;
	
	return (char)0x77;

ac85:  
	return (char)0x66;

ac88:  
	if (getTBit())
		goto aca6;
	if (getTBit())
		goto aca3;
	if (getTBit())
		goto aca0;
	
	return (char)0x47;

aca0:  
	return (char)0x4d;

aca3:  
	return (char)0x2c;

aca6:  
	if (getTBit())
		goto acb0;
	
	return (char)0x57;

acb0:  
	return (char)0x42;

acb3:  
	if (getTBit())
		goto acf9;
	if (getTBit())
		goto ace2;
	if (getTBit())
		goto acd5;
	if (getTBit())
		goto acd2;
	
	return (char)0x3f;

acd2:  
	return (char)0x50;

acd5:  
	if (getTBit())
		goto acdf;
	
	return (char)0x6b;

acdf:  
	return (char)0x70;

ace2:  
	if (getTBit())
		goto acf6;
	if (getTBit())
		goto acf3;
	
	return (char)0x76;

acf3:  
	return (char)0x46;

acf6:  
	return (char)0x4b;

acf9:  
	if (getTBit())
		goto ad35;
	if (getTBit())
		goto ad1e;
	if (getTBit())
		goto ad1b;
	if (getTBit())
		goto ad18;
	
	return (char)0x56;

ad18:  
	return (char)0x59;

ad1b:  
	return (char)0x7a;

ad1e:  
	if (getTBit())
		goto ad32;
	if (getTBit())
		goto ad2f;
	
	return (char)0x2d;

ad2f:  
	return (char)0x23;

ad32:  
	return (char)0x4a;

ad35:  
	if (getTBit())
		goto ad7b;
	if (getTBit())
		goto ad5a;
	if (getTBit())
		goto ad57;
	if (getTBit())
		goto ad54;
	
	return (char)0x5a;

ad54:  
	return (char)0x28;

ad57:  
	return (char)0x29;

ad5a:  
	if (getTBit()) 
		goto ad6e;
	if (getTBit())
		goto ad6b;
	
	return (char)0x6a;

ad6b:  
	return (char)0x2b;

ad6e:  
	if (getTBit())
		goto ad78;
	
	return (char)0x2a;

ad78:  
	return (char)0x71;

ad7b:  
	if (getTBit())
		goto adc1;
	if (getTBit())
		goto ada0;
	if (getTBit())
		goto ad9d;
	if (getTBit())
		goto ad9a;
	
	return (char)0x7d;

ad9a:  
	return (char)0x3a;

ad9d:  
	return (char)0x60;

ada0:  
	if (getTBit())
		goto adb4;
	if (getTBit())
		goto adb1;
	
	return (char)0x30;

adb1:  
	return (char)0x32;

adb4:  
	if (getTBit())
		goto adbe;
	
	return (char)0x31;

adbe:  
	return (char)0x51;

adc1:  
	if (getTBit())
		goto adfd;
	if (getTBit())
		goto ade6;
	if (getTBit())
		goto ade3;
	if (getTBit())
		goto ade0;
	
	return (char)0x58;

ade0:  
	return (char)0x78;

ade3:  
	return (char)0x33;

ade6:  
	if (getTBit())
		goto adfa;
	if (getTBit())
		goto adf7;
	
	return (char)0x39;

adf7:  
	return (char)0x34;

adfa:  
	return (char)0x3c;

adfd:  
	if (getTBit())
		goto ae2f;
	if (getTBit())
		goto ae22;
	if (getTBit())
		goto ae1f;
	if (getTBit())
		goto ae1c;
	
	return (char)0x38;

ae1c:  
	return (char)0x5d;

ae1f:  
	return (char)0x3d;

ae22:  
	if (getTBit())
		goto ae2c;
	
	return (char)0x26;

ae2c:  
	return (char)0x35;

ae2f:  
	if (getTBit())
		goto ae57;
	if (getTBit())
		goto ae4a;
	if (getTBit())
		goto ae47;
	
	return (char)0x36;

ae47:  
	return (char)0x2f;

ae4a:  
	if (getTBit())
		goto ae54;
	
	return (char)0x37;

ae54:  
	return (char)0x24;

ae57:  
	if (getTBit())
		goto ae75;
	if (getTBit())
		goto ae72;
	if (getTBit())
		goto ae6f;
	
	return (char)0x3e;

ae6f:  
	return (char)0x25;

ae72:  
	return (char)0x5e;

ae75:  
	if (getTBit())
		goto ae93;
	if (getTBit())
		goto ae90;
	if (getTBit())
		goto ae8d;
	
	return (char)0x22;

ae8d:  
	return (char)0x40;

ae90:  
	return (char)0x5b;

ae93:  
	if (getTBit())
		goto aea7;
	if (getTBit())
		goto aea4;
	
	return (char)0x5f;

aea4:  
	return (char)0x7b;

aea7:  
	if (getTBit())
		goto aeb1;
	
	return (char)0x5c;

aeb1:  
	return (char)0x7c;

}

char SkyText_v00288::getTextChar() {

	//This code is based on disassembly of the v0.0288 binary.

	if (getTBit())
		goto e5b3;
	if (getTBit())
		goto e574;
	if (getTBit())
		goto e567;

	return (char)0x20;

e567:
	if (getTBit())
		goto e571;

	return (char)0x65;

e571:
	return (char)0x73;

e574:
	if (getTBit())
		goto e59c;
	if (getTBit())
		goto e58f;
	if (getTBit())
		goto e58c;

	return (char)0x6e;

e58c:
	return (char)0x69;

e58f:  
	if (getTBit())
		goto e599;

	return (char)0x74;

e599:
	return (char)0x61;

e59c:
	if (getTBit())
		goto e5b0;
	if (getTBit())
		goto e5ad;

	return (char)0x2e;

e5ad:
	return (char)0x0;

e5b0:
	return (char)0x45;

e5b3:
	if (getTBit())
		goto e62b;
	if (getTBit())
		goto e5ec;
	if (getTBit())
		goto e5df;
	if (getTBit())
		goto e5dc;
	if (getTBit())
		goto e5d9;

	return (char)0x72;

e5d9:
	return (char)0x6f;

e5dc:
	return (char)0x75;

e5df:
	if (getTBit())
		goto e5e9;

	return (char)0x6d;

e5e9:
	return (char)0x68;

e5ec:
	if (getTBit())
		goto e614;
	if (getTBit())
		goto e607;
	if (getTBit())
		goto e604;
	
	return (char)0x49;

e604:
	return (char)0x6c;

e607:
	if (getTBit())
		goto e611;
	
	return (char)0x53;

e611:
	return (char)0x52;

e614:  
	if (getTBit())
		goto e628;
	if (getTBit())
		goto e625;

	return (char)0x64;

e625:
	return (char)0x4e;

e628:
	return (char)0x54;

e62b:  
	if (getTBit())
		goto e6a3;
	if (getTBit())
		goto e664;
	if (getTBit())
		goto e657;
	if (getTBit())
		goto e654;
	if (getTBit())
		goto e651;
	
	return (char)0x63;

e651:  
	return (char)0x41;

e654:  
	return (char)0x4f;

e657:  
	if (getTBit())
		goto e661;
	
	return (char)0x44;

e661:  
	return (char)0x4c;

e664:  
	if (getTBit())
		goto e68c;
	if (getTBit())
		goto e67f;
	if (getTBit())
		goto e67c;

	return (char)0x43;

e67c:  
	return (char)0x67;

e67f:  
	if (getTBit())
		goto e689;
	
	return (char)0x55;

e689:  
	return (char)0x27;

e68c:  
	if (getTBit())
		goto e6a0;
	if (getTBit())
		goto e69d;
	
	return (char)0x79;

e69d:  
	return (char)0x48;

e6a0:  
	return (char)0x70;

e6a3:  
	if (getTBit())
		goto e707;
	if (getTBit())
		goto e6dc;
	if (getTBit())
		goto e6cf;
	if (getTBit())
		goto e6cc;
	if (getTBit())
		goto e6c9;
	
	return (char)0x21;

e6c9:  
	return (char)0x4d;

e6cc:  
	return (char)0x62;

e6cf:  
	if (getTBit())
		goto e6d9;
	
	return (char)0x76;

e6d9:  
	return (char)0x2c;

e6dc:  
	if (getTBit())
		goto e6fa;
	if (getTBit())
		goto e6f7;
	if (getTBit())
		goto e6f4;
	
	return (char)0x50;

e6f4:  
	return (char)0x66;

e6f7:  
	return (char)0x3f;

e6fa:  
	if (getTBit())
		goto e704;
	
	return (char)0x47;

e704:  
	return (char)0x42;

e707:  
	if (getTBit())
		goto e757;
	if (getTBit())
		goto e736;
	if (getTBit())
		goto e729;
	if (getTBit())
		goto e726;
	
	return (char)0x77;

e726:  
	return (char)0x57;

e729:  
	if (getTBit())
		goto e733;
	
	return (char)0x46;

e733:  
	return (char)0x56;

e736:  
	if (getTBit())
		goto e74a;
	if (getTBit())
		goto e747;
	
	return (char)0x6b;

e747:  
	return (char)0x7a;

e74a:  
	if (getTBit())
		goto e754;
	
	return (char)0x4b;

e754:  
	return (char)0x2d;

e757:  
	if (getTBit())
		goto e793;
	if (getTBit())
		goto e77c;
	if (getTBit())
		goto e779;
	if (getTBit())
		goto e776;
	
	return (char)0x4a;

e776:  
	return (char)0x2b;

e779:  
	return (char)0x71;

e77c:  
	if (getTBit())
		goto e790;
	if (getTBit())
		goto e78d;
	
	return (char)0x59;

e78d:  
	return (char)0x6a;

e790:  
	return (char)0x5a;

e793:  
	if (getTBit())
		goto e7e3;
	if (getTBit())
		goto e7c2;
	if (getTBit())
		goto e7b5;
	if (getTBit())
		goto e7b2;
	
	return (char)0x23;

e7b2:  
	return (char)0x51;

e7b5:  
	if (getTBit())
		goto e7bf;
	
	return (char)0x28;

e7bf:  
	return (char)0x29;

e7c2:  
	if (getTBit())
		goto e7d6;
	if (getTBit())
		goto e7d3;
	
	return (char)0x3c;

e7d3:  
	return (char)0x78;

e7d6:  
	if (getTBit())
		goto e7e0;
	
	return (char)0x3a;

e7e0:  
	return (char)0x2a;

e7e3:  
	if (getTBit())
		goto e829;
	if (getTBit())
		goto e812;
	if (getTBit())
		goto e805;
	if (getTBit())
		goto e802;
	
	return (char)0x58;

e802:  
	return (char)0x7d;

e805:  
	if (getTBit())
		goto e80f;
	
	return (char)0x3d;

e80f:  
	return (char)0x60;

e812:  
	if (getTBit())
		goto e826;
	if (getTBit())
		goto e823;
	
	return (char)0x5d;

e823:  
	return (char)0x30;

e826:  
	return (char)0x32;

e829:  
	if (getTBit())
		goto e865;
	if (getTBit())
		goto e84e;
	if (getTBit())
		goto e84b;
	if (getTBit())
		goto e848;
	
	return (char)0x31;

e848:  
	return (char)0x33;

e84b:  
	return (char)0x39;

e84e:  
	if (getTBit())
		goto e862;
	if (getTBit())
		goto e85f;
	
	return (char)0x2f;

e85f:  
	return (char)0x34;

e862:  
	return (char)0x5e;

e865:  
	if (getTBit())
		goto e897;
	if (getTBit())
		goto e88a;
	if (getTBit())
		goto e887;
	if (getTBit())
		goto e884;
	
	return (char)0x38;

e884:  
	return (char)0x3e;

e887:  
	return (char)0x26;

e88a:  
	if (getTBit())
		goto e894;
	
	return (char)0x35;

e894:  
	return (char)0x40;

e897:  
	if (getTBit())
		goto e8bf;
	if (getTBit())
		goto e8b2;
	if (getTBit())
		goto e8af;
	
	return (char)0x36;

e8af:  
	return (char)0x5f;

e8b2:  
	if (getTBit())
		goto e8bc;
	
	return (char)0x5c;

e8bc:  
	return (char)0x37;

e8bf:  
	if (getTBit())
		goto e8dd;
	if (getTBit())
		goto e8da;
	if (getTBit())
		goto e8d7;
	
	return (char)0x5b;

e8d7:  
	return (char)0x24;

e8da:  
	return (char)0x7b;

e8dd:  
	if (getTBit())
		goto e8f1;
	if (getTBit())
		goto e8ee;
	
	return (char)0x25;

e8ee:  
	return (char)0x9;

e8f1:  
	if (getTBit())
		goto e8fb;
	
	return (char)0x22;

e8fb:  
	return (char)0x7c;

}

char SkyText_v00303::getTextChar() {

	//This code is based on disassembly of the v0.0303 binary.

	if (getTBit())
		goto ec6d;
	if (getTBit())
		goto ec2e;
	if (getTBit())
		goto ec21;
	if (getTBit())
		goto ec1e;
   
	return (char)0x20;

ec1e:   
	return (char)0x65;

ec21:   
	if (getTBit())
		goto ec2b;
   
	return (char)0x74;

ec2b:   
	return (char)0x73;

ec2e:   
	if (getTBit())
		goto ec56;
	if (getTBit())
		goto ec49;
	if (getTBit())
		goto ec46;
   
	return (char)0x6e;

ec46:   
	return (char)0x61;

ec49:   
	if (getTBit())
		goto ec53;
   
	return (char)0x69;

ec53:   
	return (char)0x2e;

ec56:   
	if (getTBit())
		goto ec6a;
	if (getTBit())
		goto ec67;
   
	return (char)0x6f;

ec67:   
	return (char)0x0;

ec6a:   
	return (char)0x45;

ec6d:   
	if (getTBit())
		goto ece5;
	if (getTBit())
		goto ecb0;
	if (getTBit())
		goto ec99;
	if (getTBit())
		goto ec96;
	if (getTBit())
		goto ec93;
   
	return (char)0x72;

ec93:   
	return (char)0x75;

ec96:   
	return (char)0x68;

ec99:   
	if (getTBit())
		goto ecad;
	if (getTBit())
		goto ecaa;
   
	return (char)0x49;

ecaa:   
	return (char)0x6d;

ecad:   
	return (char)0x6c;

ecb0:   
	if (getTBit())
		goto ecce;
	if (getTBit())
		goto eccb;
	if (getTBit())
		goto ecc8;
   
	return (char)0x54;

ecc8:   
	return (char)0x53;

eccb:   
	return (char)0x52;

ecce:   
	if (getTBit())
		goto ece2;
	if (getTBit())
		goto ecdf;
  
	return (char)0x64;

ecdf:   
	return (char)0x4e;

ece2:   
	return (char)0x41;

ece5:   
	if (getTBit())
		goto ed5d;
	if (getTBit())
		goto ed28;
	if (getTBit())
		goto ed11;
	if (getTBit())
		goto ed0e;
	if (getTBit())
		goto ed0b;
   
	return (char)0x63;

ed0b:   
	return (char)0x4f;

ed0e:   
	return (char)0x44;

ed11:   
	if (getTBit())
		goto ed25;
	if (getTBit())
		goto ed22;
   
	return (char)0x4c;

ed22:   
	return (char)0x43;

ed25:   
	return (char)0x79;

ed28:   
	if (getTBit())
		goto ed46;
	if (getTBit())
		goto ed43;
	if (getTBit())
		goto ed40;
   
	return (char)0x67;

ed40:   
	return (char)0x27;

ed43:   
	return (char)0x55;

ed46:   
	if (getTBit())
		goto ed5a;
	if (getTBit())
		goto ed57;
   
	return (char)0x48;

ed57:   
	return (char)0x21;

ed5a:   
	return (char)0x70;

ed5d:   
	if (getTBit())
		goto edb7;
	if (getTBit())
		goto ed96;
	if (getTBit())
		goto ed89;
	if (getTBit())
		goto ed86;
	if (getTBit())
		goto ed83;
   
	return (char)0x4d;

ed83:   
	return (char)0x62;

ed86:   
	return (char)0x3f;

ed89:   
	if (getTBit())
		goto ed93;
   
	return (char)0x76;

ed93:   
	return (char)0x66;

ed96:   
	if (getTBit())
		goto edaa;
	if (getTBit())
		goto eda7;
   
	return (char)0x50;

eda7:   
	return (char)0x2c;

edaa:   
	if (getTBit())
		goto edb4;
   
	return (char)0x77;

edb4:   
	return (char)0x47;

edb7:   
	if (getTBit())
		goto edfd;
	if (getTBit())
		goto eddc;
	if (getTBit())
		goto edd9;
	if (getTBit())
		goto edd6;
   
	return (char)0x42;

edd6:   
	return (char)0x57;

edd9:   
	return (char)0x6b;

eddc:   
	if (getTBit())
		goto edf0;
	if (getTBit())
		goto eded;
   
	return (char)0x46;

eded:   
	return (char)0x56;

edf0:   
	if (getTBit())
		goto edfa;
   
	return (char)0x59;

edfa:   
	return (char)0x4b;

edfd:   
	if (getTBit())
		goto ee39;
	if (getTBit())
		goto ee22;
	if (getTBit())
		goto ee1f;
	if (getTBit())
		goto ee1c;
   
	return (char)0x2d;

ee1c:   
	return (char)0x7a;

ee1f:   
	return (char)0x4a;

ee22:   
	if (getTBit())
		goto ee36;
	if (getTBit())
		goto ee33;
   
	return (char)0x71;

ee33:   
	return (char)0x2b;

ee36:   
	return (char)0x6a;

ee39:   
	if (getTBit())
		goto ee93;
	if (getTBit())
		goto ee68;
	if (getTBit())
		goto ee5b;
	if (getTBit())
		goto ee58;
   
	return (char)0x5a;

ee58:   
	return (char)0x23;

ee5b:   
	if (getTBit())
		goto ee65;
   
	return (char)0x51;

ee65:   
	return (char)0x78;

ee68:   
	if (getTBit())
		goto ee86;
	if (getTBit())
		goto ee83;
	if (getTBit())
		goto ee80;
   
	return (char)0x3a;

ee80:   
	return (char)0x29;

ee83:   
	return (char)0x28;

ee86:   
	if (getTBit())
		goto ee90;
   
	return (char)0x3c;

ee90:   
	return (char)0x58;

ee93:   
	if (getTBit())
		goto eee3;
	if (getTBit())
		goto eec2;
	if (getTBit())
		goto eeb5;
	if (getTBit())
		goto eeb2;
   
	return (char)0x2a;

eeb2:   
	return (char)0x60;

eeb5:   
	if (getTBit())
		goto eebf;
   
	return (char)0x7d;

eebf:   
	return (char)0x3d;

eec2:   
	if (getTBit())
		goto eed6;
	if (getTBit())
		goto eed3;
   
	return (char)0x32;

eed3:   
	return (char)0x30;

eed6:   
	if (getTBit())
		goto eee0;
   
	return (char)0x5d;

eee0:   
	return (char)0x31;

eee3:   
	if (getTBit())
		goto ef1f;
	if (getTBit())
		goto ef08;
	if (getTBit())
		goto ef05;
	if (getTBit())
		goto ef02;
   
	return (char)0x7e;

ef02:   
	return (char)0x33;

ef05:   
	return (char)0x7f;

ef08:   
	if (getTBit())
		goto ef1c;
	if (getTBit())
		goto ef19;
   
	return (char)0x39;

ef19:   
	return (char)0x34;

ef1c:   
	return (char)0x2f;

ef1f:   
	if (getTBit())
		goto ef5b;
	if (getTBit())
		goto ef44;
	if (getTBit())
		goto ef41;
	if (getTBit())
		goto ef3e;
   
	return (char)0x38;

ef3e:   
	return (char)0x5e;

ef41:   
	return (char)0x26;

ef44:   
	if (getTBit())
		goto ef58;
	if (getTBit())
		goto ef55;
   
	return (char)0x35;

ef55:   
	return (char)0x36;

ef58:   
	return (char)0x3e;

ef5b:   
	if (getTBit())
		goto ef8d;
	if (getTBit())
		goto ef80;
	if (getTBit())
		goto ef7d;
	if (getTBit())
		goto ef7a;
   
	return (char)0x40;

ef7a:   
	return (char)0x37;

ef7d:   
	return (char)0x5f;

ef80:   
	if (getTBit())
		goto ef8a;
   
	return (char)0x5c;

ef8a:   
	return (char)0x24;

ef8d:   
	if (getTBit())
		goto efb5;
	if (getTBit())
		goto efa8;
	if (getTBit())
		goto efa5;
   
	return (char)0x5b;

efa5:   
	return (char)0x80;

efa8:   
	if (getTBit())
		goto efb2;
   
	return (char)0x81;

efb2:   
	return (char)0x22;

efb5:   
	if (getTBit())
		goto efc9;
	if (getTBit())
		goto efc6;
   
	return (char)0x25;

efc6:   
	return (char)0x82;

efc9:   
	if (getTBit())
		goto efd3;
   
	return (char)0x7b;

efd3:   
	if (getTBit())
		goto efdd;
   
	return (char)0x9;

efdd:   
	return (char)0x7c;

}

char SkyText_v00331::getTextChar() {

	//This code is based on disassembly of the v0.0331 binary.
	
	if (getTBit())
		goto f3db;
	if (getTBit())
		goto f39c;
	if (getTBit())
		goto f38f;

	return (char)0x20;

f38f:
	if (getTBit())
		goto f399;

	return (char)0x65;

f399:
	return (char)0x61;

f39c:
	if (getTBit())
		goto f3c4;
	if (getTBit())
		goto f3b7;
	if (getTBit())
		goto f3b4;

	return (char)0x6f;

f3b4:  
	return (char)0x73;

f3b7:  
	if (getTBit())
		goto f3c1;
  
	return (char)0x74;

f3c1:  
	return (char)0x6e;

f3c4:  
	if (getTBit())
		goto f3d8;
	if (getTBit())
		goto f3d5;
  
	return (char)0x2e;

f3d5:  
	return (char)0x69;

f3d8:  
	return (char)0x72;

f3db:  
	if (getTBit())
		goto f453;
	if (getTBit())
		goto f414;
	if (getTBit())
		goto f407;
	if (getTBit())
		goto f404;
	if (getTBit())
		goto f401;
  
	return (char)0x0;

f401:  
	return (char)0x45;

f404:  
	return (char)0x75;

f407:  
	if (getTBit())
		goto f411;
  
	return (char)0x6d;

f411:  
	return (char)0x41;

f414:  
	if (getTBit())
		goto f43c;
	if (getTBit())
		goto f42f;
	if (getTBit())
		goto f42c;
  
	return (char)0x6c;

f42c:  
	return (char)0x49;

f42f:
	if (getTBit())
		goto f439;
  
	return (char)0x64;

f439:  
	return (char)0x52;

f43c:  
	if (getTBit())
		goto f450;
	if (getTBit())
		goto f44d;
  
	return (char)0x4e;

f44d:  
	return (char)0x53;

f450:  
	return (char)0x54;

f453:  
	if (getTBit())
		goto f4cb;
	if (getTBit())
		goto f48c;
	if (getTBit())
		goto f47f;
	if (getTBit())
		goto f47c;
	if (getTBit())
		goto f479;
  
	return (char)0x4f;

f479:  
	return (char)0x68;

f47c:  
	return (char)0x63;

f47f:  
	if (getTBit())
		goto f489;
  
	return (char)0x44;

f489:  
	return (char)0x67;

f48c:  
	if (getTBit())
		goto f4b4;
	if (getTBit())
		goto f4a7;
	if (getTBit())
	goto f4a4;
  
	return (char)0x4c;

f4a4:  
	return (char)0x43;

f4a7:  
	if (getTBit())
		goto f4b1;
  
	return (char)0x70;

f4b1:  
	return (char)0x55;

f4b4:  
	if (getTBit())
		goto f4c8;
	if (getTBit())
		goto f4c5;
  
	return (char)0x21;

f4c5:  
	return (char)0x79;

f4c8:  
	return (char)0x4d;

f4cb:  
	if (getTBit())
		goto f52f;
	if (getTBit())
		goto f504;
	if (getTBit())
		goto f4f7;
	if (getTBit())
		goto f4f4;
	if (getTBit())
		goto f4f1;
  
	return (char)0x50;

f4f1:  
	return (char)0x76;

f4f4:  
	return (char)0x48;

f4f7:  
	if (getTBit())
		goto f501;
  
	return (char)0x3f;

f501:  
	return (char)0x62;

f504:  
	if (getTBit())
		goto f522;
	if (getTBit())
		goto f51f;
	if (getTBit())
		goto f51c;
  
	return (char)0x27;

f51c:  
	return (char)0x66;

f51f:  
	return (char)0x2c;

f522:  
	if (getTBit())
		goto f52c;
  
	return (char)0x47;

f52c:  
	return (char)0x42;

f52f:  
	if (getTBit())
		goto f593;
	if (getTBit())
		goto f568;
	if (getTBit())
		goto f55b;
	if (getTBit())
		goto f558;
	if (getTBit())
		goto f555;
  
	return (char)0x56;

f555:  
	return (char)0x6b;

f558:  
	return (char)0x46;

f55b:  
	if (getTBit())
		goto f565;
  
	return (char)0x71;

f565:  
	return (char)0x2a;

f568:
	if (getTBit())
		goto f586;
	if (getTBit())
		goto f583;
	if (getTBit())
		goto f580;
  
	return (char)0x77;

f580:  
	return (char)0x4b;

f583:  
	return (char)0x2d;

f586:  
	if (getTBit())
		goto f590;
  
	return (char)0x57;

f590:  
	return (char)0x4a;

f593:  
	if (getTBit())
		goto f5ed;
	if (getTBit())
		goto f5c2;
	if (getTBit())
		goto f5b5;
	if (getTBit())
		goto f5b2;
  
	return (char)0x7a;

f5b2:  
	return (char)0x2b;

f5b5:  
	if (getTBit())
		goto f5bf;
  
	return (char)0x59;

f5bf:  
	return (char)0x6a;

f5c2:  
	if (getTBit())
		goto f5e0;
	if (getTBit())
		goto f5dd;
	if (getTBit())
		goto f5da;
  
	return (char)0x85;

f5da:  
	return (char)0x29;

f5dd:  
	return (char)0x51;

f5e0:  
	if (getTBit())
		goto f5ea;
  
	return (char)0x5a;

f5ea:  
	return (char)0x7e;

f5ed:  
	if (getTBit())
		goto f65b;
	if (getTBit())
		goto f626;
	if (getTBit())
		goto f619;
	if (getTBit())
		goto f616;
	if (getTBit())
		goto f613;
  
	return (char)0x8b;

f613:  
	return (char)0x3c;

f616:  
	return (char)0x8a;

f619:  
	if (getTBit())
		goto f623;
  
	return (char)0x7f;

f623:  
	return (char)0x3a;

f626:  
	if (getTBit())
		goto f644;
	if (getTBit())
		goto f641;
	if (getTBit())
		goto f63e;
  
	return (char)0x87;

f63e:  
	return (char)0x23;

f641:  
	return (char)0x78;

f644:  
	if (getTBit())
		goto f658;
	if (getTBit())
		goto f655;
  
	return (char)0x58;

f655:  
	return (char)0x91;

f658:  
	return (char)0x83;

f65b:  
	if (getTBit())
		goto f6bf;
	if (getTBit())
		goto f694;
	if (getTBit())
		goto f687;
	if (getTBit())
		goto f684;
	if (getTBit())
		goto f681;
  
	return (char)0x88;

f681:  
	return (char)0x60;

f684:  
	return (char)0x32;

f687:  
	if (getTBit())
		goto f691;
  
	return (char)0x30;

f691:  
	return (char)0x31;

f694:  
	if (getTBit())
		goto f6b2;
	if (getTBit())
		goto f6af;
	if (getTBit())
		goto f6ac;
  
	return (char)0x28;

f6ac:  
	return (char)0x2f;

f6af:  
	return (char)0x5d;

f6b2:  
	if (getTBit())
		goto f6bc;
  
	return (char)0x3d;

f6bc:  
	return (char)0x86;

f6bf:  
	if (getTBit())
		goto f70f;
	if (getTBit())
		goto f6ee;
	if (getTBit())
		goto f6e1;
	if (getTBit())
		goto f6de;
  
	return (char)0x5e;

f6de:  
	return (char)0x33;

f6e1:  
	if (getTBit())
		goto f6eb;
  
	return (char)0x39;

f6eb:  
	return (char)0x34;

f6ee:  
	if (getTBit())
		goto f702;
	if (getTBit())
		goto f6ff;
  
	return (char)0x7d;

f6ff:  
	return (char)0x38;

f702:  
	if (getTBit())
		goto f70c;
  
	return (char)0x5c;

f70c:  
	return (char)0x22;

f70f:  
	if (getTBit())
		goto f755;
	if (getTBit())
		goto f73e;
	if (getTBit())
		goto f731;
	if (getTBit())
		goto f72e;
  
	return (char)0x3e;

f72e:  
	return (char)0x26;

f731:  
	if (getTBit())
		goto f73b;
  
	return (char)0x8d;

f73b:  
	return (char)0x7b;

f73e:  
	if (getTBit())
		goto f752;
	if (getTBit())
		goto f74f;
  
	return (char)0x35;

f74f:  
	return (char)0x36;

f752:  
	return (char)0x8f;

f755:  
	if (getTBit())
		goto f791;
	if (getTBit())
		goto f77a;
	if (getTBit())
		goto f777;
	if (getTBit())
		goto f774;
  
	return (char)0x8e;

f774:  
	return (char)0x8c;

f777:  
	return (char)0x37;

f77a:  
	if (getTBit())
		goto f78e;
	if (getTBit())
		goto f78b;
  
	return (char)0x89;

f78b:  
	return (char)0x24;

f78e:  
	return (char)0x92;

f791:  
	if (getTBit())
		goto f7b9;
	if (getTBit())
		goto f7ac;
	if (getTBit())
		goto f7a9;
  
	return (char)0x5b;

f7a9:  
	return (char)0x80;

f7ac:  
	if (getTBit())
		goto f7b6;
  
	return (char)0x81;

f7b6:  
	return (char)0x40;

f7b9:  
	if (getTBit())
		goto f7cd;
	if (getTBit())
		goto f7ca;
  
	return (char)0x5f;

f7ca:  
	return (char)0x82;

f7cd:  
	if (getTBit())
		goto f7d7;
  
	return (char)0x25;

f7d7:  
	if (getTBit())
		goto f7e1;

	return (char)0x9;

f7e1:  
	if (getTBit())
		goto f7eb;

	return (char)0x3b;

f7eb:  
	return (char)0x7c;

}

char SkyText_v00372::getTextChar() {

	//This code is based on the original sources
	//Should work for all (known) cd versions.

	if (getTBit())
		goto label_1;
	if (getTBit())
		goto label_2;
	if (getTBit())
		goto label_3;

	return ' ';

label_3:
	if (getTBit())
		goto label_4;

	return 'e';

label_4:
	return 'a';

label_2:
	if (getTBit())
		goto label_5;
	if (getTBit())
		goto label_6;
	if (getTBit())
		goto label_7;

	return 'o';

label_7:
	return 's';

label_6:
	if (getTBit())
		goto label_8;
	
	return 't';

label_8:
	return 'n';

label_5:
	if (getTBit())
		goto label_9;
	if (getTBit())
		goto label_10;

	return '.';

label_10:
	return 'i';

label_9:
	return 'r';

label_1:
	if (getTBit())
		goto label_11;
	if (getTBit())
		goto label_12;
	if (getTBit())
		goto label_13;
	if (getTBit())
		goto label_14;
	if (getTBit())
		goto label_15;

	return (char)0; //end of line

label_15:
	return 'E';

label_14:
	return 'u';

label_13:
	if (getTBit())
		goto label_16;

	return 'm';

label_16:
	return 'A';

label_12:
	if (getTBit())
		goto label_17;
	if (getTBit())
		goto label_18;
	if (getTBit())
		goto label_19;
	
	return 'l';

label_19:
	return 'I';

label_18:
	if (getTBit())
		goto label_20;

	return 'd';

label_20:
	return 'R';

label_17:
	if (getTBit())
		goto label_21;
	if (getTBit())
		goto label_22;

	return 'N';

label_22:
	return 'S';

label_21:
	return 'T';

label_11:
	if (getTBit())
		goto label_23;
	if (getTBit())
		goto label_24;
	if (getTBit())
		goto label_25;
	if (getTBit())
		goto label_26;
	if (getTBit())
		goto label_27;

	return 'O';

label_27:
	return 'h';

label_26:
	return 'c';

label_25:
	if (getTBit())
		goto label_28;

	return 'D';

label_28:
	return 'g';

label_24:
	if (getTBit())
		goto label_29;
	if (getTBit())
		goto label_30;
	if (getTBit())
		goto label_31;

	return 'L';

label_31:
	return 'C';

label_30:
	if (getTBit())
		goto label_32;
	
	return 'p';

label_32:
	return 'U';

label_29:
	if (getTBit())
		goto label_33;
	if (getTBit())
		goto label_34;

	return '!';

label_34:
	return 'y';

label_33:
	return 'M';

label_23:
	if (getTBit())
		goto label_35;
	if (getTBit())
		goto label_36;
	if (getTBit())
		goto label_37;
	if (getTBit())
		goto label_38;
	if (getTBit())
		goto label_39;

	return 'P';

label_39:
	return 'v';

label_38:
	return 'H';

label_37:
	if (getTBit())
		goto label_40;
	
	return '?';

label_40:
	return 'b';

label_36:
	if (getTBit())
		goto label_41;
	if (getTBit())
		goto label_42;
	if (getTBit())
		goto label_43;

	return (char)39;

label_43:
	return 'f';

label_42:
	return ',';

label_41:
	if (getTBit())
		goto label_44;

	return 'G';

label_44:
	return 'B';

label_35:
	if (getTBit())
		goto label_45;
	if (getTBit())
		goto label_46;
	if (getTBit())
		goto label_47;
	if (getTBit())
		goto label_48;
	if (getTBit())
		goto label_49;

	return 'V';

label_49:
	return 'k';

label_48:
	return 'F';

label_47:
	if (getTBit())
		goto label_50;
	
	return 'q';

label_50:

	return 'w';

label_46:
	if (getTBit())
		goto label_51;
	if (getTBit())
		goto label_52;
	if (getTBit())
		goto label_53;

	return 'K';

label_53:
	return '-';

label_52:
	return 'W';

label_51:
	if (getTBit())
		goto label_54;

	return 'J';

label_54:
	return '*';

label_45:
	if (getTBit())
		goto label_55;
	if (getTBit())
		goto label_56;
	if (getTBit())
		goto label_57;
	if (getTBit())
		goto label_58;

	return 'z';

label_58:
	return 'Y';

label_57:
	if (getTBit())
		goto label_59;
	
	return 'j';

label_59:
	return '+';

label_56:
	if (getTBit())
		goto label_60;
	if (getTBit())
		goto label_61;
	if (getTBit())
		goto label_62;

	return 'Q';

label_62:
	return (char)133;

label_61:
	return ')';

label_60:
	if (getTBit())
		goto label_63;
	if (getTBit())
		goto label_64;

	return 'Z';

label_64:
	return (char)139;

label_63:
	return '<';

label_55:
	if (getTBit())
		goto label_65;
	if (getTBit())
		goto label_66;
	if (getTBit())
		goto label_67;
	if (getTBit())
		goto label_68;
	if (getTBit())
		goto label_69;

	return (char)149;

label_69:
	return (char)126;

label_68:
	return (char)138;

label_67:
	if (getTBit())
		goto label_70;

	return (char)135;

label_70:
	return ':';

label_66:

	if (getTBit())
		goto label_71;
	if (getTBit())
		goto label_72;
	if (getTBit())
		goto label_73;

	return (char)127;

label_73:
	return ']';

label_72:
	if (_gameVersion == 372)
		return '#';
	else 
		return 'x';
	
label_71:
	if (getTBit())
		goto label_74;
	if (getTBit())
		goto label_75;

	if (_gameVersion == 368)
		return 'X';
	else
		return 'x';
	
label_75:
	return 'X';

label_74:
	return (char)145;

label_65:
	if (getTBit())
		goto label_76;
	if (getTBit())
		goto label_77;
	if (getTBit())
		goto label_78;
	if (getTBit())
		goto label_79;
	if (getTBit())
		goto label_80;

	return (char)136;

label_80:
	return '`';

label_79:
	return '2';

label_78:
	if (getTBit())
		goto label_81;
	if (getTBit())
		goto label_82;

	return '0';

label_82:
	return (char)131;

label_81:
	return '1';

label_77:
	if (getTBit())
		goto label_83;
	if (getTBit())
		goto label_84;
	if (getTBit())
		goto label_85;

	return '/';

label_85:
	return '(';

label_84:
	return '=';

label_83:
	if (getTBit())
		goto label_86;

	return (char)134;

label_86:
	return '^';

label_76:
	if (getTBit())
		goto label_87;
	if (getTBit())
		goto label_88;
	if (getTBit())
		goto label_89;
	if (getTBit())
		goto label_90;	

	return '3';

label_90:
	return '9';

label_89:
	if (getTBit())
		goto label_91;

	return (char)152;

label_91:
	return '4';

label_88:
	if (getTBit())
		goto label_92;
	if (getTBit())
		goto label_93;
	if (getTBit())
		goto label_94;

	return '}';

label_94:
	return '8';

label_93:
	return '\\';	//needs to be escaped

label_92:
	if (getTBit())
		goto label_95;

	return '"';

label_95:
	return (char)144;

label_87:
	if (getTBit())
		goto label_96;
	if (getTBit())
		goto label_97;
	if (getTBit())
		goto label_98;
	if (getTBit())
		goto label_99;


	return '&';

label_99:
	return (char)141;

label_98:
	if (getTBit())
		goto label_100;

	return '5';

label_100:
	return '6';

label_97:
	if (getTBit())
		goto label_101;
	if (getTBit())
		goto label_102;

	return (char)146;

label_102:
	return (char)143;

label_101:
	if (getTBit())
		goto label_103;

	return (char)142;

label_103:
	return (char)147;

label_96:
	if (getTBit())
		goto label_104;
	if (getTBit())
		goto label_105;
	if (getTBit())
		goto label_106;
	if (getTBit())
		goto label_107;

	return (char)140;

label_107:
	return '7';

label_106:
	return (char)128;

label_105:
	if (getTBit())
		goto label_108;
	if (getTBit())
		goto label_109;

	return (char)129;

label_109:

	return (char)153;

label_108:
	if (getTBit())
		goto label_110;

	return '$';

label_110:
	return '@';

label_104:
	if (getTBit())
		goto label_111;
	if (getTBit())
		goto label_112;
	if (getTBit())
		goto label_113;
	if (getTBit())
		goto label_114;	

	return '[';

label_114:
	return (char)154;

label_113:
	return '_';

label_112:
	if (getTBit())
		goto label_115;
	return '>';

label_115:
	return (char)150;

label_111:
	if (getTBit())
		goto label_116;
	if (getTBit())
		goto label_117;
	if (getTBit())
		goto label_118;

	return (char)130;

label_118:
	return '%';

label_117: 
	return (char)9;

label_116:
	if (getTBit())
		goto label_119;
	if (getTBit())
		goto label_120;

	return (char)156;

label_120:
	return (char)151;

label_119:
	if (getTBit())
		goto label_121;

	return '{';

label_121:
	if (getTBit())
		goto label_122;

	return (char)148;

label_122:
	return '|';


	return '!'; //should never be reached
}

