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

static const KlaymanTableItem klaymanTable1[] = {
	{1, &Klayman::stDoIdlePickEar},
	{1, &Klayman::sub41FDA0},
	{1, &Klayman::sub41FDF0},
	{1, &Klayman::stDoIdleChest},
	{1, &Klayman::sub41FEB0}
}; 

static const KlaymanTableItem klaymanTable2[] = {
	{1, &Klayman::stDoIdlePickEar},
	{1, &Klayman::sub41FDA0},
	{1, &Klayman::stDoIdleChest},
	{1, &Klayman::sub41FEB0}
}; 

#if 0
static const KlaymanTableItem klaymanTable3[] = {
	{1, &Klayman::sub421430},
	{1, &Klayman::sub421480}
}; 
#endif

static const KlaymanTableItem klaymanTable4[] = {
	{1, &Klayman::sub41FDA0},
	{1, &Klayman::stDoIdleChest},
	{1, &Klayman::sub41FEB0},
};

// Klayman

Klayman::Klayman(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, int surfacePriority, int objectPriority, NRectArray *clipRects)
	: AnimatedSprite(vm, objectPriority), _soundResource1(vm), _soundResource2(vm),
	_counterMax(0), _counter(0), _flagE4(false), _counter3Max(0), _flagF8(false), _counter1(0),
	_counter2(0), /*_field118(0), */_status2(0), _flagE5(true), _attachedSprite(NULL), _flagE1(false),
	_status3(1), _parentScene(parentScene), _flagE2(false), _flagE3(false), _flagF6(false), _flagF7(false),
	_flagFA(false), _statusE0(0), _field114(0), _resourceHandle(-1), _soundFlag(false) {
	
	// TODO DirtySurface
	createSurface(surfacePriority, 320, 200);
	_x = x;
	_y = y;
	_x4 = x;
	_y4 = y;
	_flags = 2;
	setKlaymanTable1();
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
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4818:
		sub41C930(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	}
	return 0;
}

void Klayman::update() {
	AnimatedSprite::update();
	xUpdate();
}

void Klayman::setKlaymanTable(const KlaymanTableItem *table, int tableCount) {
	_table = table;
	_tableCount = tableCount;
	_tableMaxValue = 0;
	for (int i = 0; i < tableCount; i++) {
		_tableMaxValue += table[i].value;
	}
}

void Klayman::setKlaymanTable1() {
	setKlaymanTable(klaymanTable1, ARRAYSIZE(klaymanTable1));
}

void Klayman::setKlaymanTable2() {
	setKlaymanTable(klaymanTable2, ARRAYSIZE(klaymanTable2));
}

void Klayman::setKlaymanTable3() {
	// TODO setKlaymanTable(klaymanTable3, ARRAYSIZE(klaymanTable3));
}

void Klayman::stDoIdlePickEar() {
	sub41D320(0x5B20C814, AnimationCallback(&Klayman::stIdlePickEar));
}

void Klayman::stIdlePickEar() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5B20C814, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdlePickEar);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stStandAround);
	FinalizeState(&Klayman::evIdlePickEarDone);
}

uint32 Klayman::hmIdlePickEar(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x04DBC02C) {
			_soundResource1.play(0x44528AA1);
		}
		break;
	}
	return messageResult;
}

void Klayman::evIdlePickEarDone() {
	_soundResource1.stop();
}

void Klayman::sub41FDA0() {
	sub41D320(0xD122C137, AnimationCallback(&Klayman::sub41FDB0));
}

void Klayman::sub41FDB0() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0xD122C137, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41E980);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::handleMessage41E980(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x808A0008) {
			_soundResource1.play(0xD948A340);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub41FDF0() {
	sub41D320(0x543CD054, AnimationCallback(&Klayman::sub41FE00));
}

void Klayman::sub41FE00() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x543CD054, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41E9E0);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stStandAround);
	FinalizeState(&Klayman::sub41FE50);
}

void Klayman::sub41FE50() {
	_soundResource1.stop();
}

uint32 Klayman::handleMessage41E9E0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x5A0F0104) {
			_soundResource1.play(0x7970A100);
		} else if (param.asInteger() == 0x9A9A0109) {
			_soundResource1.play(0xD170CF04);
		} else if (param.asInteger() == 0x989A2169) {
			_soundResource1.play(0xD073CF14);
		}
		break;
	}
	return messageResult;
}

void Klayman::stDoIdleChest() {
	sub41D320(0x40A0C034, AnimationCallback(&Klayman::stIdleChest));
}

void Klayman::stIdleChest() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x40A0C034, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmIdleChest);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::hmIdleChest(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0D2A0288) {
			_soundResource1.play(0xD192A368);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub41FEB0() {
	sub41D320(0x5120E137, AnimationCallback(&Klayman::sub41FEC0));
}

void Klayman::sub41FEC0() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5120E137, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EFE0);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::handleMessage41EFE0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC006000C) {
			_soundResource1.play(0x9D406340);
		} else if (param.asInteger() == 0x2E4A2940) {
			_soundResource1.play(0x53A4A1D4);
		} else if (param.asInteger() == 0xAA0A0860) {
			_soundResource1.play(0x5BE0A3C6);
		} else if (param.asInteger() == 0xC0180260) {
			_soundResource1.play(0x5D418366);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub421350() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x582EC138, 0, -1);
	_counter = 0;
	SetSpriteCallback(NULL);
	SetUpdateHandler(&Klayman::update41D1C0);
	SetMessageHandler(&Klayman::handleMessage41D360);
	_counter3 = 0;
	_counterMax = 8;
	_counter3Max = _vm->_rnd->getRandomNumber(64 - 1) + 24;
}

void Klayman::update41D1C0() {
	update();
	_counter++;
	if (_counter >= _counterMax) {
		_counter = 0;
		if (_table) {
			int randomValue = _vm->_rnd->getRandomNumber(_tableMaxValue);
			for (int i = 0; i < _tableCount; i++) {
				if (randomValue < _table[_tableCount].value) {
					(this->*(_table[_tableCount].callback))();
					_counterMax = _vm->_rnd->getRandomNumber(128) + 24;
					break;
				}
				randomValue -= _table[_tableCount].value;
			}
		}
	} else {
		_counter3++;
		if (_counter3 >= _counter3Max) {
			_counter3 = 0;
			_counter3Max = _vm->_rnd->getRandomNumber(64) + 24;
			stIdleSitBlink();
		}
	}
}

void Klayman::stIdleSitBlink() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x5C24C018, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stIdleSitBlinkSecond);
}

void Klayman::stIdleSitBlinkSecond() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x5C24C018, 0, -1);
	SetUpdateHandler(&Klayman::update41D1C0);
	SetMessageHandler(&Klayman::handleMessage41D360);
	SetSpriteCallback(NULL);
}

void Klayman::stPickUpNeedle() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::stPickUpNeedle))) {
		_status2 = 1;
		_flagE5 = false;
		setFileHash(0x1449C169, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPickUpGeneric);
		SetSpriteCallback(NULL);
	}
}

void Klayman::sub41FFF0() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::sub41FFF0))) {
		_status2 = 1;
		_flagE5 = false;
		setFileHash(0x0018C032, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41D640);
		SetSpriteCallback(NULL);
	}
}

uint32 Klayman::handleMessage41D640(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC1380080) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x4806, 0);
				_soundResource1.play(0xC8004340);
			}
		} else if (param.asInteger() == 0x02B20220) {
			_soundResource1.play(0xC5408620);
		} else if (param.asInteger() == 0x03020231) {
			_soundResource1.play(0xD4C08010);
		} else if (param.asInteger() == 0x67221A03) {
			_soundResource1.play(0x44051000);
		} else if (param.asInteger() == 0x925A0C1E) {
			_soundResource1.play(0x40E5884D);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub4214D0() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0xD229823D, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::sub421510() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x9A2801E0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::stStepOver() {
	if (!stStartAction(AnimationCallback(&Klayman::stStepOver))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x004AA310, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41EC70);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	}
}

void Klayman::stSitInTeleporter() {
	if (!stStartAction(AnimationCallback(&Klayman::stSitInTeleporter))) {
		_status2 = 0;
		_flagE5 = false;
		setFileHash(0x392A0330, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41EAB0);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	}
}

uint32 Klayman::handleMessage41EAB0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x001A2832) {
			_soundResource1.play(0xC0E4884C);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub421310() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x913AB120, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
}

/////////////////////////////////////////////////////////////////

void Klayman::sub41CE70() {
	_x4 = _x;
	if (!_flagE1 && !_flagE2 && !_flagE3) {
		gotoState(NULL);
		sub41C7B0();
	}
}

void Klayman::sub41D320(uint32 fileHash, AnimationCb callback) {
	_resourceHandle = _vm->_res->useResource(fileHash);
	if (_resourceHandle != -1) {
		// TODO _vm->_res->moveToFront(_resourceHandle);
		NextState(callback);
		SetUpdateHandler(&Klayman::update41D2B0);
	}
}

void Klayman::update41D2B0() {
	// TODO Check if this odd stuff is needed or just some cache optimization
	if (_vm->_res->isResourceDataValid(_resourceHandle)) {
		sub41C7B0();
		// TODO _vm->_res->moveToBack(_resourceHandle);
		_vm->_res->unuseResource(_resourceHandle);
		_resourceHandle = -1;
	} else {
		// TODO _vm->_res->moveToFront(_resourceHandle);
	}
	update();
}

bool Klayman::stStartActionFromIdle(AnimationCb callback) {
	if (_status2 == 2) {
		_status2 = 1;
		_flagE5 = false; 
		setFileHash(0x9A7020B8, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41F140);
		SetSpriteCallback(NULL);
		NextState(callback);
		return true;
	}
	return false;
}

void Klayman::sub41C7B0() {
	if (_finalizeStateCb) {
		AnimationCb cb = _finalizeStateCb;
		_finalizeStateCb = NULL;
		(this->*cb)();
	}
	if (_nextStateCb) {
		AnimationCb cb = _nextStateCb;
		_nextStateCb = NULL;
		(this->*cb)();
#if 0 // TODO		
	} else if (_callbackList) {
		removeCallbackList();
#endif		
	} else {
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
		_flagE5 = true;
		setFileHash(0x5420E254, 0, -1);
		_counter = 0;
		_counter3 = 0;
		_counter3Max = _vm->_rnd->getRandomNumber(64) + 24;
		SetUpdateHandler(&Klayman::update41D0F0);
		SetMessageHandler(&Klayman::handleMessage41D360);
		SetSpriteCallback(NULL);
	}
}

