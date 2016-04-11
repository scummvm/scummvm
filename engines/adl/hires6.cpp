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
#include "common/memstream.h"

#include "adl/hires6.h"
#include "adl/display.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

static const char *disks[] = { "DARK1A.DSK", "DARK1B.NIB", "DARK2A.NIB", "DARK2B.NIB" };

#define SECTORS_PER_TRACK 16
#define BYTES_PER_SECTOR 256

static Common::MemoryReadStream *loadSectors(DiskImage *disk, byte track, byte sector = SECTORS_PER_TRACK - 1, byte count = SECTORS_PER_TRACK) {
	const int bufSize = count * BYTES_PER_SECTOR;
	byte *const buf = (byte *)malloc(bufSize);
	byte *p = buf;

	while (count-- > 0) {
		StreamPtr stream(disk->createReadStream(track, sector, 0, 0));
		stream->read(p, BYTES_PER_SECTOR);

		if (stream->err() || stream->eos())
			error("Error loading from disk image");

		p += BYTES_PER_SECTOR;
		if (sector > 0)
			--sector;
		else {
			++track;

			// Skip VTOC track
			if (track == 17)
				++track;

			sector = SECTORS_PER_TRACK - 1;
		}
	}

	return new Common::MemoryReadStream(buf, bufSize, DisposeAfterUse::YES);
}

void HiRes6Engine::runIntro() const {
	DiskImage_DSK *boot(new DiskImage_DSK());

	if (!boot->open(disks[0]))
		error("Failed to open disk image '%s'", disks[0]);

	StreamPtr stream(loadSectors(boot, 11, 1, 96));

	_display->setMode(DISPLAY_MODE_HIRES);
	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();
	delay(256 * 8609 / 1000);

	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();
	delay(256 * 8609 / 1000);

	_display->loadFrameBuffer(*stream);

	delete boot;

	// Load copyright string from boot file
	Files_DOS33 *files(new Files_DOS33());

	if (!files->open(disks[0]))
		error("Failed to open disk image '%s'", disks[0]);

	stream.reset(files->createReadStream("\010\010\010\010\010\010"));
	Common::String copyright(readStringAt(*stream, 0x103, APPLECHAR('\r')));

	delete files;

	_display->updateHiResScreen();
	_display->home();
	_display->setMode(DISPLAY_MODE_MIXED);
	_display->moveCursorTo(Common::Point(0, 21));
	_display->printString(copyright);
	delay(256 * 8609 / 1000);
}

void HiRes6Engine::init() {
	DiskImage_DSK *boot(new DiskImage_DSK());
	_graphics = new Graphics_v2(*_display);

	if (!boot->open(disks[0]))
		error("Failed to open disk image '%s'", disks[0]);

	StreamPtr stream(loadSectors(boot, 7));

	// Read parser messages
	_strings.verbError = readStringAt(*stream, 0x666);
	_strings.nounError = readStringAt(*stream, 0x6bd);
	_strings.enterCommand = readStringAt(*stream, 0x6e9);

	// Read line feeds
	_strings.lineFeeds = readStringAt(*stream, 0x408);

	// Read opcode strings (TODO)
	_strings_v2.saveInsert = readStringAt(*stream, 0xad8);
	readStringAt(*stream, 0xb95); // Confirm save
	// _strings_v2.saveReplace
	_strings_v2.restoreInsert = readStringAt(*stream, 0xc07);
	// _strings_v2.restoreReplace
	_strings.playAgain = readStringAt(*stream, 0xcdf, 0xff);

	_messageIds.cantGoThere = IDI_HR6_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR6_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR6_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR6_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR6_MSG_THANKS_FOR_PLAYING;

	// Load dropped item offsets
	stream.reset(boot->createReadStream(0x8, 0x9, 0x16));
	for (uint i = 0; i < IDI_HR6_NUM_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = stream->readByte();
		p.y = stream->readByte();
		_itemOffsets.push_back(p);
	}

	_disk = new DiskImage_NIB();

	if (!_disk->open(disks[1]))
		error("Failed to open disk image '%s'", disks[1]);

	// Load item picture data
	stream.reset(boot->createReadStream(0xb, 0xd, 0x08));
	for (uint i = 0; i < IDI_HR6_NUM_ITEM_PICS; ++i) {
		stream->readByte();
		_itemPics.push_back(readDataBlockPtr(*stream));
	}

	// Load global picture data
	stream.reset(_disk->createReadStream(0x1f, 0xf, 0x16));
	byte picNr;
	while ((picNr = stream->readByte()) != 0xff) {
		if (stream->eos() || stream->err())
			error("Error reading global pic list");
		_pictures[picNr] = readDataBlockPtr(*stream);
	}

	// Load message offsets
	stream.reset(_disk->createReadStream(0x1f, 0xb, 0x16, 4));
	for (uint i = 0; i < IDI_HR6_NUM_MESSAGES; ++i)
		_messages.push_back(readDataBlockPtr(*stream));

	// Load commands
	stream.reset(_disk->createReadStream(0x21, 0x4, 0x85, 7));
	readCommands(*stream, _roomCommands);

	stream.reset(_disk->createReadStream(0x20, 0xf, 0x82, 5));
	readCommands(*stream, _globalCommands);

	// Load verbs
	stream.reset(_disk->createReadStream(0x1f, 0xf, 0x56, 6));
	loadWords(*stream, _verbs, _priVerbs);

	// Load nouns
	stream.reset(_disk->createReadStream(0x20, 0x5, 0x8e, 8));
	loadWords(*stream, _nouns, _priNouns);

	delete boot;
}

