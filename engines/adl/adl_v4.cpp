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
			// Global commands
			readCommands(*stream, _globalCommands);
			break;
		case 0x9500:
			// Room commands
			readCommands(*stream, _roomCommands);
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
}

void AdlEngine_v4::loadItemPicIndex(Common::ReadStream &stream, uint items) {
	_itemPicIndex = stream.readStream(items * 5);

	if (stream.eos() || stream.err())
		error("Error reading item index");
}

} // End of namespace Adl
