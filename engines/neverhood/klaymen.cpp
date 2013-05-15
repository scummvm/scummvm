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

#include "neverhood/klaymen.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

static const KlaymenIdleTableItem klaymenIdleTable1[] = {
	{1, kIdlePickEar},
	{1, kIdleSpinHead},
	{1, kIdleArms},
	{1, kIdleChest},
	{1, kIdleHeadOff}
}; 

static const KlaymenIdleTableItem klaymenIdleTable2[] = {
	{1, kIdlePickEar},
	{1, kIdleSpinHead},
	{1, kIdleChest},
	{1, kIdleHeadOff}
}; 

static const KlaymenIdleTableItem klaymenIdleTable3[] = {
	{1, kIdleTeleporterHands},
	{1, kIdleTeleporterHands2}
}; 

static const KlaymenIdleTableItem klaymenIdleTable4[] = {
	{1, kIdleSpinHead},
	{1, kIdleChest},
	{1, kIdleHeadOff},
};

static const KlaymenIdleTableItem klaymenIdleTable1002[] = {
	{1, kIdlePickEar},
	{2, kIdleWonderAbout}
}; 

// Klaymen

Klaymen::Klaymen(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRectArray *clipRects)
	: AnimatedSprite(vm, 1000), _idleCounterMax(0), _idleCounter(0), _isMoveObjectRequested(false), _blinkCounterMax(0),
	_isWalkingOpenDoorNotified(false), _spitOutCountdown(0), _tapesToInsert(0), _keysToInsert(0), _busyStatus(0), _acceptInput(true),
	_attachedSprite(NULL), _isWalking(false), _actionStatus(1), _parentScene(parentScene), _isSneaking(false), _isLargeStep(false),
	_doYHitIncr(false), _isLeverDown(false), _isSittingInTeleporter(false), _actionStatusChanged(false), _ladderStatus(0), _pathPoints(NULL), _soundFlag(false),
	_idleTableNum(0), _otherSprite(NULL), _moveObjectCountdown(0), _readyToSpit(false), _walkResumeFrameIncr(0) {
	
	createSurface(1000, 320, 200);
	_x = x;
	_y = y;
	_destX = x;
	_destY = y;
	_flags = 2;
	setKlaymenIdleTable1();
	stTryStandIdle();
	SetUpdateHandler(&Klaymen::update);
}

void Klaymen::xUpdate() {
	// Empty
}

uint32 Klaymen::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	}
	return 0;
}

void Klaymen::update() {
	AnimatedSprite::update();
	xUpdate();
}

void Klaymen::setKlaymenIdleTable(const KlaymenIdleTableItem *table, uint tableCount) {
	_idleTable = table;
	_idleTableCount = tableCount;
	_idleTableTotalWeight = 0;
	for (uint i = 0; i < tableCount; i++)
		_idleTableTotalWeight += table[i].weight;
}

void Klaymen::setKlaymenIdleTable1() {
	setKlaymenIdleTable(klaymenIdleTable1, ARRAYSIZE(klaymenIdleTable1));
}

void Klaymen::setKlaymenIdleTable2() {
	setKlaymenIdleTable(klaymenIdleTable2, ARRAYSIZE(klaymenIdleTable2));
}

void Klaymen::setKlaymenIdleTable3() {
	setKlaymenIdleTable(klaymenIdleTable3, ARRAYSIZE(klaymenIdleTable3));
}

void Klaymen::stIdlePickEar() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x5B20C814, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmIdlePickEar);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
	FinalizeState(&Klaymen::evIdlePickEarDone);
}

uint32 Klaymen::hmIdlePickEar(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x04DBC02C) {
			playSound(0, 0x44528AA1);
		}
		break;
	}
	return messageResult;
}

void Klaymen::evIdlePickEarDone() {
	stopSound(0);
}

void Klaymen::stIdleSpinHead() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0xD122C137, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmIdleSpinHead);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
}

uint32 Klaymen::hmIdleSpinHead(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x808A0008) {
			playSound(0, 0xD948A340);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stIdleArms() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x543CD054, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmIdleArms);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
	FinalizeState(&Klaymen::evIdleArmsDone);
}

void Klaymen::evIdleArmsDone() {
	stopSound(0);
}

uint32 Klaymen::hmIdleArms(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x5A0F0104) {
			playSound(0, 0x7970A100);
		} else if (param.asInteger() == 0x9A9A0109) {
			playSound(0, 0xD170CF04);
		} else if (param.asInteger() == 0x989A2169) {
			playSound(0, 0xD073CF14);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stIdleChest() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x40A0C034, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmIdleChest);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
}

uint32 Klaymen::hmIdleChest(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0D2A0288) {
			playSound(0, 0xD192A368);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stIdleHeadOff() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x5120E137, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmIdleHeadOff);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
}

uint32 Klaymen::hmIdleHeadOff(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC006000C) {
			playSound(0, 0x9D406340);
		} else if (param.asInteger() == 0x2E4A2940) {
			playSound(0, 0x53A4A1D4);
		} else if (param.asInteger() == 0xAA0A0860) {
			playSound(0, 0x5BE0A3C6);
		} else if (param.asInteger() == 0xC0180260) {
			playSound(0, 0x5D418366);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stIdleWonderAbout() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
}

void Klaymen::stSitIdleTeleporter() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x582EC138, 0, -1);
	SetUpdateHandler(&Klaymen::upSitIdleTeleporter);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
	_idleCounter = 0;
	_blinkCounter = 0;
	_idleCounterMax = 8;
	_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

void Klaymen::upSitIdleTeleporter() {
	update();
	if (++_idleCounter >= _idleCounterMax) {
		_idleCounter = 0;
		if (_idleTable) {
			int idleWeight = _vm->_rnd->getRandomNumber(_idleTableTotalWeight - 1);
			for (uint i = 0; i < _idleTableCount; i++) {
				if (idleWeight < _idleTable[i].weight) {
					enterIdleAnimation(_idleTable[i].idleAnimation);
					_idleCounterMax = _vm->_rnd->getRandomNumber(128 - 1) + 24;
					break;
				}
				idleWeight -= _idleTable[i].weight;
			}
		}
	} else if (++_blinkCounter >= _blinkCounterMax) {
		_blinkCounter = 0;
		_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
		stSitIdleTeleporterBlink();
	}
}

void Klaymen::stSitIdleTeleporterBlink() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x5C24C018, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stSitIdleTeleporterBlinkSecond);
}

void Klaymen::stSitIdleTeleporterBlinkSecond() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x5C24C018, 0, -1);
	SetUpdateHandler(&Klaymen::upSitIdleTeleporter);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
}

void Klaymen::stPickUpNeedle() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klaymen::stPickUpNeedle))) {
		_busyStatus = 1;
		_acceptInput = false;
		startAnimation(0x1449C169, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPickUpObject);
		SetSpriteUpdate(NULL);
	}
}

void Klaymen::stPickUpTube() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klaymen::stPickUpTube))) {
		_busyStatus = 1;
		_acceptInput = false;
		startAnimation(0x0018C032, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPickUpTube);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klaymen::hmPickUpTube(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC1380080) {
			sendMessage(_attachedSprite, 0x4806, 0);
			playSound(0, 0xC8004340);
		} else if (param.asInteger() == 0x02B20220) {
			playSound(0, 0xC5408620);
		} else if (param.asInteger() == 0x03020231) {
			playSound(0, 0xD4C08010);
		} else if (param.asInteger() == 0x67221A03) {
			playSound(0, 0x44051000);
		} else if (param.asInteger() == 0x925A0C1E) {
			playSound(0, 0x40E5884D);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stTurnToUseInTeleporter() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0xD229823D, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stReturnFromUseInTeleporter() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x9A2801E0, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stStepOver() {
	if (!stStartAction(AnimationCallback(&Klaymen::stStepOver))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x004AA310, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmStartWalking);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stSitInTeleporter() {
	if (!stStartAction(AnimationCallback(&Klaymen::stSitInTeleporter))) {
		_busyStatus = 0;
		_acceptInput = false;
		startAnimation(0x392A0330, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmSitInTeleporter);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

uint32 Klaymen::hmSitInTeleporter(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x001A2832) {
			playSound(0, 0xC0E4884C);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stGetUpFromTeleporter() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x913AB120, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

/////////////////////////////////////////////////////////////////

void Klaymen::stopWalking() {
	_destX = _x;
	if (!_isWalking && !_isSneaking && !_isLargeStep) {
		gotoState(NULL);
		gotoNextStateExt();
	}
}

void Klaymen::startIdleAnimation(uint32 fileHash, AnimationCb callback) {
	debug("startIdleAnimation(%08X)", fileHash);
	NextState(callback);
	SetUpdateHandler(&Klaymen::upIdleAnimation);
}

void Klaymen::upIdleAnimation() {
	gotoNextStateExt();
	update();
}

bool Klaymen::stStartActionFromIdle(AnimationCb callback) {
	if (_busyStatus == 2) {
		_busyStatus = 1;
		_acceptInput = false; 
		startAnimation(0x9A7020B8, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmStartAction);
		SetSpriteUpdate(NULL);
		NextState(callback);
		return true;
	}
	return false;
}

void Klaymen::gotoNextStateExt() {
	if (_finalizeStateCb) {
		AnimationCb cb = _finalizeStateCb;
		_finalizeStateCb = NULL;
		(this->*cb)();
	}
	if (_nextStateCb) {
		AnimationCb cb = _nextStateCb;
		_nextStateCb = NULL;
		(this->*cb)();
	} else {
		// Inform the scene that the current Klaymen animation sequence has finished
		sendMessage(_parentScene, 0x1006, 0);
	}
}

void Klaymen::beginAction() {
	_actionStatusChanged = false;
	_actionStatus = 1;
}

void Klaymen::endAction() {
	if (_actionStatusChanged)
		_actionStatus = 0;
}

void Klaymen::stTryStandIdle() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stTryStandIdle))) {
		_busyStatus = 1;
		_acceptInput = true;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klaymen::upStandIdle);
		SetMessageHandler(&Klaymen::hmLowLevel);
		SetSpriteUpdate(NULL);
		_idleCounter = 0;
		_blinkCounter = 0;
		_blinkCounterMax = _vm->_rnd->getRandomNumber(64) + 24;
	}
}

void Klaymen::upStandIdle() {
	update();
	if (++_idleCounter >= 720) {
		_idleCounter = 0;
		if (_idleTable) {
			int idleWeight = _vm->_rnd->getRandomNumber(_idleTableTotalWeight - 1);
			for (uint i = 0; i < _idleTableCount; i++) {
				if (idleWeight < _idleTable[i].weight) {
					enterIdleAnimation(_idleTable[i].idleAnimation);
					break;
				}
				idleWeight -= _idleTable[i].weight;
			}
		}
	} else if (++_blinkCounter >= _blinkCounterMax) {
		_blinkCounter = 0;
		_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
		stIdleBlink();
	}
}

uint32 Klaymen::hmLowLevel(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	uint32 messageResult = xHandleMessage(messageNum, param);
	switch (messageNum) {
	case 0x1008:
		messageResult = _acceptInput;
		break;
	case 0x1014:
		_attachedSprite = (Sprite*)(param.asEntity());
		break;
	case 0x1019:
		gotoNextStateExt();
		break;
	case 0x101C:
		beginAction();
		break;
	case 0x1021:
		endAction();
		break;
	case 0x481C:
		_actionStatus = param.asInteger();
		_actionStatusChanged = true;
		messageResult = 1;
		break;
	case 0x482C:
		if (param.asInteger() != 0) {
			_pathPoints = _dataResource.getPointArray(param.asInteger());
		} else {
			_pathPoints = NULL;
		}
		break;
	}
	return messageResult;
}

void Klaymen::stIdleBlink() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x5900C41E, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stStandAround);
}

