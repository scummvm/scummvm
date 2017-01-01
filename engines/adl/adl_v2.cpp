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

#include "common/random.h"
#include "common/error.h"

#include "adl/adl_v2.h"
#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

AdlEngine_v2::~AdlEngine_v2() {
	delete _random;
	delete _disk;
}

AdlEngine_v2::AdlEngine_v2(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine(syst, gd),
		_linesPrinted(0),
		_disk(nullptr),
		_itemRemoved(false),
		_roomOnScreen(0),
		_picOnScreen(0),
		_itemsOnScreen(0) {
	_random = new Common::RandomSource("adl");
}

typedef Common::Functor1Mem<ScriptEnv &, int, AdlEngine_v2> OpcodeV2;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV2(this, &AdlEngine_v2::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV2(this, 0))

void AdlEngine_v2::setupOpcodeTables() {
	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_condOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o2_isFirstTime);
	Opcode(o2_isRandomGT);
	Opcode(o1_isItemInRoom);
	// 0x04
	Opcode(o2_isNounNotInRoom);
	Opcode(o1_isMovesGT);
	Opcode(o1_isVarEQ);
	Opcode(o2_isCarryingSomething);
	// 0x08
	OpcodeUnImpl();
	Opcode(o1_isCurPicEQ);
	Opcode(o1_isItemPicEQ);

	SetOpcodeTable(_actOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o1_varAdd);
	Opcode(o1_varSub);
	Opcode(o1_varSet);
	// 0x04
	Opcode(o1_listInv);
	Opcode(o2_moveItem);
	Opcode(o1_setRoom);
	Opcode(o2_setCurPic);
	// 0x08
	Opcode(o2_setPic);
	Opcode(o1_printMsg);
	Opcode(o1_setLight);
	Opcode(o1_setDark);
	// 0x0c
	Opcode(o2_moveAllItems);
	Opcode(o1_quit);
	OpcodeUnImpl();
	Opcode(o2_save);
	// 0x10
	Opcode(o2_restore);
	Opcode(o1_restart);
	Opcode(o2_placeItem);
	Opcode(o1_setItemPic);
	// 0x14
	Opcode(o1_resetPic);
	Opcode(o1_goDirection<IDI_DIR_NORTH>);
	Opcode(o1_goDirection<IDI_DIR_SOUTH>);
	Opcode(o1_goDirection<IDI_DIR_EAST>);
	// 0x18
	Opcode(o1_goDirection<IDI_DIR_WEST>);
	Opcode(o1_goDirection<IDI_DIR_UP>);
	Opcode(o1_goDirection<IDI_DIR_DOWN>);
	Opcode(o1_takeItem);
	// 0x1c
	Opcode(o1_dropItem);
	Opcode(o1_setRoomPic);
	Opcode(o2_tellTime);
	Opcode(o2_setRoomFromVar);
	// 0x20
	Opcode(o2_initDisk);
}

void AdlEngine_v2::initState() {
	AdlEngine::initState();

	_linesPrinted = 0;
	_picOnScreen = 0;
	_roomOnScreen = 0;
	_itemRemoved = false;
	_itemsOnScreen = 0;
}

byte AdlEngine_v2::roomArg(byte room) const {
	if (room == IDI_CUR_ROOM)
		return _state.room;
	return room;
}

void AdlEngine_v2::advanceClock() {
	Time &time = _state.time;

	time.minutes += 5;

	if (time.minutes == 60) {
		time.minutes = 0;

		++time.hours;

		if (time.hours == 13)
			time.hours = 1;
	}
}

