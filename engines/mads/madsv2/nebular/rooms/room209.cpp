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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _dodgeFl;
	bool _forceDodgeFl;
	bool _pitchFl;
	bool _fallFl;
	bool _forceFallFl;
	bool _playingAnimFl;
	bool _shouldFallFl;
	bool _shouldDodgeFl;
	int32 _monkeyPosition;
	int32 _counter;
	int32 _pauseMode;
	bool _binocularsDroppedFl;
	bool _startShootingInTimerFl;
	int32 _dialogAbortVal;
	bool _playingDialogFl;
	bool _shootMissedLastFl;
	bool _removeMonkeyFl;
	bool _shootReadyFl;
	int32 _pauseCounterThreshold;
	int32 _pauseCounter;
};

static Scratch local;


static void handlePause() {
	switch (_game._trigger) {
	case 124:
		if (++local._pauseCounter <= local._pauseCounterThreshold)
			_scene->_sequences.addTimer(60, 124);
		else
			local._pauseMode = 0;
		break;
	default:
		break;
	}
}

static void initPauseCounterThreshold() {
	switch (_game._trigger) {
	case 226:
		_scene->_sequences.addTimer(1, 124);
		local._pauseCounterThreshold = _vm->getRandomNumber(7, 12);
		local._pauseMode = 2;
		local._pauseCounter = 0;
		break;
	default:
		break;
	}
}

static void handlePeek() {
	switch (_game._trigger) {
	case 133:
		_vm->_sound->command(18);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 51, 52);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 134);
		break;

	case 134:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(60, 135);
	}
	break;

	case 135:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
		_scene->_sequences.addTimer(10, 136);
		break;

	case 136:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.addTimer(50, 137);
		break;

	case 137:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 51, 52);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 138);
		break;

	case 138:
		local._pauseMode = 1;
		_scene->_hotspots.activate(227, false);
		local._playingAnimFl = false;
		break;

	default:
		break;
	}
}

static void handleVerticalMove() {
	switch (_game._trigger) {
	case 140:
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 8, 0, 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 141);
		break;

	case 141:
	{
		_vm->_sound->command(18);
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 5);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(60, 142);
	}
	break;

	case 142:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 143);
		break;

	case 143:
		local._pauseMode = 1;
		local._playingAnimFl = false;
		_scene->_hotspots.activate(227, false);
		break;

	default:
		break;
	}
}

static void handleLookStay() {
	switch (_game._trigger) {
	case 145:
		_vm->_sound->command(18);
		local._monkeyPosition = 2;
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 51, 52);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 146);
		break;

	case 146:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(15, 147);
	}
	break;

	case 147:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
		_scene->_sequences.addTimer(8, 148);
		break;

	case 148:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);

		if (!local._dodgeFl) {
			_scene->_sequences.addTimer(90, 149);
		} else {
			_scene->_sequences.addTimer(1, 149);
			local._shouldDodgeFl = true;
		}
		break;

	case 149:
		local._playingAnimFl = false;
		break;

	default:
		break;
	}
}

static void handleLookRight() {
	switch (_game._trigger) {
	case 151:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 8, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 8, 14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 152);
		break;

	case 152:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(60, 153);
	}
	break;

	case 153:
		local._playingAnimFl = false;
		if (local._dodgeFl)
			local._shouldDodgeFl = true;
		break;

	default:
		break;
	}
}

static void handleBlink() {
	switch (_game._trigger) {
	case 155:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.addTimer(50, 156);
		break;

	case 156:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
		_scene->_sequences.addTimer(10, 157);
		break;

	case 157:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.addTimer(50, 158);
		break;

	case 158:
		local._playingAnimFl = false;
		if (local._dodgeFl)
			local._shouldDodgeFl = true;
		break;

	default:
		break;
	}
}

static void handleGetBinoculars() {
	switch (_game._trigger) {
	case 161:
		_vm->_sound->command(18);
		local._monkeyPosition = 3;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 8, 24);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 20, 165);
		if (!local._fallFl && !local._dodgeFl) {
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 162);
		} else {
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 163);
		}
		break;

	case 162:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 8, 6, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 23, 25);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 163);
	}
	break;

	case 163:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 8, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 23, 24);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(8, 164);
	}
	break;

	case 164:
		local._playingAnimFl = false;
		if (local._fallFl)
			local._shouldFallFl = true;
		break;

	case 165:
		_vm->_sound->command(18);
		break;

	default:
		break;
	}
}

