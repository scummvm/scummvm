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

#include "neverhood/diskplayerscene.h"
#include "neverhood/mouse.h"

namespace Neverhood {

// TODO: Maybe move hash tables into neverhood.dat

static const uint32 kDiskplayerPaletteFileHashes[] = {
	0x03B78240,
	0x34B32B08,
	0x4F2569D4,
	0x07620590,
	0x38422401
};

static const byte kDiskplayerInitArray[] = {
	2, 1, 4, 5, 3, 11, 8, 6, 7, 9, 10, 17, 16, 18, 19, 20, 15, 14, 13, 12
};

static const uint32 kDiskplayerSmackerFileHashes[] = {
	0x010A2810,
	0x020A2810,
	0x040A2810,
	0x080A2810,
	0x100A2810,
	0x200A2810,
	0x400A2810,
	0x800A2810,
	0x000A2811,
	0x010C2810,
	0x020C2810,
	0x040C2810,
	0x080C2810,
	0x100C2810,
	0x200C2810,
	0x400C2810,
	0x800C2810,
	0x000C2811,
	0x000C2812,
	0x02002810,
	0x04002810
};

static const uint32 kDiskplayerSlotFileHashes1[] = { 
	0x81312280,
	0x01312281,
	0x01312282,
	0x01312284,
	0x01312288,
	0x01312290,
	0x013122A0,
	0x013122C0,
	0x01312200,
	0x82312280,
	0x02312281,
	0x02312282,
	0x02312284,
	0x02312288,
	0x02312290,
	0x023122A0,
	0x023122C0,
	0x02312200,
	0x02312380,
	0x04312281
};

static const uint32 kDiskplayerSlotFileHashes2[] = { 
	0x90443A00,
	0x90443A18,
	0x90443A28,
	0x90443A48,
	0x90443A88,
	0x90443B08,
	0x90443808,
	0x90443E08,
	0x90443208,
	0xA0443A00,
	0xA0443A18,
	0xA0443A28,
	0xA0443A48,
	0xA0443A88,
	0xA0443B08,
	0xA0443808,
	0xA0443E08,
	0xA0443208,
	0xA0442A08,
	0xC0443A18
};

static const uint32 kDiskplayerSlotFileHashes3[] = { 
	0x10357320, 
	0x10557320,
	0x10957320,
	0x11157320,
	0x12157320,
	0x14157320,
	0x18157320,
	0x00157320,
	0x30157320,
	0x1035B320,
	0x1055B320,
	0x1095B320,
	0x1115B320,
	0x1215B320,
	0x1415B320,
	0x1815B320,
	0x0015B320,
	0x3015B320,
	0x5015B320,
	0x10543320
};

static const uint32 kDiskplayerSlotFileHashes4[] = { 
	0xDC8020E4,
	0xDC802164,
	0xDC802264,
	0xDC802464,
	0xDC802864,
	0xDC803064,
	0xDC800064,
	0xDC806064,
	0xDC80A064,
	0xDC8020E7,
	0xDC802167,
	0xDC802267,
	0xDC802467,
	0xDC802867,
	0xDC803067,
	0xDC800067,
	0xDC806067,
	0xDC80A067,
	0xDC812067,
	0xDC802161
};

Class494::Class494(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {
	
	createSurface1(0x100B90B4, 1200);
	_x = 211;
	_y = 195;
	setFileHash(0x100B90B4, 0, -1);
	_newHashListIndex = 0;
	_needRefresh = true;
	updatePosition();
	_surface->setVisible(false);
}

uint32 Class494::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void Class494::sub43BE00() {
	setFileHash1();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Sprite::handleMessage);
	_surface->setVisible(false);
}

void Class494::sub43BE20() {
	setFileHash(0x100B90B4, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Class494::handleMessage);
	SetAnimationCallback3(&Class494::sub43BE00);
	_surface->setVisible(true);
}

DiskplayerPlayButton::DiskplayerPlayButton(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene)
	: StaticSprite(vm, 1400), _soundResource1(vm), _soundResource2(vm),
	_diskplayerScene(diskplayerScene), _isPlaying(false) {
	
	_spriteResource.load2(0x24A4A664);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = _spriteResource.getDimensions().width;
	_deltaRect.height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_surface->setVisible(false);
	processDelta();
	_needRefresh = true;
	StaticSprite::update();
	_soundResource1.load(0x44043000);
	_soundResource2.load(0x44045000);
	SetMessageHandler(&DiskplayerPlayButton::handleMessage);
}

uint32 DiskplayerPlayButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_diskplayerScene->getFlag3()) {
			if (_isPlaying) {
				_diskplayerScene->sendMessage(0x2001, 0, this);
				release();
			} else {
				_diskplayerScene->sendMessage(0x2000, 0, this);
				press();
			}
		}
		StaticSprite::update();
		messageResult = 1;
		break;
	}
	return messageResult;
}

