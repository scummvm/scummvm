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
#include "common/events.h"

#include <mint/osbind.h>
#include <mint/ostruct.h>
#include <unordered_set>

#include "common/rect.h"
#include "graphics/surface.h"

template<>
struct std::hash<Common::Rect>
{
	std::size_t operator()(Common::Rect const& rect) const noexcept
	{
		return 31 * (31 * (31 * rect.left + rect.top) + rect.right) + rect.bottom;
	}
};

///////////////////////////////////////////////////////////////////////////////

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
	Graphics::PixelFormat getOverlayFormat() const override;
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight() const override { return 480; }
	int16 getOverlayWidth() const override { return _vgaMonitor ? 640 : 640*1.2; }

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
	typedef void* (*AtariMemAlloc)(size_t bytes);
	typedef void (*AtariMemFree)(void *ptr);

	void allocateSurfaces();
	void freeSurfaces();
	void convertRectToSurfaceWithKey(Graphics::Surface &dstSurface, const Graphics::Surface &srcSurface,
									 int destX, int destY, const Common::Rect &subRect, uint32 key,
									 const byte srcPalette[256*3]) const;

	enum class GraphicsMode : int {
		DirectRendering = 0,
		SingleBuffering = 1,
		TripleBuffering = 3
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
	using DirtyRects = std::unordered_set<Common::Rect>;

	enum CustomEventAction {
		kActionToggleAspectRatioCorrection = 100,
	};

	enum SteTtRezValue {
		kRezValueSTLow  = 0,	// 320x200@4bpp, ST palette
		kRezValueSTMid  = 1,	// 640x200@2bpp, ST palette
		kRezValueSTHigh = 2,	// 640x400@1bpp, ST palette
		kRezValueTTLow  = 7,	// 320x480@8bpp, TT palette
		kRezValueTTMid  = 4,	// 640x480@4bpp, TT palette
		kRezValueTTHigh = 6		// 1280x960@1bpp, TT palette
	};

	int16 getMaximumScreenHeight() const { return 480; }
	int16 getMaximumScreenWidth() const { return _tt ? 320 : (_vgaMonitor ? 640 : 640*1.2); }

	template <bool directRendering>
	bool updateScreenInternal(const Graphics::Surface &srcSurface);

	inline int getBitsPerPixel(const Graphics::PixelFormat &format) const;

	virtual AtariMemAlloc getStRamAllocFunc() const {
		return [](size_t bytes) { return (void*)Mxalloc(bytes, MX_STRAM); };
	}
	virtual AtariMemFree getStRamFreeFunc() const {
		return [](void *ptr) { Mfree(ptr); };
	}

	virtual void copyRectToSurface(Graphics::Surface &dstSurface, int dstBitsPerPixel, const Graphics::Surface &srcSurface,
								   int destX, int destY,
								   const Common::Rect &subRect) const {
		dstSurface.copyRectToSurface(srcSurface, destX, destY, subRect);
	}
	virtual void copyRectToSurfaceWithKey(Graphics::Surface &dstSurface, int dstBitsPerPixel, const Graphics::Surface &srcSurface,
										  int destX, int destY,
										  const Common::Rect &subRect, uint32 key,
										  const Graphics::Surface &bgSurface, const byte srcPalette[256*3]) const {
		convertRectToSurfaceWithKey(dstSurface, srcSurface, destX, destY, subRect, key, srcPalette);
	}

	virtual Common::Rect alignRect(int x, int y, int w, int h) const = 0;

	void cursorPositionChanged() {
		if (_overlayVisible) {
			_screen[OVERLAY_BUFFER]->cursorPositionChanged = true;
		} else {
			_screen[FRONT_BUFFER]->cursorPositionChanged
				= _screen[BACK_BUFFER1]->cursorPositionChanged
				= _screen[BACK_BUFFER2]->cursorPositionChanged
				= true;
		}
	}

	void cursorSurfaceChanged() {
		if (_overlayVisible) {
			_screen[OVERLAY_BUFFER]->cursorSurfaceChanged = true;
		} else {
			_screen[FRONT_BUFFER]->cursorSurfaceChanged
				= _screen[BACK_BUFFER1]->cursorSurfaceChanged
				= _screen[BACK_BUFFER2]->cursorSurfaceChanged
				= true;
		}
	}

	void cursorVisibilityChanged() {
		if (_overlayVisible) {
			_screen[OVERLAY_BUFFER]->cursorVisibilityChanged = true;
		} else {
			_screen[FRONT_BUFFER]->cursorVisibilityChanged
				= _screen[BACK_BUFFER1]->cursorVisibilityChanged
				= _screen[BACK_BUFFER2]->cursorVisibilityChanged
				= true;
		}
	}

	int getOverlayPaletteSize() const {
#ifndef DISABLE_FANCY_THEMES
		return _tt ? 16 : 256;
#else
		return 16;
#endif
	}

	bool _vgaMonitor = true;
	bool _tt = false;
	bool _aspectRatioCorrection = false;
	bool _oldAspectRatioCorrection = false;

	GraphicsState _currentState{ (GraphicsMode)getDefaultGraphicsMode() };

	enum PendingScreenChange {
		kPendingScreenChangeNone	= 0,
		kPendingScreenChangeMode	= 1<<0,
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

	class Palette {
	public:
		void clear() {
			memset(data, 0, sizeof(data));
		}

		uint16 *const tt = reinterpret_cast<uint16*>(data);
		_RGB *const falcon = reinterpret_cast<_RGB*>(data);

	private:
		byte data[256*4] = {};
	};

	struct Screen {
		Screen(AtariGraphicsManager *manager, int width, int height, const Graphics::PixelFormat &format, const Palette *palette);
		~Screen();

		void reset(int width, int height, int bitsPerPixel);
		// must be called before any rectangle drawing
		void addDirtyRect(const Graphics::Surface &srcSurface, const Common::Rect &rect);

		void clearDirtyRects() {
			dirtyRects.clear();
			fullRedraw = false;
		}

		Graphics::Surface surf;
		const Palette *palette;
		bool cursorPositionChanged = true;
		bool cursorSurfaceChanged = false;
		bool cursorVisibilityChanged = false;
		DirtyRects dirtyRects;
		bool fullRedraw = false;
		Common::Rect oldCursorRect;
		int rez = -1;
		int mode = -1;
		Graphics::Surface *const offsettedSurf = &_offsettedSurf;

	private:
		static constexpr size_t ALIGN = 16;	// 16 bytes

		AtariGraphicsManager *_manager;
		Graphics::Surface _offsettedSurf;
	};
	Screen *_screen[BUFFER_COUNT] = {};
	Screen *_workScreen = nullptr;
	Screen *_oldWorkScreen = nullptr;	// used in hideOverlay()

	Graphics::Surface _chunkySurface;

	bool _overlayVisible = false;
	Graphics::Surface _overlaySurface;

	bool _checkUnalignedPitch = false;

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

	Palette _palette;
	Palette _overlayPalette;
};

#endif
