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

#include "neverhood/modules/module2800.h"
#include "neverhood/gamemodule.h"
#include "neverhood/modules/module1000.h"
#include "neverhood/modules/module1200.h"
#include "neverhood/modules/module1700.h"
#include "neverhood/modules/module2200.h"
#include "neverhood/diskplayerscene.h"

namespace Neverhood {

Module2800::Module2800(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _musicResource(NULL) {

	_currentMusicFileHash = 0;
	_vm->_soundMan->addMusic(0x64210814, 0xD2FA4D14);
	setGlobalVar(V_RADIO_MOVE_DISH_VIDEO, 1);
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, which);
	} else if (which == 2) {
		createScene(4, 3);
	} else if (which == 1) {
		createScene(4, 1);
	} else {
		createScene(0, 0);
	}

}

Module2800::~Module2800() {
	if (_musicResource) {
		_musicResource->unload();
		delete _musicResource;
	}
	_vm->_soundMan->deleteGroup(0x64210814);
}

void Module2800::createScene(int sceneNum, int which) {
	debug(1, "Module2800::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_vm->_soundMan->stopMusic(0xD2FA4D14, 0, 0);
		_childObject = new Scene2801(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_vm->_soundMan->stopMusic(0xD2FA4D14, 0, 0);
		if (getGlobalVar(V_RADIO_ENABLED))
			_childObject = new Scene2802(_vm, this, which);
		else
			createStaticScene(0x000C6444, 0xC6440008);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		if (getGlobalVar(V_KLAYMEN_SMALL))
			_childObject = new Scene2803Small(_vm, this, which);
		else
			_childObject = new Scene2803(_vm, this, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_childObject = new Scene2804(_vm, this, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->stopMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2805(_vm, this, which);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2806(_vm, this, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2807(_vm, this, which);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2808(_vm, this, 0);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2809(_vm, this, which);
		break;
	case 9:
		_vm->gameState().sceneNum = 9;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2810(_vm, this, which);
		break;
	case 10:
		_vm->gameState().sceneNum = 10;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2808(_vm, this, 1);
		break;
	case 11:
		_vm->gameState().sceneNum = 11;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2812(_vm, this, which);
		break;
	case 12:
		_vm->gameState().sceneNum = 12;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x0000A245, 0x0A241008);
		break;
	case 13:
		_vm->gameState().sceneNum = 13;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x81C60635, 0x60631814);
		break;
	case 14:
		_vm->gameState().sceneNum = 14;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0xCA811204, 0x11200CA0);
		break;
	case 15:
		_vm->gameState().sceneNum = 15;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x2D438A00, 0x38A042DC);
		break;
	case 16:
		_vm->gameState().sceneNum = 16;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x0A806204, 0x062000A0);
		break;
	case 17:
		_vm->gameState().sceneNum = 17;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x010F9284, 0xF9280018);
		break;
	case 18:
		_vm->gameState().sceneNum = 18;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x0100022B, 0x0022F018);
		break;
	case 19:
		_vm->gameState().sceneNum = 19;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x10866205, 0x66201100);
		break;
	case 20:
		_vm->gameState().sceneNum = 20;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x01C58000, 0x58004014);
		break;
	case 21:
		_vm->gameState().sceneNum = 21;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new Scene2822(_vm, this, which);
		break;
	case 22:
		_vm->gameState().sceneNum = 22;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x9408121E, 0x8121A948);
		break;
	case 23:
		_vm->gameState().sceneNum = 23;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x048C0600, 0xC0604040);
		break;
	case 24:
		_vm->gameState().sceneNum = 24;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		createStaticScene(0x04270A94, 0x70A9004A);
		break;
	case 25:
		_vm->gameState().sceneNum = 25;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		if (getGlobalVar(V_SHRINK_LIGHTS_ON))
			createStaticScene(0x01600204, 0x0020001E);
		else
			createStaticScene(0x08611204, 0x1120008E);
		break;
	case 26:
		_vm->gameState().sceneNum = 26;
		_vm->_soundMan->startMusic(0xD2FA4D14, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 4);
		break;
	case 1001:
		_vm->_soundMan->stopMusic(0xD2FA4D14, 0, 0);
		createSmackerScene(0x00800801, true, true, false);
		break;
	}
	SetUpdateHandler(&Module2800::updateScene);
	_childObject->handleUpdate();
}

void Module2800::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult != 2) {
				if (_musicResource) {
					_musicResource->unload();
					delete _musicResource;
					_musicResource = NULL;
				}
				_currentMusicFileHash = 0;
			}			
			if (_moduleResult == 1) {
				createScene(2, 0);
			} else if (_moduleResult == 2) {
				createScene(1, 0);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 0) {
				createScene(0, 2);
			} else {
				createScene(1001, -1);
			}
			break;
		case 2:
			if (_moduleResult == 1)
				createScene(3, 0);
			else if (_moduleResult == 2)
				createScene(5, 0);
			else if (_moduleResult == 3)
				createScene(6, 0);
			else if (_moduleResult == 4)
				createScene(9, 0);
			else if (_moduleResult == 5)
				createScene(25, 0);
			else 
				createScene(0, 1);
			break;
		case 3:
			createScene(2, 1);
			break;
		case 4:
			if (_moduleResult == 1) {
				leaveModule(1);
			} else {
				createScene(11, 1);
			}
			break;
		case 5:
			if (_moduleResult == 1) {
				createScene(7, 0);
			} else {
				createScene(2, 2);
			}
			break;
		case 6:
			createScene(2, 3);
			break;
		case 7:
			createScene(5, _moduleResult);
			break;
		case 8:
			if (_moduleResult == 1)
				createScene(10, 0);
			else
				createScene(9, 4);
			break;
		case 9:
			if (_moduleResult == 1)
				createScene(11, 0);
			else if (_moduleResult == 2)
				createScene(2, 0);
			else if (_moduleResult == 3)
				createScene(24, 0);
			else if (_moduleResult == 4)
				createScene(8, 0);
			else if (_moduleResult == 6)
				createScene(2, 6);
			else if (_moduleResult == 11)
				createScene(12, 0);
			else if (_moduleResult == 12)
				createScene(13, 0);
			else if (_moduleResult == 13)
				createScene(14, 0);
			else if (_moduleResult == 14)
				createScene(15, 0);
			else if (_moduleResult == 15)
				createScene(16, 0);
			else if (_moduleResult == 16)
				createScene(17, 0);
			else if (_moduleResult == 17)
				createScene(18, 0);
			else if (_moduleResult == 18)
				createScene(19, 0);
			else if (_moduleResult == 19)
				createScene(20, 0);
			else if (_moduleResult == 20)
				createScene(21, 0);
			else if (_moduleResult == 21)
				createScene(22, 0);
			else if (_moduleResult == 22)
				createScene(23, 0);
			else 
				createScene(2, 4);
			break;
		case 10:
			createScene(8, _moduleResult);
			break;
		case 11:
			if (_moduleResult == 1)
				createScene(4, 0);
			else if (_moduleResult == 2)
				createScene(26, 0);
			else if (_moduleResult == 3)
				createScene(9, 5);
			else 
				createScene(9, 1);
			break;
		case 12:
			createScene(9, 11);
			break;
		case 13:
			createScene(9, 12);
			break;
		case 14:
			createScene(9, 13);
			break;
		case 15:
			createScene(9, 14);
			break;
		case 16:
			createScene(9, 15);
			break;
		case 17:
			createScene(9, 16);
			break;
		case 18:
			createScene(9, 17);
			break;
		case 19:
			createScene(9, 18);
			break;
		case 20:
			createScene(9, 19);
			break;
		case 21:
			createScene(9, 20);
			break;
		case 22:
			createScene(9, 21);
			break;
		case 23:
			createScene(9, 22);
			break;
		case 24:
			createScene(9, 3);
			break;
		case 25:
			createScene(2, 5);
			break;
		case 26:
			createScene(11, 2);
			break;
		case 1001:
			createScene(1, -1);
			break;
		}
	} else {
		switch (_sceneNum) {
		case 0:
			updateMusic(true);
			break;
		case 1:
			updateMusic(false);
			break;
		}
	}
}

void Module2800::updateMusic(bool halfVolume) {

	uint32 newMusicFileHash = _vm->_gameModule->getCurrRadioMusicFileHash();

	if (!_musicResource)
		_musicResource = new MusicResource(_vm);
		
	if (newMusicFileHash != _currentMusicFileHash) {
		_currentMusicFileHash = newMusicFileHash;
		if (_currentMusicFileHash != 0) {
			_musicResource->load(_currentMusicFileHash);
			_musicResource->setVolume(halfVolume ? 60 : 100);
			_musicResource->play(0);
		} else {
			_musicResource->stop(0);
		}
	} else if (_currentMusicFileHash != 0) {
		if (!_musicResource->isPlaying()) {
			_musicResource->setVolume(halfVolume ? 60 : 100);
			_musicResource->play(0);
		} else {
			_musicResource->setVolume(halfVolume ? 60 : 100);
		}
	} else {
		_musicResource->stop(0);
	}

}

Scene2801::Scene2801(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *_sprite1;
	Sprite *_sprite2;

	_vm->gameModule()->initRadioPuzzle();

	SetMessageHandler(&Scene2801::handleMessage);
	SetUpdateHandler(&Scene::update);

	// Display the disabled radio; only possible when the left door is open
	if (!getGlobalVar(V_RADIO_ENABLED))
		insertStaticSprite(0x0001264C, 100);

	if (which < 0) {
		insertKlaymen<KmScene2801>(194, 430);
		setMessageList(0x004B6BB8);
	} else if (which == 1) {
		insertKlaymen<KmScene2801>(443, 398);
		setMessageList(0x004B6BC0);
	} else if (which == 2) {
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene2801>(312, 432);
			_klaymen->setDoDeltaX(1);
		} else {
			insertKlaymen<KmScene2801>(194, 432);
		}
		setMessageList(0x004B6C10);
	} else {
		insertKlaymen<KmScene2801>(0, 432);
		setMessageList(0x004B6BB0);
	}

	if (getGlobalVar(V_RADIO_ROOM_LEFT_DOOR)) {
		setRectList(0x004B6CE0);
		setBackground(0x01400666);
		setPalette(0x01400666);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x01400666, 0, 256, 0);
		_sprite1 = insertStaticSprite(0x100CA0A8, 1100);
		_sprite2 = insertStaticSprite(0x287C21A4, 1100);
		_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, _sprite2->getDrawRect().x2(), 480);
		insertScreenMouse(0x0066201C);
		_asTape = insertSprite<AsScene1201Tape>(this, 8, 1100, 302, 437, 0x9148A011);
		addCollisionSprite(_asTape); 
	} else if (getGlobalVar(V_RADIO_ROOM_RIGHT_DOOR)) {
		setRectList(0x004B6CD0);
		setBackground(0x11E00684);
		setPalette(0x11E00684);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x11E00684, 0, 256, 0);
		_sprite2 = insertStaticSprite(0x061601C8, 1100);
		_klaymen->setClipRect(0, 0, _sprite2->getDrawRect().x2(), 480);
		insertScreenMouse(0x00680116);
		_asTape = insertSprite<SsScene1705Tape>(this, 8, 1100, 302, 437, 0x01142428);
		addCollisionSprite(_asTape); 
	} else {
		setRectList(0x004B6CF0);
		setBackground(0x030006E6);
		setPalette(0x030006E6);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x030006E6, 0, 256, 0);
		_sprite2 = insertStaticSprite(0x273801CE, 1100);
		_klaymen->setClipRect(0, 0, _sprite2->getDrawRect().x2(), 480);
		insertScreenMouse(0x006E2038);
		_asTape = insertSprite<AsScene1201Tape>(this, 8, 1100, 302, 437, 0x9148A011);
		addCollisionSprite(_asTape); 
	}
	
	addEntity(_palette);

	if (which == 1) {
		_palette->addPalette(0xB103B604, 0, 65, 0);
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
	} else {
		_palette->addPalette(_paletteHash, 0, 65, 0);
		_palette->addBasePalette(_paletteHash, 0, 65, 0);
	}
	
}

