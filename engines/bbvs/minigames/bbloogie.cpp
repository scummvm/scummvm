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

#include "bbvs/minigames/bbloogie.h"

#include "engines/advancedDetector.h"

namespace Bbvs {

static const int kLoogieOffY[16] = {
	0, 1, 1, 2, 2, 3, 3, 4,
	4, 5, 5, 6, 6, 7, 7, 0
};

static const int kSquirrelOffX[] = {
	-43, -43, -38, -33, -33, -27, -23, -23,
	-23, -23, -23, -23, -18, -14,  -8,  -4,
	  2,   8,  12,  18,  20,  20,  26,  31,
	 37,  37,  37,  37,  37,  37,  37,  32,
	 29,  26,  21,  14,  10,   6,   6,   6,
	  6,   6,   6,   6,   0,  -6, -15, -20,
	-27, -37, -41, -41, -41, -41
};

static const int kPlaneOffX[] = {
	0, -1, -1, -1, 0, 1, 1, 1
};

static const int kPlaneOffY[] = {
	-1, -1, 0, 1, 1, 1, 0, -1
};

static const int kLevelScores[] = {
	20, 50, 90, 140, 200, 270, 350, 440, 540, 10000
};

static const int kLevelTimes[] = {
	120, 110, 100, 90, 80, 70, 60, 50, 40, 30
};

static const uint kBeavisSounds1[] = {
	14, 15, 19, 20, 22, 23, 24, 26
};

static const uint kButtheadSounds1[] = {
	16, 14, 15, 22, 23
};

static const uint kBeavisSounds2[] = {
	9, 3, 4, 5, 7, 14, 15, 19, 20, 22, 23, 24, 26
};

static const uint kButtheadSounds2[] = {
	9, 3, 4, 5, 7, 16, 14, 15, 22, 23
};

static const uint kPrincipalSounds[] = {
	3, 4, 5, 7
};

static const char * const kSoundFilenames[] = {
	"loog1.aif", "loog2.aif", "loog3.aif", "loog4.aif", "loog5.aif",
	"loog6.aif", "loog7.aif", "loog8.aif", "loog9.aif", "loog10.aif",
	"loog11.aif", "loog12.aif", "loog13.aif", "loog14.aif", "loog15.aif",
	"loog16.aif", "loog17.aif", "loog18.aif", "loog19.aif", "loog20.aif",
	"loog21.aif", "loog22.aif", "loog23.aif", "loog24.aif", "loog25.aif",
	"loog26.aif", "loog27.aif", "meghoker.aif", "spit1.aif", "megaloog.aif",
	"megaspit.aif", "gamemuse.aif", "bing.aif", "carhit.aif", "bikehit.aif",
	"squirhit.aif", "planehit.aif", "bing2.aif"
};

static const uint kSoundFilenamesCount = ARRAYSIZE(kSoundFilenames);

static const char * const kDemoSoundFilenames[] = {
	"loog1.aif", "loog2.aif", "loog3.aif", "loog4.aif", "loog5.aif",
	"loog6.aif", "loog7.aif", "loog8.aif", "loog9.aif", "loog10.aif",
	"loog11.aif", "loog12.aif", "loog13.aif", "loog14.aif", "loog15.aif",
	"loog16.aif", "loog17.aif", "loog18.aif", "loog19.aif", "loog20.aif",
	"loog21.aif", "loog22.aif", "loog23.aif", "loog24.aif", "loog25.aif",
	"loog26.aif", "loog27.aif", "meghoker.aif", "spit1.aif", "megaloog.aif",
	"megaspit.aif", "rocktune.aif", "bing.aif"
};

static const uint kDemoSoundFilenamesCount = ARRAYSIZE(kDemoSoundFilenames);

void MinigameBbLoogie::buildDrawList(DrawList &drawList) {
	switch (_gameState) {
	case kGSTitleScreen:
		buildDrawList0(drawList);
		break;
	case kGSMainGame:
		buildDrawList1(drawList);
		break;
	case kGSStandaloneGame:
		buildDrawList2(drawList);
		break;
	case kGSScoreCountUp:
		buildDrawList3(drawList);
		break;
	default:
		break;
	}
}

void MinigameBbLoogie::buildDrawList0(DrawList &drawList) {
	drawList.add(_objects[0].anim->frameIndices[_objects[0].frameIndex], _objects[0].x, _objects[0].y, 2000);
	for (int i = 1; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind != 0)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->y + 16);
	}
	if (_titleScreenSpriteIndex)
		drawList.add(_titleScreenSpriteIndex, 0, 0, 0);
}

void MinigameBbLoogie::buildDrawList1(DrawList &drawList) {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		switch (obj->kind) {
		case 0:
			// Empty object
			break;
		case 2:
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, 400);
			break;
		case 3:
			drawList.add(obj->anim->frameIndices[obj->frameIndex + obj->frameIndexAdd], obj->x, obj->y, 1000);
			break;
		case 7:
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, 390);
			break;
		case 8:
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, 1000);
			break;
		default:
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->y + 16);
			break;
		}
	}

	if (_backgroundSpriteIndex)
		drawList.add(_backgroundSpriteIndex, 0, 0, 0);

	if (_fromMainGame) {
		drawList.add(getAnimation(8)->frameIndices[0], 8, 2, 2000);
		drawNumber(drawList, _numberOfHits, 56, 16);
	} else {
		drawList.add(getAnimation(10)->frameIndices[0], 230, 2, 2000);
		drawNumber(drawList, _levelTimeLeft, 280, 16);
		drawList.add(getAnimation(15)->frameIndices[0], 5, 2, 2000);
		int numberX2 = drawNumber(drawList, _currScore, 68, 16);
		drawList.add(getAnimation(9)->frameIndices[10], numberX2, 16, 2000);
		drawNumber(drawList, _dispLevelScore, numberX2 + 10, 16);
	}

	for (int i = 0; i < _megaLoogieCount; ++i)
		drawList.add(getAnimation(19)->frameIndices[0], 20 + i * 25, 236, 2000);

}

