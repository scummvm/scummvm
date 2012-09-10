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

#include "neverhood/module2800.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1000.h"
#include "neverhood/module1200.h"
#include "neverhood/module1700.h"
#include "neverhood/module2200.h"
#include "neverhood/diskplayerscene.h"

namespace Neverhood {

Module2800::Module2800(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_fileHash = 0;
	// TODO music stuff
	// TODO Music18hList_add(0x64210814, 0xD2FA4D14);
	setGlobalVar(0x28D8C940, 1);
	
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
	// TODO music stuff
	// TODO Sound1ChList_sub_407A50(0x64210814);
	// TODO Module2800_sub471DF0();
}

void Module2800::createScene(int sceneNum, int which) {
	debug("Module2800::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 0);
		_childObject = new Scene2801(_vm, this, which);
		break;
	case 1:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 0);
#if 0		
		_flag = true; // DEBUG!
		if (_flag) {
			_childObject = new Scene2802(_vm, this, which);
		} else {
			_childObject = new Class152(_vm, this, 0x000C6444, 0xC6440008);
		}
#endif		
		break;
	case 2:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		//setGlobalVar(0x1860C990,1);//DEBUG
		if (getGlobalVar(0x1860C990))
			_childObject = new Scene2803b(_vm, this, which);
		else
			_childObject = new Scene2803(_vm, this, which);
		break;
	case 3:
		_childObject = new Scene2804(_vm, this, which);
		break;
	case 4:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 2);
		_childObject = new Scene2805(_vm, this, which);
		break;
	case 5:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2806(_vm, this, which);
		break;
	case 6:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2807(_vm, this, which);
		break;
	case 7:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2808(_vm, this, 0);
		break;
	case 8:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2809(_vm, this, which);
		break;
	case 9:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2810(_vm, this, which);
		break;
	case 10:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2808(_vm, this, 1);
		break;
	case 11:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2812(_vm, this, which);
		break;
	case 12:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x0000A245, 0x0A241008);
		break;
	case 13:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x81C60635, 0x60631814);
		break;
	case 14:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0xCA811204, 0x11200CA0);
		break;
	case 15:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x2D438A00, 0x38A042DC);
		break;
	case 16:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x0A806204, 0x062000A0);
		break;
	case 17:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x010F9284, 0xF9280018);
		break;
	case 18:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x0100022B, 0x0022F018);
		break;
	case 19:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x10866205, 0x66201100);
		break;
	case 20:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x01C58000, 0x58004014);
		break;
	case 21:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Scene2822(_vm, this, which);
		break;
	case 22:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x9408121E, 0x8121A948);
		break;
	case 23:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x048C0600, 0xC0604040);
		break;
	case 24:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new Class152(_vm, this, 0x04270A94, 0x70A9004A);
		break;
	case 25:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		if (getGlobalVar(0x190A1D18))
			_childObject = new Class152(_vm, this, 0x01600204, 0x0020001E);
		else
			_childObject = new Class152(_vm, this, 0x08611204, 0x1120008E);
		break;
	case 26:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		_childObject = new DiskplayerScene(_vm, this, 4);
		break;
	//		
	case 1001:
		break;
	// TODO ...		
	}
	SetUpdateHandler(&Module2800::updateScene);
	_childObject->handleUpdate();
}

void Module2800::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult != 2) {
				// TODO music stuff
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
			debug("scene 9 _moduleResult = %d", _moduleResult);
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
		//		
		case 1001:
			break;
		}
	} else {
		switch (_vm->gameState().sceneNum) {
		case 0:
			// TODO Module2800_sub4731E0(true);
			break;
		case 1:
			// TODO Module2800_sub4731E0(false);
			break;
		}
	}
}

