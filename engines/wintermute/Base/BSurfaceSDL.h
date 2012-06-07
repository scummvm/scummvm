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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

#ifndef WINTERMUTE_BSURFACESDL_H
#define WINTERMUTE_BSURFACESDL_H

#include "graphics/surface.h"
#include "engines/wintermute/Base/BSurface.h"
#include "graphics/surface.h"

namespace WinterMute {

class CBSurfaceSDL : public CBSurface {
public:
	CBSurfaceSDL(CBGame *inGame);
	~CBSurfaceSDL();

	HRESULT create(const char *filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int lifeTime = -1, bool keepLoaded = false);
	HRESULT create(int Width, int Height);

	HRESULT createFromSDLSurface(Graphics::Surface *surface); //TODO: Rename function

	bool isTransparentAt(int x, int y);
	bool isTransparentAtLite(int x, int y);

	HRESULT startPixelOp();
	HRESULT endPixelOp();


	HRESULT displayTransZoom(int x, int y, RECT rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	HRESULT displayTrans(int x, int y, RECT rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	HRESULT displayTransOffset(int x, int y, RECT rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0);
	HRESULT display(int x, int y, RECT rect, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	HRESULT displayZoom(int x, int y, RECT rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, bool Transparent = false, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	HRESULT displayTransform(int x, int y, int HotX, int HotY, RECT Rect, float zoomX, float zoomY, uint32 alpha, float Rotate, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	virtual HRESULT putSurface(const Graphics::Surface &surface);
	/*  static unsigned DLL_CALLCONV ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	    static int DLL_CALLCONV SeekProc(fi_handle handle, long offset, int origin);
	    static long DLL_CALLCONV TellProc(fi_handle handle);*/
	virtual int getWidth() {
		if (_surface) {
			return _surface->w;
		}
		return _width;
	}
	virtual int getHeight() {
		if (_surface) {
			return _surface->h;
		}
		return _height;
	}

private:
//	SDL_Texture *_texture;
	Graphics::Surface *_surface;

	HRESULT drawSprite(int x, int y, RECT *Rect, float zoomX, float zoomY, uint32 alpha, bool alphaDisable, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX = 0, int offsetY = 0);
	void genAlphaMask(Graphics::Surface *surface);
	uint32 getPixel(Graphics::Surface *surface, int x, int y);

	void *_lockPixels;
	int _lockPitch;
	byte *_alphaMask;
};

} // end of namespace WinterMute

#endif // WINTERMUTE_BSURFACESDL_H
