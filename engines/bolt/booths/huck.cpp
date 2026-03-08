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
	byte *soundData = getBOLTMember(g_huckBoltLib, memberId);
	uint32 soundSize = memberSize(g_huckBoltLib, memberId);
	if (soundData) {
		_xp->playSound(soundData, soundSize, 22050);
		g_huckSoundPlaying++;
	}
}

void BoltEngine::waitSoundMapHuck() {
	if (!g_huckSoundPlaying)
		return;

	uint32 dummy;
	while (_xp->getEvent(etSound, &dummy) == etEmpty);
	g_huckSoundPlaying--;
}

void BoltEngine::setHuckColors(int16 which) {
	if (which == 0) {
		int16 count = g_huckPalRange[1] - g_huckPalRange[0] + 1;
		_xp->setPalette(count, g_huckPalRange[0], g_huckPalHighlight0);
	} else if (which == 1) {
		int16 count = g_huckPalRange[5] - g_huckPalRange[4] + 1;
		_xp->setPalette(count, g_huckPalRange[4], g_huckPalHighlight1);
	}
}

void BoltEngine::restoreHuckColors(int16 which) {
	if (which == 0) {
		int16 count = g_huckPalRange[1] - g_huckPalRange[0] + 1;
		_xp->setPalette(count, g_huckPalRange[0], g_huckPalSave0);
	} else if (which == 1) {
		int16 count = g_huckPalRange[5] - g_huckPalRange[4] + 1;
		_xp->setPalette(count, g_huckPalRange[4], g_huckPalSave1);
	}
}

void BoltEngine::startHuckShuffleTimer() {
	if (g_huckShuffleTimer) {
		_xp->killTimer(g_huckShuffleTimer);
		g_huckShuffleTimer = 0;
	}

	if (g_huckState.giftCount > 2) {
		int16 speed = READ_UINT16(g_huckGiftPic + g_huckScrollOffset * 2 + 0x6E);
		int32 ms = (int32)speed * 1000 / 60;
		g_huckShuffleTimer = _xp->startTimer((int16)ms);
	}
}

bool BoltEngine::intersectRect(const Common::Rect *a, const Common::Rect *b, Common::Rect *out) {
	*out = a->findIntersectingRect(*b);
	return !out->isEmpty();
}

void BoltEngine::drawGift(int16 slot) {
	byte *giftSprite = memberAddr(g_huckBoltLib, g_huckState.drawTable1[slot]);
	int16 sprStride = READ_UINT16(giftSprite + 0x0A);
	int16 sprHeight = READ_UINT16(giftSprite + 0x0C);

	memset(g_huckScratchPic.pixelData, 0, (int32)sprStride * sprHeight);

	int16 slotX = READ_UINT16(g_huckGiftPic + slot * 2 + 0x3E);
	int16 slotY = READ_UINT16(g_huckGiftPic + slot * 2 + 0x0E);
	Common::Rect targetRect(slotX, slotY, slotX + sprHeight, slotY + sprStride);

	int16 giftCount = READ_UINT16(g_huckGiftPic);
	for (int16 i = 0; i < giftCount; i++) {
		if (g_huckState.drawTable2[i] != 0)
			continue;

		byte *otherSprite = memberAddr(g_huckBoltLib, g_huckState.drawTable1[i]);
		int16 otherStride = READ_UINT16(otherSprite + 0x0A);
		int16 otherHeight = READ_UINT16(otherSprite + 0x0C);
		int16 ox = READ_UINT16(g_huckGiftPic + i * 2 + 0x3E);
		int16 oy = READ_UINT16(g_huckGiftPic + i * 2 + 0x0E);
		Common::Rect otherRect(ox, oy, ox + otherHeight, oy + otherStride);

		Common::Rect isect;
		if (!intersectRect(&targetRect, &otherRect, &isect))
			continue;

		byte *srcPixels = getResolvedPtr(otherSprite, 0x12);
		byte *src = srcPixels + (isect.left - ox) * otherStride + (isect.top - oy);

		byte *dst = g_huckScratchPic.pixelData + (isect.left - slotX) * sprStride + (isect.top - slotY);

		int16 blitWidth = isect.bottom - isect.top;
		int16 blitHeight = isect.right - isect.left;
		_xp->maskBlit(src, otherStride, dst, sprStride, blitWidth, blitHeight);
	}

	if ((g_huckState.hasCycle && g_huckState.selected1Slot == slot) || (g_huckState.selectionPending && g_huckState.selected2Slot == slot)) {
		byte *hlSprite = memberAddr(g_huckBoltLib, READ_UINT16(g_huckGiftPic + 0x06));
		int16 hlStride = READ_UINT16(hlSprite + 0x0A);
		int16 hlHeight = READ_UINT16(hlSprite + 0x0C);
		byte *hlPixels = getResolvedPtr(hlSprite, 0x12);
		int16 hlOffsetX = READ_UINT16(g_huckGiftPic + 0x0A);
		int16 hlOffsetY = READ_UINT16(g_huckGiftPic + 0x08);

		byte *dst = g_huckScratchPic.pixelData + hlOffsetX * sprStride + hlOffsetY;

		_xp->maskBlit(hlPixels, hlStride, dst, sprStride, hlStride, hlHeight);
	}

	g_huckScratchPic.width = sprStride;
	g_huckScratchPic.height = sprHeight;
	g_huckScratchPic.palette = nullptr;
	g_huckScratchPic.flags = 0;

	_xp->displayPic(&g_huckScratchPic, slotY, slotX, stFront);
}

