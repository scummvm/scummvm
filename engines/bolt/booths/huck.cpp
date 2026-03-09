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

void BoltEngine::playSoundMapHuck(int16 memberId) {
	byte *soundData = getBOLTMember(_huckBoltLib, memberId);
	uint32 soundSize = memberSize(_huckBoltLib, memberId);
	if (soundData) {
		_xp->playSound(soundData, soundSize, 22050);
		_huckSoundPlaying++;
	}
}

void BoltEngine::waitSoundMapHuck() {
	if (!_huckSoundPlaying)
		return;

	uint32 dummy;
	while (_xp->getEvent(etSound, &dummy) == etEmpty);
	_huckSoundPlaying--;
}

void BoltEngine::setHuckColors(int16 which) {
	if (which == 0) {
		int16 count = _huckPalRange[1] - _huckPalRange[0] + 1;
		_xp->setPalette(count, _huckPalRange[0], _huckPalHighlight0);
	} else if (which == 1) {
		int16 count = _huckPalRange[5] - _huckPalRange[4] + 1;
		_xp->setPalette(count, _huckPalRange[4], _huckPalHighlight1);
	}
}

void BoltEngine::restoreHuckColors(int16 which) {
	if (which == 0) {
		int16 count = _huckPalRange[1] - _huckPalRange[0] + 1;
		_xp->setPalette(count, _huckPalRange[0], _huckPalSave0);
	} else if (which == 1) {
		int16 count = _huckPalRange[5] - _huckPalRange[4] + 1;
		_xp->setPalette(count, _huckPalRange[4], _huckPalSave1);
	}
}

void BoltEngine::startHuckShuffleTimer() {
	if (_huckShuffleTimer) {
		_xp->killTimer(_huckShuffleTimer);
		_huckShuffleTimer = 0;
	}

	if (_huckState.giftCount > 2) {
		int16 speed = READ_UINT16(_huckGiftPic + _huckScrollOffset * 2 + 0x6E);
		int32 ms = (int32)speed * 1000 / 60;
		_huckShuffleTimer = _xp->startTimer((int16)ms);
	}
}

void BoltEngine::drawGift(int16 slot) {
	byte *giftSprite = memberAddr(_huckBoltLib, _huckState.drawTable1[slot]);
	int16 sprStride = READ_UINT16(giftSprite + 0x0A);
	int16 sprHeight = READ_UINT16(giftSprite + 0x0C);

	memset(_huckScratchPic.pixelData, 0, (int32)sprStride * sprHeight);

	int16 slotX = READ_UINT16(_huckGiftPic + slot * 2 + 0x3E);
	int16 slotY = READ_UINT16(_huckGiftPic + slot * 2 + 0x0E);
	Common::Rect targetRect(slotX, slotY, slotX + sprHeight, slotY + sprStride);

	int16 giftCount = READ_UINT16(_huckGiftPic);
	for (int16 i = 0; i < giftCount; i++) {
		if (_huckState.drawTable2[i] != 0)
			continue;

		byte *otherSprite = memberAddr(_huckBoltLib, _huckState.drawTable1[i]);
		int16 otherStride = READ_UINT16(otherSprite + 0x0A);
		int16 otherHeight = READ_UINT16(otherSprite + 0x0C);
		int16 ox = READ_UINT16(_huckGiftPic + i * 2 + 0x3E);
		int16 oy = READ_UINT16(_huckGiftPic + i * 2 + 0x0E);
		Common::Rect otherRect(ox, oy, ox + otherHeight, oy + otherStride);

		Common::Rect isect;
		if (!intersectRect(&targetRect, &otherRect, &isect))
			continue;

		byte *srcPixels = getResolvedPtr(otherSprite, 0x12);
		byte *src = srcPixels + (isect.left - ox) * otherStride + (isect.top - oy);

		byte *dst = _huckScratchPic.pixelData + (isect.left - slotX) * sprStride + (isect.top - slotY);

		int16 blitWidth = isect.bottom - isect.top;
		int16 blitHeight = isect.right - isect.left;
		_xp->maskBlit(src, otherStride, dst, sprStride, blitWidth, blitHeight);
	}

	if ((_huckState.hasCycle && _huckState.selected1Slot == slot) || (_huckState.selectionPending && _huckState.selected2Slot == slot)) {
		byte *hlSprite = memberAddr(_huckBoltLib, READ_UINT16(_huckGiftPic + 0x06));
		int16 hlStride = READ_UINT16(hlSprite + 0x0A);
		int16 hlHeight = READ_UINT16(hlSprite + 0x0C);
		byte *hlPixels = getResolvedPtr(hlSprite, 0x12);
		int16 hlOffsetX = READ_UINT16(_huckGiftPic + 0x0A);
		int16 hlOffsetY = READ_UINT16(_huckGiftPic + 0x08);

		byte *dst = _huckScratchPic.pixelData + hlOffsetX * sprStride + hlOffsetY;

		_xp->maskBlit(hlPixels, hlStride, dst, sprStride, hlStride, hlHeight);
	}

	_huckScratchPic.width = sprStride;
	_huckScratchPic.height = sprHeight;
	_huckScratchPic.palette = nullptr;
	_huckScratchPic.flags = 0;

	_xp->displayPic(&_huckScratchPic, slotY, slotX, stFront);
}