void AdlEngine_v2::checkTextOverflow(char c) {
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

Common::String AdlEngine_v2::loadMessage(uint idx) const {
	if (_messages[idx]) {
		StreamPtr strStream(_messages[idx]->createReadStream());
		return readString(*strStream, 0xff);
	}

	return Common::String();
}

void AdlEngine_v2::printString(const Common::String &str) {
	Common::String s(str);
	uint endPos = TEXT_WIDTH - 1;
	uint pos = 0;

	while (true) {
		while (pos <= endPos && pos != s.size()) {
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

void AdlEngine_v2::drawItem(Item &item, const Common::Point &pos) {
	item.isOnScreen = true;
	StreamPtr stream(_itemPics[item.picture - 1]->createReadStream());
	stream->readByte(); // Skip clear opcode
	_graphics->drawPic(*stream, pos);
}

void AdlEngine_v2::loadRoom(byte roomNr) {
	Room &room = getRoom(roomNr);
	StreamPtr stream(room.data->createReadStream());

	uint16 descOffset = stream->readUint16LE();
	uint16 commandOffset = stream->readUint16LE();

	_roomData.pictures.clear();
	// There's no picture count. The original engine always checks at most
	// five pictures. We use the description offset to bound our search.
	uint16 picCount = (descOffset - 4) / 5;

	for (uint i = 0; i < picCount; ++i) {
		byte nr = stream->readByte();
		_roomData.pictures[nr] = readDataBlockPtr(*stream);
	}

	_roomData.description = readStringAt(*stream, descOffset, 0xff);

	_roomData.commands.clear();
	if (commandOffset != 0) {
		stream->seek(commandOffset);
		readCommands(*stream, _roomData.commands);
	}

	applyRoomWorkarounds(roomNr);
}

void AdlEngine_v2::showRoom() {
	bool redrawPic = false;

	_state.curPicture = getCurRoom().curPicture;

	if (_state.room != _roomOnScreen) {
		loadRoom(_state.room);
		clearScreen();

		if (!_state.isDark)
			redrawPic = true;
	} else {
		if (_state.curPicture != _picOnScreen || _itemRemoved)
			redrawPic = true;
	}

	if (redrawPic) {
		_roomOnScreen = _state.room;
		_picOnScreen = _state.curPicture;

		drawPic(_state.curPicture);
		_itemRemoved = false;
		_itemsOnScreen = 0;

		Common::List<Item>::iterator item;
		for (item = _state.items.begin(); item != _state.items.end(); ++item)
			item->isOnScreen = false;
	}

	if (!_state.isDark)
		drawItems();

	_display->updateHiResScreen();
	printString(_roomData.description);

	// FIXME: move to main loop?
	_linesPrinted = 0;
}

// TODO: Merge this into AdlEngine?
void AdlEngine_v2::takeItem(byte noun) {
	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun == noun && item->room == _state.room && item->region == _state.region) {
			if (item->state == IDI_ITEM_DOESNT_MOVE) {
				printMessage(_messageIds.itemDoesntMove);
				return;
			}

			if (item->state == IDI_ITEM_DROPPED) {
				item->room = IDI_ANY;
				_itemRemoved = true;
				return;
			}

			Common::Array<byte>::const_iterator pic;
			for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
				if (*pic == getCurRoom().curPicture || *pic == IDI_ANY) {
					if (!isInventoryFull()) {
						item->room = IDI_ANY;
						_itemRemoved = true;
						item->state = IDI_ITEM_DROPPED;
					}
					return;
				}
			}
		}
	}

	printMessage(_messageIds.itemNotHere);
}

void AdlEngine_v2::drawItems() {
	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		// Skip items not in this room
		if (item->region == _state.region && item->room == _state.room && !item->isOnScreen) {
			if (item->state == IDI_ITEM_DROPPED) {
				// Draw dropped item if in normal view
				if (getCurRoom().picture == getCurRoom().curPicture)
					drawItem(*item, _itemOffsets[_itemsOnScreen++]);
			} else {
				// Draw fixed item if current view is in the pic list
				Common::Array<byte>::const_iterator pic;

				for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
					if (*pic == _state.curPicture || *pic == IDI_ANY) {
						drawItem(*item, item->position);
						break;
					}
				}
			}
		}
	}
}