static void handleBinocularBlink() {
	switch (_game._trigger) {
	case 167:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 8, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 23, 25);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 168);
	}
	break;

	case 168:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 8, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 23, 24);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(30, 169);
	}
	break;

	case 169:
		local._playingAnimFl = false;
		if (local._fallFl)
			local._shouldFallFl = true;
		break;

	default:
		break;
	}
}

static void handleBinocularScan() {
	switch (_game._trigger) {
	case 171:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 43, 45);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 172);
	}
	break;

	case 172:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		int randAction = _vm->getRandomNumber(1, 2);
		switch (randAction) {
		case 1:
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 2, 0, 0);
			break;
		case 2:
			_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 4, 0, 0);
			break;
		default:
			break;
		}
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 23, 25);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 173);
	}
	break;

	case 173:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 26, 30);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 174);
	}
	break;

	case 174:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 12, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 23, 24);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addTimer(60, 175);
	}
	break;

	case 175:
		local._playingAnimFl = false;
		if (local._fallFl)
			local._shouldFallFl = true;
		break;

	default:
		break;
	}
}

static void handleJumpInTree() {
	switch (_game._trigger) {
	case 178:
	{
		int oldIdx = 0;
		local._monkeyPosition = 1;
		if (local._removeMonkeyFl)
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		else
			oldIdx = _globals._sequenceIndexes[3];

		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 46, 49);
		if (!local._removeMonkeyFl)
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);

		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 179);
	}
	break;

	case 179:
	{
		_vm->_sound->command(18);
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 53, 61);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 180);
	}
	break;

	case 180:
		local._removeMonkeyFl = true;
		local._pauseMode = 1;
		local._playingAnimFl = false;
		_scene->_hotspots.activate(227, false);
		break;

	default:
		break;
	}
}

static void handleTongue() {
	switch (_game._trigger) {
	case 182:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 26, 30);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 183);
	}
	break;

	case 183:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 31, 33);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 184);
	}
	break;

	case 184:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 36, 37);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 185);
	}
	break;

	case 185:
	{
		_vm->_sound->command(18);
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], false, 6, 20, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 38, 39);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 186);
	}
	break;

	case 186:
	{
		int oldIdx = _globals._sequenceIndexes[3];
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 12, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 40, 41);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 178);
		local._removeMonkeyFl = false;
	}
	break;

	default:
		break;
	}
}

static void handleStandFromPeek() {
	switch (_game._trigger) {
	case 189:
		local._monkeyPosition = 4;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 50);
		_scene->_sequences.addTimer(8, 190);
		break;

	case 190:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		_scene->_sequences.addTimer(8, 191);
		break;

	case 191:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[6] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[6], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[6], 1, 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[6], SEQUENCE_TRIGGER_EXPIRE, 0, 192);
		break;

	case 192:
	{
		_vm->_sound->command(18);
		int oldIdx = _globals._sequenceIndexes[6];
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], oldIdx);
		_scene->_sequences.addTimer(8, 193);
	}
	break;

	case 193:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 22);
		_scene->_sequences.addTimer(5, 194);
		break;

	case 194:
		local._playingAnimFl = false;
		local._counter = 0;
		break;

	default:
		break;
	}
}

static void handleStandBlink() {
	switch (_game._trigger) {
	case 246:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 22);
		_scene->_sequences.addTimer(10, 247);
		break;

	case 247:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 23);
		_scene->_sequences.addTimer(8, 248);
		break;

	case 248:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 22);
		_scene->_sequences.addTimer(10, 249);
		break;

	case 249:
		local._playingAnimFl = false;
		break;

	default:
		break;
	}
}

static void handleJumpAndHide() {
	switch (_game._trigger) {
	case 196:
		_vm->_sound->command(18);
		local._monkeyPosition = 1;
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 16);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 197);
		break;

	case 197:
		local._pauseMode = 1;
		_scene->_hotspots.activate(227, false);
		local._playingAnimFl = false;
		break;

	default:
		break;
	}
}