void BoltEngine::drawHuckGifts() {
	_xp->fillDisplay(0, stFront);

	int16 giftCount = READ_UINT16(_huckGiftPic);

	for (int16 slot = 0; slot < giftCount; slot++) {
		if (_huckState.drawTable2[slot] != 0)
			continue;

		byte *gifPtr = memberAddr(_huckBoltLib, _huckState.drawTable1[slot]);
		int16 x = READ_UINT16(_huckGiftPic + slot * 2 + 0x0E);
		int16 y = READ_UINT16(_huckGiftPic + slot * 2 + 0x3E);
		displayPic(gifPtr, x, y, stFront);
	}

	if (_huckState.hasCycle) {
		int16 x = READ_UINT16(_huckGiftPic + _huckState.selected1Slot * 2 + 0x0E) + READ_UINT16(_huckGiftPic + 0x08);
		int16 y = READ_UINT16(_huckGiftPic + _huckState.selected1Slot * 2 + 0x3E) + READ_UINT16(_huckGiftPic + 0x0A);
		int16 hlMember = READ_UINT16(_huckGiftPic + 0x06);
		byte *hlPtr = memberAddr(_huckBoltLib, hlMember);
		displayPic(hlPtr, x, y, stFront);
	}

	if (_huckState.selectionPending) {
		int16 x = READ_UINT16(_huckGiftPic + _huckState.selected2Slot * 2 + 0x0E) + READ_UINT16(_huckGiftPic + 0x08);
		int16 y = READ_UINT16(_huckGiftPic + _huckState.selected2Slot * 2 + 0x3E) + READ_UINT16(_huckGiftPic + 0x0A);
		int16 hlMember = READ_UINT16(_huckGiftPic + 0x06);
		byte *hlPtr = memberAddr(_huckBoltLib, hlMember);
		displayPic(hlPtr, x, y, stFront);
	}
}

void BoltEngine::checkHuckLevelComplete() {
	if (_huckState.giftCount == 2) {
		// Last pair matched, level complete!
		_huckExitFlag = 1;
		_huckState.levelComplete = 1;
		_huckState.levelNumber++;

		if (_huckState.levelNumber > 10)
			_huckState.levelNumber = 10;

		_xp->fillDisplay(0, stFront);
	} else {
		// More pairs remain, redraw the matched slots as empty...
		drawGift(_huckState.selected1Slot);
		drawGift(_huckState.selected2Slot);
	}

	_huckState.selected1Slot = -1;
	_huckState.selected2Slot = -1;
	_huckState.giftCount -= 2;

	_xp->updateDisplay();

	waitSoundMapHuck();
	_huckScrollOffset++;
	startHuckShuffleTimer();
}

