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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "sherlock/screen.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/3do/scalpel_3do_screen.h"
#include "sherlock/tattoo/tattoo_screen.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/pixelformat.h"
#include "graphics/palette.h"
#include "graphics/cursorman.h"
#include "image/png.h"
#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#include "graphics/font.h"
#endif

#include <cstdlib>
#include <cstring>

namespace Sherlock {

enum RoseTattooHiresDebugMode {
	kRoseTattooHiresComposite = 0,
	kRoseTattooHiresBackground,
	kRoseTattooHiresMask,
	kRoseTattooHiresNative
};

Screen *Screen::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_RoseTattoo)
		return new Tattoo::TattooScreen(vm);
	else if (vm->getPlatform() == Common::kPlatform3DO)
		return new Scalpel::Scalpel3DOScreen(vm);
	else
		return new Scalpel::ScalpelScreen(vm);
}

static int roseTattooHiresScaleFromEnv() {
	const char *scaleEnv = getenv("SCUMMVM_SHERLOCK_TATTOO_HIRES_SCALE");
	if (!scaleEnv || !*scaleEnv)
		return 1;

	int scale = atoi(scaleEnv);
	return scale > 1 ? scale : 1;
}

static int roseTattooHiresDebugModeFromEnv() {
	const char *mode = getenv("SCUMMVM_SHERLOCK_TATTOO_HIRES_DEBUG");
	if (!mode || !*mode || !strcmp(mode, "composite"))
		return kRoseTattooHiresComposite;
	if (!strcmp(mode, "background"))
		return kRoseTattooHiresBackground;
	if (!strcmp(mode, "mask"))
		return kRoseTattooHiresMask;
	if (!strcmp(mode, "native"))
		return kRoseTattooHiresNative;

	warning("Unknown Rose Tattoo hires debug mode '%s'; using composite", mode);
	return kRoseTattooHiresComposite;
}

static Graphics::PixelFormat roseTattooHiresFormatFromEnv() {
	const char *formatEnv = getenv("SCUMMVM_SHERLOCK_TATTOO_HIRES_FORMAT");
	if (!formatEnv || !*formatEnv || !strcmp(formatEnv, "clut8"))
		return Graphics::PixelFormat::createFormatCLUT8();
	if (!strcmp(formatEnv, "rgb565"))
		return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	if (!strcmp(formatEnv, "rgba32"))
		return Graphics::PixelFormat::createFormatRGBA32();

	warning("Unknown Rose Tattoo hires format '%s'; using clut8", formatEnv);
	return Graphics::PixelFormat::createFormatCLUT8();
}

static int closestPaletteEntry(const byte *palette, byte r, byte g, byte b) {
	int closest = 0;
	int closestDistance = 256 * 256 * 3;

	for (int idx = 0; idx < 256; ++idx) {
		int dr = (int)r - palette[idx * 3];
		int dg = (int)g - palette[idx * 3 + 1];
		int db = (int)b - palette[idx * 3 + 2];
		int distance = dr * dr + dg * dg + db * db;

		if (distance < closestDistance) {
			closest = idx;
			closestDistance = distance;
		}
	}

	return closest;
}

Screen::Screen(SherlockEngine *vm) : BaseSurface(
			vm->getGameID() == GType_RoseTattoo ? 640 : g_system->getWidth(),
			vm->getGameID() == GType_RoseTattoo ? 480 : g_system->getHeight(),
			vm->getGameID() == GType_RoseTattoo ? Graphics::PixelFormat::createFormatCLUT8() : g_system->getScreenFormat()),
		_vm(vm),
		_backBuffer1(vm->getGameID() == GType_RoseTattoo ? 640 : 320,
			vm->getGameID() == GType_RoseTattoo ? 480 : 200, Graphics::PixelFormat::createFormatCLUT8()),
		_backBuffer2(vm->getGameID() == GType_RoseTattoo ? 640 : 320,
			vm->getGameID() == GType_RoseTattoo ? 480 : 200, Graphics::PixelFormat::createFormatCLUT8()) {
	_transitionSeed = 1;
	_roseTattooHiresScale = vm->getGameID() == GType_RoseTattoo ? roseTattooHiresScaleFromEnv() : 1;
	_roseTattooHiresDebugMode = vm->getGameID() == GType_RoseTattoo ? roseTattooHiresDebugModeFromEnv() : kRoseTattooHiresComposite;
	_roseTattooHiresFormat = vm->getGameID() == GType_RoseTattoo ? roseTattooHiresFormatFromEnv() : Graphics::PixelFormat::createFormatCLUT8();
#ifdef USE_FREETYPE2
	_roseTattooHiresFontMissing = false;
#endif
	_fadeStyle = false;
	Common::fill(&_cMap[0], &_cMap[Graphics::PALETTE_SIZE], 0);
	Common::fill(&_sMap[0], &_sMap[Graphics::PALETTE_SIZE], 0);
	Common::fill(&_tMap[0], &_tMap[Graphics::PALETTE_SIZE], 0);
	Common::fill(&_roseTattooPalette[0], &_roseTattooPalette[Graphics::PALETTE_SIZE], 0);

	// Set up the initial font
	setFont(IS_SERRATED_SCALPEL ? 1 : 4);

	// Rose Tattoo specific fields
	_fadeBytesRead = _fadeBytesToRead = 0;
	_oldFadePercent = 0;
	_flushScreen = false;
	_backBuffer.create(_backBuffer1, _backBuffer1.getBounds());
}

Screen::~Screen() {
	Fonts::freeFont();
#ifdef USE_FREETYPE2
	for (Common::HashMap<int, Graphics::Font *>::iterator it = _roseTattooHiresFonts.begin();
			it != _roseTattooHiresFonts.end(); ++it)
		delete it->_value;
#endif
}

void Screen::clearRoseTattooHiresBackground() {
	_roseTattooHiresBackground.free();
	_roseTattooHiresComposite.free();
}

void Screen::getPalette(byte palette[Graphics::PALETTE_SIZE]) {
	getPalette(palette, 0, Graphics::PALETTE_COUNT);
}

void Screen::getPalette(byte *palette, uint start, uint num) {
	if (_roseTattooHiresScale > 1 && !_roseTattooHiresFormat.isCLUT8()) {
		assert(start + num <= Graphics::PALETTE_COUNT);
		Common::copy(&_roseTattooPalette[start * 3],
			&_roseTattooPalette[(start + num) * 3], palette);
		return;
	}

	Graphics::Screen::getPalette(palette, start, num);
}

void Screen::setPalette(const byte palette[Graphics::PALETTE_SIZE]) {
	setPalette(palette, 0, Graphics::PALETTE_COUNT);
}

void Screen::setPalette(const byte *palette, uint start, uint num) {
	assert(start + num <= Graphics::PALETTE_COUNT);
	Common::copy(palette, palette + num * 3, &_roseTattooPalette[start * 3]);

	if (_roseTattooHiresScale > 1 && !_roseTattooHiresFormat.isCLUT8()) {
		// Graphics::Screen::setPalette() forwards to
		// g_system->getPaletteManager()->setPalette(), which some backends
		// (e.g. OpenGLGraphicsManager::setPalette()) assert requires the
		// game screen itself to be palettized/CLUT8 - no longer true once
		// we've switched the backend to RGB565/RGBA32 for the hires
		// composite, so we can't call it here without crashing.
		//
		// However, the mouse cursor (an 8-bit palettized image) still needs
		// a palette to be rendered in color: use CursorMan's dedicated
		// cursor-palette mechanism (backed by OSystem::setCursorPalette(),
		// independent of the game screen's own pixel format) instead of the
		// regular game palette, so the cursor doesn't render solid black.
		CursorMan.replaceCursorPalette(palette, start, num);
		return;
	}

	Graphics::Screen::setPalette(palette, start, num);
}