void Klayman::update41D0F0() {
	update();
	_counter++;
	if (_counter >= 720) {
		_counter = 0;
		if (_table) {
			int randomValue = _vm->_rnd->getRandomNumber(_tableMaxValue);
			for (int i = 0; i < _tableCount; i++) {
				if (randomValue < _table[_tableCount].value) {
					(this->*(_table[_tableCount].callback))();
					break;
				}
				randomValue -= _table[_tableCount].value;
			}
		}
	} else {
		_counter3++;
		if (_counter3 >= _counter3Max) {
			_counter3 = 0;
			_counter3Max = _vm->_rnd->getRandomNumber(64) + 24;
			stStand();
		}
	}
}

uint32 Klayman::handleMessage41D360(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	uint32 messageResult = xHandleMessage(messageNum, param);
	switch (messageNum) {
	case 0x1008:
		messageResult = _flagE5;
		break;
	case 0x1014:
		_attachedSprite = (Sprite*)(param.asEntity());
		break;
	case 0x1019:
		sub41C7B0();
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
		debug("#################################################");
			// TODO _rectResource.getRectangle2(param.asInteger(), &_field118, &_field114,);
		} else {
		debug("#################################################");
			// TODO _field114 = 0;
		}
		break;
	}
	return messageResult;
}

void Klayman::stStand() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5900C41E, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
	NextState(&Klayman::stStandAround);
}

uint32 Klayman::handleMessage41D480(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		sub41C7B0();
		break;
	}
	return messageResult;
}

void Klayman::stStandAround() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5420E254, 0, -1);
	SetUpdateHandler(&Klayman::update41D0F0);
	SetMessageHandler(&Klayman::handleMessage41D360);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41F140(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x271AA210) {
			_soundResource1.play(0x4924AAC4);
		} else if (param.asInteger() == 0x2B22AA81) {
			_soundResource1.play(0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}


void Klayman::sub41C930(int16 x, bool flag) {
	int16 xdiff = ABS(x - _x);
	if (x == _x) {
		_x4 = x;
		if (!_flagE1 && !_flagE2 && !_flagE3) {
			gotoState(NULL);
			sub41C7B0();
		}
	} else if (xdiff <= 36 && !_flagE1 && !_flagE2 && !_flagE3) {
		_x4 = x;
		gotoState(NULL);
		sub41C7B0();
	} else if (xdiff <= 42 && _status3 != 3) {
		if (_flagE2 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_x4 - _x) > xdiff) {
			_x4 = x;
		} else {
			_x4 = x;
			GotoState(&Klayman::stSneak);
		}
	} else if (_flagE1 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_x4 = x;
	} else if (flag) {
		_x4 = x;
		error("// TODO AnimatedSprite_GotoState(&Klayman::sub421550));");
		// TODO AnimatedSprite_GotoState(&Klayman::sub421550);
	} else {
		_x4 = x;
		GotoState(&Klayman::stStartWalking);
	}
}

void Klayman::stWakeUp() {
	_status2 = 1;
	_flagE5 = false;
	setFileHash(0x527AC970, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::stSleeping() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x5A38C110, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmSleeping);
	SetSpriteCallback(NULL);
}

uint32 Klayman::hmSleeping(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x03060012) {
			_soundResource1.play(0xC0238244);
		}
		break;
	}
	return messageResult;
}

bool Klayman::stStartAction(AnimationCb callback3) {
	if (_status2 == 1) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x5C7080D4, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41F140);
		SetSpriteCallback(&Klayman::spriteUpdate41F250);
		NextState(callback3);
		return true;
	} else {
		_x = _x4;
		return false;
	}
}

void Klayman::spriteUpdate41F250() {

	int16 xdiff = _x4 - _x;

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
	
	if (_flag) {
		if (xdiff > 6)
			_x += 6;
		else if (xdiff < -6)
			_x -= 6;
		else
			_x = _x4;				
	}
	
	processDelta();
	
}

void Klayman::spriteUpdate41F5F0() {
	
	int16 xdiff = _x4 - _x;
	
	if (_frameIndex == 9) {
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

	if (_x4 != _x) {
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdiff;
		if (_field114) {
			error("// TODO Klayman_sub_41CF70");
			// TODO Klayman_sub_41CF70
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
	_flagE2 = true;
	_flagE5 = true;
	setDoDeltaX(_x4 < _x ? 1 : 0);
	setFileHash(0x5C48C506, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41DD80);
	SetSpriteCallback(&Klayman::spriteUpdate41F5F0);
	FinalizeState(&Klayman::stSneakDone);	
}

void Klayman::stSneakDone() {
	_flagE2 = false;
}

uint32 Klayman::handleMessage41DD80(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0A2AA8E0);
		} else if (param.asInteger() == 0x32188010) {
			if (_soundFlag) {
				_soundResource1.play(0x48498E46);
			} else {
				_soundResource1.play(0x405002D8);
			}
		} else if (param.asInteger() == 0x02A2909C) {
			if (_soundFlag) {
				_soundResource1.play(0x50399F64);
			} else {
				_soundResource1.play(0x0460E2FA);
			}
		}
		break;
	case 0x3002:
		_x = _x4;
		sub41C7B0();
		break;
	}
	return messageResult;
}

void Klayman::sub41CD70(int16 x) {
	if (x > _x) {
		if (ABS(x - _x) <= 105) {
			sub41CAC0(x);
		} else {
			sub41C930(x, false);
		}
	} else if (x == _x) {
		_x4 = x;
		gotoState(NULL);
		sub41C7B0();
	} else {
		sub41C930(x, false);
	}
}

void Klayman::stStartWalking() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stStartWalking))) {
		_status2 = 0;
		_flagE1 = true;
		_flagE5 = true;
		setDoDeltaX(_x4 < _x ? 1 : 0);
		setFileHash(0x242C0198, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41EC70);
		SetSpriteCallback(&Klayman::spriteUpdate41F320);
		FinalizeState(&Klayman::stStartWalkingDone);	
		NextState(&Klayman::stWalking);
	}
}

void Klayman::stStartWalkingDone() {
	_flagE1 = false;
}

uint32 Klayman::handleMessage41EC70(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			if (_soundFlag) {
				_soundResource1.play(0x48498E46);
			} else {
				_soundResource1.play(0x405002D8);
			}
		} else if (param.asInteger() == 0x0A2A9098) {
			if (_soundFlag) {
				_soundResource1.play(0x50399F64);
			} else {
				_soundResource1.play(0x0460E2FA);
			}
		}
		break;
	}
	return messageResult;
}

void Klayman::stWalking() {
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	FinalizeState(&Klayman::stStartWalkingDone);	
	NextState(&Klayman::sub41FA40);
}

void Klayman::spriteUpdate41F300() {
	SetSpriteCallback(&Klayman::spriteUpdate41F320);
	_deltaX = 0;
}

uint32 Klayman::handleMessage41EB70(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			if (_soundFlag) {
				_soundResource1.play(0x48498E46);
			} else {
				_soundResource1.play(0x405002D8);
			}
		} else if (param.asInteger() == 0x0A2A9098) {
			if (_soundFlag) {
				_soundResource1.play(0x50399F64);
			} else {
				_soundResource1.play(0x0460E2FA);
			}
		}
		break;
	}
	return messageResult;
}

void Klayman::sub41FA40() {
	if (_status3 == 2) {
		sub41C7B0();
	} else if (_status3 == 3) {
		sub420F20();
	} else {
		_flagE2 = true;
		_flagE5 = true;
		if (ABS(_x4 - _x) <= 42 && _frameIndex >= 5 && _frameIndex <= 11) {
			if (_status3 == 0) {
				_status2 = 1;
				setFileHash(0xF234EE31, 0, -1);
			} else {
				_status2 = 2;
				setFileHash(0xF135CC21, 0, -1);
			}
		} else if (ABS(_x4 - _x) <= 10 && (_frameIndex >= 12 || _frameIndex <= 4)) {
			if (_status3 == 0) {
				_status2 = 1;
				setFileHash(0x8604A152, 0, -1);
			} else {
				_status2 = 2;
				setFileHash(0xA246A132, 0, -1);
			}
		}
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41DD80);
		SetSpriteCallback(&Klayman::spriteUpdate41F5F0);
		FinalizeState(&Klayman::stSneakDone);	
	}
}

void Klayman::spriteUpdate41F320() {
	int16 xdiff = ABS(_x4 - _x);
	int16 xdelta = _x4 - _x;
	
	if (xdelta > _deltaX)
		xdelta = _deltaX;
	else if (xdelta < -_deltaX)
		xdelta = -_deltaX;
		
	_deltaX = 0;		

	if (xdiff == 0) {
		sendMessage(this, 0x1019, 0);
	} else if (_status3 != 2 && _status3 != 3 && xdiff <= 42 && _frameIndex >= 5 && _frameIndex <= 11) {
		sendMessage(this, 0x1019, 0);
	} else if (_status3 != 2 && _status3 != 3 && xdiff <= 10 && (_frameIndex >= 12 || _frameIndex <= 4)) {
		sendMessage(this, 0x1019, 0);
	} else if (_status3 == 3 && xdiff < 30) {
		sendMessage(this, 0x1019, 0);
	} else if (_status3 == 3 && xdiff < 150 && _frameIndex >= 6) {
		sendMessage(this, 0x1019, 0);
	} else {
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdelta;
		if (_field114) {
			error("_field114");
			// TODO Klayman_sub_41CF70
		} else {
			HitRect *hitRectNext = _vm->_collisionMan->findHitRectAtPos(_x, _y);
			if (hitRectNext->type == 0x5002) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (hitRectNext->rect.x2 - _x) / 2);
			} else if (hitRectNext->type == 0x5003) {
				_y = MAX<int16>(hitRectNext->rect.y1, hitRectNext->rect.y2 - (_x - hitRectNext->rect.x1) / 2);
			} else if (hitRectPrev->type == 0x5002) {
				if (xdelta > 0) {
					_y = hitRectPrev->rect.y2;
				} else {
					_y = hitRectPrev->rect.y1;
				}
			} else if (hitRectPrev->type == 0x5003) {
				if (xdelta < 0) {
					_y = hitRectPrev->rect.y2;
				} else {
					_y = hitRectPrev->rect.y1;
				}
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

uint32 Klayman::handleMessage41E210(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
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
		_flagE5 = false;
		setFileHash(0x1C28C178, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPickUpGeneric);
		SetSpriteCallback(NULL);
	}
}

uint32 Klayman::hmPickUpGeneric(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC1380080) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x4806, 0);
			}
			_soundResource1.play(0x40208200);
		} else if (param.asInteger() == 0x02B20220) {
			_soundResource1.play(0xC5408620);
		} else if (param.asInteger() == 0x03020231) {
			_soundResource1.play(0xD4C08010);
		} else if (param.asInteger() == 0x67221A03) {
			_soundResource1.play(0x44051000);
		} else if (param.asInteger() == 0x2EAE0303) {
			_soundResource1.play(0x03630300);
		} else if (param.asInteger() == 0x61CE4467) {
			_soundResource1.play(0x03630300);
		}
		break;
	}
	return messageResult;

}

