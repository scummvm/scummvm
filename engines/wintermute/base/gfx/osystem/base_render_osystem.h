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

#ifndef WINTERMUTE_BASE_RENDERER_SDL_H
#define WINTERMUTE_BASE_RENDERER_SDL_H

#include "engines/wintermute/base/gfx/base_renderer.h"

#include "common/rect.h"
#include "common/list.h"

#include "graphics/surface.h"
#include "graphics/transform_struct.h"

namespace Wintermute {
class BaseSurfaceOSystem;
class RenderTicket;
/**
 * A 2D-renderer implementation for WME.
 * This renderer makes use of a "ticket"-system, where all draw-calls
 * are stored as tickets until flip() is called, and compared against the tickets
 * from last frame, to determine which calls were the same as last round
 * (i.e. in the exact same order, with the exact same arguments), and thus
 * figure out which parts of the screen need to be redrawn.
 *
 * Important concepts to handle here, is the ordered number of any ticket
 * which is called the "drawNum", every frame this starts from scratch, and
 * then the incoming tickets created from the draw-calls are checked to see whether
 * they came before, on, or after the drawNum they had last frame. Everything else
 * being equal, this information is then used to check whether the draw order changed,
 * which will then create a need for redrawing, as we draw with an alpha-channel here.
 *
 * There is also a draw path that draws without tickets, for debugging purposes,
 * as well as to accommodate situations with large enough amounts of draw calls,
 * that there will be too much overhead involved with comparing the generated tickets.
 */
class BaseRenderOSystem : public BaseRenderer {
public:
	BaseRenderOSystem(BaseGame *inGame);
	~BaseRenderOSystem() override;

	typedef Common::List<RenderTicket *>::iterator RenderQueueIterator;

	Common::String getName() const override;

	bool initRenderer(int width, int height, bool windowed) override;
	bool flip() override;
	bool indicatorFlip() override;
	bool forcedFlip() override;
	bool fill(byte r, byte g, byte b, Common::Rect *rect = nullptr) override;
	Graphics::PixelFormat getPixelFormat() const override;
	void fade(uint16 alpha) override;
	void fadeToColor(byte r, byte g, byte b, byte a) override;

	bool drawLine(int x1, int y1, int x2, int y2, uint32 color) override;

	BaseImage *takeScreenshot() override;
	void onWindowChange() override;
	void setWindowed(bool windowed) override;

	void invalidateTicket(RenderTicket *renderTicket);
	void invalidateTicketsFromSurface(BaseSurfaceOSystem *surf);
	/**
	 * Insert a new ticket into the queue, adding a dirty rect
	 * @param renderTicket the ticket to be added.
	 */
	void drawFromTicket(RenderTicket *renderTicket);
	/**
	 * Re-insert an existing ticket into the queue, adding a dirty rect
	 * out-of-order from last draw from the ticket.
	 * @param ticket iterator pointing to the ticket to be added.
	 */
	void drawFromQueuedTicket(const RenderQueueIterator &ticket);

	bool setViewport(int left, int top, int right, int bottom) override;
	bool setViewport(Rect32 *rect) override { return BaseRenderer::setViewport(rect); }
	void modTargetRect(Common::Rect *rect);
	void pointFromScreen(Point32 *point);
	void pointToScreen(Point32 *point);

	void dumpData(const char *filename) override;

	float getScaleRatioX() const override {
		return _ratioX;
	}
	float getScaleRatioY() const override {
		return _ratioY;
	}
	bool startSpriteBatch() override;
	bool endSpriteBatch() override;
	void endSaveLoad() override;
	void drawSurface(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, Graphics::TransformStruct &transform);
	BaseSurface *createSurface() override;
private:
	/**
	 * Mark a specified rect of the screen as dirty.
	 * @param rect the region to be marked as dirty
	 */
	void addDirtyRect(const Common::Rect &rect);
	/**
	 * Traverse the tickets that are dirty, and draw them
	 */
	void drawTickets();
	// Non-dirty-rects:
	void drawFromSurface(RenderTicket *ticket);
	// Dirty-rects:
	void drawFromSurface(RenderTicket *ticket, Common::Rect *dstRect, Common::Rect *clipRect);
	Common::Rect *_dirtyRect;
	Common::List<RenderTicket *> _renderQueue;

	bool _needsFlip;
	RenderQueueIterator _lastFrameIter;
	Common::Rect _renderRect;
	Graphics::Surface *_renderSurface;
	Graphics::Surface *_blankSurface;

	int _borderLeft;
	int _borderTop;
	int _borderRight;
	int _borderBottom;

	bool _disableDirtyRects;
	float _ratioX;
	float _ratioY;
	uint32 _clearColor;

	bool _skipThisFrame;
	int _lastScreenChangeID; // previous value of OSystem::getScreenChangeID()
};

} // End of namespace Wintermute

#endif
