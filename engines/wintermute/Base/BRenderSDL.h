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

#ifndef WINTERMUTE_BRENDERER_SDL_H
#define WINTERMUTE_BRENDERER_SDL_H

#include "engines/wintermute/Base/BRenderer.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "common/list.h"

namespace WinterMute {
class CBSurfaceSDL;
class RenderTicket {
public:
	RenderTicket(CBSurfaceSDL *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRest, bool mirrorX = false, bool mirrorY = false);
	RenderTicket() : _isValid(true), _wantsDraw(false), _drawNum(0) {}
	~RenderTicket();
	Graphics::Surface *_surface;
	Common::Rect _srcRect;
	Common::Rect _dstRect;
	bool _mirrorX;
	bool _mirrorY;
	bool _hasAlpha;

	bool _isValid;
	bool _wantsDraw;
	uint32 _drawNum;
	uint32 _colorMod;

	CBSurfaceSDL *_owner;
	bool operator==(RenderTicket &a);
};

class CBRenderSDL : public CBRenderer {
public:
	CBRenderSDL(CBGame *inGame);
	~CBRenderSDL();

	const char *getName();

	ERRORCODE initRenderer(int width, int height, bool windowed);
	ERRORCODE flip();
	ERRORCODE fill(byte  r, byte g, byte b, Common::Rect *rect = NULL);

	ERRORCODE fade(uint16 alpha);
	ERRORCODE fadeToColor(uint32 color, Common::Rect *rect = NULL);

	ERRORCODE switchFullscreen();

	ERRORCODE drawLine(int x1, int y1, int x2, int y2, uint32 color);

	CBImage *takeScreenshot();

	void setAlphaMod(byte alpha);
	void setColorMod(byte r, byte g, byte b);
	void invalidateTicket(RenderTicket *renderTicket);
	void invalidateTicketsFromSurface(CBSurfaceSDL *surf);
	void drawFromTicket(RenderTicket *renderTicket);

	ERRORCODE setViewport(int left, int top, int right, int bottom);

	void modTargetRect(Common::Rect *rect);
	void pointFromScreen(Common::Point *point);
	void pointToScreen(Common::Point *point);

	void dumpData(const char *filename);

	float getScaleRatioX() const {
		return _ratioX;
	}
	float getScaleRatioY() const {
		return _ratioY;
	}

	void drawSurface(CBSurfaceSDL *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY);
private:
	void addDirtyRect(const Common::Rect &rect);
	void drawTickets();
	void drawFromSurface(const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, Common::Rect *clipRect, bool mirrorX = false, bool mirrorY = false);
	typedef Common::List<RenderTicket*>::iterator RenderQueueIterator;
	Common::Rect *_dirtyRect;
	Common::List<RenderTicket*> _renderQueue;
	bool _needsFlip;
	uint32 _drawNum;
	Common::Rect _renderRect;
	Graphics::Surface *_renderSurface;
	AnsiString _name;

	int _borderLeft;
	int _borderTop;
	int _borderRight;
	int _borderBottom;

	bool _disableDirtyRects;
	float _ratioX;
	float _ratioY;
	uint32 _colorMod;
	uint32 _clearColor;
};

} // end of namespace WinterMute

#endif // WINTERMUTE_BRENDERER_SDL_H
