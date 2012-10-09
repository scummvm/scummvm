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

#include "neverhood/klayman.h"
#include "neverhood/collisionman.h"
#include "neverhood/resourceman.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

static const KlaymanIdleTableItem klaymanTable1[] = {
	{1, &Klayman::stDoIdlePickEar},
	{1, &Klayman::stDoIdleSpinHead},
	{1, &Klayman::stDoIdleArms},
	{1, &Klayman::stDoIdleChest},
	{1, &Klayman::stDoIdleHeadOff}
}; 

static const KlaymanIdleTableItem klaymanTable2[] = {
	{1, &Klayman::stDoIdlePickEar},
	{1, &Klayman::stDoIdleSpinHead},
	{1, &Klayman::stDoIdleChest},
	{1, &Klayman::stDoIdleHeadOff}
}; 

static const KlaymanIdleTableItem klaymanTable3[] = {
	{1, &Klayman::stDoIdleTeleporterHands},
	{1, &Klayman::stDoIdleTeleporterHands2}
}; 

static const KlaymanIdleTableItem klaymanTable4[] = {
	{1, &Klayman::stDoIdleSpinHead},
	{1, &Klayman::stDoIdleChest},
	{1, &Klayman::stDoIdleHeadOff},
};

// Klayman

Klayman::Klayman(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, int surfacePriority, int objectPriority, NRectArray *clipRects)
	: AnimatedSprite(vm, objectPriority), _idleCounterMax(0), _idleCounter(0), _isMoveObjectRequested(false), _blinkCounterMax(0),
	_isWalkingOpenDoorNotified(false), _countdown1(0), _tapesToInsert(0), _keysToInsert(0), /*_field118(0), */_status2(0), _acceptInput(true),
	_attachedSprite(NULL), _isWalking(false), _status3(1), _parentScene(parentScene), _isSneaking(false), _isLargeStep(false),
	_flagF6(false), _isLeverDown(false), _isSittingInTeleporter(false), _flagFA(false), _ladderStatus(0), _pathPoints(NULL), _soundFlag(false),
	_idleTableNum(0), _otherSprite(NULL), _moveObjectCountdown(0), _readyToSpit(false), _walkResumeFrameIncr(0) {
	
	// TODO DirtySurface
	createSurface(surfacePriority, 320, 200);
	_x = x;
	_y = y;
	_destX = x;
	_destY = y;
	_flags = 2;
	setKlaymanIdleTable1();
	stTryStandIdle();
	SetUpdateHandler(&Klayman::update);
}

void Klayman::xUpdate() {
	// Empty
}

uint32 Klayman::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	}
	return 0;
}

void Klayman::update() {
	AnimatedSprite::update();
	xUpdate();
}

void Klayman::setKlaymanIdleTable(const KlaymanIdleTableItem *table, int tableCount) {
	_idleTable = table;
	_idleTableCount = tableCount;
	_idleTableMaxValue = 0;
	for (int i = 0; i < tableCount; i++)
		_idleTableMaxValue += table[i].value;
}

void Klayman::setKlaymanIdleTable1() {
	setKlaymanIdleTable(klaymanTable1, ARRAYSIZE(klaymanTable1));
}

void Klayman::setKlaymanIdleTable2() {
	setKlaymanIdleTable(klaymanTable2, ARRAYSIZE(klaymanTable2));
}

void Klayman::setKlaymanIdleTable3() {
	setKlaymanIdleTable(klaymanTable3, ARRAYSIZE(klaymanTable3));
}

void Klayman::stDoIdlePickEar() {
	startIdleAnimation(0x5B20C814, AnimationCallback(&Klayman::stIdlePickEar));
}

void Klayman::stIdlePickEar() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x5B20C814, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdlePickEar);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stStandAround);
	FinalizeState(&Klayman::evIdlePickEarDone);
}

uint32 Klayman::hmIdlePickEar(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::evIdlePickEarDone() {
	stopSound(0);
}

void Klayman::stDoIdleSpinHead() {
	startIdleAnimation(0xD122C137, AnimationCallback(&Klayman::stIdleSpinHead));
}

void Klayman::stIdleSpinHead() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0xD122C137, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdleSpinHead);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::hmIdleSpinHead(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stDoIdleArms() {
	startIdleAnimation(0x543CD054, AnimationCallback(&Klayman::stIdleArms));
}

void Klayman::stIdleArms() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x543CD054, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdleArms);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stStandAround);
	FinalizeState(&Klayman::evIdleArmsDone);
}

void Klayman::evIdleArmsDone() {
	stopSound(0);
}

uint32 Klayman::hmIdleArms(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stDoIdleChest() {
	startIdleAnimation(0x40A0C034, AnimationCallback(&Klayman::stIdleChest));
}

void Klayman::stIdleChest() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x40A0C034, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdleChest);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::hmIdleChest(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stDoIdleHeadOff() {
	startIdleAnimation(0x5120E137, AnimationCallback(&Klayman::stIdleHeadOff));
}

void Klayman::stIdleHeadOff() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x5120E137, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdleHeadOff);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::hmIdleHeadOff(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stSitIdleTeleporter() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x582EC138, 0, -1);
	SetUpdateHandler(&Klayman::upSitIdleTeleporter);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
	_idleCounter = 0;
	_blinkCounter = 0;
	_idleCounterMax = 8;
	_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

void Klayman::upSitIdleTeleporter() {
	update();
	_idleCounter++;
	if (_idleCounter >= _idleCounterMax) {
		_idleCounter = 0;
		if (_idleTable) {
			int randomValue = _vm->_rnd->getRandomNumber(_idleTableMaxValue);
			for (int i = 0; i < _idleTableCount; i++) {
				if (randomValue < _idleTable[i].value) {
					(this->*(_idleTable[i].callback))();
					_idleCounterMax = _vm->_rnd->getRandomNumber(128 - 1) + 24;
					break;
				}
				randomValue -= _idleTable[i].value;
			}
		}
	} else if (++_blinkCounter >= _blinkCounterMax) {
		_blinkCounter = 0;
		_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
		stSitIdleTeleporterBlink();
	}
}

void Klayman::stSitIdleTeleporterBlink() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x5C24C018, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stSitIdleTeleporterBlinkSecond);
}

void Klayman::stSitIdleTeleporterBlinkSecond() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x5C24C018, 0, -1);
	SetUpdateHandler(&Klayman::upSitIdleTeleporter);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
}

void Klayman::stPickUpNeedle() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::stPickUpNeedle))) {
		_status2 = 1;
		_acceptInput = false;
		startAnimation(0x1449C169, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPickUpObject);
		SetSpriteUpdate(NULL);
	}
}

void Klayman::stPickUpTube() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::stPickUpTube))) {
		_status2 = 1;
		_acceptInput = false;
		startAnimation(0x0018C032, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPickUpTube);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klayman::hmPickUpTube(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stTurnToUseInTeleporter() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0xD229823D, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stReturnFromUseInTeleporter() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x9A2801E0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stStepOver() {
	if (!stStartAction(AnimationCallback(&Klayman::stStepOver))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x004AA310, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmStartWalking);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stSitInTeleporter() {
	if (!stStartAction(AnimationCallback(&Klayman::stSitInTeleporter))) {
		_status2 = 0;
		_acceptInput = false;
		startAnimation(0x392A0330, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmSitInTeleporter);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

uint32 Klayman::hmSitInTeleporter(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stGetUpFromTeleporter() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x913AB120, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

/////////////////////////////////////////////////////////////////

void Klayman::stopWalking() {
	_destX = _x;
	if (!_isWalking && !_isSneaking && !_isLargeStep) {
		gotoState(NULL);
		gotoNextStateExt();
	}
}

void Klayman::startIdleAnimation(uint32 fileHash, AnimationCb callback) {
	debug("startIdleAnimation(%08X)", fileHash);
	NextState(callback);
	SetUpdateHandler(&Klayman::upIdleAnimation);
}

void Klayman::upIdleAnimation() {
	gotoNextStateExt();
	update();
}

bool Klayman::stStartActionFromIdle(AnimationCb callback) {
	if (_status2 == 2) {
		_status2 = 1;
		_acceptInput = false; 
		startAnimation(0x9A7020B8, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmStartAction);
		SetSpriteUpdate(NULL);
		NextState(callback);
		return true;
	}
	return false;
}

void Klayman::gotoNextStateExt() {
	if (_finalizeStateCb) {
		AnimationCb cb = _finalizeStateCb;
		_finalizeStateCb = NULL;
		(this->*cb)();
	}
	if (_nextStateCb) {
		AnimationCb cb = _nextStateCb;
		_nextStateCb = NULL;
		(this->*cb)();
#if 0 // TODO (So far, with almost all Klayman subclasses implemented, _callbackList and related code seems unused)
	} else if (_callbackList) {
		removeCallbackList();
#endif		
	} else {
		// Inform the scene that the current Klayman animation sequence has finished
		sendMessage(_parentScene, 0x1006, 0);
	}
}

void Klayman::sub41C770() {
	_flagFA = false;
	_status3 = 1;
}

void Klayman::sub41C790() {
	if (_flagFA)
		_status3 = 0;
}

void Klayman::stTryStandIdle() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stTryStandIdle))) {
		_status2 = 1;
		_acceptInput = true;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::upStandIdle);
		SetMessageHandler(&Klayman::hmLowLevel);
		SetSpriteUpdate(NULL);
		_idleCounter = 0;
		_blinkCounter = 0;
		_blinkCounterMax = _vm->_rnd->getRandomNumber(64) + 24;
	}
}

void Klayman::upStandIdle() {
	update();
	if (++_idleCounter >= 720) {
		_idleCounter = 0;
		if (_idleTable) {
			int randomValue = _vm->_rnd->getRandomNumber(_idleTableMaxValue - 1);
			for (int i = 0; i < _idleTableCount; i++) {
				if (randomValue < _idleTable[i].value) {
					(this->*(_idleTable[i].callback))();
					break;
				}
				randomValue -= _idleTable[i].value;
			}
		}
	} else if (++_blinkCounter >= _blinkCounterMax) {
		_blinkCounter = 0;
		_blinkCounterMax = _vm->_rnd->getRandomNumber(64 - 1) + 24;
		stIdleBlink();
	}
}

uint32 Klayman::hmLowLevel(int messageNum, const MessageParam &param, Entity *sender) {
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
		sub41C770();
		break;
	case 0x1021:
		sub41C790();
		break;
	case 0x481C:
		_status3 = param.asInteger();
		_flagFA = true;
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

void Klayman::stIdleBlink() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x5900C41E, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::hmLowLevelAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextStateExt();
		break;
	}
	return messageResult;
}

