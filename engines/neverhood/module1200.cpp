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

#include "neverhood/module1200.h"
#include "neverhood/smackerscene.h"

namespace Neverhood {

Module1200::Module1200(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _moduleDone(false) {
	
	// TODO _resourceTable.setResourceList(0x004B3E68);
	SetMessageHandler(&Module1200::handleMessage);
	
	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1201(-1);
			break;
		case 1:
			createScene1202(-1);
			break;
		case 2:
			createScene1203(-1);
			break;
		}
	} else if (which == 1) {
		createScene1201(2);
	} else {
		createScene1201(0);
	}

	// TODO Music18hList_add(0x00478311, 0x62222CAE);
	// TODO Music18hList_play(0x62222CAE, 0, 0, 1);
}

Module1200::~Module1200() {
	// TODO Music18hList_deleteGroup(0x00478311);
}

uint32 Module1200::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Module::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1009:
		_moduleDone = true;
		_moduleDoneStatus = param.asInteger();
		break;
	}
	return messageResult;
}

void Module1200::createScene1201(int which) {
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene1201(_vm, this, which);
	SetUpdateHandler(&Module1200::updateScene1201);
}
			
void Module1200::createScene1202(int which) {
	_vm->gameState().sceneNum = 1;
	//_childObject = new Scene1202(_vm, this, which);
	//SetUpdateHandler(&Module1200::updateScene1202);
}

void Module1200::createScene1203(int which) {
	SmackerScene *smackerScene;
	_vm->gameState().sceneNum = 2;
	// TODO Music18hList_stop(0x62222CAE, 0, 0);
	smackerScene = new SmackerScene(_vm, this, true, true, false);
	smackerScene->setFileHash(0x31890001);
	smackerScene->nextVideo();
	_childObject = smackerScene;
	setGlobalVar(0x2A02C07B, 1);
	SetUpdateHandler(&Module1200::updateScene1203);
}

void Module1200::updateScene1201() {
	_childObject->handleUpdate();
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		if (_moduleDoneStatus == 1) {
			createScene1202(0);
			_childObject->handleUpdate();
		} else if (_moduleDoneStatus == 2) {
			_parentModule->sendMessage(0x1009, 0, this);
		} else if (getGlobalVar(0x0A18CA33) && !getGlobalVar(0x2A02C07B)) {
			createScene1203(-1);
		} else {
			_parentModule->sendMessage(0x1009, 1, this);
		}
	}
}

void Module1200::updateScene1202() {
	_childObject->handleUpdate();
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		createScene1201(1);
		_childObject->handleUpdate();
	}
}
			
void Module1200::updateScene1203() {
	_childObject->handleUpdate();
	if (_moduleDone) {
		_moduleDone = false;
		delete _childObject;
		_childObject = NULL;
		createScene1201(3);
		_childObject->handleUpdate();
		// TODO Music18hList_play(0x62222CAE, 0, 0, 1);
	}
}

// Scene1201

static const uint32 kScene1201InitArray[] = {
	1, 0, 2, 4, 5, 3, 6, 7, 8, 10, 9, 11, 13, 14, 12, 16, 17, 15
};

static const NPoint kScene1201PointArray[] = {
	{218, 193},
	{410, 225},
	{368, 277},
	{194, 227},
	{366, 174},
	{458, 224},
	{242, 228},
	{512, 228},
	{458, 277},
	{217, 233},
	{458, 173},
	{410, 276},
	{203, 280},
	{371, 226},
	{508, 279},
	{230, 273},
	{410, 171},
	{493, 174}
};

static const uint32 kScene1201SsScene1201TntFileHashList1[] = {
	0x2098212D,   
	0x1600437E,
	0x1600437E,
	0x00A840E3,
	0x1A1830F6,
	0x1A1830F6,
	0x00212062,
	0x384010B6,
	0x384010B6,
	0x07A01080,
	0xD80C2837,
	0xD80C2837,
	0x03A22092,
	0xD8802CB6,
	0xD8802CB6,
	0x03A93831,
	0xDA460476,
	0xDA460476
};