void BoltEngine::drawHuckGifts() {
	_xp->fillDisplay(0, stFront);

	int16 giftCount = READ_UINT16(g_huckGiftPic);

	for (int16 slot = 0; slot < giftCount; slot++) {
		if (g_huckState.drawTable2[slot] != 0)
			continue;

		byte *gifPtr = memberAddr(g_huckBoltLib, g_huckState.drawTable1[slot]);
		int16 x = READ_UINT16(g_huckGiftPic + slot * 2 + 0x0E);
		int16 y = READ_UINT16(g_huckGiftPic + slot * 2 + 0x3E);
		displayPic(gifPtr, x, y, stFront);
	}

	if (g_huckState.hasCycle) {
		int16 x = READ_UINT16(g_huckGiftPic + g_huckState.selected1Slot * 2 + 0x0E) + READ_UINT16(g_huckGiftPic + 0x08);
		int16 y = READ_UINT16(g_huckGiftPic + g_huckState.selected1Slot * 2 + 0x3E) + READ_UINT16(g_huckGiftPic + 0x0A);
		int16 hlMember = READ_UINT16(g_huckGiftPic + 0x06);
		byte *hlPtr = memberAddr(g_huckBoltLib, hlMember);
		displayPic(hlPtr, x, y, stFront);
	}

	if (g_huckState.selectionPending) {
		int16 x = READ_UINT16(g_huckGiftPic + g_huckState.selected2Slot * 2 + 0x0E) + READ_UINT16(g_huckGiftPic + 0x08);
		int16 y = READ_UINT16(g_huckGiftPic + g_huckState.selected2Slot * 2 + 0x3E) + READ_UINT16(g_huckGiftPic + 0x0A);
		int16 hlMember = READ_UINT16(g_huckGiftPic + 0x06);
		byte *hlPtr = memberAddr(g_huckBoltLib, hlMember);
		displayPic(hlPtr, x, y, stFront);
	}
}

void BoltEngine::checkHuckLevelComplete() {
	if (g_huckState.giftCount == 2) {
		// Last pair matched, level complete!
		g_huckExitFlag = 1;
		g_huckState.levelComplete = 1;
		g_huckState.levelNumber++;

		if (g_huckState.levelNumber > 10)
			g_huckState.levelNumber = 10;

		_xp->fillDisplay(0, stFront);
	} else {
		// More pairs remain, redraw the matched slots as empty...
		drawGift(g_huckState.selected1Slot);
		drawGift(g_huckState.selected2Slot);
	}

	g_huckState.selected1Slot = -1;
	g_huckState.selected2Slot = -1;
	g_huckState.giftCount -= 2;

	_xp->updateDisplay();

	waitSoundMapHuck();
	g_huckScrollOffset++;
	startHuckShuffleTimer();
}

