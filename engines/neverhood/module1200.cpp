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
	: Module(vm, parentModule) {
	
	SetMessageHandler(&Module1200::handleMessage);

	debug("Module1200: which = %d", which);
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(0, 2);
	} else {
		createScene(0, 0);
	}

	_vm->_soundMan->addMusic(0x00478311, 0x62222CAE);
	_vm->_soundMan->startMusic(0x62222CAE, 0, 0);
}

Module1200::~Module1200() {
	_vm->_soundMan->deleteMusicGroup(0x00478311);
}

void Module1200::createScene(int sceneNum, int which) {
	debug("Module1200::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene1201(_vm, this, which);
		break;
	case 1:
		_childObject = new Scene1202(_vm, this, which);
		break;
	case 2:
		_vm->_soundMan->stopMusic(0x62222CAE, 0, 0);
		createSmackerScene(0x31890001, true, true, false);
		setGlobalVar(0x2A02C07B, 1);
		break;
	}
	SetUpdateHandler(&Module1200::updateScene);
	_childObject->handleUpdate();
}

void Module1200::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 0);
			} else if (_moduleResult == 2) {
				if (getGlobalVar(0x0A18CA33) && !getGlobalVar(0x2A02C07B)) {
					createScene(2, -1);
				} else {
					leaveModule(1);
				}
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			createScene(0, 1);
			break;
		case 2:
			_vm->_soundMan->startMusic(0x62222CAE, 0, 0);
			createScene(0, 3);
			break;
		}
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
	setClipRect(0, 0, 640, clipY2);
	_needRefresh = true;
	StaticSprite::update();
}
	
AsScene1201Tape::AsScene1201Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 nameHash, int surfacePriority, int16 x, int16 y, uint32 fileHash)
	: AnimatedSprite(vm, fileHash, surfacePriority, x, y), _parentScene(parentScene), _nameHash(nameHash) {
	
	if (!getSubVar(0x02038314, _nameHash) && !getSubVar(0x02720344, _nameHash)) {
		SetMessageHandler(&AsScene1201Tape::handleMessage);
	} else {
		setVisible(false);
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
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

AsScene1201TntManRope::AsScene1201TntManRope(NeverhoodEngine *vm, bool flag)
	: AnimatedSprite(vm, 1200) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201TntManRope::handleMessage);
	createSurface(10, 34, 149);
	_x = 202;
	_y = -32;
	if (flag) {
		startAnimation(0x928F0C10, 15, -1);
		_newStickFrameIndex = -2;
	} else {
		startAnimation(0x928F0C10, 0, -1);
		_newStickFrameIndex = 0;
	}
}

uint32 AsScene1201TntManRope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02060018) {
			playSound(0, 0x47900E06);
		}
		break;
	case 0x2006:
		startAnimation(0x928F0C10, 1, -1);
		_newStickFrameIndex = -2;
		break;
	}
	return messageResult;
}

AsScene1201RightDoor::AsScene1201RightDoor(NeverhoodEngine *vm, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _klayman(klayman), _countdown(0) {

	createSurface1(0xD088AC30, 100);
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AsScene1201RightDoor::update);
	SetMessageHandler(&AsScene1201RightDoor::handleMessage);
	_newStickFrameIndex = -2;
	if (flag) {
		startAnimation(0xD088AC30, -1, -1);
		_newStickFrameIndex = -2;
		_countdown = 25;
	} else {
		stopAnimation();
		setVisible(false);
	}
}

void AsScene1201RightDoor::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		stCloseDoor();
	}
	AnimatedSprite::update();
}

uint32 AsScene1201RightDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	case 0x4829:
		stOpenDoor();
		break;
	}
	return messageResult;
}

void AsScene1201RightDoor::stOpenDoor() {
	startAnimation(0xD088AC30, 0, -1);
	_newStickFrameIndex = -2;
	setVisible(true);
	playSound(0, calcHash("fxDoorOpen20"));
}

