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
	_childObject = new Scene1202(_vm, this, which);
	SetUpdateHandler(&Module1200::updateScene1202);
}

void Module1200::createScene1203(int which) {
	_vm->gameState().sceneNum = 2;
	// TODO Music18hList_stop(0x62222CAE, 0, 0);
	createSmackerScene(0x31890001, true, true, false);
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
			sendMessage(_parentModule, 0x1009, 0);
		} else if (getGlobalVar(0x0A18CA33) && !getGlobalVar(0x2A02C07B)) {
			createScene1203(-1);
		} else {
			sendMessage(_parentModule, 0x1009, 1);
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

static const uint32 kScene1201TntFileHashList1[] = {
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

static const uint32 kScene1201TntFileHashList2[] = {
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
		_spriteResource.load2(kScene1201TntFileHashList1[elemIndex]);
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
		_drawRect.x = 0;
		_drawRect.y = 0;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
	} else {
		_spriteResource.load2(kScene1201TntFileHashList2[elemIndex]);
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
		sendMessage(_parentScene, 0x4826, 0);
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

AsScene1201RightDoor::AsScene1201RightDoor(NeverhoodEngine *vm, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _klayman(klayman), _countdown(0) {

	createSurface1(0xD088AC30, 100);
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AsScene1201RightDoor::update);
	SetMessageHandler(&AsScene1201RightDoor::handleMessage);
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

void AsScene1201RightDoor::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sub40D830();
	}
	AnimatedSprite::update();
}

uint32 AsScene1201RightDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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

void AsScene1201RightDoor::sub40D7E0() {
	setFileHash(0xD088AC30, 0, -1);
	_newHashListIndex = -2;
	_surface->setVisible(true);
	_soundResource.play(calcHash("fxDoorOpen20"));
}

void AsScene1201RightDoor::sub40D830() {
	setFileHash(0xD088AC30, -1, -1);
	_playBackwards = true;
	_surface->setVisible(true);
	_soundResource.play(calcHash("fxDoorClose20"));
	SetAnimationCallback3(&AsScene1201RightDoor::sub40D880);
}

void AsScene1201RightDoor::sub40D880() {
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

AsScene1201TntMan::AsScene1201TntMan(NeverhoodEngine *vm, Scene *parentScene, Sprite *class466, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _class466(class466),
	_flag(false) {

	flag = false;
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201TntMan::handleMessage);
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

AsScene1201TntMan::~AsScene1201TntMan() {
	// TODO Sound1ChList_sub_407AF0(0x01D00560);
}	 

uint32 AsScene1201TntMan::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x092870C0) {
			sendMessage(_class466, 0x2006, 0);
		} else if (param.asInteger() == 0x11CA0144) {
			_soundResource.play(0x51800A04);
		}
		break;
	case 0x1011:
		sendMessage(_parentScene, 0x2002, 0);
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

uint32 AsScene1201TntMan::handleMessage40CCD0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = AsScene1201TntMan::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene1201TntMan::spriteUpdate40CD10() {
	_x = _sprite->getX() + 100;
}

void AsScene1201TntMan::sub40CD30() {
	setFileHash(0x654913D0, 0, -1);
	SetMessageHandler(&AsScene1201TntMan::handleMessage);
	SetSpriteCallback(NULL);
}

void AsScene1201TntMan::sub40CD60() {
	setFileHash(0x356803D0, 0, -1);
	SetMessageHandler(&AsScene1201TntMan::handleMessage40CCD0);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	SetAnimationCallback3(&AsScene1201TntMan::sub40CD30);
}

void AsScene1201TntMan::sub40CD90() {
	// TODO Sound1ChList_addSoundResource(0x01D00560, 0x4B044624, true);
	// TODO Sound1ChList_playLooping(0x4B044624);
	_flag = true;
	setFileHash(0x85084190, 0, -1);
	SetMessageHandler(&AsScene1201TntMan::handleMessage);
	SetSpriteCallback(&AsScene1201TntMan::spriteUpdate40CD10);
	_newHashListIndex = -2;
}

Class465::Class465(NeverhoodEngine *vm, Sprite *asTntMan)
	: AnimatedSprite(vm, 1200), _asTntMan(asTntMan) {

	createSurface1(0x828C0411, 995);
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
	_x = _asTntMan->getX() - 18;
	_y = _asTntMan->getY() - 158;
}

AsScene1201Match::AsScene1201Match(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene) {
	
	createSurface(1100, 57, 60);
	SetUpdateHandler(&AsScene1201Match::update);
	SetMessageHandler(&AsScene1201Match::handleMessage40C2D0);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	
	switch (getGlobalVar(0x0112090A)) {
	case 0:
		_x = 521;
		_y = 112;
		_status = 0;
		sub40C4C0();
		break;
	case 1:
		_x = 521;
		_y = 112;
		_status = 2;
		sub40C470();
		_soundResource.load(0xD00230CD);
		break;
	case 2:
		setDoDeltaX(1);
		_x = 403;
		_y = 337;
		_status = 0;
		sub40C4F0();
		break;
	}
}

void AsScene1201Match::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		removeCallbacks();
	}
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene1201Match::handleMessage40C2D0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x86668011) {
			_soundResource.play();
		}
		break;
	}
	return messageResult;
}

