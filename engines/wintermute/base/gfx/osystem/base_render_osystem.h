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

#ifndef WINTERMUTE_BASE_RENDERER_SDL_H
#define WINTERMUTE_BASE_RENDERER_SDL_H

#include "engines/wintermute/base/gfx/base_renderer.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "common/list.h"

namespace Wintermute {
class BaseSurfaceOSystem;
class RenderTicket;
class BaseRenderOSystem : public BaseRenderer {
public:
	BaseRenderOSystem(BaseGame *inGame);
	~BaseRenderOSystem();

	Common::String getName() const;

	bool initRenderer(int width, int height, bool windowed);
	bool flip();
	virtual bool indicatorFlip();
	bool fill(byte r, byte g, byte b, Common::Rect *rect = NULL);
	Graphics::PixelFormat getPixelFormat() const;
	void fade(uint16 alpha);
	void fadeToColor(byte r, byte g, byte b, byte a, Common::Rect *rect = NULL);

	bool drawLine(int x1, int y1, int x2, int y2, uint32 color);

	BaseImage *takeScreenshot();

	void setAlphaMod(byte alpha);
	void setColorMod(byte r, byte g, byte b);
	void invalidateTicket(RenderTicket *renderTicket);
	void invalidateTicketsFromSurface(BaseSurfaceOSystem *surf);
	void drawFromTicket(RenderTicket *renderTicket);

	bool setViewport(int left, int top, int right, int bottom);
	bool setViewport(Rect32 *rect) { return BaseRenderer::setViewport(rect); }
	Rect32 getViewPort();
	void modTargetRect(Common::Rect *rect);
	void pointFromScreen(Point32 *point);
	void pointToScreen(Point32 *point);

	void dumpData(const char *filename);

	float getScaleRatioX() const {
		return _ratioX;
	}
	float getScaleRatioY() const {
		return _ratioY;
	}
	virtual bool startSpriteBatch();
	virtual bool endSpriteBatch();
	void endSaveLoad();
	void drawSurface(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY, bool disableAlpha = false);
	BaseSurface *createSurface();
private:
	void addDirtyRect(const Common::Rect &rect);
	void drawTickets();
	// Non-dirty-rects:
	void drawFromSurface(RenderTicket *ticket);
	// Dirty-rects:
	void drawFromSurface(RenderTicket *ticket, Common::Rect *dstRect, Common::Rect *clipRect);
	typedef Common::List<RenderTicket *>::iterator RenderQueueIterator;
	Common::Rect *_dirtyRect;
	Common::List<RenderTicket *> _renderQueue;
	RenderQueueIterator _lastAddedTicket;

	bool _needsFlip;
	uint32 _drawNum;
	Common::Rect _renderRect;
	Graphics::Surface *_renderSurface;
	Graphics::Surface *_blankSurface;

	int _borderLeft;
	int _borderTop;
	int _borderRight;
	int _borderBottom;

	bool _disableDirtyRects;
	bool _spriteBatch;
	uint32 _batchNum;
	float _ratioX;
	float _ratioY;
	uint32 _colorMod;
	uint32 _clearColor;

	bool _skipThisFrame;
};

} // end of namespace Wintermute

#endif