void Klayman::stTurnPressButton() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnPressButton))) {
		_status2 = 2;
		_flagE5 = true;
		setFileHash(0x1C02B03D, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPressButton);
		SetSpriteCallback(NULL);
	}
}

uint32 Klayman::hmPressButton(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0D01B294) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x480B, 0);
			}
		} else if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}

void Klayman::stStampFloorButton() {
	if (!stStartAction(AnimationCallback(&Klayman::stStampFloorButton))) {
		_status2 = 2;
		_flagE5 = true;
		setFileHash(0x1C16B033, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPressButton);
		SetSpriteCallback(NULL);
	}
}

void Klayman::stPressButtonSide() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::stPressButtonSide))) {
		_status2 = 1;
		_flagE5 = true;
		setFileHash(0x1CD89029, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::hmPressButton);
		SetSpriteCallback(&Klayman::spriteUpdate41F250);
	}
}

void Klayman::sub41CD00(int16 x) {
	if (_x > x) {
		if (_x - x <= 105) {
			sub41CAC0(x);
		} else {
			sub41C930(x, false);
		}
	} else if (_x < x) {
		sub41C930(x, false);
	} else {
		_x4 = x;
		gotoState(NULL);
		sub41C7B0();
	}
}

void Klayman::sub41CC40(int16 x1, int16 x2) {
	if (_x > x1) {
		if (_x == x1 + x2) {
			_x4 = x1 + x2;
			gotoState(NULL);
			sub41C7B0();
		} else if (_x < x1 + x2) {
			sub41CAC0(x1 + x2);
		} else {
			sub41C930(x1 + x2, false);
		}
	} else {
		if (_x == x1 - x2) {
			_x4 = x1 - x2;
			gotoState(NULL);
			sub41C7B0();
		} else if (_x > x1 - x2) {
			sub41CAC0(x1 - x2);
		} else {
			sub41C930(x1 - x2, false);
		}
	}
}

void Klayman::sub41CAC0(int16 x) {
	int16 xdiff = ABS(x - _x);
	if (x == _x) {
		_x4 = x;
		if (!_flagE1 && !_flagE2 && !_flagE3) {
			gotoState(NULL);
			sub41C7B0();
		}
	} else if (xdiff <= 36 && !_flagE1 && !_flagE2 && !_flagE3) {
		_x4 = x;
		gotoState(NULL);
		sub41C7B0();
	} else if (xdiff <= 42 && _status3 != 3) {
		if (_flagE2 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_x4 - _x) > xdiff) {
			_x4 = x;
		} else {
			_x4 = x;
			GotoState(&Klayman::stSneak);
		}
	} else if (_flagE3 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_x4 = x;
	} else {
		_x4 = x;
		GotoState(&Klayman::stLargeStep);
	}
}

void Klayman::stLargeStep() {
	_status2 = 2;
	_flagE3 = true;
	_flagE5 = true;
	setDoDeltaX(_x4 >= _x ? 1 : 0);
	setFileHash(0x08B28116, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmLargeStep);
	SetSpriteCallback(&Klayman::suLargeStep);
	FinalizeState(&Klayman::stLargeStepDone);	
}

void Klayman::stLargeStepDone() {
	_flagE3 = false;
}

void Klayman::suLargeStep() {
	int16 xdiff = _x4 - _x;
	
	if (_doDeltaX) {
		_deltaX = -_deltaX;
	}
	
	if (_frameIndex == 7) {
		_deltaX = xdiff;
	}

	if ((xdiff > 0 && xdiff > _deltaX) || (xdiff < 0 && xdiff < _deltaX))
		xdiff = _deltaX;

	_deltaX = 0;
	
	if (_x != _x4) {
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdiff;
		if (_field114) {
			error("// TODO Klayman_sub_41CF70();");
			// TODO Klayman_sub_41CF70();
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

uint32 Klayman::hmLargeStep(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0A2AA8E0);
		}
		break;
	case 0x3002:
		_x = _x4;
		sub41C7B0();
		break;		
	}
	return messageResult;
}

void Klayman::stWonderAboutHalf() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0xD820A114, 0, 10);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::stWonderAboutAfter() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0xD820A114, 30, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::stTurnToUseHalf() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x9B250AD2, 0, 7);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EEF0);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41EEF0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}

void Klayman::stTurnAwayFromUse() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x98F88391, 4, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EEF0);
	SetSpriteCallback(NULL);
}

void Klayman::stWonderAbout() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0xD820A114, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::stPeekWall() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0xAC20C012, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::hmPeekWall);
	SetSpriteCallback(NULL);
}

uint32 Klayman::hmPeekWall(int messageNum, const MessageParam &param, Entity *sender) {
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymanSpeedUpHash);
		if (_frameIndex < speedUpFrameIndex)
			setFileHash(0xAC20C012, speedUpFrameIndex, -1);
		return 0;
	case 0x100D:
		if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0460E2FA);
		}
		break;
	}
	return handleMessage41D480(messageNum, param, sender);
}

void Klayman::sub420210() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420210))) {
		_status2 = 0;
		setFileHash(0xD82890BA, 0, -1);
		sub4201C0();
	}
}

void Klayman::sub4201C0() {
	_flagE5 = false;
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D790);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	NextState(&Klayman::sub420340);
	sendMessage(_attachedSprite, 0x482B, 0);
}

uint32 Klayman::handleMessage41D790(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x4806, 0);
			}
			_flagE5 = true;
		} else if (param.asInteger() == 0x320AC306) {
			_soundResource1.play(0x5860C640);
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

void Klayman::spriteUpdate41F230() {
	AnimatedSprite::updateDeltaXY();
	_x4 = _x;
}

void Klayman::sub420340() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x4829E0B8, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D360);
	SetSpriteCallback(NULL);
}

void Klayman::sub420250() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420250))) {
		_status2 = 0;
		setFileHash(0x900980B2, 0, -1);
		sub4201C0();
	}
}

void Klayman::sub420290() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420290))) {
		_status2 = 0;
		_flagE5 = false;
		setFileHash(0xBA1910B2, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
		SetMessageHandler(&Klayman::handleMessage41D880);
		NextState(&Klayman::sub420380);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

uint32 Klayman::handleMessage41D880(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x4806, 0);
			}
		} else if (param.asInteger() == 0x320AC306) {
			_soundResource1.play(0x5860C640);
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub420380() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x4A293FB0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41DAA0);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41DAA0(int messageNum, const MessageParam &param, Entity *sender) {
	if (messageNum == 0x1008) {
		sub4203C0();
		return 0;
	}
	return handleMessage41D360(messageNum, param, sender);
}

void Klayman::sub4203C0() {
	_status2 = 1;
	_flagE5 = false;
	if (_attachedSprite) {
		sendMessage(_attachedSprite, 0x4807, 0);
		_attachedSprite = NULL;
	}
	setFileHash(0xB869A4B9, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::sub420300() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420300))) {
		_status2 = 0;
		setFileHash(0xB8699832, 0, -1);
		sub4201C0();
	}
}

void Klayman::sub41CCE0(int16 x) {
	sub41CC40(_attachedSprite->getX(), x);
}

void Klayman::sub420970() {
	_status2 = 0;
	_flagE5 = true;
	_statusE0 = 3;
	setFileHash2(0x3A292504, 0x01084280, 0);
	_fileHash4 = 0x01084280;
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D360);
	SetSpriteCallback(NULL);
	sub41C7B0();
}

void Klayman::sub4209D0() {
	if (!stStartAction(AnimationCallback(&Klayman::sub4209D0))) {
		_status2 = 0;
		if (_y4 < _y) {
			if (_statusE0 == 1) {
				_statusE0 = 2;
				sub420BC0();
			} else {
				sub41C7B0();
			}
		} else if (_statusE0 == 0) {
			_statusE0 = 2;
			_flagE5 = false;
			setFileHash(0x122D1505, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::handleMessage41E0D0);
			SetSpriteCallback(&Klayman::spriteUpdate41F230);
		} else if (_statusE0 == 3) {
			_statusE0 = 2;
			_flagE5 = false;
			setFileHash2(0x122D1505, 0x01084280, 0);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::handleMessage41E0D0);
			SetSpriteCallback(&Klayman::spriteUpdate41F230);
		} else if (_statusE0 == 1) {
			_statusE0 = 2;
			_flagE5 = true;
			setFileHash(0x122D1505, 29 - _frameIndex, -1);
		} 
	}
}

void Klayman::sub420BC0() {
	_status2 = 2;
	if (_statusE0 == 1) {
		_statusE0 = 0;
		_flagE5 = false;
		setFileHash2(0x3A292504, 0x02421405, 0);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41DFD0);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	} else if (_statusE0 == 1) {
		_statusE0 = 0;
		_flagE5 = false;
		setFileHash2(0x122D1505, 0x02421405, 0);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41DFD0);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	} else {
		sub41C7B0();
	}
}

uint32 Klayman::handleMessage41DFD0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x489B025C) {
			_soundResource1.play(0x52C4C2D7);
		} else if (param.asInteger() == 0x400A0E64) {
			_soundResource1.play(0x50E081D9);
		} else if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