uint32 AsScene1201Match::handleMessage40C320(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage40C2D0(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

uint32 AsScene1201Match::handleMessage40C360(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage40C2D0(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x2001, 0);
		messageResult = 1;
		break;
	case 0x4806:
		_surface->setVisible(false);
		setGlobalVar(0x0112090A, 3);
		break;
	}
	return messageResult;
}

void AsScene1201Match::sub40C3E0() {
	setFileHash(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::handleMessage40C320);
	if (_status == 0) {
		SetAnimationCallback3(&AsScene1201Match::sub40C420);
	} else {
		SetAnimationCallback3(&AsScene1201Match::sub40C470);
	}
}

void AsScene1201Match::sub40C420() {
	setGlobalVar(0x0112090A, 2);
	_x -= 199;
	_y += 119;
	setFileHash(0x018D0240, 0, -1);
	SetMessageHandler(&AsScene1201Match::handleMessage40C320);
	SetAnimationCallback3(&AsScene1201Match::sub40C4F0);
}

void AsScene1201Match::sub40C470() {
	setFileHash(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::handleMessage40C2D0);
	_newHashListIndex = 0;
	if (_status != 0) {
		_countdown = 36;
		_status--;
		SetAnimationCallback3(&AsScene1201Match::sub40C3E0);
	}
}

void AsScene1201Match::sub40C4C0() {
	setFileHash(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::handleMessage40C360);
	_newHashListIndex = 0;
}

void AsScene1201Match::sub40C4F0() {
	setDoDeltaX(1);
	_x = 403;
	_y = 337;
	setFileHash(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::handleMessage40C360);
	_newHashListIndex = 0;
}

AsScene1201Creature::AsScene1201Creature(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman)
	: AnimatedSprite(vm, 900), _soundResource(vm), _parentScene(parentScene), _klayman(klayman),
	_flag(false) {
	
	createSurface(1100, 203, 199);
	SetUpdateHandler(&AsScene1201Creature::update);
	SetMessageHandler(&AsScene1201Creature::handleMessage40C710);
	_x = 540;
	_y = 320;
	sub40C8E0();
	_countdown3 = 2;
}

void AsScene1201Creature::update() {
	bool oldFlag = _flag;
	_flag = _x >= 385;
	if (_flag != oldFlag)
		sub40C8E0();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		removeCallbacks();
	}
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene1201Creature::handleMessage40C710(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02060018) {
			_soundResource.play(0xCD298116);
		}
		break;
	case 0x2004:
		setCallback2(AnimationCallback(&AsScene1201Creature::sub40C960));
		break;
	case 0x2006:
		setCallback2(AnimationCallback(&AsScene1201Creature::sub40C9B0));
		break;
	}
	return messageResult;
}

uint32 AsScene1201Creature::handleMessage40C7B0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage40C710(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02421405) {
			if (_countdown2 != 0 && (--_countdown2 == 0)) {
				sub40C990();
			}
		}
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

