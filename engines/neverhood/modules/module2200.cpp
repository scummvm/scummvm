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

#include "common/config-manager.h"

#include "neverhood/diskplayerscene.h"
#include "neverhood/gamemodule.h"
#include "neverhood/modules/module1000_sprites.h"
#include "neverhood/modules/module1200_sprites.h"
#include "neverhood/modules/module2200.h"
#include "neverhood/modules/module2200_sprites.h"

namespace Neverhood {

Module2200::Module2200(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_vm->_soundMan->addMusic(0x11391412, 0x601C908C);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else
		createScene(0, 0);

}

Module2200::~Module2200() {
	_vm->_soundMan->deleteGroup(0x11391412);
}

void Module2200::createScene(int sceneNum, int which) {
	if (sceneNum == 46 && ConfMan.getBool("skiphallofrecordsscenes")) {
		// Skip the whole Hall of Records storyboard scenes,
		// and teleport to the last scene
		sceneNum = 41;
	}

	if (sceneNum == 40 && ConfMan.getBool("skiphallofrecordsscenes")) {
		// Skip the whole Hall of Records storyboard scenes,
		// and teleport back to the first scene
		sceneNum = 5;
	}

	debug(1, "Module2200::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_vm->_soundMan->startMusic(0x601C908C, 0, 2);
		_childObject = new Scene2201(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_vm->_soundMan->startMusic(0x601C908C, 0, 2);
		_childObject = new Scene2202(_vm, this, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->_soundMan->startMusic(0x601C908C, 0, 2);
		_childObject = new Scene2203(_vm, this, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_vm->_soundMan->stopMusic(0x601C908C, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 3);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->stopMusic(0x601C908C, 0, 2);
		_childObject = new Scene2205(_vm, this, which);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		_vm->_soundMan->stopMusic(0x601C908C, 0, 2);
		_childObject = new Scene2206(_vm, this, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_childObject = new Scene2207(_vm, this);
		break;
	case 7:
		if (which >= 0)
			_vm->gameState().which = _vm->gameState().sceneNum;
		_vm->gameState().sceneNum = 7;
		_childObject = new Scene2208(_vm, this, which);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_childObject = new Scene2208(_vm, this, which);
		break;
	case 9:
		_vm->gameState().sceneNum = 9;
		createHallOfRecordsScene(which, 0x004B7180);
		break;
	case 10:
		_vm->gameState().sceneNum = 10;
		createHallOfRecordsScene(which, 0x004B7198);
		break;
	case 11:
		_vm->gameState().sceneNum = 11;
		createHallOfRecordsScene(which, 0x004B71B0);
		break;
	case 12:
		_vm->gameState().sceneNum = 12;
		createHallOfRecordsScene(which, 0x004B71C8);
		break;
	case 13:
		_vm->gameState().sceneNum = 13;
		createHallOfRecordsScene(which, 0x004B71E0);
		break;
	case 14:
		_vm->gameState().sceneNum = 14;
		createHallOfRecordsScene(which, 0x004B71F8);
		break;
	case 15:
		_vm->gameState().sceneNum = 15;
		createHallOfRecordsScene(which, 0x004B7210);
		break;
	case 16:
		_vm->gameState().sceneNum = 16;
		createHallOfRecordsScene(which, 0x004B7228);
		break;
	case 17:
		_vm->gameState().sceneNum = 17;
		createHallOfRecordsScene(which, 0x004B7240);
		break;
	case 18:
		_vm->gameState().sceneNum = 18;
		createHallOfRecordsScene(which, 0x004B7258);
		break;
	case 19:
		_vm->gameState().sceneNum = 19;
		createHallOfRecordsScene(which, 0x004B7270);
		break;
	case 20:
		_vm->gameState().sceneNum = 20;
		createHallOfRecordsScene(which, 0x004B7288);
		break;
	case 21:
		_vm->gameState().sceneNum = 21;
		createHallOfRecordsScene(which, 0x004B72A0);
		break;
	case 22:
		_vm->gameState().sceneNum = 22;
		createHallOfRecordsScene(which, 0x004B72B8);
		break;
	case 23:
		_vm->gameState().sceneNum = 23;
		createHallOfRecordsScene(which, 0x004B72D0);
		break;
	case 24:
		_vm->gameState().sceneNum = 24;
		createHallOfRecordsScene(which, 0x004B72E8);
		break;
	case 25:
		_vm->gameState().sceneNum = 25;
		createHallOfRecordsScene(which, 0x004B7300);
		break;
	case 26:
		_vm->gameState().sceneNum = 26;
		createHallOfRecordsScene(which, 0x004B7318);
		break;
	case 27:
		_vm->gameState().sceneNum = 27;
		createHallOfRecordsScene(which, 0x004B7330);
		break;
	case 28:
		_vm->gameState().sceneNum = 28;
		createHallOfRecordsScene(which, 0x004B7348);
		break;
	case 29:
		_vm->gameState().sceneNum = 29;
		createHallOfRecordsScene(which, 0x004B7360);
		break;
	case 30:
		_vm->gameState().sceneNum = 30;
		createHallOfRecordsScene(which, 0x004B7378);
		break;
	case 31:
		_vm->gameState().sceneNum = 31;
		createHallOfRecordsScene(which, 0x004B7390);
		break;
	case 32:
		_vm->gameState().sceneNum = 32;
		createHallOfRecordsScene(which, 0x004B73A8);
		break;
	case 33:
		_vm->gameState().sceneNum = 33;
		createHallOfRecordsScene(which, 0x004B73C0);
		break;
	case 34:
		_vm->gameState().sceneNum = 34;
		createHallOfRecordsScene(which, 0x004B73D8);
		break;
	case 35:
		_vm->gameState().sceneNum = 35;
		createHallOfRecordsScene(which, 0x004B73F0);
		break;
	case 36:
		_vm->gameState().sceneNum = 36;
		createHallOfRecordsScene(which, 0x004B7408);
		break;
	case 37:
		_vm->gameState().sceneNum = 37;
		createHallOfRecordsScene(which, 0x004B7420);
		break;
	case 38:
		_vm->gameState().sceneNum = 38;
		createHallOfRecordsScene(which, 0x004B7438);
		break;
	case 39:
		_vm->gameState().sceneNum = 39;
		createHallOfRecordsScene(which, 0x004B7450);
		break;
	case 40:
		_vm->gameState().sceneNum = 40;
		createHallOfRecordsScene(which, 0x004B7468);
		break;
	case 41:
		_vm->gameState().sceneNum = 41;
		_childObject = new Scene2242(_vm, this, which);
		break;
	case 42:
		_vm->gameState().sceneNum = 42;
		createHallOfRecordsScene(which, 0x004B7480);
		break;
	case 43:
		_vm->gameState().sceneNum = 43;
		createHallOfRecordsScene(which, 0x004B7498);
		break;
	case 44:
		_vm->gameState().sceneNum = 44;
		createHallOfRecordsScene(which, 0x004B74B0);
		break;
	case 45:
		_vm->gameState().sceneNum = 45;
		createHallOfRecordsScene(which, 0x004B74C8);
		break;
	case 46:
		_vm->gameState().sceneNum = 46;
		_childObject = new Scene2247(_vm, this, which);
		break;
	case 47:
		_vm->gameState().sceneNum = 47;
		if (!getGlobalVar(V_WORLDS_JOINED)) {
			if (getGlobalVar(V_LIGHTS_ON))
				createStaticScene(0x83110287, 0x10283839);
			else
				createStaticScene(0x83412B9D, 0x12B9983C);
		} else {
			if (getGlobalVar(V_LIGHTS_ON))
				createStaticScene(0x48632087, 0x3208348E);
			else
				createStaticScene(0x08C74886, 0x74882084);
		}
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2200::updateScene);
	_childObject->handleUpdate();
}

#define HallOfRecordsSceneLink(nextSceneNum, prevSceneNum) \
	if (_moduleResult == 1) createScene(nextSceneNum, 0); else if (_moduleResult == 2) createScene(7, 0); else createScene(prevSceneNum, 1)

void Module2200::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(2, 0);
			else if (_moduleResult == 2)
				createScene(1, 0);
			else
				leaveModule(0);
			break;
		case 1:
			createScene(0, 2);
			break;
		case 2:
			if (_moduleResult == 1)
				createScene(4, 0);
			else if (_moduleResult == 2)
				createScene(3, 0);
			else
				createScene(0, 1);
			break;
		case 3:
			createScene(2, 2);
			break;
		case 4:
			if (_moduleResult == 1)
				createScene(5, 0);
			else if (_moduleResult == 2)
				createScene(4, 2);
			else
				createScene(2, 1);
			break;
		case 5:
			if (_moduleResult == 1)
				createScene(46, 0);
			else if (_moduleResult == 2)
				createScene(6, 0);
			else if (_moduleResult == 3)
				createScene(8, 0);
			else
				createScene(4, 1);
			break;
		case 6:
			createScene(5, 2);
			break;
		case 7:
			createScene(_vm->gameState().which, 2);
			break;
		case 8:
			createScene(5, 3);
			break;
		case 9:
			HallOfRecordsSceneLink(10, 46);
			break;
		case 10:
			HallOfRecordsSceneLink(11, 9);
			break;
		case 11:
			HallOfRecordsSceneLink(12, 10);
			break;
		case 12:
			HallOfRecordsSceneLink(13, 11);
			break;
		case 13:
			HallOfRecordsSceneLink(14, 12);
			break;
		case 14:
			HallOfRecordsSceneLink(15, 13);
			break;
		case 15:
			HallOfRecordsSceneLink(16, 14);
			break;
		case 16:
			HallOfRecordsSceneLink(17, 15);
			break;
		case 17:
			HallOfRecordsSceneLink(18, 16);
			break;
		case 18:
			HallOfRecordsSceneLink(19, 17);
			break;
		case 19:
			HallOfRecordsSceneLink(20, 18);
			break;
		case 20:
			HallOfRecordsSceneLink(21, 19);
			break;
		case 21:
			HallOfRecordsSceneLink(22, 20);
			break;
		case 22:
			HallOfRecordsSceneLink(23, 21);
			break;
		case 23:
			HallOfRecordsSceneLink(24, 22);
			break;
		case 24:
			HallOfRecordsSceneLink(25, 23);
			break;
		case 25:
			HallOfRecordsSceneLink(26, 24);
			break;
		case 26:
			HallOfRecordsSceneLink(27, 25);
			break;
		case 27:
			HallOfRecordsSceneLink(28, 26);
			break;
		case 28:
			HallOfRecordsSceneLink(29, 27);
			break;
		case 29:
			HallOfRecordsSceneLink(30, 28);
			break;
		case 30:
			HallOfRecordsSceneLink(31, 29);
			break;
		case 31:
			HallOfRecordsSceneLink(32, 30);
			break;
		case 32:
			HallOfRecordsSceneLink(33, 31);
			break;
		case 33:
			HallOfRecordsSceneLink(34, 32);
			break;
		case 34:
			HallOfRecordsSceneLink(42, 33);
			break;
		case 35:
			HallOfRecordsSceneLink(36, 45);
			break;
		case 36:
			HallOfRecordsSceneLink(37, 35);
			break;
		case 37:
			HallOfRecordsSceneLink(38, 36);
			break;
		case 38:
			HallOfRecordsSceneLink(39, 37);
			break;
		case 39:
			HallOfRecordsSceneLink(40, 38);
			break;
		case 40:
			HallOfRecordsSceneLink(41, 39);
			break;
		case 41:
			HallOfRecordsSceneLink(47, 40);
			break;
		case 42:
			HallOfRecordsSceneLink(43, 34);
			break;
		case 43:
			HallOfRecordsSceneLink(44, 42);
			break;
		case 44:
			HallOfRecordsSceneLink(45, 43);
			break;
		case 45:
			HallOfRecordsSceneLink(35, 44);
			break;
		case 46:
			HallOfRecordsSceneLink(9, 5);
			break;
		case 47:
			createScene(41, 1);
			break;
		default:
			break;
		}
	}
}

#undef HallOfRecordsSceneLink

void Module2200::createHallOfRecordsScene(int which, uint32 hallOfRecordsInfoId) {
	_childObject = new HallOfRecordsScene(_vm, this, which, hallOfRecordsInfoId);
}

Scene2201::Scene2201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isSoundPlaying(false) {

	Sprite *tempSprite;

	_vm->gameModule()->initCubeSymbolsPuzzle();

	SetMessageHandler(&Scene2201::handleMessage);
	SetUpdateHandler(&Scene2201::update);

	loadDataResource(0x04104242);
	loadHitRectList();
	setBackground(0x40008208);
	setPalette(0x40008208);
	insertScreenMouse(0x0820C408);

	_asTape = insertSprite<AsScene1201Tape>(this, 7, 1100, 459, 432, 0x9148A011);
	addCollisionSprite(_asTape);
	_ssDoorButton = insertSprite<SsCommonPressButton>(this, 0xE4A43E29, 0xE4A43E29, 100, 0);

	for (uint32 cubeIndex = 0; cubeIndex < 9; cubeIndex++)
		if ((int16)getSubVar(VA_CUBE_POSITIONS, cubeIndex) >= 0)
			insertSprite<SsScene2201PuzzleCube>(cubeIndex, (int16)getSubVar(VA_CUBE_POSITIONS, cubeIndex));

	_clipRects[0].y1 = 0;
	_clipRects[0].x2 = 640;
	_clipRects[1].x2 = 640;
	_clipRects[1].y2 = 480;

	if (!getGlobalVar(V_TILE_PUZZLE_SOLVED))
		insertStaticSprite(0x00026027, 900);

	tempSprite = insertStaticSprite(0x030326A0, 1100);
	_clipRects[0].x1 = tempSprite->getDrawRect().x;
	insertStaticSprite(0x811DA061, 1100);
	tempSprite = insertStaticSprite(0x11180022, 1100);
	_clipRects[1].x1 = tempSprite->getDrawRect().x;
	tempSprite = insertStaticSprite(0x0D411130, 1100);
	_clipRects[0].y2 = tempSprite->getDrawRect().y2();
	_clipRects[1].y1 = tempSprite->getDrawRect().y2();
	_ssDoorLight = insertStaticSprite(0xA4062212, 900);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2201>(300, 427, _clipRects, 2);
		setMessageList(0x004B8118);
		_asDoor = insertSprite<AsScene2201Door>(_klaymen, _ssDoorLight, false);
	} else if (which == 1) {
		// Klaymen entering from the back
		insertKlaymen<KmScene2201>(412, 393, _clipRects, 2);
		setMessageList(0x004B8130);
		_asDoor = insertSprite<AsScene2201Door>(_klaymen, _ssDoorLight, false);
	} else if (which == 2) {
		// Klaymen returning from the puzzle
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene2201>(379, 427, _clipRects, 2);
			_klaymen->setDoDeltaX(1);
		} else
			insertKlaymen<KmScene2201>(261, 427, _clipRects, 2);
		setMessageList(0x004B8178);
		_asDoor = insertSprite<AsScene2201Door>(_klaymen, _ssDoorLight, false);
	} else {
		// Klaymen entering from the left
		NPoint pt = _dataResource.getPoint(0x0304D8DC);
		insertKlaymen<KmScene2201>(pt.x, pt.y, _clipRects, 2);
		setMessageList(0x004B8120);
		_asDoor = insertSprite<AsScene2201Door>(_klaymen, _ssDoorLight, true);
	}

