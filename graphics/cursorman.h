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
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_CURSORMAN_H
#define GRAPHICS_CURSORMAN_H

#include "common/scummsys.h"
#include "common/stack.h"
#include "common/singleton.h"
#ifdef ENABLE_16BIT
#include "graphics/pixelformat.h"
#include "common/system.h"
#endif

namespace Graphics {

class CursorManager : public Common::Singleton<CursorManager> {
public:
	/** Query whether the mouse cursor is visible. */
	bool isVisible();

	/** Show or hide the mouse cursor. */
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
#ifdef ENABLE_16BIT
	void pushCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 0xFFFFFFFF, int targetScale = 1);
#else
	void pushCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetScale = 1);
#endif

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
#ifdef ENABLE_16BIT
	void replaceCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 0xFFFFFFFF, int targetScale = 1);
#else
	void replaceCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetScale = 1);
#endif

	/**
	 * Pop all of the cursors and cursor palettes from their respective stacks.
	 * The purpose is to ensure that all unecessary cursors are removed from the
	 * stack when returning to the launcher from an engine.
	 *
	 */
	void popAllCursors();

	/**
	 * Enable/Disable the current cursor palette.
	 *
	 * @param disable
	 */
	void disableCursorPalette(bool disable);

	/**
	 * Push a new cursor palette onto the stack, and set it in the backend.
	 * The palette entries from 'start' till (start+num-1) will be replaced
	 * so a full palette updated is accomplished via start=0, num=256.
	 *
	 * The palette data is specified in the same interleaved RGBA format as
	 * used by all backends.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note If num is zero, the cursor palette is disabled.
	 */
	void pushCursorPalette(const byte *colors, uint start, uint num);

	/**
	 * Pop a cursor palette from the stack, and restore the previous one to
	 * the backend. If there is no previous palette, the cursor palette is
	 * disabled instead.
	 */
	void popCursorPalette();

	/**
	 * Replace the current cursor palette on the stack. If the stack is
	 * empty, the palette is pushed instead. It's a slightly more optimized
	 * way of popping the old palette before pushing the new one.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note If num is zero, the cursor palette is disabled.
	 */
	void replaceCursorPalette(const byte *colors, uint start, uint num);

#ifdef ENABLE_16BIT
	/**
	 * Push a new cursor pixel format onto the stack, and set it in the backend.
	 *
	 * @param format	the new format data, in a Graphics::PixelFormat
	 */
	void pushCursorFormat(PixelFormat format);

	/**
	 * Pop a cursor pixel format from the stack, and restore the previous one to
	 * the backend. If there is no previous format, the screen format is
	 * used instead.
	 */
	void popCursorFormat();

	/**
	 * Replace the current cursor pixel format on the stack. If the stack is
	 * empty, the format is pushed instead. It's a slightly more optimized
	 * way of popping the old format before pushing the new one.
	 *
	 * @param format	the new format data, in a Graphics::PixelFormat
	 */
	void replaceCursorFormat(PixelFormat format);
#endif

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
//#ifdef ENABLE_16BIT
		uint32 _keycolor;
//#else
//		byte _keycolor;
//#endif
		byte _targetScale;


		uint _size;
#ifdef ENABLE_16BIT
		Cursor(const byte *data, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 0xFFFFFFFF, int targetScale = 1, uint8 bitDepth = 8) {
			{	//limit the lifespan of the format value to minimize impact on memory usage
				Graphics::PixelFormat f = g_system->getScreenFormat();
				_size = w * h * f.bytesPerPixel;
				_keycolor = keycolor & ((1 << (f.bytesPerPixel << 3)) - 1);
			}
#else
		Cursor(const byte *data, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int targetScale = 1) {
			_size = w * h;
			_keycolor = keycolor;
#endif
			_data = new byte[_size];
			if (data && _data)
				memcpy(_data, data, _size);
			_width = w;
			_height = h;
			_hotspotX = hotspotX;
			_hotspotY = hotspotY;
			_targetScale = targetScale;
		}

		~Cursor() {
			delete[] _data;
		}
	};

	struct Palette {
		byte *_data;
		uint _start;
		uint _num;
		uint _size;

		bool _disabled;

		Palette(const byte *colors, uint start, uint num) {
			_start = start;
			_num = num;
			_size = 4 * num;

			if (num) {
				_data = new byte[_size];
				memcpy(_data, colors, _size);
			} else {
				_data = NULL;
			}

			_disabled = false;
		}

		~Palette() {
			delete[] _data;
		}
	};
	Common::Stack<Cursor *> _cursorStack;
	Common::Stack<Palette *> _cursorPaletteStack;
#ifdef ENABLE_16BIT
	Common::Stack<Graphics::PixelFormat *> _cursorFormatStack;
#endif
};

} // End of namespace Graphics

#define CursorMan	(::Graphics::CursorManager::instance())

#endif
