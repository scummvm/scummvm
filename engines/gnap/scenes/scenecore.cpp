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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/scenecore.h"

#include "gnap/scenes/scene00.h"
#include "gnap/scenes/scene01.h"
#include "gnap/scenes/scene02.h"
#include "gnap/scenes/scene03.h"
#include "gnap/scenes/scene04.h"
#include "gnap/scenes/scene05.h"
#include "gnap/scenes/scene06.h"
#include "gnap/scenes/scene07.h"
#include "gnap/scenes/scene08.h"
#include "gnap/scenes/scene09.h"
#include "gnap/scenes/scene10.h"
#include "gnap/scenes/scene11.h"
#include "gnap/scenes/scene12.h"
#include "gnap/scenes/scene13.h"
#include "gnap/scenes/scene14.h"
#include "gnap/scenes/scene15.h"
#include "gnap/scenes/scene16.h"
#include "gnap/scenes/scene17.h"
#include "gnap/scenes/scene18.h"
#include "gnap/scenes/scene19.h"
#include "gnap/scenes/scene20.h"
#include "gnap/scenes/scene21.h"
#include "gnap/scenes/scene22.h"
#include "gnap/scenes/scene23.h"
#include "gnap/scenes/scene24.h"
#include "gnap/scenes/scene25.h"
#include "gnap/scenes/scene26.h"
#include "gnap/scenes/scene47.h"
#include "gnap/scenes/scene48.h"
#include "gnap/scenes/scene54.h"