	insertSprite<AsScene2201CeilingFan>();

	_vm->_soundMan->addSound(0x04106220, 0x81212040);

}

Scene2201::~Scene2201() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
	_vm->_soundMan->deleteSoundGroup(0x04106220);
}

void Scene2201::update() {
	Scene::update();
	if (!_isSoundPlaying) {
		_vm->_soundMan->playSoundLooping(0x81212040);
		_isSoundPlaying = true;
	}
}

uint32 Scene2201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klaymen, 0x1014, _ssDoorButton);
		else if (param.asInteger() == 0x35803198) {
			if (sendMessage(_asDoor, 0x2000, 0))
				setMessageList(0x004B81A0);
			else
				setMessageList(0x004B81B8);
		} else if (param.asInteger() == 0x51445010) {
			if (getGlobalVar(V_TILE_PUZZLE_SOLVED))
				setMessageList(0x004B8108);
			else
				setMessageList(0x004B8150);
		} else if (param.asInteger() == 0x1D203082)
			setMessageList(0x004B8180);
		else if (param.asInteger() == 0x00049091) {
			if (getGlobalVar(V_TILE_PUZZLE_SOLVED))
				setMessageList(0x004B8138);
			else
				setMessageList(0x004B8108);
		}
		break;
	case 0x480B:
		if (sender == _ssDoorButton)
			sendMessage(_asDoor, NM_KLAYMEN_OPEN_DOOR, 0);
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004B81C8);
		}
		break;
	default:
		break;
	}
	return 0;
}

