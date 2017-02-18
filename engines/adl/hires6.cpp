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

#include "adl/adl_v5.h"
#include "adl/display.h"
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Adl {

class HiRes6Engine : public AdlEngine_v5 {
public:
	HiRes6Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v5(syst, gd),
			_currVerb(0),
			_currNoun(0) {
	}

private:
	// AdlEngine
	void runIntro();
	void init();
	void initGameState();
	void showRoom();
	Common::String formatVerbError(const Common::String &verb) const;
	Common::String formatNounError(const Common::String &verb, const Common::String &noun) const;
	void loadState(Common::ReadStream &stream);
	void saveState(Common::WriteStream &stream);

	// AdlEngine_v2
	void printString(const Common::String &str);

	static const uint kRegions = 3;
	static const uint kItems = 15;

	byte _currVerb, _currNoun;
};

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

void HiRes6Engine::runIntro() {
	insertDisk(0);

	StreamPtr stream(loadSectors(_disk, 11, 1, 96));

	_display->setMode(DISPLAY_MODE_HIRES);
	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();
	delay(256 * 8609 / 1000);

	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();
	delay(256 * 8609 / 1000);

	_display->loadFrameBuffer(*stream);

	// Load copyright string from boot file
	Files_DOS33 *files(new Files_DOS33());

	if (!files->open(getDiskImageName(0)))
		error("Failed to open disk volume 0");

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
	_graphics = new Graphics_v3(*_display);

	insertDisk(0);

	StreamPtr stream(_disk->createReadStream(0x3, 0xf, 0x05));
	loadRegionLocations(*stream, kRegions);

	stream.reset(_disk->createReadStream(0x5, 0xa, 0x07));
	loadRegionInitDataOffsets(*stream, kRegions);

	stream.reset(loadSectors(_disk, 0x7));

	_strings.verbError = readStringAt(*stream, 0x666);
	_strings.nounError = readStringAt(*stream, 0x6bd);
	_strings.enterCommand = readStringAt(*stream, 0x6e9);

	_strings.lineFeeds = readStringAt(*stream, 0x408);

	_strings_v2.saveInsert = readStringAt(*stream, 0xad8);
	_strings_v2.saveReplace = readStringAt(*stream, 0xb95);
	_strings_v2.restoreInsert = readStringAt(*stream, 0xc07);
	_strings.playAgain = readStringAt(*stream, 0xcdf, 0xff);

	_messageIds.cantGoThere = 249;
	_messageIds.dontUnderstand = 247;
	_messageIds.itemDoesntMove = 253;
	_messageIds.itemNotHere = 254;
	_messageIds.thanksForPlaying = 252;

	stream.reset(loadSectors(_disk, 0x6, 0xb, 2));
	stream->seek(0x16);
	loadItemDescriptions(*stream, kItems);

	stream.reset(_disk->createReadStream(0x8, 0x9, 0x16));
	loadDroppedItemOffsets(*stream, 16);

	stream.reset(_disk->createReadStream(0xb, 0xd, 0x08));
	loadItemPicIndex(*stream, kItems);
}

void HiRes6Engine::initGameState() {
	_state.vars.resize(40);

	insertDisk(0);

	StreamPtr stream(_disk->createReadStream(0x3, 0xe, 0x03));
	loadItems(*stream);

	// A combined total of 91 rooms
	static const byte rooms[kRegions] = { 35, 29, 27 };

	initRegions(rooms, kRegions);

	loadRegion(1);

	_currVerb = _currNoun = 0;
}

void HiRes6Engine::showRoom() {
	_state.curPicture = getCurRoom().curPicture;

	bool redrawPic = false;

	if (getVar(26) == 0xfe)
		setVar(26, 0);
	else if (getVar(26) != 0xff)
		setVar(26, _state.room);

	if (_state.room != _roomOnScreen) {
		loadRoom(_state.room);

		if (getVar(26) < 0x80 && getCurRoom().isFirstTime)
			setVar(26, 0);

		_graphics->clearScreen();

		if (!_state.isDark)
			redrawPic = true;
	} else {
		if (getCurRoom().curPicture != _picOnScreen || _itemRemoved)
			redrawPic = true;
	}

	if (redrawPic) {
		_roomOnScreen = _state.room;
		_picOnScreen = getCurRoom().curPicture;

		drawPic(getCurRoom().curPicture);
		_itemRemoved = false;
		_itemsOnScreen = 0;

		Common::List<Item>::iterator item;
		for (item = _state.items.begin(); item != _state.items.end(); ++item)
			item->isOnScreen = false;
	}

	if (!_state.isDark)
		drawItems();

	_display->updateHiResScreen();
	setVar(2, 0xff);
	printString(_roomData.description);

	// FIXME: move to main loop?
	_linesPrinted = 0;
}

Common::String HiRes6Engine::formatVerbError(const Common::String &verb) const {
	Common::String err = _strings.verbError;

	for (uint i = 0; i < verb.size(); ++i)
		err.setChar(verb[i], i + 24);

	err.setChar(APPLECHAR(' '), 32);

	uint i = 24;
	while (err[i] != APPLECHAR(' '))
		++i;

	err.setChar(APPLECHAR('.'), i);

	return err;
}

Common::String HiRes6Engine::formatNounError(const Common::String &verb, const Common::String &noun) const {
	Common::String err = _strings.nounError;

	for (uint i = 0; i < noun.size(); ++i)
		err.setChar(noun[i], i + 24);

	for (uint  i = 35; i > 31; --i)
		err.setChar(APPLECHAR(' '), i);

	uint i = 24;
	while (err[i] != APPLECHAR(' '))
		++i;

	err.setChar(APPLECHAR('I'), i + 1);
	err.setChar(APPLECHAR('S'), i + 2);
	err.setChar(APPLECHAR('.'), i + 3);

	return err;
}

void HiRes6Engine::loadState(Common::ReadStream &stream) {
	AdlEngine_v5::loadState(stream);
	_state.moves = (getVar(39) << 8) | getVar(24);
	setVar(39, 0);
}

void HiRes6Engine::saveState(Common::WriteStream &stream) {
	// Move counter is stuffed into variables, in order to save it
	setVar(24, _state.moves & 0xff);
	setVar(39, _state.moves >> 8);
	AdlEngine_v5::saveState(stream);
	setVar(39, 0);
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
		AdlEngine_v5::printString(s);
	} else {
		if (getVar(26) == 0) {
			if (str.size() != 1 || APPLECHAR(str[0]) != APPLECHAR(' '))
				return AdlEngine_v5::printString(s);
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
