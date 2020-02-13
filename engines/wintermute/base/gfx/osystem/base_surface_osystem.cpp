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
#include "graphics/transparent_surface.h"
#include "graphics/transform_tools.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "common/stream.h"
#include "common/system.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseSurfaceOSystem::BaseSurfaceOSystem(BaseGame *inGame) : BaseSurface(inGame) {
	_surface = new Graphics::Surface();
	_alphaMask = nullptr;
	_alphaType = Graphics::ALPHA_FULL;
	_lockPixels = nullptr;
	_lockPitch = 0;
	_loaded = false;
	_rotation = 0;
}

//////////////////////////////////////////////////////////////////////////
BaseSurfaceOSystem::~BaseSurfaceOSystem() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	delete[] _alphaMask;
	_alphaMask = nullptr;

	_gameRef->addMem(-_width * _height * 4);
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);
	renderer->invalidateTicketsFromSurface(this);
}

Graphics::AlphaType hasTransparencyType(const Graphics::Surface *surf) {
	if (surf->format.bytesPerPixel != 4) {
		warning("hasTransparencyType:: non 32 bpp surface passed as argument");
		return Graphics::ALPHA_OPAQUE;
	}
	uint8 r, g, b, a;
	bool seenAlpha = false;
	bool seenFullAlpha = false;
	for (int i = 0; i < surf->h; i++) {
		if (seenFullAlpha) {
			break;
		}
		for (int j = 0; j < surf->w; j++) {
			uint32 pix = *(const uint32 *)surf->getBasePtr(j, i);
			surf->format.colorToARGB(pix, a, r, g, b);
			if (a != 255) {
				seenAlpha = true;
				if (a != 0) {
					seenFullAlpha = true;
					break;
				}
			}
		}
	}
	if (seenFullAlpha) {
		return Graphics::ALPHA_FULL;
	} else if (seenAlpha) {
		return Graphics::ALPHA_BINARY;
	} else {
		return Graphics::ALPHA_OPAQUE;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	/*  BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer); */
	_filename = filename;
//	const Graphics::Surface *surface = image->getSurface();

	if (defaultCK) {
		ckRed   = 255;
		ckGreen = 0;
		ckBlue  = 255;
	}

	_ckDefault = defaultCK;
	_ckRed = ckRed;
	_ckGreen = ckGreen;
	_ckBlue = ckBlue;

	if (_lifeTime == 0 || lifeTime == -1 || lifeTime > _lifeTime) {
		_lifeTime = lifeTime;
	}

	_keepLoaded = keepLoaded;
	if (_keepLoaded) {
		_lifeTime = -1;
	}

	return STATUS_OK;
}

bool BaseSurfaceOSystem::finishLoad() {
	BaseImage *image = new BaseImage();
	if (!image->loadFile(_filename)) {
		delete image;
		return false;
	}

	_width = image->getSurface()->w;
	_height = image->getSurface()->h;

	bool isSaveGameGrayscale = _filename.matchString("savegame:*g", true);
	if (isSaveGameGrayscale) {
		warning("grayscaleConversion not yet implemented");
		// FIBITMAP *newImg = FreeImage_ConvertToGreyscale(img); TODO
	}

	_surface->free();
	delete _surface;

	bool needsColorKey = false;
	bool replaceAlpha = true;
	if (image->getSurface()->format.bytesPerPixel == 1) {
		if (!image->getPalette()) {
			error("Missing palette while loading 8bit image %s", _filename.c_str());
		}
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat(), image->getPalette());
		needsColorKey = true;
	} else {
		if (image->getSurface()->format != g_system->getScreenFormat()) {
			_surface = image->getSurface()->convertTo(g_system->getScreenFormat());
		} else {
			_surface = new Graphics::Surface();
			_surface->copyFrom(*image->getSurface());
		}

		if (_filename.hasSuffix(".bmp") && image->getSurface()->format.bytesPerPixel == 4) {
			// 32 bpp BMPs have nothing useful in their alpha-channel -> color-key
			needsColorKey = true;
			replaceAlpha = false;
		} else if (image->getSurface()->format.aBits() == 0) {
			needsColorKey = true;
		}
	}

	if (needsColorKey) {
		Graphics::TransparentSurface trans(*_surface);
		trans.applyColorKey(_ckRed, _ckGreen, _ckBlue, replaceAlpha);
	}

	_alphaType = hasTransparencyType(_surface);
	_valid = true;

	_gameRef->addMem(_width * _height * 4);

	delete image;

	// Bug #6572 WME: Rosemary - Sprite flaw on going upwards
	// Some Rosemary sprites have non-fully transparent pixels
	// In original WME it wasn't seen because sprites were downscaled
	// Let's set alpha to 0 if it is smaller then some treshold
	if (BaseEngine::instance().getGameId() == "rosemary" && _filename.hasPrefix("actors") && _surface->format.bytesPerPixel == 4) {
		uint8 treshold = 16;
		for (int x = 0; x < _surface->w; x++) {
			for (int y = 0; y < _surface->h; y++) {
				uint32 pixel = getPixelAt(_surface, x, y);
				uint8 r, g, b, a;
				_surface->format.colorToARGB(pixel, a, r, g, b);
				if (a > 0 && a < treshold) {
					uint32 *p = (uint32 *)_surface->getBasePtr(x, y);
					*p = _surface->format.ARGBToColor(0, 0, 0, 0);
				}
			}
		}
	}

	_loaded = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void BaseSurfaceOSystem::genAlphaMask(Graphics::Surface *surface) {
	warning("BaseSurfaceOSystem::GenAlphaMask - Not ported yet");
	return;
	// TODO: Reimplement this
	delete[] _alphaMask;
	_alphaMask = nullptr;
	if (!surface) {
		return;
	}

	bool hasColorKey;
	/* uint32 colorKey; */
	uint8 ckRed, ckGreen, ckBlue;
	/*  if (SDL_GetColorKey(surface, &colorKey) == 0) {
	        hasColorKey = true;
	        SDL_GetRGB(colorKey, surface->format, &ckRed, &ckGreen, &ckBlue);
	    } else hasColorKey = false;
	*/
	_alphaMask = new byte[surface->w * surface->h];

	bool hasTransparency = false;
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			uint32 pixel = getPixelAt(surface, x, y);

			uint8 r, g, b, a;
			surface->format.colorToARGB(pixel, a, r, g, b);
			//SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

			if (hasColorKey && r == ckRed && g == ckGreen && b == ckBlue) {
				a = 0;
			}

			_alphaMask[y * surface->w + x] = a;
			if (a < 255) {
				hasTransparency = true;
			}
		}
	}

	if (!hasTransparency) {
		delete[] _alphaMask;
		_alphaMask = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
uint32 BaseSurfaceOSystem::getPixelAt(Graphics::Surface *surface, int x, int y) {
	int bpp = surface->format.bytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	uint8 *p = (uint8 *)surface->getBasePtr(x, y);

	switch (bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(uint16 *)p;
		break;

	case 3:
#ifdef SCUMM_BIG_ENDIAN
		//  if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return p[0] << 16 | p[1] << 8 | p[2];
#else
		//else
		return p[0] | p[1] << 8 | p[2] << 16;
#endif
		break;

	case 4:
		return *(uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::create(int width, int height) {
	_width = width;
	_height = height;

	_gameRef->addMem(_width * _height * 4);

	_valid = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::isTransparentAt(int x, int y) {
	return isTransparentAtLite(x, y);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::isTransparentAtLite(int x, int y) {
	if (x < 0 || x >= _surface->w || y < 0 || y >= _surface->h) {
		return true;
	}

	if (_surface->format.bytesPerPixel == 4) {
		uint32 pixel = *(uint32 *)_surface->getBasePtr(x, y);
		uint8 r, g, b, a;
		_surface->format.colorToARGB(pixel, a, r, g, b);
		if (a <= 128) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::startPixelOp() {
	//SDL_LockTexture(_texture, nullptr, &_lockPixels, &_lockPitch);
	// Any pixel-op makes the caching useless:
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);
	renderer->invalidateTicketsFromSurface(this);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::endPixelOp() {
	//SDL_UnlockTexture(_texture);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::display(int x, int y, Rect32 rect, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr, Graphics::TransformStruct(Graphics::kDefaultZoomX, Graphics::kDefaultZoomY,  mirrorX, mirrorY));
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTrans(int x, int y, Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr, Graphics::TransformStruct(Graphics::kDefaultZoomX, Graphics::kDefaultZoomY, blendMode, alpha, mirrorX, mirrorY));
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransOffset(int x, int y, Rect32 rect, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr,  Graphics::TransformStruct(Graphics::kDefaultZoomX, Graphics::kDefaultZoomY, Graphics::kDefaultAngle, Graphics::kDefaultHotspotX, Graphics::kDefaultHotspotY, blendMode, alpha, mirrorX, mirrorY, offsetX, offsetY));
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	_rotation = 0;
	return drawSprite(x, y, &rect, nullptr, Graphics::TransformStruct((int32)zoomX, (int32)zoomY, blendMode, alpha, mirrorX, mirrorY));
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, bool transparent, Graphics::TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	_rotation = 0;
	Graphics::TransformStruct transform;
	if (transparent) {
		transform = Graphics::TransformStruct((int32)zoomX, (int32)zoomY, Graphics::kDefaultAngle, Graphics::kDefaultHotspotX, Graphics::kDefaultHotspotY, blendMode, alpha,  mirrorX, mirrorY);
	} else {
		transform = Graphics::TransformStruct((int32)zoomX, (int32)zoomY, mirrorX, mirrorY);
	}
	return drawSprite(x, y, &rect, nullptr, transform);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransform(int x, int y, Rect32 rect, Rect32 newRect, const Graphics::TransformStruct &transform) {
	_rotation = (uint32)transform._angle;
	if (transform._angle < 0.0f) {
		warning("Negative rotation: %d %d", transform._angle, _rotation);
		_rotation = (uint32)(360.0f + transform._angle);
		warning("Negative post rotation: %d %d", transform._angle, _rotation);
	}
	return drawSprite(x, y, &rect, &newRect, transform);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTiled(int x, int y, Rect32 rect, int numTimesX, int numTimesY) {
	assert(numTimesX > 0 && numTimesY > 0);
	Graphics::TransformStruct transform(numTimesX, numTimesY);
	return drawSprite(x, y, &rect, nullptr, transform);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::drawSprite(int x, int y, Rect32 *rect, Rect32 *newRect, Graphics::TransformStruct transform) {
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);

	if (!_loaded) {
		finishLoad();
	}

	if (renderer->_forceAlphaColor != 0) {
		transform._rgbaMod = renderer->_forceAlphaColor;
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
	if (_alphaType == Graphics::ALPHA_OPAQUE && transform._angle == 0) {
		transform._alphaDisable = true;
	}

	renderer->drawSurface(this, _surface, &srcRect, &position, transform);
	return STATUS_OK;
}

bool BaseSurfaceOSystem::putSurface(const Graphics::Surface &surface, bool hasAlpha) {
	_loaded = true;
	if (surface.format == _surface->format && surface.pitch == _surface->pitch && surface.h == _surface->h) {
		const byte *src = (const byte *)surface.getBasePtr(0, 0);
		byte *dst = (byte *)_surface->getBasePtr(0, 0);
		memcpy(dst, src, surface.pitch * surface.h);
	} else {
		_surface->free();
		_surface->copyFrom(surface);
	}
	if (hasAlpha) {
		_alphaType = Graphics::ALPHA_FULL;
	} else {
		_alphaType = Graphics::ALPHA_OPAQUE;
	}
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);
	renderer->invalidateTicketsFromSurface(this);

	return STATUS_OK;
}

} // End of namespace Wintermute