void AsScene1201RightDoor::stCloseDoor() {
	startAnimation(0xD088AC30, -1, -1);
	_playBackwards = true;
	setVisible(true);
	playSound(0, calcHash("fxDoorClose20"));
	NextState(&AsScene1201RightDoor::stCloseDoorDone);
}

void AsScene1201RightDoor::stCloseDoorDone() {
	stopAnimation();
	setVisible(false);
}
		
AsScene1201KlaymanHead::AsScene1201KlaymanHead(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {
	
	createSurface(1200, 69, 98);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201KlaymanHead::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	setVisible(false);
}

uint32 AsScene1201KlaymanHead::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2006:
		_x = 436;
		_y = 339;
		startAnimation(0xA060C599, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		gotoNextState();
		break;
	}
	return messageResult;
}

AsScene1201TntMan::AsScene1201TntMan(NeverhoodEngine *vm, Scene *parentScene, Sprite *asTntManRope, bool isComingDown)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _asTntManRope(asTntManRope),
	_isMoving(false) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201TntMan::handleMessage);
	createSurface(990, 106, 181);
	_x = 201;
	if (isComingDown) {
		_y = 297;
		stComingDown();
	} else {
		_y = 334;
		stStanding();
	}
}

AsScene1201TntMan::~AsScene1201TntMan() {
	_vm->_soundMan->deleteSoundGroup(0x01D00560);
}	 

uint32 AsScene1201TntMan::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x092870C0) {
			sendMessage(_asTntManRope, 0x2006, 0);
		} else if (param.asInteger() == 0x11CA0144) {
			playSound(0, 0x51800A04);
		}
		break;
	case 0x1011:
		sendMessage(_parentScene, 0x2002, 0);
		messageResult = 1;
		break;
	case 0x480B:
		if (!_isMoving) {
			_sprite = (Sprite*)sender;
			stMoving();
		}
		break;
	}
	return messageResult;

}

