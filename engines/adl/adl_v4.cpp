/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "adl/adl_v4.h"
#include "adl/detection.h"

namespace Adl {

AdlEngine_v4::AdlEngine_v4(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine_v3(syst, gd),
		_currentVolume(0),
		_itemPicIndex(nullptr) {

}

AdlEngine_v4::~AdlEngine_v4() {
	delete _itemPicIndex;
}

Common::String AdlEngine_v4::loadMessage(uint idx) const {
	Common::String str = AdlEngine_v3::loadMessage(idx);

	for (uint i = 0; i < str.size(); ++i) {
		const char *xorStr = "AVISDURGAN";
		str.setChar(str[i] ^ xorStr[i % strlen(xorStr)], i);
	}

	return str;
}

Common::String AdlEngine_v4::getItemDescription(const Item &item) const {
	return _itemDesc[item.id - 1];
}

DiskImage *AdlEngine_v4::loadDisk(byte volume) const {
	const ADGameFileDescription *ag;

	for (ag = _gameDescription->desc.filesDescriptions; ag->fileName; ag++) {
		if (ag->fileType == volume) {
			DiskImage *disk = new DiskImage();
			if (!disk->open(ag->fileName))
				error("Failed to open %s", ag->fileName);
			return disk;
		}
	}

	error("Disk volume %d not found", volume);
}

void AdlEngine_v4::insertDisk(byte volume) {
	delete _disk;
	_disk = loadDisk(volume);
	_currentVolume = volume;
}

void AdlEngine_v4::loadRegionLocations(Common::ReadStream &stream, uint regions) {
	for (uint r = 0; r < regions; ++r) {
		RegionLocation loc;
		loc.track = stream.readByte();
		loc.sector = stream.readByte();

		if (stream.eos() || stream.err())
			error("Failed to read region locations");

		_regionLocations.push_back(loc);
	}
}

void AdlEngine_v4::loadRegionInitDataOffsets(Common::ReadStream &stream, uint regions) {
	for (uint r = 0; r < regions; ++r) {
		RegionInitDataOffset initOfs;
		initOfs.track = stream.readByte();
		initOfs.sector = stream.readByte();
		initOfs.offset = stream.readByte();
		initOfs.volume = stream.readByte();

		if (stream.eos() || stream.err())
			error("Failed to read region init data offsets");

		_regionInitDataOffsets.push_back(initOfs);
	}
}

void AdlEngine_v4::initRegions(const byte *roomsPerRegion, uint regions) {
	_state.regions.resize(regions);

	for (uint r = 0; r < regions; ++r) {
		Region &regn = _state.regions[r];
		// Each region has 24 variables
		regn.vars.resize(24);

		regn.rooms.resize(roomsPerRegion[r]);
		for (uint rm = 0; rm < roomsPerRegion[r]; ++rm) {
			// TODO: hires6 uses 0xff and has slightly different
			// code working on these values
			regn.rooms[rm].picture = 1;
			regn.rooms[rm].isFirstTime = 1;
		}
	}
}

void AdlEngine_v4::fixupDiskOffset(byte &track, byte &sector) const {
	if (_state.region == 0)
		return;

	sector += _regionLocations[_state.region - 1].sector;
	if (sector >= 16) {
		sector -= 16;
		++track;
	}

	track += _regionLocations[_state.region - 1].track;
}

void AdlEngine_v4::adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const {
	fixupDiskOffset(track, sector);
}

void AdlEngine_v4::loadRegion(byte region) {
	if (_currentVolume != _regionInitDataOffsets[region - 1].volume) {
		insertDisk(_regionInitDataOffsets[region - 1].volume);

		// FIXME: This shouldn't be needed, but currently is, due to
		// implementation choices made earlier on for DataBlockPtr and DiskImage.
		_state.region = 0; // To avoid region offset being applied
		_itemPics.clear();
		_itemPicIndex->seek(0);
		loadItemPictures(*_itemPicIndex, _itemPicIndex->size() / 5);
	}

	_state.region = region;

	byte track = _regionInitDataOffsets[region - 1].track;
	byte sector = _regionInitDataOffsets[region - 1].sector;
	uint offset = _regionInitDataOffsets[region - 1].offset;

	fixupDiskOffset(track, sector);

	for (uint block = 0; block < 7; ++block) {
		StreamPtr stream(_disk->createReadStream(track, sector, offset, 1));

		uint16 addr = stream->readUint16LE();
		uint16 size = stream->readUint16LE();

		stream.reset(_disk->createReadStream(track, sector, offset, size / 256 + 1));
		stream->skip(4);

		switch (addr) {
		case 0x9000: {
			// Messages
			_messages.clear();
			uint count = size / 4;
			loadMessages(*stream, count);
			break;
		}
		case 0x4a80: {
			// Global pics
			_pictures.clear();
			loadPictures(*stream);
			break;
		}
		case 0x4000:
			// Verbs
			loadWords(*stream, _verbs, _priVerbs);
			break;
		case 0x1800:
			// Nouns
			loadWords(*stream, _nouns, _priNouns);
			break;
		case 0x0e00: {
			// Rooms
			uint count = size / 14 - 1;
			stream->skip(14); // Skip invalid room 0

			_state.rooms.clear();
			loadRooms(*stream, count);
			break;
		}
		case 0x7b00:
			// TODO: hires6 has global and room lists swapped
			// Room commands
			readCommands(*stream, _roomCommands);
			break;
		case 0x9500:
			// Global commands
			readCommands(*stream, _globalCommands);
			break;
		default:
			error("Unknown data block found (addr %04x; size %04x)", addr, size);
		}

		offset += 4 + size;
		while (offset >= 256) {
			offset -= 256;
			++sector;
			if (sector >= 16) {
				sector = 0;
				++track;
			}
		}
	}

	applyRegionWorkarounds();
	restoreVars();
}

void AdlEngine_v4::loadItemPicIndex(Common::ReadStream &stream, uint items) {
	_itemPicIndex = stream.readStream(items * 5);

	if (stream.eos() || stream.err())
		error("Error reading item index");
}

void AdlEngine_v4::backupRoomState(byte room) {
	RoomState &backup = getCurRegion().rooms[room - 1];

	backup.isFirstTime = getRoom(room).isFirstTime;
	backup.picture = getRoom(room).picture;
}

void AdlEngine_v4::restoreRoomState(byte room) {
	const RoomState &backup = getCurRegion().rooms[room - 1];

	getRoom(room).isFirstTime = backup.isFirstTime;
	getRoom(room).picture = backup.picture;
}

void AdlEngine_v4::backupVars() {
	Region &region = getCurRegion();

	for (uint i = 0; i < region.vars.size(); ++i)
		region.vars[i] = getVar(i);
}

void AdlEngine_v4::restoreVars() {
	const Region &region = getCurRegion();

	for (uint i = 0; i < region.vars.size(); ++i)
		setVar(i, region.vars[i]);
}

void AdlEngine_v4::switchRegion(byte region) {
	backupVars();
	backupRoomState(_state.room);
	_state.prevRegion = _state.region;
	_state.region = region;
	loadRegion(region);
	_state.room = 1;
	_picOnScreen = _roomOnScreen = 0;
}

void AdlEngine_v4::switchRoom(byte roomNr) {
	getCurRoom().curPicture = getCurRoom().picture;
	getCurRoom().isFirstTime = false;
	backupRoomState(_state.room);
	_state.room = roomNr;
	restoreRoomState(_state.room);
}

int AdlEngine_v4::o4_isItemInRoom(ScriptEnv &e) {
	OP_DEBUG_2("\t&& GET_ITEM_ROOM(%s) == %s", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	const Item &item = getItem(e.arg(1));

	if (e.arg(2) != IDI_ANY && item.region != _state.region)
		return -1;

	if (item.room == roomArg(e.arg(2)))
		return 2;

	return -1;
}

int AdlEngine_v4::o4_isVarGT(ScriptEnv &e) {
	OP_DEBUG_2("\t&& VARS[%d] > %d", e.arg(1), e.arg(2));

	if (getVar(e.arg(1)) > e.arg(2))
		return 2;

	return -1;
}

int AdlEngine_v4::o4_moveItem(ScriptEnv &e) {
	o2_moveItem(e);
	getItem(e.arg(1)).region = _state.region;
	return 2;
}

int AdlEngine_v4::o4_setRegionToPrev(ScriptEnv &e) {
	OP_DEBUG_0("\tREGION = PREV_REGION");

	switchRegion(_state.prevRegion);
	// Long jump
	_isRestarting = true;
	return -1;
}

int AdlEngine_v4::o4_moveAllItems(ScriptEnv &e) {
	OP_DEBUG_2("\tMOVE_ALL_ITEMS(%s, %s)", itemRoomStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	byte room1 = roomArg(e.arg(1));

	if (room1 == _state.room)
		_picOnScreen = 0;

	byte room2 = roomArg(e.arg(2));

	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (room1 != item->room)
			continue;

		if (room1 != IDI_ANY) {
			if (_state.region != item->region)
				continue;
			if (room2 == IDI_ANY) {
				if (isInventoryFull())
					break;
				if (item->state == IDI_ITEM_DOESNT_MOVE)
					continue;
			}
		}

		item->room = room2;
		item->region = _state.region;

		if (room1 == IDI_ANY)
			item->state = IDI_ITEM_DROPPED;
	}

	return 2;
}

int AdlEngine_v4::o4_setRegion(ScriptEnv &e) {
	OP_DEBUG_1("\tREGION = %d", e.arg(1));

	switchRegion(e.arg(1));
	// Long jump
	_isRestarting = true;
	return -1;
}

int AdlEngine_v4::o4_setRegionRoom(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_REGION_ROOM(%d, %d)", e.arg(1), e.arg(2));

	switchRegion(e.arg(1));
	_state.room = e.arg(2);
	// Long jump
	_isRestarting = true;
	return -1;
}

int AdlEngine_v4::o4_setRoomPic(ScriptEnv &e) {
	o1_setRoomPic(e);
	backupRoomState(e.arg(1));
	return 2;
}

} // End of namespace Adl
