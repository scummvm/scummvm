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

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

#include "gnap/scenes/scenecore.h"

#include "gnap/scenes/arcade.h"
#include "gnap/scenes/groupcs.h"
#include "gnap/scenes/group0.h"
#include "gnap/scenes/group1.h"
#include "gnap/scenes/group2.h"
#include "gnap/scenes/group3.h"
#include "gnap/scenes/group4.h"
#include "gnap/scenes/group5.h"
#include "gnap/scenes/intro.h"

namespace Gnap {

int GnapEngine::initSceneLogic() {
	int backgroundId = -1;

	switch (_currentSceneNum) {
	case 0:
		_scene = new SceneIntro(this);
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
		_scene = new Scene27(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 28:
		_scene = new Scene28(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 29:
		_scene = new Scene29(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 30:
		_scene = new Scene30(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 31:
		_scene = new Scene31(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 32:
		_scene = new Scene32(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 33:
		_scene = new Scene33(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
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
		_scene = new Scene38(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 39:
		_scene = new Scene39(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 40:
		_scene = new Scene40(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 41:
		_scene = new Scene41(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 42:
		_scene = new Scene42(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 43:
		_scene = new Scene43(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 44:
		_scene = new Scene44(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 45:
		_scene = new Scene45(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 46:
		_scene = new Scene46(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 49:
		_scene = new Scene49(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 50:
		_scene = new Scene50(this);
		backgroundId = _scene->init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 51:
		_scene = new Scene51(this);
		backgroundId = _scene->init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 52:
		_scene = new Scene52(this);
		backgroundId = _scene->init();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	case 53:
		_scene = new Scene53(this);
		backgroundId = _scene->init();
		_scene->updateHotspots();
		_gameSys->setScaleValues(0, 500, 1, 1000);
		initSceneGrid(21, 146, 11, 10);
		break;
	default:
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
	case 2:
	case 3:
	case 4:
	case 5:
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
	case 12:
	case 13:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 11;
		break;
	case 11:
	case 15:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 12;
		break;
	case 14:
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 13;
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
	case 18:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
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
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
		_scene->run();
		delete _scene;
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
		} else if (!isFlag(kGFPlatypusDisguised) && _prevSceneNum == 2) {//CHECKME
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
		} else if (isFlag(kGFPlatypusDisguised) && _cursorValue == 1) {
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
		_scene->run();
		delete _scene;
		if (_newSceneNum == 55)
			_newSceneNum = 47;
		break;
	case 50:
		_scene->run();
		delete _scene;
		_newSceneNum = _prevSceneNum;
		break;
	case 51:
		_scene->run();
		delete _scene;
		break;
	case 52:
		_scene->run();
		delete _scene;
		_newSceneNum = _prevSceneNum;
		break;
	case 53:
		_scene->run();
		delete _scene;
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
	default:
		break;
	}
}

void Scene::playRandomSound(int timerIndex) {
	if (!_vm->_timers[timerIndex]) {
		_vm->_timers[timerIndex] = _vm->getRandom(40) + 50;
		switch (_vm->getRandom(4)) {
		case 0:
			_vm->playSound(0x1091B, false);
			break;
		case 1:
			_vm->playSound(0x10921, false);
			break;
		case 2:
			_vm->playSound(0x10927, false);
			break;
		case 3:
			_vm->playSound(0x1091D, false);
			break;
		default:
			break;
		}
	}
}

bool Scene::clearKeyStatus() {
	if (_vm->isKeyStatus1(Common::KEYCODE_ESCAPE)) {
		_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
		_vm->clearKeyStatus1(Common::KEYCODE_UP);
		_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
		_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
		_vm->clearKeyStatus1(Common::KEYCODE_p);
		return true;
	}

	if (_vm->isKeyStatus1(Common::KEYCODE_p)) {
		_vm->clearKeyStatus1(Common::KEYCODE_p);
		_vm->pauseGame();
		_vm->updatePause();
	}

	return false;
}

/****************************************************************************/

CutScene::CutScene(GnapEngine *vm) : Scene(vm) {
	_itemsCount = -1;

	for (int i = 0; i < 16; i++) {
		_resourceIdArr[i] = -1;
		_sequenceCountArr[i] = -1;
		_canSkip[i] = false;
	}

	for (int i = 0; i < 50; i++)
		_sequenceIdArr[i] = -1;
}

void CutScene::run() {
	GameSys& gameSys = *_vm->_gameSys;

	int itemIndex = 0;
	int soundId = -1;
	int volume = 100;
	int duration = 0;
	bool skip = false;

	if (_vm->_prevSceneNum == 2) {
		soundId = 0x36B;
		duration = MAX(1, 300 / _vm->getSequenceTotalDuration(_sequenceIdArr[_itemsCount - 1]));
		_vm->_timers[0] = 0;
	}

	if (soundId != -1)
		_vm->playSound(soundId, false);

	_vm->hideCursor();

	gameSys.drawSpriteToBackground(0, 0, _resourceIdArr[0]);

	for (int j = 0; j < _sequenceCountArr[0]; ++j)
		gameSys.insertSequence(_sequenceIdArr[j], j + 2, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(_sequenceIdArr[0], 2, 0);

	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->clearKeyStatus1(Common::KEYCODE_RETURN);

	_vm->_mouseClickState._left = false;

	int firstSequenceIndex = 0;
	while (!_vm->_sceneDone) {
		_vm->gameUpdateTick();

		if (gameSys.getAnimationStatus(0) == 2 || skip) {
			skip = false;
			gameSys.requestClear2(false);
			gameSys.requestClear1();
			gameSys.setAnimation(0, 0, 0);
			firstSequenceIndex += _sequenceCountArr[itemIndex++];
			if (itemIndex >= _itemsCount) {
				_vm->_sceneDone = true;
			} else {
				for (int m = 0; m < _sequenceCountArr[itemIndex]; ++m)
					gameSys.insertSequence(_sequenceIdArr[firstSequenceIndex + m], m + 2, 0, 0, kSeqNone, 0, 0, 0);
				gameSys.drawSpriteToBackground(0, 0, _resourceIdArr[itemIndex]);
				gameSys.setAnimation(_sequenceIdArr[firstSequenceIndex], 2, 0);
			}
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) || _vm->isKeyStatus1(Common::KEYCODE_SPACE) || _vm->isKeyStatus1(Common::KEYCODE_RETURN)) {
			_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
			_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
			_vm->clearKeyStatus1(Common::KEYCODE_RETURN);
			if (_canSkip[itemIndex])
				skip = true;
			else
				_vm->_sceneDone = true;
		}

		if (!_vm->_timers[0] && itemIndex == _itemsCount - 1) {
			_vm->_timers[0] = 2;
			volume = MAX(1, volume - duration);
			_vm->setSoundVolume(soundId, volume);
		}
	}

	if (soundId != -1)
		_vm->stopSound(soundId);
}

} // End of namespace Gnap
