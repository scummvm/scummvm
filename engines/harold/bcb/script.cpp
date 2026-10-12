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

#include "common/algorithm.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"

namespace BCB {

static const uint32 kScriptFalse = 0xc000;
static const uint32 kScriptTrue = 0xffff;
static const uint32 kScriptEndExpression = 0x7fff;
static const uint32 kScriptSecondaryStateByteOffset = 0x98;

bool BCBEngine::loadScriptData() {
	if (!loadByteFile("check.bin", _scriptData))
		return false;

	if (!loadByteFile("l_tbl.bin", _locationTableData))
		return false;

	if (!loadByteFile("l_tim.bin", _locationTimingData))
		return false;

	if (!loadByteFile("all.bin", _allBinData))
		return false;

	return true;
}

bool BCBEngine::loadByteFile(const char *fileName, Common::Array<byte> &data) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(createProgramReadStream(fileName));
	if (!stream)
		return false;

	const int32 fileSize = stream->size();
	if (fileSize <= 0)
		return false;

	data.resize(fileSize);
	return stream->read(&data[0], fileSize) == (uint32)fileSize;
}

void BCBEngine::resetGameState() {
	Common::fill(&_gameState[0], &_gameState[ARRAYSIZE(_gameState)], 0);
	stateDword(kStateGameClockTicks) = 0x474;
	stateDword(kStateCurrentScriptSequenceId) = 1;
	_pendingStillElapsedTimerTicks = 0;
	_pendingStillDurationIndex = 0;
	_gameOverElapsedTimerTicks = 0;
	_pendingClockTickDelta = 0;
	_sceneChoiceActive = false;
	_sceneChoiceTerminatorButtonIndex = 0;
	_sceneChoiceEnabledButtonLimit = 0;
	_timedCaptionRecordData.clear();
	_timedCaptionCursorOffset = 0;
	_nextTimedCaptionThreshold = 99999;
	_queuedMoviePlaybackIndex = 0;
	_previousDisplayStateBeforeSystem = 0;
	_timedCaptionText.clear();
	_scriptStopFlag = false;
}

void BCBEngine::executeCurrentScriptSequence() {
	stateDword(kStateDispatcherControlCode) = 0;

	const uint sequenceId = stateDword(kStateCurrentScriptSequenceId);
	const uint sequenceTableOffset = sequenceId * 2 - 2;
	if (sequenceId == 0 || sequenceTableOffset + 1 >= _scriptData.size()) {
		warning("BCBEngine::executeCurrentScriptSequence(): Invalid sequence id %u", sequenceId);
		_scriptStopFlag = true;
		return;
	}

	uint32 scriptPc = _scriptData[sequenceTableOffset] | (_scriptData[sequenceTableOffset + 1] << 8);
	while (!_scriptStopFlag) {
		int branchTokenSeen = -1;
		const uint32 result = executeScriptExpression(scriptPc, branchTokenSeen);
		if (_scriptStopFlag || branchTokenSeen != 0)
			break;

		if (scriptPc + 1 >= _scriptData.size()) {
			_scriptStopFlag = true;
			break;
		}

		const int branchOffset = (int8)_scriptData[scriptPc] + (int8)_scriptData[scriptPc + 1] * 0x100;
		if ((result & 0x1fff) == 0)
			scriptPc += branchOffset;
		else
			scriptPc += 2;
	}
}

