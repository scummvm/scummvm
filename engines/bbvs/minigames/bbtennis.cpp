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
 */

#include "bbvs/sound.h"
#include "bbvs/minigames/bbtennis.h"

namespace Bbvs {

static const int kLeftPlayerOffX[] = {
	-44, -44, -44, -44, -39, -39, -34,
	-26, -26, -14,  -6,  -6,  -6,  -6
};

static const int kLeftPlayerOffY[] = {
	-31, -31, -31, -31, -23, -23, -21,
	-18, -18, -14, -11, -11, -11, -11
};

static const char * const kSoundFilenames[] = {
	"tenis9.aif", "tenis10.aif", "tenis11.aif", "tenis12.aif", "tenis13.aif",
	"tenis14.aif", "tenis15.aif", "tenis16.aif", "tenis17.aif", "tenis18.aif",
	"tenis19.aif", "tenis20.aif", "tenis21.aif", "1ahh.aif", "1dammit.aif",
	"1getawy.aif", "1getthem.aif", "1owww.aif", "1pardon.aif", "1rcktbll.aif",
	"1yourout.aif", "2hey.aif", "2inhere.aif", "2stoptht.aif", "2theyare.aif",
	"3oh.aif", "3ow.aif", "3upunks.aif", "tenismus.aif", "canon1.aif",
	"canon2.aif"
};

static const uint kSoundFilenamesCount = ARRAYSIZE(kSoundFilenames);

static const int kLeftNetPlayAnims[] = {
	13, 15, 17
};

static const int kRightNetPlayAnims[] = {
	14, 16, 18
};

static const uint kYuppieHitSounds[] = {
	14, 15, 18, 22, 26, 27
};

static const uint kYuppieEnteringCourtSounds[] = {
	19, 20
};

static const uint kYuppieChargeSounds[] = {
	16, 17, 23, 24, 28,  0
};

static const uint kAllSounds[] = {
	3,  4,  7,  9, 19, 20, 16, 17, 23, 24, 28
};

void MinigameBbTennis::buildDrawList(DrawList &drawList) {
	switch (_gameState) {
	case 0:
		buildDrawList0(drawList);
		break;
	case 1:
		buildDrawList1(drawList);
		break;
	case 2:
		buildDrawList2(drawList);
		break;
	default:
		break;
	}
}

void MinigameBbTennis::buildDrawList0(DrawList &drawList) {

	drawList.add(_objects[0].anim->frameIndices[_objects[0].frameIndex], _objects[0].x, _objects[0].y, 2000);

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->y + 16);
	}

	if (_titleScreenSpriteIndex > 0)
		drawList.add(_titleScreenSpriteIndex, 0, 0, 0);

}

void MinigameBbTennis::buildDrawList1(DrawList &drawList) {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];

		if (obj->kind) {
			int index = obj->anim->frameIndices[obj->frameIndex];
			int x = obj->x;
			int y = obj->y;
			int priority = obj->y + 16;

			switch (obj->kind) {

			case 1:
				priority = 3000;
				break;

			case 2:
				priority = 550;
				if (obj->frameIndex == 0)
					drawList.add(obj->anim->frameIndices[8], obj->x, obj->y, 550);
				break;

			case 6:
				if (obj->frameIndex == 31) {
					y = 640;
					index = obj->anim->frameIndices[26];
				}
				if (obj->status == 4) {
					--obj->blinkCtr;
					if (obj->blinkCtr % 2)
						y = 600;
					if (obj->blinkCtr == 0)
						obj->kind = 0;
				}
				break;

			case 7:
				priority = 540;
				if (obj->frameIndex == 0)
					drawList.add(obj->anim->frameIndices[8], obj->x, obj->y, 550);
				break;

			case 4:
				if (obj->status == 8) {
					--obj->blinkCtr;
					if (obj->blinkCtr % 2)
						y = 600;
					if (obj->blinkCtr == 0)
						obj->kind = 0;
				}
				break;

			default:
				break;
			}

			drawList.add(index, x, y, priority);

		}
	}

	if (_rapidFireBallsCount > 0) {
		drawList.add(getAnimation(19)->frameIndices[0], 24, 208, 990);
		drawList.add(getAnimation(20)->frameIndices[_rapidFireBallsCount / 10 % 10], 19, 198, 2000);
		drawList.add(getAnimation(20)->frameIndices[_rapidFireBallsCount % 10], 29, 198, 2000);

	}

	if (_backgroundSpriteIndex > 0)
		drawList.add(_backgroundSpriteIndex, 0, 0, 0);

	drawList.add(getAnimation(8)->frameIndices[0], 9, 53, 500);
	drawList.add(getAnimation(9)->frameIndices[0], 256, 52, 500);
	drawList.add(getAnimation(10)->frameIndices[0], 60, 162, 500);
	drawList.add(getAnimation(21)->frameIndices[0], 36, 18, 2000);

	drawNumber(drawList, _score, 70, 18);

	for (int i = 0; i < _numHearts; ++i)
		drawList.add(getAnimation(7)->frameIndices[0], 20 + i * 20, 236, 990);

}