bool BoltEngine::initHuckDisplay() {
	byte *palPtr = memberAddr(_huckBoltLib, READ_UINT16(_huckBgPic));

	_huckHotSpotCount = 0;
	_huckScreensaverTimer = 0;
	_huckBlinkTimer = 0;
	_huckExitFlag = 0;

	int32 maxArea = 0;
	int16 giftCount = READ_UINT16(_huckGiftPic);
	for (int16 i = 0; i < giftCount; i++) {
		byte *spr = memberAddr(_huckBoltLib, _huckState.drawTable1[i]);
		int32 area = (int32)READ_UINT16(spr + 0x0A) * READ_UINT16(spr + 0x0C);
		if (area > maxArea)
			maxArea = area;
	}

	_huckScratchPic.pixelData = (byte *)_xp->allocMem(maxArea);
	if (!_huckScratchPic.pixelData)
		return false;

	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);
	displayColors(palPtr, stBack, 1);
	displayPic(_huckBgDisplayPic, _displayX, _displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(palPtr, stFront, 0);
	displayColors(palPtr, stBack, 1);
	displayPic(_huckBgDisplayPic, _displayX, _displayY, stBack);
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();

	drawHuckGifts();
	_xp->updateDisplay();

	if (_huckState.hasCycle) {
		XPCycleState cycleSpec[4];
		byte *cycleData = memberAddr(_huckBoltLib, READ_UINT16(_huckGiftPic + 0x0C));
		boltCycleToXPCycle(cycleData, cycleSpec);
		_xp->startCycle(cycleSpec);
	}

	_huckCursorY = 0x78;
	_huckCursorX = 0xC0;
	_xp->setCursorPos(0x78, 0xC0);
	_xp->setCursorColor(0xFF, 0xFF, 0xFF);
	_xp->showCursor();

	// Read palette ranges from gift pic data...
	_huckPalRange[0] = READ_UINT16(_huckGiftPic + 0x8C) + 0x80;
	_huckPalRange[1] = READ_UINT16(_huckGiftPic + 0x8E) + 0x80;
	_huckPalRange[2] = READ_UINT16(_huckGiftPic + 0x90) + 0x80;
	_huckPalRange[3] = READ_UINT16(_huckGiftPic + 0x92) + 0x80;
	_huckPalRange[4] = READ_UINT16(_huckGiftPic + 0x9E) + 0x80;
	_huckPalRange[5] = READ_UINT16(_huckGiftPic + 0xA0) + 0x80;
	_huckPalRange[6] = READ_UINT16(_huckGiftPic + 0xA2) + 0x80;
	_huckPalRange[7] = READ_UINT16(_huckGiftPic + 0xA4) + 0x80;

	// Save original palette ranges, load highlight palette ranges...
	_xp->getPalette(_huckPalRange[0], _huckPalRange[1] - _huckPalRange[0] + 1, _huckPalSave0);
	_xp->getPalette(_huckPalRange[2], _huckPalRange[3] - _huckPalRange[2] + 1, _huckPalHighlight0);
	_xp->getPalette(_huckPalRange[4], _huckPalRange[5] - _huckPalRange[4] + 1, _huckPalSave1);
	_xp->getPalette(_huckPalRange[6], _huckPalRange[7] - _huckPalRange[6] + 1, _huckPalHighlight1);

	return true;
}

bool BoltEngine::loadHuckResources() {
	int16 stateIdx = _huckState.levelNumber - 1;
	int16 giftGroupId = (stateIdx << 10) + 0x100;
	_huckGiftGroupId = giftGroupId;

	if (!getBOLTGroup(_huckBoltLib, giftGroupId, 1))
		return false;

	_huckGiftPic = memberAddr(_huckBoltLib, giftGroupId);

	for (int16 i = 0; i < 11; i++) {
		int16 *speed = (int16 *)(_huckGiftPic + 0x6E + i * 2);
		if (*speed < 30)
			*speed = 30;
	}

	// Load background display pic...
	int16 bgMember = (_displayMode != 0) ? READ_UINT16(_huckGiftPic + 4) : READ_UINT16(_huckGiftPic + 2);
	_huckBgDisplayPic = memberAddr(_huckBoltLib, bgMember);

	// Determine variant...
	int16 slot = _huckState.slotIndex[stateIdx];
	int16 variant = _huckGlobal[stateIdx * 3 + slot];
	int16 variantGroupId = ((stateIdx * 4 + variant + 1) << 8) + 0x100;
	_huckVariantGroupId = variantGroupId;

	if (!getBOLTGroup(_huckBoltLib, variantGroupId, 1))
		return false;

	_huckBgPic = memberAddr(_huckBoltLib, variantGroupId);

	return true;
}

