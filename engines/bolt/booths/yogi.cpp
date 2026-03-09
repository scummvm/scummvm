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
	byte *soundData = getBOLTMember(g_yogiBoltLib, memberId);
	int32 soundSize = memberSize(g_yogiBoltLib, memberId);
	if (soundData) {
		_xp->playSound(soundData, soundSize, 22050);
		g_yogiSoundActive = 1;
		g_yogiSoundPlaying++;
	}
}

void BoltEngine::waitSoundMapYogi() {
	if (!g_yogiSoundPlaying)
		return;

	uint32 dummy;
	while (_xp->getEvent(etSound, &dummy) == etEmpty);

	g_yogiSoundActive = 0;
	g_yogiSoundPlaying--;
}

void BoltEngine::stopSoundYogi() {
	if (g_yogiSoundActive) {
		_xp->stopSound();
		g_yogiSoundActive = 0;
	}
}

void BoltEngine::setYogiColors(int16 which) {
	if (which == 0)
		_xp->setPalette(g_yogiPalRange[1] - g_yogiPalRange[0] + 1, g_yogiPalRange[0], g_yogiPalHighlight0);
	else if (which == 1)
		_xp->setPalette(g_yogiPalRange[5] - g_yogiPalRange[4] + 1, g_yogiPalRange[4], g_yogiPalHighlight1);
}

void BoltEngine::restoreYogiColors(int16 which) {
	if (which == 0)
		_xp->setPalette(g_yogiPalRange[1] - g_yogiPalRange[0] + 1, g_yogiPalRange[0], g_yogiPalSave0);
	else if (which == 1)
		_xp->setPalette(g_yogiPalRange[5] - g_yogiPalRange[4] + 1, g_yogiPalRange[4], g_yogiPalSave1);
}

void BoltEngine::drawBasket(int16 slot, byte *basketSprite) {
	int16 sprStride = READ_UINT16(basketSprite + 0x0A);
	int16 sprHeight = READ_UINT16(basketSprite + 0x0C);

	memset(g_yogiScratchBuf.pixelData, 0, (int32)sprStride * sprHeight);

	int16 slotY = READ_UINT16(g_yogiBasketPic + slot * 2 + 0x42);
	int16 slotX = READ_UINT16(g_yogiBasketPic + slot * 2 + 0x12);
	Common::Rect targetRect(slotY, slotX, slotY + sprHeight, slotX + sprStride);

	int16 basketCount = READ_UINT16(g_yogiBasketPic);
	for (int16 i = 0; i < basketCount; i++) {
		if (g_yogiState.basketState[i] != 0)
			continue;

		byte *otherSprite;
		if (i == g_yogiState.selected1Slot || i == g_yogiState.selected2Slot)
			otherSprite = g_yogiHlSprite;
		else
			otherSprite = g_yogiNormalSprite;

		int16 otherStride = READ_UINT16(otherSprite + 0x0A);
		int16 otherHeight = READ_UINT16(otherSprite + 0x0C);
		int16 otherY = READ_UINT16(g_yogiBasketPic + i * 2 + 0x42);
		int16 otherX = READ_UINT16(g_yogiBasketPic + i * 2 + 0x12);
		Common::Rect otherRect(otherY, otherX, otherY + otherHeight, otherX + otherStride);

		Common::Rect isect;
		if (!intersectRect(&targetRect, &otherRect, &isect))
			continue;

		byte *srcPixels = getResolvedPtr(otherSprite, 0x12);
		byte *src = srcPixels + (isect.left - otherY) * otherStride + (isect.top - otherX);
		byte *dst = g_yogiScratchBuf.pixelData + (isect.left - slotY) * sprStride + (isect.top - slotX);

		int16 blitWidth = isect.bottom - isect.top;
		int16 blitHeight = isect.right - isect.left;
		_xp->maskBlit(src, otherStride, dst, sprStride, blitWidth, blitHeight);
	}

	g_yogiScratchBuf.width = sprStride;
	g_yogiScratchBuf.height = sprHeight;
	g_yogiScratchBuf.palette = nullptr;
	g_yogiScratchBuf.flags = 0;

	_xp->displayPic(&g_yogiScratchBuf, slotX, slotY, stFront);
}