namespace Gnap {

int GnapEngine::initSceneLogic() {
	int backgroundId = -1;

	switch (_currentSceneNum) {
	case 0:
		_scene = new Scene00(this);
		backgroundId = _scene->init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 1:
		_scene = new Scene01(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 2:
		_scene = new Scene02(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 3:
		_scene = new Scene03(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 4:
		_scene = new Scene04(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 5:
		_scene = new Scene05(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 6:
		_scene = new Scene06(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 7:
		_scene = new Scene07(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 8:
		_scene = new Scene08(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 9:
		_scene = new Scene09(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 10:
		_scene = new Scene10(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 11:
		_scene = new Scene11(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 12:
		_scene = new Scene12(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 13:
		_scene = new Scene13(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 14:
		_scene = new Scene14(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 15:
		_scene = new Scene15(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 16:
	case 47:
	case 48:
	case 54:
		backgroundId = -1;
		_gameSys->setScaleValues(0, 500, 1, 1000);
		break;
	case 17:
		_scene = new Scene17(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 18:
		_scene = new Scene18(this);
		backgroundId = _scene->init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		_scene->updateHotspots();
		break;
	case 19:
		_scene = new Scene19(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 20:
		_scene = new Scene20(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 21:
		_scene = new Scene21(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 22:
		_scene = new Scene22(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 23:
		_scene = new Scene23(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 24:
		_scene = new Scene24(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 136, 11, 10);
		break;
	case 25:
		_scene = new Scene25(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 26:
		_scene = new Scene26(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 27:
		backgroundId = scene27_init();
		scene27_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 28:
		backgroundId = scene28_init();
		scene28_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 29:
		backgroundId = scene29_init();
		scene29_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 30:
		backgroundId = scene30_init();
		scene30_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 31:
		backgroundId = scene31_init();
		scene31_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 32:
		backgroundId = scene32_init();
		scene32_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 33:
		backgroundId = scene33_init();
		scene33_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 34:
		_scene = new Scene03(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 35:
		_scene = new Scene05(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 36:
		_scene = new Scene06(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 37:
		_scene = new Scene04(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 38:
		backgroundId = scene38_init();
		scene38_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 39:
		backgroundId = scene39_init();
		scene39_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 40:
		backgroundId = scene40_init();
		scene40_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 41:
		backgroundId = scene41_init();
		scene41_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 42:
		backgroundId = scene42_init();
		scene42_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 43:
		backgroundId = scene43_init();
		scene43_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 44:
		backgroundId = scene44_init();
		scene44_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 45:
		backgroundId = scene45_init();
		scene45_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 46:
		backgroundId = scene46_init();
		scene46_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 49:
		backgroundId = scene49_init();
		scene49_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 50:
		backgroundId = scene50_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 51:
		backgroundId = scene51_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 52:
		backgroundId = scene52_init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 53:
		backgroundId = scene53_init();
		scene53_updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	}

	return backgroundId;
}

void GnapEngine::runSceneLogic() {
	switch (_currentSceneNum) {
	case 0:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 8;
		break;
	case 1:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 2:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 3:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 4:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 5:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 6:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 4;
		break;
	case 7:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 8;
		break;
	case 8:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 9;
		break;
	case 9:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 10;
		break;
	case 10:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 11:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 12;
		break;
	case 12:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 13:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 14:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 13;
		break;
	case 15:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 12;
		break;
	case 16:
		_scene = new Scene16(this);
		_scene->init();
		_newSceneNum = 17;
		_newCursorValue = 3;
		_scene->run();
		delete _scene;
		break;
	case 17:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 18:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 19:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 19;
		break;
	case 20:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 22;
		break;
	case 21:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 22:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 23:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 24:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 25:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 26:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 27:
		scene27_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 28:
		scene28_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 29:
		scene29_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 30:
		scene30_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 31:
		scene31_run();
		if (_newSceneNum == 55)
			_newSceneNum = 20;
		break;
	case 32:
		scene32_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 33:
		scene33_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 34:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 35:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 36:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 37:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 38:
		scene38_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 39:
		scene39_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 40:
		scene40_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 41:
		scene41_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 42:
		scene42_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 43:
		scene43_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 44:
		scene44_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 45:
		scene45_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 46:
		scene46_run();
		if (_newSceneNum == 55)
			_newSceneNum = 37;
		break;
	case 47:
		if (_prevSceneNum == 49) {
			_scene = new Scene471(this);
			_scene->init();
			_newSceneNum = 7;
			_newCursorValue = 2;
		} else if (_prevSceneNum == 13) {
			_scene = new Scene472(this);
			_scene->init();
			_newSceneNum = 11;
		} else if (!isFlag(kGFPlatyPussDisguised) && _prevSceneNum == 2) {//CHECKME
			if (isFlag(kGFUnk25)) {
				_scene = new Scene473(this);
				_scene->init();
				_newSceneNum = 2;
			} else {
				_scene = new Scene474(this);
				_scene->init();
				_newSceneNum = 49;
			}
		} else if (_prevSceneNum == 21) {
			_scene = new Scene475(this);
			_scene->init();
			_newSceneNum = 21;
			setFlag(kGFTwigTaken);
			setFlag(kGFKeysTaken);
		} else if (_prevSceneNum == 30) {
			_scene = new Scene476(this);
			_scene->init();
			_newSceneNum = 26;
		} else if (isFlag(kGFPlatyPussDisguised) && _cursorValue == 1) {
			_scene = new Scene477(this);
			_scene->init();
			_newSceneNum = 4;
		}
		_scene->run();
		delete _scene;
		break;
	case 48:
		_scene = new Scene48(this);
		_scene->init();
		_newSceneNum = 33;
		_newCursorValue = 4;
		_scene->run();
		delete _scene;
		break;
	case 49:
		scene49_run();
		if (_newSceneNum == 55)
			_newSceneNum = 47;
		break;
	case 50:
		scene50_run();
		_newSceneNum = _prevSceneNum;
		break;
	case 51:
		scene51_run();
		break;
	case 52:
		scene52_run();
		_newSceneNum = _prevSceneNum;
		break;
	case 53:
		scene53_run();
		if (_newSceneNum == 55)
			_newSceneNum = 53;
		break;
	case 54:
		if (_prevSceneNum == 45) {
			_scene = new Scene541(this);
			_scene->init();
			_newSceneNum = 43;
			_scene->run();
			delete _scene;
		} else {
			_scene = new Scene542(this);
			_scene->init();
			_scene->run();
			delete _scene;
			_gameDone = true;
		}
		break;
	}
}

void CutScene::run() {
	int itemIndex = 0;
	int soundId = -1;
	int volume = 100;
	int duration = 0;
	bool skip = false;

	if (_vm->_prevSceneNum == 2) {
		soundId = 0x36B;
		duration = MAX(1, 300 / _vm->getSequenceTotalDuration(_s99_sequenceIdArr[_s99_itemsCount - 1]));//CHECKME
		_vm->_timers[0] = 0;
	}

	if (soundId != -1)
		_vm->playSound(soundId, false);

	_vm->hideCursor();

	_vm->_gameSys->drawSpriteToBackground(0, 0, _s99_resourceIdArr[0]);

	for (int j = 0; j < _s99_sequenceCountArr[0]; ++j)
		_vm->_gameSys->insertSequence(_s99_sequenceIdArr[j], j + 2, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->setAnimation(_s99_sequenceIdArr[0], 2, 0);

	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->clearKeyStatus1(29);

	_vm->_mouseClickState._left = false;

	int firstSequenceIndex = 0;
	while (!_vm->_sceneDone) {
		_vm->gameUpdateTick();

		if (_vm->_gameSys->getAnimationStatus(0) == 2 || skip) {
			skip = false;
			_vm->_gameSys->requestClear2(false);
			_vm->_gameSys->requestClear1();
			_vm->_gameSys->setAnimation(0, 0, 0);
			firstSequenceIndex += _s99_sequenceCountArr[itemIndex++];
			if (itemIndex >= _s99_itemsCount) {
				_vm->_sceneDone = true;
			} else {
				for (int m = 0; m < _s99_sequenceCountArr[itemIndex]; ++m)
					_vm->_gameSys->insertSequence(_s99_sequenceIdArr[firstSequenceIndex + m], m + 2, 0, 0, kSeqNone, 0, 0, 0);
				_vm->_gameSys->drawSpriteToBackground(0, 0, _s99_resourceIdArr[itemIndex]);
				_vm->_gameSys->setAnimation(_s99_sequenceIdArr[firstSequenceIndex], 2, 0);
			}
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) || _vm->isKeyStatus1(Common::KEYCODE_SPACE) || _vm->isKeyStatus1(29)) {
			_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
			_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
			_vm->clearKeyStatus1(29);
			if (_s99_canSkip[itemIndex])
				skip = true;
			else
				_vm->_sceneDone = true;
		}

		if (!_vm->_timers[0] && itemIndex == _s99_itemsCount - 1) {
			_vm->_timers[0] = 2;
			volume = MAX(1, volume - duration);
			_vm->setSoundVolume(soundId, volume);
		}
	}

	if (soundId != -1)
		_vm->stopSound(soundId);
}

} // End of namespace Gnap