uint32 AsScene1201Creature::handleMessage40C830(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02060018) {
			_soundResource.play(0xCD298116);
			sendMessage(_parentScene, 0x4814, 0);
			sendMessage(_klayman, 0x4814, 0);
		}
		break;
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene1201Creature::sub40C8E0() {
	_countdown3--;
	if (_countdown3 == 0)
		_countdown3 = 3;
	setFileHash(0x08081513, 0, -1);
	SetMessageHandler(&AsScene1201Creature::handleMessage40C710);
	SetAnimationCallback3(&AsScene1201Creature::sub40C930);
	_countdown1 = 36;
}

void AsScene1201Creature::sub40C930() {
	if (!_flag) {
		setFileHash(0xCA287133, 0, -1);
		SetMessageHandler(&AsScene1201Creature::handleMessage40C7B0);
		SetAnimationCallback3(&AsScene1201Creature::sub40C8E0);
	}
}

void AsScene1201Creature::sub40C960() {
	setFileHash(0x08081513, 0, -1);
	SetMessageHandler(&AsScene1201Creature::handleMessage40C710);
	SetAnimationCallback3(&AsScene1201Creature::sub40C9E0);
	_countdown1 = 48;
}

void AsScene1201Creature::sub40C990() {
	setFileHash2(0x0B6E13FB, 0x01084280, 0);
}

void AsScene1201Creature::sub40C9B0() {
	setFileHash(0xCA287133, 0, -1);
	SetMessageHandler(&AsScene1201Creature::handleMessage40C830);
	SetAnimationCallback3(&AsScene1201Creature::sub40C8E0);
	_countdown1 = 0;
}

void AsScene1201Creature::sub40C9E0() {
	setFileHash(0x5A201453, 0, -1);
	SetMessageHandler(&AsScene1201Creature::handleMessage40C710);
	_countdown1 = 0;
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
	: Scene(vm, parentModule, true), _flag(false), _asMatch(NULL), _asTntMan(NULL),
	_asCreature(NULL), _class466(NULL), _asLeftDoor(NULL), _asRightDoor(NULL), _asTape(NULL) {

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

	_class466 = addSprite(new Class466(_vm, getGlobalVar(0x000CF819) && which != 1));
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
		_asRightDoor = NULL;
	} else {
		_background = addBackground(new DirtyBackground(_vm, 0x40206EC5, 0, 0));
		_palette = new Palette(_vm, 0x40206EC5);
		_palette->usePalette();
		_asRightDoor = addSprite(new AsScene1201RightDoor(_vm, _klayman, which == 2));
	}

	if (getGlobalVar(0x000CF819)) {
		addSprite(new StaticSprite(_vm, 0x10002ED8, 500));
		if (!getGlobalVar(0x0A18CA33)) {
			AsScene1201TntMan *asTntMan;
			asTntMan = new AsScene1201TntMan(_vm, this, _class466, which == 1);
			asTntMan->getSurface()->getClipRect().x1 = x1;
			asTntMan->getSurface()->getClipRect().y1 = 0;
			asTntMan->getSurface()->getClipRect().x2 = x2;
			asTntMan->getSurface()->getClipRect().y2 = 480;
			_vm->_collisionMan->addSprite(asTntMan);
			_asTntMan = addSprite(asTntMan);
			tempSprite = addSprite(new Class465(_vm, _asTntMan));
			tempSprite->getSurface()->getClipRect().x1 = x1;
			tempSprite->getSurface()->getClipRect().y1 = 0;
			tempSprite->getSurface()->getClipRect().x2 = x2;
			tempSprite->getSurface()->getClipRect().y2 = 480;
			asTntMan->setRepl(64, 0);
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

	_asMatch = NULL;

	if (getGlobalVar(0x0112090A) < 3) {
		_asMatch = addSprite(new AsScene1201Match(_vm, this));
		_vm->_collisionMan->addSprite(_asMatch);
	}

	if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
		_asCreature = addSprite(new AsScene1201Creature(_vm, this, _klayman));
		_asCreature->getSurface()->getClipRect().x1 = x1;
		_asCreature->getSurface()->getClipRect().y1 = 0;
		_asCreature->getSurface()->getClipRect().x2 = x2;
		_asCreature->getSurface()->getClipRect().y2 = 480;
	}

}

Scene1201::~Scene1201() {
}

void Scene1201::update() {
	Scene::update();
	if (_asMatch && getGlobalVar(0x0112090A)) {
		deleteSprite(&_asMatch);
	}
}

uint32 Scene1201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Scene1201::handleMessage(%04X)", messageNum);
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x07053000) {
			_flag = true;
			sendMessage(_asCreature, 0x2004, 0);
		} else if (param.asInteger() == 0x140E5744) {
			sendMessage(_asCreature, 0x2005, 0);
		} else if (param.asInteger() == 0x40253C40) {
			_messageListFlag = false;
			sendMessage(_asCreature, 0x2006, 0);
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
			sendEntityMessage(_klayman, 0x1014, _asMatch);
			setMessageList2(0x004AECC0);
		}
		break;
	case 0x2002:		
		if (getGlobalVar(0x20A0C516)) {
			sendEntityMessage(_klayman, 0x1014, _asTntMan);
			setMessageList2(0x004AECF0);
		} else if (getGlobalVar(0x0112090A) == 3) {
			sendEntityMessage(_klayman, 0x1014, _asTntMan);
			if (_klayman->getX() > _asTntMan->getX()) {
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
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004AED38);
		}
		break;
	case 0x4829:
		sendMessage(_asRightDoor, 0x4829, 0);
		break;		
	}
	return messageResult;
}

