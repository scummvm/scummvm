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

#ifndef BACKENDS_GRAPHICS_SDL_SDLGRAPHICS_H
#define BACKENDS_GRAPHICS_SDL_SDLGRAPHICS_H

#include "backends/graphics/windowed.h"
#include "backends/platform/sdl/sdl-window.h"

#include "common/events.h"
#include "common/rect.h"

class SdlEventSource;

#ifndef __SYMBIAN32__
#define USE_OSD	1
#endif

/**
 * Base class for a SDL based graphics manager.
 */
class SdlGraphicsManager : virtual public WindowedGraphicsManager, public Common::EventObserver {
public:
	SdlGraphicsManager(SdlEventSource *source, SdlWindow *window);
	virtual ~SdlGraphicsManager() {}

	/**
	 * Makes this graphics manager active. That means it should be ready to
	 * process inputs now. However, even without being active it should be
	 * able to query the supported modes and other bits.
	 */
	virtual void activateManager();

	/**
	 * Makes this graphics manager inactive. This should allow another
	 * graphics manager to become active again.
	 */
	virtual void deactivateManager();

	/**
	 * Notify the graphics manager that the graphics needs to be redrawn, since
	 * the application window was modified.
	 *
	 * This is basically called when SDL_VIDEOEXPOSE was received.
	 */
	virtual void notifyVideoExpose() = 0;

	/**
	 * Notify the graphics manager about a resize event.
	 *
	 * It is noteworthy that the requested width/height should actually be set
	 * up as is and not changed by the graphics manager, since otherwise it may
	 * lead to odd behavior for certain window managers.
	 *
	 * It is only required to overwrite this method in case you want a
	 * resizable window. The default implementation just does nothing.
	 *
	 * @param width Requested window width.
	 * @param height Requested window height.
	 */
	virtual void notifyResize(const int width, const int height) {}

	/**
	 * Notifies the graphics manager about a mouse position change.
	 *
	 * The passed point *must* be converted from window coordinates to virtual
	 * coordinates in order for the event to be processed correctly by the game
	 * engine. Just use `convertWindowToVirtual` for this unless you need to do
	 * something special.
	 *
	 * @param mouse The mouse position in window coordinates, which must be
	 * converted synchronously to virtual coordinates.
	 * @returns true if the mouse was in a valid position for the game and
	 * should cause the event to be sent to the game.
	 */
	virtual bool notifyMousePosition(Common::Point &mouse);

	virtual bool showMouse(bool visible) override;

	virtual bool saveScreenshot(const Common::String &filename) const { return false; }
	void saveScreenshot();

	// Override from Common::EventObserver
	virtual bool notifyEvent(const Common::Event &event) override;

	/**
	 * A (subset) of the graphic manager's state. This is used when switching
	 * between different SDL graphic managers at runtime.
	 */
	struct State {
		int screenWidth, screenHeight;
		bool aspectRatio;
		bool fullscreen;
		bool cursorPalette;

#ifdef USE_RGB_COLOR
		Graphics::PixelFormat pixelFormat;
#endif
	};

	/**
	 * Gets the current state of the graphics manager.
	 */
	State getState() const;

	/**
	 * Sets up a basic state of the graphics manager.
	 */
	bool setState(const State &state);

	/**
	 * @returns the SDL window.
	 */
	SdlWindow *getWindow() const { return _window; }

	virtual void initSizeHint(const Graphics::ModeList &modes) override;

	Common::Keymap *getKeymap();

protected:
	enum CustomEventAction {
		kActionToggleFullscreen = 100,
		kActionToggleMouseCapture,
		kActionSaveScreenshot,
		kActionToggleAspectRatioCorrection,
		kActionToggleFilteredScaling,
		kActionCycleStretchMode,
		kActionIncreaseScaleFactor,
		kActionDecreaseScaleFactor,
		kActionSetScaleFilter1,
		kActionSetScaleFilter2,
		kActionSetScaleFilter3,
		kActionSetScaleFilter4,
		kActionSetScaleFilter5,
		kActionSetScaleFilter6,
		kActionSetScaleFilter7,
		kActionSetScaleFilter8
	};

	virtual int getGraphicsModeScale(int mode) const = 0;

	bool defaultGraphicsModeConfig() const;
	int getGraphicsModeIdByName(const Common::String &name) const;

	/**
	 * Gets the dimensions of the window directly from SDL instead of from the
	 * values stored by the graphics manager.
	 */
	void getWindowSizeFromSdl(int *width, int *height) const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		assert(_window);
		SDL_GetWindowSize(_window->getSDLWindow(), width, height);
#else
		assert(_hwScreen);

		if (width) {
			*width = _hwScreen->w;
		}

		if (height) {
			*height = _hwScreen->h;
		}
#endif
	}

	virtual void setSystemMousePosition(const int x, const int y) override;

	virtual void handleResizeImpl(const int width, const int height, const int xdpi, const int ydpi) override;

#if SDL_VERSION_ATLEAST(2, 0, 0)
public:
	void unlockWindowSize() {
		_allowWindowSizeReset = true;
		_hintedWidth = 0;
		_hintedHeight = 0;
	}

protected:
	Uint32 _lastFlags;
	bool _allowWindowSizeReset;
	int _hintedWidth, _hintedHeight;

	bool createOrUpdateWindow(const int width, const int height, const Uint32 flags);
#endif

	SDL_Surface *_hwScreen;
	SdlEventSource *_eventSource;
	SdlWindow *_window;

private:
	void toggleFullScreen();
};

#endif
