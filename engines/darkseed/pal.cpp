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

#include "graphics/paletteman.h"
#include "darkseed/pal.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

#define DARKSEED_NUM_PAL_ENTRIES 16
#define DARKSEED_PAL_SIZE (DARKSEED_NUM_PAL_ENTRIES * 3)

Pal::Pal(const Pal &pal) {
	load(pal);
}

Pal &Pal::operator=(const Pal &pal) {
	if (this != &pal) {
		load(pal);
	}
	return *this;
}

void Pal::load(const Pal &pal) {
	memcpy(_palData, pal._palData, DARKSEED_PAL_SIZE);
}

bool Pal::load(const Common::Path &filename, bool shouldInstallPalette) {
	Common::File file;
	if (!file.open(filename)) {
		loadFromScreen();
		return false;
	}
	uint32 bytesRead = file.read(_palData, DARKSEED_PAL_SIZE);
	assert(bytesRead == DARKSEED_PAL_SIZE);

	for (int i = 0; i < DARKSEED_PAL_SIZE; i++) {
		_palData[i] = _palData[i] << 2;
	}
	if (shouldInstallPalette) {
		installPalette();
	}
	return true;
}

void Pal::loadFromScreen() {
	g_system->getPaletteManager()->grabPalette(_palData, 0, DARKSEED_NUM_PAL_ENTRIES);
}

void Pal::clear() {
	memset(_palData, 0, DARKSEED_PAL_SIZE);
}

void Pal::updatePalette(int delta, const Pal &targetPal, bool shouldInstallPalette) {
	for (int i = 0; i < DARKSEED_PAL_SIZE; i++) {
		int c = _palData[i] + delta;
		if (c < 0) {
			c = 0;
		} else if (delta > 0 && c > targetPal._palData[i]) {
			c = targetPal._palData[i];
		}
		_palData[i] = (uint8)c;
	}
	if (shouldInstallPalette) {
		installPalette();
	}
}

void Pal::installPalette() const {
	g_system->getPaletteManager()->setPalette(_palData, 0, DARKSEED_NUM_PAL_ENTRIES);
}

} // namespace Darkseed
