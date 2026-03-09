/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bolt/bolt.h"

#include "common/memstream.h"

namespace Bolt {

void BoltEngine::playSoundMapYogi(int16 memberId) {
	byte *soundData = getBOLTMember(_yogiBoltLib, memberId);
	int32 soundSize = memberSize(_yogiBoltLib, memberId);
	if (soundData) {
		_xp->playSound(soundData, soundSize, 22050);
		_yogiSoundActive = 1;
		_yogiSoundPlaying++;
	}
}

void BoltEngine::waitSoundMapYogi() {
	if (!_yogiSoundPlaying)
		return;

	uint32 dummy;
	while (_xp->getEvent(etSound, &dummy) == etEmpty);

	_yogiSoundActive = 0;
	_yogiSoundPlaying--;
}

void BoltEngine::stopSoundYogi() {
	if (_yogiSoundActive) {
		_xp->stopSound();
		_yogiSoundActive = 0;
	}
}

void BoltEngine::setYogiColors(int16 which) {
	if (which == 0)
		_xp->setPalette(_yogiPalRange[1] - _yogiPalRange[0] + 1, _yogiPalRange[0], _yogiPalHighlight0);
	else if (which == 1)
		_xp->setPalette(_yogiPalRange[5] - _yogiPalRange[4] + 1, _yogiPalRange[4], _yogiPalHighlight1);
}

void BoltEngine::restoreYogiColors(int16 which) {
	if (which == 0)
		_xp->setPalette(_yogiPalRange[1] - _yogiPalRange[0] + 1, _yogiPalRange[0], _yogiPalSave0);
	else if (which == 1)
		_xp->setPalette(_yogiPalRange[5] - _yogiPalRange[4] + 1, _yogiPalRange[4], _yogiPalSave1);
}

void BoltEngine::drawBasket(int16 slot, byte *basketSprite) {
	int16 sprStride = READ_UINT16(basketSprite + 0x0A);
	int16 sprHeight = READ_UINT16(basketSprite + 0x0C);

	memset(_yogiScratchBuf.pixelData, 0, (int32)sprStride * sprHeight);

	int16 slotY = READ_UINT16(_yogiBasketPic + slot * 2 + 0x42);
	int16 slotX = READ_UINT16(_yogiBasketPic + slot * 2 + 0x12);
	Common::Rect targetRect(slotY, slotX, slotY + sprHeight, slotX + sprStride);

	int16 basketCount = READ_UINT16(_yogiBasketPic);
	for (int16 i = 0; i < basketCount; i++) {
		if (_yogiState.basketState[i] != 0)
			continue;

		byte *otherSprite;
		if (i == _yogiState.selected1Slot || i == _yogiState.selected2Slot)
			otherSprite = _yogiHlSprite;
		else
			otherSprite = _yogiNormalSprite;

		int16 otherStride = READ_UINT16(otherSprite + 0x0A);
		int16 otherHeight = READ_UINT16(otherSprite + 0x0C);
		int16 otherY = READ_UINT16(_yogiBasketPic + i * 2 + 0x42);
		int16 otherX = READ_UINT16(_yogiBasketPic + i * 2 + 0x12);
		Common::Rect otherRect(otherY, otherX, otherY + otherHeight, otherX + otherStride);

		Common::Rect isect;
		if (!intersectRect(&targetRect, &otherRect, &isect))
			continue;

		byte *srcPixels = getResolvedPtr(otherSprite, 0x12);
		byte *src = srcPixels + (isect.left - otherY) * otherStride + (isect.top - otherX);
		byte *dst = _yogiScratchBuf.pixelData + (isect.left - slotY) * sprStride + (isect.top - slotX);

		int16 blitWidth = isect.bottom - isect.top;
		int16 blitHeight = isect.right - isect.left;
		_xp->maskBlit(src, otherStride, dst, sprStride, blitWidth, blitHeight);
	}

	_yogiScratchBuf.width = sprStride;
	_yogiScratchBuf.height = sprHeight;
	_yogiScratchBuf.palette = nullptr;
	_yogiScratchBuf.flags = 0;

	_xp->displayPic(&_yogiScratchBuf, slotX, slotY, stFront);
}

void BoltEngine::drawAllBaskets() {
	_xp->fillDisplay(0, stFront);

	int16 basketCount = READ_UINT16(_yogiBasketPic);
	for (int16 i = 0; i < basketCount; i++) {
		if (_yogiState.basketState[i] != 0)
			continue;

		byte *sprite = (i == _yogiState.selected1Slot || i == _yogiState.selected2Slot)
						   ? _yogiHlSprite
						   : _yogiNormalSprite;

		int16 slotY = READ_UINT16(_yogiBasketPic + i * 2 + 0x42);
		int16 slotX = READ_UINT16(_yogiBasketPic + i * 2 + 0x12);
		displayPic(sprite, slotX, slotY, stFront);
	}
}

void BoltEngine::handleYogiMatch() {
	playSoundMapYogi((int16)READ_UINT16(_yogiBasketPic + 0x9E));

	if (_yogiState.basketCount == 2) {
		_yogiExitFlag = 1;
		_yogiState.levelComplete = 1;
		_yogiState.levelNumber++;
		if (_yogiState.levelNumber > 10)
			_yogiState.levelNumber = 10;
		_xp->fillDisplay(0, stFront);
	} else {
		drawBasket(_yogiState.selected1Slot, _yogiHlSprite);
		drawBasket(_yogiState.selected2Slot, _yogiHlSprite);
	}

	_yogiState.selected1Slot = -1;
	_yogiState.selected2Slot = -1;
	_yogiState.basketCount -= 2;
	_xp->updateDisplay();
	waitSoundMapYogi();
}

bool BoltEngine::loadYogiBgPic() {
	int16 groupId;

	if (_displayMode == 0) {
		groupId = 0x100;
	} else if (_displayMode == 1) {
		groupId = 0x200;
	} else {
		return false;
	}

	if (!getBOLTGroup(_yogiBoltLib, groupId, 1))
		return false;

	_yogiBgPic = memberAddr(_yogiBoltLib, groupId + 1);
	return true;
}

void BoltEngine::unloadYogiBgPic() {
	freeBOLTGroup(_yogiBoltLib, 0x100, 1);
}

void BoltEngine::drawYogiLevel() {
	byte *palSprite = memberAddr(_yogiBoltLib, READ_UINT16(_yogiBasketPic + 0x72));

	_yogiSoundActive = 0;
	_yogiHotSpotCount = 0;
	_yogiBlinkTimer1 = 0;
	_yogiBlinkTimer2 = 0;
	_yogiExitFlag = 0;

	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);
	displayColors(palSprite, stBack, 0);
	displayPic(_yogiBgPic, _displayX, _displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(palSprite, stFront, 0);
	displayColors(palSprite, stBack, 1);
	displayPic(_yogiBgPic, _displayX, _displayY, stBack);

	drawAllBaskets();
	_xp->updateDisplay();

	_yogiCursorX = 0xC0;
	_yogiCursorY = 0x78;
	_xp->setCursorPos(0xC0, 0x78);
	_xp->setCursorColor(0, 0, 0xFF);
	_xp->showCursor();

	_yogiPalRange[0] = READ_UINT16(_yogiBasketPic + 0x7C) + 0x80;
	_yogiPalRange[1] = READ_UINT16(_yogiBasketPic + 0x7E) + 0x80;
	_yogiPalRange[2] = READ_UINT16(_yogiBasketPic + 0x80) + 0x80;
	_yogiPalRange[3] = READ_UINT16(_yogiBasketPic + 0x82) + 0x80;
	_yogiPalRange[4] = READ_UINT16(_yogiBasketPic + 0x8E) + 0x80;
	_yogiPalRange[5] = READ_UINT16(_yogiBasketPic + 0x90) + 0x80;
	_yogiPalRange[6] = READ_UINT16(_yogiBasketPic + 0x92) + 0x80;
	_yogiPalRange[7] = READ_UINT16(_yogiBasketPic + 0x94) + 0x80;

	_xp->getPalette(_yogiPalRange[0], _yogiPalRange[1] - _yogiPalRange[0] + 1, _yogiPalSave0);
	_xp->getPalette(_yogiPalRange[2], _yogiPalRange[3] - _yogiPalRange[2] + 1, _yogiPalHighlight0);
	_xp->getPalette(_yogiPalRange[4], _yogiPalRange[5] - _yogiPalRange[4] + 1, _yogiPalSave1);
	_xp->getPalette(_yogiPalRange[6], _yogiPalRange[7] - _yogiPalRange[6] + 1, _yogiPalHighlight1);
}

bool BoltEngine::loadYogiLevel() {
	int32 maxSize = 0;

	_yogiLevelGroupId = (_yogiState.levelNumber - 1) * 0x100 + 0x300;

	if (!getBOLTGroup(_yogiBoltLib, _yogiLevelGroupId, 1))
		return false;

	_yogiBasketPic = memberAddr(_yogiBoltLib, _yogiLevelGroupId);
	_yogiNormalSprite = memberAddr(_yogiBoltLib, READ_UINT16(_yogiBasketPic + 0x02));
	_yogiHlSprite = memberAddr(_yogiBoltLib, READ_UINT16(_yogiBasketPic + 0x04));
	_yogiAnimSprite = memberAddr(_yogiBoltLib, READ_UINT16(_yogiBasketPic + 0x06));

	int32 size;

	size = (int32)READ_UINT16(_yogiNormalSprite + 0x0A) * READ_UINT16(_yogiNormalSprite + 0x0C);
	if (size > maxSize)
		maxSize = size;

	size = (int32)READ_UINT16(_yogiHlSprite + 0x0A) * READ_UINT16(_yogiHlSprite + 0x0C);
	if (size > maxSize)
		maxSize = size;

	size = (int32)READ_UINT16(_yogiAnimSprite + 0x0A) * READ_UINT16(_yogiAnimSprite + 0x0C);
	if (size > maxSize)
		maxSize = size;

	_yogiScratchBuf.pixelData = (byte *)_xp->allocMem(maxSize);
	if (!_yogiScratchBuf.pixelData)
		return false;

	_yogiSpriteStride = READ_UINT16(_yogiNormalSprite + 0x0A);
	_yogiSpriteHeight = READ_UINT16(_yogiNormalSprite + 0x0C);
	return true;
}

void BoltEngine::unloadYogiResources() {
	_xp->hideCursor();
	int16 basketGroupId = (_yogiLevelId << 8) + 0xD00;
	freeBOLTGroup(_yogiBoltLib, basketGroupId, 1);
	freeBOLTGroup(_yogiBoltLib, _yogiLevelGroupId, 1);
}

bool BoltEngine::initYogiLevel() {
	_yogiState.currentSlot++;
	if (_yogiState.currentSlot >= 10)
		_yogiState.currentSlot = 0;

	_yogiLevelId = _yogiState.slotIndex[_yogiState.currentSlot];

	if (!loadYogiLevel())
		return false;

	int16 basketCount = READ_UINT16(_yogiBasketPic);
	_yogiState.basketCount = basketCount;
	_yogiState.levelComplete = 0;
	_yogiState.matchCount = 0;
	_yogiState.selectionPending = 0;
	_yogiState.selected1Slot = -1;
	_yogiState.selected2Slot = -1;

	for (int16 i = 0; i < 0x18; i++) {
		_yogiState.basketSound[i] = 0x10;
		_yogiState.basketState[i] = 0;
	}

	int16 idx = _yogiState.levelIndex[_yogiLevelId];
	int16 snd = _yogiGlobal[_yogiLevelId * 0x10 + idx];
	int16 flag = 0;

	for (int16 var_4 = 0; var_4 < basketCount; var_4++) {
		int16 rnd = _xp->getRandom(basketCount);
		while (_yogiState.basketSound[rnd] != 0x10) {
			rnd++;
			if (rnd >= basketCount)
				rnd = 0;
		}
		_yogiState.basketSound[rnd] = snd;

		if (flag == 0) {
			flag = 1;
		} else {
			flag = 0;
			idx++;
			if (idx == 0x10)
				idx = 0;
			snd = _yogiGlobal[_yogiLevelId * 0x10 + idx];
		}
	}

	_yogiState.levelIndex[_yogiLevelId] = idx;
	drawYogiLevel();
	return true;
}

bool BoltEngine::resumeYogiLevel() {
	_yogiLevelId = _yogiState.slotIndex[_yogiState.currentSlot];

	if (!loadYogiLevel())
		return false;

	drawYogiLevel();
	return true;
}

bool BoltEngine::initYogi() {
	_yogiSoundPlaying = 0;

	if (!loadYogiBgPic())
		return false;

	if (!vLoad(&_yogiGlobal, "YogiGlobal")) {
		int16 slotVal = 0;
		int16 globIdx = 0;
		int16 idx = 0;

		while (slotVal < 10) {
			_yogiState.slotIndex[idx] = slotVal;
			_yogiState.levelIndex[idx] = 0;

			for (int16 i = 0; i < 16; i++)
				_yogiGlobal[globIdx + i] = 0x10;

			for (int16 di = 0; di < 16; di++) {
				int16 rnd = _xp->getRandom(16);
				while (_yogiGlobal[globIdx + rnd] != 0x10) {
					rnd++;
					if (rnd >= 16)
						rnd = 0;
				}
				_yogiGlobal[globIdx + rnd] = di;
			}

			globIdx += 0x10;
			idx++;
			slotVal++;
		}
	}

	byte yogiStateBuf[0x9C] = { 0 };

	if (!vLoad(&yogiStateBuf, "Yogi")) {
		_yogiState.levelNumber = 1;
		_yogiState.currentSlot = -1;
		return initYogiLevel();
	}

	Common::SeekableReadStream *yogiStateReadStream = new Common::MemoryReadStream(yogiStateBuf, sizeof(yogiStateBuf), DisposeAfterUse::NO);

	_yogiState.levelNumber = yogiStateReadStream->readSint16BE();
	_yogiState.currentSlot = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 10; i++)
		_yogiState.levelIndex[i] = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 10; i++)
		_yogiState.slotIndex[i] = yogiStateReadStream->readSint16BE();

	_yogiState.levelComplete = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		_yogiState.basketSound[i] = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		_yogiState.basketState[i] = yogiStateReadStream->readSint16BE();

	_yogiState.basketCount = yogiStateReadStream->readSint16BE();
	_yogiState.matchCount = yogiStateReadStream->readSint16BE();
	_yogiState.selectionPending = yogiStateReadStream->readSint16BE();
	_yogiState.selected1Slot = yogiStateReadStream->readSint16BE();
	_yogiState.selected2Slot = yogiStateReadStream->readSint16BE();
	_yogiState.sound1 = yogiStateReadStream->readSint16BE();
	_yogiState.sound2 = yogiStateReadStream->readSint16BE();

	// Sanity check: should be exactly 0x9C
	assert(yogiStateReadStream->pos() == 0x9C);
	delete yogiStateReadStream;

	if (_yogiState.levelComplete)
		return initYogiLevel();
	else
		return resumeYogiLevel();
}

