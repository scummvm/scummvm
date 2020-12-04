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

enum {
	kTyphoonZ = 500
};

struct TyphoonHeadInfo {
	const char *_animDie;
	const char *_animRespawn;
	const char * _animNormal;
	const char * _hotZone;
	int _xVal;
	int _yVal;
	int _zVal;

	Common::Point getPosition() const {
		return Common::Point(_xVal, _yVal);
	}
};

static const TyphoonHeadInfo typhonHeadInfo[] = {
	{"V7210BO1", "V7210BS1", "V7210BC1", "head00c1", 275, 186, 480},
	{"V7210BO0", "V7210BS0", "V7210BC0", "head01c0", 320, 166, 481},
	{"V7210BO0", "V7210BS0", "V7210BC0", "head02c0", 313, 221, 482},
	{"V7210BO1", "V7210BS1", "V7210BC1", "head03c1", 279, 223, 483},
	{"V7210BP1", "V7210BT1", "V7210BD1", "head04d1", 237, 221, 484},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head05d0", 234, 189, 485},
	{"V7210BP1", "V7210BT1", "V7210BD1", "head06d1", 234, 160, 486},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head07d0", 289, 137, 487},
	{"V7210BO0", "V7210BS0", "V7210BC0", "head08c0", 253, 135, 488},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head09d0", 355, 219, 489},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head10d0", 368, 182, 490},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head11d0", 351, 152, 491},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head12d0", 329, 126, 492},
	{"V7210BO0", "V7210BS0", "V7210BC0", "head13c0", 289,  99, 493},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head14d0", 333, 107, 494},
	{"V7210BO0", "V7210BS0", "V7210BC0", "head15c0", 360, 135, 495},
	{"V7210BO1", "V7210BS1", "V7210BC1", "head16c1", 226, 147, 496},
	{"V7210BP0", "V7210BT0", "V7210BD0", "head17d0", 257, 107, 497}
};

Typhoon::Typhoon(Common::SharedPtr<Battleground> battleground) {
	_battleground = battleground;
	_playingTyphoonRespawnSound = false;
	_playingTyphoonDieSound = false;
	for (unsigned i = 0; i < ARRAYSIZE(_headIsAlive); i++)
		_headIsAlive[i] = false;
}

void Typhoon::handleEvent(int eventId) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	switch (eventId) {
	case 15104:
		_playingTyphoonDieSound = false;
		break;
	case 15105:
		_playingTyphoonRespawnSound = false;
		break;
	case 15152:
		room->enableMouse();
		room->playAnimLoop("v7210bx0", 490);
		room->playSFX("v7210ed0");
		_battleground->_isInFight = true;
		typhoonA();
		for (unsigned i = 0; i < ARRAYSIZE(_headIsAlive); i++) {
			showHeadNormal(i);
			_headIsAlive[i] = true;
		}
		schedule15154();
		handleEvent(15163);
		break;
	case 15153:
		typhoonA();
		break;
	case 15154:
		if (!_battleground->_isInFight || _isKilled || _battleground->_monsterNum != kTyphoon)
			return;
		room->playSFX("v7050ea0");
		schedule15154();
		break;
	case 15159:
		room->playAnim("v7210bj0", 500, PlayAnimParams::disappear().partial(7, -1), 15153);
		if (!_isKilled && _battleground->_isInFight) {
			for (int y = 351, i = 0; i < _battleground->getNumOfProjectiles(); y++, i++)
				_battleground->launchProjectile(80, Common::Point(
									220, g_vm->getRnd().getRandomNumberRng(351, y)), 0);
		}
		break;
	case 15160:
		room->playAnim("v7210bi0", 500, PlayAnimParams::disappear().partial(7, -1), 15153);
		if (!_isKilled && _battleground->_isInFight) {
			for (int y = 359, i = 0; i < _battleground->getNumOfProjectiles(); y++, i++)
				_battleground->launchProjectile(80, Common::Point(
									456, g_vm->getRnd().getRandomNumberRng(359, y)), 0);
		}
		break;
	case 15163:
		if (!_battleground->_isInFight || _isKilled || _battleground->_monsterNum != kTyphoon)
			return;
		room->playSFX(g_vm->getHeroBelt()->getSelectedStrength() == kPowerStrength
			      ? "v7210eb0" : "v7210ea0");
		g_vm->addTimer(15163, g_vm->getRnd().getRandomNumberRng(3000, 7000));
		break;
/*
  TODO:
  15167
*/
	case 15168:
		g_vm->getCurrentHandler()->handleEvent(15351);
		break;
	}
}

