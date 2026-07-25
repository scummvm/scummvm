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

#include "mads/core/game.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
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
	switch (kernel.trigger) {
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
	switch (kernel.trigger) {
	case 226:
		_scene->_sequences.addTimer(1, 124);
		local._pauseCounterThreshold = g_engine->getRandomNumber(7, 12);
		local._pauseMode = 2;
		local._pauseCounter = 0;
		break;
	default:
		break;
	}
}

static void handlePeek() {
	switch (kernel.trigger) {
	case 133:
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
	case 140:
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 8, 8, 0, 1);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 5);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 141);
		break;

	case 141:
	{
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
	case 145:
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
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
	switch (kernel.trigger) {
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
	switch (kernel.trigger) {
	case 161:
		g_engine->_soundManager->command(18, 0);
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
		g_engine->_soundManager->command(18, 0);
		break;

	default:
		break;
	}
}

static void handleBinocularBlink() {
	switch (kernel.trigger) {
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
	switch (kernel.trigger) {
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
		int randAction = g_engine->getRandomNumber(1, 2);
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
	switch (kernel.trigger) {
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
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
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
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
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
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
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
	switch (kernel.trigger) {
	case 196:
		g_engine->_soundManager->command(18, 0);
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
	switch (kernel.trigger) {
	case 199:
		g_engine->_soundManager->command(18, 0);
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
		g_engine->_soundManager->command(18, 0);
		int msgIndex = _scene->_kernelMessages.add(Common::Point(180, 25), 0xFDFC, 0, 0, 90, quote_string(kernel.quotes, 130));
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
		_scene->_kernelMessages.add(Common::Point(180, 39), 0xFDFC, 0, 0, 90, quote_string(kernel.quotes, 131));
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
	switch (kernel.trigger) {
	case 219:
	{
		g_engine->_soundManager->command(25, 0);
		_scene->_sprites.remove(_globals._spriteIndexes[7]);
		_scene->_sprites.remove(_globals._spriteIndexes[6]);
		_scene->_sprites.remove(_globals._spriteIndexes[5]);
		_scene->_sprites.remove(_globals._spriteIndexes[4]);

		_globals._spriteIndexes[8] = _scene->_sprites.addSprites(kernel_name('m', 4));
		_scene->_kernelMessages.add(Common::Point(180, 26), 0xFDFC, 0, 0, 90, quote_string(kernel.quotes, 151));
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
		g_engine->_soundManager->command(18, 0);
		_scene->_kernelMessages.add(Common::Point(182, 109), 0xFDFC, 0, 0, 90, quote_string(kernel.quotes, 159));
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
		inter_move_object(OBJ_BINOCULARS, 209);
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
		_scene->_kernelMessages.add(Common::Point(182, 109), 0xFDFC, 0, 0, 70, quote_string(kernel.quotes, 160));
		int oldIdx = _globals._sequenceIndexes[8];
		_globals._sequenceIndexes[8] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[8], false, 8, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[8], 73, 78);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[8], oldIdx);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[8], SEQUENCE_TRIGGER_EXPIRE, 0, 223);
	}
	break;

	case 223:
		_scene->loadAnimation(kernel_full_name(209, 'e', -1, "", EXT_AA), 224);
		g_engine->_soundManager->command(38, 0);
		break;

	case 224:
		local._playingAnimFl = false;
		local._fallFl = false;
		local._counter = 0;
		local._pauseMode = 0;
		text_show(20910);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void handleMonkey1() {
	switch (kernel.trigger) {
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
		int msgIndex = _scene->_kernelMessages.add(Common::Point(170, 21), 0xFDFC, 0, 0, 90, quote_string(kernel.quotes, 156));
		_scene->_kernelMessages.setQuoted(msgIndex, 3, true);
	}
	break;

	case 215:
	{
		g_engine->_soundManager->command(18, 0);
		_scene->loadAnimation(kernel_full_name(209, 'a', -1, "", EXT_AA), 251);
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
	switch (kernel.trigger) {
	case 251:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, quote_string(kernel.quotes, 137));
		g_engine->_soundManager->command(22, 0);
		_globals._sequenceIndexes[12] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[12], false, 11, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[12], Common::Point(111, 133));
		_scene->_sequences.setScale(_globals._sequenceIndexes[12], 79);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[12], 1, 6);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 252);
		player.clock = _scene->_frameStartTime - player.frame_delay;
		player.walker_visible = false;
		break;

	case 252:
	{
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 132));
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
		player.clock = _scene->_frameStartTime - player.frame_delay;
		player.walker_visible = true;
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void handleDodge() {
	switch (kernel.trigger) {
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
		g_engine->_soundManager->command(24, 0);
		break;

	case 243:
		g_engine->_soundManager->command(18, 0);
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 6);
		local._playingAnimFl = false;
		local._pauseMode = 0;
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 90, quote_string(kernel.quotes, 155));
		if (!local._shootMissedLastFl) {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 135));
			local._shootMissedLastFl = true;
		} else {
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 136));
		}
		break;

	default:
		break;
	}
}