uint32 BCBEngine::executeScriptExpression(uint32 &scriptPc, int &branchTokenSeen) {
	_scriptStackDepth = 0;

	while (!_scriptStopFlag) {
		if (scriptPc + 1 >= _scriptData.size()) {
			_scriptStopFlag = true;
			return 0xffffffff;
		}

		const uint32 token = _scriptData[scriptPc] | (_scriptData[scriptPc + 1] << 8);
		scriptPc += 2;

		if (((token >> 8) & 0xc0) != 0x40) {
			pushScriptValue(token);
			continue;
		}

		if (token == kScriptEndExpression) {
			if (_scriptStackDepth == 0)
				_scriptStackDepth = 1;
			return popScriptValue();
		}

		const uint opcode = token & 0xff;
		uint32 leftValue = 0;
		uint32 rightValue = 0;
		switch (opcode) {
		case 0x00:
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue & leftValue);
			break;
		case 0x01:
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue | leftValue);
			break;
		case 0x02:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue == leftValue ? kScriptTrue : kScriptFalse);
			break;
		case 0x03:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue != leftValue ? kScriptTrue : kScriptFalse);
			break;
		case 0x04: {
			uint32 assignedValue = popScriptValue();
			assignedValue = dereferenceScriptValue(assignedValue);
			const uint32 encodedOperand = popScriptValue();
			if (((encodedOperand >> 8) & 0xc0) != 0x80) {
				_scriptStopFlag = true;
				return 0xffffffff;
			}
			const uint byteOffset = (encodedOperand & 0x1ffe) * 2;
			const bool writesPrimaryState = (encodedOperand & 0x2000) == 0;
			scriptOperandDword(encodedOperand) = assignedValue;
			if (writesPrimaryState && byteOffset == kStateGameClockTicks)
				notifyGameClockTicksChanged();
			pushScriptValue(assignedValue == 0 ? kScriptFalse : kScriptTrue);
			break;
		}
		case 0x05:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(leftValue < rightValue ? kScriptTrue : kScriptFalse);
			break;
		case 0x06:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(leftValue > rightValue ? kScriptTrue : kScriptFalse);
			break;
		case 0x07:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(leftValue <= rightValue ? kScriptTrue : kScriptFalse);
			break;
		case 0x08:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(leftValue >= rightValue ? kScriptTrue : kScriptFalse);
			break;
		case 0x09:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue & leftValue);
			break;
		case 0x0a:
			branchTokenSeen = 0;
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue | leftValue);
			break;
		case 0x0b:
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue ^ leftValue);
			break;
		case 0x0c:
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(rightValue + leftValue);
			break;
		case 0x0d:
			popScriptOperands(leftValue, rightValue);
			pushScriptValue(leftValue - rightValue);
			break;
		default:
			warning("BCBEngine::executeScriptExpression(): Unknown script opcode 0x%02x", opcode);
			_scriptStopFlag = true;
			return 0xffffffff;
		}
	}

	return 0xffffffff;
}

bool BCBEngine::pushScriptValue(uint32 value) {
	if (_scriptStackDepth < 0 || _scriptStackDepth >= ARRAYSIZE(_scriptValueStack)) {
		_scriptStopFlag = true;
		return false;
	}

	_scriptValueStack[_scriptStackDepth++] = value;
	return true;
}

uint32 BCBEngine::popScriptValue() {
	if (_scriptStackDepth <= 0 || _scriptStackDepth > ARRAYSIZE(_scriptValueStack)) {
		_scriptStopFlag = true;
		return 0xffffffff;
	}

	return _scriptValueStack[--_scriptStackDepth];
}

void BCBEngine::popScriptOperands(uint32 &leftValue, uint32 &rightValue) {
	rightValue = dereferenceScriptValue(popScriptValue());
	leftValue = dereferenceScriptValue(popScriptValue());
}

uint32 BCBEngine::dereferenceScriptValue(uint32 value) {
	if (((value >> 8) & 0xc0) == 0x80)
		return scriptOperandDword(value);

	return value;
}

uint32 &BCBEngine::scriptOperandDword(uint32 encodedOperand) {
	const uint byteOffset = (encodedOperand & 0x1ffe) * 2;
	if ((encodedOperand & 0x2000) == 0)
		return stateDword(byteOffset);

	return stateDword(kScriptSecondaryStateByteOffset + byteOffset);
}

uint32 &BCBEngine::stateDword(uint byteOffset) {
	if ((byteOffset & 3) || byteOffset + 4 > sizeof(_gameState)) {
		warning("BCBEngine::stateDword(): Invalid state offset 0x%04x", byteOffset);
		_scriptStopFlag = true;
		return _gameState[0];
	}

	return _gameState[byteOffset / 4];
}

uint32 BCBEngine::stateDword(uint byteOffset) const {
	if ((byteOffset & 3) || byteOffset + 4 > sizeof(_gameState))
		return 0;

	return _gameState[byteOffset / 4];
}

} // End of namespace BCB
