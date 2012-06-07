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

#ifndef WINTERMUTE_BSURFACE_H
#define WINTERMUTE_BSURFACE_H

#include "engines/wintermute/Base/BBase.h"
#include "graphics/surface.h"

namespace WinterMute {

class CBSurface: public CBBase {
public:
	virtual HRESULT invalidate();
	virtual HRESULT prepareToDraw();
	bool _cKDefault;
	byte _cKRed;
	byte _cKGreen;
	byte _cKBlue;

	uint32 _lastUsedTime;
	bool _valid;
	int _lifeTime;
	bool _keepLoaded;

	bool _pixelOpReady;
	CBSurface(CBGame *inGame);
	virtual ~CBSurface();

	virtual HRESULT displayHalfTrans(int x, int y, RECT rect);
	virtual bool isTransparentAt(int x, int y);
	virtual HRESULT displayTransZoom(int x, int y, RECT rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual HRESULT displayTrans(int x, int y, RECT rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual HRESULT displayTransOffset(int x, int y, RECT rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) = 0;
	virtual HRESULT display(int x, int y, RECT rect, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool MirrorY = false) = 0;
	virtual HRESULT displayZoom(int x, int y, RECT rect, float ZoomX, float ZoomY, uint32 alpha = 0xFFFFFFFF, bool transparent = false, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual HRESULT displayTransform(int x, int y, int hotX, int hotY, RECT rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual HRESULT restore();
	virtual HRESULT create(const char *Filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int LifeTime = -1, bool KeepLoaded = false) = 0;
	virtual HRESULT create(int Width, int Height);
	virtual HRESULT putSurface(const Graphics::Surface &surface) { return E_FAIL; }
	virtual HRESULT putPixel(int x, int y, byte r, byte g, byte b, int a = -1);
	virtual HRESULT getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a = NULL);
	virtual bool comparePixel(int x, int y, byte r, byte g, byte b, int a = -1);
	virtual HRESULT startPixelOp();
	virtual HRESULT endPixelOp();
	virtual bool isTransparentAtLite(int x, int y);
	void setFilename(const char *Filename);
	void setSize(int width, int height);

	int _referenceCount;
	char *_filename;

	virtual int getWidth() {
		return _width;
	}
	virtual int getHeight() {
		return _height;
	}
	//void SetWidth(int Width){ _width = Width;    }
	//void SetHeight(int Height){ _height = Height; }
protected:
	int _height;
	int _width;

};

} // end of namespace WinterMute

#endif
