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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"
#include "kyra/animator_v1.h"

#include "common/system.h"

namespace Kyra {
void KyraEngine_v1::setupTimers() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setupTimers()");
	memset(_timers, 0, sizeof(_timers));

	for (int i = 0; i < 34; i++)
		_timers[i].active = 1;

	_timers[0].func = _timers[1].func = _timers[2].func = _timers[3].func = _timers[4].func = 0; //Unused.
	_timers[5].func = _timers[6].func = _timers[7].func = _timers[8].func = _timers[9].func = 0; //_nullsub51;
	_timers[10].func = _timers[11].func = _timers[12].func = _timers[13].func = 0; //_nullsub50;
	_timers[14].func = &KyraEngine_v1::timerCheckAnimFlag2; //_nullsub52;
	_timers[15].func = &KyraEngine_v1::timerUpdateHeadAnims; //_nullsub48;
	_timers[16].func = &KyraEngine_v1::timerSetFlags1; //_nullsub47;
	_timers[17].func = 0; //sub_15120;
	_timers[18].func = &KyraEngine_v1::timerCheckAnimFlag1; //_nullsub53;
	_timers[19].func = &KyraEngine_v1::timerRedrawAmulet; //_nullsub54;
	_timers[20].func = 0; //offset _timerDummy1
	_timers[21].func = 0; //sub_1517C; 
	_timers[22].func = 0; //offset _timerDummy2
	_timers[23].func = 0; //offset _timerDummy3, 
	_timers[24].func = 0; //_nullsub45;
	_timers[25].func = 0; //offset _timerDummy4
	_timers[26].func = 0; //_nullsub46;
	_timers[27].func = 0; //offset _timerDummy5, 
	_timers[28].func = 0; //offset _timerDummy6
	_timers[29].func = 0; //offset _timerDummy7, 
	_timers[30].func = 0; //offset _timerDummy8, 
	_timers[31].func = &KyraEngine_v1::timerFadeText; //sub_151F8;
	_timers[32].func = &KyraEngine_v1::updateAnimFlag1; //_nullsub61;
	_timers[33].func = &KyraEngine_v1::updateAnimFlag2; //_nullsub62;

	_timers[0].countdown = _timers[1].countdown = _timers[2].countdown = _timers[3].countdown = _timers[4].countdown = -1;
	_timers[5].countdown = 5;
	_timers[6].countdown = 7;
	_timers[7].countdown = 8;
	_timers[8].countdown = 9;
	_timers[9].countdown = 7;
	_timers[10].countdown = _timers[11].countdown = _timers[12].countdown = _timers[13].countdown = 420;
	_timers[14].countdown = 600;
	_timers[15].countdown = 11;
	_timers[16].countdown = _timers[17].countdown = 7200;
	_timers[18].countdown = _timers[19].countdown = 600;
	_timers[20].countdown = 7200;
	_timers[21].countdown = 18000;
	_timers[22].countdown = 7200;
	_timers[23].countdown = _timers[24].countdown = _timers[25].countdown = _timers[26].countdown = _timers[27].countdown = 10800;
	_timers[28].countdown = 21600;
	_timers[29].countdown = 7200;
	_timers[30].countdown = 10800;
	_timers[31].countdown = -1;
	_timers[32].countdown = 9;
	_timers[33].countdown = 3;
}

void KyraEngine_v1::updateGameTimers() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::updateGameTimers()");
	
	if (_system->getMillis() < _timerNextRun)
		return;

	_timerNextRun += 99999;

	for (int i = 0; i < 34; i++) {
		if (_timers[i].active && _timers[i].countdown > -1) {
			if (_timers[i].nextRun <=_system->getMillis()) {
				if (i > 4 && _timers[i].func)
					(*this.*_timers[i].func)(i);

				_timers[i].nextRun = _system->getMillis() + _timers[i].countdown * _tickLength;
			}
		}
		if (_timers[i].nextRun < _timerNextRun)
			_timerNextRun = _timers[i].nextRun;
	}
}

void KyraEngine_v1::clearNextEventTickCount() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::clearNextEventTickCount()");
	_timerNextRun = 0;
}

void KyraEngine_v1::setTimerDelay(uint8 timer, int32 countdown) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setTimerDelay(%i, %d)", timer, countdown);
	_timers[timer].countdown = countdown;
}

int16 KyraEngine_v1::getTimerDelay(uint8 timer) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::getTimerDelay(%i)", timer);
	return _timers[timer].countdown;
}

void KyraEngine_v1::setTimerCountdown(uint8 timer, int32 countdown) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setTimerCountdown(%i, %i)", timer, countdown);
	_timers[timer].countdown = countdown;
	_timers[timer].nextRun = _system->getMillis() + countdown * _tickLength;

	uint32 nextRun = _system->getMillis() + countdown * _tickLength;
	if (nextRun < _timerNextRun)
		_timerNextRun = nextRun;
}

void KyraEngine_v1::enableTimer(uint8 timer) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::enableTimer(%i)", timer);
	_timers[timer].active = 1;
}

void KyraEngine_v1::disableTimer(uint8 timer) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::disableTimer(%i)", timer);
	_timers[timer].active = 0;
}

