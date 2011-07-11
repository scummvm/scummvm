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
	{1, &Klayman::sub41FD30},
	{1, &Klayman::sub41FDA0},
	{1, &Klayman::sub41FDF0},
	{1, &Klayman::sub41FE60},
	{1, &Klayman::sub41FEB0}
}; 

static const KlaymanTableItem klaymanTable2[] = {
	{1, &Klayman::sub41FD30},
	{1, &Klayman::sub41FDA0},
	{1, &Klayman::sub41FE60},
	{1, &Klayman::sub41FEB0}
}; 

#if 0
static const KlaymanTableItem klaymanTable3[] = {
	{1, &Klayman::sub421430},
	{1, &Klayman::sub421480}
}; 
#endif

// Klayman

Klayman::Klayman(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y, int surfacePriority, int objectPriority)
	: AnimatedSprite(vm, objectPriority), _soundResource1(vm), _soundResource2(vm),
	_counterMax(0), _counter(0), _flagE4(false), _counter3Max(0), _flagF8(false), _counter1(0),
	_counter2(0), /*_field118(0), */_status2(0), _flagE5(true), _attachedSprite(NULL), _flagE1(false),
	_status3(1), _parentScene(parentScene), _flagE2(false), _flagE3(false), _flagF6(false), _flagF7(false),
	_flagFA(false), _statusE0(0), _field114(0), _resourceHandle(-1), _soundFlag(false) {
	
	// TODO
	createSurface(surfacePriority, 320, 200);
	_x = x;
	_y = y;
	_x4 = x;
	_y4 = y;
	_flags = 2;
	setKlaymanTable1();
	sub41FC80();
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
		setCallback2(AnimationCallback(&Klayman::sub41FC80));
		break;
	case 0x4818:
		// TODO sub41C930(_rectResource.getRectangle1(param.asInteger()).x, false);
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

void Klayman::sub41FD30() {
	sub41D320(0x5B20C814, AnimationCallback(&Klayman::sub41FD40));
}

void Klayman::sub41FD40() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5B20C814, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB10);
	SetSpriteCallback(NULL);
	SetAnimationCallback3(&Klayman::sub41FCF0);
	setCallback1(AnimationCallback(&Klayman::sub41FD90));
}

uint32 Klayman::handleMessage41EB10(int messageNum, const MessageParam &param, Entity *sender) {
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

void Klayman::sub41FD90() {
	// TODO _soundResource1.stop(0);
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
	SetAnimationCallback3(&Klayman::sub41FCF0);
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
	SetAnimationCallback3(&Klayman::sub41FCF0);
	setCallback1(AnimationCallback(&Klayman::sub41FE50));
}

void Klayman::sub41FE50() {
	// TODO _soundResource1.stop(0);
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

void Klayman::sub41FE60() {
	sub41D320(0x40A0C034, AnimationCallback(&Klayman::sub41FE70));
}

void Klayman::sub41FE70() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x40A0C034, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EF80);
	SetSpriteCallback(NULL);
	SetAnimationCallback3(&Klayman::sub41FCF0);
}

