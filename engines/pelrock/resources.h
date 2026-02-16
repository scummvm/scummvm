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
#ifndef PELROCK_RESOURCES_H
#define PELROCK_RESOURCES_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "pelrock/offsets.h"
#include "pelrock/types.h"

namespace Pelrock {

static const int walkingAnimLengths[4] = {8, 8, 4, 4}; // size of each inner array
static const int talkingAnimLengths[4] = {8, 8, 4, 4}; // size of each inner array
static const int interactingAnimLength = 2;

class ResourceManager {
private:
	InventoryObject *_inventoryIcons = nullptr;

public:
	ResourceManager(/* args */);
	~ResourceManager();

	void mergeRleBlocks(Common::SeekableReadStream *stream, uint32 offset, int numBlocks, byte *outputBuffer);
	void loadSettingsMenu();
	void loadCursors();
	void loadInteractionIcons();
	void loadAlfredAnims();
	void loadAlfredSpecialAnim(int numAnim, bool reverse = false);
	void clearSpecialAnim();
	void loadInventoryItems();
	void loadHardcodedText();
	void getPaletteForRoom28(byte *palette);
	Common::StringArray loadComputerText();
	void getExtraScreen(int screenIndex, byte *screenBuf, byte *palette);
	Common::Array<Common::StringArray> getCredits();
	Common::Array<Common::Array<Common::String>> processTextData(byte *data, size_t size, bool decode = false);
	Sticker getSticker(int stickerIndex);
	InventoryObject getIconForObject(byte index);
	byte *loadExtra();

	byte *alfredIdle[4]; // 4 directions

	byte **alfredWalkFrames[4]; // 4 arrays of arrays

	byte **alfredTalkFrames[4]; // 4 arrays of arrays

	byte **alfredCombFrames[2];
	byte **alfredInteractFrames[4];

	byte *_cursorMasks[5];
	byte *_verbIcons[9];
	byte *_popUpBalloon = nullptr;
	Common::Array<Common::StringArray> _ingameTexts;
	Common::String _conversationTerminator;

	// Special anims
	AlfredSpecialAnim *_currentSpecialAnim = nullptr;
	bool _isSpecialAnimFinished = false;
	static const AlfredSpecialAnimOffset alfredSpecialAnims[];

};

} // End of namespace Pelrock
#endif