void BoltEngine::drawAllBaskets() {
	_xp->fillDisplay(0, stFront);

	int16 basketCount = READ_UINT16(g_yogiBasketPic);
	for (int16 i = 0; i < basketCount; i++) {
		if (g_yogiState.basketState[i] != 0)
			continue;

		byte *sprite = (i == g_yogiState.selected1Slot || i == g_yogiState.selected2Slot)
						   ? g_yogiHlSprite
						   : g_yogiNormalSprite;

		int16 slotY = READ_UINT16(g_yogiBasketPic + i * 2 + 0x42);
		int16 slotX = READ_UINT16(g_yogiBasketPic + i * 2 + 0x12);
		displayPic(sprite, slotX, slotY, stFront);
	}
}

void BoltEngine::handleYogiMatch() {
	playSoundMapYogi((int16)READ_UINT16(g_yogiBasketPic + 0x9E));

	if (g_yogiState.basketCount == 2) {
		g_yogiExitFlag = 1;
		g_yogiState.levelComplete = 1;
		g_yogiState.levelNumber++;
		if (g_yogiState.levelNumber > 10)
			g_yogiState.levelNumber = 10;
		_xp->fillDisplay(0, stFront);
	} else {
		drawBasket(g_yogiState.selected1Slot, g_yogiHlSprite);
		drawBasket(g_yogiState.selected2Slot, g_yogiHlSprite);
	}

	g_yogiState.selected1Slot = -1;
	g_yogiState.selected2Slot = -1;
	g_yogiState.basketCount -= 2;
	_xp->updateDisplay();
	waitSoundMapYogi();
}

bool BoltEngine::loadYogiBgPic() {
	int16 groupId;

	if (g_displayMode == 0) {
		groupId = 0x100;
	} else if (g_displayMode == 1) {
		groupId = 0x200;
	} else {
		return false;
	}

	if (!getBOLTGroup(g_yogiBoltLib, groupId, 1))
		return false;

	g_yogiBgPic = memberAddr(g_yogiBoltLib, groupId + 1);
	return true;
}

void BoltEngine::unloadYogiBgPic() {
	freeBOLTGroup(g_yogiBoltLib, 0x100, 1);
}

void BoltEngine::drawYogiLevel() {
	byte *palSprite = memberAddr(g_yogiBoltLib, READ_UINT16(g_yogiBasketPic + 0x72));

	g_yogiSoundActive = 0;
	g_yogiHotSpotCount = 0;
	g_yogiBlinkTimer1 = 0;
	g_yogiBlinkTimer2 = 0;
	g_yogiExitFlag = 0;

	uint32 dummy;
	while (_xp->getEvent(etTimer, &dummy) != etTimer);

	_xp->stopCycle();
	_xp->setTransparency(false);
	displayColors(palSprite, stBack, 0);
	displayPic(g_yogiBgPic, g_displayX, g_displayY, stFront);
	_xp->updateDisplay();

	_xp->setTransparency(true);
	displayColors(palSprite, stFront, 0);
	displayColors(palSprite, stBack, 1);
	displayPic(g_yogiBgPic, g_displayX, g_displayY, stBack);

	drawAllBaskets();
	_xp->updateDisplay();

	g_yogiCursorX = 0xC0;
	g_yogiCursorY = 0x78;
	_xp->setCursorPos(0xC0, 0x78);
	_xp->setCursorColor(0, 0, 0xFF);
	_xp->showCursor();

	g_yogiPalRange[0] = READ_UINT16(g_yogiBasketPic + 0x7C) + 0x80;
	g_yogiPalRange[1] = READ_UINT16(g_yogiBasketPic + 0x7E) + 0x80;
	g_yogiPalRange[2] = READ_UINT16(g_yogiBasketPic + 0x80) + 0x80;
	g_yogiPalRange[3] = READ_UINT16(g_yogiBasketPic + 0x82) + 0x80;
	g_yogiPalRange[4] = READ_UINT16(g_yogiBasketPic + 0x8E) + 0x80;
	g_yogiPalRange[5] = READ_UINT16(g_yogiBasketPic + 0x90) + 0x80;
	g_yogiPalRange[6] = READ_UINT16(g_yogiBasketPic + 0x92) + 0x80;
	g_yogiPalRange[7] = READ_UINT16(g_yogiBasketPic + 0x94) + 0x80;

	_xp->getPalette(g_yogiPalRange[0], g_yogiPalRange[1] - g_yogiPalRange[0] + 1, g_yogiPalSave0);
	_xp->getPalette(g_yogiPalRange[2], g_yogiPalRange[3] - g_yogiPalRange[2] + 1, g_yogiPalHighlight0);
	_xp->getPalette(g_yogiPalRange[4], g_yogiPalRange[5] - g_yogiPalRange[4] + 1, g_yogiPalSave1);
	_xp->getPalette(g_yogiPalRange[6], g_yogiPalRange[7] - g_yogiPalRange[6] + 1, g_yogiPalHighlight1);
}

