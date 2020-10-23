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
#include "graphics/surface.h"
#include "twine/screens.h"
#include "twine/hqrdepack.h"
#include "twine/music.h"
#include "twine/resources.h"
#include "twine/twine.h"

namespace TwinE {

void Screens::adelineLogo() {
	_engine->_music->playMidiMusic(31);

	loadImageDelay(RESSHQR_ADELINEIMG, 7);
	palCustom = true;
}

void Screens::loadMenuImage(bool fade_in) {
	if (_engine->_hqrdepack->hqrGetEntry((uint8*)_engine->workVideoBuffer.getPixels(), Resources::HQR_RESS_FILE, RESSHQR_MENUIMG) == 0) {
		warning("Failed to load menu image");
		return;
	}
	copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	if (fade_in) {
		fadeToPal(paletteRGBA);
	} else {
		_engine->setPalette(paletteRGBA);
	}

	palCustom = false;
}

void Screens::loadCustomPalette(int32 index) {
	if (_engine->_hqrdepack->hqrGetEntry(palette, Resources::HQR_RESS_FILE, index) == 0) {
		warning("Failed to load custom palette %i", index);
		return;
	}
	convertPalToRGBA(palette, paletteRGBACustom);
}

void Screens::convertPalToRGBA(const uint8* in, uint32* out) {
	uint8* palDest = (uint8*)out;
	for (int i = 0; i < NUMOFCOLORS; i++) {
		palDest[0] = in[0];
		palDest[1] = in[1];
		palDest[2] = in[2];
		palDest += 4;
		in += 3;
	}
}

void Screens::loadImage(int32 index, bool fade_in) {
	if (_engine->_hqrdepack->hqrGetEntry((uint8*)_engine->workVideoBuffer.getPixels(), Resources::HQR_RESS_FILE, index) == 0) {
		warning("Failed to load image with index %i", index);
		return;
	}
	debug("Load image: %i", index);
	copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	loadCustomPalette(index + 1);
	if (fade_in) {
		fadeToPal(paletteRGBACustom);
	} else {
		_engine->setPalette(paletteRGBACustom);
	}

	palCustom = true;
}

void Screens::loadImageDelay(int32 index, int32 time) {
	loadImage(index);
	_engine->delaySkip(1000 * time);
	fadeOut(paletteRGBACustom);
}

void Screens::fadeIn(uint32 *pal) {
	if (_engine->cfgfile.CrossFade) {
		_engine->crossFade(_engine->frontVideoBuffer, pal);
	} else {
		fadeToPal(pal);
	}

	_engine->setPalette(pal);
}

void Screens::fadeOut(uint32 *pal) {
	/*if(cfgfile.CrossFade)
		crossFade(frontVideoBuffer, palette);
	else
		fadeToBlack(palette);*/
	if (!_engine->cfgfile.CrossFade) {
		fadeToBlack(pal);
	}
}

int32 Screens::crossDot(int32 modifier, int32 color, int32 param, int32 intensity) {
	if (!param) {
		return color;
	}
	return (((color - modifier) * intensity) / param) + modifier;
}

void Screens::adjustPalette(uint8 R, uint8 G, uint8 B, const uint32 *rgbaPal, int32 intensity) {
	uint32 pal2[NUMOFCOLORS];

	int32 counter = 0;

	const uint8 *pal = (const uint8*)rgbaPal;
	uint8 *localPalette = (uint8*)pal2;
	uint8 *newR = &localPalette[0];
	uint8 *newG = &localPalette[1];
	uint8 *newB = &localPalette[2];
	uint8 *newA = &localPalette[3];

	for (int32 i = 0; i < NUMOFCOLORS; i++) {
		*newR = crossDot(R, pal[counter], 100, intensity);
		*newG = crossDot(G, pal[counter + 1], 100, intensity);
		*newB = crossDot(B, pal[counter + 2], 100, intensity);
		*newA = 0;

		newR += 4;
		newG += 4;
		newB += 4;
		newA += 4;

		counter += 4;
	}

	_engine->setPalette(pal2);
}

void Screens::adjustCrossPalette(const uint32 *pal1, const uint32 *pal2) {
	uint32 pal[NUMOFCOLORS];

	int32 i;
	int32 counter = 0;
	int32 intensity = 0;

	const uint8 *pal1p = (const uint8*)pal1;
	const uint8 *pal2p = (const uint8*)pal2;
	uint8 *localPalette = (uint8*)pal;
	do {
		counter = 0;

		uint8 *newR = &localPalette[counter];
		uint8 *newG = &localPalette[counter + 1];
		uint8 *newB = &localPalette[counter + 2];
		uint8 *newA = &localPalette[counter + 3];

		for (i = 0; i < NUMOFCOLORS; i++) {
			*newR = crossDot(pal1p[counter], pal2p[counter], 100, intensity);
			*newG = crossDot(pal1p[counter + 1], pal2p[counter + 1], 100, intensity);
			*newB = crossDot(pal1p[counter + 2], pal2p[counter + 2], 100, intensity);
			*newA = 0;

			newR += 4;
			newG += 4;
			newB += 4;
			newA += 4;

			counter += 4;
		}

		_engine->setPalette(pal);
		_engine->_system->delayMillis(1000 / 50);

		intensity++;
	} while (intensity <= 100);
}

void Screens::fadeToBlack(uint32 *pal) {
	if (palResetted) {
		return;
	}

	for (int32 i = 100; i >= 0; i -= 3) {
		adjustPalette(0, 0, 0, pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}

	palResetted = true;
}

void Screens::fadeToPal(uint32 *pal) {
	for (int32 i = 0; i <= 100; i += 3) {
		adjustPalette(0, 0, 0, pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}

	_engine->setPalette(pal);

	palResetted = false;
}

void Screens::blackToWhite() {
	uint32 pal[NUMOFCOLORS];

	for (int32 i = 0; i < NUMOFCOLORS; i += 3) {
		memset(pal, i, sizeof(pal));

		_engine->setPalette(pal);
	}
}

void Screens::setBackPal() {
	memset(palette, 0, sizeof(palette));
	memset(paletteRGBA, 0, sizeof(paletteRGBA));

	_engine->setPalette(paletteRGBA);

	palResetted = true;
}

void Screens::fadePalRed(uint32 *pal) {
	for (int32 i = 100; i >= 0; i -= 2) {
		adjustPalette(0xFF, 0, 0, pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}
}

void Screens::fadeRedPal(uint32 *pal) {
	for (int32 i = 0; i <= 100; i += 2) {
		adjustPalette(0xFF, 0, 0, pal, i);
		_engine->_system->delayMillis(1000 / 50);
	}
}

void Screens::copyScreen(const uint8 *source, uint8 *destination) {
	if (SCALE == 1) {
		memcpy(destination, source, SCREEN_WIDTH * SCREEN_HEIGHT);
	} else if (SCALE == 2) {
		for (int32 h = 0; h < SCREEN_HEIGHT / SCALE; h++) {
			for (int32 w = 0; w < SCREEN_WIDTH / SCALE; w++) {
				*destination++ = *source;
				*destination++ = *source++;
			}
			memcpy(destination, destination - SCREEN_WIDTH, SCREEN_WIDTH);
			destination += SCREEN_WIDTH;
		}
	}
}

void Screens::copyScreen(const Graphics::ManagedSurface& source, Graphics::ManagedSurface &destination) {
	copyScreen((const uint8 *)source.getPixels(), (uint8 *)destination.getPixels());
}

void Screens::clearScreen() {
	memset(_engine->frontVideoBuffer.getPixels(), 0, SCREEN_WIDTH * SCREEN_HEIGHT);
}

} // namespace TwinE
