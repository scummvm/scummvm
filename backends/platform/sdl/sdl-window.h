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

#ifndef BACKENDS_PLATFORM_SDL_WINDOW_H
#define BACKENDS_PLATFORM_SDL_WINDOW_H

#include "backends/platform/sdl/sdl-sys.h"

#include "common/rect.h"
#include "common/str.h"

class SdlWindow {
public:
	SdlWindow();
	virtual ~SdlWindow();

	/**
	 * Setup the window icon.
	 */
	virtual void setupIcon();

	/**
	 * Change the caption of the window.
	 *
	 * @param caption New window caption in UTF-8 encoding.
	 */
	void setWindowCaption(const Common::String &caption);

	/**
	 * Grab or ungrab the mouse cursor. This decides whether the cursor can leave
	 * the window or not.
	 */
	void grabMouse(bool grab);

	/**
	 * Lock or unlock the mouse cursor within the window.
	 */
	bool lockMouse(bool lock);

	/**
	 * Check whether the application has mouse focus.
	 */
	bool hasMouseFocus() const;

	/**
	 * Warp the mouse to the specified position in window coordinates. The mouse
	 * will only be warped if the window is focused in the window manager.
	 *
	 * @returns true if the system cursor was warped.
	 */
	bool warpMouseInWindow(int x, int y);

	/**
	 * Iconifies the window.
	 */
	void iconifyWindow();

	/**
	 * Query platform specific SDL window manager information.
	 *
	 * Since this is an SDL internal structure clients are responsible
	 * for accessing it in a version safe manner.
	 */
	bool getSDLWMInformation(SDL_SysWMinfo *info) const;

	/*
	 * Retrieve the current desktop resolution.
	 */
	Common::Rect getDesktopResolution();

	bool mouseIsGrabbed() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (_window) {
			return SDL_GetWindowGrab(_window) == SDL_TRUE;
		}
#endif
		return _inputGrabState;
	}

	bool mouseIsLocked() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		return SDL_GetRelativeMouseMode() == SDL_TRUE;
#else
		return _inputLockState;
#endif
	}

private:
	Common::Rect _desktopRes;
	bool _inputGrabState, _inputLockState;

#if SDL_VERSION_ATLEAST(2, 0, 0)
public:
	/**
	 * @return The window ScummVM has setup with SDL.
	 */
	SDL_Window *getSDLWindow() const { return _window; }

	/**
	 * Creates or updates the SDL window.
	 *
	 * @param width   Width of the window.
	 * @param height  Height of the window.
	 * @param flags   SDL flags passed to SDL_CreateWindow
	 * @return true on success, false otherwise
	 */
	bool createOrUpdateWindow(int width, int height, uint32 flags);

	/**
	 * Destroys the current SDL window.
	 */
	void destroyWindow();

protected:
	SDL_Window *_window;

private:
	uint32 _lastFlags;

	/**
	 * Switching between software and OpenGL modes requires the window to be
	 * destroyed and recreated. These properties store the position of the last
	 * window so the new window will be created in the same place.
	 */
	int _lastX, _lastY;

	Common::String _windowCaption;
#endif
};

class SdlIconlessWindow : public SdlWindow {
public:
	virtual void setupIcon() {}
};

#endif