bool BoltEngine::loadYogiLevel() {
	int32 maxSize = 0;

	g_yogiLevelGroupId = (g_yogiState.levelNumber - 1) * 0x100 + 0x300;

	if (!getBOLTGroup(g_yogiBoltLib, g_yogiLevelGroupId, 1))
		return false;

	g_yogiBasketPic = memberAddr(g_yogiBoltLib, g_yogiLevelGroupId);
	g_yogiNormalSprite = memberAddr(g_yogiBoltLib, READ_UINT16(g_yogiBasketPic + 0x02));
	g_yogiHlSprite = memberAddr(g_yogiBoltLib, READ_UINT16(g_yogiBasketPic + 0x04));
	g_yogiAnimSprite = memberAddr(g_yogiBoltLib, READ_UINT16(g_yogiBasketPic + 0x06));

	int32 size;

	size = (int32)READ_UINT16(g_yogiNormalSprite + 0x0A) * READ_UINT16(g_yogiNormalSprite + 0x0C);
	if (size > maxSize)
		maxSize = size;

	size = (int32)READ_UINT16(g_yogiHlSprite + 0x0A) * READ_UINT16(g_yogiHlSprite + 0x0C);
	if (size > maxSize)
		maxSize = size;

	size = (int32)READ_UINT16(g_yogiAnimSprite + 0x0A) * READ_UINT16(g_yogiAnimSprite + 0x0C);
	if (size > maxSize)
		maxSize = size;

	g_yogiScratchBuf.pixelData = (byte *)_xp->allocMem(maxSize);
	if (!g_yogiScratchBuf.pixelData)
		return false;

	g_yogiSpriteStride = READ_UINT16(g_yogiNormalSprite + 0x0A);
	g_yogiSpriteHeight = READ_UINT16(g_yogiNormalSprite + 0x0C);
	return true;
}

void BoltEngine::unloadYogiResources() {
	_xp->hideCursor();
	int16 basketGroupId = (g_yogiLevelId << 8) + 0xD00;
	freeBOLTGroup(g_yogiBoltLib, basketGroupId, 1);
	freeBOLTGroup(g_yogiBoltLib, g_yogiLevelGroupId, 1);
}

bool BoltEngine::initYogiLevel() {
	g_yogiState.currentSlot++;
	if (g_yogiState.currentSlot >= 10)
		g_yogiState.currentSlot = 0;

	g_yogiLevelId = g_yogiState.slotIndex[g_yogiState.currentSlot];

	if (!loadYogiLevel())
		return false;

	int16 basketCount = READ_UINT16(g_yogiBasketPic);
	g_yogiState.basketCount = basketCount;
	g_yogiState.levelComplete = 0;
	g_yogiState.matchCount = 0;
	g_yogiState.selectionPending = 0;
	g_yogiState.selected1Slot = -1;
	g_yogiState.selected2Slot = -1;

	for (int16 i = 0; i < 0x18; i++) {
		g_yogiState.basketSound[i] = 0x10;
		g_yogiState.basketState[i] = 0;
	}

	int16 idx = g_yogiState.levelIndex[g_yogiLevelId];
	int16 snd = g_yogiGlobal[g_yogiLevelId * 0x10 + idx];
	int16 flag = 0;

	for (int16 var_4 = 0; var_4 < basketCount; var_4++) {
		int16 rnd = _xp->getRandom(basketCount);
		while (g_yogiState.basketSound[rnd] != 0x10) {
			rnd++;
			if (rnd >= basketCount)
				rnd = 0;
		}
		g_yogiState.basketSound[rnd] = snd;

		if (flag == 0) {
			flag = 1;
		} else {
			flag = 0;
			idx++;
			if (idx == 0x10)
				idx = 0;
			snd = g_yogiGlobal[g_yogiLevelId * 0x10 + idx];
		}
	}

	g_yogiState.levelIndex[g_yogiLevelId] = idx;
	drawYogiLevel();
	return true;
}