uint32 Klayman::handleMessage41E0D0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			_flagE5 = true;
		} else if (param.asInteger() == 0x489B025C) {
			_soundResource1.play(0x52C4C2D7);
		} else if (param.asInteger() == 0x400A0E64) {
			_soundResource1.play(0x50E081D9);
		} else if (param.asInteger() == 0x02421405) {
			if (_statusE0 == 1) {
				setFileHash2(0x3A292504, 0x01084280, 0);
			} else {
				setFileHash2(0x122D1505, 0x01084280, 0);
			}
			if (_statusE0 == 1) {
				if (_y4 >= _y - 30) {
					sendMessage(this, 0x1019, 0);
				}
			} else {
				if (_y4 <= _y) {
					sendMessage(this, 0x1019, 0);
				}
			}
		}
		break;
	}
	return messageResult;
}

void Klayman::sub420AD0() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420AD0))) {
		_status2 = 0;
		if (_y4 >= _y - 30) {
			sub41C7B0();
		} else if (_statusE0 == 0) {
			_statusE0 = 1;
			_flagE5 = false;
			setFileHash(0x3A292504, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::handleMessage41E0D0);
			SetSpriteCallback(&Klayman::spriteUpdate41F230);
		} else if (_statusE0 == 3) {
			_statusE0 = 1;
			_flagE5 = true;
			setFileHash2(0x3A292504, 0x01084280, 0);
			SetUpdateHandler(&Klayman::update);
			SetMessageHandler(&Klayman::handleMessage41E0D0);
			SetSpriteCallback(&Klayman::spriteUpdate41F230);
		} else if (_statusE0 == 2) {
			_statusE0 = 1;
			_flagE5 = true;
			setFileHash(0x3A292504, 29 - _frameIndex, -1);
		}
	}
}

void Klayman::sub421030() {
	_status2 = 2;
	_flagE5 = false;
	setFileHash2(0xF229C003, 0x14884392, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41E490);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
}

uint32 Klayman::handleMessage41E490(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x80C110B5) {
			sendMessage(_parentScene, 0x482A, 0);
		} else if (param.asInteger() == 0x110010D1) {
			sendMessage(_parentScene, 0x482B, 0);
		} else if (param.asInteger() == 0x32180101) {
			if (_soundFlag) {
				_soundResource1.play(0x48498E46);
			} else {
				_soundResource1.play(0x405002D8);
			}
		} else if (param.asInteger() == 0x0A2A9098) {
			if (_soundFlag) {
				_soundResource1.play(0x50399F64);
			} else {
				_soundResource1.play(0x0460E2FA);
			}
		}
		break;
	}
	return messageResult;
}

void Klayman::sub420FE0() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420FE0))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0xF229C003, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41E490);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	}
}

void Klayman::sub4210C0() {
	if (!stStartAction(AnimationCallback(&Klayman::sub4210C0))) {
		_status2 = 0;
		_flagE5 = false;
		setFileHash2(0xCA221107, 0x8520108C, 0);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41E490);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	}
}

void Klayman::sub421070() {
	if (!stStartAction(AnimationCallback(&Klayman::sub421070))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0xCA221107, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41E490);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	}
}

void Klayman::stLandOnFeet() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x18118554, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41E290);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41E290(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x320AC306) {
			_soundResource1.play(0x5860C640);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub420ED0() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420ED0))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x91540140, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41E2F0);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
	}
}

uint32 Klayman::handleMessage41E2F0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC61A0119) {
			_soundResource1.play(0x402338C2);
		} else if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x4924AAC4);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0A2AA8E0);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub420750() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420750))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x5CCCB330, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
		SetMessageHandler(&Klayman::handleMessage41DD20);
	}
}

void Klayman::stTurnToUse() {
	if (!stStartAction(AnimationCallback(&Klayman::stTurnToUse))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x9B250AD2, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
		SetMessageHandler(&Klayman::handleMessage41EEF0);
	}
}

void Klayman::sub4207F0() {
	_status2 = 2;
	_flagE5 = false;
	setFileHash(0x98F88391, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	SetMessageHandler(&Klayman::handleMessage41EEF0);
}

void Klayman::sub420F20() {
	_flagF8 = false;
	_flagE5 = false;
	setFileHash(0x11A8E012, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F5A0);
	SetMessageHandler(&Klayman::handleMessage41EC70);
}

void Klayman::spriteUpdate41F5A0() {
	if (!_flagF8 && ABS(_x4 - _x) < 80) {
		sendMessage(_parentScene, 0x4829, 0);
		_flagF8 = true;
	}
	AnimatedSprite::updateDeltaXY();
}

void Klayman::stMoveObjectSkipTurnFaceObject() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	_flagE4 = false;
	_flagE5 = true;
	setFileHash2(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	SetMessageHandler(&Klayman::handleMessage41D970);
}

void Klayman::sub420660() {
	sendMessage(_attachedSprite, 0x4807, 0);
}

uint32 Klayman::handleMessage41D970(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			if (_attachedSprite)
				sendMessage(_attachedSprite, 0x480B, _doDeltaX ? 1 : 0);
		} else if (param.asInteger() == 0x02421405) {
			if (_flagE4 && sendMessage(_attachedSprite, 0x480C, _doDeltaX ? 1 : 0) != 0) {
				stMoveObjectSkipTurn();
			} else {
				FinalizeState(&Klayman::sub420660);
				SetMessageHandler(&Klayman::handleMessage41D480);
			}
		} else if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0460E2FA);
		}
		break;
	case 0x480A:
		_flagE4 = true;
		return 0;		
	}
	return handleMessage41D480(messageNum, param, sender);
}

void Klayman::stMoveObjectSkipTurn() {
	_flagE4 = false;
	_flagE5 = true;
	setFileHash2(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	SetMessageHandler(&Klayman::handleMessage41D970);
}

void Klayman::stMoveObjectFaceObject() {
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	if (!stStartAction(AnimationCallback(&Klayman::stMoveObjectFaceObject))) {
		_status2 = 2;
		_flagE4 = false;
		_flagE5 = true;
		setFileHash(0x0C1CA072, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
		SetMessageHandler(&Klayman::handleMessage41D970);
	}
}

void Klayman::sub420C50() {
	if (!stStartAction(AnimationCallback(&Klayman::sub420C50))) {
		_status2 = 0;
		if (_flagF7) {
			stReleaseLeverUp();
		} else {
			sendMessage(_attachedSprite, 0x482B, 0);
			setFileHash(0x0C303040, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetSpriteCallback(&Klayman::spriteUpdate41F230);
			SetMessageHandler(&Klayman::handleMessage41E210);
			NextState(&Klayman::stPullLeverDown);
			_flagE5 = false;
		}
	}
}

// Exactly the same code as sub420DA0 which was removed
void Klayman::stPullLeverDown() {
	setFileHash(0x0D318140, 0, -1);
	sendMessage(_attachedSprite, 0x480F, 0);
	NextState(&Klayman::stHoldLeverDown);
}

void Klayman::stHoldLeverDown() {
	setFileHash(0x4464A440, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	SetMessageHandler(&Klayman::handleMessage41D360);
	_flagF7 = true;
	_flagE5 = true;
}

void Klayman::stReleaseLeverUp() {
	setFileHash(0x09018068, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	SetMessageHandler(&Klayman::handleMessage41E210);
	sendMessage(_attachedSprite, 0x4807, 0);
	NextState(&Klayman::stPullLeverDown);
	_flagE5 = false;
}

void Klayman::sub420E20() {
	if (_flagF7) {
		_status2 = 2;
		setFileHash(0x09018068, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&Klayman::spriteUpdate41F230);
		SetMessageHandler(&Klayman::handleMessage41E210);
		sendMessage(_attachedSprite, 0x4807, 0);
		NextState(&Klayman::sub420E90);
		_flagE5 = false;
		_flagF7 = false;
	} else {
		sub41C7B0();
	}
}

void Klayman::sub420E90() {
	setFileHash(0x0928C048, 0, -1);
	FinalizeState(&Klayman::sub420EB0);
}

void Klayman::sub420EB0() {
	sendMessage(_attachedSprite, 0x482A, 0);
}

void Klayman::sub420680() {
	if (!stStartActionFromIdle(AnimationCallback(&Klayman::sub420680))) {
		_status2 = 2;
		_counter2 = 0;
		for (uint32 i = 0; i < 20; i++) {
			if (getSubVar(0x02038314, i)) {
				setSubVar(0x02720344, i, 1);
				setSubVar(0x02038314, i, 0);
				_counter2++;
			}
		}
		if (_counter2 == 0) {
			gotoState(NULL);
			sub41C7B0();
		} else {
			setFileHash(0xD8C8D100, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetSpriteCallback(&Klayman::spriteUpdate41F250);
			SetMessageHandler(&Klayman::handleMessage41DB90);
			_flagE5 = false;
			_counter2--;
		}
	}
}

uint32 Klayman::handleMessage41DB90(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x06040580) {
			if (_counter2 == 0) {
				// TODO: Calc calcHash value somewhere else 
				setFileHash3(0xD8C8D100, calcHash("GoToStartLoop/Finish"), 0);
			}
		} else if (_counter2 != 0 && param.asInteger() == calcHash("GoToStartLoop/Finish")) {
			_counter2--;
			setFileHash2(0xD8C8D100, 0x01084280, 0);
		} else if (param.asInteger() == 0x062A1510) {
			_soundResource1.play(0x41688704);
		} else if (param.asInteger() == 0x02B20220) {
			_soundResource1.play(0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			_soundResource1.play(0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			_soundResource1.play(0x44051000);
		} else if (param.asInteger() == 0x0E040501) {
			_soundResource1.play(0xC6A129C1);
		}
	}
	return handleMessage41D480(messageNum, param, sender);
}

uint32 Klayman::handleMessage41DD20(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x040D4186) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x4808, 0);
			}
		}
		break;
	}
	return messageResult;
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
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x4804:
		if (param.asInteger() == 2)
			GotoState(&Klayman::stSleeping);
		break;
	case 0x480D:
		GotoState(&KmScene1001::sub44FA50);
		break;
	case 0x4812:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41C7B0();
		break;
	case 0x4836:
		if (param.asInteger() == 1) {
			sendMessage(_parentScene, 0x2002, 0);
			GotoState(&Klayman::stWakeUp);
		}
		break;		
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

void KmScene1001::sub44FA50() {
	if (!stStartAction(AnimationCallback(&KmScene1001::sub44FA50))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x00648953, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&KmScene1001::handleMessage44FA00);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	}
}

uint32 KmScene1001::handleMessage44FA00(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::handleMessage41E210(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x480F, 0);
		}
		break;
	}
	return messageResult;
}