uint32 Klaymen::hmLowLevelAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextStateExt();
		break;
	}
	return messageResult;
}

void Klaymen::stStandAround() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x5420E254, 0, -1);
	SetUpdateHandler(&Klaymen::upStandIdle);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmStartAction(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x271AA210) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x2B22AA81) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}


void Klaymen::startWalkToX(int16 x, bool walkExt) {
	int16 xdiff = ABS(x - _x);
	if (x == _x) {
		_destX = x;
		if (!_isWalking && !_isSneaking && !_isLargeStep) {
			gotoState(NULL);
			gotoNextStateExt();
		}
	} else if (xdiff <= 36 && !_isWalking && !_isSneaking && !_isLargeStep) {
		_destX = x;
		gotoState(NULL);
		gotoNextStateExt();
	} else if (xdiff <= 42 && _actionStatus != 3) {
		if (_isSneaking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_destX - _x) > xdiff) {
			_destX = x;
		} else {
			_destX = x;
			GotoState(&Klaymen::stSneak);
		}
	} else if (_isWalking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_destX = x;
	} else if (walkExt) {
		_destX = x;
		GotoState(&Klaymen::stStartWalkingExt);
	} else {
		_destX = x;
		GotoState(&Klaymen::stStartWalking);
	}
}

void Klaymen::stWakeUp() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimation(0x527AC970, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stSleeping() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x5A38C110, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmSleeping);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmSleeping(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x03060012) {
			playSound(0, 0xC0238244);
		}
		break;
	}
	return messageResult;
}

bool Klaymen::stStartAction(AnimationCb callback3) {
	if (_busyStatus == 1) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x5C7080D4, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmStartAction);
		SetSpriteUpdate(&Klaymen::suAction);
		NextState(callback3);
		return true;
	} else {
		_x = _destX;
		return false;
	}
}

void Klaymen::suAction() {

	int16 xdiff = _destX - _x;

	if (_doDeltaX) {
		_x -= _deltaX;
	} else {
		_x += _deltaX;
	}
	_deltaX = 0;
	
	if (_doDeltaY) {
		_y -= _deltaY;
	} else {
		_y += _deltaY;
	}
	_deltaY = 0;
	
	if (_frameChanged) {
		if (xdiff > 6)
			_x += 6;
		else if (xdiff < -6)
			_x -= 6;
		else
			_x = _destX;				
	}
	
	updateBounds();
	
}

void Klaymen::suSneaking() {
	
	int16 xdiff = _destX - _x;
	
	if (_currFrameIndex == 9) {
		if (xdiff > 26)
			_deltaX += xdiff - 26;
		else if (xdiff < -26)			
			_deltaX -= xdiff + 26;
	}

	if (xdiff > _deltaX)
		xdiff = _deltaX;
	else if (xdiff < -_deltaX)
		xdiff = -_deltaX;
	_deltaX = 0;				

	if (_destX != _x) {
		HitRect *hitRectPrev = _parentScene->findHitRectAtPos(_x, _y);
		_x += xdiff;
		if (_pathPoints) {
			walkAlongPathPoints();
		} else {
			HitRect *hitRectNext = _parentScene->findHitRectAtPos(_x, _y);
			if (hitRectNext->type == 0x5002) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (hitRectNext->rect.x2 - _x) / 2);
			} else if (hitRectNext->type == 0x5003) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (_x - hitRectNext->rect.x1) / 2);
			} else if (hitRectPrev->type == 0x5002) {
				if (xdiff > 0) {
					_y = hitRectPrev->rect.y2;
				} else {
					_y = hitRectPrev->rect.y1;
				}
			} else if (hitRectPrev->type == 0x5003) {
				if (xdiff < 0) {
					_y = hitRectPrev->rect.y2;
				} else {
					_y = hitRectPrev->rect.y1;
				}
			}
		}
		updateBounds();
	}
	
}

void Klaymen::stSneak() {
	_busyStatus = 1;
	_isSneaking = true;
	_acceptInput = true;
	setDoDeltaX(_destX < _x ? 1 : 0);
	startAnimation(0x5C48C506, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmSneaking);
	SetSpriteUpdate(&Klaymen::suSneaking);
	FinalizeState(&Klaymen::evSneakingDone);	
}

void Klaymen::evSneakingDone() {
	_isSneaking = false;
}

uint32 Klaymen::hmSneaking(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		} else if (param.asInteger() == 0x32188010) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x02A2909C) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	case 0x3002:
		_x = _destX;
		gotoNextStateExt();
		break;
	}
	return messageResult;
}

void Klaymen::stStartWalking() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stStartWalking))) {
		_busyStatus = 0;
		_isWalking = true;
		_acceptInput = true;
		setDoDeltaX(_destX < _x ? 1 : 0);
		startAnimation(0x242C0198, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmStartWalking);
		SetSpriteUpdate(&Klaymen::suWalkingTestExit);
		NextState(&Klaymen::stWalkingFirst);
		FinalizeState(&Klaymen::evStartWalkingDone);	
	}
}

void Klaymen::evStartWalkingDone() {
	_isWalking = false;
}

uint32 Klaymen::hmStartWalking(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stWalkingFirst() {
	_busyStatus = 0;
	_isWalking = true;
	_acceptInput = true;
	startAnimation(0x1A249001, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalking);
	SetSpriteUpdate(&Klaymen::suWalkingFirst);
	NextState(&Klaymen::stUpdateWalkingFirst);
	FinalizeState(&Klaymen::evStartWalkingDone);	
}

void Klaymen::suWalkingFirst() {
	SetSpriteUpdate(&Klaymen::suWalkingTestExit);
	_deltaX = 0;
}

uint32 Klaymen::hmWalking(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stUpdateWalkingFirst() {
	if (_actionStatus == 2) {
		gotoNextStateExt();
	} else if (_actionStatus == 3) {
		stWalkingOpenDoor();
	} else {
		_isSneaking = true;
		_acceptInput = true;
		if (ABS(_destX - _x) <= 42 && _currFrameIndex >= 5 && _currFrameIndex <= 11) {
			if (_actionStatus == 0) {
				_busyStatus = 1;
				startAnimation(0xF234EE31, 0, -1);
			} else {
				_busyStatus = 2;
				startAnimation(0xF135CC21, 0, -1);
			}
		} else if (ABS(_destX - _x) <= 10 && (_currFrameIndex >= 12 || _currFrameIndex <= 4)) {
			if (_actionStatus == 0) {
				_busyStatus = 1;
				startAnimation(0x8604A152, 0, -1);
			} else {
				_busyStatus = 2;
				startAnimation(0xA246A132, 0, -1);
			}
		}
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmSneaking);
		SetSpriteUpdate(&Klaymen::suSneaking);
		FinalizeState(&Klaymen::evSneakingDone);	
	}
}

void Klaymen::suWalkingTestExit() {
	int16 xdiff = ABS(_destX - _x);
	int16 xdelta = _destX - _x;
	
	if (xdelta > _deltaX)
		xdelta = _deltaX;
	else if (xdelta < -_deltaX)
		xdelta = -_deltaX;
		
	_deltaX = 0;		

	if (xdiff == 0 ||
		(_actionStatus != 2 && _actionStatus != 3 && xdiff <= 42 && _currFrameIndex >= 5 && _currFrameIndex <= 11) ||
		(_actionStatus != 2 && _actionStatus != 3 && xdiff <= 10 && (_currFrameIndex >= 12 || _currFrameIndex <= 4)) ||
		(_actionStatus == 3 && xdiff < 30) ||
		(_actionStatus == 3 && xdiff < 150 && _currFrameIndex >= 6)) {
		sendMessage(this, 0x1019, 0);
	} else {
		HitRect *hitRectPrev = _parentScene->findHitRectAtPos(_x, _y);
		_x += xdelta;
		if (_pathPoints) {
			walkAlongPathPoints();
		} else {
			HitRect *hitRectNext = _parentScene->findHitRectAtPos(_x, _y);
			if (hitRectNext->type == 0x5002) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (hitRectNext->rect.x2 - _x) / 2);
			} else if (hitRectNext->type == 0x5003) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (_x - hitRectNext->rect.x1) / 2);
			} else if (hitRectPrev->type == 0x5002) {
				_y = xdelta > 0 ? hitRectPrev->rect.y2 : hitRectPrev->rect.y1;
			} else if (hitRectPrev->type == 0x5003) {
				_y = xdelta < 0 ? hitRectPrev->rect.y2 : hitRectPrev->rect.y1;
			} else if (_doYHitIncr && xdelta != 0) {
				if (hitRectNext->type == 0x5000) {
					_y++;
				} else if (hitRectNext->type == 0x5001 && _y > hitRectNext->rect.y1) {
					_y--;
				}
			}
		}
		updateBounds();
	}
	
}

uint32 Klaymen::hmLever(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stPickUpGeneric() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klaymen::stPickUpGeneric))) {
		_busyStatus = 1;
		_acceptInput = false;
		startAnimation(0x1C28C178, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPickUpObject);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klaymen::hmPickUpObject(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC1380080) {
			sendMessage(_attachedSprite, 0x4806, 0);
			playSound(0, 0x40208200);
		} else if (param.asInteger() == 0x02B20220) {
			playSound(0, 0xC5408620);
		} else if (param.asInteger() == 0x03020231) {
			playSound(0, 0xD4C08010);
		} else if (param.asInteger() == 0x67221A03) {
			playSound(0, 0x44051000);
		} else if (param.asInteger() == 0x2EAE0303) {
			playSound(0, 0x03630300);
		} else if (param.asInteger() == 0x61CE4467) {
			playSound(0, 0x03630300);
		}
		break;
	}
	return messageResult;

}

void Klaymen::stPressButton() {
	if (!stStartAction(AnimationCallback(&Klaymen::stPressButton))) {
		_busyStatus = 2;
		_acceptInput = true;
		startAnimation(0x1C02B03D, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPressButton);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klaymen::hmPressButton(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0D01B294) {
			sendMessage(_attachedSprite, 0x480B, 0);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stPressFloorButton() {
	if (!stStartAction(AnimationCallback(&Klaymen::stPressFloorButton))) {
		_busyStatus = 2;
		_acceptInput = true;
		startAnimation(0x1C16B033, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPressButton);
		SetSpriteUpdate(NULL);
	}
}

void Klaymen::stPressButtonSide() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stPressButtonSide))) {
		_busyStatus = 1;
		_acceptInput = true;
		startAnimation(0x1CD89029, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPressButton);
		SetSpriteUpdate(&Klaymen::suAction);
	}
}

void Klaymen::startSpecialWalkRight(int16 x) {
	if (_x == x) {
		_destX = x;
		gotoState(NULL);
		gotoNextStateExt();
	} else if (_x < x) {
		startWalkToX(x, false);
	} else if (_x - x <= 105) {
		startWalkToXExt(x);
	} else {
		startWalkToX(x, false);
	}
}

