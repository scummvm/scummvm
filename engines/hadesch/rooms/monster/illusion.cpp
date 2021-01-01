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

#include "common/translation.h"
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/rooms/monster.h"

namespace Hadesch {

struct BirdInfo {
	int _projectileFrame;
	const char *_flyAnim;
	int _birdWidth;
	int _birdHeight;
	const char *_interceptAnim;
	const char *_shootAnim;
	int _birdShootWidth;
	int _birdShootHeight;	

	Common::Point getBirdSize() const {
		return Common::Point(_birdWidth, _birdHeight);
	}

	Common::Point getBirdShootSize() const {
		return Common::Point(_birdShootWidth, _birdShootHeight);
	}
};

static const TranscribedSound fakePhilReplics[] = {
	{"v7220xb0", _s("unclear utterance")}, // unclear
	{"v7220xc0", _s("Hey, this was close, buddy")},
	{"v7220xd0", _s("Get hold of thunderbolts")}, // unclear
	{"v7220xe0", _s("Keep going, kid. You're doing great job")},
	{"v7220xf0", _s("unclear utterance")} // unclear
};

static const BirdInfo birdInfo[] = {
	{		
		10,
		"v7220bh2",
		151, 111,
		"v7220bp2",
		"v7220bl2",
		154, 192,
	},
	{
		6,
		"v7220bi2",
		167, 175,
		"v7220bq2",
		"v7220bm2",
		190, 233,
	},
	{
		10,
		"v7220bh3",
		151, 111,
		"v7220bp3",
		"v7220bl3",
		154, 192,
	},
	{
		6,
		"v7220bi3",
		167, 175,
		"v7220bq3",
		"v7220bm3",
		190, 233,
	},
	{
		10,
		"v7220bh0",
		141, 109,
		"v7220bp0",
		"v7220bl0",
		141, 192,
	},
	{
		6,
		"v7220bi0",
		110, 172,
		"v7220bq0",
		"v7220bm0",
		94, 233,
	},
	{
		10,
		"v7220bh1",
		141, 109,
		"v7220bp1",
		"v7220bl1",
		141, 192,
	},
	{
		6,
		"v7220bi1",
		110, 172,
		"v7220bq1",
		"v7220bm1",
		94, 233,
	}
};

void Illusion::stopAnims() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	for (unsigned i = 0; i < ARRAYSIZE(birdInfo); i++)
		for (unsigned j = 0; j < 3; j++) {
			room->stopAnim(LayerId(birdInfo[i]._flyAnim, j, "bird"));
			room->stopAnim(LayerId(birdInfo[i]._interceptAnim, j, "bird"));
			room->stopAnim(LayerId(birdInfo[i]._shootAnim, j, "bird"));
		}
}


Bird::Bird(int id) {
	_id = id;
	_isActive = false;
}

void Bird::launch(int level) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	_isActive = true;
	_level = level;
	makeFlightParams();
	room->playSFX("v7220eb0");
	_flightStart = g_vm->getCurrentTime();
}

void Bird::stop() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->stopAnim(LayerId(birdInfo[_birdType]._flyAnim, _id, "bird"));
}

FlightPosition Bird::getFlightPosition(double t) {
	double t2 = t * t;
	double t3 = t2 * t;
	struct FlightPosition fp;

	// Pseudo-Bezier
	fp.centerPos = ((2 * t3 - 3 * t2 + 1.0) * _startPos
			+ (t3 - 2 * t2 + t) * _attractor1
			+ (t3 - t2) * _attractor2
			+ (-2 * t3 + 3 * t2) * _targetPos);
	fp.scale = 100 * t;

	return fp;
}

