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

#include "neverhood/module2700.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1000.h"

namespace Neverhood {

static const NRect kScene2710ClipRect = NRect(0, 0, 626, 480);

static const uint32 kScene2710StaticSprites[] = {
	0x0D2016C0,
	0
};

static const NRect kScene2711ClipRect = NRect(0, 0, 521, 480);

static const uint32 kScene2711FileHashes1[] = {
	0,
	0x100801A1,
	0x201081A0,
	0x006800A4,
	0x40390120,
	0x000001B1,
	0x001000A1,
	0
};

static const uint32 kScene2711FileHashes2[] = {
	0,
	0x40403308,
	0x71403168,
	0x80423928,
	0x224131A8,
	0x50401328,
	0x70423328,
	0
};

static const uint32 kScene2711FileHashes3[] = {
	0,
	0x1088A021,
	0x108120E5,
	0x18A02321,
	0x148221A9,
	0x10082061,
	0x188820E1,
	0
};

static const NRect kScene2724ClipRect = NRect(0, 141, 640, 480);

static const uint32 kScene2724StaticSprites[] = {
	0xC20D00A5,
	0
};

static const NRect kScene2725ClipRect = NRect(0, 0, 640, 413);

static const uint32 kScene2725StaticSprites[] = {
	0xC20E00A5,
	0
};

Module2700::Module2700(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundIndex(0), _raidoMusicInitialized(false) {
	
	_vm->_soundMan->addMusic(0x42212411, 0x04020210);
	_vm->_soundMan->startMusic(0x04020210, 24, 2);
	SetMessageHandler(&Module2700::handleMessage);

	if (which < 0) {
		which = _vm->gameState().which;
		if (_vm->gameState().sceneNum == 0 || _vm->gameState().sceneNum == 30 || _vm->gameState().sceneNum == 31)
			which = -1;
		createScene(_vm->gameState().sceneNum, which);
	} else {
		createScene(0, 0);
	}

	loadSound(0, 0x00880CCC);
	loadSound(1, 0x00880CC0);
	loadSound(2, 0x00880CCC);
	loadSound(3, 0x00880CC0);

}

Module2700::~Module2700() {
	_vm->_soundMan->deleteGroup(0x42212411);
}

void Module2700::createScene(int sceneNum, int which) {
	debug("Module2700::createScene(%d, %d)", sceneNum, which);
	if (sceneNum != 30 && sceneNum != 31)
		_vm->gameState().which = which;
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene2701(_vm, this, which);
		break;
	case 1:
		_childObject = new Scene2702(_vm, this, which);
		break;
	case 2:
		if (which == 6 || which == 7)
			createScene2703(which, 0x004B1710);
		else if (which == 4 || which == 5)
			createScene2703(which, 0x004B1738);
		else if (which == 2 || which == 3)
			createScene2703(which, 0x004B1760);
		else
			createScene2703(which, 0x004B1788);
		break;
	case 3:
		createScene2704(which, 0x004B17B0, 150);
		break;
	case 4:
		createScene2704(which, 0x004B17D8, 150);
		break;
	case 5:
		if (which >= 4) {
			_childObject = new Scene2706(_vm, this, which);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1828, 150);
		} else {
			createScene2704(which, 0x004B1800, 150);
		}
		break;
	case 6:
		createScene2704(which, 0x004B1850, 150);
		break;
	case 7:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1878, 150);
		} else {
			createScene2704(which, 0x004B18A0, 150);
		}
		break;
	case 8:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B18C8, 150);
		} else {
			createScene2704(which, 0x004B18F0, 150);
		}
		break;
	case 9:
		createScene2704(which, 0x004B1918, 150, kScene2710StaticSprites, &kScene2710ClipRect);
		break;
	case 10:
		// TODO _vm->gameModule()->initScene2808Vars2();
		_scene2711StaticSprites[0] = kScene2711FileHashes1[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 2)];
		_scene2711StaticSprites[1] = kScene2711FileHashes2[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 1)];
		_scene2711StaticSprites[2] = kScene2711FileHashes3[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 0)];
		_scene2711StaticSprites[3] = 0x0261282E;
		_scene2711StaticSprites[4] = 0x9608E5A0;
		_scene2711StaticSprites[5] = 0;
		createScene2704(which, 0x004B1950, 150, _scene2711StaticSprites, &kScene2711ClipRect);
		break;
	case 11:
		createScene2704(which, 0x004B19E0, 150);
		break;
	case 12:
		createScene2704(which, 0x004B1A08, 150);
		break;
	case 13:
		createScene2704(which, 0x004B1A30, 150);
		break;
	case 14:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1A58, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1A80, 150);
		} else {
			createScene2704(which, 0x004B1AA8, 150);
		}
		break;
	case 15:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1AD0, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1AF8, 150);
		} else {
			createScene2704(which, 0x004B1B20, 150);
		}
		break;
	case 16:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1B48, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1B70, 150);
		} else {
			createScene2704(which, 0x004B1B98, 150);
		}
		break;
	case 17:
		if (which == 4 || which == 5) {
			createScene2704(which, 0x004B1BC0, 150);
		} else if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1BE8, 150);
		} else {
			createScene2704(which, 0x004B1C10, 150);
		}
		break;
	case 18:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1C38, 150);
		} else {
			createScene2704(which, 0x004B1C60, 150);
		}
		break;
	case 19:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1CB0, 150);
		} else {
			createScene2704(which, 0x004B1C88, 150);
		}
		break;
	case 20:
		if (which == 2 || which == 3) {
			createScene2704(which, 0x004B1CD8, 150);
		} else {
			createScene2704(which, 0x004B1D00, 150);
		}
		break;
	case 21:
		createScene2704(which, 0x004B1D28, 150);
		break;
	case 22:
		createScene2704(which, 0x004B1D50, 150);
		break;
	case 23:
		createScene2704(which, 0x004B1D78, 150, kScene2724StaticSprites, &kScene2724ClipRect);
		break;
	case 24:
		createScene2704(which, 0x004B1DB0, 150, kScene2725StaticSprites, &kScene2725ClipRect);
		break;
	case 25:
		createScene2704(which, 0x004B1DE8, 150);
		break;
	case 26:
		createScene2704(which, 0x004B1E10, 150);
		break;
	case 27:
		createScene2704(which, 0x004B1E38, 150);
		break;
	case 28:
		createScene2704(which, 0x004B1E60, 150);
		break;
	case 30:
		createStaticScene(0x09507248, 0x0724C09D);
		break;
	case 31:
//TODO		_childObject = new Scene2732(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module2700::updateScene);
	_childObject->handleUpdate();
}

#define SceneLinkIf(moduleResult, sceneNum, which) \
	if (_moduleResult == moduleResult) { createScene(sceneNum, which); break; }

void Module2700::updateScene() {
	if (!updateChild()) {
	
		debug("sceneNum = %d; _moduleResult = %d", _vm->gameState().sceneNum, _moduleResult);
	
		switch (_vm->gameState().sceneNum) {
		case 0:
			SceneLinkIf(1, 1, 0);
			leaveModule(0);
			break;
		case 1:
			SceneLinkIf(1, 14, 1);
			SceneLinkIf(2,  2, 2);
			SceneLinkIf(3, 14, 3);
			SceneLinkIf(4,  2, 6);
			SceneLinkIf(5,  2, 4);
			createScene(0, 1);
			break;
		case 2:
			SceneLinkIf(1,  5, 0);
			SceneLinkIf(2,  1, 2);
			SceneLinkIf(3,  5, 2);
			SceneLinkIf(4,  1, 5);
			SceneLinkIf(5,  5, 4);
			SceneLinkIf(6,  1, 4);
			SceneLinkIf(7, 11, 0);
			createScene(3, 0);
			break;
		case 3:
			createScene(2, 0);
			break;
		case 4:
			SceneLinkIf(1, 7, 2);
			createScene(5, 5);
			break;
		case 5:
			SceneLinkIf(1, 6, 0);
			SceneLinkIf(2, 2, 3);
			SceneLinkIf(3, 8, 2);
			SceneLinkIf(4, 2, 5);
			SceneLinkIf(5, 4, 0);
			SceneLinkIf(6, 7, 0);
			createScene(2, 1);
			break;
		case 6:
			SceneLinkIf(1, 8, 0);
			createScene(5, 1);
			break;
		case 7:
			SceneLinkIf(1, 8, 3);
			SceneLinkIf(2, 4, 1);
			SceneLinkIf(3, 9, 0);
			createScene(5, 6);
			break;
		case 8:
			SceneLinkIf(1, 10, 0);
			SceneLinkIf(2,  5, 3);
			SceneLinkIf(3,  7, 1);
			createScene(6, 1);
			break;
		case 9:
			SceneLinkIf(1, 10, 1);
			createScene(7, 3);
			break;
		case 10:
			SceneLinkIf(1, 9, 1);
			createScene(8, 1);
			break;
		case 11:
			SceneLinkIf(1, 12, 0);
			createScene(2, 7);
			break;
		case 12:
			SceneLinkIf(1, 13, 0);
			createScene(11, 1);
			break;
		case 13:
			SceneLinkIf(1, 30, 0);
			createScene(12, 1);
			break;
		case 14:
			SceneLinkIf(1, 1, 1);
			SceneLinkIf(2, 15, 3);
			SceneLinkIf(3, 1, 3);
			SceneLinkIf(4, 15, 5);
			SceneLinkIf(5, 22, 0);
			createScene(15, 1);
			break;
		case 15:
			SceneLinkIf(1, 14, 0);
			SceneLinkIf(2, 16, 3);
			SceneLinkIf(3, 14, 2);
			SceneLinkIf(4, 16, 5);
			SceneLinkIf(5, 14, 4);
			createScene(16, 1);
			break;
		case 16:
			SceneLinkIf(1, 15, 0);
			SceneLinkIf(2, 17, 3);
			SceneLinkIf(3, 15, 2);
			SceneLinkIf(4, 17, 5);
			SceneLinkIf(5, 15, 4);
			createScene(17, 1);
			break;
		case 17:
			SceneLinkIf(1, 16, 0);
			SceneLinkIf(2, 18, 3);
			SceneLinkIf(3, 16, 2);
			SceneLinkIf(4, 20, 1);
			SceneLinkIf(5, 16, 4);
			createScene(18, 1);
			break;
		case 18:
			SceneLinkIf(1, 17, 0);
			SceneLinkIf(2, 19, 2);
			SceneLinkIf(3, 17, 2);
			createScene(19, 0);
			break;
		case 19:
			SceneLinkIf(1, 20, 2);
			SceneLinkIf(2, 18, 2);
			SceneLinkIf(3, 20, 0);
			createScene(18, 0);
			break;
		case 20:
			SceneLinkIf(1, 17, 4);
			SceneLinkIf(2, 19, 1);
			SceneLinkIf(3, 21, 0);
			createScene(19, 3);
			break;
		case 21:
			_vm->_soundMan->deleteMusic(_musicFileHash);
			_vm->_soundMan->startMusic(0x04020210, 0, 2);
			_vm->_soundMan->deleteSoundGroup(0x42212411);
			createScene(20, 3);
			break;
		case 22:
			SceneLinkIf(1, 23, 0);
			createScene(14, 5);
			break;
		case 23:
			SceneLinkIf(1, 24, 0);
			createScene(22, 1);
			break;
		case 24:
			SceneLinkIf(1, 25, 0);
			createScene(23, 1);
			break;
		case 25:
			SceneLinkIf(1, 26, 0);
			createScene(24, 1);
			break;
		case 26:
			SceneLinkIf(1, 27, 0);
			createScene(25, 1);
			break;
		case 27:
			SceneLinkIf(1, 28, 0);
			createScene(26, 1);
			break;
		case 28:
			SceneLinkIf(1, 31, 0);
			createScene(27, 1);
			break;
		case 29:
			createScene(13, 1);
			break;
		case 30:
			createScene(28, 1);
			break;
		}
	} else {
		switch (_vm->gameState().sceneNum) {
		case 21:
			if (!_raidoMusicInitialized) {
				_vm->_soundMan->stopMusic(0x04020210, 0, 1);
				_vm->gameModule()->initRadioPuzzle();
				_musicFileHash = getGlobalVar(V_GOOD_RADIO_MUSIC_NAME);
				_vm->_soundMan->addMusic(0x42212411, _musicFileHash);
				_vm->_soundMan->startMusic(_musicFileHash, 0, 2/*TODO ??? */);
				_vm->_soundMan->addSound(0x42212411, 0x44014282);
				_vm->_soundMan->setSoundParams(0x44014282, true, 120, 360, 72, 0);
				_raidoMusicInitialized = true;
			}
			break;
		}
	}
}

