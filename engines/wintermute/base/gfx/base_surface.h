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

#ifndef WINTERMUTE_BASE_SURFACE_H
#define WINTERMUTE_BASE_SURFACE_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/math/rect32.h"
#include "graphics/surface.h"
#include "graphics/transform_struct.h"

namespace Wintermute {

class BaseSurface: public BaseClass {
public:
	virtual bool invalidate();
	virtual bool prepareToDraw();
	uint32 _lastUsedTime;
	bool _valid;
	int32 _lifeTime;

	bool _pixelOpReady;
	BaseSurface(BaseGame *inGame);
	~BaseSurface() override;

	virtual bool displayHalfTrans(int x, int y, Rect32 rect);
	virtual bool isTransparentAt(int x, int y);
	virtual bool displayTransRotate(int x, int y, uint32 angle, int32 hotspotX, int32 hotspotY, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual bool displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual bool displayTrans(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0) = 0;
	virtual bool display(int x, int y, Rect32 rect, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false) = 0;
	virtual bool displayTiled(int x, int y, Rect32 rect, int numTimesX, int numTimesY) = 0;
	virtual bool restore();
	virtual bool create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false) = 0;
	virtual bool create(int width, int height);
	virtual bool putSurface(const Graphics::Surface &surface, bool hasAlpha = false) {
		return STATUS_FAILED;
	}
	virtual bool putPixel(int x, int y, byte r, byte g, byte b, int a = -1);
	virtual bool getPixel(int x, int y, byte *r, byte *g, byte *b, byte *a = nullptr);
	virtual bool comparePixel(int x, int y, byte r, byte g, byte b, int a = -1);
	virtual bool startPixelOp();
	virtual bool endPixelOp();
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
	//void SetWidth(int Width) { _width = Width;    }
	//void SetHeight(int Height){ _height = Height; }
protected:
	bool _ckDefault;
	byte _ckRed;
	byte _ckGreen;
	byte _ckBlue;

	bool _keepLoaded;
	Common::String _filename;
	int32 _height;
	int32 _width;

};

} // End of namespace Wintermute

#endif