void Klaymen::startSpecialWalkLeft(int16 x) {
	if (x == _x) {
		_destX = x;
		gotoState(NULL);
		gotoNextStateExt();
	} else if (x < _x) {
		startWalkToX(x, false);
	} else if (x - _x <= 105) {
		startWalkToXExt(x);
	} else {
		startWalkToX(x, false);
	}
}

void Klaymen::startWalkToXSmall(int16 x) {
	_actionStatus = 2;
	if (_x == x) {
		_destX = x;
		if (_isWalking) {
			GotoState(NULL);
			gotoNextStateExt();
		}
	} else if (_isWalking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_destX = x;
	} else {
		_destX = x;
		GotoState(&Klaymen::stStartWalkingSmall);
	}
}

void Klaymen::stStartWalkingSmall() {
	_isWalking = true;
	_acceptInput = true;
	_actionStatus = 2;
	setDoDeltaX(_destX < _x ? 1 : 0);
	startAnimation(0x3A4CD934, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkingSmall);
	SetSpriteUpdate(&Klaymen::suWalkingTestExit);
	FinalizeState(&Klaymen::evStartWalkingDone);
}

uint32 Klaymen::hmWalkingSmall(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101)
			playSound(0, 0x4924AAC4);
		else if (param.asInteger() == 0x0A2A9098)
			playSound(0, 0x0A2AA8E0);
	}
	return messageResult;
}

void Klaymen::stStandIdleSmall() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x90D0D1D0, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
}

void Klaymen::stWonderAboutAfterSmall() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x11C8D156, 30, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stWonderAboutHalfSmall() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x11C8D156, 0, 10);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stWonderAboutSmall() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x11C8D156, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stWalkToFrontNoStepSmall() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimationByHash(0x3F9CC394, 0x14884392, 0);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

uint32 Klaymen::hmWalkFrontBackSmall(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x80C110B5)
			sendMessage(_parentScene, 0x482A, 0);
		else if (param.asInteger() == 0x110010D1)
			sendMessage(_parentScene, 0x482B, 0);
		else if (param.asInteger() == 0x32180101)
			playSound(0, 0x4924AAC4);
		else if (param.asInteger() == 0x0A2A9098)
			playSound(0, 0x0A2AA8E0);
		break;
	}
	return messageResult;
}

void Klaymen::stWalkToFront2Small() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x2F1C4694, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stWalkToFrontSmall() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x3F9CC394, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stTurnToBackHalfSmall() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimationByHash(0x37ECD436, 0, 0x8520108C);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stTurnToBackWalkSmall() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x16EDDE36, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stTurnToBackSmall() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x37ECD436, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stPullCord() {
	if (!stStartAction(AnimationCallback(&Klaymen::stPullCord))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x3F28E094, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPullReleaseCord);
		SetSpriteUpdate(NULL);
		NextState(&Klaymen::stReleaseCord);
	}
}

void Klaymen::stReleaseCord() {
	_acceptInput = false;
	startAnimation(0x3A28C094, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmPullReleaseCord);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmPullReleaseCord(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
			sendMessage(_attachedSprite, 0x480F, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stUseTube() {
	if (!stStartAction(AnimationCallback(&Klaymen::stUseTube))) {
		_busyStatus = 1;
		_acceptInput = false;
		startAnimation(0x1A38A814, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmUseTube);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klaymen::hmUseTube(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02B20220)
			playSound(0, 0xC5408620);
		else if (param.asInteger() == 0x0A720138)
			playSound(0, 0xD4C08010);
		else if (param.asInteger() == 0x03020231)
			playSound(0, 0xD4C08010);
		else if (param.asInteger() == 0xB613A180)
			playSound(0, 0x44051000);
		else if (param.asInteger() == 0x67221A03)
			playSound(0, 0x44051000);
		else if (param.asInteger() == 0x038A010B)
			playSound(0, 0x00018040);
		else if (param.asInteger() == 0x422B0280)
			playSound(0, 0x166FC6E0);
		else if (param.asInteger() == 0x925A0C1E)
			playSound(0, 0x40E5884D);
		break;
	}
	return messageResult;
}

void Klaymen::stWalkingFirstExt() {
	_busyStatus = 0;
	_isWalking = true;
	_acceptInput = true;
	startAnimation(0x5A2CBC00, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalking);
	SetSpriteUpdate(&Klaymen::suWalkingFirst);
	NextState(&Klaymen::stUpdateWalkingFirst);
	FinalizeState(&Klaymen::evStartWalkingDone);
}

void Klaymen::stStartWalkingExt() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stStartWalkingExt))) {
		_busyStatus = 0;
		_isWalking = true;
		_acceptInput = true;
		setDoDeltaX(_destX < _x ? 1 : 0);
		startAnimation(0x272C1199, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmStartWalking);
		SetSpriteUpdate(&Klaymen::suWalkingTestExit);
		NextState(&Klaymen::stWalkingFirstExt);
		FinalizeState(&Klaymen::evStartWalkingDone);
	}
}

void Klaymen::startWalkToXDistance(int16 destX, int16 distance) {
	if (_x > destX) {
		if (_x == destX + distance) {
			_destX = destX + distance;
			gotoState(NULL);
			gotoNextStateExt();
		} else if (_x < destX + distance) {
			startWalkToXExt(destX + distance);
		} else {
			startWalkToX(destX + distance, false);
		}
	} else {
		if (_x == destX - distance) {
			_destX = destX - distance;
			gotoState(NULL);
			gotoNextStateExt();
		} else if (_x > destX - distance) {
			startWalkToXExt(destX - distance);
		} else {
			startWalkToX(destX - distance, false);
		}
	}
}

void Klaymen::startWalkToXExt(int16 x) {
	int16 xdiff = ABS(x - _x);
	if (x == _x) {
		_destX = x;
		if (!_isWalking && !_isSneaking && !_isLargeStep) {
			gotoState(NULL);
			gotoNextStateExt();
		}
	} else if (xdiff <= 36 && !_isWalking && !_isSneaking && !_isLargeStep) {
		_destX = x;
		gotoState(NULL);
		gotoNextStateExt();
	} else if (xdiff <= 42 && _actionStatus != 3) {
		if (_isSneaking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_destX - _x) > xdiff) {
			_destX = x;
		} else {
			_destX = x;
			GotoState(&Klaymen::stSneak);
		}
	} else if (_isLargeStep && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_destX = x;
	} else {
		_destX = x;
		GotoState(&Klaymen::stLargeStep);
	}
}

void Klaymen::stLargeStep() {
	_busyStatus = 2;
	_isLargeStep = true;
	_acceptInput = true;
	setDoDeltaX(_destX >= _x ? 1 : 0);
	startAnimation(0x08B28116, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLargeStep);
	SetSpriteUpdate(&Klaymen::suLargeStep);
	FinalizeState(&Klaymen::evLargeStepDone);	
}

void Klaymen::evLargeStepDone() {
	_isLargeStep = false;
}

void Klaymen::suLargeStep() {
	int16 xdiff = _destX - _x;
	
	if (_doDeltaX) {
		_deltaX = -_deltaX;
	}
	
	if (_currFrameIndex == 7) {
		_deltaX = xdiff;
	}

	if ((xdiff > 0 && xdiff > _deltaX) || (xdiff < 0 && xdiff < _deltaX))
		xdiff = _deltaX;

	_deltaX = 0;
	
	if (_x != _destX) {
		HitRect *hitRectPrev = _parentScene->findHitRectAtPos(_x, _y);
		_x += xdiff;
		if (_pathPoints) {
			walkAlongPathPoints();
		} else {
			HitRect *hitRectNext = _parentScene->findHitRectAtPos(_x, _y);
			if (hitRectNext->type == 0x5002) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (hitRectNext->rect.x2 - _x) / 2);
			} else if (hitRectNext->type == 0x5003) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (_x - hitRectNext->rect.x1) / 2);
			} else if (hitRectPrev->type == 0x5002) {
				_y = xdiff > 0 ? hitRectPrev->rect.y2 : hitRectPrev->rect.y1;
			} else if (hitRectPrev->type == 0x5003) {
				_y = xdiff < 0 ? hitRectPrev->rect.y2 : hitRectPrev->rect.y1;
			}
		}
		updateBounds();
	}
}

uint32 Klaymen::hmLargeStep(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	case 0x3002:
		_x = _destX;
		gotoNextStateExt();
		break;		
	}
	return messageResult;
}

void Klaymen::stWonderAboutHalf() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, 10);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stWonderAboutAfter() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0xD820A114, 30, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stTurnToUseHalf() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x9B250AD2, 0, 7);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmTurnToUse);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmTurnToUse(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stTurnAwayFromUse() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x98F88391, 4, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmTurnToUse);
	SetSpriteUpdate(NULL);
}

void Klaymen::stWonderAbout() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stPeekWall() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0xAC20C012, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmPeekWall);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmPeekWall(int messageNum, const MessageParam &param, Entity *sender) {
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymenSpeedUpHash);
		if (_currFrameIndex < speedUpFrameIndex)
			startAnimation(0xAC20C012, speedUpFrameIndex, -1);
		return 0;
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0460E2FA);
		}
		break;
	}
	return hmLowLevelAnimation(messageNum, param, sender);
}

void Klaymen::stJumpToRing1() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpToRing1))) {
		_busyStatus = 0;
		startAnimation(0xD82890BA, 0, -1);
		setupJumpToRing();
	}
}

void Klaymen::setupJumpToRing() {
	_acceptInput = false;
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmJumpToRing);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
	NextState(&Klaymen::stHangOnRing);
	sendMessage(_attachedSprite, 0x482B, 0);
}

uint32 Klaymen::hmJumpToRing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0) {
			sendMessage(_attachedSprite, 0x4806, 0);
			_acceptInput = true;
		} else if (param.asInteger() == 0x320AC306) {
			playSound(0, 0x5860C640);
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::suUpdateDestX() {
	AnimatedSprite::updateDeltaXY();
	_destX = _x;
}

void Klaymen::stHangOnRing() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x4829E0B8, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
}

void Klaymen::stJumpToRing2() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpToRing2))) {
		_busyStatus = 0;
		startAnimation(0x900980B2, 0, -1);
		setupJumpToRing();
	}
}

void Klaymen::stJumpToRing3() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpToRing3))) {
		_busyStatus = 0;
		_acceptInput = false;
		startAnimation(0xBA1910B2, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
		SetMessageHandler(&Klaymen::hmJumpToRing3);
		NextState(&Klaymen::stHoldRing3);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

uint32 Klaymen::hmJumpToRing3(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0) {
			sendMessage(_attachedSprite, 0x4806, 0);
		} else if (param.asInteger() == 0x320AC306) {
			playSound(0, 0x5860C640);
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stHoldRing3() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x4A293FB0, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmHoldRing3);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmHoldRing3(int messageNum, const MessageParam &param, Entity *sender) {
	if (messageNum == 0x1008) {
		stReleaseRing();
		return 0;
	}
	return hmLowLevel(messageNum, param, sender);
}

void Klaymen::stReleaseRing() {
	_busyStatus = 1;
	_acceptInput = false;
	sendMessage(_attachedSprite, 0x4807, 0);
	_attachedSprite = NULL;
	startAnimation(0xB869A4B9, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stJumpToRing4() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpToRing4))) {
		_busyStatus = 0;
		startAnimation(0xB8699832, 0, -1);
		setupJumpToRing();
	}
}