void Screen::setPalette(const Graphics::Palette &pal, uint start) {
	setPalette(pal.data(), start, pal.size());
}

bool Screen::loadRoseTattooHiresBackgroundFromPath(const Common::Path &overridePath) {
	if (_roseTattooHiresScale <= 1)
		return false;

	Common::File file;
	if (!file.open(Common::FSNode(overridePath)))
		return false;

	Image::PNGDecoder decoder;
	if (!decoder.loadStream(file)) {
		warning("Could not decode Rose Tattoo hires background override: %s",
			overridePath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	const Graphics::Surface *surface = decoder.getSurface();
	const int expectedWidth = _backBuffer1.width() * _roseTattooHiresScale;
	const int expectedHeight = _backBuffer1.height() * _roseTattooHiresScale;
	if (!surface || surface->w != expectedWidth || surface->h != expectedHeight) {
		warning("Ignoring Rose Tattoo hires background override with unexpected dimensions: %s",
			overridePath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	_roseTattooHiresBackground.create(expectedWidth, expectedHeight, _roseTattooHiresFormat);
	_roseTattooHiresComposite.create(SHERLOCK_SCREEN_WIDTH * _roseTattooHiresScale,
		SHERLOCK_SCREEN_HEIGHT * _roseTattooHiresScale, _roseTattooHiresFormat);

	const Graphics::Palette &sourcePalette = decoder.getPalette();
	const byte *sourcePaletteData = sourcePalette.data();

	for (int y = 0; y < surface->h; ++y) {
		for (int x = 0; x < surface->w; ++x) {
			uint32 color = surface->getPixel(x, y);
			byte r, g, b;

			if (surface->format.bytesPerPixel == 1 && sourcePaletteData && color < sourcePalette.size()) {
				r = sourcePaletteData[color * 3];
				g = sourcePaletteData[color * 3 + 1];
				b = sourcePaletteData[color * 3 + 2];
			} else {
				surface->format.colorToRGB(color, r, g, b);
			}

			uint32 outputColor = _roseTattooHiresFormat.isCLUT8() ?
				closestPaletteEntry(_cMap, r, g, b) :
				_roseTattooHiresFormat.RGBToColor(r, g, b);
			_roseTattooHiresBackground.setPixel(x, y, outputColor);
		}
	}

	return true;
}

bool Screen::loadRoseTattooHiresBackgroundOverride(int sceneNumber) {
	clearRoseTattooHiresBackground();

	if (_roseTattooHiresScale <= 1)
		return false;

	const char *overrideDir = getenv("SCUMMVM_SHERLOCK_TATTOO_ASSET_OVERRIDES");
	if (!overrideDir || !*overrideDir)
		return false;

	Common::Path overridePath(Common::String::format(
		"%s/scene_%03d/background@%dx.png", overrideDir, sceneNumber, _roseTattooHiresScale), '/');
	return loadRoseTattooHiresBackgroundFromPath(overridePath);
}

bool Screen::loadRoseTattooHiresMapOverride() {
	// Unlike normal room scenes, the overhead/travel map isn't loaded through
	// Scene::loadScene(), so TattooMap::show() calls this directly right
	// after sizing _backBuffer1 to the map's own (larger, double-size)
	// canvas. Reuses the same override plumbing as room backgrounds, just
	// with a fixed filename instead of a per-scene one.
	if (_roseTattooHiresScale <= 1)
		return false;

	const char *overrideDir = getenv("SCUMMVM_SHERLOCK_TATTOO_ASSET_OVERRIDES");
	if (!overrideDir || !*overrideDir)
		return false;

	Common::Path overridePath(Common::String::format(
		"%s/sprites/map_vgs/frame_000@%dx.png", overrideDir, _roseTattooHiresScale), '/');
	return loadRoseTattooHiresBackgroundFromPath(overridePath);
}

bool Screen::loadRoseTattooHiresCursorOverride(const Common::String &resourceName, int frameIndex,
		Graphics::Surface &outSurface) {
	if (_roseTattooHiresScale <= 1)
		return false;

	const char *overrideDir = getenv("SCUMMVM_SHERLOCK_TATTOO_ASSET_OVERRIDES");
	if (!overrideDir || !*overrideDir)
		return false;

	Common::Path overridePath(Common::String::format(
		"%s/sprites/%s/frame_%03d@%dx.png", overrideDir, resourceName.c_str(), frameIndex,
		_roseTattooHiresScale), '/');
	Common::File file;
	if (!file.open(Common::FSNode(overridePath)))
		return false;

	Image::PNGDecoder decoder;
	if (!decoder.loadStream(file)) {
		warning("Could not decode Rose Tattoo hires cursor override: %s",
			overridePath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || !surface->w || !surface->h)
		return false;

	const Graphics::PixelFormat rgba8888 = Graphics::PixelFormat::createFormatRGBA32();
	outSurface.create(surface->w, surface->h, rgba8888);

	const Graphics::Palette &sourcePalette = decoder.getPalette();
	const byte *sourcePaletteData = sourcePalette.data();
	const bool hasAlpha = decoder.hasTransparentColor() || surface->format.aBits() > 0;

	for (int y = 0; y < surface->h; ++y) {
		for (int x = 0; x < surface->w; ++x) {
			uint32 color = surface->getPixel(x, y);
			byte r, g, b, a = 255;

			if (surface->format.bytesPerPixel == 1 && sourcePaletteData && color < sourcePalette.size()) {
				r = sourcePaletteData[color * 3];
				g = sourcePaletteData[color * 3 + 1];
				b = sourcePaletteData[color * 3 + 2];
				if (decoder.hasTransparentColor() && color == decoder.getTransparentColor())
					a = 0;
			} else {
				surface->format.colorToARGB(color, a, r, g, b);
				if (!hasAlpha)
					a = 255;
			}

			outSurface.setPixel(x, y, rgba8888.ARGBToColor(a, r, g, b));
		}
	}

	return true;
}

Common::Rect Screen::blitRoseTattooHiresSpriteOverride(Graphics::ManagedSurface &layer,
		const Common::String &resourceName, int frameIndex, const ImageFrame &nativeFrame,
		const Common::Point &pt, bool horizFlip, int scaleVal, bool trackNativeProvenance) {
	if (_roseTattooHiresScale <= 1 || _roseTattooHiresFormat.isCLUT8())
		return Common::Rect();

	Graphics::Surface overrideSurface;
	if (!loadRoseTattooHiresCursorOverride(resourceName, frameIndex, overrideSurface))
		return Common::Rect();

	const int scale = _roseTattooHiresScale;

	// Mirrors BaseSurface::SHtransBlitFrom(const ImageFrame &, ...): the
	// frame's own offset (which varies frame-to-frame for walk-cycle
	// sprites, to keep feet/hands aligned) is added to the caller's pt
	// here, not baked into the override PNG's pixel content (see
	// upscale_rosetattoo_sprites.py, which only scales offset_x/offset_y
	// as metadata).
	const Common::Point drawPt(pt.x + nativeFrame.sDrawXOffset(scaleVal),
		pt.y + nativeFrame.sDrawYOffset(scaleVal));

	// Perspective-scaled sprites (walking characters, some bg-shape props)
	// need the crisp override resized by the same ratio the native draw
	// uses (see ImageFrame::sDrawXSize()/sDrawYSize()), or it would stay a
	// fixed size while its blocky native-upscaled fallback shrinks/grows
	// around it as the character walks toward/away from the camera. Nearest-
	// neighbor is used (not a smoother filter) since this runs every frame
	// for every visible perspective-scaled sprite - the AI-upscaled source
	// detail is already baked in, so a cheap resize is enough to track the
	// native silhouette's size without adding per-frame cost that scales
	// with character count.
	Graphics::Surface resized;
	Graphics::Surface *scaledSurface = &overrideSurface;
	if (scaleVal != SCALE_THRESHOLD) {
		const int clampedScale = scaleVal == 0 ? 1 : scaleVal;
		int targetW = MAX(overrideSurface.w * SCALE_THRESHOLD / clampedScale, 1);
		int targetH = MAX(overrideSurface.h * SCALE_THRESHOLD / clampedScale, 1);

		resized.create(targetW, targetH, overrideSurface.format);
		for (int dy = 0; dy < targetH; ++dy) {
			const int sy = MIN(dy * overrideSurface.h / targetH, overrideSurface.h - 1);
			for (int dx = 0; dx < targetW; ++dx) {
				const int sx = MIN(dx * overrideSurface.w / targetW, overrideSurface.w - 1);
				resized.setPixel(dx, dy, overrideSurface.getPixel(sx, sy));
			}
		}
		scaledSurface = &resized;
	}

	if (horizFlip) {
		// Mirror in place to match SHtransBlitFrom()'s own flip argument,
		// used when a character walks left instead of right.
		for (int y = 0; y < scaledSurface->h; ++y) {
			for (int x = 0; x < scaledSurface->w / 2; ++x) {
				const uint32 left = scaledSurface->getPixel(x, y);
				const uint32 right = scaledSurface->getPixel(scaledSurface->w - 1 - x, y);
				scaledSurface->setPixel(x, y, right);
				scaledSurface->setPixel(scaledSurface->w - 1 - x, y, left);
			}
		}
	}

	const int outWidth = SHERLOCK_SCREEN_WIDTH * scale;
	const int outHeight = SHERLOCK_SCREEN_HEIGHT * scale;
	if (layer.empty())
		layer.create(outWidth, outHeight, Graphics::PixelFormat::createFormatRGBA32());

	const int x = drawPt.x * scale;
	const int y = drawPt.y * scale;

	// Sprite overrides always use crisp binary (0/255) alpha cutouts (see
	// upscale_rosetattoo_sprites.py - no partial-alpha blending), so a
	// direct per-pixel copy is correct here; this also sidesteps the same
	// RGBA32/BlendBlit channel-order mismatch noted in
	// blendRoseTattooHiresTextLayer() that rules out transBlitFrom()/
	// blendBlitFrom() for this true-color format.
	// See _roseTattooHiresSceneSpriteExpectedNative's declaration: lazily
	// sized here (rather than in the constructor) since it's only ever
	// needed once hires mode with a scene-sprite override is actually
	// active.
	if (trackNativeProvenance && _roseTattooHiresSceneSpriteExpectedNative.empty()) {
		_roseTattooHiresSceneSpriteExpectedNative.resize(SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT, 0);
		_roseTattooHiresSceneSpriteExpectedValid.resize(SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT, 0);
	}

	for (int sy = 0; sy < scaledSurface->h; ++sy) {
		const int dy = y + sy;
		if (dy < 0 || dy >= layer.h)
			continue;
		for (int sx = 0; sx < scaledSurface->w; ++sx) {
			const int dx = x + sx;
			if (dx < 0 || dx >= layer.w)
				continue;

			uint8 a, r, g, b;
			scaledSurface->format.colorToARGB(scaledSurface->getPixel(sx, sy), a, r, g, b);
			if (a == 0)
				continue;

			layer.setPixel(dx, dy, layer.format.ARGBToColor(a, r, g, b));

			if (trackNativeProvenance) {
				// Record what _backBuffer1 (the native low-res scene
				// buffer this shape/character was just SHtransBlitFrom()'d
				// into, by the caller, immediately before this queue call)
				// holds at this pixel's native equivalent right now - i.e.
				// what this exact shape drew there. If a later shape in
				// this same frame draws over the same native spot
				// afterward (correctly occluding this one), its own
				// queueRoseTattooHiresSceneSprite() call will overwrite
				// this entry with its own value - see
				// blendRoseTattooHiresSceneSpriteLayer() for how this gets
				// used to detect and skip pixels that were subsequently
				// occluded.
				const int nx = dx / scale;
				const int ny = dy / scale;
				if (nx >= 0 && nx < SHERLOCK_SCREEN_WIDTH && ny >= 0 && ny < SHERLOCK_SCREEN_HEIGHT &&
						nx < _backBuffer1.w && ny < _backBuffer1.h) {
					const int idx = ny * SHERLOCK_SCREEN_WIDTH + nx;
					_roseTattooHiresSceneSpriteExpectedNative[idx] = (byte)_backBuffer1.getPixel(nx, ny);
					_roseTattooHiresSceneSpriteExpectedValid[idx] = 1;
				}
			}
		}
	}

	Common::Rect queuedRect(x, y, x + scaledSurface->w, y + scaledSurface->h);

	overrideSurface.free();
	resized.free();
	return queuedRect;
}

void Screen::queueRoseTattooHiresSprite(const Common::String &resourceName, int frameIndex,
		const ImageFrame &nativeFrame, const Common::Point &pt, bool horizFlip, int scaleVal) {
	Common::Rect queuedRect = blitRoseTattooHiresSpriteOverride(_roseTattooHiresSpriteLayer,
		resourceName, frameIndex, nativeFrame, pt, horizFlip, scaleVal);
	if (queuedRect.isEmpty())
		return;

	if (_roseTattooHiresSpriteNativeRect.isEmpty())
		_roseTattooHiresSpriteNativeRect = queuedRect;
	else
		_roseTattooHiresSpriteNativeRect.extend(queuedRect);
}

void Screen::queueRoseTattooHiresSceneSprite(const Common::String &resourceName, int frameIndex,
		const ImageFrame &nativeFrame, const Common::Point &pt, bool horizFlip, int scaleVal) {
	blitRoseTattooHiresSpriteOverride(_roseTattooHiresSceneSpriteLayer,
		resourceName, frameIndex, nativeFrame, pt, horizFlip, scaleVal, true);
}

void Screen::blendRoseTattooHiresSceneSpriteLayer() {
	if (_roseTattooHiresSceneSpriteLayer.empty())
		return;

	// Same blend approach as blendRoseTattooHiresSpriteLayer() (see its
	// comment for why no background-repaint-first step is needed/wanted).
	const int w = _roseTattooHiresSceneSpriteLayer.w;
	const int h = _roseTattooHiresSceneSpriteLayer.h;
	const int scale = _roseTattooHiresScale;
	const bool haveProvenance = !_roseTattooHiresSceneSpriteExpectedValid.empty();
	const byte *screenPixels = (const byte *)getPixels();
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uint8 a, r, g, b;
			_roseTattooHiresSceneSpriteLayer.format.colorToARGB(
				_roseTattooHiresSceneSpriteLayer.getPixel(x, y), a, r, g, b);
			if (a == 0)
				continue;

			if (haveProvenance && scale > 0) {
				// Skip this pixel if something else drew over its native
				// equivalent *after* the shape/character that queued it
				// did (see _roseTattooHiresSceneSpriteExpectedNative's
				// declaration) - e.g. a bg-shape with no AI override,
				// the verb menu, or any other native-only UI drawn later
				// this same frame. Left unguarded, this AI-upscaled pixel
				// would otherwise always render on top regardless, making
				// characters/objects appear to "float" above whatever
				// should be occluding them.
				const int nx = x / scale;
				const int ny = y / scale;
				if (nx >= 0 && nx < SHERLOCK_SCREEN_WIDTH && ny >= 0 && ny < SHERLOCK_SCREEN_HEIGHT) {
					const int idx = ny * SHERLOCK_SCREEN_WIDTH + nx;
					if (!_roseTattooHiresSceneSpriteExpectedValid[idx])
						continue;
					const byte actualNative = screenPixels[ny * pitch + nx];
					if (actualNative != _roseTattooHiresSceneSpriteExpectedNative[idx])
						continue;
				}
			}

			uint8 dstA, dstR, dstG, dstB;
			_roseTattooHiresComposite.format.colorToARGB(_roseTattooHiresComposite.getPixel(x, y), dstA, dstR, dstG, dstB);

			const uint8 outR = (uint8)((r * a + dstR * (255 - a)) / 255);
			const uint8 outG = (uint8)((g * a + dstG * (255 - a)) / 255);
			const uint8 outB = (uint8)((b * a + dstB * (255 - a)) / 255);
			_roseTattooHiresComposite.setPixel(x, y,
				_roseTattooHiresComposite.format.ARGBToColor(0xff, outR, outG, outB));
		}
	}
}

void Screen::clearRoseTattooHiresSceneSpriteLayer() {
	if (!_roseTattooHiresSceneSpriteLayer.empty())
		_roseTattooHiresSceneSpriteLayer.fillRect(Common::Rect(0, 0, _roseTattooHiresSceneSpriteLayer.w,
			_roseTattooHiresSceneSpriteLayer.h), 0);

	// Start this frame's occlusion-provenance tracking fresh too - see
	// _roseTattooHiresSceneSpriteExpectedNative's declaration. Only the
	// "valid" flags need clearing; stale expected-native values are
	// harmless since they're never consulted unless their matching valid
	// flag is set.
	if (!_roseTattooHiresSceneSpriteExpectedValid.empty())
		Common::fill(_roseTattooHiresSceneSpriteExpectedValid.begin(), _roseTattooHiresSceneSpriteExpectedValid.end(), 0);
}

void Screen::paintRoseTattooHiresWorldSprite(const Common::String &resourceName, int frameIndex,
		const Common::Point &worldPt) {
	// Unlike queueRoseTattooHiresSprite() (which targets the screen-space,
	// scroll-independent _roseTattooHiresSpriteLayer for UI overlays like
	// inventory icons), this paints directly into the persistent, full-world
	// _roseTattooHiresBackground buffer at world/native coordinates - for
	// content like the overhead map's location icons that are drawn once
	// into the map's own oversized scrollable canvas (see
	// TattooMap::drawMapIcons()) rather than redrawn as a screen-space
	// overlay every frame. Baking the icon into the world background this
	// way means the existing scroll-offset sampling in update()/the text
	// and sprite layer blend functions naturally keeps it correctly
	// positioned as the map scrolls, with no extra per-frame bookkeeping.
	if (_roseTattooHiresScale <= 1 || _roseTattooHiresFormat.isCLUT8() || _roseTattooHiresBackground.empty())
		return;

	Graphics::Surface overrideSurface;
	if (!loadRoseTattooHiresCursorOverride(resourceName, frameIndex, overrideSurface))
		return;

	const int scale = _roseTattooHiresScale;
	const int x = worldPt.x * scale;
	const int y = worldPt.y * scale;

	for (int sy = 0; sy < overrideSurface.h; ++sy) {
		const int dy = y + sy;
		if (dy < 0 || dy >= _roseTattooHiresBackground.h)
			continue;
		for (int sx = 0; sx < overrideSurface.w; ++sx) {
			const int dx = x + sx;
			if (dx < 0 || dx >= _roseTattooHiresBackground.w)
				continue;

			uint8 a, r, g, b;
			overrideSurface.format.colorToARGB(overrideSurface.getPixel(sx, sy), a, r, g, b);
			if (a == 0)
				continue;

			if (a == 255) {
				_roseTattooHiresBackground.setPixel(dx, dy,
					_roseTattooHiresBackground.format.ARGBToColor(0xff, r, g, b));
			} else {
				uint8 dstA, dstR, dstG, dstB;
				_roseTattooHiresBackground.format.colorToARGB(_roseTattooHiresBackground.getPixel(dx, dy),
					dstA, dstR, dstG, dstB);
				const uint8 outR = (uint8)((r * a + dstR * (255 - a)) / 255);
				const uint8 outG = (uint8)((g * a + dstG * (255 - a)) / 255);
				const uint8 outB = (uint8)((b * a + dstB * (255 - a)) / 255);
				_roseTattooHiresBackground.setPixel(dx, dy,
					_roseTattooHiresBackground.format.ARGBToColor(0xff, outR, outG, outB));
			}
		}
	}

	overrideSurface.free();
}

void Screen::blendRoseTattooHiresSpriteLayer() {
	if (_roseTattooHiresSpriteLayer.empty())
		return;

	// Unlike blendRoseTattooHiresTextLayer(), we deliberately do NOT repaint
	// the smooth hires *background* image under the dirty rect first: sprite
	// overrides (e.g. inventory item icons) are drawn on top of a UI widget
	// panel, not directly over the live scrolling game view, so repainting
	// the raw scene background there would blow away the panel underneath
	// and let the background bleed through the widget. The native (blocky)
	// icon blit already drawn by the caller as a fallback is a fine base to
	// blend the crisp override on top of.
	const int w = _roseTattooHiresSpriteLayer.w;
	const int h = _roseTattooHiresSpriteLayer.h;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uint8 a, r, g, b;
			_roseTattooHiresSpriteLayer.format.colorToARGB(_roseTattooHiresSpriteLayer.getPixel(x, y), a, r, g, b);
			if (a == 0)
				continue;

			uint8 dstA, dstR, dstG, dstB;
			_roseTattooHiresComposite.format.colorToARGB(_roseTattooHiresComposite.getPixel(x, y), dstA, dstR, dstG, dstB);

			const uint8 outR = (uint8)((r * a + dstR * (255 - a)) / 255);
			const uint8 outG = (uint8)((g * a + dstG * (255 - a)) / 255);
			const uint8 outB = (uint8)((b * a + dstB * (255 - a)) / 255);
			_roseTattooHiresComposite.setPixel(x, y,
				_roseTattooHiresComposite.format.ARGBToColor(0xff, outR, outG, outB));
		}
	}
}

void Screen::clearRoseTattooHiresSpriteRect(const Common::Rect &nativeRect) {
	if (_roseTattooHiresSpriteLayer.empty())
		return;

	const int scale = _roseTattooHiresScale;
	Common::Rect r(nativeRect.left * scale, nativeRect.top * scale,
		nativeRect.right * scale, nativeRect.bottom * scale);
	r.clip(Common::Rect(0, 0, _roseTattooHiresSpriteLayer.w, _roseTattooHiresSpriteLayer.h));
	if (!r.isEmpty())
		_roseTattooHiresSpriteLayer.fillRect(r, 0);
}

void Screen::clearRoseTattooHiresSpriteLayer() {
	if (!_roseTattooHiresSpriteLayer.empty())
		_roseTattooHiresSpriteLayer.fillRect(Common::Rect(0, 0, _roseTattooHiresSpriteLayer.w,
			_roseTattooHiresSpriteLayer.h), 0);
	_roseTattooHiresSpriteNativeRect = Common::Rect();
}

#ifdef USE_FREETYPE2
Graphics::Font *Screen::getRoseTattooHiresFont(int pixelHeight) {
	if (_roseTattooHiresFontMissing || pixelHeight <= 0)
		return nullptr;

	Common::HashMap<int, Graphics::Font *>::iterator it = _roseTattooHiresFonts.find(pixelHeight);
	if (it != _roseTattooHiresFonts.end())
		return it->_value;

	const char *overrideDir = getenv("SCUMMVM_SHERLOCK_TATTOO_ASSET_OVERRIDES");
	if (!overrideDir || !*overrideDir) {
		_roseTattooHiresFontMissing = true;
		return nullptr;
	}

	Common::Path fontPath(Common::String::format("%s/fonts/hires_font.ttf", overrideDir), '/');
	Common::File file;
	if (!file.open(Common::FSNode(fontPath))) {
		_roseTattooHiresFontMissing = true;
		return nullptr;
	}

	// loadTTFFont() takes ownership of the stream (DisposeAfterUse::YES) and
	// reads it fully up front, so a fresh Common::File per requested size is
	// fine - sizes are few (bounded by the handful of distinct FONT*.VGS
	// heights in the game, times _roseTattooHiresScale) and cached below.
	Graphics::Font *font = Graphics::loadTTFFont(file.readStream(file.size()), DisposeAfterUse::YES,
		pixelHeight, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeNormal);
	if (!font)
		warning("Could not load Rose Tattoo hires text font at size %d: %s",
			pixelHeight, fontPath.toString(Common::Path::kNativeSeparator).c_str());

	_roseTattooHiresFonts[pixelHeight] = font;
	return font;
}

void Screen::queueRoseTattooHiresText(const Common::String &str, const Common::Point &nativePt,
		uint32 rgbColor, int fontHeightPx) {
	// True-color anti-aliased glyphs can't be represented well once
	// quantized down to an 8-bit CLUT8 composite (the whole point is smooth
	// alpha-blended edges) - only bother in the true-color hires modes,
	// same restriction the cursor override already applies. Also skip
	// entirely when no hires background is loaded (e.g. Watson's Journal,
	// which replaces the whole screen with its own bitmap-drawn UI and
	// deliberately clears the hires background - see TattooJournal::show()):
	// blendRoseTattooHiresTextLayer() can only mask away the composite's
	// blocky nearest-neighbor-upscaled bitmap glyphs by repainting from the
	// smooth hires background first, so without one, the crisp TTF text
	// would just be blended on top of - not instead of - the native bitmap
	// glyphs, producing a doubled/ghosted look.
	//
	// This isn't just a matter of loading *some* backdrop for the journal
	// to repaint from, either: unlike WidgetTooltipBase (the only other
	// hires-text caller), which skips its native bitmap blit entirely and
	// re-queues its TTF text fresh every single frame it stays visible (see
	// WidgetTooltipBase::draw()'s comment on why - the erase-then-redraw
	// dance alone can't reliably win the race every frame), Watson's
	// Journal draws its bitmap text into _backBuffer1 the normal way
	// (Fonts::writeString(), same as any other text) and only once per
	// page, not every frame. Since the "no hires background" composite
	// path rebuilds itself from a fresh raw upscale of the *current* native
	// framebuffer every single frame (blocky glyphs and all), and this
	// method's own erase-mask rect is only registered on the frame the
	// text is actually drawn (see registerRoseTattooHiresTextRect()), any
	// subsequent frame the journal just sits idle would repaint the blocky
	// glyphs back into the composite with no accompanying erase - and then
	// blend the still-persisted crisp TTF layer on top of them anyway,
	// reproducing the doubled/ghosted look. Properly supporting the
	// journal would need the same per-frame re-draw/skip-native-blit
	// treatment as WidgetTooltipBase, which is out of scope here - tracked
	// as a known follow-up rather than attempted as a quick fix.
	if (_roseTattooHiresScale <= 1 || _roseTattooHiresFormat.isCLUT8() || str.empty() ||
			_roseTattooHiresBackground.empty())
		return;

	// Slightly undersize the requested TTF point size relative to the
	// bitmap font's total glyph-cell height: bitmap _fontHeight includes
	// descender padding baked into the sprite frames, while TTF sizing
	// measures cap-height, so an equal pixel size renders visibly larger/
	// heavier than the original font. This factor was tuned by eye against
	// the FONT4.VGS dialogue font.
	const int pixelHeight = MAX(1, (fontHeightPx * _roseTattooHiresScale * 82) / 100);

	Graphics::Font *font = getRoseTattooHiresFont(pixelHeight);
	if (!font)
		return;

	const int outWidth = SHERLOCK_SCREEN_WIDTH * _roseTattooHiresScale;
	const int outHeight = SHERLOCK_SCREEN_HEIGHT * _roseTattooHiresScale;
	if (_roseTattooHiresTextLayer.empty())
		_roseTattooHiresTextLayer.create(outWidth, outHeight, Graphics::PixelFormat::createFormatRGBA32());

	const int x = nativePt.x * _roseTattooHiresScale;
	const int y = nativePt.y * _roseTattooHiresScale;
	const uint32 color = _roseTattooHiresTextLayer.format.ARGBToColor(0xff,
		(rgbColor >> 16) & 0xff, (rgbColor >> 8) & 0xff, rgbColor & 0xff);
	font->drawString(&_roseTattooHiresTextLayer, str, x, y, outWidth - x, color);
}


void Screen::registerRoseTattooHiresTextRect(const Common::Rect &nativeRect) {
	if (_roseTattooHiresScale <= 1 || nativeRect.isEmpty())
		return;

	const int scale = _roseTattooHiresScale;
	Common::Rect r(nativeRect.left * scale, nativeRect.top * scale,
		nativeRect.right * scale, nativeRect.bottom * scale);
	r.clip(Common::Rect(0, 0, SHERLOCK_SCREEN_WIDTH * scale, SHERLOCK_SCREEN_HEIGHT * scale));
	if (r.isEmpty())
		return;

	if (_roseTattooHiresTextNativeRect.isEmpty())
		_roseTattooHiresTextNativeRect = r;
	else
		_roseTattooHiresTextNativeRect.extend(r);
}

/**
 * Alpha-blends the accumulated hires text layer (see
 * queueRoseTattooHiresText()) on top of the composite that's about to be
 * pushed to the screen. No-op if nothing has been queued (e.g. no hires
 * font override configured, or the format is CLUT8).
 *
 * Deliberately blends pixel-by-pixel via getPixel()/setPixel() rather than
 * ManagedSurface::blendBlitFrom(): that method only works when both
 * surfaces exactly match BlendBlit::getSupportedPixelFormat()'s specific
 * channel byte order, which createFormatRGBA32() does NOT match on
 * little-endian hosts (it silently no-ops with a "only accepts RGBA32!"
 * warning instead of actually blending) - format-agnostic getPixel/
 * setPixel plus colorToARGB()/ARGBToColor() sidesteps that entirely, at
 * the cost of being slower; acceptable since this only runs over the
 * relatively small glyph-covered area of a hires frame that changed.
 *
 * Unlike the composite, this layer is NOT cleared here - see its
 * declaration in screen.h for why it needs to persist across frames on its
 * own instead of being wiped every time it's blended.
 */
void Screen::blendRoseTattooHiresTextLayer() {
	if (_roseTattooHiresTextLayer.empty())
		return;

	// Before blending, repaint the smooth hires background across the
	// whole area covered by this frame's queued text (see
	// _roseTattooHiresTextNativeRect's declaration in screen.h) so none of
	// the blocky upscaled bitmap glyph pixels the composite's normal
	// foreground pass drew there can show through the gaps/edges of the
	// crisp TTF glyphs about to be blended on top. Only possible when a
	// real hires background is loaded (not the raw-upscale-only fallback,
	// where there's nothing smoother to restore from).
	if (!_roseTattooHiresTextNativeRect.isEmpty() && !_roseTattooHiresBackground.empty()) {
		const int bgOffsetX = _currentScroll.x * _roseTattooHiresScale;
		const int bgOffsetY = _currentScroll.y * _roseTattooHiresScale;
		Common::Rect r = _roseTattooHiresTextNativeRect;
		r.clip(Common::Rect(0, 0, _roseTattooHiresComposite.w, _roseTattooHiresComposite.h));
		for (int y = r.top; y < r.bottom; ++y) {
			const int bgY = bgOffsetY + y;
			if (bgY < 0 || bgY >= _roseTattooHiresBackground.h)
				continue;
			for (int x = r.left; x < r.right; ++x) {
				const int bgX = bgOffsetX + x;
				if (bgX < 0 || bgX >= _roseTattooHiresBackground.w)
					continue;
				_roseTattooHiresComposite.setPixel(x, y, _roseTattooHiresBackground.getPixel(bgX, bgY));
			}
		}
	}
	_roseTattooHiresTextNativeRect = Common::Rect();

	const int w = _roseTattooHiresTextLayer.w;
	const int h = _roseTattooHiresTextLayer.h;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uint8 a, r, g, b;
			_roseTattooHiresTextLayer.format.colorToARGB(_roseTattooHiresTextLayer.getPixel(x, y), a, r, g, b);
			if (a == 0)
				continue;

			uint8 dstA, dstR, dstG, dstB;
			_roseTattooHiresComposite.format.colorToARGB(_roseTattooHiresComposite.getPixel(x, y), dstA, dstR, dstG, dstB);

			const uint8 outR = (uint8)((r * a + dstR * (255 - a)) / 255);
			const uint8 outG = (uint8)((g * a + dstG * (255 - a)) / 255);
			const uint8 outB = (uint8)((b * a + dstB * (255 - a)) / 255);
			_roseTattooHiresComposite.setPixel(x, y,
				_roseTattooHiresComposite.format.ARGBToColor(0xff, outR, outG, outB));
		}
	}
}