uint32 Klayman::handleMessage41EF80(int messageNum, const MessageParam &param, Entity *sender) {
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
	SetAnimationCallback3(&Klayman::sub41FCF0);
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

/////////////////////////////////////////////////////////////////

void Klayman::sub41D320(uint32 fileHash, AnimationCb callback) {
	_resourceHandle = _vm->_res->useResource(fileHash);
	if (_resourceHandle != -1) {
		// TODO _vm->_res->moveToFront(_resourceHandle);
		SetAnimationCallback3(callback);
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

bool Klayman::sub41CF10(AnimationCb callback) {
	if (_status2 == 2) {
		_status2 = 1;
		_flagE5 = false; 
		setFileHash(0x9A7020B8, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41F140);
		SetSpriteCallback(NULL);
		SetAnimationCallback3(callback);
		return true;
	}
	return false;
}

 void Klayman::sub41C7B0() {
	if (_callback1Cb) {
		AnimationCb cb = _callback1Cb;
		_callback1Cb = NULL;
		(this->*cb)();
	}
	if (_callback3Cb) {
		AnimationCb cb = _callback3Cb;
		_callback3Cb = NULL;
		(this->*cb)();
#if 0 // TODO		
	} else if (_callbackList) {
		removeCallbackList();
#endif		
	} else {
		_parentScene->sendMessage(0x1006, 0, this);
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

void Klayman::sub41FC80() {
	if (!sub41CF10(AnimationCallback(&Klayman::sub41FC80))) {
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
			sub41FF00();
		}
	}
}

uint32 Klayman::handleMessage41D360(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41D360(%04X)", messageNum);
	Sprite::handleMessage(messageNum, param, sender);
	uint32 messageResult = xHandleMessage(messageNum, param);
	switch (messageNum) {
	case 0x1008:
		messageResult = _flagE5;
		break;
	case 0x1014:
		_attachedSprite = param.asEntity();
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
			// TODO _rectResource.getRectangle2(param.asInteger(), &_field118, &_field114,);
		} else {
			// TODO _field114 = 0;
		}
		break;
	}
	return messageResult;
}

void Klayman::sub41FF00() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5900C41E, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
	SetAnimationCallback3(&Klayman::sub41FCF0);
}

uint32 Klayman::handleMessage41D480(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41D480(%04X)", messageNum);
	uint32 messageResult = handleMessage41D360(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		sub41C7B0();
		break;
	}
	return messageResult;
}

void Klayman::sub41FCF0() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x5420E254, 0, -1);
	SetUpdateHandler(&Klayman::update41D0F0);
	SetMessageHandler(&Klayman::handleMessage41D360);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41F140(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41F140(%04X)", messageNum);
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
	debug("Klayman::sub41C930(%d, %d)", x, flag);
	int16 xdiff = ABS(x - _x);
	if (x == _x) {
		_x4 = x;
		if (!_flagE1 && !_flagE2 && !_flagE3) {
			setCallback2(NULL);
			sub41C7B0();
		}
	} else if (xdiff <= 36 && !_flagE1 && !_flagE2 && !_flagE3) {
		_x4 = x;
		setCallback2(NULL);
		sub41C7B0();
	} else if (xdiff <= 42 && _status3 != 3) {
		if (_flagE2 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_x4 - _x) > xdiff) {
			_x4 = x;
		} else {
			_x4 = x;
			setCallback2(AnimationCallback(&Klayman::sub41FB40));
		}
	} else if (_flagE1 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_x4 = x;
	} else if (flag) {
		_x4 = x;
		error("// TODO AnimatedSprite_setCallback2(AnimationCallback(&Klayman::sub421550));");
		// TODO AnimatedSprite_setCallback2(AnimationCallback(&Klayman::sub421550));
	} else {
		_x4 = x;
		setCallback2(AnimationCallback(&Klayman::sub41F950));
	}
}

void Klayman::sub4211F0() {
	_status2 = 1;
	_flagE5 = false;
	setFileHash(0x527AC970, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::sub4211B0() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x5A38C110, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41E920);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41E920(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41E920(%04X)", messageNum);
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

bool Klayman::sub41CEB0(AnimationCb callback3) {
	if (_status2 == 1) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x5C7080D4, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41F140);
		SetSpriteCallback(&Klayman::spriteUpdate41F250);
		SetAnimationCallback3(callback3);
		return true;
	} else {
		_x = _x4;
		return false;
	}
}

