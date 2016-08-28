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

#include "common/system.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/stream.h"

#include "adl/hires4.h"
#include "adl/detection.h"
#include "adl/display.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

HiRes4Engine::~HiRes4Engine() {
	delete _boot;
}

void HiRes4Engine::init() {
	_graphics = new Graphics_v2(*_display);

	_boot = new DiskImage();
	if (!_boot->open(getDiskImageName(0)))
		error("Failed to open disk image '%s'", getDiskImageName(0));

	_disk = new DiskImage();
	if (!_disk->open(getDiskImageName(1)))
		error("Failed to open disk image '%s'", getDiskImageName(1));

	loadCommonData();

	StreamPtr stream(createReadStream(_boot, 0x06, 0x2));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x83);
	_strings.enterCommand = readStringAt(*stream, 0xa6);

	_messageIds.cantGoThere = IDI_HR4_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR4_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR4_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR4_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR4_MSG_THANKS_FOR_PLAYING;

	stream.reset(createReadStream(_boot, 0x06, 0xd, 0x12, 2));
	loadItemDescriptions(*stream, IDI_HR4_NUM_ITEM_DESCS);

	stream.reset(createReadStream(_boot, 0x05, 0x4, 0x00, 3));
	loadWords(*stream, _verbs, _priVerbs);

	stream.reset(createReadStream(_boot, 0x03, 0xb, 0x00, 6));
	loadWords(*stream, _nouns, _priNouns);
}

Common::String HiRes4Engine::formatVerbError(const Common::String &verb) const {
	Common::String err = _strings.verbError;
	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 8);
	return err;
}

Common::String HiRes4Engine::formatNounError(const Common::String &verb, const Common::String &noun) const {
	Common::String err = _strings.nounError;
	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 8);
	for (uint i = 0; i < noun.size(); ++i)
		err.setChar(noun[i], i + 19);
	return err;
}

void HiRes4Engine::goToSideC() {
	delete _disk;

	_disk = new DiskImage();
	if (!_disk->open(getDiskImageName(2)))
		error("Failed to open disk image '%s'", getDiskImageName(2));

	// As room.data is bound to the DiskImage, we need to rebind them here
	// We cannot simply reload the rooms as that would reset their state

	// FIXME: Remove DataBlockPtr-DiskImage coupling?

	StreamPtr stream(createReadStream(_boot, 0x03, 0x1, 0x0e, 9));
	for (uint i = 0; i < IDI_HR4_NUM_ROOMS; ++i) {
		stream->skip(7);
		_state.rooms[i].data = readDataBlockPtr(*stream);
		stream->skip(3);
	}

	// Rebind data that is on both side B and C
	loadCommonData();
}

void HiRes4Engine::loadCommonData() {
	_messages.clear();
	StreamPtr stream(createReadStream(_boot, 0x0a, 0x4, 0x00, 3));
	loadMessages(*stream, IDI_HR4_NUM_MESSAGES);

	_pictures.clear();
	stream.reset(createReadStream(_boot, 0x05, 0xe, 0x80));
	loadPictures(*stream);
}

void HiRes4Engine::initGameState() {
	_state.vars.resize(IDI_HR4_NUM_VARS);

	StreamPtr stream(createReadStream(_boot, 0x03, 0x1, 0x0e, 9));
	loadRooms(*stream, IDI_HR4_NUM_ROOMS);

	stream.reset(createReadStream(_boot, 0x02, 0xc, 0x00, 12));
	loadItems(*stream);

	// FIXME
	_display->moveCursorTo(Common::Point(0, 23));
}

Common::SeekableReadStream *HiRes4Engine::createReadStream(DiskImage *disk, byte track, byte sector, byte offset, byte size) const {
	adjustDataBlockPtr(track, sector, offset, size);
	return disk->createReadStream(track, sector, offset, size);
}

void HiRes4Engine_Atari::adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const {
	// Convert the Apple II disk offsets in the game, to Atari disk offsets
	uint sectorIndex = (track * 16 + sector + 1) << 1;

	// Atari uses 128 bytes per sector vs. 256 on the Apple II
	// Note that size indicates *additional* sectors to read after reading one sector
	size *= 2;

	if (offset >= 128) {
		// Offset in the second half of an Apple II sector, skip one sector and adjust offset
		++sectorIndex;
		offset -= 128;
	} else {
		// Offset in the first half of an Apple II sector, we need to read one additional sector
		++size;
	}

	// Compute track/sector for Atari's 18 sectors per track (sectorIndex is 1-based)
	track = (sectorIndex - 1) / 18;
	sector = (sectorIndex - 1) % 18;
}

const char *HiRes4Engine_Atari::getDiskImageName(byte index) const {
	static const char *const disks[] = { "ULYS1A.XFD", "ULYS1B.XFD", "ULYS2C.XFD" };
	return disks[index];
}

Engine *HiRes4Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	switch (gd->desc.platform) {
	case Common::kPlatformAtariST:
		return new HiRes4Engine_Atari(syst, gd);
	default:
		error("Unsupported platform");
	}
}

} // End of namespace Adl