Scene2801::~Scene2801() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2801::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004B6C40);
		}
		break;
	case 0x482A:
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
		_palette->startFadeToPalette(12);
		break;
	case 0x482B:
		_palette->addBasePalette(_paletteHash, 0, 65, 0);
		_palette->startFadeToPalette(12);
		break;
	}
	return messageResult;
}

Scene2802::Scene2802(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _currTuneStatus(0), _countdown1(0), _countdown2(0) {

	SetMessageHandler(&Scene2802::handleMessage);
	SetUpdateHandler(&Scene2802::update);
	insertPuzzleMouse(0x008810A8, 20, 620);
	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, 0x8284C100, true, true, true));
	_currRadioMusicIndex = getGlobalVar(V_CURR_RADIO_MUSIC_INDEX);
	// Need to go to the first frame first to load up the palette
	_smackerPlayer->gotoFrame(0);
	// Now we can actually set the current radio frame
	_smackerPlayer->gotoFrame(_currRadioMusicIndex);
	_vm->_soundMan->addSound(0x04360A18, 0x422630C2);
	_vm->_soundMan->addSound(0x04360A18, 0x00632252);
	_vm->_soundMan->addSound(0x04360A18, 0x00372241);
	_vm->_soundMan->setSoundVolume(0x00372241, 60);
	changeTuneStatus(0, 0);
	_vm->_soundMan->playSoundLooping(0x00372241);
}

Scene2802::~Scene2802() {
	_vm->_soundMan->deleteSoundGroup(0x04360A18);
	if (_currRadioMusicIndex == 0) {
		setGlobalVar(V_RADIO_ROOM_LEFT_DOOR, 1);
		setGlobalVar(V_RADIO_ROOM_RIGHT_DOOR, 0);
	} else if (_currRadioMusicIndex == getGlobalVar(V_GOOD_RADIO_MUSIC_INDEX)) {
		setGlobalVar(V_RADIO_ROOM_LEFT_DOOR, 0);
		setGlobalVar(V_RADIO_ROOM_RIGHT_DOOR, 1);
	} else {
		setGlobalVar(V_RADIO_ROOM_LEFT_DOOR, 0);
		setGlobalVar(V_RADIO_ROOM_RIGHT_DOOR, 0);
	}
	setGlobalVar(V_CURR_RADIO_MUSIC_INDEX, _currRadioMusicIndex);
}
	
void Scene2802::update() {
	int prevTuneStatus = _currTuneStatus;
	uint prevRadioMusicIndex = _currRadioMusicIndex;

	Scene::update();
	if (_countdown1 > 0)
		--_countdown1;
	else if (_currTuneStatus == 1)
		_currTuneStatus = 3;
	else if (_currTuneStatus == 4)
		_currTuneStatus = 6;
	
	switch (_currTuneStatus) {
	case 2:
		if (_currRadioMusicIndex < 90)
			incRadioMusicIndex(+1);
		_currTuneStatus = 0;
		break;
	case 3:
		if (_countdown2 > 0)
			--_countdown2;
		else if (_currRadioMusicIndex < 90) {
			incRadioMusicIndex(+1);
			_countdown2 = 1;
		} else
			_currTuneStatus = 0;
		break;
	case 5:
		if (_currRadioMusicIndex > 0)
			incRadioMusicIndex(-1);
		_currTuneStatus = 0;
		break;
	case 6:
		if (_countdown2 > 0)
			--_countdown2;
		else if (_currRadioMusicIndex > 0) {
			incRadioMusicIndex(-1);
			_countdown2 = 1;
		} else
			_currTuneStatus = 0;
		break;
	
	}

	if (prevRadioMusicIndex != _currRadioMusicIndex)
		_smackerPlayer->gotoFrame(_currRadioMusicIndex);
		
	if (prevTuneStatus != _currTuneStatus)
		changeTuneStatus(prevTuneStatus, _currTuneStatus);
		
	if (getGlobalVar(V_RADIO_MOVE_DISH_VIDEO) && prevTuneStatus != _currTuneStatus && _currRadioMusicIndex != 0) {
		setGlobalVar(V_RADIO_MOVE_DISH_VIDEO, 0);
		leaveScene(1);
	}
	
}

uint32 Scene2802::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	int prevTuneStatus = _currTuneStatus;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		} else if (_currTuneStatus == 0) {
			if (param.asPoint().x > 180 && param.asPoint().x < 300 &&
				param.asPoint().y > 130 && param.asPoint().y < 310) {
				_currTuneStatus = 4;
			} else if (param.asPoint().x > 300 && param.asPoint().x < 400 &&
				param.asPoint().y > 130 && param.asPoint().y < 310) {
				_currTuneStatus = 1;
			}
			if (_currTuneStatus == 1 || _currTuneStatus == 4) {
				_countdown1 = 8;
				changeTuneStatus(0, _currTuneStatus);
			}
		}
		break;
	case 0x0002:
		if (_countdown1 == 0)
			_currTuneStatus = 0;
		else {
			if (_currTuneStatus == 1)
				_currTuneStatus = 2;
			else if (_currTuneStatus == 4)
				_currTuneStatus = 5;
			else
				_currTuneStatus = 0;
			_countdown1 = 0;
		}
		if (prevTuneStatus != _currTuneStatus)
			changeTuneStatus(prevTuneStatus, _currTuneStatus);
		break;
	}
	return 0;
}

void Scene2802::incRadioMusicIndex(int delta) {
	_currRadioMusicIndex += delta;
	setGlobalVar(V_CURR_RADIO_MUSIC_INDEX, _currRadioMusicIndex);
}

void Scene2802::changeTuneStatus(int prevTuneStatus, int newTuneStatus) {

	if (prevTuneStatus == 3 || prevTuneStatus == 6) {
		_vm->_soundMan->stopSound(0x422630C2);
		_vm->_soundMan->stopSound(0x00632252);
	}

	if (newTuneStatus == 0) {
		if (_vm->_gameModule->getCurrRadioMusicFileHash() != 0)
			_vm->_soundMan->stopSound(0x00632252);
		else
			_vm->_soundMan->playSoundLooping(0x00632252);
	} else if (newTuneStatus == 3 || newTuneStatus == 6) {
		_vm->_soundMan->playSoundLooping(0x422630C2);
		_vm->_soundMan->playSoundLooping(0x00632252);
	}

}

AsScene2803LightCord::AsScene2803LightCord(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _fileHash1(fileHash1), _fileHash2(fileHash2),
	_isPulled(false), _isBusy(false) {

	createSurface(1010, 28, 379);
	SetUpdateHandler(&AnimatedSprite::update);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	_x = x;
	_y = y;
	stIdle();
}

uint32 AsScene2803LightCord::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (!_isBusy && param.asInteger() == calcHash("ClickSwitch")) {
			sendMessage(_parentScene, 0x480F, 0);
			playSound(0, 0x4E1CA4A0);
		}
		break;
	case 0x480F:
		stPulled();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

uint32 AsScene2803LightCord::hmPulled(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2803LightCord::stPulled() {
	_isBusy = false;
	_isPulled = true;
	startAnimation(_fileHash2, 0, -1);
	SetMessageHandler(&AsScene2803LightCord::hmPulled);
	NextState(&AsScene2803LightCord::stIdle);
}

void AsScene2803LightCord::stIdle() {
	_isPulled = false;
	startAnimation(_fileHash1, 0, -1);
	SetMessageHandler(&AsScene2803LightCord::handleMessage);
}

void AsScene2803LightCord::setFileHashes(uint32 fileHash1, uint32 fileHash2) {
	_fileHash1 = fileHash1;
	_fileHash2 = fileHash2;
	if (_isPulled) {
		startAnimation(_fileHash2, _currFrameIndex, -1);
		_isBusy = true;
	} else {
		startAnimation(_fileHash1, 0, -1);
	}
}

AsScene2803TestTubeOne::AsScene2803TestTubeOne(NeverhoodEngine *vm, uint32 fileHash1, uint32 fileHash2)
	: AnimatedSprite(vm, 1200), _fileHash1(fileHash1), _fileHash2(fileHash2) {
	
	createSurface1(fileHash1, 100);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2803TestTubeOne::handleMessage);
	_x = 529;
	_y = 326;
}

uint32 AsScene2803TestTubeOne::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger())
			startAnimation(_fileHash2, 0, -1);
		else
			startAnimation(_fileHash1, 0, -1);
		break;
	}
	return messageResult;
}

AsScene2803Rope::AsScene2803Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {
	
	createSurface(990, 68, 476);
	SetUpdateHandler(&AnimatedSprite::update);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	SetMessageHandler(&AsScene2803Rope::handleMessage);
	startAnimation(0x9D098C23, 35, 53);
	NextState(&AsScene2803Rope::stReleased);
	_x = x;
	_y = -276;
}

uint32 AsScene2803Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		startAnimation(0x9D098C23, 50, -1);
		SetMessageHandler(&AsScene2803Rope::hmReleased);
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