void MinigameBbTennis::buildDrawList2(DrawList &drawList) {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->y + 16);
	}

	if (_backgroundSpriteIndex > 0)
		drawList.add(_backgroundSpriteIndex, 0, 0, 0);

	drawList.add(getAnimation(21)->frameIndices[0], 36, 18, 2000);

	drawNumber(drawList, _score, 70, 18);

	drawList.add(getAnimation(22)->frameIndices[0], 120, 70, 2000);
	drawList.add(getAnimation(23)->frameIndices[0], 95, 95, 2000);

	drawNumber(drawList, _hiScore, 210, 109);

}

void MinigameBbTennis::drawSprites() {
	DrawList drawList;
	buildDrawList(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

void MinigameBbTennis::initObjs() {
	for (int i = 0; i < kMaxObjectsCount; ++i)
		_objects[i].kind = 0;
}

MinigameBbTennis::Obj *MinigameBbTennis::getFreeObject() {
	for (int i = 0; i < kMaxObjectsCount; ++i)
		if (_objects[i].kind == 0)
			return &_objects[i];
	return 0;
}

MinigameBbTennis::Obj *MinigameBbTennis::findTennisBall(int startObjIndex) {
	for (int i = startObjIndex; i < kMaxObjectsCount; ++i)
		if (_objects[i].kind == 2)
			return &_objects[i];
	return 0;
}

bool MinigameBbTennis::isHit(Obj *obj1, Obj *obj2) {
	const BBRect &frameRect1 = obj1->anim->frameRects[obj1->frameIndex];
	const BBRect &frameRect2 = obj2->anim->frameRects[obj2->frameIndex];
	const int obj1X1 = obj1->x + frameRect1.x;
	const int obj1Y1 = obj1->y + frameRect1.y;
	const int obj1X2 = obj1X1 + frameRect1.width;
	const int obj1Y2 = obj1Y1 + frameRect1.height;
	const int obj2X1 = obj2->x + frameRect2.x;
	const int obj2Y1 = obj2->y + frameRect2.y;
	const int obj2X2 = obj2X1 + frameRect2.width;
	const int obj2Y2 = obj2Y1 + frameRect2.height;
	return obj1X1 <= obj2X2 && obj1X2 >= obj2X1 && obj1Y1 <= obj2Y2 && obj1Y2 >= obj2Y1;
}

void MinigameBbTennis::initObjects() {
	switch (_gameState) {
	case 0:
		initObjects0();
		break;
	case 1:
		initObjects1();
		break;
	case 2:
		initObjects2();
		break;
	default:
		break;
	}
}

void MinigameBbTennis::initObjects0() {
	_objects[0].anim = getAnimation(24);
	_objects[0].frameIndex = 0;
	_objects[0].ticks = getAnimation(24)->frameTicks[0];
	_objects[0].x = 160;
	_objects[0].y = 100;
	_objects[0].kind = 1;
	_objects[1].anim = getAnimation(25);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = getAnimation(25)->frameTicks[0];
	_objects[1].x = 40;
	_objects[1].y = 240;
	_objects[1].kind = 2;
	_objects[2].anim = getAnimation(26);
	_objects[2].frameIndex = 0;
	_objects[2].ticks = getAnimation(26)->frameTicks[0];
	_objects[2].x = 280;
	_objects[2].y = 240;
	_objects[2].kind = 2;
}

void MinigameBbTennis::initObjects1() {
	_objects[0].anim = getAnimation(5);
	_objects[0].frameIndex = 0;
	_objects[0].ticks = getAnimation(5)->frameTicks[0];
	_objects[0].status = 0;
	_objects[0].x = 160;
	_objects[0].y = 100;
	_objects[0].kind = 1;
	for (int i = 1; i < kMaxObjectsCount; ++i)
		_objects[i].kind = 0;
}

void MinigameBbTennis::initObjects2() {
	// Nothing
}

void MinigameBbTennis::initVars() {
	switch (_gameState) {
	case 0:
		initVars0();
		break;
	case 1:
		initVars1();
		break;
	case 2:
		initVars2();
		break;
	default:
		break;
	}
}

void MinigameBbTennis::initVars0() {
	// Nothing
}

void MinigameBbTennis::initVars1() {
	_numHearts = 15;
	_allHeartsGone = false;
	_squirrelDelay = 500;
	_tennisPlayerDelay = 300;
	_throwerDelay = 400;
	_netPlayerDelay = 340;
	_playerDecrease = 0;
	_delayDecreaseTimer = 0;
	_numBalls = 0;
	_newBallTimer = 1;
	_initBallTimer = 10;
	_maxBalls = 5;
	_rapidFireBallsCount = 0;
	_score = 0;
	_hitMissRatio = 0;
	_playedThisIsTheCoolest = false;
	_startSoundPlayed = false;
	_endSoundPlaying = false;
	stopSound(12);
}

void MinigameBbTennis::initVars2() {
	if (_score > _hiScore)
		_hiScore = _score;
}

bool MinigameBbTennis::updateStatus(int mouseX, int mouseY, uint mouseButtons) {
	switch (_gameState) {
	case 0:
		return updateStatus0(mouseX, mouseY, mouseButtons);
	case 1:
		return updateStatus1(mouseX, mouseY, mouseButtons);
	case 2:
		return updateStatus2(mouseX, mouseY, mouseButtons);
	default:
		break;
	}
	return false;
}

bool MinigameBbTennis::updateStatus0(int mouseX, int mouseY, uint mouseButtons) {

	if ((mouseButtons & kLeftButtonDown) || (mouseButtons & kRightButtonDown)) {
		_gameState = 1;
		initObjects();
		initVars();
		_gameTicks = 0;
		return true;
	}

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind == 2) {
			if (--obj->ticks == 0) {
				++obj->frameIndex;
				if (obj->frameIndex >= obj->anim->frameCount)
					obj->frameIndex = 0;
				obj->ticks = obj->anim->frameTicks[obj->frameIndex];
			}
		}
	}

	return true;
}