void BoltEngine::yogiToggleBlinking(int16 which, int16 *state) {
	*state = (*state == 0) ? 1 : 0;
	if (*state)
		setYogiColors(which);
	else
		restoreYogiColors(which);
}

void BoltEngine::yogiUpdateHotSpots(int16 x, int16 y) {
	Common::Rect helpRect(
		READ_UINT16(_yogiBasketPic + 0x74), READ_UINT16(_yogiBasketPic + 0x78),
		READ_UINT16(_yogiBasketPic + 0x76), READ_UINT16(_yogiBasketPic + 0x7A));

	Common::Rect exitRect(
		READ_UINT16(_yogiBasketPic + 0x86), READ_UINT16(_yogiBasketPic + 0x8A),
		READ_UINT16(_yogiBasketPic + 0x88), READ_UINT16(_yogiBasketPic + 0x8C));

	if (helpRect.contains(x, y)) {
		if (!_yogiBlinkTimer1 && !_yogiHotSpotCount)
			setYogiColors(0);
	} else {
		if (!_yogiBlinkTimer1 && !_yogiHotSpotCount)
			restoreYogiColors(0);
	}

	if (exitRect.contains(x, y)) {
		if (!_yogiBlinkTimer2)
			setYogiColors(1);
		return;
	} else {
		if (!_yogiBlinkTimer2)
			restoreYogiColors(1);
	}
}

