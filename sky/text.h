/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef SKYTEXT_H
#define SKYTEXT_H

#include "common/stdafx.h"
#include "common/scummsys.h"

namespace Sky {

struct Compact;
class Disk;
class SkyCompact;

struct HuffTree {
	unsigned char lChild;
	unsigned char rChild;
	unsigned char value;
};

#define NUM_PATCH_MSG 8

struct PatchMessage {
	uint32 textNr;
	char text[100];
};

class Text {
public:
	Text(Disk *skyDisk, SkyCompact *skyCompact);
	~Text(void);
	struct displayText_t displayText(uint32 textNum, uint8 *dest, bool centre, uint16 pixelWidth, uint8 color);
	struct displayText_t displayText(char *textPtr, uint8 *dest, bool centre, uint16 pixelWidth, uint8 color);
	struct lowTextManager_t lowTextManager(uint32 textNum, uint16 width, uint16 logicNum, uint8 color, bool centre);
	void fnSetFont(uint32 fontNr);
	void fnTextModule(uint32 textInfoId, uint32 textNo);
	void fnPointerText(uint32 pointedId, uint16 mouseX, uint16 mouseY);
	void logicCursor(Compact *textCompact, uint16 mouseX, uint16 mouseY);
	void changeTextSpriteColour(uint8 *sprData, uint8 newCol);
	uint32 giveCurrentCharSet(void);
	
	uint32 _numLetters;	//no of chars in message

private:
	void initHuffTree();
	void getText(uint32 textNr);
	char getTextChar(uint8 **data, uint32 *bitPos);
	bool getTextBit(uint8 **data, uint32 *bitPos);
	void makeGameCharacter(uint8 textChar, uint8 *charSetPtr, uint8 *&data, uint8 color, uint16 bufPitch);

	void patchChar(byte *charSetPtr, int width, int height, int c, const uint16 *data);
	void patchLINCCharset();
	bool patchMessage(uint32 textNum);

	Disk *_skyDisk;
	SkyCompact *_skyCompact;

	const HuffTree *_huffTree;

	struct charSet {
		uint8 *addr;
		uint32 charHeight;
		uint32 charSpacing;
	} _mainCharacterSet, _linkCharacterSet, _controlCharacterSet;

	uint32	_curCharSet;
	uint8	*_characterSet;
	uint8	_charHeight;

	char _textBuffer[1024];

	uint32	_dtCharSpacing;	//character separation adjustment
	uint32	_mouseOfsX, _mouseOfsY;
	static const PatchMessage _patchedMessages[NUM_PATCH_MSG];
	static const uint16 _patchLangIdx[8];
	static const uint16 _patchLangNum[8];

#ifndef PALMOS_68K
	static const HuffTree _huffTree_00109[]; // trees moved to hufftext.cpp
	static const HuffTree _huffTree_00267[];
	static const HuffTree _huffTree_00288[];
	static const HuffTree _huffTree_00303[];
	static const HuffTree _huffTree_00331[];
	static const HuffTree _huffTree_00348[];
	static const HuffTree _huffTree_00365[];
	static const HuffTree _huffTree_00368[];
	static const HuffTree _huffTree_00372[];
#else
public:
	static const HuffTree *_huffTree_00109; // trees moved to hufftext.cpp
	static const HuffTree *_huffTree_00267;
	static const HuffTree *_huffTree_00288;
	static const HuffTree *_huffTree_00303;
	static const HuffTree *_huffTree_00331;
	static const HuffTree *_huffTree_00348;
	static const HuffTree *_huffTree_00365;
	static const HuffTree *_huffTree_00368;
	static const HuffTree *_huffTree_00372;
#endif
};

} // End of namespace Sky

#endif
