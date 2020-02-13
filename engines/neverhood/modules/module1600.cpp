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

#include "neverhood/gamemodule.h"
#include "neverhood/modules/module1200_sprites.h"
#include "neverhood/modules/module1600.h"
#include "neverhood/modules/module1600_sprites.h"
#include "neverhood/modules/module2200_sprites.h"
#include "neverhood/modules/module3000_sprites.h"

namespace Neverhood {

static const uint32 kModule1600SoundList[] = {
	0x90805C50, 0x90804450, 0xB4005E60,
	0x91835066, 0x90E14440, 0
};

Module1600::Module1600(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 1)
		createScene(4, 1);
	else if (which == 2)
		createScene(5, 0);
	else if (which == 3)
		createScene(6, 1);
	else if (which == 4)
		createScene(1, 0);
	else
		createScene(0, 0);

	_vm->_soundMan->addSoundList(0x1A008D8, kModule1600SoundList);
	_vm->_soundMan->setSoundListParams(kModule1600SoundList, true, 50, 600, 5, 150);
	_vm->_soundMan->playTwoSounds(0x1A008D8, 0x41861371, 0x43A2507F, 0);

}

Module1600::~Module1600() {
	_vm->_soundMan->deleteGroup(0x1A008D8);
}

void Module1600::createScene(int sceneNum, int which) {
	debug(1, "Module1600::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		createNavigationScene(0x004B39D0, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		createNavigationScene(0x004B3A30, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		createNavigationScene(0x004B3A60, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		createNavigationScene(0x004B3A90, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		createNavigationScene(0x004B3B20, which);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		createNavigationScene(0x004B3B50, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		createNavigationScene(0x004B3B80, which);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_childObject = new Scene1608(_vm, this, which);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_childObject = new Scene1609(_vm, this);
		break;
	case 1001:
		_vm->gameState().sceneNum = 1;
		if (getGlobalVar(V_TALK_COUNTING_INDEX) == 1)
			createSmackerScene(0x80050200, true, true, false);
		else if (getGlobalVar(V_TALK_COUNTING_INDEX) == 2)
			createSmackerScene(0x80090200, true, true, false);
		else
			createSmackerScene(0x80000200, true, true, false);
		if (getGlobalVar(V_TALK_COUNTING_INDEX) >= 2)
			setGlobalVar(V_TALK_COUNTING_INDEX, 0);
		else
			incGlobalVar(V_TALK_COUNTING_INDEX, +1);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1600::updateScene);
	_childObject->handleUpdate();
}

void Module1600::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 0)
				createScene(2, 0);
			else if (_moduleResult == 1)
				createScene(1, 0);
			else if (_moduleResult == 2)
				leaveModule(4);
			break;
		case 1:
			if (_moduleResult == 0)
				createScene(1001, -1);
			else if (_moduleResult == 1)
				createScene(0, 3);
			break;
		case 2:
			if (_moduleResult == 0)
				createScene(3, 0);
			else if (_moduleResult == 1)
				createScene(0, 2);
			break;
		case 3:
			if (_moduleResult == 0)
				createScene(5, 0);
			else if (_moduleResult == 2)
				createScene(6, 0);
			else if (_moduleResult == 3)
				createScene(2, 1);
			else if (_moduleResult == 4)
				createScene(4, 0);
			break;
		case 4:
			if (_moduleResult == 0)
				leaveModule(1);
			else if (_moduleResult == 1)
				createScene(3, 1);
			break;
		case 5:
			if (_moduleResult == 0)
				leaveModule(2);
			else if (_moduleResult == 1)
				createScene(3, 3);
			break;
		case 6:
			if (_moduleResult == 0)
				createScene(8, -1);
			else if (_moduleResult == 1)
				createScene(3, 5);
			break;
		case 7:
			createScene(6, 1);
			break;
		case 8:
			if (_moduleResult == 0)
				createScene(6, 0);
			else
				createScene(7, 0);
			break;
		case 1001:
			createScene(1, 0);
			break;
		default:
			break;
		}
	}
}

