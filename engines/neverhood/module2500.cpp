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

#include "neverhood/module2500.h"
#include "neverhood/module1600.h"

namespace Neverhood {

// TODO Maybe move these to the DAT

static const uint32 kScene2505StaticSprites[] = {
	0x4000A226,
	0
};

static const NRect kScene2505ClipRect = NRect(0, 0, 564, 480);

static const uint32 kScene2506StaticSprites[] = {
	0x4027AF02,
	0
};

static const NRect kScene2506ClipRect = NRect(0, 0, 640, 441);

static const uint32 kScene2508StaticSprites1[] = {
	0x2F08E610,
	0xD844E6A0,
	0
};

static const NRect kScene2508ClipRect1 = NRect(0, 0, 594, 448);

static const uint32 kScene2508StaticSprites2[] = {
	0x2F08E610,
	0
};

static const NRect kScene2508ClipRect2 = NRect(0, 0, 594, 448);

Module2500::Module2500(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundIndex(0) {
	
	_vm->_soundMan->addMusic(0x29220120, 0x05343184);
	_vm->_soundMan->startMusic(0x05343184, 0, 0);
	SetMessageHandler(&Module2500::handleMessage);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, _vm->gameState().which);
	} else {
		createScene(0, 0);
	}

	loadSound(0, 0x00880CCC);
	loadSound(1, 0x00880CC0);
	loadSound(2, 0x00880CCC);
	loadSound(3, 0x00880CC0);

}

Module2500::~Module2500() {
	_vm->_soundMan->deleteMusicGroup(0x29220120);
}

void Module2500::createScene(int sceneNum, int which) {
	debug("Module2500::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene2501(_vm, this, which);
		break;
	case 1:
		_vm->gameState().which = which;
		createScene2704(which, 0x004B01B8, 220);
		break;
	case 2:
		setGlobalVar(0x98109F12, 1);// TODO DEBUG! Join the tracks.
		_vm->gameState().which = which;
		if (getGlobalVar(0x98109F12))
			createScene2704(which, 0x004B01E0, 150);
		else
			createScene2704(which, 0x004B0208, 150);
		break;
	case 3:
		_childObject = new Scene2504(_vm, this, which);
		break;
	case 4:
		_vm->gameState().which = which;
		createScene2704(which, 0x004B0230, 150, kScene2505StaticSprites, &kScene2505ClipRect);
		break;
	case 5:
		setGlobalVar(0x21E60190, 1);
		_vm->gameState().which = which;
		createScene2704(which, 0x004B0268, 150, kScene2506StaticSprites, &kScene2506ClipRect);
		break;
	case 6:
		_vm->gameState().which = which;
		createScene2704(which, 0x004B02A0, 150);
		break;
	case 7:
		_vm->gameState().which = which;
		if (getGlobalVar(0xD0A14D10))
			createScene2704(which, 0x004B02C8, 150, kScene2508StaticSprites1, &kScene2508ClipRect1);
		else
			createScene2704(which, 0x004B02C8, 150, kScene2508StaticSprites2, &kScene2508ClipRect2);
		break;
	case 8:
		_childObject = new Scene1608(_vm, this, which);
		break;
	case 9:
		if (getGlobalVar(0xD0A14D10))
			createStaticScene(0xC62A0645, 0xA0641C6A);
		else
			createStaticScene(0x7A343546, 0x435427AB);
		break;
	}
	SetUpdateHandler(&Module2500::updateScene);
	_childObject->handleUpdate();
}

void Module2500::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
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
	}
	return messageResult;
}
			
void Module2500::createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites, const NRect *clipRect) {
	// TODO Move to module class?
	_childObject = new Scene2704(_vm, this, which, sceneInfoId, value, staticSprites, clipRect);
}