void Klayman::stStandAround() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x5420E254, 0, -1);
	SetUpdateHandler(&Klayman::upStandIdle);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmStartAction(int messageNum, const MessageParam &param, Entity *sender) {
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


void Klayman::startWalkToX(int16 x, bool flag) {
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
	} else if (xdiff <= 42 && _status3 != 3) {
		if (_isSneaking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_destX - _x) > xdiff) {
			_destX = x;
		} else {
			_destX = x;
			GotoState(&Klayman::stSneak);
		}
	} else if (_isWalking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_destX = x;
	} else if (flag) {
		_destX = x;
		GotoState(&Klayman::stStartWalkingExt);
	} else {
		_destX = x;
		GotoState(&Klayman::stStartWalking);
	}
}

void Klayman::stWakeUp() {
	_status2 = 1;
	_acceptInput = false;
	startAnimation(0x527AC970, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stSleeping() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x5A38C110, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmSleeping);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmSleeping(int messageNum, const MessageParam &param, Entity *sender) {
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

bool Klayman::stStartAction(AnimationCb callback3) {
	if (_status2 == 1) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x5C7080D4, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmStartAction);
		SetSpriteUpdate(&Klayman::suAction);
		NextState(callback3);
		return true;
	} else {
		_x = _destX;
		return false;
	}
}

void Klayman::suAction() {

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
	
	processDelta();
	
}

void Klayman::suSneaking() {
	
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
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdiff;
		if (_pathPoints) {
			walkAlongPathPoints();
		} else {
			HitRect *hitRectNext = _vm->_collisionMan->findHitRectAtPos(_x, _y);
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
		processDelta();
	}
	
}

void Klayman::stSneak() {
	_status2 = 1;
	_isSneaking = true;
	_acceptInput = true;
	setDoDeltaX(_destX < _x ? 1 : 0);
	startAnimation(0x5C48C506, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmSneaking);
	SetSpriteUpdate(&Klayman::suSneaking);
	FinalizeState(&Klayman::evSneakingDone);	
}

void Klayman::evSneakingDone() {
	_isSneaking = false;
}

uint32 Klayman::hmSneaking(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stStartWalking() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stStartWalking))) {
		_status2 = 0;
		_isWalking = true;
		_acceptInput = true;
		setDoDeltaX(_destX < _x ? 1 : 0);
		startAnimation(0x242C0198, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmStartWalking);
		SetSpriteUpdate(&Klayman::suWalkingTestExit);
		NextState(&Klayman::stWalkingFirst);
		FinalizeState(&Klayman::evStartWalkingDone);	
	}
}

void Klayman::evStartWalkingDone() {
	_isWalking = false;
}

uint32 Klayman::hmStartWalking(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stWalkingFirst() {
	_status2 = 0;
	_isWalking = true;
	_acceptInput = true;
	startAnimation(0x1A249001, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalking);
	SetSpriteUpdate(&Klayman::suWalkingFirst);
	NextState(&Klayman::stUpdateWalkingFirst);
	FinalizeState(&Klayman::evStartWalkingDone);	
}

void Klayman::suWalkingFirst() {
	SetSpriteUpdate(&Klayman::suWalkingTestExit);
	_deltaX = 0;
}

uint32 Klayman::hmWalking(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stUpdateWalkingFirst() {
	if (_status3 == 2) {
		gotoNextStateExt();
	} else if (_status3 == 3) {
		stWalkingOpenDoor();
	} else {
		_isSneaking = true;
		_acceptInput = true;
		if (ABS(_destX - _x) <= 42 && _currFrameIndex >= 5 && _currFrameIndex <= 11) {
			if (_status3 == 0) {
				_status2 = 1;
				startAnimation(0xF234EE31, 0, -1);
			} else {
				_status2 = 2;
				startAnimation(0xF135CC21, 0, -1);
			}
		} else if (ABS(_destX - _x) <= 10 && (_currFrameIndex >= 12 || _currFrameIndex <= 4)) {
			if (_status3 == 0) {
				_status2 = 1;
				startAnimation(0x8604A152, 0, -1);
			} else {
				_status2 = 2;
				startAnimation(0xA246A132, 0, -1);
			}
		}
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmSneaking);
		SetSpriteUpdate(&Klayman::suSneaking);
		FinalizeState(&Klayman::evSneakingDone);	
	}
}

void Klayman::suWalkingTestExit() {
	int16 xdiff = ABS(_destX - _x);
	int16 xdelta = _destX - _x;
	
	if (xdelta > _deltaX)
		xdelta = _deltaX;
	else if (xdelta < -_deltaX)
		xdelta = -_deltaX;
		
	_deltaX = 0;		

	if (xdiff == 0 ||
		(_status3 != 2 && _status3 != 3 && xdiff <= 42 && _currFrameIndex >= 5 && _currFrameIndex <= 11) ||
		(_status3 != 2 && _status3 != 3 && xdiff <= 10 && (_currFrameIndex >= 12 || _currFrameIndex <= 4)) ||
		(_status3 == 3 && xdiff < 30) ||
		(_status3 == 3 && xdiff < 150 && _currFrameIndex >= 6)) {
		sendMessage(this, 0x1019, 0);
	} else {
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdelta;
		if (_pathPoints) {
			walkAlongPathPoints();
		} else {
			HitRect *hitRectNext = _vm->_collisionMan->findHitRectAtPos(_x, _y);
			if (hitRectNext->type == 0x5002) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (hitRectNext->rect.x2 - _x) / 2);
			} else if (hitRectNext->type == 0x5003) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (_x - hitRectNext->rect.x1) / 2);
			} else if (hitRectPrev->type == 0x5002) {
				_y = xdelta > 0 ? hitRectPrev->rect.y2 : hitRectPrev->rect.y1;
			} else if (hitRectPrev->type == 0x5003) {
				_y = xdelta < 0 ? hitRectPrev->rect.y2 : hitRectPrev->rect.y1;
			} else if (_flagF6 && xdelta != 0) {
				if (hitRectNext->type == 0x5000) {
					_y++;
				} else if (hitRectNext->type == 0x5001 && _y > hitRectNext->rect.y1) {
					_y--;
				}
			}
		}
		processDelta();
	}
	
}