void BoltEngine::unloadHuckResources() {
	_xp->stopCycle();
	_xp->hideCursor();
	freeBOLTGroup(_huckBoltLib, _huckVariantGroupId, 1);
	freeBOLTGroup(_huckBoltLib, _huckGiftGroupId, 1);
}

bool BoltEngine::initHuckLevel() {
	// Advance slot variant (wraps 0..2)
	int16 stateIdx = _huckState.levelNumber - 1;
	_huckState.slotIndex[stateIdx]++;
	if (_huckState.slotIndex[stateIdx] >= 3)
		_huckState.slotIndex[stateIdx] = 0;

	if (!loadHuckResources())
		return false;

	_huckState.giftCount = READ_UINT16(_huckGiftPic);
	_huckState.levelComplete = 0;
	_huckState.hasCycle = 0;
	_huckState.selectionPending = 0;
	_huckState.selected1Slot = -1;
	_huckState.selected2Slot = -1;

	for (int16 i = 0; i < 24; i++) {
		_huckState.drawTable1[i] = 0;
		_huckState.drawTable2[i] = 0;
	}

	int16 baseId = READ_UINT16(_huckBgPic + 2);

	// Distribute gift pairs into random empty slots...
	int16 count = 0;
	int16 giftCount = READ_UINT16(_huckGiftPic);
	while (count < giftCount) {
		int16 slot = _xp->getRandom(giftCount);
		while (_huckState.drawTable1[slot] > 0) {
			slot++;
			if (slot >= giftCount)
				slot = 0;
		}

		// Pairs share the same member ID
		_huckState.drawTable1[slot] = count / 2 + baseId;
		count++;
	}

	if (!initHuckDisplay())
		return false;

	_huckScrollOffset = 0;
	startHuckShuffleTimer();
	return true;
}

bool BoltEngine::resumeHuckLevel() {
	if (!loadHuckResources())
		return false;

	if (!initHuckDisplay())
		return false;

	if (_huckState.giftCount > 2) {
		_huckScrollOffset = (READ_UINT16(_huckGiftPic) - _huckState.giftCount) >> 1;
		startHuckShuffleTimer();
	}

	return true;
}

bool BoltEngine::initHuck() {
	_xp->randomize();
	_huckSoundPlaying = 0;

	if (!vLoad(&_huckGlobal, "HuckGlobal")) {
		// First run, generate random variant permutations for all 10 levels...
		for (int16 level = 0; level < 10; level++) {
			_huckState.slotIndex[level] = -1;

			// Fill row with sentinel value 3...
			for (int16 j = 0; j < 3; j++)
				_huckGlobal[level * 3 + j] = 3;

			// Place variants 0, 1, 2 into random empty slots...
			for (int16 variant = 0; variant < 3; variant++) {
				int16 slot = _xp->getRandom(3);
				while (_huckGlobal[level * 3 + slot] != 3) {
					slot++;
					if (slot >= 3)
						slot = 0;
				}
				_huckGlobal[level * 3 + slot] = variant;
			}
		}
	}

	byte huckStateBuf[0x86] = { 0 };

	if (!vLoad(&huckStateBuf, "Huck")) {
		_huckState.levelNumber = 1;
		return initHuckLevel();
	}

	Common::SeekableReadStream *huckStateReadStream = new Common::MemoryReadStream(huckStateBuf, sizeof(huckStateBuf), DisposeAfterUse::NO);

	_huckState.levelNumber = huckStateReadStream->readSint16BE();

	for (int i = 0; i < 10; i++)
		_huckState.slotIndex[i] = huckStateReadStream->readSint16BE();

	_huckState.levelComplete = huckStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		_huckState.drawTable1[i] = huckStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		_huckState.drawTable2[i] = huckStateReadStream->readSint16BE();

	_huckState.giftCount = huckStateReadStream->readSint16BE();
	_huckState.hasCycle = huckStateReadStream->readSint16BE();
	_huckState.selectionPending = huckStateReadStream->readSint16BE();
	_huckState.selected1Slot = huckStateReadStream->readSint16BE();
	_huckState.selected2Slot = huckStateReadStream->readSint16BE();
	_huckState.selected1SpriteId = huckStateReadStream->readSint16BE();
	_huckState.selected2SpriteId = huckStateReadStream->readSint16BE();

	// Sanity check: should be exactly 0x86
	assert(huckStateReadStream->pos() == 0x86);
	delete huckStateReadStream;

	if (_huckState.levelComplete)
		return initHuckLevel();
	else
		return resumeHuckLevel();
}

