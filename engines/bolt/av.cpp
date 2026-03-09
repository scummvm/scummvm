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

namespace Bolt {

void BoltEngine::setAVBufferSize(uint32 bufSize) {
	_avTargetBufSize = bufSize;
}

bool BoltEngine::prepareAV(RTFResource *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff) {
	if (!initAV(rtfHandle, animIndex, width, height, xOff, yOff)) {
		cleanUpAV();
		return false;
	}

	return true;
}

bool BoltEngine::maintainAV(int16 mode) {
	RTFPacket *frameData;

	if (!maintainRTF(mode, &frameData)) {
		cleanUpAV();
		return false;
	}

	if (frameData)
		processPacket(frameData);

	return true;
}

void BoltEngine::stopAV() {
	killRTF(nullptr);
	cleanUpAV();
}

bool BoltEngine::playAV(RTFResource *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff) {
	RTFPacket *frameData = nullptr;
	uint32 eventBuf = 0;
	int16 eventType = 0;

	// Init if not already loaded...
	if (!_avRingBuffer) {
		if (!initAV(rtfHandle, animIndex, width, height, xOff, yOff)) {
			cleanUpAV();
			return false;
		}
	}

	// Flush pending mouse-down events...
	while (_xp->getEvent(etMouseDown, &eventBuf) != etEmpty && !shouldQuit()) {
		// getEvent already calls pollEvent and delayMillis
	}

	// Main playback loop...
	while (!shouldQuit()) {
		eventType = _xp->getEvent(etEmpty, &eventBuf);

		if (eventType == etMouseDown) {
			// Mouse click, abort playback...
			killRTF(nullptr);
			cleanUpAV();
			return false;
		}

		if (eventType != etSound && eventType != etEmpty)
			continue;

		// Advance: mode=1 on sound event, mode=0 on idle
		if (!maintainRTF(eventType == etSound ? 1 : 0, &frameData)) {
			cleanUpAV();
			return true; // finished normally
		}

		if (frameData)
			processPacket(frameData);
	}

	// Not in the original, but it helps cleaning up
	// if the user wants to quit during a video...
	assert(shouldQuit());
	killRTF(nullptr);
	cleanUpAV();
	return false;
}

void BoltEngine::processPacket(RTFPacket *packet) {
	uint32 tag = packet->tag;

	// Adaptive frame rate control...
	if (tag == MKTAG('R', 'L', '7', ' ') || tag == MKTAG('R', 'L', '7', 'F')) {
		int16 frameRate = packet->frameRate;

		// Skip this frame if: low framerate, only 1 behind, or haven't accumulated enough...
		if (frameRate <= 10 || packet->skipCount > 1 || _avFrameAccum < _avSkipLevel) {
			_avFrameAccum += packet->duration;
			debug(4, "BoltEngine::processPacket(): Skipping frame...");
			return;
		}

		// Adjust skip level based on how far behind we are...
		if (_avFrameAccum > _avSkipLevel)
			_avSkipLevel = (_avFrameAccum > 4) ? 4 : _avFrameAccum;

		_avFrameAccum = 0;

		int16 targetSkip;
		if (frameRate <= 12)
			targetSkip = 4;
		else if (frameRate <= 15)
			targetSkip = 3;
		else if (frameRate <= 20)
			targetSkip = 2;
		else
			targetSkip = 1;

		// If keeping up fine, stop skipping entirely...
		if (_avSkipLevel <= 1 && targetSkip == 1 && packet->skipCount == 0) {
			_avSkipLevel = 0;
		} else {
			// Ramp toward target skip level...
			if (targetSkip > _avSkipLevel)
				_avSkipLevel = targetSkip;
			else if (targetSkip < _avSkipLevel)
				_avSkipLevel--;
		}
	} else if (tag == MKTAG('r', 'l', '7', ' ') || tag == MKTAG('r', 'l', '7', 'f')) {
		_avFrameAccum = 0;
	}

	// Process frame, palette and trigger packets...
	tag = packet->tag;

	if (tag == MKTAG('R', 'L', '7', ' ') || tag == MKTAG('R', 'L', '7', 'B') || tag == MKTAG('R', 'L', '7', 'F') ||
		tag == MKTAG('r', 'l', '7', ' ') || tag == MKTAG('r', 'l', '7', 'f')) {
		processRL7(packet);
	} else if (tag == MKTAG('P', 'L', 'T', 'B') || tag == MKTAG('P', 'L', 'T', 'E') || tag == MKTAG('P', 'L', 'T', 'F')) {
		processPLTE(packet);
	} else if (tag == MKTAG('T', 'R', 'G', 'R')) {
		_xp->postEvent(etTrigger, 0);
	}
}

void BoltEngine::processRL7(RTFPacket *packet) {
	XPPicDesc *displayDesc;
	int page;
	bool transparency;
	bool updateDisplay;

	uint32 tag = packet->tag;

	if (tag == MKTAG('R', 'L', '7', 'B')) {
		displayDesc = &_avBackBufDesc;
	} else {
		displayDesc = &_avFrontBufDesc;
	}

	switch (tag) {
	case MKTAG('R', 'L', '7', ' '):
	case MKTAG('r', 'l', '7', ' '):
		page = stFront;
		transparency = false;
		updateDisplay = true;
		break;
	case MKTAG('R', 'L', '7', 'F'):
	case MKTAG('r', 'l', '7', 'f'):
		page = stFront;
		transparency = true;
		updateDisplay = true;
		break;
	case MKTAG('R', 'L', '7', 'B'):
		page = stBack;
		transparency = true;
		updateDisplay = false;
		break;
	default:
		return;
	}

	_xp->setTransparency(transparency);

	displayDesc->pixelData = packet->dataPtr;

	_xp->displayPic(displayDesc, _avDisplayX, _avDisplayY, page);

	if (updateDisplay)
		_xp->updateDisplay();

	_avFrontBufDesc.palette = nullptr;
	_avFrontBufDesc.paletteStart = 0;
	_avFrontBufDesc.paletteCount = 0;

	if (transparency) {
		_avBackBufDesc.palette = nullptr;
		_avBackBufDesc.paletteStart = 0;
		_avBackBufDesc.paletteCount = 0;
	}
}

void BoltEngine::processPLTE(RTFPacket *packet) {
	byte *srcData = packet->dataPtr;

	int16 startIndex = READ_BE_INT16(srcData); srcData += 2;
	int16 count = READ_BE_INT16(srcData); srcData += 2;

	XPPicDesc *desc;
	byte *palBuffer;

	if (packet->tag == MKTAG('P', 'L', 'T', 'B')) {
		desc = &_avBackBufDesc;
		palBuffer = _avFrontPalette;
	} else {
		desc = &_avFrontBufDesc;
		palBuffer = _avBackPalette;
	}

	desc->paletteStart = startIndex;
	desc->paletteCount = count;
	desc->palette = palBuffer;

	// Copy RGB triplets into palette buffer...
	while (count-- > 0) {
		palBuffer[0] = *srcData++;
		palBuffer[1] = *srcData++;
		palBuffer[2] = *srcData++;
		palBuffer += 3;
	}
}

bool BoltEngine::initAV(RTFResource *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff) {
	uint32 savedAllocSize = _avTargetBufSize;
	uint32 allocSize = 0;

	_avTargetBufSize = 0xFA000; // ~1MB

	_avDisplayY = yOff;
	_avDisplayX = xOff;

	_avSavedInactivityTimer = _xp->setInactivityTimer(0);
	_avSavedScreenSaverTimer = _xp->setScreenSaverTimer(0);

	_avBackPalette = nullptr;
	_avFrontPalette = nullptr;
	_avFrameAccum = 0;

	// Allocate two 384-byte palette buffers (128 RGB triplets each)
	_avBackPalette = (byte *)_xp->allocMem(128 * 3);
	if (!_avBackPalette)
		return false;

	_avFrontPalette = (byte *)_xp->allocMem(128 * 3);
	if (!_avFrontPalette)
		return false;

	// Allocate ring buffer: try saved size, shrink by 50KB until 300KB minimum
	allocSize = savedAllocSize;
	do {
		_avRingBuffer = (byte *)_xp->tryAllocMem(allocSize);
		if (_avRingBuffer)
			break;

		allocSize -= 0xC800;
	} while (allocSize >= 0x4B000);

	if (!_avRingBuffer) {
		error("BoltEngine::initAV(): Not enough memory");
		return false;
	}

	_avBackBufDesc.width = width;
	_avFrontBufDesc.width = width;
	_avBackBufDesc.height = height;
	_avFrontBufDesc.height = height;
	_avBackBufDesc.flags = 1;
	_avFrontBufDesc.flags = 1;
	_avBackBufDesc.palette = nullptr;
	_avFrontBufDesc.palette = nullptr;
	_avBackBufDesc.paletteStart = 0;
	_avFrontBufDesc.paletteStart = 0;
	_avBackBufDesc.paletteCount = 0;
	_avFrontBufDesc.paletteCount = 0;

	// Open and start streaming!
	if (!playRTF(rtfHandle, animIndex, _avRingBuffer, allocSize))
		return false;

	return true;
}

void BoltEngine::cleanUpAV() {
	if (_avRingBuffer) {
		_xp->freeMem(_avRingBuffer);
		_avRingBuffer = nullptr;
	}

	if (_avFrontPalette) {
		_xp->freeMem(_avFrontPalette);
		_avFrontPalette = nullptr;
	}

	if (_avBackPalette) {
		_xp->freeMem(_avBackPalette);
		_avBackPalette = nullptr;
	}

	_xp->setInactivityTimer(_avSavedInactivityTimer);
	_xp->setScreenSaverTimer(_avSavedScreenSaverTimer);
}

} // End of namespace Bolt
