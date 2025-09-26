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

#ifndef WINTERMUTE_BASE_SURFACESDL_H
#define WINTERMUTE_BASE_SURFACESDL_H

#include "graphics/surface.h"
#include "graphics/transform_struct.h" // for Graphics::AlphaType

#include "engines/wintermute/base/gfx/base_surface.h"

#include "common/list.h"

namespace Wintermute {
class BaseImage;
class BaseSurfaceOSystem : public BaseSurface {
public:
	BaseSurfaceOSystem(BaseGame *inGame);
	~BaseSurfaceOSystem() override;

	bool create(const char *filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false) override;
	bool create(int width, int height) override;

	bool setAlphaImage(const char *filename) override;

	bool invalidate() override;

	bool isTransparentAtLite(int x, int y) const override;
	bool startPixelOp() override;
	bool endPixelOp() override;

	bool displayTransRotate(int x, int y, float rotate, int32 hotspotX, int32 hotspotY, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha = Graphics::kDefaultRgbaMod, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTransZoom(int x, int y, Common::Rect32 rect, float zoomX, float zoomY, uint32 alpha = Graphics::kDefaultRgbaMod, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTrans(int x, int y, Common::Rect32 rect, uint32 alpha = Graphics::kDefaultRgbaMod, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) override;
	bool display(int x, int y, Common::Rect32 rect, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTiled(int x, int y, Common::Rect32 rect, int numTimesX, int numTimesY) override;
	bool putSurface(const Graphics::Surface &surface, bool hasAlpha = false) override;
	int getWidth() override {
		return _width;
	}
	int getHeight() override {
		return _height;
	}
	bool putPixel(int x, int y, byte r, byte g, byte b, byte a) override {
		if (!_pixelOpReady) {
			return STATUS_FAILED;
		}
		if (_surface) {
			_surface->setPixel(x, y, _surface->format.ARGBToColor(a, r, g, b));
			_surfaceModified = true;
			return STATUS_OK;
		}
		return STATUS_FAILED;
	}
	bool getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a) const override {
		if (!_pixelOpReady) {
			return STATUS_FAILED;
		}
		if (_surface) {
			_surface->format.colorToARGB(_surface->getPixel(x, y), *a, *r, *g, *b);
			return STATUS_OK;
		}
		return STATUS_FAILED;
	}

	Graphics::AlphaType getAlphaType() const { return _alphaType; }
private:
	Graphics::Surface *_surface;
	bool loadImage();
	bool drawSprite(int x, int y, Common::Rect32 *rect, Common::Rect32 *newRect, Graphics::TransformStruct transformStruct);
	void writeAlpha(Graphics::Surface *surface, const Graphics::Surface *mask);

	bool _pixelOpReady;
	bool _surfaceModified;
	float _rotation;
	Graphics::AlphaType _alphaType;
	Graphics::Surface *_alphaMask;
	Graphics::AlphaType _alphaMaskType;
};

} // End of namespace Wintermute

#endif