void BoltEngine::huckToggleBlinking(int16 *state, int16 which) {
	*state = (*state == 0) ? 1 : 0;
	if (*state)
		setHuckColors(which);
	else
		restoreHuckColors(which);
}

void BoltEngine::huckUpdateHotSpots(int16 x, int16 y) {
	Common::Rect helpRect(
		READ_UINT16(_huckGiftPic + 0x84), READ_UINT16(_huckGiftPic + 0x88),
		READ_UINT16(_huckGiftPic + 0x86), READ_UINT16(_huckGiftPic + 0x8A));

	Common::Rect exitRect(
		READ_UINT16(_huckGiftPic + 0x96), READ_UINT16(_huckGiftPic + 0x9A),
		READ_UINT16(_huckGiftPic + 0x98), READ_UINT16(_huckGiftPic + 0x9C));

	if (helpRect.contains(x, y)) {
		if (!_huckScreensaverTimer && !_huckHotSpotCount)
			setHuckColors(0);
	} else {
		if (!_huckScreensaverTimer && !_huckHotSpotCount)
			restoreHuckColors(0);
	}

	if (exitRect.contains(x, y)) {
		if (!_huckBlinkTimer)
			setHuckColors(1);

		return;
	} else {
		if (!_huckBlinkTimer)
			restoreHuckColors(1);
	}
}

int16 BoltEngine::findGift(int16 x, int16 y) {
	byte *sprite0 = memberAddr(_huckBoltLib, _huckState.drawTable1[0]);
	int16 sprH = READ_UINT16(sprite0 + 0x0A);
	int16 sprW = READ_UINT16(sprite0 + 0x0C);

	int16 giftCount = READ_UINT16(_huckGiftPic);

	// Iterate in reverse, topmost gift wins...
	for (int16 slot = giftCount - 1; slot >= 0; slot--) {
		if (_huckState.drawTable2[slot] != 0)
			continue;

		int16 slotY = READ_UINT16(_huckGiftPic + slot * 2 + 0x3E);
		int16 slotX = READ_UINT16(_huckGiftPic + slot * 2 + 0x0E);
		int16 slotY2 = slotY + sprH - 1;
		int16 slotX2 = slotX + sprW - 1;

		byte *sprite = memberAddr(_huckBoltLib, _huckState.drawTable1[slot]);

		if (y < slotY || y > slotY2)
			continue;
		if (x < slotX || x > slotX2)
			continue;

		// Non-transparent pixel hit test...
		if (getPixel(sprite, x - slotX, y - slotY) == 0)
			continue;

		return slot;
	}
	return -1;
}

bool BoltEngine::handleGiftSelect(int16 x, int16 y) {
	int16 slot = findGift(x, y);
	if (slot == -1)
		return false;

	if (!_huckState.hasCycle) {
		// No selection yet, select first gift...
		_huckState.hasCycle = 1;
		_huckState.selected1SpriteId = _huckState.drawTable1[slot];
		_huckState.selected1Slot = slot;
		playSoundMapHuck((int16)READ_UINT16(_huckGiftPic + 0xA8));
		drawGift(slot);
		_xp->updateDisplay();

		// Start palette cycle for highlight...
		XPCycleState cycleSpec[4];
		byte *cycleData = memberAddr(_huckBoltLib, (int16)READ_UINT16(_huckGiftPic + 0x0C));
		boltCycleToXPCycle(cycleData, cycleSpec);
		_xp->startCycle(cycleSpec);

	} else if (!_huckState.selectionPending) {
		if (_huckState.selected1Slot == slot) {
			// Same gift clicked again, deselect...
			_huckState.hasCycle = 0;
			_huckState.selected1Slot = -1;
			playSoundMapHuck((int16)READ_UINT16(_huckGiftPic + 0xAA));
			drawGift(slot);
			_xp->updateDisplay();
			_xp->stopCycle();
		} else {
			// Different gift, select second...
			_huckState.selectionPending = 1;
			_huckState.selected2SpriteId = _huckState.drawTable1[slot];
			_huckState.selected2Slot = slot;
			playSoundMapHuck((int16)READ_UINT16(_huckGiftPic + 0xAC));
			drawGift(slot);
			_xp->updateDisplay();
		}
	}

	// else: already have two selections pending, ignore click...

	waitSoundMapHuck();
	startHuckShuffleTimer();
	return true;
}

