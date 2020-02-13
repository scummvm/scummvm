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

#include "neverhood/modules/module1300_sprites.h"

namespace Neverhood {

AsScene1302Bridge::AsScene1302Bridge(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	_x = 320;
	_y = 240;
	createSurface1(0x88148150, 500);
	if (!getGlobalVar(V_FLYTRAP_RING_BRIDGE)) {
		startAnimation(0x88148150, 0, -1);
		_newStickFrameIndex = 0;
	} else {
		startAnimation(0x88148150, -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	}
	loadSound(0, 0x68895082);
	loadSound(1, 0x689BD0C1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1302Bridge::handleMessage);
}

uint32 AsScene1302Bridge::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		stLowerBridge();
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		stRaiseBridge();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1302Bridge::stLowerBridge() {
	startAnimation(0x88148150, 0, -1);
	playSound(1);
	NextState(&AsScene1302Bridge::cbLowerBridgeEvent);
}

void AsScene1302Bridge::stRaiseBridge() {
	startAnimation(0x88148150, 7, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
	playSound(0);
}

void AsScene1302Bridge::cbLowerBridgeEvent() {
	sendMessage(_parentScene, 0x2032, 0);
	startAnimation(0x88148150, -1, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

SsScene1302Fence::SsScene1302Fence(NeverhoodEngine *vm)
	: StaticSprite(vm, 0x11122122, 200) {

	_firstY = _y;
	if (getGlobalVar(V_FLYTRAP_RING_FENCE))
		_y += 152;
	loadSound(0, 0x7A00400C);
	loadSound(1, 0x78184098);
	SetUpdateHandler(&SsScene1302Fence::update);
	SetMessageHandler(&SsScene1302Fence::handleMessage);
	SetSpriteUpdate(NULL);
}

void SsScene1302Fence::update() {
	handleSpriteUpdate();
	updatePosition();
}

uint32 SsScene1302Fence::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_OPEN_DOOR:
		playSound(0);
		SetMessageHandler(NULL);
		SetSpriteUpdate(&SsScene1302Fence::suMoveDown);
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		playSound(1);
		SetMessageHandler(NULL);
		SetSpriteUpdate(&SsScene1302Fence::suMoveUp);
		break;
	default:
		break;
	}
	return messageResult;
}

void SsScene1302Fence::suMoveDown() {
	if (_y < _firstY + 152)
		_y += 8;
	else {
		SetMessageHandler(&SsScene1302Fence::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

void SsScene1302Fence::suMoveUp() {
	if (_y > _firstY)
		_y -= 8;
	else {
		SetMessageHandler(&SsScene1302Fence::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

AsScene1303Balloon::AsScene1303Balloon(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(200, 128, 315);
	_x = 289;
	_y = 390;
	startAnimation(0x800278D2, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1303Balloon::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 AsScene1303Balloon::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_ANIMATION_UPDATE:
		stPopBalloon();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1303Balloon::hmBalloonPopped(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x020B0003)
			playSound(0, 0x742B0055);
		break;
	case NM_ANIMATION_STOP:
		playSound(0, 0x470007EE);
		stopAnimation();
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1303Balloon::stPopBalloon() {
	startAnimation(0xAC004CD0, 0, -1);
	SetMessageHandler(&AsScene1303Balloon::hmBalloonPopped);
}

AsScene1304Needle::AsScene1304Needle(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, 0x548E9411, surfacePriority, x, y), _parentScene(parentScene) {

	// NOTE: Skipped check if Klaymen already has the needle since that's done in the scene itself
	SetMessageHandler(&AsScene1304Needle::handleMessage);
}

uint32 AsScene1304Needle::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_KLAYMEN_USE_OBJECT:
		setGlobalVar(V_HAS_NEEDLE, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene1306Elevator::AsScene1306Elevator(NeverhoodEngine *vm, Scene *parentScene, AnimatedSprite *asElevatorDoor)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _asElevatorDoor(asElevatorDoor), _isUp(false), _isDown(true),
	_countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x043B0270, 100);
	startAnimation(0x043B0270, 0, -1);
	_newStickFrameIndex = 0;
	loadSound(0, 0x1C100E83);
	loadSound(1, 0x1C08CEC5);
	loadSound(2, 0x5D011E87);
	SetMessageHandler(&AsScene1306Elevator::handleMessage);
}

void AsScene1306Elevator::update() {
	if (_isUp && _countdown != 0 && (--_countdown == 0))
		stGoingDown();
	AnimatedSprite::update();
	if (_currFrameIndex == 7 && _asElevatorDoor->getVisible()) {
		playSound(1);
		_asElevatorDoor->setVisible(false);
	}
}

void AsScene1306Elevator::upGoingDown() {
	AnimatedSprite::update();
	if (_currFrameIndex == 5)
		_asElevatorDoor->setVisible(true);
}

uint32 AsScene1306Elevator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		if (_isUp)
			_countdown = 144;
		messageResult = _isUp ? 1 : 0;
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		if (_isDown)
			stGoingUp();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1306Elevator::stGoingUp() {
	setVisible(true);
	_isDown = false;
	startAnimation(0x043B0270, 0, -1);
	playSound(0);
	SetUpdateHandler(&AsScene1306Elevator::update);
	NextState(&AsScene1306Elevator::cbGoingUpEvent);
}

void AsScene1306Elevator::cbGoingUpEvent() {
	sendMessage(_parentScene, NM_KLAYMEN_OPEN_DOOR, 0);
	_isUp = true;
	_countdown = 144;
	stopAnimation();
	setVisible(false);
	SetUpdateHandler(&AsScene1306Elevator::update);
}

void AsScene1306Elevator::stGoingDown() {
	_isUp = false;
	setVisible(true);
	startAnimation(0x043B0270, -1, -1);
	_playBackwards = true;
	playSound(1);
	SetUpdateHandler(&AsScene1306Elevator::upGoingDown);
	NextState(&AsScene1306Elevator::cbGoingDownEvent);
}

void AsScene1306Elevator::cbGoingDownEvent() {
	_isDown = true;
	sendMessage(_parentScene, NM_KLAYMEN_CLOSE_DOOR, 0);
	stopAnimation();
	SetUpdateHandler(&AsScene1306Elevator::update);
}

static const uint32 kAsScene1307KeyResourceList1[] = {
	0x0438069C, 0x45B0023C, 0x05700217
};

static const uint32 kAsScene1307KeyResourceList2[] = {
	0x04441334, 0x061433F0, 0x06019390
};

static const uint32 kAsScene1307KeyResourceList3[] = {
	0x11A80030, 0x178812B1, 0x1488121C
};

static const uint32 *kAsScene1307KeyResourceLists[] = {
	kAsScene1307KeyResourceList1,
	kAsScene1307KeyResourceList2,
	kAsScene1307KeyResourceList3
};

static const int kAsScene1307KeySurfacePriorities[] = {
	700, 500, 300, 100
};

const uint kAsScene1307KeyPointsCount = 12;

static const NPoint kAsScene1307KeyPoints[] = {
	{-2,  0}, {-5,  0}, { 5,  0},
	{12,  0}, {17,  0}, {25,  0},
	{16, -2}, {10, -6}, { 0, -7},
	{-7, -3}, {-3,  4}, { 2,  2}
};

const uint kAsScene1307KeyFrameIndicesCount = 20;

static const int16 kAsScene1307KeyFrameIndices[] = {
	 1,  4,  8, 11, 15, 16, 17, 17, 17, 16,
	15, 14, 12, 10,  9,  7,  5,  3,  2,  1
};

const int kAsScene1307KeyDivValue = 200;
const int16 kAsScene1307KeyXDelta = 70;
const int16 kAsScene1307KeyYDelta = -12;

AsScene1307Key::AsScene1307Key(NeverhoodEngine *vm, Scene *parentScene, uint keyIndex, NRect *clipRects)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _keyIndex(keyIndex), _clipRects(clipRects),
	_isClickable(true) {

	NPoint pt;
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex];

	_dataResource.load(0x22102142);
	_pointList = _dataResource.getPointArray(0xAC849240);
	pt = (*_pointList)[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex)];
	_x = pt.x;
	_y = pt.y;
	createSurface(kAsScene1307KeySurfacePriorities[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex) % 4], 190, 148);
	startAnimation(fileHashes[0], 0, -1);
	loadSound(0, 0xDC4A1280);
	loadSound(1, 0xCC021233);
	loadSound(2, 0xC4C23844);
	loadSound(3, 0xC4523208);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1307Key::handleMessage);
}

uint32 AsScene1307Key::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_isClickable) {
			sendMessage(_parentScene, 0x4826, 0);
			stRemoveKey();
			messageResult = 1;
		}
		break;
	case NM_ANIMATION_UPDATE:
		_isClickable = param.asInteger() != 0;
		break;
	case 0x2001:
		setSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex, param.asInteger());
		stMoveKey();
		break;
	case 0x2003:
		playSound(3);
		stUnlock();
		break;
	case 0x2004:
		playSound(2);
		stInsert();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1307Key::suRemoveKey() {
	if (_pointIndex < kAsScene1307KeyPointsCount) {
		_x += kAsScene1307KeyPoints[_pointIndex].x;
		_y += kAsScene1307KeyPoints[_pointIndex].y;
		updateBounds();
		_pointIndex++;
	} else {
		SetSpriteUpdate(NULL);
	}
}

void AsScene1307Key::suInsertKey() {
	if (_pointIndex < kAsScene1307KeyPointsCount) {
		_x -= kAsScene1307KeyPoints[kAsScene1307KeyPointsCount - _pointIndex - 1].x;
		_y -= kAsScene1307KeyPoints[kAsScene1307KeyPointsCount - _pointIndex - 1].y;
		updateBounds();
		_pointIndex++;
		if (_pointIndex == 7)
			playSound(0);
	} else {
		SetSpriteUpdate(NULL);
		sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
	}
}

void AsScene1307Key::suMoveKey() {
	if (_pointIndex < kAsScene1307KeyFrameIndicesCount) {
		_frameIndex += kAsScene1307KeyFrameIndices[_pointIndex];
		_x = _prevX + (_deltaX * _frameIndex) / kAsScene1307KeyDivValue;
		_y = _prevY + (_deltaY * _frameIndex) / kAsScene1307KeyDivValue;
		updateBounds();
		_pointIndex++;
	} else {
		NPoint pt = (*_pointList)[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex)];
		_x = pt.x + kAsScene1307KeyXDelta;
		_y = pt.y + kAsScene1307KeyYDelta;
		stInsertKey();
	}
}

void AsScene1307Key::stRemoveKey() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex];
	_pointIndex = 0;
	startAnimation(fileHashes[0], 0, -1);
	playSound(1);
	SetSpriteUpdate(&AsScene1307Key::suRemoveKey);
}

void AsScene1307Key::stInsertKey() {
	_pointIndex = 0;
	sendMessage(_parentScene, NM_PRIORITY_CHANGE, kAsScene1307KeySurfacePriorities[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex) % 4]);
	setClipRect(_clipRects[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex) % 4]);
	_newStickFrameIndex = STICK_LAST_FRAME;
	SetSpriteUpdate(&AsScene1307Key::suInsertKey);
}

