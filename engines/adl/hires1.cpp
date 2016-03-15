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
	Common::File file;

	openFile(file, IDS_HR1_EXE_0);

	file.seek(IDI_HR1_OFS_LOGO_0);
	_display->setMode(DISPLAY_MODE_HIRES);
	_display->loadFrameBuffer(file);
	_display->updateHiResScreen();
	delay(4000);

	if (shouldQuit())
		return;

	_display->setMode(DISPLAY_MODE_TEXT);

	Common::File basic;
	openFile(basic, IDS_HR1_LOADER);

	Common::String str;

	str = readStringAt(basic, IDI_HR1_OFS_PD_TEXT_0, '"');
	_display->printAsciiString(str + '\r');

	str = readStringAt(basic, IDI_HR1_OFS_PD_TEXT_1, '"');
	_display->printAsciiString(str + "\r\r");

	str = readStringAt(basic, IDI_HR1_OFS_PD_TEXT_2, '"');
	_display->printAsciiString(str + "\r\r");

	str = readStringAt(basic, IDI_HR1_OFS_PD_TEXT_3, '"');
	_display->printAsciiString(str + '\r');

	inputKey();
	if (g_engine->shouldQuit())
		return;

	_display->setMode(DISPLAY_MODE_MIXED);

	str = readStringAt(file, IDI_HR1_OFS_GAME_OR_HELP);

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
		file.seek(IDI_HR1_OFS_INTRO_TEXT);

		const uint pages[] = { 6, 6, 4, 5, 8, 7, 0 };

		uint page = 0;
		while (pages[page] != 0) {
			_display->home();

			uint count = pages[page++];
			for (uint i = 0; i < count; ++i) {
				str = readString(file);
				_display->printString(str);
				file.seek(3, SEEK_CUR);
			}

			inputString();

			if (g_engine->shouldQuit())
				return;

			file.seek(6, SEEK_CUR);
		}
	}

	_display->printAsciiString("\r");

	file.close();

	_display->setMode(DISPLAY_MODE_MIXED);

	// Title screen shown during loading
	openFile(file, IDS_HR1_EXE_1);
	file.seek(IDI_HR1_OFS_LOGO_1);
	_display->loadFrameBuffer(file);
	_display->updateHiResScreen();
	delay(2000);
}

void HiRes1Engine::init() {
	_graphics = new Graphics_v1(*_display);

	Common::File f;
	openFile(f, IDS_HR1_MESSAGES);

	for (uint i = 0; i < IDI_HR1_NUM_MESSAGES; ++i)
		_messages.push_back(readString(f, APPLECHAR('\r')) + APPLECHAR('\r'));

	f.close();
	openFile(f, IDS_HR1_EXE_1);

	// Some messages have overrides inside the executable
	_messages[IDI_HR1_MSG_CANT_GO_THERE - 1] = readStringAt(f, IDI_HR1_OFS_STR_CANT_GO_THERE);
	_messages[IDI_HR1_MSG_DONT_HAVE_IT - 1] = readStringAt(f, IDI_HR1_OFS_STR_DONT_HAVE_IT);
	_messages[IDI_HR1_MSG_DONT_UNDERSTAND - 1] = readStringAt(f, IDI_HR1_OFS_STR_DONT_UNDERSTAND);
	_messages[IDI_HR1_MSG_GETTING_DARK - 1] = readStringAt(f, IDI_HR1_OFS_STR_GETTING_DARK);

	// Load other strings from executable
	_strings.enterCommand = readStringAt(f, IDI_HR1_OFS_STR_ENTER_COMMAND);
	_strings.verbError = readStringAt(f, IDI_HR1_OFS_STR_VERB_ERROR);
	_strings.nounError = readStringAt(f, IDI_HR1_OFS_STR_NOUN_ERROR);
	_strings.playAgain = readStringAt(f, IDI_HR1_OFS_STR_PLAY_AGAIN);
	_strings.pressReturn = readStringAt(f, IDI_HR1_OFS_STR_PRESS_RETURN);

	// Set message IDs
	_messageIds.cantGoThere = IDI_HR1_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR1_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR1_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR1_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR1_MSG_THANKS_FOR_PLAYING;

	// Load picture data from executable
	f.seek(IDI_HR1_OFS_PICS);
	for (uint i = 0; i < IDI_HR1_NUM_PICS; ++i) {
		struct Picture pic;
		pic.block = f.readByte();
		pic.offset = f.readUint16LE();
		_pictures.push_back(pic);
	}

	// Load commands from executable
	f.seek(IDI_HR1_OFS_CMDS_1);
	readCommands(f, _roomCommands);

	f.seek(IDI_HR1_OFS_CMDS_0);
	readCommands(f, _globalCommands);

	// Load dropped item offsets
	f.seek(IDI_HR1_OFS_ITEM_OFFSETS);
	for (uint i = 0; i < IDI_HR1_NUM_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = f.readByte();
		p.y = f.readByte();
		_itemOffsets.push_back(p);
	}

	// Load right-angle line art
	f.seek(IDI_HR1_OFS_CORNERS);
	uint16 cornersCount = f.readUint16LE();
	for (uint i = 0; i < cornersCount; ++i)
		_corners.push_back(IDI_HR1_OFS_CORNERS + f.readUint16LE());

	if (f.eos() || f.err())
		error("Failed to read game data from '" IDS_HR1_EXE_1 "'");

	f.seek(IDI_HR1_OFS_VERBS);
	loadWords(f, _verbs);

	f.seek(IDI_HR1_OFS_NOUNS);
	loadWords(f, _nouns);
}