void Klayman::spriteUpdate41F250() {
	debug("Klayman::spriteUpdate41F250()");

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
	debug("Klayman::spriteUpdate41F5F0()");
	
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
			// TODO KlaymanSprite_sub_41CF70
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

void Klayman::sub41FB40() {
	_status2 = 1;
	_flagE2 = true;
	_flagE5 = true;
	setDoDeltaX(_x4 < _x);
	setFileHash(0x5C48C506, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41DD80);
	SetSpriteCallback(&Klayman::spriteUpdate41F5F0);
	setCallback1(AnimationCallback(&Klayman::sub41FBB0));	
}

void Klayman::sub41FBB0() {
	_flagE2 = false;
}

uint32 Klayman::handleMessage41DD80(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41DD80(%04X)", messageNum);
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
	debug("Klayman::sub41CD70(%d)", x);
	if (x > _x) {
		if (ABS(x - _x) <= 105) {
			sub41CAC0(x);
		} else {
			sub41C930(x, false);
		}
	} else if (x == _x) {
		_x4 = x;
		setCallback2(NULL);
		sub41C7B0();
	} else {
		sub41C930(x, false);
	}
}

void Klayman::sub41F950() {
	if (!sub41CF10(AnimationCallback(&Klayman::sub41F950))) {
		_status2 = 0;
		_flagE1 = true;
		_flagE5 = true;
		setDoDeltaX(_x4 < _x);
		setFileHash(0x242C0198, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41EC70);
		SetSpriteCallback(&Klayman::spriteUpdate41F320);
		setCallback1(AnimationCallback(&Klayman::sub41FB30));	
		SetAnimationCallback3(&Klayman::sub41F9E0);
	}
}

void Klayman::sub41FB30() {
	_flagE1 = false;
}

uint32 Klayman::handleMessage41EC70(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41EC70(%04X)", messageNum);
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

void Klayman::sub41F9E0() {
	_status2 = 0;
	_flagE1 = true;
	_flagE5 = true;
	setFileHash(0x1A249001, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EB70);
	SetSpriteCallback(&Klayman::spriteUpdate41F300);
	setCallback1(AnimationCallback(&Klayman::sub41FB30));	
	SetAnimationCallback3(&Klayman::sub41FA40);
}

void Klayman::spriteUpdate41F300() {
	SetSpriteCallback(&Klayman::spriteUpdate41F320);
	_deltaX = 0;
}

uint32 Klayman::handleMessage41EB70(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41EB70(%04X)", messageNum);
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
		error("// TODO sub420F20();");
		// TODO sub420F20();
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
		setCallback1(AnimationCallback(&Klayman::sub41FBB0));	
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
		sendMessage(0x1019, 0, this);
	} else if (_status3 != 2 && _status3 != 3 && xdiff <= 42 && _frameIndex >= 5 && _frameIndex <= 11) {
		sendMessage(0x1019, 0, this);
	} else if (_status3 != 2 && _status3 != 3 && xdiff <= 10 && (_frameIndex >= 12 || _frameIndex <= 4)) {
		sendMessage(0x1019, 0, this);
	} else if (_status3 == 3 && xdiff < 30) {
		sendMessage(0x1019, 0, this);
	} else if (_status3 == 3 && xdiff < 150 && _frameIndex >= 6) {
		sendMessage(0x1019, 0, this);
	} else {
		HitRect *hitRectPrev = _vm->_collisionMan->findHitRectAtPos(_x, _y);
		_x += xdelta;
		if (_field114) {
			debug("_field114");
			// TODO KlaymanSprite_sub_41CF70
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
			_attachedSprite->sendMessage(0x482A, 0, this);
		} else if (param.asInteger() == 0x88001184) {
			_attachedSprite->sendMessage(0x482B, 0, this);
		}
		break;
	}
	return messageResult;
}

void Klayman::sub41FF80() {
	setDoDeltaX(((Sprite*)_attachedSprite)->getX() < _x);
	if (!sub41CEB0(AnimationCallback(&Klayman::sub41FF80))) {
		_status2 = 1;
		_flagE5 = false;
		setFileHash(0x1C28C178, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41D4C0);
		SetSpriteCallback(NULL);
	}
}

uint32 Klayman::handleMessage41D4C0(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41D4C0(%04X)", messageNum);
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xC1380080) {
			if (_attachedSprite) {
				_attachedSprite->sendMessage(0x4806, 0, this);
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

void Klayman::sub420120() {
	if (!sub41CEB0(AnimationCallback(&Klayman::sub420120))) {
		_status2 = 2;
		_flagE5 = true;
		setFileHash(0x1C02B03D, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41DAD0);
		SetSpriteCallback(NULL);
	}
}

uint32 Klayman::handleMessage41DAD0(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41DAD0(%04X)", messageNum);
	uint32 messageResult = handleMessage41D480(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0D01B294) {
			if (_attachedSprite) {
				_attachedSprite->sendMessage(0x480B, 0, this);
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

void Klayman::sub420170() {
	if (!sub41CEB0(AnimationCallback(&Klayman::sub420170))) {
		_status2 = 2;
		_flagE5 = true;
		setFileHash(0x1C16B033, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41DAD0);
		SetSpriteCallback(NULL);
	}
}

void Klayman::sub4200D0() {
	if (!sub41CF10(AnimationCallback(&Klayman::sub4200D0))) {
		_status2 = 1;
		_flagE5 = true;
		setFileHash(0x1CD89029, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&Klayman::handleMessage41DAD0);
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
		setCallback2(NULL);
		sub41C7B0();
	}
}

void Klayman::sub41CC40(int16 x1, int16 x2) {
	if (_x > x1) {
		if (_x == x1 + x2) {
			_x4 = x1 + x2;
			setCallback2(NULL);
			sub41C7B0();
		} else if (_x < x1 + x2) {
			sub41CAC0(x1 + x2);
		} else {
			sub41C930(x1 + x2, false);
		}
	} else {
		if (_x == x1 - x2) {
			_x4 = x1 - x2;
			setCallback2(NULL);
			sub41C7B0();
		} else if (_x > x1 - x2) {
			sub41CAC0(x1 - x2);
		} else {
			sub41C930(x1 - x2, false);
		}
	}
}

void Klayman::sub41CAC0(int16 x) {
	debug("Klayman::sub41CAC0(%d)", x);
	int16 xdiff = ABS(x - _x);
	if (x == _x) {
		_x4 = x;
		if (!_flagE1 && !_flagE2 && !_flagE3) {
			setCallback2(NULL);
			sub41C7B0();
		}
	} else if (xdiff <= 36 && !_flagE1 && !_flagE2 && !_flagE3) {
		_x4 = x;
		setCallback2(NULL);
		sub41C7B0();
	} else if (xdiff <= 42 && _status3 != 3) {
		if (_flagE2 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0)) && ABS(_x4 - _x) > xdiff) {
			_x4 = x;
		} else {
			_x4 = x;
			setCallback2(AnimationCallback(&Klayman::sub41FB40));
		}
	} else if (_flagE3 && ((!_doDeltaX && x - _x > 0) || (_doDeltaX && x - _x < 0))) {
		_x4 = x;
	} else {
		_x4 = x;
		setCallback2(AnimationCallback(&Klayman::sub41FBC0));
	}
}

void Klayman::sub41FBC0() {
	_status2 = 2;
	_flagE3 = true;
	_flagE5 = true;
	setDoDeltaX(_x4 >= _x);
	setFileHash(0x08B28116, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41DF10);
	SetSpriteCallback(&Klayman::spriteUpdate41F780);
	setCallback1(AnimationCallback(&Klayman::sub41FC30));	
}

void Klayman::sub41FC30() {
	_flagE3 = false;
}

void Klayman::spriteUpdate41F780() {
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
			debug("// TODO KlaymanSprite_sub_41CF70();");
			// TODO KlaymanSprite_sub_41CF70();
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

uint32 Klayman::handleMessage41DF10(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41DF10(%04X)", messageNum);
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

void Klayman::sub420870() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0xD820A114, 0, 10);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::sub4208B0() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0xD820A114, 30, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

void Klayman::sub4208F0() {
	_status2 = 0;
	_flagE5 = true;
	setFileHash(0x9B250AD2, 0, 7);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EEF0);
	SetSpriteCallback(NULL);
}

uint32 Klayman::handleMessage41EEF0(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Klayman::handleMessage41EEF0(%04X)", messageNum);
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

void Klayman::sub420930() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0x98F88391, 4, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41EEF0);
	SetSpriteCallback(NULL);
}

void Klayman::sub420830() {
	_status2 = 1;
	_flagE5 = true;
	setFileHash(0xD820A114, 0, -1);
	SetUpdateHandler(&Klayman::update);
	SetMessageHandler(&Klayman::handleMessage41D480);
	SetSpriteCallback(NULL);
}

// KmScene1001

KmScene1001::KmScene1001(NeverhoodEngine *vm, Entity *parentScene, int16 x, int16 y)
	: Klayman(vm, parentScene, x, y, 1000, 1000) {
}

uint32 KmScene1001::xHandleMessage(int messageNum, const MessageParam &param) {
	debug("KmScene1001::xHandleMessage() messageNum = %04X", messageNum);
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		sub41C930(param.asPoint().x, false);
		break;
	case 0x4004:
		setCallback2(AnimationCallback(&Klayman::sub41FC80));
		break;		
	case 0x4804:
		if (param.asInteger() == 2) {
			setCallback2(AnimationCallback(&Klayman::sub4211B0));
		}
		break;
	case 0x480D:
		setCallback2(AnimationCallback(&KmScene1001::sub44FA50));
		break;
	case 0x4812:
		setCallback2(AnimationCallback(&Klayman::sub41FF80));
		break;
	case 0x4816:
		if (param.asInteger() == 1) {
			setCallback2(AnimationCallback(&Klayman::sub420120));
		} else if (param.asInteger() == 2) {
			setCallback2(AnimationCallback(&Klayman::sub420170));
		}else {
			setCallback2(AnimationCallback(&Klayman::sub4200D0));
		} 
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		sub41C7B0();
		break;		

	case 0x481B:
		// TODO: It's not really a point but an x1/x2 pair
		if (param.asPoint().x != 0) {
			sub41CC40(param.asPoint().x, param.asPoint().y);
		} else {
			error("// TODO sub41CCE0(param.asPoint().y);");
			// TODO sub41CCE0(param.asPoint().y);
		}
		break;

	case 0x481F:
		if (param.asInteger() == 0) {
			setCallback2(AnimationCallback(&Klayman::sub420870));
		} else if (param.asInteger() == 1) {
			setCallback2(AnimationCallback(&Klayman::sub4208B0));
		} else if (param.asInteger() == 3) {
			setCallback2(AnimationCallback(&Klayman::sub4208F0));
		} else if (param.asInteger() == 4) {
			setCallback2(AnimationCallback(&Klayman::sub420930));
		} else {
			setCallback2(AnimationCallback(&Klayman::sub420830));
		}
		break;

	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger());
		sub41C7B0();
		break;

	case 0x4836:
		if (param.asInteger() == 1) {
			_parentScene->sendMessage(0x2002, 0, this);
			setCallback2(AnimationCallback(&Klayman::sub4211F0));
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
	if (!sub41CEB0(AnimationCallback(&KmScene1001::sub44FA50))) {
		_status2 = 2;
		_flagE5 = false;
		setFileHash(0x00648953, 0, -1);
		SetUpdateHandler(&Klayman::update);
		SetMessageHandler(&KmScene1001::handleMessage44FA00);
		SetSpriteCallback(&AnimatedSprite::updateDeltaXY);
	}
}

uint32 KmScene1001::handleMessage44FA00(int messageNum, const MessageParam &param, Entity *sender) {
	debug("KmScene1001::handleMessage44FA00(%04X)", messageNum);
	uint32 messageResult = Klayman::handleMessage41E210(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			_attachedSprite->sendMessage(0x480F, 0, this);
		}
		break;
	}
	return messageResult;
}


} // End of namespace Neverhood