bool BoltEngine::resumeYogiLevel() {
	g_yogiLevelId = g_yogiState.slotIndex[g_yogiState.currentSlot];

	if (!loadYogiLevel())
		return false;

	drawYogiLevel();
	return true;
}

bool BoltEngine::initYogi() {
	g_yogiSoundPlaying = 0;

	if (!loadYogiBgPic())
		return false;

	if (!vLoad(&g_yogiGlobal, "YogiGlobal")) {
		int16 slotVal = 0;
		int16 globIdx = 0;
		int16 idx = 0;

		while (slotVal < 10) {
			g_yogiState.slotIndex[idx] = slotVal;
			g_yogiState.levelIndex[idx] = 0;

			for (int16 i = 0; i < 16; i++)
				g_yogiGlobal[globIdx + i] = 0x10;

			for (int16 di = 0; di < 16; di++) {
				int16 rnd = _xp->getRandom(16);
				while (g_yogiGlobal[globIdx + rnd] != 0x10) {
					rnd++;
					if (rnd >= 16)
						rnd = 0;
				}
				g_yogiGlobal[globIdx + rnd] = di;
			}

			globIdx += 0x10;
			idx++;
			slotVal++;
		}
	}

	byte yogiStateBuf[0x9C] = { 0 };

	if (!vLoad(&yogiStateBuf, "Yogi")) {
		g_yogiState.levelNumber = 1;
		g_yogiState.currentSlot = -1;
		return initYogiLevel();
	}

	Common::SeekableReadStream *yogiStateReadStream = new Common::MemoryReadStream(yogiStateBuf, sizeof(yogiStateBuf), DisposeAfterUse::NO);

	g_yogiState.levelNumber = yogiStateReadStream->readSint16BE();
	g_yogiState.currentSlot = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 10; i++)
		g_yogiState.levelIndex[i] = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 10; i++)
		g_yogiState.slotIndex[i] = yogiStateReadStream->readSint16BE();

	g_yogiState.levelComplete = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		g_yogiState.basketSound[i] = yogiStateReadStream->readSint16BE();

	for (int i = 0; i < 24; i++)
		g_yogiState.basketState[i] = yogiStateReadStream->readSint16BE();

	g_yogiState.basketCount = yogiStateReadStream->readSint16BE();
	g_yogiState.matchCount = yogiStateReadStream->readSint16BE();
	g_yogiState.selectionPending = yogiStateReadStream->readSint16BE();
	g_yogiState.selected1Slot = yogiStateReadStream->readSint16BE();
	g_yogiState.selected2Slot = yogiStateReadStream->readSint16BE();
	g_yogiState.sound1 = yogiStateReadStream->readSint16BE();
	g_yogiState.sound2 = yogiStateReadStream->readSint16BE();

	// Sanity check: should be exactly 0x9C
	assert(yogiStateReadStream->pos() == 0x9C);
	delete yogiStateReadStream;

	if (g_yogiState.levelComplete)
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
		READ_UINT16(g_yogiBasketPic + 0x74), READ_UINT16(g_yogiBasketPic + 0x78),
		READ_UINT16(g_yogiBasketPic + 0x76), READ_UINT16(g_yogiBasketPic + 0x7A));

	Common::Rect exitRect(
		READ_UINT16(g_yogiBasketPic + 0x86), READ_UINT16(g_yogiBasketPic + 0x8A),
		READ_UINT16(g_yogiBasketPic + 0x88), READ_UINT16(g_yogiBasketPic + 0x8C));

	if (helpRect.contains(x, y)) {
		if (!g_yogiBlinkTimer1 && !g_yogiHotSpotCount)
			setYogiColors(0);
	} else {
		if (!g_yogiBlinkTimer1 && !g_yogiHotSpotCount)
			restoreYogiColors(0);
	}

	if (exitRect.contains(x, y)) {
		if (!g_yogiBlinkTimer2)
			setYogiColors(1);
		return;
	} else {
		if (!g_yogiBlinkTimer2)
			restoreYogiColors(1);
	}
}

