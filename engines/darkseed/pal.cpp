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

#include "pal.h"
#include "darkseed.h"
#include "graphics/paletteman.h"

namespace Darkseed {

#define DARKSEED_NUM_PAL_ENTRIES 16
#define DARKSEED_PAL_SIZE DARKSEED_NUM_PAL_ENTRIES * 3

Pal::Pal(const Pal &pal) {
	load(pal);
}

void Pal::load(const Pal &pal) {
	memcpy(palData, pal.palData, DARKSEED_PAL_SIZE);
}

bool Pal::load(const Common::Path &filename, bool shouldInstallPalette) {
	Common::File file;
	if(!file.open(filename)) {
		return false;
	}
	uint32 bytesRead = file.read(palData, DARKSEED_PAL_SIZE);
	assert(bytesRead == DARKSEED_PAL_SIZE);

	for (int i=0; i < DARKSEED_PAL_SIZE; i++) {
		palData[i] = palData[i] << 2;
	}
	if (shouldInstallPalette) {
		installPalette();
	}
	return true;
}

void Pal::loadFromScreen() {
	g_system->getPaletteManager()->grabPalette(palData, 0, DARKSEED_NUM_PAL_ENTRIES);
}

void Pal::clear() {
	memset(palData, 0, DARKSEED_PAL_SIZE);
}

void Pal::updatePalette(int delta, const Pal &targetPal, bool shouldInstallPalette) {
	for (int i = 0; i < DARKSEED_PAL_SIZE; i++) {
		int c = palData[i] + delta;
		if (c < 0) {
			c = 0;
		} else if (delta > 0 && c > targetPal.palData[i]) {
			c = targetPal.palData[i];
		}
		palData[i] = (uint8)c;
	}
	if (shouldInstallPalette) {
		installPalette();
	}
}

void Pal::installPalette() {
	g_system->getPaletteManager()->setPalette(palData, 0, DARKSEED_NUM_PAL_ENTRIES);
}

} // namespace Darkseed