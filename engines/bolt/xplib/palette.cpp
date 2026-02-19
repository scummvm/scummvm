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

void XpLib::getPalette(uint16 startIndex, uint16 count, byte *destBuf) {
	Common::StackLock lock(g_paletteMutex);

	memcpy(destBuf, &g_paletteBuffer[startIndex * 3], count * 3);
}

void XpLib::setPalette(uint16 count, uint16 startIndex, byte *srcBuf) {
	Common::StackLock lock(g_paletteMutex);

	if (startIndex == 0 || (startIndex == 128 && count != 1)) {
		startIndex++;
		srcBuf += 3;
		count--;
	}

	if (count == 0)
		return;

	memcpy(&g_paletteBuffer[startIndex * 3], srcBuf, count * 3);

	// Apply brightness shift into shifted buffer...
	const byte *src = &g_paletteBuffer[startIndex * 3];
	byte *dst = &g_shiftedPaletteBuffer[startIndex * 3];

	for (uint16 i = 0; i < count * 3; i++)
		dst[i] = src[i] >> g_brightnessShift;

	// Finally set the palette!
	g_system->getPaletteManager()->setPalette(&g_shiftedPaletteBuffer[startIndex * 3], startIndex, count);
}

void XpLib::cycleColorsCallback(void *refCon) {
	XpLib *xp = (XpLib *)refCon;
	xp->cycleColors();
}

bool XpLib::startCycle(XPCycleState *specs) {
	Common::StackLock lock(g_paletteMutex);

	stopCycle();

	bool anyActive = false;
	uint32 now = g_system->getMillis();

	for (int16 i = 0; i < 4; i++) {
		if (specs[i].startIndex == specs[i].endIndex) {
			g_cycleStates[i].active = false;
			continue;
		}

		g_cycleStates[i].startIndex = specs[i].startIndex;
		g_cycleStates[i].endIndex = specs[i].endIndex;
		g_cycleStates[i].delay = specs[i].delay;
		g_cycleStates[i].nextFire = now + specs[i].delay;
		g_cycleStates[i].active = true;
		anyActive = true;
	}

	if (anyActive) {
		_bolt->getTimerManager()->installTimerProc(
			cycleColorsCallback, 50 * 1000, this, "BoltPalCycle");
	}

	return true;
}

void XpLib::cycleColors() {
	Common::StackLock lock(g_paletteMutex);

	for (int i = 0; i < 4; i++) {
		if (!g_cycleStates[i].active)
			continue;

		uint32 now = g_system->getMillis();
		if (now < g_cycleStates[i].nextFire)
			continue;

		g_cycleStates[i].nextFire = now + g_cycleStates[i].delay;

		int16 startIdx = g_cycleStates[i].startIndex;
		int16 endIdx = g_cycleStates[i].endIndex;

		if (startIdx < endIdx) {
			int16 count = endIdx - startIdx + 1;
			if (count > 19 || startIdx < 0 || endIdx > 255)
				continue;

			// Read current palette range...
			getPalette(startIdx, count, &g_cycleTempPalette[3]);

			// Save last entry into first position...
			memcpy(g_cycleTempPalette, &g_cycleTempPalette[count * 3], 3);

			// Write back shifted by one (swap buffer starts 3 bytes earlier)...
			setPalette(count, startIdx, g_cycleTempPalette);

		} else if (startIdx > endIdx) {
			int16 count = startIdx - endIdx + 1;
			if (count > 19 || endIdx < 0 || startIdx > 255)
				continue;

			// Read current palette range...
			getPalette(endIdx, count, g_cycleTempPalette);

			// Save first entry, append after last...
			memcpy(&g_cycleTempPalette[count * 3], g_cycleTempPalette, 3);

			// Write back shifted by one (starts one entry later)...
			setPalette(count, endIdx, &g_cycleTempPalette[3]);
		}
	}
}

void XpLib::stopCycle() {
	Common::StackLock lock(g_paletteMutex);

	bool wasActive = false;
	for (int16 i = 0; i < 4; i++) {
		if (g_cycleStates[i].active) {
			wasActive = true;
			g_cycleStates[i].active = false;
		}
	}

	if (wasActive)
		_bolt->getTimerManager()->removeTimerProc(cycleColorsCallback);
}

void XpLib::setScreenBrightness(uint8 percent) {
	Common::StackLock lock(g_paletteMutex);

	if (percent >= 100) {
		g_brightnessShift = 2; // Full brightness (VGA DAC is 6-bit)
	} else if (percent >= 50) {
		g_brightnessShift = 3; // 50%
	} else if (percent >= 25) {
		g_brightnessShift = 4; // 25%
	} else if (percent >= 12) {
		g_brightnessShift = 5; // 12%
	} else {
		g_brightnessShift = 6; // Near black
	}

	// Re-apply entire palette with new brightness...
	setPalette(256, 0, g_paletteBuffer);
}

} // End of namespace Bolt