void Bird::makeFlightParams() {
	Common::RandomSource &rnd = g_vm->getRnd();
	_startPos = Common::Point(
		rnd.getRandomNumberRng(250, 350),
		rnd.getRandomNumberRng(160, 310));

	if (rnd.getRandomBit()) {
		_targetPos = Common::Point(
			650, rnd.getRandomNumberRng(100, 300));
		_field84 = 1;
		_birdType = rnd.getRandomNumberRng(0, 3);
	} else {
		_targetPos = Common::Point(-50, rnd.getRandomNumberRng(100, 300));
		_field84 = -1;
		_birdType = 4 + rnd.getRandomNumberRng(0, 3);
	}

	int _flightLengthFrames;
	if (_level <= 19) {
		_flightLengthFrames = 51 - _level;
	} else {
		_flightLengthFrames = 50 - _level;
	}

	_flightLengthMs = _flightLengthFrames * 100;
		
	_attractor1 = Common::Point(
		rnd.getRandomNumberRngSigned(-600, 600),
		rnd.getRandomNumberRngSigned(-600, 600));
	_attractor2 = Common::Point(
		rnd.getRandomNumberRngSigned(-600, 600),
		rnd.getRandomNumberRngSigned(-600, 600));

	unsigned lastGoodShootFrame = 11;
	for (; (int) lastGoodShootFrame < _flightLengthFrames; lastGoodShootFrame++) {
		Common::Point p = getFlightPosition(lastGoodShootFrame / (double) _flightLengthFrames).centerPos;
		if (p.x < 50 || p.x > 550 || p.y < 50 || p.y > 350)
			break;
	}
	lastGoodShootFrame--;
	_flightShootAnimFrame = rnd.getRandomNumberRng(10, lastGoodShootFrame);
	_flightShootProjectileFrame = _flightShootAnimFrame + birdInfo[_birdType]._projectileFrame;
	_flightShootEndFrame = _flightShootAnimFrame + (birdInfo[_birdType]._projectileFrame == 6 ? 13 : 18);
	_hasShot = false;
}

// 15201
void Bird::tick(Common::SharedPtr<Bird> backRef, Common::SharedPtr<Battleground> battleground) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (!_isActive)
		return;
	LayerId flyLayer = LayerId(birdInfo[_birdType]._flyAnim, _id, "bird");
	LayerId shootLayer = LayerId(birdInfo[_birdType]._shootAnim, _id, "bird");
	_flightCounterMs = (g_vm->getCurrentTime() - _flightStart);
	if (_flightCounterMs < _flightLengthMs) {
		int frame = _flightCounterMs / 100;
		FlightPosition fp = getFlightPosition(_flightCounterMs / (double) _flightLengthMs);
		int scale = fp.scale;
		if (frame < _flightShootAnimFrame) {
			Common::Point cornerPos = fp.centerPos - (scale / 100.0)
				* birdInfo[_birdType].getBirdSize();
			room->selectFrame(flyLayer, 500, frame % 5, cornerPos);
			room->setScale(flyLayer, scale);
			room->stopAnim(shootLayer);
		} else if (frame < _flightShootEndFrame) {
			Common::Point cornerPos = fp.centerPos - (scale / 100.0)
				* birdInfo[_birdType].getBirdShootSize();
			room->selectFrame(shootLayer, 500, frame - _flightShootAnimFrame, cornerPos);
			room->setScale(shootLayer, scale);
			room->stopAnim(flyLayer);
		} else { // 15204
			Common::Point cornerPos = fp.centerPos - (scale / 100.0)
				* birdInfo[_birdType].getBirdSize();
			room->selectFrame(flyLayer, 500, (frame - _flightShootEndFrame) % 5, cornerPos);
			room->setScale(flyLayer, scale);
			room->stopAnim(shootLayer);
		}

		if (frame >= _flightShootProjectileFrame && !_hasShot) {
			_hasShot = true;
			battleground->launchProjectile(scale / 2, fp.centerPos, _targetPos.x < _startPos.x ? -1 : +1);
		}
	} else {
		room->stopAnim(flyLayer);
		room->stopAnim(shootLayer);
		_isActive = false;
	}

	return;
}

void Bird::handleAbsoluteClick(Common::Point p) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (!_isActive || _flightCounterMs >= _flightLengthMs)
		return;
	FlightPosition fp = getFlightPosition(_flightCounterMs / (double) _flightLengthMs);
	int r = fp.scale * 40 / 100;
	if ((int) p.sqrDist(fp.centerPos) > r * r)
		return;
	room->stopAnim(LayerId(birdInfo[_birdType]._flyAnim, _id, "bird"));
	room->stopAnim(LayerId(birdInfo[_birdType]._shootAnim, _id, "bird"));
	_isActive = false;
	LayerId l = LayerId(birdInfo[_birdType]._interceptAnim, _id, "bird");
	room->playAnimWithSFX(l, "v7220ec0", 500, PlayAnimParams::disappear(),
			      EventHandlerWrapper(),
			      fp.centerPos - birdInfo[_birdType].getBirdSize()
			      * (fp.scale / 100.0));
}

Illusion::Illusion(Common::SharedPtr<Battleground> battleground) {
	_battleground = battleground;
	for (unsigned i = 0; i < 3; i++)
		_birds[i] = Common::SharedPtr<Bird>(new Bird(i));
}