void HiRes6Engine::initGameState() {
	_state.vars.resize(IDI_HR6_NUM_VARS);

	StreamPtr stream(_disk->createReadStream(0x20, 0xd, 0x94, 2));

	for (uint i = 0; i < IDI_HR6_NUM_ROOMS; ++i) {
		Room room;
		stream->readByte(); // number
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = stream->readByte();
		room.data = readDataBlockPtr(*stream);
		room.picture = stream->readByte();
		room.curPicture = stream->readByte();
		room.isFirstTime = stream->readByte();
		_state.rooms.push_back(room);
	}

	stream.reset(_disk->createReadStream(0x22, 0x0, 0x07, 0));

	byte id;
	while ((id = stream->readByte()) != 0xff) {
		Item item = { };
		item.id = id;
		item.noun = stream->readByte();
		item.room = stream->readByte();
		item.picture = stream->readByte();
		item.isLineArt = stream->readByte(); // Now seems to be disk number
		item.position.x = stream->readByte();
		item.position.y = stream->readByte();
		item.state = stream->readByte();
		item.description = stream->readByte();

		stream->readByte(); // Struct size

		byte picListSize = stream->readByte();

		// Flag to keep track of what has been drawn on the screen
		stream->readByte();

		for (uint i = 0; i < picListSize; ++i)
			item.roomPictures.push_back(stream->readByte());

		_state.items.push_back(item);
	}

	_currVerb = _currNoun = 0;
}

void HiRes6Engine::printRoomDescription() {
	setVar(2, 0xff);
	AdlEngine_v3::printRoomDescription();
}

void HiRes6Engine::applyDataBlockOffset(byte &track, byte &sector) const {
	// FIXME: this uses a table
	++track;
}

void HiRes6Engine::printString(const Common::String &str) {
	Common::String s;
	uint found = 0;

	// This does not emulate the corner cases of the original, hence this check
	if (getVar(27) > 1)
		error("Invalid value %i encountered for variable 27", getVar(27));

	for (uint i = 0; i < str.size(); ++i) {
		if (str[i] == '%') {
			++found;
			if (found == 3)
				found = 0;
		} else {
			if (found == 0 || found - 1 == getVar(27))
				s += str[i];
		}
	}

	if (getVar(2) != 0xff) {
		AdlEngine_v2::printString(s);
	} else {
		if (getVar(26) == 0) {
			if (str.size() != 1 || APPLECHAR(str[0]) != APPLECHAR(' '))
				return AdlEngine_v2::printString(s);
			setVar(2, APPLECHAR(' '));
		} else if (getVar(26) != 0xff) {
			setVar(2, 'P');
		} else {
			setVar(26, _state.room);
			setVar(2, 1);
		}

		doAllCommands(_globalCommands, _currVerb, _currNoun);
	}
}

Engine *HiRes6Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes6Engine(syst, gd);
}

} // End of namespace Adl
