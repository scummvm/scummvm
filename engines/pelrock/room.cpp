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
	loadWaterPaletteRemap();
}

RoomManager::~RoomManager() {
	if (_pixelsShadows != nullptr) {
		delete[] _pixelsShadows;
		_pixelsShadows = nullptr;
	}
	delete[] _resetData;
}

void RoomManager::loadWaterPaletteRemap() {
	//Extra remap for water effect
	Common::File exe;
	if(!exe.open("JUEGO.EXE")) {
		error("Couldnt find file JUEGO.EXE");
	}
	exe.seek(0x4C77C, SEEK_SET);
	exe.read(_paletteRemaps[4], 256);
	exe.close();
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

void RoomManager::addSticker(int stickerId, int persist) {
	addStickerToRoom(_currentRoomNumber, stickerId, persist);
}

void RoomManager::addStickerToRoom(byte room, int stickerId, int persist) {
	Sticker sticker = g_engine->_res->getSticker(stickerId);
	if (room == _currentRoomNumber && persist & PERSIST_TEMP) {
		_roomStickers.push_back(sticker);
	}
	if (persist & PERSIST_PERM) {
		g_engine->_state->stickersPerRoom[room].push_back(sticker);
	}
}

void RoomManager::removeSticker(int stickerId) {
	removeStickerFromRoom(_currentRoomNumber, stickerId);
}

void RoomManager::removeStickerFromRoom(byte room, int stickerId) {
	// First check and remove from room stickers
	for (uint i = 0; i < _roomStickers.size(); i++) {
		if (_roomStickers[i].stickerIndex == stickerId) {
			_roomStickers.remove_at(i);
			return;
		}
	}

	// Then check and remove from persisted stickers
	for (uint i = 0; i < g_engine->_state->stickersPerRoom[room].size(); i++) {
		if (g_engine->_state->stickersPerRoom[room][i].stickerIndex == stickerId) {
			g_engine->_state->stickersPerRoom[room].remove_at(i);
			return;
		}
	}
}

bool RoomManager::hasSticker(int index) {
	return hasSticker(_currentRoomNumber, index);
}

bool RoomManager::hasSticker(byte room, int index) {
	for (uint i = 0; i < _roomStickers.size(); i++) {
		if (_roomStickers[i].stickerIndex == index) {
			return true;
		}
	}

	if (room != _currentRoomNumber) {
		return false;
	}

	for (uint i = 0; i < g_engine->_state->stickersPerRoom[room].size(); i++) {
		if (g_engine->_state->stickersPerRoom[room][i].stickerIndex == index) {
			return true;
		}
	}

	return false;
}

void RoomManager::changeExit(byte index, bool enabled, int persist) {
	changeExit(_currentRoomNumber, index, enabled, persist);
}

void RoomManager::changeExit(byte room, byte index, bool enabled, int persist) {
	if (room == _currentRoomNumber && persist & PERSIST_TEMP) {
		_currentRoomExits[index].isEnabled = enabled;
	}
	if (persist & PERSIST_PERM)
		g_engine->_state->roomExitChanges[room].push_back({room, index, enabled});
}

void RoomManager::disableExit(byte index, int persist) {
	changeExit(index, false, persist);
}

void RoomManager::disableExit(byte room, byte index, int persist) {
	changeExit(room, index, false, persist);
}

void RoomManager::enableExit(byte index, int persist) {
	changeExit(index, true, persist);
}

void RoomManager::enableExit(byte room, byte index, int persist) {
	changeExit(room, index, true, persist);
}

void RoomManager::changeWalkBox(WalkBox walkbox, int persist) {
	changeWalkbox(_currentRoomNumber, walkbox, persist);
}

void RoomManager::changeWalkbox(byte room, WalkBox walkbox, int persist) {
	if (room == _currentRoomNumber && persist & PERSIST_TEMP) {
		_currentRoomWalkboxes[walkbox.index] = walkbox;
	}
	if (persist & PERSIST_PERM) {
		g_engine->_state->roomWalkBoxChanges[room].push_back({room, walkbox.index, walkbox});
	}
}

void RoomManager::changeHotSpot(HotSpot hotspot, int persist) {
	changeHotspot(_currentRoomNumber, hotspot, persist);
}

void RoomManager::changeHotspot(byte room, HotSpot hotspot, int persist) {
	if (room == _currentRoomNumber && persist & PERSIST_TEMP) {
		for (uint i = 0; i < _currentRoomHotspots.size(); i++) {
			if (!_currentRoomHotspots[i].isSprite && _currentRoomHotspots[i].innerIndex == hotspot.innerIndex) {
				_currentRoomHotspots[i] = hotspot;
				break;
			}
		}
	}
	if (persist & PERSIST_PERM) {
		g_engine->_state->roomHotSpotChanges[room].push_back({_currentRoomNumber, hotspot.innerIndex, hotspot});
	}
}

void RoomManager::disableSprite(byte spriteIndex, int persist) {
	disableSprite(_currentRoomNumber, spriteIndex, persist);
}

void RoomManager::disableSprite(byte roomNumber, byte spriteIndex, int persist) {
	if (roomNumber == _currentRoomNumber && persist & PERSIST_TEMP) {
		_currentRoomAnims[spriteIndex].zOrder = 255;
		_currentRoomAnims[spriteIndex].isHotspotDisabled = true;
	}
	if (persist & PERSIST_PERM) {
		g_engine->_state->spriteChanges[roomNumber].push_back({roomNumber, spriteIndex, 255});
	}
}

void RoomManager::enableSprite(byte spriteIndex, byte zOrder, int persist) {
	enableSprite(_currentRoomNumber, spriteIndex, zOrder, persist);
}

void RoomManager::enableSprite(byte roomNumber, byte spriteIndex, byte zOrder, int persist) {
	if (roomNumber == _currentRoomNumber && persist & PERSIST_TEMP) {
		_currentRoomAnims[spriteIndex].zOrder = zOrder;
	}
	if (persist & PERSIST_PERM) {
		g_engine->_state->spriteChanges[roomNumber].push_back({roomNumber, spriteIndex, zOrder});
	}
}

void RoomManager::enableHotspot(HotSpot *hotspot, int persist) {
	enableHotspot(_currentRoomNumber, hotspot, persist);
}

void RoomManager::enableHotspot(byte room, HotSpot *hotspot, int persist) {
	if (persist & PERSIST_TEMP && room == _currentRoomNumber) {
		hotspot->isEnabled = true;
	}
	if (persist & PERSIST_PERM) {
		changeHotspot(room, *hotspot);
	}
}

void RoomManager::disableHotspot(HotSpot *hotspot, int persist) {
	disableHotspot(_currentRoomNumber, hotspot, persist);
}

void RoomManager::disableHotspot(byte room, HotSpot *hotspot, int persist) {
	if (persist & PERSIST_TEMP && room == _currentRoomNumber) {
		hotspot->isEnabled = false;
	}
	if (persist & PERSIST_PERM) {
		changeHotspot(room, *hotspot);
	}
}

void RoomManager::moveHotspot(HotSpot *hotspot, int16 newX, int16 newY, int persist) {
	if (persist & PERSIST_TEMP) {
		hotspot->x = newX;
		hotspot->y = newY;
	}
	if (persist & PERSIST_PERM) {
		changeHotspot(_currentRoomNumber, *hotspot, persist);
	}
}

void RoomManager::setActionMask(HotSpot *hotspot, byte actionMask, int persist) {
	if (persist & PERSIST_TEMP) {
		hotspot->actionFlags = actionMask;
	}
	if (persist & PERSIST_PERM) {
		changeHotspot(_currentRoomNumber, *hotspot, persist);
	}
}

void RoomManager::addWalkbox(WalkBox walkbox, int persist) {
	if (persist & PERSIST_TEMP) {
		_currentRoomWalkboxes.push_back(walkbox);
	}
	if (persist & PERSIST_PERM) {
		g_engine->_state->roomWalkBoxChanges[_currentRoomNumber].push_back({_currentRoomNumber, walkbox.index, walkbox});
	}
}

Sprite *RoomManager::findSpriteByIndex(byte index) {
	for (uint i = 0; i < _currentRoomAnims.size(); i++) {
		if (_currentRoomAnims[i].index == index) {
			return &_currentRoomAnims[i];
		}
	}
	return nullptr;
}

HotSpot *RoomManager::findHotspotByIndex(byte index) {
	for (uint i = 0; i < _currentRoomHotspots.size(); i++) {
		if (!_currentRoomHotspots[i].isSprite && _currentRoomHotspots[i].innerIndex == index) {
			debug("Found hotspot %d at index %d, extra = %d", index, i, _currentRoomHotspots[i].extra);
			return &_currentRoomHotspots[i];
		}
	}
	return nullptr;
}

HotSpot *RoomManager::findHotspotByExtra(uint16 extra) {
	for (uint i = 0; i < _currentRoomHotspots.size(); i++) {
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
	case 9:
		offset = 0x0004B874;
		break;
	case 17:
		offset = 0x0004B86C;
		break;
	case 18:
		offset = 0x0004B870;
		break;
	case 19:
		offset = 0x0004B878;
		break;
	case 21:
		offset = 0x0004B884;
		break;
	case 25:
		offset = 0x0004B890;
		break;
	case 32:
		offset = 0x0004B898;
		break;
	case 33:
		offset = 0x0004B89C;
		break;
	case 38:
		offset = 0x0004B894;
		break;
	case 39:
		offset = 0x0004B888;
		break;
	case 46:
		offset = 0x0004B8A0;
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
		// FADE mode: shift RGB values to convert from 6-bit VGA to 8-bit
		// data[0-2] = current R,G,B
		// data[3-5] = min R,G,B
		// data[6-8] = max R,G,B
		// data[9] = flags (R/G/B increments + direction) - NOT shifted
		for (int i = 0; i < 9; i++) {
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

		if (g_engine->_state->roomExitChanges.contains(_currentRoomNumber)) {
			// if the exit has been changed, load the changed version
			for (int j = 0; j < g_engine->_state->roomExitChanges[_currentRoomNumber].size(); j++) {
				if (g_engine->_state->roomExitChanges[_currentRoomNumber][j].exitIndex == i) {
					exit.isEnabled = g_engine->_state->roomExitChanges[_currentRoomNumber][j].enabled;
					break;
				}
			}
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
		if (g_engine->_state->roomHotSpotChanges.contains(_currentRoomNumber)) {
			// if the hotspot has been changed, load the changed version
			for (int j = 0; j < g_engine->_state->roomHotSpotChanges[_currentRoomNumber].size(); j++) {
				if (g_engine->_state->roomHotSpotChanges[_currentRoomNumber][j].hotspotIndex == spot.innerIndex) {
					debug("Hotspot %d has been changed, loading changed version, Hotspot x=%d, y = %d, extra = %d", spot.innerIndex, g_engine->_state->roomHotSpotChanges[_currentRoomNumber][j].hotspot.x, g_engine->_state->roomHotSpotChanges[_currentRoomNumber][j].hotspot.y, g_engine->_state->roomHotSpotChanges[_currentRoomNumber][j].hotspot.extra);
					hotspots.push_back(g_engine->_state->roomHotSpotChanges[_currentRoomNumber][j].hotspot);
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
		debug("Hotspot %d: type=%d x=%d y=%d w=%d h=%d extra=%d, index =%d, isEnabled=%d", spot.innerIndex, spot.actionFlags, spot.x, spot.y, spot.w, spot.h, spot.extra, spot.innerIndex, spot.isEnabled);
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
		Common::copy(entry.data, entry.data + entry.dataSize, conversationData + entry.offset);
		// delete[] entry.data;
	}
	alfredB.close();
}

void RoomManager::loadRoomMetadata(Common::File *roomFile, int roomNumber) {

	_roomStickers.clear();
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
	_roomStickers = g_engine->_state->stickersPerRoom[roomNumber];
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

	PaletteAnim *anim = getPaletteAnimForRoom(roomNumber);
	if (anim != nullptr) {
		if (_currentPaletteAnim != nullptr) {
			delete _currentPaletteAnim;
		}
		_currentPaletteAnim = anim;
	} else {
		_currentPaletteAnim = nullptr;
	}

	if (_passerByAnims != nullptr) {
		delete _passerByAnims;
	}
	_passerByAnims = loadPasserByAnims(roomNumber);

	delete[] pair10;
	delete[] pair12;
}

int streetWalkerIndices[] = {
	-1, // room 0,
	5,  // room 1,
	3,  // room 2,
	6,  // room 3,
	-1, // room 4,
	-1, // room 5,
	-1, // room 6,
	-1, // room 7,
	7,  // room 8,
	-1, // room 9,
	-1, // room 10,
	-1, // room 11,
	-1, // room 12,
	-1, // room 13,
	2,  // room 14,
	-1, // room 15,
	2

};
RoomPasserBys *RoomManager::loadPasserByAnims(int roomNumber) {
	RoomPasserBys *anims = nullptr;
	switch (roomNumber) {
	case 9: {
		Sprite *mouse = findSpriteByIndex(2);
		Sprite *blank = findSpriteByIndex(4);
		mouse->animData[0].loopCount = 3;
		mouse->animData[1].loopCount = 1;
		mouse->animData[1].movementFlags = 0x3FF;
		mouse->animData[2].loopCount = 1;
		mouse->animData[2].movementFlags = 0x801F;
		mouse->animData[3].loopCount = 4;
		mouse->animData[3].movementFlags = 0x3C0;

		anims = new RoomPasserBys(roomNumber, 1);
		PasserByAnim anim;
		anim.spriteIndex = 2;
		anim.startX = mouse->x;
		anim.startY = mouse->y;
		anim.dir = PASSERBY_DOWN;
		anim.targetZIndex = blank->zOrder + 1;
		anim.resetCoord = blank->y;
		anims->passerByAnims[0] = anim;
		debug("Loaded passerby animation for room %d, direction = %d", roomNumber, anim.dir);
		break;
	}

	case 1:
	case 2:
	case 3:
	case 8:
	case 14:
	case 16: {
		anims = new RoomPasserBys(roomNumber, 1);
		PasserByAnim anim;
		anim.spriteIndex = streetWalkerIndices[roomNumber];
		Sprite *camel = findSpriteByIndex(anim.spriteIndex);
		anim.startX = camel->x;
		anim.startY = camel->y;
		anim.dir = PASSERBY_RIGHT;
		anim.frameTrigger = 0x1FFF;
		anim.targetZIndex = 1;
		anim.resetCoord = 639 + camel->w;

		anims->passerByAnims[0] = anim;
		break;
	}
	case 21: {
		anims = new RoomPasserBys(roomNumber, 1);
		PasserByAnim anim;
		anim.spriteIndex = 3;
		Sprite *camel = findSpriteByIndex(3);
		anim.startX = camel->x;
		anim.startY = camel->y;
		anim.dir = PASSERBY_LEFT;
		anim.resetCoord = 0 - camel->w;
		anim.targetZIndex = 1;

		anims->passerByAnims[0] = anim;
		break;
	}
	case 29: {
		Sprite *carLeft = findSpriteByIndex(2);
		Sprite *carRight = findSpriteByIndex(3);

		anims = new RoomPasserBys(roomNumber, 2);
		PasserByAnim animA;
		animA.spriteIndex = 2;
		animA.startX = carLeft->x;
		animA.startY = carLeft->y;
		animA.dir = PASSERBY_LEFT;
		animA.resetCoord = carRight->x + carRight->w - carLeft->w;
		animA.targetZIndex = 100;

		anims->passerByAnims[0] = animA;
		PasserByAnim animB;
		animB.spriteIndex = 3;
		animB.startX = carRight->x;
		animB.startY = carRight->y;
		animB.dir = PASSERBY_RIGHT;
		animB.targetZIndex = 100;
		animB.resetCoord = 639 + carRight->w;
		anims->passerByAnims[1] = animB;
		break;
	}
	case 31: {

		anims = new RoomPasserBys(roomNumber, 1);
		Sprite *walker = findSpriteByIndex(2);
		Sprite *dark = findSpriteByIndex(5);
		PasserByAnim anim;
		anim.spriteIndex = 2;
		anim.startX = walker->x;
		anim.startY = walker->y;
		anim.dir = PASSERBY_RIGHT;
		anim.resetCoord = dark->x;
		anim.targetZIndex = dark->zOrder + 1;
		anims->passerByAnims[0] = anim;
		break;
	}
	case 46: {
		Sprite *catRight = findSpriteByIndex(2);
		Sprite *catLeft = findSpriteByIndex(3);
		Sprite *blank = findSpriteByIndex(0);
		anims = new RoomPasserBys(roomNumber, 2);
		PasserByAnim animA;
		animA.spriteIndex = 2;
		animA.startX = catRight->x;
		animA.startY = catRight->y;
		animA.dir = PASSERBY_RIGHT;
		animA.resetCoord = catLeft->x;
		animA.targetZIndex = blank->zOrder + 1;

		anims->passerByAnims[0] = animA;
		PasserByAnim animB;
		animB.spriteIndex = 3;
		animB.startX = catLeft->x;
		animB.startY = catLeft->y;
		animB.dir = PASSERBY_LEFT;
		animB.resetCoord = blank->x;
		animB.targetZIndex = blank->zOrder + 1;
		anims->passerByAnims[1] = animB;
		break;
	}
	case 47: {
		Sprite *mouseRight = findSpriteByIndex(3);
		Sprite *mouseLeft = findSpriteByIndex(4);
		Sprite *papers = findSpriteByIndex(1);

		anims = new RoomPasserBys(roomNumber, 2);
		PasserByAnim animA;
		animA.spriteIndex = 3;
		animA.startX = mouseRight->x;
		animA.startY = mouseRight->y;
		animA.dir = PASSERBY_RIGHT;
		animA.resetCoord = mouseLeft->x;
		animA.targetZIndex = papers->zOrder + 1;
		anims->passerByAnims[0] = animA;

		PasserByAnim animB;
		animB.spriteIndex = 4;
		animB.startX = mouseLeft->x;
		animB.startY = mouseLeft->y;
		animB.dir = PASSERBY_LEFT;
		animB.resetCoord = mouseRight->x;
		animB.targetZIndex = papers->zOrder + 1;
		anims->passerByAnims[1] = animB;
		break;
	}
	case 50: {
		Sprite *mummyLeft = findSpriteByIndex(2);
		Sprite *mummyRight = findSpriteByIndex(3);

		anims = new RoomPasserBys(roomNumber, 2);
		PasserByAnim animA;
		animA.spriteIndex = 2;
		animA.startX = mummyLeft->x;
		animA.startY = mummyLeft->y;
		animA.dir = PASSERBY_LEFT;
		animA.resetCoord = 0 - mummyLeft->w;
		animA.targetZIndex = 1;

		anims->passerByAnims[0] = animA;
		PasserByAnim animB;
		animB.spriteIndex = 3;
		animB.startX = mummyRight->x;
		animB.startY = mummyRight->y;
		animB.dir = PASSERBY_RIGHT;
		animB.targetZIndex = 1;
		animB.resetCoord = 639 + mummyRight->w;
		anims->passerByAnims[1] = animB;
		break;
	}
	default:
		break;
	}
	if (anims != nullptr)
		debug("Loaded passerby anims for room %d, count = %d", roomNumber, anims->numAnims);
	return anims;
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
		thisHotspot.isEnabled = !anims[i].isHotspotDisabled;
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
		if(_currentRoomNumber != 40) {
			outSize = rleDecompress(pixelData, size, 0, size, &buffer, true);
		}
		else {
			// room 40 has uncompressed animation data for some reason
			buffer = new byte[size];
			Common::copy(pixelData, pixelData + size, buffer);
			outSize = size;
		}
	}


}

Common::Array<Sprite> RoomManager::loadRoomAnimations(byte *pixelData, size_t pixelDataSize, byte *data, size_t size) {

	Common::Array<Sprite> anims = Common::Array<Sprite>();
	uint32_t spriteCountPos = 5;
	byte spriteCount = data[spriteCountPos] - 2;
	debug("Sprite count: %d", spriteCount);
	uint32_t metadata_start = spriteCountPos + (44 * 2 + 5);
	uint32_t picOffset = 0;

	Common::Array<SpriteChange> spriteChanges = g_engine->_state->spriteChanges[_currentRoomNumber];

	for (int i = 0; i < spriteCount; i++) {
		uint32_t animOffset = metadata_start + (i * 44);
		Sprite sprite;
		sprite.index = i;
		sprite.x = READ_LE_INT16(data + animOffset + 0);
		sprite.y = READ_LE_INT16(data + animOffset + 2);
		sprite.w = data[animOffset + 4];
		sprite.h = data[animOffset + 5];
		sprite.stride = READ_LE_INT16(data + animOffset + 6);
		sprite.numAnims = data[animOffset + 8];
		sprite.zOrder = data[animOffset + 23];
		sprite.extra = data[animOffset + 32];
		sprite.spriteType = data[animOffset + 33];
		sprite.actionFlags = data[animOffset + 34];
		sprite.isHotspotDisabled = data[animOffset + 38];
		for (int j = 0; j < spriteChanges.size(); j++) {
			if (spriteChanges[j].spriteIndex == sprite.index) {
				sprite.zOrder = spriteChanges[j].zIndex;
				break;
			}
		}
		if (sprite.numAnims == 0) {
			break;
		}
		sprite.animData = new Anim[sprite.numAnims];
		// debug("Sprite %d has %d sub-anims, type %d, actionFlags %d, isDisabled? %d", i, sprite.numAnims, sprite.spriteType, sprite.actionFlags, sprite.isHotspotDisabled);
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
					// debug("Extracting frame %d for anim %d-%d, w=%d h=%d, pixelDataSize=%d, current offset %d", i, j, anim.nframes, anim.w, anim.h, pixelDataSize, picOffset);
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
		if (g_engine->_state->roomWalkBoxChanges.contains(_currentRoomNumber)) {
			// if the walkbox has been changed, load the changed version
			for (int j = 0; j < g_engine->_state->roomWalkBoxChanges[_currentRoomNumber].size(); j++) {
				if (g_engine->_state->roomWalkBoxChanges[_currentRoomNumber][j].walkboxIndex == i) {
					walkboxes.push_back(g_engine->_state->roomWalkBoxChanges[_currentRoomNumber][j].walkbox);
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

	if (g_engine->_state->roomWalkBoxChanges.contains(_currentRoomNumber)) {
		// Add any new walkboxes that were added
		for (int j = 0; j < g_engine->_state->roomWalkBoxChanges[_currentRoomNumber].size(); j++) {
			bool found = false;
			for (int i = 0; i < walkboxes.size(); i++) {
				if (g_engine->_state->roomWalkBoxChanges[_currentRoomNumber][j].walkboxIndex == walkboxes[i].index) {
					found = true;
					break;
				}
			}
			if (!found) {
				walkboxes.push_back(g_engine->_state->roomWalkBoxChanges[_currentRoomNumber][j].walkbox);
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
			// Hardcoded fix in the original!
			if (_currentRoomNumber == 3 && description.text.size() == 1 && description.text[0] == 0x2D) {
				outDescriptions.push_back(description);
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
	if (g_engine->_state->disabledBranches.contains(_currentRoomNumber)) {
		applyDisabledChoices(_currentRoomNumber, outConversationData, outConversationDataSize);
	}
}

void RoomManager::applyDisabledChoices(byte roomNumber, byte *conversationData, size_t conversationDataSize) {
	Common::Array<ResetEntry> disabledBranches = g_engine->_state->disabledBranches[roomNumber];
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
	// Write 0xFA at offset+2 (after FB/F1 marker and level byte)
	// This marks the choice as disabled without destroying the marker structure
	uint32 disableOffset = choice.dataOffset + 2;
	debug("Adding disabled branch for room %d at offset %d (FA written at %d)",
		  choice.room, choice.dataOffset, disableOffset);
	ResetEntry resetEntry = ResetEntry();
	resetEntry.room = choice.room;
	resetEntry.offset = disableOffset;
	resetEntry.dataSize = 1;
	resetEntry.data = new byte[1];
	resetEntry.data[0] = 0xFA; // Disabled marker
	// Apply immediately
	applyDisabledChoice(resetEntry, _conversationData, _conversationDataSize);
	// Store for future loads
	g_engine->_state->addDisabledBranch(resetEntry);
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
	debug("Decompressed talking anim A size: %zu, decompressed size: %zu", dataSize, decompressedSize);
	for (int i = 0; i < talkHeader.numFramesAnimA; i++) {
		talkHeader.animA[i] = new byte[talkHeader.wAnimA * talkHeader.hAnimA];
		extractSingleFrame(decompressed, talkHeader.animA[i], i, talkHeader.wAnimA, talkHeader.hAnimA);
	}

	if (talkHeader.numFramesAnimB > 0) {
		talkHeader.animB = new byte *[talkHeader.numFramesAnimB];
		for (int i = 0; i < talkHeader.numFramesAnimB; i++) {
			talkHeader.animB[i] = new byte[talkHeader.wAnimB * talkHeader.hAnimB];
			uint32 offset = animASize + (i * talkHeader.wAnimB * talkHeader.hAnimB);
			debug("Extracting talking anim B frame %d at offset %d, size = %d", i, animASize + (i * talkHeader.wAnimB * talkHeader.hAnimB), talkHeader.wAnimB * talkHeader.hAnimB);
			if (offset + talkHeader.wAnimB * talkHeader.hAnimB >= decompressedSize) {
				debug("Error: offset %d is beyond decompressed size %zu", offset, decompressedSize);
				talkHeader.numFramesAnimB = 0;
			} else {
				extractSingleFrame(decompressed + animASize, talkHeader.animB[i], i, talkHeader.wAnimB, talkHeader.hAnimB);
			}
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
	rleDecompress(compressed, compressedSize, 0, 640 * 400, &shadows);
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
	remapFile.read(_paletteRemaps[0], 256);
	remapFile.read(_paletteRemaps[1], 256);
	remapFile.read(_paletteRemaps[2], 256);
	remapFile.read(_paletteRemaps[3], 256);
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

	roomFile->seek(pair9_data_offset, SEEK_SET);
	byte musicTrack = roomFile->readByte();
	// debug("Music track for room at offset %d is %d", roomOffset, musicTrack);
	return musicTrack > 0 ? musicTrack + 1 : 0;
}

Common::Array<byte> RoomManager::loadRoomSfx(Common::File *roomFile, int roomOffset) {
	uint32_t pair9offset = roomOffset + (9 * 8);
	roomFile->seek(pair9offset, SEEK_SET);
	uint32_t pair9_data_offset = roomFile->readUint32LE();

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