static void handleMonkeyEating() {
	switch (_game._trigger) {
	case 199:
		_vm->_sound->command(18);
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 200);
		break;

	case 200:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 10, 10, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 15, 16);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 201);
	}
	break;

	case 201:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 12);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addTimer(20, 202);
	}
	break;

	case 202:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 19);
		_scene->_sequences.addTimer(20, 203);
		break;

	case 203:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 17);
		_scene->_sequences.addTimer(20, 204);
		break;

	case 204:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 10, 8, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 18, 19);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 205);
		break;

	case 205:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 10, 8, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 20, 21);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 206);
	}
	break;

	case 206:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 22, 25);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		if (!local._dodgeFl && !local._fallFl)
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 207);
		else
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 209);
	}
	break;

	case 207:
	{
		_vm->_sound->command(18);
		int msgIndex = _scene->_kernelMessages.add(Common::Point(180, 25), 0xFDFC, 0, 0, 90, _game.getQuote(130));
		_scene->_kernelMessages.setQuoted(msgIndex, 4, true);

		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 15, 4, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 26, 27);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 208);
	}
	break;

	case 208:
	{
		_scene->_kernelMessages.add(Common::Point(180, 39), 0xFDFC, 0, 0, 90, _game.getQuote(131));
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 10, 4, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 28, 29);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 209);
	}
	break;

	case 209:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 22);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addTimer(1, 210);
	}
	break;

	case 210:
		local._playingAnimFl = false;
		break;

	default:
		break;
	}
}

static void handleMonkeyFall() {
	switch (_game._trigger) {
	case 219:
	{
		_vm->_sound->command(25);
		_scene->_sprites.remove(_globals._spriteIndexes[7]);
		_scene->_sprites.remove(_globals._spriteIndexes[6]);
		_scene->_sprites.remove(_globals._spriteIndexes[5]);
		_scene->_sprites.remove(_globals._spriteIndexes[4]);

		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('m', 4));
		_scene->_kernelMessages.add(Common::Point(180, 26), 0xFDFC, 0, 0, 90, _game.getQuote(151));
		_scene->_sequences.addTimer(40, 100);
		_scene->_hotspots.activate(227, false);
		int oldIdx = _globals._sequenceIndexes[3];
		local._monkeyPosition = 1;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 1, 35);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 220);
	}
	break;

	case 220:
	{
		_vm->_sound->command(18);
		_scene->_kernelMessages.add(Common::Point(182, 109), 0xFDFC, 0, 0, 90, _game.getQuote(159));
		_scene->_hotspots.activate(227, false);
		int oldIdx = _globals._sequenceIndexes[3];
		local._monkeyPosition = 1;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 36, 42);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 221);
	}
	break;

	case 221:
	{
		_game._objects.setRoom(OBJ_BINOCULARS, 209);
		local._binocularsDroppedFl = true;
		int oldIdx = _globals._sequenceIndexes[8];
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], oldIdx);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(201, 131));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[8], oldIdx);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 43, 72);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 222);
		int idx = _scene->_dynamicHotspots.add(words_binoculars, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(200, 133), FACING_NORTH);
	}
	break;

	case 222:
	{
		_scene->_kernelMessages.add(Common::Point(182, 109), 0xFDFC, 0, 0, 70, _game.getQuote(160));
		int oldIdx = _globals._sequenceIndexes[8];
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 73, 78);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[8], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 223);
	}
	break;

	case 223:
		_scene->loadAnimation(Resources::formatName(209, 'e', -1, EXT_AA, ""), 224);
		_vm->_sound->command(38);
		break;

	case 224:
		local._playingAnimFl = false;
		local._fallFl = false;
		local._counter = 0;
		local._pauseMode = 0;
		_vm->_dialogs->show(20910);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void handleMonkey1() {
	switch (_game._trigger) {
	case 212:
		_scene->_sequences.remove(_globals._sequenceIndexes[4]);
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 13);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 213);
		break;

	case 213:
	{
		int oldIdx = _globals._sequenceIndexes[4];
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 1, 22);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[7], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 214);
	}
	break;

	case 214:
	{
		int oldIdx = _globals._sequenceIndexes[7];
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 23, 26);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[7], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 215);
		int msgIndex = _scene->_kernelMessages.add(Common::Point(170, 21), 0xFDFC, 0, 0, 90, _game.getQuote(156));
		_scene->_kernelMessages.setQuoted(msgIndex, 3, true);
	}
	break;

	case 215:
	{
		_vm->_sound->command(18);
		_scene->loadAnimation(Resources::formatName(209, 'a', -1, EXT_AA, ""), 251);
		int oldIdx = _globals._sequenceIndexes[7];
		_globals._sequenceIndexes[7] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[7], false, 7, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[7], 27, 35);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[7], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[7], SEQUENCE_TRIGGER_EXPIRE, 0, 216);
	}
	break;

	case 216:
	{
		int oldIdx = _globals._sequenceIndexes[7];
		_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, 22);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], oldIdx);
		_scene->_sequences.addTimer(25, 217);
	}
	break;

	case 217:
		local._pitchFl = false;
		local._counter = 0;
		local._pauseMode = 0;
		_scene->_sequences.addTimer(1, 196);
		break;

	default:
		break;
	}
}