static const uint32 kScene1201SsScene1201TntFileHashList2[] = {
	0x3040C676,  
	0x10914448,
	0x10914448,
	0x3448A066,
	0x1288C049,
	0x1288C049,
	0x78C0E026,
	0x3098D05A,
	0x3098D05A,
	0x304890E6,
	0x1284E048,
	0x1284E048,
	0xB140A1E6,
	0x5088A068,
	0x5088A068,
	0x74C4C866,
	0x3192C059,
	0x3192C059
};
			
SsScene1201Tnt::SsScene1201Tnt(NeverhoodEngine *vm, uint32 elemIndex, uint32 pointIndex, int16 clipY2)
	: StaticSprite(vm, 900), _field7A(-1) {

	int16 x = kScene1201PointArray[pointIndex].x;
	int16 y = kScene1201PointArray[pointIndex].y;
	if (x < 300) {
		_spriteResource.load2(kScene1201SsScene1201TntFileHashList1[elemIndex]);
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
		_drawRect.x = 0;
		_drawRect.y = 0;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
	} else {
		_spriteResource.load2(kScene1201SsScene1201TntFileHashList2[elemIndex]);
		_x = x;
		_y = y;
		_drawRect.x = -(_spriteResource.getDimensions().width / 2);
		_drawRect.y = -_spriteResource.getDimensions().height;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
	
	}
	createSurface(50, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_surface->getClipRect().x1 = 0;
	_surface->getClipRect().y1 = 0;
	_surface->getClipRect().x2 = 640;
	_surface->getClipRect().y2 = clipY2;
	_needRefresh = true;
	StaticSprite::update();
}
	
AsScene1201Tape::AsScene1201Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 nameHash, int surfacePriority, int16 x, int16 y, uint32 fileHash)
	: AnimatedSprite(vm, fileHash, surfacePriority, x, y), _parentScene(parentScene), _nameHash(nameHash) {
	
	if (!getSubVar(0x02038314, _nameHash) && !getSubVar(0x02720344, _nameHash)) {
		SetMessageHandler(&AsScene1201Tape::handleMessage);
	} else {
		_surface->setVisible(false);
		SetMessageHandler(NULL);
	}
}

uint32 AsScene1201Tape::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		_parentScene->sendMessage(0x4826, 0, this);
		messageResult = 1;
		break;
	case 0x4806:
		setSubVar(0x02038314, _nameHash, 1);
		_surface->setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

Class466::Class466(NeverhoodEngine *vm, bool flag)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class466::handleMessage);
	createSurface(10, 34, 149);
	_x = 202;
	_y = -32;
	if (flag) {
		sub40D380();
	} else {
		sub40D340();
	}
}

uint32 Class466::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02060018) {
			_soundResource.play(0x47900E06);
		}
		break;
	case 0x2006:
		sub40D360();
		break;
	}
	return messageResult;
}

void Class466::sub40D340() {
	setFileHash(0x928F0C10, 0, -1);
	_newHashListIndex = 0;
}

void Class466::sub40D360() {
	setFileHash(0x928F0C10, 1, -1);
	_newHashListIndex = -2;
}

void Class466::sub40D380() {
	setFileHash(0x928F0C10, 15, -1);
	_newHashListIndex = -2;
}

Class468::Class468(NeverhoodEngine *vm, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman), _countdown(0) {

	createSurface1(100, 0xD088AC30);
	_x = 320;
	_y = 240;
	SetUpdateHandler(&Class468::update);
	SetMessageHandler(&Class468::handleMessage);
	_newHashListIndex = -2;
	if (flag) {
		setFileHash(0xD088AC30, -1, -1);
		_newHashListIndex = -2;
		_countdown = 25;
	} else {
		setFileHash1();
		_surface->setVisible(false);
	}
}

void Class468::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sub40D830();
	}
	AnimatedSprite::update();
}

uint32 Class468::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	case 0x4829:
		sub40D7E0();
		break;
	}
	return messageResult;
}