// Scene1202

static const uint32 kScene1202Table[] = {
	1, 2, 0, 4, 5, 3, 7, 8, 6, 10, 11, 9, 13, 14, 12, 16, 17, 15
};

static const NPoint kScene1202Points[] = {
	{203, 140},
	{316, 212},
	{277, 264},
	{176, 196},
	{275, 159},
	{366, 212},
	{230, 195},
	{412, 212},
	{368, 263},
	{204, 192},
	{365, 164},
	{316, 262},
	{191, 255},
	{280, 213},
	{406, 266},
	{214, 254},
	{316, 158},
	{402, 161}
};

static const uint32 kScene1202FileHashes[] = {
	0x1AC00B8,
	0x1AC14B8,
	0x1AC14B8,
	0x1AC30B8,
	0x1AC14B8,
	0x1AC14B8,
	0x1AC00B8,
	0x1AC14B8,
	0x1AC14B8,
	0x1AC90B8,
	0x1AC18B8,
	0x1AC18B8,
	0x1AC30B8,
	0x1AC14B8,
	0x1AC14B8,
	0x1AC50B8,
	0x1AC14B8,
	0x1AC14B8
};

AsScene1202TntItem::AsScene1202TntItem(NeverhoodEngine *vm, Scene *parentScene, int index)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _index(index) {

	int positionIndex;

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1202TntItem::handleMessage453FE0);
	positionIndex = getSubVar(0x10055D14, _index);
	createSurface(900, 37, 67);
	_x = kScene1202Points[positionIndex].x;
	_y = kScene1202Points[positionIndex].y;
	sub4540A0();
}

uint32 AsScene1202TntItem::handleMessage453FE0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x2000, _index);
		messageResult = 1;
		break;
	case 0x2001:
		_index2 = (int)param.asInteger();
		sub4540D0();
		break;		
	}
	return messageResult;
}

