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

#include "neverhood/modules/module1600.h"	// for Scene1608
#include "neverhood/modules/module1600_sprites.h"
#include "neverhood/modules/module2500.h"
#include "neverhood/modules/module2500_sprites.h"
#include "neverhood/modules/module2700.h"	// for Scene2704
#include "neverhood/modules/module2700_sprites.h"

namespace Neverhood {

static const uint32 kScene2505StaticSprites[] = {
	0x4000A226, 0
};

static const NRect kScene2505ClipRect = { 0, 0, 564, 480 };

static const uint32 kScene2506StaticSprites[] = {
	0x4027AF02, 0
};

static const NRect kScene2506ClipRect = { 0, 0, 640, 441 };

static const uint32 kScene2508StaticSprites1[] = {
	0x2F08E610, 0xD844E6A0, 0
};

static const NRect kScene2508ClipRect1 = { 0, 0, 594, 448 };

static const uint32 kScene2508StaticSprites2[] = {
	0x2F08E610, 0
};

static const NRect kScene2508ClipRect2 = { 0, 0, 594, 448 };

Module2500::Module2500(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundIndex(0) {

	_vm->_soundMan->addMusic(0x29220120, 0x05343184);
	_vm->_soundMan->startMusic(0x05343184, 0, 0);
	SetMessageHandler(&Module2500::handleMessage);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, _vm->gameState().which);
	else
		createScene(0, 0);

	loadSound(0, 0x00880CCC);
	loadSound(1, 0x00880CC0);
	loadSound(2, 0x00880CCC);
	loadSound(3, 0x00880CC0);

}

Module2500::~Module2500() {
	_vm->_soundMan->deleteMusicGroup(0x29220120);
}

void Module2500::createScene(int sceneNum, int which) {
	debug(1, "Module2500::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene2501(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_vm->gameState().which = which;
		createScene2704(which, 0x004B01B8, 220);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->gameState().which = which;
		if (getGlobalVar(V_WORLDS_JOINED))
			createScene2704(which, 0x004B01E0, 150);
		else
			createScene2704(which, 0x004B0208, 150);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_childObject = new Scene2504(_vm, this, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->gameState().which = which;
		createScene2704(which, 0x004B0230, 150, kScene2505StaticSprites, &kScene2505ClipRect);
		break;
	case 5:
		setGlobalVar(V_CAR_DELTA_X, 1);
		_vm->gameState().sceneNum = 5;
		_vm->gameState().which = which;
		createScene2704(which, 0x004B0268, 150, kScene2506StaticSprites, &kScene2506ClipRect);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_vm->gameState().which = which;
		createScene2704(which, 0x004B02A0, 150);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_vm->gameState().which = which;
		if (getGlobalVar(V_ENTRANCE_OPEN))
			createScene2704(which, 0x004B02C8, 150, kScene2508StaticSprites1, &kScene2508ClipRect1);
		else
			createScene2704(which, 0x004B02C8, 150, kScene2508StaticSprites2, &kScene2508ClipRect2);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_childObject = new Scene1608(_vm, this, which);
		break;
	case 9:
		_vm->gameState().sceneNum = 9;
		if (getGlobalVar(V_ENTRANCE_OPEN))
			createStaticScene(0xC62A0645, 0xA0641C6A);
		else
			createStaticScene(0x7A343546, 0x435427AB);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2500::updateScene);
	_childObject->handleUpdate();
}

void Module2500::updateScene() {
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
			if (_moduleResult == 1)
				createScene(3, -1);
			else
				createScene(0, 2);
			break;
		case 2:
			if (_moduleResult == 1)
				createScene(4, 0);
			else
				createScene(0, 1);
			break;
		case 3:
			createScene(1, 1);
			break;
		case 4:
			if (_moduleResult == 1)
				createScene(5, 0);
			else
				createScene(2, 1);
			break;
		case 5:
			if (_moduleResult == 1)
				createScene(6, 0);
			else
				createScene(4, 1);
			break;
		case 6:
			if (_moduleResult == 1)
				createScene(7, 0);
			else
				createScene(5, 1);
			break;
		case 7:
			if (_moduleResult == 1)
				createScene(8, 1);
			else
				createScene(6, 1);
			break;
		case 8:
			if (_moduleResult == 2)
				createScene(9, -1);
			else
				createScene(7, 1);
			break;
		case 9:
			createScene(8, 2);
			break;
		default:
			break;
		}
	}
}

uint32 Module2500::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x200D:
		playSound(_soundIndex);
		_soundIndex++;
		if (_soundIndex >= 4)
			_soundIndex = 0;
		break;
	default:
		break;
	}
	return messageResult;
}

void Module2500::createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites, const NRect *clipRect) {
	_childObject = new Scene2704(_vm, this, which, sceneInfoId, value, staticSprites, clipRect);
}