void BoltEngine::huckHandleActionButton(int16 x, int16 y) {
	bool var_6 = false;
	bool var_4 = false;

	if (_huckHotSpotCount != 0) {
		_huckHotSpotCount = 0;
		var_6 = true;
		stopAnimation();

		if (_huckBlinkTimer) {
			_xp->killTimer(_huckBlinkTimer);
			_huckBlinkTimer = 0;
		}

		huckUpdateHotSpots(_huckCursorX, _huckCursorY);
	}

	Common::Rect helpRect(
		READ_UINT16(_huckGiftPic + 0x84), READ_UINT16(_huckGiftPic + 0x88),
		READ_UINT16(_huckGiftPic + 0x86), READ_UINT16(_huckGiftPic + 0x8A));

	if (helpRect.contains(x, y)) {
		var_4 = true;
		if (_huckScreensaverTimer) {
			_xp->killTimer(_huckScreensaverTimer);
			_huckScreensaverTimer = 0;
			setHuckColors(0);
		}

		if (!_huckHotSpotCount && !var_6) {
			_huckHotSpotCount = startAnimation(_rtfHandle, 0x1A);
			_huckActionState = 0;
		}
	} else {
		Common::Rect exitRect(
			READ_UINT16(_huckGiftPic + 0x96), READ_UINT16(_huckGiftPic + 0x9A),
			READ_UINT16(_huckGiftPic + 0x98), READ_UINT16(_huckGiftPic + 0x9C));

		if (exitRect.contains(x, y)) {
			setHuckColors(1);
			restoreHuckColors(0);
			_huckReturnBooth = 3;
			_huckExitFlag = 1;
		}
	}

	if (handleGiftSelect(x, y))
		var_4 = true;

	if (var_4) {
		if (_huckScreensaverTimer) {
			_xp->killTimer(_huckScreensaverTimer);
			_huckScreensaverTimer = 0;
			restoreHuckColors(0);
		}

		_xp->setInactivityTimer(30);
	}
}

void BoltEngine::giftSwap() {
	if (_huckState.giftCount <= 2)
		return;

	// Determine pool size excluding selected gifts...
	int16 pool;
	if (_huckState.selectionPending)
		pool = _huckState.giftCount - 2;
	else if (_huckState.hasCycle)
		pool = _huckState.giftCount - 1;
	else
		pool = _huckState.giftCount;

	// Pick two distinct random indices into the eligible pool...
	int16 randA = _xp->getRandom(pool);
	int16 randB = _xp->getRandom(pool);
	if (randA == randB) {
		randB++;
		if (randB >= pool)
			randB = 0;
	}

	// Map random indices to actual slots, skipping matched/selected...
	int16 slotA = -1;
	int16 slotB = 0;
	int16 counter = -1;
	for (int16 i = 0, j = 0; j <= READ_UINT16(_huckGiftPic); i++, j++) {
		if (_huckState.drawTable2[j] != 0)
			continue;

		if (j == _huckState.selected1Slot)
			continue;

		if (j == _huckState.selected2Slot)
			continue;

		counter++;

		if (counter == randA)
			slotA = j;

		if (counter == randB)
			slotB = j;
	}

	// If the two chosen slots have the same sprite, find a different slotB...
	if (_huckState.drawTable1[slotA] == _huckState.drawTable1[slotB]) {
		// Walk forward from slotB, skipping invalid candidates...
		while (true) {
			slotB++;
			if (slotB >= READ_UINT16(_huckGiftPic))
				slotB = 0;

			if (_huckState.drawTable2[slotB] != 0)
				continue;

			if (slotB == _huckState.selected2Slot)
				continue;

			if (slotB == _huckState.selected1Slot)
				continue;

			if (slotB == slotA)
				continue;

			break;
		}
	}

	// Swap entries in draw table...
	int16 tmp = _huckState.drawTable1[slotA];
	_huckState.drawTable1[slotA] = _huckState.drawTable1[slotB];
	_huckState.drawTable1[slotB] = tmp;

	drawGift(slotA);
	drawGift(slotB);
}

