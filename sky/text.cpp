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

#define FIRST_TEXT_SEC	77
#define NO_OF_TEXT_SECTIONS	8	// 8 sections per language
#define	CHAR_SET_FILE	60150
#define	SET_FONT_DATA_SIZE	12
#define MAX_SPEECH_SECTION	7 

void SkyState::initialiseText(void) {

	_mainCharacterSet.addr = (byte *)loadFile(CHAR_SET_FILE, NULL);
	fnSetFont(0);
	
	if (!_isDemo) {
		_controlCharacterSet.addr = loadFile(60520, NULL);
		_linkCharacterSet.addr = loadFile(60521, NULL);
	}

	if (_isCDVersion) {
		_preAfterTableArea = loadFile(60522, NULL);
	}
}

void SkyState::setupVersionSpecifics(uint32 version) {

	switch (version) {

	case 267:
		//floppy demo
		getTextChar = &SkyState::getTextChar_v00267;
		break;
	
	case 288:
		//floppy - old version
		getTextChar = &SkyState::getTextChar_v00288;
		break;
	
	case 331:
		//floppy - new version
		//getTextChar = getTextChar_v00331;
		break;
		
	case 365:
		//cd demo, uses a slightly modified version of v00368
	case 368:
		//cd version
		getTextChar = &SkyState::getTextChar_v00368;
		break;

	default:
		error("Unknown game version!");
		break;
	}

}

void SkyState::fnSetFont(uint32 fontNr) { 

	_curCharSet = fontNr;
	byte *charSetPtr = _mainCharacterSet.addr + (fontNr * SET_FONT_DATA_SIZE);		
	_characterSet = READ_LE_UINT32(charSetPtr);
	_charHeight = READ_LE_UINT32(charSetPtr + 4);
	_dtCharSpacing = READ_LE_UINT32(charSetPtr + 8);

}

