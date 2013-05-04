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

#include "neverhood/modules/module1700.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

static const uint32 kModule1700SoundList[] = {
	0xB288D450,
	0x90804450,
	0x99801500,
	0xB288D455,
	0x93825040,
	0
};

Module1700::Module1700(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	_vm->_soundMan->addMusic(0x04212331, 0x31114225);
	_vm->_soundMan->addSoundList(0x04212331, kModule1700SoundList);
	_vm->_soundMan->setSoundListParams(kModule1700SoundList, true, 50, 600, 5, 150);
	_vm->_soundMan->playTwoSounds(0x04212331, 0x41861371, 0x43A2507F, 0);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 0)
		createScene(0, -1);
	else if (which == 1)
		createScene(4, 1);
	else
		createScene(4, 3);

}

Module1700::~Module1700() {
	_vm->_soundMan->deleteGroup(0x04212331);
}

void Module1700::createScene(int sceneNum, int which) {
	debug("Module1700::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_vm->_soundMan->setSoundListParams(kModule1700SoundList, false, 0, 0, 0, 0);
		createSmackerScene(0x3028A005, true, true, false);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		createNavigationScene(0x004AE8B8, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		createNavigationScene(0x004AE8E8, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_vm->_soundMan->setSoundListParams(kModule1700SoundList, false, 0, 0, 0, 0);
		createSmackerScene(0x01190041, true, true, false);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->setSoundListParams(kModule1700SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->startMusic(0x31114225, 0, 2);
		_childObject = new Scene1705(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module1700::updateScene);
	_childObject->handleUpdate();
}

void Module1700::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			_vm->_soundMan->setSoundListParams(kModule1700SoundList, true, 0, 0, 0, 0);
			createScene(1, 0);
			break;
		case 1:
			if (_moduleResult == 0)
				createScene(2, 0);
			else if (_moduleResult == 1)
				createScene(1, 1);
			break;
		case 2:
			if (_moduleResult == 0)
				createScene(3, -1);
			else if (_moduleResult == 1)
				createScene(1, 1);
			else if (_moduleResult == 2) {
				if (!isSoundPlaying(0)) {
					setSoundVolume(0, 60);
					playSound(0, 0x58B45E58);
				}
				createScene(2, 2);
			}
			break;
		case 3:
			createScene(4, 0);
			break;
		case 4:
			leaveModule(1);
			break;
		}
	}
}
		
// Scene1705

static const uint32 kScene1705FileHashes[] = {
	0x910EA801, 0x920EA801, 0x940EA801,
	0x980EA801, 0x800EA801, 0xB00EA801,
	0xD00EA801, 0x100EA801, 0x900EA800,
	0xD10EA801, 0x110EA801, 0x910EA800
};

SsScene1705WallSymbol::SsScene1705WallSymbol(NeverhoodEngine *vm, uint32 fileHash, int symbolIndex)
	: StaticSprite(vm, fileHash, 100) {
	
	_x = _spriteResource.getPosition().x + symbolIndex * 30;
	_y = _spriteResource.getPosition().y + 160;
	updatePosition();
}

SsScene1705Tape::SsScene1705Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 tapeIndex, int surfacePriority, int16 x, int16 y, uint32 fileHash)
	: StaticSprite(vm, fileHash, surfacePriority, x - 24, y - 4), _parentScene(parentScene), _tapeIndex(tapeIndex) {

	if (!getSubVar(VA_HAS_TAPE, _tapeIndex) && !getSubVar(VA_IS_TAPE_INSERTED, _tapeIndex)) {
		SetMessageHandler(&SsScene1705Tape::handleMessage);
	} else {
		setVisible(false);
		SetMessageHandler(NULL);
	}
	_collisionBoundsOffset = _drawOffset;
	_collisionBoundsOffset.x -= 4;
	_collisionBoundsOffset.y -= 8;
	_collisionBoundsOffset.width += 8;
	_collisionBoundsOffset.height += 16;
	Sprite::updateBounds();
}

uint32 SsScene1705Tape::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setSubVar(VA_HAS_TAPE, _tapeIndex, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;		
	}
	return messageResult;
}