static void handleMonkey2() {
	switch (_game._trigger) {
	case 251:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, _game.getQuote(137));
		_vm->_sound->command(22);
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 11, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[12], Common::Point(111, 133));
		_scene->_sequences.setScale(_globals._sequenceIndexes[12], 79);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[12], 1, 6);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 252);
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = false;
		break;

	case 252:
	{
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(132));
		int oldIdx = _globals._sequenceIndexes[12];
		_globals._sequenceIndexes[12] = _scene->_sequences.startCycle(_globals._spriteIndexes[12], false, 7);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[12], Common::Point(111, 133));
		_scene->_sequences.setScale(_globals._sequenceIndexes[12], 79);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[12], oldIdx);
		_scene->_sequences.addTimer(120, 253);
	}
	break;

	case 253:
		_scene->_sequences.remove(_globals._sequenceIndexes[12]);
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void handleDodge() {
	switch (_game._trigger) {
	case 241:
		_scene->_hotspots.activate(227, true);
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 7);
		_scene->_sequences.addTimer(6, 242);
		break;

	case 242:
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 5);
		_scene->_sequences.addTimer(25, 243);
		_vm->_sound->command(24);
		break;

	case 243:
		_vm->_sound->command(18);
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		local._playingAnimFl = false;
		local._pauseMode = 0;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 90, _game.getQuote(155));
		if (!local._shootMissedLastFl) {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(135));
			local._shootMissedLastFl = true;
		} else {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(136));
		}
		break;

	default:
		break;
	}
}

static void room_209_init() {
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('a', 1));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('e', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('a', 0));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('m', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('m', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('m', 3));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('m', 6));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('m', 8));

	_game.loadQuoteSet(0x82, 0x83, 0x84, 0x9C, 0x97, 0x95, 0x99, 0x9E, 0x98, 0x9B, 0xA0, 0x96, 0x9F,
		0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x91, 0x92, 0x93, 0x94, 0x89, 0x85, 0x8A, 0x86, 0x87, 0x88, 0);

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);

	if (_game._objects.isInRoom(OBJ_PLANT_STALK)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_plant_stalk, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(263, 129), FACING_SOUTH);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 13);
	}

	if (_scene->_priorSceneId == 208) {
		_game._player._playerPos = Common::Point(11, 121);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(28, 121);
		_game._player._facing = FACING_SOUTH;
	}

	if (_game._objects.isInRoom(OBJ_BINOCULARS)) {
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(201, 131));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		int idx = _scene->_dynamicHotspots.add(words_binoculars, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(200, 133), FACING_NORTH);
	}

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_POISON_DARTS);
		_game._objects.addToInventory(OBJ_BLOWGUN);
		_globals[kMonkeyStatus] = MONKEY_HAS_BINOCULARS;
	}

	local._pitchFl = false;
	local._fallFl = false;
	local._dodgeFl = false;
	local._playingAnimFl = false;
	local._monkeyPosition = 1;
	local._counter = 0;
	local._pauseMode = 0;
	local._forceFallFl = false;
	local._shouldFallFl = false;
	local._forceDodgeFl = false;
	local._binocularsDroppedFl = false;
	local._shouldDodgeFl = false;
	local._startShootingInTimerFl = false;
	local._dialogAbortVal = 5;
	local._playingDialogFl = false;
	local._shootMissedLastFl = false;
	local._removeMonkeyFl = true;
	local._shootReadyFl = false;

	_scene->_hotspots.activate(227, false);

	section_2_music();
}

