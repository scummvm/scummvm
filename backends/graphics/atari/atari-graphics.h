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

#ifndef BACKENDS_GRAPHICS_ATARI_H
#define BACKENDS_GRAPHICS_ATARI_H

#include "backends/graphics/graphics.h"

#include <mint/osbind.h>
#include <utility>	// std::pair
#include <vector>

#include "common/events.h"
#include "common/rect.h"
#include "graphics/surface.h"

class AtariGraphicsManager : public GraphicsManager, Common::EventObserver {
public:
	AtariGraphicsManager();
	virtual ~AtariGraphicsManager();

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	int getDefaultGraphicsMode() const override { return (int)GraphicsMode::TripleBuffering; }
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	int getGraphicsMode() const override { return (int)_currentState.mode; }

	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) override;

	int getScreenChangeID() const override { return 0; }

	void beginGFXTransaction() override;
	OSystem::TransactionError endGFXTransaction() override;

	int16 getHeight() const override { return _currentState.height; }
	int16 getWidth() const override { return _currentState.width; }
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void fillScreen(uint32 col) override;
	void updateScreen() override;
	void setShakePos(int shakeXOffset, int shakeYOffset) override;
	void setFocusRectangle(const Common::Rect& rect) override {}
	void clearFocusRectangle() override {}

	void showOverlay(bool inGUI) override;
	void hideOverlay() override;
	bool isOverlayVisible() const override { return _overlayVisible; }
	Graphics::PixelFormat getOverlayFormat() const override { return PIXELFORMAT_RGB332; }
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight() const override { return 480; }
	int16 getOverlayWidth() const override { return 640; }

	bool showMouse(bool visible) override;
	void warpMouse(int x, int y) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor,
						bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = NULL) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;

	Common::Point getMousePosition() const { return _cursor.getPosition(); }
	void updateMousePosition(int deltaX, int deltaY);

	bool notifyEvent(const Common::Event &event) override;
	Common::Keymap *getKeymap() const;

protected:
	const Graphics::PixelFormat PIXELFORMAT_CLUT8 = Graphics::PixelFormat::createFormatCLUT8();
	const Graphics::PixelFormat PIXELFORMAT_RGB332 = Graphics::PixelFormat(1, 3, 3, 2, 0, 5, 2, 0, 0);

	typedef void* (*AtariMemAlloc)(size_t bytes);
	typedef void (*AtariMemFree)(void *ptr);

	virtual AtariMemAlloc getStRamAllocFunc() const {
		return [](size_t bytes) { return (void*)Mxalloc(bytes, MX_STRAM); };
	}

	virtual AtariMemFree getStRamFreeFunc() const {
		return [](void *ptr) { Mfree(ptr); };
	}

	void allocateSurfaces();
	void freeSurfaces();

	enum class GraphicsMode : int {
		DirectRendering,
		SingleBuffering,
		DoubleBuffering,
		TripleBuffering
	};

	struct GraphicsState {
		GraphicsState(GraphicsMode mode_)
			: mode(mode_)
			, width(0)
			, height(0) {
		}

		GraphicsMode mode;
		int width;
		int height;
		Graphics::PixelFormat format;
	};
	GraphicsState _pendingState{ (GraphicsMode)getDefaultGraphicsMode() };

