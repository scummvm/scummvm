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

#include "adl/hires2.h"
#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

void HiRes2Engine::runIntro() const {
	Common::File f;
	openFile(f, IDS_HR2_DISK_IMAGE);
	f.seek(IDI_HR2_OFS_INTRO_TEXT);

	_display->setMode(DISPLAY_MODE_TEXT);

	Common::String str = readStringAt(f, IDI_HR2_OFS_INTRO_TEXT);

	if (f.eos() || f.err())
		error("Error reading disk image");

	_display->printString(str);
	delay(2000);
}

void HiRes2Engine::init() {
	_graphics = new Graphics_v2(*_display);

	Common::File f;
	openFile(f, IDS_HR2_DISK_IMAGE);

	for (uint i = 0; i < IDI_HR2_NUM_MESSAGES; ++i) {
		f.seek(IDI_HR2_OFS_MESSAGES + i * 4);
		byte track = f.readByte();
		byte sector = f.readByte();
		byte offset = f.readByte();
		// One more byte follows, disk? or size maybe?

		uint diskOffset = TSO(track, sector, offset);

		Common::String str;

		if (diskOffset != 0)
			str = readStringAt(f, TSO(track, sector, offset), 0xff);

		_messages.push_back(str);
	}

	_strings.enterCommand = readStringAt(f, IDI_HR2_OFS_STR_ENTER_COMMAND);
	_strings.verbError = readStringAt(f, IDI_HR2_OFS_STR_VERB_ERROR);
	_strings.nounError = readStringAt(f, IDI_HR2_OFS_STR_NOUN_ERROR);
	_strings.playAgain = readStringAt(f, IDI_HR2_OFS_STR_PLAY_AGAIN);
	_strings.pressReturn = readStringAt(f, IDI_HR2_OFS_STR_PRESS_RETURN);

	_messageIds.cantGoThere = IDI_HR2_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR2_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR2_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR2_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR2_MSG_THANKS_FOR_PLAYING;

	f.seek(IDI_HR2_OFS_VERBS);
	loadWords(f, _verbs);

	f.seek(IDI_HR2_OFS_NOUNS);
	loadWords(f, _nouns);
}

void HiRes2Engine::initState() {
	Common::File f;
	openFile(f, IDS_HR2_DISK_IMAGE);

	_state.rooms.clear();
	f.seek(IDI_HR2_OFS_ROOMS);
	for (uint i = 0; i < IDI_HR2_NUM_ROOMS; ++i) {
		Room room = { };
		f.readByte(); // number
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = f.readByte();
		room.track = f.readByte();
		room.sector = f.readByte();
		room.offset = f.readByte();
		f.readByte(); // always 1, possibly disk?
		room.picture = f.readByte();
		room.curPicture = f.readByte();
		f.readByte(); // always 1, possibly disk?
		_state.rooms.push_back(room);
	}
}

void HiRes2Engine::loadRoom(byte roomNr) {
	Common::File f;
	openFile(f, IDS_HR2_DISK_IMAGE);
	Room &room = getRoom(roomNr);
	uint offset = TSO(room.track, room.sector, room.offset);
	f.seek(offset);
	uint16 descOffset = f.readUint16LE();
	/* uint16 commandOffset =*/ f.readUint16LE();

	// There's no picture count. The original engine always checks at most
	// five pictures. We use the description offset to bound our search.
	uint16 picCount = (descOffset - 4) / 5;

	for (uint i = 0; i < picCount; ++i) {
		Picture2 pic;
		pic.nr = f.readByte();
		pic.track = f.readByte();
		pic.sector = f.readByte();
		pic.offset = f.readByte();
		f.readByte();
		_roomData.pictures.push_back(pic);
	}
	_roomData.description = readStringAt(f, offset + descOffset, 0xff);
}

void HiRes2Engine::restartGame() {
	initState();
}

void HiRes2Engine::drawPic(byte pic, Common::Point pos) const {
	Common::Array<Picture2>::const_iterator roomPic;

	for (roomPic = _roomData.pictures.begin(); roomPic != _roomData.pictures.end(); ++roomPic) {
		if (roomPic->nr == pic) {
			Common::File f;
			openFile(f, IDS_HR2_DISK_IMAGE);
			f.seek(TSO(roomPic->track, roomPic->sector, roomPic->offset));
			_graphics->drawPic(f, pos, 0);
		}
	}

	// Check global pic list here
}

void HiRes2Engine::showRoom() {
	loadRoom(_state.room);
	_linesPrinted = 0;
	drawPic(getCurRoom().curPicture, Common::Point());
	_display->updateHiResScreen();
	printString(_roomData.description);
}

void HiRes2Engine::printMessage(uint idx, bool wait) {
	printString(_messages[idx - 1]);
}

void HiRes2Engine::checkTextOverflow(char c) {
	if (c != APPLECHAR('\r'))
		return;

	++_linesPrinted;

	if (_linesPrinted < 4)
		return;

	_linesPrinted = 0;
	_display->updateTextScreen();
	bell();

	while (true) {
		char key = inputKey(false);

		if (shouldQuit())
			return;

		if (key == APPLECHAR('\r'))
			break;

		bell(3);
	}
}

void HiRes2Engine::printString(const Common::String &str) {
	Common::String s(str);
	byte endPos = TEXT_WIDTH - 1;
	byte pos = 0;

	while (true) {
		while (pos != endPos && pos != s.size()) {
			s.setChar(APPLECHAR(s[pos]), pos);
			++pos;
		}

		if (pos == s.size())
			break;

		while (s[pos] != APPLECHAR(' ') && s[pos] != APPLECHAR('\r'))
			--pos;

		s.setChar(APPLECHAR('\r'), pos);
		endPos = pos + TEXT_WIDTH;
		++pos;
	}

	pos = 0;
	while (pos != s.size()) {
		checkTextOverflow(s[pos]);
		_display->printChar(s[pos]);
		++pos;
	}

	checkTextOverflow(APPLECHAR('\r'));
	_display->printChar(APPLECHAR('\r'));
	_display->updateTextScreen();
}

Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes2Engine(syst, gd);
}

} // End of namespace Adl
