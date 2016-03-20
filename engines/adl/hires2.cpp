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

static void readPictureMeta(Common::ReadStream &f, Picture2 &pic) {
	pic.nr = f.readByte();
	pic.track = f.readByte();
	pic.sector = f.readByte();
	pic.offset = f.readByte();
	f.readByte();
}

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
	_strings_v2.time = readStringAt(f, IDI_HR2_OFS_STR_TIME, 0xff);

	_messageIds.cantGoThere = IDI_HR2_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR2_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR2_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR2_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR2_MSG_THANKS_FOR_PLAYING;

	// Load item picture data
	f.seek(IDI_HR2_OFS_ITEM_PICS);
	for (uint i = 0; i < IDI_HR2_NUM_ITEM_PICS; ++i) {
		Picture2 pic;
		readPictureMeta(f, pic);
		_itemPics.push_back(pic);
	}

	// Load commands from executable
	f.seek(IDI_HR2_OFS_CMDS_1);
	readCommands(f, _roomCommands);

	f.seek(IDI_HR2_OFS_CMDS_0);
	readCommands(f, _globalCommands);

	// Load dropped item offsets
	f.seek(IDI_HR2_OFS_ITEM_OFFSETS);
	for (uint i = 0; i < IDI_HR2_NUM_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = f.readByte();
		p.y = f.readByte();
		_itemOffsets.push_back(p);
	}

	f.seek(IDI_HR2_OFS_VERBS);
	loadWords(f, _verbs);

	f.seek(IDI_HR2_OFS_NOUNS);
	loadWords(f, _nouns);
}

void HiRes2Engine::initState() {
	_state.vars.clear();
	_state.vars.resize(IDI_HR2_NUM_VARS);

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
		room.isFirstTime = f.readByte();
		_state.rooms.push_back(room);
	}

	_state.items.clear();
	f.seek(IDI_HR2_OFS_ITEMS);
	while (f.readByte() != 0xff) {
		Item item = { };
		item.noun = f.readByte();
		item.room = f.readByte();
		item.picture = f.readByte();
		item.isLineArt = f.readByte(); // Is this still used in this way?
		item.position.x = f.readByte();
		item.position.y = f.readByte();
		item.state = f.readByte();
		item.description = f.readByte();

		f.readByte(); // Struct size

		byte picListSize = f.readByte();

		// Flag to keep track of what has been drawn on the screen
		f.readByte();

		for (uint i = 0; i < picListSize; ++i)
			item.roomPictures.push_back(f.readByte());

		_state.items.push_back(item);
	}
}

void HiRes2Engine::loadRoom(byte roomNr) {
	Common::File f;
	openFile(f, IDS_HR2_DISK_IMAGE);
	Room &room = getRoom(roomNr);
	uint offset = TSO(room.track, room.sector, room.offset);
	f.seek(offset);
	uint16 descOffset = f.readUint16LE();
	uint16 commandOffset = f.readUint16LE();

	// There's no picture count. The original engine always checks at most
	// five pictures. We use the description offset to bound our search.
	uint16 picCount = (descOffset - 4) / 5;

	for (uint i = 0; i < picCount; ++i) {
		Picture2 pic;
		readPictureMeta(f, pic);
		_roomData.pictures.push_back(pic);
	}

	_roomData.description = readStringAt(f, offset + descOffset, 0xff);

	f.seek(offset + commandOffset);

	readCommands(f, _roomData.commands);
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
			return;
		}
	}

	// Check global pic list here
}

void HiRes2Engine::drawItem(const Item &item, const Common::Point &pos) const {
	const Picture2 &pic = _itemPics[item.picture - 1];
	Common::File f;
	openFile(f, IDS_HR2_DISK_IMAGE);
	f.seek(TSO(pic.track, pic.sector, pic.offset));
	f.readByte(); // Skip clear opcode
	_graphics->drawPic(f, pos, 0);
}

void HiRes2Engine::showRoom() {
	loadRoom(_state.room);
	drawPic(getCurRoom().curPicture, Common::Point());
	drawItems();
	_display->updateHiResScreen();
	printString(_roomData.description);
	_linesPrinted = 0;
}

void HiRes2Engine::printMessage(uint idx, bool wait) {
	printString(_messages[idx - 1]);
}

void HiRes2Engine::checkInput(byte verb, byte noun) {
	if (doOneCommand(_roomData.commands, verb, noun))
		return;

	AdlEngine::checkInput(verb, noun);
}

Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes2Engine(syst, gd);
}

} // End of namespace Adl
