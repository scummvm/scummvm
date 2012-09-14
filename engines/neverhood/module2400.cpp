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

#include "neverhood/module2400.h"

namespace Neverhood {

Module2400::Module2400(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	// TODO Music18hList_add(0x202D1010, 0xB110382D);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, _vm->gameState().which);
	} else {
		createScene(0, 0);
	}

}

Module2400::~Module2400() {
	// TODO Music18hList_deleteGroup(0x202D1010);
}

void Module2400::createScene(int sceneNum, int which) {
	debug("Module2400::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		// TODO Music18hList_stop(0xB110382D, 0, 0);
		_childObject = new Scene2401(_vm, this, which);
		break;
	case 1:
		// TODO Music18hList_play(0xB110382D, 0, 2, 1);
		_childObject = new Scene2402(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module2400::updateScene);
	_childObject->handleUpdate();
}

void Module2400::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(1, 0);
			else
				leaveModule(0);
			break;
		case 1:
			if (_moduleResult == 1)
				createScene(5, 0);
			else if (_moduleResult == 2)
				createScene(7, -1);
			else
				createScene(0, 1);
			break;
		}
	}
}

static const NPoint kScene2401Points[] = {
	{384, 389},
	{406, 389},
	{429, 389},
	{453, 389},
	{477, 389}
};

static const uint32 kScene2401FileHashes1[] = {
	0x02842920,
	0x02882920,
	0x02902920,
	0x02A02920,
	0x02C02920,
	0x02002920,
	0x03802920,
	0x00802920,
	0x06802920,
	0x03842920
};

static const uint32 kScene2401FileHashes2[] = {
	0xD0910020,
	0xD0910038,
	0xD0910008,
	0xD0910068,
	0xD09100A8,
	0
};

static const uint32 kScene2401FileHashes3[] = {
	0xD0910020,
	0xD0910038,
	0xD0910008,
	0xD0910068,
	0xD09100A8,
	0
};

static const NRect kScene2401Rects[] = {
	NRect(369, 331, 394, 389),
	NRect(395, 331, 419, 389),
	NRect(420, 331, 441, 389),
	NRect(442, 331, 464, 389),
	NRect(465, 331, 491, 389)
};

static const uint32 kAsScene2401WaterSpitFileHashes2[] = {
	0x5C044690,
	0x5C644690,
	0x5CA44690,
	0x5D244690,
	0x5E244690
};

static const uint32 kAsScene2401WaterSpitFileHashes1[] = {
	0xF4418408,
	0xF4418808,
	0xF4419008,
	0xF441A008,
	0xCD4F8411
};

AsScene2401WaterSpit::AsScene2401WaterSpit(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {
	
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2401WaterSpit::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
	createSurface(100, 146, 74);
	_x = 240;
	_y = 447;
	setVisible(false);
}

uint32 AsScene2401WaterSpit::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x120A0013)
			_soundResource.play(kAsScene2401WaterSpitFileHashes1[_soundIndex]);
		break;
	case 0x2000:
		_x = 240;
		_y = 447;
		_soundIndex = getSubVar(0x0800547C, param.asInteger());
		_soundResource.play(0x48640244);
		startAnimation(kAsScene2401WaterSpitFileHashes2[param.asInteger()], 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}

AsScene2401FlowingWater::AsScene2401FlowingWater(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _isWaterFlowing(false) {
	
	createSurface1(0x10203116, 100);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2401FlowingWater::handleMessage);
	_x = 88;
	_y = 421;
	setVisible(false);
}

AsScene2401FlowingWater::~AsScene2401FlowingWater() {
	// TODO Sound1ChList_sub_407AF0(0x40F11C09);
}

uint32 AsScene2401FlowingWater::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (_isWaterFlowing && param.asInteger() == 0x02421405) {
			startAnimationByHash(0x10203116, 0x01084280, 0);
		}
		break;
	case 0x2002:
		if (!_isWaterFlowing) {
			// TODO Sound1ChList_addSoundResource(0x40F11C09, 0x980C1420, true);
			// TODO Sound1ChList_playLooping(0x980C1420);
			startAnimation(0x10203116, 0, -1);
			setVisible(true);
			_isWaterFlowing = true;
		}
		break;
	case 0x2003:
		// TODO Sound1ChList_deleteSoundByHash(0x980C1420);
		_isWaterFlowing = false;
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}
	