void DiskplayerPlayButton::press() {
	if (!_isPlaying) {
		_surface->setVisible(true);
		StaticSprite::update();
		_soundResource1.play();
		_isPlaying = true;
	}
}

void DiskplayerPlayButton::release() {
	if (_isPlaying) {
		_surface->setVisible(false);
		StaticSprite::update();
		_soundResource2.play();
		_isPlaying = false;
	}
}

DiskplayerSlot::DiskplayerSlot(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene, int elementIndex, int value)
	: Entity(vm, 0), _diskplayerScene(diskplayerScene), _soundResource(vm), _elementIndex(elementIndex),
	_value(value), _flag2(false), _flag(false), _countdown(0), _initialCountdown(2),
	_inactiveSlot(NULL), _appearSlot(NULL), _activeSlot(NULL) {

	if (value != 0 && elementIndex < 20) {
		_inactiveSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes1[_elementIndex], 1100));
		_appearSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes2[_elementIndex], 1000));
		_activeSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes3[_elementIndex], 1100));
		_inactiveSlot->getSurface()->setVisible(false);
		_appearSlot->getSurface()->setVisible(false);
		_activeSlot->getSurface()->setVisible(false);
		_soundResource.load(0x46210074);
		// TODO sound panning stuff
	} else if (elementIndex != 20) {
		_activeSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes4[_elementIndex], 1100));
		_activeSlot->getSurface()->setVisible(false);
	}
	SetUpdateHandler(&DiskplayerSlot::update);
}

void DiskplayerSlot::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		if (_flag) {
			if (_inactiveSlot) {
				_inactiveSlot->getSurface()->setVisible(true);
			}
			if (_activeSlot) {
				_activeSlot->getSurface()->setVisible(false);
			}
			_countdown = _initialCountdown / 2;
		} else {
			if (_inactiveSlot) {
				_inactiveSlot->getSurface()->setVisible(false);
			}
			if (_activeSlot) {
				_activeSlot->getSurface()->setVisible(true);
			}
			_countdown = _initialCountdown;
		}
		_flag = !_flag;
	}
}

void DiskplayerSlot::appear() {
	if (_inactiveSlot) {
		_inactiveSlot->getSurface()->setVisible(true);
	}
	if (_appearSlot) {
		_appearSlot->getSurface()->setVisible(true);
	}
	if (_inactiveSlot) {
		_soundResource.play();
	}
}

void DiskplayerSlot::play() {
	if (!_flag2) {
		if (_inactiveSlot) {
			_inactiveSlot->getSurface()->setVisible(false);
		}
		if (_activeSlot) {
			_activeSlot->getSurface()->setVisible(true);
		}
		_flag = true;
		_countdown = 0;
	}
}

void DiskplayerSlot::activate() {
	if (!_flag2) {
		_countdown = _initialCountdown;
	}
}

void DiskplayerSlot::stop() {
	if (!_flag2) {
		if (_inactiveSlot) {
			_inactiveSlot->getSurface()->setVisible(true);
		}
		if (_activeSlot) {
			_activeSlot->getSurface()->setVisible(false);
		}
		_flag = false;
		_countdown = 0;
	}
}