Scene2801::Scene2801(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	// TODO Weird palette glitches in the mouse cursor and sprite, check this later

	// TODO _vm->gameModule()->initScene2801Vars();

	_surfaceFlag = true;
	SetMessageHandler(&Scene2801::handleMessage);
	SetUpdateHandler(&Scene::update);

	if (getGlobalVar(0x4DE80AC0) == 0) {
		insertStaticSprite(0x0001264C, 100);
	}

	if (which < 0) {
		insertKlayman<KmScene2801>(194, 430);
		setMessageList(0x004B6BB8);
	} else if (which == 1) {
		insertKlayman<KmScene2801>(443, 398);
		setMessageList(0x004B6BC0);
	} else if (which == 2) {
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene2801>(312, 432);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene2801>(194, 432);
		}
		setMessageList(0x004B6C10);
	} else {
		insertKlayman<KmScene2801>(0, 432);
		setMessageList(0x004B6BB0);
	}

	if (getGlobalVar(0x09880D40)) {
		setRectList(0x004B6CE0);
		setBackground(0x01400666);
		setPalette(0x01400666);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x01400666, 0, 256, 0);
		_sprite1 = insertStaticSprite(0x100CA0A8, 1100);
		_sprite2 = insertStaticSprite(0x287C21A4, 1100);
		_klayman->setClipRect(_sprite1->getDrawRect().x, 0, _sprite2->getDrawRect().x2(), 480);
		insertMouse433(0x0066201C);
		_asTape = insertSprite<AsScene1201Tape>(this, 8, 1100, 302, 437, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape); 
	} else if (getGlobalVar(0x08180ABC)) {
		setRectList(0x004B6CD0);
		setBackground(0x11E00684);
		setPalette(0x11E00684);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x11E00684, 0, 256, 0);
		_sprite2 = insertStaticSprite(0x061601C8, 1100);
		_klayman->setClipRect(0, 0, _sprite2->getDrawRect().x2(), 480);
		insertMouse433(0x00680116);
		_asTape = insertSprite<Class606>(this, 8, 1100, 302, 437, 0x01142428);
		_vm->_collisionMan->addSprite(_asTape); 
	} else {
		setRectList(0x004B6CF0);
		setBackground(0x030006E6);
		setPalette(0x030006E6);
		_paletteHash = 0x15021024;
		_palette->addBasePalette(0x030006E6, 0, 256, 0);
		_sprite2 = insertStaticSprite(0x273801CE, 1100);
		_klayman->setClipRect(0, 0, _sprite2->getDrawRect().x2(), 480);
		insertMouse433(0x006E2038);
		_asTape = insertSprite<AsScene1201Tape>(this, 8, 1100, 302, 437, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape); 
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
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene2801::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	// TODO: case 0x000D:
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
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

AsScene2803LightCord::AsScene2803LightCord(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _fileHash1(fileHash1), _fileHash2(fileHash2),
	_flag1(false), _flag2(false), _soundResource(vm) {

	createSurface(1010, 640, 480); // TODO Use correct size	from the two hashes
	SetUpdateHandler(&AnimatedSprite::update);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	_x = x;
	_y = y;
	sub4343C0();
}

uint32 AsScene2803LightCord::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (!_flag2 && param.asInteger() == calcHash("ClickSwitch")) {
			sendMessage(_parentScene, 0x480F, 0);
			_soundResource.play(0x4E1CA4A0);
		}
		break;
	case 0x480F:
		sub434380();
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

uint32 AsScene2803LightCord::handleMessage434340(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2803LightCord::sub434380() {
	_flag2 = false;
	_flag1 = true;
	startAnimation(_fileHash2, 0, -1);
	SetMessageHandler(&AsScene2803LightCord::handleMessage434340);
	NextState(&AsScene2803LightCord::sub4343C0);
}

void AsScene2803LightCord::sub4343C0() {
	_flag1 = false;
	startAnimation(_fileHash1, 0, -1);
	SetMessageHandler(&AsScene2803LightCord::handleMessage);
}

void AsScene2803LightCord::setFileHashes(uint32 fileHash1, uint32 fileHash2) {
	_fileHash1 = fileHash1;
	_fileHash2 = fileHash2;
	if (_flag1) {
		startAnimation(_fileHash2, _currFrameIndex, -1);
		_flag2 = true;
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
	NextState(&AsScene2803Rope::sub476A70);
	_x = x;
	_y = -276;
}

uint32 AsScene2803Rope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		startAnimation(0x9D098C23, 50, -1);
		SetMessageHandler(&AsScene2803Rope::handleMessage4769E0);
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

uint32 AsScene2803Rope::handleMessage4769E0(int messageNum, const MessageParam &param, Entity *sender) {
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

void AsScene2803Rope::sub476A70() {
	startAnimation(0x8258A030, 0, 1);
	NextState(&AsScene2803Rope::stHide);
}

void AsScene2803Rope::stHide() {
	stopAnimation();
	setVisible(false);
}

Scene2803::Scene2803(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _field134(0) {
	
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

	setGlobalVar(0x1C1B8A9A, 1);
	_vm->gameModule()->initScene2808Vars1();
	
	SetMessageHandler(&Scene2803::handleMessage);
	
	loadDataResource(0x00900849);
	
	_surfaceFlag = true;
	
	_background = new DirtyBackground(_vm, 0);
	_background->createSurface(0, 640, 480);
	addBackground(_background);
	
	setPalette(0x412A423E);
	addEntity(_palette);
	
	insertMouse433(0xA423A41A);
	
	if (getSubVar(0x0C601058, 0) == 0) {
		_asTestTubeOne = (StaticSprite*)insertStaticSprite(0x66121222, 100);
	} else {
		_asTestTubeOne = (StaticSprite*)insertSprite<AsScene2803TestTubeOne>(
			kScene2803FileHashes1[getSubVar(0x0C601058, 0)],
			kScene2803FileHashes2[getSubVar(0x0C601058, 0)]);
	}
	
	if (getSubVar(0x0C601058, 1) == 3)
		_asTestTubeTwo = (StaticSprite*)insertStaticSprite(0x64330236, 100);

	if (getSubVar(0x0C601058, 2) == 3)
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

	_clipRects1[0].x1 = 0;
	_clipRects1[0].y1 = 0;
	_clipRects1[0].x2 = 640;
	_clipRects1[0].y2 = _sprite8->getDrawRect().y2();
	
	_clipRects1[1].x1 = _sprite8->getDrawRect().x2();
	_clipRects1[1].y1 = _sprite8->getDrawRect().y2();
	_clipRects1[1].x2 = 640;
	_clipRects1[1].y2 = 480;

	_clipRects2[0].x1 = _sprite5->getDrawRect().x;
	_clipRects2[0].y1 = 0;
	_clipRects2[0].x2 = _sprite5->getDrawRect().x2();
	_clipRects2[0].y2 = _sprite5->getDrawRect().y2();
	
	_clipRects2[1].x1 = _sprite6->getDrawRect().x;
	_clipRects2[1].y1 = 0;
	_clipRects2[1].x2 = _sprite3->getDrawRect().x;
	_clipRects2[1].y2 = _sprite6->getDrawRect().y2();
	
	_clipRects2[2].x1 = _sprite3->getDrawRect().x;
	_clipRects2[2].y1 = 0;
	_clipRects2[2].x2 = _sprite4->getDrawRect().x2();
	_clipRects2[2].y2 = 480;

	if (which < 0) {
		insertKlayman<KmScene2803>(302, 445, _clipRects1, 2);
		setMessageList(0x004B79F0);
		sub476090();
	} else if (which == 1) {
		insertKlayman<KmScene2803>(200, 445, _clipRects1, 2);
		setMessageList(0x004B79C8);
		sub476090();
	} else if (which == 3) {
		NPoint pt = _dataResource.getPoint(0xC2A08694);
		insertKlayman<KmScene2803>(pt.x, pt.y, _clipRects2, 3);
		setMessageList(0x004B7A00);
		sub475FF0();
	} else if (which == 5) {
		insertKlayman<KmScene2803>(253, 298, _clipRects2, 3);
		setMessageList(0x004B7A00);
		sub475FF0();
	} else if (which == 6) {
		_asRope = insertSprite<AsScene2803Rope>(this, 384);
		_asRope->setClipRect(0, 25, 640, 480);
		insertKlayman<KmScene2803>(384, 0, _clipRects1, 2);
		sendEntityMessage(_klayman, 0x1014, _asRope);
		_klayman->setClipRect(0, 25, 640, 480);
		setMessageList(0x004B7A78);
		sub476090();
	} else if (which == 2) {
		insertKlayman<KmScene2803>(400, 445, _clipRects1, 2);
		setMessageList(0x004B79F8);
		sub476090();
	} else {
		insertKlayman<KmScene2803>(50, 231, _clipRects2, 3);
		setMessageList(0x004B79C0);
		sub475FF0();
	}

	sub476180();

}

void Scene2803::update475E40() {
	if (_klayman->getX() < 350) {
		sub4765D0();
	} else {
		sub4765F0();
	}
	Scene::update();
}

uint32 Scene2803::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480F:
		sub476130();
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
			sendEntityMessage(_klayman, 0x1014, _asLightCord);
			setMessageList(0x004B7A68);
		} else if (param.asInteger() == 0x9626F390)
			setMessageList(0x004B7A88);
		break;
	case 0x482A:
		sub475FF0();
		sub4765F0();
		break;
	case 0x482B:
		sub476090();
		sub4765D0();
		break;
	}
	return messageResult;
}

void Scene2803::sub475FF0() {
	SetUpdateHandler(&Scene2803::update475E40);
	// TODO _klayman->setClipRects(_clipRects2, 3);
	sendMessage(_klayman, 0x482C, 0xE5A48297);
	_sprite3->setVisible(true);
	_sprite4->setVisible(true);
	_sprite5->setVisible(true);
	_sprite6->setVisible(true);
	_sprite7->setVisible(true);
	_sprite8->setVisible(false);
	_sprite9->setVisible(false);
}

void Scene2803::sub476090() {
	SetUpdateHandler(&Scene::update);
	// TODO _klayman->setClipRects(_clipRects1, 2);
	sendMessage(_klayman, 0x482C, 0);
	_sprite3->setVisible(false);
	_sprite4->setVisible(false);
	_sprite5->setVisible(false);
	_sprite6->setVisible(false);
	_sprite7->setVisible(false);
	_sprite8->setVisible(true);
	_sprite9->setVisible(true);
}

void Scene2803::sub476130() {
	setGlobalVar(0x190A1D18, getGlobalVar(0x190A1D18) ? 0 : 1);
	sub476180();
}

void Scene2803::sub476180() {
	// TODO? g_screen->resetDirtyRects();
	if (getGlobalVar(0x190A1D18)) {
		_asLightCord->setFileHashes(0x8FAD5932, 0x276E1A3D);
		_background->load(0x412A423E);
		_palette->addPalette(0x412A423E, 0, 256, 0);
		_palette->addBasePalette(0x412A423E, 0, 256, 0);
		_sprite3->load(0xA40EF2FB, false, false);
		_sprite3->update();
		_sprite4->load(0x0C03AA23, false, false);
		_sprite4->update();
		_sprite5->load(0x2A822E2E, false, false);
		_sprite5->update();
		_sprite6->load(0x2603A202, false, false);
		_sprite6->update();
		_sprite7->load(0x24320220, false, false);
		_sprite7->update();
		_mouseCursor->load(0xA423A41A);
		_mouseCursor->updateCursor();
		_sprite8->load(0x3C42022F, false, false);
		_sprite8->update();
		_sprite9->load(0x341A0237, false, false);
		_sprite9->update();
		if (getSubVar(0x0C601058, 0) == 0) {
			_asTestTubeOne->load(0x66121222, false, false);
			_asTestTubeOne->update();
		} else
			sendMessage(_asTestTubeOne, 0x2000, 0);
		if (getSubVar(0x0C601058, 1) == 3) {
			_asTestTubeTwo->load(0x64330236, false, false);
			_asTestTubeTwo->update();
		}
		if (getSubVar(0x0C601058, 2) == 3) {
			_asTestTubeThree->load(0x2E4A22A2, false, false);
			_asTestTubeThree->update();
		}
		_sprite10->setVisible(true);
	} else {
		_asLightCord->setFileHashes(0xAFAD591A, 0x276E321D);
		_background->load(0x29800A01);
		_palette->addPalette(0x29800A01, 0, 256, 0);
		_palette->addBasePalette(0x29800A01, 0, 256, 0);
		_sprite3->load(0x234340A0, false, false);
		_sprite3->update();
		_sprite4->load(0x16202200, false, false);
		_sprite4->update();
		_sprite5->load(0x1030169A, false, false);
		_sprite5->update();
		_sprite6->load(0x1600A6A8, false, false);
		_sprite6->update();
		_sprite7->load(0xD0802EA0, false, false);
		_sprite7->update();
		_mouseCursor->load(0x00A05290);
		_mouseCursor->updateCursor();
		_sprite8->load(0x108012C1, false, false);
		_sprite8->update();
		_sprite9->load(0x708072E0, false, false);
		_sprite9->update();
		if (getSubVar(0x0C601058, 0) == 0) {
			_asTestTubeOne->load(0x50C027A8, false, false);
			_asTestTubeOne->update();
		} else
			sendMessage(_asTestTubeOne, 0x2000, 1);
		if (getSubVar(0x0C601058, 1) == 3) {
			_asTestTubeTwo->load(0xD48077A0, false, false);
			_asTestTubeTwo->update();
		}
		if (getSubVar(0x0C601058, 2) == 3) {
			_asTestTubeThree->load(0x30022689, false, false);
			_asTestTubeThree->update();
		}
		_sprite10->setVisible(false);
	}
	sub476610();
}

void Scene2803::sub4765D0() {
	if (_field134) {
		_field134 = 0;
		sub476610();
	}
}

void Scene2803::sub4765F0() {
	if (!_field134) {
		_field134 = 1;
		sub476610();
	}
}

void Scene2803::sub476610() {
	uint32 fadePaletteHash;
	if (getGlobalVar(0x190A1D18))
		fadePaletteHash = _field134 ? 0xB103B604 : 0x412A423E;
	else
		fadePaletteHash = _field134 ? 0x0263D144 : 0x29800A01;
	_palette->addBasePalette(fadePaletteHash, 0, 64, 0);
	_palette->startFadeToPalette(12);
}

Scene2803b::Scene2803b(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _palStatus(0) {

	static const uint32 kScene2803bFileHashes1[] = {
		0, 0x081000F1, 0x08100171, 0x08100271
	};

	static const uint32 kScene2803bFileHashes2[] = {
		0, 0x286800D4, 0x286806D4, 0x28680AD4
	};
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2803b::handleMessage);

	loadDataResource(0x81120132);
	insertMouse433(0x00A05290);

	insertSprite<AsScene2803LightCord>(this, 0xAFAD591A, 0x276E321D, 578, 200);

	if (getGlobalVar(0x190A1D18)) {
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

		if (getSubVar(0x0C601058, 0) == 0)
			insertStaticSprite(0x66121222, 100);
		else
			insertSprite<AnimatedSprite>(kScene2803bFileHashes1[getSubVar(0x0C601058, 0)], 100, 529, 326);

		if (getSubVar(0x0C601058, 1) == 3)
			insertStaticSprite(0x64330236, 100);

		if (getSubVar(0x0C601058, 2) == 3)
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

		setSubVar(0x0C601058, 0, 1);//DEBUG, FIXME crashes when not done?!
		if (getSubVar(0x0C601058, 0) == 0)
			insertStaticSprite(0x50C027A8, 100);
		else
			insertSprite<AnimatedSprite>(kScene2803bFileHashes2[getSubVar(0x0C601058, 0)], 100, 529, 326);

		if (getSubVar(0x0C601058, 1) == 3)
			insertStaticSprite(0xD48077A0, 100);

		if (getSubVar(0x0C601058, 2) == 3)
			insertStaticSprite(0x30022689, 100);

	}
	
	_sprite6->setVisible(false);
	_sprite7->setVisible(false);

	if (which < 0) {
		insertKlayman<KmScene2803b>(479, 435);
		sub460110();
		setMessageList(0x004B60D8);
	} else if (which == 3) {
		NPoint pt = _dataResource.getPoint(0x096520ED);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460090();
		setMessageList(0x004B6100);
		_klayman->setRepl(64, 0);
	} else if (which == 4) {
		NPoint pt = _dataResource.getPoint(0x20C6238D);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460090();
		setMessageList(0x004B60F8);
		_klayman->setRepl(64, 0);
	} else if (which == 5) {
		NPoint pt = _dataResource.getPoint(0x2146690D);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460090();
		setMessageList(0x004B6100);
		_klayman->setRepl(64, 0);
	} else if (which == 2) {
		NPoint pt = _dataResource.getPoint(0x104C03ED);
		insertKlayman<KmScene2803b>(pt.x, pt.y);
		sub460110();
		setMessageList(0x004B6138);
	} else {
		insertKlayman<KmScene2803b>(135, 444);
		sub460110();
		setMessageList(0x004B60E0, false);
		_sprite6->setVisible(true);
		_sprite7->setVisible(true);
	}

}

uint32 Scene2803b::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xB4E4884C) {
			setMessageList(0x004B6180);
		} else if (param.asInteger() == 0xB1FDAB2E) {
			NPoint pt = _dataResource.getPoint(0x0D84A1AD);
			_klayman->setX(pt.x);
			_klayman->setY(pt.y);
			_klayman->processDelta();
			sub460110();
			_klayman->setClipRect(517, 401, 536, 480);
			setMessageList(0x004B6198);
		} else if (param.asInteger() == 0xB00C7C48) {
			setMessageList(0x004B6108);
		} else if (param.asInteger() == 0x61F64346) {
			setMessageList(0x004B6150);
		} else if (param.asInteger() == 0xAC69A28D) {
			setMessageList(0x004B6168);
		} else if (param.asInteger() == 0x00086212) {
			_klayman->setClipRect(0, 0, 560, 315);
			_klayman->setX(560);
			_klayman->setY(315);
			_klayman->processDelta();
			sub460090();
			setMessageList(0x004B61A0);
		} else if (param.asInteger() == 0x002CAA68) {
			setMessageList(0x004B61A8);
		}
		break;
	case 0x482A:
		if (_klayman->getX() < 200) {
			sub4601D0();
		} else if (_klayman->getX() < 500) {
			setSurfacePriority(_sprite5->getSurface(), 1100);
			sendMessage(_klayman, 0x482C, 0);
			sub4601B0();
		} else {
			_klayman->setClipRect(517, 401, 536, 480);
			sub4601B0();
		}
		break;
	case 0x482B:
		_sprite6->setVisible(false);
		_sprite7->setVisible(false);
		_klayman->setClipRect(0, 0, 640, 480);
		setSurfacePriority(_sprite5->getSurface(), 900);
		sendMessage(_klayman, 0x482C, 0x2086222D);
		break;
	}
	return 0;
}

