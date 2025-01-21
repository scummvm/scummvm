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

#include "got/gfx/palette.h"
#include "common/events.h"
#include "common/system.h"
#include "got/got.h"
#include "got/utils/file.h"
#include "got/vars.h"
#include "graphics/paletteman.h"

namespace Got {
namespace Gfx {

#define FADE_STEPS 10

static byte saved_palette[PALETTE_SIZE];

Palette63::Palette63(const byte *pal) {
	for (uint i = 0; i < PALETTE_SIZE; ++i)
		_pal[i] = pal[i] << 2;
}

void loadPalette() {
	if (resourceRead("PALETTE", saved_palette) < 0)
		error("Cannot Read PALETTE");

	g_system->getPaletteManager()->setPalette(saved_palette, 0, 256);
}

void setScreenPal() {
	byte pal[3];

	xGetPal(pal, 1, _G(scrn)._palColors[0]);
	xSetPal(251, pal[0], pal[1], pal[2]);
	xGetPal(pal, 1, _G(scrn)._palColors[1]);
	xSetPal(252, pal[0], pal[1], pal[2]);
	xGetPal(pal, 1, _G(scrn)._palColors[2]);
	xSetPal(253, pal[0], pal[1], pal[2]);
}

void xSetPal(const byte color, const byte R, const byte G, const byte B) {
	byte rgb[3] = {R, G, B};
	g_system->getPaletteManager()->setPalette(rgb, color, 1);
}

void xSetPal(const byte *pal) {
	g_system->getPaletteManager()->setPalette(pal, 0, PALETTE_COUNT);
}

void setPalette(const byte *pal) {
	xSetPal(pal);
	Common::copy(pal, pal + PALETTE_SIZE, saved_palette);
}

void xGetPal(byte *pal, const int numColors, const int startIndex) {
	g_system->getPaletteManager()->grabPalette(pal, startIndex, numColors);
}

void fadeOut() {
	byte tempPal[PALETTE_SIZE];
	const byte *srcP;
	byte *destP;
	int count;
	Common::Event evt;

	xGetPal(saved_palette, PALETTE_COUNT, 0);

	for (int step = FADE_STEPS - 1; step >= 0; --step) {
		// Set each palette RGB proportionately
		for (srcP = &saved_palette[0], destP = &tempPal[0], count = 0;
			 count < PALETTE_SIZE; ++count, ++srcP, ++destP) {
			*destP = *srcP * step / FADE_STEPS;
		}

		// Set new palette
		xSetPal(tempPal);

		// Use up any pending events and update the screen
		while (g_system->getEventManager()->pollEvent(evt)) {
			if (evt.type == Common::EVENT_QUIT)
				return;
		}

		g_events->getScreen()->update();
		g_system->delayMillis(10);
	}
}

void fadeIn(const byte *pal) {
	byte tempPal[PALETTE_SIZE];
	const byte *srcP;
	byte *destP;
	int count;
	Common::Event evt;

	if (pal)
		Common::copy(pal, pal + PALETTE_SIZE, saved_palette);

	// Start with a black palette
	Common::fill(tempPal, tempPal + PALETTE_SIZE, 0);
	xSetPal(tempPal);

	for (int step = 1; step <= FADE_STEPS; ++step) {
		// Set each palette RGB proportionately
		for (srcP = &saved_palette[0], destP = &tempPal[0], count = 0;
			 count < PALETTE_SIZE; ++count, ++srcP, ++destP) {
			*destP = *srcP * step / FADE_STEPS;
		}

		// Set new palette
		xSetPal(tempPal);

		// Use up any pending events and update the screen
		while (g_system->getEventManager()->pollEvent(evt)) {
			if (evt.type == Common::EVENT_QUIT)
				return;
		}

		g_events->getScreen()->update();
		g_system->delayMillis(10);
	}
}

} // namespace Gfx
} // namespace Got
