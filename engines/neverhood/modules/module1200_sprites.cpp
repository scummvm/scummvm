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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/modules/module1200_sprites.h"

namespace Neverhood {

static const uint32 kScene1201TntFileHashList1[] = {
	0x2098212D, 0x1600437E, 0x1600437E,
	0x00A840E3, 0x1A1830F6, 0x1A1830F6,
	0x00212062, 0x384010B6, 0x384010B6,
	0x07A01080, 0xD80C2837, 0xD80C2837,
	0x03A22092, 0xD8802CB6, 0xD8802CB6,
	0x03A93831, 0xDA460476, 0xDA460476
};

static const uint32 kScene1201TntFileHashList2[] = {
	0x3040C676, 0x10914448, 0x10914448,
	0x3448A066, 0x1288C049, 0x1288C049,
	0x78C0E026, 0x3098D05A, 0x3098D05A,
	0x304890E6, 0x1284E048, 0x1284E048,
	0xB140A1E6, 0x5088A068, 0x5088A068,
	0x74C4C866, 0x3192C059, 0x3192C059
};

SsScene1201Tnt::SsScene1201Tnt(NeverhoodEngine *vm, uint32 elemIndex, uint32 pointIndex, int16 clipY2)
	: StaticSprite(vm, 900) {

	int16 x = kScene1201PointArray[pointIndex].x;
	int16 y = kScene1201PointArray[pointIndex].y;
	if (x < 300)
		loadSprite(kScene1201TntFileHashList1[elemIndex], kSLFDefDrawOffset | kSLFDefPosition, 50);
	else
		loadSprite(kScene1201TntFileHashList2[elemIndex], kSLFCenteredDrawOffset | kSLFSetPosition, 50, x, y - 20);
	setClipRect(0, 0, 640, clipY2);
}

AsScene1201Tape::AsScene1201Tape(NeverhoodEngine *vm, Scene *parentScene, uint32 nameHash, int surfacePriority, int16 x, int16 y, uint32 fileHash)
	: AnimatedSprite(vm, fileHash, surfacePriority, x, y), _parentScene(parentScene), _nameHash(nameHash) {

	if (!getSubVar(VA_HAS_TAPE, _nameHash) && !getSubVar(VA_IS_TAPE_INSERTED, _nameHash)) {
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
	case NM_KLAYMEN_USE_OBJECT:
		setSubVar(VA_HAS_TAPE, _nameHash, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene1201TntManRope::AsScene1201TntManRope(NeverhoodEngine *vm, bool isDummyHanging)
	: AnimatedSprite(vm, 1200) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201TntManRope::handleMessage);
	createSurface(10, 34, 149);
	_x = 202;
	_y = -32;
	if (isDummyHanging) {
		startAnimation(0x928F0C10, 15, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	} else {
		startAnimation(0x928F0C10, 0, -1);
		_newStickFrameIndex = 0;
	}
}

uint32 AsScene1201TntManRope::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x02060018)
			playSound(0, 0x47900E06);
		break;
	case NM_KLAYMEN_STOP_CLIMBING:
		startAnimation(0x928F0C10, 1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene1201RightDoor::AsScene1201RightDoor(NeverhoodEngine *vm, Sprite *klaymen, bool isOpen)
	: AnimatedSprite(vm, 1100), _klaymen(klaymen), _countdown(0) {

	createSurface1(0xD088AC30, 100);
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AsScene1201RightDoor::update);
	SetMessageHandler(&AsScene1201RightDoor::handleMessage);
	_newStickFrameIndex = STICK_LAST_FRAME;
	if (isOpen) {
		startAnimation(0xD088AC30, -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
		_countdown = 25;
	} else {
		stopAnimation();
		setVisible(false);
	}
}

void AsScene1201RightDoor::update() {
	if (_countdown != 0 && (--_countdown == 0))
		stCloseDoor();
	AnimatedSprite::update();
}

uint32 AsScene1201RightDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case 0x4829:
		stOpenDoor();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1201RightDoor::stOpenDoor() {
	startAnimation(0xD088AC30, 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
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

AsScene1201KlaymenHead::AsScene1201KlaymenHead(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200) {

	createSurface(1200, 69, 98);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201KlaymenHead::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	setVisible(false);
}

uint32 AsScene1201KlaymenHead::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_STOP_CLIMBING:
		_x = 436;
		_y = 339;
		startAnimation(0xA060C599, 0, -1);
		setVisible(true);
		break;
	case NM_ANIMATION_STOP:
		stopAnimation();
		setVisible(false);
		gotoNextState();
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x092870C0)
			sendMessage(_asTntManRope, NM_KLAYMEN_STOP_CLIMBING, 0);
		else if (param.asInteger() == 0x11CA0144)
			playSound(0, 0x51800A04);
		break;
	case 0x1011:
		sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
		messageResult = 1;
		break;
	case 0x480B:
		if (!_isMoving) {
			_sprite = (Sprite*)sender;
			stMoving();
		}
		break;
	default:
		break;
	}
	return messageResult;

}

uint32 AsScene1201TntMan::hmComingDown(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = AsScene1201TntMan::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
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
	_newStickFrameIndex = STICK_LAST_FRAME;
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
	if (getGlobalVar(V_TNT_DUMMY_FUSE_LIT)) {
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
	switch (getGlobalVar(V_MATCH_STATUS)) {
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
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x86668011)
			playSound(0);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1201Match::hmOnDoorFrameMoving(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmOnDoorFrameAboutToMove(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
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
	case NM_KLAYMEN_USE_OBJECT:
		setVisible(false);
		setGlobalVar(V_MATCH_STATUS, 3);
		break;
	default:
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
	setGlobalVar(V_MATCH_STATUS, 2);
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

AsScene1201Creature::AsScene1201Creature(NeverhoodEngine *vm, Scene *parentScene, Sprite *klaymen)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _klaymen(klaymen), _klaymenTooClose(false) {

	// NOTE: _countdown2 and _countdown3 were unused/without effect and thus removed

	createSurface(1100, 203, 199);
	SetUpdateHandler(&AsScene1201Creature::update);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	_x = 540;
	_y = 320;
	stWaiting();
}

void AsScene1201Creature::update() {
	bool oldKlaymenTooClose = _klaymenTooClose;
	_klaymenTooClose = _klaymen->getX() >= 385;
	if (_klaymenTooClose != oldKlaymenTooClose)
		stWaiting();
	if (_countdown != 0 && (--_countdown == 0))
		gotoNextState();
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene1201Creature::hmWaiting(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x02060018)
			playSound(0, 0xCD298116);
		break;
	case 0x2004:
		GotoState(&AsScene1201Creature::stStartReachForTntDummy);
		break;
	case NM_KLAYMEN_STOP_CLIMBING:
		GotoState(&AsScene1201Creature::stPincerSnapKlaymen);
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1201Creature::hmPincerSnap(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmWaiting(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1201Creature::hmPincerSnapKlaymen(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x02060018) {
			playSound(0, 0xCD298116);
			sendMessage(_parentScene, 0x4814, 0);
			sendMessage(_klaymen, 0x4814, 0);
		}
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1201Creature::stWaiting() {
	startAnimation(0x08081513, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	NextState(&AsScene1201Creature::stPincerSnap);
	_countdown = 36;
}

void AsScene1201Creature::stPincerSnap() {
	if (!_klaymenTooClose) {
		startAnimation(0xCA287133, 0, -1);
		SetMessageHandler(&AsScene1201Creature::hmPincerSnap);
		NextState(&AsScene1201Creature::stWaiting);
	}
}

void AsScene1201Creature::stStartReachForTntDummy() {
	startAnimation(0x08081513, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	NextState(&AsScene1201Creature::stReachForTntDummy);
	_countdown = 48;
}

void AsScene1201Creature::stReachForTntDummy() {
	startAnimation(0x5A201453, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmWaiting);
	_countdown = 0;
}

void AsScene1201Creature::stPincerSnapKlaymen() {
	startAnimation(0xCA287133, 0, -1);
	SetMessageHandler(&AsScene1201Creature::hmPincerSnapKlaymen);
	NextState(&AsScene1201Creature::stWaiting);
	_countdown = 0;
}

AsScene1201LeftDoor::AsScene1201LeftDoor(NeverhoodEngine *vm, Sprite *klaymen)
	: AnimatedSprite(vm, 1100), _klaymen(klaymen) {

	_x = 320;
	_y = 240;
	createSurface(800, 55, 199);
	if (_klaymen->getX() < 100) {
		startAnimation(0x508A111B, 0, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
		playSound(0, calcHash("fxDoorOpen03"));
	} else {
		startAnimation(0x508A111B, -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	}
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1201LeftDoor::handleMessage);
}

uint32 AsScene1201LeftDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_CLOSE_DOOR:
		stCloseDoor();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1201LeftDoor::stCloseDoor() {
	startAnimation(0x508A111B, -1, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
}

static const NPoint kScene1202Points[] = {
	{203, 140}, {316, 212}, {277, 264},
	{176, 196}, {275, 159}, {366, 212},
	{230, 195}, {412, 212}, {368, 263},
	{204, 192}, {365, 164}, {316, 262},
	{191, 255}, {280, 213}, {406, 266},
	{214, 254}, {316, 158}, {402, 161}
};

static const uint32 kScene1202FileHashes[] = {
	0x1AC00B8, 0x1AC14B8, 0x1AC14B8,
	0x1AC30B8, 0x1AC14B8, 0x1AC14B8,
	0x1AC00B8, 0x1AC14B8, 0x1AC14B8,
	0x1AC90B8, 0x1AC18B8, 0x1AC18B8,
	0x1AC30B8, 0x1AC14B8, 0x1AC14B8,
	0x1AC50B8, 0x1AC14B8, 0x1AC14B8
};

AsScene1202TntItem::AsScene1202TntItem(NeverhoodEngine *vm, Scene *parentScene, int itemIndex)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _itemIndex(itemIndex) {

	int positionIndex;

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1202TntItem::hmShowIdle);
	positionIndex = getSubVar(VA_TNT_POSITIONS, _itemIndex);
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
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1202TntItem::hmChangePosition(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
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
	_playBackwards = true;
	SetMessageHandler(&AsScene1202TntItem::hmChangePosition);
	NextState(&AsScene1202TntItem::stChangePositionDone);
}

void AsScene1202TntItem::stChangePositionDone() {
	sendMessage(_parentScene, NM_POSITION_CHANGE, _itemIndex);
	stShowIdle();
}

static const KlaymenIdleTableItem klaymenIdleTable1201[] = {
	{1, kIdleSpinHead},
	{1, kIdleChest},
	{1, kIdleHeadOff},
};

KmScene1201::KmScene1201(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	setKlaymenIdleTable(klaymenIdleTable1201, ARRAYSIZE(klaymenIdleTable1201));
	_doYHitIncr = true;
}

uint32 KmScene1201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_MOVE_OBJECT:
		GotoState(&Klaymen::stMoveObject);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4813:
		GotoState(&KmScene1201::stFetchMatch);
		break;
	case 0x4814:
		GotoState(&KmScene1201::stTumbleHeadless);
		break;
	case 0x4815:
		GotoState(&KmScene1201::stCloseEyes);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x481F:
		GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

void KmScene1201::stTumbleHeadless() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene1201::stTumbleHeadless))) {
		_busyStatus = 1;
		_acceptInput = false;
		setDoDeltaX(0);
		startAnimation(0x2821C590, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&KmScene1201::hmTumbleHeadless);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		NextState(&Klaymen::stTryStandIdle);
		sendMessage(_parentScene, 0x8000, 0);
		playSound(0, 0x62E0A356);
	}
}

void KmScene1201::stCloseEyes() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene1201::stCloseEyes))) {
		_busyStatus = 1;
		_acceptInput = false;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmLowLevel);
		SetSpriteUpdate(NULL);
	}
}

uint32 KmScene1201::hmMatch(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x51281850) {
			setGlobalVar(V_TNT_DUMMY_FUSE_LIT, 1);
		} else if (param.asInteger() == 0x43000538) {
			playSound(0, 0x21043059);
		} else if (param.asInteger() == 0x02B20220) {
			playSound(0, 0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			playSound(0, 0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			playSound(0, 0x44051000);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void KmScene1201::stFetchMatch() {
	if (!stStartAction(AnimationCallback(&KmScene1201::stFetchMatch))) {
		_busyStatus = 0;
		_acceptInput = false;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		startAnimation(0x9CAA0218, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&KmScene1201::hmMatch);
		SetSpriteUpdate(NULL);
		NextState(&KmScene1201::stLightMatch);
	}
}

void KmScene1201::stLightMatch() {
	_busyStatus = 1;
	_acceptInput = false;
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	startAnimation(0x1222A513, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&KmScene1201::hmMatch);
	SetSpriteUpdate(NULL);
}

uint32 KmScene1201::hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x000F0082) {
			playSound(0, 0x74E2810F);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