AsScene2401WaterFlushing::AsScene2401WaterFlushing(NeverhoodEngine *vm, int16 x, int16 y)
	: AnimatedSprite(vm, 1200), _countdown(0), _flushLoopCount(0) {
	
	createSurface1(0xB8596884, 100);
	SetUpdateHandler(&AsScene2401WaterFlushing::update);
	SetMessageHandler(&AsScene2401WaterFlushing::handleMessage);
	_x = x;
	_y = y;
	setVisible(false);
}

void AsScene2401WaterFlushing::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		setDoDeltaX(_vm->_rnd->getRandomNumber(1));
		startAnimation(0xB8596884, 0, -1);
		setVisible(true);
	}
	AnimatedSprite::update();
}

uint32 AsScene2401WaterFlushing::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (_flushLoopCount > 0 && param.asInteger() == 0x02421405) {
			startAnimationByHash(0xB8596884, 0x01084280, 0);
			_flushLoopCount--;
		}
		break;
	case 0x2002:
		if (param.asInteger() > 0) {
			_flushLoopCount = param.asInteger() - 1;
			_countdown = _vm->_rnd->getRandomNumber(3) + 1;
		}
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}

AsScene2401Door::AsScene2401Door(NeverhoodEngine *vm, bool isOpen)
	: AnimatedSprite(vm, 1100), _countdown(0), _isOpen(isOpen), _soundResource(vm) {
	
	createSurface1(0x44687810, 100);
	_x = 320;
	_y = 240;
	_newStickFrameIndex = -2;
	SetUpdateHandler(&AsScene2401Door::update);
	SetMessageHandler(&AsScene2401Door::handleMessage);
	if (_isOpen) {
		stopAnimation();
		setVisible(false);
		_countdown = 48;
	} else {
		startAnimation(0x44687810, 0, -1);
		_newStickFrameIndex = 0;
	}
}

void AsScene2401Door::update() {
	if (_isOpen && _countdown != 0 && (--_countdown) == 0) {
		_isOpen = false;
		setVisible(true);
		startAnimation(0x44687810, -1, -1);
		_newStickFrameIndex = 0;
		_playBackwards = true;
		_soundResource.play(calcHash("fxDoorClose38"));
	}
	AnimatedSprite::update();
}

uint32 AsScene2401Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		if (_isOpen)
			_countdown = 168;
		messageResult = _isOpen ? 1 : 0;
		break;
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		if (!_isOpen) {
			_countdown = 168;
			_isOpen = true;
			setVisible(true);
			startAnimation(0x44687810, 0, -1);
			_soundResource.play(calcHash("fxDoorOpen38"));
			NextState(&AsScene2401Door::stDoorOpenFinished);
		}
		break;
	}
	return messageResult;
}

void AsScene2401Door::stDoorOpenFinished() {
	stopAnimation();
	setVisible(false);
}

Scene2401::Scene2401(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _countdown1(0), _countdown2(0), _flag(false),
	_soundToggle(false), _asWaterSpitIndex(0), _soundResource1(vm), _soundResource2(vm) {

	_vm->gameModule()->initScene2401Vars();

	setGlobalVar(0x4E0BE910, 1); //DEBUG! Enables water
	setGlobalVar(0x45080C38, 1); //DEBUG! Gives the test tube

	_surfaceFlag = true;
	SetMessageHandler(&Scene2401::handleMessage);
	SetUpdateHandler(&Scene2401::update);

	setRectList(0x004B3140);
	setBackground(0x8C030206);
	setPalette(0x8C030206);
	addEntity(_palette);
	_palette->addBasePalette(0x8C030206, 0, 256, 0);
	_palette->addPalette(0x91D3A391, 0, 65, 0);
	insertMouse433(0x302028C8);

	_sprite1 = insertStaticSprite(0x2E068A23, 200);
	insertStaticSprite(0x401410A6, 200);
	_asFlowingWater = insertSprite<AsScene2401FlowingWater>();
	insertStaticSprite(0x90C0A4B4, 200);
	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x0092916A, 100, 0);
	_ssFloorButton = insertSprite<Class427>(this, 0x28001120, 0x00911068, 100, 0);

	for (uint i = 0; i < 5; i++)
		_asWaterFlushing[i] = insertSprite<AsScene2401WaterFlushing>(kScene2401Points[i].x, kScene2401Points[i].y);

	for (uint i = 0; i < 10; i++) {
		_ssWaterPipes[i] = insertStaticSprite(kScene2401FileHashes1[i], 300);
		_ssWaterPipes[i]->setVisible(false);
	}
	
	_asWaterSpit[0] = insertSprite<AsScene2401WaterSpit>();
	_asWaterSpit[1] = insertSprite<AsScene2401WaterSpit>();

	if (which < 0) {
		insertKlayman<KmScene2401>(200, 447);
		setMessageList(0x004B2F70);
		_asDoor = insertSprite<AsScene2401Door>(false);
	} else if (which == 1) {
		insertKlayman<KmScene2401>(280, 413);
		setMessageList(0x004B2F80);
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
		_palette->addPalette(0xB103B604, 0, 65, 0);
		_asDoor = insertSprite<AsScene2401Door>(true);
	} else {
		insertKlayman<KmScene2401>(-20, 447);
		setMessageList(0x004B2F78);
		_asDoor = insertSprite<AsScene2401Door>(false);
	}

}