static void room_209_init() {
	_globals._spriteIndexes[11] = _scene->_sprites.addSprites("*RXMBD_2");
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites(kernel_name('a', 1));
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('e', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('a', 0));
	_globals._spriteIndexes[9] = _scene->_sprites.addSprites(kernel_name('b', 1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('m', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('m', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(kernel_name('m', 3));
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(kernel_name('m', 6));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(kernel_name('m', 8));

	kernel.quotes = quote_load(0x82, 0x83, 0x84, 0x9C, 0x97, 0x95, 0x99, 0x9E, 0x98, 0x9B, 0xA0, 0x96, 0x9F,
		0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x91, 0x92, 0x93, 0x94, 0x89, 0x85, 0x8A, 0x86, 0x87, 0x88, 0);

	pal_change_color(252, 63, 44, 30);
	pal_change_color(253, 63, 20, 22);

	if (object_is_here(OBJ_PLANT_STALK)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_plant_stalk, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(263, 129), FACING_SOUTH);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 13);
	}

	if (_scene->_priorSceneId == 208) {
		player.x = 11;
		player.y = 121;
		player.facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 28;
		player.y = 121;
		player.facing = FACING_SOUTH;
	}

	if (object_is_here(OBJ_BINOCULARS)) {
		_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[9], Common::Point(201, 131));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 8);
		int idx = _scene->_dynamicHotspots.add(words_binoculars, words_walkto, _globals._sequenceIndexes[9], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(200, 133), FACING_NORTH);
	}

	if (_scene->_roomChanged) {
		inter_give_to_player(OBJ_POISON_DARTS);
		inter_give_to_player(OBJ_BLOWGUN);
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
		int randAction = g_engine->getRandomNumber(1, 50);
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
			if ((local._monkeyPosition == 3) && (player.x < 120)) {
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

		switch (kernel.trigger) {
		case 228:
			player.walker_visible = false;
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
			player.clock = _scene->_frameStartTime - player.frame_delay;
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], -1);
			player.walker_visible = false;
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
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 34463, quote_string(kernel.quotes, 138));
			if (local._fallFl && (local._monkeyPosition != 3))
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 34463, quote_string(kernel.quotes, 138));
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
				g_engine->_soundManager->command(23, 0);
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
			player.walker_visible = true;
			player.clock = _scene->_frameStartTime - player.frame_delay;
			_scene->_sequences.addTimer(1, 238);
			break;

		case 238:
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			if (local._dodgeFl)
				player.commands_allowed = true;

			local._startShootingInTimerFl = false;

			if (local._fallFl) {
				_globals[kMonkeyStatus] = MONKEY_IS_GONE;
				inter_move_object(OBJ_POISON_DARTS, NOWHERE);
			}
			local._dodgeFl = false;
			local._fallFl = false;
			local._forceFallFl = false;
			local._forceDodgeFl = false;
			local._shouldFallFl = false;
			local._shouldDodgeFl = false;
			break;

		case 239:
			g_engine->_soundManager->command(23, 0);
			break;

		default:
			break;
		}
	}

	if (kernel.trigger == 100)
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 134));

	if (local._shootReadyFl && (local._shouldFallFl || local._shouldDodgeFl)) {
		_scene->_sequences.addTimer(4, 234);
		local._shootReadyFl = false;
	}
}

static void room_209_pre_parser() {
	if (player_said_2(walk_towards, field_to_west))
		player.walk_off_edge_to_room = 208;

	if (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS) {
		if ((player_said_1(shoot) || player_said_1(hose_down)) && player_said_1(monkey)
			&& player_said_1(blowgun) && player_has(OBJ_BLOWGUN) && player_has(OBJ_POISON_DARTS)) {
			player.prepare_walk_x = 111;
			player.prepare_walk_y = 129;
			player.prepare_walk_facing = FACING_NORTHEAST;
			player.need_to_walk = true;
			player.ready_to_walk = true;
		}

		if (player_said_2(look, monkey) || player_said_2(talkto, monkey)) {
			player.prepare_walk_x = 111;
			player.prepare_walk_y = 129;
			player.prepare_walk_facing = FACING_NORTHEAST;
			player.need_to_walk = true;
			player.ready_to_walk = true;
		}
	}
}

