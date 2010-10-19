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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
 * BS_Window
 * ---------
 * Simple window class interface. This is being encapsulated in a class for
 * reasons of portability.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_WINDOW_H
#define SWORD25_WINDOW_H

#include "sword25/kernel/common.h"

namespace Sword25 {

/**
 * A simple window class interface
 *
 * Windows are exclusively created by BS_Window::CreateBSWindow().
 * BS_Windows selects the correct class for the environment.
 */
class Window {
protected:
	bool _initSuccess;
	bool _closeWanted;

public:
	virtual ~Window() {}

	/**
	 * Returns the visibility of the window.
	 */
	virtual bool isVisible() = 0;

	/**
	 * Sets the visibility of the window
	 * @param Visible       Specifies whether the window should be visible or hidden
	 */
	virtual void setVisible(bool visible) = 0;
	/**
	 * Returns the X position of the window
	 */
	virtual int getX() = 0;
	/**
	 * Sets the X position of the window
	 * @paramX      The new X position for the window, or -1 for centre aligned
	 */
	virtual void setX(int X) = 0;
	/**
	 * Gets the Y position of the window
	 */
	virtual int getY() = 0;
	/**
	 * Sets the Y position of the window
	 * @param Y     The new Y position for the window, or -1 for centre aligned
	 */
	virtual void setY(int X) = 0;
	/**
	 * Returns the X position of the window's client area
	 */
	virtual int getClientX() = 0;
	/**
	 * Returns the Y position of the window's client area
	 */
	virtual int getClientY() = 0;
	/**
	 * Returns the width of the window without the frame
	 */
	virtual int getWidth() = 0;
	/**
	 * Sets the width of the window without the frame
	 */
	virtual void setWidth(int width) = 0;
	/**
	 * Gets the height of the window without the frame
	 */
	virtual int getHeight() = 0;
	/**
	 * Sets the height of the window without the frame
	 */
	virtual void setHeight(int height) = 0;
	/**
	 * Returns the title of the window
	 */
	virtual Common::String getTitle() = 0;
	/**
	 * Sets the title of the window
	 * @param Title     The new window title
	*/
	virtual void setTitle(const Common::String &title) = 0;
	/**
	 * Handle the processing of any pending window messages. This method should be called
	 * during the main loop.
	 */
	virtual bool processMessages() = 0;
	/**
	 * Pauses the applicaiton until the window has focus, or has been closed.
	 * Returns false if the window was closed.
	 */
	virtual bool waitForFocus() = 0;
	/**
	 * Returns true if the window has focus, false otherwise.
	 */
	virtual bool hasFocus() = 0;
	/**
	 * Returns the system handle that represents the window. Note that any use of the handle
	 * will not be portable code.
	 */
	virtual uint getWindowHandle() = 0;

	virtual void setWindowAlive(bool v) = 0;

	/**
	 * Specifies whether the window is wanted to be closed. This is used together with CloseWanted()
	 * to allow scripts to query when the main window should be closed, or the user is asking it to close
	 **/
	void setCloseWanted(bool wanted);
	/**
	 * Returns the previous value set in a call to SetCloseWanted.
	 * Note that calling this also resets the value back to false, until such time as the SetCloseWanted()
	 * method is called again.
	**/
	bool closeWanted();

	/**
	 * Creates a new window instance. Returns a pointer to the window, or NULL if the creation failed.
	 * Note: It is the responsibility of the client to free the pointer when done with it.
	 * @param X         The X position of the window, or -1 for centre horizontal alignment
	 * @param Y         The Y position of the window, or -1 for centre vertical alignment
	 * @param Width     The width of the window without the frame
	 * @param Height    The height of the window without the frame
	 * @param Visible   Specifies whether window should be visible
	 */
	static Window *createBSWindow(int x, int y, int width, int height, bool visible);
};

} // End of namespace Sword25

#endif