void Scene2401::update() {

	if (_countdown1 != 0 && (--_countdown1) == 0) {
		if (_pipeStatus > 0 && _pipeStatus <= 10)
			_ssWaterPipes[_pipeStatus - 1]->setVisible(false);
		if (_pipeStatus >= 10) {
			bool puzzleSolved = true, waterInside = false;
			for (uint i = 0; i < 5; i++) {
				if (getSubVar(0x0800547C, i) != getSubVar(0x90405038, i))
					puzzleSolved = false;
				if (getSubVar(0x0800547C, i) != 0)
					waterInside = true;
			}
			if (puzzleSolved) {
				setGlobalVar(0x0045D021, 1);
				setGlobalVar(0x86615030, 1);
				sendMessage(_asDoor, 0x4808, 0);
			} else if (waterInside) {
				playPipeSound(0xD0431020);
				for (uint i = 0; i < 5; i++) {
					sendMessage(_asWaterFlushing[i], 0x2002, getSubVar(0x0800547C, i));
					setSubVar(0x0800547C, i, 0);
				}
			}
		} else if (_pipeStatus >= 5) {
			_ssWaterPipes[_pipeStatus]->setVisible(true);
			_countdown1 = 8;
			playPipeSound(kScene2401FileHashes3[getSubVar(0x0800547C, _pipeStatus - 5)]);
		} else {
			_ssWaterPipes[_pipeStatus]->setVisible(true);
			_countdown1 = _pipeStatus == 4 ? 16 : 8;
			playPipeSound(kScene2401FileHashes3[getSubVar(0x90405038, _pipeStatus)]);
		}
		_pipeStatus++;
	}

	if (_countdown2 != 0 && (--_countdown2) == 0) {
		sendMessage(_asFlowingWater, 0x2003, 0);
	}
	
	Scene::update();

}

uint32 Scene2401::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	// case 0x0001: TODO DEBUG
	// case 0x000D: TODO DEBUG
	case 0x100D:
		if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		else if (param.asInteger() == 0x02144CB1)
			sendEntityMessage(_klayman, 0x1014, _ssFloorButton);
		else if (param.asInteger() == 0x11C40840) {
			if (getGlobalVar(0x0045D021) && sendMessage(_asDoor, 0x2004, 0)) {
				setMessageList(0x004B3090);
			} else {
				setMessageList(0x004B30B0);
			}
		} else if (param.asInteger() == 0x412722C0) {
			if (_countdown2 > 0 && getGlobalVar(0x45080C38)) {
				_countdown2 = 144;
				setMessageList(0x004B3020);
			} else {
				setMessageList(0x004B3050);
			}
		} else if (param.asInteger() == 0x21142050) {
			if (_flag && _countdown1 == 0 && getGlobalVar(0x86615030) == 0) {
				setMessageList(0x004B2FA8);
			} else {
				setMessageList(0x004B2FC8);
			}
		} else if (param.asInteger() == 0x87441031) {
			setSurfacePriority(_sprite1->getSurface(), 1100);
		} else if (param.asInteger() == 0x80C40322) {
			setSurfacePriority(_sprite1->getSurface(), 200);
			messageList402220();
			_flag = true;
		} else if (param.asInteger() == 0x09C4B40A && _countdown2 > 12) {
			_countdown2 = 12;
		}
		break;
	case 0x2000:
		messageResult = 0;
		for (uint32 i = 0; i < 5; i++)
			if (kScene2401Rects[i].contains(_mouseClickPos.x, _mouseClickPos.y)) {
				messageResult = i;
				break;
			}
		break;
	case 0x2001:
		sendMessage(_asWaterSpit[_asWaterSpitIndex], 0x2000, param.asInteger());
		_asWaterSpitIndex = (_asWaterSpitIndex + 1) & 1;
		incSubVar(0x0800547C, param.asInteger(), 1);
		if (getSubVar(0x0800547C, param.asInteger()) >= 5)
			setSubVar(0x0800547C, param.asInteger(), 4);
		break;
	case 0x480B:
		if (sender == _ssButton) {
			_pipeStatus = 0;
			_countdown1 = 8;
		} else if (sender == _ssFloorButton && getGlobalVar(0x4E0BE910)) {
			_countdown2 = 144;
			sendMessage(_asFlowingWater, 0x2002, 0);
			_soundResource1.play(0xE1130324);
		}
		break;
	case 0x482A:
		_palette->addBasePalette(0xB103B604, 0, 65, 0);
		_palette->startFadeToPalette(12);
		break;
	case 0x482B:
		_palette->addBasePalette(0x91D3A391, 0, 65, 0);
		_palette->startFadeToPalette(12);
		break;
	}
	return messageResult;
}