Scene2202::Scene2202(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isSolved(false), _leaveScene(false), _isCubeMoving(false),
	_ssMovingCube(NULL), _ssDoneMovingCube(NULL) {

	_vm->gameModule()->initCubeSymbolsPuzzle();

	SetMessageHandler(&Scene2202::handleMessage);
	SetUpdateHandler(&Scene2202::update);

	setBackground(0x08100A0C);
	setPalette(0x08100A0C);
	addEntity(_palette);
	insertPuzzleMouse(0x00A08089, 20, 620);

	for (uint32 cubePosition = 0; cubePosition < 9; cubePosition++) {
		int16 cubeSymbol = (int16)getSubVar(VA_CUBE_POSITIONS, cubePosition);
		if (cubeSymbol >= 0) {
			Sprite *puzzleCubeSprite = insertSprite<SsScene2202PuzzleCube>(this, cubePosition, cubeSymbol);
			addCollisionSprite(puzzleCubeSprite);
		}
	}

	insertStaticSprite(0x55C043B8, 200);
	insertStaticSprite(0x85500158, 400);
	insertStaticSprite(0x25547028, 600);

	loadSound(0, 0x68E25540);
	loadSound(1, 0x40400457);

	_vm->_soundMan->addSound(0x60400854, 0x8101A241);
	_vm->_soundMan->playSoundLooping(0x8101A241);

}

Scene2202::~Scene2202() {
	_vm->_soundMan->deleteSoundGroup(0x60400854);
}

void Scene2202::update() {
	Scene::update();

	if (_leaveScene && !isSoundPlaying(1))
		leaveScene(0);

	if (_isSolved && !isSoundPlaying(0)) {
		playSound(1);
		_isSolved = false;
		_leaveScene = true;
	}

	if (_ssMovingCube && !_isCubeMoving) {
		int16 freeCubePosition = getFreeCubePosition(_movingCubePosition);
		if (freeCubePosition != -1) {
			setSurfacePriority(_ssMovingCube->getSurface(), 700);
			sendMessage(_ssMovingCube, 0x2001, freeCubePosition);
			_ssMovingCube = NULL;
			_isCubeMoving = true;
		}
	}

	if (_ssDoneMovingCube) {
		setSurfacePriority(_ssDoneMovingCube->getSurface(), _surfacePriority);
		_ssDoneMovingCube = NULL;
		if (testIsSolved()) {
			playSound(0);
			setGlobalVar(V_TILE_PUZZLE_SOLVED, 1);
			_isSolved = true;
		}
	}

}

uint32 Scene2202::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	case NM_ANIMATION_UPDATE:
		_movingCubePosition = (int16)param.asInteger();
		_ssMovingCube = (Sprite*)sender;
		break;
	case NM_POSITION_CHANGE:
		_isCubeMoving = false;
		_ssDoneMovingCube = (Sprite*)sender;
		if (param.asInteger() <= 2)
			_surfacePriority = 100;
		else if (param.asInteger() >= 3 && param.asInteger() <= 5)
			_surfacePriority = 300;
		else
			_surfacePriority = 500;
		break;
	default:
		break;
	}
	return 0;
}

int16 Scene2202::getFreeCubePosition(int16 cubePosition) {
	if (cubePosition >= 3 && (int16)getSubVar(VA_CUBE_POSITIONS, cubePosition - 3) == -1)
		return cubePosition - 3;
	else if (cubePosition <= 5 && (int16)getSubVar(VA_CUBE_POSITIONS, cubePosition + 3) == -1)
		return cubePosition + 3;
	else if (cubePosition != 0 && cubePosition != 3 && cubePosition != 6 && (int16)getSubVar(VA_CUBE_POSITIONS, cubePosition - 1) == -1)
		return cubePosition - 1;
	else if (cubePosition != 2 && cubePosition != 5 && cubePosition != 8 && (int16)getSubVar(VA_CUBE_POSITIONS, cubePosition + 1) == -1)
		return cubePosition + 1;
	else
		return -1;
}

bool Scene2202::testIsSolved() {
	return
		getSubVar(VA_CUBE_POSITIONS, 0) == 0 &&
		getSubVar(VA_CUBE_POSITIONS, 2) == 2 &&
		getSubVar(VA_CUBE_POSITIONS, 3) == 3 &&
		getSubVar(VA_CUBE_POSITIONS, 4) == 4 &&
		getSubVar(VA_CUBE_POSITIONS, 5) == 5 &&
		getSubVar(VA_CUBE_POSITIONS, 6) == 6 &&
		getSubVar(VA_CUBE_POSITIONS, 8) == 7;
}