bool BoltEngine::initHuckDisplay() {
	byte *palPtr = memberAddr(g_huckBoltLib, READ_UINT16(g_huckBgPic));

	g_huckHotSpotCount = 0;
	g_huckScreensaverTimer = 0;
	g_huckBlinkTimer = 0;
	g_huckExitFlag = 0;

	int32 maxArea = 0;
	int16 giftCount = READ_UINT16(g_huckGiftPic);
	for (int16 i = 0; i < giftCount; i++) {
		byte *spr = memberAddr(g_huckBoltLib, g_huckState.drawTable1[i]);
		int32 area = (int32)READ_UINT16(spr + 0x0A) * READ_UINT16(spr + 0x0C);
		if (area > maxArea)
			maxArea = area;
	}

	g_huckScratchPic.pixelData = (byte *)_xp->allocMem(maxArea);
	if (!g_huckScratchPic.pixelData)
		return false;

	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);
	displayColors(palPtr, stBack, 1);
	displayPic(g_huckBgDisplayPic, g_displayX, g_displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(palPtr, stFront, 0);
	displayColors(palPtr, stBack, 1);
	displayPic(g_huckBgDisplayPic, g_displayX, g_displayY, stBack);
	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();

	drawHuckGifts();
	_xp->updateDisplay();

	if (g_huckState.hasCycle) {
		XPCycleState cycleSpec[4];
		byte *cycleData = memberAddr(g_huckBoltLib, READ_UINT16(g_huckGiftPic + 0x0C));
		boltCycleToXPCycle(cycleData, cycleSpec);
		_xp->startCycle(cycleSpec);
	}

	g_huckCursorY = 0x78;
	g_huckCursorX = 0xC0;
	_xp->setCursorPos(0x78, 0xC0);
	_xp->setCursorColor(0xFF, 0xFF, 0xFF);
	_xp->showCursor();

	// Read palette ranges from gift pic data...
	g_huckPalRange[0] = READ_UINT16(g_huckGiftPic + 0x8C) + 0x80;
	g_huckPalRange[1] = READ_UINT16(g_huckGiftPic + 0x8E) + 0x80;
	g_huckPalRange[2] = READ_UINT16(g_huckGiftPic + 0x90) + 0x80;
	g_huckPalRange[3] = READ_UINT16(g_huckGiftPic + 0x92) + 0x80;
	g_huckPalRange[4] = READ_UINT16(g_huckGiftPic + 0x9E) + 0x80;
	g_huckPalRange[5] = READ_UINT16(g_huckGiftPic + 0xA0) + 0x80;
	g_huckPalRange[6] = READ_UINT16(g_huckGiftPic + 0xA2) + 0x80;
	g_huckPalRange[7] = READ_UINT16(g_huckGiftPic + 0xA4) + 0x80;

	// Save original palette ranges, load highlight palette ranges...
	_xp->getPalette(g_huckPalRange[0], g_huckPalRange[1] - g_huckPalRange[0] + 1, g_huckPalSave0);
	_xp->getPalette(g_huckPalRange[2], g_huckPalRange[3] - g_huckPalRange[2] + 1, g_huckPalHighlight0);
	_xp->getPalette(g_huckPalRange[4], g_huckPalRange[5] - g_huckPalRange[4] + 1, g_huckPalSave1);
	_xp->getPalette(g_huckPalRange[6], g_huckPalRange[7] - g_huckPalRange[6] + 1, g_huckPalHighlight1);

	return true;
}