void Module2700::update() {

}

uint32 Module2700::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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
			
void Module2700::createScene2703(int which, uint32 sceneInfoId, const uint32 *staticSprites, const NRect *clipRect) {
	// TODO
}

void Module2700::createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites, const NRect *clipRect) {
	_childObject = new Scene2704(_vm, this, which, sceneInfoId, value, staticSprites, clipRect);
}

static const NPoint kCarShadowOffsets[] = {
	{-63,  3},
	{-48, 40},
	{-33, 58},
	{  0, 65},
	{ 40, 53},
	{ 56, 27},
	{ 63,  0},
	{-30, 26},
	{  0, 30},
	{ 26, 25}
};

SsCommonTrackShadowBackground::SsCommonTrackShadowBackground(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, 0) {
	
	loadSprite(fileHash, kSLFDefDrawOffset | kSLFDefPosition, 0);
}

AsCommonCarShadow::AsCommonCarShadow(NeverhoodEngine *vm, AnimatedSprite *asCar, BaseSurface *shadowSurface, uint index)
	: AnimatedSprite(vm, 1100), _asCar(asCar), _index(index), _animFileHash(0) {

	SetUpdateHandler(&AsCommonCarShadow::update);
	createShadowSurface(shadowSurface, 211, 147, 100);
	updateShadow();
} 