Scene2203::Scene2203(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	if (getGlobalVar(V_HAS_FINAL_KEY) && getGlobalVar(V_KEY3_LOCATION) == 0)
		setGlobalVar(V_KEY3_LOCATION, 1);

	SetMessageHandler(&Scene2203::handleMessage);

	setBackground(0x82C80334);
	setPalette(0x82C80334);
	insertScreenMouse(0x80330824);
	setHitRects(0x004B8320);
	setRectList(0x004B8420);

	if (getGlobalVar(V_KEY3_LOCATION) == 1) {
		_asKey = insertSprite<AsCommonKey>(this, 2, 1100, 282, 432);
		addCollisionSprite(_asKey);
	}

	_asTape = insertSprite<AsScene1201Tape>(this, 1, 1100, 435, 432, 0x9148A011);
	addCollisionSprite(_asTape);
	_asLeftDoor = insertSprite<AsScene2203Door>(this, 0);
	_asRightDoor = insertSprite<AsScene2203Door>(this, 1);
	_ssSmallLeftDoor = insertStaticSprite(0x542CC072, 1100);
	_ssSmallRightDoor = insertStaticSprite(0x0A2C0432, 1100);
	_leftDoorClipRect.set(_ssSmallLeftDoor->getDrawRect().x, 0, 640, 480);
	_rightDoorClipRect.set(0, 0, _ssSmallRightDoor->getDrawRect().x2(), 480);
	sendEntityMessage(_asLeftDoor, 0x2000, _asRightDoor);
	sendEntityMessage(_asRightDoor, 0x2000, _asLeftDoor);
	addCollisionSprite(_asLeftDoor);
	addCollisionSprite(_asRightDoor);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2203>(200, 427);
		setMessageList(0x004B8340);
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene2203>(640, 427);
		setMessageList(0x004B8350);
	} else if (which == 2) {
		// Klaymen returning from the displayer
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene2203>(362, 427);
			_klaymen->setDoDeltaX(1);
		} else
			insertKlaymen<KmScene2203>(202, 427);
		setMessageList(0x004B8358);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene2203>(0, 427);
		setMessageList(0x004B8348);
	}

	if (getGlobalVar(V_LARGE_DOOR_NUMBER)) {
		_ssSmallLeftDoor->setVisible(false);
		_klaymen->setClipRect(_rightDoorClipRect);
	} else {
		_ssSmallRightDoor->setVisible(false);
		_klaymen->setClipRect(_leftDoorClipRect);
	}

}

Scene2203::~Scene2203() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2203::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		sendEntityMessage(_klaymen, 0x1014, sender);
		if (sender == _asLeftDoor)
			setMessageList2(0x004B83B0);
		else
			setMessageList2(0x004B83C8);
		break;
	case NM_POSITION_CHANGE:
		if (sender == _asLeftDoor)
			setMessageList2(0x004B8370);
		else
			setMessageList2(0x004B8360);
		break;
	case 0x2003:
		if (sender == _asLeftDoor)
			_ssSmallLeftDoor->setVisible(false);
		else
			_ssSmallRightDoor->setVisible(false);
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		if (sender == _asLeftDoor) {
			_ssSmallLeftDoor->setVisible(true);
			_klaymen->setClipRect(_leftDoorClipRect);
		} else {
			_ssSmallRightDoor->setVisible(true);
			_klaymen->setClipRect(_rightDoorClipRect);
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004B83E0);
		} else if (sender == _asKey) {
			sendEntityMessage(_klaymen, 0x1014, _asKey);
			setMessageList(0x004B83F0);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

Scene2205::Scene2205(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene2205::handleMessage);
	SetUpdateHandler(&Scene2205::update);

	setHitRects(0x004B0620);
	if (getGlobalVar(V_LIGHTS_ON)) {
		_isLightOn = true;
		setBackground(0x0008028D);
		setPalette(0x0008028D);
		addEntity(_palette);
		insertScreenMouse(0x80289008);
		_ssLightSwitch = insertSprite<SsCommonPressButton>(this, 0x2D339030, 0x2D309030, 100, 0);
	} else {
		_isLightOn = false;
		setBackground(0xD00A028D);
		setPalette(0xD00A028D);
		addEntity(_palette);
		insertScreenMouse(0xA0289D08);
		_ssLightSwitch = insertSprite<SsCommonPressButton>(this, 0x2D339030, 0xDAC86E84, 100, 0);
	}
	_palette->addBasePalette(0xD00A028D, 0, 256, 0);
	_ssDoorFrame = insertSprite<SsScene2205DoorFrame>();

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2205>(320, 417);
		setMessageList(0x004B0658);
		if (!getGlobalVar(V_LIGHTS_ON))
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		_isKlaymenInLight = false;
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene2205>(640, 417);
		setMessageList(0x004B0648);
		if (!getGlobalVar(V_LIGHTS_ON))
			_palette->addPalette(0x68033B1C, 0, 65, 0);
		_isKlaymenInLight = false;
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene2205>(0, 417);
		setMessageList(0x004B0640);
		_isKlaymenInLight = true;
	}

	_klaymen->setClipRect(_ssDoorFrame->getDrawRect().x, 0, 640, 480);
	_klaymen->setSoundFlag(true);

	loadDataResource(0x00144822);

}

void Scene2205::update() {
	Scene::update();
	if (!_isLightOn && getGlobalVar(V_LIGHTS_ON)) {
		_palette->addPalette(0x0008028D, 0, 256, 0);
		changeBackground(0x0008028D);
		_ssLightSwitch->setFileHashes(0x2D339030, 0x2D309030);
		sendMessage(_ssDoorFrame, 0x2000, 0);
		changeMouseCursor(0x80289008);
		_isLightOn = true;
	} else if (_isLightOn && !getGlobalVar(V_LIGHTS_ON)) {
		_palette->addPalette(0xD00A028D, 0, 256, 0);
		changeBackground(0xD00A028D);
		_ssLightSwitch->setFileHashes(0xD6C86E84, 0xDAC86E84);
		sendMessage(_ssDoorFrame, 0x2000, 0);
		changeMouseCursor(0xA0289D08);
		_isKlaymenInLight = true;
		if (_klaymen->getX() > 85) {
			_palette->addPalette(0x68033B1C, 0, 65, 0);
			_isKlaymenInLight = false;
		}
		_isLightOn = false;
	}
	if (!getGlobalVar(V_LIGHTS_ON)) {
		if (_isKlaymenInLight && _klaymen->getX() > 85) {
			_palette->addBasePalette(0x68033B1C, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymenInLight = false;
		} else if (!_isKlaymenInLight && _klaymen->getX() <= 85) {
			_palette->addBasePalette(0xD00A028D, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymenInLight = true;
		}
	}
}

uint32 Scene2205::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x6449569A)
			setMessageList(0x004B0690);
		else if (param.asInteger() == 0x2841369C)
			setMessageList(0x004B0630);
		else if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klaymen, 0x1014, _ssLightSwitch);
		break;
	case 0x480B:
		setGlobalVar(V_LIGHTS_ON, getGlobalVar(V_LIGHTS_ON) ? 0 : 1);
		break;
	default:
		break;
	}
	return 0;
}

static const int16 kScene2206XPositions[] = {
	384, 480, 572
};

