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
#include "pelrock/types.h"

namespace Pelrock {

static const int walkingAnimLengths[4] = {8, 8, 4, 4}; // size of each inner array
static const int talkingAnimLengths[4] = {8, 8, 4, 4}; // size of each inner array
static const int interactingAnimLength = 2;

class ResourceManager {
private:
	void mergeRleBlocks(Common::SeekableReadStream *stream, uint32 offset, int numBlocks, byte *outputBuffer);
	void loadInventoryDescriptions();
	InventoryObject *_inventoryIcons = nullptr;
public:
	ResourceManager(/* args */);
	~ResourceManager();

	void loadSettingsMenu();
	void loadCursors();
	void loadInteractionIcons();
	void loadAlfredAnims();
	void loadInventoryItems();
	InventoryObject getInventoryObject(byte index);
	byte *loadExtra();

	byte *alfredIdle[4] = {nullptr}; // 4 directions

	byte **alfredWalkFrames[4]; // 4 arrays of arrays

	byte **alfredTalkFrames[4]; // 4 arrays of arrays

	byte **alfredCombFrames[2];
	byte **alfredInteractFrames[4];

	byte *_cursorMasks[5] = {nullptr};
	byte *_verbIcons[9] = {nullptr};
	byte *_popUpBalloon = nullptr;

	byte *_mainMenu = nullptr;
	byte _mainMenuPalette[768] = {0};
	Common::Array<Common::String> _inventoryDescriptions;
};

} // End of namespace Pelrock
#endif