void AsCommonCarShadow::update() {
	updateShadow();
	AnimatedSprite::update();
}

void AsCommonCarShadow::updateShadow() {
	if (_asCar->getFrameIndex() != _currFrameIndex || _asCar->getCurrAnimFileHash() != _animFileHash) {
		uint32 fileHash = _asCar->getCurrAnimFileHash();
		if (fileHash == 0x35698F78 || fileHash == 0x192ADD30 || fileHash == 0x9C220DA4 ||
			fileHash == 0x9966B138 || fileHash == 0xB579A77C || fileHash == 0xA86A9538 ||
			fileHash == 0xD4220027 || fileHash == 0xD00A1364 || fileHash == 0xD4AA03A4 ||
			fileHash == 0xF46A0324) {
			startAnimation(fileHash, _asCar->getFrameIndex(), -1);
			_newStickFrameIndex = _asCar->getFrameIndex();
		}
		_animFileHash = fileHash;
	}
	_x = _asCar->getX() + kCarShadowOffsets[_index].x;
	_y = _asCar->getY() + kCarShadowOffsets[_index].y;
	if (!_asCar->getVisible()) {
		startAnimation(0x1209E09F, 0, -1);
		_newStickFrameIndex = 0;
	}
	setDoDeltaX(_asCar->isDoDeltaX() ? 1 : 0);
}

AsCommonCarConnectorShadow::AsCommonCarConnectorShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, uint index)
	: AnimatedSprite(vm, 1100), _asCar(asCar), _index(index) {

	SetUpdateHandler(&AsCommonCarConnectorShadow::update);
	createShadowSurface1(shadowSurface, 0x60281C10, 150);
	startAnimation(0x60281C10, -1, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;	
} 

void AsCommonCarConnectorShadow::update() {
	_x = _asCar->getX() + kCarShadowOffsets[_index].x;
	_y = _asCar->getY() + kCarShadowOffsets[_index].y;
	AnimatedSprite::update();
}

AsCommonCarTrackShadow::AsCommonCarTrackShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, int16 frameIndex)
	: AnimatedSprite(vm, 1100), _asCar(asCar) {

	SetUpdateHandler(&AsCommonCarTrackShadow::update);
	createShadowSurface1(shadowSurface, 0x0759129C, 100);
	startAnimation(0x0759129C, frameIndex, -1);
	_newStickFrameIndex = frameIndex;
} 

void AsCommonCarTrackShadow::update() {
	_x = _asCar->getX();
	_y = _asCar->getY();
	AnimatedSprite::update();
}