void AsScene1307Key::stMoveKey() {
	NPoint pt = (*_pointList)[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex)];
	int16 newX = pt.x + kAsScene1307KeyXDelta;
	int16 newY = pt.y + kAsScene1307KeyYDelta;
	sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1000);
	setClipRect(0, 0, 640, 480);
	_prevX = _x;
	_prevY = _y;
	if (newX == _x && newY == _y) {
		stInsertKey();
	} else {
		const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex];
		_pointIndex = 0;
		_frameIndex = 0;
		_deltaX = newX - _x;
		_deltaY = newY - _y;
		startAnimation(fileHashes[0], 0, -1);
		SetSpriteUpdate(&AsScene1307Key::suMoveKey);
	}
}

void AsScene1307Key::stUnlock() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex];
	startAnimation(fileHashes[1], 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

void AsScene1307Key::stInsert() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex];
	startAnimation(fileHashes[2], 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

AsScene1308JaggyDoor::AsScene1308JaggyDoor(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0xBA0AE050, 1100, 320, 240), _parentScene(parentScene) {

	setVisible(false);
	stopAnimation();
	SetMessageHandler(&AsScene1308JaggyDoor::handleMessage);
}

uint32 AsScene1308JaggyDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		stOpenDoor();
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		stCloseDoor();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1308JaggyDoor::stOpenDoor() {
	startAnimation(0xBA0AE050, 0, -1);
	setVisible(true);
	playSound(0, calcHash("fxDoorOpen38"));
	NextState(&AsScene1308JaggyDoor::stOpenDoorDone);
}

void AsScene1308JaggyDoor::stOpenDoorDone() {
	sendMessage(_parentScene, 0x2000, 0);
	stopAnimation();
	setVisible(false);
}

void AsScene1308JaggyDoor::stCloseDoor() {
	startAnimation(0xBA0AE050, -1, -1);
	_playBackwards = true;
	setVisible(true);
	playSound(0, calcHash("fxDoorClose38"));
	NextState(&AsScene1308JaggyDoor::stCloseDoorDone);
}

void AsScene1308JaggyDoor::stCloseDoorDone() {
	sendMessage(_parentScene, 0x2001, 0);
	stopAnimation();
}

AsScene1308KeyboardDoor::AsScene1308KeyboardDoor(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0xA08A0851, 1100, 320, 240), _parentScene(parentScene) {

	playSound(0, 0x51456049);
	SetMessageHandler(&AsScene1308KeyboardDoor::handleMessage);
	NextState(&AsScene1308KeyboardDoor::stFallingKeys);
}

uint32 AsScene1308KeyboardDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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

void AsScene1308KeyboardDoor::stFallingKeys() {
	startAnimation(0x6238B191, 0, -1);
	_x = 580;
	_y = 383;
	NextState(&AsScene1308KeyboardDoor::stFallingKeysDone);
}

void AsScene1308KeyboardDoor::stFallingKeysDone() {
	sendMessage(_parentScene, 0x2004, 0);
	stopAnimation();
	setVisible(false);
}

AsScene1308LightWallSymbols::AsScene1308LightWallSymbols(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0x80180A10, 100, 320, 240), _parentScene(parentScene) {

	setVisible(false);
	stopAnimation();
	Entity::_priority = 1200;
	SetMessageHandler(&AsScene1308LightWallSymbols::handleMessage);
}

uint32 AsScene1308LightWallSymbols::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_POSITION_CHANGE:
		stFadeIn();
		break;
	case 0x2003:
		stFadeOut();
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1308LightWallSymbols::stFadeIn() {
	startAnimation(0x80180A10, 0, -1);
	setVisible(true);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

void AsScene1308LightWallSymbols::stFadeOut() {
	startAnimation(0x80180A10, -1, -1);
	_playBackwards = true;
	NextState(&AsScene1308LightWallSymbols::stFadeOutDone);
}

void AsScene1308LightWallSymbols::stFadeOutDone() {
	sendMessage(_parentScene, 0x2003, 0);
	stopAnimation();
	setVisible(false);
}

SsScene1308Number::SsScene1308Number(NeverhoodEngine *vm, uint32 fileHash, int index)
	: StaticSprite(vm, fileHash, 100) {

	setVisible(false);
	_x = _spriteResource.getPosition().x + index * 20;
	updatePosition();
}

AsScene1308Mouse::AsScene1308Mouse(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	_x = 286;
	_y = 429;
	createSurface1(0xA282C472, 100);
	startAnimation(0xA282C472, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1308Mouse::handleMessage);
}

uint32 AsScene1308Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x66382026)
			playSound(0, 0x0CD84468);
		else if (param.asInteger() == 0x6E28061C)
			playSound(0, 0x78C8402C);
		else if (param.asInteger() == 0x462F0410)
			playSound(0, 0x60984E28);
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene1303::KmScene1303(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1303::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4804:
		GotoState(&Klaymen::stPeekWall1);
		break;
	case 0x483B:
		GotoState(&Klaymen::stPeekWallReturn);
		break;
	case 0x483C:
		GotoState(&Klaymen::stPeekWall2);
		break;
	default:
		break;
	}
	return 0;
}

