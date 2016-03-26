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
#include "adl/disk.h"

namespace Adl {

DataBlockPtr HiRes2Engine::readDataBlockPtr(Common::ReadStream &f) const {
	byte track = f.readByte();
	byte sector = f.readByte();
	byte offset = f.readByte();
	byte size = f.readByte();
	return _disk.getDataBlock(track, sector, offset, size);
}

void HiRes2Engine::readPictureMeta(Common::ReadStream &f, Picture2 &pic) const {
	pic.nr = f.readByte();
	pic.data = readDataBlockPtr(f);
}

void HiRes2Engine::runIntro() const {
	StreamPtr stream(_disk.createReadStream(0x00, 0xd, 0x17, 1));

	_display->setMode(DISPLAY_MODE_TEXT);

	Common::String str = readString(*stream);

	if (stream->eos() || stream->err())
		error("Error reading disk image");

	_display->printString(str);
	delay(2000);
}

void HiRes2Engine::init() {
	_graphics = new Graphics_v2(*_display);

	if (!_disk.open(IDS_HR2_DISK_IMAGE))
		error("Failed to open disk image '" IDS_HR2_DISK_IMAGE "'");

	StreamPtr stream(_disk.createReadStream(0x1f, 0x2, 0x04, 4));

	for (uint i = 0; i < IDI_HR2_NUM_MESSAGES; ++i) {
		DataBlockPtr str(readDataBlockPtr(*stream));

		if (str->isValid()) {
			StreamPtr strStream(str->createReadStream());
			_messages.push_back(readString(*strStream, 0xff));
		}
	}

	// Read parser messages
	stream.reset(_disk.createReadStream(0x1a, 0x1));
	_strings.verbError = readStringAt(*stream, 0x4f);
	_strings.nounError = readStringAt(*stream, 0x8e);
	_strings.enterCommand = readStringAt(*stream, 0xbc);

	// Read time string
	stream.reset(_disk.createReadStream(0x19, 0x7, 0xd7));
	_strings_v2.time = readString(*stream, 0xff);

	// Read opcode strings
	stream.reset(_disk.createReadStream(0x1a, 0x6, 0x00, 2));
	_strings_v2.saveInsert = readStringAt(*stream, 0x5f);
	_strings_v2.saveReplace = readStringAt(*stream, 0xe5);
	_strings_v2.restoreInsert = readStringAt(*stream, 0x132);
	_strings_v2.restoreReplace = readStringAt(*stream, 0x1c2);
	_strings.playAgain = readStringAt(*stream, 0x225);
	_strings.pressReturn = readStringAt(*stream, 0x25f);

	_messageIds.cantGoThere = IDI_HR2_MSG_CANT_GO_THERE;
	_messageIds.dontUnderstand = IDI_HR2_MSG_DONT_UNDERSTAND;
	_messageIds.itemDoesntMove = IDI_HR2_MSG_ITEM_DOESNT_MOVE;
	_messageIds.itemNotHere = IDI_HR2_MSG_ITEM_NOT_HERE;
	_messageIds.thanksForPlaying = IDI_HR2_MSG_THANKS_FOR_PLAYING;

	// Load item picture data
	stream.reset(_disk.createReadStream(0x1e, 0x9, 0x05));
	for (uint i = 0; i < IDI_HR2_NUM_ITEM_PICS; ++i) {
		Picture2 pic;
		readPictureMeta(*stream, pic);
		_itemPics.push_back(pic);
	}

	// Load commands from executable
	stream.reset(_disk.createReadStream(0x1d, 0x7, 0x00, 4));
	readCommands(*stream, _roomCommands);

	stream.reset(_disk.createReadStream(0x1f, 0x7, 0x00, 2));
	readCommands(*stream, _globalCommands);

	// Load dropped item offsets
	stream.reset(_disk.createReadStream(0x1b, 0x4, 0x15));
	for (uint i = 0; i < IDI_HR2_NUM_ITEM_OFFSETS; ++i) {
		Common::Point p;
		p.x = stream->readByte();
		p.y = stream->readByte();
		_itemOffsets.push_back(p);
	}

	// Load verbs
	stream.reset(_disk.createReadStream(0x19, 0x0, 0x00, 3));
	loadWords(*stream, _verbs);

	// Load nouns
	stream.reset(_disk.createReadStream(0x22, 0x2, 0x00, 7));
	loadWords(*stream, _nouns);
}

void HiRes2Engine::initState() {
	_state.vars.clear();
	_state.vars.resize(IDI_HR2_NUM_VARS);

	StreamPtr stream(_disk.createReadStream(0x21, 0x5, 0x0e, 7));

	_state.rooms.clear();
	for (uint i = 0; i < IDI_HR2_NUM_ROOMS; ++i) {
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

	stream.reset(_disk.createReadStream(0x21, 0x0, 0x00, 2));

	_state.items.clear();
	while (stream->readByte() != 0xff) {
		Item item = { };
		item.noun = stream->readByte();
		item.room = stream->readByte();
		item.picture = stream->readByte();
		item.isLineArt = stream->readByte(); // Is this still used in this way?
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
}

void HiRes2Engine::loadRoom(byte roomNr) {
	Room &room = getRoom(roomNr);
	StreamPtr stream(room.data->createReadStream());

	uint16 descOffset = stream->readUint16LE();
	uint16 commandOffset = stream->readUint16LE();

	// There's no picture count. The original engine always checks at most
	// five pictures. We use the description offset to bound our search.
	uint16 picCount = (descOffset - 4) / 5;

	for (uint i = 0; i < picCount; ++i) {
		Picture2 pic;
		readPictureMeta(*stream, pic);
		_roomData.pictures.push_back(pic);
	}

	_roomData.description = readStringAt(*stream, descOffset, 0xff);

	stream->seek(commandOffset);
	readCommands(*stream, _roomData.commands);
}

void HiRes2Engine::restartGame() {
	initState();
}

void HiRes2Engine::drawPic(byte pic, Common::Point pos) const {
	Common::Array<Picture2>::const_iterator roomPic;

	for (roomPic = _roomData.pictures.begin(); roomPic != _roomData.pictures.end(); ++roomPic) {
		if (roomPic->nr == pic) {
			StreamPtr stream(roomPic->data->createReadStream());
			_graphics->drawPic(*stream, pos, 0);
			return;
		}
	}

	// Check global pic list here
}

void HiRes2Engine::drawItem(const Item &item, const Common::Point &pos) const {
	const Picture2 &pic = _itemPics[item.picture - 1];
	StreamPtr stream(pic.data->createReadStream());
	stream->readByte(); // Skip clear opcode
	_graphics->drawPic(*stream, pos, 0);
}

void HiRes2Engine::showRoom() {
	loadRoom(_state.room);
	drawPic(getCurRoom().curPicture, Common::Point());
	drawItems();
	_display->updateHiResScreen();
	printString(_roomData.description);
	_linesPrinted = 0;
}

void HiRes2Engine::printMessage(uint idx) {
	printString(_messages[idx - 1]);
}

Engine *HiRes2Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes2Engine(syst, gd);
}

} // End of namespace Adl
