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

#include "bbvs/minigames/bbant.h"

namespace Bbvs {

static const BBPoint kPosIncrTbl1[] = {
	{0, -1}, {-1, -1}, {-1, 0}, {-1,  1},
	{ 0, 1}, { 1,  1}, { 1, 0}, { 1, -1}
};

static const BBPoint kPosIncrTbl2[] = {
	{0, -2}, {-2, -2}, {-2, 0}, {-2,  2},
	{ 0, 2}, { 2,  2}, { 2, 0}, { 2, -2}
};

static const int kScoreTbl[] = {
	0, 1, 1, 3, 2, 4
};

static const char * const kSoundFilenames[] = {
	"ant1.aif", "ant2.aif", "ant3.aif", "ant4.aif", "ant5.aif",
	"ant6.aif", "ant7.aif", "ant8.aif", "ant9.aif", "ant10.aif",
	"ant11.aif", "antmus1.aif", "fryant.aif", "stomp.aif", "bing.aif",
	"bvyell.aif"
};

static const uint kSoundFilenamesCount = ARRAYSIZE(kSoundFilenames);

static const uint kSoundTbl1[] = {
	2, 3, 4, 6
};

static const uint kSoundTbl2[] = {
	5, 7, 11
};

static const uint kSoundTbl3[] = {
	8, 10, 11
};

static const uint kSoundTbl4[] = {
	2, 3, 4, 6, 8, 10, 11, 5, 7, 16
};

void MinigameBbAnt::buildDrawList0(DrawList &drawList) {

	if (_titleScreenSpriteIndex)
		drawList.add(_titleScreenSpriteIndex, 0, 0, 0);

	for (int i = 0; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind)
			drawList.add(obj->anim->frameIndices[obj->frameIndex], obj->x, obj->y, obj->priority);
	}

}

void MinigameBbAnt::buildDrawList1(DrawList &drawList) {

	if (_backgroundSpriteIndex)
		drawList.add(_backgroundSpriteIndex, _stompX, _stompY, 0);

	for (int i = 1; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];
		if (obj->kind) {
			drawList.add(obj->anim->frameIndices[obj->frameIndex],
				_stompX + (obj->x / 65536), _stompY + (obj->y / 65536),
				obj->priority);
		}
	}

	drawList.add(getAnimation(164)->frameIndices[0], 5, 2, 2000);
	drawNumber(drawList, _score, 68, 16);
	drawList.add(getAnimation(166)->frameIndices[0], 230, 2, 2000);
	drawNumber(drawList, _levelTimeLeft, 280, 16);

	for (int i = 0; i < _stompCount; ++i)
		drawList.add(getAnimation(130)->frameIndices[0], 20 + i * 30, 230, 2000);

}

void MinigameBbAnt::buildDrawList2(DrawList &drawList) {
	buildDrawList1(drawList);
	drawList.add(getAnimation(168)->frameIndices[0], 40, 100, 2000);
	drawNumber(drawList, _counter1, 190, 112);
	drawNumber(drawList, _countdown5, 258, 112);
	drawList.add(getAnimation(169)->frameIndices[0], 120, 120, 2000);
	drawNumber(drawList, _counter4, 192, 132);
}

void MinigameBbAnt::buildDrawList3(DrawList &drawList) {
	buildDrawList1(drawList);
	drawList.add(getAnimation(163)->frameIndices[0], 120, 70, 2000);
	drawList.add(getAnimation(165)->frameIndices[0], 95, 95, 2000);
	drawNumber(drawList, _hiScore, 208, 107);
}

void MinigameBbAnt::drawMagnifyingGlass(DrawList &drawList) {
	scale2x(_objects[0].x - 28, _objects[0].y - 27);
	drawList.clear();
	drawList.add(_objects[0].anim->frameIndices[0], _objects[0].x, _objects[0].y, _objects[0].priority);
	drawList.add(_objects[0].anim->frameIndices[1], _objects[0].x, _objects[0].y, _objects[0].priority);
	drawList.add(_objects[0].anim->frameIndices[2], _objects[0].x, _objects[0].y, _objects[0].priority);
}

void MinigameBbAnt::drawSprites() {
	switch (_gameState) {
	case 0:
		drawSprites0();
		break;
	case 1:
		drawSprites1();
		break;
	case 2:
		drawSprites2();
		break;
	case 3:
		drawSprites3();
		break;
	default:
		break;
	}
}