Scene1705::Scene1705(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _paletteArea(1) {

	Sprite *tempSprite;
	
	setGlobalVar(V_FELL_DOWN_HOLE, 1);
	_vm->gameModule()->initCannonSymbolsPuzzle();

	SetMessageHandler(&Scene1705::handleMessage);
	SetUpdateHandler(&Scene1705::update);

	setHitRects(0x004B69D8);
	setBackground(0x03118226);
	setPalette(0x03118226);
	_palette->addBasePalette(0x91D3A391, 0, 64, 0);
	_palette->copyBasePalette(0, 256, 0);
	addEntity(_palette);
	insertScreenMouse(0x18222039);

	insertSprite<SsScene1705WallSymbol>(kScene1705FileHashes[getSubVar(VA_GOOD_CANNON_SYMBOLS_2, 0)], 0);
	insertSprite<SsScene1705WallSymbol>(kScene1705FileHashes[getSubVar(VA_GOOD_CANNON_SYMBOLS_2, 1)], 1);
	insertSprite<SsScene1705WallSymbol>(kScene1705FileHashes[getSubVar(VA_GOOD_CANNON_SYMBOLS_2, 2)], 2);
	_sprite = insertStaticSprite(0x31313A22, 1100);
	_ssTape = insertSprite<SsScene1705Tape>(this, 15, 1100, 238, 439, 0x02363852);
	addCollisionSprite(_ssTape);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1705>(231, 434);
		setMessageList(0x004B69E8);
		sendMessage(this, 0x2000, 0);
		_klaymen->setClipRect(0, 0, _sprite->getDrawRect().x2(), 480);
	} else if (which == 1) {
		// Klaymen teleporting in
		insertKlaymen<KmScene1705>(431, 434);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B6A08, false);
		sendMessage(this, 0x2000, 1);
		_klaymen->setClipRect(0, 0, _sprite->getDrawRect().x2(), 480);
	} else if (which == 2) {
		// Klaymen teleporting out
		insertKlaymen<KmScene1705>(431, 434);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B6AA0, false);
		sendMessage(this, 0x2000, 1);
		_klaymen->setClipRect(0, 0, _sprite->getDrawRect().x2(), 480);
	} else if (which == 3) {
		// Klaymen returning from teleporter console
		insertKlaymen<KmScene1705>(431, 434);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B6A18, false);
		sendMessage(this, 0x2000, 1);
		_klaymen->setClipRect(0, 0, _sprite->getDrawRect().x2(), 480);
	} else {
		// Klaymen falling through the hole
		insertKlaymen<KmScene1705>(231, 74);
		sendMessage(_klaymen, 0x2000, 0);
		setMessageList(0x004B69F0);
		sendMessage(this, 0x2000, 0);
		tempSprite = insertStaticSprite(0x30303822, 1100);
		_klaymen->setClipRect(0, tempSprite->getDrawRect().y, _sprite->getDrawRect().x2(), 480);
	}

}

void Scene1705::update() {
	Scene::update();
	if (_klaymen->getX() < 224 && _paletteArea != 0) {
		_palette->addBasePalette(0xF2210C15, 0, 64, 0);
		_palette->startFadeToPalette(12);
		_paletteArea = 0;
	} else if (_klaymen->getX() >= 224 && _paletteArea == 0) {
		_palette->addBasePalette(0x91D3A391, 0, 64, 0);
		_palette->startFadeToPalette(12);
		_paletteArea = 1;
	}
}

uint32 Scene1705::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger()) {
			setRectList(0x004B6B40);
			_klaymen->setKlaymenIdleTable3();
		} else {
			setRectList(0x004B6B30);
			_klaymen->setKlaymenIdleTable1();
		}
		break;
	case 0x4826:
		if (sender == _ssTape && _klaymen->getX() <= 318) {
			sendEntityMessage(_klaymen, 0x1014, sender);
			setMessageList(0x004B6AC0);
		}
		break;		
	}
	return 0;
}

} // End of namespace Neverhood
