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

#define FIRST_TEXT_SEC	77
#define NO_OF_TEXT_SECTIONS	8	// 8 sections per language
#define	CHAR_SET_FILE	60150
#define MAX_SPEECH_SECTION	7 

SkyText::SkyText(SkyDisk *skyDisk, uint32 gameVersion) {
	_skyDisk = skyDisk;
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
	_charHeight = newCharSet->charHeight;
	_dtCharSpacing = newCharSet->charSpacing;
}

void SkyText::getText(uint32 textNr, void **itemList, uint16 language) { //load text #"textNr" into textBuffer
	uint32 sectionNo = (textNr & 0x0F000) >> 10;
	
	if (itemList[FIRST_TEXT_SEC + sectionNo] == (void*)NULL) { //check if already loaded
		debug(5, "Loading Text item(s) for Section %d", (sectionNo>>2));
		
		uint32 fileNo = (sectionNo >> 2); 
		fileNo += ((language * NO_OF_TEXT_SECTIONS) + 60600);
		itemList[FIRST_TEXT_SEC + sectionNo] = (void *)_skyDisk->loadFile((uint16)fileNo, NULL);
	}
	_textItemPtr = (uint8 *)itemList[FIRST_TEXT_SEC + sectionNo];
	
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