uint32 AsScene2803Rope::hmReleased(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

void AsScene2803Rope::stReleased() {
	startAnimation(0x8258A030, 0, 1);
	NextState(&AsScene2803Rope::stHide);
}

void AsScene2803Rope::stHide() {
	stopAnimation();
	setVisible(false);
}

Scene2803::Scene2803(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _paletteArea(0) {
	
	static const uint32 kScene2803FileHashes1[] = {
		0,
		0x081000F1,
		0x08100171,
		0x08100271
	};

	static const uint32 kScene2803FileHashes2[] = {
		0,
		0x286800D4,
		0x286806D4,
		0x28680AD4
	};

	setGlobalVar(V_BEEN_SHRINKING_ROOM, 1);
	_vm->gameModule()->initTestTubes1Puzzle();
	
	SetMessageHandler(&Scene2803::handleMessage);
	
	loadDataResource(0x00900849);
	
	_background = new Background(_vm, 0);
	_background->createSurface(0, 640, 480);
	addBackground(_background);
	
	setPalette(0x412A423E);
	addEntity(_palette);
	
	insertScreenMouse(0xA423A41A);
	
	if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) == 0) {
		_asTestTubeOne = (StaticSprite*)insertStaticSprite(0x66121222, 100);
	} else {
		_asTestTubeOne = (StaticSprite*)insertSprite<AsScene2803TestTubeOne>(
			kScene2803FileHashes1[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0)],
			kScene2803FileHashes2[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0)]);
	}
	
	if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 3)
		_asTestTubeTwo = (StaticSprite*)insertStaticSprite(0x64330236, 100);

	if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 3)
		_asTestTubeThree = (StaticSprite*)insertStaticSprite(0x2E4A22A2, 100);
	
	_asLightCord = insertSprite<AsScene2803LightCord>(this, 0x8FAD5932, 0x276E1A3D, 578, 200);
	_sprite3 = (StaticSprite*)insertStaticSprite(0xA40EF2FB, 1100);
	_sprite4 = (StaticSprite*)insertStaticSprite(0x0C03AA23, 1100);
	_sprite5 = (StaticSprite*)insertStaticSprite(0x2A822E2E, 1100);
	_sprite6 = (StaticSprite*)insertStaticSprite(0x2603A202, 1100);
	_sprite7 = (StaticSprite*)insertStaticSprite(0x24320220, 1100);
	_sprite8 = (StaticSprite*)insertStaticSprite(0x3C42022F, 1100);
	_sprite9 = (StaticSprite*)insertStaticSprite(0x341A0237, 1100);
	_sprite10 = insertStaticSprite(0x855820A3, 1200);

	_clipRectsFloor[0].x1 = 0;
	_clipRectsFloor[0].y1 = 0;
	_clipRectsFloor[0].x2 = 640;
	_clipRectsFloor[0].y2 = _sprite8->getDrawRect().y2();
	
	_clipRectsFloor[1].x1 = _sprite8->getDrawRect().x2();
	_clipRectsFloor[1].y1 = _sprite8->getDrawRect().y2();
	_clipRectsFloor[1].x2 = 640;
	_clipRectsFloor[1].y2 = 480;

	_clipRectsStairs[0].x1 = _sprite5->getDrawRect().x;
	_clipRectsStairs[0].y1 = 0;
	_clipRectsStairs[0].x2 = _sprite5->getDrawRect().x2();
	_clipRectsStairs[0].y2 = _sprite5->getDrawRect().y2();
	
	_clipRectsStairs[1].x1 = _sprite6->getDrawRect().x;
	_clipRectsStairs[1].y1 = 0;
	_clipRectsStairs[1].x2 = _sprite3->getDrawRect().x;
	_clipRectsStairs[1].y2 = _sprite6->getDrawRect().y2();
	
	_clipRectsStairs[2].x1 = _sprite3->getDrawRect().x;
	_clipRectsStairs[2].y1 = 0;
	_clipRectsStairs[2].x2 = _sprite4->getDrawRect().x2();
	_clipRectsStairs[2].y2 = 480;

	if (which < 0) {
		insertKlaymen<KmScene2803>(302, 445, _clipRectsFloor, 2);
		setMessageList(0x004B79F0);
		klaymenFloor();
	} else if (which == 1) {
		insertKlaymen<KmScene2803>(200, 445, _clipRectsFloor, 2);
		setMessageList(0x004B79C8);
		klaymenFloor();
	} else if (which == 3) {
		NPoint pt = _dataResource.getPoint(0xC2A08694);
		insertKlaymen<KmScene2803>(pt.x, pt.y, _clipRectsStairs, 3);
		setMessageList(0x004B7A00);
		klaymenStairs();
	} else if (which == 5) {
		insertKlaymen<KmScene2803>(253, 298, _clipRectsStairs, 3);
		setMessageList(0x004B7A00);
		klaymenStairs();
	} else if (which == 6) {
		_asRope = insertSprite<AsScene2803Rope>(this, 384);
		_asRope->setClipRect(0, 25, 640, 480);
		insertKlaymen<KmScene2803>(384, 0, _clipRectsFloor, 2);
		sendEntityMessage(_klaymen, 0x1014, _asRope);
		_klaymen->setClipRect(0, 25, 640, 480);
		setMessageList(0x004B7A78);
		klaymenFloor();
	} else if (which == 2) {
		insertKlaymen<KmScene2803>(400, 445, _clipRectsFloor, 2);
		setMessageList(0x004B79F8);
		klaymenFloor();
	} else {
		insertKlaymen<KmScene2803>(50, 231, _clipRectsStairs, 3);
		setMessageList(0x004B79C0);
		klaymenStairs();
	}

	changeBackground();

}

void Scene2803::upKlaymenStairs() {
	if (_klaymen->getX() < 350) {
		setPaletteArea0();
	} else {
		setPaletteArea1();
	}
	Scene::update();
}

uint32 Scene2803::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480F:
		toggleBackground();
		// NOTE Intentional fall-through
	case 0x100D:
		if (param.asInteger() == 0x84251F82)
			setMessageList(0x004B7A50);
		else if (param.asInteger() == 0x4254A2D2)
			setMessageList(0x004B7A58);
		else if (param.asInteger() == 0xE90A40A0)
			setMessageList(0x004B7A08);
		else if (param.asInteger() == 0x482D1210)
			setMessageList(0x004B7A30);
		else if (param.asInteger() == 0x802402B2) {
			sendEntityMessage(_klaymen, 0x1014, _asLightCord);
			setMessageList(0x004B7A68);
		} else if (param.asInteger() == 0x9626F390)
			setMessageList(0x004B7A88);
		break;
	case 0x482A:
		klaymenStairs();
		setPaletteArea1();
		break;
	case 0x482B:
		klaymenFloor();
		setPaletteArea0();
		break;
	}
	return messageResult;
}

void Scene2803::klaymenStairs() {
	SetUpdateHandler(&Scene2803::upKlaymenStairs);
	_klaymen->getSurface()->setClipRects(_clipRectsStairs, 3);
	sendMessage(_klaymen, 0x482C, 0xE5A48297);
	_sprite3->setVisible(true);
	_sprite4->setVisible(true);
	_sprite5->setVisible(true);
	_sprite6->setVisible(true);
	_sprite7->setVisible(true);
	_sprite8->setVisible(false);
	_sprite9->setVisible(false);
}

void Scene2803::klaymenFloor() {
	SetUpdateHandler(&Scene::update);
	_klaymen->getSurface()->setClipRects(_clipRectsFloor, 2);
	sendMessage(_klaymen, 0x482C, 0);
	_sprite3->setVisible(false);
	_sprite4->setVisible(false);
	_sprite5->setVisible(false);
	_sprite6->setVisible(false);
	_sprite7->setVisible(false);
	_sprite8->setVisible(true);
	_sprite9->setVisible(true);
}

void Scene2803::toggleBackground() {
	setGlobalVar(V_SHRINK_LIGHTS_ON, getGlobalVar(V_SHRINK_LIGHTS_ON) ? 0 : 1);
	changeBackground();
}

void Scene2803::changeBackground() {
	if (getGlobalVar(V_SHRINK_LIGHTS_ON)) {
		_asLightCord->setFileHashes(0x8FAD5932, 0x276E1A3D);
		_background->load(0x412A423E);
		_palette->addPalette(0x412A423E, 0, 256, 0);
		_palette->addBasePalette(0x412A423E, 0, 256, 0);
		_sprite3->loadSprite(0xA40EF2FB);
		_sprite4->loadSprite(0x0C03AA23);
		_sprite5->loadSprite(0x2A822E2E);
		_sprite6->loadSprite(0x2603A202);
		_sprite7->loadSprite(0x24320220);
		_mouseCursor->load(0xA423A41A);
		_mouseCursor->updateCursor();
		_sprite8->loadSprite(0x3C42022F);
		_sprite9->loadSprite(0x341A0237);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) == 0)
			_asTestTubeOne->loadSprite(0x66121222);
		else
			sendMessage(_asTestTubeOne, 0x2000, 0);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 3)
			_asTestTubeTwo->loadSprite(0x64330236);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 3)
			_asTestTubeThree->loadSprite(0x2E4A22A2);
		_sprite10->setVisible(true);
	} else {
		_asLightCord->setFileHashes(0xAFAD591A, 0x276E321D);
		_background->load(0x29800A01);
		_palette->addPalette(0x29800A01, 0, 256, 0);
		_palette->addBasePalette(0x29800A01, 0, 256, 0);
		_sprite3->loadSprite(0x234340A0);
		_sprite4->loadSprite(0x16202200);
		_sprite5->loadSprite(0x1030169A);
		_sprite6->loadSprite(0x1600A6A8);
		_sprite7->loadSprite(0xD0802EA0);
		_mouseCursor->load(0x00A05290);
		_mouseCursor->updateCursor();
		_sprite8->loadSprite(0x108012C1);
		_sprite9->loadSprite(0x708072E0);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) != 0)
			sendMessage(_asTestTubeOne, 0x2000, 1);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 3)
			_asTestTubeTwo->loadSprite(0xD48077A0);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 3)
			_asTestTubeThree->loadSprite(0x30022689);
		_sprite10->setVisible(false);
	}
	updatePaletteArea();
}

void Scene2803::setPaletteArea0() {
	if (_paletteArea != 0) {
		_paletteArea = 0;
		updatePaletteArea();
	}
}

void Scene2803::setPaletteArea1() {
	if (_paletteArea != 1) {
		_paletteArea = 1;
		updatePaletteArea();
	}
}

void Scene2803::updatePaletteArea() {
	uint32 fadePaletteHash;
	if (getGlobalVar(V_SHRINK_LIGHTS_ON))
		fadePaletteHash = (_paletteArea == 1) ? 0xB103B604 : 0x412A423E;
	else
		fadePaletteHash = (_paletteArea == 1) ? 0x0263D144 : 0x29800A01;
	_palette->addBasePalette(fadePaletteHash, 0, 64, 0);
	_palette->startFadeToPalette(12);
}

Scene2803Small::Scene2803Small(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _paletteArea(0) {

	static const uint32 kScene2803SmallFileHashes1[] = {
		0, 0x081000F1, 0x08100171, 0x08100271
	};

	static const uint32 kScene2803SmallFileHashes2[] = {
		0, 0x286800D4, 0x286806D4, 0x28680AD4
	};
	
	SetMessageHandler(&Scene2803Small::handleMessage);

	loadDataResource(0x81120132);
	insertScreenMouse(0x00A05290);

	insertSprite<AsScene2803LightCord>(this, 0xAFAD591A, 0x276E321D, 578, 200);

	if (getGlobalVar(V_SHRINK_LIGHTS_ON)) {
		setBackground(0x412A423E);
		setPalette(0x412A423E);
		_palette->addBasePalette(0x412A423E, 0, 256, 0);
		addEntity(_palette);
		_sprite1 = insertStaticSprite(0x0C03AA23, 1100);
		_sprite2 = insertStaticSprite(0x24320220, 1100);
		_sprite3 = insertStaticSprite(0x1A032204, 1100);
		_sprite4 = insertStaticSprite(0x18032204, 1100);
		_sprite5 = insertStaticSprite(0x34422912, 1100);
		_sprite6 = insertStaticSprite(0x3C42022F, 1100);
		_sprite7 = insertStaticSprite(0x341A0237, 1100);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) == 0)
			insertStaticSprite(0x66121222, 100);
		else
			insertSprite<AnimatedSprite>(kScene2803SmallFileHashes1[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0)], 100, 529, 326);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 3)
			insertStaticSprite(0x64330236, 100);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 3)
			insertStaticSprite(0x2E4A22A2, 100);
	} else {
		setBackground(0x29800A01);
		setPalette(0x29800A01);
		_palette->addBasePalette(0x29800A01, 0, 256, 0);
		addEntity(_palette);
		_sprite1 = insertStaticSprite(0x16202200, 1100);
		_sprite2 = insertStaticSprite(0xD0802EA0, 1100);
		_sprite3 = insertStaticSprite(0x780C2E30, 1100);
		_sprite4 = insertStaticSprite(0x700C2E30, 1100);
		_sprite5 = insertStaticSprite(0x102CE6E1, 900);
		_sprite6 = insertStaticSprite(0x108012C1, 1100);
		_sprite7 = insertStaticSprite(0x708072E0, 1100);
		insertStaticSprite(0x90582EA4, 100);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) != 0)
			insertSprite<AnimatedSprite>(kScene2803SmallFileHashes2[getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0)], 100, 529, 326);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 3)
			insertStaticSprite(0xD48077A0, 100);
		if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 3)
			insertStaticSprite(0x30022689, 100);
	}
	
	_sprite6->setVisible(false);
	_sprite7->setVisible(false);

	if (which < 0) {
		insertKlaymen<KmScene2803Small>(479, 435);
		klaymenFloor();
		setMessageList(0x004B60D8);
	} else if (which == 3) {
		NPoint pt = _dataResource.getPoint(0x096520ED);
		insertKlaymen<KmScene2803Small>(pt.x, pt.y);
		klaymenSlope();
		setMessageList(0x004B6100);
		_klaymen->setRepl(64, 0);
	} else if (which == 4) {
		NPoint pt = _dataResource.getPoint(0x20C6238D);
		insertKlaymen<KmScene2803Small>(pt.x, pt.y);
		klaymenSlope();
		setMessageList(0x004B60F8);
		_klaymen->setRepl(64, 0);
	} else if (which == 5) {
		NPoint pt = _dataResource.getPoint(0x2146690D);
		insertKlaymen<KmScene2803Small>(pt.x, pt.y);
		klaymenSlope();
		setMessageList(0x004B6100);
		_klaymen->setRepl(64, 0);
	} else if (which == 2) {
		NPoint pt = _dataResource.getPoint(0x104C03ED);
		insertKlaymen<KmScene2803Small>(pt.x, pt.y);
		klaymenFloor();
		setMessageList(0x004B6138);
	} else {
		insertKlaymen<KmScene2803Small>(135, 444);
		klaymenFloor();
		setMessageList(0x004B60E0, false);
		_sprite6->setVisible(true);
		_sprite7->setVisible(true);
	}

}

