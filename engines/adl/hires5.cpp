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
	void setupOpcodeTables();
	void runIntro();
	void init();
	void initGameState();
	void applyRegionWorkarounds();
	void applyRoomWorkarounds(byte roomNr);

	// AdlEngine_v4
	bool isInventoryFull();

	int o_checkItemTimeLimits(ScriptEnv &e);
	int o_startAnimation(ScriptEnv &e);
	int o_winGame(ScriptEnv &e);

	static const uint kRegions = 41;
	static const uint kItems = 69;

	Common::Array<byte> _itemTimeLimits;
	Common::String _itemTimeLimitMsg;

	struct {
		Common::String itemTimeLimit;
		Common::String carryingTooMuch;
	} _gameStrings;
};

typedef Common::Functor1Mem<ScriptEnv &, int, HiRes5Engine> OpcodeH5;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeH5(this, &HiRes5Engine::x))
#define OpcodeUnImpl() table->push_back(new OpcodeH5(this, 0))

void HiRes5Engine::setupOpcodeTables() {
	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_condOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o2_isFirstTime);
	Opcode(o2_isRandomGT);
	Opcode(o4_isItemInRoom);
	// 0x04
	Opcode(o3_isNounNotInRoom);
	Opcode(o1_isMovesGT);
	Opcode(o1_isVarEQ);
	Opcode(o2_isCarryingSomething);
	// 0x08
	Opcode(o4_isVarGT);
	Opcode(o1_isCurPicEQ);
	OpcodeUnImpl();

	SetOpcodeTable(_actOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o1_varAdd);
	Opcode(o1_varSub);
	Opcode(o1_varSet);
	// 0x04
	Opcode(o1_listInv);
	Opcode(o4_moveItem);
	Opcode(o1_setRoom);
	Opcode(o2_setCurPic);
	// 0x08
	Opcode(o2_setPic);
	Opcode(o1_printMsg);
	Opcode(o4_setRegionToPrev);
	Opcode(o_checkItemTimeLimits);
	// 0x0c
	Opcode(o4_moveAllItems);
	Opcode(o1_quit);
	Opcode(o4_setRegion);
	Opcode(o4_save);
	// 0x10
	Opcode(o4_restore);
	Opcode(o4_restart);
	Opcode(o4_setRegionRoom);
	Opcode(o_startAnimation);
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
	Opcode(o4_setRoomPic);
	Opcode(o_winGame);
	OpcodeUnImpl();
	// 0x20
	Opcode(o2_initDisk);
}

bool HiRes5Engine::isInventoryFull() {
	Common::List<Item>::const_iterator item;
	byte weight = 0;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->room == IDI_ANY)
			weight += item->description;
	}

	if (weight >= 100) {
		printString(_gameStrings.carryingTooMuch);
		inputString();
		return true;
	}

	return false;
}

int HiRes5Engine::o_checkItemTimeLimits(ScriptEnv &e) {
	OP_DEBUG_1("\tCHECK_ITEM_TIME_LIMITS(VARS[%d])", e.arg(1));

	bool lostAnItem = false;
	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		const byte room = item->room;
		const byte region = item->region;

		if (room == IDI_ANY || room == IDI_CUR_ROOM || (room == _state.room && region == _state.region)) {
			if (getVar(e.arg(1)) < _itemTimeLimits[item->id - 1]) {
				item->room = IDI_VOID_ROOM;
				lostAnItem = true;
			}
		}
	}

	if (lostAnItem) {
		printString(_gameStrings.itemTimeLimit);
		inputString();
	}

	return 1;
}

int HiRes5Engine::o_startAnimation(ScriptEnv &e) {
	OP_DEBUG_0("\tSTART_ANIMATION()");

	// TODO: sets a flag that triggers an animation

	return 0;
}

int HiRes5Engine::o_winGame(ScriptEnv &e) {
	OP_DEBUG_0("\tWIN_GAME()");

	// TODO: draws room and plays music

	return o1_quit(e);
}

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

	stream.reset(_disk->createReadStream(0x8, 0x3, 0x00, 2));
	_strings_v2.saveInsert = readStringAt(*stream, 0x66);
	_strings_v2.saveReplace = readStringAt(*stream, 0x112);
	_strings_v2.restoreInsert = readStringAt(*stream, 0x180);
	_strings.playAgain = readStringAt(*stream, 0x247, 0xff);

	_messageIds.cantGoThere = 110;
	_messageIds.dontUnderstand = 112;
	_messageIds.itemDoesntMove = 114;
	_messageIds.itemNotHere = 115;
	_messageIds.thanksForPlaying = 113;

	stream.reset(_disk->createReadStream(0xe, 0x1, 0x13, 4));
	loadItemDescriptions(*stream, kItems);

	stream.reset(_disk->createReadStream(0x8, 0xd, 0xfd, 1));
	loadDroppedItemOffsets(*stream, 16);

	stream.reset(_disk->createReadStream(0xb, 0xa, 0x05, 1));
	loadItemPicIndex(*stream, kItems);

	stream.reset(_disk->createReadStream(0x7, 0x8, 0x01));
	for (uint i = 0; i < kItems; ++i)
		_itemTimeLimits.push_back(stream->readByte());

	if (stream->eos() || stream->err())
		error("Failed to read item time limits");

	stream.reset(_disk->createReadStream(0x8, 0x2, 0x2d));
	_gameStrings.itemTimeLimit = readString(*stream);

	stream.reset(_disk->createReadStream(0x8, 0x7, 0x02));
	_gameStrings.carryingTooMuch = readString(*stream);
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

void HiRes5Engine::applyRegionWorkarounds() {
	// WORKAROUND: Remove/fix buggy commands
	switch (_state.region) {
	case 3:
		// "USE PIN" references a missing message, but cannot
		// be triggered due to shadowing of the "USE" verb.
		// We remove it anyway to allow script dumping to proceed.
		// TODO: Investigate if we should fix this command instead
		// of removing it.
		removeCommand(_roomCommands, 12);
		break;
	case 14:
		// "WITH SHOVEL" references a missing message. This bug
		// is game-breaking in the original, but unlikely to occur
		// in practice due to the "DIG" command not asking for what
		// to dig with. Probably a remnant of an earlier version
		// of the script.
		removeCommand(_roomCommands, 0);
	}
}

void HiRes5Engine::applyRoomWorkarounds(byte roomNr) {
	// WORKAROUND: Remove/fix buggy commands
	if (_state.region == 17 && roomNr == 49) {
		// "GET WATER" references a missing message when you already
		// have water. This message should be 117 instead of 17.
		getCommand(_roomData.commands, 8).script[4] = 117;
	}
}

Engine *HiRes5Engine_create(OSystem *syst, const AdlGameDescription *gd) {
	return new HiRes5Engine(syst, gd);
}

} // End of namespace Adl
