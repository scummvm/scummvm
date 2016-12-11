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

#include "adl/adl_v4.h"
#include "adl/detection.h"
#include "adl/display.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

class HiRes5Engine : public AdlEngine_v4 {
public:
	HiRes5Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v4(syst, gd) { }

private:
	// AdlEngine
	void runIntro();
	void init();
	void initGameState();

	static const uint kRegions = 41;
	static const uint kItems = 69;
};

void HiRes5Engine::runIntro() {
	insertDisk(2);

	StreamPtr stream(_disk->createReadStream(0x10, 0x0, 0x00, 31));

	_display->setMode(DISPLAY_MODE_HIRES);
	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();

	inputKey();

	_display->home();
	_display->setMode(DISPLAY_MODE_TEXT);

	stream.reset(_disk->createReadStream(0x03, 0xc, 0x34, 1));
	Common::String menu(readString(*stream));

	while (!g_engine->shouldQuit()) {
		_display->home();
		_display->printString(menu);

		Common::String cmd(inputString());

		// We ignore the backup and format menu options
		if (!cmd.empty() && cmd[0] == APPLECHAR('1'))
			break;
	};
}

void HiRes5Engine::init() {
	_graphics = new Graphics_v2(*_display);

	insertDisk(2);

	StreamPtr stream(_disk->createReadStream(0x5, 0x0, 0x02));
	loadRegionLocations(*stream, kRegions);

	stream.reset(_disk->createReadStream(0xd, 0x2, 0x04));
	loadRegionInitDataOffsets(*stream, kRegions);

	stream.reset(_disk->createReadStream(0x7, 0xe));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x8e);
	_strings.enterCommand = readStringAt(*stream, 0xbc);

	stream.reset(_disk->createReadStream(0x7, 0xc));
	_strings.lineFeeds = readString(*stream);

	// TODO: opcode strings

	_messageIds.cantGoThere = 110;
	_messageIds.dontUnderstand = 112;
	_messageIds.itemDoesntMove = 114;
	_messageIds.itemNotHere = 115;
	_messageIds.thanksForPlaying = 113;

	stream.reset(_disk->createReadStream(0xe, 0x1, 0x13, 4));
	loadItemDescriptions(*stream, kItems);

	stream.reset(_disk->createReadStream(0xb, 0xa, 0x05, 1));
	loadItemPicIndex(*stream, kItems);

	if (stream->eos() || stream->err())
		error("Error reading item index");
}

void HiRes5Engine::initGameState() {
	_state.vars.resize(40);

	insertDisk(2);

	StreamPtr stream(_disk->createReadStream(0x5, 0x1, 0x00, 3));
	loadItems(*stream);

	// A combined total of 1213 rooms
	static const byte rooms[kRegions] = {
		6, 16, 24, 57, 40, 30, 76, 40,
		54, 38, 44, 21, 26, 42, 49, 32,
		59, 69, 1, 1, 1, 1, 1, 18,
		25, 13, 28, 28, 11, 23, 9, 31,
		6, 29, 29, 34, 9, 10, 95, 86,
		1
	};

	initRegions(rooms, kRegions);

	loadRegion(1);
	_state.room = 5;
}

Engine *HiRes5Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes5Engine(syst, gd);
}

} // End of namespace Adl
