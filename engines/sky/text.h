/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SKY_TEXT_H
#define SKY_TEXT_H


#include "common/scummsys.h"
#include "sky/sky.h"

namespace Sky {

struct Compact;
class Disk;
class SkyCompact;

struct HuffTree {
	unsigned char lChild;
	unsigned char rChild;
	unsigned char value;
};

#define NUM_PATCH_MSG 9

struct PatchMessage {
	uint32 textNr;
	char text[100];
};

class Text {
public:
	Text(SkyEngine *vm, Disk *skyDisk, SkyCompact *skyCompact);
	~Text();
	struct DisplayedText displayText(uint32 textNum, uint8 *dest, bool center, uint16 pixelWidth, uint8 color);
	struct DisplayedText displayText(char *textPtr, uint32 bufLen, uint8 *dest, bool center, uint16 pixelWidth, uint8 color);
	struct DisplayedText lowTextManager(uint32 textNum, uint16 width, uint16 logicNum, uint8 color, bool center);
	void fnSetFont(uint32 fontNr);
	void fnTextModule(uint32 textInfoId, uint32 textNo);
	void fnPointerText(uint32 pointedId, uint16 mouseX, uint16 mouseY);
	void logicCursor(Compact *textCompact, uint16 mouseX, uint16 mouseY);
	void changeTextSpriteColor(uint8 *sprData, uint8 newCol);
	uint32 giveCurrentCharSet();

	uint32 _numLetters;	//no of chars in message

private:
	void initHuffTree();
	void getText(uint32 textNr);
	char getTextChar(uint8 **data, uint32 *bitPos);
	bool getTextBit(uint8 **data, uint32 *bitPos);
	void makeGameCharacter(uint8 textChar, uint8 *charSetPtr, uint8 *&data, uint8 color, uint16 bufPitch);
	void makeChineseGameCharacter(uint16 textChar, uint8 *charSetPtr, uint8 *&dest, uint8 color, uint16 bufPitch);

	bool patchMessage(uint32 textNum);

	Disk *_skyDisk;
	SkyCompact *_skyCompact;
	SkyEngine *_vm;

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

	static const HuffTree _huffTree_00109[]; // trees moved to hufftext.cpp
	static const HuffTree _huffTree_00267[];
	static const HuffTree _huffTree_00288[];
	static const HuffTree _huffTree_00303[];
	static const HuffTree _huffTree_00331[];
	static const HuffTree _huffTree_00348[];
	static const HuffTree _huffTree_00365[];
	static const HuffTree _huffTree_00368[];
	static const HuffTree _huffTree_00372[];
};

} // End of namespace Sky

#endif