uint32 Klayman::hmLever(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stPickUpGeneric() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::stPickUpGeneric))) {
		_status2 = 1;
		_acceptInput = false;
		startAnimation(0x1C28C178, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPickUpObject);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klayman::hmPickUpObject(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stPressButton() {
	if (!stStartAction(AnimationCallback(&Klayman::stPressButton))) {
		_status2 = 2;
		_acceptInput = true;
		startAnimation(0x1C02B03D, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPressButton);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klayman::hmPressButton(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stPressFloorButton() {
	if (!stStartAction(AnimationCallback(&Klayman::stPressFloorButton))) {
		_status2 = 2;
		_acceptInput = true;
		startAnimation(0x1C16B033, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPressButton);
		SetSpriteUpdate(NULL);
	}
}

void Klayman::stPressButtonSide() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stPressButtonSide))) {
		_status2 = 1;
		_acceptInput = true;
		startAnimation(0x1CD89029, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPressButton);
		SetSpriteUpdate(&Klayman::suAction);
	}
}

void Klayman::startSpecialWalkRight(int16 x) {
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

void Klayman::startSpecialWalkLeft(int16 x) {
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

void Klayman::startWalkToXSmall(int16 x) {
	_status3 = 2;
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
		GotoState(&Klayman::stStartWalkingSmall);
	}
}

void Klayman::stStartWalkingSmall() {
	_isWalking = true;
	_acceptInput = true;
	_status3 = 2;
	setDoDeltaX(_destX < _x ? 1 : 0);
	startAnimation(0x3A4CD934, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkingSmall);
	SetSpriteUpdate(&Klayman::suWalkingTestExit);
	FinalizeState(&Klayman::evStartWalkingDone);
}

uint32 Klayman::hmWalkingSmall(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stStandIdleSmall() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x90D0D1D0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
}

void Klayman::stWonderAboutAfterSmall() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x11C8D156, 30, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stWonderAboutHalfSmall() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x11C8D156, 0, 10);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stWonderAboutSmall() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x11C8D156, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stWalkToFrontNoStepSmall() {
	_status2 = 0;
	_acceptInput = false;
	startAnimationByHash(0x3F9CC394, 0x14884392, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

uint32 Klayman::hmWalkFrontBackSmall(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stWalkToFront2Small() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x2F1C4694, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stWalkToFrontSmall() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x3F9CC394, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stTurnToBackHalfSmall() {
	_status2 = 0;
	_acceptInput = false;
	startAnimationByHash(0x37ECD436, 0, 0x8520108C);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stTurnToBackWalkSmall() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x16EDDE36, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stTurnToBackSmall() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x37ECD436, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkFrontBackSmall);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stPullCord() {
	if (!stStartAction(AnimationCallback(&Klayman::stPullCord))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x3F28E094, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPullReleaseCord);
		SetSpriteUpdate(NULL);
		NextState(&Klayman::stReleaseCord);
	}
}

void Klayman::stReleaseCord() {
	_acceptInput = false;
	startAnimation(0x3A28C094, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmPullReleaseCord);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmPullReleaseCord(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stUseTube() {
	if (!stStartAction(AnimationCallback(&Klayman::stUseTube))) {
		_status2 = 1;
		_acceptInput = false;
		startAnimation(0x1A38A814, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmUseTube);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klayman::hmUseTube(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stWalkingFirstExt() {
	_status2 = 0;
	_isWalking = true;
	_acceptInput = true;
	startAnimation(0x5A2CBC00, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalking);
	SetSpriteUpdate(&Klayman::suWalkingFirst);
	NextState(&Klayman::stUpdateWalkingFirst);
	FinalizeState(&Klayman::evStartWalkingDone);
}

void Klayman::stStartWalkingExt() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stStartWalkingExt))) {
		_status2 = 0;
		_isWalking = true;
		_acceptInput = true;
		setDoDeltaX(_destX < _x ? 1 : 0);
		startAnimation(0x272C1199, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmStartWalking);
		SetSpriteUpdate(&Klayman::suWalkingTestExit);
		NextState(&Klayman::stWalkingFirstExt);
		FinalizeState(&Klayman::evStartWalkingDone);
	}
}

void Klayman::sub41CC40(int16 x1, int16 x2) {
	if (_x > x1) {
		if (_x == x1 + x2) {
			_destX = x1 + x2;
			gotoState(NULL);
			gotoNextStateExt();
		} else if (_x < x1 + x2) {
			startWalkToXExt(x1 + x2);
		} else {
			startWalkToX(x1 + x2, false);
		}
	} else {
		if (_x == x1 - x2) {
			_destX = x1 - x2;
			gotoState(NULL);
			gotoNextStateExt();
		} else if (_x > x1 - x2) {
			startWalkToXExt(x1 - x2);
		} else {
			startWalkToX(x1 - x2, false);
		}
	}
}

void Klayman::startWalkToXExt(int16 x) {
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
	} else if (xdiff <= 42 && _status3 != 3) {
		if (_isSneaking && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_destX - _x) > xdiff) {
			_destX = x;
		} else {
			_destX = x;
			GotoState(&Klayman::stSneak);
		}
	} else if (_isLargeStep && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_destX = x;
	} else {
		_destX = x;
		GotoState(&Klayman::stLargeStep);
	}
}

void Klayman::stLargeStep() {
	_status2 = 2;
	_isLargeStep = true;
	_acceptInput = true;
	setDoDeltaX(_destX >= _x ? 1 : 0);
	startAnimation(0x08B28116, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLargeStep);
	SetSpriteUpdate(&Klayman::suLargeStep);
	FinalizeState(&Klayman::evLargeStepDone);	
}

void Klayman::evLargeStepDone() {
	_isLargeStep = false;
}

void Klayman::suLargeStep() {
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
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdiff;
		if (_pathPoints) {
			walkAlongPathPoints();
		} else {
			HitRect *hitRectNext = _vm->_collisionMan->findHitRectAtPos(_x, _y);
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
		processDelta();
	}
}

uint32 Klayman::hmLargeStep(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stWonderAboutHalf() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, 10);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stWonderAboutAfter() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0xD820A114, 30, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stTurnToUseHalf() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x9B250AD2, 0, 7);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmTurnToUse);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmTurnToUse(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stTurnAwayFromUse() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x98F88391, 4, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmTurnToUse);
	SetSpriteUpdate(NULL);
}

void Klayman::stWonderAbout() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stPeekWall() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0xAC20C012, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmPeekWall);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmPeekWall(int messageNum, const MessageParam &param, Entity *sender) {
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymanSpeedUpHash);
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

void Klayman::stJumpToRing1() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpToRing1))) {
		_status2 = 0;
		startAnimation(0xD82890BA, 0, -1);
		setupJumpToRing();
	}
}

void Klayman::setupJumpToRing() {
	_acceptInput = false;
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmJumpToRing);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
	NextState(&Klayman::stHangOnRing);
	sendMessage(_attachedSprite, 0x482B, 0);
}

uint32 Klayman::hmJumpToRing(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::suUpdateDestX() {
	AnimatedSprite::updateDeltaXY();
	_destX = _x;
}

void Klayman::stHangOnRing() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x4829E0B8, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
}

void Klayman::stJumpToRing2() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpToRing2))) {
		_status2 = 0;
		startAnimation(0x900980B2, 0, -1);
		setupJumpToRing();
	}
}

void Klayman::stJumpToRing3() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpToRing3))) {
		_status2 = 0;
		_acceptInput = false;
		startAnimation(0xBA1910B2, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
		SetMessageHandler(&Klayman::hmJumpToRing3);
		NextState(&Klayman::stHoldRing3);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

uint32 Klayman::hmJumpToRing3(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stHoldRing3() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x4A293FB0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmHoldRing3);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmHoldRing3(int messageNum, const MessageParam &param, Entity *sender) {
	if (messageNum == 0x1008) {
		stReleaseRing();
		return 0;
	}
	return hmLowLevel(messageNum, param, sender);
}

void Klayman::stReleaseRing() {
	_status2 = 1;
	_acceptInput = false;
	sendMessage(_attachedSprite, 0x4807, 0);
	_attachedSprite = NULL;
	startAnimation(0xB869A4B9, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
}

void Klayman::stJumpToRing4() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpToRing4))) {
		_status2 = 0;
		startAnimation(0xB8699832, 0, -1);
		setupJumpToRing();
	}
}

void Klayman::sub41CCE0(int16 x) {
	sub41CC40(_attachedSprite->getX(), x);
}

void Klayman::stContinueClimbLadderUp() {
	_status2 = 0;
	_acceptInput = true;
	_ladderStatus = 3;
	startAnimationByHash(0x3A292504, 0x01084280, 0);
	_newStickFrameHash = 0x01084280;
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
	gotoNextStateExt();
}

void Klayman::stStartClimbLadderDown() {
	if (!stStartAction(AnimationCallback(&Klayman::stStartClimbLadderDown))) {
		_status2 = 0;
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
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klayman::suUpdateDestX);
		} else if (_ladderStatus == 3) {
			_ladderStatus = 2;
			_acceptInput = true;
			startAnimationByHash(0x122D1505, 0x01084280, 0);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klayman::suUpdateDestX);
		} else if (_ladderStatus == 1) {
			_ladderStatus = 2;
			_acceptInput = true;
			startAnimation(0x122D1505, 29 - _currFrameIndex, -1);
		} 
	}
}

void Klayman::stClimbLadderHalf() {
	_status2 = 2;
	if (_ladderStatus == 1) {
		_ladderStatus = 0;
		_acceptInput = false;
		startAnimationByHash(0x3A292504, 0x02421405, 0);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmClimbLadderHalf);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	} else if (_ladderStatus == 2) {
		_ladderStatus = 0;
		_acceptInput = false;
		startAnimationByHash(0x122D1505, 0x02421405, 0);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmClimbLadderHalf);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	} else {
		gotoNextStateExt();
	}
}