void Scene2803b::update45FCB0() {
	if (_klayman->getX() < 388) {
		_klayman->setClipRect(_sprite3->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		sub460170();
	} else if (_klayman->getX() < 500) {
		_klayman->setClipRect(0, 0, _sprite1->getDrawRect().x2(), _sprite1->getDrawRect().y2());
		sub460190();
	}
	Scene::update();
}

void Scene2803b::update45FD50() {
	if (_klayman->getX() > 194 && _klayman->getX() < 273)
		sub4601B0();
	else if (_klayman->getX() > 155 && _klayman->getX() < 300)
		sub460170();
	Scene::update();
}

void Scene2803b::sub460090() {
	SetUpdateHandler(&Scene2803b::update45FCB0);
	sendMessage(_klayman, 0x482C, 0x23C630D9);
	_klayman->setClipRect(0, 0, _sprite1->getDrawRect().x2(), _sprite1->getDrawRect().y2());
	_klayman->setRepl(64, 0);
	_sprite1->setVisible(true);
}

void Scene2803b::sub460110() {
	SetUpdateHandler(&Scene2803b::update45FD50);
	sendMessage(_klayman, 0x482C, 0x2086222D);
	_klayman->setClipRect(0, 0, 640, 480);
	_klayman->clearRepl();
	_sprite1->setVisible(false);
}

void Scene2803b::sub460170() {
	if (_palStatus != 0) {
		_palStatus = 0;
		sub4601F0(false);
	}
}

void Scene2803b::sub460190() {
	if (_palStatus != 1) {
		_palStatus = 1;
		sub4601F0(false);
	}
}

void Scene2803b::sub4601B0() {
	if (_palStatus != 2) {
		_palStatus = 2;
		sub4601F0(false);
	}
}

void Scene2803b::sub4601D0() {
	if (_palStatus != 3) {
		_palStatus = 3;
		sub4601F0(true);
	}
}

void Scene2803b::sub4601F0(bool flag) {
	if (getGlobalVar(0x190A1D18)) {
		switch (_palStatus) {
		case 1:
			_palette->addBasePalette(0x0A938204, 0, 64, 0);
			break;
		case 2:
			_palette->addBasePalette(0xB103B604, 0, 64, 0);
			break;
		case 3:
			// TODO _palette->sub_47BFB0(0, 64);
			break;
		default:
			_palette->addBasePalette(0x412A423E, 0, 64, 0);
			break;
		}
	} else {
		switch (_palStatus) {
		case 2:
			_palette->addBasePalette(0x0263D144, 0, 64, 0);
			break;
		case 3:
			// TODO _palette->sub_47BFB0(0, 64);
			break;
		default:
			_palette->addBasePalette(0x29800A01, 0, 64, 0);
			break;
		}
	}
	if (flag) {
		_palette->startFadeToPalette(0);
	} else {
		_palette->startFadeToPalette(12);
	}
}

SsScene2804RedButton::SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene),
	_soundResource(vm) {
	
	if (getGlobalVar(0x190A1D18))
		_spriteResource.load2(0x51A10202);
	else
		_spriteResource.load2(0x11814A21);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&SsScene2804RedButton::update);
	SetMessageHandler(&SsScene2804RedButton::handleMessage);
	_soundResource.load(0x44241240);
}

