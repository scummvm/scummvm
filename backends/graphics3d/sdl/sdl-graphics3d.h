/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef BACKENDS_GRAPHICS3D_SDL_SDLGRAPHICS3D_H
#define BACKENDS_GRAPHICS3D_SDL_SDLGRAPHICS3D_H

#include "backends/graphics3d/graphics3d.h"
#include "backends/platform/sdl/sdl-window.h"

#include "common/events.h"
#include "common/rect.h"

#include "math/rect2d.h"

class SdlEventSource;

/**
 * Base class for a SDL based graphics manager.
 */
class SdlGraphics3dManager : virtual public Graphics3dManager, public Common::EventObserver {
public:
	SdlGraphics3dManager(SdlEventSource *source, SdlWindow *window);
	virtual ~SdlGraphics3dManager() {}

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
	virtual void notifyVideoExpose() {};

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
	virtual void notifyResize(const int width, const int height) {};

	/**
	 * Transforms real screen coordinates into the current active screen
	 * coordinates (may be either game screen or overlay).
	 *
	 * @param point Mouse coordinates to transform.
	 */
	virtual void transformMouseCoordinates(Common::Point &point) = 0;

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
	virtual bool lockMouse(bool lock) override;

	virtual bool saveScreenshot(const Common::String &filename) const { return false; }
	void saveScreenshot() override;

	// Override from Common::EventObserver
	virtual bool notifyEvent(const Common::Event &event) override;

	/**
	 * A (subset) of the graphic manager's state. This is used when switching
	 * between different SDL graphic managers at runtime.
	 */
	struct State {
		bool aspectRatio;
		bool fullscreen;
	};

	/** Obtain the user configured fullscreen resolution, or default to the desktop resolution */
	virtual Common::Rect getPreferredFullscreenResolution();

	/**
	 * @returns the SDL window.
	 */
	SdlWindow *getWindow() const { return _window; }

	Common::Keymap *getKeymap();

protected:
	enum CustomEventAction {
		kActionToggleFullscreen = 100,
		kActionToggleMouseCapture,
		kActionSaveScreenshot,
		kActionToggleAspectRatioCorrection
	};

protected:

	SdlEventSource *_eventSource;
	SdlWindow *_window;

private:
	void toggleFullScreen();
};

#endif