void Screen::clearRoseTattooHiresTextRect(const Common::Rect &nativeRect) {
	if (_roseTattooHiresTextLayer.empty())
		return;

	const int scale = _roseTattooHiresScale;
	Common::Rect r(nativeRect.left * scale, nativeRect.top * scale,
		nativeRect.right * scale, nativeRect.bottom * scale);
	r.clip(Common::Rect(0, 0, _roseTattooHiresTextLayer.w, _roseTattooHiresTextLayer.h));
	if (!r.isEmpty())
		_roseTattooHiresTextLayer.fillRect(r, 0);
}

void Screen::clearRoseTattooHiresTextLayer() {
	if (!_roseTattooHiresTextLayer.empty())
		_roseTattooHiresTextLayer.fillRect(Common::Rect(0, 0, _roseTattooHiresTextLayer.w,
			_roseTattooHiresTextLayer.h), 0);
}
#endif

void Screen::update() {
	if (_roseTattooHiresScale <= 1) {
		Graphics::Screen::update();
		return;
	}

	const int scale = _roseTattooHiresScale;
	const int outWidth = SHERLOCK_SCREEN_WIDTH * scale;
	const int outHeight = SHERLOCK_SCREEN_HEIGHT * scale;

	if (_roseTattooHiresBackground.empty()) {
		// No hires background override is loaded for the current scene (e.g. the
		// overhead/travel map, which bypasses Scene::loadScene() and therefore
		// never calls loadRoseTattooHiresBackgroundOverride(), or an ordinary
		// room whose override asset is missing). Falling all the way back to
		// Graphics::Screen::update() here would draw only a native-resolution
		// SHERLOCK_SCREEN_WIDTH x SHERLOCK_SCREEN_HEIGHT rectangle into the
		// much larger hires window's top-left corner, leaving the remaining
		// area filled with whatever pixels were already in the OS-level
		// framebuffer (typically stale hires composite data from the last
		// room), which reads as garbled static. Instead, do a simple
		// nearest-neighbor upscale of the native framebuffer so the whole
		// hires window is filled with real, correctly-scaled content.
		if (_roseTattooHiresComposite.empty())
			_roseTattooHiresComposite.create(outWidth, outHeight, _roseTattooHiresFormat);

		const byte *screenPixels = (const byte *)getPixels();
		for (int y = 0; y < SHERLOCK_SCREEN_HEIGHT; ++y) {
			const byte *screenRow = screenPixels + y * pitch;
			for (int x = 0; x < SHERLOCK_SCREEN_WIDTH; ++x) {
				const byte nativePixel = screenRow[x];
				const uint32 outputPixel = _roseTattooHiresFormat.isCLUT8() ? nativePixel :
					_roseTattooHiresFormat.RGBToColor(_cMap[nativePixel * 3],
						_cMap[nativePixel * 3 + 1], _cMap[nativePixel * 3 + 2]);
				for (int yy = 0; yy < scale; ++yy) {
					for (int xx = 0; xx < scale; ++xx)
						_roseTattooHiresComposite.setPixel(x * scale + xx, y * scale + yy, outputPixel);
				}
			}
		}

#ifdef USE_FREETYPE2
		blendRoseTattooHiresTextLayer();
#endif
		blendRoseTattooHiresSceneSpriteLayer();
		blendRoseTattooHiresSpriteLayer();
		g_system->copyRectToScreen((const byte *)_roseTattooHiresComposite.getPixels(),
			_roseTattooHiresComposite.pitch, 0, 0, outWidth, outHeight);
		updateScreen();
		_dirtyRects.clear();
		return;
	}

	const int bgOffsetX = _currentScroll.x * scale;
	const int bgOffsetY = _currentScroll.y * scale;
	const uint32 maskOff = _roseTattooHiresFormat.isCLUT8() ?
		closestPaletteEntry(_cMap, 0, 0, 0) :
		_roseTattooHiresFormat.RGBToColor(0, 0, 0);
	const uint32 maskOn = _roseTattooHiresFormat.isCLUT8() ?
		closestPaletteEntry(_cMap, 255, 0, 255) :
		_roseTattooHiresFormat.RGBToColor(255, 0, 255);

	if (_roseTattooHiresDebugMode == kRoseTattooHiresMask) {
		_roseTattooHiresComposite.clear(maskOff);
	} else if (_roseTattooHiresDebugMode == kRoseTattooHiresNative) {
		_roseTattooHiresComposite.clear(maskOff);
	} else {
		for (int y = 0; y < outHeight; ++y) {
			for (int x = 0; x < outWidth; ++x) {
				uint32 color = 0;
				const int bgX = bgOffsetX + x;
				const int bgY = bgOffsetY + y;
				if (bgX >= 0 && bgY >= 0 && bgX < _roseTattooHiresBackground.w && bgY < _roseTattooHiresBackground.h)
					color = _roseTattooHiresBackground.getPixel(bgX, bgY);
				_roseTattooHiresComposite.setPixel(x, y, color);
			}
		}
	}

	if (_roseTattooHiresDebugMode == kRoseTattooHiresBackground) {
#ifdef USE_FREETYPE2
		blendRoseTattooHiresTextLayer();
#endif
		blendRoseTattooHiresSceneSpriteLayer();
		blendRoseTattooHiresSpriteLayer();
		g_system->copyRectToScreen((const byte *)_roseTattooHiresComposite.getPixels(),
			_roseTattooHiresComposite.pitch, 0, 0, outWidth, outHeight);
		updateScreen();
		_dirtyRects.clear();
		return;
	}

	const byte *screenPixels = (const byte *)getPixels();
	const byte *backgroundPixels = (const byte *)_backBuffer2.getPixels();
	for (int y = 0; y < SHERLOCK_SCREEN_HEIGHT; ++y) {
		const byte *screenRow = screenPixels + y * pitch;
		const int bgNativeY = y + _currentScroll.y;
		if (bgNativeY < 0 || bgNativeY >= _backBuffer2.h)
			continue;

		const byte *bgRow = backgroundPixels + bgNativeY * _backBuffer2.pitch;
		for (int x = 0; x < SHERLOCK_SCREEN_WIDTH; ++x) {
			const int bgNativeX = x + _currentScroll.x;
			if (bgNativeX < 0 || bgNativeX >= _backBuffer2.w)
				continue;

			const byte nativePixel = screenRow[x];
			const bool overlayPixel = _roseTattooHiresDebugMode == kRoseTattooHiresNative || nativePixel != bgRow[bgNativeX];
			if (!overlayPixel)
				continue;

			const uint32 outputPixel = _roseTattooHiresDebugMode == kRoseTattooHiresMask ? maskOn :
				_roseTattooHiresFormat.isCLUT8() ? nativePixel :
				_roseTattooHiresFormat.RGBToColor(_cMap[nativePixel * 3],
					_cMap[nativePixel * 3 + 1], _cMap[nativePixel * 3 + 2]);
			for (int yy = 0; yy < scale; ++yy) {
				for (int xx = 0; xx < scale; ++xx)
					_roseTattooHiresComposite.setPixel(x * scale + xx, y * scale + yy, outputPixel);
			}
		}
	}

#ifdef USE_FREETYPE2
	blendRoseTattooHiresTextLayer();
#endif
	blendRoseTattooHiresSceneSpriteLayer();
	blendRoseTattooHiresSpriteLayer();
	g_system->copyRectToScreen((const byte *)_roseTattooHiresComposite.getPixels(),
		_roseTattooHiresComposite.pitch, 0, 0, outWidth, outHeight);
	updateScreen();
	_dirtyRects.clear();
}