Scene2206::Scene2206(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	uint32 fileHash;

	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene2206::handleMessage);

	if (getGlobalVar(V_LIGHTS_ON)) {
		fileHash = 0x41983216;
		_sprite1 = insertStaticSprite(0x2201266A, 100);
		_sprite2 = insertStaticSprite(0x3406A333, 300);
		_sprite3 = insertStaticSprite(0x24A223A2, 100);
		_asDoorSpikes = insertSprite<AsScene2206DoorSpikes>(0x26133023);
		_asDoorSpikes->setClipRect(_sprite2->getDrawRect().x, 0, 640, 480);
		setRectList(0x004B8AF8);
		_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x0E038022, 100, 0);
		insertScreenMouse(0x83212411);
		_ssTestTube = insertSprite<SsScene2206TestTube>(this, 1100, /*464, 433, */0x5E00E262);
		_asPlatform = insertSprite<AsScene2206Platform>(0x085E25E0);
	} else {
		fileHash = 0xE0102A45;
		_sprite1 = insertStaticSprite(0x1C1106B8, 100);
		_sprite2 = insertStaticSprite(0x020462E0, 300);
		_sprite3 = insertStaticSprite(0x900626A2, 100);
		_asDoorSpikes = insertSprite<AsScene2206DoorSpikes>(0x544822A8);
		_asDoorSpikes->setClipRect(_sprite2->getDrawRect().x, 0, 640, 480);
		setRectList(0x004B8B58);
		_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x16882608, 100, 0);
		insertScreenMouse(0x02A41E09);
		_ssTestTube = insertSprite<SsScene2206TestTube>(this, 1100, /*464, 433, */0x52032563);
		_asPlatform = insertSprite<AsScene2206Platform>(0x317831A0);
	}

	_asPlatform->setClipRect(_sprite2->getDrawRect().x, 0, _sprite3->getDrawRect().x2(), _sprite1->getDrawRect().y2());
	setBackground(fileHash);
	setPalette(fileHash);
	addEntity(_palette);
	_palette->addBasePalette(fileHash, 0, 256, 0);
	if (!getGlobalVar(V_LIGHTS_ON))
		_palette->addPalette(0x0263D144, 0, 65, 0);
	addCollisionSprite(_ssTestTube);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2206>(200, 430);
		setMessageList(0x004B88A8);
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene2206>(640, 430);
		setMessageList(0x004B88B8);
	} else if (which == 2) {
		// Klaymen entering from the back
		insertKlaymen<KmScene2206>(205, 396);
		setMessageList(0x004B88C8);
		_palette->addPalette(getGlobalVar(V_LIGHTS_ON) ? 0xB103B604 : 0x0263D144, 0, 65, 0);
		klaymenBehindSpikes();
		playSound(0, 0x53B8284A);
	} else if (which == 3) {
		// Klaymen entering from reading a text column
		insertKlaymen<KmScene2206>(kScene2206XPositions[getGlobalVar(V_CLICKED_COLUMN_INDEX)], 430);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X))
			_klaymen->setDoDeltaX(1);
		setMessageList(0x004B8A70);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene2206>(0, 430);
		setMessageList(0x004B88B0);
	}

	_klaymen->setSoundFlag(true);
	_klaymen->setKlaymenIdleTable2();

}

Scene2206::~Scene2206() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2206::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x800C6694)
			readClickedColumn();
		else if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klaymen, 0x1014, _ssButton);
		else if (param.asInteger() == 0x11C40840) {
			if (getGlobalVar(V_SPIKES_RETRACTED))
				setMessageList(0x004B8948);
			else
				setMessageList(0x004B8970);
		}
		break;
	case 0x4803:
		sendMessage(_asPlatform, 0x4803, 0);
		break;
	case 0x480B:
		if (sender == _ssButton) {
			setGlobalVar(V_SPIKES_RETRACTED, getGlobalVar(V_SPIKES_RETRACTED) ? 0 : 1);
			if (getGlobalVar(V_SPIKES_RETRACTED))
				sendMessage(_asDoorSpikes, NM_KLAYMEN_OPEN_DOOR, 0);
			else
				sendMessage(_asDoorSpikes, NM_KLAYMEN_CLOSE_DOOR, 0);
		}
		break;
	case 0x4826:
		sendEntityMessage(_klaymen, 0x1014, _ssTestTube);
		setMessageList(0x004B8988);
		break;
	case NM_MOVE_TO_BACK:
		klaymenBehindSpikes();
		break;
	case NM_MOVE_TO_FRONT:
		klaymenInFrontSpikes();
		break;
	default:
		break;
	}
	return messageResult;
}

void Scene2206::klaymenInFrontSpikes() {
	if (getGlobalVar(V_LIGHTS_ON)) {
		_palette->addBasePalette(0x41983216, 0, 65, 0);
		_palette->startFadeToPalette(12);
	}
	setSurfacePriority(_sprite1->getSurface(), 100);
	setSurfacePriority(_sprite2->getSurface(), 300);
	setSurfacePriority(_sprite3->getSurface(), 100);
	setSurfacePriority(_asDoorSpikes->getSurface(), 200);
	_klaymen->setClipRect(0, 0, 640, 480);
}

void Scene2206::klaymenBehindSpikes() {
	if (getGlobalVar(V_LIGHTS_ON)) {
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
		_palette->startFadeToPalette(12);
	}
	setSurfacePriority(_sprite1->getSurface(), 1100);
	setSurfacePriority(_sprite2->getSurface(), 1300);
	setSurfacePriority(_sprite3->getSurface(), 1100);
	setSurfacePriority(_asDoorSpikes->getSurface(), 1200);
	_klaymen->setClipRect(_sprite2->getDrawRect().x, 0, _sprite3->getDrawRect().x2(), _sprite1->getDrawRect().y2());
}

static const uint32 kScene2206MessageIds1[] = {
	0x004B8998, 0x004B89B8, 0x004B89D8
};

static const uint32 kScene2206MessageIds2[] = {
	0x004B89F8, 0x004B8A20, 0x004B8A48
};

void Scene2206::readClickedColumn() {
	setGlobalVar(V_CLICKED_COLUMN_INDEX, (_mouseClickPos.x - 354) / 96);
	if (getGlobalVar(V_CLICKED_COLUMN_INDEX) > 2)
		setGlobalVar(V_CLICKED_COLUMN_INDEX, 2);
	setGlobalVar(V_CLICKED_COLUMN_ROW, (_mouseClickPos.y - 183) / 7);
	setGlobalVar(V_COLUMN_TEXT_NAME, calcHash("stLineagex"));
	setGlobalVar(V_COLUMN_BACK_NAME, 0);
	if (ABS(kScene2206XPositions[getGlobalVar(V_CLICKED_COLUMN_INDEX)] - _klaymen->getX()) >= 144)
		setMessageList2(kScene2206MessageIds1[getGlobalVar(V_CLICKED_COLUMN_INDEX)]);
	else
		setMessageList2(kScene2206MessageIds2[getGlobalVar(V_CLICKED_COLUMN_INDEX)]);
}

static const uint32 kScene2207FileHashes[] = {
	0x33B1E12E, 0x33D1E12E, 0x3311E12E,
	0x3291E12E, 0x3191E12E, 0x3791E12E,
	0x3B91E12E, 0x2391E12E, 0x1391E12E,
	0x3BB1E12E, 0x23B1E12E, 0x13B1E12E
};

