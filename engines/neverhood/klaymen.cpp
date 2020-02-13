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

// Klaymen

Klaymen::Klaymen(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRectArray *clipRects)
	: AnimatedSprite(vm, 1000), _idleCounterMax(0), _idleCounter(0), _isMoveObjectRequested(false), _blinkCounterMax(0),
	_isWalkingOpenDoorNotified(false), _spitOutCountdown(0), _tapesToInsert(0), _keysToInsert(0), _busyStatus(0), _acceptInput(true),
	_attachedSprite(NULL), _isWalking(false), _actionStatus(1), _parentScene(parentScene), _isSneaking(false), _isLargeStep(false),
	_doYHitIncr(false), _isLeverDown(false), _isSittingInTeleporter(false), _actionStatusChanged(false), _ladderStatus(0), _pathPoints(NULL), _soundFlag(false),
	_idleTableNum(0), _otherSprite(NULL), _moveObjectCountdown(0), _walkResumeFrameIncr(0) {

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
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x04DBC02C) {
			playSound(0, 0x44528AA1);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x808A0008) {
			playSound(0, 0xD948A340);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x5A0F0104) {
			playSound(0, 0x7970A100);
		} else if (param.asInteger() == 0x9A9A0109) {
			playSound(0, 0xD170CF04);
		} else if (param.asInteger() == 0x989A2169) {
			playSound(0, 0xD073CF14);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x0D2A0288) {
			playSound(0, 0xD192A368);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
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
	default:
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
	startAnimation(0x582EC138, 0, -1);
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0xC1380080) {
			sendMessage(_attachedSprite, NM_KLAYMEN_USE_OBJECT, 0);
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
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x001A2832) {
			playSound(0, 0xC0E4884C);
		}
		break;
	default:
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
	debug(1, "startIdleAnimation(%08X)", fileHash);
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
	case NM_SCENE_LEAVE:
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
	default:
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
	case NM_ANIMATION_STOP:
		gotoNextStateExt();
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x271AA210) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x2B22AA81) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
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
	case NM_ANIMATION_STOP:
		_x = _destX;
		gotoNextStateExt();
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x32180101) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x32180101) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	default:
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
		sendMessage(this, NM_SCENE_LEAVE, 0);
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, NM_MOVE_TO_BACK, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, NM_MOVE_TO_FRONT, 0);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0xC1380080) {
			sendMessage(_attachedSprite, NM_KLAYMEN_USE_OBJECT, 0);
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
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x0D01B294) {
			sendMessage(_attachedSprite, 0x480B, 0);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x32180101)
			playSound(0, 0x4924AAC4);
		else if (param.asInteger() == 0x0A2A9098)
			playSound(0, 0x0A2AA8E0);
		break;
	default:
		break;
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x80C110B5)
			sendMessage(_parentScene, NM_MOVE_TO_BACK, 0);
		else if (param.asInteger() == 0x110010D1)
			sendMessage(_parentScene, NM_MOVE_TO_FRONT, 0);
		else if (param.asInteger() == 0x32180101)
			playSound(0, 0x4924AAC4);
		else if (param.asInteger() == 0x0A2A9098)
			playSound(0, 0x0A2AA8E0);
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, NM_MOVE_TO_BACK, 0);
			sendMessage(_attachedSprite, NM_KLAYMEN_LOWER_LEVER, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, NM_MOVE_TO_FRONT, 0);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
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
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	case NM_ANIMATION_STOP:
		_x = _destX;
		gotoNextStateExt();
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x32180101) {
			playSound(0, 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0460E2FA);
		}
		break;
	default:
		break;
	}
	return hmLowLevelAnimation(messageNum, param, sender);
}

void Klaymen::suUpdateDestX() {
	AnimatedSprite::updateDeltaXY();
	_destX = _x;
}

