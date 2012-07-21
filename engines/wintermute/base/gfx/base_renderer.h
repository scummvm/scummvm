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
#include "engines/wintermute/base/base.h"
#include "common/rect.h"

namespace WinterMute {

class BaseImage;
class BaseActiveRect;
class BaseObject;
class BaseSurface;
class BaseRenderer: public BaseClass {
public:
	int _realWidth;
	int _realHeight;
	int _drawOffsetX;
	int _drawOffsetY;

	virtual void dumpData(const char *filename) {};
	virtual BaseImage *takeScreenshot();
	virtual bool setViewport(int left, int top, int right, int bottom);
	virtual bool setViewport(Rect32 *Rect);
	virtual bool setScreenViewport();
	virtual bool fade(uint16 Alpha);
	virtual bool fadeToColor(uint32 Color, Common::Rect *rect = NULL);
	virtual bool drawLine(int x1, int y1, int x2, int y2, uint32 color);
	virtual bool drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1);
	BaseRenderer(BaseGame *inGame = NULL);
	virtual ~BaseRenderer();
	virtual bool setProjection() {
		return STATUS_OK;
	};

	virtual bool windowedBlt();
	virtual bool fill(byte r, byte g, byte b, Common::Rect *rect = NULL);
	virtual void onWindowChange();
	virtual bool initRenderer(int width, int height, bool windowed);
	virtual bool flip();
	virtual void initLoop();
	virtual bool switchFullscreen();
	virtual bool setup2D(bool force = false);
	virtual bool setupLines();

	virtual const char *getName() {
		return "";
	};
	virtual bool displayDebugInfo() {
		return STATUS_FAILED;
	};
	virtual bool drawShaderQuad() {
		return STATUS_FAILED;
	}

	virtual float getScaleRatioX() const {
		return 1.0f;
	}
	virtual float getScaleRatioY() const {
		return 1.0f;
	}

	virtual BaseSurface *createSurface() = 0;

	bool clipCursor();
	bool unclipCursor();

	BaseObject *getObjectAt(int x, int y);
	void deleteRectList();

	virtual bool startSpriteBatch() {
		return STATUS_OK;
	};
	virtual bool endSpriteBatch() {
		return STATUS_OK;
	};
	bool pointInViewport(Point32 *P);
	uint32 _forceAlphaColor;
	uint32 _window;
	uint32 _clipperWindow;
	bool _active;
	bool _ready;
	bool _windowed;
	Rect32 _windowRect;
	Rect32 _viewportRect;
	Rect32 _screenRect;
	Rect32 _monitorRect;
	int _bPP;
	int _height;
	int _width;

	BaseArray<BaseActiveRect *, BaseActiveRect *> _rectList;
};

BaseRenderer *makeOSystemRenderer(BaseGame *inGame); // Implemented in BRenderSDL.cpp

} // end of namespace WinterMute

#endif