uint32 Scene2803Small::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xB4E4884C) {
			setMessageList(0x004B6180);
		} else if (param.asInteger() == 0xB1FDAB2E) {
			NPoint pt = _dataResource.getPoint(0x0D84A1AD);
			_klaymen->setX(pt.x);
			_klaymen->setY(pt.y);
			_klaymen->updateBounds();
			klaymenFloor();
			_klaymen->setClipRect(517, 401, 536, 480);
			setMessageList(0x004B6198);
		} else if (param.asInteger() == 0xB00C7C48) {
			setMessageList(0x004B6108);
		} else if (param.asInteger() == 0x61F64346) {
			setMessageList(0x004B6150);
		} else if (param.asInteger() == 0xAC69A28D) {
			setMessageList(0x004B6168);
		} else if (param.asInteger() == 0x00086212) {
			_klaymen->setClipRect(0, 0, 560, 315);
			_klaymen->setX(560);
			_klaymen->setY(315);
			_klaymen->updateBounds();
			klaymenSlope();
			setMessageList(0x004B61A0);
		} else if (param.asInteger() == 0x002CAA68) {
			setMessageList(0x004B61A8);
		}
		break;
	case 0x482A:
		if (_klaymen->getX() < 200) {
			setPaletteArea3();
		} else if (_klaymen->getX() < 500) {
			setSurfacePriority(_sprite5->getSurface(), 1100);
			sendMessage(_klaymen, 0x482C, 0);
			setPaletteArea2();
		} else {
			_klaymen->setClipRect(517, 401, 536, 480);
			setPaletteArea2();
		}
		break;
	case 0x482B:
		_sprite6->setVisible(false);
		_sprite7->setVisible(false);
		_klaymen->setClipRect(0, 0, 640, 480);
		setSurfacePriority(_sprite5->getSurface(), 900);
		sendMessage(_klaymen, 0x482C, 0x2086222D);
		break;
	}
	return 0;
}

void Scene2803Small::upKlaymenSlope() {
	if (_klaymen->getX() < 388) {
		_klaymen->setClipRect(_sprite3->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		setPaletteArea0();
	} else if (_klaymen->getX() < 500) {
		_klaymen->setClipRect(0, 0, _sprite1->getDrawRect().x2(), _sprite1->getDrawRect().y2());
		setPaletteArea1();
	}
	Scene::update();
}

void Scene2803Small::upKlaymenFloor() {
	if (_klaymen->getX() > 194 && _klaymen->getX() < 273)
		setPaletteArea2();
	else if (_klaymen->getX() > 155 && _klaymen->getX() < 300)
		setPaletteArea0();
	Scene::update();
}

void Scene2803Small::klaymenSlope() {
	SetUpdateHandler(&Scene2803Small::upKlaymenSlope);
	sendMessage(_klaymen, 0x482C, 0x23C630D9);
	_klaymen->setClipRect(0, 0, _sprite1->getDrawRect().x2(), _sprite1->getDrawRect().y2());
	_klaymen->setRepl(64, 0);
	_sprite1->setVisible(true);
}

void Scene2803Small::klaymenFloor() {
	SetUpdateHandler(&Scene2803Small::upKlaymenFloor);
	sendMessage(_klaymen, 0x482C, 0x2086222D);
	_klaymen->setClipRect(0, 0, 640, 480);
	_klaymen->clearRepl();
	_sprite1->setVisible(false);
}

void Scene2803Small::setPaletteArea0() {
	if (_paletteArea != 0) {
		_paletteArea = 0;
		updatePaletteArea(false);
	}
}

void Scene2803Small::setPaletteArea1() {
	if (_paletteArea != 1) {
		_paletteArea = 1;
		updatePaletteArea(false);
	}
}

void Scene2803Small::setPaletteArea2() {
	if (_paletteArea != 2) {
		_paletteArea = 2;
		updatePaletteArea(false);
	}
}

void Scene2803Small::setPaletteArea3() {
	if (_paletteArea != 3) {
		_paletteArea = 3;
		updatePaletteArea(true);
	}
}

void Scene2803Small::updatePaletteArea(bool instantly) {
	if (getGlobalVar(V_SHRINK_LIGHTS_ON)) {
		switch (_paletteArea) {
		case 1:
			_palette->addBasePalette(0x0A938204, 0, 64, 0);
			break;
		case 2:
			_palette->addBasePalette(0xB103B604, 0, 64, 0);
			break;
		case 3:
			_palette->fillBaseBlack(0, 64);
			break;
		default:
			_palette->addBasePalette(0x412A423E, 0, 64, 0);
			break;
		}
	} else {
		switch (_paletteArea) {
		case 2:
			_palette->addBasePalette(0x0263D144, 0, 64, 0);
			break;
		case 3:
			_palette->fillBaseBlack(0, 64);
			break;
		default:
			_palette->addBasePalette(0x29800A01, 0, 64, 0);
			break;
		}
	}
	_palette->startFadeToPalette(instantly ? 0 : 12);
}

SsScene2804RedButton::SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene) {
	
	loadSprite(getGlobalVar(V_SHRINK_LIGHTS_ON) ? 0x51A10202 : 0x11814A21, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	SetUpdateHandler(&SsScene2804RedButton::update);
	SetMessageHandler(&SsScene2804RedButton::handleMessage);
	loadSound(0, 0x44241240);
}

void SsScene2804RedButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804RedButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			playSound(0);
			setVisible(true);
			_countdown = 4;
			sendMessage(_parentScene, 0x2000, 0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804LightCoil::SsScene2804LightCoil(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	loadSprite(0x8889B008, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
	SetMessageHandler(&SsScene2804LightCoil::handleMessage);
}

uint32 SsScene2804LightCoil::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		setVisible(true);
		updatePosition();
		messageResult = 1;
		break;
	case 0x2003:
		setVisible(false);
		updatePosition();
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804LightTarget::SsScene2804LightTarget(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	loadSprite(0x06092132, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
	SetMessageHandler(&SsScene2804LightTarget::handleMessage);
}

uint32 SsScene2804LightTarget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		setVisible(true);
		updatePosition();
		messageResult = 1;
		break;
	case 0x2005:
		setVisible(false);
		updatePosition();
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804Flash::SsScene2804Flash(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	loadSprite(0x211003A0, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
	loadSound(0, 0xCB36BA54);
}

void SsScene2804Flash::show() {
	setVisible(true);
	updatePosition();
	playSound(0);
}

SsScene2804BeamCoilBody::SsScene2804BeamCoilBody(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	loadSprite(0x9A816000, kSLFDefDrawOffset | kSLFDefPosition, 400);
	setVisible(false);
}

AsScene2804CrystalWaves::AsScene2804CrystalWaves(NeverhoodEngine *vm, uint crystalIndex)
	: AnimatedSprite(vm, 1100), _crystalIndex(crystalIndex) {

	static const NPoint kAsScene2804CrystalWavesPoints[] = {
		{323, 245},
		{387, 76},
		{454, 260},
		{527, 70}
	};

	_x = kAsScene2804CrystalWavesPoints[crystalIndex].x;
	_y = kAsScene2804CrystalWavesPoints[crystalIndex].y;
	createSurface1(0x840C41F0, 1200);
	if (crystalIndex & 1)
		setDoDeltaY(1);
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Sprite::handleMessage);
}

void AsScene2804CrystalWaves::show() {
	setVisible(true);
	startAnimation(0x840C41F0, 0, -1);
}

void AsScene2804CrystalWaves::hide() {
	setVisible(false);
	stopAnimation();
}

static const int16 kAsScene2804CrystalFrameNums[] = {
	0, 6, 2, 8, 1, 10, 0, 0
};

static const uint32 kAsScene2804CrystalFileHashes[] = {
	0x000540B0,
	0x001280D0,
	0x003D0010,
	0x00620190,
	0x00DC0290
};

AsScene2804Crystal::AsScene2804Crystal(NeverhoodEngine *vm, AsScene2804CrystalWaves *asCrystalWaves, uint crystalIndex)
	: AnimatedSprite(vm, 1100), _asCrystalWaves(asCrystalWaves), _crystalIndex(crystalIndex), _isShowing(false) {

	static const NPoint kAsScene2804CrystalPoints[] = {
		{204, 196},
		{272, 316},
		{334, 206},
		{410, 334},
		{470, 180}
	};

	_colorNum = (int16)getSubVar(VA_CURR_CRYSTAL_COLORS, crystalIndex);
	_isLightOn = getGlobalVar(V_SHRINK_LIGHTS_ON) != 0;
	if (_isLightOn) {
		_x = kAsScene2804CrystalPoints[crystalIndex].x;
		_y = kAsScene2804CrystalPoints[crystalIndex].y;
		createSurface1(0x108DFB12, 1200);
		startAnimation(0x108DFB12, kAsScene2804CrystalFrameNums[_colorNum], -1);
		_needRefresh = true;
		_newStickFrameIndex = kAsScene2804CrystalFrameNums[_colorNum];
	} else {
		_x = 320;
		_y = 240;
		createSurface1(kAsScene2804CrystalFileHashes[crystalIndex], 1200);
		startAnimation(kAsScene2804CrystalFileHashes[crystalIndex], _colorNum, -1);
		setVisible(false);
		_needRefresh = true;
		_newStickFrameIndex = _colorNum;
	}
	loadSound(0, 0x725294D4);
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene2804Crystal::show() {
	if (!_isLightOn) {
		setVisible(true);
		_isShowing = true;
		if (_asCrystalWaves)
			_asCrystalWaves->show();
		playSound(0);
	}
}

void AsScene2804Crystal::hide() {
	if (!_isLightOn) {
		setVisible(false);
		_isShowing = false;
		if (_asCrystalWaves)
			_asCrystalWaves->hide();
	}
}

void AsScene2804Crystal::activate() {
	if (!_isShowing) {
		int16 frameNum = kAsScene2804CrystalFrameNums[_colorNum];
		_colorNum++;
		if (_colorNum >= 6)
			_colorNum = 0;
		if (_isLightOn) {
			startAnimation(0x108DFB12, frameNum, kAsScene2804CrystalFrameNums[_colorNum]);
			_playBackwards = kAsScene2804CrystalFrameNums[_colorNum] < _colorNum;
			_newStickFrameIndex = kAsScene2804CrystalFrameNums[_colorNum];
		} else {
			startAnimation(kAsScene2804CrystalFileHashes[_crystalIndex], _colorNum, -1);
			_newStickFrameIndex = _colorNum;
		}
		setSubVar(VA_CURR_CRYSTAL_COLORS, _crystalIndex, _colorNum);
	}
}

SsScene2804CrystalButton::SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene), _asCrystal(asCrystal), _crystalIndex(crystalIndex) {

	static const uint32 kSsScene2804CrystalButtonFileHashes1[] = {
		0x911101B0,
		0x22226001,
		0x4444A362,
		0x888925A4,
		0x11122829
	};

	static const uint32 kSsScene2804CrystalButtonFileHashes2[] = {
		0xB500A1A0,
		0x6A012021,
		0xD4022322,
		0xA8042525,
		0x5008292B
	};
	
	loadSprite(getGlobalVar(V_SHRINK_LIGHTS_ON) ? kSsScene2804CrystalButtonFileHashes1[crystalIndex] : kSsScene2804CrystalButtonFileHashes2[crystalIndex],
		kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x44045140);
	SetUpdateHandler(&SsScene2804CrystalButton::update);
	SetMessageHandler(&SsScene2804CrystalButton::handleMessage);
}

void SsScene2804CrystalButton::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804CrystalButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			playSound(0);
			setVisible(true);
			_countdown = 4;
			_asCrystal->activate();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene2804BeamCoil::AsScene2804BeamCoil(NeverhoodEngine *vm, Scene *parentScene, SsScene2804BeamCoilBody *ssBeamCoilBody)
	: AnimatedSprite(vm, 1400), _parentScene(parentScene), _ssBeamCoilBody(ssBeamCoilBody), _countdown(0) {
	
	createSurface1(0x00494891, 1000);
	_x = 125;
	_y = 184;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	loadSound(0, 0x6352F051);
	_vm->_soundMan->addSound(0xC5EA0B28, 0xEF56B094);
	SetUpdateHandler(&AsScene2804BeamCoil::update);
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
}

AsScene2804BeamCoil::~AsScene2804BeamCoil() {
	_vm->_soundMan->deleteSoundGroup(0xC5EA0B28);
}

void AsScene2804BeamCoil::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		sendMessage(_parentScene, 0x2001, 0);
	}
}

uint32 AsScene2804BeamCoil::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		show();
		_countdown = 92;
		messageResult = 1;
		break;
	case 0x2003:
		hide();
		messageResult = 1;
		break;
	}
	return messageResult;
}
	
