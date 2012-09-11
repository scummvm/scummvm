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

namespace Neverhood {

Module2500::Module2500(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundResource1(vm), _soundResource2(vm),
	_soundResource3(vm), _soundResource4(vm), _soundIndex(0) {
	
	// TODO Music18hList_add(0x29220120, 0x05343184);
	// TODO Music18hList_play(0x05343184, 0, 0, 1);
	SetMessageHandler(&Module2500::handleMessage);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, _vm->gameState().which);
	} else {
		createScene(0, 0);
	}

	_soundResource1.load(0x00880CCC);
	_soundResource2.load(0x00880CC0);
	_soundResource3.load(0x00880CCC);
	_soundResource4.load(0x00880CC0);

}

Module2500::~Module2500() {
}

void Module2500::createScene(int sceneNum, int which) {
	debug("Module2500::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene2501(_vm, this, which);
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
		}
	}
}

uint32 Module2500::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x200D:
		switch (_soundIndex) {
		case 0:
			_soundResource1.play();
			break;
		case 1:
			_soundResource2.play();
			break;
		case 2:
			_soundResource3.play();
			break;
		case 3:
			_soundResource4.play();
			break;
		}
		_soundIndex++;
		if (_soundIndex >= 4)
			_soundIndex = 0;
		break;
	}
	return messageResult;
}
			
Class541::Class541(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 0x1209E09F, 1100, x, y) {
	
	startAnimation(0x1209E09F, 1, -1);
	_newStickFrameIndex = 1;
	setDoDeltaX(1);
}

Class542::Class542(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 0x1209E09F, 100, x, y) {
	
	_newStickFrameIndex = 0;
	setDoDeltaX(1);
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

	_class437 = createSprite<Class437>(0x99BE9015); // Don't add this to the sprite list
	addEntity(_class437);

	_class521 = createSprite<Class521>(this, 211, 400); // Create but don't add to the sprite list yet
	_class541 = insertSprite<Class541>(211, 400);
	_class542 = insertSprite<Class542>(211, 400);
	insertStaticSprite(0xC42AC521, 1500);

	if (which < 0) {
		insertKlayman<KmScene2501>(162, 393);
		_kmScene2501 = _klayman;
		_klaymanInCar = false;
		setMessageList(0x004B2538);
		setRectList(0x004B2608);
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		sendMessage(_class521, 0x2009, 0);
		_class521->setVisible(false);
		_currTrackIndex = 0;
	} else if (which == 1 || which == 2) {
		addSprite(_class521);
		_kmScene2501 = (Klayman*)new KmScene2501(_vm, this, 275, 393);
		_klaymanInCar = true;
		sendMessage(_kmScene2501, 0x2000, 1);
		_kmScene2501->setDoDeltaX(1);
		SetMessageHandler(&Scene2501::hmRidingCar);
		SetUpdateHandler(&Scene2501::upRidingCar);
		_class541->setVisible(false);
		_class542->setVisible(false);
		_currTrackIndex = which;
	} else {
		insertKlayman<KmScene2501>(162, 393);
		_kmScene2501 = _klayman;
		_klaymanInCar = false;
		setMessageList(0x004B2538);
		setRectList(0x004B2608);
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		sendMessage(_class521, 0x2009, 0);
		_class521->setVisible(false);
		_currTrackIndex = 0;
	}

	// TODO _class517 = insertSprite<Class517>(_class521, _class437->getSurface(), 4);
	// TODO _class520 = insertSprite<Class520>(_class521, _class437->getSurface(), 4);
	// TODO _class519 = insertSprite<Class519>(_class521, _class437->getSurface(), 4);
	insertSprite<Class518>(_class521);
	
	_pointListsCount = 3;
	_newTrackIndex = -1;
	_dataResource.load(calcHash("Ashooded"));

	_trackPoints = _dataResource.getPointArray(_sceneInfos[_currTrackIndex]->pointListName);
	_class521->setPathPoints(_trackPoints);

	if (which >= 0 && _sceneInfos[_currTrackIndex]->which2 == which) {
		NPoint testPoint = (*_trackPoints)[_trackPoints->size() - 1];
		sendMessage(_class521, 0x2002, _trackPoints->size() - 1);
		if (testPoint.x < 0 || testPoint.x >= 640 || testPoint.y < 0 || testPoint.y >= 480)
			sendMessage(_class521, 0x2007, 150);
	} else {
		NPoint testPoint = (*_trackPoints)[0];
		sendMessage(_class521, 0x2002, 0);
		if (testPoint.x < 0 || testPoint.x >= 640 || testPoint.y < 0 || testPoint.y >= 480)
			sendMessage(_class521, 0x2008, 150);
	}
	
	_carStatus = 0;
	
}

Scene2501::~Scene2501() {
	// Free sprites not currently in the sprite list
	if (_klaymanInCar)
		delete _kmScene2501;
	else
		delete _class521;
}

void Scene2501::update() {
	Scene::update();
	if (_carStatus == 1) {
		removeSprite(_klayman);
		addSprite(_class521);
		clearRectList();
		_klaymanInCar = true;
		SetMessageHandler(&Scene2501::hmCarAtHome);
		SetUpdateHandler(&Scene2501::upCarAtHome);
		_class541->setVisible(false);
		_class542->setVisible(false);
		_class521->setVisible(true);
		sendMessage(_class521, 0x2009, 0);
		_class521->handleUpdate();
		_klayman = NULL;
		_carStatus = 0;
	}
	updateKlaymanCliprect();
}

void Scene2501::upCarAtHome() {
	Scene::update();
	if (_mouseClicked) {
		if (_mouseClickPos.x <= 210 && _class521->getX() == 211 && _class521->getY() == 400) {
			sendMessage(_class521, 0x200A, 0);
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
		removeSprite(_class521);
		addSprite(_klayman);
		_klaymanInCar = false;
		SetMessageHandler(&Scene2501::handleMessage);
		SetUpdateHandler(&Scene2501::update);
		setRectList(0x004B2608);
		_class541->setVisible(true);
		_class542->setVisible(true);
		_class521->setVisible(false);
		setMessageList(0x004B2570);
		runMessageList();
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
			sendMessage(_class521, 0x200F, 1);
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
			sendMessage(_class521, 0x200F, 1);
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
	_class521->setPathPoints(_trackPoints);
	if (_currTrackIndex == 0) {
		sendMessage(_class521, 0x2002, _trackPoints->size() - 1);
	} else {
		sendMessage(_class521, 0x2002, 0);
	}
	sendPointMessage(_class521, 0x2004, _clickPoint);
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
			sendMessage(_class521, 0x2003, _trackPoints->size() - 1);
		else
			sendMessage(_class521, 0x2003, 0);
	} else {
		_newTrackIndex = -1;
		sendMessage(_class521, 0x2004, pt);
	}
}

void Scene2501::updateKlaymanCliprect() {
	if (_kmScene2501->getX() <= 211)
		_kmScene2501->setClipRect(0, 0, 640, 480);
	else
		_kmScene2501->setClipRect(0, 0, 640, 388);
}

} // End of namespace Neverhood
