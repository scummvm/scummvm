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

#include "common/str.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/debugger.h"
#include "harold/bcb/script.h"

namespace BCB {

Debugger::Debugger(BCBEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("time", WRAP_METHOD(Debugger, cmdTime));
	registerCmd("add_time", WRAP_METHOD(Debugger, cmdAddTime));
	registerCmd("seq", WRAP_METHOD(Debugger, cmdSequence));
	registerCmd("state", WRAP_METHOD(Debugger, cmdState));
	registerCmd("flag", WRAP_METHOD(Debugger, cmdFlag));
	registerCmd("choice", WRAP_METHOD(Debugger, cmdChoice));
	registerCmd("gameover", WRAP_METHOD(Debugger, cmdGameOver));
	registerCmd("clock_chip", WRAP_METHOD(Debugger, cmdClockChip));
	registerCmd("night", WRAP_METHOD(Debugger, cmdNight));
}

bool Debugger::parseNumber(const char *text, uint32 &value) const {
	return _vm->parseCheatNumber(Common::String(text), value);
}

Common::String Debugger::formatGameClock(uint32 gameClockTicks) const {
	const uint clockTicks = gameClockTicks % 1440;
	const uint dayOfMonth = 10 + CLIP<uint32>(_vm->stateDword(kStateRequiredDiscIndex), 0, 4);
	const uint hour = clockTicks / 60;
	const uint minute = clockTicks % 60;

	return Common::String::format("11/%02u %02u:%02u", dayOfMonth, hour, minute);
}

bool Debugger::cmdTime(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Format: time [ticks]\n");
		debugPrintf("Current: %s, ticks=%u / 0x%04x\n",
					formatGameClock(_vm->stateDword(kStateGameClockTicks)).c_str(),
					_vm->stateDword(kStateGameClockTicks),
					_vm->stateDword(kStateGameClockTicks));
		return true;
	}

	if (argc == 2) {
		uint32 ticks = 0;
		if (!parseNumber(argv[1], ticks)) {
			debugPrintf("Invalid tick value: %s\n", argv[1]);
			return true;
		}
		_vm->stateDword(kStateGameClockTicks) = ticks;
		_vm->notifyGameClockTicksChanged();
		if (_vm->_currentDisplayState == 1 || _vm->_currentDisplayState == 2 ||
			_vm->_currentDisplayState == 7 || _vm->_currentDisplayState == 8)
			_vm->drawClockAndChipIndicator();
	}

	debugPrintf("Current: %s, ticks=%u / 0x%04x\n",
				formatGameClock(_vm->stateDword(kStateGameClockTicks)).c_str(),
				_vm->stateDword(kStateGameClockTicks),
				_vm->stateDword(kStateGameClockTicks));
	return true;
}

bool Debugger::cmdAddTime(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: add_time <minutes>\n");
		return true;
	}

	uint32 minutes = 0;
	if (!parseNumber(argv[1], minutes)) {
		debugPrintf("Invalid minute value: %s\n", argv[1]);
		return true;
	}

	_vm->addCheatTime((int)minutes);
	debugPrintf("Current: %s, ticks=%u / 0x%04x\n",
				formatGameClock(_vm->stateDword(kStateGameClockTicks)).c_str(),
				_vm->stateDword(kStateGameClockTicks),
				_vm->stateDword(kStateGameClockTicks));
	return true;
}

bool Debugger::cmdSequence(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: seq <sequence-id>\n");
		return true;
	}

	uint32 sequenceId = 0;
	if (!parseNumber(argv[1], sequenceId)) {
		debugPrintf("Invalid sequence id: %s\n", argv[1]);
		return true;
	}

	_vm->jumpToCheatSequence(sequenceId);
	return false;
}

bool Debugger::cmdState(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Format: state <byte-offset> [value]\n");
		return true;
	}

	uint32 byteOffset = 0;
	if (!parseNumber(argv[1], byteOffset)) {
		debugPrintf("Invalid state offset: %s\n", argv[1]);
		return true;
	}

	if (argc == 3) {
		uint32 value = 0;
		if (!parseNumber(argv[2], value)) {
			debugPrintf("Invalid value: %s\n", argv[2]);
			return true;
		}
		_vm->stateDword(byteOffset) = value;
	}

	debugPrintf("state[0x%04x] = %u / 0x%08x\n", byteOffset, _vm->stateDword(byteOffset), _vm->stateDword(byteOffset));
	return true;
}