uint32 Klayman::hmClimbLadderHalf(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmClimbLadderUpDown(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stStartClimbLadderUp() {
	if (!stStartAction(AnimationCallback(&Klayman::stStartClimbLadderUp))) {
		_status2 = 0;
		if (_destY >= _y - 30) {
			gotoNextStateExt();
		} else if (_ladderStatus == 0) {
			_ladderStatus = 1;
			_acceptInput = false;
			startAnimation(0x3A292504, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klayman::suUpdateDestX);
		} else if (_ladderStatus == 3) {
			_ladderStatus = 1;
			_acceptInput = true;
			startAnimationByHash(0x3A292504, 0x01084280, 0);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmClimbLadderUpDown);
			SetSpriteUpdate(&Klayman::suUpdateDestX);
		} else if (_ladderStatus == 2) {
			_ladderStatus = 1;
			_acceptInput = true;
			startAnimation(0x3A292504, 29 - _currFrameIndex, -1);
		}
	}
}

void Klayman::stWalkToFrontNoStep() {
	_status2 = 2;
	_acceptInput = false;
	startAnimationByHash(0xF229C003, 0x14884392, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalkToFront);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

uint32 Klayman::hmWalkToFront(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stWalkToFront() {
	if (!stStartAction(AnimationCallback(&Klayman::stWalkToFront))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0xF229C003, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmWalkToFront);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stTurnToFront() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnToFront))) {
		_status2 = 0;
		_acceptInput = false;
		startAnimationByHash(0xCA221107, 0x8520108C, 0);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmWalkToFront);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stTurnToBack() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnToBack))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0xCA221107, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmWalkToFront);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stLandOnFeet() {
	_status2 = 1;
	_acceptInput = true;
	startAnimation(0x18118554, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLandOnFeet);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmLandOnFeet(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stTurnToBackToUse() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnToBackToUse))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x91540140, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmTurnToBackToUse);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

uint32 Klayman::hmTurnToBackToUse(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stClayDoorOpen() {
	if (!stStartAction(AnimationCallback(&Klayman::stClayDoorOpen))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x5CCCB330, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmClayDoorOpen);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

uint32 Klayman::hmClayDoorOpen(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stTurnToUse() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnToUse))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x9B250AD2, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmTurnToUse);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stReturnFromUse() {
	_status2 = 2;
	_acceptInput = false;
	startAnimation(0x98F88391, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmTurnToUse);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stWalkingOpenDoor() {
	_isWalkingOpenDoorNotified = false;
	_acceptInput = false;
	startAnimation(0x11A8E012, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmStartWalking);
	SetSpriteUpdate(&Klayman::suWalkingOpenDoor);
}

void Klayman::suWalkingOpenDoor() {
	if (!_isWalkingOpenDoorNotified && ABS(_destX - _x) < 80) {
		sendMessage(_parentScene, 0x4829, 0);
		_isWalkingOpenDoorNotified = true;
	}
	AnimatedSprite::updateDeltaXY();
}

void Klayman::stMoveObjectSkipTurnFaceObject() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	_isMoveObjectRequested = false;
	_acceptInput = true;
	startAnimationByHash(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
	SetMessageHandler(&Klayman::hmMoveObjectTurn);
}

void Klayman::evMoveObjectTurnDone() {
	sendMessage(_attachedSprite, 0x4807, 0);
}

uint32 Klayman::hmMoveObjectTurn(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			sendMessage(_attachedSprite, 0x480B, _doDeltaX ? 1 : 0);
		} else if (param.asInteger() == 0x02421405) {
			if (_isMoveObjectRequested && sendMessage(_attachedSprite, 0x480C, _doDeltaX ? 1 : 0) != 0) {
				stMoveObjectSkipTurn();
			} else {
				FinalizeState(&Klayman::evMoveObjectTurnDone);
				SetMessageHandler(&Klayman::hmLowLevelAnimation);
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

void Klayman::stMoveObjectSkipTurn() {
	_isMoveObjectRequested = false;
	_acceptInput = true;
	startAnimationByHash(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
	SetMessageHandler(&Klayman::hmMoveObjectTurn);
}

void Klayman::stMoveObjectFaceObject() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::stMoveObjectFaceObject))) {
		_status2 = 2;
		_isMoveObjectRequested = false;
		_acceptInput = true;
		startAnimation(0x0C1CA072, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmMoveObjectTurn);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stUseLever() {
	if (!stStartAction(AnimationCallback(&Klayman::stUseLever))) {
		_status2 = 0;
		if (_isLeverDown) {
			stUseLeverRelease();
		} else {
			sendMessage(_attachedSprite, 0x482B, 0);
			startAnimation(0x0C303040, 0, -1);
			SetSpriteUpdate(&Klayman::suUpdateDestX);
			SetMessageHandler(&Klayman::hmLever);
			SetUpdateHandler(&Klayman::update);
			NextState(&Klayman::stPullLeverDown);
			_acceptInput = false;
		}
	}
}

// Exactly the same code as sub420DA0 which was removed
void Klayman::stPullLeverDown() {
	startAnimation(0x0D318140, 0, -1);
	sendMessage(_attachedSprite, 0x480F, 0);
	NextState(&Klayman::stHoldLeverDown);
}

void Klayman::stHoldLeverDown() {
	startAnimation(0x4464A440, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
	_isLeverDown = true;
	_acceptInput = true;
}

void Klayman::stUseLeverRelease() {
	startAnimation(0x09018068, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLever);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
	sendMessage(_attachedSprite, 0x4807, 0);
	NextState(&Klayman::stPullLeverDown);
	_acceptInput = false;
}

void Klayman::stReleaseLever() {
	if (_isLeverDown) {
		_status2 = 2;
		startAnimation(0x09018068, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmLever);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
		sendMessage(_attachedSprite, 0x4807, 0);
		NextState(&Klayman::stLetGoOfLever);
		_acceptInput = false;
		_isLeverDown = false;
	} else {
		gotoNextStateExt();
	}
}

void Klayman::stLetGoOfLever() {
	startAnimation(0x0928C048, 0, -1);
	FinalizeState(&Klayman::evLeverReleasedEvent);
}

void Klayman::evLeverReleasedEvent() {
	sendMessage(_attachedSprite, 0x482A, 0);
}

void Klayman::stInsertDisk() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stInsertDisk))) {
		_status2 = 2;
		_tapesToInsert = 0;
		for (uint32 i = 0; i < 20; i++) {
			if (getSubVar(0x02038314, i)) {
				setSubVar(0x02720344, i, 1);
				setSubVar(0x02038314, i, 0);
				_tapesToInsert++;
			}
		}
		if (_tapesToInsert == 0) {
			GotoState(NULL);
			gotoNextStateExt();
		} else {
			startAnimation(0xD8C8D100, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmInsertDisk);
			SetSpriteUpdate(&Klayman::suAction);
			_acceptInput = false;
			_tapesToInsert--;
		}
	}
}

uint32 Klayman::hmInsertDisk(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::hmLowLevelAnimation(messageNum, param, sender);
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

void Klayman::walkAlongPathPoints() {
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

void Klayman::stJumpToGrab() {
	_status2 = 0;
	_acceptInput = false;
	startAnimationByHash(0x00AB8C10, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(&Klayman::suJumpToGrab);
	SetMessageHandler(&Klayman::hmJumpToGrab);
}

void Klayman::suJumpToGrab() {
	updateDeltaXY();
	if (_y >= _destY) {
		_y = _destY;
		processDelta();
		gotoNextStateExt();
	}
}

uint32 Klayman::hmJumpToGrab(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::sub421230() {//stGrow
	_status2 = 2;
	_acceptInput = false;
	startAnimationByHash(0x38445000, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klayman::handleMessage41F1D0);
}

uint32 Klayman::handleMessage41F1D0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x040C4C01)
			playSound(0, 0x01E11140);
		break;
	}
	return messageResult;
}

void Klayman::stTurnToUseExt() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnToUseExt))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x1B3D8216, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmTurnToUse);
		SetSpriteUpdate(&Klayman::suUpdateDestX);
	}
}

void Klayman::stJumpToGrabFall() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpToGrabFall))) {
		_status2 = 0;
		_acceptInput = false;
		startAnimation(0x00AB8C10, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmJumpToGrab);
		SetSpriteUpdate(&Klayman::suJumpToGrab);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

void Klayman::stJumpToGrabRelease() {
	_status2 = 1;
	_acceptInput = false;
	startAnimationByHash(0x00AB8C10, 0x320AC306, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmJumpToGrabRelease);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stReleaseRing);
}

uint32 Klayman::hmJumpToGrabRelease(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x320AC306)
			playSound(0, 0x5860C640);
		break;
	}
	return messageResult;
}

void Klayman::stDoIdleTeleporterHands() {
	startIdleAnimation(0x90EF8D38, AnimationCallback(&Klayman::stIdleTeleporterHands));
}

void Klayman::stIdleTeleporterHands() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x90EF8D38, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stSitIdleTeleporterBlinkSecond);
}