int16 BoltEngine::findBasket(int16 x, int16 y) {
	int16 basketCount = READ_UINT16(_yogiBasketPic);
	for (int16 i = basketCount - 1; i >= 0; i--) {
		if (_yogiState.basketState[i] != 0)
			continue;

		int16 slotX = READ_UINT16(_yogiBasketPic + i * 2 + 0x12);
		int16 slotX2 = slotX + _yogiSpriteStride - 1;
		int16 slotY = READ_UINT16(_yogiBasketPic + i * 2 + 0x42);
		int16 slotY2 = slotY + _yogiSpriteHeight - 1;

		if (x < slotX || x > slotX2 || y < slotY || y > slotY2)
			continue;

		byte *sprite = (i == _yogiState.selected1Slot || i == _yogiState.selected2Slot)
						   ? _yogiHlSprite
						   : _yogiNormalSprite;

		if (!getPixel(sprite, x - slotX, y - slotY))
			continue;

		return i;
	}

	return -1;
}

void BoltEngine::resolveYogiSelection() {
	if (_yogiState.sound1 == _yogiState.sound2) {
		_yogiState.basketState[_yogiState.selected1Slot] = 1;
		_yogiState.basketState[_yogiState.selected2Slot] = 1;
		handleYogiMatch();
		_yogiState.matchCount = 0;
		_yogiState.selectionPending = 0;
	} else {
		int16 slot1 = _yogiState.selected1Slot;
		_yogiState.matchCount = 0;
		_yogiState.selected1Slot = -1;
		drawBasket(slot1, _yogiNormalSprite);
		_xp->updateDisplay();
		playSoundMapYogi((int32)(int16)READ_UINT16(_yogiBasketPic + 0x9A));
		waitSoundMapYogi();

		int16 slot2 = _yogiState.selected2Slot;
		_yogiState.selectionPending = 0;
		_yogiState.selected2Slot = -1;
		drawBasket(slot2, _yogiNormalSprite);
		_xp->updateDisplay();
		playSoundMapYogi((int32)(int16)READ_UINT16(_yogiBasketPic + 0x9C));
		waitSoundMapYogi();
	}
}

