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
#include "common/ptr.h"

#include "adl/hires1.h"
#include "adl/display.h"

namespace Adl {

void HiRes1Engine::runIntro() const {
	StreamPtr stream(_files->createReadStream(IDS_HR1_EXE_0));

	stream->seek(IDI_HR1_OFS_LOGO_0);
	_display->setMode(DISPLAY_MODE_HIRES);
	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();
	delay(4000);

	if (shouldQuit())
		return;

	_display->setMode(DISPLAY_MODE_TEXT);

	StreamPtr basic(_files->createReadStream(IDS_HR1_LOADER));
	Common::String str;

	str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_0, '"');
	_display->printAsciiString(str + '\r');

	str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_1, '"');
	_display->printAsciiString(str + "\r\r");

	str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_2, '"');
	_display->printAsciiString(str + "\r\r");

	str = readStringAt(*basic, IDI_HR1_OFS_PD_TEXT_3, '"');
	_display->printAsciiString(str + '\r');

	inputKey();
	if (g_engine->shouldQuit())
		return;

	_display->setMode(DISPLAY_MODE_MIXED);

	str = readStringAt(*stream, IDI_HR1_OFS_GAME_OR_HELP);

	bool instructions = false;

	while (1) {
		_display->printString(str);
		Common::String s = inputString();

		if (g_engine->shouldQuit())
			break;

		if (s.empty())
			continue;

		if (s[0] == APPLECHAR('I')) {
			instructions = true;
			break;
		} else if (s[0] == APPLECHAR('G')) {
			break;
		}
	};

	if (instructions) {
		_display->setMode(DISPLAY_MODE_TEXT);
		stream->seek(IDI_HR1_OFS_INTRO_TEXT);

		const uint pages[] = { 6, 6, 4, 5, 8, 7, 0 };

		uint page = 0;
		while (pages[page] != 0) {
			_display->home();

			uint count = pages[page++];
			for (uint i = 0; i < count; ++i) {
				str = readString(*stream);
				_display->printString(str);
				stream->seek(3, SEEK_CUR);
			}

			inputString();

			if (g_engine->shouldQuit())
				return;

			stream->seek(6, SEEK_CUR);
		}
	}

	_display->printAsciiString("\r");

	_display->setMode(DISPLAY_MODE_MIXED);

	// Title screen shown during loading
	stream.reset(_files->createReadStream(IDS_HR1_EXE_1));
	stream->seek(IDI_HR1_OFS_LOGO_1);
	_display->loadFrameBuffer(*stream);
	_display->updateHiResScreen();
	delay(2000);
}

void HiRes1Engine::init() {
	if (Common::File::exists("MYSTHOUS.DSK")) {
		_files = new Files_DOS33();
		if (!static_cast<Files_DOS33 *>(_files)->open("MYSTHOUS.DSK"))
			error("Failed to open MYSTHOUS.DSK");
	} else
		_files = new Files_Plain();

	_graphics = new Graphics_v1(*_display);

	StreamPtr stream(_files->createReadStream(IDS_HR1_MESSAGES));

	for (uint i = 0; i < IDI_HR1_NUM_MESSAGES; ++i)
		_messages.push_back(readString(*stream, APPLECHAR('\r')) + APPLECHAR('\r'));

	stream.reset(_files->createReadStream(IDS_HR1_EXE_1));

	// Some messages have overrides inside the executable
	_messages[IDI_HR1_MSG_CANT_GO_THERE - 1] = readStringAt(*stream, IDI_HR1_OFS_STR_CANT_GO_THERE);
	_messages[IDI_HR1_MSG_DONT_HAVE_IT - 1] = readStringAt(*stream, IDI_HR1_OFS_STR_DONT_HAVE_IT);
	_messages[IDI_HR1_MSG_DONT_UNDERSTAND - 1] = readStringAt(*stream, IDI_HR1_OFS_STR_DONT_UNDERSTAND);
	_messages[IDI_HR1_MSG_GETTING_DARK - 1] = readStringAt(*stream, IDI_HR1_OFS_STR_GETTING_DARK);

	// Load other strings from executable
	_strings.enterCommand = readStringAt(*stream, IDI_HR1_OFS_STR_ENTER_COMMAND);
	_strings.verbError = readStringAt(*stream, IDI_HR1_OFS_STR_VERB_ERROR);
	_strings.nounError = readStringAt(*stream, IDI_HR1_OFS_STR_NOUN_ERROR);
	_strings.playAgain = readStringAt(*stream, IDI_HR1_OFS_STR_PLAY_AGAIN);
	_strings.pressReturn = readStringAt(*stream, IDI_HR1_OFS_STR_PRESS_RETURN);
	_strings.lineFeeds = readStringAt(*stream, IDI_HR1_OFS_STR_LINE_FEEDS);

	// Set message IDs
	_messageIds.cantGoThere = IDI_HR1_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR1_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR1_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR1_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR1_MSG_THANKS_FOR_PLAYING;

	// Load picture data from executable
	stream->seek(IDI_HR1_OFS_PICS);
	for (uint i = 1; i <= IDI_HR1_NUM_PICS; ++i) {
		byte block = stream->readByte();
		Common::String name = Common::String::format("BLOCK%i", block);
		uint16 offset = stream->readUint16LE();
		_pictures[i] = _files->getDataBlock(name, offset);
	}

	// Load commands from executable
	stream->seek(IDI_HR1_OFS_CMDS_1);
	readCommands(*stream, _roomCommands);

	stream->seek(IDI_HR1_OFS_CMDS_0);
	readCommands(*stream, _globalCommands);

	// Load dropped item offsets
	stream->seek(IDI_HR1_OFS_ITEM_OFFSETS);
	for (uint i = 0; i < IDI_HR1_NUM_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = stream->readByte();
		p.y = stream->readByte();
		_itemOffsets.push_back(p);
	}

	// Load right-angle line art
	stream->seek(IDI_HR1_OFS_CORNERS);
	uint16 cornersCount = stream->readUint16LE();
	for (uint i = 0; i < cornersCount; ++i)
		_corners.push_back(_files->getDataBlock(IDS_HR1_EXE_1, IDI_HR1_OFS_CORNERS + stream->readUint16LE()));

	if (stream->eos() || stream->err())
		error("Failed to read game data from '" IDS_HR1_EXE_1 "'");

	stream->seek(IDI_HR1_OFS_VERBS);
	loadWords(*stream, _verbs, _priVerbs);

	stream->seek(IDI_HR1_OFS_NOUNS);
	loadWords(*stream, _nouns, _priNouns);
}