static void room_209_parser() {
	if (player.look_around) {
		text_show(20912);
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_towards, rocky_area_to_north)) {
		_scene->_nextSceneId = 203;
		player.command_ready = false;
		return;
	}

	if (player_said_2(talkto, monkey) && !local._pitchFl && !local._playingDialogFl) {
		_scene->_sequences.addTimer(1, local._dialogAbortVal);
		local._playingDialogFl = true;
		player.commands_allowed = false;
		player.command_ready = false;
		return;
	}

	switch (kernel.trigger) {
	case 130:
		player.commands_allowed = true;
		local._playingDialogFl = false;
		player.command_ready = false;
		return;

	case 5:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 6, 180, quote_string(kernel.quotes, 139));
		player.command_ready = false;
		return;

	case 6:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 151));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 7;
		player.command_ready = false;
		return;

	case 7:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 8, 180, quote_string(kernel.quotes, 140));
		player.command_ready = false;
		return;

	case 8:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 149));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 9;
		player.command_ready = false;
		return;

	case 9:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, quote_string(kernel.quotes, 141));
		_scene->_sequences.addTimer(200, 10);
		player.command_ready = false;
		return;

	case 10:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 11, 180, quote_string(kernel.quotes, 142));
		player.command_ready = false;
		return;

	case 11:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 152));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 12;
		player.command_ready = false;
		return;

	case 12:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 130, quote_string(kernel.quotes, 143));
		_scene->_sequences.addTimer(150, 13);
		player.command_ready = false;
		return;

	case 13:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 14, 180, quote_string(kernel.quotes, 145));
		player.command_ready = false;
		return;

	case 14:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 0, 60, quote_string(kernel.quotes, 151));
		_scene->_sequences.addTimer(60, 130);
		local._dialogAbortVal = 15;
		player.command_ready = false;
		return;

	case 15:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 16, 180, quote_string(kernel.quotes, 146));
		player.command_ready = false;
		return;

	case 16:
		_scene->_kernelMessages.add(Common::Point(180, 21), 0xFDFC, 0, 17, 60, quote_string(kernel.quotes, 154));
		player.command_ready = false;
		return;

	case 17:
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 130, 60, quote_string(kernel.quotes, 147));
		local._dialogAbortVal = 18;
		player.command_ready = false;
		return;

	case 18:
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 180, quote_string(kernel.quotes, 148));
		local._pitchFl = true;
		local._playingDialogFl = false;
		local._dialogAbortVal = 5;
		player.command_ready = false;
		return;

	default:
		break;
	}

	if (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS) {
		if ((player_said_1(shoot) || player_said_1(hose_down)) && player_said_1(monkey)
			&& player_said_1(blowgun) && player_has(OBJ_BLOWGUN) && player_has(OBJ_POISON_DARTS)) {
			if (player_said_3(shoot, blowgun, monkey) && !local._startShootingInTimerFl) {
				kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(1, 231);
				local._startShootingInTimerFl = true;
				player.commands_allowed = false;
				local._dodgeFl = true;
				player.command_ready = false;
				return;
			}

			if (player_said_3(hose_down, blowgun, monkey) && !local._startShootingInTimerFl) {
				kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
				_scene->_sequences.addTimer(1, 228);
				player.commands_allowed = false;
				local._fallFl = true;
				local._startShootingInTimerFl = true;
				player.command_ready = false;
				return;
			}
		}

		if (player_said_2(look, monkey)) {
			local._pitchFl = true;
			player.commands_allowed = false;
			text_show(20914);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, plant_stalk) && (kernel.trigger || object_is_here(OBJ_PLANT_STALK))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[11] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[11], false, 3, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[11]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			inter_give_to_player(OBJ_PLANT_STALK);
			break;

		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			_scene->_sequences.addTimer(4, 3);
			object_examine(OBJ_PLANT_STALK, 0x328, 0);
			break;

		case 3:
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, binoculars) && (kernel.trigger || object_is_here(OBJ_BINOCULARS))) {
		switch (kernel.trigger) {
		case 0:
			_globals._spriteIndexes[10] = _scene->_sprites.addSprites("*RXMBD_8");
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 3, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[10]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[10], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[9]);
			inter_give_to_player(OBJ_BINOCULARS);
			break;

		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			local._binocularsDroppedFl = false;
			_scene->_sequences.addTimer(4, 3);
			break;

		case 3:
			object_examine(OBJ_BINOCULARS, 0x51AF, 0);
			_scene->_sprites.remove(_globals._spriteIndexes[10]);
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, sky)) {
		text_show(20901);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, bamboo_like_plant)) {
		text_show(20902);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, mountainside)) {
		text_show(20903);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, grassy_field)) {
		text_show(20904);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, field_to_west)) {
		text_show(20905);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, rocky_area_to_north)) {
		text_show(20906);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, plant_stalk) && (player.main_object_source == 4)) {
		text_show(20907);
		player.command_ready = false;
		return;
	}

	if (player_said_3(give, twinkifruit, monkey) || player_said_3(throw, twinkifruit, monkey)) {
		text_show(20909);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, trees)) {
		text_show(20913);
		player.command_ready = false;
		return;
	}

	if (player_said_2(throw, monkey) && player_has(object_named(player2.words[1]))) {
		if (!player_said_1(poison_darts)) {
			text_show(20915);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_3(throw, poison_darts, monkey)) {
		text_show(20916);
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, palm_tree)) {
		if (_globals[kMonkeyStatus] == MONKEY_HAS_BINOCULARS) {
			if (local._monkeyPosition == 1)
				text_show(20917);
			else
				text_show(20918);
		} else {
			if (_globals[kMonkeyStatus] == MONKEY_AMBUSH_READY)
				text_show(20917);
			else
				text_show(20919);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(look, melon_mush)) {
		text_show(20920);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, melon_mush)) {
		text_show(20921);
		player.command_ready = false;
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
} // namespace MADS