int16 BoltEngine::findBasket(int16 x, int16 y) {
	int16 basketCount = READ_UINT16(g_yogiBasketPic);
	for (int16 i = basketCount - 1; i >= 0; i--) {
		if (g_yogiState.basketState[i] != 0)
			continue;

		int16 slotX = READ_UINT16(g_yogiBasketPic + i * 2 + 0x12);
		int16 slotX2 = slotX + g_yogiSpriteStride - 1;
		int16 slotY = READ_UINT16(g_yogiBasketPic + i * 2 + 0x42);
		int16 slotY2 = slotY + g_yogiSpriteHeight - 1;

		if (x < slotX || x > slotX2 || y < slotY || y > slotY2)
			continue;

		byte *sprite = (i == g_yogiState.selected1Slot || i == g_yogiState.selected2Slot)
						   ? g_yogiHlSprite
						   : g_yogiNormalSprite;

		if (!getPixel(sprite, x - slotX, y - slotY))
			continue;

		return i;
	}

	return -1;
}

void BoltEngine::resolveYogiSelection() {
	if (g_yogiState.sound1 == g_yogiState.sound2) {
		g_yogiState.basketState[g_yogiState.selected1Slot] = 1;
		g_yogiState.basketState[g_yogiState.selected2Slot] = 1;
		handleYogiMatch();
		g_yogiState.matchCount = 0;
		g_yogiState.selectionPending = 0;
	} else {
		int16 slot1 = g_yogiState.selected1Slot;
		g_yogiState.matchCount = 0;
		g_yogiState.selected1Slot = -1;
		drawBasket(slot1, g_yogiNormalSprite);
		_xp->updateDisplay();
		playSoundMapYogi((int32)(int16)READ_UINT16(g_yogiBasketPic + 0x9A));
		waitSoundMapYogi();

		int16 slot2 = g_yogiState.selected2Slot;
		g_yogiState.selectionPending = 0;
		g_yogiState.selected2Slot = -1;
		drawBasket(slot2, g_yogiNormalSprite);
		_xp->updateDisplay();
		playSoundMapYogi((int32)(int16)READ_UINT16(g_yogiBasketPic + 0x9C));
		waitSoundMapYogi();
	}
}

