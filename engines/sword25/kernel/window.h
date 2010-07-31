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
 * BS_Window
 * ---------
 * Simple window class interface. This is being encapsulated in a class for
 * reasons of portability.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_WINDOW_H
#define SWORD25_WINDOW_H

// Includes
#include "sword25/kernel/common.h"

namespace Sword25 {

// Class definitions

/**
 * A simple window class interface
 *
 * Windows are exclusively created by BS_Window::CreateBSWindow().
 * BS_Windows selects the correct class for the environment.
 */
class BS_Window {
protected:
	bool _InitSuccess;
	bool _CloseWanted;

public:
	virtual ~BS_Window(){};

	/**
	 * Returns the visibility of the window.
	 */
	virtual bool IsVisible() = 0;

	/**
	 * Sets the visibility of the window
	 * @param Visible		Specifies whether the window should be visible or hidden
	 */
	virtual void SetVisible(bool Visible) = 0;
	/**
	 * Returns the X position of the window
	 */
	virtual int GetX() = 0;
	/**
	 * Sets the X position of the window
	 * @paramX		The new X position for the window, or -1 for centre aligned
	 */
	virtual void SetX(int X) = 0;
	/**
	 * Gets the Y position of the window
	 */
	virtual int GetY() = 0;
	/**
	 * Sets the Y position of the window
	 * @param Y		The new Y position for the window, or -1 for centre aligned
	 */
	virtual void SetY(int X) = 0;
	/**
	 * Returns the X position of the window's client area
	 */
	virtual int GetClientX() = 0;
	/**
	 * Returns the Y position of the window's client area
	 */
	virtual int GetClientY() = 0;
	/**
	 * Returns the width of the window without the frame
	 */
	virtual int GetWidth() = 0;
	/**
	 * Sets the width of the window without the frame
	 */
	virtual void SetWidth(int Width) = 0;
	/**
	 * Gets the height of the window without the frame
	 */
	virtual int GetHeight() = 0;
	/**
	 * Sets the height of the window without the frame
	 */
	virtual void SetHeight(int Height) = 0;
	/**
	 * Returns the title of the window
	 */
	virtual Common::String GetTitle() = 0;
	/**
	 * Sets the title of the window
	 * @param Title		The new window title
	*/
	virtual void SetTitle(const Common::String &Title) = 0;
	/**
	 * Handle the processing of any pending window messages. This method should be called
	 * during the main loop.
	 */
	virtual bool ProcessMessages() = 0;
	/**
	 * Pauses the applicaiton until the window has focus, or has been closed.
	 * Returns false if the window was closed.
	 */
	virtual bool WaitForFocus() = 0;	
	/**
	 * Returns true if the window has focus, false otherwise.
	 */
	virtual bool HasFocus() = 0;
	/**
	 * Returns the system handle that represents the window. Note that any use of the handle
	 * will not be portable code.
	 */
	virtual unsigned int GetWindowHandle() = 0;


	/**
	 * Specifies whether the window is wanted to be closed. This is used together with CloseWanted()
	 * to allow scripts to query when the main window should be closed, or the user is asking it to close
	 **/
	void SetCloseWanted(bool Wanted);	
	/**
	 * Returns the previous value set in a call to SetCloseWanted. 
	 * Note that calling this also resets the value back to false, until such time as the SetCloseWanted()
	 * method is called again.
	**/
	bool CloseWanted();

	/**
	 * Creates a new window instance. Returns a pointer to the window, or NULL if the creation failed.
	 * Note: It is the responsibility of the client to free the pointer when done with it.
	 * @param X			The X position of the window, or -1 for centre horizontal alignment
	 * @param Y			The Y position of the window, or -1 for centre vertical alignment
	 * @param Width		The width of the window without the frame
	 * @param Height	The height of the window without the frame
	 * @param Visible	Specifies whether window should be visible
	 */
	static BS_Window *CreateBSWindow(int X, int Y, int Width, int Height, bool Visible);
};

} // End of namespace Sword25

#endif
