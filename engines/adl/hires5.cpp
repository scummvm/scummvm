/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/stream.h"

#include "adl/adl_v4.h"
#include "adl/detection.h"
#include "adl/display_a2.h"
#include "adl/graphics.h"
#include "adl/disk.h"
#include "adl/sound.h"

namespace Adl {

class HiRes5Engine : public AdlEngine_v4 {
public:
	HiRes5Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine_v4(syst, gd),
			_doAnimation(false) { }

private:
	// AdlEngine
	void setupOpcodeTables() override;
	void runIntro() override;
	void init() override;
	void initGameState() override;
	void applyRegionWorkarounds() override;
	void applyRoomWorkarounds(byte roomNr) override;
	Common::String getLine() override;

	// AdlEngine_v4
	bool isInventoryFull() override;

	void loadSong(Common::ReadStream &stream);
	void drawLight(uint index, byte color) const;
	void animateLights() const;

	int o_checkItemTimeLimits(ScriptEnv &e);
	int o_startAnimation(ScriptEnv &e);
	int o_winGame(ScriptEnv &e);

	static const uint kClock = 1022727; // Apple II CPU clock rate
	static const uint kRegions = 41;
	static const uint kItems = 69;

	Common::Array<byte> _itemTimeLimits;
	Common::String _itemTimeLimitMsg;
	Tones _song;
	bool _doAnimation;

	struct {
		Common::String itemTimeLimit;
		Common::String carryingTooMuch;
	} _gameStrings;
};

Common::String HiRes5Engine::getLine() {
	if (_doAnimation) {
		animateLights();
		_doAnimation = false;
	}

	return AdlEngine_v4::getLine();
}

void HiRes5Engine::drawLight(uint index, byte color) const {
	Display_A2 *display = static_cast<Display_A2 *>(_display);
	const byte xCoord[5] = { 189, 161, 133, 105, 77 };
	const byte yCoord = 72;

	assert(index < 5);

	for (int yDelta = 0; yDelta < 4; ++yDelta)
		for (int xDelta = 0; xDelta < 7; ++xDelta)
			display->putPixel(Common::Point(xCoord[index] + xDelta, yCoord + yDelta), color);

	display->renderGraphics();
}

void HiRes5Engine::animateLights() const {
	// Skip this if we're running a debug script
	if (_inputScript)
		return;

	int index;
	byte color = 0x2a;

	for (index = 4; index >= 0; --index)
		drawLight(index, color);

	index = 4;

	while (!g_engine->shouldQuit()) {
		drawLight(index, color ^ 0x7f);

		// There's a delay here in the original engine. We leave it out as
		// we're already slower than the original without any delay.

		const uint kLoopCycles = 25;
		const byte period = (index + 1) << 4;
		const double freq = kClock / 2.0 / (period * kLoopCycles);
		const double len = 128 * period * kLoopCycles * 1000 / (double)kClock;

		Tones tone;
		tone.push_back(Tone(freq, len));

		if (playTones(tone, false, true))
			break;

		drawLight(index, color ^ 0xff);

		if (--index < 0) {
			index = 4;
			color ^= 0xff;
		}
	}
}

void HiRes5Engine::setupOpcodeTables() {
	AdlEngine_v4::setupOpcodeTables();

	_actOpcodes[0x0b] = opcode(&HiRes5Engine::o_checkItemTimeLimits);
	_actOpcodes[0x13] = opcode(&HiRes5Engine::o_startAnimation);
	_actOpcodes[0x1e] = opcode(&HiRes5Engine::o_winGame);
}

bool HiRes5Engine::isInventoryFull() {
	Common::List<Item>::const_iterator item;
	byte weight = 0;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->room == IDI_ANY)
			weight += item->description;
	}

	if (weight >= 100) {
		_display->printString(_gameStrings.carryingTooMuch);
		inputString();
		return true;
	}

	return false;
}

void HiRes5Engine::loadSong(Common::ReadStream &stream) {
	while (true) {
		const byte period = stream.readByte();

		if (stream.err() || stream.eos())
			error("Error loading song");

		if (period == 0xff)
			return;

		byte length = stream.readByte();

		if (stream.err() || stream.eos())
			error("Error loading song");

		const uint kLoopCycles = 20; // Delay loop cycles

		double freq = 0.0;

		// This computation ignores CPU cycles spent on overflow handling and
		// speaker control. As a result, our tone frequencies will be slightly
		// higher than those on original hardware.
		if (period != 0)
			freq = kClock / 2.0 / (period * kLoopCycles);

		const double len = (length > 0 ? length - 1 : 255) * 256 * kLoopCycles * 1000 / (double)kClock;

		_song.push_back(Tone(freq, len));
	}
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
		_display->printString(_gameStrings.itemTimeLimit);
		inputString();
	}

	return 1;
}

int HiRes5Engine::o_startAnimation(ScriptEnv &e) {
	OP_DEBUG_0("\tSTART_ANIMATION()");

	_doAnimation = true;

	return 0;
}

int HiRes5Engine::o_winGame(ScriptEnv &e) {
	OP_DEBUG_0("\tWIN_GAME()");

	showRoom();
	playTones(_song, true);

	return o_quit(e);
}

void HiRes5Engine::runIntro() {
	Display_A2 *display = static_cast<Display_A2 *>(_display);

	insertDisk(2);

	Common::StreamPtr stream(_disk->createReadStream(0x10, 0x0, 0x00, 31));

	display->setMode(Display::kModeGraphics);
	display->loadFrameBuffer(*stream);
	display->renderGraphics();

	inputKey();

	display->home();
	display->setMode(Display::kModeText);

	stream.reset(_disk->createReadStream(0x03, 0xc, 0x34, 1));
	Common::String menu(readString(*stream));

	while (!g_engine->shouldQuit()) {
		display->home();
		display->printString(menu);

		Common::String cmd(inputString());

		// We ignore the backup and format menu options
		if (!cmd.empty() && cmd[0] == _display->asciiToNative('1'))
			break;
	};
}

void HiRes5Engine::init() {
	_graphics = new GraphicsMan_v3<Display_A2>(*static_cast<Display_A2 *>(_display));

	insertDisk(2);

	Common::StreamPtr stream(_disk->createReadStream(0x5, 0x0, 0x02));
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

	stream.reset(_disk->createReadStream(0xc, 0xb, 0x20));
	loadSong(*stream);
}

void HiRes5Engine::initGameState() {
	_state.vars.resize(40);

	insertDisk(2);

	Common::StreamPtr stream(_disk->createReadStream(0x5, 0x1, 0x00, 3));
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

	_doAnimation  = false;
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
		break;
	case 32:
		// This broken message appears right before the game restarts,
		// and should probably explain that the user fell down some
		// stairs. We remove the broken message.
		// TODO: Maybe we could put in a new string?
		removeMessage(29);
		break;
	case 40:
		// Locking the gate prints a broken message, followed by
		// "O.K.". Maybe there was supposed to be a more elaborate
		// message, in the style of the one printed when you unlock
		// the gate. But "O.K." should be enough, so we remove the
		// first one.
		removeMessage(172);
		break;
	default:
		break;
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