void SsScene2804RedButton::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804RedButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			_soundResource.play();
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
	
	_spriteResource.load2(0x8889B008);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
	SetMessageHandler(&SsScene2804LightCoil::handleMessage);
}

uint32 SsScene2804LightCoil::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		setVisible(true);
		StaticSprite::update();
		messageResult = 1;
		break;
	case 0x2003:
		setVisible(false);
		StaticSprite::update();
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804LightTarget::SsScene2804LightTarget(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(0x06092132);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
	SetMessageHandler(&SsScene2804LightTarget::handleMessage);
}

uint32 SsScene2804LightTarget::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		setVisible(true);
		StaticSprite::update();
		messageResult = 1;
		break;
	case 0x2005:
		setVisible(false);
		StaticSprite::update();
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2804Flash::SsScene2804Flash(NeverhoodEngine *vm)
	: StaticSprite(vm, 900), _soundResource(vm) {
	
	_spriteResource.load2(0x211003A0);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
	_soundResource.load(0xCB36BA54);
}

void SsScene2804Flash::show() {
	setVisible(true);
	StaticSprite::update();
	_soundResource.play();
}

SsScene2804BeamCoilBody::SsScene2804BeamCoilBody(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {
	
	_spriteResource.load2(0x9A816000);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
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
		setDoDeltaX(1);
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
	: AnimatedSprite(vm, 1100), _asCrystalWaves(asCrystalWaves), _crystalIndex(crystalIndex),
	_isShowing(false), _soundResource(vm) {

	static const NPoint kAsScene2804CrystalPoints[] = {
		{204, 196},
		{272, 316},
		{334, 206},
		{410, 334},
		{470, 180}
	};

	_colorNum = (int16)getSubVar(0xE11A1929, crystalIndex);
	_isLightOn = getGlobalVar(0x190A1D18) != 0;
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
	_soundResource.load(0x725294D4);
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene2804Crystal::show() {
	if (!_isLightOn) {
		setVisible(true);
		_isShowing = true;
		if (_asCrystalWaves)
			_asCrystalWaves->show();
		_soundResource.play();
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
		setSubVar(0xE11A1929, _crystalIndex, _colorNum);
	}
}

SsScene2804CrystalButton::SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex)
	: StaticSprite(vm, 900), _countdown(0), _parentScene(parentScene), _asCrystal(asCrystal),
	_crystalIndex(crystalIndex), _soundResource(vm) {

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
	
	if (getGlobalVar(0x190A1D18))
		_spriteResource.load2(kSsScene2804CrystalButtonFileHashes1[crystalIndex]);
	else
		_spriteResource.load2(kSsScene2804CrystalButtonFileHashes2[crystalIndex]);

	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	setVisible(false);
	_soundResource.load(0x44045140);
	_needRefresh = true;
	SetUpdateHandler(&SsScene2804CrystalButton::update);
	SetMessageHandler(&SsScene2804CrystalButton::handleMessage);
}

void SsScene2804CrystalButton::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2804CrystalButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_parentScene->isWorking()) {
			_soundResource.play();
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
	: AnimatedSprite(vm, 1400), _parentScene(parentScene), _ssBeamCoilBody(ssBeamCoilBody),
	_countdown(0), _soundResource(vm) {
	
	createSurface1(0x00494891, 1000);
	_x = 125;
	_y = 184;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	_soundResource.load(0x6352F051);
	// TODO Sound1ChList_addSoundResource(0xC5EA0B28, 0xEF56B094, true);
	SetUpdateHandler(&AsScene2804BeamCoil::update);
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
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
	// TODO _ssBeamCoilBody->update(); -> show()
	setVisible(true);
	startAnimation(0x00494891, 0, -1);
	_soundResource.play();
	SetMessageHandler(&AsScene2804BeamCoil::hmBeaming);
	NextState(&AsScene2804BeamCoil::stBeaming);
}

void AsScene2804BeamCoil::hide() {
	stopAnimation();
	SetMessageHandler(&AsScene2804BeamCoil::handleMessage);
	setVisible(false);
	_ssBeamCoilBody->setVisible(false);
	// TODO _ssBeamCoilBody->update(); -> hide()
	// TODO Sound1ChList_stop(0xEF56B094);
}

void AsScene2804BeamCoil::stBeaming() {
	startAnimation(0x00494891, 93, -1);
	NextState(&AsScene2804BeamCoil::stBeaming);
	// TODO Sound1ChList_playLooping(0xEF56B094);
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
	: Scene(vm, parentModule, true), _countdown1(0), _countdown2(0), _countdown3(0),
	_beamStatus(0), _isSolved(false), _isWorking(false) {

	initCrystalColors();

	_surfaceFlag = true;
	SetMessageHandler(&Scene2804::handleMessage);
	SetUpdateHandler(&Scene2804::update);

	//setGlobalVar(0x190A1D18, 1); // DEBUG Set lights on

	if (getGlobalVar(0x190A1D18)) {
		setBackground(0xA1D03005);
		setPalette(0xA1D03005);
		addEntity(_palette);
		insertMouse435(0x03001A15, 20, 620);
		_asCoil = insertSprite<SsScene2804LightCoil>();
		_asTarget = insertSprite<SsScene2804LightTarget>();
	} else {
		SsScene2804BeamCoilBody *ssBeamCoilBody;
		setBackground(0x01C01414);
		setPalette(0x01C01414);
		addEntity(_palette);
		insertMouse435(0x01410014, 20, 620);
		ssBeamCoilBody = insertSprite<SsScene2804BeamCoilBody>();
		_asCoil = insertSprite<AsScene2804BeamCoil>(this, ssBeamCoilBody);
		_asTarget = insertSprite<AsScene2804BeamTarget>();
		_ssFlash = insertSprite<SsScene2804Flash>();
	}
	
	_ssRedButton = insertSprite<SsScene2804RedButton>(this);
	_vm->_collisionMan->addSprite(_ssRedButton);

	for (uint crystalIndex = 0; crystalIndex < 5; crystalIndex++) {
		AsScene2804CrystalWaves *asCrystalWaves = NULL;
		if (crystalIndex < 4 && getGlobalVar(0x190A1D18) == 0)
			asCrystalWaves = insertSprite<AsScene2804CrystalWaves>(crystalIndex);
		_asCrystals[crystalIndex] = insertSprite<AsScene2804Crystal>(asCrystalWaves, crystalIndex);
		_ssCrystalButtons[crystalIndex] = insertSprite<SsScene2804CrystalButton>(this, _asCrystals[crystalIndex], crystalIndex);
		_vm->_collisionMan->addSprite(_ssCrystalButtons[crystalIndex]);
	}

}

uint32 Scene2804::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO DEBUG
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x000D:
		// TODO DEBUG
		break;
	case 0x2000:
		_isWorking = true;
		sendMessage(_asCoil, 0x2002, 0);
		if (getGlobalVar(0x190A1D18)) {
			sendMessage(_asTarget, 0x2004, 0);
			_countdown2 = 48;
		}
		break;
	case 0x2001:
		_countdown3 = 2;
		_isSolved = true;
		_beamStatus = 0;
		for (uint index = 0; index < 5; index++)
			if (_asCrystals[index]->getColorNum() != (int16)getSubVar(0xD4B2089C, index))
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
				setGlobalVar(0x1860C990, 1);
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

void Scene2804::initCrystalColors() {
	// TODO Maybe move this into the GameModule so all puzzle init code is together
	if (getGlobalVar(0xDE2EC914) == 0) {
		TextResource textResource(_vm);
		const char *textStart, *textEnd;
		textResource.load(0x46691611);
		textStart = textResource.getString(0, textEnd);
		for (uint index = 0; index < 5; index++) {
			char colorLetter = (byte)textStart[index];
			byte correctColorNum = 0, misalignedColorNum;
			switch (colorLetter) {
			case 'B':
				correctColorNum = 4;
				break;
			case 'G':
				correctColorNum = 3;
				break;
			case 'O':
				correctColorNum = 1;
				break;
			case 'R':
				correctColorNum = 0;
				break;
			case 'V':
				correctColorNum = 5;
				break;
			case 'Y':
				correctColorNum = 2;
				break;
			}
			do {
				misalignedColorNum = _vm->_rnd->getRandomNumber(6 - 1);
			} while (misalignedColorNum == correctColorNum);
			setSubVar(0xD4B2089C, index, correctColorNum);
			setSubVar(0xE11A1929, index, misalignedColorNum);
		}
		setGlobalVar(0xDE2EC914, 1);
	}
}

Scene2805::Scene2805(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2805::handleMessage);

	setBackground(0x08021E04);
	setPalette(0x08021E04);
	_palette->addPalette(0x8A6B1F91, 0, 65, 0);
	insertMouse433(0x21E00088);

	_sprite1 = insertStaticSprite(0x008261E7, 1100);
	_sprite2 = insertStaticSprite(0x020CE421, 1100);

	if (which < 0) {
		insertKlayman<KmScene2805>(380, 338);
		setMessageList(0x004AE1C8);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE1D0, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE288, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE1E0, false);
		sendMessage(this, 0x2000, 1);
	} else {
		insertKlayman<KmScene2805>(340, 338);
		setMessageList(0x004AE1C0);
		sendMessage(this, 0x2000, 0);
	}

	_klayman->setClipRect(_sprite1->getDrawRect().x, 0, _sprite2->getDrawRect().x2(), 480);

}

uint32 Scene2805::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger()) {
			setRectList(0x004AE318);
			_klayman->setKlaymanIdleTable3();
		} else {
			setRectList(0x004AE308);
			_klayman->setKlaymanIdleTable1();
		}
		break;
	}
	return 0;
}

