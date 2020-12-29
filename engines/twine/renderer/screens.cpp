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

#include "twine/renderer/screens.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "twine/audio/music.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/twine.h"

namespace TwinE {

bool Screens::adelineLogo() {
	_engine->_music->playMidiMusic(31);

	return loadImageDelay(RESSHQR_ADELINEIMG, RESSHQR_ADELINEPAL, 7);
}

void Screens::loadMenuImage(bool fadeIn) {
	loadImage(RESSHQR_MENUIMG, -1, fadeIn);
	_engine->workVideoBuffer.blitFrom(_engine->frontVideoBuffer);
}

void Screens::loadCustomPalette(int32 index) {
	if (HQR::getEntry(palette, Resources::HQR_RESS_FILE, index) == 0) {
		warning("Failed to load custom palette %i", index);
		return;
	}
	convertPalToRGBA(palette, paletteRGBACustom);
}

void Screens::convertPalToRGBA(const uint8 *in, uint32 *out) {
	uint8 *paletteOut = (uint8 *)out;
	for (int i = 0; i < NUMOFCOLORS; i++) {
		paletteOut[0] = in[0];
		paletteOut[1] = in[1];
		paletteOut[2] = in[2];
		paletteOut[3] = 0xFF;
		paletteOut += 4;
		in += 3;
	}
}

void Screens::loadImage(int32 index, int32 paletteIndex, bool fadeIn) {
	Graphics::ManagedSurface& src = _engine->imageBuffer;
	if (HQR::getEntry((uint8 *)src.getPixels(), Resources::HQR_RESS_FILE, index) == 0) {
		warning("Failed to load image with index %i", index);
		return;
	}
	debug(0, "Load image: %i", index);
	Graphics::ManagedSurface& target = _engine->frontVideoBuffer;
	target.transBlitFrom(src, src.getBounds(), target.getBounds(), 0, false, 0, 0xff, nullptr, true);
	const uint32 *pal = paletteRGBA;
	if (paletteIndex != -1) {
		loadCustomPalette(paletteIndex);
		pal = paletteRGBACustom;
	}
	if (fadeIn) {
		fadeToPal(pal);
	} else {
		_engine->setPalette(pal);
	}
}

bool Screens::loadImageDelay(int32 index, int32 paletteIndex, int32 seconds) {
	loadImage(index, paletteIndex);
	if (_engine->delaySkip(1000 * seconds)) {
		adjustPalette(0, 0, 0, paletteRGBACustom, 100);
		return true;
	}
	fadeOut(paletteRGBACustom);
	return false;
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
		crossFade(frontVideoBuffer, pal);
	else
		fadeToBlack(pal);*/
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

void Screens::adjustPalette(uint8 r, uint8 g, uint8 b, const uint32 *rgbaPal, int32 intensity) {
	uint32 pal[NUMOFCOLORS];

	int32 counter = 0;

	const uint8 *paletteIn = (const uint8 *)rgbaPal;
	uint8 *paletteOut = (uint8 *)pal;
	uint8 *newR = &paletteOut[0];
	uint8 *newG = &paletteOut[1];
	uint8 *newB = &paletteOut[2];
	uint8 *newA = &paletteOut[3];

	for (int32 i = 0; i < NUMOFCOLORS; i++) {
		*newR = crossDot(r, paletteIn[counter], 100, intensity);
		*newG = crossDot(g, paletteIn[counter + 1], 100, intensity);
		*newB = crossDot(b, paletteIn[counter + 2], 100, intensity);
		*newA = 0xFF;

		newR += 4;
		newG += 4;
		newB += 4;
		newA += 4;

		counter += 4;
	}

	_engine->setPalette(pal);
}

void Screens::adjustCrossPalette(const uint32 *pal1, const uint32 *pal2) {
	uint32 pal[NUMOFCOLORS];

	int32 counter = 0;
	int32 intensity = 0;

	const uint8 *pal1p = (const uint8 *)pal1;
	const uint8 *pal2p = (const uint8 *)pal2;
	uint8 *paletteOut = (uint8 *)pal;
	do {
		ScopedFPS scopedFps(50);
		counter = 0;

		uint8 *newR = &paletteOut[counter];
		uint8 *newG = &paletteOut[counter + 1];
		uint8 *newB = &paletteOut[counter + 2];
		uint8 *newA = &paletteOut[counter + 3];

		for (int32 i = 0; i < NUMOFCOLORS; i++) {
			*newR = crossDot(pal1p[counter + 0], pal2p[counter + 0], 100, intensity);
			*newG = crossDot(pal1p[counter + 1], pal2p[counter + 1], 100, intensity);
			*newB = crossDot(pal1p[counter + 2], pal2p[counter + 2], 100, intensity);
			*newA = 0xFF;

			newR += 4;
			newG += 4;
			newB += 4;
			newA += 4;

			counter += 4;
		}

		_engine->setPalette(pal);
		intensity++;
	} while (intensity <= 100);
}

void Screens::fadeToBlack(const uint32 *pal) {
	if (palResetted) {
		return;
	}

	for (int32 i = 100; i >= 0; i -= 3) {
		ScopedFPS scopedFps(50);
		adjustPalette(0, 0, 0, pal, i);
	}

	palResetted = true;
}

void Screens::fadeToPal(const uint32 *pal) {
	for (int32 i = 0; i <= 100; i += 3) {
		ScopedFPS scopedFps(50);
		adjustPalette(0, 0, 0, pal, i);
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

void Screens::fadePalRed(const uint32 *pal) {
	for (int32 i = 100; i >= 0; i -= 2) {
		ScopedFPS scopedFps(50);
		adjustPalette(0xFF, 0, 0, pal, i);
	}
}

void Screens::fadeRedPal(const uint32 *pal) {
	for (int32 i = 0; i <= 100; i += 2) {
		ScopedFPS scopedFps(50);
		adjustPalette(0xFF, 0, 0, pal, i);
	}
}

void Screens::copyScreen(const Graphics::ManagedSurface &source, Graphics::ManagedSurface &destination) {
	destination.blitFrom(source);
}

void Screens::clearScreen() {
	_engine->frontVideoBuffer.clear(0);
}

} // namespace TwinE
