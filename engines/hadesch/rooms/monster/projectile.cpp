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

Projectile::Projectile(int id, int level, Monster monster, int startScale, Common::Point startPoint, int xmomentum) {
	_level = level;
	switch (monster) {
	case kCyclops:
		_flyAnim = "V7140BA0";
		_interceptAnim = "V7130BD0";
		_hitAnim = "V7140BD0";
		break;
	case kTyphoon:
		_flyAnim = "V7140BB0";
		_interceptAnim = "V7130BD1";
		_hitAnim = "V7140BE0";
		break;
	case kIllusion:
		_flyAnim = "V7140BC0";
		_interceptAnim = "V7130BD2";
		_hitAnim = "V7140BF0";
		break;			
	}
	_isMiss = g_vm->getRnd().getRandomNumberRng(0, getProjectileHitChance()) == 0;
	_isFlightFinished = false;
	_flightCounterMs = -1;
	_projectileId = id;
	_pending = 0;

	_flightStart = g_vm->getCurrentTime();
	_startScale = startScale;
	_start = startPoint;
	makeFlightParams(xmomentum);
}

void Projectile::handleEvent(int ev) {
	switch (ev) {
	case 15053:
		g_vm->handleEvent(kHitReceived);
		// TODO: stop red
		_pending--;
		break;
	case 15054:
		_pending--;
		break;
	}
}

void Projectile::stop() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->stopAnim(LayerId(_flyAnim, _projectileId, "projectile"));
	room->stopAnim(LayerId(_hitAnim, _projectileId, "projectile"));
	room->stopAnim(LayerId(_interceptAnim, _projectileId, "projectile"));
}

void Projectile::makeFlightParams(int xmomentum) {
	Common::RandomSource &rnd = g_vm->getRnd();
	_flightLengthMs = getProjectileFlightLength(_level) * 100;
		
	if (_isMiss) {
		switch (rnd.getRandomNumberRng(0, 2)) {
		case 0:
			_target = Common::Point(
				-50, rnd.getRandomNumberRngSigned(-50, 400));
			break;
		case 1:
			_target = Common::Point(
				rnd.getRandomNumberRngSigned(-50, 650), -50);
			break;
		case 2:
			_target = Common::Point(
				650, rnd.getRandomNumberRngSigned(-50, 400));
			break;
		}
	} else {
		_target = Common::Point(
			rnd.getRandomNumberRng(100, 500),
			rnd.getRandomNumberRng(100, 300));
	}
		
	switch (xmomentum) {
	case 1:
		_attractor1 = Common::Point(
			rnd.getRandomNumberRng(0, 600),
			rnd.getRandomNumberRng(0, 300));
		break;
	case -1:
		_attractor1 = Common::Point(
			rnd.getRandomNumberRngSigned(-600, 0),
			rnd.getRandomNumberRng(0, 300));
		break;
	case 0:
		_attractor1 = Common::Point(
			rnd.getRandomNumberRngSigned(-600, 600),
			rnd.getRandomNumberRngSigned(-600, 600));
		break;
	}
	_attractor2 = Common::Point(
		rnd.getRandomNumberRngSigned(-600, 600),
		rnd.getRandomNumberRng(0, 600));
}

FlightPosition Projectile::getFlightPosition(double t) {
	double t2 = t * t;
	double t3 = t2 * t;
	struct FlightPosition fp;

	// Pseudo-Bezier
	fp.centerPos = ((2 * t3 - 3 * t2 + 1.0) * _start
			+ (t3 - 2 * t2 + t) * _attractor1
			+ (t3 - t2) * _attractor2
			+ (-2 * t3 + 3 * t2) * _target);
	fp.scale = _startScale + (120 - _startScale) * t;

	return fp;
}

Projectile::~Projectile() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->purgeAnim(LayerId(_flyAnim, _projectileId, "projectile"));
	room->purgeAnim(LayerId(_hitAnim, _projectileId, "projectile"));
	room->purgeAnim(LayerId(_interceptAnim, _projectileId, "projectile"));
}

int Projectile::getProjectileFlightLength(int level) {
	return 41 - _level;
}

int Projectile::getProjectileHitChance() {
	if (_level >= 26)
		return 6;
	if (_level >= 17)
		return 5;
	if (_level >= 12)
		return 4;
	if (_level >= 7)
		return 3;
	return 2;
}

Battleground::Battleground() {
	_level = 1;
	_projectileId = 0;
	_isInFight = false;
}

int Battleground::getNumOfProjectiles() {
	return (_level - 1) / 10 + 1;
}