void Klaymen::startWalkToAttachedSpriteXDistance(int16 distance) {
	startWalkToXDistance(_attachedSprite->getX(), distance);
}

void Klaymen::stContinueClimbLadderUp() {
	_busyStatus = 0;
	_acceptInput = true;
	_ladderStatus = 3;
	startAnimationByHash(0x3A292504, 0x01084280, 0);
	_newStickFrameHash = 0x01084280;
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
	gotoNextStateExt();
}

void Klaymen::stStartClimbLadderDown() {
	if (!stStartAction(AnimationCallback(&Klaymen::stStartClimbLadderDown))) {
		_busyStatus = 0;
		if (_destY < _y) {
			if (_ladderStatus == 1) {
				_ladderStatus = 2;
				stClimbLadderHalf();
			} else {
				gotoNextStateExt();
			}
		} else if (_ladderStatus == 0) {
			_ladderStatus = 2;
			_acceptInput = false;
			startAnimation(0x122D1505, 0, -1);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klaymen::suUpdateDestX);
		} else if (_ladderStatus == 3) {
			_ladderStatus = 2;
			_acceptInput = true;
			startAnimationByHash(0x122D1505, 0x01084280, 0);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klaymen::suUpdateDestX);
		} else if (_ladderStatus == 1) {
			_ladderStatus = 2;
			_acceptInput = true;
			startAnimation(0x122D1505, 29 - _currFrameIndex, -1);
		} 
	}
}

void Klaymen::stClimbLadderHalf() {
	_busyStatus = 2;
	if (_ladderStatus == 1) {
		_ladderStatus = 0;
		_acceptInput = false;
		startAnimationByHash(0x3A292504, 0x02421405, 0);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmClimbLadderHalf);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	} else if (_ladderStatus == 2) {
		_ladderStatus = 0;
		_acceptInput = false;
		startAnimationByHash(0x122D1505, 0x02421405, 0);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmClimbLadderHalf);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	} else {
		gotoNextStateExt();
	}
}

uint32 Klaymen::hmClimbLadderHalf(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x489B025C) {
			playSound(0, 0x52C4C2D7);
		} else if (param.asInteger() == 0x400A0E64) {
			playSound(0, 0x50E081D9);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmClimbLadderUpDown(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			_acceptInput = true;
		} else if (param.asInteger() == 0x489B025C) {
			playSound(0, 0x52C4C2D7);
		} else if (param.asInteger() == 0x400A0E64) {
			playSound(0, 0x50E081D9);
		} else if (param.asInteger() == 0x02421405) {
			if (_ladderStatus == 1) {
				startAnimationByHash(0x3A292504, 0x01084280, 0);
				if (_destY >= _y - 30)
					sendMessage(this, 0x1019, 0);
			} else {
				startAnimationByHash(0x122D1505, 0x01084280, 0);
				if (_destY <= _y)
					sendMessage(this, 0x1019, 0);
			}
		}
		break;
	}
	return messageResult;
}

void Klaymen::stStartClimbLadderUp() {
	if (!stStartAction(AnimationCallback(&Klaymen::stStartClimbLadderUp))) {
		_busyStatus = 0;
		if (_destY >= _y - 30) {
			gotoNextStateExt();
		} else if (_ladderStatus == 0) {
			_ladderStatus = 1;
			_acceptInput = false;
			startAnimation(0x3A292504, 0, -1);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klaymen::suUpdateDestX);
		} else if (_ladderStatus == 3) {
			_ladderStatus = 1;
			_acceptInput = true;
			startAnimationByHash(0x3A292504, 0x01084280, 0);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klaymen::suUpdateDestX);
		} else if (_ladderStatus == 2) {
			_ladderStatus = 1;
			_acceptInput = true;
			startAnimation(0x3A292504, 29 - _currFrameIndex, -1);
		}
	}
}

void Klaymen::stWalkToFrontNoStep() {
	_busyStatus = 2;
	_acceptInput = false;
	startAnimationByHash(0xF229C003, 0x14884392, 0);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalkToFront);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

uint32 Klaymen::hmWalkToFront(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x80C110B5) {
			sendMessage(_parentScene, 0x482A, 0);
		} else if (param.asInteger() == 0x110010D1) {
			sendMessage(_parentScene, 0x482B, 0);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stWalkToFront() {
	if (!stStartAction(AnimationCallback(&Klaymen::stWalkToFront))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0xF229C003, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmWalkToFront);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stTurnToFront() {
	if (!stStartAction(AnimationCallback(&Klaymen::stTurnToFront))) {
		_busyStatus = 0;
		_acceptInput = false;
		startAnimationByHash(0xCA221107, 0, 0x8520108C);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmWalkToFront);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stTurnToBack() {
	if (!stStartAction(AnimationCallback(&Klaymen::stTurnToBack))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0xCA221107, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmWalkToFront);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stLandOnFeet() {
	_busyStatus = 1;
	_acceptInput = true;
	startAnimation(0x18118554, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLandOnFeet);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmLandOnFeet(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x320AC306) {
			playSound(0, 0x5860C640);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stTurnToBackToUse() {
	if (!stStartAction(AnimationCallback(&Klaymen::stTurnToBackToUse))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x91540140, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmTurnToBackToUse);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

uint32 Klaymen::hmTurnToBackToUse(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC61A0119) {
			playSound(0, 0x402338C2);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stClayDoorOpen() {
	if (!stStartAction(AnimationCallback(&Klaymen::stClayDoorOpen))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x5CCCB330, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmClayDoorOpen);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

uint32 Klaymen::hmClayDoorOpen(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x040D4186) {
			sendMessage(_attachedSprite, 0x4808, 0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stTurnToUse() {
	if (!stStartAction(AnimationCallback(&Klaymen::stTurnToUse))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x9B250AD2, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmTurnToUse);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stReturnFromUse() {
	_busyStatus = 2;
	_acceptInput = false;
	startAnimation(0x98F88391, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmTurnToUse);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stWalkingOpenDoor() {
	_isWalkingOpenDoorNotified = false;
	_acceptInput = false;
	startAnimation(0x11A8E012, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmStartWalking);
	SetSpriteUpdate(&Klaymen::suWalkingOpenDoor);
}

void Klaymen::suWalkingOpenDoor() {
	if (!_isWalkingOpenDoorNotified && ABS(_destX - _x) < 80) {
		sendMessage(_parentScene, 0x4829, 0);
		_isWalkingOpenDoorNotified = true;
	}
	AnimatedSprite::updateDeltaXY();
}

void Klaymen::stMoveObjectSkipTurnFaceObject() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	_isMoveObjectRequested = false;
	_acceptInput = true;
	startAnimationByHash(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
	SetMessageHandler(&Klaymen::hmMoveObjectTurn);
}

void Klaymen::evMoveObjectTurnDone() {
	sendMessage(_attachedSprite, 0x4807, 0);
}

uint32 Klaymen::hmMoveObjectTurn(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			sendMessage(_attachedSprite, 0x480B, _doDeltaX ? 1 : 0);
		} else if (param.asInteger() == 0x02421405) {
			if (_isMoveObjectRequested && sendMessage(_attachedSprite, 0x480C, _doDeltaX ? 1 : 0) != 0) {
				stMoveObjectSkipTurn();
			} else {
				FinalizeState(&Klaymen::evMoveObjectTurnDone);
				SetMessageHandler(&Klaymen::hmLowLevelAnimation);
			}
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0460E2FA);
		}
		break;
	case 0x480A:
		_isMoveObjectRequested = true;
		return 0;		
	}
	return hmLowLevelAnimation(messageNum, param, sender);
}

void Klaymen::stMoveObjectSkipTurn() {
	_isMoveObjectRequested = false;
	_acceptInput = true;
	startAnimationByHash(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
	SetMessageHandler(&Klaymen::hmMoveObjectTurn);
}

void Klaymen::stMoveObjectFaceObject() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klaymen::stMoveObjectFaceObject))) {
		_busyStatus = 2;
		_isMoveObjectRequested = false;
		_acceptInput = true;
		startAnimation(0x0C1CA072, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmMoveObjectTurn);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stUseLever() {
	if (!stStartAction(AnimationCallback(&Klaymen::stUseLever))) {
		_busyStatus = 0;
		if (_isLeverDown) {
			stUseLeverRelease();
		} else {
			sendMessage(_attachedSprite, 0x482B, 0);
			startAnimation(0x0C303040, 0, -1);
			SetSpriteUpdate(&Klaymen::suUpdateDestX);
			SetMessageHandler(&Klaymen::hmLever);
			SetUpdateHandler(&Klaymen::update);
			NextState(&Klaymen::stPullLeverDown);
			_acceptInput = false;
		}
	}
}

// Exactly the same code as sub420DA0 which was removed
void Klaymen::stPullLeverDown() {
	startAnimation(0x0D318140, 0, -1);
	sendMessage(_attachedSprite, 0x480F, 0);
	NextState(&Klaymen::stHoldLeverDown);
}

void Klaymen::stHoldLeverDown() {
	startAnimation(0x4464A440, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
	_isLeverDown = true;
	_acceptInput = true;
}

void Klaymen::stUseLeverRelease() {
	startAnimation(0x09018068, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLever);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
	sendMessage(_attachedSprite, 0x4807, 0);
	NextState(&Klaymen::stPullLeverDown);
	_acceptInput = false;
}

void Klaymen::stReleaseLever() {
	if (_isLeverDown) {
		_busyStatus = 2;
		startAnimation(0x09018068, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmLever);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
		sendMessage(_attachedSprite, 0x4807, 0);
		NextState(&Klaymen::stLetGoOfLever);
		_acceptInput = false;
		_isLeverDown = false;
	} else {
		gotoNextStateExt();
	}
}

void Klaymen::stLetGoOfLever() {
	startAnimation(0x0928C048, 0, -1);
	FinalizeState(&Klaymen::evLeverReleasedEvent);
}

void Klaymen::evLeverReleasedEvent() {
	sendMessage(_attachedSprite, 0x482A, 0);
}

void Klaymen::stInsertDisk() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stInsertDisk))) {
		_busyStatus = 2;
		_tapesToInsert = 0;
		for (uint32 i = 0; i < 20; i++) {
			if (getSubVar(VA_HAS_TAPE, i)) {
				setSubVar(VA_IS_TAPE_INSERTED, i, 1);
				setSubVar(VA_HAS_TAPE, i, 0);
				_tapesToInsert++;
			}
		}
		if (_tapesToInsert == 0) {
			GotoState(NULL);
			gotoNextStateExt();
		} else {
			startAnimation(0xD8C8D100, 0, -1);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmInsertDisk);
			SetSpriteUpdate(&Klaymen::suAction);
			_acceptInput = false;
			_tapesToInsert--;
		}
	}
}

uint32 Klaymen::hmInsertDisk(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (_tapesToInsert == 0 && param.asInteger() == 0x06040580) {
			nextAnimationByHash(0xD8C8D100, calcHash("GoToStartLoop/Finish"), 0);
		} else if (_tapesToInsert != 0 && param.asInteger() == calcHash("GoToStartLoop/Finish")) {
			_tapesToInsert--;
			startAnimationByHash(0xD8C8D100, 0x01084280, 0);
		} else if (param.asInteger() == 0x062A1510) {
			playSound(0, 0x41688704);
		} else if (param.asInteger() == 0x02B20220) {
			playSound(0, 0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			playSound(0, 0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			playSound(0, 0x44051000);
		} else if (param.asInteger() == 0x0E040501) {
			playSound(1, 0xC6A129C1);
		}
	}
	return messageResult;
}

void Klaymen::walkAlongPathPoints() {
	if (_x <= (*_pathPoints)[0].x)
		_y = (*_pathPoints)[0].y;
	else if (_x >= (*_pathPoints)[_pathPoints->size() - 1].x)
		_y = (*_pathPoints)[_pathPoints->size() - 1].y;
	else {
		int16 deltaX = _x - (*_pathPoints)[0].x, deltaXIncr = 0;
		uint index = 0;
		while (deltaX > 0) {
			NPoint pt2 = (*_pathPoints)[index];
			NPoint pt1 = index + 1 >= _pathPoints->size() ? (*_pathPoints)[0] : (*_pathPoints)[index + 1];
			int16 xd = ABS(pt1.x - pt2.x);
			int16 yd = ABS(pt1.y - pt2.y);
			if (deltaX + deltaXIncr >= xd) {
				deltaX -= xd;
				deltaX += deltaXIncr;
				++index;
				if (index >= _pathPoints->size())
					index = 0;
				_y = (*_pathPoints)[index].y;
			} else {
				deltaXIncr += deltaX;
				if (pt1.y >= pt2.y) {
					_y = pt2.y + (yd * deltaXIncr) / xd;
				} else {
					_y = pt2.y - (yd * deltaXIncr) / xd;
				}
				deltaX = 0;
			}
		}
	}
}

void Klaymen::enterIdleAnimation(uint idleAnimation) {
	switch (idleAnimation) {
	case kIdlePickEar:
		startIdleAnimation(0x5B20C814, AnimationCallback(&Klaymen::stIdlePickEar));
		break;
	case kIdleSpinHead:
		startIdleAnimation(0xD122C137, AnimationCallback(&Klaymen::stIdleSpinHead));
		break;
	case kIdleArms:
		startIdleAnimation(0x543CD054, AnimationCallback(&Klaymen::stIdleArms));
		break;
	case kIdleChest:
		startIdleAnimation(0x40A0C034, AnimationCallback(&Klaymen::stIdleChest));
		break;
	case kIdleHeadOff:
		startIdleAnimation(0x5120E137, AnimationCallback(&Klaymen::stIdleHeadOff));
		break;
	case kIdleTeleporterHands:
		startIdleAnimation(0x90EF8D38, AnimationCallback(&Klaymen::stIdleTeleporterHands));
		break;
	case kIdleTeleporterHands2:
		startIdleAnimation(0x900F0930, AnimationCallback(&Klaymen::stIdleTeleporterHands2));
		break;
	case kIdleWonderAbout:
		stIdleWonderAbout();
		break;
	}
}

void Klaymen::stJumpToGrab() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimationByHash(0x00AB8C10, 0x01084280, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(&Klaymen::suJumpToGrab);
	SetMessageHandler(&Klaymen::hmJumpToGrab);
}

void Klaymen::suJumpToGrab() {
	updateDeltaXY();
	if (_y >= _destY) {
		_y = _destY;
		updateBounds();
		gotoNextStateExt();
	}
}

uint32 Klaymen::hmJumpToGrab(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0)
			sendMessage(_attachedSprite, 0x4806, 0);
		else if (param.asInteger() == 0x320AC306)
			startAnimationByHash(0x00AB8C10, 0x01084280, 0);
		else if (param.asInteger() == 0x4AB28209)
			sendMessage(_attachedSprite, 0x482A, 0);
		else if (param.asInteger() == 0x88001184)
			sendMessage(_attachedSprite, 0x482B, 0);
		break;
	}
	return messageResult;
}

void Klaymen::stFinishGrow() {
	_busyStatus = 2;
	_acceptInput = false;
	startAnimation(0x38445000, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klaymen::hmFinishGrow);
}

uint32 Klaymen::hmFinishGrow(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x040C4C01)
			playSound(0, 0x01E11140);
		break;
	}
	return messageResult;
}

void Klaymen::stTurnToUseExt() {
	if (!stStartAction(AnimationCallback(&Klaymen::stTurnToUseExt))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x1B3D8216, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmTurnToUse);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

void Klaymen::stJumpToGrabFall() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpToGrabFall))) {
		_busyStatus = 0;
		_acceptInput = false;
		startAnimation(0x00AB8C10, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmJumpToGrab);
		SetSpriteUpdate(&Klaymen::suJumpToGrab);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

void Klaymen::stJumpToGrabRelease() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimationByHash(0x00AB8C10, 0x320AC306, 0);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmJumpToGrabRelease);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stReleaseRing);
}