Scene2501::Scene2501(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	_sceneInfos[0] = _vm->_staticData->getSceneInfo2700(0x004B2628);
	_sceneInfos[1] = _vm->_staticData->getSceneInfo2700(0x004B264C);
	_sceneInfos[2] = _vm->_staticData->getSceneInfo2700(0x004B2670);

	setGlobalVar(0x21E60190, 1);
	SetUpdateHandler(&Scene2501::update);
	_surfaceFlag = true;
	setBackground(0x1B8E8115);
	setPalette(0x1B8E8115);
	_palette->addPalette(0x00128842, 65, 31, 65);
	_palette->addPalette("paKlayRed", 0, 64, 0);
	insertMouse433(0xE81111B0);

	_ssTrackShadowBackground = createSprite<SsCommonTrackShadowBackground>(0x99BE9015); // Don't add this to the sprite list
	addEntity(_ssTrackShadowBackground);

	_asCar = createSprite<AsCommonCar>(this, 211, 400); // Create but don't add to the sprite list yet
	_asIdleCarLower = insertSprite<AsCommonIdleCarLower>(211, 400);
	_asIdleCarFull = insertSprite<AsCommonIdleCarFull>(211, 400);
	insertStaticSprite(0xC42AC521, 1500);

	if (which < 0) {
		insertKlayman<KmScene2501>(162, 393);
		_kmScene2501 = _klayman;
		_klaymanInCar = false;
		setMessageList(0x004B2538);
		setRectList(0x004B2608);
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		sendMessage(_asCar, 0x2009, 0);
		_asCar->setVisible(false);
		_currTrackIndex = 0;
	} else if (which == 1 || which == 2) {
		addSprite(_asCar);
		_kmScene2501 = (Klayman*)new KmScene2501(_vm, this, 275, 393);
		_klaymanInCar = true;
		sendMessage(_kmScene2501, 0x2000, 1);
		_kmScene2501->setDoDeltaX(1);
		SetMessageHandler(&Scene2501::hmRidingCar);
		SetUpdateHandler(&Scene2501::upRidingCar);
		_asIdleCarLower->setVisible(false);
		_asIdleCarFull->setVisible(false);
		_currTrackIndex = which;
	} else {
		insertKlayman<KmScene2501>(162, 393);
		_kmScene2501 = _klayman;
		_klaymanInCar = false;
		setMessageList(0x004B2538);
		setRectList(0x004B2608);
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		sendMessage(_asCar, 0x2009, 0);
		_asCar->setVisible(false);
		_currTrackIndex = 0;
	}

	_asCarShadow = insertSprite<AsCommonCarShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarTrackShadow = insertSprite<AsCommonCarTrackShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarConnectorShadow = insertSprite<AsCommonCarConnectorShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	insertSprite<AsCommonCarConnector>(_asCar);
	
	_pointListsCount = 3;
	_newTrackIndex = -1;
	_dataResource.load(calcHash("Ashooded"));

	_trackPoints = _dataResource.getPointArray(_sceneInfos[_currTrackIndex]->pointListName);
	_asCar->setPathPoints(_trackPoints);

	if (which >= 0 && _sceneInfos[_currTrackIndex]->which2 == which) {
		NPoint testPoint = (*_trackPoints)[_trackPoints->size() - 1];
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
		if (testPoint.x < 0 || testPoint.x >= 640 || testPoint.y < 0 || testPoint.y >= 480)
			sendMessage(_asCar, 0x2007, 150);
	} else {
		NPoint testPoint = (*_trackPoints)[0];
		sendMessage(_asCar, 0x2002, 0);
		if (testPoint.x < 0 || testPoint.x >= 640 || testPoint.y < 0 || testPoint.y >= 480)
			sendMessage(_asCar, 0x2008, 150);
	}
	
	_carStatus = 0;
	
}

Scene2501::~Scene2501() {
	// Free sprites not currently in the sprite list
	if (_klaymanInCar)
		delete _kmScene2501;
	else
		delete _asCar;
}

void Scene2501::update() {
	Scene::update();
	if (_carStatus == 1) {
		removeSprite(_klayman);
		addSprite(_asCar);
		clearRectList();
		_klaymanInCar = true;
		SetMessageHandler(&Scene2501::hmCarAtHome);
		SetUpdateHandler(&Scene2501::upCarAtHome);
		_asIdleCarLower->setVisible(false);
		_asIdleCarFull->setVisible(false);
		_asCar->setVisible(true);
		sendMessage(_asCar, 0x2009, 0);
		_asCar->handleUpdate();
		_klayman = NULL;
		_carStatus = 0;
	}
	updateKlaymanCliprect();
}

void Scene2501::upCarAtHome() {
	Scene::update();
	if (_mouseClicked) {
		if (_mouseClickPos.x <= 210 && _asCar->getX() == 211 && _asCar->getY() == 400) {
			sendMessage(_asCar, 0x200A, 0);
			SetUpdateHandler(&Scene2501::upGettingOutOfCar);
		} else {
			findClosestTrack(_mouseClickPos);
			SetMessageHandler(&Scene2501::hmRidingCar);
			SetUpdateHandler(&Scene2501::upRidingCar);
		}
		_mouseClicked = false;
	}
	updateKlaymanCliprect();
}

void Scene2501::upGettingOutOfCar() {
	Scene::update();
	if (_carStatus == 2) {
		_klayman = _kmScene2501;
		removeSprite(_asCar);
		addSprite(_klayman);
		_klaymanInCar = false;
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		setRectList(0x004B2608);
		_asIdleCarLower->setVisible(true);
		_asIdleCarFull->setVisible(true);
		_asCar->setVisible(false);
		setMessageList(0x004B2570);
		processMessageList();
		_klayman->handleUpdate();
		_carStatus = 0;
	}
	updateKlaymanCliprect();
}

void Scene2501::upRidingCar() {
	Scene::update();
	if (_mouseClicked) {
		findClosestTrack(_mouseClickPos);
		_mouseClicked = false;
	}
}