void Class468::sub40D7E0() {
	setFileHash(0xD088AC30, 0, -1);
	_newHashListIndex = -2;
	_surface->setVisible(true);
	_soundResource.play(calcHash("fxDoorOpen20"));
}

void Class468::sub40D830() {
	setFileHash(0xD088AC30, -1, -1);
	_playBackwards = true;
	_surface->setVisible(true);
	_soundResource.play(calcHash("fxDoorClose20"));
	SetAnimationCallback3(&Class468::sub40D880);
}

void Class468::sub40D880() {
	setFileHash1();
	_surface->setVisible(false);
}
		
Class464::Class464(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {
	
	createSurface(1200, 69, 98);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class464::handleMessage);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	_surface->setVisible(false);
}

uint32 Class464::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2006:
		_x = 436;
		_y = 339;
		setFileHash(0xA060C599, 0, -1);
		_surface->setVisible(true);
		break;
	case 0x3002:
		setFileHash1();
		_surface->setVisible(false);
		removeCallbacks();
		break;
	}
	return messageResult;
}

Class463::Class463(NeverhoodEngine *vm, Scene *parentScene, Sprite *class466, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _class466(class466),
	_flag(false) {
	
	//TODO_field_F0 = -1;
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class463::handleMessage);
	createSurface(990, 106, 181);
	_x = 201;
	if (flag) {
		_y = 297;
		sub40CD60();
	} else {
		_y = 334;
		sub40CD30();
	}
}

Class463::~Class463() {
	// TODO Sound1ChList_sub_407AF0(0x01D00560);
}	 

uint32 Class463::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x092870C0) {
			_class466->sendMessage(0x2006, 0, this);
		} else if (param.asInteger() == 0x11CA0144) {
			_soundResource.play(0x51800A04);
		}
		break;
	case 0x1011:
		_parentScene->sendMessage(0x2002, 0, this);
		messageResult = 1;
	case 0x480B:
		if (!_flag) {
			_sprite = (Sprite*)sender;
			sub40CD90();
		}
		break;
	}
	return messageResult;

}

uint32 Class463::handleMessage40CCD0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Class463::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void Class463::spriteUpdate40CD10() {
	_x = _sprite->getX() + 100;
}

void Class463::sub40CD30() {
	setFileHash(0x654913D0, 0, -1);
	SetMessageHandler(&Class463::handleMessage);
	SetSpriteCallback(NULL);
}

void Class463::sub40CD60() {
	setFileHash(0x356803D0, 0, -1);
	SetMessageHandler(&Class463::handleMessage40CCD0);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	SetAnimationCallback3(&Class463::sub40CD30);
}

void Class463::sub40CD90() {
	// TODO Sound1ChList_addSoundResource(0x01D00560, 0x4B044624, true);
	// TODO Sound1ChList_playLooping(0x4B044624);
	_flag = true;
	setFileHash(0x85084190, 0, -1);
	SetMessageHandler(&Class463::handleMessage);
	SetSpriteCallback(&Class463::spriteUpdate40CD10);
	_newHashListIndex = -2;
}

Class465::Class465(NeverhoodEngine *vm, Sprite *class463)
	: AnimatedSprite(vm, 1200), _class463(class463) {

	createSurface1(995, 0x828C0411);
	SetUpdateHandler(&Class465::update);
	SetMessageHandler(&Sprite::handleMessage);
	SetSpriteCallback(&Class465::spriteUpdate40D150);
	setFileHash(0x828C0411, 0, -1);
	_surface->setVisible(false);
}

Class465::~Class465() {
	// TODO Sound1ChList_sub_407AF0(0x041080A4);
}

void Class465::update() {
	AnimatedSprite::update();
	if (getGlobalVar(0x20A0C516)) {
		_surface->setVisible(true);
		SetUpdateHandler(&AnimatedSprite::update);
		// TODO Sound1ChList_addSoundResource(0x041080A4, 0x460A1050, true);
		// TODO Sound1ChList_playLooping(0x460A1050);
	}
}

void Class465::spriteUpdate40D150() {
	_x = _class463->getX() - 18;
	_y = _class463->getY() - 158;
}