AsScene2806Spew::AsScene2806Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {
	
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
		_soundResource.play(0x48640244);
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
	: Scene(vm, parentModule, true) {

	Sprite *tempSprite;

	_surfaceFlag = true;
	SetMessageHandler(&Scene2806::handleMessage);
	SetUpdateHandler(&Scene2806::update);
	
	loadDataResource(0x98182003);
	loadHitRectList();
	
	_pointList = _dataResource.getPointArray(0x3606A422);

	insertMouse433(0x22114C13);	
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
	
	_fieldEC = true;

	tempSprite = insertStaticSprite(0xD2012C02, 1100);
	_clipRects[2].x1 = tempSprite->getDrawRect().x;
	_clipRects[2].y2 = tempSprite->getDrawRect().y2();
	_clipRects[3].y1 = tempSprite->getDrawRect().y2();
	_clipRects[1].x2 = tempSprite->getDrawRect().x2();

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
		insertKlayman<KmScene2806>(441, 423, false, _clipRects, 4);
		setMessageList(0x004AF098);
	} else if (which == 1) {
		insertKlayman<KmScene2806>(378, 423, false, _clipRects, 4);
		setMessageList(0x004AF098);
	} else if (which == 2) {
		insertKlayman<KmScene2806>(378, 423, false, _clipRects, 4);
		setMessageList(0x004AF0C8, false);
	} else if (which == 3) {
		insertKlayman<KmScene2806>(378, 423, true, _clipRects, 4);
		setMessageList(0x004AF0A0, false);
		setGlobalVar(0x1860C990, 0);
	} else {
		insertKlayman<KmScene2806>(670, 423, false, _clipRects, 4);
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

	int16 x = MIN<int16>(_klayman->getX(), 639);
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
	: Scene(vm, parentModule, true) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2807::handleMessage);

	if (getSubVar(0x0C601058, 0) == 1) {
		insertStaticSprite(0x103021E2, 300);
	} else if (getSubVar(0x0C601058, 0) == 2) {
		insertStaticSprite(0x103022E2, 300);
	} else if (getSubVar(0x0C601058, 0) == 3) {
		insertStaticSprite(0x103024E2, 300);
	}

	if (getSubVar(0x0C601058, 1) == 1) {
		insertStaticSprite(0x4800A52A, 200);
	} else if (getSubVar(0x0C601058, 1) == 2) {
		insertStaticSprite(0x4800A62A, 200);
	} else if (getSubVar(0x0C601058, 1) == 3) {
		insertStaticSprite(0x4800A02A, 200);
	}

	if (getSubVar(0x0C601058, 2) == 1) {
		insertStaticSprite(0x31203430, 100);
	} else if (getSubVar(0x0C601058, 2) == 2) {
		insertStaticSprite(0x31203400, 100);
	} else if (getSubVar(0x0C601058, 2) == 3) {
		insertStaticSprite(0x31203460, 100);
	}

	_surfaceFlag = true;
	setBackground(0x3E049A95);
	setPalette(0x3E049A95);
	insertMouse435(0x49A913E8, 20, 620);

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
	
	_spriteResource.load2(kClass428FileHashes[testTubeSetNum * 3 + testTubeIndex]);
	createSurface(1500, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_deltaRect = _drawRect;
	processDelta();
	SetUpdateHandler(&SsScene2808Dispenser::update);
	SetMessageHandler(&SsScene2808Dispenser::handleMessage);
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
}

void SsScene2808Dispenser::update() {
	StaticSprite::update();
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
	StaticSprite::update();
	if (_testTubeSetNum == 0) {
		_countdown = kClass428Countdowns1[index];
	} else {
		_countdown = kClass428Countdowns2[index];
	}
}

AsScene2808TestTube::AsScene2808TestTube(NeverhoodEngine *vm, int testTubeSetNum, int testTubeIndex, SsScene2808Dispenser *ssDispenser)
	: AnimatedSprite(vm, 1100), _testTubeSetNum(testTubeSetNum), _testTubeIndex(testTubeIndex), _ssDispenser(ssDispenser),
	_soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_soundResource4(vm), _soundResource5(vm), _soundResource6(vm),
	_soundResource7(vm), _soundResource8(vm), _soundResource9(vm), _fillLevel(0) {

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
		_soundResource1.load(0x30809E2D);
		_soundResource2.load(0x72811E2D);
		_soundResource3.load(0x78B01625);
	} else {
		_soundResource4.load(0x70A41E0C);
		_soundResource5.load(0x50205E2D);
		_soundResource6.load(0xF8621E2D);
		_soundResource7.load(0xF1A03C2D);
		_soundResource8.load(0x70A43D2D);
		_soundResource9.load(0xF0601E2D);
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


	if ((int)_fillLevel >= _testTubeSetNum * 3 + 3)
		return;
		
	if (_testTubeSetNum == 0) {
		switch (_fillLevel) {
		case 0:
			_soundResource1.play();
			break;
		case 1:
			_soundResource2.play();
			break;
		case 2:
			_soundResource3.play();
			break;
		}
		setVisible(true);
		startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices1[_fillLevel], kClass490FrameIndices1[_fillLevel + 1]);
		_newStickFrameIndex = kClass490FrameIndices1[_fillLevel + 1];
	} else {
		switch (_fillLevel) {
		case 0:
			_soundResource4.play();
			break;
		case 1:
			_soundResource5.play();
			break;
		case 2:
			_soundResource6.play();
			break;
		case 3:
			_soundResource7.play();
			break;
		case 4:
			_soundResource8.play();
			break;
		case 5:
			_soundResource9.play();
			break;
		}
		setVisible(true);
		startAnimation(kClass490FileHashes[_testTubeIndex], kClass490FrameIndices2[_fillLevel], kClass490FrameIndices2[_fillLevel + 1]);
		_newStickFrameIndex = kClass490FrameIndices2[_fillLevel + 1];
	}
	_ssDispenser->startCountdown(_fillLevel);
	_fillLevel++;
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
	: AnimatedSprite(vm, 1300), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum), _isActivated(false),
	_soundResource(vm) {
	
	_soundResource.load(0xE18D1F30);
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
			_soundResource.play();
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
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _testTubeSetNum(testTubeSetNum),
	_soundResource(vm) {

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
	_soundResource.load(0x6389B652);
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
	_soundResource.play();
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
	: Scene(vm, parentModule, true), _countdown(0), _testTubeSetNum(which), _leaveResult(0), _isFlowing(false) {

	Sprite *tempSprite;

	if (which == 0) {
		_vm->gameModule()->initScene2808Vars1();
	} else {
		_vm->gameModule()->initScene2808Vars2();
	}
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2808::handleMessage);
	SetUpdateHandler(&Scene2808::update);

	setBackground(kScene2808FileHashes1[which]);
	setPalette(kScene2808FileHashes1[which]);

	tempSprite = insertSprite<AsScene2808Handle>(this, which);
	_vm->_collisionMan->addSprite(tempSprite);

	_asFlow = insertSprite<AsScene2808Flow>(this, which);
	insertSprite<AsScene2808LightEffect>(which);

	for (int testTubeIndex = 0; testTubeIndex < 3; testTubeIndex++) {
		SsScene2808Dispenser *ssDispenser = insertSprite<SsScene2808Dispenser>(this, which, testTubeIndex);
		_vm->_collisionMan->addSprite(ssDispenser);
		_asTestTubes[testTubeIndex] = insertSprite<AsScene2808TestTube>(which, testTubeIndex, ssDispenser);
		_vm->_collisionMan->addSprite(_asTestTubes[testTubeIndex]);
	}
	
	insertMouse433(kScene2808FileHashes2[which]);

}

uint32 Scene2808::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO DEBUG Stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !isAnyTestTubeFilled()) {
			leaveScene(1);
		}
		break;
	case 0x000D:
		// TODO DEBUG Stuff
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

	// DEBUG: Show correct values
	debug("---------------");
	debug("%03d %03d %03d", getSubVar(0x0C601058, 0), getSubVar(0x0C601058, 1), getSubVar(0x0C601058, 2));
	debug("%03d %03d %03d", _asTestTubes[0]->getFillLevel(), _asTestTubes[1]->getFillLevel(), _asTestTubes[2]->getFillLevel());
	
	Scene::update();
	if (_countdown != 0 && (--_countdown) == 0) {
		leaveScene(_leaveResult);
	}
}