bool MinigameBbTennis::updateStatus1(int mouseX, int mouseY, uint mouseButtons) {

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	if (_allHeartsGone) {
		_gameState = 2;
		initObjects();
		initVars();
		_gameTicks = 0;
		return true;
	}

	if (!_startSoundPlayed) {
		playSound(12);
		_startSoundPlayed = true;
	}

	if (((mouseButtons & kLeftButtonClicked) || (_rapidFireBallsCount > 0 && (mouseButtons & kLeftButtonDown))) &&
		_newBallTimer == 0 && _numBalls < _maxBalls) {
		// Insert a ball
		Obj *obj = getFreeObject();
		obj->anim = getAnimation(6);
		obj->frameIndex = 0;
		obj->ticks = getAnimation(6)->frameTicks[0];
		obj->x = 160;
		obj->y = 240;
		obj->kind = 2;
		obj->targetX = mouseX;
		obj->targetY = mouseY;
		obj->ballStep = 12;
		obj->ballStepCtr = 0;
		obj->fltX = 160.0f;
		obj->fltY = 240.0f;
		obj->fltStepX = ((160 - mouseX) * 0.75f) / 12.0f;
		obj->fltStepY = ((240 - mouseY) * 0.75f) / 12.0f;
		_newBallTimer = _initBallTimer;
		++_numBalls;
		playSound(31);
		if (_rapidFireBallsCount > 0 && --_rapidFireBallsCount == 0) {
			_initBallTimer = 10;
			_maxBalls = 5;
		}
	}

	if (_newBallTimer > 0)
		--_newBallTimer;

	if (++_delayDecreaseTimer == 30) {
		_delayDecreaseTimer = 0;
		if (_playerDecrease < 199)
			++_playerDecrease;
	}

	updateObjs();

	if (!_playedThisIsTheCoolest && _score > 3 && _vm->getRandom(10) == 1 && !isAnySoundPlaying(kAllSounds, 11)) {
		_playedThisIsTheCoolest = true;
		playSound(9);
	}

	return true;
}

bool MinigameBbTennis::updateStatus2(int mouseX, int mouseY, uint mouseButtons) {
	if (_endSoundPlaying) {
		if (!isSoundPlaying(21) && _fromMainGame) {
			//_vm->delayMillis(1000);
			_gameDone = true;
		}
	} else {
		playSound(21);
		_endSoundPlaying = true;
	}
	return true;
}

