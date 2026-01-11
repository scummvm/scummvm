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
#include "common/scummsys.h"

#include "pelrock/pelrock.h"
#include "pelrock/room.h"
#include "pelrock/util.h"
#include "room.h"

namespace Pelrock {

RoomManager::RoomManager() {
	_pixelsShadows = new byte[640 * 400];
	_roomNames = loadRoomNames();
}

RoomManager::~RoomManager() {
	if (_pixelsShadows != nullptr) {
		delete[] _pixelsShadows;
		_pixelsShadows = nullptr;
	}
	delete[] _resetData;
}

void RoomManager::getPalette(Common::File *roomFile, int roomOffset, byte *palette) {
	// get palette
	int paletteOffset = roomOffset + (11 * 8);
	roomFile->seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile->readUint32LE();
	uint32 size = roomFile->readUint32LE();

	roomFile->seek(offset, SEEK_SET);

	roomFile->read(palette, size);
	for (int i = 0; i < 256; i++) {
		palette[i * 3] = palette[i * 3] << 2;
		palette[i * 3 + 1] = palette[i * 3 + 1] << 2;
		palette[i * 3 + 2] = palette[i * 3 + 2] << 2;
	}
	memcpy(_roomPalette, palette, 768);
}

void RoomManager::getBackground(Common::File *roomFile, int roomOffset, byte *background) {
	roomFile->seek(0, SEEK_SET);
	// get screen
	size_t combined_size = 0;
	for (int pair_idx = 0; pair_idx < 8; pair_idx++) {
		uint32_t pair_offset = roomOffset + (pair_idx * 8);
		if (pair_offset + 8 > roomFile->size())
			continue;

		roomFile->seek(pair_offset, SEEK_SET);
		uint32_t offset = roomFile->readUint32LE();
		uint32_t size = roomFile->readUint32LE();
		if (offset > 0 && size > 0 && offset < roomFile->size()) {
			byte *data = new byte[size];
			roomFile->seek(offset, SEEK_SET);
			roomFile->read(data, size);
			uint8_t *block_data = NULL;
			size_t block_size = rleDecompress(data, size, 0, 640 * 400, &block_data);
			if (block_size + combined_size > 640 * 400) {
				debug(" Warning: decompressed background size exceeds buffer size!");
				block_size = 640 * 400 - combined_size;
			}
			memcpy(background + combined_size, block_data, block_size);
			combined_size += block_size;
			free(block_data);
			delete[] data;
		}
	}
}

void RoomManager::addSticker(int stickerId, bool persist) {
	Sticker sticker = g_engine->_res->getSticker(stickerId);
	if (persist)
		g_engine->_state.roomStickers[_currentRoomNumber].push_back(sticker);
	else
		_transientStickers.push_back(sticker);
}

void RoomManager::removeSticker(int stickerIndex) {
	int index = -1;
	for (int i = 0; i < g_engine->_state.roomStickers[_currentRoomNumber].size(); i++) {
		if (g_engine->_state.roomStickers[_currentRoomNumber][i].stickerIndex == stickerIndex) {
			index = i;
			break;
		}
	}
	if (index != -1 && index < g_engine->_state.roomStickers[_currentRoomNumber].size())
		g_engine->_state.roomStickers[_currentRoomNumber].remove_at(index);
}

bool RoomManager::hasSticker(int index) {
	for (int i = 0; i < g_engine->_state.roomStickers[_currentRoomNumber].size(); i++) {
		if (g_engine->_state.roomStickers[_currentRoomNumber][i].stickerIndex == index) {
			return true;
		}
	}
	return false;
}

void RoomManager::changeExit(int index, bool enabled, bool persist) {
	_currentRoomExits[index].isEnabled = enabled;
	if(persist)
		g_engine->_state.roomExitChanges[_currentRoomNumber].push_back({_currentRoomNumber, _currentRoomExits[index].index, _currentRoomExits[index]});
}

void RoomManager::disableExit(int index, bool persist) {
	changeExit(index, false, persist);
}

void RoomManager::enableExit(int index, bool persist) {
	changeExit(index, true, persist);
}

void RoomManager::changeWalkBox(WalkBox walkbox) {
	g_engine->_state.roomWalkBoxChanges[_currentRoomNumber].push_back({_currentRoomNumber, walkbox.index, walkbox});
}

void RoomManager::changeHotSpot(HotSpot hotspot) {
	g_engine->_state.roomHotSpotChanges[_currentRoomNumber].push_back({_currentRoomNumber, hotspot.innerIndex, hotspot});
}

void RoomManager::enableHotspot(HotSpot *hotspot, bool persist) {
	hotspot->isEnabled = true;
	if (persist) {
		changeHotSpot(*hotspot);
	}
}

void RoomManager::disableHotspot(HotSpot *hotspot, bool persist) {
	hotspot->isEnabled = false;
	if (persist) {
		changeHotSpot(*hotspot);
	}
}

void RoomManager::addWalkbox(WalkBox walkbox) {
	g_engine->_state.roomWalkBoxChanges[_currentRoomNumber].push_back({_currentRoomNumber, walkbox.index, walkbox});
}

HotSpot *RoomManager::findHotspotByExtra(uint16 extra) {
	for (int i = 0; i < _currentRoomHotspots.size(); i++) {
		if (_currentRoomHotspots[i].extra == extra) {
			return &_currentRoomHotspots[i];
		}
	}
	return nullptr;
}

PaletteAnim *RoomManager::getPaletteAnimForRoom(int roomNumber) {
	Common::File exeFile;

	if (!exeFile.open("JUEGO.EXE")) {
		debug("Could not open JUEGO.EXE for palette animation!");
		return nullptr;
	}
	uint32_t offset = 0;
	switch (roomNumber) {
	case 0:
		offset = 0x0004B88C;
		break;
	case 2:
		offset = 0x0004B860;
		break;
	default:
		exeFile.close();
		return nullptr;
	}

	exeFile.seek(offset, SEEK_SET);
	PaletteAnim *anim = new PaletteAnim();
	anim->startIndex = exeFile.readByte();
	anim->paletteMode = exeFile.readByte();
	exeFile.read(anim->data, 10);
	if (anim->paletteMode == 1) {
		for (int i = 2; i < 10; i++) {
			anim->data[i] = anim->data[i] << 2;
		}
	}

	exeFile.close();
	return anim;
}

Common::Array<Exit> RoomManager::loadExits(byte *data, size_t size) {
	Common::Array<Exit> exits;
	int exitCountOffset = 0x1BE;
	byte exitCount = data[exitCountOffset];
	int exitDataOffset = 0x1BF;
	for (int i = 0; i < exitCount; i++) {
		int exitOffset = exitDataOffset + i * 14;
		bool isChanged = false;
		if (g_engine->_state.roomExitChanges.contains(_currentRoomNumber)) {
			// if the exit has been changed, load the changed version
			for (int j = 0; j < g_engine->_state.roomExitChanges[_currentRoomNumber].size(); j++) {
				if (g_engine->_state.roomExitChanges[_currentRoomNumber][j].exitIndex == i) {
					exits.push_back(g_engine->_state.roomExitChanges[_currentRoomNumber][j].exit);
					isChanged = true;
					break;
				}
			}
		}

		if (isChanged)
			continue;

		Exit exit;
		exit.index = i;
		exit.targetRoom = READ_LE_INT16(data + exitOffset);
		exit.isEnabled = data[exitOffset + 2];
		exit.x = READ_LE_INT16(data + exitOffset + 3);
		exit.y = READ_LE_INT16(data + exitOffset + 5);
		exit.w = data[exitOffset + 7];
		exit.h = data[exitOffset + 8];

		exit.targetX = READ_LE_INT16(data + exitOffset + 9);
		exit.targetY = READ_LE_INT16(data + exitOffset + 11);
		byte dir = data[exitOffset + 13];
		switch (dir) {
		case ALFRED_RIGHT:
			exit.dir = ALFRED_RIGHT;
			break;
		case ALFRED_LEFT:
			exit.dir = ALFRED_LEFT;
			break;
		case ALFRED_DOWN:
			exit.dir = ALFRED_DOWN;
			break;
		case ALFRED_UP:
			exit.dir = ALFRED_UP;
			break;
		default:
			exit.dir = ALFRED_DOWN;
			break;
		}

		exits.push_back(exit);
		// debug("Exit %d: targetRoom=%d isEnabled=%d x=%d y=%d w=%d h=%d targetX=%d targetY=%d dir=%d",
		// 	  i, exit.targetRoom, exit.isEnabled, exit.x, exit.y, exit.w, exit.h,
		// 	  exit.targetX, exit.targetY, exit.dir);
	}
	return exits;
}

Common::Array<HotSpot> RoomManager::loadHotspots(byte *data, size_t size) {
	int pair10StartingPos = 0x47a;

	byte hotspot_count = data[pair10StartingPos];
	int hotspotsDataStart = pair10StartingPos + 2;
	Common::Array<HotSpot> hotspots;
	for (int i = 0; i < hotspot_count; i++) {
		int hotspotOffset = hotspotsDataStart + i * 9;
		HotSpot spot;
		spot.innerIndex = i;
		spot.index = i;
		bool isChanged = false;
		if (g_engine->_state.roomHotSpotChanges.contains(_currentRoomNumber)) {
			// if the hotspot has been changed, load the changed version
			for (int j = 0; j < g_engine->_state.roomHotSpotChanges[_currentRoomNumber].size(); j++) {
				if (g_engine->_state.roomHotSpotChanges[_currentRoomNumber][j].hotspotIndex == spot.innerIndex) {
					hotspots.push_back(g_engine->_state.roomHotSpotChanges[_currentRoomNumber][j].hotspot);
					isChanged = true;
					break;
				}
			}
		}
		if (isChanged)
			continue;
		spot.actionFlags = data[hotspotOffset];
		spot.x = READ_LE_INT16(data + hotspotOffset + 1);
		spot.y = READ_LE_INT16(data + hotspotOffset + 3);
		spot.w = data[hotspotOffset + 5];
		spot.h = data[hotspotOffset + 6];
		spot.isSprite = false;
		spot.extra = READ_LE_INT16(data + hotspotOffset + 7);
		debug("Hotspot %d: type=%d x=%d y=%d w=%d h=%d extra=%d, isEnabled=%d", spot.innerIndex, spot.actionFlags, spot.x, spot.y, spot.w, spot.h, spot.extra, spot.isEnabled);
		hotspots.push_back(spot);
	}

	return hotspots;
}

void RoomManager::resetConversationStates(byte roomNumber, byte *conversationData, size_t conversationDataSize) {
	Common::File alfredB;
	if (!alfredB.open("ALFRED.B")) {
		debug("Could not open ALFRED.B to reset conversation states!");
		return;
	}
	bool roomDone = false;
	while (!alfredB.eos() && !roomDone) {
		ResetEntry entry;
		entry.room = alfredB.readUint16LE();
		entry.offset = alfredB.readUint16LE();
		entry.dataSize = alfredB.readByte();
		entry.data = new byte[entry.dataSize];
		alfredB.read(entry.data, entry.dataSize);
		if (roomNumber < entry.room) {
			// We've passed the room we care about
			roomDone = true;
			break;
		}
		if (roomNumber > entry.room) {
			// Not the room we care about, skip
			continue;
		}
		debug("Resetting room %d conversation data at offset %d, size %d", entry.room, entry.offset, entry.dataSize);
		Common::copy(entry.data, entry.data + entry.dataSize, conversationData + entry.offset);
		// delete[] entry.data;
	}
	alfredB.close();
}

void RoomManager::loadRoomMetadata(Common::File *roomFile, int roomNumber) {

	uint32_t outPos = 0;
	_transientStickers.clear();
	_currentRoomNumber = roomNumber;
	int roomOffset = roomNumber * kRoomStructSize;

	// Pairs 0-7 are background data, already loaded

	// Pair 8 - Animation Pixel Data
	byte *pic = nullptr;
	size_t pixelDataSize = 0;
	loadAnimationPixelData(roomFile, roomOffset, pic, pixelDataSize);

	// Pair 9 - Music and sound
	_musicTrack = loadMusicTrackForRoom(roomFile, roomOffset);
	_roomSfx = loadRoomSfx(roomFile, roomOffset);

	// Pair 10
	uint32_t pair10offset = roomOffset + (10 * 8);
	roomFile->seek(pair10offset, SEEK_SET);
	uint32_t pair10dataOffset = roomFile->readUint32LE();
	uint32_t pair10size = roomFile->readUint32LE();

	byte *pair10 = new byte[pair10size];
	roomFile->seek(pair10dataOffset, SEEK_SET);
	roomFile->read(pair10, pair10size);

	// The user's game can be in any state so we reset to defaults first
	resetMetadataDefaults(roomNumber, pair10, pair10size);

	Common::Array<Sprite> sprites = loadRoomAnimations(pic, pixelDataSize, pair10, pair10size);
	Common::Array<HotSpot> staticHotspots = loadHotspots(pair10, pair10size);

	_currentRoomAnims = sprites;
	_currentRoomHotspots = unifyHotspots(sprites, staticHotspots);
	_currentRoomExits = loadExits(pair10, pair10size);
	_currentRoomWalkboxes = loadWalkboxes(pair10, pair10size);
	_scaleParams = loadScalingParams(pair10, pair10size);

	// Pair 11 is the palette, already loaded

	// Pair 12 - Room Texts
	uint32_t pair12offset = roomOffset + (12 * 8);
	roomFile->seek(pair12offset, SEEK_SET);
	uint32_t pair12dataOffset = roomFile->readUint32LE();
	uint32_t pair12size = roomFile->readUint32LE();

	byte *pair12 = new byte[pair12size];
	roomFile->seek(pair12dataOffset, SEEK_SET);
	roomFile->read(pair12, pair12size);

	resetConversationStates(roomNumber, pair12, pair12size);

	_conversationOffset = loadDescriptions(pair12, pair12size, _currentRoomDescriptions);
	loadConversationData(pair12, pair12size, _conversationOffset, _conversationDataSize, _conversationData);

	if (_pixelsShadows != nullptr)
		delete[] _pixelsShadows;
	_pixelsShadows = loadShadowMap(roomNumber);

	loadRemaps(roomNumber);

	for (int i = 0; i < _currentRoomHotspots.size(); i++) {
		HotSpot hotspot = _currentRoomHotspots[i];
		drawRect(g_engine->_screen, hotspot.x, hotspot.y, hotspot.w, hotspot.h, 200 + i);
	}

	for (int i = 0; i < _currentRoomExits.size(); i++) {
		Exit exit = _currentRoomExits[i];
	}

	PaletteAnim *anim = getPaletteAnimForRoom(roomNumber);
	if (anim != nullptr) {
		if (_currentPaletteAnim != nullptr) {
			delete _currentPaletteAnim;
		}
		_currentPaletteAnim = anim;
	} else {
		_currentPaletteAnim = nullptr;
	}

	delete[] pair10;
	delete[] pair12;
}

Common::Array<HotSpot> RoomManager::unifyHotspots(Common::Array<Pelrock::Sprite> &anims, Common::Array<Pelrock::HotSpot> &staticHotspots) {
	Common::Array<HotSpot> unifiedHotspots;
	for (int i = 0; i < anims.size(); i++) {
		HotSpot thisHotspot;
		thisHotspot.index = i;
		thisHotspot.x = anims[i].x;
		thisHotspot.y = anims[i].y;
		thisHotspot.w = anims[i].w;
		thisHotspot.h = anims[i].h;
		thisHotspot.extra = anims[i].extra;
		thisHotspot.actionFlags = anims[i].actionFlags;
		thisHotspot.isEnabled = !anims[i].isDisabled;
		thisHotspot.isSprite = true;
		thisHotspot.zOrder = anims[i].zOrder;
		unifiedHotspots.push_back(thisHotspot);
	}

	// debug("total descriptions = %d, anims = %d, hotspots = %d", descriptions.size(), anims.size(), staticHotspots.size());
	for (int i = 0; i < staticHotspots.size(); i++) {
		HotSpot hotspot = staticHotspots[i];
		hotspot.index = anims.size() + i;
		unifiedHotspots.push_back(hotspot);
	}
	return unifiedHotspots;
}

void RoomManager::init() {
	Common::File alfred8;
	if (!alfred8.open("ALFRED.8")) {
		error("Couldnt find file ALFRED.8");
	}
	// _resetDataSize = alfred8.size();
	// _resetData = new byte[_resetDataSize];
	// alfred8.read(_resetData, _resetDataSize);
	// alfred8.close();
}

void RoomManager::loadAnimationPixelData(Common::File *roomFile, int roomOffset, byte *&buffer, size_t &outSize) {
	uint32_t pair_offset = roomOffset + (8 * 8);
	roomFile->seek(pair_offset, SEEK_SET);
	uint32_t offset = roomFile->readUint32LE();
	uint32_t size = roomFile->readUint32LE();

	byte *pixelData = new byte[size];
	roomFile->seek(offset, SEEK_SET);
	roomFile->read(pixelData, size);
	if (offset > 0 && size > 0) {
		outSize = rleDecompress(pixelData, size, 0, size, &buffer, true);
	}
}

Common::Array<Sprite> RoomManager::loadRoomAnimations(byte *pixelData, size_t pixelDataSize, byte *data, size_t size) {

	Common::Array<Sprite> anims = Common::Array<Sprite>();
	uint32_t spriteCountPos = 5;
	byte spriteCount = data[spriteCountPos] - 2;
	debug("Sprite count: %d", spriteCount);
	uint32_t metadata_start = spriteCountPos + (44 * 2 + 5);
	uint32_t picOffset = 0;
	for (int i = 0; i < spriteCount; i++) {
		uint32_t animOffset = metadata_start + (i * 44);
		Sprite sprite;
		sprite.index = i;
		sprite.x = READ_LE_INT16(data + animOffset + 0);
		sprite.y = READ_LE_INT16(data + animOffset + 2);
		sprite.w = data[animOffset + 4];
		sprite.h = data[animOffset + 5];
		sprite.extra = data[animOffset + 6];
		sprite.numAnims = data[animOffset + 8];
		sprite.zOrder = data[animOffset + 23];
		sprite.spriteType = data[animOffset + 33];
		sprite.actionFlags = data[animOffset + 34];
		sprite.isDisabled = data[animOffset + 38];
		if (sprite.numAnims == 0) {
			break;
		}
		sprite.animData = new Anim[sprite.numAnims];
		// debug("AnimSet %d has %d sub-anims, type %d, actionFlags %d, isDisabled? %d", i, animSet.numAnims, animSet.spriteType, animSet.actionFlags, animSet.isDisabled);
		int subAnimOffset = animOffset + 10;
		for (int j = 0; j < sprite.numAnims; j++) {

			Anim anim;
			anim.x = sprite.x;
			anim.y = sprite.y;
			anim.w = sprite.w;
			anim.h = sprite.h;
			anim.curFrame = 0;

			anim.nframes = data[subAnimOffset + j];
			anim.loopCount = data[subAnimOffset + 4 + j];
			anim.speed = data[subAnimOffset + 8 + j];
			anim.movementFlags = data[subAnimOffset + 14 + (j * 2)] | (data[subAnimOffset + 14 + (j * 2) + 1] << 8);

			anim.animData = new byte *[anim.nframes];
			if (anim.w > 0 && anim.h > 0 && anim.nframes > 0) {
				uint32_t needed = anim.w * anim.h * anim.nframes;
				for (int i = 0; i < anim.nframes; i++) {
					anim.animData[i] = new byte[anim.w * anim.h];
					extractSingleFrame(pixelData + picOffset, anim.animData[i], i, anim.w, anim.h);
				}
				sprite.animData[j] = anim;
				// debug("  Anim %d-%d: x=%d y=%d w=%d h=%d nframes=%d loopCount=%d speed=%d", i, j, anim.x, anim.y, anim.w, anim.h, anim.nframes, anim.loopCount, anim.speed);
				// debug("  Movement flags: 0x%04X", anim.movementFlags);
				picOffset += needed;
			} else {
				continue;
				// debug("Anim %d-%d: invalid dimensions, skipping", i, j);
			}
			sprite.animData[j] = anim;
		}

		anims.push_back(sprite);
	}
	return anims;
}

Common::Array<WalkBox> RoomManager::loadWalkboxes(byte *data, size_t size) {

	int walkboxCountOffset = 0x213;
	byte walkboxCount = data[walkboxCountOffset];

	// debug("Walkbox count: %d", walkbox_count);
	uint32_t walkboxOffset = 0x218;
	Common::Array<WalkBox> walkboxes;
	for (int i = 0; i < walkboxCount; i++) {
		uint32_t boxOffset = walkboxOffset + i * 9;
		int16 x1 = READ_LE_INT16(data + boxOffset);
		int16 y1 = READ_LE_INT16(data + boxOffset + 2);
		int16 w = READ_LE_INT16(data + boxOffset + 4);
		int16 h = READ_LE_INT16(data + boxOffset + 6);
		byte flags = data[boxOffset + 8];
		// debug("Walkbox %d: x1=%d y1=%d w=%d h=%d", i, x1, y1, w, h);
		WalkBox box;
		box.index = i;
		bool isChanged = false;
		if (g_engine->_state.roomWalkBoxChanges.contains(_currentRoomNumber)) {
			// if the walkbox has been changed, load the changed version
			for (int j = 0; j < g_engine->_state.roomWalkBoxChanges[_currentRoomNumber].size(); j++) {
				if (g_engine->_state.roomWalkBoxChanges[_currentRoomNumber][j].walkboxIndex == i) {
					walkboxes.push_back(g_engine->_state.roomWalkBoxChanges[_currentRoomNumber][j].walkbox);
					isChanged = true;
					break;
				}
			}
		}
		if (isChanged)
			continue;
		box.x = x1;
		box.y = y1;
		box.w = w;
		box.h = h;
		box.flags = flags;
		walkboxes.push_back(box);
	}

	if (g_engine->_state.roomWalkBoxChanges.contains(_currentRoomNumber)) {
		// Add any new walkboxes that were added
		for (int j = 0; j < g_engine->_state.roomWalkBoxChanges[_currentRoomNumber].size(); j++) {
			bool found = false;
			for (int i = 0; i < walkboxes.size(); i++) {
				if (g_engine->_state.roomWalkBoxChanges[_currentRoomNumber][j].walkboxIndex == walkboxes[i].index) {
					found = true;
					break;
				}
			}
			if (!found) {
				walkboxes.push_back(g_engine->_state.roomWalkBoxChanges[_currentRoomNumber][j].walkbox);
			}
		}
	}

	return walkboxes;
}

uint32 RoomManager::loadDescriptions(byte *pair12data, size_t pair12size, Common::Array<Description> &outDescriptions) {
	uint32_t pos = 0;
	uint32_t lastDescPos = 0;
	outDescriptions.clear();
	while (pos < (pair12size)) {
		int desc_pos = 0;
		if (pair12data[pos] == 0xFF) {
			Description description;

			description.itemId = pair12data[pos + 1];
			pos += 4;
			description.index = pair12data[pos++];
			description.text = "";

			while (pos < (pair12size) && pair12data[pos] != 0xFD && pos < (pair12size)) {

				if (pair12data[pos] != 0x00) {
					description.text.append(1, (char)pair12data[pos]);
				}
				if (pair12data[pos] == 0xF8) {
					description.actionTrigger = pair12data[pos + 1] | pair12data[pos + 2] << 8;
					if (description.actionTrigger != 0) {
						description.isAction = true;
					}
					pos += 2;
					break;
				}
				pos++;
			}
			outDescriptions.push_back(description);
			lastDescPos = pos;
		}
		pos++;
	}
	return lastDescPos + 1;
}

void RoomManager::loadConversationData(byte *pair12data, size_t pair12size, uint32 startPos, size_t &outConversationDataSize, byte *&outConversationData) {
	size_t conversationStart = startPos;
	outConversationDataSize = pair12size - conversationStart;
	if (outConversationData != nullptr) {
		delete[] outConversationData;
	}
	outConversationData = new byte[outConversationDataSize];
	Common::copy(pair12data + conversationStart, pair12data + conversationStart + outConversationDataSize, outConversationData);
	if (g_engine->_state.disabledBranches.contains(_currentRoomNumber)) {
		applyDisabledChoices(_currentRoomNumber, outConversationData, outConversationDataSize);
	}
}

void RoomManager::applyDisabledChoices(int roomNumber, byte *conversationData, size_t conversationDataSize) {
	Common::Array<ResetEntry> disabledBranches = g_engine->_state.disabledBranches[roomNumber];
	if (disabledBranches.size() == 0) {
		return;
	}
	debug("Disabling %d conversation branches for room %d", disabledBranches.size(), roomNumber);
	for (int i = 0; i < disabledBranches.size(); i++) {
		ResetEntry resetEntry = disabledBranches[i];
		applyDisabledChoice(resetEntry, conversationData, conversationDataSize);
	}
}

void RoomManager::applyDisabledChoice(ResetEntry entry, byte *conversationData, size_t conversationDataSize) {
	Common::copy(entry.data, entry.data + entry.dataSize, conversationData + entry.offset);
}

void RoomManager::addDisabledChoice(ChoiceOption choice) {
	debug("Adding disabled branch for room %d at offset %d", choice.room, choice.dataOffset);
	ResetEntry resetEntry = ResetEntry();
	resetEntry.room = choice.room;
	resetEntry.offset = choice.dataOffset;
	resetEntry.dataSize = 1;
	resetEntry.data = new byte[1];
	resetEntry.data[0] = 0xFA; // Disabled
	// Apply immediately
	applyDisabledChoice(resetEntry, _conversationData, _conversationDataSize);
	// Store for future loads
	g_engine->_state.addDisabledBranch(resetEntry);
}

void RoomManager::resetMetadataDefaults(byte room, byte *&data, size_t size) {
	Common::File alfred8;
	if (!alfred8.open("ALFRED.8")) {
		error("Couldnt find file ALFRED.8");
	}
	bool roomDone = false;
	while (!alfred8.eos() && !roomDone) {
		ResetEntry entry;
		entry.room = alfred8.readUint16LE();
		entry.offset = alfred8.readUint16LE();
		entry.dataSize = alfred8.readByte();
		entry.data = new byte[entry.dataSize];
		alfred8.read(entry.data, entry.dataSize);
		if (room < entry.room) {
			// We've passed the room we care about
			roomDone = true;
			break;
		}
		if (room > entry.room) {
			// Not the room we care about, skip
			continue;
		}
		debug("Resetting room %d metadata at offset %d, size %d", entry.room, entry.offset, entry.dataSize);
		Common::copy(entry.data, entry.data + entry.dataSize, data + entry.offset);
		// delete[] entry.data;
	}
	alfred8.close();
}

void RoomManager::loadRoomTalkingAnimations(int roomNumber) {

	int headerIndex = roomNumber;
	uint32 offset = kTalkingAnimHeaderSize * headerIndex;

	TalkingAnims talkHeader;
	Common::File talkFile;
	if (!talkFile.open("ALFRED.2")) {
		error("Couldnt find file ALFRED.2");
	}

	talkFile.seek(offset, SEEK_SET);

	talkHeader.spritePointer = talkFile.readUint32LE();
	talkFile.read(&talkHeader.unknown2, 3);
	talkHeader.offsetXAnimA = talkFile.readByte();
	talkHeader.offsetYAnimA = talkFile.readByte();
	talkHeader.wAnimA = talkFile.readByte();
	talkHeader.hAnimA = talkFile.readByte();
	talkFile.read(&talkHeader.unknown3, 2);
	talkHeader.numFramesAnimA = talkFile.readByte();
	talkFile.read(&talkHeader.unknown4, 5);

	talkHeader.offsetXAnimB = talkFile.readByte();
	talkHeader.offsetYAnimB = talkFile.readByte();
	talkHeader.wAnimB = talkFile.readByte();
	talkHeader.hAnimB = talkFile.readByte();
	talkFile.read(&talkHeader.unknown5, 2);
	talkHeader.numFramesAnimB = talkFile.readByte();
	talkFile.read(&talkHeader.unknown6, 29);
	// debug("Talking anim header for room %d: spritePointer=%d, wA=%d, hA=%d, framesA=%d, wB=%d, hB=%d, framesB=%d", roomNumber, talkHeader.spritePointer, talkHeader.wAnimA, talkHeader.hAnimA, talkHeader.numFramesAnimA, talkHeader.wAnimB, talkHeader.hAnimB, talkHeader.numFramesAnimB);

	if (talkHeader.spritePointer == 0) {
		debug("No talking animation for room %d", roomNumber);
		talkFile.close();
		return;
	}

	talkHeader.animA = new byte *[talkHeader.numFramesAnimA];

	byte *data = nullptr;
	int animASize = talkHeader.wAnimA * talkHeader.hAnimA * talkHeader.numFramesAnimA;
	byte *decompressed = nullptr;
	size_t dataSize = 0;
	readUntilBuda(&talkFile, talkHeader.spritePointer, data, dataSize);
	size_t decompressedSize = rleDecompress(data, dataSize, 0, dataSize, &decompressed);
	free(data);
	// debug("Decompressed talking anim A size: %zu, decompressed size: %zu", dataSize, decompressedSize);
	for (int i = 0; i < talkHeader.numFramesAnimA; i++) {
		talkHeader.animA[i] = new byte[talkHeader.wAnimA * talkHeader.hAnimA];
		extractSingleFrame(decompressed, talkHeader.animA[i], i, talkHeader.wAnimA, talkHeader.hAnimA);
	}

	if (talkHeader.numFramesAnimB > 0) {
		talkHeader.animB = new byte *[talkHeader.numFramesAnimB];
		for (int i = 0; i < talkHeader.numFramesAnimB; i++) {
			talkHeader.animB[i] = new byte[talkHeader.wAnimB * talkHeader.hAnimB];
			extractSingleFrame(decompressed + animASize, talkHeader.animB[i], i, talkHeader.wAnimB, talkHeader.hAnimB);
		}
	}
	free(decompressed);
	_talkingAnimHeader = talkHeader;

	talkFile.close();
}

ScalingParams RoomManager::loadScalingParams(byte *data, size_t size) {

	uint32_t scalingParamsOffset = 0x214;

	ScalingParams scalingParams;
	scalingParams.yThreshold = READ_LE_INT16(data + scalingParamsOffset);
	scalingParams.scaleDivisor = data[scalingParamsOffset + 2];
	scalingParams.scaleMode = data[scalingParamsOffset + 3];
	return scalingParams;
}

static uint32 readUint24(Common::ReadStream &stream) {
	uint32 value = stream.readUint16LE();
	value |= stream.readByte() << 16;
	return value;
}

byte *RoomManager::loadShadowMap(int roomNumber) {
	Common::File shadowMapFile;
	if (!shadowMapFile.open("ALFRED.5")) {
		error("Couldnt find file ALFRED.5");
	}

	uint32 entryOffset = roomNumber * 6;

	shadowMapFile.seek(entryOffset, SEEK_SET);
	uint32 shadowOffset = readUint24(shadowMapFile);

	byte *compressed = nullptr;
	size_t compressedSize = 0;
	readUntilBuda(&shadowMapFile, shadowOffset, compressed, compressedSize);

	byte *shadows = nullptr;
	size_t output = rleDecompress(compressed, compressedSize, 0, 640 * 400, &shadows);
	free(compressed);
	shadowMapFile.close();
	return shadows;
}

void RoomManager::loadRemaps(int roomNumber) {

	Common::File remapFile;
	if (!remapFile.open("ALFRED.9")) {
		error("Couldnt find file ALFRED.9");
	}

	uint32 remapOffset = 0x200 + (roomNumber * 1024);

	remapFile.seek(remapOffset, SEEK_SET);
	remapFile.read(paletteRemaps[0], 256);
	remapFile.read(paletteRemaps[1], 256);
	remapFile.read(paletteRemaps[2], 256);
	remapFile.read(paletteRemaps[3], 256);
	remapFile.close();
}

Common::StringArray RoomManager::loadRoomNames() {
	Common::StringArray roomNames;
	Common::File juegoExe;
	if (!juegoExe.open(Common::Path("JUEGO.EXE"))) {
		error("Couldnt find file JUEGO.EXE");
	}

	size_t namesSize = 1335;
	juegoExe.seek(0x49315, SEEK_SET);
	byte *namesData = new byte[namesSize];
	juegoExe.read(namesData, namesSize);
	uint32 pos = 0;
	Common::String currentName = "";
	while (pos < namesSize) {
		if (namesData[pos] == 0xFD &&
			namesData[pos + 1] == 0x00 &&
			namesData[pos + 2] == 0x08 &&
			namesData[pos + 3] == 0x02) {
			if (currentName.size() > 0) {
				roomNames.push_back(currentName);
			}
			currentName = "";
			pos += 4;
			continue;
		}
		currentName += (char)namesData[pos];
		pos++;
	}
	delete[] namesData;
	juegoExe.close();
	return roomNames;
}

byte RoomManager::loadMusicTrackForRoom(Common::File *roomFile, int roomOffset) {
	uint32_t pair9offset = roomOffset + (9 * 8);
	roomFile->seek(pair9offset, SEEK_SET);
	uint32_t pair9_data_offset = roomFile->readUint32LE();
	uint32_t pair9_size = roomFile->readUint32LE();

	roomFile->seek(pair9_data_offset, SEEK_SET);
	byte musicTrack = roomFile->readByte();
	// debug("Music track for room at offset %d is %d", roomOffset, musicTrack);
	return musicTrack > 0 ? musicTrack + 1 : 0;
}

Common::Array<byte> RoomManager::loadRoomSfx(Common::File *roomFile, int roomOffset) {
	uint32_t pair9offset = roomOffset + (9 * 8);
	roomFile->seek(pair9offset, SEEK_SET);
	uint32_t pair9_data_offset = roomFile->readUint32LE();
	uint32_t pair9_size = roomFile->readUint32LE();

	roomFile->seek(pair9_data_offset, SEEK_SET);
	roomFile->skip(1); // skip music track byte
	Common::Array<byte> roomSfx(kNumSfxPerRoom);
	for (int i = 0; i < kNumSfxPerRoom; i++) {
		byte sfx = roomFile->readByte();
		roomSfx[i] = sfx;
		// debug("SFX %d for room at offset %d is %d (%s)", i, roomOffset, sfx, SOUND_FILENAMES[sfx]);
	}
	return roomSfx;
}

} // End of namespace Pelrock
