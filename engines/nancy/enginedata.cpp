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
	s.skip(0x43, kGameTypeNancy2, kGameTypeNancy3);
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
	readRect(*chunkStream, inventoryBoxScreenPosition);
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
	s.syncAsUint16LE(buttonPressTimeDelay);
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

	readRectArray(s, ornamentSrcs, 6, kGameTypeVampire, kGameTypeNancy1);
	readRectArray(s, ornamentDests, 6, kGameTypeVampire, kGameTypeNancy1);

	uint numFrames = g_nancy->getStaticData().numCurtainAnimationFrames;

	readRectArray(s, curtainAnimationSrcs, numFrames * 2);

	readRect(s, curtainsScreenPosition);
	s.syncAsUint16LE(curtainsFrameTime);

	readFilename(s, inventoryBoxIconsImageName);
	readFilename(s, inventoryCursorsImageName);

	s.skip(0x4); // inventory box icons surface w/h
	s.skip(0x4); // inventory cursors surface w/h

	s.skip(0x10); // unknown rect, same size as a hotspot

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
		readRect(s, item.sourceRect);
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

MAP::MAP(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	Common::Serializer s(chunkStream, nullptr);
	s.setVersion(g_nancy->getGameType());
	uint numLocations = s.getVersion() == kGameTypeVampire ? 7 : 4;
	uint numMaps = s.getVersion() == kGameTypeVampire ? 4 : 2;

	readFilenameArray(s, mapNames, numMaps);
	readFilenameArray(s, mapPaletteNames, numMaps, kGameTypeVampire, kGameTypeVampire);

	s.skip(4);

	sounds.resize(numMaps);
	for (uint i = 0; i < numMaps; ++i) {
		sounds[i].read(*chunkStream, SoundDescription::kMenu);
	}

	s.skip(0x20);

	s.syncAsUint16LE(globeFrameTime, kGameTypeVampire, kGameTypeVampire);
	readRectArray(s, globeSrcs, 8, kGameTypeVampire, kGameTypeVampire);
	readRect(s, globeDest, kGameTypeVampire, kGameTypeVampire);

	s.skip(2, kGameTypeNancy1, kGameTypeNancy1);
	readRect(s, buttonSrc, kGameTypeNancy1, kGameTypeNancy1);
	readRect(s, buttonDest, kGameTypeNancy1, kGameTypeNancy1);

	locations.resize(numLocations);

	for (uint i = 0; i < numLocations; ++i) {
		readRect(*chunkStream, locations[i].labelSrc);
	}

	readRect(s, closedLabelSrc);

	readRect(s, globeGargoyleSrc, kGameTypeVampire, kGameTypeVampire);
	readRect(s, globeGargoyleDest, kGameTypeVampire, kGameTypeVampire);

	char buf[30];

	for (uint i = 0; i < numLocations; ++i) {
		s.syncBytes((byte *)buf, 30);
		buf[29] = '\0';
		locations[i].description = buf;
	}

	for (uint i = 0; i < numLocations; ++i) {
		readRect(*chunkStream, locations[i].hotspot);
	}

	s.skip(numLocations * 2);
	s.skip(0x10);

	s.syncAsUint16LE(cursorPosition.x);
	s.syncAsUint16LE(cursorPosition.y);

	for (uint j = 0; j < 2; ++j) {
		for (uint i = 0; i < numLocations; ++i) {
			SceneChangeDescription &sc = locations[i].scenes[j];
			s.syncAsUint16LE(sc.sceneID);
			s.syncAsUint16LE(sc.frameID);
			s.syncAsUint16LE(sc.verticalOffset);
			s.syncAsUint16LE(sc.paletteID, kGameTypeVampire, kGameTypeVampire);
		}
	}

	delete chunkStream;
}

HELP::HELP(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	readFilename(*chunkStream, imageName);

	chunkStream->skip(20);
	buttonDest.left = chunkStream->readUint16LE();
	buttonDest.top = chunkStream->readUint16LE();
	buttonDest.right = chunkStream->readUint16LE();
	buttonDest.bottom = chunkStream->readUint16LE();
	buttonSrc.left = chunkStream->readUint16LE();
	buttonSrc.top = chunkStream->readUint16LE();
	buttonSrc.right = chunkStream->readUint16LE();
	buttonSrc.bottom = chunkStream->readUint16LE();

	delete chunkStream;
}

CRED::CRED(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	bool isVampire = g_nancy->getGameType() == kGameTypeVampire;
	chunkStream->seek(0);

	readFilename(*chunkStream, imageName);

	textNames.resize(isVampire ? 7 : 1);
	for (Common::String &str : textNames) {
		readFilename(*chunkStream, str);
	}

	chunkStream->skip(0x20);
	readRect(*chunkStream, textScreenPosition);
	chunkStream->skip(0x10);

	updateTime = chunkStream->readUint16LE();
	pixelsToScroll = chunkStream->readUint16LE();
	sound.read(*chunkStream, SoundDescription::kMenu);

	delete chunkStream;
}

HINT::HINT(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	numHints.resize(chunkStream->size());
	for (uint i = 0; i < chunkStream->size(); ++i) {
		numHints[i] = chunkStream->readByte();
	}

	delete chunkStream;
}

SPUZ::SPUZ(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	tileOrder.resize(3);

	for (uint i = 0; i < 3; ++i) {
		tileOrder[i].resize(36);
		for (uint j = 0; j < 36; ++j) {
			tileOrder[i][j] = chunkStream->readSint16LE();
		}
	}
	
	delete chunkStream;
}

CLOK::CLOK(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	Common::Serializer s(chunkStream, nullptr);
	s.setVersion(g_nancy->getGameType());

	readRectArray(s, animSrcs, 8);

	readRectArray(s, hoursHandSrcs, 12);
	readRectArray(s, minutesHandSrcs, 4);

	readRect(s, screenPosition);

	readRectArray(s, hoursHandDests, 12);
	readRectArray(s, minutesHandDests, 4);

	readRect(*chunkStream, gargoyleEyesSrc);
	readRect(*chunkStream, gargoyleEyesDest);

	s.syncAsUint32LE(timeToKeepOpen);
	s.syncAsUint16LE(frameTime);

	delete chunkStream;
}

ImageChunk::ImageChunk(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	readFilename(*chunkStream, imageName);
	width = chunkStream->readUint16LE();
	height = chunkStream->readUint16LE();

	delete chunkStream;
}

} // End of namespace Nancy
