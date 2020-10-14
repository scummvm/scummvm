/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "twine/screens.h"
#include "twine/hqrdepack.h"
#include "twine/music.h"
#include "twine/resources.h"
#include "twine/twine.h"

namespace TwinE {

void Screens::adelineLogo() {
	_engine->_music->playMidiMusic(31, 0);

	loadImage(RESSHQR_ADELINEIMG);
	_engine->delaySkip(7000);
	fadeOut(paletteRGBCustom);
	palCustom = 1;
}

void Screens::loadMenuImage(bool fade_in) {
	_engine->_hqrdepack->hqrGetEntry(_engine->workVideoBuffer, Resources::HQR_RESS_FILE, RESSHQR_MENUIMG);
	copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	if (fade_in) {
		fadeToPal(paletteRGB);
	} else {
		_engine->setPalette(paletteRGB);
	}

	palCustom = 0;
}

void Screens::loadCustomPalette(int32 index) {
	_engine->_hqrdepack->hqrGetEntry(palette, Resources::HQR_RESS_FILE, index);
	copyPal(palette, paletteRGBCustom);
}

void Screens::copyPal(const uint8* in, uint8* out) {
	memcpy(out, in, NUMOFCOLORS * 3);
}

void Screens::loadImage(int32 index, bool fade_in) {
	_engine->_hqrdepack->hqrGetEntry(_engine->workVideoBuffer, Resources::HQR_RESS_FILE, index);
	copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	loadCustomPalette(index + 1);
	if (fade_in) {
		fadeToPal(paletteRGBCustom);
	} else {
		_engine->setPalette(paletteRGBCustom);
	}

	palCustom = 1;
}

void Screens::loadImageDelay(int32 index, int32 time) {
	loadImage(index);
	_engine->delaySkip(1000 * time);
	fadeOut(paletteRGBCustom);
}

void Screens::fadeIn(uint8 *pal) {
	if (_engine->cfgfile.CrossFade)
		_engine->crossFade(_engine->frontVideoBuffer, pal);
	else
		fadeToPal(pal);

	_engine->setPalette(pal);
}

void Screens::fadeOut(uint8 *pal) {
	/*if(cfgfile.CrossFade)
		crossFade(frontVideoBuffer, palette);
	else
		fadeToBlack(palette);*/
	if (!_engine->cfgfile.CrossFade)
		fadeToBlack(pal);
}

int32 Screens::crossDot(int32 modifier, int32 color, int32 param, int32 intensity) {
	if (!param)
		return (color);
	return (((color - modifier) * intensity) / param) + modifier;
}

void Screens::adjustPalette(uint8 R, uint8 G, uint8 B, uint8 *pal, int32 intensity) {
	uint8 localPalette[NUMOFCOLORS * 3];

	int32 counter = 0;

	uint8 *newR = &localPalette[0];
	uint8 *newG = &localPalette[1];
	uint8 *newB = &localPalette[2];

	for (int32 i = 0; i < NUMOFCOLORS; i++) {
		*newR = crossDot(R, pal[counter], 100, intensity);
		*newG = crossDot(G, pal[counter + 1], 100, intensity);
		*newB = crossDot(B, pal[counter + 2], 100, intensity);

		newR += 3;
		newG += 3;
		newB += 3;

		counter += 3;
	}

	_engine->setPalette(localPalette);
}

void Screens::adjustCrossPalette(uint8 *pal1, uint8 *pal2) {
	uint8 localPalette[NUMOFCOLORS * 4];

	uint8 *newR;
	uint8 *newG;
	uint8 *newB;
	uint8 *newA;

	int32 i;
	int32 counter = 0;
	int32 intensity = 0;

	do {
		counter = 0;

		newR = &localPalette[counter];
		newG = &localPalette[counter + 1];
		newB = &localPalette[counter + 2];
		newA = &localPalette[counter + 3];

		for (i = 0; i < NUMOFCOLORS; i++) {
			*newR = crossDot(pal1[counter], pal2[counter], 100, intensity);
			*newG = crossDot(pal1[counter + 1], pal2[counter + 1], 100, intensity);
			*newB = crossDot(pal1[counter + 2], pal2[counter + 2], 100, intensity);
			*newA = 0;

			newR += 4;
			newG += 4;
			newB += 4;
			newA += 4;

			counter += 4;
		}

		_engine->setPalette(localPalette);
		_engine->_system->delayMillis(1000 / 50);

		intensity++;
	} while (intensity <= 100);
}

void Screens::fadeToBlack(uint8 *pal) {
	int32 i = 0;

	if (palReseted == 0) {
		for (i = 100; i >= 0; i -= 3) {
			adjustPalette(0, 0, 0, (uint8 *)pal, i);
			_engine->_system->delayMillis(1000 / 50);
		}
	}

	palReseted = 1;
}

void Screens::fadeToPal(uint8 *pal) {
	int32 i = 100;

	for (i = 0; i <= 100; i += 3) {
		adjustPalette(0, 0, 0, (uint8 *)pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}

	_engine->setPalette((uint8 *)pal);

	palReseted = 0;
}

void Screens::blackToWhite() {
	uint8 pal[NUMOFCOLORS * 4];

	for (int32 i = 0; i < NUMOFCOLORS; i += 3) {
		memset(pal, i, sizeof(pal));

		_engine->setPalette(pal);
	}
}

void Screens::setBackPal() {
	memset(palette, 0, sizeof(palette));
	memset(paletteRGB, 0, sizeof(paletteRGB));

	_engine->setPalette(paletteRGB);

	palReseted = 1;
}

void Screens::fadePalRed(uint8 *pal) {
	int32 i = 100;

	for (i = 100; i >= 0; i -= 2) {
		adjustPalette(0xFF, 0, 0, (uint8 *)pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}
}

void Screens::fadeRedPal(uint8 *pal) {
	int32 i = 0;

	for (i = 0; i <= 100; i += 2) {
		adjustPalette(0xFF, 0, 0, (uint8 *)pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}
}

void Screens::copyScreen(const uint8 *source, uint8 *destination) {
	int32 w, h;

	if (SCALE == 1) {
		memcpy(destination, source, SCREEN_WIDTH * SCREEN_HEIGHT);
	} else if (SCALE == 2) {
		for (h = 0; h < SCREEN_HEIGHT / SCALE; h++) {
			for (w = 0; w < SCREEN_WIDTH / SCALE; w++) {
				*destination++ = *source;
				*destination++ = *source++;
			}
			memcpy(destination, destination - SCREEN_WIDTH, SCREEN_WIDTH);
			destination += SCREEN_WIDTH;
		}
	}
}

void Screens::clearScreen() {
	memset(_engine->frontVideoBuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
}

} // namespace TwinE