bool Scene2808::isMixtureGood() {
	if (_testTubeSetNum == 0) {
		return
			_asTestTubes[0]->getFillLevel() == getSubVar(0x0C601058, 0) &&
			_asTestTubes[1]->getFillLevel() == getSubVar(0x0C601058, 1) &&
			_asTestTubes[2]->getFillLevel() == getSubVar(0x0C601058, 2);
	} else {
		return
			_asTestTubes[0]->getFillLevel() == getSubVar(0x40005834, 0) &&
			_asTestTubes[1]->getFillLevel() == getSubVar(0x40005834, 1) &&
			_asTestTubes[2]->getFillLevel() == getSubVar(0x40005834, 2);
	}
}

bool Scene2808::isAnyTestTubeFilled() {
	return
		_asTestTubes[0]->getFillLevel() > 0 ||
		_asTestTubes[1]->getFillLevel() > 0 ||
		_asTestTubes[2]->getFillLevel() > 0;
}

AsScene2809Spew::AsScene2809Spew(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {

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
		_soundResource.play(0x48640244);
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
	: Scene(vm, parentModule, true) {

	Sprite *tempSprite;
		
	_surfaceFlag = true;
	SetMessageHandler(&Scene2809::handleMessage);
	SetUpdateHandler(&Scene2809::update);
	
	loadDataResource(0x1830009A);
	loadHitRectList();
	
	_pointList = _dataResource.getPointArray(0x064A310E);

	setBackground(0xB22116C5);
	setPalette(0xB22116C5);
	insertMouse433(0x116C1B2A);

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
		insertKlayman<KmScene2809>(226, 423, false, _clipRects, 4);
		setMessageList(0x004B5B90);
	} else if (which == 1) {
		insertKlayman<KmScene2809>(262, 423, false, _clipRects, 4);
		setMessageList(0x004B5B90);
	} else if (which == 2) {
		insertKlayman<KmScene2809>(262, 423, false, _clipRects, 4);
		setMessageList(0x004B5BD0);
	} else if (which == 3) {
		insertKlayman<KmScene2809>(262, 423, true, _clipRects, 4);
		setMessageList(0x004B5BA8, false);
		setGlobalVar(0x1860C990, 0);
	} else {
		insertKlayman<KmScene2809>(-30, 423, false, _clipRects, 4);
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

	int16 x = MAX<int16>(_klayman->getX(), 2);
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
	: Scene(vm, parentModule, true) {

	Sprite *tempSprite;

	_surfaceFlag = true;
	SetMessageHandler(&Scene2810::handleMessage);
	
	setBackground(0x26508804);
	setPalette(0x26508804);
	insertMouse433(0x0880026D);

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

	if (getGlobalVar(0x1860C990)) {
		_asTape = insertSprite<AsScene1201Tape>(this, 0, 900, 245, 429, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape);
	} else {
		_asTape = insertSprite<AsScene1201Tape>(this, 0, 1100, 245, 429, 0x9148A011);
		_vm->_collisionMan->addSprite(_asTape);
	}

	_sprite1 = insertStaticSprite(0x430001C4, 1200);

	if (getGlobalVar(0x0018CA22)) {
		setGlobalVar(0xCB45DE03, 1);
		if (getGlobalVar(0x1860C990)) {
			_sprite4 = insertStaticSprite(0x82653808, 100);
		} else {
			_sprite4 = insertStaticSprite(0x82653808, 1100);
		}
		_sprite4->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
	}
	
	if (which < 0) {
		if (getGlobalVar(0x1860C990)) {
			insertKlayman<KmScene2810Small>(240, 448);
			_klayman->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
			setMessageList(0x004AE438);
			setRectList(0x004AE810);
			_flag1 = false;
			_vm->_collisionMan->removeSprite(_asTape);
		} else {
			insertKlayman<KmScene2810>(300, 424, _clipRects, 2);
			setMessageList(0x004AE438);
			if (getGlobalVar(0x0018CA22))
				loadDataResource(0x84130112);
			else
				loadDataResource(0x84500132);
			tempSprite = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
			tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
			_clipRects[0].y1 = _sprite1->getDrawRect().y;
			_flag1 = false;
		}
	} else if (which == 1) {
		insertKlayman<KmScene2810>(186, 64, _clipRects, 2);
		setMessageList(0x004AE440);
		loadDataResource(0x84130112);
		tempSprite = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
		tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
		_flag1 = true;
		_clipRects[0].y1 = _sprite1->getDrawRect().y;
	} else if (which == 5) {
		insertStaticSprite(0xC3007EA0, 100);
		_sprite2 = insertStaticSprite(0x02780936, 1100);
		_sprite3 = insertStaticSprite(0x1CA02160, 1100);
		_asRope = insertSprite<AsScene2810Rope>(this, 384);
		insertKlayman<KmScene2810>(384, 0, _clipRects, 0);
		sendEntityMessage(_klayman, 0x1014, _asRope);
		setMessageList(0x004AE738);
		_klayman->setClipRect(0, _sprite2->getDrawRect().y, 640, _sprite3->getDrawRect().y2());
		_asRope->setClipRect(0, _sprite2->getDrawRect().y, 640, _sprite3->getDrawRect().y2());
		// TODO Sound1ChList_addSoundResource(0x84400112, 0xC874EE6C, true);
		// TODO Sound1ChList_playLooping(0xC874EE6C);
		// TODO Sound1ChList_setVolume(0xC874EE6C, 50);
		_flag1 = false;
	} else if ((which >= 11 && which <= 14) || (which >= 19 && which <= 22) || which == 3) {
		if (getGlobalVar(0x1860C990)) {
			insertKlayman<KmScene2810Small>((int16)getGlobalVar(0x00D30138), 448);
			if (getGlobalVar(0xC0418A02))
				_klayman->setDoDeltaX(1);
			_klayman->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
			setMessageList(0x004AE6D8);
			setRectList(0x004AE810);
			_flag1 = false;
			_vm->_collisionMan->removeSprite(_asTape);
		} else {
			sub406650();
			if (getGlobalVar(0x00188211)) {
				setMessageList(0x004AE6E8);
				setGlobalVar(0x00188211, 0);
				_flag1 = false;
			} else {
				setMessageList(0x004AE6D8);
				_flag1 = false;
			}
		}
	} else if (which >= 15 && which <= 18) {
		sub406650();
		setMessageList(0x004AE6E0);
		_flag1 = false;
	} else if (which == 4) {
		if (getGlobalVar(0x1860C990)) {
			insertKlayman<KmScene2810Small>(473, 448);
			_klayman->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
			setMessageList(0x004AE428);
			setRectList(0x004AE810);
			_flag1 = false;
			_vm->_collisionMan->removeSprite(_asTape);
		} else {
			insertKlayman<KmScene2810>(450, 424, _clipRects, 2);
			setMessageList(0x004AE418);
			if (getGlobalVar(0x0018CA22))
				loadDataResource(0x84130112);
			else
				loadDataResource(0x84500132);
			tempSprite = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
			tempSprite->setClipRect(0, _sprite1->getDrawRect().y, 640, 480);
			_clipRects[0].y1 = _sprite1->getDrawRect().y;
			_flag1 = false;
		}
	} else {
		insertKlayman<KmScene2810Small>(120, 448);
		_klayman->setClipRect(_sprite5->getDrawRect().x, 0, 640, 480);
		setMessageList(0x004AE410);
		setRectList(0x004AE810);
		_flag1 = false;
		_vm->_collisionMan->removeSprite(_asTape);
	}

}

Scene2810::~Scene2810() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
	setGlobalVar(0x00D30138, _klayman->getX());
	// TODO Sound1ChList_sub_407AF0(0x84400112);
}

void Scene2810::sub406650() {
	Sprite *tempSprite;

	if (getGlobalVar(0x00188211)) {
		insertKlayman<KmScene2810>(430, 424, _clipRects, 2);
		_klayman->setDoDeltaX(1);
	} else {
		insertKlayman<KmScene2810>((int16)getGlobalVar(0x00D30138), 424, _clipRects, 2);
		if (getGlobalVar(0xC0418A02))
			_klayman->setDoDeltaX(1);
	}
	if (getGlobalVar(0x0018CA22))
		loadDataResource(0x84130112);
	else
		loadDataResource(0x84500132);
	tempSprite = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
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
		_flag1 = true;
		break;
	case 0x2001:
		if (getGlobalVar(0x0018CA22))
			loadDataResource(0x84130112);
		else
			loadDataResource(0x84500132);
		_flag1 = false;
		break;
	case 0x4826:
		if (sender == _asTape && getGlobalVar(0x1860C990) == 0 && !_flag1) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
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
	// TODO Sound1ChList_sub_407AF0(0x00B000E2);
}

uint32 AsScene2812Winch::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		startAnimation(0x20DA08A0, 0, -1);
		setVisible(true);
		// TODO Sound1ChList_addSoundResource(0x00B000E2, 0xC874EE6C, true);
		// TODO Sound1ChList_playLooping(0xC874EE6C);
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
		sub413E00();
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

uint32 AsScene2812Rope::handleMessage413DC0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene2812Rope::sub413E00() {
	sendMessage(_parentScene, 0x4806, 0);
	startAnimation(0x9D098C23, 0, -1);
	SetMessageHandler(&AsScene2812Rope::handleMessage413DC0);
}

AsScene2812TrapDoor::AsScene2812TrapDoor(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 0x805D0029, 100, 320, 240), _soundResource(vm) {
	
	SetMessageHandler(&AsScene2812TrapDoor::handleMessage);
	_newStickFrameIndex = 0;
}

uint32 AsScene2812TrapDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		startAnimation(0x805D0029, 0, -1);
		_soundResource.play(0xEA005F40);
		_newStickFrameIndex = -2;
		break;
	}
	return messageResult;
}

