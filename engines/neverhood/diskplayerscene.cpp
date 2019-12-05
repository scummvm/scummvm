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

#include "neverhood/diskplayerscene.h"
#include "neverhood/mouse.h"
#include "neverhood/smackerplayer.h"

namespace Neverhood {

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

AsDiskplayerSceneKey::AsDiskplayerSceneKey(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	createSurface1(0x100B90B4, 1200);
	_x = 211;
	_y = 195;
	startAnimation(0x100B90B4, 0, -1);
	_newStickFrameIndex = 0;
	_needRefresh = true;
	updatePosition();
	setVisible(false);
}

uint32 AsDiskplayerSceneKey::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
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

void AsDiskplayerSceneKey::stDropKey() {
	startAnimation(0x100B90B4, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsDiskplayerSceneKey::handleMessage);
	NextState(&AsDiskplayerSceneKey::stDropKeyDone);
	setVisible(true);
}

void AsDiskplayerSceneKey::stDropKeyDone() {
	stopAnimation();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&Sprite::handleMessage);
	setVisible(false);
}

DiskplayerPlayButton::DiskplayerPlayButton(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene)
	: StaticSprite(vm, 1400), _diskplayerScene(diskplayerScene), _isPlaying(false) {

	loadSprite(0x24A4A664, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x44043000);
	loadSound(1, 0x44045000);
	SetMessageHandler(&DiskplayerPlayButton::handleMessage);
}

uint32 DiskplayerPlayButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_diskplayerScene->getDropKey()) {
			if (_isPlaying) {
				sendMessage(_diskplayerScene, 0x2001, 0);
				release();
			} else {
				sendMessage(_diskplayerScene, 0x2000, 0);
				press();
			}
		}
		updatePosition();
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void DiskplayerPlayButton::press() {
	if (!_isPlaying) {
		setVisible(true);
		updatePosition();
		playSound(0);
		_isPlaying = true;
	}
}

void DiskplayerPlayButton::release() {
	if (_isPlaying) {
		setVisible(false);
		updatePosition();
		playSound(1);
		_isPlaying = false;
	}
}

DiskplayerSlot::DiskplayerSlot(NeverhoodEngine *vm, DiskplayerScene *diskplayerScene, int slotIndex, bool isAvailable)
	: Entity(vm, 0), _diskplayerScene(diskplayerScene), _isLocked(false), _isBlinking(false),
	_blinkCountdown(0), _initialBlinkCountdown(2), _inactiveSlot(NULL), _appearSlot(NULL), _activeSlot(NULL) {

	if (isAvailable && slotIndex < 20) {
		_inactiveSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes1[slotIndex], 1100));
		_appearSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes2[slotIndex], 1000));
		_activeSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes3[slotIndex], 1100));
		_inactiveSlot->setVisible(false);
		_appearSlot->setVisible(false);
		_activeSlot->setVisible(false);
		loadSound(0, 0x46210074);
		setSoundPan(0, slotIndex * 100 / 19);
	} else if (slotIndex != 20) {
		_activeSlot = _diskplayerScene->addSprite(new StaticSprite(_vm, kDiskplayerSlotFileHashes4[slotIndex], 1100));
		_activeSlot->setVisible(false);
	}
	SetUpdateHandler(&DiskplayerSlot::update);
}

void DiskplayerSlot::update() {
	if (_blinkCountdown != 0 && (--_blinkCountdown == 0)) {
		if (_isBlinking) {
			if (_inactiveSlot)
				_inactiveSlot->setVisible(true);
			if (_activeSlot)
				_activeSlot->setVisible(false);
			_blinkCountdown = _initialBlinkCountdown / 2;
		} else {
			if (_inactiveSlot)
				_inactiveSlot->setVisible(false);
			if (_activeSlot)
				_activeSlot->setVisible(true);
			_blinkCountdown = _initialBlinkCountdown;
		}
		_isBlinking = !_isBlinking;
	}
}

void DiskplayerSlot::appear() {
	if (_inactiveSlot)
		_inactiveSlot->setVisible(true);
	if (_appearSlot)
		_appearSlot->setVisible(true);
	if (_inactiveSlot)
		playSound(0);
}

void DiskplayerSlot::play() {
	if (!_isLocked) {
		if (_inactiveSlot)
			_inactiveSlot->setVisible(false);
		if (_activeSlot)
			_activeSlot->setVisible(true);
		_isBlinking = true;
		_blinkCountdown = 0;
	}
}

void DiskplayerSlot::activate() {
	if (!_isLocked)
		_blinkCountdown = _initialBlinkCountdown;
}

void DiskplayerSlot::stop() {
	if (!_isLocked) {
		if (_inactiveSlot)
			_inactiveSlot->setVisible(true);
		if (_activeSlot)
			_activeSlot->setVisible(false);
		_isBlinking = false;
		_blinkCountdown = 0;
	}
}