bool BoltEngine::loadHuckResources() {
	int16 stateIdx = g_huckState.levelNumber - 1;
	int16 giftGroupId = (stateIdx << 10) + 0x100;
	g_huckGiftGroupId = giftGroupId;

	if (!getBOLTGroup(g_huckBoltLib, giftGroupId, 1))
		return false;

	g_huckGiftPic = memberAddr(g_huckBoltLib, giftGroupId);

	for (int16 i = 0; i < 11; i++) {
		int16 *speed = (int16 *)(g_huckGiftPic + 0x6E + i * 2);
		if (*speed < 30)
			*speed = 30;
	}

	// Load background display pic...
	int16 bgMember = (g_displayMode != 0) ? READ_UINT16(g_huckGiftPic + 4) : READ_UINT16(g_huckGiftPic + 2);
	g_huckBgDisplayPic = memberAddr(g_huckBoltLib, bgMember);

	// Determine variant...
	int16 slot = g_huckState.slotIndex[stateIdx];
	int16 variant = g_huckGlobal[stateIdx * 3 + slot];
	int16 variantGroupId = ((stateIdx * 4 + variant + 1) << 8) + 0x100;
	g_huckVariantGroupId = variantGroupId;

	if (!getBOLTGroup(g_huckBoltLib, variantGroupId, 1))
		return false;

	g_huckBgPic = memberAddr(g_huckBoltLib, variantGroupId);

	return true;
}

void BoltEngine::unloadHuckResources() {
	_xp->stopCycle();
	_xp->hideCursor();
	freeBOLTGroup(g_huckBoltLib, g_huckVariantGroupId, 1);
	freeBOLTGroup(g_huckBoltLib, g_huckGiftGroupId, 1);
}

bool BoltEngine::initHuckLevel() {
	// Advance slot variant (wraps 0..2)
	int16 stateIdx = g_huckState.levelNumber - 1;
	g_huckState.slotIndex[stateIdx]++;
	if (g_huckState.slotIndex[stateIdx] >= 3)
		g_huckState.slotIndex[stateIdx] = 0;

	if (!loadHuckResources())
		return false;

	g_huckState.giftCount = READ_UINT16(g_huckGiftPic);
	g_huckState.levelComplete = 0;
	g_huckState.hasCycle = 0;
	g_huckState.selectionPending = 0;
	g_huckState.selected1Slot = -1;
	g_huckState.selected2Slot = -1;

	for (int16 i = 0; i < 24; i++) {
		g_huckState.drawTable1[i] = 0;
		g_huckState.drawTable2[i] = 0;
	}

	int16 baseId = READ_UINT16(g_huckBgPic + 2);

	// Distribute gift pairs into random empty slots...
	int16 count = 0;
	int16 giftCount = READ_UINT16(g_huckGiftPic);
	while (count < giftCount) {
		int16 slot = _xp->getRandom(giftCount);
		while (g_huckState.drawTable1[slot] > 0) {
			slot++;
			if (slot >= giftCount)
				slot = 0;
		}

		// Pairs share the same member ID
		g_huckState.drawTable1[slot] = count / 2 + baseId;
		count++;
	}

	if (!initHuckDisplay())
		return false;

	g_huckScrollOffset = 0;
	startHuckShuffleTimer();
	return true;
}

bool BoltEngine::resumeHuckLevel() {
	if (!loadHuckResources())
		return false;

	if (!initHuckDisplay())
		return false;

	if (g_huckState.giftCount > 2) {
		g_huckScrollOffset = (READ_UINT16(g_huckGiftPic) - g_huckState.giftCount) >> 1;
		startHuckShuffleTimer();
	}

	return true;
}