void SkyState::getText(uint32 textNr) { //load text #"textNr" to _textBuffer

	uint32 sectionNo = (textNr & 0x0F000) >> 10;
	
	if (_itemList[FIRST_TEXT_SEC + sectionNo] == (void*)NULL) { //check if already loaded
		debug(5, "Loading Text item(s) for Section %d", (sectionNo>>2));
		
		uint32 fileNo = (sectionNo >> 2); 
		fileNo += ((_language * NO_OF_TEXT_SECTIONS) + 60600);
		_itemList[FIRST_TEXT_SEC + sectionNo] = (void *)loadFile(fileNo, NULL);
	}
	uint8 *textItemPtr = (uint8 *)_itemList[FIRST_TEXT_SEC + sectionNo];
	
	uint32 offset = 0; 
	uint32 nr32MsgBlocks = (textNr & 0x0fe0);
	uint32 skipBytes; 
	byte *blockPtr;
	bool bitSeven; 
	
	if (nr32MsgBlocks) { 
		blockPtr = (byte *)(textItemPtr + 4);
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
		remItems += READ_LE_UINT16(textItemPtr);
		blockPtr = textItemPtr + remItems; 
		
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
	offset += READ_LE_UINT16(textItemPtr + 2); 
	textItemPtr += offset;

	//bit pointer: 0->8, 1->6, 2->4 ...
	numBits &= 3;
	numBits ^= 3; 
	numBits++;
	numBits <<= 1;

	uint8 inputValue = *textItemPtr++;
	char *dest = (char *)_textBuffer;
	char textChar;
	uint8 shiftBits = (uint8) numBits; 

	do {
		textChar = (this->*getTextChar)(&inputValue, &shiftBits, textItemPtr);
		*dest++ = textChar;	
	} while(textChar);
}

char SkyState::getTextChar_v00267(uint8 *inputValue, uint8 *shiftBits, uint8 *&inputStream) {

	//NOTE: The original sources only contained a text decoder that, AFAIK, is not used in 
	//any commercially sold version, but differs only slightly from the CD (and CD Demo) implementations.
	//The decoding code for the floppy versions (both demo and full version) is _completely_ different.
	//This code is based on disassembly of the v0.0267 binary.
	

	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab73;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab34;

	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab31;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab2e;

	return (char)0x20;

ab2e:  
	return (char)0x65;

ab31:  
	return (char)0x73;

ab34:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab5c;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab4f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab4c;

	return (char)0x6d;

ab4c:  
	return (char)0x61;

ab4f:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab59;

	return (char)0x0;

ab59:  
	return (char)0x6e;

ab5c:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab70;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab6d;
	
	return (char)0x69;

ab6d:  
	return (char)0x74;

ab70:  
	return (char)0x75;

ab73:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abeb;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abac;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab9f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab9c;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ab99;
	
	return (char)0x2e;

ab99:  
	return (char)0x72;

ab9c:  
	return (char)0x45;

ab9f:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aba9;
	
	return (char)0x6f;

aba9:  
	return (char)0x68;

abac:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abd4;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abc7;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abc4;

	return (char)0x44;

abc4:  
	return (char)0x67;

abc7:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abd1;
	
	return (char)0x49;

abd1:  
	return (char)0x79;

abd4:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abe8;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto abe5;
	
	return (char)0x53;

abe5:  
	return (char)0x64;

abe8:  
	return (char)0x6c;

abeb:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac4f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac1a;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac0d;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac0a;
       
	return (char)0x54;

ac0a:  
	return (char)0x4e;

ac0d:  
       if (getTBit(inputValue, shiftBits, inputStream))
		goto ac17;

	return (char)0x63;

ac17:  
	return (char)0x52;

ac1a:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac38;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac35;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac32;
	
	return (char)0x41;

ac32:  
	return (char)0x4f;

ac35:  
	return (char)0x4c;

ac38:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac4c;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac49;
	
	return (char)0x48;

ac49:  
	return (char)0x43;

ac4c:  
	return (char)0x55;

ac4f:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acb3;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac88;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac7b;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac78;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac75;
	
	return (char)0x62;

ac75:  
	return (char)0x21;

ac78:  
	return (char)0x27;

ac7b:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ac85;
	
	return (char)0x77;

ac85:  
	return (char)0x66;

ac88:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aca6;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aca3;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aca0;
	
	return (char)0x47;

aca0:  
	return (char)0x4d;

aca3:  
	return (char)0x2c;

aca6:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acb0;
	
	return (char)0x57;

acb0:  
	return (char)0x42;

acb3:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acf9;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ace2;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acd5;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acd2;
	
	return (char)0x3f;

acd2:  
	return (char)0x50;

acd5:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acdf;
	
	return (char)0x6b;

acdf:  
	return (char)0x70;

ace2:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acf6;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto acf3;
	
	return (char)0x76;

acf3:  
	return (char)0x46;

acf6:  
	return (char)0x4b;

acf9:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad35;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad1e;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad1b;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad18;
	
	return (char)0x56;

ad18:  
	return (char)0x59;

ad1b:  
	return (char)0x7a;

ad1e:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad32;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad2f;
	
	return (char)0x2d;

ad2f:  
	return (char)0x23;

ad32:  
	return (char)0x4a;

ad35:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad7b;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad5a;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad57;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad54;
	
	return (char)0x5a;

ad54:  
	return (char)0x28;

ad57:  
	return (char)0x29;

ad5a:  
	if (getTBit(inputValue, shiftBits, inputStream)) 
		goto ad6e;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad6b;
	
	return (char)0x6a;

ad6b:  
	return (char)0x2b;

ad6e:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad78;
	
	return (char)0x2a;

ad78:  
	return (char)0x71;

ad7b:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adc1;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ada0;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad9d;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ad9a;
	
	return (char)0x7d;

ad9a:  
	return (char)0x3a;

ad9d:  
	return (char)0x60;

ada0:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adb4;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adb1;
	
	return (char)0x30;

adb1:  
	return (char)0x32;

adb4:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adbe;
	
	return (char)0x31;

adbe:  
	return (char)0x51;

adc1:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adfd;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ade6;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ade3;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ade0;
	
	return (char)0x58;

ade0:  
	return (char)0x78;

ade3:  
	return (char)0x33;

ade6:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adfa;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto adf7;
	
	return (char)0x39;

adf7:  
	return (char)0x34;

adfa:  
	return (char)0x3c;

adfd:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae2f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae22;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae1f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae1c;
	
	return (char)0x38;

ae1c:  
	return (char)0x5d;

ae1f:  
	return (char)0x3d;

ae22:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae2c;
	
	return (char)0x26;

ae2c:  
	return (char)0x35;

ae2f:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae57;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae4a;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae47;
	
	return (char)0x36;

ae47:  
	return (char)0x2f;

ae4a:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae54;
	
	return (char)0x37;

ae54:  
	return (char)0x24;

ae57:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae75;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae72;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae6f;
	
	return (char)0x3e;

ae6f:  
	return (char)0x25;

ae72:  
	return (char)0x5e;

ae75:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae93;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae90;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto ae8d;
	
	return (char)0x22;

ae8d:  
	return (char)0x40;

ae90:  
	return (char)0x5b;

ae93:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aea7;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aea4;
	
	return (char)0x5f;

aea4:  
	return (char)0x7b;

aea7:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto aeb1;
	
	return (char)0x5c;

aeb1:  
	return (char)0x7c;

}

char SkyState::getTextChar_v00288(uint8 *inputValue, uint8 *shiftBits, uint8 *&inputStream) {

	//This code is based on disassembly of the v0.0288 binary.

	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5b3;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e574;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e567;

	return (char)0x20;

e567:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e571;

	return (char)0x65;

e571:
	return (char)0x73;

e574:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e59c;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e58f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e58c;

	return (char)0x6e;

e58c:
	return (char)0x69;

e58f:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e599;

	return (char)0x74;

e599:
	return (char)0x61;

e59c:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5b0;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5ad;

	return (char)0x2e;

e5ad:
	return (char)0x0;

e5b0:
	return (char)0x45;

e5b3:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e62b;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5ec;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5df;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5dc;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5d9;

	return (char)0x72;

e5d9:
	return (char)0x6f;

e5dc:
	return (char)0x75;

e5df:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e5e9;

	return (char)0x6d;

e5e9:
	return (char)0x68;

e5ec:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e614;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e607;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e604;
	
	return (char)0x49;

e604:
	return (char)0x6c;

e607:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e611;
	
	return (char)0x53;

e611:
	return (char)0x52;

e614:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e628;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e625;

	return (char)0x64;

e625:
	return (char)0x4e;

e628:
	return (char)0x54;

e62b:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6a3;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e664;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e657;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e654;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e651;
	
	return (char)0x63;

e651:  
	return (char)0x41;

e654:  
	return (char)0x4f;

e657:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e661;
	
	return (char)0x44;

e661:  
	return (char)0x4c;

e664:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e68c;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e67f;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e67c;

	return (char)0x43;

e67c:  
	return (char)0x67;

e67f:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e689;
	
	return (char)0x55;

e689:  
	return (char)0x27;

e68c:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6a0;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e69d;
	
	return (char)0x79;

e69d:  
	return (char)0x48;

e6a0:  
	return (char)0x70;

e6a3:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e707;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6dc;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6cf;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6cc;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6c9;
	
	return (char)0x21;

e6c9:  
	return (char)0x4d;

e6cc:  
	return (char)0x62;

e6cf:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6d9;
	
	return (char)0x76;

e6d9:  
	return (char)0x2c;

e6dc:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6fa;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6f7;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e6f4;
	
	return (char)0x50;

e6f4:  
	return (char)0x66;

e6f7:  
	return (char)0x3f;

e6fa:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e704;
	
	return (char)0x47;

e704:  
	return (char)0x42;

e707:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e757;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e736;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e729;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e726;
	
	return (char)0x77;

e726:  
	return (char)0x57;

e729:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e733;
	
	return (char)0x46;

e733:  
	return (char)0x56;

e736:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e74a;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e747;
	
	return (char)0x6b;

e747:  
	return (char)0x7a;

e74a:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e754;
	
	return (char)0x4b;

e754:  
	return (char)0x2d;

e757:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e793;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e77c;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e779;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e776;
	
	return (char)0x4a;

e776:  
	return (char)0x2b;

e779:  
	return (char)0x71;

e77c:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e790;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e78d;
	
	return (char)0x59;

e78d:  
	return (char)0x6a;

e790:  
	return (char)0x5a;

e793:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7e3;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7c2;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7b5;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7b2;
	
	return (char)0x23;

e7b2:  
	return (char)0x51;

e7b5:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7bf;
	
	return (char)0x28;

e7bf:  
	return (char)0x29;

e7c2:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7d6;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7d3;
	
	return (char)0x3c;

e7d3:  
	return (char)0x78;

e7d6:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e7e0;
	
	return (char)0x3a;

e7e0:  
	return (char)0x2a;

e7e3:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e829;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e812;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e805;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e802;
	
	return (char)0x58;

e802:  
	return (char)0x7d;

e805:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e80f;
	
	return (char)0x3d;

e80f:  
	return (char)0x60;

e812:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e826;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e823;
	
	return (char)0x5d;

e823:  
	return (char)0x30;

e826:  
	return (char)0x32;

e829:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e865;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e84e;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e84b;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e848;
	
	return (char)0x31;

e848:  
	return (char)0x33;

e84b:  
	return (char)0x39;

e84e:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e862;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e85f;
	
	return (char)0x2f;

e85f:  
	return (char)0x34;

e862:  
	return (char)0x5e;

e865:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e897;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e88a;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e887;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e884;
	
	return (char)0x38;

e884:  
	return (char)0x3e;

e887:  
	return (char)0x26;

e88a:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e894;
	
	return (char)0x35;

e894:  
	return (char)0x40;

e897:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8bf;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8b2;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8af;
	
	return (char)0x36;

e8af:  
	return (char)0x5f;

e8b2:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8bc;
	
	return (char)0x5c;

e8bc:  
	return (char)0x37;

e8bf:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8dd;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8da;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8d7;
	
	return (char)0x5b;

e8d7:  
	return (char)0x24;

e8da:  
	return (char)0x7b;

e8dd:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8f1;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8ee;
	
	return (char)0x25;

e8ee:  
	return (char)0x9;

e8f1:  
	if (getTBit(inputValue, shiftBits, inputStream))
		goto e8fb;
	
	return (char)0x22;

e8fb:  
	return (char)0x7c;

}

char SkyState::getTextChar_v00368(uint8 *inputValue, uint8 *shiftBits, uint8 *&inputStream) {

	//This code is based on the original sources
	//Slightly modified to work for both the CD and CD-Demo versions.

	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_1;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_2;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_3;

	return ' ';

label_3:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_4;

	return 'e';

label_4:
	return 'a';

label_2:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_5;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_6;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_7;

	return 'o';

label_7:
	return 's';

label_6:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_8;
	
	return 't';

label_8:
	return 'n';

label_5:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_9;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_10;

	return '.';

label_10:
	return 'i';

label_9:
	return 'r';

label_1:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_11;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_12;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_13;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_14;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_15;

	return (char)0; //end of line

label_15:
	return 'E';

label_14:
	return 'u';

label_13:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_16;

	return 'm';

label_16:
	return 'A';

label_12:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_17;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_18;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_19;
	
	return 'l';

label_19:
	return 'I';

label_18:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_20;

	return 'd';

label_20:
	return 'R';

label_17:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_21;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_22;

	return 'N';

label_22:
	return 'S';

label_21:
	return 'T';

label_11:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_23;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_24;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_25;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_26;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_27;

	return 'O';

label_27:
	return 'h';

label_26:
	return 'c';

label_25:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_28;

	return 'D';

label_28:
	return 'g';

label_24:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_29;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_30;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_31;

	return 'L';

label_31:
	return 'C';

label_30:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_32;
	
	return 'p';

label_32:
	return 'U';

label_29:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_33;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_34;

	return '!';

label_34:
	return 'y';

label_33:
	return 'M';

label_23:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_35;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_36;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_37;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_38;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_39;

	return 'P';

label_39:
	return 'v';

label_38:
	return 'H';

label_37:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_40;
	
	return '?';

label_40:
	return 'b';

label_36:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_41;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_42;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_43;

	return (char)39;

label_43:
	return 'f';

label_42:
	return ',';

label_41:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_44;

	return 'G';

label_44:
	return 'B';

label_35:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_45;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_46;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_47;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_48;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_49;

	return 'V';

label_49:
	return 'k';

label_48:
	return 'F';

label_47:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_50;
	
	return 'q';

label_50:

	return 'w';

label_46:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_51;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_52;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_53;

	return 'K';

label_53:
	return '-';

label_52:
	return 'W';

label_51:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_54;

	return 'J';

label_54:
	return '*';

label_45:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_55;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_56;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_57;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_58;

	return 'z';

label_58:
	return 'Y';

label_57:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_59;
	
	return 'j';

label_59:
	return '+';

label_56:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_60;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_61;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_62;

	return 'Q';

label_62:
	return (char)133;

label_61:
	return ')';

label_60:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_63;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_64;

	return 'Z';

label_64:
	return (char)139;

label_63:
	return '<';

label_55:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_65;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_66;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_67;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_68;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_69;

	return (char)149;

label_69:
	return (char)126;

label_68:
	return (char)138;

label_67:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_70;

	return (char)135;

label_70:
	return ':';

label_66:

	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_71;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_72;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_73;

	return (char)127;

label_73:
	return ']';

label_72:
	return 'x';
	
label_71:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_74;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_75;

	if (_gameVersion == 365)
		return 'x';
	else
		return 'X';
	
label_75:
	return 'X';

label_74:
	return (char)145;

label_65:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_76;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_77;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_78;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_79;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_80;

	return (char)136;

label_80:
	return '`';

label_79:
	return '2';

label_78:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_81;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_82;

	return '0';

label_82:
	return (char)131;

label_81:
	return '1';

label_77:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_83;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_84;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_85;

	return '/';

label_85:
	return '(';

label_84:
	return '=';

label_83:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_86;

	return (char)134;

label_86:
	return '^';

label_76:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_87;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_88;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_89;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_90;	

	return '3';

label_90:
	return '9';

label_89:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_91;

	return (char)152;

label_91:
	return '4';

label_88:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_92;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_93;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_94;

	return '}';

label_94:
	return '8';

label_93:
	return '\\';	//needs to be escaped

label_92:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_95;

	return '"';

label_95:
	return (char)144;

label_87:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_96;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_97;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_98;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_99;


	return '&';

label_99:
	return (char)141;

label_98:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_100;

	return '5';

label_100:
	return '6';

label_97:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_101;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_102;

	return (char)146;

label_102:
	return (char)143;

label_101:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_103;

	return (char)142;

label_103:
	return (char)147;

label_96:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_104;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_105;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_106;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_107;

	return (char)140;

label_107:
	return '7';

label_106:
	return (char)128;

label_105:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_108;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_109;

	return (char)129;

label_109:

	return (char)153;

label_108:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_110;

	return '$';

label_110:
	return '@';

label_104:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_111;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_112;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_113;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_114;	

	return '[';

label_114:
	return (char)154;

label_113:
	return '_';

label_112:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_115;
	return '>';

label_115:
	return (char)150;

label_111:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_116;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_117;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_118;

	return (char)130;

label_118:
	return '%';

label_117: 
	return (char)9;

label_116:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_119;
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_120;

	return (char)156;

label_120:
	return (char)151;

label_119:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_121;

	return '{';

label_121:
	if (getTBit(inputValue, shiftBits, inputStream))
		goto label_122;

	return (char)148;

label_122:
	return '|';


	return '!'; //should never be reached
}

bool SkyState::getTBit(uint8 *inputValue, uint8 *shiftBits, byte *&inputStream) {
	
	if (*shiftBits) {
		(*shiftBits)--;
	} else {
		*inputValue = *inputStream++;
		*shiftBits = 7;
	} 
	
	return (bool)(((*inputValue) >> (*shiftBits)) & 1);
}