static void room_209_daemon() {
	if (!local._playingAnimFl && !local._pitchFl && !local._fallFl && !local._dodgeFl && (local._pauseMode == 0) && (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS)) {
		int randAction = _vm->getRandomNumber(1, 50);
		switch (randAction) {
		case 1:
			if ((local._monkeyPosition == 1) && (local._counter < 2)) {
				_scene->_sequences.addTimer(1, 133);
				local._playingAnimFl = true;
				_scene->_hotspots.activate(227, true);
				++local._counter;
			}
			break;

		case 2:
			if ((local._monkeyPosition == 1) && (local._counter < 2)) {
				_scene->_sequences.addTimer(1, 140);
				_scene->_hotspots.activate(227, true);
				local._playingAnimFl = true;
				++local._counter;
			}
			break;

		case 3:
			if (local._monkeyPosition == 1) {
				_scene->_sequences.addTimer(1, 145);
				_scene->_hotspots.activate(227, true);
				local._playingAnimFl = true;
				local._counter = 0;
			}
			break;

		case 4:
			if ((local._monkeyPosition == 2) && (local._counter < 2)) {
				_scene->_sequences.addTimer(1, 151);
				_scene->_hotspots.activate(227, true);
				++local._counter;
				local._playingAnimFl = true;
			}
			break;

		case 5:
			if (local._monkeyPosition == 2) {
				_scene->_sequences.addTimer(1, 161);
				_scene->_hotspots.activate(227, true);
				local._counter = 0;
				local._playingAnimFl = true;
			}
			break;

		case 6:
			if (local._monkeyPosition == 2) {
				_scene->_sequences.addTimer(1, 189);
				_scene->_hotspots.activate(227, true);
				local._counter = 0;
				local._playingAnimFl = true;
			}
			break;
		case 7:
			if (local._monkeyPosition == 3) {
				_scene->_hotspots.activate(227, true);
				_scene->_sequences.addTimer(1, 167);
				local._playingAnimFl = true;
			}
			break;

		case 8:
			if (local._monkeyPosition == 3) {
				_scene->_sequences.addTimer(1, 178);
				local._playingAnimFl = true;
				_scene->_hotspots.activate(227, true);
				local._counter = 0;
			}
			break;

		case 9:
			if ((local._monkeyPosition == 3) && (_game._player._playerPos.x < 120)) {
				_scene->_sequences.addTimer(1, 182);
				_scene->_hotspots.activate(227, true);
				local._counter = 0;
				local._playingAnimFl = true;
			}
			break;

		case 10:
			if (local._monkeyPosition == 4) {
				_scene->_sequences.addTimer(1, 196);
				_scene->_hotspots.activate(227, true);
				local._playingAnimFl = true;
				local._counter = 0;
			}
			break;

		case 11:
			if ((local._monkeyPosition == 4) && (local._counter < 3)) {
				_scene->_sequences.addTimer(1, 199);
				_scene->_hotspots.activate(227, true);
				++local._counter;
				local._playingAnimFl = true;
			}
			break;

		case 30:
			if (local._monkeyPosition == 4) {
				_scene->_sequences.addTimer(1, 246);
				_scene->_hotspots.activate(227, true);
				local._counter = 0;
				local._playingAnimFl = true;
			}
			break;

		default:
			if ((randAction >= 12) && (randAction <= 20) && (local._monkeyPosition == 2) && (local._counter < 5)) {
				_scene->_sequences.addTimer(1, 155);
				++local._counter;
				local._playingAnimFl = true;
			}

			if ((randAction >= 21) && (randAction <= 29) && (local._monkeyPosition == 3) && (local._counter < 3)) {
				_scene->_sequences.addTimer(1, 171);
				local._playingAnimFl = true;
				++local._counter;
			}
			break;
		}
	}

	if (!local._dodgeFl && !local._pitchFl && !local._fallFl && (local._pauseMode == 1))
		_scene->_sequences.addTimer(1, 226);

	if (!local._dodgeFl && !local._pitchFl && !local._fallFl && (local._pauseMode == 2))
		handlePause();

	if (!local._dodgeFl && !local._pitchFl && !local._fallFl && (local._pauseMode == 1))
		initPauseCounterThreshold();

	handlePeek();
	handleVerticalMove();
	handleLookStay();
	handleLookRight();
	handleBlink();
	handleGetBinoculars();
	handleStandFromPeek();
	handleDodge();
	handleBinocularBlink();
	handleBinocularScan();
	handleJumpInTree();
	handleTongue();
	handleMonkeyFall();
	handleJumpAndHide();
	handleMonkeyEating();
	handleMonkey1();
	handleStandBlink();
	handleMonkey2();

	if ((local._monkeyPosition == 1) && !local._playingAnimFl && local._fallFl) {
		_scene->_sequences.addTimer(1, 145);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 2) && !local._playingAnimFl && local._fallFl) {
		_scene->_sequences.addTimer(1, 161);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 4) && !local._playingAnimFl && local._fallFl) {
		_scene->_sequences.addTimer(1, 196);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 3) && !local._playingAnimFl && local._fallFl && local._forceFallFl) {
		_scene->_sequences.addTimer(1, 219);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 1) && !local._playingAnimFl && local._pitchFl) {
		_scene->_sequences.addTimer(1, 145);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 2) && !local._playingAnimFl && local._pitchFl) {
		_scene->_sequences.addTimer(1, 189);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 4) && !local._playingAnimFl && local._pitchFl) {
		_scene->_sequences.addTimer(1, 212);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 3) && !local._playingAnimFl && local._pitchFl) {
		_scene->_sequences.addTimer(1, 178);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 1) && !local._playingAnimFl && local._dodgeFl) {
		_scene->_sequences.addTimer(1, 145);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 4) && !local._playingAnimFl && local._dodgeFl) {
		_scene->_sequences.addTimer(1, 196);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 3) && !local._playingAnimFl && local._dodgeFl) {
		_scene->_sequences.addTimer(1, 178);
		local._playingAnimFl = true;
	}

	if ((local._monkeyPosition == 2) && !local._playingAnimFl && local._dodgeFl && local._forceDodgeFl) {
		_scene->_sequences.addTimer(1, 241);
		local._playingAnimFl = true;
	}

	if (local._dodgeFl || local._fallFl) { /* if want to dodge or fall */
		if (!local._playingAnimFl && (local._monkeyPosition == 2))
			local._shouldDodgeFl = true;

		if (!local._playingAnimFl && (local._monkeyPosition == 3))
			local._shouldFallFl = true;

		switch (_game._trigger) {
		case 228:
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 229);
			break;

		case 229:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(35, 230);
		}
		break;

		case 230:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 7);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 231);
			break;

		case 231:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 8, 10);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 232);
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], -1);
			_game._player._visible = false;
		}
		break;

		case 232:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 10);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(2, 233);
			_scene->_kernelMessages.reset();
			if (local._dodgeFl && (local._monkeyPosition != 1) && (local._monkeyPosition != 2))
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 34463, _game.getQuote(138));
			if (local._fallFl && (local._monkeyPosition != 3))
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 34463, _game.getQuote(138));
		}
		break;

		case 233:
			local._shootReadyFl = true;
			break;

		case 234:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_scene->_kernelMessages.reset();
			if (player_said_3(hose_down, blowgun, monkey)) {
				_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 7, 16, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 11, 12);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 12, 239);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 235);
			} else if (player_said_3(shoot, blowgun, monkey)) {
				_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 11, 12);
				_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 235);
				_vm->_sound->command(23);
			}
			break;

		case 235:
		{
			int oldIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 13);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], oldIdx);
			_scene->_sequences.addTimer(12, 236);
			local._forceFallFl = true;
			local._forceDodgeFl = true;
		}
		break;

		case 236:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 14, 16);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(116, 131));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 237);
			break;

		case 237:
			_game._player._visible = true;
			_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
			_scene->_sequences.addTimer(1, 238);
			break;

		case 238:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			if (local._dodgeFl)
				_game._player._stepEnabled = true;

			local._startShootingInTimerFl = false;

			if (local._fallFl) {
				_globals[kMonkeyStatus] = MONKEY_IS_GONE;
				_game._objects.setRoom(OBJ_POISON_DARTS, NOWHERE);
			}
			local._dodgeFl = false;
			local._fallFl = false;
			local._forceFallFl = false;
			local._forceDodgeFl = false;
			local._shouldFallFl = false;
			local._shouldDodgeFl = false;
			break;

		case 239:
			_vm->_sound->command(23);
			break;

		default:
			break;
		}
	}

	if (_game._trigger == 100)
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(134));

	if (local._shootReadyFl && (local._shouldFallFl || local._shouldDodgeFl)) {
		_scene->_sequences.addTimer(4, 234);
		local._shootReadyFl = false;
	}
}