bool BoltEngine::initHuck() {
	_xp->randomize();
	g_huckSoundPlaying = 0;

	if (!vLoad(&g_huckGlobal, "HuckGlobal")) {
		// First run, generate random variant permutations for all 10 levels...
		for (int16 level = 0; level < 10; level++) {
			g_huckState.slotIndex[level] = -1;

			// Fill row with sentinel value 3...
			for (int16 j = 0; j < 3; j++)
				g_huckGlobal[level * 3 + j] = 3;

			// Place variants 0, 1, 2 into random empty slots...
			for (int16 variant = 0; variant < 3; variant++) {
				int16 slot = _xp->getRandom(3);
				while (g_huckGlobal[level * 3 + slot] != 3) {
					slot++;
					if (slot >= 3)
						slot = 0;
				}
				g_huckGlobal[level * 3 + slot] = variant;
			}
		}
	}

	byte huckStateBuf[0x86] = { 0 };

	if (!vLoad(&huckStateBuf, "Huck")) {
		g_huckState.levelNumber = 1;
		return initHuckLevel();
	}

	Common::SeekableReadStream *huckStateReadStream = new Common::MemoryReadStream(huckStateBuf, sizeof(huckStateBuf), DisposeAfterUse::NO);

	g_huckState.levelNumber = huckStateReadStream->readSint16BE();

	for (int i = 0; i < 10; i++)
		g_huckState.slotIndex[i] = huckStateReadStream->readSint16BE();

	g_huckState.levelComplete = huckStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		g_huckState.drawTable1[i] = huckStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		g_huckState.drawTable2[i] = huckStateReadStream->readSint16BE();

	g_huckState.giftCount = huckStateReadStream->readSint16BE();
	g_huckState.hasCycle = huckStateReadStream->readSint16BE();
	g_huckState.selectionPending = huckStateReadStream->readSint16BE();
	g_huckState.selected1Slot = huckStateReadStream->readSint16BE();
	g_huckState.selected2Slot = huckStateReadStream->readSint16BE();
	g_huckState.selected1SpriteId = huckStateReadStream->readSint16BE();
	g_huckState.selected2SpriteId = huckStateReadStream->readSint16BE();

	// Sanity check: should be exactly 0x86
	assert(huckStateReadStream->pos() == 0x86);
	delete huckStateReadStream;

	if (g_huckState.levelComplete)
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
		READ_UINT16(g_huckGiftPic + 0x84), READ_UINT16(g_huckGiftPic + 0x88),
		READ_UINT16(g_huckGiftPic + 0x86), READ_UINT16(g_huckGiftPic + 0x8A));

	Common::Rect exitRect(
		READ_UINT16(g_huckGiftPic + 0x96), READ_UINT16(g_huckGiftPic + 0x9A),
		READ_UINT16(g_huckGiftPic + 0x98), READ_UINT16(g_huckGiftPic + 0x9C));

	if (helpRect.contains(x, y)) {
		if (!g_huckScreensaverTimer && !g_huckHotSpotCount)
			setHuckColors(0);
	} else {
		if (!g_huckScreensaverTimer && !g_huckHotSpotCount)
			restoreHuckColors(0);
	}

	if (exitRect.contains(x, y)) {
		if (!g_huckBlinkTimer)
			setHuckColors(1);

		return;
	} else {
		if (!g_huckBlinkTimer)
			restoreHuckColors(1);
	}
}

