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

#include "adl/adl_v3.h"
#include "adl/detection.h"
#include "adl/display.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

#define IDI_HR4_NUM_ROOMS 164
#define IDI_HR4_NUM_MESSAGES 255
#define IDI_HR4_NUM_VARS 40
#define IDI_HR4_NUM_ITEM_DESCS 44
#define IDI_HR4_NUM_ITEM_PICS 41
#define IDI_HR4_NUM_ITEM_OFFSETS 40

// Messages used outside of scripts
#define IDI_HR4_MSG_CANT_GO_THERE      110
#define IDI_HR4_MSG_DONT_UNDERSTAND    112
#define IDI_HR4_MSG_ITEM_DOESNT_MOVE   114
#define IDI_HR4_MSG_ITEM_NOT_HERE      115
#define IDI_HR4_MSG_THANKS_FOR_PLAYING 113

class HiRes4Engine_Atari : public AdlEngine_v3 {
public:
	HiRes4Engine_Atari(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v3(syst, gd),
			_boot(nullptr),
			_curDisk(0) { }
	~HiRes4Engine_Atari();

private:
	// AdlEngine
	void init();
	void initGameState();
	void loadRoom(byte roomNr);
	Common::String formatVerbError(const Common::String &verb) const;
	Common::String formatNounError(const Common::String &verb, const Common::String &noun) const;

	// AdlEngine_v2
	void adjustDataBlockPtr(byte &track, byte &sector, byte &offset, byte &size) const;

	Common::SeekableReadStream *createReadStream(DiskImage *disk, byte track, byte sector, byte offset = 0, byte size = 0) const;
	void loadCommonData();
	void insertDisk(byte diskNr);
	void rebindDisk();

	DiskImage *_boot;
	byte _curDisk;
};

static const char *const atariDisks[] = { "ULYS1A.XFD", "ULYS1B.XFD", "ULYS2C.XFD" };

HiRes4Engine_Atari::~HiRes4Engine_Atari() {
	delete _boot;
}

void HiRes4Engine_Atari::init() {
	_graphics = new Graphics_v2(*_display);

	_boot = new DiskImage();
	if (!_boot->open(atariDisks[0]))
		error("Failed to open disk image '%s'", atariDisks[0]);

	insertDisk(1);
	loadCommonData();

	StreamPtr stream(createReadStream(_boot, 0x06, 0x2));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x83);
	_strings.enterCommand = readStringAt(*stream, 0xa6);

	stream.reset(createReadStream(_boot, 0x05, 0xb, 0xd7));
	_strings_v2.time = readString(*stream, 0xff);

	stream.reset(createReadStream(_boot, 0x06, 0x7, 0x00, 2));
	_strings_v2.saveInsert = readStringAt(*stream, 0x62);
	_strings_v2.saveReplace = readStringAt(*stream, 0xdd);
	_strings_v2.restoreInsert = readStringAt(*stream, 0x12a);
	_strings_v2.restoreReplace = readStringAt(*stream, 0x1b8);
	_strings.playAgain = readStringAt(*stream, 0x21b);
	// TODO: restart sequence has "insert side a/b" strings

	_messageIds.cantGoThere = IDI_HR4_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR4_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR4_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR4_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR4_MSG_THANKS_FOR_PLAYING;

	stream.reset(createReadStream(_boot, 0x06, 0xd, 0x12, 2));
	loadItemDescriptions(*stream, IDI_HR4_NUM_ITEM_DESCS);

	stream.reset(createReadStream(_boot, 0x07, 0x1, 0xf4));
	loadDroppedItemOffsets(*stream, IDI_HR4_NUM_ITEM_OFFSETS);

	stream.reset(createReadStream(_boot, 0x08, 0xe, 0xa5, 5));
	readCommands(*stream, _roomCommands);

	stream.reset(createReadStream(_boot, 0x0a, 0x9, 0x00, 3));
	readCommands(*stream, _globalCommands);

	stream.reset(createReadStream(_boot, 0x05, 0x4, 0x00, 3));
	loadWords(*stream, _verbs, _priVerbs);

	stream.reset(createReadStream(_boot, 0x03, 0xb, 0x00, 6));
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes4Engine_Atari::loadRoom(byte roomNr) {
	if (roomNr >= 59 && roomNr < 113) {
		if (_curDisk != 2) {
			insertDisk(2);
			rebindDisk();
		}
	} else if (_curDisk != 1) {
		insertDisk(1);
		rebindDisk();
	}

	if (roomNr == 121) {
		// Room 121 is not present in the Atari version. This causes
		// problems when we're dumping scripts with the debugger, so
		// we intercept this room load here.
		// FIXME: Find out if the Apple II version does have this room
		// FIXME: Implement more generic handling of invalid rooms?
		debug("Warning: attempt to load non-existent room 121");
		_roomData.description.clear();
		_roomData.pictures.clear();
		_roomData.commands.clear();
		return;
	}

	AdlEngine_v3::loadRoom(roomNr);
}

Common::String HiRes4Engine_Atari::formatVerbError(const Common::String &verb) const {
	Common::String err = _strings.verbError;
	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 8);
	return err;
}

Common::String HiRes4Engine_Atari::formatNounError(const Common::String &verb, const Common::String &noun) const {
	Common::String err = _strings.nounError;
	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 8);
	for (uint i = 0; i < noun.size(); ++i)
		err.setChar(noun[i], i + 19);
	return err;
}

void HiRes4Engine_Atari::insertDisk(byte diskNr) {
	if (_curDisk == diskNr)
		return;

	_curDisk = diskNr;

	delete _disk;
	_disk = new DiskImage();
	if (!_disk->open(atariDisks[diskNr]))
		error("Failed to open disk image '%s'", atariDisks[diskNr]);
}

void HiRes4Engine_Atari::rebindDisk() {
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

void HiRes4Engine_Atari::loadCommonData() {
	_messages.clear();
	StreamPtr stream(createReadStream(_boot, 0x0a, 0x4, 0x00, 3));
	loadMessages(*stream, IDI_HR4_NUM_MESSAGES);

	_pictures.clear();
	stream.reset(createReadStream(_boot, 0x05, 0xe, 0x80));
	loadPictures(*stream);

	_itemPics.clear();
	stream.reset(createReadStream(_boot, 0x09, 0xe, 0x05));
	loadItemPictures(*stream, IDI_HR4_NUM_ITEM_PICS);
}

void HiRes4Engine_Atari::initGameState() {
	_state.vars.resize(IDI_HR4_NUM_VARS);

	StreamPtr stream(createReadStream(_boot, 0x03, 0x1, 0x0e, 9));
	loadRooms(*stream, IDI_HR4_NUM_ROOMS);

	stream.reset(createReadStream(_boot, 0x02, 0xc, 0x00, 12));
	loadItems(*stream);

	// FIXME
	_display->moveCursorTo(Common::Point(0, 23));
}

Common::SeekableReadStream *HiRes4Engine_Atari::createReadStream(DiskImage *disk, byte track, byte sector, byte offset, byte size) const {
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

Engine *HiRes4Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	switch (gd->desc.platform) {
	case Common::kPlatformAtari8Bit:
		return new HiRes4Engine_Atari(syst, gd);
	default:
		error("Unsupported platform");
	}
}

} // End of namespace Adl