// KmScene1002

KmScene1002::KmScene1002(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Sprite *class599, Sprite *ssLadderArch)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _otherSprite(NULL), _class599(class599), _ssLadderArch(ssLadderArch),
	_status(0) {
	
	setKlaymanTable1();
	
}

void KmScene1002::xUpdate() {
	if (_x >= 250 && _x <= 435 && _y >= 420) {
		if (_status == 0) {
			// TODO setKlaymanTable(stru_4B44C8);
			_status = 1;
		}
	} else if (_status == 1) {
		setKlaymanTable1();
		_status = 0;
	}
}
	
uint32 KmScene1002::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&KmScene1002::sub449E90);
		break;
	case 0x2007:
		_otherSprite = (Sprite*)param.asEntity();
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004: 
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4803:
		if (param.asInteger() == 1) {
			GotoState(&KmScene1002::stJumpAndFall);
		} else if (param.asInteger() == 2) {
			GotoState(&KmScene1002::stDropFromRing);
		}
		break;
	case 0x4804:
		GotoState(&Klayman::stPeekWall);
		break;
	case 0x4805:
		switch (param.asInteger()) {
		case 1:
			GotoState(&Klayman::sub420210);
			break;
		case 2:
			GotoState(&Klayman::sub420250);
			break;
		case 3:
			GotoState(&Klayman::sub420290);
			break;
		case 4:
			GotoState(&Klayman::sub420300);
			break;
		}
		break;
	case 0x480A:	  
		GotoState(&KmScene1002::stMoveVenusFlyTrap);
		break;
	case 0x480D:			   
		GotoState(&KmScene1002::sub449E20);
		break;
	case 0x4816:  
		if (param.asInteger() == 0) {
			GotoState(&KmScene1002::stPressDoorButton);
		}
		break;
	case 0x4817:				  
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	case 0x481B:				
		sub41CCE0(param.asInteger());
		break;
	case 0x4820:  
		sendMessage(_parentScene, 0x2005, 0);
		GotoState(&Klayman::sub420970);	 
		break;
	case 0x4821:	
		sendMessage(_parentScene, 0x2005, 0);
		_y4 = param.asInteger();
		GotoState(&Klayman::sub4209D0);	 
		break;
	case 0x4822:  
		sendMessage(_parentScene, 0x2005, 0);
		_y4 = param.asInteger();
		GotoState(&Klayman::sub420AD0);	 
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2006, 0);
		GotoState(&Klayman::sub420BC0);	 
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub421030);
		} else {
			GotoState(&Klayman::sub420FE0);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub4210C0);
		} else {
			GotoState(&Klayman::sub421070);
		}
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;
	case 0x4840: 
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

void KmScene1002::update4497D0() {
	Klayman::update();
	if (_counter1 != 0 && (--_counter1 == 0)) {
		_surface->setVisible(true);
		SetUpdateHandler(&Klayman::update);
	}
}

uint32 KmScene1002::handleMessage449800(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x168050A0) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x480F, 0);
			}
		} else if (param.asInteger() == 0x586B0300) {
			if (_otherSprite) {
				sendMessage(_otherSprite, 0x480E, 1);
			}
		} else if (param.asInteger() == 0x4AB28209) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x482A, 0);
			}
		} else if (param.asInteger() == 0x88001184) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x482B, 0);
			}
		}
		break;
	}
	return messageResult;
}

uint32 KmScene1002::handleMessage4498E0(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x4811:
		_soundResource1.play(0x5252A0E4);
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		if (_doDeltaX) {
			_x = ((Sprite*)sender)->getX() - 75;
		} else {
			_x = ((Sprite*)sender)->getX() + 75;
		}
		_y = ((Sprite*)sender)->getY() - 200;
		if (param.asInteger() == 0) {
			sub449EF0();
		} else if (param.asInteger() == 1) {
			sub44A0D0();
		} else if (param.asInteger() == 2) {
			stSpitOutFall();
		}
		break;
	}
	return 0;
}

uint32 KmScene1002::hmPressDoorButton(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x942D2081) {
			_flagE5 = false;
			sendMessage(_attachedSprite, 0x2003, 0);
		} else if (param.asInteger() == 0xDA600012) {
			stHitByBoxingGlove();
		} else if (param.asInteger() == 0x0D01B294) {
			_flagE5 = false;
			sendMessage(_attachedSprite, 0x480B, 0);
		}
		break;
	}
	return messageResult;
}

uint32 KmScene1002::hmMoveVenusFlyTrap(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x480B, (uint32)_doDeltaX);
			}
		} else if (param.asInteger() == 0x02421405) {
			if (_flagE4) {
				if (_attachedSprite) {
					if (sendMessage(_attachedSprite, 0x480C, (uint32)_doDeltaX) != 0) {
						stContinueMovingVenusFlyTrap();
					}
				}
			} else {
				SetMessageHandler(&KmScene1002::handleMessage449BA0);
			}
		} else if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		} else if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0460E2FA);
		}
		break;
	case 0x480A:
		_flagE4 = true;
		return 0;
	}
	return handleMessage41D480(messageNum, param, sender);
}

uint32 KmScene1002::handleMessage449BA0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_attachedSprite, 0x482A, 0);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_attachedSprite, 0x482B, 0);
		} else if (param.asInteger() == 0x32180101) {
			_soundResource1.play(0x405002D8);
		} else if (param.asInteger() == 0x0A2A9098) {
			_soundResource1.play(0x0460E2FA);
		}
		break;
	}
	return messageResult;
}

uint32 KmScene1002::handleMessage449C90(int messageNum, const MessageParam &param, Entity *sender) {
	int16 speedUpFrameIndex;
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymanSpeedUpHash);
		if (_frameIndex < speedUpFrameIndex) {
			setFileHash(0x35AA8059, speedUpFrameIndex, -1);
			_y = 435;
		}
		messageResult = 0;
		break;
	case 0x100D:
		if (param.asInteger() == 0x1A1A0785) {
			_soundResource1.play(0x40F0A342);
		} else if (param.asInteger() == 0x60428026) {
			_soundResource1.play(0x40608A59);
		}
		break;
	}
	return messageResult;
}

uint32 KmScene1002::handleMessage449D60(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x1307050A) {
			_soundResource1.play(0x40428A09);
		}
		break;
	}
	return messageResult;
}

void KmScene1002::suFallDown() {
	AnimatedSprite::updateDeltaXY();
	HitRect *hitRect = _vm->_collisionMan->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		processDelta();
		sendMessage(this, 0x1019, 0);
	}
	_vm->_collisionMan->checkCollision(this, 0xFFFF, 0x4810, 0);
}

void KmScene1002::sub449E20() {
	if (!stStartAction(AnimationCallback(&KmScene1002::sub449E20))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x584984B4, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
		SetMessageHandler(&KmScene1002::handleMessage449800);
		NextState(&Klayman::stLandOnFeet);
		sendMessage(_attachedSprite, 0x482B, 0);
	}
}

void KmScene1002::sub449E90() {
	_soundResource1.play(0x56548280);
	_status2 = 0;
	_flagE5 = false;
	_surface->setVisible(false);
	setFileHash(0x5420E254, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1002::handleMessage4498E0);
}

void KmScene1002::sub449EF0() {
	_counter1 = 1;
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x000BAB02, 0, -1);
	SetUpdateHandler(&KmScene1002::update4497D0);
	// Weird stuff happening
	SetMessageHandler(&Klayman::handleMessage41D360);
	//SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(&KmScene1002::suFallDown);
	NextState(&KmScene1002::sub449F70);
	sendMessage(_class599, 0x482A, 0);
	sendMessage(_ssLadderArch, 0x482A, 0);
}

void KmScene1002::sub449F70() {
	sendMessage(_parentScene, 0x1024, 1);
	_soundResource1.play(0x41648271);
	_status2 = 1;
	_flagE5 = false;
	_flagE1 = false;
	setFileHash2(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1002::handleMessage41D480);
	NextState(&KmScene1002::sub44A230);
	sendMessage(_parentScene, 0x2002, 0);
	// TODO _callbackList = NULL;
	_attachedSprite = NULL;
	sendMessage(_class599, 0x482B, 0);
	sendMessage(_ssLadderArch, 0x482B, 0);
}

void KmScene1002::stSpitOutFall() {
	_counter1 = 1;
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x9308C132, 0, -1);
	SetUpdateHandler(&KmScene1002::update4497D0);
	SetSpriteCallback(&KmScene1002::suFallDown);
	SetMessageHandler(&Klayman::handleMessage41D480);
	NextState(&KmScene1002::sub449F70);
	sendMessage(_class599, 0x482A, 0);
	sendMessage(_ssLadderArch, 0x482A, 0);
}

void KmScene1002::sub44A0D0() {
	_counter1 = 1;
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x0013A206, 0, -1);
	SetUpdateHandler(&KmScene1002::update4497D0);
	SetMessageHandler(&Klayman::handleMessage41D360);
	SetSpriteCallback(&KmScene1002::suFallDown);
	NextState(&KmScene1002::sub44A150);
	sendMessage(_class599, 0x482A, 0);
	sendMessage(_ssLadderArch, 0x482A, 0);
}

void KmScene1002::sub44A150() {
	sendMessage(_parentScene, 0x1024, 1);
	_soundResource1.play(0x41648271);
	_status2 = 1;
	_flagE5 = false;
	_flagE1 = false;
	setFileHash2(0x0013A206, 0x88003000, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&KmScene1002::handleMessage41D480);
	SetSpriteCallback(NULL);
	NextState(&KmScene1002::sub44A230);
	sendMessage(_parentScene, 0x2002, 0);
	// TODO _callbackList = NULL;
	_attachedSprite = NULL;
	sendMessage(_class599, 0x482B, 0);
	sendMessage(_ssLadderArch, 0x482B, 0);
}

void KmScene1002::sub44A230() {
	setDoDeltaX(2);
	stTryStandIdle();
}