DataBlockPtr AdlEngine_v2::readDataBlockPtr(Common::ReadStream &f) const {
	byte track = f.readByte();
	byte sector = f.readByte();
	byte offset = f.readByte();
	byte size = f.readByte();

	if (f.eos() || f.err())
		error("Error reading DataBlockPtr");

	if (track == 0 && sector == 0 && offset == 0 && size == 0)
		return DataBlockPtr();

	adjustDataBlockPtr(track, sector, offset, size);

	return _disk->getDataBlock(track, sector, offset, size);
}

void AdlEngine_v2::loadItems(Common::ReadStream &stream) {
	byte id;
	while ((id = stream.readByte()) != 0xff && !stream.eos() && !stream.err()) {
		Item item;

		item.id = id;
		item.noun = stream.readByte();
		item.room = stream.readByte();
		item.picture = stream.readByte();
		item.region = stream.readByte();
		item.position.x = stream.readByte();
		item.position.y = stream.readByte();
		item.state = stream.readByte();
		item.description = stream.readByte();

		stream.readByte(); // Struct size

		byte picListSize = stream.readByte();

		// Flag to keep track of what has been drawn on the screen
		stream.readByte();

		for (uint i = 0; i < picListSize; ++i)
			item.roomPictures.push_back(stream.readByte());

		_state.items.push_back(item);
	}

	if (stream.eos() || stream.err())
		error("Error loading items");
}

void AdlEngine_v2::loadRooms(Common::ReadStream &stream, byte count) {
	for (uint i = 0; i < count; ++i) {
		Room room;

		stream.readByte(); // number
		for (uint j = 0; j < 6; ++j)
			room.connections[j] = stream.readByte();
		room.data = readDataBlockPtr(stream);
		room.picture = stream.readByte();
		room.curPicture = stream.readByte();
		room.isFirstTime = stream.readByte();

		_state.rooms.push_back(room);
	}

	if (stream.eos() || stream.err())
		error("Error loading rooms");
}

void AdlEngine_v2::loadMessages(Common::ReadStream &stream, byte count) {
	for (uint i = 0; i < count; ++i)
		_messages.push_back(readDataBlockPtr(stream));
}

void AdlEngine_v2::loadPictures(Common::ReadStream &stream) {
	byte picNr;
	while ((picNr = stream.readByte()) != 0xff) {
		if (stream.eos() || stream.err())
			error("Error reading global pic list");

		_pictures[picNr] = readDataBlockPtr(stream);
	}
}

void AdlEngine_v2::loadItemPictures(Common::ReadStream &stream, byte count) {
	for (uint i = 0; i < count; ++i) {
		stream.readByte(); // number
		_itemPics.push_back(readDataBlockPtr(stream));
	}
}

int AdlEngine_v2::o2_isFirstTime(ScriptEnv &e) {
	OP_DEBUG_0("\t&& IS_FIRST_TIME()");

	bool oldFlag = getCurRoom().isFirstTime;

	getCurRoom().isFirstTime = false;

	if (!oldFlag)
		return -1;

	return 0;
}

int AdlEngine_v2::o2_isRandomGT(ScriptEnv &e) {
	OP_DEBUG_1("\t&& RAND() > %d", e.arg(1));

	byte rnd = _random->getRandomNumber(255);

	if (rnd > e.arg(1))
		return 1;

	return -1;
}

int AdlEngine_v2::o2_isNounNotInRoom(ScriptEnv &e) {
	OP_DEBUG_1("\t&& NO_SUCH_ITEMS_IN_ROOM(%s)", itemRoomStr(e.arg(1)).c_str());

	Common::List<Item>::const_iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->noun == e.getNoun() && (item->room == roomArg(e.arg(1))))
			return -1;

	return 1;
}

int AdlEngine_v2::o2_isCarryingSomething(ScriptEnv &e) {
	OP_DEBUG_0("\t&& IS_CARRYING_SOMETHING()");

	Common::List<Item>::const_iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->room == IDI_ANY)
			return 0;
	return -1;
}

int AdlEngine_v2::o2_moveItem(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_ITEM_ROOM(%s, %s)", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	byte room = roomArg(e.arg(2));

	Item &item = getItem(e.arg(1));

	if (item.room == _roomOnScreen)
		_picOnScreen = 0;

	// Set items that move from inventory to a room to state "dropped"
	if (item.room == IDI_ANY && room != IDI_VOID_ROOM)
		item.state = IDI_ITEM_DROPPED;

	item.room = room;
	return 2;
}