Scene2812::Scene2812(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _palStatus(0) {
	
	if (getGlobalVar(0xC0780812) && getGlobalVar(0x13382860) == 0)
		setGlobalVar(0x13382860, 3);

	_surfaceFlag = true;
	SetMessageHandler(&Scene2812::handleMessage);
	SetUpdateHandler(&Scene2812::update);
	
	setRectList(0x004AF700);
	
	setBackground(0x03600606);
	setPalette(0x03600606);
	addEntity(_palette);
	_palette->addBasePalette(0x03600606, 0, 256, 0);

	_sprite1 = insertStaticSprite(0x0C06C860, 1100);
	insertMouse433(0x0060203E);

	if (getGlobalVar(0x13382860) == 3) {
		_class545 = insertSprite<Class545>(this, 2, 1100, 474, 437);
		_vm->_collisionMan->addSprite(_class545);
	}

	_class606 = insertSprite<Class606>(this, 6, 1100, 513, 437, 0xA1361863);
	_vm->_collisionMan->addSprite(_class606);
	
	_asWinch = insertSprite<AsScene2812Winch>();
	_asTrapDoor = insertSprite<AsScene2812TrapDoor>();
	_asRope = insertSprite<AsScene2812Rope>(this);

	_sprite2 = insertStaticSprite(0x08478078, 1100);
	_sprite3 = insertStaticSprite(0x2203B821, 1100);
	_sprite4 = insertStaticSprite(0x08592134, 1100);

	if (which < 0) {
		_flag1 = false;
		insertKlayman<KmScene2812>(272, 432);
		setMessageList(0x004AF560);
		_sprite1->setVisible(false);
		_klayman->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
	} else if (which == 1) {
		_flag1 = false;
		insertKlayman<KmScene2812>(338, 398);
		setMessageList(0x004AF588);
		setPalStatus1(1);
		_klayman->setClipRect(_sprite1->getDrawRect().x, 0, _sprite1->getDrawRect().x2(), _sprite3->getDrawRect().y2());
	} else if (which == 2) {
		_flag1 = false;
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene2812>(554, 432);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene2812>(394, 432);
		}
		setMessageList(0x004AF5F0);
		_sprite1->setVisible(false);
		_klayman->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
	} else {
		_flag1 = true;
		insertKlayman<KmScene2812>(150, 582);
		setMessageList(0x004AF568);
		setPalStatus2(1);
		_sprite1->setVisible(false);
		_klayman->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
	}
	
	_asRope->setClipRect(0, _sprite2->getDrawRect().y, 640, _sprite3->getDrawRect().y2());
	
}