bool BoltEngine::handleBasketSelect(int16 x, int16 y) {
	int16 slot = findBasket(x, y);
	if (slot == -1)
		return false;

	if (_yogiState.matchCount == 0) {
		_yogiState.matchCount = 1;
		_yogiState.sound1 = _yogiState.basketSound[slot];
		_yogiState.selected1Slot = slot;

		playSoundMapYogi((int32)(int16)READ_UINT16(_yogiBasketPic + 0x98));
		drawBasket(slot, _yogiHlSprite);
		_xp->updateDisplay();
		waitSoundMapYogi();

		int32 soundId = (int32)((_yogiLevelId << 8) + _yogiState.sound1 + 0xD00);
		playSoundMapYogi(soundId);

	} else {
		if (_yogiState.selectionPending)
			return true;

		if (_yogiState.selected1Slot == slot) {
			// De-select basket...
			_yogiState.matchCount = 0;
			_yogiState.selected1Slot = -1;

			stopSoundYogi();
			playSoundMapYogi((int32)(int16)READ_UINT16(_yogiBasketPic + 0x9A));
			drawBasket(slot, _yogiNormalSprite);
			_xp->updateDisplay();
			waitSoundMapYogi();

		} else {
			_yogiState.selectionPending = 1;
			_yogiState.sound2 = _yogiState.basketSound[slot];
			_yogiState.selected2Slot = slot;

			if (_yogiSoundActive) {
				int16 slotY = READ_UINT16(_yogiBasketPic + slot * 2 + 0x42);
				int16 slotX = READ_UINT16(_yogiBasketPic + slot * 2 + 0x12);
				displayPic(_yogiAnimSprite, slotX + READ_UINT16(_yogiBasketPic + 0x0A), slotY + READ_UINT16(_yogiBasketPic + 0x0C), stFront);
				_xp->updateDisplay();
				waitSoundMapYogi();
			}

			playSoundMapYogi((int32)(int16)READ_UINT16(_yogiBasketPic + 0x98));
			drawBasket(slot, _yogiHlSprite);
			_xp->updateDisplay();
			waitSoundMapYogi();

			int32 soundId = (int32)((_yogiLevelId << 8) + _yogiState.sound2 + 0xD00);
			playSoundMapYogi(soundId);
			waitSoundMapYogi();

			resolveYogiSelection();
		}
	}

	return true;
}