Scene1608::Scene1608(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _asCar(NULL), _countdown1(0) {

	setGlobalVar(V_CAR_DELTA_X, 1);

	SetMessageHandler(&Scene1608::hmLowerFloor);

	_asKey = insertSprite<AsCommonKey>(this, 1, 1100, 198, 220);
	addCollisionSprite(_asKey);

	if (which < 0) {
		// Restoring game
		if (_vm->gameState().which == 1)
			// Klaymen is in the car
			which = 1;
		else {
			// Klaymen is standing around
			setRectList(0x004B47D0);
			insertKlaymen<KmScene1608>(380, 438);
			_kmScene1608 = _klaymen;
			_klaymenInCar = false;
			_sprite1 = insertStaticSprite(0x7D0404E8, 1100);
			setMessageList(0x004B46A8);
			setBackground(0x10080E01);
			setPalette(0x10080E01);
			_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
			addCollisionSprite(_asTape);
			_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
			SetUpdateHandler(&Scene1608::upLowerFloor);
			insertScreenMouse(0x80E05108);
			insertStaticSprite(0x4B18F868, 1200);
		}
	} else if (which == 0) {
		// Klaymen entering from the left
		_vm->gameState().which = 0;
		setRectList(0x004B47D0);
		insertKlaymen<KmScene1608>(0, 438);
		_kmScene1608 = _klaymen;
		_klaymenInCar = false;
		setMessageList(0x004B46B0);
		setBackground(0x10080E01);
		setPalette(0x10080E01);
		_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
		addCollisionSprite(_asTape);
		insertScreenMouse(0x80E05108);
		_sprite1 = insertStaticSprite(0x7D0404E8, 1100);
		_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
		SetUpdateHandler(&Scene1608::upLowerFloor);
		insertStaticSprite(0x4B18F868, 1200);
	} else if (which == 2) {
		// Klaymen returning from looking through the upper window
		_vm->gameState().which = 1;
		_dataResource.load(0x003C0492);
		_roomPathPoints = _dataResource.getPointArray(calcHash("meArchroArchRoomPath"));
		setBackground(0x98001604);
		setPalette(0x98001604);
		_palette->addPalette("paPodRed", 65, 31, 65);
		insertScreenMouse(0x01600988);
		_sprite2 = insertStaticSprite(0x491F38A8, 1100);
		_asCar = createSprite<AsCommonCar>(this, 375, 227); // Create but don't add to the sprite list yet
		_asIdleCarLower = insertSprite<AsCommonIdleCarLower>(375, 227);
		_asIdleCarFull = insertSprite<AsCommonIdleCarFull>(375, 227);
		_asCar->setVisible(false);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene1608>(373, 220);
			_klaymen->setDoDeltaX(1);
		} else
			insertKlaymen<KmScene1608>(283, 220);
		_kmScene1608 = _klaymen;
		setMessageList(0x004B47A8);
		SetMessageHandler(&Scene1608::hmUpperFloor);
		SetUpdateHandler(&Scene1608::upUpperFloor);
		_asCar->setPathPoints(_roomPathPoints);
		sendMessage(_asCar, NM_POSITION_CHANGE, _roomPathPoints->size() - 1);
		_sprite3 = insertStaticSprite(0xB47026B0, 1100);
		_clipRect1.set(_sprite3->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_clipRect3.set(_sprite2->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_clipRect2 = _clipRect1;
		_clipRect2.y2 = 215;
		_klaymen->setClipRect(_clipRect1);
		_asCar->setClipRect(_clipRect1);
		_asIdleCarLower->setClipRect(_clipRect1);
		_asIdleCarFull->setClipRect(_clipRect1);
		_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
		addCollisionSprite(_asTape);
		insertSprite<AsCommonCarConnector>(_asCar)->setClipRect(_clipRect1);
		_klaymenInCar = false;
		_carClipFlag = false;
		_carStatus = 0;
		setRectList(0x004B4810);
	}

	// NOTE: Not in the else because 'which' is set to 1 in the true branch
	if (which == 1) {
		// Klaymen riding the car
		_vm->gameState().which = 1;
		_dataResource.load(0x003C0492);
		_roomPathPoints = _dataResource.getPointArray(calcHash("meArchroArchRoomPath"));
		setBackground(0x98001604);
		setPalette(0x98001604);
		_palette->addPalette("paPodRed", 65, 31, 65);
		insertScreenMouse(0x01600988);
		_asCar = insertSprite<AsCommonCar>(this, 375, 227);
		_asIdleCarLower = insertSprite<AsCommonIdleCarLower>(375, 227);
		_asIdleCarFull = insertSprite<AsCommonIdleCarFull>(375, 227);
		_sprite2 = insertStaticSprite(0x491F38A8, 1100);
		_kmScene1608 = createSprite<KmScene1608>(this, 439, 220);
		sendMessage(_kmScene1608, 0x2032, 1);
		_kmScene1608->setDoDeltaX(1);
		SetMessageHandler(&Scene1608::hmRidingCar);
		SetUpdateHandler(&Scene1608::upRidingCar);
		_asIdleCarLower->setVisible(false);
		_asIdleCarFull->setVisible(false);
		_asCar->setPathPoints(_roomPathPoints);
		sendMessage(_asCar, NM_POSITION_CHANGE, 0);
		sendMessage(_asCar, NM_CAR_MOVE_TO_NEXT_POINT, 90);
		_sprite3 = insertStaticSprite(0xB47026B0, 1100);
		_clipRect1.set(_sprite3->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_clipRect3.set(_sprite2->getDrawRect().x, _sprite3->getDrawRect().y, 640, _sprite2->getDrawRect().y2());
		_clipRect2 = _clipRect1;
		_clipRect2.y2 = 215;
		_kmScene1608->setClipRect(_clipRect1);
		_asCar->setClipRect(_clipRect3);
		_asIdleCarLower->setClipRect(_clipRect1);
		_asIdleCarFull->setClipRect(_clipRect1);
		_asTape = insertSprite<AsScene1201Tape>(this, 13, 1100, 412, 443, 0x9148A011);
		// ... addCollisionSprite(_asTape);
		insertSprite<AsCommonCarConnector>(_asCar)->setClipRect(_clipRect1);
		_klaymenInCar = true;
		_carClipFlag = true;
		_carStatus = 0;
	}

	_palette->addPalette("paKlayRed", 0, 64, 0);

}

Scene1608::~Scene1608() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _kmScene1608->isDoDeltaX() ? 1 : 0);
	if (_klaymenInCar)
		delete _kmScene1608;
	else
		delete _asCar;
}

void Scene1608::upLowerFloor() {
	Scene::update();
	if (_countdown1 != 0 && (--_countdown1 == 0))
		leaveScene(0);
}

void Scene1608::upUpperFloor() {
	Scene::update();
	if (_carStatus == 1) {
		removeSurface(_klaymen->getSurface());
		removeEntity(_klaymen);
		addSprite(_asCar);
		_klaymenInCar = true;
		clearRectList();
		SetUpdateHandler(&Scene1608::upCarAtHome);
		SetMessageHandler(&Scene1608::hmCarAtHome);
		_asIdleCarLower->setVisible(false);
		_asIdleCarFull->setVisible(false);
		_asCar->setVisible(true);
		sendMessage(_asCar, NM_CAR_ENTER, 0);
		_asCar->handleUpdate();
		_klaymen = NULL;
		_carStatus = 0;
	}
	updateKlaymenCliprect();
}

void Scene1608::upCarAtHome() {
	Scene::update();
	if (_mouseClicked) {
		if (_mouseClickPos.x <= 329 && _asCar->getX() == 375 && _asCar->getY() == 227) {
			sendMessage(_asCar, NM_CAR_LEAVE, 0);
			SetUpdateHandler(&Scene1608::upGettingOutOfCar);
		} else {
			sendPointMessage(_asCar, 0x2004, _mouseClickPos);
			SetMessageHandler(&Scene1608::hmRidingCar);
			SetUpdateHandler(&Scene1608::upRidingCar);
		}
		_mouseClicked = false;
	}
	updateKlaymenCliprect();
}

void Scene1608::upGettingOutOfCar() {
	Scene::update();
	if (_carStatus == 2) {
		_klaymen = _kmScene1608;
		removeSurface(_asCar->getSurface());
		removeEntity(_asCar);
		addSprite(_klaymen);
		_klaymenInCar = false;
		SetMessageHandler(&Scene1608::hmUpperFloor);
		SetUpdateHandler(&Scene1608::upUpperFloor);
		setRectList(0x004B4810);
		_asIdleCarLower->setVisible(true);
		_asIdleCarFull->setVisible(true);
		_asCar->setVisible(false);
		setMessageList(0x004B4748);
		processMessageList();
		_klaymen->handleUpdate();
		_carStatus = 0;
	}
	updateKlaymenCliprect();
}

void Scene1608::upRidingCar() {
	Scene::update();
	if (_mouseClicked) {
		sendPointMessage(_asCar, 0x2004, _mouseClickPos);
		_mouseClicked = false;
	}
	if (_asCar->getY() < 330) {
		if (_carClipFlag) {
			_carClipFlag = false;
			_asCar->setClipRect(_clipRect1);
			if (!_asCar->isDoDeltaX())
				sendMessage(_asCar, NM_CAR_TURN, 0);
		}
	} else if (!_carClipFlag) {
		_carClipFlag = true;
		_asCar->setClipRect(_clipRect3);
	}
}

uint32 Scene1608::hmLowerFloor(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x20250B1A) {
			clearRectList();
			_klaymen->setVisible(false);
			showMouse(false);
			_sprite1->setVisible(false);
			//sendMessage(_asDoor, NM_KLAYMEN_CLOSE_DOOR, 0); // Play sound?
			_countdown1 = 28;
		}
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_kmScene1608, 0x1014, _asTape);
			setMessageList(0x004B4770);
		} else if (sender == _asKey)
			setMessageList(0x004B46C8);
		break;
	default:
		break;
	}
	return 0;
}