void HiRes1Engine::initState() {
	Common::File f;

	_state.room = 1;
	_state.moves = 0;
	_state.isDark = false;

	_state.vars.clear();
	_state.vars.resize(IDI_HR1_NUM_VARS);

	openFile(f, IDS_HR1_EXE_1);

	// Load room data from executable
	_state.rooms.clear();
	_roomDesc.clear();
	f.seek(IDI_HR1_OFS_ROOMS);
	for (uint i = 0; i < IDI_HR1_NUM_ROOMS; ++i) {
		Room room = { };
		f.readByte();
		_roomDesc.push_back(f.readByte());
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = f.readByte();
		room.picture = f.readByte();
		room.curPicture = f.readByte();
		_state.rooms.push_back(room);
	}

	// Load item data from executable
	_state.items.clear();
	f.seek(IDI_HR1_OFS_ITEMS);
	while (f.readByte() != 0xff) {
		Item item;
		item.noun = f.readByte();
		item.room = f.readByte();
		item.picture = f.readByte();
		item.isLineArt = f.readByte();
		item.position.x = f.readByte();
		item.position.y = f.readByte();
		item.state = f.readByte();
		item.description = f.readByte();

		f.readByte();

		byte size = f.readByte();

		for (uint i = 0; i < size; ++i)
			item.roomPictures.push_back(f.readByte());

		_state.items.push_back(item);
	}
}

void HiRes1Engine::restartGame() {
	initState();
	_display->printString(_strings.pressReturn);
	inputString(); // Missing in the original
	_display->printAsciiString("\r\r\r\r\r");
}

void HiRes1Engine::drawPic(byte pic, Common::Point pos) const {
	Common::File f;
	Common::String name = Common::String::format("BLOCK%i", _pictures[pic].block);

	openFile(f, name);
	f.seek(_pictures[pic].offset);
	_graphics->drawPic(f, pos, 0x7f);
}

void HiRes1Engine::printMessage(uint idx, bool wait) {
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
		wait = false;
	}

	AdlEngine::printMessage(idx, wait);
}

void HiRes1Engine::drawItem(const Item &item, const Common::Point &pos) const {
	if (item.isLineArt) {
		Common::File f;
		openFile(f, IDS_HR1_EXE_1);
		f.seek(_corners[item.picture - 1]);
		static_cast<Graphics_v1 *>(_graphics)->drawCorners(f, pos);
	} else
		drawPic(item.picture, pos);
}

void HiRes1Engine::showRoom() {
	if (!_state.isDark) {
		drawPic(getCurRoom().curPicture);
		drawItems();
	}

	_display->updateHiResScreen();
	printMessage(_roomDesc[_state.room - 1], false);
}

Engine *HiRes1Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes1Engine(syst, gd);
}

} // End of namespace Adl
