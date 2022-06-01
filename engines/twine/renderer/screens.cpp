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

#include "twine/renderer/screens.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"
#include "image/bmp.h"
#include "twine/audio/music.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/twine.h"

namespace TwinE {

bool Screens::adelineLogo() {
	_engine->_music->playMidiMusic(31);

	return loadImageDelay(_engine->_resources->adelineLogo(), 7);
}

void Screens::loadMenuImage(bool fadeIn) {
	loadImage(_engine->_resources->menuBackground(), fadeIn);
	_engine->_workVideoBuffer.blitFrom(_engine->_frontVideoBuffer);
}

void Screens::loadCustomPalette(const TwineResource &resource) {
	const int32 size = HQR::getEntry(_palette, resource.hqr, resource.index);
	if (size == 0) {
		warning("Failed to load custom palette %s:%i", resource.hqr, resource.index);
		return;
	}
	if (size != (int32)sizeof(_palette)) {
		warning("Unexpected palette size %s:%i", resource.hqr, resource.index);
	}
	debug(3, "palette %s:%i with size %i", resource.hqr, resource.index, size);
	convertPalToRGBA(_palette, _paletteRGBACustom);
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

void Screens::loadImage(TwineImage image, bool fadeIn) {
	Graphics::ManagedSurface& src = _engine->_imageBuffer;
	if (HQR::getEntry((uint8 *)src.getPixels(), image.image) == 0) {
		warning("Failed to load image with index %i", image.image.index);
		return;
	}
	debug(0, "Load image: %i", image.image.index);
	Graphics::ManagedSurface& target = _engine->_frontVideoBuffer;
	target.transBlitFrom(src, src.getBounds(), target.getBounds(), 0, false, 0, 0xff, nullptr, true);
	const uint32 *pal = _paletteRGBA;
	if (image.palette.index != -1) {
		loadCustomPalette(image.palette);
		pal = _paletteRGBACustom;
	}
	if (fadeIn) {
		fadeToPal(pal);
	} else {
		_engine->setPalette(pal);
	}
}

bool Screens::loadImageDelay(TwineImage image, int32 seconds) {
	loadImage(image);
	if (_engine->delaySkip(1000 * seconds)) {
		adjustPalette(0, 0, 0, _paletteRGBACustom, 100);
		return true;
	}
	fadeOut(_paletteRGBACustom);
	return false;
}

bool Screens::loadBitmapDelay(const char *image, int32 seconds) {
	Common::File fileHandle;
	if (!fileHandle.open(image)) {
		warning("Failed to open %s", image);
		return false;
	}

	Image::BitmapDecoder bitmap;
	if (!bitmap.loadStream(fileHandle)) {
		warning("Failed to load %s", image);
		return false;
	}
	const Graphics::Surface *src = bitmap.getSurface();
	if (src == nullptr) {
		warning("Failed to decode %s", image);
		return false;
	}
	Graphics::ManagedSurface &target = _engine->_frontVideoBuffer;
	Common::Rect rect(src->w, src->h);
	_engine->setPalette(bitmap.getPaletteStartIndex(), bitmap.getPaletteColorCount(), bitmap.getPalette());
	target.transBlitFrom(*src, rect, target.getBounds(), 0, false, 0, 0xff, nullptr, true);
	if (_engine->delaySkip(1000 * seconds)) {
		return true;
	}
	return false;
}

void Screens::fadeIn(const uint32 *pal) {
	fadeToPal(pal);

	_engine->setPalette(pal);
}

void Screens::fadeOut(const uint32 *pal) {
	fadeToBlack(pal);
}

int32 Screens::lerp(int32 value, int32 start, int32 end, int32 t) {
	if (!end) {
		return start;
	}
	return (((start - value) * t) / end) + value;
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
		*newR = lerp(r, paletteIn[counter], 100, intensity);
		*newG = lerp(g, paletteIn[counter + 1], 100, intensity);
		*newB = lerp(b, paletteIn[counter + 2], 100, intensity);
		*newA = 0xFF;

		newR += 4;
		newG += 4;
		newB += 4;
		newA += 4;

		counter += 4;
	}

	_engine->setPalette(pal);
	_engine->_frontVideoBuffer.update();
}

void Screens::adjustCrossPalette(const uint32 *pal1, const uint32 *pal2) {
	uint32 pal[NUMOFCOLORS];

	int32 counter = 0;
	int32 intensity = 0;

	const uint8 *pal1p = (const uint8 *)pal1;
	const uint8 *pal2p = (const uint8 *)pal2;
	uint8 *paletteOut = (uint8 *)pal;
	do {
		FrameMarker frame(_engine, DEFAULT_HZ);
		counter = 0;

		uint8 *newR = &paletteOut[counter];
		uint8 *newG = &paletteOut[counter + 1];
		uint8 *newB = &paletteOut[counter + 2];
		uint8 *newA = &paletteOut[counter + 3];

		for (int32 i = 0; i < NUMOFCOLORS; i++) {
			*newR = lerp(pal1p[counter + 0], pal2p[counter + 0], 100, intensity);
			*newG = lerp(pal1p[counter + 1], pal2p[counter + 1], 100, intensity);
			*newB = lerp(pal1p[counter + 2], pal2p[counter + 2], 100, intensity);
			*newA = 0xFF;

			newR += 4;
			newG += 4;
			newB += 4;
			newA += 4;

			counter += 4;
		}

		_engine->setPalette(pal);
		intensity++;
		_engine->_frontVideoBuffer.update();
	} while (intensity <= 100);
}

void Screens::fadeToBlack(const uint32 *pal) {
	if (_palResetted) {
		return;
	}

	for (int32 i = 100; i >= 0; i -= 3) {
		FrameMarker frame(_engine, DEFAULT_HZ);
		adjustPalette(0, 0, 0, pal, i);
	}

	_palResetted = true;
}

void Screens::fadeToPal(const uint32 *pal) {
	for (int32 i = 0; i <= 100; i += 3) {
		FrameMarker frame(_engine, DEFAULT_HZ);
		adjustPalette(0, 0, 0, pal, i);
	}

	_engine->setPalette(pal);

	_palResetted = false;
}

void Screens::blackToWhite() {
	uint32 pal[NUMOFCOLORS];

	for (int32 i = 0; i < NUMOFCOLORS; i += 3) {
		memset(pal, i, sizeof(pal));

		_engine->setPalette(pal);
		_engine->_frontVideoBuffer.update();
	}
}

void Screens::setDarkPal() {
	ScopedEngineFreeze scoped(_engine);
	HQR::getEntry(_palette, Resources::HQR_RESS_FILE, RESSHQR_DARKPAL);
	convertPalToRGBA(_palette, _paletteRGBA);
	if (!_fadePalette) {
		// set the palette hard if it should not get faded
		_engine->setPalette(_paletteRGBA);
	}
	_useAlternatePalette = true;
}

void Screens::setNormalPal() {
	_useAlternatePalette = false;
	if (!_fadePalette) {
		// reset the palette hard if it should not get faded
		_engine->setPalette(_mainPaletteRGBA);
	}
}

void Screens::setBackPal() {
	memset(_palette, 0, sizeof(_palette));
	memset(_paletteRGBA, 0, sizeof(_paletteRGBA));

	_engine->setPalette(_paletteRGBA);

	_palResetted = true;
}

void Screens::fadePalRed(const uint32 *pal) {
	for (int32 i = 100; i >= 0; i -= 2) {
		FrameMarker frame(_engine, DEFAULT_HZ);
		adjustPalette(0xFF, 0, 0, pal, i);
	}
}

void Screens::fadeRedPal(const uint32 *pal) {
	for (int32 i = 0; i <= 100; i += 2) {
		FrameMarker frame(_engine, DEFAULT_HZ);
		adjustPalette(0xFF, 0, 0, pal, i);
	}
}

void Screens::copyScreen(const Graphics::ManagedSurface &source, Graphics::ManagedSurface &destination) {
	destination.blitFrom(source);
}

void Screens::clearScreen() {
	_engine->_frontVideoBuffer.clear(0);
}

} // namespace TwinE