bool Debugger::cmdFlag(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Format: flag <secondary-word-index> [value]\n");
		return true;
	}

	uint32 flagIndex = 0;
	if (!parseNumber(argv[1], flagIndex)) {
		debugPrintf("Invalid flag index: %s\n", argv[1]);
		return true;
	}

	const uint32 byteOffset = 0x98 + flagIndex * 4;
	if (argc == 3) {
		uint32 value = 0;
		if (!parseNumber(argv[2], value)) {
			debugPrintf("Invalid value: %s\n", argv[2]);
			return true;
		}
		_vm->stateDword(byteOffset) = value;
	}

	debugPrintf("secondary[%u] state[0x%04x] = %u / 0x%08x\n",
				flagIndex, byteOffset, _vm->stateDword(byteOffset), _vm->stateDword(byteOffset));
	return true;
}

bool Debugger::cmdChoice(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Format: choice [value]\n");
		return true;
	}

	if (argc == 2) {
		uint32 value = 0;
		if (!parseNumber(argv[1], value)) {
			debugPrintf("Invalid choice value: %s\n", argv[1]);
			return true;
		}
		_vm->stateDword(kStateSelectedSceneChoiceValue) = value;
	}

	debugPrintf("selectedSceneChoiceValue = %u / 0x%02x\n",
				_vm->stateDword(kStateSelectedSceneChoiceValue),
				_vm->stateDword(kStateSelectedSceneChoiceValue));
	return true;
}

bool Debugger::cmdGameOver(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Format: gameover <1..9>\n");
		return true;
	}

	uint32 messageIndex = 0;
	if (!parseNumber(argv[1], messageIndex) || messageIndex < 1 || messageIndex > 9) {
		debugPrintf("Invalid game-over message index: %s\n", argv[1]);
		return true;
	}

	_vm->stateDword(kStateSelectedSceneChoiceValue) = messageIndex;
	if (!_vm->dispatchGameOverScreen())
		debugPrintf("Could not display game-over screen %u\n", messageIndex);
	return false;
}

bool Debugger::cmdClockChip(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Format: clock_chip [0..4]\n");
		return true;
	}

	if (argc == 2) {
		uint32 chipIndex = 0;
		if (!parseNumber(argv[1], chipIndex) || chipIndex > 4) {
			debugPrintf("Invalid clock/chip index: %s\n", argv[1]);
			return true;
		}
		_vm->stateDword(kStateRequiredDiscIndex) = chipIndex;
		if (_vm->_currentDisplayState == 1 || _vm->_currentDisplayState == 2 ||
			_vm->_currentDisplayState == 7 || _vm->_currentDisplayState == 8)
			_vm->drawClockAndChipIndicator();
	}

	debugPrintf("clock/chip state index = %u (state[0x%04x])\n",
				_vm->stateDword(kStateRequiredDiscIndex), kStateRequiredDiscIndex);
	return true;
}

bool Debugger::cmdNight(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Format: night [0|1]\n");
		return true;
	}

	if (argc == 2) {
		uint32 value = 0;
		if (!parseNumber(argv[1], value)) {
			debugPrintf("Invalid night flag: %s\n", argv[1]);
			return true;
		}
		_vm->stateDword(kStateMainSceneNightFlag) = value != 0 ? 1 : 0;
	}

	debugPrintf("mainSceneNightFlag = %u\n", _vm->stateDword(kStateMainSceneNightFlag));
	return true;
}

bool BCBEngine::parseCheatNumber(const Common::String &text, uint32 &value) const {
	if (text.empty())
		return false;

	const char *cursor = text.c_str();
	int base = 10;
	if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
		cursor += 2;
		base = 16;
	}

	char *end = nullptr;
	const unsigned long parsedValue = strtoul(cursor, &end, base);
	if (!end || *end != 0)
		return false;

	value = (uint32)parsedValue;
	return true;
}

void BCBEngine::addCheatTime(int minutes) {
	stateDword(kStateGameClockTicks) += minutes;
	notifyGameClockTicksChanged();

	if (_currentDisplayState == 1 || _currentDisplayState == 2 || _currentDisplayState == 7 || _currentDisplayState == 8)
		drawClockAndChipIndicator();
}

void BCBEngine::jumpToCheatSequence(uint sequenceId) {
	if (sequenceId == 0 || sequenceId * 2 > _scriptData.size()) {
		warning("BCBEngine::jumpToCheatSequence(): Invalid sequence id %u", sequenceId);
		return;
	}

	stopActiveMovie();
	stopSceneIdleMovie();
	stopMapMoveMovie();
	restoreActiveCaptionBackingRect();
	freeSceneChoiceBacking();
	_sceneChoiceActive = false;
	_mapControlsActive = false;
	_scriptStopFlag = false;
	stateDword(kStateMovieToPlayId) = 0;
	stateDword(kStateStillImageToShowId) = 0;
	stateDword(kStateDispatcherControlCode) = 0;
	stateDword(kStateCurrentScriptSequenceId) = sequenceId;
	processGameStateDispatcher(true);
}

} // End of namespace BCB