void Illusion::launchBird() {
	for (unsigned i = 0; i < 3; i++) {
		if (!_birds[i]->_isActive) {
			_birds[i]->launch(_battleground->_level);
			break;
		}
	}
}

void Illusion::handleAbsoluteClick(Common::Point p) {
	for (unsigned i = 0; i < ARRAYSIZE(_birds); i++)
		_birds[i]->handleAbsoluteClick(p);
}

void Illusion::movePhil() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (_illusionIsKilled || !_battleground->_isInFight)
		return;
	room->disableHotzone(Common::String::format("Phil%d", _philPosition));
	room->stopAnim(Common::String::format("v7220bt%d", _philPosition));
	_philPosition = g_vm->getRnd().getRandomNumberRng(0, 5);
	room->enableHotzone(Common::String::format("Phil%d", _philPosition));
	room->playAnim(Common::String::format("v7220bt%d", _philPosition), 600,
		       PlayAnimParams::keepLastFrame().partial(0, 12), 15301);
}

void Illusion::enterIllusion(int level) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	Typhoon::disableHotzones();
	for (unsigned i = 0; i < 6; i++)
		room->enableHotzone(Common::String::format("Phil%d", i));
	room->playAnimWithSpeech(Common::String::format("v7220bg%d", g_vm->getRnd().getRandomNumberRng(0, 5)),
				 TranscribedSound::make("v7220xc1",
						  "It's me, Phil. These beasts are all that stands between me and freedom"), 600, // unclear
				 PlayAnimParams::disappear(),
				 15306);
	_battleground->_level = level;
	_battleground->_leavesRemaining = 9;
	_battleground->_monsterNum = kIllusion;
	_philPosition = -1;
	_illusionIsKilled = false;
	g_vm->getHeroBelt()->setBranchOfLifeFrame(0);
}

void Illusion::handleClick(const Common::String &name) {
	if (_battleground->_isInFight && _battleground->_monsterNum == kIllusion && g_vm->getHeroBelt()->getSelectedStrength() == kPowerWisdom && !_illusionIsKilled
	    && _philPosition >= 0 && name == Common::String::format("Phil%d", _philPosition)) {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		_illusionIsKilled = true;
		_battleground->stopFight();
		room->disableMouse();
		room->playAnimKeepLastFrame(Common::String::format("v7220bv%d", _philPosition), 600);
		room->playSFX("v7220eg0", 15307);
		return;
	}
}

void Illusion::handleEvent(int eventId) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	switch (eventId) {
	case 15301:
		g_vm->addTimer(15302, 3500 - (_battleground->_level - 1) * 100);
		break;
	case 15302:
		movePhil();
		break;
	case 15306:
		room->enableMouse();
		_battleground->_isInFight = true;
		movePhil();
		g_vm->addTimer(15312, g_vm->getRnd().getRandomNumberRng(2000, 4000));
		room->playAnimKeepLastFrame("v7220oa0", 600);
		g_vm->addTimer(15313, g_vm->getRnd().getRandomNumberRng(500, 5000));
		launchBird();
		break;
	case 15307:
		room->playSpeech(TranscribedSound::make("v7220wg0", "Oh no, we're gonna fry"), 15308);
		break;
	case 15308:
		room->playSpeech(TranscribedSound::make("v7220wh0", "Let's get outta here"), 15309);
		break;
	case 15309:
		g_vm->getCurrentHandler()->handleEvent(15383);
		break;
	case 15312:
		if (!_battleground->_isInFight || _illusionIsKilled || _battleground->_monsterNum != kIllusion)
			return;
		room->playSpeech(fakePhilReplics[g_vm->getRnd().getRandomNumberRng(0, ARRAYSIZE(fakePhilReplics) - 1)]);
		g_vm->addTimer(15312, g_vm->getRnd().getRandomNumberRng(6000, 10000));
		break;
	case 15313:
		if (!_battleground->_isInFight || _illusionIsKilled || _battleground->_monsterNum != kIllusion)
			return;
		g_vm->addTimer(15313, g_vm->getRnd().getRandomNumberRng(500, 5000));
		launchBird();
		break;
		// TODO: 15300, 15304, 15305, 15311
	}
}

void Illusion::tick() {
	if (!_battleground->_isInFight) {
		for (unsigned i = 0; i < 3; i++)
			_birds[i]->_isActive = false;
		return;
	}
	for (unsigned i = 0; i < 3; i++) {
		_birds[i]->tick(_birds[i], _battleground);
	}
}
}