AsScene1201LeftDoor::AsScene1201LeftDoor(NeverhoodEngine *vm, Sprite *klayman)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman) {

	_x = 320;
	_y = 240;
	createSurface(800, 55, 199);
	if (_klayman->getX() < 100) {
		setFileHash(0x508A111B, 0, -1);
		_newHashListIndex = -2;
		_soundResource.play(calcHash("fxDoorOpen03"));
	} else {
		setFileHash(0x508A111B, -1, -1);
		_newHashListIndex = -2;
	}
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201LeftDoor::handleMessage);
}

uint32 AsScene1201LeftDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4809:
		sub40D590();
		break;
	}
	return messageResult;
}

void AsScene1201LeftDoor::sub40D590() {
	setFileHash(0x508A111B, -1, -1);
	_playBackwards = true;
	_newHashListIndex = 0;
}

Scene1201::Scene1201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag(false) {

	int16 topY1, topY2, topY3, topY4;
	int16 x1, x2;
	Sprite *tempSprite, *class464;

	// TODO _resourceTable2.setResourceList(ex_sub_41C730(), true);
	if (getGlobalVar(0x0A18CA33)) {
		// TODO _resourceTable1.setResourceList(0x004AEA10, true);
	} else if (getGlobalVar(0x0A310817)) {
		// TODO _resourceTable1.setResourceList(0x004AEA70, true);
	} else {
		// TODO _resourceTable1.setResourceList(0x004AEB18, true);
	}

	SetUpdateHandler(&Scene1201::update);
	SetMessageHandler(&Scene1201::handleMessage);

	_vm->_collisionMan->setHitRects(0x004AEBD0);
	
	_surfaceFlag = true;

	if (!getSubVar(0x40050052, 0xE8058B52)) {
		setSubVar(0x40050052, 0xE8058B52, 1);
		for (uint32 index = 0; index < 18; index++) {
			setSubVar(0x10055D14, index, kScene1201InitArray[index]);
		}
	}

	_mouseCursor = addSprite(new Mouse433(_vm, 0x9A2C0409, NULL));
	
	_asTape = addSprite(new AsScene1201Tape(_vm, this, 3, 1100, 243, 340, 0x9148A011));
	_vm->_collisionMan->addSprite(_asTape);
	
	tempSprite = addSprite(new StaticSprite(_vm, 0x03C82530, 100));
	topY1 = tempSprite->getY() + tempSprite->getSurface()->getDrawRect().height; 

	tempSprite = addSprite(new StaticSprite(_vm, 0x88182069, 200));
	topY2 = tempSprite->getY() + tempSprite->getSurface()->getDrawRect().height; 

	tempSprite = addSprite(new StaticSprite(_vm, 0x476014E0, 300));
	topY3 = tempSprite->getY() + tempSprite->getSurface()->getDrawRect().height; 

	tempSprite = addSprite(new StaticSprite(_vm, 0x04063110, 500));
	topY4 = tempSprite->getY() + 1; 

	_class466 = addSprite(new Class466(_vm, getGlobalVar(0x000CF819) && which != 1 ? 1 : 0));
	_class466->getSurface()->getClipRect().x1 = 0;
	_class466->getSurface()->getClipRect().y1 = topY4;
	_class466->getSurface()->getClipRect().x2 = 640;
	_class466->getSurface()->getClipRect().y2 = 480;

	addSprite(new StaticSprite(_vm, 0x400B04B0, 1200));

	tempSprite = addSprite(new StaticSprite(_vm, 0x40295462, 1200));
	x1 = tempSprite->getX();

	tempSprite = addSprite(new StaticSprite(_vm, 0xA29223FA, 1200));
	x2 = tempSprite->getX() + tempSprite->getSurface()->getDrawRect().width;

	class464 = addSprite(new Class464(_vm));

	if (which < 0) {
		_klayman = new KmScene1201(_vm, this, class464, 364, 333);
		setMessageList(0x004AEC08);
	} else if (which == 3) {
		_klayman = new KmScene1201(_vm, this, class464, 400, 329);
		setMessageList(0x004AEC08);
	} else if (which == 2) {
		if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
			_klayman = new KmScene1201(_vm, this, class464, 374, 333);
			setMessageList(0x004AEC08);
		} else {
			_klayman = new KmScene1201(_vm, this, class464, 640, 329);
			setMessageList(0x004AEC20);
		}
	} else if (which == 1) {
		if (getGlobalVar(0xC0418A02)) {
			_klayman = new KmScene1201(_vm, this, class464, 364, 333);
			_klayman->setDoDeltaX(1);
		} else {
			_klayman = new KmScene1201(_vm, this, class464, 246, 333);
		}
		setMessageList(0x004AEC30);
	} else {
		_klayman = new KmScene1201(_vm, this, class464, 0, 336);
		setMessageList(0x004AEC10);
	}

	_klayman->getSurface()->getClipRect().x1 = x1;
	_klayman->getSurface()->getClipRect().y1 = 0;
	_klayman->getSurface()->getClipRect().x2 = x2;
	_klayman->getSurface()->getClipRect().y2 = 480;
	_klayman->setRepl(64, 0);
	
	addSprite(_klayman);

	if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
		_background = addBackground(new DirtyBackground(_vm, 0x4019A2C4, 0, 0));
		_palette = new Palette(_vm, 0x4019A2C4);
		_palette->usePalette();
		_class468 = NULL;
	} else {
		_background = addBackground(new DirtyBackground(_vm, 0x40206EC5, 0, 0));
		_palette = new Palette(_vm, 0x40206EC5);
		_palette->usePalette();
		_class468 = addSprite(new Class468(_vm, _klayman, which - 2 != 1)); // CHECKME???
	}

	if (getGlobalVar(0x000CF819)) {
		addSprite(new StaticSprite(_vm, 0x10002ED8, 500));
		if (!getGlobalVar(0x0A18CA33)) {
			Class463 *class463;
			class463 = new Class463(_vm, this, _class466, which - 1 != 1);
			class463->getSurface()->getClipRect().x1 = x1;
			class463->getSurface()->getClipRect().y1 = 0;
			class463->getSurface()->getClipRect().x2 = x2;
			class463->getSurface()->getClipRect().y2 = 480;
			_vm->_collisionMan->addSprite(_class463);
			_class463 = addSprite(class463);
			tempSprite = addSprite(new Class465(_vm, _class463));
			tempSprite->getSurface()->getClipRect().x1 = x1;
			tempSprite->getSurface()->getClipRect().y1 = 0;
			tempSprite->getSurface()->getClipRect().x2 = x2;
			tempSprite->getSurface()->getClipRect().y2 = 480;
			class463->setRepl(64, 0);
		}
		
		uint32 tntIndex = 1; 
		while (tntIndex < 18) {
			uint32 elemIndex = getSubVar(0x10055D14, tntIndex);
			int16 clipY2;
			if (kScene1201PointArray[elemIndex].y < 175)
				clipY2 = topY1;
			else if (kScene1201PointArray[elemIndex].y < 230)
				clipY2 = topY2;
			else
				clipY2 = topY3;
			addSprite(new SsScene1201Tnt(_vm, tntIndex, getSubVar(0x10055D14, tntIndex), clipY2));
			elemIndex = getSubVar(0x10055D14, tntIndex + 1);
			if (kScene1201PointArray[elemIndex].y < 175)
				clipY2 = topY1;
			else if (kScene1201PointArray[elemIndex].y < 230)
				clipY2 = topY2;
			else
				clipY2 = topY3;
			addSprite(new SsScene1201Tnt(_vm, tntIndex + 1, getSubVar(0x10055D14, tntIndex + 1), clipY2));
			tntIndex += 3;
		}

		if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
			setRectList(0x004AEE58);
		} else {
			setRectList(0x004AEDC8);
		} 
		
	} else {
	
		addSprite(new StaticSprite(_vm, 0x8E8A1981, 900));

		uint32 tntIndex = 0;
		while (tntIndex < 18) {
			uint32 elemIndex = getSubVar(0x10055D14, tntIndex);
			int16 clipY2;
			if (kScene1201PointArray[elemIndex].x < 300) {
				clipY2 = 480;
			} else { 
				if (kScene1201PointArray[elemIndex].y < 175)
					clipY2 = topY1;
				else if (kScene1201PointArray[elemIndex].y < 230)
					clipY2 = topY2;
				else
					clipY2 = topY3;
			}
			addSprite(new SsScene1201Tnt(_vm, tntIndex, getSubVar(0x10055D14, tntIndex), clipY2));
			tntIndex++;
		}

		if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
			setRectList(0x004AEE18);
		} else {
			setRectList(0x004AED88);
		}
		 
	}

	tempSprite = addSprite(new StaticSprite(_vm, 0x63D400BC, 900));

	_asLeftDoor = addSprite(new AsScene1201LeftDoor(_vm, _klayman));
	_asLeftDoor->getSurface()->getClipRect().x1 = x1;
	_asLeftDoor->getSurface()->getClipRect().y1 = tempSprite->getSurface()->getDrawRect().y;
	_asLeftDoor->getSurface()->getClipRect().x2 = tempSprite->getSurface()->getDrawRect().x + tempSprite->getSurface()->getDrawRect().width;
	_asLeftDoor->getSurface()->getClipRect().y2 = 480;

	if (getGlobalVar(0x0A310817) && ! getGlobalVar(0x0112090A)) {
		setGlobalVar(0x0112090A, 1);
	}

	_class461 = NULL;