Scene2207::Scene2207(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _klaymenAtElevator(true), _elevatorSurfacePriority(0) {

	_vm->gameModule()->initCannonSymbolsPuzzle();

	if (!getSubVar(VA_IS_PUZZLE_INIT, 0x88460852))
		setSubVar(VA_IS_PUZZLE_INIT, 0x88460852, 1);

	SetMessageHandler(&Scene2207::handleMessage);
	SetUpdateHandler(&Scene2207::update);

	insertKlaymen<KmScene2207>(0, 0);
	_klaymen->setRepl(64, 0);
	setMessageList(0x004B38E8);
	_asElevator = insertSprite<AsScene2207Elevator>(this);

	if (getGlobalVar(V_LIGHTS_ON)) {
		setBackground(0x88C00241);
		setPalette(0x88C00241);
		insertScreenMouse(0x00245884);
		_ssMaskPart1 = insertStaticSprite(0xE20A28A0, 1200);
		_ssMaskPart2 = insertStaticSprite(0x688F62A5, 1100);
		_ssMaskPart3 = insertStaticSprite(0x0043B038, 1100);
		_asTape = insertSprite<AsScene1201Tape>(this, 4, 1100, 277, 428, 0x9148A011);
		addCollisionSprite(_asTape);
		_asLever = insertSprite<AsScene2207Lever>(this, 527, 333, 0);
		addCollisionSprite(_asLever);
		_asWallRobotAnimation = insertSprite<AsScene2207WallRobotAnimation>(this);
		_asWallCannonAnimation = insertSprite<AsScene2207WallCannonAnimation>();
		_asWallRobotAnimation->setVisible(false);
		_asWallCannonAnimation->setVisible(false);
		_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x2C4061C4, 100, 0);
		_asLever->setClipRect(0, 0, _ssMaskPart3->getDrawRect().x2(), 480);
		_klaymen->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, _ssMaskPart2->getDrawRect().y2());
		_asElevator->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, _ssMaskPart2->getDrawRect().y2());
	} else {
		setGlobalVar(V_SEEN_SYMBOLS_NO_LIGHT, 1);
		setBackground(0x05C02A55);
		setPalette(0x05C02A55);
		insertScreenMouse(0x02A51054);
		_ssMaskPart1 = insertStaticSprite(0x980E46A4, 1200);
		insertSprite<SsScene2207Symbol>(kScene2207FileHashes[getSubVar(VA_GOOD_CANNON_SYMBOLS_1, 0)], 0);
		insertSprite<SsScene2207Symbol>(kScene2207FileHashes[getSubVar(VA_GOOD_CANNON_SYMBOLS_1, 1)], 1);
		insertSprite<SsScene2207Symbol>(kScene2207FileHashes[getSubVar(VA_GOOD_CANNON_SYMBOLS_1, 2)], 2);
		_asTape = NULL;
		_asLever = NULL;
		_asWallRobotAnimation = NULL;
		_asWallCannonAnimation = NULL;
		_ssButton = NULL;
		_klaymen->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, 480);
		_asElevator->setClipRect(0, _ssMaskPart1->getDrawRect().y, 640, 480);
	}

	_dataResource.load(0x00524846);
	setRectList(0x004B38B8);

	sendEntityMessage(_klaymen, 0x1014, _asElevator);
	sendMessage(_klaymen, 0x2001, 0);
	sendMessage(_asElevator, 0x2000, 480);

	loadSound(1, calcHash("fxFogHornSoft"));

}

void Scene2207::update() {
	Scene::update();
	if (_elevatorSurfacePriority != 0) {
		setSurfacePriority(_asElevator->getSurface(), _elevatorSurfacePriority);
		_elevatorSurfacePriority = 0;
	}
	if (_klaymen->getY() == 423)
		_klaymenAtElevator = _klaymen->getX() > 459 && _klaymen->getX() < 525;
}

uint32 Scene2207::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x0014F275) {
			if (_klaymenAtElevator) {
				sendMessage(_asElevator, 0x2000, _mouseClickPos.y);
				sendEntityMessage(_klaymen, 0x1014, _asElevator);
				sendMessage(_klaymen, 0x2001, 0);
			} else
				cancelMessageList();
		} else if (param.asInteger() == 0x34569073) {
			if (_klaymenAtElevator) {
				_isKlaymenBusy = true;
				sendMessage(_asElevator, 0x2000, 0);
				sendEntityMessage(_klaymen, 0x1014, _asElevator);
				sendMessage(_klaymen, 0x2001, 0);
			} else
				cancelMessageList();
		} else if (param.asInteger() == 0x4054C877) {
			if (_klaymenAtElevator) {
				sendMessage(_asElevator, 0x2000, 480);
				sendEntityMessage(_klaymen, 0x1014, _asElevator);
				sendMessage(_klaymen, 0x2001, 0);
			} else
				cancelMessageList();
		} else if (param.asInteger() == 0x0CBC6211) {
			sendEntityMessage(_klaymen, 0x1014, _asElevator);
			sendMessage(_klaymen, 0x2001, 0);
			setRectList(0x004B38B8);
		} else if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klaymen, 0x1014, _ssButton);
		else if (param.asInteger() == 0x231DA241) {
			if (_ssButton)
				setMessageList(0x004B38F0);
			else
				setMessageList(0x004B37D8);
		}
		break;
	case NM_POSITION_CHANGE:
		_elevatorSurfacePriority = param.asInteger();
		break;
	case 0x2003:
		_isKlaymenBusy = false;
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		sendMessage(_asWallRobotAnimation, NM_CAR_MOVE_TO_PREV_POINT, 0);
		sendMessage(_asWallCannonAnimation, NM_CAR_MOVE_TO_PREV_POINT, 0);
		break;
	case 0x480B:
		if (sender == _ssButton) {
			if (getSubVar(VA_LOCKS_DISABLED, 0x40119852)) {
				setSubVar(VA_LOCKS_DISABLED, 0x40119852, 0);
				playSound(0, calcHash("fx3LocksDisable"));
			} else {
				setSubVar(VA_LOCKS_DISABLED, 0x40119852, 1);
				playSound(1);
			}
		}
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		sendMessage(_asWallRobotAnimation, NM_KLAYMEN_STOP_CLIMBING, 0);
		sendMessage(_asWallCannonAnimation, NM_KLAYMEN_STOP_CLIMBING, 0);
		_asWallRobotAnimation->setVisible(true);
		_asWallCannonAnimation->setVisible(true);
		break;
	case 0x4826:
		if (sender == _asTape) {
			if (_klaymen->getY() == 423) {
				sendEntityMessage(_klaymen, 0x1014, _asTape);
				setMessageList(0x004B3958);
			}
		} else if (_klaymenAtElevator) {
			SetMessageHandler(&Scene2207::handleMessage2);
			sendMessage(_asElevator, 0x2000, 347);
			sendEntityMessage(_klaymen, 0x1014, _asElevator);
			sendMessage(_klaymen, 0x2001, 0);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 Scene2207::handleMessage2(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_POSITION_CHANGE:
		_elevatorSurfacePriority = param.asInteger();
		break;
	case 0x2004:
		SetMessageHandler(&Scene2207::handleMessage);
		sendMessage(_klaymen, NM_KLAYMEN_CLIMB_LADDER, 0);
		sendEntityMessage(_klaymen, 0x1014, _asLever);
		setMessageList(0x004B3920);
		setRectList(0x004B3948);
		break;
	default:
		break;
	}
	return messageResult;
}

static const uint32 kScene2208FileHashes1[] = {
	0x041023CB, 0x041020CB, 0x041026CB, 0x04102ACB,
	0x041032CB, 0x041002CB
};

static const uint32 kScene2208FileHashes2[] = {
	0x091206C9, 0x091406C9, 0x091806C9, 0x090006C9,
	0x093006C9, 0x095006C9
};