uint32 Klaymen::hmJumpToGrabRelease(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x320AC306)
			playSound(0, 0x5860C640);
		break;
	}
	return messageResult;
}

void Klaymen::stIdleTeleporterHands() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x90EF8D38, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stSitIdleTeleporterBlinkSecond);
}

void Klaymen::stIdleTeleporterHands2() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0x900F0930, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stSitIdleTeleporterBlinkSecond);
}

void Klaymen::teleporterAppear(uint32 fileHash) {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(fileHash, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmTeleporterAppearDisappear);
	SetSpriteUpdate(NULL);
}

void Klaymen::teleporterDisappear(uint32 fileHash) {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(fileHash, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmTeleporterAppearDisappear);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4E0A2C24) {
			playSound(0, 0x85B10BB8);
		} else if (param.asInteger() == 0x4E6A0CA0) {
			playSound(0, 0xC5B709B0);
		}
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmShrink(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x80C110B5)
			sendMessage(_parentScene, 0x482A, 0);
		else if (param.asInteger() == 0x33288344)
			playSound(2, 0x10688664);
		break;
	}
	return messageResult;
}

void Klaymen::stShrink() {
	_busyStatus = 0;
	_acceptInput = false;
	playSound(0, 0x4C69EA53);
	startAnimation(0x1AE88904, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmShrink);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

void Klaymen::stStandWonderAbout() {
	if (_x > 260)
		setDoDeltaX(1);
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
	_newStickFrameIndex = 10;
}

uint32 Klaymen::hmDrinkPotion(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x1008:
		if (_potionFlag1) {
			startAnimationByHash(0x1C388C04, 0x004A2148, 0);
			messageResult = 0;
		} else
			_potionFlag2 = true;
		break;
	case 0x100D:
		if (param.asInteger() == 0x0002418E)
			sendMessage(_parentScene, 0x2000, 0);
		else if (param.asInteger() == 0x924090C2) {
			_potionFlag1 = true;
			if (_potionFlag2) {
				startAnimationByHash(0x1C388C04, 0x004A2148, 0);
				messageResult = 0;
			}
		} else if (param.asInteger() == 0x004A2148)
			_potionFlag1 = false;
		else if (param.asInteger() == 0x02B20220)
			playSound(0, 0xC5408620);
		else if (param.asInteger() == 0x0A720138)
			playSound(0, 0xD4C08010);
		else if (param.asInteger() == 0x03020231)
			playSound(0, 0xD4C08010);
		else if (param.asInteger() == 0xB613A180)
			playSound(0, 0x44051000);
		else if (param.asInteger() == 0x67221A03)
			playSound(0, 0x44051000);
		else if (param.asInteger() == 0x038A010B)
			playSound(0, 0x00018040);
		else if (param.asInteger() == 0x422B0280)
			playSound(0, 0x166FC6E0);
		else if (param.asInteger() == 0x925A0C1E)
			playSound(0, 0x40E5884D);
		else if (param.asInteger() == 0x000F0082)
			playSound(0, 0x546CDCC1);
		else if (param.asInteger() == 0x00020814)
			playSound(0, 0x786CC6D0);
		else if (param.asInteger() == 0x06020500)
			playSound(0, 0x1069C0E1);
		else if (param.asInteger() == 0x02128C00)
			playSound(0, 0x5068C4C3);
		else if (param.asInteger() == 0x82022030)
			playSound(0, 0x5C48C0E8);
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmGrow(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101)
			playSound(0, 0x405002D8);
		else if (param.asInteger() == 0x0A2A9098)
			playSound(0, 0x0460E2FA);
		else if (param.asInteger() == 0xD00A0C0C)
			playSound(3);
		else if (param.asInteger() == 0x04121920)
			playSound(4);
		else if (param.asInteger() == 0x030B4480)
			playSound(5);
		else if (param.asInteger() == 0x422B0280)
			playSound(6);
		else if (param.asInteger() == 0x038A010B)
			playSound(7);
		else if (param.asInteger() == 0x67221A03)
			playSound(0, 0x44051000);
		else if (param.asInteger() == 0x02B20220)
			playSound(0, 0xC5408620);
		else if (param.asInteger() == 0x925A0C1E)
			playSound(0, 0x40E5884D);
		else if (param.asInteger() == 0x03020231)
			playSound(0, 0xD4C08010);
		else if (param.asInteger() == 0x08040840)
			setDoDeltaX(2);
		break;
	}
	return messageResult;
}

void Klaymen::stGrow() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x2838C010, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmGrow);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

void Klaymen::stDrinkPotion() {
	_busyStatus = 1;
	_acceptInput = false;
	_potionFlag1 = false;
	_potionFlag2 = false;
	startAnimation(0x1C388C04, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmDrinkPotion);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 Klaymen::hmInsertKey(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (_keysToInsert == 0 && param.asInteger() == 0x06040580) {
			nextAnimationByHash(0xDC409440, 0x46431401, 0);
		} else if (_keysToInsert != 0 && param.asInteger() == 0x46431401) {
			_keysToInsert--;
			startAnimationByHash(0xDC409440, 0x01084280, 0);
		} else if (param.asInteger() == 0x062A1510) {
			playSound(0, 0x41688704);
		} else if (param.asInteger() == 0x02B20220) {
			playSound(0, 0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			playSound(0, 0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			playSound(0, 0x44051000);
		} else if (param.asInteger() == 0x0E4C8141) {
			playSound(0, 0xDC4A1280);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stInsertKey() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stInsertKey))) {
		_busyStatus = 2;
		_keysToInsert = 0;
		for (uint32 i = 0; i < 3; i++) {
			if (getSubVar(VA_HAS_KEY, i)) {
				bool more;
				setSubVar(VA_IS_KEY_INSERTED, i, 1);
				setSubVar(VA_HAS_KEY, i, 0);
				do {
					more = false;
					setSubVar(VA_CURR_KEY_SLOT_NUMBERS, i, _vm->_rnd->getRandomNumber(16 - 1));
					for (uint j = 0; j < i && !more; j++) {
						if (getSubVar(VA_IS_KEY_INSERTED, j) && getSubVar(VA_CURR_KEY_SLOT_NUMBERS, j) == getSubVar(VA_CURR_KEY_SLOT_NUMBERS, i))
							more = true;
					}
					if (getSubVar(VA_CURR_KEY_SLOT_NUMBERS, i) == getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, i))
						more = true;
				} while (more);
				_keysToInsert++;
			} 
		}
		if (_keysToInsert == 0) {
			GotoState(NULL);
			gotoNextStateExt();
		} else {
			_acceptInput = false;
			startAnimation(0xDC409440, 0, -1);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmInsertKey);
			SetSpriteUpdate(&Klaymen::suAction);
			_keysToInsert--;
		}
	}
}