void AsScene2804BeamCoil::show() {
	_ssBeamCoilBody->setVisible(true);
	setVisible(true);
	startAnimation(0x00494891, 0, -1);
	playSound(0);
	SetMessageHandler(&AsScene2804BeamCoil::hmBeaming);
	NextState(&AsScene2804BeamCoil::stBeaming);
}

void AsScene2804BeamCoil::hide() {
	stopAnimation();
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
	setVisible(false);
	_ssBeamCoilBody->setVisible(false);
	_vm->_soundMan->stopSound(0xEF56B094);
}

void AsScene2804BeamCoil::stBeaming() {
	startAnimation(0x00494891, 93, -1);
	NextState(&AsScene2804BeamCoil::stBeaming);
	_vm->_soundMan->playSoundLooping(0xEF56B094);
}

uint32 AsScene2804BeamCoil::hmBeaming(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

AsScene2804BeamTarget::AsScene2804BeamTarget(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {
	
	createSurface1(0x03842000, 1000);
	_x = 475;
	_y = 278;
	setVisible(false);
	_needRefresh = true;
	updatePosition();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2804BeamTarget::handleMessage);
}

uint32 AsScene2804BeamTarget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		setVisible(true);
		startAnimation(0x03842000, 0, -1);
		messageResult = 1;
		break;
	case 0x2005:
		setVisible(false);
		stopAnimation();
		messageResult = 1;
		break;
	}
	return messageResult;
}

Scene2804::Scene2804(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _countdown1(0), _countdown2(0), _countdown3(0),
	_beamStatus(0), _isSolved(false), _isWorking(false) {

	_vm->gameModule()->initCrystalColorsPuzzle();

	SetMessageHandler(&Scene2804::handleMessage);
	SetUpdateHandler(&Scene2804::update);

	if (getGlobalVar(V_SHRINK_LIGHTS_ON)) {
		setBackground(0xA1D03005);
		setPalette(0xA1D03005);
		addEntity(_palette);
		insertPuzzleMouse(0x03001A15, 20, 620);
		_asCoil = insertSprite<SsScene2804LightCoil>();
		_asTarget = insertSprite<SsScene2804LightTarget>();
	} else {
		SsScene2804BeamCoilBody *ssBeamCoilBody;
		setBackground(0x01C01414);
		setPalette(0x01C01414);
		addEntity(_palette);
		insertPuzzleMouse(0x01410014, 20, 620);
		ssBeamCoilBody = insertSprite<SsScene2804BeamCoilBody>();
		_asCoil = insertSprite<AsScene2804BeamCoil>(this, ssBeamCoilBody);
		_asTarget = insertSprite<AsScene2804BeamTarget>();
		_ssFlash = insertSprite<SsScene2804Flash>();
	}
	
	_ssRedButton = insertSprite<SsScene2804RedButton>(this);
	addCollisionSprite(_ssRedButton);

	for (uint crystalIndex = 0; crystalIndex < 5; crystalIndex++) {
		AsScene2804CrystalWaves *asCrystalWaves = NULL;
		if (crystalIndex < 4 && getGlobalVar(V_SHRINK_LIGHTS_ON) == 0)
			asCrystalWaves = insertSprite<AsScene2804CrystalWaves>(crystalIndex);
		_asCrystals[crystalIndex] = insertSprite<AsScene2804Crystal>(asCrystalWaves, crystalIndex);
		_ssCrystalButtons[crystalIndex] = insertSprite<SsScene2804CrystalButton>(this, _asCrystals[crystalIndex], crystalIndex);
		addCollisionSprite(_ssCrystalButtons[crystalIndex]);
	}

}

uint32 Scene2804::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x2000:
		_isWorking = true;
		sendMessage(_asCoil, 0x2002, 0);
		if (getGlobalVar(V_SHRINK_LIGHTS_ON)) {
			sendMessage(_asTarget, 0x2004, 0);
			_countdown2 = 48;
		}
		break;
	case 0x2001:
		_countdown3 = 2;
		_isSolved = true;
		_beamStatus = 0;
		for (uint index = 0; index < 5; index++)
			if (_asCrystals[index]->getColorNum() != (int16)getSubVar(VA_GOOD_CRYSTAL_COLORS, index))
				_isSolved = false;
		_countdown2 = 48;
		break;
	}
	return 0;
}

void Scene2804::update() {

	Scene::update();
	
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		leaveScene(0);
	}

	if (_countdown2 != 0 && (--_countdown2) == 0) {
		_isWorking = false;
		sendMessage(_asCoil, 0x2003, 0);
		sendMessage(_asTarget, 0x2005, 0);
		for (uint index = 0; index < 5; index++)
			_asCrystals[index]->hide();
	}

	if (_countdown3 != 0 && (--_countdown3) == 0) {
		if (_beamStatus == 5) {
			sendMessage(_asTarget, 0x2004, 0);
			if (_isSolved) {
				_palette->fillBaseWhite(0, 256);
				_palette->startFadeToPalette(18);
				setGlobalVar(V_KLAYMEN_SMALL, 1);
				_countdown1 = 48;
			}
		} else if (_beamStatus == 6) {
			if (_isSolved)
				_ssFlash->show();
		} else {
			_asCrystals[_beamStatus]->show();
		}
		_beamStatus++;
		if (_beamStatus < 6)
			_countdown3 = 2;
		else if (_beamStatus < 7)
			_countdown3 = 4;
	}

}

Scene2805::Scene2805(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {
	
	SetMessageHandler(&Scene2805::handleMessage);

	setBackground(0x08021E04);
	setPalette(0x08021E04);
	_palette->addPalette(0x8A6B1F91, 0, 65, 0);
	insertScreenMouse(0x21E00088);

	_sprite1 = insertStaticSprite(0x008261E7, 1100);
	_sprite2 = insertStaticSprite(0x020CE421, 1100);

	if (which < 0) {
		insertKlaymen<KmScene2805>(380, 338);
		setMessageList(0x004AE1C8);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		insertKlaymen<KmScene2805>(493, 338);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004AE1D0, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		insertKlaymen<KmScene2805>(493, 338);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004AE288, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		insertKlaymen<KmScene2805>(493, 338);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004AE1E0, false);
		sendMessage(this, 0x2000, 1);
	} else {
		insertKlaymen<KmScene2805>(340, 338);
		setMessageList(0x004AE1C0);
		sendMessage(this, 0x2000, 0);
	}

	_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, _sprite2->getDrawRect().x2(), 480);

}

uint32 Scene2805::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger()) {
			setRectList(0x004AE318);
			_klaymen->setKlaymenIdleTable3();
		} else {
			setRectList(0x004AE308);
			_klaymen->setKlaymenIdleTable1();
		}
		break;
	}
	return 0;
}

AsScene2806Spew::AsScene2806Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {
	
	createSurface1(0x04211490, 1200);
	_x = 378;
	_y = 423;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2806Spew::handleMessage);
	setDoDeltaX(1);
	setVisible(false);
}

uint32 AsScene2806Spew::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		playSound(0, 0x48640244);
		startAnimation(0x04211490, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}
	
Scene2806::Scene2806(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;

	SetMessageHandler(&Scene2806::handleMessage);
	SetUpdateHandler(&Scene2806::update);
	
	loadDataResource(0x98182003);
	loadHitRectList();
	
	_pointList = _dataResource.getPointArray(0x3606A422);

	insertScreenMouse(0x22114C13);	
	setBackground(0xC1B22110);
	setPalette(0xC1B22110);
	
	_sprite1 = insertStaticSprite(0xA21F82CB, 1100);
	_clipRects[0].x1 = _sprite1->getDrawRect().x;
	_clipRects[0].y1 = _sprite1->getDrawRect().y;
	_clipRects[0].x2 = _sprite1->getDrawRect().x2();
	_clipRects[0].y2 = _sprite1->getDrawRect().y2();

	_sprite2 = insertStaticSprite(0x92035301, 1100);
	_clipRects[1].y2 = _sprite2->getDrawRect().y2();

	_sprite3 = insertStaticSprite(0x3182220E, 1100);

	_sprite4 = insertStaticSprite(0x72090342, 1100);
	_clipRects[1].x1 = _sprite4->getDrawRect().x;
	_clipRects[1].y1 = _sprite4->getDrawRect().y;
	
	tempSprite = insertStaticSprite(0xD2012C02, 1100);
	_clipRects[2].x1 = tempSprite->getDrawRect().x;
	_clipRects[2].y2 = tempSprite->getDrawRect().y2();
	_clipRects[3].y1 = tempSprite->getDrawRect().y2();
	_clipRects[1].x2 = tempSprite->getDrawRect().x;

	tempSprite = insertStaticSprite(0x72875F42, 1100);
	_clipRects[3].x1 = tempSprite->getDrawRect().x;

	insertStaticSprite(0x0201410A, 1100);
	insertStaticSprite(0x72875F42, 1100);

	_asSpew = insertSprite<AsScene2806Spew>();

	_clipRects[2].y1 = 0;
	_clipRects[3].y2 = 480;
	_clipRects[2].x2 = 640;
	_clipRects[3].x2 = 640;

	if (which < 0) {
		insertKlaymen<KmScene2806>(441, 423, false, _clipRects, 4);
		setMessageList(0x004AF098);
	} else if (which == 1) {
		insertKlaymen<KmScene2806>(378, 423, false, _clipRects, 4);
		setMessageList(0x004AF098);
	} else if (which == 2) {
		insertKlaymen<KmScene2806>(378, 423, false, _clipRects, 4);
		setMessageList(0x004AF0C8, false);
	} else if (which == 3) {
		insertKlaymen<KmScene2806>(378, 423, true, _clipRects, 4);
		setMessageList(0x004AF0A0, false);
		setGlobalVar(V_KLAYMEN_SMALL, 0);
	} else {
		insertKlaymen<KmScene2806>(670, 423, false, _clipRects, 4);
		setMessageList(0x004AF090);
	}

	_pointIndex = -1;
	findClosestPoint();

}