void MinigameBbTennis::updateObjs() {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		switch (obj->kind) {
		case 2:
			updateTennisBall(i);
			break;
		case 3:
			updateSquirrel(i);
			break;
		case 4:
			updateTennisPlayer(i);
			break;
		case 5:
			updateThrower(i);
			break;
		case 6:
			updateNetPlayer(i);
			break;
		case 7:
			updateEnemyTennisBall(i);
			break;
		default:
			break;
		}
	}

	if (_rapidFireBallsCount == 0) {
		--_squirrelDelay;
		if (--_squirrelDelay == 0) {
			Obj *obj = getFreeObject();
			obj->kind = 3;
			obj->x = 100;
			obj->y = 69;
			obj->anim = getAnimation(1);
			obj->frameIndex = 0;
			obj->ticks = getAnimation(1)->frameTicks[0];
			obj->status = 0;
			obj->blinkCtr = _vm->getRandom(128) + 10;
			_squirrelDelay = _vm->getRandom(512) + 1000;
		}
	}

	if (--_tennisPlayerDelay == 0) {
		Obj *obj = getFreeObject();
		obj->kind = 4;
		obj->y = 146;
		obj->anim = getAnimation(11);
		obj->ticks = getAnimation(11)->frameTicks[0];
		if (_vm->getRandom(2) == 1) {
			obj->x = 40;
			obj->frameIndex = 0;
			obj->status = 0;
		} else {
			obj->x = _vm->getRandom(2) == 1 ? 40 : 274;
			obj->frameIndex = 16;
			obj->status = 4;
		}
		obj->blinkCtr = _vm->getRandom(64) + 60;
		_tennisPlayerDelay = _vm->getRandom(128) + 400 - _playerDecrease;
		if (_vm->getRandom(10) == 1 && !isAnySoundPlaying(kAllSounds, 11))
			playSound(kYuppieEnteringCourtSounds[_vm->getRandom(2)]);
	}

	if (--_throwerDelay == 0) {
		Obj *obj = getFreeObject();
		obj->kind = 5;
		obj->x = 50;
		obj->y = 62;
		obj->anim = getAnimation(12);
		obj->frameIndex = 0;
		obj->ticks = getAnimation(12)->frameTicks[0];
		obj->status = 0;
		_throwerDelay = _vm->getRandom(128) + 200 - _playerDecrease;
		if (_vm->getRandom(10) == 1 && !isAnySoundPlaying(kAllSounds, 11))
			playSound(kYuppieChargeSounds[_vm->getRandom(2)]);
	}

	if (--_netPlayerDelay == 0) {
		Obj *obj = getFreeObject();
		obj->kind = 6;
		obj->y = 176;
		if (_vm->getRandom(2) == 1) {
			obj->x = 110;
			obj->netPlayDirection = 1;
			obj->anim = getAnimation(kLeftNetPlayAnims[_vm->getRandom(3)]);
		} else {
			obj->x = 216;
			obj->netPlayDirection = 0;
			obj->anim = getAnimation(kRightNetPlayAnims[_vm->getRandom(3)]);
		}
		obj->frameIndex = 1;
		obj->ticks = obj->anim->frameTicks[1];
		obj->status = 0;
		obj->blinkCtr = 1;
		_netPlayerDelay = _vm->getRandom(128) + 250 - _playerDecrease;
		if (_vm->getRandom(10) == 1 && !isAnySoundPlaying(kAllSounds, 11))
			playSound(kYuppieChargeSounds[_vm->getRandom(2)]);
	}

}

void MinigameBbTennis::updateTennisBall(int objIndex) {
	Obj *obj = &_objects[objIndex];

	if (--obj->ticks == 0) {
		++obj->frameIndex;
		if (obj->frameIndex == 7) {
			obj->kind = 0;
			--_numBalls;
			if (_hitMissRatio > 0) {
				if (--_hitMissRatio == 0 && _vm->getRandom(8) == 1 && !isAnySoundPlaying(kAllSounds, 11))
					playSound(3);
			} else {
				if (_vm->getRandom(10) == 1 && !isAnySoundPlaying(kAllSounds, 11))
					playSound(3);
			}
			return;
		}
		obj->ticks = getAnimation(6)->frameTicks[obj->frameIndex];
	}

	if (--obj->ballStep == 0) {
		obj->ballStep = 12;
		++obj->ballStepCtr;
		if (obj->ballStepCtr == 1) {
			obj->fltStepX = ((obj->fltX - (float)obj->targetX) * 0.75f) / 12.0f;
			obj->fltStepY = ((obj->fltY - (float)obj->targetY) * 0.75f) / 12.0f;
		} else if (obj->ballStepCtr == 2) {
			obj->fltStepX = (obj->fltX - (float)obj->targetX) / 12.0f;
			obj->fltStepY = (obj->fltY - (float)obj->targetY) / 12.0f;
		} else {
			obj->fltStepX = 0.0f;
			obj->fltStepY = 0.0f;
		}
	}

	obj->fltX = obj->fltX - obj->fltStepX;
	obj->x = (int)obj->fltX;
	obj->fltY = obj->fltY - obj->fltStepY;
	obj->y = (int)obj->fltY;

}

