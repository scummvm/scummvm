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

#include "common/algorithm.h"

#include "graphics/transform_tools.h"

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/gfx/base_image.h"

#if defined(USE_TINYGL)

#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/base/gfx/tinygl/base_surface_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/base_render_tinygl.h"

namespace Wintermute {

BaseSurfaceTinyGL::BaseSurfaceTinyGL(BaseGame *game, BaseRenderer3D *renderer)
: BaseSurface(game), _renderer(renderer), _imageData(nullptr), _maskData(nullptr), _pixelOpReady(false), _surfaceModified(false) {
	_blitImage = tglGenBlitImage();
}

BaseSurfaceTinyGL::~BaseSurfaceTinyGL() {
	_renderer->invalidateTexture(this);

	if (_imageData) {
		_imageData->free();
		delete _imageData;
		_imageData = nullptr;
	}

	if (_maskData) {
		_maskData->free();
		delete _maskData;
		_maskData = nullptr;
	}

	tglDeleteBlitImage(_blitImage);
}

bool BaseSurfaceTinyGL::invalidate() {
	_renderer->invalidateTexture(this);

	if (_imageData) {
		_imageData->free();
		delete _imageData;
		_imageData = nullptr;
	}

	_valid = false;
	_surfaceModified = false;
	return true;
}

bool BaseSurfaceTinyGL::prepareToDraw() {
	_lastUsedTime = _game->_liveTimer;

	if (!_valid) {
		loadImage();
	}

	return true;
}

bool BaseSurfaceTinyGL::displayTransZoom(int x, int y, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	prepareToDraw();

	_renderer->drawSprite(dynamic_cast<BaseSurface *>(this), rect, zoomX, zoomY, DXVector2(x, y), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceTinyGL::displayTrans(int x, int y, Common::Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	prepareToDraw();

	_renderer->drawSprite(dynamic_cast<BaseSurface *>(this), rect, 100, 100, DXVector2(x + offsetX, y + offsetY), alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceTinyGL::display(int x, int y, Common::Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	prepareToDraw();

	_renderer->drawSprite(dynamic_cast<BaseSurface *>(this), rect, 100, 100, DXVector2(x, y), 0xFFFFFFFF, true, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceTinyGL::displayTransRotate(int x, int y, float rotate, int32 hotspotX, int32 hotspotY, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	prepareToDraw();

	x -= hotspotX;
	y -= hotspotY;

	DXVector2 position(x, y);
	DXVector2 rotation;
	rotation._x = x + hotspotX * (zoomX / 100.0f);
	rotation._y = y + hotspotY * (zoomY / 100.0f);
	DXVector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	float angle = degToRad(rotate);

	_renderer->drawSpriteEx(dynamic_cast<BaseSurface *>(this), rect, position, rotation, scale, angle, alpha, false, blendMode, mirrorX, mirrorY);
	return true;
}

bool BaseSurfaceTinyGL::displayTiled(int x, int y, Common::Rect32 rect, int numTimesX, int numTimesY) {
	prepareToDraw();

	DXVector2 scale(numTimesX, numTimesY);
	_renderer->drawSpriteEx(dynamic_cast<BaseSurface *>(this), rect, DXVector2(x, y), DXVector2(0, 0), scale, 0, 0xFFFFFFFF, false, Graphics::BLEND_NORMAL, false, false);
	return true;
}

bool BaseSurfaceTinyGL::create(const char *filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	if (defaultCK) {
		ckRed = 255;
		ckGreen = 0;
		ckBlue = 255;
	}

	Common::String surfacefilename = filename;
	BaseImage img = BaseImage();
	if (!img.getImageInfo(surfacefilename, _width, _height)) {
		return false;
	}

	if (lifeTime != -1 && _lifeTime == 0) {
		_valid = false;
	}

	_ckDefault = defaultCK;
	_ckRed = ckRed;
	_ckGreen = ckGreen;
	_ckBlue = ckBlue;

	if (!_filename || scumm_stricmp(_filename, filename) != 0) {
		setFilename(filename);
	}

	if (_lifeTime == 0 || lifeTime == -1 || lifeTime > _lifeTime) {
		_lifeTime = lifeTime;
	}

	_keepLoaded = keepLoaded;
	if (_keepLoaded) {
		_lifeTime = -1;
	}

	return true;
}

bool BaseSurfaceTinyGL::loadImage() {
	if (!_filename || !_filename[0]) {
		return false;
	}
	Common::String filename = _filename;

	BaseImage img = BaseImage();
	if (!img.loadFile(filename)) {
		return false;
	}

	if (img.getSurface()->format.bytesPerPixel == 1 && img.getPalette() == nullptr) {
		return false;
	}

	bool needsColorKey = false;
	bool replaceAlpha = true;

	if (_imageData) {
		_imageData->free();
		delete _imageData;
		_imageData = nullptr;
	}

	_imageData = img.getSurface()->convertTo(Graphics::PixelFormat::createFormatRGBA32(), img.getPalette(), img.getPaletteCount());

	if (filename.matchString("savegame:*g", true)) {
		uint8 r, g, b, a;
		for (int x = 0; x < _imageData->w; x++) {
			for (int y = 0; y < _imageData->h; y++) {
				_imageData->format.colorToARGB(_imageData->getPixel(x, y), a, r, g, b);
				uint8 grey = (uint8)((0.2126f * r + 0.7152f * g + 0.0722f * b) + 0.5f);
				_imageData->setPixel(x, y, _imageData->format.ARGBToColor(a, grey, grey, grey));
			}
		}
	}

	if (filename.hasSuffix(".bmp")) {
		// Ignores alpha channel for BMPs
		needsColorKey = true;
	} else if (filename.hasSuffix(".jpg")) {
		// Ignores alpha channel for JPEGs
		needsColorKey = true;
	} else if (BaseEngine::instance().getTargetExecutable() < WME_LITE) {
		// WME 1.x always use colorkey, even for images with transparency
		needsColorKey = true;
		replaceAlpha = false;
	} else if (BaseEngine::instance().isFoxTail()) {
		// FoxTail does not use colorkey
		needsColorKey = false;
	} else if (img.getSurface()->format.aBits() == 0) {
		// generic WME Lite does not use colorkey for non-BMPs with transparency
		needsColorKey = true;
	}

	if (needsColorKey) {
		// We set the pixel color to transparent black,
		// like D3DX, if it matches the color key.
		_imageData->applyColorKey(_ckRed, _ckGreen, _ckBlue, replaceAlpha, 0, 0, 0);
	}

	// Bug #6572 WME: Rosemary - Sprite flaw on going upwards
	// Some Rosemary sprites have non-fully transparent pixels
	// In original WME it wasn't seen because sprites were downscaled
	// Let's set alpha to 0 if it is smaller then some treshold
	if (BaseEngine::instance().getGameId() == "rosemary" && filename.hasPrefix("actors") && _imageData->format.bytesPerPixel == 4) {
		uint32 mask = _imageData->format.ARGBToColor(255, 0, 0, 0);
		uint32 treshold = _imageData->format.ARGBToColor(16, 0, 0, 0);
		uint32 blank = _imageData->format.ARGBToColor(0, 0, 0, 0);

		for (int x = 0; x < _imageData->w; x++) {
			for (int y = 0; y < _imageData->h; y++) {
				uint32 pixel = _imageData->getPixel(x, y);
				if ((pixel & mask) > blank && (pixel & mask) < treshold) {
					_imageData->setPixel(x, y, blank);
				}
			}
		}
	}

	putSurface(*_imageData);

	/* TODO: Delete _imageData if we no longer need to access the pixel data? */

	_valid = true;

	return true;
}

bool BaseSurfaceTinyGL::create(int width, int height) {
	_width = width;
	_height = height;

	_valid = true;
	return true;
}

bool BaseSurfaceTinyGL::putSurface(const Graphics::Surface &surface, bool hasAlpha) {
	if (!_imageData) {
		_imageData = new Graphics::Surface();
	}

	if (_imageData && _imageData != &surface) {
		_imageData->copyFrom(surface);
		writeAlpha(_imageData, _maskData);
	}

	_width = surface.w;
	_height = surface.h;

	tglUploadBlitImage(_blitImage, *_imageData, 0, false);
	_valid = true;

	return true;
}

bool BaseSurfaceTinyGL::putPixel(int x, int y, byte r, byte g, byte b, byte a) {
	if (!_pixelOpReady) {
		return false;
	}

	if (x < 0 || y < 0 || x >= _width || y >= _height) {
		return false;
	}

	if (_imageData == nullptr) {
		return false;
	}

	_imageData->setPixel(x, y, _imageData->format.ARGBToColor(a, r, g, b));

	_surfaceModified = true;

	return true;
}

bool BaseSurfaceTinyGL::getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a) const {
	if (!_pixelOpReady) {
		return false;
	}

	if (x < 0 || y < 0 || x >= _width || y >= _height) {
		return false;
	}

	if (_imageData == nullptr) {
		return false;
	}

	uint8 alpha, red, green, blue;
	_imageData->format.colorToARGB(_imageData->getPixel(x, y), alpha, red, green, blue);
	*r = red;
	*g = green;
	*b = blue;
	*a = alpha;
	return true;
}

bool BaseSurfaceTinyGL::startPixelOp() {
	if (!prepareToDraw())
		return false;
	_pixelOpReady = true;
	return true;
}

bool BaseSurfaceTinyGL::endPixelOp() {
	_pixelOpReady = false;
	if (_surfaceModified) {
		tglUploadBlitImage(_blitImage, *_imageData, 0, false);
		_surfaceModified = false;
	}
	return true;
}

bool BaseSurfaceTinyGL::isTransparentAtLite(int x, int y) const {
	if (!_pixelOpReady) {
		return false;
	}

	if (x < 0 || y < 0 || x >= _width || y >= _height) {
		return false;
	}

	if (_imageData == nullptr) {
		return false;
	}

	uint8 a, r, g, b;
	_imageData->format.colorToARGB(_imageData->getPixel(x, y), a, r, g, b);
	return a == 0;
}

void BaseSurfaceTinyGL::setTexture() {
	prepareToDraw();
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceTinyGL::setAlphaImage(const char *filename) {
	BaseImage *alphaImage = new BaseImage();
	if (!alphaImage->loadFile(filename)) {
		delete alphaImage;
		return false;
	}

	if (_maskData) {
		_maskData->free();
		delete _maskData;
		_maskData = nullptr;
	}

	Graphics::AlphaType type = alphaImage->getSurface()->detectAlpha();
	if (type != Graphics::ALPHA_OPAQUE) {
		_maskData = alphaImage->getSurface()->convertTo(Graphics::PixelFormat::createFormatRGBA32());
	}

	delete alphaImage;

	return true;
}

void BaseSurfaceTinyGL::writeAlpha(Graphics::Surface *surface, const Graphics::Surface *mask) {
	if (mask && surface->w == mask->w && surface->h == mask->h) {
		assert(mask->pitch == mask->w * 4);
		assert(mask->format.bytesPerPixel == 4);
		assert(surface->pitch == surface->w * 4);
		assert(surface->format.bytesPerPixel == 4);
		const byte *alphaData = (const byte *)mask->getPixels();
#ifdef SCUMM_LITTLE_ENDIAN
		int alphaPlace = (mask->format.aShift / 8);
#else
		int alphaPlace = 3 - (mask->format.aShift / 8);
#endif
		alphaData += alphaPlace;
		byte *imgData = (byte *)surface->getPixels();
#ifdef SCUMM_LITTLE_ENDIAN
		imgData += (surface->format.aShift / 8);
#else
		imgData += 3 - (surface->format.aShift / 8);
#endif
		for (int i = 0; i < surface->w * surface->h; i++) {
			*imgData = *alphaData;
			alphaData += 4;
			imgData += 4;
		}
	}
}

} // End of namespace Wintermute

#endif // defined(USE_TINYGL)