void Screen::activateBackBuffer1() {
	_backBuffer.create(_backBuffer1, _backBuffer1.getBounds());
}

void Screen::activateBackBuffer2() {
	_backBuffer.create(_backBuffer2, _backBuffer2.getBounds());
}

int Screen::equalizePalette(const byte palette[Graphics::PALETTE_SIZE]) {
	int total = 0;
	byte tempPalette[Graphics::PALETTE_SIZE];
	getPalette(tempPalette);

	// For any palette component that doesn't already match the given destination
	// palette, change by 1 towards the reference palette component
	for (int idx = 0; idx < Graphics::PALETTE_SIZE; ++idx) {
		if (tempPalette[idx] > palette[idx]) {
			tempPalette[idx] = MAX((int)palette[idx], (int)tempPalette[idx] - 4);
			++total;
		} else if (tempPalette[idx] < palette[idx]) {
			tempPalette[idx] = MIN((int)palette[idx], (int)tempPalette[idx] + 4);
			++total;
		}
	}

	if (total > 0)
		// Palette changed, so reload it
		setPalette(tempPalette);

	return total;
}

void Screen::fadeToBlack(int speed) {
	byte tempPalette[Graphics::PALETTE_SIZE];
	Common::fill(&tempPalette[0], &tempPalette[Graphics::PALETTE_SIZE], 0);

	while (equalizePalette(tempPalette)) {
		_vm->_events->delay(15 * speed);
	}

	setPalette(tempPalette);
	fillRect(Common::Rect(0, 0, this->w, this->h), 0);
}