bool BoltEngine::handleBasketSelect(int16 x, int16 y) {
	int16 slot = findBasket(x, y);
	if (slot == -1)
		return false;

	if (g_yogiState.matchCount == 0) {
		g_yogiState.matchCount = 1;
		g_yogiState.sound1 = g_yogiState.basketSound[slot];
		g_yogiState.selected1Slot = slot;

		playSoundMapYogi((int32)(int16)READ_UINT16(g_yogiBasketPic + 0x98));
		drawBasket(slot, g_yogiHlSprite);
		_xp->updateDisplay();
		waitSoundMapYogi();

		int32 soundId = (int32)((g_yogiLevelId << 8) + g_yogiState.sound1 + 0xD00);
		playSoundMapYogi(soundId);

	} else {
		if (g_yogiState.selectionPending)
			return true;

		if (g_yogiState.selected1Slot == slot) {
			// De-select basket...
			g_yogiState.matchCount = 0;
			g_yogiState.selected1Slot = -1;

			stopSoundYogi();
			playSoundMapYogi((int32)(int16)READ_UINT16(g_yogiBasketPic + 0x9A));
			drawBasket(slot, g_yogiNormalSprite);
			_xp->updateDisplay();
			waitSoundMapYogi();

		} else {
			g_yogiState.selectionPending = 1;
			g_yogiState.sound2 = g_yogiState.basketSound[slot];
			g_yogiState.selected2Slot = slot;

			if (g_yogiSoundActive) {
				int16 slotY = READ_UINT16(g_yogiBasketPic + slot * 2 + 0x42);
				int16 slotX = READ_UINT16(g_yogiBasketPic + slot * 2 + 0x12);
				displayPic(g_yogiAnimSprite, slotX + READ_UINT16(g_yogiBasketPic + 0x0A), slotY + READ_UINT16(g_yogiBasketPic + 0x0C), stFront);
				_xp->updateDisplay();
				waitSoundMapYogi();
			}

			playSoundMapYogi((int32)(int16)READ_UINT16(g_yogiBasketPic + 0x98));
			drawBasket(slot, g_yogiHlSprite);
			_xp->updateDisplay();
			waitSoundMapYogi();

			int32 soundId = (int32)((g_yogiLevelId << 8) + g_yogiState.sound2 + 0xD00);
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

	if (g_yogiHotSpotCount != 0) {
		g_yogiHotSpotCount = 0;
		stoppedAnim = true;
		stopAnimation();
		if (g_yogiBlinkTimer2) {
			_xp->killTimer(g_yogiBlinkTimer2);
			g_yogiBlinkTimer2 = 0;
		}

		yogiUpdateHotSpots(g_yogiCursorX, g_yogiCursorY);
	}

	Common::Rect helpRect(
		READ_UINT16(g_yogiBasketPic + 0x74), READ_UINT16(g_yogiBasketPic + 0x78),
		READ_UINT16(g_yogiBasketPic + 0x76), READ_UINT16(g_yogiBasketPic + 0x7A));

	if (helpRect.contains(x, y)) {
		didAction = true;
		stopSoundYogi();
		if (g_yogiBlinkTimer1) {
			_xp->killTimer(g_yogiBlinkTimer1);
			g_yogiBlinkTimer1 = 0;
			setYogiColors(0);
		}
		if (!g_yogiHotSpotCount && !stoppedAnim) {
			g_yogiHotSpotCount = startAnimation(g_rtfHandle, 0x1D);
			g_yogiAnimActive = 0;
		}
	} else {
		Common::Rect exitRect(
			READ_UINT16(g_yogiBasketPic + 0x86), READ_UINT16(g_yogiBasketPic + 0x8A),
			READ_UINT16(g_yogiBasketPic + 0x88), READ_UINT16(g_yogiBasketPic + 0x8C));

		if (exitRect.contains(x, y)) {
			stopSoundYogi();
			setYogiColors(1);
			restoreYogiColors(0);
			g_yogiReturnBooth = 6;
			g_yogiExitFlag = 1;
		}
	}

	if (handleBasketSelect(x, y))
		didAction = true;

	if (didAction) {
		if (g_yogiBlinkTimer1) {
			_xp->killTimer(g_yogiBlinkTimer1);
			g_yogiBlinkTimer1 = 0;
			restoreYogiColors(0);
		}
		_xp->setInactivityTimer(0x1E);
	}
}

void BoltEngine::handleYogiEvent(int16 eventType, uint32 eventData) {
	switch (eventType) {
	case etMouseDown:
		yogiHandleActionButton(g_yogiCursorX, g_yogiCursorY);
		break;

	case etMouseMove:
		g_yogiCursorX = (int16)(eventData >> 16);
		g_yogiCursorY = (int16)(eventData & -1);
		yogiUpdateHotSpots(g_yogiCursorX, g_yogiCursorY);
		break;

	case etTimer: { // 1
		int32 blinkInterval = ((int32)READ_UINT16(g_yogiBasketPic + 0x84)) * 1000 / 60;

		if (eventData == g_yogiBlinkTimer1) {
			yogiToggleBlinking(0, &g_yogiBlinkState1);
			g_yogiBlinkTimer1 = _xp->startTimer((int16)blinkInterval);
		} else if (eventData == g_yogiBlinkTimer2) {
			yogiToggleBlinking(1, &g_yogiBlinkState2);
			g_yogiBlinkTimer2 = _xp->startTimer((int16)blinkInterval);
		}

		break;
	}

	case etInactivity:
		if (!g_yogiBlinkTimer1) {
			int32 blinkInterval = ((int32)READ_UINT16(g_yogiBasketPic + 0x84)) * 1000 / 60;
			g_yogiBlinkTimer1 = _xp->startTimer((int16)blinkInterval);
			g_yogiBlinkState1 = 0;
			yogiToggleBlinking(0, &g_yogiBlinkState1);
		}

		break;

	case etSound:
		if (g_yogiHotSpotCount != 0) {
			g_yogiHotSpotCount = maintainAudioPlay(1);
			if (g_yogiHotSpotCount == 0)
				yogiUpdateHotSpots(g_yogiCursorX, g_yogiCursorY);
		} else {
			g_yogiSoundActive = 0;
		}

		if (g_yogiSoundPlaying != 0)
			g_yogiSoundPlaying--;

		break;

	case etTrigger:
		g_yogiAnimActive++;

		if (g_yogiAnimActive == 1) {
			int32 blinkInterval = ((int32)READ_UINT16(g_yogiBasketPic + 0x84)) * 1000 / 60;
			g_yogiBlinkTimer2 = _xp->startTimer((int16)blinkInterval);
			g_yogiBlinkState2 = 0;
			yogiToggleBlinking(1, &g_yogiBlinkState2);
		} else if (g_yogiAnimActive == 2) {
			_xp->killTimer(g_yogiBlinkTimer2);
			g_yogiBlinkTimer2 = 0;
			restoreYogiColors(1);
		}

		break;

	default:
		break;
	}
}

void BoltEngine::playYogi() {
	g_yogiReturnBooth = 16;

	while (!shouldQuit()) {
		if (g_yogiExitFlag)
			break;

		uint32 eventData = 0;
		int16 eventType = _xp->getEvent(etEmpty, &eventData);

		if (eventType)
			handleYogiEvent(eventType, eventData);

		if (g_yogiHotSpotCount != 0) {
			g_yogiHotSpotCount = maintainAudioPlay(0);
			if (g_yogiHotSpotCount == 0)
				yogiUpdateHotSpots(g_yogiCursorX, g_yogiCursorY);
		}
	}

	if (shouldQuit())
		g_yogiReturnBooth = 0;
}

void BoltEngine::cleanUpYogi() {
	if (g_yogiScratchBuf.pixelData) {
		_xp->freeMem(g_yogiScratchBuf.pixelData);
		g_yogiScratchBuf.pixelData = nullptr;
		g_yogiScratchBuf.width = 0;
		g_yogiScratchBuf.height = 0;
	}

	vSave(&g_yogiGlobal, 0x140, "YogiGlobal");

	byte yogiSaveBuf[0x9C] = { 0 };
	Common::MemoryWriteStream *yogiStateWriteStream = new Common::MemoryWriteStream(yogiSaveBuf, sizeof(yogiSaveBuf));

	yogiStateWriteStream->writeSint16BE(g_yogiState.levelNumber);
	yogiStateWriteStream->writeSint16BE(g_yogiState.currentSlot);

	for (int i = 0; i < 10; i++)
		yogiStateWriteStream->writeSint16BE(g_yogiState.levelIndex[i]);

	for (int i = 0; i < 10; i++)
		yogiStateWriteStream->writeSint16BE(g_yogiState.slotIndex[i]);

	yogiStateWriteStream->writeSint16BE(g_yogiState.levelComplete);

	for (int i = 0; i < 24; i++)
		yogiStateWriteStream->writeSint16BE(g_yogiState.basketSound[i]);

	for (int i = 0; i < 24; i++)
		yogiStateWriteStream->writeSint16BE(g_yogiState.basketState[i]);

	yogiStateWriteStream->writeSint16BE(g_yogiState.basketCount);
	yogiStateWriteStream->writeSint16BE(g_yogiState.matchCount);
	yogiStateWriteStream->writeSint16BE(g_yogiState.selectionPending);
	yogiStateWriteStream->writeSint16BE(g_yogiState.selected1Slot);
	yogiStateWriteStream->writeSint16BE(g_yogiState.selected2Slot);
	yogiStateWriteStream->writeSint16BE(g_yogiState.sound1);
	yogiStateWriteStream->writeSint16BE(g_yogiState.sound2);

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
	if (!openBOLTLib(&g_yogiBoltLib, &g_yogiBoltCallbacks, assetPath("yogi.blt")))
		return g_yogiReturnBooth;

	int16 savedTimer = _xp->setInactivityTimer(30);

	if (initYogi())
		playYogi();

	cleanUpYogi();
	_xp->setInactivityTimer(savedTimer);
	closeBOLTLib(&g_yogiBoltLib);

	return g_yogiReturnBooth;
}

void BoltEngine::swapYogiAllWords() {
	byte *ptr = g_boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	int16 count = (int16)(g_boltCurrentMemberEntry->decompSize >> 1);
	for (int16 i = 0; i < count; i++) {
		WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
		ptr += 2;
	}
}

void BoltEngine::swapYogiFirstWord() {
	byte *ptr = (byte *)g_boltCurrentMemberEntry->dataPtr;
	if (!ptr)
		return;

	WRITE_UINT16(ptr, READ_BE_UINT16(ptr));
}

} // End of namespace Bolt