uint32 Klaymen::hmReadNote(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x04684052) {
			_acceptInput = true;
			sendMessage(_parentScene, 0x2002, 0);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stReadNote() {
	_busyStatus = 2;
	_acceptInput = false;
	startAnimation(0x123E9C9F, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmReadNote);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 Klaymen::hmHitByDoor(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymenSpeedUpHash);
		if (_currFrameIndex < speedUpFrameIndex) {
			startAnimation(0x35AA8059, speedUpFrameIndex, -1);
			_y = 438;
		}
		messageResult = 0;
		break;
	case 0x100D:
		if (param.asInteger() == 0x1A1A0785) {
			playSound(0, 0x40F0A342);
		} else if (param.asInteger() == 0x60428026) {
			playSound(0, 0x40608A59);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stHitByDoor() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimation(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmHitByDoor);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	playSound(0, 0x402E82D4);
}

uint32 Klaymen::hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == calcHash("PopBalloon")) {
			sendMessage(_parentScene, 0x2000, 0);
		} else if (param.asInteger() == 0x02B20220) {
			playSound(0, 0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			playSound(0, 0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			playSound(0, 0x44051000);
		}
		break;
	}
	return messageResult;
}

void Klaymen::upPeekWallBlink() {
	Klaymen::update();
	_blinkCounter++;
	if (_blinkCounter >= _blinkCounterMax)
		stPeekWallBlink();
}

void Klaymen::stPeekWall1() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0xAC20C012, 8, 37);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stPeekWallBlink);
}

void Klaymen::stPeekWall2() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimation(0xAC20C012, 43, 49);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
}

void Klaymen::stPeekWallBlink() {
	_blinkCounter = 0;
	_busyStatus = 0;
	_acceptInput = true;
	_blinkCounterMax = _vm->_rnd->getRandomNumber(64) + 24;
	startAnimation(0xAC20C012, 38, 42);
	SetUpdateHandler(&Klaymen::upPeekWallBlink);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klaymen::hmLowLevel);
	_newStickFrameIndex = 42;
}

void Klaymen::stPeekWallReturn() {
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x2426932E, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmPeekWallReturn);
	SetSpriteUpdate(NULL);
}

void Klaymen::stPullHammerLever() {
	if (!stStartAction(AnimationCallback(&Klaymen::stPullHammerLever))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x00648953, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmPullHammerLever);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	}
}

uint32 Klaymen::hmPullHammerLever(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLever(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209)
			sendMessage(_attachedSprite, 0x480F, 0);
		break;
	}
	return messageResult;
}

void Klaymen::suRidePlatformDown() {
	_platformDeltaY++;
	_y += _platformDeltaY;
	if (_y > 600)
		sendMessage(this, 0x1019, 0);
}

void Klaymen::stRidePlatformDown() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stRidePlatformDown))) {
		_busyStatus = 1;
		sendMessage(_parentScene, 0x4803, 0);
		_acceptInput = false;
		_platformDeltaY = 0;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmLowLevel);
		SetSpriteUpdate(&Klaymen::suRidePlatformDown);
		_vm->_soundMan->playSoundLooping(0xD3B02847);
	}
}

void Klaymen::stCrashDown() {
	playSound(0, 0x41648271);
	_busyStatus = 1;
	_acceptInput = false;
	startAnimationByHash(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	NextState(&Klaymen::stCrashDownFinished);
}

void Klaymen::stCrashDownFinished() {
	setDoDeltaX(2);
	stTryStandIdle();
}

void Klaymen::upSpitOutFall() {
	Klaymen::update();
	if (_spitOutCountdown != 0 && (--_spitOutCountdown == 0)) {
		_surface->setVisible(true);
		SetUpdateHandler(&Klaymen::update);
	}
}

uint32 Klaymen::hmJumpToRingVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0) {
			sendMessage(_attachedSprite, 0x480F, 0);
		} else if (param.asInteger() == 0x586B0300) {
			sendMessage(_otherSprite, 0x480E, 1);
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmStandIdleSpecial(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x4811:
		playSound(0, 0x5252A0E4);
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		if (_doDeltaX) {
			_x = ((Sprite*)sender)->getX() - 75;
		} else {
			_x = ((Sprite*)sender)->getX() + 75;
		}
		_y = ((Sprite*)sender)->getY() - 200;
		if (param.asInteger() == 0) {
			stSpitOutFall0();
		} else if (param.asInteger() == 1) {
			// NOTE This is never used and the code was removed
			// Also the animations used here in the original don't exist...
		} else if (param.asInteger() == 2) {
			stSpitOutFall2();
		}
		break;
	}
	return 0;
}

uint32 Klaymen::hmPressDoorButton(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x942D2081) {
			_acceptInput = false;
			sendMessage(_attachedSprite, 0x2003, 0);
		} else if (param.asInteger() == 0xDA600012) {
			stHitByBoxingGlove();
		} else if (param.asInteger() == 0x0D01B294) {
			_acceptInput = false;
			sendMessage(_attachedSprite, 0x480B, 0);
		}
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			sendMessage(_attachedSprite, 0x480B, (uint32)_doDeltaX);
		} else if (param.asInteger() == 0x02421405) {
			if (_isMoveObjectRequested) {
				if (sendMessage(_attachedSprite, 0x480C, (uint32)_doDeltaX) != 0)
					stContinueMovingVenusFlyTrap();
			} else {
				SetMessageHandler(&Klaymen::hmFirstMoveVenusFlyTrap);
			}
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0460E2FA);
		}
		break;
	case 0x480A:
		_isMoveObjectRequested = true;
		return 0;
	}
	return hmLowLevelAnimation(messageNum, param, sender);
}

uint32 Klaymen::hmFirstMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmHitByBoxingGlove(int messageNum, const MessageParam &param, Entity *sender) {
	int16 speedUpFrameIndex;
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymenSpeedUpHash);
		if (_currFrameIndex < speedUpFrameIndex) {
			startAnimation(0x35AA8059, speedUpFrameIndex, -1);
			_y = 435;
		}
		messageResult = 0;
		break;
	case 0x100D:
		if (param.asInteger() == 0x1A1A0785) {
			playSound(0, 0x40F0A342);
		} else if (param.asInteger() == 0x60428026) {
			playSound(0, 0x40608A59);
		}
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmJumpAndFall(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x1307050A) {
			playSound(0, 0x40428A09);
		}
		break;
	}
	return messageResult;
}

void Klaymen::suFallDown() {
	AnimatedSprite::updateDeltaXY();
	HitRect *hitRect = _parentScene->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		updateBounds();
		sendMessage(this, 0x1019, 0);
	}
	_parentScene->checkCollision(this, 0xFFFF, 0x4810, 0);
}

void Klaymen::stJumpToRingVenusFlyTrap() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpToRingVenusFlyTrap))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x584984B4, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmJumpToRingVenusFlyTrap);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		NextState(&Klaymen::stLandOnFeet);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

void Klaymen::stStandIdleSpecial() {
	playSound(0, 0x56548280);
	_busyStatus = 0;
	_acceptInput = false;
	_surface->setVisible(false);
	startAnimation(0x5420E254, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmStandIdleSpecial);
	SetSpriteUpdate(NULL);
}

void Klaymen::stSpitOutFall0() {
	_spitOutCountdown = 1;
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x000BAB02, 0, -1);
	SetUpdateHandler(&Klaymen::upSpitOutFall);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(&Klaymen::suFallDown);
	NextState(&Klaymen::stFalling);
	sendMessage(_parentScene, 0x8000, 0);
}

void Klaymen::stSpitOutFall2() {
	_spitOutCountdown = 1;
	_busyStatus = 0;
	_acceptInput = false;
	startAnimation(0x9308C132, 0, -1);
	SetUpdateHandler(&Klaymen::upSpitOutFall);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(&Klaymen::suFallDown);
	NextState(&Klaymen::stFalling);
	sendMessage(_parentScene, 0x8000, 0);
}

void Klaymen::stFalling() {
	sendMessage(_parentScene, 0x1024, 1);
	playSound(0, 0x41648271);
	_busyStatus = 1;
	_acceptInput = false;
	_isWalking = false;
	startAnimationByHash(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	NextState(&Klaymen::stFallTouchdown);
	sendMessage(_parentScene, 0x2002, 0);
	_attachedSprite = NULL;
	sendMessage(_parentScene, 0x8001, 0);
}

void Klaymen::stFallTouchdown() {
	setDoDeltaX(2);
	stTryStandIdle();
}

void Klaymen::stJumpAndFall() {
	if (!stStartAction(AnimationCallback(&Klaymen::stJumpAndFall))) {
		sendMessage(_parentScene, 0x1024, 3);
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0xB93AB151, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmJumpAndFall);
		SetSpriteUpdate(&Klaymen::suFallDown);
		NextState(&Klaymen::stLandOnFeet);
	}
}

void Klaymen::stDropFromRing() {
	if (_attachedSprite) {
		_x = _attachedSprite->getX();
		sendMessage(_attachedSprite, 0x4807, 0);
		_attachedSprite = NULL;
	}
	_busyStatus = 2;
	_acceptInput = false;
	startAnimation(0x586984B1, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(&Klaymen::suFallDown);
	NextState(&Klaymen::stLandOnFeet);
}

void Klaymen::stPressDoorButton() {
	_busyStatus = 2;
	_acceptInput = true;
	setDoDeltaX(0);
	startAnimation(0x1CD89029, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmPressDoorButton);
	SetSpriteUpdate(&Klaymen::suAction);
}

void Klaymen::stHitByBoxingGlove() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimation(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmHitByBoxingGlove);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	FinalizeState(&Klaymen::evHitByBoxingGloveDone);
}

void Klaymen::evHitByBoxingGloveDone() {
	sendMessage(_parentScene, 0x1024, 1);
}

void Klaymen::stMoveVenusFlyTrap() {
	if (!stStartAction(AnimationCallback(&Klaymen::stMoveVenusFlyTrap))) {
		_busyStatus = 2;
		_isMoveObjectRequested = false;
		_acceptInput = true;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		startAnimation(0x5C01A870, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmMoveVenusFlyTrap);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		FinalizeState(&Klaymen::evMoveVenusFlyTrapDone);
	}
}

void Klaymen::stContinueMovingVenusFlyTrap() {
	_isMoveObjectRequested = false;
	_acceptInput = true;
	startAnimationByHash(0x5C01A870, 0x01084280, 0);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmMoveVenusFlyTrap);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	FinalizeState(&Klaymen::evMoveVenusFlyTrapDone);
}

void Klaymen::evMoveVenusFlyTrapDone() {
	sendMessage(_attachedSprite, 0x482A, 0);
}

void Klaymen::suFallSkipJump() {
	updateDeltaXY();
	HitRect *hitRect = _parentScene->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		updateBounds();
		sendMessage(this, 0x1019, 0);
	}
}