void Klaymen::stReleaseRing() {
	_busyStatus = 1;
	_acceptInput = false;
	sendMessage(_attachedSprite, NM_KLAYMEN_RAISE_LEVER, 0);
	_attachedSprite = NULL;
	startAnimation(0xB869A4B9, 0, -1);
	SetUpdateHandler(&Klaymen::update);
	SetMessageHandler(&Klaymen::hmLowLevelAnimation);
	SetSpriteUpdate(NULL);
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
	case NM_ANIMATION_START:
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
	default:
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmClimbLadderUpDown(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevel(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
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
					sendMessage(this, NM_SCENE_LEAVE, 0);
			} else {
				startAnimationByHash(0x122D1505, 0x01084280, 0);
				if (_destY <= _y)
					sendMessage(this, NM_SCENE_LEAVE, 0);
			}
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x80C110B5) {
			sendMessage(_parentScene, NM_MOVE_TO_BACK, 0);
		} else if (param.asInteger() == 0x110010D1) {
			sendMessage(_parentScene, NM_MOVE_TO_FRONT, 0);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, _soundFlag ? 0x48498E46 : 0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, _soundFlag ? 0x50399F64 : 0x0460E2FA);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x320AC306) {
			playSound(0, 0x5860C640);
		}
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0xC61A0119) {
			playSound(0, 0x402338C2);
		} else if (param.asInteger() == 0x32180101) {
			playSound(0, 0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			playSound(0, 0x0A2AA8E0);
		}
		break;
	default:
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
	sendMessage(_attachedSprite, NM_KLAYMEN_RAISE_LEVER, 0);
}

uint32 Klaymen::hmMoveObjectTurn(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case NM_ANIMATION_START:
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
	case NM_KLAYMEN_MOVE_OBJECT:
		_isMoveObjectRequested = true;
		return 0;
	default:
		break;
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
			sendMessage(_attachedSprite, NM_MOVE_TO_FRONT, 0);
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
	sendMessage(_attachedSprite, NM_KLAYMEN_LOWER_LEVER, 0);
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
	sendMessage(_attachedSprite, NM_KLAYMEN_RAISE_LEVER, 0);
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
		sendMessage(_attachedSprite, NM_KLAYMEN_RAISE_LEVER, 0);
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
	sendMessage(_attachedSprite, NM_MOVE_TO_BACK, 0);
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
	case NM_ANIMATION_START:
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
		break;
	default:
		break;
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
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x168050A0)
			sendMessage(_attachedSprite, NM_KLAYMEN_USE_OBJECT, 0);
		else if (param.asInteger() == 0x320AC306)
			startAnimationByHash(0x00AB8C10, 0x01084280, 0);
		else if (param.asInteger() == 0x4AB28209)
			sendMessage(_attachedSprite, NM_MOVE_TO_BACK, 0);
		else if (param.asInteger() == 0x88001184)
			sendMessage(_attachedSprite, NM_MOVE_TO_FRONT, 0);
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x040C4C01)
			playSound(0, 0x01E11140);
		break;
	default:
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
		sendMessage(_attachedSprite, NM_MOVE_TO_FRONT, 0);
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x320AC306)
			playSound(0, 0x5860C640);
		break;
	default:
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
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x4E0A2C24) {
			playSound(0, 0x85B10BB8);
		} else if (param.asInteger() == 0x4E6A0CA0) {
			playSound(0, 0xC5B709B0);
		}
		break;
	default:
		break;
	}
	return messageResult;
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
	case NM_ANIMATION_START:
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
	default:
		break;
	}
	return messageResult;
}

uint32 Klaymen::hmGrow(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
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
	default:
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
	case NM_ANIMATION_START:
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
	default:
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

uint32 Klaymen::hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
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
	default:
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

void Klaymen::upSpitOutFall() {
	Klaymen::update();
	if (_spitOutCountdown != 0 && (--_spitOutCountdown == 0)) {
		_surface->setVisible(true);
		SetUpdateHandler(&Klaymen::update);
	}
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
	default:
		break;
	}
	return 0;
}

void Klaymen::suFallDown() {
	AnimatedSprite::updateDeltaXY();
	HitRect *hitRect = _parentScene->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		updateBounds();
		sendMessage(this, NM_SCENE_LEAVE, 0);
	}
	_parentScene->checkCollision(this, 0xFFFF, 0x4810, 0);
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
	sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
	_attachedSprite = NULL;
	sendMessage(_parentScene, 0x8001, 0);
}

void Klaymen::stFallTouchdown() {
	setDoDeltaX(2);
	stTryStandIdle();
}

void Klaymen::suFallSkipJump() {
	updateDeltaXY();
	HitRect *hitRect = _parentScene->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		updateBounds();
		sendMessage(this, NM_SCENE_LEAVE, 0);
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

uint32 Klaymen::hmMoveObject(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case NM_ANIMATION_START:
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
	default:
		break;
	}
	return Klaymen::hmLowLevelAnimation(messageNum, param, sender);
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
	sendMessage(_attachedSprite, NM_KLAYMEN_LOWER_LEVER, 0);
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

} // End of namespace Neverhood
