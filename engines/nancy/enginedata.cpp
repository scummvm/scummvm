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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

#include "common/serializer.h"

namespace Nancy {

BSUM::BSUM(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);
	
	chunkStream->seek(0);
	Common::Serializer s(chunkStream, nullptr);
	s.setVersion(g_nancy->getGameType());

	// The header is used to verify savegames
	s.syncBytes(header, 90);
	
	s.skip(0x17, kGameTypeVampire, kGameTypeVampire);
	s.skip(0x49, kGameTypeNancy1, kGameTypeNancy1);
	s.skip(0xF7, kGameTypeNancy2, kGameTypeNancy3);
	s.syncAsUint16LE(firstScene.sceneID);
	s.skip(0xC, kGameTypeVampire, kGameTypeVampire); // Palette name + unknown 2 bytes
	s.syncAsUint16LE(firstScene.frameID);
	s.syncAsUint16LE(firstScene.verticalOffset);
	s.syncAsUint16LE(startTimeHours);
	s.syncAsUint16LE(startTimeMinutes);

	s.skip(0xA7, kGameTypeVampire, kGameTypeNancy2);
	s.skip(4, kGameTypeNancy3);

	s.skip(8, kGameTypeVampire, kGameTypeVampire);
	if (s.getVersion() == kGameTypeVampire) {
		readRect(*chunkStream, mapButtonHotspot);
	}

	s.skip(0x10, kGameTypeVampire, kGameTypeNancy1);
	s.skip(0x20, kGameTypeNancy2, kGameTypeNancy3);
	readRect(*chunkStream, textboxScreenPosition);

	s.skip(0x10);
	readRect(*chunkStream, menuButtonSrc);
	readRect(*chunkStream, helpButtonSrc);
	readRect(*chunkStream, menuButtonDest);
	readRect(*chunkStream, helpButtonDest);

	s.skip(0xE, kGameTypeVampire, kGameTypeVampire);
	s.skip(9, kGameTypeNancy1, kGameTypeNancy1);
	s.skip(0x39, kGameTypeNancy2, kGameTypeNancy3);
	s.syncAsUint16LE(horizontalEdgesSize);
	s.syncAsUint16LE(verticalEdgesSize);

	s.skip(0x1A, kGameTypeVampire, kGameTypeVampire);
	s.skip(0x1C, kGameTypeNancy1);
	s.syncAsSint16LE(playerTimeMinuteLength);

	s.skip(2);
	s.syncAsByte(overrideMovementTimeDeltas);
	s.syncAsSint16LE(slowMovementTimeDelta);
	s.syncAsSint16LE(fastMovementTimeDelta);

	delete chunkStream;
}

VIEW::VIEW(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);
	
	chunkStream->seek(0);
	readRect(*chunkStream, screenPosition);
	readRect(*chunkStream, bounds);

	delete chunkStream;
}

INV::INV(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	Common::Serializer s(chunkStream, nullptr);
	s.setVersion(g_nancy->getGameType());

	readRect(*chunkStream, scrollbarSrcBounds);
	s.syncAsUint16LE(scrollbarDefaultPos.x);
	s.syncAsUint16LE(scrollbarDefaultPos.y);
	s.syncAsUint16LE(scrollbarMaxScroll);

	s.skip(0xC0);

	uint numFrames = g_nancy->getStaticData().numCurtainAnimationFrames;
	curtainAnimationSrcs.resize(numFrames * 2);
	for (uint i = 0; i < numFrames * 2; ++i) {
		readRect(*chunkStream, curtainAnimationSrcs[i]);
	}

	readRect(*chunkStream, inventoryScreenPosition);
	s.syncAsUint16LE(curtainsFrameTime);

	readFilename(*chunkStream, inventoryBoxIconsImageName);
	readFilename(*chunkStream, inventoryCursorsImageName);

	s.skip(0x18);
	byte itemName[20];
	uint itemNameLength = g_nancy->getGameType() == kGameTypeVampire ? 15 : 20;

	uint16 numItems = g_nancy->getStaticData().numItems;
	itemDescriptions.resize(numItems);
	for (uint i = 0; i < numItems; ++i) {
		ItemDescription &item = itemDescriptions[i];
		
		s.syncBytes(itemName, itemNameLength);
		itemName[itemNameLength - 1] = '\0';
		item.name = (char *)itemName;
		s.syncAsUint16LE(item.keepItem);
		readRect(*chunkStream, item.sourceRect);
	}

	delete chunkStream;
}

TBOX::TBOX(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	bool isVampire = g_nancy->getGameType() == Nancy::GameType::kGameTypeVampire;

	chunkStream->seek(0);
	readRect(*chunkStream, scrollbarSrcBounds);

	chunkStream->seek(0x20);
	readRect(*chunkStream, innerBoundingBox);

	scrollbarDefaultPos.x = chunkStream->readUint16LE() - (isVampire ? 1 : 0);
	scrollbarDefaultPos.y = chunkStream->readUint16LE();
	scrollbarMaxScroll = chunkStream->readUint16LE();

	firstLineOffset = chunkStream->readUint16LE() + 1;
	lineHeight = chunkStream->readUint16LE() + (isVampire ? 1 : 0);
	borderWidth = chunkStream->readUint16LE() - 1;
	maxWidthDifference = chunkStream->readUint16LE();

	if (isVampire) {
		ornamentSrcs.resize(14);
		ornamentDests.resize(14);
		
		chunkStream->seek(0x3E);
		for (uint i = 0; i < 14; ++i) {
			readRect(*chunkStream, ornamentSrcs[i]);
		}

		for (uint i = 0; i < 14; ++i) {
			readRect(*chunkStream, ornamentDests[i]);
		}
	}

	chunkStream->seek(0x1FE);
	fontID = chunkStream->readUint16LE();

	delete chunkStream;
}

} // End of namespace Nancy