void MinigameBbLoogie::buildDrawList2(DrawList &drawList) {

	buildDrawList1(drawList);

	if (_level > 0 && (_bonusDisplayDelay1 > 0 || _bonusDisplayDelay2 > 0)) {
		drawList.add(getAnimation(12)->frameIndices[0], 100, 80, 2000);
		drawNumber(drawList, _timeBonusCtr, 212, 94);
	}

	if (_bonusDisplayDelay3 > 0) {
		drawList.add(getAnimation(14)->frameIndices[0], 65, 80, 2000);
		int numberX2 = drawNumber(drawList, _nextLevelScore, 170, 92);
		drawList.add(getAnimation(11)->frameIndices[0], numberX2, 80, 2000);
	}

}

void MinigameBbLoogie::buildDrawList3(DrawList &drawList) {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind == 2)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, 400);
		else
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->y + 16);
	}

	if (_backgroundSpriteIndex)
		drawList.add(_backgroundSpriteIndex, 0, 0, 0);

	drawList.add(getAnimation(10)->frameIndices[0], 230, 2, 2000);

	drawNumber(drawList, _levelTimeLeft, 280, 16);

	drawList.add(getAnimation(15)->frameIndices[0], 5, 2, 2000);

	int numberX2 = drawNumber(drawList, _currScore, 68, 16);
	drawList.add(getAnimation(9)->frameIndices[10], numberX2, 16, 2000);
	drawNumber(drawList, _dispLevelScore, numberX2 + 10, 16);

	drawList.add(getAnimation(20)->frameIndices[0], 120, 70, 2000);
	drawList.add(getAnimation(13)->frameIndices[0], 95, 95, 2000);

	drawNumber(drawList, _hiScore, 210, 109);

}

