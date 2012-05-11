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


#include "coll_templ.h"
#include "BBase.h"

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

	virtual void DumpData(const char *Filename) {};
	virtual CBImage *TakeScreenshot();
	virtual HRESULT SetViewport(int left, int top, int right, int bottom);
	virtual HRESULT SetViewport(RECT *Rect);
	virtual HRESULT SetScreenViewport();
	virtual HRESULT Fade(WORD Alpha);
	virtual HRESULT FadeToColor(uint32 Color, RECT *rect = NULL);
	virtual HRESULT DrawLine(int X1, int Y1, int X2, int Y2, uint32 Color);
	virtual HRESULT DrawRect(int X1, int Y1, int X2, int Y2, uint32 Color, int Width = 1);
	CBRenderer(CBGame *inGame = NULL);
	virtual ~CBRenderer();
	virtual HRESULT SetProjection() {
		return S_OK;
	};

	virtual HRESULT WindowedBlt();
	virtual HRESULT Fill(byte  r, byte g, byte b, RECT *rect = NULL);
	virtual void OnWindowChange();
	virtual HRESULT InitRenderer(int width, int height, bool windowed);
	virtual HRESULT Flip();
	virtual void InitLoop();
	virtual HRESULT SwitchFullscreen();
	virtual HRESULT Setup2D(bool Force = false);
	virtual HRESULT SetupLines();

	virtual const char *GetName() {
		return "";
	};
	virtual HRESULT DisplayDebugInfo() {
		return E_FAIL;
	};
	virtual HRESULT DrawShaderQuad() {
		return E_FAIL;
	}

	virtual float GetScaleRatioX() const {
		return 1.0f;
	}
	virtual float GetScaleRatioY() const {
		return 1.0f;
	}

	HRESULT ClipCursor();
	HRESULT UnclipCursor();

	CBObject *GetObjectAt(int X, int Y);
	void DeleteRectList();

	virtual HRESULT StartSpriteBatch() {
		return S_OK;
	};
	virtual HRESULT EndSpriteBatch() {
		return S_OK;
	};
	bool PointInViewport(POINT *P);
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