DiskplayerScene::DiskplayerScene(NeverhoodEngine *vm, Module *parentModule, int paletteIndex)
	: Scene(vm, parentModule), _diskIndex(0), _appearCountdown(0), _tuneInCountdown(0),
	_hasAllDisks(false), _dropKey(false), _inputDisabled(true), _updateStatus(kUSStopped) {

	int availableDisksCount = 0;

	setBackground(0x8A000044);
	setPalette(kDiskplayerPaletteFileHashes[paletteIndex]);

	_ssPlayButton = insertSprite<DiskplayerPlayButton>(this);
	addCollisionSprite(_ssPlayButton);

	_asKey = insertSprite<AsDiskplayerSceneKey>();

	for (int i = 0; i < 20; i++) {
		_diskAvailable[i] = false;
		if (getSubVar(VA_IS_TAPE_INSERTED, i))
			availableDisksCount++;
	}

	for (int i = 0; i < availableDisksCount; i++)
		_diskAvailable[kDiskplayerInitArray[i] - 1] = true;

	for (int slotIndex = 0; slotIndex < 20; slotIndex++) {
		_diskSlots[slotIndex] = new DiskplayerSlot(_vm, this, slotIndex, _diskAvailable[slotIndex]);
		addEntity(_diskSlots[slotIndex]);
	}

	_hasAllDisks = availableDisksCount == 20;

	if (_hasAllDisks && !getGlobalVar(V_HAS_FINAL_KEY))
		_dropKey = true;

	_finalDiskSlot = new DiskplayerSlot(_vm, this, 20, false);
	addEntity(_finalDiskSlot);

	insertPuzzleMouse(0x000408A8, 20, 620);
	showMouse(false);

	_diskSmackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, 0x08288103, false, true));
	_diskSmackerPlayer->setDrawPos(154, 86);
	_vm->_screen->setSmackerDecoder(_diskSmackerPlayer->getSmackerDecoder());

	_palette->usePalette();

	SetMessageHandler(&DiskplayerScene::handleMessage);
	SetUpdateHandler(&DiskplayerScene::update);
	_appearCountdown = 6;

}

void DiskplayerScene::update() {
	Scene::update();

	if (_updateStatus == kUSTuningIn && _diskSmackerPlayer->isDone()) {
		if (_diskAvailable[_diskIndex])
			playDisk();
		else
			playStatic();
	} else if (_updateStatus == kUSPlaying && _diskSmackerPlayer->isDone()) {
		_diskSlots[_diskIndex]->stop();
		_diskIndex++;
		if (_hasAllDisks) {
			if (_diskIndex != 20) {
				playDisk();
			} else if (_dropKey) {
				playDisk();
				_updateStatus = kUSPlayingFinal;
			} else {
				_diskIndex = 0;
				stop();
			}
		} else if (_diskIndex != 20) {
			tuneIn();
		} else {
			_diskIndex = 0;
			stop();
		}
	} else if (_updateStatus == kUSPlayingFinal) {
		if (_diskSmackerPlayer->getFrameNumber() == 133) {
			_asKey->stDropKey();
			setGlobalVar(V_HAS_FINAL_KEY, 1);
		} else if (_diskSmackerPlayer->isDone()) {
			for (int i = 0; i < 20; i++) {
				_diskSlots[i]->setLocked(false);
				_diskSlots[i]->stop();
			}
			_diskIndex = 0;
			stop();
			showMouse(true);
			_dropKey = false;
		}
	}

	if (_appearCountdown != 0 && (--_appearCountdown == 0)) {
		_diskSlots[_diskIndex]->appear();
		if (_dropKey) {
			_diskSlots[_diskIndex]->activate();
			_diskSlots[_diskIndex]->setLocked(true);
		}
		_diskIndex++;
		while (!_diskAvailable[_diskIndex] && _diskIndex < 19)
			_diskIndex++;
		if (_diskIndex < 20) {
			_appearCountdown = 1;
		} else {
			_diskIndex = 0;
			_inputDisabled = false;
			if (_dropKey) {
				_ssPlayButton->press();
				_tuneInCountdown = 2;
			} else {
				showMouse(true);
				_diskSlots[_diskIndex]->activate();
			}
		}
	}

	if (_tuneInCountdown != 0 && (--_tuneInCountdown == 0))
		playDisk();

}

uint32 DiskplayerScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	if (!_inputDisabled) {
		switch (messageNum) {
		case NM_MOUSE_CLICK:
			if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
				sendMessage(_parentModule, 0x1009, 0);
			} else if (!_dropKey &&
				param.asPoint().x > 38 && param.asPoint().x < 598 &&
				param.asPoint().y > 400 && param.asPoint().y < 460) {

				_diskSlots[_diskIndex]->stop();
				_diskIndex = (param.asPoint().x - 38) / 28;
				_diskSlots[_diskIndex]->activate();
				if (_updateStatus == kUSPlaying) {
					if (_diskAvailable[_diskIndex])
						playDisk();
					else
						playStatic();
				}
			}
			break;
		case NM_ANIMATION_UPDATE:
			tuneIn();
			break;
		case 0x2001:
			stop();
			break;
		default:
			break;
		}
	}
	return 0;
}

void DiskplayerScene::openSmacker(uint32 fileHash, bool keepLastFrame) {
	_diskSmackerPlayer->open(fileHash, keepLastFrame);
	_vm->_screen->setSmackerDecoder(_diskSmackerPlayer->getSmackerDecoder());
	_palette->usePalette();
}

void DiskplayerScene::stop() {
	openSmacker(0x08288103, true);
	_ssPlayButton->release();
	_updateStatus = kUSStopped;
	_diskSlots[_diskIndex]->activate();
}

void DiskplayerScene::tuneIn() {
	openSmacker(0x900001C1, false);
	_ssPlayButton->release();
	_updateStatus = kUSTuningIn;
	_diskSlots[_diskIndex]->activate();
}

void DiskplayerScene::playDisk() {
	openSmacker(kDiskplayerSmackerFileHashes[_diskIndex], false);
	_updateStatus = kUSPlaying;
	_diskSlots[_diskIndex]->play();
}

void DiskplayerScene::playStatic() {
	openSmacker(0x90000101, false);
	_ssPlayButton->release();
	_updateStatus = kUSPlaying;
	_diskSlots[_diskIndex]->activate();
}

} // End of namespace Neverhood