uint32 Scene2501::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x60842040)
			_carStatus = 1;
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return messageResult;
}
		
uint32 Scene2501::hmRidingCar(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2005:
		if (_sceneInfos[_currTrackIndex]->which1 < 0 && _newTrackIndex >= 0) {
			changeTrack();
		} else if (_sceneInfos[_currTrackIndex]->which1 == 0) {
			SetMessageHandler(&Scene2501::hmCarAtHome);
			SetUpdateHandler(&Scene2501::upCarAtHome);
			sendMessage(_asCar, 0x200F, 1);
		} else if (_sceneInfos[_currTrackIndex]->which1 > 0) {
			leaveScene(_sceneInfos[_currTrackIndex]->which1);
		}
		break;
	case 0x2006:
		if (_sceneInfos[_currTrackIndex]->which2 < 0 && _newTrackIndex >= 0) {
			changeTrack();
		} else if (_sceneInfos[_currTrackIndex]->which2 == 0) {
			SetMessageHandler(&Scene2501::hmCarAtHome);
			SetUpdateHandler(&Scene2501::upCarAtHome);
			sendMessage(_asCar, 0x200F, 1);
		} else if (_sceneInfos[_currTrackIndex]->which2 > 0) {
			leaveScene(_sceneInfos[_currTrackIndex]->which2);
		}
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return messageResult;
}

uint32 Scene2501::hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x200A:
		_carStatus = 2;
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return messageResult;
}
	
void Scene2501::changeTrack() {
	_currTrackIndex = _newTrackIndex;
	_trackPoints = _dataResource.getPointArray(_sceneInfos[_currTrackIndex]->pointListName);
	_asCar->setPathPoints(_trackPoints);
	if (_currTrackIndex == 0) {
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
	} else {
		sendMessage(_asCar, 0x2002, 0);
	}
	sendPointMessage(_asCar, 0x2004, _clickPoint);
	_newTrackIndex = -1;
}

void Scene2501::findClosestTrack(NPoint &pt) {
	// TODO NOTE This is uses with minor variations in other scenes, maybe merge them? 
	int minMatchDistance = 640;
	int minMatchTrackIndex = -1;
	// Find the track which contains a point closest to pt
	for (int infoIndex = 0; infoIndex < _pointListsCount; infoIndex++) {
		NPointArray *pointList = _dataResource.getPointArray(_sceneInfos[infoIndex]->pointListName);
		for (uint pointIndex = 0; pointIndex < pointList->size(); pointIndex++) {
			NPoint testPt = (*pointList)[pointIndex];
			int distance = calcDistance(testPt.x, testPt.y, pt.x, pt.y);
			if (distance < minMatchDistance) {
				minMatchTrackIndex = infoIndex;
				minMatchDistance = distance;
			}
		}
	}
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

void Scene2501::updateKlaymanCliprect() {
	if (_kmScene2501->getX() <= 211)
		_kmScene2501->setClipRect(0, 0, 640, 480);
	else
		_kmScene2501->setClipRect(0, 0, 640, 388);
}

SsScene2504Button::SsScene2504Button(NeverhoodEngine *vm)
	: StaticSprite(vm, 1400), _countdown(0), _isSoundPlaying(false) {
	
	_spriteResource.load2(0x070220D9);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	_needRefresh = true;
	setVisible(false);
	loadSound(0, 0x4600204C);
	loadSound(1, 0x408C0034);
	loadSound(2, 0x44043000);
	loadSound(3, 0x44045000);
	SetMessageHandler(&SsScene2504Button::handleMessage);
	SetUpdateHandler(&SsScene2504Button::update);
}

void SsScene2504Button::update() {
	StaticSprite::update();
	if (_isSoundPlaying && !isSoundPlaying(0) && !isSoundPlaying(1)) {
		playSound(3);
		setVisible(false);
		_isSoundPlaying = false;
	}
	if (_countdown != 0 && (--_countdown) == 0) {
		if (getSubVar(0x14800353, 0x01180951)) {
			playSound(0);
		} else {
			playSound(1);
		}
		_isSoundPlaying = true;
	}
}

uint32 SsScene2504Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_isSoundPlaying) {
			setVisible(true);
			_countdown = 2;
			if (getSubVar(0x14800353, 0x01180951)) {
				setSubVar(0x14800353, 0x01180951, 0);
			} else {
				setSubVar(0x14800353, 0x01180951, 1);
			}
			playSound(2);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

Scene2504::Scene2504(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	Sprite *ssButton;
	
	_surfaceFlag = true;
	setBackground(0x90791B80);
	setPalette(0x90791B80);
	ssButton = insertSprite<SsScene2504Button>();
	_vm->_collisionMan->addSprite(ssButton);
	insertMouse435(0x91B8490F, 20, 620);
	SetMessageHandler(&Scene2504::handleMessage);
	SetUpdateHandler(&Scene::update);
}

uint32 Scene2504::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