void Scene2401::playPipeSound(uint32 fileHash) {
	if (_soundToggle)
		_soundResource1.play(fileHash);
	else
		_soundResource2.play(fileHash);
	_soundToggle = !_soundToggle;
}

static const uint32 kScene2402FileHashes[] = {
	0xD0910020,
	0xD0910038,
	0xD0910008,
	0xD0910068,
	0xD09100A8
};

AsScene2402Door::AsScene2402Door(NeverhoodEngine *vm, Scene *parentScene, bool isOpen)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _isOpen(isOpen), _soundResource(vm) {

	SetUpdateHandler(&AsScene2402Door::update);
	SetMessageHandler(&AsScene2402Door::handleMessage);
	createSurface1(0x80495831, 100);
	_x = 320;
	_y = 240;
	_newStickFrameIndex = -2;
	if (_isOpen) {
		startAnimation(0x80495831, -1, -1);
		_countdown = 48;
	} else {
		stopAnimation();
		setVisible(false);
	}	
}

void AsScene2402Door::update() {
	if (_isOpen && _countdown != 0 && (--_countdown) == 0) {
		_isOpen = false;
		setVisible(true);
		startAnimation(0x80495831, -1, -1);
		_playBackwards = true;
		_soundResource.play(calcHash("fxDoorClose38"));
		NextState(&AsScene2402Door::stDoorClosingFinished);
	}
	AnimatedSprite::update();
}

uint32 AsScene2402Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (_isOpen)
			_countdown = 144;
		messageResult = _isOpen ? 1 : 0;
		break;
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		_countdown = 144;
		_isOpen = true;
		setVisible(true);
		startAnimation(0x80495831, 0, -1);
		_newStickFrameIndex = -2;
		_soundResource.play(calcHash("fxDoorOpen38"));
		break;
	}
	return messageResult;
}

void AsScene2402Door::stDoorClosingFinished() {
	sendMessage(_parentScene, 0x2001, 0);
	setVisible(false);
}

AsScene2402TV::AsScene2402TV(NeverhoodEngine *vm, Klayman *klayman)
	: AnimatedSprite(vm, 1100), _klayman(klayman), _countdown1(0), _countdown2(0),
	_soundResource(vm) {

	createSurface(100, 640, 480); // TODO Use correct size	from the two hashes
	SetMessageHandler(&Sprite::handleMessage);
	_x = 260;
	_y = 210;
	setDoDeltaX(1);

	if (!getGlobalVar(0x92603A79)) {
		_soundResource.load(0x58208810);
		_countdown1 = 48;
		startAnimation(0x4919397A, 0, -1);
		_newStickFrameIndex = 0;
		SetUpdateHandler(&AsScene2402TV::upWait);
	} else {
		int16 frameIndex;
		if (_klayman->getX() > 320)
			_currFrameIndex = 29;
		frameIndex = CLIP<int16>((_klayman->getX() - _x + 150) / 10, 0, 29);
		startAnimation(0x050A0103, frameIndex, -1);
		_newStickFrameIndex = frameIndex;
		_countdown1 = 0;
		SetUpdateHandler(&AsScene2402TV::upFocusKlayman);
	}

}

AsScene2402TV::~AsScene2402TV() {
	// TODO Sound1ChList_sub_407AF0(0x01520123);
}

void AsScene2402TV::upWait() {
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		startAnimation(0x4919397A, 0, -1);
		SetMessageHandler(&AsScene2402TV::hmJoke);
		NextState(&AsScene2402TV::stJokeFinished);
	}
	AnimatedSprite::update();
}