void HiRes1Engine::initState() {
	_state.room = 1;
	_state.moves = 1;
	_state.isDark = false;

	_state.vars.clear();
	_state.vars.resize(IDI_HR1_NUM_VARS);

	StreamPtr stream(_files->createReadStream(IDS_HR1_EXE_1));

	// Load room data from executable
	_state.rooms.clear();
	_roomDesc.clear();
	stream->seek(IDI_HR1_OFS_ROOMS);
	for (uint i = 0; i < IDI_HR1_NUM_ROOMS; ++i) {
		Room room;
		stream->readByte();
		_roomDesc.push_back(stream->readByte());
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = stream->readByte();
		room.picture = stream->readByte();
		room.curPicture = stream->readByte();
		_state.rooms.push_back(room);
	}

	// Load item data from executable
	_state.items.clear();
	stream->seek(IDI_HR1_OFS_ITEMS);
	byte id;
	while ((id = stream->readByte()) != 0xff) {
		Item item = { };
		item.id = id;
		item.noun = stream->readByte();
		item.room = stream->readByte();
		item.picture = stream->readByte();
		item.isLineArt = stream->readByte();
		item.position.x = stream->readByte();
		item.position.y = stream->readByte();
		item.state = stream->readByte();
		item.description = stream->readByte();

		stream->readByte();

		byte size = stream->readByte();

		for (uint i = 0; i < size; ++i)
			item.roomPictures.push_back(stream->readByte());

		_state.items.push_back(item);
	}
}

void HiRes1Engine::restartGame() {
	_display->printString(_strings.pressReturn);
	initState();
	_display->printAsciiString(_strings.lineFeeds);
}

void HiRes1Engine::printString(const Common::String &str) {
	Common::String wrap = str;
	wordWrap(wrap);
	_display->printString(wrap);

	if (_messageDelay)
		delay(14 * 166018 / 1000);
}

void HiRes1Engine::printMessage(uint idx) {
	const Common::String &msg = _messages[idx - 1];

	// Messages with hardcoded overrides don't delay after printing.
	// It's unclear if this is a bug or not. In some cases the result
	// is that these strings will scroll past the four-line text window
	// before the user gets a chance to read them.
	// NOTE: later games seem to wait for a key when the text window
	// overflows and don't use delays. It might be better to use
	// that system for this game as well.
	switch (idx) {
	case IDI_HR1_MSG_CANT_GO_THERE:
	case IDI_HR1_MSG_DONT_HAVE_IT:
	case IDI_HR1_MSG_DONT_UNDERSTAND:
	case IDI_HR1_MSG_GETTING_DARK:
		_display->printString(msg);
		return;
	}

	printString(msg);
}

void HiRes1Engine::drawItem(const Item &item, const Common::Point &pos) const {
	if (item.isLineArt) {
		StreamPtr stream(_corners[item.picture - 1]->createReadStream());
		static_cast<Graphics_v1 *>(_graphics)->drawCorners(*stream, pos);
	} else
		drawPic(item.picture, pos);
}

void HiRes1Engine::loadRoom(byte roomNr) {
	_roomData.description = _messages[_roomDesc[_state.room - 1] - 1];
}

void HiRes1Engine::showRoom() {
	if (!_state.isDark) {
		drawPic(getCurRoom().curPicture);
		drawItems();
	}

	_display->updateHiResScreen();
	_messageDelay = false;
	printString(_roomData.description);
	_messageDelay = true;
}

void HiRes1Engine::wordWrap(Common::String &str) const {
	uint end = 39;

	while (1) {
		if (str.size() <= end)
			return;

		while (str[end] != APPLECHAR(' '))
			--end;

		str.setChar(APPLECHAR('\r'), end);
		end += 40;
	}
}

Engine *HiRes1Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes1Engine(syst, gd);
}

} // End of namespace Adl