void Klaymen::stFallSkipJump() {
	_busyStatus = 2;
	_acceptInput = false;
	startAnimationByHash(0xB93AB151, 0x40A100F8, 0);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(&Klaymen::suFallSkipJump);
	SetMessageHandler(&Klaymen::hmLowLevel);
	NextState(&Klaymen::stLandOnFeet);
}

void Klaymen::upMoveObject() {
	if (_x >= 380)
		gotoNextStateExt();
	Klaymen::update();		
}

uint32 Klaymen::hmMatch(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
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
	}
	return messageResult;
}

void Klaymen::stFetchMatch() {
	if (!stStartAction(AnimationCallback(&Klaymen::stFetchMatch))) {
		_busyStatus = 0;
		_acceptInput = false;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		startAnimation(0x9CAA0218, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmMatch);
		SetSpriteUpdate(NULL);
		NextState(&Klaymen::stLightMatch);
	}
}

void Klaymen::stLightMatch() {
	_busyStatus = 1;
	_acceptInput = false;
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	startAnimation(0x1222A513, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmMatch);
	SetSpriteUpdate(NULL);
}

uint32 Klaymen::hmMoveObject(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			playSound(0, 0x405002D8);
			sendMessage(_attachedSprite, 0x480B, 0);
		} else if (param.asInteger() == 0x02421405) {
			if (_moveObjectCountdown != 0) {
				_moveObjectCountdown--;
				stContinueMoveObject();
			} else {
				SetMessageHandler(&Klaymen::hmLowLevelAnimation);
			}
		}
		break;
	}
	return Klaymen::hmLowLevelAnimation(messageNum, param, sender);
}

uint32 Klaymen::hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000F0082) {
			playSound(0, 0x74E2810F);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stMoveObject() {
	if (!stStartAction(AnimationCallback(&Klaymen::stMoveObject))) {
		_busyStatus = 2;
		_acceptInput = false;
		_moveObjectCountdown = 8;
		setDoDeltaX(0);
		startAnimation(0x0C1CA072, 0, -1);
		SetUpdateHandler(&Klaymen::upMoveObject);
		SetMessageHandler(&Klaymen::hmMoveObject);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	}
}

void Klaymen::stContinueMoveObject() {
	_acceptInput = false;
	startAnimationByHash(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klaymen::upMoveObject);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	SetMessageHandler(&Klaymen::hmMoveObject);
}

void Klaymen::stTumbleHeadless() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stTumbleHeadless))) {
		_busyStatus = 1;
		_acceptInput = false;
		setDoDeltaX(0);
		startAnimation(0x2821C590, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmTumbleHeadless);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		NextState(&Klaymen::stTryStandIdle);
		sendMessage(_parentScene, 0x8000, 0);
		playSound(0, 0x62E0A356);
	}
}

void Klaymen::stCloseEyes() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stCloseEyes))) {
		_busyStatus = 1;
		_acceptInput = false;		
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmLowLevel);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klaymen::hmSpit(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klaymen::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x16401CA6) {
			_canSpitPipe = true;
			if (_contSpitPipe)
				spitIntoPipe();
		} else if (param.asInteger() == 0xC11C0008) {
			_canSpitPipe = false;
			_acceptInput = false;
			_readyToSpit = false;
		} else if (param.asInteger() == 0x018A0001) {
			sendMessage(_parentScene, 0x2001, _spitDestPipeIndex);
		}
		break;
	}
	return messageResult;
}

void Klaymen::stTrySpitIntoPipe() {
	if (_readyToSpit) {
		_contSpitPipe = true;
		_spitContDestPipeIndex = _spitPipeIndex;
		if (_canSpitPipe)
			spitIntoPipe();
	} else if (!stStartAction(AnimationCallback(&Klaymen::stTrySpitIntoPipe))) {
		_busyStatus = 2;
		_acceptInput = true;
		_spitDestPipeIndex = _spitPipeIndex;
		_readyToSpit = true;
		_canSpitPipe = false;
		_contSpitPipe = false;
		startAnimation(0x1808B150, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmSpit);
		SetSpriteUpdate(NULL);
	}
}

void Klaymen::spitIntoPipe() {
	_contSpitPipe = false;
	_spitDestPipeIndex = _spitContDestPipeIndex;
	_canSpitPipe = false;
	_acceptInput = false;
	startAnimation(0x1B08B553, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmSpit);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stContSpitIntoPipe);
}

void Klaymen::stContSpitIntoPipe() {
	_canSpitPipe = true;
	_acceptInput = true;
	startAnimationByHash(0x1808B150, 0x16401CA6, 0);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmSpit);
	SetSpriteUpdate(NULL);
}

void Klaymen::suRidePlatform() {
	_x = _attachedSprite->getX() - 20;
	_y = _attachedSprite->getY() + 46;
	updateBounds();
}

void Klaymen::stRidePlatform() {
	if (!stStartActionFromIdle(AnimationCallback(&Klaymen::stRidePlatform))) {
		_busyStatus = 1;
		_acceptInput = true;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmLowLevel);
		SetSpriteUpdate(&Klaymen::suRidePlatform);
	}
}

void Klaymen::stInteractLever() {
	if (!stStartAction(AnimationCallback(&Klaymen::stInteractLever))) {
		_busyStatus = 0;
		if (_isLeverDown) {
			stUseLeverRelease();
		} else {
			_acceptInput = false;
			startAnimation(0x0C303040, 0, -1);
			SetUpdateHandler(&Klaymen::update);
			SetMessageHandler(&Klaymen::hmLever);
			SetSpriteUpdate(&Klaymen::suUpdateDestX);
			NextState(&Klaymen::stPullLever);
		}
	}
}

void Klaymen::stPullLever() {
	startAnimation(0x0D318140, 0, -1);
	NextState(&Klaymen::stLookLeverDown);
	sendMessage(_attachedSprite, 0x480F, 0);
}

void Klaymen::stLookLeverDown() {
	_acceptInput = true;
	_isLeverDown = true;
	startAnimation(0x1564A2C0, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
	NextState(&Klaymen::stWaitLeverDown);
}

void Klaymen::stWaitLeverDown() {
	_acceptInput = true;
	_isLeverDown = true;
	startAnimation(0x4464A440, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(&Klaymen::suUpdateDestX);
}

void Klaymen::stStartWalkingResume() {
	int16 frameIndex = getGlobalVar(V_KLAYMEN_FRAMEINDEX) + _walkResumeFrameIncr;
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_busyStatus = 0;
	_isWalking = true;
	_acceptInput = true;
	startAnimation(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmWalking);
	SetSpriteUpdate(&Klaymen::suWalkingFirst);
	NextState(&Klaymen::stUpdateWalkingFirst);
	FinalizeState(&Klaymen::evStartWalkingDone);
}

void Klaymen::upPeekInsideBlink() {
	update();
	++_blinkCounter;
	if (_blinkCounter >= _blinkCounterMax)
		stPeekInsideBlink();
}

void Klaymen::stPeekInside() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0xAC20C012, 8, 37);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klaymen::stPeekInsideBlink);
}

void Klaymen::stPeekInsideReturn() {
	_busyStatus = 1;
	_acceptInput = false;
	startAnimation(0xAC20C012, 43, 49);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klaymen::stPeekInsideBlink() {
	_busyStatus = 0;
	_acceptInput = true;
	startAnimation(0xAC20C012, 38, 42);
	_newStickFrameIndex = 42;
	SetUpdateHandler(&Klaymen::upPeekInsideBlink);
	SetMessageHandler(&Klaymen::hmLowLevel);
	SetSpriteUpdate(NULL);
	_blinkCounter = 0;
	_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

// KmScene1001

KmScene1001::KmScene1001(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
}

uint32 KmScene1001::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;		
	case 0x4804:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stSleeping);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullHammerLever);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4836:
		if (param.asInteger() == 1) {
			sendMessage(_parentScene, 0x2002, 0);
			GotoState(&Klaymen::stWakeUp);
		}
		break;		
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

// KmScene1002

KmScene1002::KmScene1002(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	setKlaymenIdleTable1();
}

void KmScene1002::xUpdate() {
	if (_x >= 250 && _x <= 435 && _y >= 420) {
		if (_idleTableNum == 0) {
			setKlaymenIdleTable(klaymenIdleTable1002, ARRAYSIZE(klaymenIdleTable1002));
			_idleTableNum = 1;
		}
	} else if (_idleTableNum == 1) {
		setKlaymenIdleTable1();
		_idleTableNum = 0;
	}
}
	
uint32 KmScene1002::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&Klaymen::stStandIdleSpecial);
		break;
	case 0x2007:
		_otherSprite = (Sprite*)param.asEntity();
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004: 
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stJumpAndFall);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stDropFromRing);
		break;
	case 0x4804:
		GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4805:
		switch (param.asInteger()) {
		case 1:
			GotoState(&Klaymen::stJumpToRing1);
			break;
		case 2:
			GotoState(&Klaymen::stJumpToRing2);
			break;
		case 3:
			GotoState(&Klaymen::stJumpToRing3);
			break;
		case 4:
			GotoState(&Klaymen::stJumpToRing4);
			break;
		}
		break;
	case 0x480A:	  
		GotoState(&Klaymen::stMoveVenusFlyTrap);
		break;
	case 0x480D:			   
		GotoState(&Klaymen::stJumpToRingVenusFlyTrap);
		break;
	case 0x4816:  
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressDoorButton);
		break;
	case 0x4817:				  
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:				
		startWalkToAttachedSpriteXDistance(param.asInteger());
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2005, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2005, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2005, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2006, 0);
		GotoState(&Klaymen::stClimbLadderHalf);	 
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
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840: 
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

// KmScene1004

KmScene1004::KmScene1004(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	_dataResource.load(0x01900A04);	
}

uint32 KmScene1004::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReadNote);
		break;
	case 0x4820:
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);
		break;
	case 0x4821:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);
		break;
	case 0x4822:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stClimbLadderHalf);
		break;
	case 0x4824:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klaymen::stStartClimbLadderDown);
		break;
	case 0x4825:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klaymen::stStartClimbLadderUp);
		break;
	case 0x4828:
		GotoState(&Klaymen::stTurnToBackToUse);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

KmScene1109::KmScene1109(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	// Empty
}

uint32 KmScene1109::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;		
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
		teleporterAppear(0x2C2A4A1C);
		break;
	case 0x483E:
		teleporterDisappear(0x3C2E4245);
		break;
	}
	return messageResult;
}

// KmScene1201

KmScene1201::KmScene1201(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	setKlaymenIdleTable(klaymenIdleTable4, ARRAYSIZE(klaymenIdleTable4));
	_doYHitIncr = true;
}

uint32 KmScene1201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480A:
		GotoState(&Klaymen::stMoveObject);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4813:
		GotoState(&Klaymen::stFetchMatch);
		break;
	case 0x4814:
		GotoState(&Klaymen::stTumbleHeadless);
		break;
	case 0x4815:
		GotoState(&Klaymen::stCloseEyes);
		break;
	case 0x4816:
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
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
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
	}
	return 0;
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
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;		
	case 0x4812:
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
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;		
	case 0x4804:
		GotoState(&Klaymen::stCrashDown);
		break;		
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	}
	return 0;
}

KmScene1306::KmScene1306(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	// Empty
}