void Typhoon::enterTyphoon(int level) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->playAnimKeepLastFrame("v7210oa0", 600);
	room->playAnim("v7210ba0", kTyphoonZ,
		       PlayAnimParams::disappear(),
		       15152);
	room->playSFX("v7050eb0");
	for (unsigned i = 0; i < ARRAYSIZE(typhonHeadInfo); i++) {
		room->enableHotzone(typhonHeadInfo[i]._hotZone);
		room->setHotZoneOffset(typhonHeadInfo[i]._hotZone, typhonHeadInfo[i].getPosition());
	}
	for (unsigned i = 0; i < 6; i++)
		room->disableHotzone(Common::String::format("Phil%d", i));
	_battleground->_level = level;
	_battleground->_leavesRemaining = 9;
	_battleground->_monsterNum = kTyphoon;
	_isKilled = false;
	_playingTyphoonDieSound = false;
	g_vm->getHeroBelt()->setBranchOfLifeFrame(0);
}

void Typhoon::handleClick(Common::SharedPtr<Typhoon> backRef,
			  const Common::String &name) {
	if (_battleground->_isInFight && _battleground->_monsterNum == kTyphoon
	    && g_vm->getHeroBelt()->getSelectedStrength() == kPowerStrength && !_isKilled) {
		for (unsigned i = 0; i < ARRAYSIZE(typhonHeadInfo); i++)
			if (name == typhonHeadInfo[i]._hotZone) {
				hitTyphoonHead(backRef, i);
				return;
			}
	}
}


void Typhoon::typhoonA() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (_isKilled)
		return;

	if (g_vm->getRnd().getRandomNumberRng(0, 3)) {
		room->playAnim("v7050ba0", 500, PlayAnimParams::disappear(), 15153);
	} else if (g_vm->getRnd().getRandomBit()) {
		room->playAnim("v7210bi0", 500, PlayAnimParams::disappear().partial(0, 6), 15160);
		room->playSFX("v7140ec0");
	}
	else {
		room->playAnim("v7210bj0", 500, PlayAnimParams::disappear().partial(0, 6), 15159);
		room->playSFX("v7140ec0");
	}
}

void Typhoon::schedule15154() {
	int ha = typhonGetNumAliveHeads() * 50;
	g_vm->addTimer(15154, g_vm->getRnd().getRandomNumberRng(1100-ha, 1200 - ha));
}

int Typhoon::typhonGetNumAliveHeads() {
	int v = 0;
	for (unsigned i = 0; i < ARRAYSIZE(_headIsAlive); i++)
		v += !!_headIsAlive[i];
	return v;
}

void Typhoon::hideHead(int idx) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->stopAnim(LayerId(typhonHeadInfo[idx]._animNormal, idx, "head"));
	room->stopAnim(LayerId(typhonHeadInfo[idx]._animDie, idx, "head"));
	room->stopAnim(LayerId(typhonHeadInfo[idx]._animRespawn, idx, "head"));
}

void Typhoon::showHeadNormal(int idx) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	hideHead(idx);
	room->playAnimLoop(LayerId(typhonHeadInfo[idx]._animNormal, idx, "head"),
			   typhonHeadInfo[idx]._zVal,
			   typhonHeadInfo[idx].getPosition());
}

// 15103
class TyphoonHeadRespawnComplete : public EventHandler {
public:
	void operator()() override {
		_typhoon->showHeadNormal(_idx);
	}

	TyphoonHeadRespawnComplete(Common::SharedPtr<Typhoon> typhoon, int idx) {
		_idx = idx;
		_typhoon = typhoon;
	}
private:
	int _idx;
	Common::SharedPtr<Typhoon> _typhoon;
};