uint32 AsScene1202TntItem::handleMessage454060(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene1202TntItem::sub4540A0() {
	setFileHash(kScene1202FileHashes[_index], 0, -1);
	SetMessageHandler(&AsScene1202TntItem::handleMessage453FE0);
	_newHashListIndex = 0;
}

void AsScene1202TntItem::sub4540D0() {
	setFileHash(kScene1202FileHashes[_index], 0, -1);
	SetMessageHandler(&AsScene1202TntItem::handleMessage454060);
	SetAnimationCallback3(&AsScene1202TntItem::sub454100);
}

void AsScene1202TntItem::sub454100() {
	_x = kScene1202Points[_index2].x;
	_y = kScene1202Points[_index2].y;
	setFileHash(kScene1202FileHashes[_index], 6, -1);
	SetMessageHandler(&AsScene1202TntItem::handleMessage454060);
	SetAnimationCallback3(&AsScene1202TntItem::sub454160);
	_playBackwards = true;
}

void AsScene1202TntItem::sub454160() {
	sendMessage(_parentScene, 0x2002, _index);
	sub4540A0();
}

Scene1202::Scene1202(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _paletteResource(vm), _soundResource1(vm), 
	_soundResource2(vm), _soundResource3(vm), _soundResource4(vm),
	_flag(true), _soundFlag(false), _counter(0), _index(-1) {

	Palette2 *palette2;

	SetMessageHandler(&Scene1202::handleMessage453C10);
	SetUpdateHandler(&Scene1202::update);

	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x60210ED5, 0, 0));

	palette2 = new Palette2(_vm, 0x60210ED5);
	palette2->usePalette();
	_palette = palette2;
	addEntity(_palette);

	_paletteResource.load(0x60250EB5);
	_paletteResource.copyPalette(_paletteData);

	_mouseCursor = addSprite(new Mouse435(_vm, 0x10ED160A, 20, 620));

	for (int i = 0; i < 18; i++) {
		_asTntItems[i] = addSprite(new AsScene1202TntItem(_vm, this, i));
		_vm->_collisionMan->addSprite(_asTntItems[i]);
	}

	addSprite(new StaticSprite(_vm, 0x8E8419C1, 1100));

	if (getGlobalVar(0x000CF819)) {
		SetMessageHandler(&Scene1202::handleMessage453D90);
	}

	_soundResource1.play(0x40106542);
	_soundResource2.load(0x40005446);
	_soundResource2.load(0x40005446);
	_soundResource2.load(0x68E25540);

}

Scene1202::~Scene1202() {
	if (isSolved()) {
		setGlobalVar(0x000CF819, 1);
	}
}

void Scene1202::update() {
	Scene::update();
	if (_soundFlag) {
		if (!_soundResource4.isPlaying()) {
			sendMessage(_parentModule, 0x1009, 0);
		}
	} else if (_counter == 0 && isSolved()) {
		SetMessageHandler(&Scene1202::handleMessage453D90);
		setGlobalVar(0x000CF819, 1);
		doPaletteEffect();
		_soundResource4.play();
		_soundFlag = true;
	} else if (_index >= 0 && _counter == 0) {
		int index2 = kScene1202Table[_index];
		sendMessage(_asTntItems[_index], 0x2001, getSubVar(0x10055D14, index2));
		sendMessage(_asTntItems[index2], 0x2001, getSubVar(0x10055D14, _index));
		int temp = getSubVar(0x10055D14, index2);
		setSubVar(0x10055D14, index2, getSubVar(0x10055D14, _index));
		setSubVar(0x10055D14, _index, temp);
		_counter = 2;
		_index = -1;
		if (_flag) {
			_soundResource2.play();
		} else {
			_soundResource3.play();
		}
		_flag = !_flag;
	}
}

uint32 Scene1202::handleMessage453C10(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug/Cheat stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !_soundFlag) {
			sendMessage(_parentModule, 0x1009, 0);
		}
		break;
	case 0x000D:
		if (param.asInteger() == 0x14210006) {
			// TODO: Debug/Cheat stuff
			messageResult = 1;
		}
		break;
	case 0x2000:
		_index = (int)param.asInteger();
		break;
	case 0x2002:
		_counter--;
		break;
	}
	return messageResult;
}

uint32 Scene1202::handleMessage453D90(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			sendMessage(_parentModule, 0x1009, 0);
		}
		break;
	}
	return 0;
}

bool Scene1202::isSolved() {
	return 
		getSubVar(0x10055D14,  0) ==  0 && getSubVar(0x10055D14,  3) ==  3 && 
		getSubVar(0x10055D14,  6) ==  6 && getSubVar(0x10055D14,  9) ==  9 &&
		getSubVar(0x10055D14, 12) == 12 && getSubVar(0x10055D14, 15) == 15;
}

void Scene1202::doPaletteEffect() {
#if 0 // TODO
	Palette2 *palette2 = (Palette2*)_palette;
	palette2->startFadeToPalette(24);
#endif
}

} // End of namespace Neverhood