void Screen::fadeIn(const byte palette[Graphics::PALETTE_SIZE], int speed) {
	int count = 50;
	while (equalizePalette(palette) && --count) {
		_vm->_events->delay(15 * speed);
	}

	setPalette(palette);
}

void Screen::randomTransition() {
	Events &events = *_vm->_events;
	const int TRANSITION_MULTIPLIER = 0x15a4e35;
	clearDirtyRects();
	assert(IS_SERRATED_SCALPEL);

	for (int idx = 0; idx <= 65535 && !_vm->shouldQuit(); ++idx) {
		_transitionSeed = _transitionSeed * TRANSITION_MULTIPLIER + 1;
		int offset = _transitionSeed & 0xFFFF;

		if (offset < (this->width() * this->height()))
			*((byte *)getPixels() + offset) = *((const byte *)_backBuffer.getPixels() + offset);

		if (idx != 0 && (idx % 300) == 0) {
			// Ensure there's a full screen dirty rect for the next frame update
			if (!isDirty())
				addDirtyRect(Common::Rect(0, 0, this->w, this->h));

			events.pollEvents();
			events.delay(1);
		}
	}

	// Make sure everything has been transferred
	SHblitFrom(_backBuffer);
}

void Screen::verticalTransition() {
	Events &events = *_vm->_events;

	byte table[640];
	Common::fill(&table[0], &table[640], 0);

	for (int yp = 0; yp < this->height(); ++yp) {
		for (int xp = 0; xp < this->width(); ++xp) {
			int temp = (table[xp] >= (this->height() - 3)) ? this->height() - table[xp] :
				_vm->getRandomNumber(3) + 1;

			if (temp) {
				SHblitFrom(_backBuffer1, Common::Point(xp, table[xp]),
					Common::Rect(xp, table[xp], xp + 1, table[xp] + temp));
				table[xp] += temp;
			}
		}

		events.delay(10);
	}
}