void AsScene2402TV::upFocusKlayman() {
	int16 frameIndex = CLIP<int16>((_klayman->getX() - _x + 150) / 10, 0, 29);
	if (frameIndex != _currFrameIndex) {
		if (frameIndex > _currFrameIndex) {
			_currFrameIndex++;
		} else if (frameIndex < _currFrameIndex) {
			_currFrameIndex--;
		}
		startAnimation(0x050A0103, _currFrameIndex, -1);
		_newStickFrameIndex = _currFrameIndex;
		if (_countdown2 == 0) {
			// TODO Sound1ChList_addSoundResource(0x01520123, 0xC42D4528, true);
			// TODO Sound1ChList_playLooping(0xC42D4528); 
		}
		_countdown2 = 5;
	} else if (_countdown2 != 0 && (--_countdown2 == 0)) {
		// TODO Sound1ChList_deleteSoundByHash(0xC42D4528);
	}
	AnimatedSprite::update();
}

void AsScene2402TV::stJokeFinished() {
	setGlobalVar(0x92603A79, 1);
	startAnimation(0x050A0103, 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AsScene2402TV::upFocusKlayman);
}

uint32 AsScene2402TV::hmJoke(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x431EA0B0) {
			_soundResource.play();
		}
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

Scene2402::Scene2402(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _countdown(0), _soundToggle(false),
	_soundResource1(vm), _soundResource2(vm) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene2402::handleMessage);
	SetUpdateHandler(&Scene2402::update);

	setRectList(0x004AF900);
	setBackground(0x81660220);
	setPalette(0x81660220);
	insertMouse433(0x6022481E);
	_asTape = insertSprite<AsScene1201Tape>(this, 9, 1100, 286, 409, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape);
	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x15288120, 100, 0);
	
	if (which < 0) {
		insertKlayman<KmScene2402>(198, 404);
		setMessageList(0x004AF7C8);
	} else if (which == 1) {
		insertKlayman<KmScene2402>(660, 404);
		setMessageList(0x004AF7D8);
	} else if (which == 2) {
		insertKlayman<KmScene2402>(409, 404);
		_klayman->setDoDeltaX(getGlobalVar(0xC0418A02) ? 1 : 0);
		setMessageList(0x004AF888);
	} else {
		insertKlayman<KmScene2402>(0, 404);
		setMessageList(0x004AF7D0);
	}

	_sprite1 = insertStaticSprite(0x081A60A8, 1100);
	_ssDoorFrame = (StaticSprite*)insertStaticSprite(0x406C0AE0, 1100);
	_klayman->setClipRect(_ssDoorFrame->getDrawRect().x, 0, 639, _sprite1->getDrawRect().y2());
	_asDoor = insertSprite<AsScene2402Door>(this, which == 1/*CHECKME or != ?*/);
	insertSprite<AsScene2402TV>(_klayman);
	insertStaticSprite(0x3A01A020, 200);

}

Scene2402::~Scene2402() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX() ? 1 : 0);
}

void Scene2402::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		if (_pipeStatus >= 10) {
			sendMessage(_asDoor, 0x4808, 0);
			_ssDoorFrame->load(0x00B415E0, true, true);
			_ssDoorFrame->update();
		} else if (_pipeStatus >= 5) {
			_countdown = 8;
			playPipeSound(kScene2402FileHashes[getSubVar(0x0800547C, _pipeStatus - 5)]);
		} else {
			_countdown = _pipeStatus == 4 ? 16 : 8;
			playPipeSound(kScene2402FileHashes[getSubVar(0x90405038, _pipeStatus)]);
		}
		_pipeStatus++;
	}
	Scene::update();
}

uint32 Scene2402::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x01C66840) {
			if (sendMessage(_asDoor, 0x2000, 0))
				setMessageList(0x004AF800);
			else
				setMessageList(0x004AF818);
		}
		break;
	case 0x2001:
		_ssDoorFrame->load(0x406C0AE0, true, true);
		_ssDoorFrame->update();
		break;
	case 0x480B:
		if (sender == _ssButton) {
			_pipeStatus = 0;
			_countdown = 8;
		}
		break;
	case 0x4826:
		if (sender == _asTape) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004AF890);
		}
		break;
	}
	return messageResult;
}
		
void Scene2402::playPipeSound(uint32 fileHash) {
	if (_soundToggle)
		_soundResource1.play(fileHash);
	else
		_soundResource2.play(fileHash);
	_soundToggle = !_soundToggle;
}

} // End of namespace Neverhood
