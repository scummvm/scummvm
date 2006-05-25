/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_CURSORMAN_H
#define GRAPHICS_CURSORMAN_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/stack.h"
#include "common/singleton.h"

namespace Graphics {

class CursorManager : public Common::Singleton<CursorManager> {
public:
	bool isVisible();
	bool showMouse(bool visible);

	/**
	 * Push a new cursor onto the stack, and set it in the backend. A local
	 * copy will be made of the cursor data, so the original buffer can be
	 * safely freed afterwards.
	 *
	 * @param buf		the new cursor data
	 * @param w		the width
	 * @param h		the height
	 * @param hotspotX	the hotspot X coordinate
	 * @param hotspotY	the hotspot Y coordinate
	 * @param keycolor	the index for the transparent color
	 * @param targetScale	the scale for which the cursor is designed
	 *
	 * @note It is ok for the buffer to be a NULL pointer. It is sometimes
	 *       useful to push a "dummy" cursor and modify it later. The
	 *       cursor will be added to the stack, but not to the backend.
	 */
	void pushCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetScale = 1);

	/**
	 * Pop a cursor from the stack, and restore the previous one to the
	 * backend. If there is no previous cursor, the cursor is hidden.
	 */
	void popCursor();

	/**
	 * Replace the current cursor on the stack. If the stack is empty, the
	 * cursor is pushed instead. It's a slightly more optimized way of
	 * popping the old cursor before pushing the new one.
	 *
	 * @param buf		the new cursor data
	 * @param w		the width
	 * @param h		the height
	 * @param hotspotX	the hotspot X coordinate
	 * @param hotspotY	the hotspot Y coordinate
	 * @param keycolor	the index for the transparent color
	 * @param targetScale	the scale for which the cursor is designed
	 */
	void replaceCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetScale = 1);

private:
	friend class Common::Singleton<SingletonBaseType>;
	CursorManager();

	struct Cursor {
		byte *_data;
		bool _visible;
		uint _width;
		uint _height;
		int _hotspotX;
		int _hotspotY;
		byte _keycolor;
		byte _targetScale;

		uint _size;

		Cursor(const byte *data, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetScale = 1) {
			_size = w * h;
			_data = new byte[_size];
			if (data && _data)
				memcpy(_data, data, _size);
			_width = w;
			_height = h;
			_hotspotX = hotspotX;
			_hotspotY = hotspotY;
			_keycolor = keycolor;
			_targetScale = targetScale;
		}

		~Cursor() {
			delete [] _data;
		}
	};

	Common::Stack<Cursor *> _cursorStack;
};


} // End of namespace Graphics

/** Shortcut for accessing the cursor manager. */
#define CursorMan	(::Graphics::CursorManager::instance())

#endif
