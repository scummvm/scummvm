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
#include "bolt/xplib/xplib.h"

#include "common/timer.h"

namespace Bolt {

void XpLib::getPalette(int16 startIndex, int16 count, byte *destBuf) {
	memcpy(destBuf, &_paletteBuffer[startIndex * 3], count * 3);
}

void XpLib::setPalette(int16 count, int16 startIndex, byte *srcBuf) {
	if (startIndex == 0 || (startIndex == 128 && count != 1)) {
		startIndex++;
		srcBuf += 3;
		count--;
	}

	if (count > 0) {
		memcpy(&_paletteBuffer[startIndex * 3], srcBuf, count * 3);

		// Apply brightness shift into shifted buffer...
		const byte *src = &_paletteBuffer[startIndex * 3];
		byte *dst = &_shiftedPaletteBuffer[startIndex * 3];

		for (uint16 i = 0; i < count * 3; i++)
			dst[i] = src[i] >> _brightnessShift;

		// Finally set the palette!
		g_system->getPaletteManager()->setPalette(&_shiftedPaletteBuffer[startIndex * 3], startIndex, count);
	}
}

bool XpLib::startCycle(XPCycleState *specs) {
	stopCycle();

	uint32 now = g_system->getMillis();

	for (int16 i = 0; i < 4; i++) {
		if (specs[i].startIndex == specs[i].endIndex) {
			_cycleStates[i].active = false;
			continue;
		}

		_cycleStates[i].startIndex = specs[i].startIndex;
		_cycleStates[i].endIndex = specs[i].endIndex;
		_cycleStates[i].delay = specs[i].delay;
		_cycleStates[i].nextFire = now + specs[i].delay;
		_cycleStates[i].active = true;
	}

	return true;
}

void XpLib::cycleColors() {
	for (int i = 0; i < 4; i++) {
		if (!_cycleStates[i].active)
			continue;

		uint32 now = g_system->getMillis();
		if (now < _cycleStates[i].nextFire)
			continue;

		_cycleStates[i].nextFire = now + _cycleStates[i].delay;

		int16 startIdx = _cycleStates[i].startIndex;
		int16 endIdx = _cycleStates[i].endIndex;

		if (startIdx < endIdx) {
			int16 count = endIdx - startIdx + 1;
			if (count > 19 || startIdx < 0 || endIdx > 255)
				continue;

			// Read current palette range...
			getPalette(startIdx, count, &_cycleTempPalette[3]);

			// Save last entry into first position...
			memcpy(_cycleTempPalette, &_cycleTempPalette[count * 3], 3);

			// Write back shifted by one (swap buffer starts 3 bytes earlier)...
			setPalette(count, startIdx, _cycleTempPalette);
		} else if (startIdx > endIdx) {
			int16 count = startIdx - endIdx + 1;
			if (count > 19 || endIdx < 0 || startIdx > 255)
				continue;

			// Read current palette range...
			getPalette(endIdx, count, _cycleTempPalette);

			// Save first entry, append after last...
			memcpy(&_cycleTempPalette[count * 3], _cycleTempPalette, 3);

			// Write back shifted by one (starts one entry later)...
			setPalette(count, endIdx, &_cycleTempPalette[3]);
		}

		g_system->updateScreen();
	}
}

void XpLib::stopCycle() {
	for (int16 i = 0; i < 4; i++) {
		if (_cycleStates[i].active) {
			_cycleStates[i].active = false;
		}
	}
}

void XpLib::setScreenBrightness(uint8 percent) {
	if (percent >= 100) {
		_brightnessShift = 0; // Full brightness
	} else if (percent >= 50) {
		_brightnessShift = 1; // 50%
	} else if (percent >= 25) {
		_brightnessShift = 2; // 25%
	} else if (percent >= 12) {
		_brightnessShift = 3; // 12%
	} else {
		_brightnessShift = 4; // Near black
	}

	// Re-apply entire palette with new brightness...
	setPalette(256, 0, _paletteBuffer);

	g_system->updateScreen();
}

} // End of namespace Bolt