Scene2501::Scene2501(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	_tracks.push_back(_vm->_staticData->getTrackInfo(0x004B2628));
	_tracks.push_back(_vm->_staticData->getTrackInfo(0x004B264C));
	_tracks.push_back(_vm->_staticData->getTrackInfo(0x004B2670));

	setGlobalVar(V_CAR_DELTA_X, 1);
	SetUpdateHandler(&Scene2501::update);
	setBackground(0x1B8E8115);
	setPalette(0x1B8E8115);
	_palette->addPalette(0x00128842, 65, 31, 65);
	_palette->addPalette("paKlayRed", 0, 64, 0);
	insertScreenMouse(0xE81111B0);

	_ssTrackShadowBackground = createSprite<SsCommonTrackShadowBackground>(0x99BE9015); // Don't add this to the sprite list
	addEntity(_ssTrackShadowBackground);
	_asCar = createSprite<AsCommonCar>(this, 211, 400); // Create but don't add to the sprite list yet
	_asIdleCarLower = insertSprite<AsCommonIdleCarLower>(211, 400);
	_asIdleCarFull = insertSprite<AsCommonIdleCarFull>(211, 400);
	insertStaticSprite(0xC42AC521, 1500);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2501>(162, 393);
		_kmScene2501 = _klaymen;
		_klaymenInCar = false;
		setMessageList(0x004B2538);
		setRectList(0x004B2608);
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		sendMessage(_asCar, NM_CAR_ENTER, 0);
		_asCar->setVisible(false);
		_currTrackIndex = 0;
	} else if (which == 1 || which == 2) {
		// 1: Klaymen entering riding the car on the left track
		// 2: Klaymen entering riding the car on the bottom track
		addSprite(_asCar);
		_kmScene2501 = (Klaymen*)new KmScene2501(_vm, this, 275, 393);
		_klaymenInCar = true;
		sendMessage(_kmScene2501, 0x2000, 1);
		_kmScene2501->setDoDeltaX(1);
		SetMessageHandler(&Scene2501::hmRidingCar);
		SetUpdateHandler(&Scene2501::upRidingCar);
		_asIdleCarLower->setVisible(false);
		_asIdleCarFull->setVisible(false);
		_currTrackIndex = which;
	} else {
		// Klaymen entering the car
		insertKlaymen<KmScene2501>(162, 393);
		_kmScene2501 = _klaymen;
		_klaymenInCar = false;
		setMessageList(0x004B2538);
		setRectList(0x004B2608);
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		sendMessage(_asCar, NM_CAR_ENTER, 0);
		_asCar->setVisible(false);
		_currTrackIndex = 0;
	}

	_asCarShadow = insertSprite<AsCommonCarShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarTrackShadow = insertSprite<AsCommonCarTrackShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarConnectorShadow = insertSprite<AsCommonCarConnectorShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	insertSprite<AsCommonCarConnector>(_asCar);

	_newTrackIndex = -1;
	_dataResource.load(calcHash("Ashooded"));

	_trackPoints = _dataResource.getPointArray(_tracks[_currTrackIndex]->trackPointsName);
	_asCar->setPathPoints(_trackPoints);

	if (which >= 0 && _tracks[_currTrackIndex]->which2 == which) {
		NPoint testPoint = (*_trackPoints)[_trackPoints->size() - 1];
		sendMessage(_asCar, NM_POSITION_CHANGE, _trackPoints->size() - 1);
		if (testPoint.x < 0 || testPoint.x >= 640 || testPoint.y < 0 || testPoint.y >= 480)
			sendMessage(_asCar, NM_CAR_MOVE_TO_PREV_POINT, 150);
	} else {
		NPoint testPoint = (*_trackPoints)[0];
		sendMessage(_asCar, NM_POSITION_CHANGE, 0);
		if (testPoint.x < 0 || testPoint.x >= 640 || testPoint.y < 0 || testPoint.y >= 480)
			sendMessage(_asCar, NM_CAR_MOVE_TO_NEXT_POINT, 150);
	}

	_carStatus = 0;

}

Scene2501::~Scene2501() {
	// Free sprites not currently in the sprite list
	if (_klaymenInCar)
		delete _kmScene2501;
	else
		delete _asCar;
}

void Scene2501::update() {
	Scene::update();
	if (_carStatus == 1) {
		removeSprite(_klaymen);
		addSprite(_asCar);
		clearRectList();
		_klaymenInCar = true;
		SetMessageHandler(&Scene2501::hmCarAtHome);
		SetUpdateHandler(&Scene2501::upCarAtHome);
		_asIdleCarLower->setVisible(false);
		_asIdleCarFull->setVisible(false);
		_asCar->setVisible(true);
		sendMessage(_asCar, NM_CAR_ENTER, 0);
		_asCar->handleUpdate();
		_klaymen = NULL;
		_carStatus = 0;
	}
	updateKlaymenClipRect();
}