static void room_209_pre_parser() {
	if (player_said_2(walk_towards, field_to_west))
		_game._player._walkOffScreenSceneId = 208;

	if (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS) {
		if ((player_said_1(shoot) || player_said_1(hose_down)) && player_said_1(monkey)
			&& player_said_1(blowgun) && _game._objects.isInInventory(OBJ_BLOWGUN) && _game._objects.isInInventory(OBJ_POISON_DARTS)) {
			_game._player._prepareWalkPos = Common::Point(111, 129);
			_game._player._prepareWalkFacing = FACING_NORTHEAST;
			_game._player._needToWalk = true;
			_game._player._readyToWalk = true;
		}

		if (player_said_2(look, monkey) || player_said_2(talkto, monkey)) {
			_game._player._prepareWalkPos = Common::Point(111, 129);
			_game._player._prepareWalkFacing = FACING_NORTHEAST;
			_game._player._needToWalk = true;
			_game._player._readyToWalk = true;
		}
	}
}

static void room_209_parser() {
	if (_action._lookFlag) {
		_vm->_dialogs->show(20912);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(walk_towards, rocky_area_to_north)) {
		_scene->_nextSceneId = 203;
		_action._inProgress = false;
		return;
	}

	if (player_said_2(talkto, monkey) && !local._pitchFl && !local._playingDialogFl) {
		_scene->_sequences.addTimer(1, local._dialogAbortVal);
		local._playingDialogFl = true;
		_game._player._stepEnabled = false;
		_action._inProgress = false;
		return;
	}

	switch (_game._trigger) {
	case 130:
		_game._player._stepEnabled = true;
		local._playingDialogFl = false;
		_action._inProgress = false;
		return;

	case 5:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 6, 180, _game.getQuote(139));
		_action._inProgress = false;
		return;

	case 6:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, _game.getQuote(151));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 7;
		_action._inProgress = false;
		return;

	case 7:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 8, 180, _game.getQuote(140));
		_action._inProgress = false;
		return;

	case 8:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, _game.getQuote(149));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 9;
		_action._inProgress = false;
		return;

	case 9:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, _game.getQuote(141));
		_scene->_sequences.addTimer(200, 10);
		_action._inProgress = false;
		return;

	case 10:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 11, 180, _game.getQuote(142));
		_action._inProgress = false;
		return;

	case 11:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, _game.getQuote(152));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 12;
		_action._inProgress = false;
		return;

	case 12:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 130, _game.getQuote(143));
		_scene->_sequences.addTimer(150, 13);
		_action._inProgress = false;
		return;

	case 13:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 14, 180, _game.getQuote(145));
		_action._inProgress = false;
		return;

	case 14:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, _game.getQuote(151));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 15;
		_action._inProgress = false;
		return;

	case 15:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 16, 180, _game.getQuote(146));
		_action._inProgress = false;
		return;

	case 16:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 17, 60, _game.getQuote(154));
		_action._inProgress = false;
		return;

	case 17:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 130, 60, _game.getQuote(147));
		local._dialogAbortVal = 18;
		_action._inProgress = false;
		return;

	case 18:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, _game.getQuote(148));
		local._pitchFl = true;
		local._playingDialogFl = false;
		local._dialogAbortVal = 5;
		_action._inProgress = false;
		return;

	default:
		break;
	}

	if (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS) {
		if ((player_said_1(shoot) || player_said_1(hose_down)) && player_said_1(monkey)
			&& player_said_1(blowgun) && _game._objects.isInInventory(OBJ_BLOWGUN) && _game._objects.isInInventory(OBJ_POISON_DARTS)) {
			if (player_said_3(shoot, blowgun, monkey) && !local._startShootingInTimerFl) {
				_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(1, 231);
				local._startShootingInTimerFl = true;
				_game._player._stepEnabled = false;
				local._dodgeFl = true;
				_action._inProgress = false;
				return;
			}

			if (player_said_3(hose_down, blowgun, monkey) && !local._startShootingInTimerFl) {
				_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(1, 228);
				_game._player._stepEnabled = false;
				local._fallFl = true;
				local._startShootingInTimerFl = true;
				_action._inProgress = false;
				return;
			}
		}

		if (player_said_2(look, monkey)) {
			local._pitchFl = true;
			_game._player._stepEnabled = false;
			_vm->_dialogs->show(20914);
			_action._inProgress = false;
			return;
		}
	}

	if (player_said_2(take, plant_stalk) && (_game._trigger || _game._objects.isInRoom(OBJ_PLANT_STALK))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 3, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[11]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_game._objects.addToInventory(OBJ_PLANT_STALK);
			break;

		case 2:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_scene->_sequences.addTimer(4, 3);
			_vm->_dialogs->showItem(OBJ_PLANT_STALK, 0x328);
			break;

		case 3:
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, binoculars) && (_game._trigger || _game._objects.isInRoom(OBJ_BINOCULARS))) {
		switch (_game._trigger) {
		case 0:
			_globals._spriteIndexes[10] = _scene->_sprites.addSprites("*RXMBD_8");
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 3, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[10]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			_game._objects.addToInventory(OBJ_BINOCULARS);
			break;

		case 2:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			local._binocularsDroppedFl = false;
			_scene->_sequences.addTimer(4, 3);
			break;

		case 3:
			_vm->_dialogs->showItem(OBJ_BINOCULARS, 0x51AF);
			_scene->_sprites.remove(_globals._spriteIndexes[10]);
			break;

		default:
			break;
		}
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, sky)) {
		_vm->_dialogs->show(20901);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, bamboo_like_plant)) {
		_vm->_dialogs->show(20902);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, mountainside)) {
		_vm->_dialogs->show(20903);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, grassy_field)) {
		_vm->_dialogs->show(20904);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, field_to_west)) {
		_vm->_dialogs->show(20905);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, rocky_area_to_north)) {
		_vm->_dialogs->show(20906);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, plant_stalk) && (_action._savedFields._mainObjectSource == 4)) {
		_vm->_dialogs->show(20907);
		_action._inProgress = false;
		return;
	}

	if (player_said_3(give, twinkifruit, monkey) || player_said_3(throw, twinkifruit, monkey)) {
		_vm->_dialogs->show(20909);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, trees)) {
		_vm->_dialogs->show(20913);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(throw, monkey) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId))) {
		if (!player_said_1(poison_darts)) {
			_vm->_dialogs->show(20915);
		}
		_action._inProgress = false;
		return;
	}

	if (player_said_3(throw, poison_darts, monkey)) {
		_vm->_dialogs->show(20916);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, palm_tree)) {
		if (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS) {
			if (local._monkeyPosition == 1)
				_vm->_dialogs->show(20917);
			else
				_vm->_dialogs->show(20918);
		} else {
			if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY)
				_vm->_dialogs->show(20917);
			else
				_vm->_dialogs->show(20919);
		}
		_action._inProgress = false;
		return;
	}

	if (player_said_2(look, melon_mush)) {
		_vm->_dialogs->show(20920);
		_action._inProgress = false;
		return;
	}

	if (player_said_2(take, melon_mush)) {
		_vm->_dialogs->show(20921);
		_action._inProgress = false;
		return;
	}
}