// 15102
class TyphoonHeadRespawnEvent : public EventHandler {
public:
	void operator()() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		if (_typhoon->_headIsAlive[_idx] || _typhoon->_isKilled)
			return;
		room->enableHotzone(typhonHeadInfo[_idx]._hotZone);
		_typhoon->_headIsAlive[_idx] = true;
		if (!_typhoon->_playingTyphoonRespawnSound) {
			_typhoon->_playingTyphoonRespawnSound = true;
			room->playSFX("v7050ed0", 15105);
		}
		_typhoon->hideHead(_idx);
		room->playAnim(LayerId(typhonHeadInfo[_idx]._animRespawn, _idx, "head"),
			       typhonHeadInfo[_idx]._zVal,
			       PlayAnimParams::disappear(),
			       Common::SharedPtr<EventHandler>(new TyphoonHeadRespawnComplete(_typhoon, _idx)),
			       typhonHeadInfo[_idx].getPosition());
	}

	TyphoonHeadRespawnEvent(Common::SharedPtr<Typhoon> typhoon, int idx) {
		_idx = idx;
		_typhoon = typhoon;
	}
private:
	int _idx;
	Common::SharedPtr<Typhoon> _typhoon;
};

// 15101
class TyphoonHeadDieAnimFinishedEvent : public EventHandler {
public:
	void operator()() override {
		int minRespawnInterval = 10000;
		int maxRespawnInterval = 10000;

		if (_level <= 21)
			minRespawnInterval = 15000 - 500 * (_level - 1);
		else if (_level == 22)
			minRespawnInterval = 4600;
		else if (_level <= 25)
			minRespawnInterval = 4200 - 200 * (_level - 23);
		else if (_level == 26)
			minRespawnInterval = 3700;
		else
			minRespawnInterval = 3600 - 200 * (_level - 27);

		if (_level <= 21)
			maxRespawnInterval = 20000 - 500 * (_level - 1);
		else
			maxRespawnInterval = 9600 - 200 * (_level - 22);

		g_vm->addTimer(Common::SharedPtr<EventHandler>(new TyphoonHeadRespawnEvent(_typhoon, _idx)),
			       g_vm->getRnd().getRandomNumberRng(minRespawnInterval, maxRespawnInterval));
	}

	TyphoonHeadDieAnimFinishedEvent(Common::SharedPtr<Typhoon> typhoon, int idx, int level) {
		_idx = idx;
		_level = level;
		_typhoon = typhoon;
	}
private:
	int _idx;
	int _level;
	Common::SharedPtr<Typhoon> _typhoon;
};

void Typhoon::hitTyphoonHead(Common::SharedPtr<Typhoon> backRef, int idx) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (!_headIsAlive[idx])
		return;

	if (!_playingTyphoonDieSound) {
		room->playSFX("v7050ec0", 15104);
		_playingTyphoonDieSound = true;
	}
	_headIsAlive[idx] = false;
	hideHead(idx);
	room->playAnimKeepLastFrame(LayerId(typhonHeadInfo[idx]._animDie, idx, "head"),
				    typhonHeadInfo[idx]._zVal,
				    Common::SharedPtr<EventHandler>(new TyphoonHeadDieAnimFinishedEvent(backRef, idx, _battleground->_level)),
				    typhonHeadInfo[idx].getPosition());
	room->disableHotzone(typhonHeadInfo[idx]._hotZone);
	bool isKilled = true;
	for (unsigned i = 0; i < ARRAYSIZE(_headIsAlive); i++) {
		if (_headIsAlive[i])
			isKilled = false;
	}

	if (!isKilled)
		return;
	_isKilled = true;
	_battleground->stopFight();

	room->disableMouse();
	room->playAnimWithSFX("v7210bw0", "v7050ee0", 500, PlayAnimParams::disappear(), 15168);
}

void Typhoon::stopAnims() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	for (unsigned i = 0; i < ARRAYSIZE(typhonHeadInfo); i++) {
		room->stopAnim(LayerId(typhonHeadInfo[i]._animNormal, i, "head"));
		room->stopAnim(LayerId(typhonHeadInfo[i]._animDie, i, "head"));
		room->stopAnim(LayerId(typhonHeadInfo[i]._animRespawn, i, "head"));
		room->stopAnim("v7050ba0");
		room->stopAnim("v7210bi0");
		room->stopAnim("v7140ec0");
		room->stopAnim("v7210bj0");
		room->stopAnim("v7140ec0");
	}
}

void Typhoon::disableHotzones() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	for (unsigned i = 0; i < ARRAYSIZE(typhonHeadInfo); i++)
		room->disableHotzone(typhonHeadInfo[i]._hotZone);
}

}