void MinigameBbAnt::drawSprites0() {
	DrawList drawList;
	buildDrawList0(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

void MinigameBbAnt::drawSprites1() {
	DrawList drawList;
	buildDrawList1(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	drawMagnifyingGlass(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

void MinigameBbAnt::drawSprites2() {
	DrawList drawList;
	buildDrawList2(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	drawMagnifyingGlass(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

void MinigameBbAnt::drawSprites3() {
	DrawList drawList;
	buildDrawList3(drawList);
	_vm->_screen->drawDrawList(drawList, _spriteModule);
	_vm->_screen->copyToScreen();
}

MinigameBbAnt::Obj *MinigameBbAnt::getFreeObject() {
	for (int i = 12; i < kMaxObjectsCount; ++i)
		if (_objects[i].kind == 0)
			return &_objects[i];
	return 0;
}

void MinigameBbAnt::initObjects() {
	switch (_gameState) {
	case 0:
		initObjects0();
		break;
	case 1:
		initObjects1();
		break;
	case 2:
	case 3:
	default:
		// Nothing
		break;
	}
}

void MinigameBbAnt::initObjects0() {
	_objects[0].anim = getAnimation(172);
	_objects[0].frameIndex = 0;
	_objects[0].ticks = getAnimation(172)->frameTicks[0];
	_objects[0].x = 160;
	_objects[0].y = 120;
	_objects[0].priority = 2000;
	_objects[0].kind = 1;
	_objects[1].anim = getAnimation(170);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = getAnimation(170)->frameTicks[0];
	_objects[1].x = 40;
	_objects[1].y = 240;
	_objects[1].priority = 100;
	_objects[1].kind = 2;
	_objects[2].anim = getAnimation(171);
	_objects[2].frameIndex = 0;
	_objects[2].ticks = getAnimation(171)->frameTicks[0];
	_objects[2].x = 280;
	_objects[2].y = 240;
	_objects[2].priority = 100;
	_objects[2].kind = 2;
}

void MinigameBbAnt::initObjects1() {
	_objects[0].kind = 0;
	_objects[0].x = 160;
	_objects[0].y = 120;
	_objects[0].xIncr = 0;
	_objects[0].yIncr = 0;
	_objects[0].anim = getAnimation(159);
	_objects[0].frameIndex = 0;
	_objects[0].ticks = _objects[0].anim->frameTicks[0];
	_objects[0].priority = 1000;
	_objects[1].kind = 8;
	_objects[1].x = 0x1E0000;
	_objects[1].y = 0x280000;
	_objects[1].xIncr = 0;
	_objects[1].yIncr = 0;
	_objects[1].anim = getAnimation(160);
	_objects[1].frameIndex = 0;
	_objects[1].ticks = _objects[0].anim->frameTicks[0];
	_objects[1].priority = 900;
	_objects[1].smokeCtr = 0;
	_objects[1].hasSmoke = false;
	_objects[1].status = 0;
	_objects[2].kind = 8;
	_objects[2].x = 0x280000;
	_objects[2].y = 0x4B0000;
	_objects[2].xIncr = 0;
	_objects[2].yIncr = 0;
	_objects[2].anim = getAnimation(161);
	_objects[2].frameIndex = 0;
	_objects[2].ticks = _objects[0].anim->frameTicks[0];
	_objects[2].priority = 900;
	_objects[2].smokeCtr = 0;
	_objects[2].hasSmoke = false;
	_objects[2].status = 0;
	for (int i = 3; i < 12; ++i) {
		const ObjInit *objInit = getObjInit(i - 3);
		_objects[i].kind = 6;
		_objects[i].x = objInit->x * 65536;
		_objects[i].y = objInit->y * 65536;
		_objects[i].xIncr = 0;
		_objects[i].yIncr = 0;
		_objects[i].anim = objInit->anim1;
		_objects[i].frameIndex = 0;
		_objects[i].ticks = _objects[0].anim->frameTicks[0];
		_objects[i].priority = 600;
		_objects[i].status = 9;
		_objects[i].damageCtr = 0;

	}
}

void MinigameBbAnt::initVars() {
	switch (_gameState) {
	default:
	case 0:
		// Nothing
		break;
	case 1:
		initVars1();
		break;
	case 2:
		initVars2();
		break;
	case 3:
		initVars3();
		break;
	}
}

void MinigameBbAnt::initVars1() {
	_stompX = 0;
	_stompY = 0;
	_stompDelay1 = 0;
	_stompCount = 1;
	_stompCounter1 = 80;
	_stompCounter2 = 80;
	_totalBugsCount = 0;
	_hasLastStompObj = false;
	_counter1 = 9;
	_countdown10 = 140;
	_score = 0;
	_counter4 = 1;
	_gameTicks = 0;
	_skullBugCtr = 500;
	_levelTimeDelay = 58;
	_levelTimeLeft = 30;
	_bugsChanceByKind[0] = 0;
	_bugsChanceByKind[1] = 20;
	_bugsChanceByKind[2] = 20;
	_bugsChanceByKind[3] = 5;
	_bugsChanceByKind[4] = 7;
	_bugsCountByKind[0] = 0;
	_bugsCountByKind[1] = 0;
	_bugsCountByKind[2] = 0;
	_bugsCountByKind[3] = 0;
	_bugsCountByKind[4] = 0;
	_bugsCountByKind[5] = 0;
}

void MinigameBbAnt::initVars2() {
	_countdown4 = 0;
	_countdown3 = 0;
	_levelTimeDelay = 58;
	_countdown6 = 60;
	_countdown5 = 50 * _counter1;
}

void MinigameBbAnt::initVars3() {
	if (_score > _hiScore)
		_hiScore = _score;
	playSound(9);
}

bool MinigameBbAnt::updateStatus(int mouseX, int mouseY, uint mouseButtons) {
	switch (_gameState) {
	case 0:
		return updateStatus0(mouseX, mouseY, mouseButtons);
	case 1:
		return updateStatus1(mouseX, mouseY, mouseButtons);
	case 2:
		return updateStatus2(mouseX, mouseY, mouseButtons);
	case 3:
		return updateStatus3(mouseX, mouseY, mouseButtons);
	default:
		break;
	}
	return false;
}

bool MinigameBbAnt::updateStatus0(int mouseX, int mouseY, uint mouseButtons) {

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	if (_objects[0].x >= 320)
		_objects[0].x = 320 - 1;
	if (_objects[0].y >= 240)
		_objects[0].y = 240 - 1;
	if (_objects[0].x < 0)
		_objects[0].x = 0;
	if (_objects[0].y < 0)
		_objects[0].y = 0;

	if ((mouseButtons & kLeftButtonDown) || (mouseButtons & kRightButtonDown)) {
		_gameState = 1;
		initObjects();
		initVars();
		_gameTicks = 0;
		playSound(1);
	} else {
		for (int i = 0; i < kMaxObjectsCount; ++i) {
			Obj *obj = &_objects[i];
			if (obj->kind == 2) {
				if (--obj->ticks == 0) {
					++obj->frameIndex;
					if (obj->frameIndex >= obj->anim->frameCount)
						obj->frameIndex = 0;
					obj->ticks = obj->anim->frameTicks[0];
				}
			}
		}
	}

	return true;
}

bool MinigameBbAnt::updateStatus1(int mouseX, int mouseY, uint mouseButtons) {
	const int kMaxBugsCount = 52;

	--_levelTimeDelay;
	if (!_levelTimeDelay) {
		_levelTimeDelay = 58;
		--_levelTimeLeft;
	}

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	if (_objects[0].x >= 320)
		_objects[0].x = 320 - 1;
	if (_objects[0].y >= 240)
		_objects[0].y = 240 - 1;
	if (_objects[0].x < 0)
		_objects[0].x = 0;
	if (_objects[0].y < 0)
		_objects[0].y = 0;

	if (!_levelTimeLeft) {
		_gameState = 2;
		initVars();
		initObjects();
		_gameTicks = 0;
		return true;
	}

	if (_counter1 == 0) {
		_gameState = 3;
		initVars();
		initObjects();
		_gameTicks = 0;
		return true;
	}

	if ((mouseButtons & kRightButtonClicked) && (_stompCount > 0|| _hasLastStompObj) && !_objects[2].status) {
		if (_hasLastStompObj)
			removeStompObj(_lastStompObj);
		--_stompCount;
		_objects[2].status = 1;
	}

	if ((mouseButtons & kLeftButtonClicked) && _objects[2].status == 0 && isMagGlassAtBeavisLeg(2)) {
		if (_vm->getRandom(10) == 1 && !isAnySoundPlaying(kSoundTbl4, 10))
			playSound(16);
		insertSmokeObj(_objects[0].x * 65536, _objects[0].y * 65536);
	}

	if (_skullBugCtr > 0) {
		if (--_skullBugCtr == 0) {
			_skullBugCtr = _vm->getRandom(150) + 500;
			insertRandomBugObj(5);
		}
	}

	if (_stompCounter2 > 0)
		--_stompCounter2;

	if (_totalBugsCount < kMaxBugsCount && _vm->getRandom(_stompCounter2) == 0) {
		int testTbl[4];
		int maxKindCount = 0, objKind = 0;

		_stompCounter2 = _stompCounter1;

		for (int i = 0; i < 4; ++i)
			testTbl[i] = _vm->getRandom(_bugsChanceByKind[i] - _bugsCountByKind[i]);

		for (int i = 0; i < 4; ++i) {
			if (testTbl[i] >= maxKindCount) {
				maxKindCount = testTbl[i];
				objKind = i + 1;
			}
		}

		if (objKind)
			insertRandomBugObj(objKind);

	}

	updateObjs(mouseButtons);
	updateFootObj(2);

	if (--_countdown10 == 0) {
		_countdown10 = 140;
		if (_stompCounter1 > 20)
			--_stompCounter1;
	}

	return true;
}

bool MinigameBbAnt::updateStatus2(int mouseX, int mouseY, uint mouseButtons) {

	_objects[0].x = mouseX;
	_objects[0].y = mouseY;

	if (_objects[0].x >= 320)
		_objects[0].x = 320 - 1;
	if (_objects[0].y >= 240)
		_objects[0].y = 240 - 1;
	if (_objects[0].x < 0)
		_objects[0].x = 0;
	if (_objects[0].y < 0)
		_objects[0].y = 0;

	if (_countdown6 > 0) {
		if (--_countdown6 == 0) {
			_countdown4 = 150;
			playSound(15, true);
		}
	} else if (_countdown4 > 0) {
		if (--_countdown4 == 0) {
			_countdown3 = 150;
		} else if (_countdown5 > 0) {
			++_countdown4;
			++_score;
			if (--_countdown5 == 0) {
				stopSound(15);
				_bugsChanceByKind[5] = 10;
				_countdown7 = 40;
				_countdown4 = 10 * (13 - _counter1);
				return true;
			}
		} else {
			if (--_countdown7 == 0) {
				bool flag1 = false;
				_countdown7 = _bugsChanceByKind[5];
				for (int i = 3; i < 12 && !flag1; ++i) {
					Obj *obj = &_objects[i];
					if (obj->status == 13) {
						const ObjInit *objInit = getObjInit(i - 3);
						obj->x = objInit->x * 65536;
						obj->y = objInit->y * 65536;
						obj->anim = objInit->anim3;
						obj->frameIndex = 0;
						obj->ticks = _objects[0].anim->frameTicks[0];
						obj->status = 9;
						obj->damageCtr = 0;
						obj->priority = 600;
						++_counter1;
						playSound(15);
						flag1 = true;
					}
				}
			}
		}
	} else if (_countdown3 > 0) {
		if ((mouseButtons & kLeftButtonDown) || (mouseButtons & kRightButtonDown) || (--_countdown3 == 0)) {
			_levelTimeDelay = 58;
			_levelTimeLeft = 30;
			_gameState = 1;
			_gameTicks = 0;
			++_counter4;
		}
	}

	return true;
}

bool MinigameBbAnt::updateStatus3(int mouseX, int mouseY, uint mouseButtons) {
	if (!isSoundPlaying(9) && _fromMainGame) {
		_vm->_system->delayMillis(1000);
		_gameDone = true;
	}
	return true;
}

void MinigameBbAnt::getRandomBugObjValues(int &x, int &y, int &animIndexIncr, int &field30) {
	field30 = _vm->getRandom(4);
	switch (field30) {
	case 0:
		y = -5;
		x = _vm->getRandom(190) + 120;
		animIndexIncr = 4;
		break;
	case 1:
		x = 325;
		y = _vm->getRandom(220) + 10;
		animIndexIncr = 2;
		break;
	case 2:
		y = 245;
		x = _vm->getRandom(300) + 10;
		animIndexIncr = 0;
		break;
	case 3:
		x = -5;
		y = _vm->getRandom(190) + 120;
		animIndexIncr = 6;
		break;
	default:
		break;
	}
}

void MinigameBbAnt::insertBugSmokeObj(int x, int y, int bugObjIndex) {
	Obj *obj = getFreeObject();
	if (obj) {
		Obj *bugObj = &_objects[bugObjIndex];
		bugObj->hasSmoke = true;
		obj->kind = 7;
		obj->x = x;
		obj->y = y;
		obj->priority = 950;
		if (bugObj->status >= 4 && (bugObj->status <= 6 || bugObj->status == 8)) {
			obj->xIncr = 0;
			obj->yIncr = (-1 * 65536);
		} else {
			obj->xIncr = bugObj->xIncr / 8;
			obj->yIncr = bugObj->yIncr / 8;
		}
		obj->anim = getAnimation(158);
		obj->frameIndex = 0;
		obj->ticks = obj->anim->frameTicks[0];
	}
}

void MinigameBbAnt::insertSmokeObj(int x, int y) {
	Obj *obj = getFreeObject();
	if (obj) {
		obj->kind = 7;
		obj->x = x;
		obj->y = y;
		obj->priority = 950;
		obj->xIncr = 0x2000;
		obj->yIncr = -0xC000;
		obj->anim = getAnimation(158);
		obj->frameIndex = 0;
		obj->ticks = obj->anim->frameTicks[0];
	}
}

void MinigameBbAnt::resetObj(int objIndex) {
	_objects[objIndex].kind = 0;
}

void MinigameBbAnt::insertStompObj(int x, int y) {
	Obj *obj = getFreeObject();
	if (obj) {
		obj->kind = 9;
		obj->x = x;
		obj->y = y;
		obj->priority = 2000;
		obj->xIncr = (0x1E0000 * _stompCount - x + 0x140000) / 15;
		obj->yIncr = (0xE60000 - y) / 15;
		obj->anim = getAnimation(130);
		obj->frameIndex = 0;
		obj->ticks = 15;
		_lastStompObj = obj;
		_hasLastStompObj = true;
	}
}

void MinigameBbAnt::removeStompObj(Obj *obj) {
	++_stompCount;
	_hasLastStompObj = false;
	obj->kind = 0;
}

void MinigameBbAnt::insertBugObj(int kind, int animIndexIncr, int always0, int x, int y, int field30, int always1) {
	Obj *obj = getFreeObject();
	if (obj) {
		const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(kind);
		obj->field30 = field30;
		obj->animIndexIncr = animIndexIncr;
		obj->kind = kind;
		obj->x = x * 65536;
		obj->y = y * 65536;
		obj->priority = 610;
		obj->xIncr = kPosIncrTbl1[0].x * 65536;
		obj->yIncr = kPosIncrTbl1[0].y * 65536;
		obj->anim = objKindAnimTable[0];
		obj->frameIndex = 0;
		obj->ticks = obj->anim->frameTicks[0];
		obj->animIndex = 0;
		obj->status = 1;
		obj->damageCtr = 0;
		obj->hasSmoke = false;
		obj->flag = 0;
		++_bugsCountByKind[kind];
		++_totalBugsCount;
	}
}

void MinigameBbAnt::removeBugObj(int objIndex) {
	Obj *obj = &_objects[objIndex];
	--_totalBugsCount;
	--_bugsCountByKind[obj->kind];
	obj->hasSmoke = false;
	obj->kind = 0;
}

void MinigameBbAnt::updateBugObjAnim(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->field30) {
	case 0:
		obj->animIndexIncr = 4;
		break;
	case 1:
		obj->animIndexIncr = 2;
		break;
	case 2:
		obj->animIndexIncr = 0;
		break;
	case 3:
		obj->animIndexIncr = 6;
		break;
	default:
		break;
	}
	const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(obj->kind);
	obj->xIncr = kPosIncrTbl1[obj->animIndexIncr].x * 65536;
	obj->yIncr = kPosIncrTbl1[obj->animIndexIncr].y * 65536;
	obj->anim = objKindAnimTable[obj->animIndexIncr];
	obj->frameIndex = 0;
	obj->ticks = obj->anim->frameTicks[0];
}

void MinigameBbAnt::updateObjAnim2(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->animIndexIncr += _vm->getRandom(3) - 1;
	if (obj->animIndexIncr < 0)
		obj->animIndexIncr = 7;
	if (obj->animIndexIncr > 7)
		obj->animIndexIncr = 0;
	obj->animIndexIncr += 4;
	if (obj->animIndexIncr >= 8)
		obj->animIndexIncr %= 8;
	const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(obj->kind);
	obj->xIncr = kPosIncrTbl1[obj->animIndex + obj->animIndexIncr].x * 65536;
	obj->yIncr = kPosIncrTbl1[obj->animIndex + obj->animIndexIncr].y * 65536;
	obj->anim = objKindAnimTable[obj->animIndex + obj->animIndexIncr];
	obj->frameIndex = 0;
	obj->ticks = obj->anim->frameTicks[0];
	obj->x += obj->xIncr;
	obj->y += obj->yIncr;
}

void MinigameBbAnt::insertRandomBugObj(int kind) {
	int x, y, animIndexIncr, field30;
	getRandomBugObjValues(x, y, animIndexIncr, field30);
	insertBugObj(kind, animIndexIncr, 0, x, y, field30, 1);
}

bool MinigameBbAnt::isBugOutOfScreen(int objIndex) {
	Obj *obj = &_objects[objIndex];

	return
		obj->x < (-10 * 65536) || obj->x > (330 * 65536) ||
		obj->y < (-10 * 65536) || obj->y > (250 * 65536);
}

void MinigameBbAnt::updateObjAnim3(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->animIndexIncr += _vm->getRandom(3) - 1;
	if (obj->animIndexIncr < 0)
		obj->animIndexIncr = 7;
	if (obj->animIndexIncr > 7)
		obj->animIndexIncr = 0;
	const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(obj->kind);
	obj->xIncr = kPosIncrTbl1[obj->animIndexIncr].x * 65536;
	obj->yIncr = kPosIncrTbl1[obj->animIndexIncr].y * 65536;
	obj->anim = objKindAnimTable[obj->animIndexIncr];
}

void MinigameBbAnt::updateBugObj1(int objIndex) {
	Obj *obj = &_objects[objIndex];
	bool flag1 = false;
	bool flag2 = false;

	if (--obj->ticks == 0) {
		++obj->frameIndex;
		if (obj->anim->frameCount == obj->frameIndex) {
			obj->frameIndex = 0;
			obj->ticks = obj->anim->frameTicks[0];
			flag1 = true;
		} else {
			obj->ticks = obj->anim->frameTicks[obj->frameIndex];
			flag2 = true;
		}
	}

	obj->x += obj->xIncr;
	obj->y += obj->yIncr;

	if (obj->status != 7) {
		if (obj->damageCtr <= 5) {
			obj->hasSmoke = false;
		} else if (!obj->hasSmoke) {
			obj->smokeCtr = 6;
			insertBugSmokeObj(obj->x, obj->y, objIndex);
		} else if (obj->damageCtr > 200 && obj->status != 4 && obj->status != 6) {
			_score += kScoreTbl[obj->kind];
			if (obj->status == 3) {
				_objects[obj->otherObjIndex].status = 9;
				_objects[obj->otherObjIndex].priority = 600;
				if (_vm->getRandom(3) == 1 && !isAnySoundPlaying(kSoundTbl4, 10))
					playSound(kSoundTbl3[_vm->getRandom(3)]);
			} else {
				if (_vm->getRandom(3) == 1 && !isAnySoundPlaying(kSoundTbl4, 10))
					playSound(kSoundTbl2[_vm->getRandom(3)]);
			}
			flag1 = false;
			const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(obj->kind);
			obj->hasSmoke = false;
			obj->status = 4;
			obj->xIncr = 0;
			obj->yIncr = 0;
			obj->anim = objKindAnimTable[16];
			obj->frameIndex = 0;
			obj->ticks = obj->anim->frameTicks[0];
			obj->priority = 605;
			if (obj->kind == 5) {
				// Skull Beetle
				if (_stompCount < 10)
					insertStompObj(obj->x, obj->y);
				obj->kind = 4;
				obj->anim = getObjAnim(70);
				obj->ticks = obj->anim->frameTicks[0];
			}
		} else if (--obj->smokeCtr == 0) {
			obj->smokeCtr = 6;
			insertBugSmokeObj(obj->x, obj->y, objIndex);
		}
	}

	switch (obj->status) {

	case 1:
		if (isBugOutOfScreen(objIndex))
			removeBugObj(objIndex);
		else if (flag1 && !obj->flag)
			updateObjAnim3(objIndex);
		break;

	case 3:
		// Bug carries candy
		_objects[obj->otherObjIndex].x = obj->x;
		_objects[obj->otherObjIndex].y = obj->y;
		if (isBugOutOfScreen(objIndex)) {
			_objects[obj->otherObjIndex].status = 13;
			_objects[obj->otherObjIndex].x = (500 * 65536);
			_objects[obj->otherObjIndex].y = (500 * 65536);
			removeBugObj(objIndex);
			--_counter1;
		}
		break;

	case 4:
		if (flag1) {
			const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(obj->kind);
			obj->status = 6;
			obj->xIncr = 0;
			obj->yIncr = 0;
			obj->anim = objKindAnimTable[17];
			obj->frameIndex = 0;
			obj->ticks = obj->anim->frameTicks[0];
		}
		break;

	case 6:
		if (flag1) {
			const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(obj->kind);
			obj->status = 7;
			obj->xIncr = kPosIncrTbl2[obj->animIndexIncr].x * 65536;
			obj->yIncr = kPosIncrTbl2[obj->animIndexIncr].y * 65536;
			obj->anim = objKindAnimTable[obj->animIndexIncr + 8];
			obj->frameIndex = 0;
			obj->ticks = obj->anim->frameTicks[0];
			obj->animIndex = 8;
			obj->priority = 610;
		}
		break;

	case 7:
		if (isBugOutOfScreen(objIndex))
			removeBugObj(objIndex);
		break;

	case 8:
		if (--obj->counter != 0) {
			if (flag2 && obj->frameIndex == 13) {
				obj->frameIndex = 4;
				obj->ticks = obj->anim->frameTicks[4];
			}
		} else {
			obj->status = obj->status2;
			obj->anim = obj->anim2;
			obj->frameIndex = obj->frameIndex2;
			obj->ticks = obj->ticks2;
			obj->xIncr = kPosIncrTbl1[obj->animIndex + obj->animIndexIncr].x * 65536;
			obj->yIncr = kPosIncrTbl1[obj->animIndex + obj->animIndexIncr].y * 65536;
			obj->priority = 610;
		}
		break;

	default:
		break;
	}

}

void MinigameBbAnt::updateObjKind2(int objIndex) {
	updateBugObj1(objIndex);
}

void MinigameBbAnt::updateObjKind3(int objIndex) {
	updateBugObj1(objIndex);
}

void MinigameBbAnt::updateObjKind4(int objIndex) {
	updateBugObj1(objIndex);
}

void MinigameBbAnt::updateObjKind5(int objIndex) {
	++_skullBugCtr;
	updateBugObj1(objIndex);
}

void MinigameBbAnt::updateStompObj(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->x += obj->xIncr;
	obj->y += obj->yIncr;
	if (--obj->ticks == 0)
		removeStompObj(obj);
}

void MinigameBbAnt::updateSmokeObj(int objIndex) {
	Obj *obj = &_objects[objIndex];

	obj->x += obj->xIncr;
	obj->y += obj->yIncr;

	if (--obj->ticks == 0) {
		++obj->frameIndex;
		if (obj->anim->frameCount == obj->frameIndex)
			resetObj(objIndex);
		else
			obj->ticks = obj->anim->frameTicks[obj->frameIndex];
	}
}

void MinigameBbAnt::updateFootObj(int objIndex) {
	Obj *obj = &_objects[objIndex];

	switch (obj->status) {

	case 1:
		obj->xIncr = -0x8000;
		obj->yIncr = (-4 * 65536);
		obj->status = 2;
		_stompCounter1 += 5;
		_stompCounter2 = 100;
		break;

	case 2:
		obj->x += obj->xIncr;
		obj->y += obj->yIncr;
		obj->yIncr += 0x2000;
		if (obj->y < (20 * 65536)) {
			obj->xIncr = 0x8000;
			obj->yIncr = (7 * 65536);
			obj->status = 3;
		}
		break;

	case 3:
		obj->x += obj->xIncr;
		obj->y += obj->yIncr;
		obj->yIncr += 0x2000;
		if (obj->y >= 0x4B0000) {
			obj->x = (40 * 65536);
			obj->y = (75 * 65536);
			obj->status = 4;
			_stompDelay1 = 6;
			_stompY = 0;
			playSound(14);
		}
		break;

	case 4:
		if (--_stompDelay1 == 0) {
			_gameTicks = 0;
			if (_stompDelay1 % 2)
				_stompY = _stompY < 1 ? -8 : 0;
		} else {
			obj->status = 0;
			_stompX = 0;
			_stompY = 0;
			// Stun all bugs
			for (int i = 12; i < kMaxObjectsCount; ++i) {
				Obj *bugObj = &_objects[i];
				if (bugObj->kind >= 1 && bugObj->kind <= 5) {
					bugObj->counter = _vm->getRandom(200) + 360;
					const ObjAnimation * const *objKindAnimTable = getObjKindAnimTable(bugObj->kind);
					if (bugObj->status == 8) {
						bugObj->hasSmoke = false;
						bugObj->xIncr = 0;
						bugObj->yIncr = 0;
						bugObj->status2 = 7;
						bugObj->anim2 = objKindAnimTable[bugObj->animIndexIncr + 8];
						bugObj->frameIndex2 = 0;
						bugObj->ticks2 = obj->anim->frameTicks[0];
						bugObj->anim = objKindAnimTable[17];
						bugObj->frameIndex = 0;
						bugObj->ticks = _vm->getRandom(4) + obj->anim->frameTicks[0];
						bugObj->animIndex = 8;
					} else {
						if (bugObj->status == 3) {
							bugObj->priority = 610;
							_objects[bugObj->otherObjIndex].status = 9;
							_objects[bugObj->otherObjIndex].priority = 600;
						}
						bugObj->hasSmoke = false;
						bugObj->xIncr = 0;
						bugObj->yIncr = 0;
						bugObj->status2 = 1;
						bugObj->anim2 = bugObj->anim;
						bugObj->frameIndex2 = bugObj->frameIndex;
						bugObj->ticks2 = bugObj->ticks;
						bugObj->anim = objKindAnimTable[17];
						bugObj->frameIndex = 0;
						bugObj->ticks = _vm->getRandom(4) + obj->anim->frameTicks[0];
					}
					bugObj->status = 8;
					bugObj->priority = 605;
				}
			}
		}
		break;

	default:
		break;
	}

}

bool MinigameBbAnt::isBugAtCandy(int objIndex, int &candyObjIndex) {
	Obj *obj = &_objects[objIndex];
	bool result = false;

	if (obj->kind >= 1 && obj->kind <= 4) {
		const BBRect &frameRect1 = obj->anim->frameRects[obj->frameIndex];
		const int obj1X1 = frameRect1.x + (obj->x / 65536);
		const int obj1Y1 = frameRect1.y + (obj->y / 65536);
		const int obj1X2 = obj1X1 + frameRect1.width;
		const int obj1Y2 = obj1Y1 + frameRect1.height;
		for (int i = 3; i < 12 && !result; ++i) {
			Obj *obj2 = &_objects[i];
			const BBRect &frameRect2 = obj->anim->frameRects[obj2->frameIndex]; // sic
			const int obj2X1 = (obj2->x / 65536) + frameRect2.x;
			const int obj2Y1 = (obj2->y / 65536) + frameRect2.y;
			const int obj2X2 = obj2X1 + frameRect2.width;
			const int obj2Y2 = obj2Y1 + frameRect2.height;
			if (obj2->status == 9 && obj1X1 <= obj2X2 && obj1X2 >= obj2X1 && obj1Y1 <= obj2Y2 && obj1Y2 >= obj2Y1) {
				result = true;
				candyObjIndex = i;
			}
		}
	}
	return result;
}

bool MinigameBbAnt::isMagGlassAtBug(int objIndex) {
	Obj *obj = &_objects[objIndex];
	Obj *obj0 = &_objects[0];
	bool result = false;

	if (obj->kind >= 1 && obj->kind <= 5) {
		const BBRect &frameRect1 = obj0->anim->frameRects[0];
		const int obj1X1 = obj0->x + frameRect1.x;
		const int obj1Y1 = obj0->y + frameRect1.y;
		const int obj1X2 = obj1X1 + frameRect1.width;
		const int obj1Y2 = obj1Y1 + frameRect1.height;
		const BBRect &frameRect2 = obj->anim->frameRects[obj->frameIndex];
		const int obj2X1 = (obj->x / 65536) + frameRect2.x;
		const int obj2Y1 = (obj->y / 65536) + frameRect2.y;
		const int obj2X2 = obj2X1 + frameRect2.width;
		const int obj2Y2 = obj2Y1 + frameRect2.height;
		if (obj2X2 >= obj1X1 && obj1X2 >= obj2X1 && obj1Y1 <= obj2Y2 && obj1Y2 >= obj2Y1)
			result = true;
	}
	return result;
}

bool MinigameBbAnt::isMagGlassAtBeavisLeg(int objIndex) {
	Obj *obj = &_objects[objIndex];
	Obj *magGlassObj = &_objects[0];
	bool result = false;

	const BBRect &frameRect1 = magGlassObj->anim->frameRects[0];
	const int obj1X1 = magGlassObj->x + frameRect1.x;
	const int obj1Y1 = magGlassObj->y + frameRect1.y;
	const int obj1X2 = obj1X1 + frameRect1.width;
	const int obj1Y2 = obj1Y1 + frameRect1.height;
	const BBRect &frameRect2 = obj->anim->frameRects[obj->frameIndex];
	const int obj2X1 = (obj->x / 65536) + frameRect2.x;
	const int obj2Y1 = (obj->y / 65536) + frameRect2.y;
	const int obj2X2 = obj2X1 + frameRect2.width;
	const int obj2Y2 = obj2Y1 + frameRect2.height;
	if (obj2X2 >= obj1X1 && obj1X2 >= obj2X1 && obj1Y1 <= obj2Y2 && obj1Y2 >= obj2Y1)
		result = true;
	return result;
}

bool MinigameBbAnt::testObj5(int objIndex) {
	Obj *obj = &_objects[objIndex];
	bool result = false;
	if (obj->kind >= 1 && obj->kind <= 5) {
		const int x = obj->x / 65536;
		const int y = obj->y / 65536;
		if (x < 0 || x >= 110 || y < 0 || y >= 110) {
			obj->flag = 0;
		} else if (!obj->flag) {
			obj->flag = 1;
			result = true;
		}
	}
	return result;
}

void MinigameBbAnt::updateObjs(uint mouseButtons) {

	for (int i = 12; i < kMaxObjectsCount; ++i) {
		Obj *obj = &_objects[i];

		if (obj->kind) {

			if ((mouseButtons & kLeftButtonClicked) && isMagGlassAtBug(i))
				obj->damageCtr += 100;

			if (obj->status == 1) {
				int candyObjIndex;
				if (isBugAtCandy(i, candyObjIndex)) {
					obj->status = 3;
					obj->otherObjIndex = candyObjIndex;
					_objects[candyObjIndex].otherObjIndex = i;
					_objects[candyObjIndex].status = 10;
					_objects[candyObjIndex].priority = 620;
					_objects[candyObjIndex].status = 11;
					_objects[candyObjIndex].anim = getObjInit(candyObjIndex - 3)->anim3;
					updateBugObjAnim(i);
					if (_vm->getRandom(3) == 1 && !isAnySoundPlaying(kSoundTbl4, 10))
						playSound(kSoundTbl1[_vm->getRandom(4)]);
				}
			}

			if (testObj5(i)) {
				updateObjAnim2(i);
			}

			if (obj->damageCtr) {
				--obj->damageCtr;
				if (!isSoundPlaying(13))
					playSound(13);
			}

			switch (obj->kind) {
			case 1:
				updateBugObj1(i);
				break;
			case 2:
				updateObjKind2(i);
				break;
			case 3:
				updateObjKind3(i);
				break;
			case 4:
				updateObjKind4(i);
				break;
			case 5:
				updateObjKind5(i);
				break;
			case 7:
				updateSmokeObj(i);
				break;
			case 9:
				updateStompObj(i);
				break;
			default:
				break;
			}

		}

	}

}

bool MinigameBbAnt::run(bool fromMainGame) {

	memset(_objects, 0, sizeof(_objects));

	_numbersAnim = getAnimation(167);

	_backgroundSpriteIndex = 303;
	_titleScreenSpriteIndex = 304;

	_fromMainGame = fromMainGame;

	_hiScore = 0;
	if (!_fromMainGame)
		_hiScore = loadHiscore(kMinigameBbAnt);

	_gameState = 0;
	_gameResult = false;
	_gameDone = false;
	initObjects();
	initVars();

	_spriteModule = new SpriteModule();
	_spriteModule->load("bbant/bbant.000");

	Palette palette = _spriteModule->getPalette();
	_vm->_screen->setPalette(palette);

	loadSounds();

	_gameTicks = 0;
	playSound(12, true);

	while (!_vm->shouldQuit() &&!_gameDone) {
		_vm->updateEvents();
		update();
	}

	_vm->_sound->unloadSounds();

	if (!_fromMainGame)
		saveHiscore(kMinigameBbAnt, _hiScore);

	delete _spriteModule;

	return _gameResult;
}

void MinigameBbAnt::update() {

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

void MinigameBbAnt::scale2x(int x, int y) {
	Graphics::Surface *surface = _vm->_screen->_surface;

	int srcX = x + 14, srcY = y + 14;
	int srcW = kScaleDim, srcH = kScaleDim;

	if (srcX < 0) {
		srcW += srcX;
		srcX = 0;
	}

	if (srcY < 0) {
		srcH += srcY;
		srcY = 0;
	}

	if (srcX + srcW >= 320)
		srcW = 320 - srcX - 1;

	if (srcY + srcH >= 240)
		srcH = 240 - srcY - 1;

	for (int yc = 0; yc < srcH; ++yc) {
		byte *src = (byte*)surface->getBasePtr(srcX, srcY + yc);
		memcpy(&_scaleBuf[yc * kScaleDim], src, srcW);
	}

	int dstX = x, dstY = y;
	int dstW = 2 * kScaleDim, dstH = 2 * kScaleDim;

	if (dstX < 0) {
		dstW += dstX;
		dstX = 0;
	}

	if (dstY < 0) {
		dstH += dstY;
		dstY = 0;
	}

	if (dstX + dstW >= 320)
		dstW = 320 - dstX - 1;

	if (dstY + dstH >= 240)
		dstH = 240 - dstY - 1;

	int w = MIN(srcW * 2, dstW), h = MIN(srcH * 2, dstH);

	for (int yc = 0; yc < h; ++yc) {
		byte *src = _scaleBuf + kScaleDim * (yc / 2);
		byte *dst = (byte*)surface->getBasePtr(dstX, dstY + yc);
		for (int xc = 0; xc < w; ++xc)
			dst[xc] = src[xc / 2];
	}

}

void MinigameBbAnt::loadSounds() {
	for (uint i = 0; i < kSoundFilenamesCount; ++i) {
		Common::String filename = Common::String::format("bbant/%s", kSoundFilenames[i]);
		_vm->_sound->loadSound(filename.c_str());
	}
}

} // End of namespace Bbvs
