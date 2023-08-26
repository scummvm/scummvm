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
	s.skip(0x43, kGameTypeNancy2);

	s.syncAsUint16LE(firstScene.sceneID);
	s.skip(0xC, kGameTypeVampire, kGameTypeVampire); // Palette name + unknown 2 bytes
	s.syncAsUint16LE(firstScene.frameID);
	s.syncAsUint16LE(firstScene.verticalOffset);
	s.syncAsUint16LE(startTimeHours);
	s.syncAsUint16LE(startTimeMinutes);

	s.skip(0xA7, kGameTypeVampire, kGameTypeNancy2);
	s.skip(4, kGameTypeNancy3, kGameTypeNancy3);
	s.skip(3, kGameTypeNancy4);

	s.skip(8, kGameTypeVampire, kGameTypeVampire);
	readRect(s, extraButtonHotspot, kGameTypeVampire, kGameTypeVampire);
	readRect(s, extraButtonHotspot, kGameTypeNancy2);
	readRect(s, extraButtonHighlightDest, kGameTypeNancy1);
	s.skip(0x10, kGameTypeVampire, kGameTypeVampire);
	readRect(s, textboxScreenPosition);
	readRect(s, inventoryBoxScreenPosition);
	readRect(s, menuButtonSrc);
	readRect(s, helpButtonSrc);
	readRect(s, menuButtonDest);
	readRect(s, helpButtonDest);
	readRect(s, menuButtonHighlightSrc, kGameTypeNancy2);
	readRect(s, helpButtonHighlightSrc, kGameTypeNancy2);
	readRect(s, clockHighlightSrc, kGameTypeNancy2);

	s.skip(0xE, kGameTypeVampire, kGameTypeVampire);
	s.skip(9, kGameTypeNancy1);
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

	s.skip(2, kGameTypeNancy3); // Unknown, 3000

	readFilename(s, inventoryBoxIconsImageName);
	readFilename(s, inventoryCursorsImageName);

	s.skip(0x4, kGameTypeVampire, kGameTypeNancy1); // inventory box icons surface w/h
	s.skip(0x4, kGameTypeVampire, kGameTypeNancy1); // inventory cursors surface w/h

	s.skip(0x10); // unknown rect, same size as a hotspot

	byte textBuf[60];

	if (s.getVersion() >= kGameTypeNancy2) {
		cantSound.readNormal(*chunkStream);
		s.syncBytes(textBuf, 60);
		textBuf[59] = '\0';
		cantText = (char *)textBuf;
	}

	uint itemNameLength;
	switch (s.getVersion()) {
		case kGameTypeVampire :
			itemNameLength = 15;
			break;
		case kGameTypeNancy1 :
			itemNameLength = 20;
			break;
		case kGameTypeNancy2 :
			// fall through
		default:
			itemNameLength = 48;
			break;
	}

	uint16 numItems = g_nancy->getStaticData().numItems;
	itemDescriptions.resize(numItems);
	for (uint i = 0; i < numItems; ++i) {
		ItemDescription &item = itemDescriptions[i];

		s.syncBytes(textBuf, itemNameLength);
		textBuf[itemNameLength - 1] = '\0';
		item.name = (char *)textBuf;

		s.syncAsUint16LE(item.keepItem);
		readRect(s, item.sourceRect);
		readRect(s, item.highlightedSourceRect, kGameTypeNancy2);

		if (s.getVersion() == kGameTypeNancy2) {
			s.syncBytes(textBuf, 60);
			textBuf[59] = '\0';
			item.specificCantText = (char *)textBuf;

			s.syncBytes(textBuf, 60);
			textBuf[59] = '\0';
			item.generalCantText = (char *)textBuf;

			item.specificCantSound.readNormal(*chunkStream);
			item.generalCantSound.readNormal(*chunkStream);
		} else if (s.getVersion() >= kGameTypeNancy3) {
			s.syncBytes(textBuf, 60);
			textBuf[59] = '\0';
			item.specificCantText = (char *)textBuf;

			item.specificCantSound.readNormal(*chunkStream);
		}
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
	defaultFontID = chunkStream->readUint16LE();

	if (g_nancy->getGameType() >= kGameTypeNancy2) {
		chunkStream->skip(2);
		conversationFontID = chunkStream->readUint16LE();
		highlightConversationFontID = chunkStream->readUint16LE();
	} else {
		conversationFontID = defaultFontID;
		highlightConversationFontID = defaultFontID;
	}

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
		sounds[i].readMenu(*chunkStream);
	}

	s.skip(0x20);

	s.syncAsUint16LE(globeFrameTime, kGameTypeVampire, kGameTypeVampire);
	readRectArray(s, globeSrcs, 8, kGameTypeVampire, kGameTypeVampire);
	readRect(s, globeDest, kGameTypeVampire, kGameTypeVampire);

	s.skip(2, kGameTypeNancy1);
	readRect(s, buttonSrc, kGameTypeNancy1);
	readRect(s, buttonDest, kGameTypeNancy1);

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

	if (g_nancy->getGameType() <= kGameTypeNancy1) {
		buttonDest.left = chunkStream->readUint16LE();
		buttonDest.top = chunkStream->readUint16LE();
		buttonDest.right = chunkStream->readUint16LE();
		buttonDest.bottom = chunkStream->readUint16LE();
		buttonSrc.left = chunkStream->readUint16LE();
		buttonSrc.top = chunkStream->readUint16LE();
		buttonSrc.right = chunkStream->readUint16LE();
		buttonSrc.bottom = chunkStream->readUint16LE();
	} else {
		readRect(*chunkStream, buttonDest);
		readRect(*chunkStream, buttonSrc);
		readRect(*chunkStream, buttonHoverSrc);
	}

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
	sound.readMenu(*chunkStream);

	delete chunkStream;
}