void MinigameBbTennis::updateSquirrel(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->status) {

		case 0:
			--obj->ticks;
			if (--obj->ticks == 0) {
				if (++obj->frameIndex == 4) {
					obj->anim = getAnimation(0);
					obj->frameIndex = 0;
					obj->ticks = getAnimation(0)->frameTicks[0];
					obj->y += 2;
					++obj->status;
				} else {
					obj->ticks = getAnimation(1)->frameTicks[obj->frameIndex];
					++_squirrelDelay;
				}
			} else {
				++_squirrelDelay;
			}
			break;

		case 1:
			if (--obj->ticks == 0) {
				++obj->frameIndex;
				if (obj->frameIndex == 4)
					obj->frameIndex = 0;
				obj->ticks = getAnimation(0)->frameTicks[obj->frameIndex];
			}
			++obj->x;
			if (obj->x < 230) {
				if (--obj->blinkCtr <= 0) {
					obj->anim = getAnimation(4);
					obj->frameIndex = 0;
					obj->ticks = getAnimation(4)->frameTicks[obj->frameIndex];
					obj->status = 3;
				}
				++_squirrelDelay;
			} else {
				obj->anim = getAnimation(2);
				obj->frameIndex = 0;
				obj->ticks = getAnimation(2)->frameTicks[0];
				obj->y -= 2;
				++obj->status;
			}
			break;

		case 2:
			if (--obj->ticks == 0) {
				if (++obj->frameIndex == 4) {
					obj->kind = 0;
				} else {
					obj->ticks = getAnimation(2)->frameTicks[0];
					++_squirrelDelay;
				}
			} else {
				++_squirrelDelay;
			}
			break;

		case 3:
			if (--obj->ticks) {
				if (++obj->frameIndex == 2) {
					obj->anim = getAnimation(0);
					obj->frameIndex = 0;
					obj->ticks = getAnimation(0)->frameTicks[0];
					obj->status = 1;
					obj->blinkCtr = _vm->getRandom(128) + 10;
				} else {
					obj->ticks = getAnimation(4)->frameTicks[obj->frameIndex];
					++_squirrelDelay;
				}
			} else {
				++_squirrelDelay;
			}
			break;

		case 4:
			if (--obj->ticks == 0) {
				if (++obj->frameIndex == 5) {
					obj->kind = 0;
				} else {
					obj->ticks = getAnimation(3)->frameTicks[obj->frameIndex];
					++_squirrelDelay;
				}
			} else {
				++_squirrelDelay;
			}
			break;

		default:
			break;
	}

	if (obj->status != 4) {
		int tennisBallObjIndex = 0;
		Obj *tennisBallObj = findTennisBall(tennisBallObjIndex++);
		while (tennisBallObj) {
			if (tennisBallObj->frameIndex >= 6 && isHit(obj, tennisBallObj)) {
				hitSomething();
				tennisBallObj->kind = 0;
				--_numBalls;
				obj->status = 4;
				obj->anim = getAnimation(3);
				obj->frameIndex = 0;
				obj->ticks = getAnimation(3)->frameTicks[0];
				_rapidFireBallsCount = 50;
				_maxBalls = 10;
				_initBallTimer = 6;
				if (!isAnySoundPlaying(kAllSounds, 11))
					playSound(4);
				break;
			}
			tennisBallObj = findTennisBall(tennisBallObjIndex++);
		}
	}

}

