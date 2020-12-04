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
 * Copyright 2020 Google
 *
 */

#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/rooms/monster.h"

namespace Hadesch {
static const char *kCyclopsShootingEyeOpen = "v7180bh0";
static const char *kCyclopsShootingEyeClosed = "v7180bh1";

enum {
	kCyclopsZ = 500
};

static const PrePoint cyclopsEyePositions[21] = {
	{247, 175},
	{235, 187},
	{227, 183},
	{221, 178},
	{220, 170},
	{230, 168},
	{230, 168},
	{224, 170},
	{0, 0},
	{0, 0},
	{0, 0},
	{281, 175},
	{282, 170},
	{282, 170},
	{284, 175},
	{270, 178},
	{257, 179},
	{250, 176},
	{249, 176},
	{248, 176},
	{246, 176}
};

static const PrePoint cyclopsEyePositionsBA0[8] = {
	{246, 176},
	{248, 174},
	{249, 166},
	{248, 171},
	{0, 0},
	{241, 183},
	{244, 181},
	{246, 176}
};

void Cyclops::handleClick(Common::Point p) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	int frame;
	if (g_vm->getHeroBelt()->getSelectedStrength() != kPowerStealth || (_currentCyclopsState != 0 && _currentCyclopsState != 1))
		return;
	switch (_currentCyclopsState) {
	case 0:
		if (!_cyclopsIsHiding)
			return;
		frame = room->getAnimFrameNum("v7180bh0");
		break;
	case 1:
		frame = room->getAnimFrameNum("v7040ba0");
		break;
	default:
		return;
	}

	if (!cyclopsIsHit(p, frame))
		return;
	room->disableMouse();
	_battleground->stopFight();
	room->playAnimWithSFX("v7180bj0", "v7180xc0", 500, PlayAnimParams::disappear(), 15352);
}

bool Cyclops::cyclopsIsHit(Common::Point p, int frame) {
	if (frame < 0 || frame >= ARRAYSIZE(cyclopsEyePositions) || cyclopsEyePositions[frame].get() == Common::Point(0, 0))
		return false;
	return cyclopsEyePositions[frame].get().sqrDist(p) <= getSquareOfPrecision();
}

bool Cyclops::cyclopsIsHitBA0(Common::Point p, int frame) {
	if (frame < 0 || frame >= ARRAYSIZE(cyclopsEyePositionsBA0) || cyclopsEyePositionsBA0[frame].get() == Common::Point(0, 0))
		return false;
	return cyclopsEyePositionsBA0[frame].get().sqrDist(p) <= getSquareOfPrecision();
}

unsigned Cyclops::getSquareOfPrecision() {
	return 2050 - 50 * _battleground->_level;
}

void Cyclops::cyclopsState0() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_currentCyclopsState = 0;
	Common::Point mousePos = g_vm->getMousePos();
	_cyclopsIsHiding = (g_vm->getHeroBelt()->getSelectedStrength() == kPowerStealth || !cyclopsIsHit(mousePos, 0));
	room->playAnim(kCyclopsShootingEyeClosed, kCyclopsZ, PlayAnimParams::disappear().partial(0, 11), kCyclopsShootingEyeClosedMidAnim);
}

void Cyclops::cyclopsState1() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_currentCyclopsState = 1;

	room->playAnimWithSFX("v7040ba0", "v7040ea0",
			      kCyclopsZ,
			      PlayAnimParams::disappear(),
			      15257);
}

void Cyclops::cyclopsState2() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	room->playAnimWithSFX(
		"v7180be0", "v7180ee0", kCyclopsZ,
		PlayAnimParams::disappear()
		.partial(0, 4), 15258);
	_currentCyclopsState = 2;
}

void Cyclops::cyclopsState3() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_currentCyclopsState = 3;
	room->playAnim("v7180be0", kCyclopsZ, PlayAnimParams::disappear().partial(5, 11), 15259);
}

void Cyclops::cyclopsState4() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_currentCyclopsState = 4;
	room->playAnimWithSFX("v7180bk0", "v7180sc0", kCyclopsZ, PlayAnimParams::disappear(), 15260);
}

void Cyclops::cyclopsState5() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_currentCyclopsState = 5;
	room->playAnimWithSFX("v7180bi0", "v7180sa0", kCyclopsZ, PlayAnimParams::disappear().partial(0, 4), 15262);
}

void Cyclops::cyclopsState6() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_currentCyclopsState = 6;

	room->playAnimWithSFX("v7180bi0", "v7180ee0", kCyclopsZ, PlayAnimParams::disappear().partial(5, 11), 15264);
}

Cyclops::Cyclops(Common::SharedPtr<Battleground> battleground) {
	_battleground = battleground;
}

void Cyclops::enterCyclops(int level) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->playAnimKeepLastFrame("v7180oa0", 600);
	room->playAnimWithSFX("v7180ba0", "v7180ea0", kCyclopsZ,
			      PlayAnimParams::disappear(),
			      15252);
	Typhoon::disableHotzones();
	_cyclopsProximityCheckCountdown = 0;
	_currentCyclopsState = 0;
	_cyclopsIsHiding = true;
	_battleground->_level = level;
	_battleground->_leavesRemaining = 9;
	_battleground->_monsterNum = kCyclops;
	g_vm->getHeroBelt()->setBranchOfLifeFrame(0);
}