Scene2701::Scene2701(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	Sprite *tempSprite;
	
	NRect clipRect;
	SceneInfo2700 *sceneInfo = _vm->_staticData->getSceneInfo2700(0x004B2240);
	setGlobalVar(V_CAR_DELTA_X, 1);
	
	setBackground(sceneInfo->bgFilename);
	setPalette(sceneInfo->bgFilename);

	_palette->addPalette(calcHash("paPodFloor"), 65, 31, 65);
	_palette->addPalette(calcHash("paKlayFloor"), 0, 65, 0);
	
	insertMouse433(0x08B08180);
	
	tempSprite = insertStaticSprite(0x1E086325, 1200);
	
	clipRect.set(0, 0, 640, tempSprite->getDrawRect().y2());

	if (sceneInfo->class437Filename) {

		_ssTrackShadowBackground = createSprite<SsCommonTrackShadowBackground>(sceneInfo->class437Filename);
		addEntity(_ssTrackShadowBackground);

		_asCar = insertSprite<AsCommonCar>(this, 320, 240);
		_asCarShadow = insertSprite<AsCommonCarShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
		_asCarTrackShadow = insertSprite<AsCommonCarTrackShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
		_asCarConnectorShadow = insertSprite<AsCommonCarConnectorShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	} else {
		_ssTrackShadowBackground = NULL;
		_asCar = insertSprite<AsCommonCar>(this, 320, 240);
	}

	_asCarConnector = insertSprite<AsCommonCarConnector>(_asCar);

	_which1 = sceneInfo->which1;
	_which2 = sceneInfo->which2;

	_dataResource.load(sceneInfo->dataResourceFilename);
	_trackPoints = _dataResource.getPointArray(sceneInfo->pointListName);
	_asCar->setPathPoints(_trackPoints);

	if (which == _which2) {
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
	
	_asCar->setClipRect(clipRect);
	_asCarConnector->setClipRect(clipRect);

	if (which == 1) {
		SetMessageHandler(&Scene2701::hmRidingCar);
	} else {
		sendMessage(_asCar, 0x2009, 0);
		SetMessageHandler(&Scene2701::hmCarAtHome);
	}

}

uint32 Scene2701::hmRidingCar(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		sendPointMessage(_asCar, 0x2004, param.asPoint());
		break;
	case 0x2005:
		if (_which1 >= 0)
			SetMessageHandler(&Scene2701::hmCarAtHome);
		break;
	case 0x2006:
		if (_which2 >= 0)
			leaveScene(_which2);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}

uint32 Scene2701::hmCarAtHome(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x >= 385) {
			leaveScene(0);
		} else {
			sendPointMessage(_asCar, 0x2004, param.asPoint());
			SetMessageHandler(&Scene2701::hmRidingCar);
		}
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}

static const uint32 kScene2702Infos[2][3] = {
	{0x004B5F68, 0x004B5F8C, 0x004B5FB0},
	{0x004B5FD8, 0x004B5FFC, 0x004B6020}
};


Scene2702::Scene2702(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag1(true), _newTrackIndex(-1), _count(3) {
	
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
			_sceneInfos[i][j] = _vm->_staticData->getSceneInfo2700(kScene2702Infos[i][j]);
	
	SetMessageHandler(&Scene2702::handleMessage);
	SetUpdateHandler(&Scene2702::update);
	
	setBackground(0x18808B00);
	setPalette(0x18808B00);
	
	_palette->addPalette(calcHash("paPodFloor"), 65, 31, 65);
	_palette->addPalette(calcHash("paKlayFloor"), 0, 65, 0);
	addEntity(_palette);
	
	insertMouse433(0x08B04180);

	_ssTrackShadowBackground = createSprite<SsCommonTrackShadowBackground>(0x12002035);
	addEntity(_ssTrackShadowBackground);
	
	_asCar = insertSprite<AsCommonCar>(this, 320, 240);
	_asCarShadow = insertSprite<AsCommonCarShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	insertSprite<AsCommonCarConnector>(_asCar);
	_asCarTrackShadow = insertSprite<AsCommonCarTrackShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarConnectorShadow = insertSprite<AsCommonCarConnectorShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);

	_dataResource.load(0x04310014);
	
	if (which == 1) {
		_currSceneInfos = _sceneInfos[1];
		_currTrackIndex = 1;
	} else if (which == 2) {
		_currSceneInfos = _sceneInfos[1];
		_currTrackIndex = 2;
		_palette->addPalette(calcHash("paPodShade"), 65, 31, 65);
		_palette->addPalette(calcHash("paKlayShade"), 0, 65, 0);
		_flag1 = false;
	} else if (which == 3) {
		_currSceneInfos = _sceneInfos[0];
		_currTrackIndex = 0;
	} else if (which == 4) {
		_currSceneInfos = _sceneInfos[0];
		_currTrackIndex = 2;
		_palette->addPalette(calcHash("paPodShade"), 65, 31, 65);
		_palette->addPalette(calcHash("paKlayShade"), 0, 65, 0);
		_flag1 = false;
	} else if (which == 5) {
		_currSceneInfos = _sceneInfos[0];
		_currTrackIndex = 1;
		_palette->addPalette(calcHash("paPodShade"), 65, 31, 65);
		_palette->addPalette(calcHash("paKlayShade"), 0, 65, 0);
		_flag1 = false;
	} else {
		_currSceneInfos = _sceneInfos[1];
		_currTrackIndex = 0;
	}

	_trackPoints = _dataResource.getPointArray(_currSceneInfos[_currTrackIndex]->pointListName);
	_asCar->setPathPoints(_trackPoints);

	if (which == _currSceneInfos[_currTrackIndex]->which2) {
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
		sendMessage(_asCar, 0x2007, 150);
	} else {
		sendMessage(_asCar, 0x2002, 0);
		sendMessage(_asCar, 0x2008, 150);
	}

	_palette->copyBasePalette(0, 256, 0);

}