uint32 Scene2806::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x44262B12) {
			setMessageList(0x004AF0E0);
		}
		break;
	case 0x2000:
		sendMessage(_asSpew, 0x2000, 0);
		break;
	}
	return 0;
}

void Scene2806::update() {
	Scene::update();
	findClosestPoint();
}

void Scene2806::findClosestPoint() {

	static const uint32 kScene2806PaletteFileHashes[] = {
		0x48052508,
		0x01139404,
		0x01138C04,
		0x01138004,
		0x01138604,
		0x086B8890
	};

	int16 x = MIN<int16>(_klaymen->getX(), 639);
	int index = 1;
	
	while (index < (int)_pointList->size() && (*_pointList)[index].x < x)
		++index;
	--index;

	if (_pointIndex != index) {
		_pointIndex = index;
		_palette->addPalette(kScene2806PaletteFileHashes[index], 0, 64, 0);
	}
	
}

Scene2807::Scene2807(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {
	
	SetMessageHandler(&Scene2807::handleMessage);

	if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) == 1) {
		insertStaticSprite(0x103021E2, 300);
	} else if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) == 2) {
		insertStaticSprite(0x103022E2, 300);
	} else if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) == 3) {
		insertStaticSprite(0x103024E2, 300);
	}

	if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 1) {
		insertStaticSprite(0x4800A52A, 200);
	} else if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 2) {
		insertStaticSprite(0x4800A62A, 200);
	} else if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) == 3) {
		insertStaticSprite(0x4800A02A, 200);
	}

	if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 1) {
		insertStaticSprite(0x31203430, 100);
	} else if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 2) {
		insertStaticSprite(0x31203400, 100);
	} else if (getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2) == 3) {
		insertStaticSprite(0x31203460, 100);
	}

	setBackground(0x3E049A95);
	setPalette(0x3E049A95);
	insertPuzzleMouse(0x49A913E8, 20, 620);

}

uint32 Scene2807::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	}
	return 0;
}

static const uint32 kScene2808FileHashes1[] = {
	0x90B0392,
	0x90B0192
};

static const uint32 kScene2808FileHashes2[] = {
	0xB0396098,
	0xB0196098
};

static const uint32 kClass428FileHashes[] = {
	0x140022CA,
	0x4C30A602,
	0xB1633402,
	0x12982135,
	0x0540B728,
	0x002A81E3,
	0x08982841,
	0x10982841,
	0x20982841,
	0x40982841,
	0x80982841,
	0x40800711
};

static const int kClass428Countdowns1[] = {
	18, 16, 10, 0
};

static const int kClass428Countdowns2[] = {
	9, 9, 8, 8, 5, 5, 0, 0
};

static const uint32 kClass490FileHashes[] = {
	0x08100071,
	0x24084215,
	0x18980A10
};

static const int16 kClass490FrameIndices1[] = {
	0, 8, 15, 19
};

static const int16 kClass490FrameIndices2[] = {
	0, 4, 8, 11, 15, 17, 19, 0
};

SsScene2808Dispenser::SsScene2808Dispenser(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum, int testTubeIndex)
	: StaticSprite(vm, 900), _parentScene(parentScene), _countdown(0), _testTubeSetNum(testTubeSetNum),
	_testTubeIndex(testTubeIndex) {
	
	loadSprite(kClass428FileHashes[testTubeSetNum * 3 + testTubeIndex], kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 1500);
	setVisible(false);
	SetUpdateHandler(&SsScene2808Dispenser::update);
	SetMessageHandler(&SsScene2808Dispenser::handleMessage);
}

void SsScene2808Dispenser::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}
	
uint32 SsScene2808Dispenser::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x2000, _testTubeIndex);
		messageResult = 1;
		break;
	}
	return messageResult;
}

void SsScene2808Dispenser::startCountdown(int index) {
	setVisible(true);
	updatePosition();
	if (_testTubeSetNum == 0) {
		_countdown = kClass428Countdowns1[index];
	} else {
		_countdown = kClass428Countdowns2[index];
	}
}

AsScene2808TestTube::AsScene2808TestTube(NeverhoodEngine *vm, int testTubeSetNum, int testTubeIndex, SsScene2808Dispenser *ssDispenser)
	: AnimatedSprite(vm, 1100), _testTubeSetNum(testTubeSetNum), _testTubeIndex(testTubeIndex), _ssDispenser(ssDispenser), _fillLevel(0) {

	if (testTubeSetNum == 0) {
		_x = 504;
		_y = 278;
	} else {
		setDoDeltaX(1);
		_x = 136;
		_y = 278;
	}

	createSurface1(kClass490FileHashes[testTubeIndex], 1100);

	if (testTubeSetNum == 0) {
		loadSound(0, 0x30809E2D);
		loadSound(1, 0x72811E2D);
		loadSound(2, 0x78B01625);
	} else {
		loadSound(3, 0x70A41E0C);
		loadSound(4, 0x50205E2D);
		loadSound(5, 0xF8621E2D);
		loadSound(6, 0xF1A03C2D);
		loadSound(7, 0x70A43D2D);
		loadSound(8, 0xF0601E2D);
	}
	
	startAnimation(kClass490FileHashes[testTubeIndex], 0, -1);
	_newStickFrameIndex = 0;
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2808TestTube::handleMessage);
	
	if (_fillLevel == 0)
		setVisible(false);
	
}

uint32 AsScene2808TestTube::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		fill();
		messageResult = 1;
		break;
	}
	return messageResult;
}

void AsScene2808TestTube::fill() {
	if ((int)_fillLevel < _testTubeSetNum * 3 + 3) {
		if (_testTubeSetNum == 0) {
			playSound(_fillLevel);
			setVisible(true);
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices1[_fillLevel], kClass490FrameIndices1[_fillLevel + 1]);
			_newStickFrameIndex = kClass490FrameIndices1[_fillLevel + 1];
		} else {
			playSound(3 + _fillLevel);
			setVisible(true);
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices2[_fillLevel], kClass490FrameIndices2[_fillLevel + 1]);
			_newStickFrameIndex = kClass490FrameIndices2[_fillLevel + 1];
		}
		_ssDispenser->startCountdown(_fillLevel);
		_fillLevel++;
	}
}

void AsScene2808TestTube::flush() {
	if (_fillLevel != 0) {
		if (_testTubeSetNum == 0) {
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices1[_fillLevel], -1);
		} else {
			startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices2[_fillLevel], -1);
		}
		_newStickFrameIndex = 0;
		_playBackwards = true;
		setVisible(true);
	}
}

AsScene2808Handle::AsScene2808Handle(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum)
	: AnimatedSprite(vm, 1300), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum), _isActivated(false) {
	
	loadSound(0, 0xE18D1F30);
	_x = 320;
	_y = 240;
	if (_testTubeSetNum == 1)
		setDoDeltaX(1);
	createSurface1(0x040900D0, 1300);
	startAnimation(0x040900D0, 0, -1);
	_needRefresh = true;
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2808Handle::handleMessage);
	AnimatedSprite::updatePosition();
}

uint32 AsScene2808Handle::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isActivated) {
			sendMessage(_parentScene, 0x2001, 0);
			playSound(0);
			activate();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

uint32 AsScene2808Handle::hmActivating(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2808Handle::activate() {
	startAnimation(0x040900D0, 0, -1);
	SetMessageHandler(&AsScene2808Handle::hmActivating);
	NextState(&AsScene2808Handle::stActivated);
	_isActivated = true;
	_newStickFrameIndex = -1;
}

void AsScene2808Handle::stActivated() {
	stopAnimation();
	sendMessage(_parentScene, 0x2002, 0);
}

AsScene2808Flow::AsScene2808Flow(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum) {

	if (testTubeSetNum == 0) {
		_x = 312;
		_y = 444;
	} else {
		_x = 328;
		_y = 444;
	}
	createSurface1(0xB8414818, 1200);
	startAnimation(0xB8414818, 0, -1);
	setVisible(false);
	_newStickFrameIndex = 0;
	_needRefresh = true;
	loadSound(0, 0x6389B652);
	SetUpdateHandler(&AnimatedSprite::update);
	AnimatedSprite::updatePosition();
}
	
uint32 AsScene2808Flow::hmFlowing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2808Flow::start() {
	startAnimation(0xB8414818, 0, -1);
	setVisible(true);
	SetMessageHandler(&AsScene2808Flow::hmFlowing);
	NextState(&AsScene2808Flow::stKeepFlowing);
	playSound(0);
}

void AsScene2808Flow::stKeepFlowing() {
	startAnimation(0xB8414818, 1, -1);
	NextState(&AsScene2808Flow::stKeepFlowing);
}

AsScene2808LightEffect::AsScene2808LightEffect(NeverhoodEngine *vm, int testTubeSetNum)
	: AnimatedSprite(vm, 800), _countdown(1) {
	
	_x = 320;
	_y = 240;
	if (testTubeSetNum == 1)
		setDoDeltaX(1);
	createSurface1(0x804C2404, 800);
	SetUpdateHandler(&AsScene2808LightEffect::update);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
}

void AsScene2808LightEffect::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		int16 frameIndex = _vm->_rnd->getRandomNumber(3 - 1);
		startAnimation(0x804C2404, frameIndex, frameIndex);
		updateAnim();
		updatePosition();
		_countdown = _vm->_rnd->getRandomNumber(3 - 1) + 1;
	}
}

Scene2808::Scene2808(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _countdown(0), _testTubeSetNum(which), _leaveResult(0), _isFlowing(false) {

	Sprite *asHandle;

	if (which == 0)
		_vm->gameModule()->initTestTubes1Puzzle();
	else
		_vm->gameModule()->initTestTubes2Puzzle();
	
	SetMessageHandler(&Scene2808::handleMessage);
	SetUpdateHandler(&Scene2808::update);

	setBackground(kScene2808FileHashes1[which]);
	setPalette(kScene2808FileHashes1[which]);

	asHandle = insertSprite<AsScene2808Handle>(this, which);
	addCollisionSprite(asHandle);

	_asFlow = insertSprite<AsScene2808Flow>(this, which);
	insertSprite<AsScene2808LightEffect>(which);

	for (int testTubeIndex = 0; testTubeIndex < 3; testTubeIndex++) {
		SsScene2808Dispenser *ssDispenser = insertSprite<SsScene2808Dispenser>(this, which, testTubeIndex);
		addCollisionSprite(ssDispenser);
		_asTestTubes[testTubeIndex] = insertSprite<AsScene2808TestTube>(which, testTubeIndex, ssDispenser);
		addCollisionSprite(_asTestTubes[testTubeIndex]);
	}
	
	insertScreenMouse(kScene2808FileHashes2[which]);

}

uint32 Scene2808::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !isAnyTestTubeFilled()) {
			leaveScene(1);
		}
		break;
	case 0x2000:
		if (!_isFlowing)
			_asTestTubes[param.asInteger()]->fill();
		break;
	case 0x2001:
		_isFlowing = true;
		break;
	case 0x2002:
		if (isAnyTestTubeFilled()) {
			_leaveResult = 3;
			if (!isMixtureGood())
				_leaveResult = 2;
			_asFlow->start();
			for (int i = 0; i < 3; i++)
				_asTestTubes[i]->flush();
			_mouseCursor->setVisible(false);
			_countdown = 16;
		} else {
			leaveScene(1);
		}
		break;
	}
	return 0;
}

void Scene2808::update() {
	Scene::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		leaveScene(_leaveResult);
	}
}

