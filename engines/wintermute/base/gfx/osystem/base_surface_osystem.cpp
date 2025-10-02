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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"
#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/platform_osystem.h"

#include "graphics/managed_surface.h"
#include "graphics/transform_tools.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/stream.h"
#include "common/system.h"

#define TS_COLOR(wmeColor) \
	MS_ARGB(RGBCOLGetA(wmeColor), RGBCOLGetR(wmeColor), RGBCOLGetG(wmeColor), RGBCOLGetB(wmeColor))

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseSurfaceOSystem::BaseSurfaceOSystem(BaseGame *inGame) : BaseSurface(inGame) {
	_surface = new Graphics::Surface();
	_pixelOpReady = false;
	_alphaMask = nullptr;
	_alphaType = Graphics::ALPHA_FULL;
	_alphaMaskType = Graphics::ALPHA_OPAQUE;
	_rotation = 0;
	_surfaceModified = false;
}

//////////////////////////////////////////////////////////////////////////
BaseSurfaceOSystem::~BaseSurfaceOSystem() {
	if (_surface) {
		if (_valid)
			_game->addMem(-_width * _height * 4);
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	if (_alphaMask) {
		_alphaMask->free();
		delete _alphaMask;
		_alphaMask = nullptr;
	}

	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_game->_renderer);
	renderer->invalidateTicketsFromSurface(this);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::create(const char *filename, bool texture2D, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	if (defaultCK) {
		ckRed   = 255;
		ckGreen = 0;
		ckBlue  = 255;
	}

	BaseImage img = BaseImage();
	if (!img.getImageInfo(filename, _width, _height)) {
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

	return STATUS_OK;
}

bool BaseSurfaceOSystem::loadImage() {
	if (!_filename || !_filename[0]) {
		return false;
	}
	Common::String filename = _filename;

	BaseImage *image = new BaseImage();
	if (!image->loadFile(filename)) {
		delete image;
		return false;
	}

	if (_surface) {
		if (_valid)
			_game->addMem(-_width * _height * 4);
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	_width = image->getSurface()->w;
	_height = image->getSurface()->h;

	bool needsColorKey = false;
	bool replaceAlpha = true;
	if (image->getSurface()->format.bytesPerPixel == 1) {
		if (!image->getPalette()) {
			error("Missing palette while loading 8bit image %s", _filename);
		}
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat(), image->getPalette(), image->getPaletteCount());
	} else if (image->getSurface()->format != g_system->getScreenFormat()) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat());
	} else {
		_surface = new Graphics::Surface();
		_surface->copyFrom(*image->getSurface());
	}

	_game->addMem(_width * _height * 4);

	if (filename.matchString("savegame:*g", true)) {
		uint8 r, g, b, a;
		for (int x = 0; x < _surface->w; x++) {
			for (int y = 0; y < _surface->h; y++) {
				_surface->format.colorToARGB(_surface->getPixel(x, y), a, r, g, b);
				uint8 grey = (uint8)((0.2126f * r + 0.7152f * g + 0.0722f * b) + 0.5f);
				_surface->setPixel(x, y, _surface->format.ARGBToColor(a, grey, grey, grey));
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
	} else if (image->getSurface()->format.aBits() == 0) {
		// generic WME Lite does not use colorkey for non-BMPs with transparency
		needsColorKey = true;
	}

	if (needsColorKey) {
		// We set the pixel color to transparent black,
		// like D3DX, if it matches the color key.
		bool applied = _surface->applyColorKey(_ckRed, _ckGreen, _ckBlue, replaceAlpha, 0, 0, 0);

		if (replaceAlpha || image->getSurface()->format.aBits() == 0 || image->getSurface()->format.isCLUT8())
			_alphaType = applied ? Graphics::ALPHA_BINARY : Graphics::ALPHA_OPAQUE;
		else
			_alphaType = _surface->detectAlpha();
	} else {
		_alphaType = image->getSurface()->detectAlpha();
	}

	_valid = true;

	delete image;

	// Bug #6572 WME: Rosemary - Sprite flaw on going upwards
	// Some Rosemary sprites have non-fully transparent pixels
	// In original WME it wasn't seen because sprites were downscaled
	// Let's set alpha to 0 if it is smaller then some treshold
	if (BaseEngine::instance().getGameId() == "rosemary" && filename.hasPrefix("actors") &&
	    _alphaType == Graphics::ALPHA_FULL && _surface->format.aBits() > 4) {
		uint32 mask = _surface->format.ARGBToColor(255, 0, 0, 0);
		uint32 treshold = _surface->format.ARGBToColor(16, 0, 0, 0);
		uint32 blank = _surface->format.ARGBToColor(0, 0, 0, 0);

		for (int x = 0; x < _surface->w; x++) {
			for (int y = 0; y < _surface->h; y++) {
				uint32 pixel = _surface->getPixel(x, y);
				if ((pixel & mask) > blank && (pixel & mask) < treshold) {
					_surface->setPixel(x, y, blank);
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::create(int width, int height) {
	if (_valid)
		_game->addMem(-_width * _height * 4);
	_surface->free();

	_width = width;
	_height = height;

	_surface->create(_width, _height, g_system->getScreenFormat());
	_game->addMem(_width * _height * 4);

	_valid = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::invalidate() {
	if (_pixelOpReady) {
		return STATUS_FAILED;
	}

	if (_valid) {
		_game->addMem(-_width * _height * 4);
		_surface->free();
		_valid = false;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::isTransparentAtLite(int x, int y) const {
	if (!_pixelOpReady) {
		return false;
	}

	if (x < 0 || x >= _surface->w || y < 0 || y >= _surface->h) {
		return true;
	}

	uint32 pixel = _surface->getPixel(x, y);
	uint8 r, g, b, a;
	_surface->format.colorToARGB(pixel, a, r, g, b);
	if (a <= 128) {
		return true;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::startPixelOp() {
	if (!_valid) {
		if (DID_FAIL(loadImage())) {
			return STATUS_FAILED;
		}
	}
	_pixelOpReady = true;
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::endPixelOp() {
	_lastUsedTime = _game->_liveTimer;
	_pixelOpReady = false;
	if (_surfaceModified) {
		BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_game->_renderer);
		renderer->invalidateTicketsFromSurface(this);
		_surfaceModified = false;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::display(int x, int y, Common::Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr, Graphics::TransformStruct(Graphics::kDefaultZoomX, Graphics::kDefaultZoomY,  mirrorX, mirrorY));
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTrans(int x, int y, Common::Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr,  Graphics::TransformStruct(Graphics::kDefaultZoomX, Graphics::kDefaultZoomY, Graphics::kDefaultAngle, Graphics::kDefaultHotspotX, Graphics::kDefaultHotspotY, blendMode, TS_COLOR(alpha), mirrorX, mirrorY, offsetX, offsetY));
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransZoom(int x, int y, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr, Graphics::TransformStruct((int32)zoomX, (int32)zoomY, blendMode, TS_COLOR(alpha), mirrorX, mirrorY));
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransRotate(int x, int y, float rotate, int32 hotspotX, int32 hotspotY, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	Common::Point newHotspot;
	Common::Rect oldRect(rect.left, rect.top, rect.right, rect.bottom);
	Graphics::TransformStruct transform = Graphics::TransformStruct(zoomX, zoomY, rotate, hotspotX, hotspotY, blendMode, TS_COLOR(alpha), mirrorX, mirrorY, 0, 0);
	Common::Rect newRect = Graphics::TransformTools::newRect(oldRect, transform, &newHotspot);
	Common::Rect32 newRect32(newRect.left, newRect.top, newRect.right, newRect.bottom);

	x -= newHotspot.x;
	y -= newHotspot.y;

	_rotation = transform._angle;
	if (transform._angle < 0.0f) {
		warning("Negative rotation: %d %d", (int32)transform._angle, (int32)_rotation);
		_rotation = 360.0f + transform._angle;
		warning("Negative post rotation: %d %d", (int32)transform._angle, (int32)_rotation);
	}
	return drawSprite(x, y, &rect, &newRect32, transform);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTiled(int x, int y, Common::Rect32 rect, int numTimesX, int numTimesY) {
	assert(numTimesX > 0 && numTimesY > 0);
	Graphics::TransformStruct transform(numTimesX, numTimesY);
	return drawSprite(x, y, &rect, nullptr, transform);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::drawSprite(int x, int y, Common::Rect32 *rect, Common::Rect32 *newRect, Graphics::TransformStruct transform) {
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_game->_renderer);

	_lastUsedTime = _game->_liveTimer;

	// TODO: Skip this check if we can reuse an existing ticket?
	if (!_valid) {
		loadImage();
	}

	if (renderer->_forceAlphaColor != 0) {
		transform._rgbaMod = TS_COLOR(renderer->_forceAlphaColor);
	}

	// TODO: This _might_ miss the intended behaviour by 1 in each direction
	// But I think it fits the model used in Wintermute.
	Common::Rect srcRect;
	srcRect.left = rect->left;
	srcRect.top = rect->top;
	srcRect.setWidth(rect->right - rect->left);
	srcRect.setHeight(rect->bottom - rect->top);

	Common::Rect position;

	if (newRect) {
		position.top = y;
		position.left = x;
		position.setWidth(newRect->width());
		position.setHeight(newRect->height());
	} else {

		Common::Rect r;
		r.top = 0;
		r.left = 0;
		r.setWidth(rect->width());
		r.setHeight(rect->height());

		r = Graphics::TransformTools::newRect(r, transform, 0);

		position.top = r.top + y + transform._offset.y;
		position.left = r.left + x + transform._offset.x;
		position.setWidth(r.width() * transform._numTimesX);
		position.setHeight(r.height() * transform._numTimesY);
	}
	renderer->modTargetRect(&position);

	// TODO: This actually requires us to have the SAME source-offsets every time,
	// But no checking is in place for that yet.

	// Optimize by not doing alpha-blits if we lack alpha
	// If angle is not 0, then transparent regions are added near the corners
	if (_alphaType == Graphics::ALPHA_OPAQUE && _alphaMaskType == Graphics::ALPHA_OPAQUE &&
			transform._angle == 0) {
		transform._alphaDisable = true;
	}

	renderer->drawSurface(this, _surface, &srcRect, &position, transform);
	return STATUS_OK;
}

bool BaseSurfaceOSystem::putSurface(const Graphics::Surface &surface, bool hasAlpha) {
	_surface->copyRectToSurface(surface, 0, 0, Common::Rect(surface.w, surface.h));
	writeAlpha(_surface, _alphaMask);

	if (hasAlpha) {
		_alphaType = _surface->detectAlpha();
	} else {
		_alphaType = _alphaMaskType;
	}
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_game->_renderer);
	renderer->invalidateTicketsFromSurface(this);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::setAlphaImage(const char *filename) {
	BaseImage *alphaImage = new BaseImage();
	if (!alphaImage->loadFile(filename)) {
		delete alphaImage;
		return false;
	}

	if (_alphaMask) {
		_alphaMask->free();
		delete _alphaMask;
		_alphaMask = nullptr;
	}

	_alphaMaskType = alphaImage->getSurface()->detectAlpha();
	if (_alphaMaskType != Graphics::ALPHA_OPAQUE) {
		_alphaMask = alphaImage->getSurface()->convertTo(g_system->getScreenFormat());
	}

	delete alphaImage;

	return true;
}

void BaseSurfaceOSystem::writeAlpha(Graphics::Surface *surface, const Graphics::Surface *mask) {
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