void MinigameBbLoogie::drawSprites() {
	DrawList drawList;
	buildDrawList(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

void MinigameBbLoogie::initObjs() {
	for (int i = 0; i < kMaxObjectsCount; ++i)
		_objects[i].kind = 0;
}

MinigameBbLoogie::Obj *MinigameBbLoogie::getFreeObject() {
	for (int i = 0; i < kMaxObjectsCount; ++i)
		if (_objects[i].kind == 0)
			return &_objects[i];
	return 0;
}

MinigameBbLoogie::Obj *MinigameBbLoogie::findLoogieObj(int startObjIndex) {
	for (int i = startObjIndex; i < kMaxObjectsCount; ++i)
		if (_objects[i].kind == 3)
			return &_objects[i];
	return 0;
}

bool MinigameBbLoogie::isHit(Obj *obj1, Obj *obj2) {
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

bool MinigameBbLoogie::isCursorAtObj(int objIndex) {
	return isHit(&_objects[0], &_objects[objIndex]);
}

void MinigameBbLoogie::initObjects() {
	switch (_gameState) {
	case kGSTitleScreen:
		initObjects0();
		break;
	case kGSMainGame:
		initObjects1();
		break;
	case kGSStandaloneGame:
		// Nothing
		break;
	case kGSScoreCountUp:
		initObjects3();
		break;
	default:
		break;
	}
}

void MinigameBbLoogie::initObjects0() {
	initObjs();
	_objects[0].anim = getAnimation(25);
	_objects[0].frameIndex = 0;
	_objects[0].ticks = getAnimation(25)->frameTicks[0];
	_objects[0].x = 160;
	_objects[0].y = 120;
	_objects[0].kind = 1;
	_objects[1].anim = getAnimation(21);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = getAnimation(21)->frameTicks[0];
	_objects[1].x = 40;
	_objects[1].y = 240;
	_objects[1].kind = 1;
	_objects[2].anim = getAnimation(23);
	_objects[2].frameIndex = 0;
	_objects[2].ticks = getAnimation(23)->frameTicks[0];
	_objects[2].x = 280;
	_objects[2].y = 240;
	_objects[2].kind = 1;
	_objects[3].anim = getAnimation(22);
	_objects[3].frameIndex = 0;
	_objects[3].ticks = getAnimation(22)->frameTicks[0];
	_objects[3].x = 40;
	_objects[3].y = 240;
	_objects[3].kind = 0;
	_objects[4].anim = getAnimation(24);
	_objects[4].frameIndex = 0;
	_objects[4].ticks = getAnimation(24)->frameTicks[0];
	_objects[4].x = 280;
	_objects[4].y = 240;
	_objects[4].kind = 0;
}

void MinigameBbLoogie::initObjects1() {
	initObjs();
	_objects[0].anim = _playerAnim;
	_objects[0].frameIndex = 0;
	_objects[0].ticks = _playerAnim->frameTicks[0];
	_objects[0].status = 0;
	_objects[0].x = 160;
	_objects[0].y = 240;
	_objects[0].kind = 1;
	_objects[1].anim = getAnimation(4);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = getAnimation(4)->frameTicks[0];
	_objects[1].x = 248;
	_objects[1].y = 24;
	_objects[1].kind = 2;
}

void MinigameBbLoogie::initObjects3() {
	initObjs();
	_objects[0].anim = _playerAnim;
	_objects[0].frameIndex = 0;
	_objects[0].ticks = _playerAnim->frameTicks[0];
	_objects[0].status = 0;
	_objects[0].kind = 1;
	_objects[1].anim = getAnimation(4);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = getAnimation(4)->frameTicks[0];
	_objects[1].x = 248;
	_objects[1].y = 24;
	_objects[1].kind = 2;
}

void MinigameBbLoogie::initVars() {
	switch (_gameState) {
	case kGSTitleScreen:
		initVars0();
		break;
	case kGSMainGame:
		initVars1();
		break;
	case kGSStandaloneGame:
		initVars2();
		break;
	case kGSScoreCountUp:
		initVars3();
		break;
	default:
		break;
	}
}

void MinigameBbLoogie::initVars0() {
	_carDelay = 120;
	_bikeDelay = 250;
	_squirrelDelay = 40;
	_paperPlaneDelay = 400; // Uninitialized in the original
	_principalDelay = 1750;
	_levelTimeDelay = 58;
	_principalAngry = false;
	_squirrelDirection = false;
	_numberOfHits = 0;
	_megaLoogieCount = 0;
	_level = 0;
	_levelTimeLeft = 0;
	_currScore = 0;
	_dispLevelScore = 0;
}

void MinigameBbLoogie::initVars1() {
	_carDelay = 120;
	_bikeDelay = 250;
	_squirrelDelay = 40;
	_paperPlaneDelay = 400; // Uninitialized in the original
	_principalDelay = 1750;
	_squirrelDirection = false;
	_numberOfHits = 0;
	_megaLoogieCount = 0;
}

void MinigameBbLoogie::initVars2() {
	_timeBonusCtr = _levelTimeLeft;
	_levelTimeDelay = 58;
	_bonusDisplayDelay1 = 60;
	_levelTimeLeft = kLevelTimes[_level];
	_nextLevelScore = kLevelScores[_level] + _currScore;
	_bonusDisplayDelay2 = 0;
	_bonusDisplayDelay3 = 0;
}

void MinigameBbLoogie::initVars3() {
	if (_currScore > _hiScore)
		_hiScore = _currScore;
	if (_playerKind) {
		playSound(11);
	} else {
		playSound(21);
	}
}

bool MinigameBbLoogie::updateStatus(int mouseX, int mouseY, uint mouseButtons) {
	switch (_gameState) {
	case kGSTitleScreen:
		return updateStatus0(mouseX, mouseY, mouseButtons);
	case kGSMainGame:
		return updateStatus1(mouseX, mouseY, mouseButtons);
	case kGSStandaloneGame:
		return updateStatus2(mouseX, mouseY, mouseButtons);
	case kGSScoreCountUp:
		return updateStatus3(mouseX, mouseY, mouseButtons);
	default:
		break;
	}
	return false;
}

bool MinigameBbLoogie::updateStatus0(int mouseX, int mouseY, uint mouseButtons) {

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	if (_objects[1].kind != 0 && isCursorAtObj(1)) {
		_objects[0].frameIndex = 1;
		_objects[1].kind = 0;
		_objects[3].kind = 11;
		_objects[3].frameIndex = 0;
		_objects[3].ticks = _objects[3].anim->frameTicks[0];
	} else if (!isCursorAtObj(3)) {
		if (_objects[4].kind == 0)
			_objects[0].frameIndex = 0;
		_objects[3].kind = 0;
		_objects[1].kind = 1;
	}

	if (_objects[2].kind && isCursorAtObj(2)) {
		_objects[0].frameIndex = 1;
		_objects[2].kind = 0;
		_objects[4].kind = 11;
		_objects[4].frameIndex = 0;
		_objects[4].ticks = _objects[4].anim->frameTicks[0];
	} else if (!isCursorAtObj(4)) {
		if (_objects[3].kind == 0)
			_objects[0].frameIndex = 0;
		_objects[4].kind = 0;
		_objects[2].kind = 1;
	}

	if (_vm->_gameDescription->flags & ADGF_DEMO) {
		_objects[0].frameIndex = 0;
	}

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind == 11) {
			if (--obj->ticks == 0) {
				++obj->frameIndex;
				if (obj->frameIndex >= obj->anim->frameCount)
					obj->frameIndex = 0;
				obj->ticks = obj->anim->frameTicks[obj->frameIndex];
			}
		}
	}

	if ((mouseButtons & kLeftButtonDown) &&
		(_objects[3].kind != 0 || _objects[4].kind != 0)) {
		if (_objects[4].kind != 0) {
			// Beavis
			_playerKind = 0;
			_playerAnim = getAnimation(0);
			_playerSounds1 = kBeavisSounds1;
			_playerSounds1Count = 8;
			_playerSounds2 = kBeavisSounds2;
			_playerSounds2Count = 13;
			playSound(15);
			while (isSoundPlaying(15)) { }
		} else {
			// Butt-head
			_playerKind = 1;
			_playerAnim = getAnimation(1);
			_playerSounds1 = kButtheadSounds1;
			_playerSounds1Count = 5;
			_playerSounds2 = kButtheadSounds2;
			_playerSounds2Count = 10;
			playSound(23);
			while (isSoundPlaying(23)) { }
		}
		_gameState = _fromMainGame ? kGSMainGame : kGSStandaloneGame;
		initObjects1();
		initObjects();
		initVars();
		_gameTicks = 0;
	}

	return true;
}

bool MinigameBbLoogie::updateStatus1(int mouseX, int mouseY, uint mouseButtons) {

	if (--_levelTimeDelay == 0) {
		_levelTimeDelay = 58;
		--_levelTimeLeft;
	}

	if (!_fromMainGame && _levelTimeLeft == 0) {
		_gameState = kGSScoreCountUp;
		initObjects();
		initVars();
	} else if (_fromMainGame || _currScore < _nextLevelScore) {
		_objects->x = CLIP(mouseX, 0, 319);
		_objects->y = 240;
		if (!_principalAngry &&
			((mouseButtons & kLeftButtonDown) || ((mouseButtons & kRightButtonDown) && _megaLoogieCount)) &&
			_objects[0].status == 0 && mouseX != 32512 && mouseY != 32512) {
			_objects[0].ticks = _playerAnim->frameTicks[13];
			_objects[0].frameIndex = 14;
			_objects[0].status = 1;
			_objects[0].unk2 = 0;
			Obj *newObj = getFreeObject();
			newObj->anim = getAnimation(17);
			newObj->frameIndex = 0;
			newObj->ticks = 1;
			newObj->x = 0;
			newObj->y = 140;
			newObj->kind = 8;
			if (mouseButtons & kLeftButtonDown) {
				_doubleScore = 0;
				playSound(28);
			} else {
				_doubleScore = 17;
				playSound(30);
			}
		}
		updateObjs(mouseButtons);
	} else {
		_gameState = kGSStandaloneGame;
		++_level;
		initObjects();
		initVars();
	}
	return true;
}

bool MinigameBbLoogie::updateStatus2(int mouseX, int mouseY, uint mouseButtons) {

	_objects[0].x = mouseX;

	if (_bonusDisplayDelay1 > 0) {
		if (--_bonusDisplayDelay1 == 0) {
			_bonusDisplayDelay2 = 60;
			if (_timeBonusCtr)
				playSound(33, true);
		}
	} else if (_bonusDisplayDelay2 > 0) {
		if (--_bonusDisplayDelay2 == 0) {
			_bonusDisplayDelay3 = 150;
			if (!(_vm->_gameDescription->flags & ADGF_DEMO)) {
				playSound(38);
			}
		} else if (_timeBonusCtr > 0) {
			++_bonusDisplayDelay2;
			++_levelTimeLeft;
			if (--_timeBonusCtr == 0)
				stopSound(33);
		}
	} else if (_bonusDisplayDelay3 > 0) {
		if ((mouseButtons & kAnyButtonDown) || (--_bonusDisplayDelay3 == 0)) {
			_dispLevelScore = _nextLevelScore;
			_gameState = kGSMainGame;
			_gameTicks = 0;
		}
	}
	return true;
}

bool MinigameBbLoogie::updateStatus3(int mouseX, int mouseY, uint mouseButtons) {

	_objects[0].x = mouseX;

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

void MinigameBbLoogie::updateObjs(uint mouseButtons) {

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		switch (obj->kind) {
		case 1:
			updatePlayer(i, mouseButtons);
			break;
		case 2:
			updateObjKind2(i);
			break;
		case 3:
			updateLoogie(i);
			break;
		case 4:
			updateCar(i);
			break;
		case 5:
			updateBike(i);
			break;
		case 6:
			updateSquirrel(i);
			break;
		case 7:
			updatePaperPlane(i);
			break;
		case 8:
			updateIndicator(i);
			break;
		case 9:
			updatePrincipal(i);
			break;
		default:
			break;
		}
	}

	if (--_carDelay == 0) {
		// Car
		Obj *obj = getFreeObject();
		obj->anim = getAnimation(2);
		obj->kind = 4;
		obj->frameIndex = 0;
		obj->x = 379;
		obj->y = 22;
		obj->xIncr = -2;
		obj->yIncr = 0;
		_carDelay = _vm->getRandom(256) + 800;
	}

	if (--_bikeDelay == 0) {
		// Bike
		Obj *obj = getFreeObject();
		obj->kind = 5;
		obj->anim = getAnimation(3);
		obj->frameIndex = 0;
		obj->x = 360;
		obj->y = _vm->getRandom(32) + 82;
		obj->xIncr = -1;
		obj->yIncr = 0;
		_bikeDelay = _vm->getRandom(512) + 500;
	}

	if (--_squirrelDelay == 0) {
		// Squirrel
		Obj *obj = getFreeObject();
		obj->kind = 6;
		obj->anim = getAnimation(7);
		obj->frameIndex = !_squirrelDirection ? 0 : 29;
		obj->x = 160;
		obj->y = 36;
		obj->xIncr = 0;
		obj->yIncr = 0;
		_squirrelDirection = !_squirrelDirection;
		if (_vm->getRandom(5) == 1 && !isAnySoundPlaying(_playerSounds2, _playerSounds2Count))
			playSound(9);
		_squirrelDelay = _vm->getRandom(512) + 300;
	}

	if (--_paperPlaneDelay == 0) {
		// Paper plane
		Obj *obj = getFreeObject();
		obj->kind = 7;
		obj->anim = getAnimation(16);
		obj->frameIndex = 0;
		obj->x = 86;
		obj->y = 187;
		obj->xIncr = 0;
		obj->yIncr = -1;
		switch (_vm->getRandom(3)) {
		case 1:
			obj->frameIndex = 1;
			obj->xIncr = -1;
			break;
		case 2:
			obj->frameIndex = 7;
			obj->xIncr = 1;
			break;
		default:
			break;
		}
		_paperPlaneDelay = 400;
	}

	if (_principalDelay >= 0 && --_principalDelay == 0) {
		// Principal
		Obj *obj = getFreeObject();
		obj->kind = 9;
		obj->anim = getAnimation(18);
		obj->frameIndex = 11;
		obj->x = -20;
		obj->y = 130;
		obj->xIncr = 1;
		obj->yIncr = 0;
		obj->status = 0;
		obj->frameIndexAdd = 0;
		obj->unk2 = _vm->getRandom(256) + 100;
		_principalCtr = 0;
		_principalFirstFrameIndex = 11;
		_principalLastFrameIndex = 16;
	}

}

void MinigameBbLoogie::updatePlayer(int objIndex, uint mouseButtons) {

	Obj *obj = &_objects[0];

	switch (obj->status) {

	case 1:
		if (obj->ticks-- == 0) {
			if (obj->frameIndex != 15) {
				++obj->frameIndex;
				obj->ticks = _playerAnim->frameTicks[obj->frameIndex];
			}
		}
		if ((((mouseButtons & kLeftButtonDown) && _doubleScore == 0) ||
			((mouseButtons & kRightButtonDown) && _doubleScore == 17))
			&& obj->unk2 != 61) {
			++obj->unk2;
		} else {
			obj->status = 2;
			obj->frameIndex = 16;
			obj->ticks = _playerAnim->frameTicks[16];
			if (obj->unk2 >= 30) {
				obj->status = 3;
				obj->frameIndex = 21;
				obj->ticks = _playerAnim->frameTicks[21];
			}
			if (obj->unk2 < 30) {
				Obj *newObj = getFreeObject();
				newObj->kind = 3;
				newObj->anim = getAnimation(5);
				newObj->frameIndex = 0;
				newObj->ticks = getAnimation(5)->frameTicks[0];
				newObj->x = obj->x;
				newObj->y = 172;
				newObj->unk2 = obj->unk2;
				newObj->frameIndexAdd = _doubleScore;
				if (_doubleScore)
					--_megaLoogieCount;
			}
			if (_doubleScore) {
				stopSound(30);
				playSound(31);
			} else {
				stopSound(28);
				playSound(29);
			}
		}
		break;

	case 2:
		if (obj->ticks-- == 0) {
			if (obj->frameIndex == 17) {
				obj->frameIndex = 0;
				obj->status = 0;
			} else {
				++obj->frameIndex;
				obj->ticks = _playerAnim->frameTicks[obj->frameIndex];
			}
		}
		break;

	case 3:
		if (obj->ticks-- == 0) {
			if (obj->frameIndex == 23) {
				obj->frameIndex = 0;
				obj->status = 0;
			} else {
				++obj->frameIndex;
				obj->ticks = _playerAnim->frameTicks[obj->frameIndex];
				if (obj->frameIndex == 22) {
					Obj *newObj = getFreeObject();
					newObj->kind = 3;
					newObj->anim = getAnimation(5);
					newObj->frameIndex = 0;
					newObj->ticks = getAnimation(5)->frameTicks[0];
					newObj->x = obj->x;
					newObj->y = 154;
					newObj->unk2 = obj->unk2;
					newObj->frameIndexAdd = _doubleScore;
					if (_doubleScore)
						--_megaLoogieCount;
				}
			}
		}
		break;

	default:
		break;
	}

}

void MinigameBbLoogie::updateObjKind2(int objIndex) {

	Obj *obj = &_objects[objIndex];

	if (obj->ticks-- == 0) {
		obj->ticks = getAnimation(4)->frameTicks[0];
		if (obj->frameIndex > 7)
			obj->frameIndex = 1;
		if (obj->frameIndex++ >= 7)
			obj->frameIndex = 0;
	}

}

void MinigameBbLoogie::updateLoogie(int objIndex) {
	Obj *obj = &_objects[objIndex];

	if (obj->unk2 > 0) {
		obj->y -= kLoogieOffY[obj->unk2 / 8];
		--obj->unk2;
	}

	if (obj->ticks-- == 0) {
		obj->ticks = getAnimation(5)->frameTicks[0];
		++obj->frameIndex;
		if (obj->frameIndex >= 17) {
			obj->kind = 0;
			obj->anim = getAnimation(6);
			obj->frameIndex = 0;
		}
	}

}

void MinigameBbLoogie::updateCar(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->x += obj->xIncr;

	if (obj->ticks-- == 0) {
		if (obj->frameIndex++ == 3 || obj->frameIndex == 6)
			obj->frameIndex = 0;
		obj->ticks = getAnimation(2)->frameTicks[obj->frameIndex];
	}

	if (obj->x <= -60) {
		obj->kind = 0;
		obj->anim = getAnimation(6);
		obj->frameIndex = 0;
	} else if (!_principalAngry && obj->frameIndex <= 3) {
		int loogieObjIndex = 0;
		Obj *loogieObj = findLoogieObj(loogieObjIndex++);
		while (loogieObj) {
			if (loogieObj->frameIndex >= 8 && loogieObj->frameIndex <= 10 && isHit(obj, loogieObj)) {
				incNumberOfHits();
				incScore(7);
				loogieObj->frameIndex = 13;
				loogieObj->ticks = getAnimation(5)->frameTicks[12];
				obj->frameIndex = 4;
				obj->ticks = getAnimation(2)->frameTicks[4];
				if (!(_vm->_gameDescription->flags & ADGF_DEMO)) {
					playSound(34);
				}
				playRndSound();
			}
			loogieObj = findLoogieObj(loogieObjIndex++);
		}
	}

}

void MinigameBbLoogie::updateBike(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->x += obj->xIncr;

	if (obj->ticks-- == 0) {
		if (obj->frameIndex++ == 3 || obj->frameIndex == 7)
			obj->frameIndex = 0;
		obj->ticks = getAnimation(3)->frameTicks[obj->frameIndex];
	}

	if (obj->x == -40) {
		obj->kind = 0;
		obj->anim = getAnimation(6);
		obj->frameIndex = 0;
	} else if (!_principalAngry && obj->frameIndex <= 3) {
		int loogieObjIndex = 0;
		Obj *loogieObj = findLoogieObj(loogieObjIndex++);
		while (loogieObj) {
			if (loogieObj->frameIndex >= 7 && loogieObj->frameIndex <= 11 && isHit(obj, loogieObj)) {
				incNumberOfHits();
				incScore(2);
				loogieObj->frameIndex = 13;
				loogieObj->ticks = getAnimation(5)->frameTicks[12];
				obj->frameIndex = 4;
				obj->ticks = getAnimation(3)->frameTicks[4];
				if (!(_vm->_gameDescription->flags & ADGF_DEMO)) {
					playSound(35);
				}
				playRndSound();
			}
			loogieObj = findLoogieObj(loogieObjIndex++);
		}
	}

}

void MinigameBbLoogie::updateSquirrel(int objIndex) {
	Obj *obj = &_objects[objIndex];

	if (obj->ticks-- == 0) {
		++obj->frameIndex;
		if (obj->frameIndex == 29 || obj->frameIndex == 54 ||
			obj->frameIndex == 58 || obj->frameIndex == 62) {
			obj->kind = 0;
			obj->anim = getAnimation(6);
			obj->frameIndex = 0;
		}
		obj->ticks = getAnimation(7)->frameTicks[obj->frameIndex];
	}

	if (!_principalAngry && obj->frameIndex <= 53) {
		int loogieObjIndex = 0;
		Obj *loogieObj = findLoogieObj(loogieObjIndex++);
		while (loogieObj) {
			if (loogieObj->frameIndex >= 7 && loogieObj->frameIndex <= 9 && isHit(obj, loogieObj)) {
				incNumberOfHits();
				incScore(10);
				loogieObj->frameIndex = 13;
				loogieObj->ticks = getAnimation(5)->frameTicks[12];
				obj->x += kSquirrelOffX[obj->frameIndex];
				obj->frameIndex = obj->frameIndex < 29 ? 54 : 58;
				obj->ticks = getAnimation(7)->frameTicks[obj->frameIndex];
				if (!(_vm->_gameDescription->flags & ADGF_DEMO)) {
					playSound(36);
				}
				playRndSound();
			}
			loogieObj = findLoogieObj(loogieObjIndex++);
		}
  	}

}

void MinigameBbLoogie::updatePaperPlane(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->x += obj->xIncr;
	obj->y += obj->yIncr;

	if (obj->x == -16 || obj->x == 336 || obj->y == -16) {
		obj->kind = 0;
		obj->anim = getAnimation(6);
		obj->frameIndex = 0;
	}

	if (!_principalAngry && obj->frameIndex <= 53) {
		int loogieObjIndex = 0;
		Obj *loogieObj = findLoogieObj(loogieObjIndex++);
		while (loogieObj) {
			if (loogieObj->frameIndex >= 4 && loogieObj->frameIndex <= 7 && isHit(obj, loogieObj)) {
				incNumberOfHits();
				incScore(5);
				loogieObj->frameIndex = 13;
				loogieObj->ticks = getAnimation(5)->frameTicks[12];
				obj->frameIndex = (obj->frameIndex + 1) % 8;
				obj->xIncr = kPlaneOffX[obj->frameIndex];
				obj->yIncr = kPlaneOffY[obj->frameIndex];
				if (!(_vm->_gameDescription->flags & ADGF_DEMO)) {
					playSound(37);
				}
				playRndSound();
			}
			loogieObj = findLoogieObj(loogieObjIndex++);
		}
	}

}

void MinigameBbLoogie::updateIndicator(int objIndex) {
	Obj *obj = &_objects[objIndex];
	Obj *loogieObj = &_objects[0];

	if (obj->ticks-- == 0) {
		obj->frameIndex = (obj->frameIndex + 1) % 2;
		obj->ticks = getAnimation(17)->frameTicks[0];
	}

	if (loogieObj->status != 0) {
		int unk2mod = loogieObj->unk2 / 8;
		int unk2div = loogieObj->unk2 / 8 * 8;
		int v6 = 0;
		if (unk2div > 8) {
			int v7 = 1;
			do {
				v6 += 8 * kLoogieOffY[v7++];
			} while (v7 != unk2mod);
		}
		int yOfs = (loogieObj->unk2 % 8 + 1) * kLoogieOffY[loogieObj->unk2 / 8] + v6;
		if (loogieObj->unk2 >= 30)
			yOfs += 18;
		obj->y = 140 - yOfs;
	} else {
		obj->kind = 0;
		obj->anim = getAnimation(6);
	}

}

void MinigameBbLoogie::updatePrincipal(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->status) {

	case 0:
		if (obj->unk2--) {
			if (obj->ticks-- == 0) {
				++obj->frameIndex;
				if (obj->frameIndex == _principalLastFrameIndex)
					obj->frameIndex = _principalFirstFrameIndex;
				obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
			}
			++_principalCtr;
			if (_principalCtr % 2 != 0) {
				obj->x += obj->xIncr;
				obj->y += obj->yIncr;
				if (obj->xIncr > 0 && obj->x == 340) {
					obj->xIncr = -1;
					_principalLastFrameIndex = 34;
					_principalFirstFrameIndex = 29;
					obj->frameIndex = 29;
					obj->status = 2;
					obj->ticks = _vm->getRandom(256) + 60;
				}
				if (obj->xIncr < 0 && obj->x == -20) {
					obj->xIncr = 1;
					_principalLastFrameIndex = 16;
					_principalFirstFrameIndex = 11;
					obj->frameIndex = 11;
					obj->status = 2;
					obj->ticks = _vm->getRandom(256) + 60;
				}
			}
		} else {
			obj->unk2 = _vm->getRandom(64) + 20;
			++obj->status;
			if (_vm->getRandom(2) == 1) {
				obj->frameIndex = _principalFirstFrameIndex < 11 ? 17 : 26;
				_principalFirstFrameIndex = 19;
			} else {
				obj->frameIndex = _principalFirstFrameIndex < 11 ? 8 : 35;
				_principalFirstFrameIndex = 1;
			}
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		break;

	case 1:
		if (obj->unk2--) {
			if (obj->ticks-- == 0)
				obj->frameIndex = _principalFirstFrameIndex;
		} else {
			obj->unk2 = _vm->getRandom(256) + 100;
			++obj->status;
			if (_vm->getRandom(2) == 1) {
				_principalLastFrameIndex = 16;
				_principalFirstFrameIndex = 11;
				obj->frameIndex = obj->frameIndex < 1 ? 8 : 17;
				obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
				obj->xIncr = 1;
			} else {
				_principalLastFrameIndex = 34;
				_principalFirstFrameIndex = 29;
				obj->frameIndex = obj->frameIndex < 1 ? 35 : 26;
				obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
				obj->xIncr = -1;
			}
		}
		break;

	case 2:
		if (obj->ticks-- == 0) {
			obj->status = 0;
			obj->frameIndex = _principalFirstFrameIndex;
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		break;

	case 3:
		if (obj->ticks-- == 0) {
			obj->status = _prevPrincipalStatus;
			obj->frameIndex = _principalFirstFrameIndex;
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		break;

	case 4:
		if (obj->ticks-- == 0) {
			switch (obj->frameIndex) {
			case 8:
				obj->frameIndex = 36;
				break;
			case 26:
				obj->frameIndex = 28;
				break;
			case 28:
				obj->frameIndex = 35;
				break;
			case 35:
				++obj->frameIndex;
				break;
			case 36:
				obj->status = 5;
				++obj->frameIndex;
				break;
			default:
				break;
			}
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		break;

	case 5:
		if (obj->ticks-- == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == 48)
				obj->frameIndex = 36;
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		if (!isSoundPlaying(1)) {
			_gameResult = true;
			if (_fromMainGame) {
				_principalAngry = true;
				if (obj->x <= 140 || obj->x >= 165) {
					obj->status = 6;
					if (obj->x >= 160) {
						_principalLastFrameIndex = 34;
						_principalFirstFrameIndex = 29;
						obj->frameIndex = 29;
						obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
						obj->xIncr = -1;
					} else {
						_principalLastFrameIndex = 16;
						_principalFirstFrameIndex = 11;
						obj->frameIndex = 11;
						obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
						obj->xIncr = 1;
					}
				} else {
					obj->status = 7;
					_principalFirstFrameIndex = 2;
					_principalLastFrameIndex = 7;
					obj->frameIndex = 2;
					obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
					obj->xIncr = 0;
					obj->yIncr = 1;
				}
			} else {
				obj->status = _prevPrincipalStatus;
				obj->frameIndex = _principalFirstFrameIndex;
				obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
			}
		}
		break;

	case 6:
		obj->x += obj->xIncr;
		obj->y += obj->yIncr;
		if (obj->ticks-- == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == _principalLastFrameIndex)
				obj->frameIndex = _principalFirstFrameIndex;
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		if (obj->x > 145 && obj->x < 160) {
			obj->status = 7;
			_principalFirstFrameIndex = 2;
			_principalLastFrameIndex = 7;
			obj->frameIndex = 2;
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
			obj->xIncr = 0;
			obj->yIncr = 1;
		}
		break;

	case 7:
		obj->x += obj->xIncr;
		obj->y += obj->yIncr;
		if (obj->ticks-- == 0) {
			++obj->frameIndex;
			if (obj->frameIndex == _principalLastFrameIndex)
				obj->frameIndex = _principalFirstFrameIndex;
			obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
		}
		if (obj->y > 175) {
			// Angry principal enters school, end the minigame
			_gameDone = true;
		}
		break;

	default:
		break;
	}

	if (!_principalAngry) {
		int loogieObjIndex = 0;
		Obj *loogieObj = findLoogieObj(loogieObjIndex++);
		while (loogieObj) {
			if (loogieObj->frameIndex >= 7 && loogieObj->frameIndex <= 12 && isHit(obj, loogieObj)) {
				incNumberOfHits();
				incScore(1);
				loogieObj->frameIndex = 13;
				loogieObj->ticks = getAnimation(5)->frameTicks[12];
				if (obj->status != 3 && obj->status != 4 && obj->status != 5) {
					_prevPrincipalStatus = obj->status;
					obj->status = 3;
					if (_principalFirstFrameIndex == 1 || _principalFirstFrameIndex == 19)
						obj->frameIndex = _principalFirstFrameIndex - 1;
					else
						obj->frameIndex = _principalFirstFrameIndex - 2;
					obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
					if (loogieObj->frameIndexAdd > 0) {
						obj->status = 4;
						switch (obj->frameIndex) {
						case 0:
							obj->frameIndex = 36;
							break;
						case 9:
							obj->frameIndex = 8;
							break;
						case 27:
							obj->frameIndex = 35;
							break;
						case 18:
							obj->frameIndex = 26;
							break;
						default:
							break;
						}
						obj->ticks = getAnimation(18)->frameTicks[obj->frameIndex];
						playSound(1);
					} else {
						if (!isAnySoundPlaying(_playerSounds2, _playerSounds2Count))
							playSound(kPrincipalSounds[_vm->getRandom(4)]);
						playRndSound();
					}
				}
			}
			loogieObj = findLoogieObj(loogieObjIndex++);
		}
	}

}

void MinigameBbLoogie::incNumberOfHits() {
	++_numberOfHits;
	if (_numberOfHits == 1000)
		_numberOfHits = 0;
	if (_numberOfHits % 10 == 0) {
		++_megaLoogieCount;
		if (_megaLoogieCount > 11)
			_megaLoogieCount = 11;
	}
}

void MinigameBbLoogie::incScore(int incrAmount) {
	if (_doubleScore)
		_currScore += 2 * incrAmount;
	else
		_currScore += incrAmount;
}

void MinigameBbLoogie::playRndSound() {
	if (!isAnySoundPlaying(_playerSounds2, _playerSounds2Count))
		playSound(_playerSounds1[_vm->getRandom(_playerSounds1Count)]);
}

bool MinigameBbLoogie::run(bool fromMainGame) {

	if (!_vm->isLoogieDemo()) {
		Common::strlcpy(_prefix, "bbloogie/", 20);
	} else {
		_prefix[0] = 0;
	}

	memset(_objects, 0, sizeof(_objects));

	_numbersAnim = getAnimation(9);

	if (_vm->_gameDescription->flags & ADGF_DEMO) {
		_backgroundSpriteIndex = 209;
		_titleScreenSpriteIndex = 210;
	} else {
		_backgroundSpriteIndex = 210;
		_titleScreenSpriteIndex = 211;
	}

	_fromMainGame = fromMainGame;

	_hiScore = 0;
	if (!_fromMainGame)
		_hiScore = loadHiscore(kMinigameBbLoogie);

	_gameState = kGSTitleScreen;
	_gameTicks = 0;
	_gameResult = false;
	_gameDone = false;
	initObjects();
	initVars();

	_spriteModule = new SpriteModule();
	_spriteModule->load(Common::String::format("%sbbloogie.000", _prefix).c_str());

	Palette palette = _spriteModule->getPalette();
	_vm->_screen->setPalette(palette);

	loadSounds();

	playSound(32, true);

	while (!_vm->shouldQuit() &&!_gameDone) {
		_vm->updateEvents();
		update();
	}

	_vm->_sound->unloadSounds();

	if (!_fromMainGame)
		saveHiscore(kMinigameBbLoogie, _hiScore);

	delete _spriteModule;

	return _gameResult;
}

void MinigameBbLoogie::update() {

	int inputTicks;

	if (_gameTicks > 0) {
		int currTicks = _vm->_system->getMillis();
		inputTicks = (currTicks - _gameTicks) / 17;
		_gameTicks = currTicks - (currTicks - _gameTicks) % 17;
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

void MinigameBbLoogie::loadSounds() {
	if (_vm->_gameDescription->flags & ADGF_DEMO) {
		for (uint i = 0; i < kDemoSoundFilenamesCount; ++i) {
			Common::String filename = Common::String::format("%s%s", _prefix, kDemoSoundFilenames[i]);
			_vm->_sound->loadSound(filename.c_str());
		}
	} else {
		for (uint i = 0; i < kSoundFilenamesCount; ++i) {
			Common::String filename = Common::String::format("%s%s", _prefix, kSoundFilenames[i]);
			_vm->_sound->loadSound(filename.c_str());
		}
	}
}

} // End of namespace Bbvs
