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
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "image/bmp.h"
#include "image/image_decoder.h"
#include "image/png.h"
#include "twine/audio/music.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

int32 Screens::mapLba2Palette(int32 palIndex) {
	static const int32 palettes[] {
		RESSHQR_MAINPAL,
		-1, // TODO: current palette
		RESSHQR_BLACKPAL,
		RESSHQR_ECLAIRPAL
	};
	if (palIndex < 0 || palIndex >= ARRAYSIZE(palettes)) {
		return -1;
	}
	return palettes[palIndex];
}

bool Screens::adelineLogo() {
	_engine->_music->playMidiFile(31);

	// TODO: whiteFade and fadeWhiteToPal
	return loadImageDelay(_engine->_resources->adelineLogo(), 7);
}

void Screens::loadMenuImage(bool fadeIn) {
	loadImage(_engine->_resources->menuBackground(), fadeIn);
	_engine->_workVideoBuffer.blitFrom(_engine->_frontVideoBuffer);
}

void Screens::loadCustomPalette(const TwineResource &resource) {
	if (!HQR::getPaletteEntry(_palettePcx, resource)) {
		error("Failed to get palette entry for custom palette: %s:%d", resource.hqr, resource.index);
	}

	if (_palettePcx.size() != NUMOFCOLORS) {
		warning("Unexpected palette size %s:%i", resource.hqr, resource.index);
	}
}

void Screens::loadImage(TwineImage image, bool fadeIn) {
	Graphics::ManagedSurface& src = _engine->_imageBuffer;
	if (HQR::getEntry((uint8 *)src.getPixels(), image.image) == 0) {
		warning("Failed to load image with index %i", image.image.index);
		return;
	}
	debugC(1, TwinE::kDebugResources, "Load image: %i", image.image.index);
	Graphics::ManagedSurface& target = _engine->_frontVideoBuffer;
	target.transBlitFrom(src, src.getBounds(), target.getBounds(), 0, false, 0xff, nullptr, true);
	const Graphics::Palette *pal = &_ptrPal;
	if (image.palette.index != -1) {
		loadCustomPalette(image.palette);
		pal = &_palettePcx;
	}
	if (fadeIn) {
		fadeToPal(*pal);
	} else {
		_engine->setPalette(*pal);
	}
}

bool Screens::loadImageDelay(TwineImage image, int32 seconds) {
	loadImage(image);
	if (_engine->delaySkip(1000 * seconds)) {
		fadePal(0, 0, 0, _palettePcx, 100);
		return true;
	}
	fadeToBlack(_palettePcx);
	return false;
}

template<class ImageDecoder>
static bool loadImageDelayViaDecoder(TwinEEngine *engine, const Common::Path &fileName, int32 seconds) {
	ImageDecoder decoder;
	Common::File fileHandle;
	if (!fileHandle.open(fileName)) {
		warning("Failed to open %s", fileName.toString().c_str());
		return false;
	}
	if (!decoder.loadStream(fileHandle)) {
		warning("Failed to load %s", fileName.toString().c_str());
		return false;
	}
	const Graphics::Surface *src = decoder.getSurface();
	if (src == nullptr) {
		warning("Failed to decode %s", fileName.toString().c_str());
		return false;
	}
	Graphics::ManagedSurface &target = engine->_frontVideoBuffer;
	Common::Rect rect(src->w, src->h);
	if (decoder.getPaletteColorCount() == 0) {
		uint8 pal[PALETTE_SIZE];
		engine->_frontVideoBuffer.getPalette(pal, 0, 256);
		Graphics::Surface *source = decoder.getSurface()->convertTo(target.format, nullptr, 0, pal, 256);
		target.blitFrom(*source, rect, target.getBounds());
		source->free();
		delete source;
	} else {
		engine->setPalette(0, decoder.getPaletteColorCount(), decoder.getPalette());
		target.transBlitFrom(*src, rect, target.getBounds(), 0, false, 0xff, nullptr, true);
	}
	if (engine->delaySkip(1000 * seconds)) {
		return true;
	}
	return false;
}

