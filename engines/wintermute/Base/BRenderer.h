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

#ifndef WINTERMUTE_BRENDERER_H
#define WINTERMUTE_BRENDERER_H


#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/Base/BBase.h"
#include "common/rect.h"

namespace WinterMute {

class CBImage;
class CBActiveRect;
class CBObject;

class CBRenderer: public CBBase {
public:
	int _realWidth;
	int _realHeight;
	int _drawOffsetX;
	int _drawOffsetY;

	virtual void dumpData(const char *filename) {};
	virtual CBImage *takeScreenshot();
	virtual HRESULT setViewport(int left, int top, int right, int bottom);
	virtual HRESULT setViewport(RECT *Rect);
	virtual HRESULT setScreenViewport();
	virtual HRESULT fade(uint16 Alpha);
	virtual HRESULT fadeToColor(uint32 Color, Common::Rect *rect = NULL);
	virtual HRESULT drawLine(int X1, int Y1, int X2, int Y2, uint32 Color);
	virtual HRESULT drawRect(int X1, int Y1, int X2, int Y2, uint32 Color, int Width = 1);
	CBRenderer(CBGame *inGame = NULL);
	virtual ~CBRenderer();
	virtual HRESULT setProjection() {
		return S_OK;
	};

	virtual HRESULT windowedBlt();
	virtual HRESULT fill(byte  r, byte g, byte b, Common::Rect *rect = NULL);
	virtual void onWindowChange();
	virtual HRESULT initRenderer(int width, int height, bool windowed);
	virtual HRESULT flip();
	virtual void initLoop();
	virtual HRESULT switchFullscreen();
	virtual HRESULT setup2D(bool Force = false);
	virtual HRESULT setupLines();

	virtual const char *getName() {
		return "";
	};
	virtual HRESULT displayDebugInfo() {
		return E_FAIL;
	};
	virtual HRESULT drawShaderQuad() {
		return E_FAIL;
	}

	virtual float getScaleRatioX() const {
		return 1.0f;
	}
	virtual float getScaleRatioY() const {
		return 1.0f;
	}

	HRESULT clipCursor();
	HRESULT unclipCursor();

	CBObject *getObjectAt(int X, int Y);
	void deleteRectList();

	virtual HRESULT startSpriteBatch() {
		return S_OK;
	};
	virtual HRESULT endSpriteBatch() {
		return S_OK;
	};
	bool pointInViewport(POINT *P);
	uint32 _forceAlphaColor;
	HINSTANCE _instance;
	HWND _window;
	HWND _clipperWindow;
	bool _active;
	bool _ready;
	bool _windowed;
	RECT _windowRect;
	RECT _viewportRect;
	RECT _screenRect;
	RECT _monitorRect;
	int _bPP;
	int _height;
	int _width;

	CBArray<CBActiveRect *, CBActiveRect *> _rectList;
};

} // end of namespace WinterMute

#endif
