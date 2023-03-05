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

#include "common/array.h"
#include "common/events.h"
#include "common/rect.h"
#include "graphics/surface.h"

// maximum screen dimensions
constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;

// minimum overlay dimensions
constexpr int OVERLAY_WIDTH = 320;
constexpr int OVERLAY_HEIGHT = 240;

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
	Graphics::PixelFormat getOverlayFormat() const override { return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0); }
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;

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
	const Graphics::PixelFormat PIXELFORMAT8 = Graphics::PixelFormat::createFormatCLUT8();

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

	bool _vgaMonitor = true;

	enum class GraphicsMode : int {
		DirectRendering,
		SingleBuffering,
		DoubleBuffering,
		TripleBuffering
	};

	struct GraphicsState {
		bool operator==(const GraphicsState &other) const {
			return mode == other.mode
				&& width == other.width
				&& height == other.height
				&& format == other.format;
		}
		bool operator!=(const GraphicsState &other) const {
			return !(*this == other);
		}

		GraphicsMode mode;
		int width;
		int height;
		Graphics::PixelFormat format;
	};
	GraphicsState _pendingState = {};

	static const int SCREENS = 3;
	static const int FRONT_BUFFER = 0;
	static const int BACK_BUFFER1 = 1;
	static const int BACK_BUFFER2 = 2;

	Graphics::Surface _chunkySurface;	// for Videl's copyRectToSurfaceWithKey

private:
	enum CustomEventAction {
		kActionToggleAspectRatioCorrection = 100,
	};

	void setVidelResolution() const;
	void waitForVbl() const;

	bool updateOverlay();
	bool updateDirectBuffer();
	bool updateSingleBuffer();
	bool updateDoubleAndTripleBuffer();

	byte *allocateAtariSurface(Graphics::Surface &surface,
							  int width, int height, const Graphics::PixelFormat &format,
							  const AtariMemAlloc &allocFunc);

	void freeAtariSurface(byte *ptr, Graphics::Surface &surface, const AtariMemFree &freeFunc);

	virtual void copyRectToSurface(Graphics::Surface &dstSurface,
								   const Graphics::Surface &srcSurface, int destX, int destY,
								   const Common::Rect &subRect) const = 0;
	virtual void copyRectToSurfaceWithKey(Graphics::Surface &dstSurface,
										  const Graphics::Surface &srcSurface, int destX, int destY,
										  const Common::Rect &subRect, uint32 key) const = 0;
	virtual void alignRect(const Graphics::Surface &srcSurface, Common::Rect &rect) const {}

	enum class ScaleMode {
		NONE,
		UPSCALE,
		DOWNSCALE
	};
	void copySurface8ToSurface16(const Graphics::Surface &srcSurface, const byte *srcPalette,
								 Graphics::Surface &dstSurface, int destX, int destY,
								 const Common::Rect subRect, ScaleMode scaleMode) const;
	void copySurface8ToSurface16WithKey(const Graphics::Surface &srcSurface, const byte* srcPalette,
										Graphics::Surface &dstSurface, int destX, int destY,
										const Common::Rect subRect, uint32 key) const;

	void handleModifiedRect(const Graphics::Surface &surface, Common::Rect rect, Common::Array<Common::Rect> &rects) const;

	void updateCursorRect();

	bool _aspectRatioCorrection = false;
	bool _oldAspectRatioCorrection = false;
	bool _vsync = true;

	GraphicsState _currentState = {};

	enum PendingScreenChange {
		kPendingScreenChangeNone	= 0,
		kPendingScreenChangeOverlay	= 1<<0,
		kPendingScreenChangeScreen	= 1<<1,
		kPendingScreenChangePalette	= 1<<2
	};
	int _pendingScreenChange = kPendingScreenChangeNone;

	byte *_screen[SCREENS] = {};	// for Mfree() purposes only
	byte *_screenAligned[SCREENS] = {};
	Graphics::Surface _screenSurface;
	Common::Rect _modifiedScreenRect;	// direct rendering only
	bool _screenModified = false;	// double/triple buffering only

	Common::Array<Common::Rect> _modifiedChunkyRects;

	byte *_overlayScreen = nullptr;	// for Mfree() purposes only
	Graphics::Surface _screenOverlaySurface;
	bool _overlayVisible = false;

	Graphics::Surface _overlaySurface;
	Common::Array<Common::Rect> _modifiedOverlayRects;

	struct Cursor {
		void update(const Graphics::Surface &screen);

		bool isModified() const {
			return surfaceChanged || positionChanged;
		}

		bool visible = false;

		// position
		Common::Point getPosition() const {
			return Common::Point(x, y);
		}
		void setPosition(int x_, int y_, bool override = false) {
			if (x == x_ && y == y_)
				return;

			if (!visible && !override)
				return;

			x = x_;
			y = y_;
			positionChanged = true;
		}
		void updatePosition(int deltaX, int deltaY, const Graphics::Surface &screen);
		bool positionChanged = false;
		void swap() {
			const int tmpX = oldX;
			const int tmpY = oldY;

			oldX = x;
			oldY = y;

			x = tmpX;
			y = tmpY;

			positionChanged = false;
		}

		// surface
		void setSurface(const void *buf, int w, int h, int _hotspotX, int _hotspotY, uint32 _keycolor, const Graphics::PixelFormat &format);
		bool surfaceChanged = false;
		Graphics::Surface surface;
		uint32 keycolor;

		// rects (valid only if !outOfScreen)
		bool outOfScreen = true;
		Common::Rect srcRect;
		Common::Rect dstRect;

		// palette (only used for 16bpp screen surfaces)
		byte palette[256*3] = {};

	private:
		int x = -1, y = -1;
		int oldX = -1, oldY = -1;

		int hotspotX;
		int hotspotY;
	} _cursor;

	Common::Rect _oldCursorRect;

	byte _palette[256*3] = {};
};

#endif