void Scene2702::update() {
	Scene::update();
	if (_flag1 && _asCar->getX() > 422) {
		debug("fade #1");
		_palette->addBasePalette(calcHash("paPodShade"), 65, 31, 65);
		_palette->addBasePalette(calcHash("paKlayShade"), 0, 65, 0);
		_palette->startFadeToPalette(12);
		_flag1 = false;
	} else if (!_flag1 && _asCar->getX() <= 422) {
		debug("fade #2");
		_palette->addBasePalette(calcHash("paPodFloor"), 65, 31, 65);
		_palette->addBasePalette(calcHash("paKlayFloor"), 0, 65, 0);
		_palette->startFadeToPalette(12);
		_flag1 = true;
	}
}

uint32 Scene2702::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		findClosestTrack(param.asPoint());
		break;
	case 0x2005:
		if (_newTrackIndex >= 0) {
			if (_currSceneInfos[_currTrackIndex]->which1 < 0)
				changeTrack();
		} else if (_currSceneInfos[_currTrackIndex]->which1 >= 0)
			leaveScene(_currSceneInfos[_currTrackIndex]->which1);
		break;
	case 0x2006:
		if (_newTrackIndex >= 0) {
			if (_currSceneInfos[_currTrackIndex]->which2 < 0)
				changeTrack();
		} else if (_currSceneInfos[_currTrackIndex]->which2 >= 0)
			leaveScene(_currSceneInfos[_currTrackIndex]->which2);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}

void Scene2702::findClosestTrack(NPoint pt) {
	int minMatchTrackIndex = -1;
	int minMatchDistance = 640;
	// Find the track which contains a point closest to pt
	for (int infoIndex = 0; infoIndex < _count; infoIndex++) {
		NPointArray *pointList = _dataResource.getPointArray(_currSceneInfos[infoIndex]->pointListName);
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
		_newTrackDestX = pt.x;
		if (_currSceneInfos == _sceneInfos[0]) {
			if (_currTrackIndex == 0)
				sendMessage(_asCar, 0x2003, _trackPoints->size() - 1);
			else
				sendMessage(_asCar, 0x2003, 0);
		} else if (_currTrackIndex == 2) {
			sendMessage(_asCar, 0x2003, 0);
		} else {
			sendMessage(_asCar, 0x2003, _trackPoints->size() - 1);
		}
	} else {
		_newTrackIndex = -1;
		sendMessage(_asCar, 0x2004, pt.x);
	}
}

void Scene2702::changeTrack() {
	_currTrackIndex = _newTrackIndex;
	_trackPoints = _dataResource.getPointArray(_currSceneInfos[_currTrackIndex]->pointListName);
	_asCar->setPathPoints(_trackPoints);
	if (_currSceneInfos == _sceneInfos[0]) {
		if (_currTrackIndex == 0)
			sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
		else
			sendMessage(_asCar, 0x2002, 0);
	} else if (_currTrackIndex == 2) {
		sendMessage(_asCar, 0x2002, 0);
	} else {
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
	}
	sendMessage(_asCar, 0x2004, _newTrackDestX);
	_newTrackIndex = -1;
}