int AdlEngine_v2::o2_setCurPic(ScriptEnv &e) {
	OP_DEBUG_1("\tSET_CURPIC(%d)", e.arg(1));

	getCurRoom().curPicture = _state.curPicture = e.arg(1);
	return 1;
}

int AdlEngine_v2::o2_setPic(ScriptEnv &e) {
	OP_DEBUG_1("\tSET_PIC(%d)", e.arg(1));

	getCurRoom().picture = getCurRoom().curPicture = _state.curPicture = e.arg(1);
	return 1;
}

int AdlEngine_v2::o2_moveAllItems(ScriptEnv &e) {
	OP_DEBUG_2("\tMOVE_ALL_ITEMS(%s, %s)", itemRoomStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	byte room1 = roomArg(e.arg(1));

	if (room1 == _state.room)
		_picOnScreen = 0;

	byte room2 = roomArg(e.arg(2));

	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->room == room1) {
			item->room = room2;
			if (room1 == IDI_ANY)
				item->state = IDI_ITEM_DROPPED;
		}

	return 2;
}

int AdlEngine_v2::o2_save(ScriptEnv &e) {
	OP_DEBUG_0("\tSAVE_GAME()");

	int slot = askForSlot(_strings_v2.saveInsert);

	if (slot < 0)
		return -1;

	saveGameState(slot, "");

	_display->printString(_strings_v2.saveReplace);
	inputString();
	return 0;
}

int AdlEngine_v2::o2_restore(ScriptEnv &e) {
	OP_DEBUG_0("\tRESTORE_GAME()");

	int slot = askForSlot(_strings_v2.restoreInsert);

	if (slot < 0)
		return -1;

	loadGameState(slot);
	_isRestoring = false;

	_display->printString(_strings_v2.restoreReplace);
	inputString();
	_picOnScreen = 0;
	_roomOnScreen = 0;
	return 0;
}

int AdlEngine_v2::o2_placeItem(ScriptEnv &e) {
	OP_DEBUG_4("\tPLACE_ITEM(%s, %s, (%d, %d))", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str(), e.arg(3), e.arg(4));

	Item &item = getItem(e.arg(1));

	item.room = roomArg(e.arg(2));
	item.position.x = e.arg(3);
	item.position.y = e.arg(4);
	item.state = IDI_ITEM_NOT_MOVED;

	return 4;
}

int AdlEngine_v2::o2_tellTime(ScriptEnv &e) {
	OP_DEBUG_0("\tTELL_TIME()");

	Common::String time = _strings_v2.time;

	time.setChar(APPLECHAR('0') + _state.time.hours / 10, 12);
	time.setChar(APPLECHAR('0') + _state.time.hours % 10, 13);
	time.setChar(APPLECHAR('0') + _state.time.minutes / 10, 15);
	time.setChar(APPLECHAR('0') + _state.time.minutes % 10, 16);

	printString(time);

	return 0;
}

int AdlEngine_v2::o2_setRoomFromVar(ScriptEnv &e) {
	OP_DEBUG_1("\tROOM = VAR[%d]", e.arg(1));
	getCurRoom().curPicture = getCurRoom().picture;
	_state.room = getVar(e.arg(1));
	return 1;
}

int AdlEngine_v2::o2_initDisk(ScriptEnv &e) {
	OP_DEBUG_0("\tINIT_DISK()");

	_display->printAsciiString("NOT REQUIRED\r");
	return 0;
}

int AdlEngine_v2::askForSlot(const Common::String &question) {
	while (1) {
		_display->printString(question);

		Common::String input = inputString();

		if (shouldQuit())
			return -1;

		if (input.size() > 0 && input[0] >= APPLECHAR('A') && input[0] <= APPLECHAR('O'))
			return input[0] - APPLECHAR('A');
	}
}

} // End of namespace Adl