bool Screens::loadBitmapDelay(const char *image, int32 seconds) {
	Common::Path path(image);
	Common::String filename = path.baseName();
	size_t extPos = filename.rfind(".");
	if (extPos == Common::String::npos) {
		warning("Failed to extract extension %s", image);
		return false;
	}

	struct ImageLoader {
		const char *extension;
		bool (*loadImageDelay)(TwinEEngine *engine, const Common::Path &fileName, int32 seconds);
	};

	static const ImageLoader s_imageLoaders[] = {
		{ "bmp", loadImageDelayViaDecoder<Image::BitmapDecoder> },
		{ "png", loadImageDelayViaDecoder<Image::PNGDecoder> },
		{ nullptr, nullptr }
	};
	const Common::String &ext = filename.substr(extPos + 1);
	for (const ImageLoader *loader = s_imageLoaders; loader->extension; ++loader) {
		if (!scumm_stricmp(loader->extension, ext.c_str())) {
			return loader->loadImageDelay(_engine, path, seconds);
		}
	}
	warning("Failed to find suitable image handler %s", image);
	return false;
}

void Screens::fadePal(uint8 r, uint8 g, uint8 b, const Graphics::Palette &rgbaPal, int32 intensity) {
	Graphics::Palette pal{NUMOFCOLORS};

	for (int32 i = 0; i < NUMOFCOLORS; i++) {
		byte rIn, gIn, bIn;
		rgbaPal.get(i, rIn, gIn, bIn);
		const byte newR = ruleThree32(r, rIn, 100, intensity);
		const byte newG = ruleThree32(g, gIn, 100, intensity);
		const byte newB = ruleThree32(b, bIn, 100, intensity);
		pal.set(i, newR, newG, newB);
	}

	_engine->setPalette(pal);
	_engine->_frontVideoBuffer.update();
}

void Screens::fadeToBlack(const Graphics::Palette &ptrpal) {
	if (_flagBlackPal) {
		return;
	}

	for (int32 n = 100; n >= 0; n -= 2) {
		FrameMarker frame(_engine, 66); // VSync()
		fadePal(0, 0, 0, ptrpal, n);
	}

	_flagBlackPal = true;
}

void Screens::whiteFade() {
	Graphics::Palette workpal{NUMOFCOLORS};

	for (int32 n = 0; n <= 255; n++) {
		FrameMarker frame(_engine, 66); // VSync()
		for (int i = 0; i < NUMOFCOLORS; i++) {
			workpal.set(i, n, n, n);
		}

		_engine->setPalette(workpal);
		_engine->_frontVideoBuffer.update();
	}
}

void Screens::fadeWhiteToPal(const Graphics::Palette &ptrpal) {
	for (int32 n = 0; n <= 100; ++n) {
		FrameMarker frame(_engine, 66); // VSync()
		fadePal(255, 255, 255, ptrpal, n);
	}
}

void Screens::fadeToPal(const Graphics::Palette &ptrpal) {
	for (int32 i = 0; i <= 100; i += 3) {
		FrameMarker frame(_engine, 66); // VSync()
		fadePal(0, 0, 0, ptrpal, i);
	}

	_engine->setPalette(ptrpal);

	_flagBlackPal = false;
}

void Screens::setBlackPal() {
	Graphics::Palette workPal(NUMOFCOLORS);

	_engine->setPalette(workPal);

	_flagBlackPal = true;
}

void Screens::fadePalToPal(const Graphics::Palette &ptrpal, const Graphics::Palette &ptrpal2) {
	Graphics::Palette workpal{NUMOFCOLORS};

	for (int m = 0; m < 100; ++m) {
		FrameMarker frame(_engine, 66); // VSync()
		for (int32 i = 0; i < NUMOFCOLORS; i++) {
			byte r1, g1, b1;
			ptrpal.get(i, r1, g1, b1);

			byte r2, g2, b2;
			ptrpal2.get(i, r2, g2, b2);

			byte newR = ruleThree32(r1, r2, 100, m);
			byte newG = ruleThree32(g1, g2, 100, m);
			byte newB = ruleThree32(b1, b2, 100, m);
			workpal.set(i, newR, newG, newB);
		}

		_engine->setPalette(workpal);
		_engine->_frontVideoBuffer.update();
	}
}

void Screens::fadeToRed(const Graphics::Palette &ptrpal) {
	for (int32 i = 100; i >= 0; i -= 2) {
		FrameMarker frame(_engine, 66); // VSync()
		fadePal(255, 0, 0, ptrpal, i);
	}
}

void Screens::fadeRedToPal(const Graphics::Palette &ptrpal) {
	for (int32 i = 0; i <= 100; i += 2) {
		FrameMarker frame(_engine, 66); // VSync()
		fadePal(255, 0, 0, ptrpal, i);
	}
}

void Screens::copyScreen(const Graphics::ManagedSurface &source, Graphics::ManagedSurface &destination) {
	destination.blitFrom(source);
}

void Screens::clearScreen() {
	_engine->_frontVideoBuffer.clear(0);
}

} // namespace TwinE