void BoltEngine::yogiHandleActionButton(int16 x, int16 y) {
	bool didAction = false;
	bool stoppedAnim = false;

	if (_yogiHotSpotCount != 0) {
		_yogiHotSpotCount = 0;
		stoppedAnim = true;
		stopAnimation();
		if (_yogiBlinkTimer2) {
			_xp->killTimer(_yogiBlinkTimer2);
			_yogiBlinkTimer2 = 0;
		}

		yogiUpdateHotSpots(_yogiCursorX, _yogiCursorY);
	}

	Common::Rect helpRect(
		READ_UINT16(_yogiBasketPic + 0x74), READ_UINT16(_yogiBasketPic + 0x78),
		READ_UINT16(_yogiBasketPic + 0x76), READ_UINT16(_yogiBasketPic + 0x7A));

	if (helpRect.contains(x, y)) {
		didAction = true;
		stopSoundYogi();
		if (_yogiBlinkTimer1) {
			_xp->killTimer(_yogiBlinkTimer1);
			_yogiBlinkTimer1 = 0;
			setYogiColors(0);
		}
		if (!_yogiHotSpotCount && !stoppedAnim) {
			_yogiHotSpotCount = startAnimation(_rtfHandle, 0x1D);
			_yogiAnimActive = 0;
		}
	} else {
		Common::Rect exitRect(
			READ_UINT16(_yogiBasketPic + 0x86), READ_UINT16(_yogiBasketPic + 0x8A),
			READ_UINT16(_yogiBasketPic + 0x88), READ_UINT16(_yogiBasketPic + 0x8C));

		if (exitRect.contains(x, y)) {
			stopSoundYogi();
			setYogiColors(1);
			restoreYogiColors(0);
			_yogiReturnBooth = 6;
			_yogiExitFlag = 1;
		}
	}

	if (handleBasketSelect(x, y))
		didAction = true;

	if (didAction) {
		if (_yogiBlinkTimer1) {
			_xp->killTimer(_yogiBlinkTimer1);
			_yogiBlinkTimer1 = 0;
			restoreYogiColors(0);
		}
		_xp->setInactivityTimer(0x1E);
	}
}