void Klayman::stDoIdleTeleporterHands2() {
	startIdleAnimation(0x900F0930, AnimationCallback(&Klayman::stIdleTeleporterHands2));
}

void Klayman::stIdleTeleporterHands2() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0x900F0930, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stSitIdleTeleporterBlinkSecond);
}

void Klayman::teleporterAppear(uint32 fileHash) {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(fileHash, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmTeleporterAppearDisappear);
	SetSpriteUpdate(NULL);
}

void Klayman::teleporterDisappear(uint32 fileHash) {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(fileHash, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmTeleporterAppearDisappear);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmTeleporterAppearDisappear(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmShrink(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stShrink() {
	_status2 = 0;
	_acceptInput = false;
	playSound(0, 0x4C69EA53);
	startAnimation(0x1AE88904, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmShrink);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

void Klayman::stStandWonderAbout() {
	if (_x > 260)
		setDoDeltaX(1);
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0xD820A114, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(NULL);
	_newStickFrameIndex = 10;
}

uint32 Klayman::hmDrinkPotion(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmGrow(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stGrow() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x2838C010, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmGrow);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

void Klayman::stDrinkPotion() {
	_status2 = 1;
	_acceptInput = false;
	_potionFlag1 = false;
	_potionFlag2 = false;
	startAnimation(0x1C388C04, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmDrinkPotion);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 Klayman::hmInsertKey(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::hmLowLevelAnimation(messageNum, param, sender);
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

void Klayman::stInsertKey() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stInsertKey))) {
		_status2 = 2;
		_keysToInsert = 0;
		for (uint32 i = 0; i < 3; i++) {
			if (getSubVar(0x0090EA95, i)) {
				bool more;
				setSubVar(0x08D0AB11, i, 1);
				setSubVar(0x0090EA95, i, 0);
				do {
					more = false;
					setSubVar(0xA010B810, i, _vm->_rnd->getRandomNumber(16 - 1));
					for (uint j = 0; j < i && !more; j++) {
						if (getSubVar(0x08D0AB11, j) && getSubVar(0xA010B810, j) == getSubVar(0xA010B810, i))
							more = true;
					}
					if (getSubVar(0xA010B810, i) == getSubVar(0x0C10A000, i))
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
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmInsertKey);
			SetSpriteUpdate(&Klayman::suAction);
			_keysToInsert--;
		}
	}
}

uint32 Klayman::hmReadNote(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::stReadNote() {
	_status2 = 2;
	_acceptInput = false;
	startAnimation(0x123E9C9F, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmReadNote);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 Klayman::hmHitByDoor(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymanSpeedUpHash);
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

void Klayman::stHitByDoor() {
	_status2 = 1;
	_acceptInput = false;
	startAnimation(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmHitByDoor);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	playSound(0, 0x402E82D4);
}

uint32 Klayman::hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::upPeekWallBlink() {
	Klayman::update();
	_blinkCounter++;
	if (_blinkCounter >= _blinkCounterMax)
		stPeekWallBlink();
}

void Klayman::stPeekWall1() {
	_status2 = 0;
	_acceptInput = true;
	startAnimation(0xAC20C012, 8, 37);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stPeekWallBlink);
}

void Klayman::stPeekWall2() {
	_status2 = 1;
	_acceptInput = false;
	startAnimation(0xAC20C012, 43, 49);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
}

void Klayman::stPeekWallBlink() {
	_blinkCounter = 0;
	_status2 = 0;
	_acceptInput = true;
	_blinkCounterMax = _vm->_rnd->getRandomNumber(64) + 24;
	startAnimation(0xAC20C012, 38, 42);
	SetUpdateHandler(&Klayman::upPeekWallBlink);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klayman::hmLowLevel);
	_newStickFrameIndex = 42;
}

void Klayman::stPeekWallReturn() {
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x2426932E, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmPeekWallReturn);
	SetSpriteUpdate(NULL);
}

void Klayman::stPullHammerLever() {
	if (!stStartAction(AnimationCallback(&Klayman::stPullHammerLever))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x00648953, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPullHammerLever);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	}
}

uint32 Klayman::hmPullHammerLever(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::hmLever(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209)
			sendMessage(_attachedSprite, 0x480F, 0);
		break;
	}
	return messageResult;
}

void Klayman::suRidePlatformDown() {
	_platformDeltaY++;
	_y += _platformDeltaY;
	if (_y > 600)
		sendMessage(this, 0x1019, 0);
}

void Klayman::stRidePlatformDown() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stRidePlatformDown))) {
		_status2 = 1;
		sendMessage(_parentScene, 0x4803, 0);
		_acceptInput = false;
		_platformDeltaY = 0;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmLowLevel);
		SetSpriteUpdate(&Klayman::suRidePlatformDown);
		_vm->_soundMan->playSoundLooping(0xD3B02847);
	}
}

void Klayman::stCrashDown() {
	playSound(0, 0x41648271);
	_status2 = 1;
	_acceptInput = false;
	startAnimationByHash(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	NextState(&Klayman::stCrashDownFinished);
}

void Klayman::stCrashDownFinished() {
	setDoDeltaX(2);
	stTryStandIdle();
}

void Klayman::upSpitOutFall() {
	Klayman::update();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		_surface->setVisible(true);
		SetUpdateHandler(&Klayman::update);
	}
}

uint32 Klayman::hmJumpToRingVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmStandIdleSpecial(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmPressDoorButton(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			sendMessage(_attachedSprite, 0x480B, (uint32)_doDeltaX);
		} else if (param.asInteger() == 0x02421405) {
			if (_isMoveObjectRequested) {
				if (sendMessage(_attachedSprite, 0x480C, (uint32)_doDeltaX) != 0)
					stContinueMovingVenusFlyTrap();
			} else {
				SetMessageHandler(&Klayman::hmFirstMoveVenusFlyTrap);
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

uint32 Klayman::hmFirstMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
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

uint32 Klayman::hmHitByBoxingGlove(int messageNum, const MessageParam &param, Entity *sender) {
	int16 speedUpFrameIndex;
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymanSpeedUpHash);
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

uint32 Klayman::hmJumpAndFall(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::suFallDown() {
	AnimatedSprite::updateDeltaXY();
	HitRect *hitRect = _vm->_collisionMan->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		processDelta();
		sendMessage(this, 0x1019, 0);
	}
	_vm->_collisionMan->checkCollision(this, 0xFFFF, 0x4810, 0);
}

void Klayman::stJumpToRingVenusFlyTrap() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpToRingVenusFlyTrap))) {
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0x584984B4, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmJumpToRingVenusFlyTrap);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		NextState(&Klayman::stLandOnFeet);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

void Klayman::stStandIdleSpecial() {
	playSound(0, 0x56548280);
	_status2 = 0;
	_acceptInput = false;
	_surface->setVisible(false);
	startAnimation(0x5420E254, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmStandIdleSpecial);
	SetSpriteUpdate(NULL);
}

void Klayman::stSpitOutFall0() {
	_countdown1 = 1;
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x000BAB02, 0, -1);
	SetUpdateHandler(&Klayman::upSpitOutFall);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(&Klayman::suFallDown);
	NextState(&Klayman::stFalling);
	sendMessage(_parentScene, 0x8000, 0);
}

void Klayman::stSpitOutFall2() {
	_countdown1 = 1;
	_status2 = 0;
	_acceptInput = false;
	startAnimation(0x9308C132, 0, -1);
	SetUpdateHandler(&Klayman::upSpitOutFall);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	SetSpriteUpdate(&Klayman::suFallDown);
	NextState(&Klayman::stFalling);
	sendMessage(_parentScene, 0x8000, 0);
}

void Klayman::stFalling() {
	sendMessage(_parentScene, 0x1024, 1);
	playSound(0, 0x41648271);
	_status2 = 1;
	_acceptInput = false;
	_isWalking = false;
	startAnimationByHash(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&Klayman::hmLowLevelAnimation);
	NextState(&Klayman::stFallTouchdown);
	sendMessage(_parentScene, 0x2002, 0);
	// TODO _callbackList = NULL; (See comment above)
	_attachedSprite = NULL;
	sendMessage(_parentScene, 0x8001, 0);
}

void Klayman::stFallTouchdown() {
	setDoDeltaX(2);
	stTryStandIdle();
}

void Klayman::stJumpAndFall() {
	if (!stStartAction(AnimationCallback(&Klayman::stJumpAndFall))) {
		sendMessage(_parentScene, 0x1024, 3);
		_status2 = 2;
		_acceptInput = false;
		startAnimation(0xB93AB151, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmJumpAndFall);
		SetSpriteUpdate(&Klayman::suFallDown);
		NextState(&Klayman::stLandOnFeet);
	}
}