void MinigameBbTennis::updateTennisPlayer(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->status) {

	case 0:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 6)
				obj->frameIndex = 0;
			obj->ticks = getAnimation(11)->frameTicks[0];
		}
		++obj->x;
		if (obj->x == 280)
			obj->kind = 0;
		--obj->blinkCtr;
		if (obj->blinkCtr <= 0) {
			obj->frameIndex = 6;
			obj->ticks = getAnimation(11)->frameTicks[6];
			++obj->status;
		}
		++_tennisPlayerDelay;
		break;

	case 1:
		if (--obj->ticks == 0) {
			if (++obj->frameIndex == 9) {
				if (obj->x < 210) {
					obj->frameIndex = 9;
					obj->status = 2;
				} else {
					obj->frameIndex = 15;
					obj->status = 3;
				}
				obj->blinkCtr = _vm->getRandom(64) + 40;
			}
			obj->ticks = getAnimation(11)->frameTicks[obj->frameIndex];
		}
		if ((obj->ticks % 2) && obj->frameIndex != 8) {
			++obj->x;
			if (obj->x == 280)
				obj->kind = 0;
		}
		++_tennisPlayerDelay;
		break;

	case 2:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 15)
				++obj->status;
			obj->ticks = getAnimation(11)->frameTicks[obj->frameIndex];
			if (obj->frameIndex == 13)
				makeEnemyBall(obj->x, obj->y - 31, 4);
		}
		++_tennisPlayerDelay;
		break;

	case 3:
		if (--obj->ticks == 0) {
			++obj->status;
			obj->frameIndex = 16;
			obj->ticks = getAnimation(11)->frameTicks[16];
		}
		if (obj->ticks % 2) {
			--obj->x;
			if (obj->x <= 40)
				obj->kind = 0;
		} else
			++_tennisPlayerDelay;
		break;

	case 4:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 22)
				obj->frameIndex = 16;
			obj->ticks = getAnimation(11)->frameTicks[obj->frameIndex];
		}
		--obj->x;
		if (obj->x > 40) {
			if (--obj->blinkCtr <= 0) {
				++obj->status;
				obj->frameIndex = 22;
				obj->ticks = getAnimation(11)->frameTicks[22];
			}
			++_tennisPlayerDelay;
		} else {
			obj->kind = 0;
		}
		break;

	case 5:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 25) {
				if (obj->x <= 70) {
					obj->frameIndex = 33;
					obj->status = 7;
				} else {
					obj->frameIndex = 25;
					obj->status = 6;
				}
				obj->blinkCtr = _vm->getRandom(64) + 40;
			}
			obj->ticks = getAnimation(11)->frameTicks[obj->frameIndex];
		}
		if ((obj->ticks % 2) && obj->frameIndex != 24) {
			--obj->x;
			if (obj->x <= 40)
				obj->kind = 0;
		} else
			++_tennisPlayerDelay;
		break;

	case 6:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 33)
				++obj->status;
			obj->ticks = getAnimation(11)->frameTicks[obj->frameIndex];
			if (obj->frameIndex == 31)
				makeEnemyBall(obj->x + 8, obj->y - 49, 4);
		}
		++_tennisPlayerDelay;
		break;

	case 7:
		if (--obj->ticks == 0) {
			obj->frameIndex = 0;
			obj->ticks = getAnimation(11)->frameTicks[0];
			obj->status = 0;
		}
		if (obj->ticks % 2) {
			++obj->x;
			if (obj->x == 280)
				obj->kind = 0;
		}
		++_tennisPlayerDelay;
		break;

	case 8:
		break;

	default:
		break;
	}

	if (obj->status != 8) {
		int tennisBallObjIndex = 0;
		Obj *tennisBallObj = findTennisBall(tennisBallObjIndex++);
		while (tennisBallObj) {
			if (tennisBallObj->frameIndex >= 6 && isHit(obj, tennisBallObj)) {
				hitSomething();
				tennisBallObj->kind = 0;
				--_numBalls;
				obj->status = 8;
				obj->blinkCtr = 20;
				playSound(kYuppieHitSounds[_vm->getRandom(6)]);
				break;
			}
			tennisBallObj = findTennisBall(tennisBallObjIndex++);
		}
	}

}

void MinigameBbTennis::updateThrower(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->status) {

	case 0:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 4)
				++obj->status;
			obj->ticks = getAnimation(12)->frameTicks[obj->frameIndex];
		}
		++_throwerDelay;
		break;

	case 1:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 8)
				++obj->status;
			obj->ticks = getAnimation(12)->frameTicks[obj->frameIndex];
			if (obj->frameIndex == 7)
				makeEnemyBall(obj->x - 10, obj->y - 10, 3);
		}
		++_throwerDelay;
		break;

	case 2:
		--obj->ticks;
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 12) {
				obj->kind = 0;
			} else {
				obj->ticks = getAnimation(12)->frameTicks[obj->frameIndex];
				++_throwerDelay;
			}
		} else {
			++_throwerDelay;
		}
		break;

	case 3:
		--obj->ticks;
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 14) {
				obj->kind = 0;
			} else {
				obj->ticks = getAnimation(12)->frameTicks[obj->frameIndex];
				++_throwerDelay;
			}
		} else {
			++_throwerDelay;
		}
		break;

	default:
		break;
	}

	if (obj->status != 3) {
		int tennisBallObjIndex = 0;
		Obj *tennisBallObj = findTennisBall(tennisBallObjIndex++);
		while (tennisBallObj) {
			if (tennisBallObj->frameIndex >= 5 && tennisBallObj->frameIndex <= 7 && isHit(obj, tennisBallObj)) {
				hitSomething();
				tennisBallObj->kind = 0;
				--_numBalls;
				obj->status = 3;
				obj->frameIndex = 12;
				obj->ticks = getAnimation(12)->frameTicks[12];
				playSound(kYuppieHitSounds[_vm->getRandom(6)]);
				break;
			}
			tennisBallObj = findTennisBall(tennisBallObjIndex++);
		}
	}

}

