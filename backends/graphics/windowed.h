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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_WINDOWED_H
#define BACKENDS_GRAPHICS_WINDOWED_H

#include "backends/graphics/graphics.h"
#include "common/frac.h"
#include "common/rect.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "graphics/scaler/aspect.h"

enum {
	STRETCH_CENTER = 0,
	STRETCH_INTEGRAL = 1,
	STRETCH_FIT = 2,
	STRETCH_STRETCH = 3,
	STRETCH_FIT_FORCE_ASPECT = 4
};

class WindowedGraphicsManager : virtual public GraphicsManager {
public:
	WindowedGraphicsManager() :
		_windowWidth(0),
		_windowHeight(0),
		_overlayVisible(false),
		_gameScreenShakeXOffset(0),
		_gameScreenShakeYOffset(0),
		_forceRedraw(false),
		_cursorVisible(false),
		_cursorX(0),
		_cursorY(0),
		_xdpi(90),
		_ydpi(90),
		_cursorNeedsRedraw(false),
		_cursorLastInActiveArea(true) {}

	virtual void showOverlay() override {
		if (_overlayVisible)
			return;

		_activeArea.drawRect = _overlayDrawRect;
		_activeArea.width = getOverlayWidth();
		_activeArea.height = getOverlayHeight();
		_overlayVisible = true;
		_forceRedraw = true;
	}

	virtual void hideOverlay() override {
		if (!_overlayVisible)
			return;

		_activeArea.drawRect = _gameDrawRect;
		_activeArea.width = getWidth();
		_activeArea.height = getHeight();
		_overlayVisible = false;
		_forceRedraw = true;
	}

	virtual bool isOverlayVisible() const override { return _overlayVisible; }

	virtual void setShakePos(int shakeXOffset, int shakeYOffset) override {
		if (_gameScreenShakeXOffset != shakeXOffset || _gameScreenShakeYOffset != shakeYOffset) {
			_gameScreenShakeXOffset = shakeXOffset;
			_gameScreenShakeYOffset = shakeYOffset;
			recalculateDisplayAreas();
			_cursorNeedsRedraw = true;
		}
	}

	int getWindowWidth() const { return _windowWidth; }
	int getWindowHeight() const { return _windowHeight; }

protected:
	/**
	 * @returns whether or not the game screen must have aspect ratio correction
	 * applied for correct rendering.
	 */
	virtual bool gameNeedsAspectRatioCorrection() const = 0;

	/**
	 * Backend-specific implementation for updating internal surfaces that need
	 * to reflect the new window size.
	 */
	virtual void handleResizeImpl(const int width, const int height, const int xdpi, const int ydpi) = 0;

	/**
	 * Converts the given point from the active virtual screen's coordinate
	 * space to the window's coordinate space (i.e. game-to-window or
	 * overlay-to-window).
	 */
	Common::Point convertVirtualToWindow(const int x, const int y) const {
		const int targetX = _activeArea.drawRect.left;
		const int targetY = _activeArea.drawRect.top;
		const int targetWidth = _activeArea.drawRect.width();
		const int targetHeight = _activeArea.drawRect.height();
		const int sourceWidth = _activeArea.width;
		const int sourceHeight = _activeArea.height;

		if (sourceWidth == 0 || sourceHeight == 0) {
			error("convertVirtualToWindow called without a valid draw rect");
		}

		int windowX = targetX + (x * targetWidth + sourceWidth / 2) / sourceWidth;
		int windowY = targetY + (y * targetHeight + sourceHeight / 2) / sourceHeight;

		return Common::Point(CLIP<int>(windowX, targetX, targetX + targetWidth - 1),
		                     CLIP<int>(windowY, targetY, targetY + targetHeight - 1));
	}

	/**
	 * Converts the given point from the window's coordinate space to the
	 * active virtual screen's coordinate space (i.e. window-to-game or
	 * window-to-overlay).
	 */
	Common::Point convertWindowToVirtual(int x, int y) const {
		const int sourceX = _activeArea.drawRect.left;
		const int sourceY = _activeArea.drawRect.top;
		const int sourceMaxX = _activeArea.drawRect.right - 1;
		const int sourceMaxY = _activeArea.drawRect.bottom - 1;
		const int sourceWidth = _activeArea.drawRect.width();
		const int sourceHeight = _activeArea.drawRect.height();
		const int targetWidth = _activeArea.width;
		const int targetHeight = _activeArea.height;

		if (sourceWidth == 0 || sourceHeight == 0) {
			error("convertWindowToVirtual called without a valid draw rect");
		}

		x = CLIP<int>(x, sourceX, sourceMaxX);
		y = CLIP<int>(y, sourceY, sourceMaxY);

		int virtualX = ((x - sourceX) * targetWidth + sourceWidth / 2) / sourceWidth;
		int virtualY = ((y - sourceY) * targetHeight + sourceHeight / 2) / sourceHeight;

		return Common::Point(CLIP<int>(virtualX, 0, targetWidth - 1),
		                     CLIP<int>(virtualY, 0, targetHeight - 1));
	}