uint32 KmScene1306::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4812:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481A:
		GotoState(&Klaymen::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		else
			GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
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
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480A:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
		break;		
	case 0x480D:
		GotoState(&Klaymen::stUseLever);
		break;
	case 0x4812:
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
	case 0x481A:
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
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x4827:
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
	}
	return 0;
}

// KmScene1401

KmScene1401::KmScene1401(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	// Empty	
}

uint32 KmScene1401::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;		
	case 0x480A:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
		break;		
	case 0x4816:
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
	}
	return 0;
}

// KmScene1402

KmScene1402::KmScene1402(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	SetFilterY(&Sprite::defFilterY);	
}

uint32 KmScene1402::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;		
	case 0x480A:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
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
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	}
	return 0;
}

// KmScene1403

KmScene1403::KmScene1403(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	setKlaymenIdleTable(klaymenIdleTable4, ARRAYSIZE(klaymenIdleTable4));
}

uint32 KmScene1403::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480A:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
		break;		
	case 0x480D:
		GotoState(&Klaymen::stUseLever);
		break;
	case 0x4812:
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
	case 0x4827:
		GotoState(&Klaymen::stReleaseLever);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

// KmScene1404

KmScene1404::KmScene1404(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty	
}

uint32 KmScene1404::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480A:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
		break;		
	case 0x4812:
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
	case 0x481A:
		GotoState(&Klaymen::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
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
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

KmScene1608::KmScene1608(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1608::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2032:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4812:
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
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2032, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klaymen::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2032, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klaymen::stGetUpFromTeleporter);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return messageResult;
}

// KmScene1705

KmScene1705::KmScene1705(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty	
}

uint32 KmScene1705::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		GotoState(&Klaymen::stFallSkipJump);
		break;				
	case 0x4812:
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
	case 0x481D:
		if (_isSittingInTeleporter) {
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		}
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
		teleporterAppear(0x5E0A4905);
		break;																				
	case 0x483E:
		teleporterDisappear(0xD86E4477);
		break;																				
	}
	return messageResult;
}

KmScene1901::KmScene1901(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty	
}

uint32 KmScene1901::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
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
	}
	return 0;
}

KmScene2001::KmScene2001(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty	
}

uint32 KmScene2001::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
		teleporterAppear(0xBE68CC54);
		break;
	case 0x483E:
		teleporterDisappear(0x18AB4ED4);
		break;
	}
	return messageResult;
}

KmScene2101::KmScene2101(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	// Empty
}

uint32 KmScene2101::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4811:
		GotoState(&Klaymen::stHitByDoor);
		break;
	case 0x4812:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
		teleporterAppear(0xFF290E30);
		break;
	case 0x483E:
		teleporterDisappear(0x9A28CA1C);
		break;
	}
	return messageResult;	
}

KmScene2201::KmScene2201(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	_surface->setClipRects(clipRects, clipRectsCount);
	_dataResource.load(0x04104242);
}

uint32 KmScene2201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
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
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;	
}

KmScene2203::KmScene2203(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}
	
uint32 KmScene2203::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4812:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4819:
		GotoState(&Klaymen::stClayDoorOpen);
		break;
	case 0x481A:
		GotoState(&Klaymen::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
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
	}
	return 0;
}

KmScene2205::KmScene2205(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

void KmScene2205::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}
	
uint32 KmScene2205::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4816:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

KmScene2206::KmScene2206(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	_walkResumeFrameIncr = 1;
	_vm->_soundMan->addSound(0x80101800, 0xD3B02847);
}

KmScene2206::~KmScene2206() {
	_vm->_soundMan->deleteSoundGroup(0x80101800);
}

void KmScene2206::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}
	
uint32 KmScene2206::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		GotoState(&Klaymen::stRidePlatformDown);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4812:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
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
	case 0x4837:
		stopWalking();
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

KmScene2207::KmScene2207(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}
	
uint32 KmScene2207::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&Klaymen::stRidePlatform);
		break;
	case 0x2005:
		suRidePlatform();
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&Klaymen::stInteractLever);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x4827:
		GotoState(&Klaymen::stReleaseLever);
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
	}
	return 0;
}

KmScene2242::KmScene2242(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

void KmScene2242::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmScene2242::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4812:
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
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmHallOfRecords::KmHallOfRecords(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	// Empty
}

void KmHallOfRecords::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmHallOfRecords::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmScene2247::KmScene2247(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

void KmScene2247::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmScene2247::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}
  
KmScene2401::KmScene2401(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}
	
uint32 KmScene2401::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4816:
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
	case 0x4832:
		GotoState(&Klaymen::stUseTube);
		break;
	case 0x4833:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAbout);
		else {
			_spitPipeIndex = sendMessage(_parentScene, 0x2000, 0);
			GotoState(&Klaymen::stTrySpitIntoPipe);
		}
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;		
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return messageResult;
}

KmScene2402::KmScene2402(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2402::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (!getGlobalVar(V_TV_JOKE_TOLD))
			GotoState(&Klaymen::stStandWonderAbout);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return messageResult;
}

KmScene2403::KmScene2403(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2403::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4816:
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
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stClimbLadderHalf);	 
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
	}
	return messageResult;
}
	
KmScene2406::KmScene2406(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {
	
	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2406::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stWalkingFirst);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4812:
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
	case 0x481A:
		GotoState(&Klaymen::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stClimbLadderHalf);	 
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840: 
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return messageResult;
}
	
KmScene2501::KmScene2501(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}
	
uint32 KmScene2501::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
	}
	return messageResult;
}

KmScene2732::KmScene2732(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}
	
uint32 KmScene2732::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4804:
		GotoState(&Klaymen::stPeekInside);
		break;
	case 0x483C:
		GotoState(&Klaymen::stPeekInsideReturn);
		break;
	}
	return 0;
}

KmScene2801::KmScene2801(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2801::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4812:
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
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
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
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmScene2803::KmScene2803(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {
	
	_surface->setClipRects(clipRects, clipRectsCount);
	_dataResource.load(0x00900849);
}

uint32 KmScene2803::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		_destY = param.asInteger();
		GotoState(&Klaymen::stJumpToGrab);
		break;
	case 0x4804:
		if (param.asInteger() == 3)
			GotoState(&Klaymen::stFinishGrow);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else
			GotoState(&Klaymen::stWonderAboutHalf);
		break;
	case 0x482E:	 
		GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x4834:
		GotoState(&Klaymen::stStepOver);
		break;
	case 0x4838:
		GotoState(&Klaymen::stJumpToGrabRelease);
		break;
	}
	return 0;
}

KmScene2803Small::KmScene2803Small(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	
	_dataResource.load(0x81120132);
}

uint32 KmScene2803Small::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToXSmall(param.asPoint().x);
		break;
	case 0x4004:
		GotoState(&Klaymen::stStandIdleSmall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToXSmall(_dataResource.getPoint(param.asInteger()).x);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfterSmall);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalfSmall);
		else
			GotoState(&Klaymen::stWonderAboutSmall);
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStepSmall);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stWalkToFront2Small);
		else
			GotoState(&Klaymen::stWalkToFrontSmall);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToBackHalfSmall);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stTurnToBackWalkSmall);
		else
			GotoState(&Klaymen::stTurnToBackSmall);
		break;
	case 0x4830:
		GotoState(&Klaymen::stShrink);
		break;
	}
	return 0;
}

KmScene2805::KmScene2805(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2805::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stSitIdleTeleporter);
		else
			GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klaymen::stReturnFromUseInTeleporter);
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
		teleporterAppear(0xDE284B74);
		break;
	case 0x483E:
		teleporterDisappear(0xD82A4094);
		break;
	}
	return messageResult;
}

KmScene2806::KmScene2806(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
	bool needsLargeSurface, NRect *clipRects, uint clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	if (needsLargeSurface) {
		NDimensions dimensions = _animResource.loadSpriteDimensions(0x2838C010);
		delete _surface;
		createSurface(1000, dimensions.width, dimensions.height);
		loadSound(3, 0x58E0C341);
		loadSound(4, 0x40A00342);
		loadSound(5, 0xD0A1C348);
		loadSound(6, 0x166FC6E0);
		loadSound(7, 0x00018040);
	}
	
	_dataResource.load(0x98182003);
	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2806::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		startWalkToX(440, true);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case 0x4816:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide); 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4831:
		GotoState(&Klaymen::stGrow);
		break;
	case 0x4832:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stDrinkPotion);
		else
			GotoState(&Klaymen::stUseTube);
		break;
	}
	return 0;
}

KmScene2809::KmScene2809(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y,
	bool needsLargeSurface, NRect *clipRects, uint clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	if (needsLargeSurface) {
		NDimensions dimensions = _animResource.loadSpriteDimensions(0x2838C010);
		delete _surface;
		createSurface(1000, dimensions.width, dimensions.height);
		loadSound(3, 0x58E0C341);
		loadSound(4, 0x40A00342);
		loadSound(5, 0xD0A1C348);
		loadSound(6, 0x166FC6E0);
		loadSound(7, 0x00018040);
	}

	_dataResource.load(0x1830009A);
	_surface->setClipRects(clipRects, clipRectsCount);
}

uint32 KmScene2809::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		startWalkToX(226, true);
		break;
	case 0x480D:
		GotoState(&Klaymen::stPullCord);
		break;
	case 0x4816:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide); 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4831:
		GotoState(&Klaymen::stGrow);
		break;
	case 0x4832:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stDrinkPotion);
		else
			GotoState(&Klaymen::stUseTube);
		break;
	}
	return 0;
}

KmScene2810Small::KmScene2810Small(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y) 
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2810Small::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToXSmall(param.asPoint().x);
		break;
	case 0x4004:
		GotoState(&Klaymen::stStandIdleSmall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToXSmall(_dataResource.getPoint(param.asInteger()).x);
		break;
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfterSmall);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalfSmall);
		else
			GotoState(&Klaymen::stWonderAboutSmall);
		break;
	case 0x482E:	 
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStepSmall);
		else
			GotoState(&Klaymen::stWalkToFrontSmall);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToBackHalfSmall);
		else
			GotoState(&Klaymen::stTurnToBackSmall);
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmScene2810::KmScene2810(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, uint clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	_surface->setClipRects(clipRects, clipRectsCount);
}
		
uint32 KmScene2810::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		_destY = param.asInteger();
		GotoState(&Klaymen::stJumpToGrab);
		break;
	case 0x4804:
		if (param.asInteger() == 3)
			GotoState(&Klaymen::stFinishGrow);
		break;
	case 0x4812:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 5)
			GotoState(&Klaymen::stTurnToUseExt);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stClimbLadderHalf);	 
		break;
	case 0x4824:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klaymen::stStartClimbLadderDown);
		break;
	case 0x4825:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klaymen::stStartClimbLadderUp);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmScene2812::KmScene2812(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2812::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4805:
		_destY = param.asInteger();
		GotoState(&Klaymen::stJumpToGrabFall);
		break;
	case 0x4812:
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
	case 0x481A:
		GotoState(&Klaymen::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klaymen::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2001, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2001, 0);
		_destY = param.asInteger();
		GotoState(&Klaymen::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2002, 0);
		GotoState(&Klaymen::stClimbLadderHalf);	 
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
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840: 
		startSpecialWalkLeft(param.asInteger());
		break;
	}
	return 0;
}

} // End of namespace Neverhood
