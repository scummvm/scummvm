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
	virtual ERRORCODE setViewport(int left, int top, int right, int bottom);
	virtual ERRORCODE setViewport(Common::Rect *Rect);
	virtual ERRORCODE setScreenViewport();
	virtual ERRORCODE fade(uint16 Alpha);
	virtual ERRORCODE fadeToColor(uint32 Color, Common::Rect *rect = NULL);
	virtual ERRORCODE drawLine(int x1, int y1, int x2, int y2, uint32 color);
	virtual ERRORCODE drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1);
	CBRenderer(CBGame *inGame = NULL);
	virtual ~CBRenderer();
	virtual ERRORCODE setProjection() {
		return STATUS_OK;
	};

	virtual ERRORCODE windowedBlt();
	virtual ERRORCODE fill(byte  r, byte g, byte b, Common::Rect *rect = NULL);
	virtual void onWindowChange();
	virtual ERRORCODE initRenderer(int width, int height, bool windowed);
	virtual ERRORCODE flip();
	virtual void initLoop();
	virtual ERRORCODE switchFullscreen();
	virtual ERRORCODE setup2D(bool force = false);
	virtual ERRORCODE setupLines();

	virtual const char *getName() {
		return "";
	};
	virtual ERRORCODE displayDebugInfo() {
		return STATUS_FAILED;
	};
	virtual ERRORCODE drawShaderQuad() {
		return STATUS_FAILED;
	}

	virtual float getScaleRatioX() const {
		return 1.0f;
	}
	virtual float getScaleRatioY() const {
		return 1.0f;
	}

	ERRORCODE clipCursor();
	ERRORCODE unclipCursor();

	CBObject *getObjectAt(int x, int y);
	void deleteRectList();

	virtual ERRORCODE startSpriteBatch() {
		return STATUS_OK;
	};
	virtual ERRORCODE endSpriteBatch() {
		return STATUS_OK;
	};
	bool pointInViewport(Common::Point *P);
	uint32 _forceAlphaColor;
	uint32 _window;
	uint32 _clipperWindow;
	bool _active;
	bool _ready;
	bool _windowed;
	Common::Rect _windowRect;
	Common::Rect _viewportRect;
	Common::Rect _screenRect;
	Common::Rect _monitorRect;
	int _bPP;
	int _height;
	int _width;

	CBArray<CBActiveRect *, CBActiveRect *> _rectList;
};

} // end of namespace WinterMute

#endif