void Klayman::stDropFromRing() {
	if (_attachedSprite) {
		_x = _attachedSprite->getX();
		sendMessage(_attachedSprite, 0x4807, 0);
		_attachedSprite = NULL;
	}
	_status2 = 2;
	_acceptInput = false;
	startAnimation(0x586984B1, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(&Klayman::suFallDown);
	NextState(&Klayman::stLandOnFeet);
}

void Klayman::stPressDoorButton() {
	_status2 = 2;
	_acceptInput = true;
	setDoDeltaX(0);
	startAnimation(0x1CD89029, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmPressDoorButton);
	SetSpriteUpdate(&Klayman::suAction);
}

void Klayman::stHitByBoxingGlove() {
	_status2 = 1;
	_acceptInput = false;
	startAnimation(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmHitByBoxingGlove);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	FinalizeState(&Klayman::evHitByBoxingGloveDone);
}

void Klayman::evHitByBoxingGloveDone() {
	sendMessage(_parentScene, 0x1024, 1);
}

void Klayman::stMoveVenusFlyTrap() {
	if (!stStartAction(AnimationCallback(&Klayman::stMoveVenusFlyTrap))) {
		_status2 = 2;
		_isMoveObjectRequested = false;
		_acceptInput = true;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		startAnimation(0x5C01A870, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmMoveVenusFlyTrap);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		FinalizeState(&Klayman::evMoveVenusFlyTrapDone);
	}
}

void Klayman::stContinueMovingVenusFlyTrap() {
	_isMoveObjectRequested = false;
	_acceptInput = true;
	startAnimationByHash(0x5C01A870, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmMoveVenusFlyTrap);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	FinalizeState(&Klayman::evMoveVenusFlyTrapDone);
}

void Klayman::evMoveVenusFlyTrapDone() {
	sendMessage(_attachedSprite, 0x482A, 0);
}

void Klayman::suFallSkipJump() {
	updateDeltaXY();
	HitRect *hitRect = _vm->_collisionMan->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		processDelta();
		sendMessage(this, 0x1019, 0);
	}
}

void Klayman::stFallSkipJump() {
	_status2 = 2;
	_acceptInput = false;
	startAnimationByHash(0xB93AB151, 0x40A100F8, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(&Klayman::suFallSkipJump);
	SetMessageHandler(&Klayman::hmLowLevel);
	NextState(&Klayman::stLandOnFeet);
}

void Klayman::upMoveObject() {
	if (_x >= 380)
		gotoNextStateExt();
	Klayman::update();		
}

uint32 Klayman::hmMatch(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x51281850) {
			setGlobalVar(0x20A0C516, 1);
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

void Klayman::stFetchMatch() {
	if (!stStartAction(AnimationCallback(&Klayman::stFetchMatch))) {
		_status2 = 0;
		_acceptInput = false;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		startAnimation(0x9CAA0218, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmMatch);
		SetSpriteUpdate(NULL);
		NextState(&Klayman::stLightMatch);
	}
}

void Klayman::stLightMatch() {
	_status2 = 1;
	_acceptInput = false;
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	startAnimation(0x1222A513, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmMatch);
	SetSpriteUpdate(NULL);
}

uint32 Klayman::hmMoveObject(int messageNum, const MessageParam &param, Entity *sender) {
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
				SetMessageHandler(&Klayman::hmLowLevelAnimation);
			}
		}
		break;
	}
	return Klayman::hmLowLevelAnimation(messageNum, param, sender);
}

uint32 Klayman::hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000F0082) {
			playSound(0, 0x74E2810F);
		}
		break;
	}
	return messageResult;
}

void Klayman::stMoveObject() {
	if (!stStartAction(AnimationCallback(&Klayman::stMoveObject))) {
		_status2 = 2;
		_acceptInput = false;
		_moveObjectCountdown = 8;
		setDoDeltaX(0);
		startAnimation(0x0C1CA072, 0, -1);
		SetUpdateHandler(&Klayman::upMoveObject);
		SetMessageHandler(&Klayman::hmMoveObject);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	}
}

void Klayman::stContinueMoveObject() {
	_acceptInput = false;
	startAnimationByHash(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klayman::upMoveObject);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	SetMessageHandler(&Klayman::hmMoveObject);
}

void Klayman::stTumbleHeadless() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stTumbleHeadless))) {
		_status2 = 1;
		_acceptInput = false;
		setDoDeltaX(0);
		startAnimation(0x2821C590, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmTumbleHeadless);
		SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
		NextState(&Klayman::stTryStandIdle);
		sendMessage(_parentScene, 0x8000, 0);
		playSound(0, 0x62E0A356);
	}
}

void Klayman::stCloseEyes() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stCloseEyes))) {
		_status2 = 1;
		_acceptInput = false;		
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmLowLevel);
		SetSpriteUpdate(NULL);
	}
}

uint32 Klayman::hmSpit(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::hmLowLevelAnimation(messageNum, param, sender);
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

void Klayman::stTrySpitIntoPipe() {
	if (_readyToSpit) {
		_contSpitPipe = true;
		_spitContDestPipeIndex = _spitPipeIndex;
		if (_canSpitPipe)
			spitIntoPipe();
	} else if (!stStartAction(AnimationCallback(&Klayman::stTrySpitIntoPipe))) {
		_status2 = 2;
		_acceptInput = true;
		_spitDestPipeIndex = _spitPipeIndex;
		_readyToSpit = true;
		_canSpitPipe = false;
		_contSpitPipe = false;
		startAnimation(0x1808B150, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmSpit);
		SetSpriteUpdate(NULL);
	}
}

void Klayman::spitIntoPipe() {
	_contSpitPipe = false;
	_spitDestPipeIndex = _spitContDestPipeIndex;
	_canSpitPipe = false;
	_acceptInput = false;
	startAnimation(0x1B08B553, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmSpit);
	SetSpriteUpdate(NULL);
	NextState(&Klayman::stContSpitIntoPipe);
}

void Klayman::stContSpitIntoPipe() {
	_canSpitPipe = true;
	_acceptInput = true;
	startAnimationByHash(0x1808B150, 0x16401CA6, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmSpit);
	SetSpriteUpdate(NULL);
}

void Klayman::suRidePlatform() {
	_x = _attachedSprite->getX() - 20;
	_y = _attachedSprite->getY() + 46;
	processDelta();
}

void Klayman::stRidePlatform() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stRidePlatform))) {
		_status2 = 1;
		_acceptInput = true;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmLowLevel);
		SetSpriteUpdate(&Klayman::suRidePlatform);
	}
}

void Klayman::stInteractLever() {
	if (!stStartAction(AnimationCallback(&Klayman::stInteractLever))) {
		_status2 = 0;
		if (_isLeverDown) {
			stUseLeverRelease();
		} else {
			_acceptInput = false;
			startAnimation(0x0C303040, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::hmLever);
			SetSpriteUpdate(&Klayman::suUpdateDestX);
			NextState(&Klayman::stPullLever);
		}
	}
}

void Klayman::stPullLever() {
	startAnimation(0x0D318140, 0, -1);
	NextState(&Klayman::stLookLeverDown);
	sendMessage(_attachedSprite, 0x480F, 0);
}

void Klayman::stLookLeverDown() {
	_acceptInput = true;
	_isLeverDown = true;
	startAnimation(0x1564A2C0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
	NextState(&Klayman::stWaitLeverDown);
}

void Klayman::stWaitLeverDown() {
	_acceptInput = true;
	_isLeverDown = true;
	startAnimation(0x4464A440, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLowLevel);
	SetSpriteUpdate(&Klayman::suUpdateDestX);
}

void Klayman::stStartWalkingResume() {
	int16 frameIndex = getGlobalVar(0x18288913) + _walkResumeFrameIncr;
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_status2 = 0;
	_isWalking = true;
	_acceptInput = true;
	startAnimation(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmWalking);
	SetSpriteUpdate(&Klayman::suWalkingFirst);
	NextState(&Klayman::stUpdateWalkingFirst);
	FinalizeState(&Klayman::evStartWalkingDone);
}

//##############################################################################

// KmScene1001

KmScene1001::KmScene1001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
}

uint32 KmScene1001::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x4804:
		if (param.asInteger() == 2)
			GotoState(&Klayman::stSleeping);
		break;
	case 0x480D:
		GotoState(&Klayman::stPullHammerLever);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		// TODO: It's not really a point but an x1/x2 pair
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4836:
		if (param.asInteger() == 1) {
			sendMessage(_parentScene, 0x2002, 0);
			GotoState(&Klayman::stWakeUp);
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

KmScene1002::KmScene1002(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	setKlaymanIdleTable1();
	
}

void KmScene1002::xUpdate() {
	if (_x >= 250 && _x <= 435 && _y >= 420) {
		if (_idleTableNum == 0) {
			// TODO setKlaymanIdleTable(stru_4B44C8);
			_idleTableNum = 1;
		}
	} else if (_idleTableNum == 1) {
		setKlaymanIdleTable1();
		_idleTableNum = 0;
	}
}
	
uint32 KmScene1002::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&Klayman::stStandIdleSpecial);
		break;
	case 0x2007:
		_otherSprite = (Sprite*)param.asEntity();
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004: 
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4803:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stJumpAndFall);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stDropFromRing);
		}
		break;
	case 0x4804:
		GotoState(&Klayman::stPeekWall);
		break;
	case 0x4805:
		switch (param.asInteger()) {
		case 1:
			GotoState(&Klayman::stJumpToRing1);
			break;
		case 2:
			GotoState(&Klayman::stJumpToRing2);
			break;
		case 3:
			GotoState(&Klayman::stJumpToRing3);
			break;
		case 4:
			GotoState(&Klayman::stJumpToRing4);
			break;
		}
		break;
	case 0x480A:	  
		GotoState(&Klayman::stMoveVenusFlyTrap);
		break;
	case 0x480D:			   
		GotoState(&Klayman::stJumpToRingVenusFlyTrap);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:  
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressDoorButton);
		}
		break;
	case 0x4817:				  
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:				
		sub41CCE0(param.asInteger());
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2005, 0);
		GotoState(&Klayman::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2005, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2005, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2006, 0);
		GotoState(&Klayman::stClimbLadderHalf);	 
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
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