void BoltEngine::resolveHuckSelection() {
	if (!_huckState.selectionPending)
		return;

	_xp->stopCycle();

	if (_huckState.selected1SpriteId == _huckState.selected2SpriteId) {
		// Match, play match sound, mark both slots as matched...
		playSoundMapHuck((int16)READ_UINT16(_huckGiftPic + 0xAE));
		_huckState.drawTable2[_huckState.selected1Slot] = 1;
		_huckState.drawTable2[_huckState.selected2Slot] = 1;
		_huckState.hasCycle = 0;
		_huckState.selectionPending = 0;
		checkHuckLevelComplete();
	} else {
		// Mismatch, play mismatch sound, redraw both gifts unselected...
		playSoundMapHuck((int16)READ_UINT16(_huckGiftPic + 0xB0));
		_huckState.hasCycle = 0;
		_huckState.selectionPending = 0;
		drawGift(_huckState.selected1Slot);
		drawGift(_huckState.selected2Slot);
		_huckState.selected1Slot = -1;
		_huckState.selected2Slot = -1;
		waitSoundMapHuck();
	}
}

void BoltEngine::handleEvent(int16 eventType, uint32 eventData) {
	switch (eventType) {
	case etMouseDown:
		huckHandleActionButton(_huckCursorX, _huckCursorY);
		resolveHuckSelection();

		// The original does:
		// if (_huckExitFlag)
		// 	return;
		//
		// and then breaks, which is redundant...

		break;

	case etMouseMove:
		_huckCursorX = (int16)(eventData >> 16);
		_huckCursorY = (int16)(eventData & -1);
		huckUpdateHotSpots(_huckCursorX, _huckCursorY);
		break;

	case etTimer: {
		if (eventData == _huckScreensaverTimer) {
			huckToggleBlinking(&_huckScreensaverFlag, 0);
			int16 ms = (int16)((int32)READ_UINT16(_huckGiftPic + 0x94) * 1000 / 60);
			_huckScreensaverTimer = _xp->startTimer(ms);
		} else if (eventData == _huckBlinkTimer) {
			huckToggleBlinking(&_huckBlinkFlag, 1);
			int16 ms = (int16)((int32)READ_UINT16(_huckGiftPic + 0x94) * 1000 / 60);
			_huckBlinkTimer = _xp->startTimer(ms);
		} else if (eventData == _huckShuffleTimer) {
			giftSwap();
			_xp->updateDisplay();
			startHuckShuffleTimer();
		}

		break;
	}

	case etInactivity: {
		if (!_huckScreensaverTimer) {
			int16 ms = (int16)((int32)READ_UINT16(_huckGiftPic + 0x94) * 1000 / 60);
			_huckScreensaverTimer = _xp->startTimer(ms);
			_huckScreensaverFlag = 0;
			huckToggleBlinking(&_huckScreensaverFlag, 0);
			_xp->setInactivityTimer(1800);
		} else {
			bool exitLoop = false;
			_xp->setScreenBrightness(25);

			while (!shouldQuit()) {
				uint32 innerData = 0;
				int16 innerType = _xp->getEvent(etEmpty, &innerData);
				switch (innerType) {
				case etSound:
					if (_huckSoundPlaying > 0)
						_huckSoundPlaying--;

					break;
				default:
					_xp->setScreenBrightness(100);
					_xp->setInactivityTimer(1800);
					exitLoop = true;
					break;
				}

				handleEvent(innerType, innerData);
				if (exitLoop)
					break;
			}
		}

		break;
	}

	case etSound: {
		if (_huckHotSpotCount != 0) {
			_huckHotSpotCount = maintainAudioPlay(1);
			if (_huckHotSpotCount == 0)
				huckUpdateHotSpots(_huckCursorX, _huckCursorY);
		}

		if (_huckSoundPlaying > 0)
			_huckSoundPlaying--;

		break;
	}

	case etTrigger: {
		_huckActionState++;
		if (_huckActionState == 1) {
			int16 ms = (int16)((int32)READ_UINT16(_huckGiftPic + 0x94) * 1000 / 60);
			_huckBlinkTimer = _xp->startTimer(ms);
			_huckBlinkFlag = 0;
			huckToggleBlinking(&_huckBlinkFlag, 1);
		} else if (_huckActionState == 2) {
			_xp->killTimer(_huckBlinkTimer);
			_huckBlinkTimer = 0;
			restoreHuckColors(1);
		}

		break;
	}

	default:
		break;
	}
}