void BoltEngine::handleYogiEvent(int16 eventType, uint32 eventData) {
	switch (eventType) {
	case etMouseDown:
		yogiHandleActionButton(_yogiCursorX, _yogiCursorY);
		break;

	case etMouseMove:
		_yogiCursorX = (int16)(eventData >> 16);
		_yogiCursorY = (int16)(eventData & -1);
		yogiUpdateHotSpots(_yogiCursorX, _yogiCursorY);
		break;

	case etTimer: { // 1
		int32 blinkInterval = ((int32)READ_UINT16(_yogiBasketPic + 0x84)) * 1000 / 60;

		if (eventData == _yogiBlinkTimer1) {
			yogiToggleBlinking(0, &_yogiBlinkState1);
			_yogiBlinkTimer1 = _xp->startTimer((int16)blinkInterval);
		} else if (eventData == _yogiBlinkTimer2) {
			yogiToggleBlinking(1, &_yogiBlinkState2);
			_yogiBlinkTimer2 = _xp->startTimer((int16)blinkInterval);
		}

		break;
	}

	case etInactivity:
		if (!_yogiBlinkTimer1) {
			int32 blinkInterval = ((int32)READ_UINT16(_yogiBasketPic + 0x84)) * 1000 / 60;
			_yogiBlinkTimer1 = _xp->startTimer((int16)blinkInterval);
			_yogiBlinkState1 = 0;
			yogiToggleBlinking(0, &_yogiBlinkState1);
		}

		break;

	case etSound:
		if (_yogiHotSpotCount != 0) {
			_yogiHotSpotCount = maintainAudioPlay(1);
			if (_yogiHotSpotCount == 0)
				yogiUpdateHotSpots(_yogiCursorX, _yogiCursorY);
		} else {
			_yogiSoundActive = 0;
		}

		if (_yogiSoundPlaying != 0)
			_yogiSoundPlaying--;

		break;

	case etTrigger:
		_yogiAnimActive++;

		if (_yogiAnimActive == 1) {
			int32 blinkInterval = ((int32)READ_UINT16(_yogiBasketPic + 0x84)) * 1000 / 60;
			_yogiBlinkTimer2 = _xp->startTimer((int16)blinkInterval);
			_yogiBlinkState2 = 0;
			yogiToggleBlinking(1, &_yogiBlinkState2);
		} else if (_yogiAnimActive == 2) {
			_xp->killTimer(_yogiBlinkTimer2);
			_yogiBlinkTimer2 = 0;
			restoreYogiColors(1);
		}

		break;

	default:
		break;
	}
}