void Battleground::launchProjectile(int startScale, Common::Point startPoint, int xmomentum) {
	++_projectileId;
	Common::SharedPtr<Projectile> pj(new Projectile(_projectileId, _level, _monsterNum, startScale, startPoint, xmomentum));
	_projectiles.push_back(pj);
	pj->tick(pj);
}

void Battleground::handleAbsoluteClick(Common::Point p) {
	for (Common::Array<Common::SharedPtr<Projectile> >::iterator it = _projectiles.begin(); it != _projectiles.end(); it++) {
		it->operator->()->handleAbsoluteClick(*it, p);
	}
}

void Battleground::tick() {
	if (!_isInFight)
		_projectiles.clear();
	else
		for (Common::Array<Common::SharedPtr<Projectile> >::iterator it = _projectiles.begin(); it != _projectiles.end();) {
			if (it->operator->()->tick(*it)) {
				it++;
			} else {
				it = _projectiles.erase(it);
			}
		}
}

void Battleground::stopFight() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_isInFight = false;
	room->stopAnim("v7040ba0");
	room->stopAnim("V7100BJ0");
	room->stopAnim("v7180ba0");
	room->stopAnim("V7180BB0");
	room->stopAnim("v7180be0");
	room->stopAnim("v7180bh0");
	room->stopAnim("v7180bh1");
	room->stopAnim("v7180bi0");
	room->stopAnim("v7180bk0");
	room->stopAnim("v7180bl0");
	room->stopAnim("v7180oa0");
	room->stopAnim("v7210bx0");

	stopProjectiles();

	Typhoon::stopAnims();
	Illusion::stopAnims();

	for (unsigned i = 0; i < 6; i++) {
		room->stopAnim(Common::String::format("v7220bt%d", i));
		room->stopAnim(Common::String::format("v7220bg%d", i));
	}

	room->dumpLayers();
}

void Battleground::stopProjectiles() {
	for (Common::Array<Common::SharedPtr<Projectile> >::iterator it = _projectiles.begin(); it != _projectiles.end(); it++)
		it->operator->()->stop();
}

class HandlerProjectile : public EventHandler {
public:
	void operator()() {
		_projectile->handleEvent(_event);
	}

	HandlerProjectile(Common::SharedPtr <Projectile> projectile,
			  int event) {
		_projectile = projectile;
		_event = event;
	}

private:
	Common::SharedPtr <Projectile> _projectile;
	int _event;
};

bool Projectile::tick(Common::SharedPtr <Projectile> backRef) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (_isFlightFinished)
		return _pending > 0;
	LayerId flyLayer = LayerId(_flyAnim, _projectileId, "projectile");
	_flightCounterMs = (g_vm->getCurrentTime() - _flightStart);
	if (_flightCounterMs < _flightLengthMs) {
		FlightPosition fp = getFlightPosition(_flightCounterMs / (double) _flightLengthMs);
		int scale = fp.scale;
		Common::Point cornerPos = fp.centerPos - (scale / 100.0)
			* Common::Point(186, 210);
		room->selectFrame(flyLayer, 400, (_flightCounterMs / 100) % 8, cornerPos);
		room->setScale(flyLayer, scale);
	} else {
		room->stopAnim(flyLayer);
		_isFlightFinished = true;
		if (_isMiss) {
			_pending = 0;
		} else {
			FlightPosition fp = getFlightPosition(_flightCounterMs / (double) _flightLengthMs);
			LayerId l = LayerId(_hitAnim, _projectileId, "projectile");
			room->playAnimWithSFX(l, "v7130ea0", 400, PlayAnimParams::disappear(),
					      Common::SharedPtr<EventHandler>(new HandlerProjectile(backRef, 15053)),
					      fp.centerPos - Common::Point(182, 205));
			_pending = 1;
			// TODO: fade to red, in 100 ms, callback 15055
			// TODO: shake camera for 1s
		}
	}

	return true;
}
	
void Projectile::handleAbsoluteClick(Common::SharedPtr <Projectile> backRef, Common::Point p) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (_isFlightFinished || _flightCounterMs >= _flightLengthMs)
		return;
	FlightPosition fp = getFlightPosition(_flightCounterMs / (double) _flightLengthMs);
	int r = fp.scale * 40 / 100;
	if ((int) p.sqrDist(fp.centerPos) > r * r)
		return;
	room->stopAnim(LayerId(_flyAnim, _projectileId, "projectile"));
	_isFlightFinished = true;
	_pending = 1;
	LayerId l = LayerId(_interceptAnim, _projectileId, "projectile");
	room->playAnimWithSFX(l, "v7130eg0", 400, PlayAnimParams::disappear(),
			      Common::SharedPtr<EventHandler>(new HandlerProjectile(backRef, 15054)),
			      fp.centerPos - Common::Point(186, 210) * (fp.scale / 100.0));
}

}