#if 0	
	if (getGlobalVar(0x0112090A) < 3) {
		_class461 = addSprite(new Class461(_vm, this));
		_vm->_collisionMan->addSprite(_class461);
	}

	if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
		_class462 = addSprite(new Class462(_vm, this, _klayman));
		_class462->getSurface()->getClipRect().x1 = x1;
		_class462->getSurface()->getClipRect().y1 = 0;
		_class462->getSurface()->getClipRect().x2 = x2;
		_class462->getSurface()->getClipRect().y2 = 480;
	}
#endif

}

Scene1201::~Scene1201() {
}

void Scene1201::update() {
	Scene::update();
	if (_class461 && getGlobalVar(0x0112090A)) {
		deleteSprite(&_class461);
	}
}

uint32 Scene1201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Scene1201::handleMessage(%04X)", messageNum);
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x07053000) {
			_flag = true;
			_class462->sendMessage(0x2004, 0, this);
		} else if (param.asInteger() == 0x140E5744) {
			_class462->sendMessage(0x2005, 0, this);
		} else if (param.asInteger() == 0x40253C40) {
			_messageListFlag = false;
			_class462->sendMessage(0x2006, 0, this);
		} else if (param.asInteger() == 0x090EB048) {
			if (_klayman->getX() < 572) {
				setMessageList2(0x004AEC90);
			} else {
				setMessageList2(0x004AEC20);
			}
		}
		break;
	case 0x2001:
		if (!getGlobalVar(0x0112090A)) {
			setMessageList2(0x004AECB0);
		} else {
			_klayman->sendEntityMessage(0x1014, _class461, this);
			setMessageList2(0x004AECC0);
		}
		break;
	case 0x2002:		
		if (getGlobalVar(0x20A0C516)) {
			_klayman->sendEntityMessage(0x1014, _class463, this);
			setMessageList2(0x004AECF0);
		} else if (getGlobalVar(0x0112090A) == 3) {
			_klayman->sendEntityMessage(0x1014, _class463, this);
			if (_klayman->getX() > _class463->getX()) {
				setMessageList(0x004AECD0);
			} else {
				setMessageList(0x004AECE0);
			}
		}
		break;
	case 0x4814:
		messageList402220();
		break;
	case 0x4826:
		if (sender == _asTape) {
			_klayman->sendEntityMessage(0x1014, _asTape, this);
			setMessageList(0x004AED38);
		}
		break;
	case 0x4829:
		_class468->sendMessage(0x4829, 0, this);
		break;		
	}
	return messageResult;
}

} // End of namespace Neverhood