void BoltEngine::playHuck() {
	_huckReturnBooth = 0x10;

	while (!shouldQuit()) {
		if (_huckExitFlag)
			break;

		uint32 eventData = 0;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		if (eventType)
			handleEvent(eventType, eventData);

		if (_huckHotSpotCount != 0) {
			_huckHotSpotCount = maintainAudioPlay(0);
			if (_huckHotSpotCount == 0)
				huckUpdateHotSpots(_huckCursorX, _huckCursorY);
		}
	}

	if (shouldQuit())
		_huckReturnBooth = 0;
}

void BoltEngine::cleanUpHuck() {
	unloadHuckResources();

	if (_huckScratchPic.pixelData) {
		_xp->freeMem(_huckScratchPic.pixelData);
		_huckScratchPic.pixelData = nullptr;
		_huckScratchPic.palette = nullptr;
	}

	vSave(&_huckGlobal, sizeof(_huckGlobal), "HuckGlobal");

	byte huckSaveBuf[0x86] = { 0 };
	Common::MemoryWriteStream *huckStateWriteStream = new Common::MemoryWriteStream(huckSaveBuf, sizeof(huckSaveBuf));

	huckStateWriteStream->writeSint16BE(_huckState.levelNumber);

	for (int i = 0; i < 10; i++)
		huckStateWriteStream->writeSint16BE(_huckState.slotIndex[i]);

	huckStateWriteStream->writeSint16BE(_huckState.levelComplete);

	for (int i = 0; i < 24; i++)
		huckStateWriteStream->writeSint16BE(_huckState.drawTable1[i]);

	for (int i = 0; i < 24; i++)
		huckStateWriteStream->writeSint16BE(_huckState.drawTable2[i]);

	huckStateWriteStream->writeSint16BE(_huckState.giftCount);
	huckStateWriteStream->writeSint16BE(_huckState.hasCycle);
	huckStateWriteStream->writeSint16BE(_huckState.selectionPending);
	huckStateWriteStream->writeSint16BE(_huckState.selected1Slot);
	huckStateWriteStream->writeSint16BE(_huckState.selected2Slot);
	huckStateWriteStream->writeSint16BE(_huckState.selected1SpriteId);
	huckStateWriteStream->writeSint16BE(_huckState.selected2SpriteId);

	// Sanity check: should be exactly 0x86
	assert(huckStateWriteStream->pos() == 0x86);
	delete huckStateWriteStream;

	vSave(&huckSaveBuf, 0x86, "Huck");

	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

int16 BoltEngine::huckGame(int16 prevBooth) {
	if (!openBOLTLib(&_huckBoltLib, &_huckBoltCallbacks, assetPath("huck.blt")))
		return _huckReturnBooth;

	int16 savedTimer = _xp->setInactivityTimer(30);

	if (initHuck())
		playHuck();

	cleanUpHuck();

	_xp->setInactivityTimer(savedTimer);
	closeBOLTLib(&_huckBoltLib);

	return _huckReturnBooth;
}

void BoltEngine::swapHuckWordArray() {
	byte *ptr = _boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	int16 count = (int16)(_boltCurrentMemberEntry->decompSize / 2);
	for (int16 i = 0; i < count; i++, ptr += 2)
		WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
}

void BoltEngine::swapHuckWords() {
	byte *ptr = _boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
	WRITE_UINT16(ptr + 2, READ_BE_UINT16(ptr + 2));
}

} // End of namespace Bolt
