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

#ifndef GLK_SELECTION_H
#define GLK_SELECTION_H

#include "glk/glk_types.h"
#include "glk/utils.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/ustr.h"

namespace Glk {

enum ClipSource { PRIMARY = 0, CLIPBOARD = 1 };

class Window;

/**
 * Acts as interface to and from the system's clipboard storage
 */
class Clipboard {
private:
	Common::U32String _text;
public:
	/**
	 * Makes a copy of selected text in preparation for the user copying it
	 * to the clpboard
	 */
	void clipboardStore(const Common::U32String &text);

	/**
	 * Send previously designated text to the clipboard
	 */
	void clipboardSend(ClipSource source);

	/**
	 * Receive text from the clipboard, and paste it into the current window
	 */
	void clipboardReceive(ClipSource source);
};

/**
 * Manages hyperlinks for the screen
 */
class WindowMask {
private:
	/**
	 * Clear the links data
	 */
	void clear();
public:
	size_t _hor, _ver;
	uint **_links;
	Rect _select;
	Point _last;
public:
	/**
	 * Constructor
	 */
	WindowMask();

	/**
	 * Destructor
	 */
	~WindowMask();

	/**
	 * Resize the links array
	 */
	void resize(size_t x, size_t y);

	void putHyperlink(uint linkval, uint x0, uint y0, uint x1, uint y1);

	uint getHyperlink(const Point &pos) const;
};

/**
 * Overall manager for selecting areas on the screen, copying to/from the clipboard,
 * and managing hyperlinks
 */
class Selection : public Clipboard, public WindowMask {
public:
	/**
	 * Start selecting an area of the screen
	 * @param pos       Position to start selection area at
	 */
	void startSelection(const Point &pos);

	/**
	 * Move the end point of the selection area
	 * @param pos       Position to end selection area at
	 */
	void moveSelection(const Point &pos);

	/**
	 * Remove any previously selected area
	 */
	void clearSelection();

	/**
	 * Checks whether the passed area intersects the selection area
	 */
	bool checkSelection(const Rect &r) const;

	bool getSelection(const Rect &r, int *rx0, int *rx1) const;
};

} // End of namespace Glk

#endif