void room_209_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._dodgeFl);
	s.syncAsByte(local._forceDodgeFl);
	s.syncAsByte(local._shouldDodgeFl);
	s.syncAsByte(local._pitchFl);
	s.syncAsByte(local._fallFl);
	s.syncAsByte(local._forceFallFl);
	s.syncAsByte(local._shouldFallFl);
	s.syncAsByte(local._playingAnimFl);
	s.syncAsByte(local._playingDialogFl);

	s.syncAsSint32LE(local._pauseMode);
	s.syncAsSint32LE(local._pauseCounterThreshold);
	s.syncAsSint32LE(local._pauseCounter);

	s.syncAsByte(local._removeMonkeyFl);

	s.syncAsSint32LE(local._monkeyPosition);

	s.syncAsByte(local._shootReadyFl);
	s.syncAsByte(local._startShootingInTimerFl);
	s.syncAsByte(local._shootMissedLastFl);
	s.syncAsByte(local._binocularsDroppedFl);

	s.syncAsSint32LE(local._dialogAbortVal);
	s.syncAsSint32LE(local._counter);
}

void room_209_preload() {
	room_init_code_pointer = room_209_init;
	room_pre_parser_code_pointer = room_209_pre_parser;
	room_parser_code_pointer = room_209_parser;
	room_daemon_code_pointer = room_209_daemon;

	section_2_walker();
	section_2_interface();
	_scene->addActiveVocab(words_plant_stalk);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