void KmScene1002::stJumpAndFall() {
	if (!stStartAction(AnimationCallback(&KmScene1002::stJumpAndFall))) {
		sendMessage(_parentScene, 0x1024, 3);
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0xB93AB151, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&KmScene1002::handleMessage449D60);
		SetSpriteCallback(&KmScene1002::suFallDown);
		NextState(&Klayman::stLandOnFeet);
	}
}

void KmScene1002::stDropFromRing() {
	if (_attachedSprite) {
		_x = _attachedSprite->getX();
		sendMessage(_attachedSprite, 0x4807, 0);
		_attachedSprite = NULL;
	}
	_status2 = 2;
	_flagE5 = false;
	setFileHash(0x586984B1, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&KmScene1002::handleMessage41D360);
	SetSpriteCallback(&KmScene1002::suFallDown);
	NextState(&Klayman::stLandOnFeet);
}

void KmScene1002::stPressDoorButton() {
	_status2 = 2;
	_flagE5 = true;
	setDoDeltaX(0);
	setFileHash(0x1CD89029, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&KmScene1002::hmPressDoorButton);
	SetSpriteCallback(&Klayman::spriteUpdate41F250);
}

void KmScene1002::stHitByBoxingGlove() {
	_status2 = 1;
	_flagE5 = false;
	setFileHash(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&KmScene1002::handleMessage449C90);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	FinalizeState(&KmScene1002::stHitByBoxingGloveDone);
}

void KmScene1002::stHitByBoxingGloveDone() {
	sendMessage(_parentScene, 0x1024, 1);
}

void KmScene1002::stMoveVenusFlyTrap() {
	if (!stStartAction(AnimationCallback(&KmScene1002::stMoveVenusFlyTrap))) {
		_status2 = 2;
		_flagE4 = false;
		_flagE5 = true;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		setFileHash(0x5C01A870, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&KmScene1002::hmMoveVenusFlyTrap);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
		FinalizeState(&KmScene1002::stMoveVenusFlyTrapDone);
	}
}

void KmScene1002::stContinueMovingVenusFlyTrap() {
	_flagE4 = false;
	_flagE5 = true;
	setFileHash2(0x5C01A870, 0x01084280, 0);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&KmScene1002::hmMoveVenusFlyTrap);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	FinalizeState(&KmScene1002::stMoveVenusFlyTrapDone);
}

void KmScene1002::stMoveVenusFlyTrapDone() {
	sendMessage(_attachedSprite, 0x482A, 0);
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
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	case 0x4818:
		sub41C930(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481E:
		GotoState(&KmScene1004::stReadNote);
		break;
	case 0x4820:
		sendMessage(_parentScene, 0x2000, 0);
		GotoState(&Klayman::sub420970);
		break;
	case 0x4821:
		sendMessage(_parentScene, 0x2000, 0);
		_y4 = param.asInteger();
		GotoState(&Klayman::sub4209D0);
		break;
	case 0x4822:
		sendMessage(_parentScene, 0x2000, 0);
		_y4 = param.asInteger();
		GotoState(&Klayman::sub420AD0);
		break;
	case 0x4823:
		sendMessage(_parentScene, 0x2001, 0);
		GotoState(&Klayman::sub420BC0);
		break;
	case 0x4824:
		sendMessage(_parentScene, 0x2000, 0);
		_y4 = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klayman::sub4209D0);
		break;
	case 0x4825:
		sendMessage(_parentScene, 0x2000, 0);
		_y4 = _dataResource.getPoint(param.asInteger()).y;
		GotoState(&Klayman::sub420AD0);
		break;
	case 0x4828:
		GotoState(&Klayman::sub420ED0);
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

uint32 KmScene1004::hmReadNote(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x04684052) {
			_flagE5 = true;
			sendMessage(_parentScene, 0x2002, 0);
		}
		break;
	}
	return messageResult;
}

void KmScene1004::stReadNote() {
	_status2 = 2;
	_flagE5 = false;
	setFileHash(0x123E9C9F, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&KmScene1004::hmReadNote);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
}

KmScene1109::KmScene1109(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _flag1(false) {
	
	// Empty
}

uint32 KmScene1109::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2000:
		_flag1 = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_flag1)
			GotoState(&Klayman::sub421350);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&Klayman::stWalking);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	case 0x481D:
		if (_flag1)
			GotoState(&Klayman::sub4214D0);
		break;
	case 0x481E:
		if (_flag)
			GotoState(&Klayman::sub421510);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_flag1 = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_flag1 = false;
		GotoState(&Klayman::sub421310);
		break;
	case 0x483D:
		sub461F30();
		break;
	case 0x483E:
		sub461F70();
		break;
	}
	return 0;
}

uint32 KmScene1109::handleMessage461EA0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4E0A2C24) {
			_soundResource1.play(0x85B10BB8);
		} else if (param.asInteger() == 0x4E6A0CA0) {
			_soundResource1.play(0xC5B709B0);
		}
		break;
	}
	return messageResult;
}

void KmScene1109::sub461F30() {
	_status2 = 0;
	_flagE5 = false;
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1109::handleMessage461EA0);
	setFileHash(0x2C2A4A1C, 0, -1);
}

void KmScene1109::sub461F70() {
	_status2 = 0;
	_flagE5 = false;
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1109::handleMessage461EA0);
	setFileHash(0x3C2E4245, 0, -1);
}

// KmScene1201

KmScene1201::KmScene1201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, Entity *class464)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _class464(class464), _countdown(0) {
	
	// TODO setKlaymanTable(dword_4AEF10, 3);
	_flagF6 = true;
	
}

uint32 KmScene1201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480A:
		GotoState(&KmScene1201::sub40DF00);
		break;
	case 0x4812:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4813:
		GotoState(&KmScene1201::stFetchMatch);
		break;
	case 0x4814:
		GotoState(&KmScene1201::stTumbleHeadless);
		break;
	case 0x4815:
		GotoState(&KmScene1201::sub40E040);
		break;
	case 0x4816:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		GotoState(&Klayman::sub4207F0);
		break;
	case 0x481F:
		GotoState(&Klayman::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		sub41C7B0();
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

void KmScene1201::update40DBE0() {
	if (_x >= 380)
		sub41C7B0();
	Klayman::update();		
}

uint32 KmScene1201::hmMatch(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x51281850) {
			setGlobalVar(0x20A0C516, 1);
		} else if (param.asInteger() == 0x43000538) {
			_soundResource1.play(0x21043059);
		} else if (param.asInteger() == 0x02B20220) {
			_soundResource1.play(0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			_soundResource1.play(0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			_soundResource1.play(0x44051000);
		}
		break;
	}
	return messageResult;
}

void KmScene1201::stFetchMatch() {
	if (!stStartAction(AnimationCallback(&KmScene1201::stFetchMatch))) {
		_status2 = 0;
		_flagE5 = false;
		setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
		setFileHash(0x9CAA0218, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(NULL);
		SetMessageHandler(&KmScene1201::hmMatch);
		NextState(&KmScene1201::stLightMatch);
	}
}

void KmScene1201::stLightMatch() {
	_status2 = 1;
	_flagE5 = false;
	setDoDeltaX(_attachedSprite->getX() < _x ? 1 : 0);
	setFileHash(0x1222A513, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1201::hmMatch);
}

uint32 KmScene1201::handleMessage40DDF0(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x01084280) {
			_soundResource1.play(0x405002D8);
			if (_attachedSprite) {
				sendMessage(_attachedSprite, 0x480B, 0);
			}
		} else if (param.asInteger() == 0x02421405) {
			if (_countdown != 0) {
				_countdown--;
				stMoveObjectSkipTurn();
			} else {
				SetMessageHandler(&Klayman::handleMessage41D480);
			}
		}
		break;
	}
	return Klayman::handleMessage41D480(messageNum, param, sender);
}

uint32 KmScene1201::hmTumbleHeadless(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000F0082) {
			_soundResource1.play(0x74E2810F);
		}
		break;
	}
	return messageResult;
}

void KmScene1201::sub40DF00() {
	if (!stStartAction(AnimationCallback(&KmScene1201::sub40DF00))) {
		_status2 = 2;
		_flagE5 = false;
		_countdown = 8;
		setDoDeltaX(0);
		setFileHash(0x0C1CA072, 0, -1);
		SetUpdateHandler(&KmScene1201::update40DBE0);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
		SetMessageHandler(&KmScene1201::handleMessage40DDF0);
	}
}

void KmScene1201::stMoveObjectSkipTurn() {
	_flagE5 = false;
	setFileHash2(0x0C1CA072, 0x01084280, 0);
	SetUpdateHandler(&KmScene1201::update40DBE0);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	SetMessageHandler(&KmScene1201::handleMessage40DDF0);
}

void KmScene1201::stTumbleHeadless() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene1201::stTumbleHeadless))) {
		_status2 = 1;
		_flagE5 = false;
		setDoDeltaX(0);
		setFileHash(0x2821C590, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
		SetMessageHandler(&KmScene1201::hmTumbleHeadless);
		NextState(&Klayman::stTryStandIdle);
		sendMessage(_class464, 0x2006, 0);
		_soundResource1.play(0x62E0A356);
	}
}

void KmScene1201::sub40E040() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene1201::sub40E040))) {
		_status2 = 1;
		_flagE5 = false;		
		setFileHash(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(NULL);
		SetMessageHandler(&Klayman::handleMessage41D360);
	}
}

KmScene1303::KmScene1303(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty
}

uint32 KmScene1303::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4804:
		GotoState(&KmScene1303::stPeekWall1);
		break;
	case 0x483B:
		GotoState(&KmScene1303::stPeekWallReturn);
		break;
	case 0x483C:
		GotoState(&KmScene1303::stPeekWall2);
		break;
	}
	return 0;
}

uint32 KmScene1303::hmPeekWallReturn(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == calcHash("PopBalloon")) {
			sendMessage(_parentScene, 0x2000, 0);
		} else if (param.asInteger() == 0x02B20220) {
			_soundResource1.play(0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			_soundResource1.play(0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			_soundResource1.play(0x44051000);
		}
		break;
	}
	return messageResult;
}

void KmScene1303::update4161A0() {
	Klayman::update();
	_counter3++;
	if (_counter3 >= _counter3Max)
		stPeekWall3();
}