// KmScene1004

KmScene1004::KmScene1004(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	_dataResource.load(0x01900A04);	
}

uint32 KmScene1004::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481E:
		GotoState(&Klayman::stReadNote);
		break;
	case 0x4820:
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klayman::stContinueClimbLadderUp);
		break;
	case 0x4821:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderDown);
		break;
	case 0x4822:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderUp);
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klayman::stClimbLadderHalf);
		break;
	case 0x4824:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klayman::stStartClimbLadderDown);
		break;
	case 0x4825:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klayman::stStartClimbLadderUp);
		break;
	case 0x4828:
		GotoState(&Klayman::stTurnToBackToUse);
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

KmScene1109::KmScene1109(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty
}

uint32 KmScene1109::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stSitIdleTeleporter);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stWalkingFirst);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
		break;
	case 0x483D:
		teleporterAppear(0x2C2A4A1C);
		break;
	case 0x483E:
		teleporterDisappear(0x3C2E4245);
		break;
	}
	return 0;
}

// KmScene1201

KmScene1201::KmScene1201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// TODO setKlaymanIdleTable(dword_4AEF10, 3);
	_flagF6 = true;
	
}

uint32 KmScene1201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480A:
		GotoState(&Klayman::stMoveObject);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4813:
		GotoState(&Klayman::stFetchMatch);
		break;
	case 0x4814:
		GotoState(&Klayman::stTumbleHeadless);
		break;
	case 0x4815:
		GotoState(&Klayman::stCloseEyes);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x481F:
		GotoState(&Klayman::stWonderAbout);
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

KmScene1303::KmScene1303(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty
}

uint32 KmScene1303::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4804:
		GotoState(&Klayman::stPeekWall1);
		break;
	case 0x483B:
		GotoState(&Klayman::stPeekWallReturn);
		break;
	case 0x483C:
		GotoState(&Klayman::stPeekWall2);
		break;
	}
	return 0;
}

KmScene1304::KmScene1304(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty	
}

uint32 KmScene1304::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;						
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
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

KmScene1305::KmScene1305(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	// Empty	
}

uint32 KmScene1305::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x4804:
		GotoState(&Klayman::stCrashDown);
		break;		
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	}
	return 0;
}

KmScene1306::KmScene1306(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
}

uint32 KmScene1306::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stSitIdleTeleporter);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case NM_KLAYMAN_INSERT_DISK:
		GotoState(&Klayman::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		else
			GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		else
			GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
		}
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
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

KmScene1308::KmScene1308(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	// Empty	
}

uint32 KmScene1308::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480A:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stMoveObjectSkipTurnFaceObject);
		} else {
			GotoState(&Klayman::stMoveObjectFaceObject);
		}	
		break;		
	case 0x480D:
		GotoState(&Klayman::stUseLever);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case NM_KLAYMAN_INSERT_DISK:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stInsertKey);		
		} else {
			GotoState(&Klayman::stInsertDisk);		
		}
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case NM_KLAYMAN_RELEASE_LEVER:
		GotoState(&Klayman::stReleaseLever);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
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

KmScene1401::KmScene1401(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty	
}

uint32 KmScene1401::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x480A:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stMoveObjectSkipTurnFaceObject);
		} else {
			GotoState(&Klayman::stMoveObjectFaceObject);
		}	
		break;		
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;						
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
		}
		break;
	}
	return 0;
}

// KmScene1402

KmScene1402::KmScene1402(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	SetFilterY(&Sprite::defFilterY);	
}

uint32 KmScene1402::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x480A:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stMoveObjectSkipTurnFaceObject);
		} else {
			GotoState(&Klayman::stMoveObjectFaceObject);
		}
		break;		
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;						
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	}
	return 0;
}

// KmScene1403

KmScene1403::KmScene1403(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	setKlaymanIdleTable(klaymanTable4, ARRAYSIZE(klaymanTable4));
}

uint32 KmScene1403::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480A:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stMoveObjectSkipTurnFaceObject);
		} else {
			GotoState(&Klayman::stMoveObjectFaceObject);
		}
		break;		
	case 0x480D:
		GotoState(&Klayman::stUseLever);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case NM_KLAYMAN_RELEASE_LEVER:
		GotoState(&Klayman::stReleaseLever);
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

KmScene1404::KmScene1404(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty	
}

uint32 KmScene1404::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480A:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stMoveObjectSkipTurnFaceObject);
		} else {
			GotoState(&Klayman::stMoveObjectFaceObject);
		}
		break;		
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case NM_KLAYMAN_INSERT_DISK:
		GotoState(&Klayman::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
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

KmScene1608::KmScene1608(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
}

uint32 KmScene1608::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2032:
		_isSittingInTeleporter = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stSitIdleTeleporter);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2032, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2032, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
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

// KmScene1705

KmScene1705::KmScene1705(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

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
		if (_isSittingInTeleporter) {
			GotoState(&Klayman::stSitIdleTeleporter);
		} else {
			GotoState(&Klayman::stTryStandIdle);
		}
		break;
	case 0x4803:
		GotoState(&Klayman::stFallSkipJump);
		break;				
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_isSittingInTeleporter) {
			GotoState(&Klayman::stTurnToUseInTeleporter);
		}
		break;
	case 0x481E:
		if (_isSittingInTeleporter) {
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
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

KmScene1901::KmScene1901(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	// Empty	
}

uint32 KmScene1901::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
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

KmScene2001::KmScene2001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	// Empty	
}

uint32 KmScene2001::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter) {
			GotoState(&Klayman::stSitIdleTeleporter);
		} else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stWalkingFirst);
		} else
			GotoState(&Klayman::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter) {
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		}
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
		break;
	case 0x483D:
		teleporterAppear(0xBE68CC54);
		break;
	case 0x483E:
		teleporterDisappear(0x18AB4ED4);
		break;
	}
	return 0;
}

KmScene2101::KmScene2101(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
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
			GotoState(&Klayman::stSitIdleTeleporter);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4811:
		GotoState(&Klayman::stHitByDoor);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)//CHECKME
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
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

KmScene2201::KmScene2201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	_surface->setClipRects(clipRects, clipRectsCount);

	_dataResource.load(0x04104242);
	_flagF6 = false;
}

uint32 KmScene2201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
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

KmScene2203::KmScene2203(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}
	
uint32 KmScene2203::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4819:
		GotoState(&Klayman::stClayDoorOpen);
		break;
	case NM_KLAYMAN_INSERT_DISK:
		GotoState(&Klayman::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
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

KmScene2205::KmScene2205(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmScene2205::xUpdate() {
	setGlobalVar(0x18288913, _currFrameIndex);
}
	
uint32 KmScene2205::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stStartWalkingResume);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide);
		} 
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

KmScene2206::KmScene2206(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	_walkResumeFrameIncr = 1;
	_vm->_soundMan->addSound(0x80101800, 0xD3B02847);
}

KmScene2206::~KmScene2206() {
	_vm->_soundMan->deleteSoundGroup(0x80101800);
}

void KmScene2206::xUpdate() {
	setGlobalVar(0x18288913, _currFrameIndex);
}
	
uint32 KmScene2206::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4803:
		GotoState(&Klayman::stRidePlatformDown);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stStartWalkingResume);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
		}
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

KmScene2207::KmScene2207(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}
	
uint32 KmScene2207::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&Klayman::stRidePlatform);
		break;
	case 0x2005:
		suRidePlatform();
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&Klayman::stInteractLever);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case NM_KLAYMAN_RELEASE_LEVER:
		GotoState(&Klayman::stReleaseLever);
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

KmScene2242::KmScene2242(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmScene2242::xUpdate() {
	setGlobalVar(0x18288913, _currFrameIndex);
}

uint32 KmScene2242::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stStartWalkingResume);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stPickUpTube);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
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

KmHallOfRecords::KmHallOfRecords(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmHallOfRecords::xUpdate() {
	setGlobalVar(0x18288913, _currFrameIndex);
}

uint32 KmHallOfRecords::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stStartWalkingResume);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
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