MENU::MENU(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	readFilename(*chunkStream, _imageName);
	chunkStream->skip(22);

	uint numOptions = 8;

	_buttonDests.resize(numOptions);
	_buttonDownSrcs.resize(numOptions);

	if (g_nancy->getGameType() <= kGameTypeNancy1) {
		for (uint i = 0; i < numOptions; ++i) {
			Common::Rect &rect = _buttonDests[i];
			rect.left = chunkStream->readSint16LE();
			rect.top = chunkStream->readSint16LE();
			rect.right = chunkStream->readSint16LE();
			rect.bottom = chunkStream->readSint16LE();
		}

		for (uint i = 0; i < numOptions; ++i) {
			Common::Rect &rect = _buttonDownSrcs[i];
			rect.left = chunkStream->readSint16LE();
			rect.top = chunkStream->readSint16LE();
			rect.right = chunkStream->readSint16LE();
			rect.bottom = chunkStream->readSint16LE();
		}
	} else {
		_buttonHighlightSrcs.resize(numOptions);

		readRectArray(*chunkStream, _buttonDests, numOptions);
		readRectArray(*chunkStream, _buttonDownSrcs, numOptions);
		readRectArray(*chunkStream, _buttonDisabledSrcs, numOptions);
		readRectArray(*chunkStream, _buttonHighlightSrcs, numOptions);
	}

	delete chunkStream;
}

SET::SET(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	readFilename(*chunkStream, _imageName);
	chunkStream->skip(20); // image info
	chunkStream->skip(16); // bounds for all scrollbars

	uint numButtons = g_nancy->getGameType() == kGameTypeVampire ? 5 : 4;

	readRectArray(*chunkStream, _scrollbarBounds, 3);
	readRectArray(*chunkStream, _buttonDests, numButtons);
	readRectArray(*chunkStream, _buttonDownSrcs, numButtons);

	if (g_nancy->getGameType() >= kGameTypeNancy2) {
		readRect(*chunkStream, _doneButtonHighlightSrc);
	}
	
	readRectArray(*chunkStream, _scrollbarSrcs, 3);

	_scrollbarsCenterYPos.resize(3);
	_scrollbarsCenterXPosL.resize(3);
	_scrollbarsCenterXPosR.resize(3);
	for (uint i = 0; i < 3; ++i) {
		_scrollbarsCenterYPos[i] = chunkStream->readUint16LE();
		_scrollbarsCenterXPosL[i] = chunkStream->readUint16LE();
		_scrollbarsCenterXPosR[i] = chunkStream->readUint16LE();
	}

	_sounds.resize(3);
	for (uint i = 0; i < 3; ++i) {
		_sounds[i].readMenu(*chunkStream);
	}

	delete chunkStream;
}

HINT::HINT(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	uint size = chunkStream->size();
	numHints.resize(size);
	for (uint i = 0; i < size; ++i) {
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

	uint numFrames = s.getVersion() == kGameTypeVampire? 8 : 7;

	readRectArray(s, animSrcs, numFrames);
	readRectArray(s, animDests, numFrames, kGameTypeNancy2);

	readRect(s, staticImageSrc, kGameTypeNancy2);
	readRect(s, staticImageDest, kGameTypeNancy2);

	readRectArray(s, hoursHandSrcs, 12);
	readRectArray(s, hoursHandDests, 12, kGameTypeNancy2);

	readRectArray(s, minutesHandSrcs, 4);
	readRectArray(s, minutesHandDests, 4, kGameTypeNancy2);

	readRect(s, screenPosition, kGameTypeVampire, kGameTypeVampire);

	readRectArray(s, hoursHandDests, 12, kGameTypeVampire, kGameTypeVampire);
	readRectArray(s, minutesHandDests, 4, kGameTypeVampire, kGameTypeVampire);

	readRect(s, staticImageSrc, kGameTypeVampire, kGameTypeVampire);
	readRect(s, staticImageDest, kGameTypeVampire, kGameTypeVampire);

	s.syncAsUint32LE(timeToKeepOpen);
	s.syncAsUint16LE(frameTime);

	delete chunkStream;
}

SPEC::SPEC(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);

	fadeToBlackNumFrames = chunkStream->readByte();
	fadeToBlackFrameTime = chunkStream->readUint16LE();
	crossDissolveNumFrames = chunkStream->readUint16LE();
}