Scene2704::Scene2704(NeverhoodEngine *vm, Module *parentModule, int which, uint32 sceneInfoId, int16 value,
	const uint32 *staticSprites, const NRect *clipRect)
	: Scene(vm, parentModule, true) {

	SceneInfo2700 *sceneInfo = _vm->_staticData->getSceneInfo2700(sceneInfoId);
	
	SetMessageHandler(&Scene2704::handleMessage);
	SetUpdateHandler(&Scene2704::update);
	
	setBackground(sceneInfo->bgFilename);
	setPalette(sceneInfo->bgFilename);
	
	if (sceneInfo->exPaletteFilename1)
		_palette->addPalette(sceneInfo->exPaletteFilename1, 0, 65, 0);
	
	if (sceneInfo->exPaletteFilename2)
		_palette->addPalette(sceneInfo->exPaletteFilename2, 65, 31, 65);
	
	while (staticSprites && *staticSprites)
		insertStaticSprite(*staticSprites++, 1100);

	insertMouse433(sceneInfo->mouseCursorFilename);
	
	if (sceneInfo->class437Filename) {

		_ssTrackShadowBackground = createSprite<SsCommonTrackShadowBackground>(sceneInfo->class437Filename);
		addEntity(_ssTrackShadowBackground);

		_asCar = insertSprite<AsCommonCar>(this, 320, 240);
		_asCarShadow = insertSprite<AsCommonCarShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
		_asCarTrackShadow = insertSprite<AsCommonCarTrackShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
		_asCarConnectorShadow = insertSprite<AsCommonCarConnectorShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	} else {
		_ssTrackShadowBackground = NULL;
		_asCarShadow = NULL;
		_asCar = insertSprite<AsCommonCar>(this, 320, 240);
	}

	_asCarConnector = insertSprite<AsCommonCarConnector>(_asCar);
	
	_which1 = sceneInfo->which1;
	_which2 = sceneInfo->which2;

	_dataResource.load(sceneInfo->dataResourceFilename);
	_trackPoints = _dataResource.getPointArray(sceneInfo->pointListName);
	_asCar->setPathPoints(_trackPoints);
	
	if (sceneInfo->rectListName) {
		_rectList = _dataResource.getRectArray(sceneInfo->rectListName);
		// TODO _asCar->setPathRects(_rectList);
	}

	if (which == _which2) {
		NPoint testPoint = (*_trackPoints)[_trackPoints->size() - 1];
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
		if (testPoint.x > 0 && testPoint.x < 640 && testPoint.y > 0 && testPoint.y < 480)
			sendMessage(_asCar, 0x2009, 0);
		else
			sendMessage(_asCar, 0x2007, 0);
	} else {
		NPoint testPoint = (*_trackPoints)[0];
		sendMessage(_asCar, 0x2002, 0);
		if (testPoint.x > 0 && testPoint.x < 640 && testPoint.y > 0 && testPoint.y < 480)
			sendMessage(_asCar, 0x2009, 0);
		else
			sendMessage(_asCar, 0x2008, 0);
	}
	
	if (clipRect) {
		_asCar->getClipRect() = *clipRect;
		if (_asCarShadow)
			_asCarShadow->getClipRect() = *clipRect; 
		if (_asCarTrackShadow)
			_asCarTrackShadow->getClipRect() = *clipRect; 
		if (_asCarConnectorShadow)
			_asCarConnectorShadow->getClipRect() = *clipRect; 
		if (_asCarConnector)
			_asCarConnector->getClipRect() = *clipRect;
	}

}
		
void Scene2704::update() {
	Scene::update();
	if (_mouseClicked) {
		sendPointMessage(_asCar, 0x2004, _mouseClickPos);
		_mouseClicked = false;
	}
}

uint32 Scene2704::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2005:
		if (_which1 >= 0)
			leaveScene(_which1);
		break;
	case 0x2006:
		if (_which2 >= 0)
			leaveScene(_which2);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}