void KmScene1303::stPeekWall1() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0xAC20C012, 8, 37);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&Klayman::handleMessage41D480);
	NextState(&KmScene1303::stPeekWall3);
}

void KmScene1303::stPeekWall2() {
	_status2 = 1;
	_flagE5 = false;
	setFileHash(0xAC20C012, 43, 49);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&Klayman::handleMessage41D480);
}

void KmScene1303::stPeekWall3() {
	_counter3 = 0;
	_status2 = 0;
	_flagE5 = true;
	_counter3Max = _vm->_rnd->getRandomNumber(64) + 24;
	setFileHash(0xAC20C012, 38, 42);
	SetUpdateHandler(&KmScene1303::update4161A0);
	SetSpriteCallback(NULL);
	SetMessageHandler(&Klayman::handleMessage41D360);
	_newHashListIndex = 42;
}

void KmScene1303::stPeekWallReturn() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x2426932E, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1303::hmPeekWallReturn);
}

KmScene1304::KmScene1304(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	// Empty	
}

uint32 KmScene1304::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
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
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;		
	case 0x4804:
		GotoState(&KmScene1305::stCrashDown);
		break;		
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	}
	return 0;
}

void KmScene1305::stCrashDown() {
	_soundResource1.play(0x41648271);
	_status2 = 1;
	_flagE5 = false;
	setFileHash2(0x000BAB02, 0x88003000, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&Klayman::handleMessage41D480);
	NextState(&KmScene1305::cbCrashDownEvent);
}

void KmScene1305::cbCrashDownEvent() {
	setDoDeltaX(2);
	stTryStandIdle();
}

KmScene1306::KmScene1306(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	_flag1 = false;	
}

uint32 KmScene1306::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_flag1 = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_flag1)
			GotoState(&Klayman::sub421350);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481A:
		GotoState(&Klayman::sub420680);		
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_flag1)
			GotoState(&Klayman::sub4214D0);
		else
			GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		if (_flag1)
			GotoState(&Klayman::sub421510);
		else
			GotoState(&Klayman::sub4207F0);
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
		sub41C7B0();
		break;
	case 0x482E:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub421030);
		} else {
			GotoState(&Klayman::sub420FE0);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub4210C0);
		} else {
			GotoState(&Klayman::sub421070);
		}
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_flag1 = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_flag1 = false;
		GotoState(&Klayman::sub421310);
		break;
	case 0x483D:
		sub417D40();
		break;																				
	case 0x483E:
		sub417D80();
		break;																				
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return messageResult;
}

void KmScene1306::sub417D40() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0xEE084A04, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1306::handleMessage417CB0);
}

void KmScene1306::sub417D80() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0xB86A4274, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1306::handleMessage417CB0);
}

uint32 KmScene1306::handleMessage417CB0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4E0A2C24) {
			_soundResource1.play(0x85B10BB8);
		} else if (param.asInteger() == 0x4E6A0CA0) {
			_soundResource1.play(0xC5B709B0);
		}
	}
	return messageResult;
}

KmScene1308::KmScene1308(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	
	_flag1 = false;
}

uint32 KmScene1308::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
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
		GotoState(&KmScene1001::sub420C50);
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481A:
		if (param.asInteger() == 1) {
			GotoState(&KmScene1308::sub456150);		
		} else {
			GotoState(&Klayman::sub420680);		
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
		GotoState(&Klayman::sub4207F0);
		break;
	case 0x4827:
		GotoState(&Klayman::sub420E20);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

uint32 KmScene1308::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Klayman::handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (!_flag1 && param.asInteger() == 0x06040580) {
			setFileHash3(0xDC409440, 0x46431401, 0);
		} else if (_flag1 && param.asInteger() == 0x46431401) {
			_flag1 = false;
			setFileHash2(0xDC409440, 0x01084280, 0);
		} else if (param.asInteger() == 0x062A1510) {
			_soundResource1.play(0x41688704);
		} else if (param.asInteger() == 0x02B20220) {
			_soundResource1.play(0xC5408620);
		} else if (param.asInteger() == 0x0A720138) {
			_soundResource1.play(0xD4C08010);
		} else if (param.asInteger() == 0xB613A180) {
			_soundResource1.play(0x44051000);
		} else if (param.asInteger() == 0x0E4C8141) {
			_soundResource1.play(0xDC4A1280);
		}
		break;
	}
	return messageResult;
}

void KmScene1308::sub456150() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene1308::sub456150))) {
		_status2 = 2;
		_flag1 = false;
		for (uint i = 0; i < 3; i++) {
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
				_flag1 = true;
			} 
		}
		if (!_flag1) {
			gotoState(NULL);
			sub41C7B0();
		} else {
			_flagE5 = false;
			setFileHash(0xDC409440, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetSpriteCallback(&Klayman::spriteUpdate41F250);
			SetMessageHandler(&KmScene1308::handleMessage);
			_flag1 = false;
		}
	}
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
		sub41C930(param.asPoint().x, false);
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
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41C7B0();
		break;
	case 0x482E:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub421030);
		} else {
			GotoState(&Klayman::sub420FE0);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub4210C0);
		} else {
			GotoState(&Klayman::sub421070);
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
		sub41C930(param.asPoint().x, false);
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
		sub41C7B0();
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
		GotoState(&Klayman::sub4207F0);
		break;
	}
	return 0;
}

// KmScene1403

KmScene1403::KmScene1403(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	setKlaymanTable(klaymanTable4, ARRAYSIZE(klaymanTable4));
}

uint32 KmScene1403::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
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
		GotoState(&KmScene1001::sub420C50);
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x4827:
		GotoState(&Klayman::sub420E20);
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
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
		sub41C930(param.asPoint().x, false);
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
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	case 0x481A:
		GotoState(&Klayman::sub420680);		
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
		GotoState(&Klayman::sub4207F0);
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
		sub41C7B0();
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

KmScene1608::KmScene1608(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _flag1(false) {
}

uint32 KmScene1608::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2032:
		_flag1 = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_flag1)
			GotoState(&Klayman::sub421350);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_flag1)
			GotoState(&Klayman::sub4214D0);
		break;
	case 0x481E:
		if (_flag)
			GotoState(&Klayman::sub421510);
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
		sub41C7B0();
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2032, 1);
		_flag1 = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2032, 0);
		_flag1 = false;
		GotoState(&Klayman::sub421310);
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

// KmScene1705

KmScene1705::KmScene1705(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _flag(false) {

	// Empty	
}

uint32 KmScene1705::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_flag = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_flag) {
			GotoState(&Klayman::sub421350);
		} else {
			GotoState(&Klayman::stTryStandIdle);
		}
		break;
	case 0x4803:
		GotoState(&KmScene1705::stFallSkipJump);
		break;				
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_flag) {
			GotoState(&Klayman::sub4214D0);
		}
		break;
	case 0x481E:
		if (_flag) {
			GotoState(&Klayman::sub421510);
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
		_flag = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_flag = false;
		GotoState(&Klayman::sub421310);
		break;
	case 0x483D:
		sub468AD0();
		break;																				
	case 0x483E:
		sub468B10();
		break;																				
	}
	return messageResult;
}

uint32 KmScene1705::handleMessage4689A0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4E0A2C24) {
			_soundResource1.play(0x85B10BB8);
		} else if (param.asInteger() == 0x4E6A0CA0) {
			_soundResource1.play(0xC5B709B0);
		}
		break;
	}
	return messageResult;
}

void KmScene1705::spriteUpdate468A30() {
	updateDeltaXY();
	HitRect *hitRect = _vm->_collisionMan->findHitRectAtPos(_x, _y + 10);
	if (hitRect->type == 0x5001) {
		_y = hitRect->rect.y1;
		processDelta();
		sendMessage(this, 0x1019, 0);
	}
}

void KmScene1705::stFallSkipJump() {
	_status2 = 2;
	_flagE5 = false;
	setFileHash2(0xB93AB151, 0x40A100F8, 0);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&KmScene1705::spriteUpdate468A30);
	SetMessageHandler(&Klayman::handleMessage41D360);
	NextState(&Klayman::stLandOnFeet);
}

void KmScene1705::sub468AD0() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x5E0A4905, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1705::handleMessage4689A0);
}

void KmScene1705::sub468B10() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0xD86E4477, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene1705::handleMessage4689A0);
}

KmScene1901::KmScene1901(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	// Empty	
}

uint32 KmScene1901::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481D:
		GotoState(&Klayman::stTurnToUse);
		break;
	case 0x481E:
		GotoState(&Klayman::sub4207F0);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		sub41C7B0();
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

KmScene2001::KmScene2001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _flag(false) {

	// Empty	
}

uint32 KmScene2001::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2000:
		_flag = param.asInteger() != 0;
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_flag) {
			GotoState(&Klayman::sub421350);
		} else {
			GotoState(&Klayman::stTryStandIdle);
		}
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&Klayman::stWalking);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;
	case 0x481D:
		if (_flag) {
			GotoState(&Klayman::sub4214D0);
		}
		break;
	case 0x481E:
		if (_flag) {
			GotoState(&Klayman::sub421510);
		}
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_flag = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_flag = false;
		GotoState(&Klayman::sub421310);
		break;
	case 0x483D:
		sub440230();
		break;
	case 0x483E:
		stDoTeleport();
		break;
	}
	return 0;
}

uint32 KmScene2001::handleMessage4401A0(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4E0A2C24) {
			_soundResource1.play(0x85B10BB8);
		} if (param.asInteger() == 0x4E6A0CA0) {
			_soundResource1.play(0xC5B709B0);
		}
		break;
	}
	return messageResult;
}

void KmScene2001::sub440230() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0xBE68CC54, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene2001::handleMessage4401A0);
}

void KmScene2001::stDoTeleport() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x18AB4ED4, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene2001::handleMessage4401A0);
}

KmScene2101::KmScene2101(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000), _flag1(false) {
	
	// Empty
}