bool Scene2808::isMixtureGood() {
	if (_testTubeSetNum == 0) {
		return
			_asTestTubes[0]->getFillLevel() == getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 0) &&
			_asTestTubes[1]->getFillLevel() == getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 1) &&
			_asTestTubes[2]->getFillLevel() == getSubVar(VA_GOOD_TEST_TUBES_LEVEL_1, 2);
	} else {
		return
			_asTestTubes[0]->getFillLevel() == getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 0) &&
			_asTestTubes[1]->getFillLevel() == getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 1) &&
			_asTestTubes[2]->getFillLevel() == getSubVar(VA_GOOD_TEST_TUBES_LEVEL_2, 2);
	}
}

bool Scene2808::isAnyTestTubeFilled() {
	return
		_asTestTubes[0]->getFillLevel() > 0 ||
		_asTestTubes[1]->getFillLevel() > 0 ||
		_asTestTubes[2]->getFillLevel() > 0;
}

AsScene2809Spew::AsScene2809Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2809Spew::handleMessage);
	createSurface1(0x04211490, 1200);
	_x = 262;
	_y = 423;
	setDoDeltaX(0);
	setVisible(false);
}

uint32 AsScene2809Spew::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		playSound(0, 0x48640244);
		startAnimation(0x04211490, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}

Scene2809::Scene2809(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;
		
	SetMessageHandler(&Scene2809::handleMessage);
	SetUpdateHandler(&Scene2809::update);
	
	loadDataResource(0x1830009A);
	loadHitRectList();
	
	_pointList = _dataResource.getPointArray(0x064A310E);

	setBackground(0xB22116C5);
	setPalette(0xB22116C5);
	insertScreenMouse(0x116C1B2A);

	_sprite1 = insertStaticSprite(0x1FA2EB82, 1100);

	_clipRects[0].x1 = _sprite1->getDrawRect().x;
	_clipRects[0].y1 = _sprite1->getDrawRect().y;
	_clipRects[0].x2 = _sprite1->getDrawRect().x2();
	_clipRects[0].y2 = _sprite1->getDrawRect().y2();

	_sprite2 = insertStaticSprite(0x037321B2, 1100);
	_clipRects[1].y2 = _sprite2->getDrawRect().y2();

	_sprite3 = insertStaticSprite(0x82022E11, 1100);

	_sprite4 = insertStaticSprite(0x09236252, 1100);
	_clipRects[1].x2 = _sprite4->getDrawRect().x2();
	_clipRects[1].y1 = _sprite4->getDrawRect().y;

	tempSprite = insertStaticSprite(0x010C22F2, 1100);
	_clipRects[2].x2 = tempSprite->getDrawRect().x2();
	_clipRects[2].y2 = tempSprite->getDrawRect().y2();
	_clipRects[3].y1 = tempSprite->getDrawRect().y2();
	_clipRects[1].x1 = tempSprite->getDrawRect().x2();

	tempSprite = insertStaticSprite(0x877F6252, 1100);
	_clipRects[3].x2 = tempSprite->getDrawRect().x2();
	
	insertStaticSprite(0x01612A22, 1100);
	insertStaticSprite(0x877F6252, 1100);

	_asSpew = insertSprite<AsScene2809Spew>();
	_clipRects[2].y1 = 0;
	_clipRects[3].y2 = 480;
	_clipRects[2].x1 = 0;
	_clipRects[3].x1 = 0;

	if (which < 0) {
		insertKlaymen<KmScene2809>(226, 423, false, _clipRects, 4);
		setMessageList(0x004B5B90);
	} else if (which == 1) {
		insertKlaymen<KmScene2809>(262, 423, false, _clipRects, 4);
		setMessageList(0x004B5B90);
	} else if (which == 2) {
		insertKlaymen<KmScene2809>(262, 423, false, _clipRects, 4);
		setMessageList(0x004B5BD0);
	} else if (which == 3) {
		insertKlaymen<KmScene2809>(262, 423, true, _clipRects, 4);
		setMessageList(0x004B5BA8, false);
		setGlobalVar(V_KLAYMEN_SMALL, 0);
	} else {
		insertKlaymen<KmScene2809>(-30, 423, false, _clipRects, 4);
		setMessageList(0x004B5B88);
	}

	_pointIndex = -1;
	findClosestPoint();

}

void Scene2809::update() {
	Scene::update();
	findClosestPoint();
}

uint32 Scene2809::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x160DA937) {
			setMessageList(0x004B5B98);
		}
		break;
	case 0x2000:
		sendMessage(_asSpew, 0x2000, 0);
		break;
	}
	return 0;
}

void Scene2809::findClosestPoint() {

	static const uint32 kScene2809PaletteFileHashes[] = {
		0x04260848,
		0x12970401,
		0x128F0401,
		0x12830401,
		0x12850401,
		0x6A8B9008
	};

	int16 x = MAX<int16>(_klaymen->getX(), 2);
	int index = 1;

	while (index < (int)_pointList->size() && (*_pointList)[index].x >= x)
		++index;
	--index;

	if (_pointIndex != index) {
		_pointIndex = index;
		_palette->addPalette(kScene2809PaletteFileHashes[index], 0, 64, 0);
	}
	
}

AsScene2810Rope::AsScene2810Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x)
	: AnimatedSprite(vm, 1100) {

	createSurface(990, 68, 476);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2810Rope::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	_x = x;
	_y = -276;
	startAnimation(0x9D098C23, 35, 53);
}

uint32 AsScene2810Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		startAnimation(0x9D098C23, 35, 53);
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

Scene2810::Scene2810(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;

	SetMessageHandler(&Scene2810::handleMessage);
	
	setBackground(0x26508804);
	setPalette(0x26508804);
	insertScreenMouse(0x0880026D);

	_sprite6 = insertStaticSprite(0x03615227, 1100);
	_sprite5 = insertStaticSprite(0xE059A224, 1100);

	_clipRects[0].x1 = 0;
	_clipRects[0].y1 = 0;
	_clipRects[0].x2 = 640;
	_clipRects[0].y2 = 400;
	_clipRects[1].x1 = _sprite5->getDrawRect().x;
	_clipRects[1].y1 = 400;
	_clipRects[1].x2 = _sprite6->getDrawRect().x2();
	_clipRects[1].y2 = 480;

	if (getGlobalVar(V_KLAYMEN_SMALL)) {
		_asTape = insertSprite<AsScene1201Tape>(this, 0, 900, 245, 429, 0x9148A011);
		addCollisionSprite(_asTape);
	} else {
		_asTape = insertSprite<AsScene1201Tape>(this, 0, 1100, 245, 429, 0x9148A011);
		addCollisionSprite(_asTape);
	}

	_sprite1 = insertStaticSprite(0x430001C4, 1200);

	if (getGlobalVar(V_LADDER_DOWN)) {
		setGlobalVar(V_BEEN_STATUE_ROOM, 1);
		if (getGlobalVar(V_KLAYMEN_SMALL)) {
			_sprite4 = insertStaticSprite(0x82653808, 100);
		} else {
			_sprite4 = insertStaticSprite(0x82653808, 1100);
		}
		_sprite4->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
	}
	
	if (which < 0) {
		if (getGlobalVar(V_KLAYMEN_SMALL)) {
			insertKlaymen<KmScene2810Small>(240, 448);
			_klaymen->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
			setMessageList(0x004AE438);
			setRectList(0x004AE810);
			_isRopingDown = false;
			removeCollisionSprite(_asTape);
		} else {
			insertKlaymen<KmScene2810>(300, 424, _clipRects, 2);
			setMessageList(0x004AE438);
			if (getGlobalVar(V_LADDER_DOWN))
				loadDataResource(0x84130112);
			else
				loadDataResource(0x84500132);
			tempSprite = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
			tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
			_clipRects[0].y1 = _sprite1->getDrawRect().y;
			_isRopingDown = false;
		}
	} else if (which == 1) {
		insertKlaymen<KmScene2810>(186, 64, _clipRects, 2);
		setMessageList(0x004AE440);
		loadDataResource(0x84130112);
		tempSprite = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
		tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
		_isRopingDown = true;
		_clipRects[0].y1 = _sprite1->getDrawRect().y;
	} else if (which == 5) {
		insertStaticSprite(0xC3007EA0, 100);
		_sprite2 = insertStaticSprite(0x02780936, 1100);
		_sprite3 = insertStaticSprite(0x1CA02160, 1100);
		_asRope = insertSprite<AsScene2810Rope>(this, 384);
		insertKlaymen<KmScene2810>(384, 0, _clipRects, 0);
		sendEntityMessage(_klaymen, 0x1014, _asRope);
		setMessageList(0x004AE738);
		_klaymen->setClipRect(0, _sprite2->getDrawRect().y, 640, _sprite3->getDrawRect().y2());
		_asRope->setClipRect(0, _sprite2->getDrawRect().y, 640, _sprite3->getDrawRect().y2());
		_vm->_soundMan->addSound(0x84400112, 0xC874EE6C);
		_vm->_soundMan->playSoundLooping(0xC874EE6C);
		_vm->_soundMan->setSoundVolume(0xC874EE6C, 50);
		_isRopingDown = false;
	} else if ((which >= 11 && which <= 14) || (which >= 19 && which <= 22) || which == 3) {
		if (getGlobalVar(V_KLAYMEN_SMALL)) {
			insertKlaymen<KmScene2810Small>((int16)getGlobalVar(V_KLAYMEN_SAVED_X), 448);
			if (getGlobalVar(V_KLAYMEN_IS_DELTA_X))
				_klaymen->setDoDeltaX(1);
			_klaymen->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
			setMessageList(0x004AE6D8);
			setRectList(0x004AE810);
			_isRopingDown = false;
			removeCollisionSprite(_asTape);
		} else {
			insertKlaymenLadder();
			if (getGlobalVar(V_LADDER_DOWN_ACTION)) {
				setMessageList(0x004AE6E8);
				setGlobalVar(V_LADDER_DOWN_ACTION, 0);
				_isRopingDown = false;
			} else {
				setMessageList(0x004AE6D8);
				_isRopingDown = false;
			}
		}
	} else if (which >= 15 && which <= 18) {
		insertKlaymenLadder();
		setMessageList(0x004AE6E0);
		_isRopingDown = false;
	} else if (which == 4) {
		if (getGlobalVar(V_KLAYMEN_SMALL)) {
			insertKlaymen<KmScene2810Small>(473, 448);
			_klaymen->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
			setMessageList(0x004AE428);
			setRectList(0x004AE810);
			_isRopingDown = false;
			removeCollisionSprite(_asTape);
		} else {
			insertKlaymen<KmScene2810>(450, 424, _clipRects, 2);
			setMessageList(0x004AE418);
			if (getGlobalVar(V_LADDER_DOWN))
				loadDataResource(0x84130112);
			else
				loadDataResource(0x84500132);
			tempSprite = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
			tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
			_clipRects[0].y1 = _sprite1->getDrawRect().y;
			_isRopingDown = false;
		}
	} else {
		insertKlaymen<KmScene2810Small>(120, 448);
		_klaymen->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
		setMessageList(0x004AE410);
		setRectList(0x004AE810);
		_isRopingDown = false;
		removeCollisionSprite(_asTape);
	}

}

Scene2810::~Scene2810() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
	setGlobalVar(V_KLAYMEN_SAVED_X, _klaymen->getX());
	_vm->_soundMan->deleteSoundGroup(0x84400112);
}

void Scene2810::insertKlaymenLadder() {
	Sprite *tempSprite;

	if (getGlobalVar(V_LADDER_DOWN_ACTION)) {
		insertKlaymen<KmScene2810>(430, 424, _clipRects, 2);
		_klaymen->setDoDeltaX(1);
	} else {
		insertKlaymen<KmScene2810>((int16)getGlobalVar(V_KLAYMEN_SAVED_X), 424, _clipRects, 2);
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X))
			_klaymen->setDoDeltaX(1);
	}
	if (getGlobalVar(V_LADDER_DOWN))
		loadDataResource(0x84130112);
	else
		loadDataResource(0x84500132);
	tempSprite = insertSprite<AsScene1002KlaymenLadderHands>(_klaymen);
	tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
	_clipRects[0].y1 = _sprite1->getDrawRect().y;
}