void Scene2501::upCarAtHome() {
	Scene::update();
	if (_mouseClicked) {
		if (_mouseClickPos.x <= 210 && _asCar->getX() == 211 && _asCar->getY() == 400) {
			sendMessage(_asCar, NM_CAR_LEAVE, 0);
			SetUpdateHandler(&Scene2501::upGettingOutOfCar);
		} else {
			moveCarToPoint(_mouseClickPos);
			SetMessageHandler(&Scene2501::hmRidingCar);
			SetUpdateHandler(&Scene2501::upRidingCar);
		}
		_mouseClicked = false;
	}
	updateKlaymenClipRect();
}

void Scene2501::upGettingOutOfCar() {
	Scene::update();
	if (_carStatus == 2) {
		_klaymen = _kmScene2501;
		removeSprite(_asCar);
		addSprite(_klaymen);
		_klaymenInCar = false;
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		setRectList(0x004B2608);
		_asIdleCarLower->setVisible(true);
		_asIdleCarFull->setVisible(true);
		_asCar->setVisible(false);
		setMessageList(0x004B2570);
		processMessageList();
		_klaymen->handleUpdate();
		_carStatus = 0;
	}
	updateKlaymenClipRect();
}

void Scene2501::upRidingCar() {
	Scene::update();
	if (_mouseClicked) {
		moveCarToPoint(_mouseClickPos);
		_mouseClicked = false;
	}
}

uint32 Scene2501::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x60842040)
			_carStatus = 1;
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 Scene2501::hmRidingCar(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_CLIMB_LADDER:
		if (_tracks[_currTrackIndex]->which1 < 0 && _newTrackIndex >= 0)
			changeTrack();
		else if (_tracks[_currTrackIndex]->which1 == 0) {
			SetMessageHandler(&Scene2501::hmCarAtHome);
			SetUpdateHandler(&Scene2501::upCarAtHome);
			sendMessage(_asCar, NM_CAR_AT_HOME, 1);
		} else if (_tracks[_currTrackIndex]->which1 > 0)
			leaveScene(_tracks[_currTrackIndex]->which1);
		break;
	case NM_KLAYMEN_STOP_CLIMBING:
		if (_tracks[_currTrackIndex]->which2 < 0 && _newTrackIndex >= 0)
			changeTrack();
		else if (_tracks[_currTrackIndex]->which2 == 0) {
			SetMessageHandler(&Scene2501::hmCarAtHome);
			SetUpdateHandler(&Scene2501::upCarAtHome);
			sendMessage(_asCar, NM_CAR_AT_HOME, 1);
		} else if (_tracks[_currTrackIndex]->which2 > 0)
			leaveScene(_tracks[_currTrackIndex]->which2);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 Scene2501::hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
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
	return messageResult;
}

void Scene2501::moveCarToPoint(NPoint &pt) {
	int minMatchTrackIndex, minMatchDistance;
	_tracks.findTrackPoint(pt, minMatchTrackIndex, minMatchDistance, _dataResource);
	if (minMatchTrackIndex >= 0 && minMatchTrackIndex != _currTrackIndex) {
		_newTrackIndex = minMatchTrackIndex;
		_clickPoint = pt;
		if (_currTrackIndex == 0)
			sendMessage(_asCar, 0x2003, _trackPoints->size() - 1);
		else
			sendMessage(_asCar, 0x2003, 0);
	} else {
		_newTrackIndex = -1;
		sendMessage(_asCar, 0x2004, pt);
	}
}

void Scene2501::changeTrack() {
	_currTrackIndex = _newTrackIndex;
	_trackPoints = _dataResource.getPointArray(_tracks[_currTrackIndex]->trackPointsName);
	_asCar->setPathPoints(_trackPoints);
	if (_currTrackIndex == 0)
		sendMessage(_asCar, NM_POSITION_CHANGE, _trackPoints->size() - 1);
	else
		sendMessage(_asCar, NM_POSITION_CHANGE, 0);
	sendPointMessage(_asCar, 0x2004, _clickPoint);
	_newTrackIndex = -1;
}

void Scene2501::updateKlaymenClipRect() {
	if (_kmScene2501->getX() <= 211)
		_kmScene2501->setClipRect(0, 0, 640, 480);
	else
		_kmScene2501->setClipRect(0, 0, 640, 388);
}

Scene2504::Scene2504(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *ssButton;

	setBackground(0x90791B80);
	setPalette(0x90791B80);
	ssButton = insertSprite<SsScene2504Button>();
	addCollisionSprite(ssButton);
	insertPuzzleMouse(0x91B8490F, 20, 620);
	SetMessageHandler(&Scene2504::handleMessage);
	SetUpdateHandler(&Scene::update);
}

uint32 Scene2504::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