void Screen::restoreBackground(const Common::Rect &r) {
	if (r.width() > 0 && r.height() > 0)
		_backBuffer.SHblitFrom(_backBuffer2, Common::Point(r.left, r.top), r);
}

void Screen::slamArea(int16 xp, int16 yp, int16 width_, int16 height_) {
	slamRect(Common::Rect(xp, yp, xp + width_, yp + height_));
}

void Screen::slamRect(const Common::Rect &r) {
	if (r.width() && r.height() > 0) {
		Common::Rect srcRect = r, destRect = r;

		destRect.translate(-_currentScroll.x, -_currentScroll.y);

		if (destRect.left < 0) {
			srcRect.left += -destRect.left;
			destRect.left = 0;
		}
		if (destRect.top < 0) {
			srcRect.top += -destRect.top;
			destRect.top = 0;
		}
		if (destRect.right > SHERLOCK_SCREEN_WIDTH) {
			srcRect.right -= (destRect.left - SHERLOCK_SCREEN_WIDTH);
			destRect.right = SHERLOCK_SCREEN_WIDTH;
		}
		if (destRect.bottom > SHERLOCK_SCREEN_HEIGHT) {
			srcRect.bottom -= (destRect.bottom - SHERLOCK_SCREEN_HEIGHT);
			destRect.bottom = SHERLOCK_SCREEN_HEIGHT;
		}

		if (srcRect.isValidRect())
			SHblitFrom(_backBuffer, Common::Point(destRect.left, destRect.top), srcRect);
	}
}