uint32 AsScene1201TntMan::hmComingDown(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = AsScene1201TntMan::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1201TntMan::suMoving() {
	_x = _sprite->getX() + 100;
}

void AsScene1201TntMan::stStanding() {
	startAnimation(0x654913D0, 0, -1);
	SetMessageHandler(&AsScene1201TntMan::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsScene1201TntMan::stComingDown() {
	startAnimation(0x356803D0, 0, -1);
	SetMessageHandler(&AsScene1201TntMan::hmComingDown);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	NextState(&AsScene1201TntMan::stStanding);
}

void AsScene1201TntMan::stMoving() {
	_vm->_soundMan->addSound(0x01D00560, 0x4B044624);
	_vm->_soundMan->playSoundLooping(0x4B044624);
	_isMoving = true;
	startAnimation(0x85084190, 0, -1);
	SetMessageHandler(&AsScene1201TntMan::handleMessage);
	SetSpriteUpdate(&AsScene1201TntMan::suMoving);
	_newStickFrameIndex = -2;
}

AsScene1201TntManFlame::AsScene1201TntManFlame(NeverhoodEngine *vm, Sprite *asTntMan)
	: AnimatedSprite(vm, 1200), _asTntMan(asTntMan) {

	createSurface1(0x828C0411, 995);
	SetUpdateHandler(&AsScene1201TntManFlame::update);
	SetMessageHandler(&Sprite::handleMessage);
	SetSpriteUpdate(&AsScene1201TntManFlame::suUpdate);
	startAnimation(0x828C0411, 0, -1);
	setVisible(false);
}

AsScene1201TntManFlame::~AsScene1201TntManFlame() {
	_vm->_soundMan->deleteSoundGroup(0x041080A4);
}

void AsScene1201TntManFlame::update() {
	AnimatedSprite::update();
	if (getGlobalVar(0x20A0C516)) {
		setVisible(true);
		SetUpdateHandler(&AnimatedSprite::update);
		_vm->_soundMan->addSound(0x041080A4, 0x460A1050);
		_vm->_soundMan->playSoundLooping(0x460A1050);
	}
}

void AsScene1201TntManFlame::suUpdate() {
	_x = _asTntMan->getX() - 18;
	_y = _asTntMan->getY() - 158;
}

AsScene1201Match::AsScene1201Match(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _countdown(0) {
	
	createSurface(1100, 57, 60);
	SetUpdateHandler(&AsScene1201Match::update);
	SetMessageHandler(&AsScene1201Match::hmOnDoorFrameAboutToMove);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	switch (getGlobalVar(0x0112090A)) {
	case 0:
		_x = 521;
		_y = 112;
		_status = 0;
		stIdleOnDoorFrame();
		break;
	case 1:
		_x = 521;
		_y = 112;
		_status = 2;
		stOnDoorFrameAboutToMove();
		loadSound(0, 0xD00230CD);
		break;
	case 2:
		setDoDeltaX(1);
		_x = 403;
		_y = 337;
		_status = 0;
		stIdleOnFloor();
		break;
	}
}

void AsScene1201Match::update() {
	if (_countdown != 0 && (--_countdown == 0))
		gotoNextState();
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene1201Match::hmOnDoorFrameAboutToMove(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x86668011) {
			playSound(0);
		}
		break;
	}
	return messageResult;
}

uint32 AsScene1201Match::hmOnDoorFrameMoving(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmOnDoorFrameAboutToMove(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

uint32 AsScene1201Match::hmIdle(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmOnDoorFrameAboutToMove(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x2001, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setVisible(false);
		setGlobalVar(0x0112090A, 3);
		break;
	}
	return messageResult;
}

void AsScene1201Match::stOnDoorFrameMoving() {
	startAnimation(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::hmOnDoorFrameMoving);
	if (_status == 0) {
		NextState(&AsScene1201Match::stFallingFromDoorFrame);
	} else {
		NextState(&AsScene1201Match::stOnDoorFrameAboutToMove);
	}
}

void AsScene1201Match::stFallingFromDoorFrame() {
	setGlobalVar(0x0112090A, 2);
	_x -= 199;
	_y += 119;
	startAnimation(0x018D0240, 0, -1);
	SetMessageHandler(&AsScene1201Match::hmOnDoorFrameMoving);
	NextState(&AsScene1201Match::stIdleOnFloor);
}

void AsScene1201Match::stOnDoorFrameAboutToMove() {
	startAnimation(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::hmOnDoorFrameAboutToMove);
	_newStickFrameIndex = 0;
	if (_status != 0) {
		_countdown = 36;
		_status--;
		NextState(&AsScene1201Match::stOnDoorFrameMoving);
	}
}

void AsScene1201Match::stIdleOnDoorFrame() {
	startAnimation(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::hmIdle);
	_newStickFrameIndex = 0;
}

void AsScene1201Match::stIdleOnFloor() {
	setDoDeltaX(1);
	_x = 403;
	_y = 337;
	startAnimation(0x00842374, 0, -1);
	SetMessageHandler(&AsScene1201Match::hmIdle);
	_newStickFrameIndex = 0;
}

AsScene1201Creature::AsScene1201Creature(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _klayman(klayman), _klaymanTooClose(false) {

	// NOTE: _countdown2 and _countdown3 were unused/without effect and thus removed
	
	createSurface(1100, 203, 199);
	SetUpdateHandler(&AsScene1201Creature::update);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	_x = 540;
	_y = 320;
	stWaiting();
}

void AsScene1201Creature::update() {
	bool oldFlag = _klaymanTooClose;
	_klaymanTooClose = _klayman->getX() >= 385;
	if (_klaymanTooClose != oldFlag)
		stWaiting();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		gotoNextState();
	}
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene1201Creature::hmWaiting(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02060018) {
			playSound(0, 0xCD298116);
		}
		break;
	case 0x2004:
		GotoState(&AsScene1201Creature::stStartReachForTntDummy);
		break;
	case 0x2006:
		GotoState(&AsScene1201Creature::stPincerSnapKlayman);
		break;
	}
	return messageResult;
}

uint32 AsScene1201Creature::hmPincerSnap(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmWaiting(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

uint32 AsScene1201Creature::hmPincerSnapKlayman(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02060018) {
			playSound(0, 0xCD298116);
			sendMessage(_parentScene, 0x4814, 0);
			sendMessage(_klayman, 0x4814, 0);
		}
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1201Creature::stWaiting() {
	startAnimation(0x08081513, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	NextState(&AsScene1201Creature::stPincerSnap);
	_countdown1 = 36;
}

void AsScene1201Creature::stPincerSnap() {
	if (!_klaymanTooClose) {
		startAnimation(0xCA287133, 0, -1);
		SetMessageHandler(&AsScene1201Creature::hmPincerSnap);
		NextState(&AsScene1201Creature::stWaiting);
	}
}

void AsScene1201Creature::stStartReachForTntDummy() {
	startAnimation(0x08081513, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	NextState(&AsScene1201Creature::stReachForTntDummy);
	_countdown1 = 48;
}

void AsScene1201Creature::stReachForTntDummy() {
	startAnimation(0x5A201453, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	_countdown1 = 0;
}

void AsScene1201Creature::stPincerSnapKlayman() {
	startAnimation(0xCA287133, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmPincerSnapKlayman);
	NextState(&AsScene1201Creature::stWaiting);
	_countdown1 = 0;
}

AsScene1201LeftDoor::AsScene1201LeftDoor(NeverhoodEngine *vm, Sprite *klayman)
	: AnimatedSprite(vm, 1100), _klayman(klayman) {

	_x = 320;
	_y = 240;
	createSurface(800, 55, 199);
	if (_klayman->getX() < 100) {
		startAnimation(0x508A111B, 0, -1);
		_newStickFrameIndex = -2;
		playSound(0, calcHash("fxDoorOpen03"));
	} else {
		startAnimation(0x508A111B, -1, -1);
		_newStickFrameIndex = -2;
	}
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201LeftDoor::handleMessage);
}

uint32 AsScene1201LeftDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4809:
		stCloseDoor();
		break;
	}
	return messageResult;
}

void AsScene1201LeftDoor::stCloseDoor() {
	startAnimation(0x508A111B, -1, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
}

Scene1201::Scene1201(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _flag(false), _asMatch(NULL), _asTntMan(NULL),
	_asCreature(NULL), _asTntManRope(NULL), _asLeftDoor(NULL), _asRightDoor(NULL), _asTape(NULL) {

	int16 topY1, topY2, topY3, topY4;
	int16 x1, x2;
	Sprite *tempSprite;

	SetUpdateHandler(&Scene1201::update);
	SetMessageHandler(&Scene1201::handleMessage);

	setHitRects(0x004AEBD0);
	
	_surfaceFlag = true;

	if (!getSubVar(0x40050052, 0xE8058B52)) {
		setSubVar(0x40050052, 0xE8058B52, 1);
		for (uint32 index = 0; index < 18; index++) {
			setSubVar(0x10055D14, index, kScene1201InitArray[index]);
		}
	}

	insertMouse433(0x9A2C0409);
	
	_asTape = insertSprite<AsScene1201Tape>(this, 3, 1100, 243, 340, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape);
	
	tempSprite = insertStaticSprite(0x03C82530, 100);
	topY1 = tempSprite->getY() + tempSprite->getDrawRect().height; 

	tempSprite = insertStaticSprite(0x88182069, 200);
	topY2 = tempSprite->getY() + tempSprite->getDrawRect().height; 

	tempSprite = insertStaticSprite(0x476014E0, 300);
	topY3 = tempSprite->getY() + tempSprite->getDrawRect().height; 

	tempSprite = insertStaticSprite(0x04063110, 500);
	topY4 = tempSprite->getY() + 1; 

	_asTntManRope = insertSprite<AsScene1201TntManRope>(getGlobalVar(0x000CF819) && which != 1);
	_asTntManRope->setClipRect(0, topY4, 640, 480);
	
	insertStaticSprite(0x400B04B0, 1200);

	tempSprite = insertStaticSprite(0x40295462, 1200);
	x1 = tempSprite->getX();

	tempSprite = insertStaticSprite(0xA29223FA, 1200);
	x2 = tempSprite->getX() + tempSprite->getDrawRect().width;

	_asKlaymanHead = insertSprite<AsScene1201KlaymanHead>();

	if (which < 0) {
		insertKlayman<KmScene1201>(364, 333);
		setMessageList(0x004AEC08);
	} else if (which == 3) {
		insertKlayman<KmScene1201>(400, 329);
		setMessageList(0x004AEC08);
	} else if (which == 2) {
		if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
			insertKlayman<KmScene1201>(374, 333);
			setMessageList(0x004AEC08);
		} else {
			insertKlayman<KmScene1201>(640, 329);
			setMessageList(0x004AEC20);
		}
	} else if (which == 1) {
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene1201>(364, 333);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene1201>(246, 333);
		}
		setMessageList(0x004AEC30);
	} else {
		insertKlayman<KmScene1201>(0, 336);
		setMessageList(0x004AEC10);
	}

	_klayman->setClipRect(x1, 0, x2, 480);
	_klayman->setRepl(64, 0);
	
	if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
		setBackground(0x4019A2C4);
		setPalette(0x4019A2C4);
		_asRightDoor = NULL;
	} else {
		setBackground(0x40206EC5);
		setPalette(0x40206EC5);
		_asRightDoor = insertSprite<AsScene1201RightDoor>(_klayman, which == 2);
	}

	if (getGlobalVar(0x000CF819)) {
		insertStaticSprite(0x10002ED8, 500);
		if (!getGlobalVar(0x0A18CA33)) {
			_asTntMan = insertSprite<AsScene1201TntMan>(this, _asTntManRope, which == 1);
			_asTntMan->setClipRect(x1, 0, x2, 480);
			_asTntMan->setRepl(64, 0);
			_vm->_collisionMan->addSprite(_asTntMan);
			tempSprite = insertSprite<AsScene1201TntManFlame>(_asTntMan);
			tempSprite->setClipRect(x1, 0, x2, 480);
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
			insertSprite<SsScene1201Tnt>(tntIndex, getSubVar(0x10055D14, tntIndex), clipY2);
			elemIndex = getSubVar(0x10055D14, tntIndex + 1);
			if (kScene1201PointArray[elemIndex].y < 175)
				clipY2 = topY1;
			else if (kScene1201PointArray[elemIndex].y < 230)
				clipY2 = topY2;
			else
				clipY2 = topY3;
			insertSprite<SsScene1201Tnt>(tntIndex + 1, getSubVar(0x10055D14, tntIndex + 1), clipY2);
			tntIndex += 3;
		}

		if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
			setRectList(0x004AEE58);
		} else {
			setRectList(0x004AEDC8);
		} 
		
	} else {
	
		insertStaticSprite(0x8E8A1981, 900);

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
			insertSprite<SsScene1201Tnt>(tntIndex, getSubVar(0x10055D14, tntIndex), clipY2);
			tntIndex++;
		}

		if (getGlobalVar(0x0A310817) && !getGlobalVar(0x0A18CA33)) {
			setRectList(0x004AEE18);
		} else {
			setRectList(0x004AED88);
		}
		 
	}

	tempSprite = insertStaticSprite(0x63D400BC, 900);

	_asLeftDoor = insertSprite<AsScene1201LeftDoor>(_klayman);
	_asLeftDoor->setClipRect(x1, tempSprite->getDrawRect().y, tempSprite->getDrawRect().x2(), 480);

	if (getGlobalVar(0x0A310817) && getGlobalVar(0x0112090A) == 0) {
		setGlobalVar(0x0112090A, 1);
	}

	_asMatch = NULL;

	if (getGlobalVar(0x0112090A) < 3) {
		_asMatch = insertSprite<AsScene1201Match>(this);
		_vm->_collisionMan->addSprite(_asMatch);
	}

	if (getGlobalVar(0x0A310817) && getGlobalVar(0x0A18CA33) == 0) {
		_asCreature = insertSprite<AsScene1201Creature>(this, _klayman);
		_asCreature->setClipRect(x1, 0, x2, 480);
	}

}

Scene1201::~Scene1201() {
}

void Scene1201::update() {
	Scene::update();
	if (_asMatch && getGlobalVar(0x0112090A) == 3)
		deleteSprite(&_asMatch);
}

uint32 Scene1201::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x07053000) {
			_flag = true;
			sendMessage(_asCreature, 0x2004, 0);
		} else if (param.asInteger() == 0x140E5744) {
			sendMessage(_asCreature, 0x2005, 0);
		} else if (param.asInteger() == 0x40253C40) {
			_canAcceptInput = false;
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
			// Move the TNT dummy
			sendEntityMessage(_klayman, 0x1014, _asTntMan);
			setMessageList2(0x004AECF0, false);
		} else if (getGlobalVar(0x0112090A) == 3) {
			// Light the TNT dummy
			sendEntityMessage(_klayman, 0x1014, _asTntMan);
			if (_klayman->getX() > _asTntMan->getX()) {
				setMessageList(0x004AECD0);
			} else {
				setMessageList(0x004AECE0);
			}
		}
		break;
	case 0x4814:
		cancelMessageList();
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
	case 0x8000:
		sendMessage(_asKlaymanHead, 0x2006, 0);
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

AsScene1202TntItem::AsScene1202TntItem(NeverhoodEngine *vm, Scene *parentScene, int itemIndex)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _itemIndex(itemIndex) {

	int positionIndex;

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1202TntItem::hmShowIdle);
	positionIndex = getSubVar(0x10055D14, _itemIndex);
	createSurface(900, 37, 67);
	_x = kScene1202Points[positionIndex].x;
	_y = kScene1202Points[positionIndex].y;
	stShowIdle();
}

uint32 AsScene1202TntItem::hmShowIdle(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x2000, _itemIndex);
		messageResult = 1;
		break;
	case 0x2001:
		_newPosition = (int)param.asInteger();
		stChangePositionFadeOut();
		break;		
	}
	return messageResult;
}