uint32 Scene1608::hmUpperFloor(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x60842040)
			_carStatus = 1;
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	case 0x4826:
		if (sender == _asKey) {
			sendEntityMessage(_kmScene1608, 0x1014, _asKey);
			setMessageList(0x004B4760);
		}
		break;
	default:
		break;
	}
	return 0;
}

uint32 Scene1608::hmRidingCar(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_CLIMB_LADDER:
		leaveScene(1);
		break;
	case NM_KLAYMEN_STOP_CLIMBING:
		SetMessageHandler(&Scene1608::hmCarAtHome);
		SetUpdateHandler(&Scene1608::upCarAtHome);
		sendMessage(_asCar, NM_CAR_AT_HOME, 1);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	default:
		break;
	}
	return 0;
}

uint32 Scene1608::hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_CAR_LEAVE:
		_carStatus = 2;
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	default:
		break;
	}
	return 0;
}

void Scene1608::updateKlaymenCliprect() {
	if (_kmScene1608->getX() <= 375)
		_kmScene1608->setClipRect(_clipRect1);
	else
		_kmScene1608->setClipRect(_clipRect2);
}

Scene1609::Scene1609(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _countdown1(1), _currentSymbolIndex(0), _symbolPosition(0), _changeCurrentSymbol(true), _isSolved(false) {

	_vm->gameModule()->initCodeSymbolsPuzzle();
	_noisySymbolIndex = getGlobalVar(V_NOISY_SYMBOL_INDEX);

	SetMessageHandler(&Scene1609::handleMessage);
	SetUpdateHandler(&Scene1609::update);

	setBackground(0x92124A14);
	setPalette(0x92124A14);
	insertPuzzleMouse(0x24A10929, 20, 620);

	for (int symbolPosition = 0; symbolPosition < 12; symbolPosition++)
		_asSymbols[symbolPosition] = insertSprite<AsScene3011Symbol>(symbolPosition, false);

	_ssButton = insertSprite<SsScene3011Button>(this, true);
	addCollisionSprite(_ssButton);
	loadSound(0, 0x68E25540);

}

void Scene1609::update() {
	if (!_isSolved && _countdown1 != 0 && (--_countdown1 == 0)) {
		if (_changeCurrentSymbol) {
			_currentSymbolIndex++;
			if (_currentSymbolIndex >= 12)
				_currentSymbolIndex = 0;
			_asSymbols[_symbolPosition]->change(_currentSymbolIndex + 12, _currentSymbolIndex == (int)getSubVar(VA_CODE_SYMBOLS, _noisySymbolIndex));
			_changeCurrentSymbol = false;
			_countdown1 = 36;
		} else {
			_asSymbols[_symbolPosition]->hide();
			_changeCurrentSymbol = true;
			_countdown1 = 12;
		}
	}
	if (_isSolved && !isSoundPlaying(0))
		leaveScene(1);
	Scene::update();
}

uint32 Scene1609::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	case NM_ANIMATION_UPDATE:
		if (!_isSolved) {
			if (_changeCurrentSymbol)
				_asSymbols[_symbolPosition]->change(_currentSymbolIndex + 12, false);
			_asSymbols[_symbolPosition]->stopSymbolSound();
			_symbolPosition++;
			if (_symbolPosition >= 12) {
				if (testVars()) {
					playSound(0);
					setGlobalVar(V_CODE_SYMBOLS_SOLVED, 1);
					_isSolved = true;
				} else {
					_symbolPosition = 0;
					for (int i = 0; i < 12; i++)
						_asSymbols[i]->hide();
				}
			}
			_changeCurrentSymbol = true;
			_countdown1 = 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

bool Scene1609::testVars() {
	int cmpSymbolIndex = 0;

	// Find the position of the first symbol
	while ((int)getSubVar(VA_CODE_SYMBOLS, cmpSymbolIndex) != _asSymbols[0]->getSymbolIndex())
		cmpSymbolIndex++;

	// Check if the entered symbols match
	for (int enteredSymbolIndex = 0; enteredSymbolIndex < 12; enteredSymbolIndex++) {
		if ((int)getSubVar(VA_CODE_SYMBOLS, cmpSymbolIndex) != _asSymbols[enteredSymbolIndex]->getSymbolIndex())
			return false;
		cmpSymbolIndex++;
		if (cmpSymbolIndex >= 12)
			cmpSymbolIndex = 0;
	}

	return true;
}

} // End of namespace Neverhood
