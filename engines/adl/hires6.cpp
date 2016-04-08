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

	delete boot;
}

void HiRes6Engine::initState() {
}

Engine *HiRes6Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes6Engine(syst, gd);
}

} // End of namespace Adl