void Screen::flushImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width_, int16 *height_) {
	Common::Point imgPos = pt + frame->_offset;
	Common::Rect newBounds(imgPos.x, imgPos.y, imgPos.x + frame->_frame.w, imgPos.y + frame->_frame.h);
	Common::Rect oldBounds(*xp, *yp, *xp + *width_, *yp + *height_);

	if (!_flushScreen) {
		// See if the areas of the old and new overlap, and if so combine the areas
		if (newBounds.intersects(oldBounds)) {
			Common::Rect mergedBounds = newBounds;
			mergedBounds.extend(oldBounds);
			mergedBounds.right += 1;
			mergedBounds.bottom += 1;

			slamRect(mergedBounds);
		} else {
			// The two areas are independent, so copy them both
			slamRect(newBounds);
			slamRect(oldBounds);
		}
	}

	*xp = newBounds.left;
	*yp = newBounds.top;
	*width_ = newBounds.width();
	*height_ = newBounds.height();
}

void Screen::flushScaleImage(ImageFrame *frame, const Common::Point &pt, int16 *xp, int16 *yp,
		int16 *width_, int16 *height_, int scaleVal) {
	Common::Point imgPos(pt.x + frame->sDrawXOffset(scaleVal), pt.y + frame->sDrawYOffset(scaleVal));
	Common::Rect newBounds(imgPos.x, imgPos.y, imgPos.x + frame->sDrawXSize(scaleVal),
		imgPos.y + frame->sDrawYSize(scaleVal));
	Common::Rect oldBounds(*xp, *yp, *xp + *width_, *yp + *height_);

	if (!_flushScreen) {
		// See if the areas of the old and new overlap, and if so combine the areas
		if (newBounds.intersects(oldBounds)) {
			Common::Rect mergedBounds = newBounds;
			mergedBounds.extend(oldBounds);
			mergedBounds.right += 1;
			mergedBounds.bottom += 1;

			slamRect(mergedBounds);
		} else {
			// The two areas are independent, so copy them both
			slamRect(newBounds);
			slamRect(oldBounds);
		}
	}

	*xp = newBounds.left;
	*yp = newBounds.top;
	*width_ = newBounds.width();
	*height_ = newBounds.height();
}