KmScene1304::KmScene1304(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1304::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
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
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stTurnToUseHalf);
		else
			GotoState(&Klaymen::stWonderAbout);
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

KmScene1305::KmScene1305(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1305::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		GotoState(&KmScene1305::stCrashDown);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	default:
		break;
	}
	return 0;
}

void KmScene1305::stCrashDown() {
	playSound(0, 0x41648271);
	_busyStatus = 1;
	_acceptInput = false;
	startAnimationByHash(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	NextState(&KmScene1305::stCrashDownFinished);
}

void KmScene1305::stCrashDownFinished() {
	setDoDeltaX(2);
	stTryStandIdle();
}

KmScene1306::KmScene1306(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1306::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case NM_KLAYMEN_INSERT_DISK:
		GotoState(&Klaymen::stInsertDisk);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		else
			GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
		else
			GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStep);
		else
			GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToFront);
		else
			GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klaymen::stSitInTeleporter);
		break;
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klaymen::stGetUpFromTeleporter);
		break;
	case 0x483D:
		teleporterAppear(0xEE084A04);
		break;
	case 0x483E:
		teleporterDisappear(0xB86A4274);
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
	return messageResult;
}

KmScene1308::KmScene1308(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1308::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_MOVE_OBJECT:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
		break;
	case 0x480D:
		GotoState(&Klaymen::stUseLever);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case NM_KLAYMEN_INSERT_DISK:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stInsertKey);
		else
			GotoState(&Klaymen::stInsertDisk);
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
	case NM_KLAYMEN_RELEASE_LEVER:
		GotoState(&Klaymen::stReleaseLever);
		break;
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
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

} // End of namespace Neverhood