static const int kSceneInfo2706Count = 3;
static const struct { const char *pointListName; int which1, which2; } kSceneInfo2706[] = {
	{"me06slotSlotPath2", 4, -1},
	{"me06slotSlotPath3", -1, 6},
	{"me06slotSlotPath4", -1, 5}
};

Scene2706::Scene2706(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _newTrackIndex(-1) {
	
	SetMessageHandler(&Scene2706::handleMessage);
	
	setBackground(0x18808B88);
	setPalette(0x18808B88);
	
	_palette->addPalette(calcHash("paPodShade"), 65, 31, 65);
	_palette->addPalette(calcHash("paKlayShade"), 0, 65, 0);
	
	insertMouse433(0x08B8C180);

	_ssTrackShadowBackground = createSprite<SsCommonTrackShadowBackground>(0x18808B88);
	addEntity(_ssTrackShadowBackground);
	
	_asCar = insertSprite<AsCommonCar>(this, 320, 240);
	_asCarShadow = insertSprite<AsCommonCarShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarConnector = insertSprite<AsCommonCarConnector>(_asCar);
	_asCarTrackShadow = insertSprite<AsCommonCarTrackShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);
	_asCarConnectorShadow = insertSprite<AsCommonCarConnectorShadow>(_asCar, _ssTrackShadowBackground->getSurface(), 4);

	_dataResource.load(0x06000162);
	
	if (which == 5)
		_currTrackIndex = 2;
	else if (which == 6)		
		_currTrackIndex = 1;
	else
		_currTrackIndex = 0;

	_trackPoints = _dataResource.getPointArray(calcHash(kSceneInfo2706[_currTrackIndex].pointListName));
	_asCar->setPathPoints(_trackPoints);

	if (which == kSceneInfo2706[_currTrackIndex].which2) {
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
		if (which == 5)
			sendMessage(_asCar, 0x2007, 50);
		else			
			sendMessage(_asCar, 0x2007, 150);
	} else {
		sendMessage(_asCar, 0x2002, 0);
		if (which == 5)
			sendMessage(_asCar, 0x2008, 50);
		else			
			sendMessage(_asCar, 0x2008, 150);
	}
	
}

uint32 Scene2706::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		findClosestTrack(param.asPoint());
		break;
	case 0x2005:
		if (_newTrackIndex >= 0) {
			if (kSceneInfo2706[_currTrackIndex].which1 < 0)
				changeTrack();
		} else if (kSceneInfo2706[_currTrackIndex].which1 >= 0)
			leaveScene(kSceneInfo2706[_currTrackIndex].which1);
		break;
	case 0x2006:
		if (_newTrackIndex >= 0) {
			if (kSceneInfo2706[_currTrackIndex].which2 < 0)
				changeTrack();
		} else if (kSceneInfo2706[_currTrackIndex].which2 >= 0)
			leaveScene(kSceneInfo2706[_currTrackIndex].which2);
		break;
	case 0x200D:
		sendMessage(_parentModule, 0x200D, 0);
		break;
	}
	return 0;
}

void Scene2706::findClosestTrack(NPoint pt) {
	int minMatchTrackIndex = -1;
	int minMatchDistance = 640;
	// Find the track which contains a point closest to pt
	for (int infoIndex = 0; infoIndex < kSceneInfo2706Count; infoIndex++) {
		NPointArray *pointList = _dataResource.getPointArray(calcHash(kSceneInfo2706[infoIndex].pointListName));
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
		_newTrackDestX = pt.x;
		if (_currTrackIndex == 0)
			sendMessage(_asCar, 0x2003, _trackPoints->size() - 1);
		else
			sendMessage(_asCar, 0x2003, 0);
	} else {
		_newTrackIndex = -1;
		sendMessage(_asCar, 0x2004, pt.x);
	}
}

void Scene2706::changeTrack() {
	_currTrackIndex = _newTrackIndex;
	_trackPoints = _dataResource.getPointArray(calcHash(kSceneInfo2706[_currTrackIndex].pointListName));
	_asCar->setPathPoints(_trackPoints);
	if (_currTrackIndex == 0)
		sendMessage(_asCar, 0x2002, _trackPoints->size() - 1);
	else
		sendMessage(_asCar, 0x2002, 0);
	sendMessage(_asCar, 0x2004, _newTrackDestX);
	_newTrackIndex = -1;
}

} // End of namespace Neverhood