void Screen::flushImage(ImageFrame *frame, const Common::Point &pt, Common::Rect &newBounds, int scaleVal) {
	Common::Point newPos(newBounds.left, newBounds.top);
	Common::Point newSize(newBounds.width(), newBounds.height());

	if (scaleVal == SCALE_THRESHOLD)
		flushImage(frame, pt, &newPos.x, &newPos.y, &newSize.x, &newSize.y);
	else
		flushScaleImage(frame, pt, &newPos.x, &newPos.y, &newSize.x, &newSize.y, scaleVal);

	// Transfer the pos and size amounts into a single bounds rect
	newBounds = Common::Rect(newPos.x, newPos.y, newPos.x + newSize.x, newPos.y + newSize.y);
}

void Screen::blockMove(const Common::Rect &r) {
	Common::Rect bounds = r;
	slamRect(bounds);
}

void Screen::blockMove() {
	blockMove(Common::Rect(0, 0, width(), height()));
}

void Screen::print(const Common::Point &pt, uint color, const char *formatStr, ...) {
	// Create the string to display
	va_list args;
	va_start(args, formatStr);
	Common::String str = Common::String::vformat(formatStr, args);
	va_end(args);

	// Figure out area to draw text in
	Common::Point pos = pt;
	int width_ = stringWidth(str);
	pos.y--;		// Font is always drawing one line higher
	if (!pos.x)
		// Center text horizontally
		pos.x = (this->width() - width_) / 2;

	Common::Rect textBounds(pos.x, pos.y, pos.x + width_, pos.y + fontHeight());
	if (textBounds.right > this->width())
		textBounds.moveTo(this->width() - width_, textBounds.top);
	if (textBounds.bottom > this->height())
		textBounds.moveTo(textBounds.left, this->height() - fontHeight());

	// Write out the string at the given position
	writeString(str, Common::Point(textBounds.left, textBounds.top), color);

	// Copy the affected area to the screen
	slamRect(textBounds);
}

void Screen::gPrint(const Common::Point &pt, uint color, const char *formatStr, ...) {
	// Create the string to display
	va_list args;
	va_start(args, formatStr);
	Common::String str = Common::String::vformat(formatStr, args);
	va_end(args);

	// Print the text
	writeString(str, pt, color);
}

void Screen::writeString(const Common::String &str, const Common::Point &pt, uint overrideColor) {
	Fonts::writeString(&_backBuffer, str, pt, overrideColor);
}

void Screen::vgaBar(const Common::Rect &r, int color) {
	_backBuffer.fillRect(r, color);
	slamRect(r);
}

void Screen::setDisplayBounds(const Common::Rect &r) {
	_backBuffer.create(_backBuffer1, r);
	assert(_backBuffer.width()  == r.width());
	assert(_backBuffer.height() == r.height());
}

void Screen::resetDisplayBounds() {
	_backBuffer.create(_backBuffer1, _backBuffer1.getBounds());
}

Common::Rect Screen::getDisplayBounds() {
	return _backBuffer.getBounds();
}

void Screen::synchronize(Serializer &s) {
	int fontNumb = fontNumber();
	s.syncAsByte(fontNumb);
	if (s.isLoading())
		setFont(fontNumb);
}

void Screen::initPaletteFade(int bytesToRead) {
	Common::copy(&_cMap[0], &_cMap[Graphics::PALETTE_SIZE], &_sMap[0]);
	Common::copy(&_cMap[0], &_cMap[Graphics::PALETTE_SIZE], &_tMap[0]);

	// Set how many bytes need to be read / have been read
	_fadeBytesRead = 0;
	_fadeBytesToRead = bytesToRead;
	_oldFadePercent = 0;
}

int Screen::fadeRead(Common::SeekableReadStream &stream, byte *buf, int totalSize) {
	warning("TODO: fadeRead");
	stream.read(buf, totalSize);
	return totalSize;
}

void Screen::translatePalette(byte palette[Graphics::PALETTE_SIZE]) {
	for (int idx = 0; idx < Graphics::PALETTE_SIZE; ++idx)
		palette[idx] = PALETTE_6BIT_TO_8BIT(palette[idx]);
}

} // End of namespace Sherlock