void MinigameBbTennis::updateNetPlayer(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->status) {

	case 0:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 15) {
				obj->blinkCtr = _vm->getRandom(32) + 10;
				++obj->status;
				obj->frameIndex = 31;
			} else {
				obj->ticks = obj->anim->frameTicks[obj->frameIndex];
				++_netPlayerDelay;
			}
		} else {
			++_netPlayerDelay;
		}
		break;

	case 1:
		if (--obj->blinkCtr <= 0) {
			++obj->status;
			obj->frameIndex = 15;
			obj->ticks = obj->anim->frameTicks[15];
			obj->x = _vm->getRandom(128) + 100;
		}
		++_netPlayerDelay;
		break;

	case 2:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 24) {
				++obj->status;
				obj->frameIndex = 28;
			}
			obj->ticks = obj->anim->frameTicks[obj->frameIndex];
			if (obj->frameIndex == 23)
				makeEnemyBall(obj->x - 8, obj->y - 40, 3);
		}
		++_netPlayerDelay;
		break;

	case 3:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 31) {
				obj->status = 1;
				obj->frameIndex = 31;
				obj->blinkCtr = _vm->getRandom(32) + 10;
			} else {
				obj->ticks = obj->anim->frameTicks[obj->frameIndex];
				++_netPlayerDelay;
			}
		} else {
			++_netPlayerDelay;
		}
		break;

	case 4:
		break;

	case 5:
		if (--obj->ticks == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 27)
				obj->kind = 0;
			obj->ticks = obj->anim->frameTicks[obj->frameIndex];
		}
		break;

	default:
		break;
	}

	if (obj->status < 4 && obj->frameIndex != 31) {
		int tennisBallObjIndex = 0;
		Obj *tennisBallObj = findTennisBall(tennisBallObjIndex++);
		while (tennisBallObj) {
			if (obj->status == 0 && tennisBallObj->frameIndex >= 3 && tennisBallObj->frameIndex <= 6 &&
				isHit(obj, tennisBallObj)) {
				hitSomething();
				tennisBallObj->kind = 0;
				--_numBalls;
				if (obj->netPlayDirection) {
					obj->x += kLeftPlayerOffX[obj->frameIndex] + 10;
					obj->y += kLeftPlayerOffY[obj->frameIndex] + 10;
				} else {
					obj->x -= kLeftPlayerOffX[obj->frameIndex] + 12;
					obj->y += kLeftPlayerOffY[obj->frameIndex] + 10;
				}
				obj->status = 4;
				obj->frameIndex = 0;
				obj->blinkCtr = 20;
				playSound(kYuppieHitSounds[_vm->getRandom(6)]);
				break;
			} else if (obj->status > 1 && obj->status < 4 && tennisBallObj->frameIndex >= 3	&& tennisBallObj->frameIndex <= 4 &&
				isHit(obj, tennisBallObj)) {
				hitSomething();
				tennisBallObj->kind = 0;
				--_numBalls;
				obj->status = 5;
				obj->frameIndex = 24;
				obj->ticks = obj->anim->frameTicks[24];
				playSound(kYuppieHitSounds[_vm->getRandom(6)]);
				break;
			}
			tennisBallObj = findTennisBall(tennisBallObjIndex++);
		}
	}

}

void MinigameBbTennis::updateEnemyTennisBall(int objIndex) {
	Obj *obj = &_objects[objIndex];

	if (--obj->ticks == 0) {
		--obj->frameIndex;
		obj->ticks = getAnimation(6)->frameTicks[obj->frameIndex];
	}

	if (--obj->ballStep == 0) {
		obj->ballStep = 12;
		--obj->ballStepCtr;
		if (obj->ballStepCtr == 1) {
			obj->fltStepX = (obj->fltX - (float)obj->targetX) / 12.0f;
			obj->fltStepY = (obj->fltY - (float)obj->targetY) / 12.0f;
		} else if (obj->ballStepCtr == 2) {
			obj->fltStepX = ((obj->fltX - (float)obj->targetX) * 0.18f) / 12.0f;
			obj->fltStepY = ((obj->fltY - (float)obj->targetY) * 0.18f) / 12.0f;
		} else {
			obj->kind = 0;
			if (_numHearts > 0 && --_numHearts == 0)
				_allHeartsGone = true;
		}
	}

	obj->fltX = obj->fltX - obj->fltStepX;
	obj->x = (int)obj->fltX;
	obj->fltY = obj->fltY - obj->fltStepY;
	obj->y = (int)obj->fltY;

}