void Cyclops::handleEvent(int eventId) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	switch (eventId) {
	case 15252:
		room->enableMouse();
		room->playAnimLoop("v7180bl0", 550);
		_battleground->_isInFight = true;
		g_vm->addTimer(15266, 100, -1);
		switch (g_vm->getRnd().getRandomNumberRng(0, 2)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState1();
			break;
		case 2:
			cyclopsState2();
			break;
		}
		break;
	case 15255: {
		if (_cyclopsIsHiding) {
			switch (g_vm->getRnd().getRandomNumberRng(0, 4)) {
			case 0:
				cyclopsState0();
				break;
			case 1:
				cyclopsState1();
				break;
			case 2:
			case 3:
			case 4:
				cyclopsState2();
				break;
			}
			break;
		}
		switch (g_vm->getRnd().getRandomNumberRng(0, 4)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState1();
			break;
		case 2:
			cyclopsState3();
			break;
		case 3:
			cyclopsState4();
			break;
		case 4:
			cyclopsState5();
			break;
		}
		break;
	}
	case 15257:
		switch (g_vm->getRnd().getRandomNumberRng(0, 2)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState1();
			break;
		case 2:
			cyclopsState2();
			break;
		}
		break;
	case 15258:
		switch (g_vm->getRnd().getRandomNumberRng(0, 3)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState3();
			break;
		case 2:
			cyclopsState4();
			break;
		case 3:
			cyclopsState5();
			break;
		}
		break;
	case 15259:
		switch(g_vm->getRnd().getRandomNumberRng(0, 2)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState1();
			break;
		case 2:
			cyclopsState2();
			break;
		}
		break;
	case 15260:
		switch (g_vm->getRnd().getRandomNumberRng(0, 3)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState3();
			break;
		case 2:
			cyclopsState4();
			break;
		case 3:
			cyclopsState5();
			break;
		}
		break;
	case 15262:
		switch(g_vm->getRnd().getRandomNumberRng(0, 2)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState3();
			break;
		case 2:
			cyclopsState6();
			break;
		}
		break;
	case 15264:
		switch(g_vm->getRnd().getRandomNumberRng(0, 3)) {
		case 0:
			cyclopsState0();
			break;
		case 1:
			cyclopsState3();
			break;
		case 2:
			cyclopsState4();
			break;
		case 3:
			cyclopsState5();
			break;
		}
		break;
	case 15266:
		if (!_battleground->_isInFight || _battleground->_monsterNum != kCyclops)
			break;
		if (_currentCyclopsState == 0) {
			Common::Point mousePos = g_vm->getMousePos();
			if (_cyclopsProximityCheckCountdown)
				_cyclopsProximityCheckCountdown--;
			if (_cyclopsProximityCheckCountdown == 0) {
				if (_cyclopsIsHiding) {
					int frame = room->getAnimFrameNum("v7180bh0");
					if (g_vm->getHeroBelt()->getSelectedStrength() != kPowerStealth
					    && cyclopsIsHit(mousePos, frame)) {
						room->stopAnim("v7180bh0");
						room->playAnim("v7180bh1", kCyclopsZ, PlayAnimParams::disappear().partial(frame, -1), 15255);
						_cyclopsProximityCheckCountdown = 5;
						_cyclopsIsHiding = false;
					}
				} else {
					int frame = room->getAnimFrameNum("v7180bh1");
					if (frame <= 20 && frame >= 0 && !cyclopsIsHit(mousePos, frame) ) {
						room->stopAnim("v7180bh1");
						room->playAnim("v7180bh0", kCyclopsZ, PlayAnimParams::disappear().partial(frame, -1), 15255);
						_cyclopsIsHiding = true;
					}
				}
			}
		}
		if (_currentCyclopsState == 1) {
			Common::Point mousePos = g_vm->getMousePos();
			if (g_vm->getHeroBelt()->getSelectedStrength() != kPowerStealth
			    && cyclopsIsHitBA0(mousePos, room->getAnimFrameNum("v7040ba0"))) {
				room->stopAnim("v7040ba0");
				_currentCyclopsState = 2;
				room->playAnim(
					"v7180be0", kCyclopsZ,
					PlayAnimParams::disappear()
					.partial(0, 4), 15258);
			}
		}
		break;
	case 15269:
		for (int i = 0; i < _battleground->getNumOfProjectiles(); i++) {
			_battleground->launchProjectile(50, Common::Point(60, 203), 0);
		}
		room->playSFX("v7140eb0");
		break;
	case kCyclopsShootingEyeClosedMidAnim:
		room->playAnim(kCyclopsShootingEyeClosed, kCyclopsZ, PlayAnimParams::disappear().partial(12, -1), 15255);
		handleEvent(15269);
		break;
	case kCyclopsShootingEyeOpenMidAnim:
		room->playAnim(kCyclopsShootingEyeOpen, kCyclopsZ, PlayAnimParams::disappear().partial(12, -1), 15255);
		handleEvent(15269);
		break;
	}
}

}