DiskplayerScene::DiskplayerScene(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _which(which), _diskIndex(0), _appearCountdown(0), _tuneInCountdown(0),
	_fullFlag(false), _flag3(false), _inputDisabled(true), _updateStatus(0) { 

	int count = 0;
	
	_surfaceFlag = true;

	_background = addBackground(new DirtyBackground(_vm, 0x8A000044, 0, 0));
	_palette = new Palette(_vm, kDiskplayerPaletteFileHashes[_which]);
	_playButton = new DiskplayerPlayButton(_vm, this);
	addSprite(_playButton);
	_vm->_collisionMan->addSprite(_playButton);
	_class494 = new Class494(_vm);
	addSprite(_class494);

	// DEBUG: Give all disks
	for (int i = 0; i < 19; i++) {
		setSubVar(0x02720344, i, 1);
	}

	for (int i = 0; i < 20; i++) {
		_diskAvailable[i] = 0;
		if (getSubVar(0x02720344, i))
			count++;
	}

	for (int i = 0; i < count; i++) {
		_diskAvailable[kDiskplayerInitArray[i] - 1] = 1;
	}

	for (int i = 0; i < 20; i++) {
		_diskSlots[i] = new DiskplayerSlot(_vm, this, i, _diskAvailable[i]);
		addEntity(_diskSlots[i]);
	}

	_fullFlag = count == 20;
	
	if (_fullFlag && !getGlobalVar(0xC0780812))
		_flag3 = true;

	_flag4 = _flag3;	

	_class650 = new DiskplayerSlot(_vm, this, 20, 0);
	addEntity(_class650);

	_mouseCursor = addSprite(new Mouse435(_vm, 0x000408A8, 20, 620));
	_mouseCursor->getSurface()->setVisible(false);

	_smackerPlayer = new SmackerPlayer(_vm, this, 0x08288103, false, true);
	addEntity(_smackerPlayer);
	addSurface(_smackerPlayer->getSurface());
	_smackerPlayer->setDrawPos(154, 86);
	// TODO _smackerPlayer->gotoFrame(0);

	_palette->usePalette();

	SetMessageHandler(&DiskplayerScene::handleMessage); 
	SetUpdateHandler(&DiskplayerScene::update); 
	_appearCountdown = 6;

}

void DiskplayerScene::update() {
	Scene::update();

	debug("_updateStatus = %d", _updateStatus);

	if (_updateStatus == 1) {
		if (_smackerPlayer->getFrameNumber() == _smackerPlayer->getFrameCount() - 1) {
			if (_diskAvailable[_diskIndex]) {
				playDisk();
			} else {
				playStatic();
			}
		}
	} else if (_updateStatus == 2) {
		if (_smackerPlayer->getFrameNumber() == _smackerPlayer->getFrameCount() - 1) {
			_diskSlots[_diskIndex]->stop();
			_diskIndex++;
			if (_fullFlag) {
				if (_diskIndex == 20) {
					if (_flag3) {
						playDisk();
						_updateStatus = 3;
					} else {
						_diskIndex = 0;
						stop();
					}
				} else {
					playDisk();
				}
			} else {
				if (_diskIndex == 20) {
					_diskIndex = 0;
					stop();
				} else {
					tuneIn();
				}
			}
		}
	} else if (_updateStatus == 3) {
		if (_smackerPlayer->getFrameNumber() == 133) {
			_class494->sub43BE20();
			setGlobalVar(0xC0780812, 1);
		} else if (_smackerPlayer->getFrameNumber() == _smackerPlayer->getFrameCount() - 1) {
			for (int i = 0; i < 20; i++) {
				_diskSlots[i]->setFlag2(false);
				_diskSlots[i]->stop();
			}
			_diskIndex = 0;
			stop();
			_mouseCursor->getSurface()->setVisible(true);
			_flag3 = false;
		}
	}

	if (_appearCountdown != 0 && (--_appearCountdown == 0)) {
		_diskSlots[_diskIndex]->appear();
		if (_flag3) {
			_diskSlots[_diskIndex]->activate();
			_diskSlots[_diskIndex]->setFlag2(true);
		}
		_diskIndex++;
		while (_diskAvailable[_diskIndex] == 0 && _diskIndex < 19)
			_diskIndex++;					
		if (_diskIndex < 20) {
			_appearCountdown = 1;
		} else {
			_diskIndex = 0;
			_inputDisabled = false;
			if (_flag3) {
				_playButton->press();
				_tuneInCountdown = 2;
			} else {
				_mouseCursor->getSurface()->setVisible(true);
				_diskSlots[_diskIndex]->activate();
			}
		}
	}

	if (_tuneInCountdown != 0 && (--_tuneInCountdown == 0)) {
		playDisk();
	}

}

uint32 DiskplayerScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	if (!_inputDisabled) {
		switch (messageNum) {
		case 0x0001:
			// TODO: Debug/Cheat
			if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
				_parentModule->sendMessage(0x1009, 0, this);
			} else if (!_flag3 &&
				param.asPoint().x > 38 && param.asPoint().x < 598 &&
				param.asPoint().y > 400 && param.asPoint().y < 460) {
				
				_diskSlots[_diskIndex]->stop();
				_diskIndex = (param.asPoint().x - 38) / 28;
				_diskSlots[_diskIndex]->activate();
				if (_updateStatus == 2) {
					if (_diskAvailable[_diskIndex]) {
						playDisk();
					} else {
						playStatic();
					}
				}
			}
			break;
		// case 0x000D: TODO: Debug/Cheat
		case 0x2000:
			tuneIn();
			break;			
		case 0x2001:
			stop();
			break;			
		}
	}
	return messageResult;
}

void DiskplayerScene::stop() {
	_smackerPlayer->open(0x08288103, true);
	_palette->usePalette();
	_playButton->release();
	_updateStatus = 0;
	_diskSlots[_diskIndex]->activate();
}

void DiskplayerScene::tuneIn() {
	_smackerPlayer->open(0x900001C1, false);
	_palette->usePalette();
	_playButton->release();
	_updateStatus = 1;
	_diskSlots[_diskIndex]->activate();
}

void DiskplayerScene::playDisk() {
	_smackerPlayer->open(kDiskplayerSmackerFileHashes[_diskIndex], false);
	_palette->usePalette();
	_updateStatus = 2;
	_diskSlots[_diskIndex]->play();
}

void DiskplayerScene::playStatic() {
	_smackerPlayer->open(0x90000101, false);
	_palette->usePalette();
	_playButton->release();
	_updateStatus = 2;
	_diskSlots[_diskIndex]->activate();
}

} // End of namespace Neverhood
