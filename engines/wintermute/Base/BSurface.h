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
#include "engines/wintermute/Math/Rect32.h"
#include "graphics/surface.h"

namespace WinterMute {

class CBSurface: public CBBase {
public:
	virtual ERRORCODE invalidate();
	virtual ERRORCODE prepareToDraw();
	bool _ckDefault;
	byte _ckRed;
	byte _ckGreen;
	byte _ckBlue;

	uint32 _lastUsedTime;
	bool _valid;
	int _lifeTime;
	bool _keepLoaded;

	bool _pixelOpReady;
	CBSurface(CBGame *inGame);
	virtual ~CBSurface();

	virtual ERRORCODE displayHalfTrans(int x, int y, Rect32 rect);
	virtual bool isTransparentAt(int x, int y);
	virtual ERRORCODE displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual ERRORCODE displayTrans(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual ERRORCODE displayTransOffset(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) = 0;
	virtual ERRORCODE display(int x, int y, Rect32 rect, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool MirrorY = false) = 0;
	virtual ERRORCODE displayZoom(int x, int y, Rect32 rect, float ZoomX, float ZoomY, uint32 alpha = 0xFFFFFFFF, bool transparent = false, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual ERRORCODE displayTransform(int x, int y, int hotX, int hotY, Rect32 rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual ERRORCODE restore();
	virtual ERRORCODE create(const char *filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false) = 0;
	virtual ERRORCODE create(int Width, int Height);
	virtual ERRORCODE putSurface(const Graphics::Surface &surface, bool hasAlpha = false) {
		return STATUS_FAILED;
	}
	virtual ERRORCODE putPixel(int x, int y, byte r, byte g, byte b, int a = -1);
	virtual ERRORCODE getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a = NULL);
	virtual bool comparePixel(int x, int y, byte r, byte g, byte b, int a = -1);
	virtual ERRORCODE startPixelOp();
	virtual ERRORCODE endPixelOp();
	virtual bool isTransparentAtLite(int x, int y);
	void setSize(int width, int height);

	int _referenceCount;

	virtual int getWidth() {
		return _width;
	}
	virtual int getHeight() {
		return _height;
	}
	Common::String getFileNameStr() { return _filename; }
	const char* getFileName() { return _filename.c_str(); }
	//void SetWidth(int Width){ _width = Width;    }
	//void SetHeight(int Height){ _height = Height; }
protected:
	Common::String _filename;
	int _height;
	int _width;

};

} // end of namespace WinterMute

#endif
