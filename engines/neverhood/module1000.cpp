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

#include "neverhood/module1000.h"

namespace Neverhood {

Module1000::Module1000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module1000(%d)", which);

	_musicFileHash = _vm->getGlobalVar(0xD0A14D10) ? 0x81106480 : 0x00103144;		

	// TODO Music18hList_add(0x03294419, 0x061880C6);
	// TODO Music18hList_add(0x03294419, _musicFileHash);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1001(-1);
			break;
		case 1:
			createScene1002(-1);
			break;
		case 2:
			createScene1003(-1);
			break;
		case 3:
			createScene1004(-1);
			break;
		case 4:
			createScene1005(-1);
			break;
		}
	} else if (which == 0) {
		createScene1001(0);
	} else if (which == 1) {
		createScene1002(1);
	}

}

Module1000::~Module1000() {
	// TODO Music18hList_deleteGroup(0x03294419);
}

void Module1000::createScene1001(int which) {
	debug("createScene1501");
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene1001(_vm, this, which);
	// TODO ResourceTable_multiLoad(&_resourceTable1, &_resourceTable2, &_resourceTable3);
	// TODO Music18hList_play(0x061880C6, 0, 0, 1);
	SetUpdateHandler(&Module1000::updateScene1001);
}
			
void Module1000::createScene1002(int which) {
}

void Module1000::createScene1003(int which) {
}

void Module1000::createScene1004(int which) {
}

void Module1000::createScene1005(int which) {
}

void Module1000::updateScene1001() {
}

void Module1000::updateScene1002() {
}
			
void Module1000::updateScene1003() {
}
			
void Module1000::updateScene1004() {
}
			
void Module1000::updateScene1005() {
}
			
// Scene1001			
				
Scene1001::Scene1001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _fieldE4(-1), _fieldE6(-1) {

	// TODO: Implement Sprite classes

	Sprite *staticSprite1;

	SetMessageHandler(&Scene1001::handleMessage);
	
	_vm->_collisionMan->setHitRects(0x004B4858);
	_surfaceFlag = false;
	_background = addBackground(new DirtyBackground(_vm, 0x4086520E, 0, 0));
	_palette = new Palette(_vm, 0x4086520E);
	_palette->usePalette();
	
	// TODO Mouse

#if 0
	// TODO: Player sprites...	
	if (which < 0) {
		setRectList(0x004B49F0);
		_playerSprite = new Class572(_vm, this, 200, 433, 1000, 1000);
		setMessageList(0x004B4888);
	} else if (which == 1) {
		setRectList(0x004B49F0);
		_playerSprite = new Class572(_vm, this, 640, 433, 1000, 1000);
		setMessageList(0x004B4898);
	} else if (which == 2) {
		setRectList(0x004B49F0);
		if (_vm->getGlobalVar(0xC0418A02)) {
			_playerSprite = new Class572(_vm, this, 390, 433, 1000, 1000);
			_playerSprite->setDoDeltaX(1);
		} else {
			_playerSprite = new Class572(_vm, this, 300, 433, 1000, 1000);
		}
		setMessageList(0x004B4970);
	} else {
		setRectList(0x004B4A00);
		_playerSprite = new Class572(_vm, this, 200, 433, 1000, 1000);
		setMessageList(0x004B4890);
	}
	addSprite(_playerSprite);
#endif

	staticSprite1 = addSprite(new StaticSprite(_vm, 0x2080A3A8, 1300));

#if 0
	// TODO: This sucks somehow, find a better way
	_playerSprite->getSurface()->getClipRect().x1 = 0;
	_playerSprite->getSurface()->getClipRect().y1 = 0;
	_playerSprite->getSurface()->getClipRect().x2 = staticSprite1->getSurface()->getDrawRect().x + staticSprite1->getSurface()->getDrawRect().width;
	_playerSprite->getSurface()->getClipRect().y2 = 480;
	
	if (_vm->getGlobalVar(0xD217189D) == 0) {
		_class509 = addSprite(new Class509(_vm));
		_class509->getSurface()->getClipRect().x1 = 0;
		_class509->getSurface()->getClipRect().y1 = 0;
		_class509->getSurface()->getClipRect().x2 = staticSprite1->getSurface()->getDrawRect().x + staticSprite1->getSurface()->getDrawRect().width;
		_class509->getSurface()->getClipRect().y2 = 480;
	} else {
		_class509 = NULL;
	}

	_class511 = addSprite(new Class511(_vm, this, 150, 433, 1));
#endif

	addSprite(new StaticSprite(_vm, 0x809861A6, 950));
	addSprite(new StaticSprite(_vm, 0x89C03848, 1100));

#if 0
	_class608 = addSprite(new Class608(_vm, 0x15288120, 100, 0));
#endif

	if (_vm->getGlobalVar(0x03C698DA) == 0) {
		staticSprite1 = addSprite(new StaticSprite(_vm, 0x8C066150, 200));
#if 0
		_class510 = addSprite(new Class510(_vm));
		_class510->getSurface()->getClipRect().x1 = staticSprite1->getSurface()->getDrawRect().x;
		_class510->getSurface()->getClipRect().y1 = staticSprite1->getSurface()->getDrawRect().y;
		_class510->getSurface()->getClipRect().x2 = staticSprite1->getSurface()->getDrawRect().x + staticSprite1->getSurface()->getDrawRect().width;
		_class510->getSurface()->getClipRect().y2 = staticSprite1->getSurface()->getDrawRect().y + staticSprite1->getSurface()->getDrawRect().height;
#endif
	} else {
		_class510= NULL;
	}

#if 0
	_class508 = addSprite(new Class508(_vm, _class509));
#endif

}

void Scene1001::update() {
}

uint32 Scene1001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	return 0;
}

} // End of namespace Neverhood