void BoltEngine::playYogi() {
	_yogiReturnBooth = 16;

	while (!shouldQuit()) {
		if (_yogiExitFlag)
			break;

		uint32 eventData = 0;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		if (eventType)
			handleYogiEvent(eventType, eventData);

		if (_yogiHotSpotCount != 0) {
			_yogiHotSpotCount = maintainAudioPlay(0);
			if (_yogiHotSpotCount == 0)
				yogiUpdateHotSpots(_yogiCursorX, _yogiCursorY);
		}
	}

	if (shouldQuit())
		_yogiReturnBooth = 0;
}

void BoltEngine::cleanUpYogi() {
	if (_yogiScratchBuf.pixelData) {
		_xp->freeMem(_yogiScratchBuf.pixelData);
		_yogiScratchBuf.pixelData = nullptr;
		_yogiScratchBuf.width = 0;
		_yogiScratchBuf.height = 0;
	}

	vSave(&_yogiGlobal, 0x140, "YogiGlobal");

	byte yogiSaveBuf[0x9C] = { 0 };
	Common::MemoryWriteStream *yogiStateWriteStream = new Common::MemoryWriteStream(yogiSaveBuf, sizeof(yogiSaveBuf));

	yogiStateWriteStream->writeSint16BE(_yogiState.levelNumber);
	yogiStateWriteStream->writeSint16BE(_yogiState.currentSlot);

	for (int i = 0; i < 10; i++)
		yogiStateWriteStream->writeSint16BE(_yogiState.levelIndex[i]);

	for (int i = 0; i < 10; i++)
		yogiStateWriteStream->writeSint16BE(_yogiState.slotIndex[i]);

	yogiStateWriteStream->writeSint16BE(_yogiState.levelComplete);

	for (int i = 0; i < 24; i++)
		yogiStateWriteStream->writeSint16BE(_yogiState.basketSound[i]);

	for (int i = 0; i < 24; i++)
		yogiStateWriteStream->writeSint16BE(_yogiState.basketState[i]);

	yogiStateWriteStream->writeSint16BE(_yogiState.basketCount);
	yogiStateWriteStream->writeSint16BE(_yogiState.matchCount);
	yogiStateWriteStream->writeSint16BE(_yogiState.selectionPending);
	yogiStateWriteStream->writeSint16BE(_yogiState.selected1Slot);
	yogiStateWriteStream->writeSint16BE(_yogiState.selected2Slot);
	yogiStateWriteStream->writeSint16BE(_yogiState.sound1);
	yogiStateWriteStream->writeSint16BE(_yogiState.sound2);

	// Sanity check: should be exactly 0x9C
	assert(yogiStateWriteStream->pos() == 0x9C);
	delete yogiStateWriteStream;

	vSave(&yogiSaveBuf, 0x9C, "Yogi");

	unloadYogiResources();
	unloadYogiBgPic();

	_xp->stopCycle();
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

int16 BoltEngine::yogiGame(int16 prevBooth) {
	if (!openBOLTLib(&_yogiBoltLib, &_yogiBoltCallbacks, assetPath("yogi.blt")))
		return _yogiReturnBooth;

	int16 savedTimer = _xp->setInactivityTimer(30);

	if (initYogi())
		playYogi();

	cleanUpYogi();
	_xp->setInactivityTimer(savedTimer);
	closeBOLTLib(&_yogiBoltLib);

	return _yogiReturnBooth;
}

void BoltEngine::swapYogiAllWords() {
	byte *ptr = _boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	int16 count = (int16)(_boltCurrentMemberEntry->decompSize >> 1);
	for (int16 i = 0; i < count; i++) {
		WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
		ptr += 2;
	}
}

void BoltEngine::swapYogiFirstWord() {
	byte *ptr = (byte *)_boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
}

} // End of namespace Bolt