uint32 AsScene1202TntItem::hmChangePosition(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1202TntItem::stShowIdle() {
	startAnimation(kScene1202FileHashes[_itemIndex], 0, -1);
	SetMessageHandler(&AsScene1202TntItem::hmShowIdle);
	_newStickFrameIndex = 0;
}

void AsScene1202TntItem::stChangePositionFadeOut() {
	startAnimation(kScene1202FileHashes[_itemIndex], 0, -1);
	SetMessageHandler(&AsScene1202TntItem::hmChangePosition);
	NextState(&AsScene1202TntItem::stChangePositionFadeIn);
}

void AsScene1202TntItem::stChangePositionFadeIn() {
	_x = kScene1202Points[_newPosition].x;
	_y = kScene1202Points[_newPosition].y;
	startAnimation(kScene1202FileHashes[_itemIndex], 6, -1);
	SetMessageHandler(&AsScene1202TntItem::hmChangePosition);
	NextState(&AsScene1202TntItem::stChangePositionDone);
	_playBackwards = true;
}

void AsScene1202TntItem::stChangePositionDone() {
	sendMessage(_parentScene, 0x2002, _itemIndex);
	stShowIdle();
}

Scene1202::Scene1202(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _paletteResource(vm),  
	_flag(true), _soundFlag(false), _counter(0), _clickedIndex(-1) {

	SetMessageHandler(&Scene1202::handleMessage);
	SetUpdateHandler(&Scene1202::update);

	_surfaceFlag = true;

	setBackground(0x60210ED5);

	setPalette(0x60210ED5);
	addEntity(_palette);

	_paletteResource.load(0x60250EB5);
	_paletteResource.copyPalette(_paletteData);

	insertMouse435(0x10ED160A, 20, 620);

	for (int i = 0; i < 18; i++) {
		_asTntItems[i] = insertSprite<AsScene1202TntItem>(this, i);
		_vm->_collisionMan->addSprite(_asTntItems[i]);
	}

	insertStaticSprite(0x8E8419C1, 1100);

	if (getGlobalVar(0x000CF819)) {
		SetMessageHandler(&Scene1202::hmSolved);
	}

	playSound(0, 0x40106542);
	loadSound(1, 0x40005446);
	loadSound(2, 0x40005446);
	loadSound(3, 0x68E25540);

}

Scene1202::~Scene1202() {
	if (isSolved()) {
		setGlobalVar(0x000CF819, 1);
	}
}

void Scene1202::update() {
	Scene::update();
	if (_soundFlag) {
		debug("CHECK SOLVED");
		if (!isSoundPlaying(3))
			leaveScene(0);
	} else if (_counter == 0 && isSolved()) {
		_clickedIndex = 0;
		SetMessageHandler(&Scene1202::hmSolved);
		setGlobalVar(0x000CF819, 1);
		doPaletteEffect();
		playSound(3);
		_soundFlag = true;
	} else if (_clickedIndex >= 0 && _counter == 0) {
		int destIndex = kScene1202Table[_clickedIndex];
		sendMessage(_asTntItems[_clickedIndex], 0x2001, getSubVar(0x10055D14, destIndex));
		sendMessage(_asTntItems[destIndex], 0x2001, getSubVar(0x10055D14, _clickedIndex));
		int temp = getSubVar(0x10055D14, destIndex);
		setSubVar(0x10055D14, destIndex, getSubVar(0x10055D14, _clickedIndex));
		setSubVar(0x10055D14, _clickedIndex, temp);
		_counter = 2;
		_clickedIndex = -1;
		if (_flag) {
			playSound(1);
		} else {
			playSound(2);
		}
		_flag = !_flag;
	}
}

uint32 Scene1202::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug/Cheat stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !_soundFlag) {
			leaveScene(0);
		}
		break;
	case 0x000D:
		if (param.asInteger() == 0x14210006) {
			// TODO: Debug/Cheat stuff
			messageResult = 1;
		}
		break;
	case 0x2000:
		_clickedIndex = (int)param.asInteger();
		break;
	case 0x2002:
		_counter--;
		break;
	}
	return messageResult;
}

uint32 Scene1202::hmSolved(int messageNum, const MessageParam &param, Entity *sender) {
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

bool Scene1202::isSolved() {

	debug("isSolved() %d %d %d %d %d %d", 
		getSubVar(0x10055D14,  0), getSubVar(0x10055D14,  3),
		getSubVar(0x10055D14,  6), getSubVar(0x10055D14,  9),
		getSubVar(0x10055D14,  12), getSubVar(0x10055D14,  15));

	return 
		getSubVar(0x10055D14,  0) ==  0 && getSubVar(0x10055D14,  3) ==  3 && 
		getSubVar(0x10055D14,  6) ==  6 && getSubVar(0x10055D14,  9) ==  9 &&
		getSubVar(0x10055D14, 12) == 12 && getSubVar(0x10055D14, 15) == 15;
}

void Scene1202::doPaletteEffect() {
	// TODO
}

} // End of namespace Neverhood