int16 BoltEngine::findGift(int16 x, int16 y) {
	byte *sprite0 = memberAddr(g_huckBoltLib, g_huckState.drawTable1[0]);
	int16 sprH = READ_UINT16(sprite0 + 0x0A);
	int16 sprW = READ_UINT16(sprite0 + 0x0C);

	int16 giftCount = READ_UINT16(g_huckGiftPic);

	// Iterate in reverse, topmost gift wins...
	for (int16 slot = giftCount - 1; slot >= 0; slot--) {
		if (g_huckState.drawTable2[slot] != 0)
			continue;

		int16 slotY = READ_UINT16(g_huckGiftPic + slot * 2 + 0x3E);
		int16 slotX = READ_UINT16(g_huckGiftPic + slot * 2 + 0x0E);
		int16 slotY2 = slotY + sprH - 1;
		int16 slotX2 = slotX + sprW - 1;

		byte *sprite = memberAddr(g_huckBoltLib, g_huckState.drawTable1[slot]);

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

	if (!g_huckState.hasCycle) {
		// No selection yet, select first gift...
		g_huckState.hasCycle = 1;
		g_huckState.selected1SpriteId = g_huckState.drawTable1[slot];
		g_huckState.selected1Slot = slot;
		playSoundMapHuck((int16)READ_UINT16(g_huckGiftPic + 0xA8));
		drawGift(slot);
		_xp->updateDisplay();

		// Start palette cycle for highlight...
		XPCycleState cycleSpec[4];
		byte *cycleData = memberAddr(g_huckBoltLib, (int16)READ_UINT16(g_huckGiftPic + 0x0C));
		boltCycleToXPCycle(cycleData, cycleSpec);
		_xp->startCycle(cycleSpec);

	} else if (!g_huckState.selectionPending) {
		if (g_huckState.selected1Slot == slot) {
			// Same gift clicked again, deselect...
			g_huckState.hasCycle = 0;
			g_huckState.selected1Slot = -1;
			playSoundMapHuck((int16)READ_UINT16(g_huckGiftPic + 0xAA));
			drawGift(slot);
			_xp->updateDisplay();
			_xp->stopCycle();
		} else {
			// Different gift, select second...
			g_huckState.selectionPending = 1;
			g_huckState.selected2SpriteId = g_huckState.drawTable1[slot];
			g_huckState.selected2Slot = slot;
			playSoundMapHuck((int16)READ_UINT16(g_huckGiftPic + 0xAC));
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

	if (g_huckHotSpotCount != 0) {
		g_huckHotSpotCount = 0;
		var_6 = true;
		stopAnimation();

		if (g_huckBlinkTimer) {
			_xp->killTimer(g_huckBlinkTimer);
			g_huckBlinkTimer = 0;
		}

		huckUpdateHotSpots(g_huckCursorX, g_huckCursorY);
	}

	Common::Rect helpRect(
		READ_UINT16(g_huckGiftPic + 0x84), READ_UINT16(g_huckGiftPic + 0x88),
		READ_UINT16(g_huckGiftPic + 0x86), READ_UINT16(g_huckGiftPic + 0x8A));

	if (helpRect.contains(x, y)) {
		var_4 = true;
		if (g_huckScreensaverTimer) {
			_xp->killTimer(g_huckScreensaverTimer);
			g_huckScreensaverTimer = 0;
			setHuckColors(0);
		}

		if (!g_huckHotSpotCount && !var_6) {
			g_huckHotSpotCount = startAnimation(g_rtfHandle, 0x1A);
			g_huckActionState = 0;
		}
	} else {
		Common::Rect exitRect(
			READ_UINT16(g_huckGiftPic + 0x96), READ_UINT16(g_huckGiftPic + 0x9A),
			READ_UINT16(g_huckGiftPic + 0x98), READ_UINT16(g_huckGiftPic + 0x9C));

		if (exitRect.contains(x, y)) {
			setHuckColors(1);
			restoreHuckColors(0);
			g_returnBooth = 3;
			g_huckExitFlag = 1;
		}
	}

	if (handleGiftSelect(x, y))
		var_4 = true;

	if (var_4) {
		if (g_huckScreensaverTimer) {
			_xp->killTimer(g_huckScreensaverTimer);
			g_huckScreensaverTimer = 0;
			restoreHuckColors(0);
		}

		_xp->setInactivityTimer(30);
	}
}

void BoltEngine::giftSwap() {
	if (g_huckState.giftCount <= 2)
		return;

	// Determine pool size excluding selected gifts...
	int16 pool;
	if (g_huckState.selectionPending)
		pool = g_huckState.giftCount - 2;
	else if (g_huckState.hasCycle)
		pool = g_huckState.giftCount - 1;
	else
		pool = g_huckState.giftCount;

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
	for (int16 i = 0, j = 0; j <= READ_UINT16(g_huckGiftPic); i++, j++) {
		if (g_huckState.drawTable2[j] != 0)
			continue;

		if (j == g_huckState.selected1Slot)
			continue;

		if (j == g_huckState.selected2Slot)
			continue;

		counter++;

		if (counter == randA)
			slotA = j;

		if (counter == randB)
			slotB = j;
	}

	// If the two chosen slots have the same sprite, find a different slotB...
	if (g_huckState.drawTable1[slotA] == g_huckState.drawTable1[slotB]) {
		// Walk forward from slotB, skipping invalid candidates...
		while (true) {
			slotB++;
			if (slotB >= READ_UINT16(g_huckGiftPic))
				slotB = 0;

			if (g_huckState.drawTable2[slotB] != 0)
				continue;

			if (slotB == g_huckState.selected2Slot)
				continue;

			if (slotB == g_huckState.selected1Slot)
				continue;

			if (slotB == slotA)
				continue;

			break;
		}
	}

	// Swap entries in draw table...
	int16 tmp = g_huckState.drawTable1[slotA];
	g_huckState.drawTable1[slotA] = g_huckState.drawTable1[slotB];
	g_huckState.drawTable1[slotB] = tmp;

	drawGift(slotA);
	drawGift(slotB);
}

void BoltEngine::resolveHuckSelection() {
	if (!g_huckState.selectionPending)
		return;

	_xp->stopCycle();

	if (g_huckState.selected1SpriteId == g_huckState.selected2SpriteId) {
		// Match, play match sound, mark both slots as matched...
		playSoundMapHuck((int16)READ_UINT16(g_huckGiftPic + 0xAE));
		g_huckState.drawTable2[g_huckState.selected1Slot] = 1;
		g_huckState.drawTable2[g_huckState.selected2Slot] = 1;
		g_huckState.hasCycle = 0;
		g_huckState.selectionPending = 0;
		checkHuckLevelComplete();
	} else {
		// Mismatch, play mismatch sound, redraw both gifts unselected...
		playSoundMapHuck((int16)READ_UINT16(g_huckGiftPic + 0xB0));
		g_huckState.hasCycle = 0;
		g_huckState.selectionPending = 0;
		drawGift(g_huckState.selected1Slot);
		drawGift(g_huckState.selected2Slot);
		g_huckState.selected1Slot = -1;
		g_huckState.selected2Slot = -1;
		waitSoundMapHuck();
	}
}

void BoltEngine::handleEvent(int16 eventType, uint32 eventData) {
	switch (eventType) {
	case etMouseDown:
		huckHandleActionButton(g_huckCursorX, g_huckCursorY);
		resolveHuckSelection();

		// The original does:
		// if (g_huckExitFlag)
		// 	return;
		//
		// and then breaks, which is redundant...

		break;

	case etMouseMove:
		g_huckCursorX = (int16)(eventData >> 16);
		g_huckCursorY = (int16)(eventData & -1);
		huckUpdateHotSpots(g_huckCursorX, g_huckCursorY);
		break;

	case etTimer: {
		if (eventData == g_huckScreensaverTimer) {
			huckToggleBlinking(&g_huckScreensaverFlag, 0);
			int16 ms = (int16)((int32)READ_UINT16(g_huckGiftPic + 0x94) * 1000 / 60);
			g_huckScreensaverTimer = _xp->startTimer(ms);
		} else if (eventData == g_huckBlinkTimer) {
			huckToggleBlinking(&g_huckBlinkFlag, 1);
			int16 ms = (int16)((int32)READ_UINT16(g_huckGiftPic + 0x94) * 1000 / 60);
			g_huckBlinkTimer = _xp->startTimer(ms);
		} else if (eventData == g_huckShuffleTimer) {
			giftSwap();
			_xp->updateDisplay();
			startHuckShuffleTimer();
		}

		break;
	}

	case etInactivity: {
		if (!g_huckScreensaverTimer) {
			int16 ms = (int16)((int32)READ_UINT16(g_huckGiftPic + 0x94) * 1000 / 60);
			g_huckScreensaverTimer = _xp->startTimer(ms);
			g_huckScreensaverFlag = 0;
			huckToggleBlinking(&g_huckScreensaverFlag, 0);
			_xp->setInactivityTimer(1800);
		} else {
			bool exitLoop = false;
			_xp->setScreenBrightness(25);

			while (!shouldQuit()) {
				uint32 innerData = 0;
				int16 innerType = _xp->getEvent(etEmpty, &innerData);
				switch (innerType) {
				case etSound:
					if (g_huckSoundPlaying > 0)
						g_huckSoundPlaying--;

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
	}

	case etSound: {
		if (g_huckHotSpotCount != 0) {
			g_huckHotSpotCount = maintainAudioPlay(1);
			if (g_huckHotSpotCount == 0)
				huckUpdateHotSpots(g_huckCursorX, g_huckCursorY);
		}

		if (g_huckSoundPlaying > 0)
			g_huckSoundPlaying--;

		break;
	}

	case etTrigger: {
		g_huckActionState++;
		if (g_huckActionState == 1) {
			int16 ms = (int16)((int32)READ_UINT16(g_huckGiftPic + 0x94) * 1000 / 60);
			g_huckBlinkTimer = _xp->startTimer(ms);
			g_huckBlinkFlag = 0;
			huckToggleBlinking(&g_huckBlinkFlag, 1);
		} else if (g_huckActionState == 2) {
			_xp->killTimer(g_huckBlinkTimer);
			g_huckBlinkTimer = 0;
			restoreHuckColors(1);
		}

		break;
	}

	default:
		break;
	}
}

void BoltEngine::playHuck() {
	g_returnBooth = 0x10;

	while (!shouldQuit()) {
		if (g_huckExitFlag)
			break;

		uint32 eventData = 0;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);
		if (eventType)
			handleEvent(eventType, eventData);

		if (g_huckHotSpotCount != 0) {
			g_huckHotSpotCount = maintainAudioPlay(0);
			if (g_huckHotSpotCount == 0)
				huckUpdateHotSpots(g_huckCursorX, g_huckCursorY);
		}
	}

	if (shouldQuit())
		g_returnBooth = 0;
}

void BoltEngine::cleanUpHuck() {
	unloadHuckResources();

	if (g_huckScratchPic.pixelData) {
		_xp->freeMem(g_huckScratchPic.pixelData);
		g_huckScratchPic.pixelData = nullptr;
		g_huckScratchPic.palette = nullptr;
	}

	vSave(&g_huckGlobal, sizeof(g_huckGlobal), "HuckGlobal");

	byte huckSaveBuf[0x86] = { 0 };
	Common::MemoryWriteStream *huckStateWriteStream = new Common::MemoryWriteStream(huckSaveBuf, sizeof(huckSaveBuf));

	huckStateWriteStream->writeSint16BE(g_huckState.levelNumber);

	for (int i = 0; i < 10; i++)
		huckStateWriteStream->writeSint16BE(g_huckState.slotIndex[i]);

	huckStateWriteStream->writeSint16BE(g_huckState.levelComplete);

	for (int i = 0; i < 24; i++)
		huckStateWriteStream->writeSint16BE(g_huckState.drawTable1[i]);

	for (int i = 0; i < 24; i++)
		huckStateWriteStream->writeSint16BE(g_huckState.drawTable2[i]);

	huckStateWriteStream->writeSint16BE(g_huckState.giftCount);
	huckStateWriteStream->writeSint16BE(g_huckState.hasCycle);
	huckStateWriteStream->writeSint16BE(g_huckState.selectionPending);
	huckStateWriteStream->writeSint16BE(g_huckState.selected1Slot);
	huckStateWriteStream->writeSint16BE(g_huckState.selected2Slot);
	huckStateWriteStream->writeSint16BE(g_huckState.selected1SpriteId);
	huckStateWriteStream->writeSint16BE(g_huckState.selected2SpriteId);

	// Sanity check: should be exactly 0x86
	assert(huckStateWriteStream->pos() == 0x86);
	delete huckStateWriteStream;

	vSave(&huckSaveBuf, 0x86, "Huck");

	_xp->fillDisplay(0, stFront);
	_xp->updateDisplay();
}

int16 BoltEngine::huckGame(int16 prevBooth) {
	if (!openBOLTLib(&g_huckBoltLib, &g_huckBoltCallbacks, assetPath("huck.blt")))
		return g_returnBooth;

	int16 savedTimer = _xp->setInactivityTimer(30);

	if (initHuck())
		playHuck();

	cleanUpHuck();

	_xp->setInactivityTimer(savedTimer);
	closeBOLTLib(&g_huckBoltLib);

	return g_returnBooth;
}

void BoltEngine::swapHuckWordArray() {
	byte *ptr = g_boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	int16 count = (int16)(g_boltCurrentMemberEntry->decompSize / 2);
	for (int16 i = 0; i < count; i++, ptr += 2)
		WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
}

void BoltEngine::swapHuckWords() {
	byte *ptr = g_boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
	WRITE_UINT16(ptr + 2, READ_BE_UINT16(ptr + 2));
}

} // End of namespace Bolt