void MinigameBbTennis::makeEnemyBall(int x, int y, int frameIndex) {
	Obj *obj = getFreeObject();

	obj->kind = 7;
	obj->x = x;
	obj->y = y;
	obj->anim = getAnimation(6);
	obj->frameIndex = frameIndex;
	obj->ticks = getAnimation(6)->frameTicks[frameIndex];
	obj->targetX = 160;
	obj->targetY = 180;
	obj->fltX = (float)x;
	obj->fltY = (float)y;

	switch (frameIndex) {

	case 6:
		obj->ballStep = 18;
		obj->ballStepCtr = 3;
		obj->fltStepX = 0.0f;
		obj->fltStepY = 0.0f;
		break;

	case 5:
		obj->ballStep = 12;
		obj->ballStepCtr = 3;
		obj->fltStepX = ((float)(x - 160) * 0.07f) / 12.0f;
		obj->fltStepY = ((float)(y - 180) * 0.07f) / 12.0f;
		break;

	case 4:
		obj->ballStep = 6;
		obj->ballStepCtr = 3;
		obj->fltStepX = ((float)(x - 160) * 0.07f) / 6.0f;
		obj->fltStepY = ((float)(y - 180) * 0.07f) / 6.0f;
		break;

	case 3:
		obj->ballStep = 12;
		obj->ballStepCtr = 2;
		obj->fltStepX = ((float)(x - 160) * 0.18f) / 12.0f;
		obj->fltStepY = ((float)(y - 180) * 0.18f) / 12.0f;
		break;

	case 2:
		obj->ballStep = 6;
		obj->ballStepCtr = 2;
		obj->fltStepX = ((float)(x - 160) * 0.18f) / 6.0f;
		obj->fltStepY = ((float)(y - 180) * 0.18f) / 6.0f;
		break;

	case 1:
		obj->ballStep = 12;
		obj->ballStepCtr = 1;
		obj->fltStepX = (float)((x - 160) / 12);
		obj->fltStepY = (float)((y - 180) / 12);
		break;

	case 0:
		obj->ballStep = 6;
		obj->ballStepCtr = 1;
		obj->fltStepX = (float)((x - 160) / 6);
		obj->fltStepY = (float)((y - 180) / 6);
		break;

	default:
		break;
	}

}

void MinigameBbTennis::hitSomething() {
	if (_hitMissRatio < 15)
		_hitMissRatio += 3;
	++_score;
}

bool MinigameBbTennis::run(bool fromMainGame) {

	memset(_objects, 0, sizeof(_objects));

	_numbersAnim = getAnimation(20);

	_backgroundSpriteIndex = 272;
	_titleScreenSpriteIndex = 273;

	_fromMainGame = fromMainGame;

	_hiScore = 0;
	if (!_fromMainGame)
		_hiScore = loadHiscore(kMinigameBbTennis);

	_gameState = 0;
	_gameResult = false;
	_gameDone = false;
	initObjects();
	initVars();

	_spriteModule = new SpriteModule();
	_spriteModule->load("bbtennis/bbtennis.000");

	Palette palette = _spriteModule->getPalette();
	_vm->_screen->setPalette(palette);

	loadSounds();

	_gameTicks = 0;
	playSound(29, true);

	while (!_vm->shouldQuit() &&!_gameDone) {
		_vm->updateEvents();
		update();
	}

	_vm->_sound->unloadSounds();

	if (!_fromMainGame)
		saveHiscore(kMinigameBbTennis, _hiScore);

	delete _spriteModule;

	return _gameResult;
}

void MinigameBbTennis::update() {

	int inputTicks;

	if (_gameTicks > 0) {
		int currTicks = _vm->_system->getMillis();
		inputTicks = 3 * (currTicks - _gameTicks) / 50;
		_gameTicks = currTicks - (currTicks - _gameTicks - 50 * inputTicks / 3);
	} else {
		inputTicks = 1;
		_gameTicks = _vm->_system->getMillis();
	}

	if (_vm->_keyCode == Common::KEYCODE_ESCAPE) {
		_gameDone = true;
		return;
	}

	if (inputTicks == 0)
		return;

	bool done;

	do {
		done = !updateStatus(_vm->_mouseX, _vm->_mouseY, _vm->_mouseButtons);
		_vm->_mouseButtons &= ~kLeftButtonClicked;
		_vm->_mouseButtons &= ~kRightButtonClicked;
		_vm->_keyCode = Common::KEYCODE_INVALID;
	} while (--inputTicks && _gameTicks > 0 && !done);

	drawSprites();

	_vm->_system->delayMillis(10);

}

void MinigameBbTennis::loadSounds() {
	for (uint i = 0; i < kSoundFilenamesCount; ++i) {
		Common::String filename = Common::String::format("bbtennis/%s", kSoundFilenames[i]);
		_vm->_sound->loadSound(filename.c_str());
	}
}

} // End of namespace Bbvs