uint32 Scene2810::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xE574F14C)
			setMessageList(0x004AE458);
		else if (param.asInteger() == 0x7214A05C || param.asInteger() == 0x2905E574)
			setMessageList(0x004AE4A8);
		else if (param.asInteger() == 0x7274E24C || param.asInteger() == 0x2D24E572)
			setMessageList(0x004AE4D0);
		else if (param.asInteger() == 0x4A07A040 || param.asInteger() == 0x190426F5)
			setMessageList(0x004AE4F8);
		else if (param.asInteger() == 0x6604200C || param.asInteger() == 0x2100E435)
			setMessageList(0x004AE520);
		else if (param.asInteger() == 0xE216A05C || param.asInteger() == 0x0905EC74)
			setMessageList(0x004AE548);
		else if (param.asInteger() == 0x721DA05C || param.asInteger() == 0xB905E574)
			setMessageList(0x004AE570);
		else if (param.asInteger() == 0x6214E09C || param.asInteger() == 0x2D09E474)
			setMessageList(0x004AE598);
		else if (param.asInteger() == 0x6276A04C || param.asInteger() == 0x0904E472)
			setMessageList(0x004AE5C0);
		else if (param.asInteger() == 0x6E14A00C || param.asInteger() == 0x2900E4B4)
			setMessageList(0x004AE5E8);
		else if (param.asInteger() == 0x6014A04D || param.asInteger() == 0x2904F454)
			setMessageList(0x004AE610);
		else if (param.asInteger() == 0x6215A3C4 || param.asInteger() == 0x393C6474)
			setMessageList(0x004AE638);
		else if (param.asInteger() == 0x6A54E24D || param.asInteger() == 0x2D24F4F0)
			setMessageList(0x004AE660);
		else if (param.asInteger() == 0x2064294C || param.asInteger() == 0x2194E053)
			setMessageList(0x004AE688);
		break;
	case 0x2000:
		setRectList(0x004AE800);
		_isRopingDown = true;
		break;
	case 0x2001:
		if (getGlobalVar(V_LADDER_DOWN))
			loadDataResource(0x84130112);
		else
			loadDataResource(0x84500132);
		_isRopingDown = false;
		break;
	case 0x4826:
		if (sender == _asTape && getGlobalVar(V_KLAYMEN_SMALL) == 0 && !_isRopingDown) {
			sendEntityMessage(_klaymen, 0x1014, _asTape);
			setMessageList(0x004AE750);
		}
		break;
	}
	return messageResult;
}

AsScene2812Winch::AsScene2812Winch(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {
	
	createSurface1(0x20DA08A0, 1200);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2812Winch::handleMessage);
	setVisible(false);
	_x = 280;
	_y = 184;
}

AsScene2812Winch::~AsScene2812Winch() {
	_vm->_soundMan->deleteSoundGroup(0x00B000E2);
}

uint32 AsScene2812Winch::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		startAnimation(0x20DA08A0, 0, -1);
		setVisible(true);
		_vm->_soundMan->addSound(0x00B000E2, 0xC874EE6C);
		_vm->_soundMan->playSoundLooping(0xC874EE6C);
		break;
	case 0x3002:
		startAnimation(0x20DA08A0, 7, -1);
		break;
	}
	return messageResult;
}

AsScene2812Rope::AsScene2812Rope(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {
	
	createSurface(990, 68, 476);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2812Rope::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	startAnimation(0xAE080551, 0, -1);
	_x = 334;
	_y = 201;
}

uint32 AsScene2812Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4806:
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		stRopingDown();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

uint32 AsScene2812Rope::hmRopingDown(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2812Rope::stRopingDown() {
	sendMessage(_parentScene, 0x4806, 0);
	startAnimation(0x9D098C23, 0, -1);
	SetMessageHandler(&AsScene2812Rope::hmRopingDown);
}

AsScene2812TrapDoor::AsScene2812TrapDoor(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 0x805D0029, 100, 320, 240) {
	
	SetMessageHandler(&AsScene2812TrapDoor::handleMessage);
	_newStickFrameIndex = 0;
}

uint32 AsScene2812TrapDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		startAnimation(0x805D0029, 0, -1);
		playSound(0, 0xEA005F40);
		_newStickFrameIndex = STICK_LAST_FRAME;
		break;
	}
	return messageResult;
}

Scene2812::Scene2812(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _paletteArea(0) {
	
	if (getGlobalVar(V_HAS_FINAL_KEY) && getGlobalVar(V_KEY3_LOCATION) == 0)
		setGlobalVar(V_KEY3_LOCATION, 3);

	SetMessageHandler(&Scene2812::handleMessage);
	SetUpdateHandler(&Scene2812::update);
	
	setRectList(0x004AF700);
	
	setBackground(0x03600606);
	setPalette(0x03600606);
	addEntity(_palette);
	_palette->addBasePalette(0x03600606, 0, 256, 0);

	_sprite1 = insertStaticSprite(0x0C06C860, 1100);
	insertScreenMouse(0x0060203E);

	if (getGlobalVar(V_KEY3_LOCATION) == 3) {
		_asKey = insertSprite<AsCommonKey>(this, 2, 1100, 474, 437);
		addCollisionSprite(_asKey);
	}

	_ssTape = insertSprite<SsScene1705Tape>(this, 6, 1100, 513, 437, 0xA1361863);
	addCollisionSprite(_ssTape);
	
	_asWinch = insertSprite<AsScene2812Winch>();
	_asTrapDoor = insertSprite<AsScene2812TrapDoor>();
	_asRope = insertSprite<AsScene2812Rope>(this);

	_sprite2 = insertStaticSprite(0x08478078, 1100);
	_sprite3 = insertStaticSprite(0x2203B821, 1100);
	_sprite4 = insertStaticSprite(0x08592134, 1100);

	if (which < 0) {
		_isRopingDown = false;
		insertKlaymen<KmScene2812>(272, 432);
		setMessageList(0x004AF560);
		_sprite1->setVisible(false);
		_klaymen->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
	} else if (which == 1) {
		_isRopingDown = false;
		insertKlaymen<KmScene2812>(338, 398);
		setMessageList(0x004AF588);
		setPaletteArea1(true);
		_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, _sprite1->getDrawRect().x2(), _sprite3->getDrawRect().y2());
	} else if (which == 2) {
		_isRopingDown = false;
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene2812>(554, 432);
			_klaymen->setDoDeltaX(1);
		} else {
			insertKlaymen<KmScene2812>(394, 432);
		}
		setMessageList(0x004AF5F0);
		_sprite1->setVisible(false);
		_klaymen->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
	} else {
		_isRopingDown = true;
		insertKlaymen<KmScene2812>(150, 582);
		setMessageList(0x004AF568);
		setPaletteArea2(true);
		_sprite1->setVisible(false);
		_klaymen->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
	}
	
	_asRope->setClipRect(0, _sprite2->getDrawRect().y, 640, _sprite3->getDrawRect().y2());
	
}

void Scene2812::update() {
	if (_klaymen->getX() < 220)
		setPaletteArea2(false);
	else if (_klaymen->getX() < 240)
		setPaletteArea0(false);
	Scene::update();
}

uint32 Scene2812::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0004269B)
			sendEntityMessage(_klaymen, 0x1014, _asRope);
		break;
	case 0x2001:
		_isRopingDown = true;
		setRectList(0x004AF710);
		_klaymen->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite4->getDrawRect().y2());
		break;
	case 0x2002:
		_isRopingDown = false;
		setRectList(0x004AF700);
		_klaymen->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		break;
	case 0x4806:
		sendMessage(_asWinch, 0x2000, 0);
		sendMessage(_asTrapDoor, 0x2000, 0);
		break;
	case 0x4826:
		if (sender == _ssTape && !_isRopingDown) {
			sendEntityMessage(_klaymen, 0x1014, _ssTape);
			setMessageList(0x004AF658);
		} else if (sender == _asKey && !_isRopingDown) {
			sendEntityMessage(_klaymen, 0x1014, _asKey);
			setMessageList(0x004AF668);
		}
		break;
	case 0x482A:
		setPaletteArea1(false);
		_sprite1->setVisible(true);
		_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, _sprite1->getDrawRect().x2(), _sprite3->getDrawRect().y2());
		break;
	case 0x482B:
		setPaletteArea0(false);
		_sprite1->setVisible(false);
		_klaymen->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		break;
	}
	return messageResult;
}

void Scene2812::setPaletteArea0(bool instantly) {
	if (_paletteArea != 0) {
		_paletteArea = 0;
		updatePaletteArea(instantly);
	}
}

void Scene2812::setPaletteArea1(bool instantly) {
	if (_paletteArea != 1) {
		_paletteArea = 1;
		updatePaletteArea(instantly);
	}
}

void Scene2812::setPaletteArea2(bool instantly) {
	if (_paletteArea != 2) {
		_paletteArea = 2;
		updatePaletteArea(instantly);
	}
}

void Scene2812::updatePaletteArea(bool instantly) {
	if (_paletteArea == 0)
		_palette->addBasePalette(0x05D30F11, 0, 64, 0);
	else if (_paletteArea == 1)
		_palette->addBasePalette(0x92CA2C9B, 0, 64, 0);
	else if (_paletteArea == 2)
		_palette->addBasePalette(0x381F92C5, 0, 64, 0);
	_palette->startFadeToPalette(instantly ? 0 : 12);
}

Scene2822::Scene2822(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _countdown(0), _scrollIndex(0) {

	SetMessageHandler(&Scene2822::handleMessage);
	SetUpdateHandler(&Scene2822::update);
	_background = new Background(_vm, 0xD542022E, 0, 0);
	addBackground(_background);
	_background->getSurface()->getDrawRect().y = -10;
	setPalette(0xD542022E);
	insertPuzzleMouse(0x2022AD5C, 20, 620);
	_ssButton = insertStaticSprite(0x1A4D4120, 1100);
	_ssButton->setVisible(false);
	loadSound(2, 0x19044E72);
}

void Scene2822::update() {

	static const int16 kScene2822BackgroundYPositions[] = {
		0, -20, -5, -15, -8, -12, -9, -11, -10, 0
	};

	Scene::update();

	if (_countdown != 0) {
		if ((--_countdown) == 0) {
			if (_countdownStatus == 0) {
				_ssButton->setVisible(false);
				_countdownStatus = 1;
				_countdown = 48;
			} else if (_countdownStatus == 1) {
				playSound(0, 0x1384CB60);
				_countdownStatus = 2;
				_countdown = 12;
			} else if (_countdownStatus == 2 && getGlobalVar(V_LADDER_DOWN_ACTION)) {
				leaveScene(0);
			}
		} else if (_countdownStatus == 2 && getGlobalVar(V_LADDER_DOWN_ACTION)) {
			if (_scrollIndex < 9) {
				_background->getSurface()->getDrawRect().y = kScene2822BackgroundYPositions[_scrollIndex];
				_scrollIndex++;
			} else {
				_background->getSurface()->getDrawRect().y = -10;
			}
		}
	}

}

uint32 Scene2822::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		} else if (param.asPoint().x >= 257 && param.asPoint().y >= 235 &&
			param.asPoint().x <= 293 && param.asPoint().y <= 273) {
			_ssButton->setVisible(true);
			_countdownStatus = 0;
			_countdown = 12;
			playSound(1, 0x44061000);
			if (getGlobalVar(V_LADDER_DOWN) == 0) {
				setGlobalVar(V_LADDER_DOWN, 1);
				setGlobalVar(V_LADDER_DOWN_ACTION, 1);
				SetMessageHandler(NULL);
				playSound(2);
				_mouseCursor->setVisible(false);
			}
		}
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
