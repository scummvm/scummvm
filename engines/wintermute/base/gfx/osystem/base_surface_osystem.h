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

#ifndef WINTERMUTE_BASE_SURFACESDL_H
#define WINTERMUTE_BASE_SURFACESDL_H

#include "graphics/surface.h"
#include "graphics/transparent_surface.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "common/list.h"

namespace Wintermute {
struct TransparentSurface;
class BaseImage;
class BaseSurfaceOSystem : public BaseSurface {
public:
	BaseSurfaceOSystem(BaseGame *inGame);
	~BaseSurfaceOSystem() override;

	bool create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false) override;
	bool create(int width, int height) override;

	bool isTransparentAt(int x, int y) override;
	bool isTransparentAtLite(int x, int y) override;

	bool startPixelOp() override;
	bool endPixelOp() override;


	bool displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = Graphics::kDefaultRgbaMod, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTrans(int x, int y, Rect32 rect, uint32 alpha = Graphics::kDefaultRgbaMod, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTransOffset(int x, int y, Rect32 rect, uint32 alpha = Graphics::kDefaultRgbaMod, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) override;
	bool display(int x, int y, Rect32 rect, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = Graphics::kDefaultRgbaMod, bool transparent = false, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) override;
	bool displayTransform(int x, int y, Rect32 rect, Rect32 newRect, const Graphics::TransformStruct &transform) override;
	bool displayTiled(int x, int y, Rect32 rect, int numTimesX, int numTimesY) override;
	bool putSurface(const Graphics::Surface &surface, bool hasAlpha = false) override;
	/*  static unsigned DLL_CALLCONV ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	    static int DLL_CALLCONV SeekProc(fi_handle handle, long offset, int origin);
	    static long DLL_CALLCONV TellProc(fi_handle handle);*/
	int getWidth() override {
		if (!_loaded) {
			finishLoad();
		}
		if (_surface) {
			return _surface->w;
		}
		return _width;
	}
	int getHeight() override {
		if (!_loaded) {
			finishLoad();
		}
		if (_surface) {
			return _surface->h;
		}
		return _height;
	}
	bool getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a) override {
		if (!_loaded) {
			finishLoad();
		}
		if (_surface) {
			uint32 pixel = getPixelAt(_surface, x, y);
			_surface->format.colorToARGB(pixel, *a, *r, *g, *b);
			return STATUS_OK;
		}
		return STATUS_FAILED;
	}

	Graphics::AlphaType getAlphaType() const { return _alphaType; }
private:
	Graphics::Surface *_surface;
	bool _loaded;
	bool finishLoad();
	bool drawSprite(int x, int y, Rect32 *rect, Rect32 *newRect, Graphics::TransformStruct transformStruct);
	void genAlphaMask(Graphics::Surface *surface);
	uint32 getPixelAt(Graphics::Surface *surface, int x, int y);

	uint32 _rotation;
	Graphics::AlphaType _alphaType;
	void *_lockPixels;
	int _lockPitch;
	byte *_alphaMask;
};

} // End of namespace Wintermute

#endif