private:
	enum {
		// maximum screen dimensions
		SCREEN_WIDTH = 640,
		SCREEN_HEIGHT = 480
	};

	// use std::vector as its clear() doesn't reset capacity
	using DirtyRects = std::vector<Common::Rect>;

	enum CustomEventAction {
		kActionToggleAspectRatioCorrection = 100,
	};

	void setVidelResolution() const;
	void waitForVbl() const;

	bool updateDirect();
	bool updateBuffered(const Graphics::Surface &srcSurface, Graphics::Surface &dstSurface, const DirtyRects &dirtyRects);

	void allocateAtariSurface(Graphics::Surface &surface,
							  int width, int height, const Graphics::PixelFormat &format,
							  const AtariMemAlloc &allocFunc);

	void freeAtariSurface(byte *ptr, const AtariMemFree &freeFunc);

	virtual void copyRectToSurface(Graphics::Surface &dstSurface,
								   const Graphics::Surface &srcSurface, int destX, int destY,
								   const Common::Rect &subRect) const {
		dstSurface.copyRectToSurface(srcSurface, destX, destY, subRect);
	}
	virtual void copyRectToSurfaceWithKey(Graphics::Surface &dstSurface, const Graphics::Surface &bgSurface,
										  const Graphics::Surface &srcSurface, int destX, int destY,
										  const Common::Rect &subRect, uint32 key, const byte srcPalette[256*3]) const {
		dstSurface.copyRectToSurfaceWithKey(srcSurface, destX, destY, subRect, key);
	}
	virtual void alignRect(const Graphics::Surface &srcSurface, Common::Rect &rect) const {}

	void addDirtyRect(const Graphics::Surface &surface, DirtyRects &rects, Common::Rect rect) const;

	void cursorPositionChanged() {
		if (_overlayVisible) {
			_buffer[OVERLAY_BUFFER]->cursorPositionChanged = true;
		} else {
			_buffer[FRONT_BUFFER]->cursorPositionChanged
				= _buffer[BACK_BUFFER1]->cursorPositionChanged
				= _buffer[BACK_BUFFER2]->cursorPositionChanged
				= true;
		}
	}

	void cursorSurfaceChanged() {
		if (_overlayVisible) {
			_buffer[OVERLAY_BUFFER]->cursorSurfaceChanged = true;
		} else {
			_buffer[FRONT_BUFFER]->cursorSurfaceChanged
				= _buffer[BACK_BUFFER1]->cursorSurfaceChanged
				= _buffer[BACK_BUFFER2]->cursorSurfaceChanged
				= true;
		}
	}

	bool _vgaMonitor = true;
	bool _aspectRatioCorrection = false;
	bool _oldAspectRatioCorrection = false;
	std::pair<bool, bool> _guiVsync;	// poor man's std::optional (first - value, second - has_value)

	GraphicsState _currentState{ (GraphicsMode)getDefaultGraphicsMode() };

	enum PendingScreenChange {
		kPendingScreenChangeNone	= 0,
		kPendingScreenChangeOverlay	= 1<<0,
		kPendingScreenChangeScreen	= 1<<1,
		kPendingScreenChangePalette	= 1<<2
	};
	int _pendingScreenChange = kPendingScreenChangeNone;

	enum {
		FRONT_BUFFER,
		BACK_BUFFER1,
		BACK_BUFFER2,
		OVERLAY_BUFFER,
		BUFFER_COUNT
	};

	struct ScreenInfo {
		ScreenInfo(byte *p_)
			: p(p_) {
		}

		void reset() {
			cursorPositionChanged = true;
			cursorSurfaceChanged = false;
			dirtyRects.clear();
			oldCursorRect = Common::Rect();
		}

		byte *p;
		bool cursorPositionChanged = true;
		bool cursorSurfaceChanged = false;
		DirtyRects dirtyRects = DirtyRects(100);	// reserve 100 rects
		Common::Rect oldCursorRect;
	};
	ScreenInfo *_buffer[BUFFER_COUNT] = {};
	ScreenInfo *_workScreen = nullptr;
	ScreenInfo *_oldWorkScreen = nullptr;	// used in hideOverlay()

	Graphics::Surface _screenSurface;
	Common::Rect _dirtyScreenRect;	// direct rendering only
	Graphics::Surface _chunkySurface;

	Graphics::Surface _screenOverlaySurface;
	bool _overlayVisible = false;
	Graphics::Surface _overlaySurface;

	struct Cursor {
		void update(const Graphics::Surface &screen, bool isModified);

		bool visible = false;

		// position
		Common::Point getPosition() const {
			return Common::Point(x, y);
		}
		void setPosition(int x_, int y_) {
			x = x_;
			y = y_;
		}
		void updatePosition(int deltaX, int deltaY, const Graphics::Surface &screen);
		void swap() {
			const int tmpX = oldX;
			const int tmpY = oldY;

			oldX = x;
			oldY = y;

			x = tmpX;
			y = tmpY;
		}

		// surface
		void setSurface(const void *buf, int w, int h, int _hotspotX, int _hotspotY, uint32 _keycolor, const Graphics::PixelFormat &format);
		Graphics::Surface surface;
		uint32 keycolor;

		// rects (valid only if !outOfScreen)
		bool outOfScreen = true;
		Common::Rect srcRect;
		Common::Rect dstRect;

		// palette (only used for the overlay)
		byte palette[256*3] = {};

	private:
		int x = -1, y = -1;
		int oldX = -1, oldY = -1;

		int hotspotX;
		int hotspotY;
	} _cursor;

	byte _palette[256*3] = {};
	byte _overlayPalette[256*3] = {};
};

#endif