void KyraEngine_v1::timerUpdateHeadAnims(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerUpdateHeadAnims(%i)", timerNum);
	static int8 currentFrame = 0;
	static const int8 frameTable[] = {4, 5, 4, 5, 4, 5, 0, 1, 4, 5,
								4, 4, 6, 4, 8, 1, 9, 4, -1};

	if (_talkingCharNum < 0)
		return;

	_currHeadShape = frameTable[currentFrame];
	currentFrame++;

	if (frameTable[currentFrame] == -1)
		currentFrame = 0;

	_animator->animRefreshNPC(0);
	_animator->animRefreshNPC(_talkingCharNum);
}

void KyraEngine_v1::timerSetFlags1(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerSetFlags(%i)", timerNum);
	if (_currentCharacter->sceneId == 0x1C)
		return;

	int rndNr = _rnd.getRandomNumberRng(0, 3);

	for (int i = 0; i < 4; i++) {
		if (!queryGameFlag(rndNr + 17)) {
			setGameFlag(rndNr + 17);
			break;
		} else {
			rndNr++;
			if (rndNr > 3)
				rndNr = 0;
		}
	}
}

void KyraEngine_v1::timerFadeText(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerFadeText(%i)", timerNum);
	_fadeText = true;
}

void KyraEngine_v1::updateAnimFlag1(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::updateAnimFlag1(%d)", timerNum);
	if (_brandonStatusBit & 2) {
		_brandonStatusBit0x02Flag = 1;
	}
}

void KyraEngine_v1::updateAnimFlag2(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::updateAnimFlag2(%d)", timerNum);
	if (_brandonStatusBit & 0x20) {
		_brandonStatusBit0x20Flag = 1;
	}
}

void KyraEngine_v1::setTextFadeTimerCountdown(int16 countdown) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setTextFadeTimerCountdown(%i)", countdown);
	//if (countdown == -1)
		//countdown = 32000;

	setTimerCountdown(31, countdown*60);
}

void KyraEngine_v1::timerSetFlags2(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerSetFlags2(%i)", timerNum);
	if (!((uint32*)(_flagsTable+0x2D))[timerNum])
		((uint32*)(_flagsTable+0x2D))[timerNum] = 1;	
}

void KyraEngine_v1::timerCheckAnimFlag1(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerCheckAnimFlag1(%i)", timerNum);
	if (_brandonStatusBit & 0x20) {
		checkAmuletAnimFlags();
		setTimerCountdown(18, -1);
	}
}

void KyraEngine_v1::timerCheckAnimFlag2(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerCheckAnimFlag1(%i)", timerNum);
	if (_brandonStatusBit & 0x2) {
		checkAmuletAnimFlags();
		setTimerCountdown(14, -1);
	}
}

void KyraEngine_v1::checkAmuletAnimFlags() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::checkSpecialAnimFlags()");
	if (_brandonStatusBit & 2) {
		seq_makeBrandonNormal2();
		setTimerCountdown(19, 300);
	}

	if (_brandonStatusBit & 0x20) {
		seq_makeBrandonNormal();
		setTimerCountdown(19, 300);
	}
}

void KyraEngine_v1::timerRedrawAmulet(int timerNum) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::timerRedrawAmulet(%i)", timerNum);
	if (queryGameFlag(0xF1)) {
		drawAmulet();
		setTimerCountdown(19, -1);
	}
}

void KyraEngine_v1::drawAmulet() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::drawAmulet()");
	static const int16 amuletTable1[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x150, 0x155, 0x15A, 0x15F, 0x164, 0x145, -1};
	static const int16 amuletTable3[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x14F, 0x154, 0x159, 0x15E, 0x163, 0x144, -1};
	static const int16 amuletTable2[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x152, 0x157, 0x15C, 0x161, 0x166, 0x147, -1};
	static const int16 amuletTable4[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x151, 0x156, 0x15B, 0x160, 0x165, 0x146, -1};

	resetGameFlag(0xF1);
	_screen->hideMouse();

	int i = 0;
	while (amuletTable1[i] != -1) {
		if (queryGameFlag(87))
			_screen->drawShape(0, _shapes[amuletTable1[i]], _amuletX[0], _amuletY[0], 0, 0);

		if (queryGameFlag(89))
			_screen->drawShape(0, _shapes[amuletTable2[i]], _amuletX[1], _amuletY[1], 0, 0);

		if (queryGameFlag(86))
			_screen->drawShape(0, _shapes[amuletTable3[i]], _amuletX[2], _amuletY[2], 0, 0);

		if (queryGameFlag(88))
			_screen->drawShape(0, _shapes[amuletTable4[i]], _amuletX[3], _amuletY[3], 0, 0);

		_screen->updateScreen();
		delayWithTicks(3);
		i++;
	}
	_screen->showMouse();
}

void KyraEngine_v1::setWalkspeed(uint8 newSpeed) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setWalkspeed(%i)", newSpeed);
	static const uint8 speeds[] = {11, 9, 6, 5, 3};

	assert(newSpeed < ARRAYSIZE(speeds));
	setTimerDelay(5, speeds[newSpeed]);
}

} // end of namespace Kyra