Scene2208::Scene2208(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _textResource(vm) {

	SpriteResource spriteResource(_vm);
	const char *textStart, *textEnd;

	if (!getGlobalVar(V_COLUMN_TEXT_NAME))
		setGlobalVar(V_COLUMN_TEXT_NAME, calcHash("stLineagex"));

	_textResource.load(getGlobalVar(V_COLUMN_TEXT_NAME));

	textStart = _textResource.getString(getGlobalVar(V_CLICKED_COLUMN_INDEX), textEnd);
	while (textStart < textEnd) {
		_strings.push_back(textStart);
		textStart += strlen(textStart) + 1;
	}

	_maxRowIndex = 8 + 10 * (3 - (getGlobalVar(V_COLUMN_TEXT_NAME) == calcHash("stLineagex") ? 1 : 0));

	_background = new Background(_vm, 0);
	_background->createSurface(0, 640, 528);
	_background->getSpriteResource().getPosition().y = 480;
	addBackground(_background);
	setPalette(0x08100289);
	addEntity(_palette);
	insertPuzzleMouse(0x0028D089, 40, 600);

	_fontSurface = FontSurface::createFontSurface(_vm, 0x0800090C);

	_backgroundSurface = new BaseSurface(_vm, 0, 640, 480, "background");
	spriteResource.load(0x08100289, true);
	_backgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	_topBackgroundSurface = new BaseSurface(_vm, 0, 640, 192, "top background");
	spriteResource.load(!getGlobalVar(V_COLUMN_BACK_NAME)
		? kScene2208FileHashes1[getGlobalVar(V_CLICKED_COLUMN_INDEX) % 6]
		: getGlobalVar(V_COLUMN_BACK_NAME), true);
	_topBackgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	_bottomBackgroundSurface = new BaseSurface(_vm, 0, 640, 192, "bottom background");
	spriteResource.load(kScene2208FileHashes2[getGlobalVar(V_CLICKED_COLUMN_INDEX) % 6], true);
	_bottomBackgroundSurface->drawSpriteResourceEx(spriteResource, false, false, 0, 0);

	SetUpdateHandler(&Scene2208::update);
	SetMessageHandler(&Scene2208::handleMessage);

	_visibleRowsCount = 10;
	_newRowIndex = (int16)getGlobalVar(V_CLICKED_COLUMN_ROW);
	if (_newRowIndex + _visibleRowsCount > _maxRowIndex)
		_newRowIndex = _maxRowIndex - _visibleRowsCount;
	if (_newRowIndex < 6)
		_newRowIndex = 0;
	_rowScrollY = 0;
	_backgroundScrollY = 48 * _newRowIndex;
	_currRowIndex = _newRowIndex;

	for (int16 rowIndex = 0; rowIndex < _visibleRowsCount; rowIndex++)
		drawRow(_newRowIndex + rowIndex);

	_background->getSurface()->getSysRect().y = _backgroundScrollY;

}

Scene2208::~Scene2208() {
	delete _fontSurface;
	delete _backgroundSurface;
	delete _topBackgroundSurface;
	delete _bottomBackgroundSurface;
}

void Scene2208::update() {

	int16 mouseY = _vm->getMouseY();

	if (mouseY < 48) {
		if (_currRowIndex > 0)
			_newRowIndex = _currRowIndex - 1;
	} else if (mouseY > 432) {
		if (_currRowIndex < _maxRowIndex - _visibleRowsCount)
			_newRowIndex = _currRowIndex + 1;
	} else {
		if (_currRowIndex > _newRowIndex)
			_newRowIndex = _currRowIndex;
	}

	if (_currRowIndex < _newRowIndex) {
		if (_rowScrollY == 0)
			drawRow(_currRowIndex + _visibleRowsCount);
		_backgroundScrollY += 4;
		_rowScrollY += 4;
		if (_rowScrollY == 48) {
			_rowScrollY = 0;
			_currRowIndex++;
		}
		_background->getSurface()->getSysRect().y = _backgroundScrollY;
	} else if (_currRowIndex > _newRowIndex || _rowScrollY > 0) {
		if (_rowScrollY == 0) {
			drawRow(_currRowIndex - 1);
			_currRowIndex--;
		}
		_backgroundScrollY -= 4;
		if (_rowScrollY == 0)
			_rowScrollY = 48;
		_rowScrollY -= 4;
		_background->getSurface()->getSysRect().y = _backgroundScrollY;
	}

	Scene::update();

}

uint32 Scene2208::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 40 || param.asPoint().x >= 600)
			leaveScene(0);
		break;
	default:
		break;
	}
	return messageResult;
}

void Scene2208::drawRow(int16 rowIndex) {
	NDrawRect sourceRect;
	int16 y = (rowIndex * 48) % 528;
	if (rowIndex < 4) {
		sourceRect.x = 0;
		sourceRect.y = y;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_topBackgroundSurface->getSurface(), 0, y, sourceRect);
	} else if (rowIndex > _maxRowIndex - 5) {
		sourceRect.x = 0;
		sourceRect.y = (rowIndex - _maxRowIndex + 4) * 48;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_bottomBackgroundSurface->getSurface(), 0, y, sourceRect);
	} else {
		rowIndex -= 4;
		sourceRect.x = 0;
		sourceRect.y = (rowIndex * 48) % 480;
		sourceRect.width = 640;
		sourceRect.height = 48;
		_background->getSurface()->copyFrom(_backgroundSurface->getSurface(), 0, y, sourceRect);
		if (rowIndex < (int)_strings.size()) {
			const char *text = _strings[rowIndex];
			_fontSurface->drawString(_background->getSurface(), 95, y, (const byte*)text);
		}
	}
}

static const int16 kScene2242XPositions[] = {
	68, 158
};

static const uint32 kScene2242MessageListIds2[] = {
	0x004B3CB8, 0x004B3CD8
};

static const uint32 kScene2242MessageListIds1[] = {
	0x004B3CF8, 0x004B3D20
};

Scene2242::Scene2242(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isKlaymenInLight(false) {

	SetMessageHandler(&Scene2242::handleMessage);
	SetUpdateHandler(&Scene2242::update);

	if (getGlobalVar(V_LIGHTS_ON)) {
		setBackground(0x11840E24);
		setPalette(0x11840E24);
		insertScreenMouse(0x40E20110);
		setRectList(0x004B3DC8);
	} else {
		setBackground(0x25848E24);
		setPalette(0x25848E24);
		addEntity(_palette);
		_palette->copyBasePalette(0, 256, 0);
		_palette->addPalette(0x68033B1C, 0, 65, 0);
		insertScreenMouse(0x48E20250);
		setRectList(0x004B3E18);
	}

	_asTape = insertSprite<AsScene1201Tape>(this, 10, 1100, 464, 435, 0x9148A011);
	addCollisionSprite(_asTape);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2242>(200, 430);
		setMessageList(0x004B3C18);
	} else if (which == 1) {
		// Klaymen entering from looking through the window
		insertKlaymen<KmScene2242>(530, 430);
		setMessageList(0x004B3D60);
	} else if (which == 2) {
		// Klaymen returning from reading a text column
		insertKlaymen<KmScene2242>(kScene2242XPositions[!getGlobalVar(V_CLICKED_COLUMN_INDEX) ? 0 : 1], 430);
		setMessageList(0x004B3D48);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X))
			_klaymen->setDoDeltaX(1);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene2242>(0, 430);
		setMessageList(0x004B3C20);
	}

	_klaymen->setSoundFlag(true);

}

Scene2242::~Scene2242() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