	/**
	 * @returns the desired aspect ratio of the game surface.
	 */
	frac_t getDesiredGameAspectRatio() const {
		if (getHeight() == 0 || gameNeedsAspectRatioCorrection()) {
			return intToFrac(4) / 3;
		}

		return intToFrac(getWidth()) / getHeight();
	}

	/**
	 * @returns the scale used between the game size and the surface on which it is rendered.
	 */
	virtual int getGameRenderScale() const {
		return 1;
	}

	/**
	 * Called after the window has been updated with new dimensions.
	 *
	 * @param width The new width of the window, excluding window decoration.
	 * @param height The new height of the window, excluding window decoration.
	 */
	void handleResize(const int width, const int height, const int xdpi, const int ydpi) {
		_windowWidth = width;
		_windowHeight = height;
		_xdpi = xdpi;
		_ydpi = ydpi;
		handleResizeImpl(width, height, xdpi, ydpi);
	}

	/**
	 * Recalculates the display areas for the game and overlay surfaces within
	 * the window.
	 */
	virtual void recalculateDisplayAreas() {
		if (_windowHeight == 0) {
			return;
		}

		populateDisplayAreaDrawRect(getDesiredGameAspectRatio(), getWidth() * getGameRenderScale(), _gameDrawRect);

		if (getOverlayHeight()) {
			const frac_t overlayAspect = intToFrac(getOverlayWidth()) / getOverlayHeight();
			populateDisplayAreaDrawRect(overlayAspect, getOverlayWidth(), _overlayDrawRect);
		}

		if (_overlayVisible) {
			_activeArea.drawRect = _overlayDrawRect;
			_activeArea.width = getOverlayWidth();
			_activeArea.height = getOverlayHeight();
		} else {
			_activeArea.drawRect = _gameDrawRect;
			_activeArea.width = getWidth();
			_activeArea.height = getHeight();
		}
	}

	/**
	 * Sets the position of the hardware mouse cursor in the host system,
	 * relative to the window.
	 *
	 * @param x X coordinate in window coordinates.
	 * @param y Y coordinate in window coordinates.
	 */
	virtual void setSystemMousePosition(const int x, const int y) = 0;

	virtual bool showMouse(bool visible) override {
		if (_cursorVisible == visible) {
			return visible;
		}

		const bool last = _cursorVisible;
		_cursorVisible = visible;
		_cursorNeedsRedraw = true;
		return last;
	}

	/**
	 * Move ("warp") the mouse cursor to the specified position.
	 *
	 * @param x	The new X position of the mouse in virtual screen coordinates.
	 * @param y	The new Y position of the mouse in virtual screen coordinates.
	 */
	void warpMouse(int x, int y) override {
		// Check active coordinate instead of window coordinate to avoid warping
		// the mouse if it is still within the same virtual pixel
		const Common::Point virtualCursor = convertWindowToVirtual(_cursorX, _cursorY);
		if (virtualCursor.x != x || virtualCursor.y != y) {
			// Warping the mouse in SDL generates a mouse movement event, so
			// `setMousePosition` would be called eventually through the
			// `notifyMousePosition` callback if we *only* set the system mouse
			// position here. However, this can cause problems with some games.
			// For example, the cannon script in CoMI calls to warp the mouse
			// twice each time the cannon is reloaded, and unless we update the
			// mouse position immediately, the second call is ignored, which
			// causes the cannon to change its aim.
			const Common::Point windowCursor = convertVirtualToWindow(x, y);
			setMousePosition(windowCursor.x, windowCursor.y);
			setSystemMousePosition(windowCursor.x, windowCursor.y);
		}
	}