RCLB::RCLB(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);

	lightSwitchID = chunkStream->readUint16LE();
	unk2 = chunkStream->readUint16LE();

	char buf[100];

	while (chunkStream->pos() < chunkStream->size()) {
		themes.push_back(Theme());
		Theme &theme = themes.back();

		chunkStream->read(buf, 100);
		theme.themeName = buf;

		for (uint i = 0; i < 10; ++i) {
			int32 val = chunkStream->readSint32LE();
			if (val != -1) {
				theme.wallIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int16 val = chunkStream->readUint16LE();
			if (val != -1) {
				theme.floorIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int16 val = chunkStream->readSint16LE();
			if (val != -1) {
				theme.exitFloorIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int16 val = chunkStream->readSint16LE();
			if (val != -1) {
				theme.ceilingIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int32 val = chunkStream->readSint32LE();
			if (val != -1) {
				theme.doorIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int32 val = chunkStream->readSint32LE();
			if (val != -1) {
				theme.transparentwallIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int32 val = chunkStream->readSint32LE();
			if (val != -1) {
				theme.objectwallIDs.push_back(val);
			}
		}

		for (uint i = 0; i < 10; ++i) {
			int16 val = chunkStream->readSint16LE();
			if (val != -1) {
				theme.objectWallHeights.push_back(val);
			}
		}

		theme.generalLighting = chunkStream->readUint16LE();
		theme.hasLightSwitch = chunkStream->readUint16LE();
		theme.transparentWallDensity = chunkStream->readSint16LE();
		theme.objectWallDensity = chunkStream->readSint16LE();
		theme.doorDensity = chunkStream->readSint16LE();
	}
}

RCPR::RCPR(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);

	readRectArray(*chunkStream, screenViewportSizes, 6);
	viewportSizeUsed = chunkStream->readUint16LE();

	wallColor[0] = chunkStream->readByte();
	wallColor[1] = chunkStream->readByte();
	wallColor[2] = chunkStream->readByte();

	playerColor[0] = chunkStream->readByte();
	playerColor[1] = chunkStream->readByte();
	playerColor[2] = chunkStream->readByte();

	doorColor[0] = chunkStream->readByte();
	doorColor[1] = chunkStream->readByte();
	doorColor[2] = chunkStream->readByte();

	lightSwitchColor[0] = chunkStream->readByte();
	lightSwitchColor[1] = chunkStream->readByte();
	lightSwitchColor[2] = chunkStream->readByte();

	exitColor[0] = chunkStream->readByte();
	exitColor[1] = chunkStream->readByte();
	exitColor[2] = chunkStream->readByte();

	uColor6[0] = chunkStream->readByte();
	uColor6[1] = chunkStream->readByte();
	uColor6[2] = chunkStream->readByte();

	uColor7[0] = chunkStream->readByte();
	uColor7[1] = chunkStream->readByte();
	uColor7[2] = chunkStream->readByte();

	uColor8[0] = chunkStream->readByte();
	uColor8[1] = chunkStream->readByte();
	uColor8[2] = chunkStream->readByte();

	transparentWallColor[0] = chunkStream->readByte();
	transparentWallColor[1] = chunkStream->readByte();
	transparentWallColor[2] = chunkStream->readByte();

	uColor10[0] = chunkStream->readByte();
	uColor10[1] = chunkStream->readByte();
	uColor10[2] = chunkStream->readByte();

	Common::String tmp;
	while (chunkStream->pos() < chunkStream->size()) {
		readFilename(*chunkStream, tmp);
		if (tmp.hasPrefix("Wall")) {
			wallNames.push_back(tmp);
		} else if (tmp.hasPrefix("SpW")) {
			specialWallNames.push_back(tmp);
		} else if (tmp.hasPrefix("Ceil")) {
			ceilingNames.push_back(tmp);
		} else if (tmp.hasPrefix("Floor")) {
			floorNames.push_back(tmp);
		}
	}
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