void Scene2242::update() {
	if (!getGlobalVar(V_LIGHTS_ON)) {
		if (_isKlaymenInLight && _klaymen->getX() < 440) {
			_palette->addBasePalette(0x68033B1C, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymenInLight = false;
		} else if (!_isKlaymenInLight && _klaymen->getX() >= 440) {
			_palette->addBasePalette(0x25848E24, 0, 65, 0);
			_palette->startFadeToPalette(12);
			_isKlaymenInLight = true;
		}
	}
	Scene::update();
}

uint32 Scene2242::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x800C6694)
			readClickedColumn();
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004B3D50);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void Scene2242::readClickedColumn() {
	int index;
	if (_mouseClickPos.x < 108) {
		setGlobalVar(V_COLUMN_TEXT_NAME, 0x04290188);
		setGlobalVar(V_CLICKED_COLUMN_INDEX, 42);
		setGlobalVar(V_COLUMN_BACK_NAME, calcHash("bgRecPanelStart1"));
		index = 0;
	} else {
		setGlobalVar(V_COLUMN_TEXT_NAME, 0x04290188);
		setGlobalVar(V_CLICKED_COLUMN_INDEX, 43);
		setGlobalVar(V_COLUMN_BACK_NAME, calcHash("bgRecPanelStart2"));
		index = 1;
	}
	setGlobalVar(V_CLICKED_COLUMN_ROW, (_mouseClickPos.y - 100) / 7);
	if (ABS(_klaymen->getX() - kScene2242XPositions[index]) < 133)
		setMessageList2(kScene2242MessageListIds1[index]);
	else
		setMessageList2(kScene2242MessageListIds2[index]);
}

static const int16 kHallOfRecordsKlaymenXPos[] = {
	 68, 157, 246, 335,
	424, 513, 602
};

static const uint32 kHallOfRecordsSceneMessageListIds2[] = {
	0x004B2978,	0x004B2998,	0x004B29B8,	0x004B29D8,
	0x004B29F8,	0x004B2A18,	0x004B2A38
};

static const uint32 kHallOfRecordsSceneMessageListIds1[] = {
	0x004B2A58,	0x004B2A80,	0x004B2AA8,	0x004B2AD0,
	0x004B2AF8,	0x004B2B20,	0x004B2B48
};

HallOfRecordsScene::HallOfRecordsScene(NeverhoodEngine *vm, Module *parentModule, int which, uint32 hallOfRecordsInfoId)
	: Scene(vm, parentModule) {

	_hallOfRecordsInfo = _vm->_staticData->getHallOfRecordsInfoItem(hallOfRecordsInfoId);

	SetMessageHandler(&HallOfRecordsScene::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (!getGlobalVar(V_LIGHTS_ON) && _hallOfRecordsInfo->bgFilename2) {
		setRectList(0x004B2BF8);
		setBackground(_hallOfRecordsInfo->bgFilename2);
		setPalette(_hallOfRecordsInfo->bgFilename2);
		insertScreenMouse(0x14320138);
	} else {
		setRectList(0x004B2BB8);
		setBackground(_hallOfRecordsInfo->bgFilename1);
		setPalette(_hallOfRecordsInfo->bgFilename1);
		insertScreenMouse(0x63A40028);
	}

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmHallOfRecords>(200, 430);
		setMessageList(0x004B2900);
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmHallOfRecords>(640, 430);
		setMessageList(0x004B2910);
	} else if (which == 2) {
		// Klaymen returning from reading a text column
		insertKlaymen<KmHallOfRecords>(kHallOfRecordsKlaymenXPos[getGlobalVar(V_CLICKED_COLUMN_INDEX) - _hallOfRecordsInfo->xPosIndex], 430);
		setMessageList(0x004B2B70);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X))
			_klaymen->setDoDeltaX(1);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmHallOfRecords>(0, 430);
		setMessageList(0x004B2908);
	}

	_klaymen->setSoundFlag(true);
	_klaymen->setKlaymenIdleTable2();

}

HallOfRecordsScene::~HallOfRecordsScene() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 HallOfRecordsScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x800C6694)
			readClickedColumn();
		break;
	default:
		break;
	}
	return messageResult;
}

void HallOfRecordsScene::readClickedColumn() {
	int16 index = (_mouseClickPos.x - 23) / 89;
	if (index >= _hallOfRecordsInfo->count)
		setMessageList2(0x004B2920);
	else {
		setGlobalVar(V_CLICKED_COLUMN_INDEX, _hallOfRecordsInfo->xPosIndex + index);
		setGlobalVar(V_CLICKED_COLUMN_ROW, (_mouseClickPos.y - 100) / 7);
		setGlobalVar(V_COLUMN_TEXT_NAME, _hallOfRecordsInfo->txFilename);
		if (index == 0 && _hallOfRecordsInfo->bgFilename3)
			setGlobalVar(V_COLUMN_BACK_NAME, _hallOfRecordsInfo->bgFilename3);
		else
			setGlobalVar(V_COLUMN_BACK_NAME, 0);
		if (ABS(_klaymen->getX() - kHallOfRecordsKlaymenXPos[index]) < 133)
			setMessageList2(kHallOfRecordsSceneMessageListIds1[index]);
		else
			setMessageList2(kHallOfRecordsSceneMessageListIds2[index]);
	}
}

static const int16 kScene2247XPositions[] = {
	513, 602
};

static const uint32 kScene2247MessageListIds2[] = {
	0x004B54A0,	0x004B54C0
};

static const uint32 kScene2247MessageListIds1[] = {
	0x004B54E0,	0x004B5508
};

Scene2247::Scene2247(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene2247::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (getGlobalVar(V_LIGHTS_ON)) {
		setRectList(0x004B5588);
		setBackground(0x40339414);
		setPalette(0x40339414);
		insertScreenMouse(0x3941040B);
	} else {
		setRectList(0x004B55C8);
		setBackground(0x071963E5);
		setPalette(0x071963E5);
		insertScreenMouse(0x14320138);
	}

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2247>(200, 430);
		setMessageList(0x004B5428);
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene2247>(640, 430);
		setMessageList(0x004B5438);
	} else if (which == 2) {
		// Klaymen returning from reading a text column
		insertKlaymen<KmScene2247>(kScene2247XPositions[getGlobalVar(V_COLUMN_TEXT_NAME) == 0x0008E486 ? 0 : 1], 430);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X))
			_klaymen->setDoDeltaX(1);
		setMessageList(0x004B5530);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene2247>(0, 430);
		setMessageList(0x004B5430);
	}

	_klaymen->setSoundFlag(true);

}

Scene2247::~Scene2247() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2247::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x800C6694)
			readClickedColumn();
		break;
	default:
		break;
	}
	return messageResult;
}

void Scene2247::readClickedColumn() {
	int index;
	if (_mouseClickPos.x < 553) {
		setGlobalVar(V_COLUMN_TEXT_NAME, 0x0008E486);
		setGlobalVar(V_COLUMN_BACK_NAME, calcHash("bgFatherHeader"));
		index = 0;
	} else {
		setGlobalVar(V_COLUMN_TEXT_NAME, 0x03086004);
		setGlobalVar(V_COLUMN_BACK_NAME, calcHash("bgQuaterHeader"));
		index = 1;
	}
	setGlobalVar(V_CLICKED_COLUMN_INDEX, 0);
	setGlobalVar(V_CLICKED_COLUMN_ROW, (_mouseClickPos.y - 100) / 7);
	if (ABS(_klaymen->getX() - kScene2247XPositions[index]) < 133)
		setMessageList2(kScene2247MessageListIds1[index]);
	else
		setMessageList2(kScene2247MessageListIds2[index]);
}

} // End of namespace Neverhood