	/**
	 * Sets the position of the rendered mouse cursor in the window.
	 *
	 * @param x X coordinate in window coordinates.
	 * @param y Y coordinate in window coordinates.
	 */
	void setMousePosition(int x, int y) {
		if (_cursorX != x || _cursorY != y) {
			_cursorNeedsRedraw = true;
		}

		_cursorX = x;
		_cursorY = y;
	}

	/**
	 * The width of the window, excluding window decoration.
	 */
	int _windowWidth;

	/**
	 * The height of the window, excluding window decoration.
	 */
	int _windowHeight;

	/**
	 * The DPI of the window.
	 */
	int _xdpi, _ydpi;

	/**
	 * Whether the overlay (i.e. launcher, including the out-of-game launcher)
	 * is visible or not.
	 */
	bool _overlayVisible;

	/**
	 * The offset by which the screen is moved horizontally.
	 */
	int _gameScreenShakeXOffset;

	/**
	* The offset by which the screen is moved vertically.
	*/
	int _gameScreenShakeYOffset;

	/**
	 * The scaled draw rectangle for the game surface within the window.
	 */
	Common::Rect _gameDrawRect;

	/**
	 * The scaled draw rectangle for the overlay (launcher) surface within the
	 * window.
	 */
	Common::Rect _overlayDrawRect;

	/**
	 * Data about the display area of a virtual screen.
	 */
	struct DisplayArea {
		/**
		 * The scaled area where the virtual screen is drawn within the window.
		 */
		Common::Rect drawRect;

		/**
		 * The width of the virtual screen's unscaled coordinate space.
		 */
		int width;

		/**
		 * The height of the virtual screen's unscaled coordinate space.
		 */
		int height;
	};

	/**
	 * Display area information about the currently active virtual screen. This
	 * will be the overlay screen when the overlay is active, and the game
	 * screen otherwise.
	 */
	DisplayArea _activeArea;

	/**
	 * Whether the screen must be redrawn on the next frame.
	 */
	bool _forceRedraw;

	/**
	 * Whether the cursor is actually visible.
	 */
	bool _cursorVisible;

	/**
	 * Whether the mouse cursor needs to be redrawn on the next frame.
	 */
	bool _cursorNeedsRedraw;

	/**
	 * Whether the last position of the system cursor was within the active area
	 * of the window.
	 */
	bool _cursorLastInActiveArea;

	/**
	 * The position of the mouse cursor, in window coordinates.
	 */
	int _cursorX, _cursorY;

private:
	void populateDisplayAreaDrawRect(const frac_t displayAspect, int originalWidth, Common::Rect &drawRect) const {
		int mode = getStretchMode();
		// Mode Center   = use original size, or divide by an integral amount if window is smaller than game surface
		// Mode Integral = scale by an integral amount.
		// Mode Fit      = scale to fit the window while respecting the aspect ratio
		// Mode Stretch  = scale and stretch to fit the window without respecting the aspect ratio
		// Mode Fit Force Aspect = scale to fit the window while forcing a 4:3 aspect ratio

		int width = 0, height = 0;
		if (mode == STRETCH_CENTER || mode == STRETCH_INTEGRAL) {
			width = originalWidth;
			height = intToFrac(width) / displayAspect;
			if (width > _windowWidth || height > _windowHeight) {
				int fac = 1 + MAX((width - 1) / _windowWidth, (height - 1) / _windowHeight);
				width /= fac;
				height /= fac;
			} else if (mode == STRETCH_INTEGRAL) {
				int fac = MIN(_windowWidth / width, _windowHeight / height);
				width *= fac;
				height *= fac;
			}
		} else {
			frac_t windowAspect = intToFrac(_windowWidth) / _windowHeight;
			width = _windowWidth;
			height = _windowHeight;
			if (mode == STRETCH_FIT_FORCE_ASPECT) {
				frac_t ratio = intToFrac(4) / 3;
				if (windowAspect < ratio)
					height = intToFrac(width) / ratio;
				else if (windowAspect > ratio)
					width = fracToInt(height * ratio);
			} else if (mode != STRETCH_STRETCH) {
				if (windowAspect < displayAspect)
					height = intToFrac(width) / displayAspect;
				else if (windowAspect > displayAspect)
					width = fracToInt(height * displayAspect);
			}
		}
		
		drawRect.left = ((_windowWidth - width) / 2) + _gameScreenShakeXOffset * width / getWidth();
		drawRect.top = ((_windowHeight - height) / 2) + _gameScreenShakeYOffset * height / getHeight();
		drawRect.setWidth(width);
		drawRect.setHeight(height);
	}
};

#endif