uint32 KmScene2101::xHandleMessage(int messageNum, const MessageParam &param) {
	uint32 messageResult = 0;
	switch (messageNum) {
	case 0x2000:
		_flag1 = param.asInteger() != 0;
		messageResult = 1;
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		if (_flag1)
			GotoState(&Klayman::sub421350);
		else
			GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4811:
		GotoState(&KmScene2101::sub4862C0);
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x481D:
		if (_flag1)
			GotoState(&Klayman::sub4214D0);
		break;
	case 0x481E:
		if (_flag)
			GotoState(&Klayman::sub421510);
		break;
	case 0x4834:
		GotoState(&Klayman::stStepOver);
		break;
	case 0x4835:
		sendMessage(_parentScene, 0x2000, 1);
		_flag1 = true;
		GotoState(&Klayman::stSitInTeleporter);
		break;																		
	case 0x4836:
		sendMessage(_parentScene, 0x2000, 0);
		_flag1 = false;
		GotoState(&Klayman::sub421310);
		break;
	case 0x483D:
		sub486320();
		break;
	case 0x483E:
		sub486360();
		break;
	}
	return messageResult;	
}

uint32 KmScene2101::handleMessage486160(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	int16 speedUpFrameIndex;
	switch (messageNum) {
	case 0x1008:
		speedUpFrameIndex = getFrameIndex(kKlaymanSpeedUpHash);
		if (_frameIndex < speedUpFrameIndex) {
			setFileHash(0x35AA8059, speedUpFrameIndex, -1);
			_y = 438;
		}
		messageResult = 0;
		break;
	case 0x100D:
		if (param.asInteger() == 0x1A1A0785) {
			_soundResource1.play(0x40F0A342);
		} else if (param.asInteger() == 0x60428026) {
			_soundResource1.play(0x40608A59);
		}
		break;
	}
	return messageResult;
}

uint32 KmScene2101::handleMessage486230(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4E0A2C24) {
			_soundResource1.play(0x85B10BB8);
		} else if (param.asInteger() == 0x4E6A0CA0) {
			_soundResource1.play(0xC5B709B0);
		}
		break;
	}
	return messageResult;
}

void KmScene2101::sub4862C0() {
	_status2 = 1;
	_flagE5 = false;
	setFileHash(0x35AA8059, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	SetMessageHandler(&KmScene2101::handleMessage486160);
	_soundResource1.play(0x402E82D4);
}

void KmScene2101::sub486320() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0xFF290E30, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene2101::handleMessage486230);
}

void KmScene2101::sub486360() {
	_status2 = 0;
	_flagE5 = false;
	setFileHash(0x9A28CA1C, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(NULL);
	SetMessageHandler(&KmScene2101::handleMessage486230);
}

KmScene2201::KmScene2201(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	// TODO: NRect *rect1, int16 unk in Klayman ctor
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	_dataResource.load(0x04104242);
	_flagF6 = false;
}

uint32 KmScene2201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4812:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4816:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger() ? 1 : 0);
		sub41C7B0();
		break;
	case 0x4818:
		sub41C930(_dataResource.getPoint(param.asInteger()).x, false);
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
		GotoState(&Klayman::sub4207F0);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		sub41C7B0();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub421030);
		} else {
			GotoState(&Klayman::sub420FE0);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub4210C0);
		} else {
			GotoState(&Klayman::sub421070);
		}
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
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
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x4818:
		sub41C930(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4819:
		GotoState(&Klayman::sub420750);
		break;
	case 0x481A:
		GotoState(&Klayman::sub420680);		
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
		GotoState(&Klayman::sub4207F0);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		sub41C7B0();
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

KmScene2205::KmScene2205(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmScene2205::xUpdate() {
	setGlobalVar(0x18288913, _frameIndex);
}
	
uint32 KmScene2205::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&KmScene2205::sub423980);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4816:
		if (param.asInteger() == 0) {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x4818:
		sub41C930(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

void KmScene2205::sub423980() {
	int16 frameIndex = getGlobalVar(0x18288913);
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	NextState(&Klayman::sub41FA40);
	FinalizeState(&Klayman::stStartWalkingDone);
}

KmScene2206::KmScene2206(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {

	// TODO Sound1ChList_addSoundResource(0x80101800, 0xD3B02847);
}

KmScene2206::~KmScene2206() {
	// TODO Sound1ChList_sub_407AF0(0x80101800);
}

void KmScene2206::xUpdate() {
	setGlobalVar(0x18288913, _frameIndex);
}
	
uint32 KmScene2206::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4803:
		GotoState(&KmScene2206::sub482490);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&KmScene2206::sub482530);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4812:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41C7B0();
		break;
	case 0x482E:	 
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub421030);
		} else {
			GotoState(&Klayman::sub420FE0);
		}
		break;
	case 0x482F:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::sub4210C0);
		} else {
			GotoState(&Klayman::sub421070);
		}
		break;
	case 0x4837:
		sub41CE70();
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

void KmScene2206::spriteUpdate482450() {
	_yDelta++;
	_y += _yDelta;
	if (_y > 600) {
		sendMessage(this, 0x1019, 0);
	}
}

void KmScene2206::sub482490() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene2206::sub482490))) {
		_status2 = 1;
		sendMessage(_parentScene, 0x4803, 0);
		_flagE5 = false;
		_yDelta = 0;
		setFileHash(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41D360);
		SetSpriteCallback(&KmScene2206::spriteUpdate482450);
		// TODO Sound1ChList_playLooping(0xD3B02847);
	}
}

void KmScene2206::sub482530() {
	int16 frameIndex = getGlobalVar(0x18288913) + 1;
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	NextState(&Klayman::sub41FA40);
	FinalizeState(&Klayman::stStartWalkingDone);
}

KmScene2207::KmScene2207(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}
	
uint32 KmScene2207::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&KmScene2207::sub442460);
		break;
	case 0x2005:
		spriteUpdate442430();
		GotoState(&KmScene2207::stTryStandIdle);
		break;
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&KmScene2207::sub4424B0);
		break;
	case 0x4812:
		GotoState(&Klayman::stPickUpGeneric);
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			GotoState(&Klayman::stTurnPressButton);
		} else if (param.asInteger() == 2) {
			GotoState(&Klayman::stStampFloorButton);
		} else {
			GotoState(&Klayman::stPressButtonSide);
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		
	case 0x481B:
		if (param.asPoint().y != 0) {
			sub41CC40(param.asPoint().y, param.asPoint().x);
		} else {
			sub41CCE0(param.asPoint().x);
		}
		break;
	case 0x4827:
		GotoState(&Klayman::sub420E20);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		sub41C7B0();
		break;
	case 0x483F:
		sub41CD00(param.asInteger());
		break;		
	case 0x4840:
		sub41CD70(param.asInteger());
		break;
	}
	return 0;
}

void KmScene2207::spriteUpdate442430() {
	_x = _attachedSprite->getX() - 20;
	_y = _attachedSprite->getY() + 46;
	processDelta();
}

void KmScene2207::sub442460() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene2207::sub442460))) {
		_status2 = 1;
		_flagE5 = true;
		setFileHash(0x5420E254, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetSpriteCallback(&KmScene2207::spriteUpdate442430);
		SetMessageHandler(&Klayman::handleMessage41D360);
	}
}

void KmScene2207::sub4424B0() {
	if (!stStartAction(AnimationCallback(&KmScene2207::sub4424B0))) {
		_status2 = 0;
		if (_flagF7) {
			stReleaseLeverUp();
		} else {
			_flagE5 = false;
			setFileHash(0x0C303040, 0, -1);
			SetUpdateHandler(&Klayman::update);
			SetSpriteCallback(&KmScene2207::spriteUpdate41F230);
			SetMessageHandler(&Klayman::handleMessage41E210);
			NextState(&KmScene2207::sub442520);
		}
	}
}

void KmScene2207::sub442520() {
	setFileHash(0x0D318140, 0, -1);
	sendMessage(_attachedSprite, 0x480F, 0);
	NextState(&KmScene2207::sub442560);
}

void KmScene2207::sub442560() {
	setFileHash(0x1564A2C0, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	NextState(&KmScene2207::sub4425A0);
	_flagE5 = true;
	_flagF7 = true;
}

void KmScene2207::sub4425A0() {
	setFileHash(0x4464A440, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetSpriteCallback(&Klayman::spriteUpdate41F230);
	SetMessageHandler(&Klayman::handleMessage41D360);
	_flagE5 = true;
	_flagF7 = true;
}

KmScene2242::KmScene2242(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmScene2242::xUpdate() {
	setGlobalVar(0x18288913, _frameIndex);
}

uint32 KmScene2242::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&KmScene2242::sub444D20);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4812:
		if (param.asInteger() == 2) {
			GotoState(&Klayman::stPickUpNeedle);
		} else if (param.asInteger() == 1) {
			GotoState(&Klayman::sub41FFF0);
		} else {
			GotoState(&Klayman::stPickUpGeneric);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41C7B0();
		break;
	case 0x4837:
		sub41CE70();
		break;
	}
	return 0;
}

void KmScene2242::sub444D20() {
	int16 frameIndex = (int16)getGlobalVar(0x18288913);
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	NextState(&Klayman::sub41FA40);
	FinalizeState(&Klayman::stStartWalkingDone);
}

KmHallOfRecords::KmHallOfRecords(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmHallOfRecords::xUpdate() {
	setGlobalVar(0x18288913, _frameIndex);
}

uint32 KmHallOfRecords::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&KmHallOfRecords::sub43B130);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41C7B0();
		break;
	case 0x4837:
		sub41CE70();
		break;
	}
	return 0;
}

void KmHallOfRecords::sub43B130() {
	int16 frameIndex = (int16)getGlobalVar(0x18288913);
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	NextState(&Klayman::sub41FA40);
	FinalizeState(&Klayman::stStartWalkingDone);
}

KmScene2247::KmScene2247(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
	// Empty
}

void KmScene2247::xUpdate() {
	setGlobalVar(0x18288913, _frameIndex);
}

uint32 KmScene2247::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		GotoState(&Klayman::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_x4 = param.asInteger();
			GotoState(&KmScene2247::sub453520);
		} else {
			GotoState(&Klayman::stPeekWall);
		}
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
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
		sub41C7B0();
		break;
	case 0x4837:
		sub41CE70();
		break;
	}
	return 0;
}
  
void KmScene2247::sub453520() {
	int16 frameIndex = (int16)getGlobalVar(0x18288913);
	if (frameIndex < 0 || frameIndex > 13)
		frameIndex = 0;
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, frameIndex, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	NextState(&Klayman::sub41FA40);
	FinalizeState(&Klayman::stStartWalkingDone);
}

} // End of namespace Neverhood