void Scene2812::update() {
	if (_klayman->getX() < 220)
		setPalStatus2(0);
	else if (_klayman->getX() < 240)
		setPalStatus0(0);
	Scene::update();
}

uint32 Scene2812::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0004269B)
			sendEntityMessage(_klayman, 0x1014, _asRope);
		break;
	case 0x2001:
		_flag1 = true;
		setRectList(0x004AF710);
		_klayman->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite4->getDrawRect().y2());
		break;
	case 0x2002:
		_flag1 = false;
		setRectList(0x004AF700);
		_klayman->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		break;
	case 0x4806:
		sendMessage(_asWinch, 0x2000, 0);
		sendMessage(_asTrapDoor, 0x2000, 0);
		break;
	case 0x4826:
		if (sender == _class606 && !_flag1) {
			sendEntityMessage(_klayman, 0x1014, _class606);
			setMessageList(0x004AF658);
		} else if (sender == _class545 && !_flag1) {
			sendEntityMessage(_klayman, 0x1014, _class545);
			setMessageList(0x004AF668);
		}
		break;
	case 0x482A:
		setPalStatus1(0);
		_sprite1->setVisible(true);
		_klayman->setClipRect(_sprite1->getDrawRect().x, 0, _sprite1->getDrawRect().x2(), _sprite3->getDrawRect().y2());
		break;
	case 0x482B:
		setPalStatus0(false);
		_sprite1->setVisible(false);
		_klayman->setClipRect(_sprite4->getDrawRect().x, 0, 640, _sprite3->getDrawRect().y2());
		break;
	}
	return messageResult;
}

void Scene2812::setPalStatus0(int fadeTime) {
	if (_palStatus != 0) {
		_palStatus = 0;
		setPalStatus(fadeTime);
	}
}

void Scene2812::setPalStatus1(int fadeTime) {
	if (_palStatus != 1) {
		_palStatus = 1;
		setPalStatus(fadeTime);
	}
}

void Scene2812::setPalStatus2(int fadeTime) {
	if (_palStatus != 2) {
		_palStatus = 2;
		setPalStatus(fadeTime);
	}
}

void Scene2812::setPalStatus(int fadeTime) {
	if (_palStatus == 0)
		_palette->addBasePalette(0x05D30F11, 0, 64, 0);
	else if (_palStatus == 1)
		_palette->addBasePalette(0x92CA2C9B, 0, 64, 0);
	else if (_palStatus == 2)
		_palette->addBasePalette(0x381F92C5, 0, 64, 0);
	if (fadeTime > 0) {
		_palette->startFadeToPalette(0);
	} else {
		_palette->startFadeToPalette(12);
	}
}

Scene2822::Scene2822(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _countdown(0), _scrollIndex(0),
	_soundResource1(vm), _soundResource2(vm), _soundResource3(vm) {

	SetMessageHandler(&Scene2822::handleMessage);
	SetUpdateHandler(&Scene2822::update);
	_background = new Background(_vm, 0xD542022E, 0, 0);
	addBackground(_background);
	_background->getSurface()->getDrawRect().y = -10;
	setPalette(0xD542022E);
	insertMouse435(0x0028D089, 20, 620);
	_ssButton = insertStaticSprite(0x1A4D4120, 1100);
	_ssButton->setVisible(false);
	_soundResource3.load(0x19044E72);
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
				_soundResource1.play(0x1384CB60);
				_countdownStatus = 2;
				_countdown = 12;
			} else if (_countdownStatus == 2 && getGlobalVar(0x00188211)) {
				leaveScene(0);
			}
		} else if (_countdownStatus == 2 && getGlobalVar(0x00188211)) {
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
			_soundResource2.play(0x44061000);
			if (getGlobalVar(0x0018CA22) == 0) {
				setGlobalVar(0x0018CA22, 1);
				setGlobalVar(0x00188211, 1);
				SetMessageHandler(NULL);
				_soundResource3.play();
				_mouseCursor->setVisible(false);
			}
		}
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
