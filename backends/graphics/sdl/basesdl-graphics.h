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
 * $URL$
 * $Id$
 *
 */

#ifndef BACKENDS_GRAPHICS_BASESDL_H
#define BACKENDS_GRAPHICS_BASESDL_H

#include "backends/graphics/graphics.h"

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

/**
 * Base SDL graphics manager, contains common functions
 * used by other SDL managers
 */
class BaseSdlGraphicsManager : public GraphicsManager {
public:
	/**
	 * Marks the screen for a full redraw
	 */
	virtual void forceFullRedraw() = 0;

	/**
	 * Handles the scalar hotkeys
	 */
	virtual bool handleScalerHotkeys(const SDL_KeyboardEvent &key) = 0;

	/**
	 * Returns if the event passed is a hotkey for the graphics scalers
	 */
	virtual bool isScalerHotkey(const Common::Event &event) = 0;

	/**
	 * Adjusts mouse event coords for the current scaler
	 */
	virtual void adjustMouseEvent(Common::Event &event) = 0;

	/**
	 * Updates the mouse cursor position
	 */
	virtual void setMousePos(int x, int y) = 0;

	/**
	 * Toggles fullscreen
	 */
	virtual void toggleFullScreen() = 0;

	/**
	 * Saves a screenshot to a file
	 */
	virtual bool saveScreenshot(const char *filename) = 0;
};

#endif
