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
		setGlobalVar(0x1860C990,1);//DEBUG
		if (getGlobalVar(0x1860C990))
			_childObject = new Scene2803b(_vm, this, which);
		else {
			// TODO _childObject = new Scene2803(_vm, this, which);
		}
		break;
	case 4:
		// TODO Music18hList_stop(0xD2FA4D14, 0, 2);
		_childObject = new Scene2805(_vm, this, which);
		break;
	case 25:
		// TODO Music18hList_play(0xD2FA4D14, 0, 2, 1);
		if (getGlobalVar(0x190A1D18))
			_childObject = new Class152(_vm, this, 0x01600204, 0x0020001E);
		else
			_childObject = new Class152(_vm, this, 0x08611204, 0x1120008E);
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
		case 4:
			if (_moduleResult == 1) {
				leaveModule(1);
			} else {
				createScene(11, 1);
			}
			break;
		case 25:
			createScene(2, 5);
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

	// TODO insertSprite<Class488>(this, 0xAFAD591A, 0x276E321D, 578, 200);

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
		setMessageList(0x004B60E0);
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
		setMessageList(0x004AE1D0);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE288);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		insertKlayman<KmScene2805>(493, 338);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004AE1E0);
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

} // End of namespace Neverhood