KmScene2247::KmScene2247(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmScene2247::xUpdate() {
	setGlobalVar(0x18288913, _currFrameIndex);
}

uint32 KmScene2247::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stStartWalkingResume);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
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
  
KmScene2401::KmScene2401(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
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
		GotoState(&Klayman::stTryStandIdle);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
		}
		break;
	case 0x4832:
		GotoState(&Klayman::stUseTube);
		break;
	case 0x4833:
		if (param.asInteger() == 1)
			GotoState(&Klayman::stWonderAbout);
		else {
			_spitPipeIndex = sendMessage(_parentScene, 0x2000, 0);
			GotoState(&Klayman::stTrySpitIntoPipe);
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

KmScene2402::KmScene2402(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
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
		if (!getGlobalVar(0x92603A79))
			GotoState(&Klayman::stStandWonderAbout);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stWalkingFirst);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
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

KmScene2403::KmScene2403(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
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
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&Klayman::stPullCord);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case NM_KLAYMAN_PRESS_BUTTON:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stPressFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klayman::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klayman::stClimbLadderHalf);	 
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
	
KmScene2406::KmScene2406(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
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
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klayman::stWalkingFirst);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klayman::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klayman::stPickUpTube);
		else
			GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481A:
		GotoState(&Klayman::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			sub41CC40(param.asPoint().y, param.asPoint().x);
		else
			sub41CCE0(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klayman::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klayman::stClimbLadderHalf);	 
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
	
KmScene2501::KmScene2501(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
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
			GotoState(&Klayman::stSitIdleTeleporter);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
		break;
	}
	return messageResult;
}

KmScene2801::KmScene2801(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

uint32 KmScene2801::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;						
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
		}
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmScene2803::KmScene2803(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
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
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4803:
		_destY = param.asInteger();
		GotoState(&Klayman::stJumpToGrab);
		break;
	case 0x4804:
		if (param.asInteger() == 3)
			GotoState(&Klayman::sub421230);
		break;
	case 0x480D:
		GotoState(&Klayman::stPullCord);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x481F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else {
			GotoState(&Klayman::stWonderAboutHalf);
		}
		break;
	case 0x482E:	 
		GotoState(&Klayman::stWalkToFront);
		break;
	case 0x482F:
		GotoState(&Klayman::stTurnToBack);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4838:
		GotoState(&Klayman::stJumpToGrabRelease);
		break;
	}
	return 0;
}

KmScene2803Small::KmScene2803Small(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	_dataResource.load(0x81120132);
}

uint32 KmScene2803Small::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToXSmall(param.asPoint().x);
		break;
	case 0x4004:
		GotoState(&Klayman::stStandIdleSmall);
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
			GotoState(&Klayman::stWonderAboutAfterSmall);
		else if (param.asInteger() == 0)
			GotoState(&Klayman::stWonderAboutHalfSmall);
		else
			GotoState(&Klayman::stWonderAboutSmall);
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klayman::stWalkToFrontNoStepSmall);
		else if (param.asInteger() == 2)
			GotoState(&Klayman::stWalkToFront2Small);
		else
			GotoState(&Klayman::stWalkToFrontSmall);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klayman::stTurnToBackHalfSmall);
		else if (param.asInteger() == 2)
			GotoState(&Klayman::stTurnToBackWalkSmall);
		else
			GotoState(&Klayman::stTurnToBackSmall);
		break;
	case 0x4830:
		GotoState(&Klayman::stShrink);
		break;
	}
	return 0;
}

KmScene2805::KmScene2805(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

uint32 KmScene2805::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2000:
		_isSittingInTeleporter = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stSitIdleTeleporter);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x481D:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stTurnToUseInTeleporter);
		break;
	case 0x481E:
		if (_isSittingInTeleporter)
			GotoState(&Klayman::stReturnFromUseInTeleporter);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_isSittingInTeleporter = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_isSittingInTeleporter = false;
		GotoState(&Klayman::stGetUpFromTeleporter);
		break;
	case 0x483D:
		teleporterAppear(0xDE284B74);
		break;
	case 0x483E:
		teleporterDisappear(0xD82A4094);
		break;
	}
	return 0;
}

KmScene2806::KmScene2806(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y,
	bool flag, NRect *clipRects, uint clipRectsCount)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty

	_surface->setClipRects(clipRects, clipRectsCount);

	if (flag) {
		// TODO Maybe? Don't know. Set Klayman clip rects
		loadSound(3, 0x58E0C341);
		loadSound(4, 0x40A00342);
		loadSound(5, 0xD0A1C348);
		loadSound(6, 0x166FC6E0);
		loadSound(7, 0x00018040);
	}
	
	_dataResource.load(0x98182003);

}

uint32 KmScene2806::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		startWalkToX(440, true);
		break;
	case 0x480D:
		GotoState(&Klayman::stPullCord);
		break;
	case 0x4816:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide); 
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4831:
		GotoState(&Klayman::stGrow);
		break;
	case 0x4832:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stDrinkPotion);
		} else {
			GotoState(&Klayman::stUseTube);
		}
		break;
	}
	return 0;
}

KmScene2809::KmScene2809(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y,
	bool flag, NRect *clipRects, uint clipRectsCount)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	_surface->setClipRects(clipRects, clipRectsCount);

	if (flag) {
		// TODO Maybe? Don't know. Set Klayman clip rects
		loadSound(3, 0x58E0C341);
		loadSound(4, 0x40A00342);
		loadSound(5, 0xD0A1C348);
		loadSound(6, 0x166FC6E0);
		loadSound(7, 0x00018040);
	}

	_dataResource.load(0x1830009A);
	
}

uint32 KmScene2809::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		startWalkToX(226, true);
		break;
	case 0x480D:
		GotoState(&Klayman::stPullCord);
		break;
	case 0x4816:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide); 
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4831:
		GotoState(&Klayman::stGrow);
		break;
	case 0x4832:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stDrinkPotion);
		} else {
			GotoState(&Klayman::stUseTube);
		}
		break;
	}
	return 0;
}

KmScene2810Small::KmScene2810Small(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y) 
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

uint32 KmScene2810Small::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToXSmall(param.asPoint().x);
		break;
	case 0x4004:
		GotoState(&Klayman::stStandIdleSmall);
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
			GotoState(&Klayman::stWonderAboutAfterSmall);
		else if (param.asInteger() == 0)
			GotoState(&Klayman::stWonderAboutHalfSmall);
		else
			GotoState(&Klayman::stWonderAboutSmall);
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStepSmall);
		} else {
			GotoState(&Klayman::stWalkToFrontSmall);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToBackHalfSmall);
		} else {
			GotoState(&Klayman::stTurnToBackSmall);
		}
		break;
	case 0x4837:
		stopWalking();
		break;
	}
	return 0;
}

KmScene2810::KmScene2810(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, uint clipRectsCount)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	_surface->setClipRects(clipRects, clipRectsCount);
	
}
		
uint32 KmScene2810::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4803:
		_destY = param.asInteger();
		GotoState(&Klayman::stJumpToGrab);
		break;
	case 0x4804:
		if (param.asInteger() == 3)
			GotoState(&Klayman::sub421230);
		break;
	case NM_KLAYMAN_PICKUP:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;		
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481F:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stWonderAboutHalf);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::stWonderAboutAfter);
		} else if (param.asInteger() == 3) {
			GotoState(&Klayman::stTurnToUseHalf);
		} else if (param.asInteger() == 4) {
			GotoState(&Klayman::stTurnAwayFromUse);
		} else if (param.asInteger() == 5) {
			GotoState(&Klayman::stTurnToUseExt);
		} else {
			GotoState(&Klayman::stWonderAbout);
		}
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klayman::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2000, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klayman::stClimbLadderHalf);	 
		break;
	case 0x4824:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klayman::stStartClimbLadderDown);
		break;
	case 0x4825:
		sendMessage(_parentScene, 0x2000, 0);
		_destY = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klayman::stStartClimbLadderUp);
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

KmScene2812::KmScene2812(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

uint32 KmScene2812::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4805:
		_destY = param.asInteger();
		GotoState(&Klayman::stJumpToGrabFall);
		break;
	case NM_KLAYMAN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klayman::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klayman::stPickUpTube);
		else
			GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481A:
		GotoState(&Klayman::stInsertDisk);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			sub41CC40(param.asPoint().y, param.asPoint().x);
		else
			sub41CCE0(param.asPoint().x);
		break;
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::stReturnFromUse);
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klayman::stContinueClimbLadderUp);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2001, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderDown);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2001, 0);
		_destY = param.asInteger();
		GotoState(&Klayman::stStartClimbLadderUp);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2002, 0);
		GotoState(&Klayman::stClimbLadderHalf);	 
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stWalkToFrontNoStep);
		} else {
			GotoState(&Klayman::stWalkToFront);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnToFront);
		} else {
			GotoState(&Klayman::stTurnToBack);
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

} // End of namespace Neverhood
