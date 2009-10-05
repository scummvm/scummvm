/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "graphics/cursorman.h"

#include "common/system.h"
#include "common/stack.h"

DECLARE_SINGLETON(Graphics::CursorManager);

namespace Graphics {

static bool g_initialized = false;

CursorManager::CursorManager() {
	if (!g_initialized) {
		g_initialized = true;
		_cursorStack.clear();
		_cursorPaletteStack.clear();
	}
}

bool CursorManager::isVisible() {
	if (_cursorStack.empty())
		return false;
	return _cursorStack.top()->_visible;
}

bool CursorManager::showMouse(bool visible) {
	if (_cursorStack.empty())
		return false;

	_cursorStack.top()->_visible = visible;

	// Should work, even if there's just a dummy cursor on the stack.
	return g_system->showMouse(visible);
}

void CursorManager::pushCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int targetScale, const Graphics::PixelFormat *format) {
	Cursor *cur = new Cursor(buf, w, h, hotspotX, hotspotY, keycolor, targetScale, format);

	cur->_visible = isVisible();
	_cursorStack.push(cur);

	if (buf) {
		g_system->setMouseCursor(cur->_data, w, h, hotspotX, hotspotY, keycolor, targetScale, format);
	}
}

void CursorManager::popCursor() {
	if (_cursorStack.empty())
		return;

	Cursor *cur = _cursorStack.pop();
	delete cur;

	if (!_cursorStack.empty()) {
		cur = _cursorStack.top();
		g_system->setMouseCursor(cur->_data, cur->_width, cur->_height, cur->_hotspotX, cur->_hotspotY, cur->_keycolor, cur->_targetScale, &cur->_format);
	}

	g_system->showMouse(isVisible());
}


void CursorManager::popAllCursors() {
	while (!_cursorStack.empty()) {
		Cursor *cur = _cursorStack.pop();
		delete cur;
	}

	g_system->showMouse(isVisible());
}

void CursorManager::replaceCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int targetScale, const Graphics::PixelFormat *format) {

	if (_cursorStack.empty()) {
		pushCursor(buf, w, h, hotspotX, hotspotY, keycolor, targetScale, format);
		return;
	}

	Cursor *cur = _cursorStack.top();

#ifdef USE_RGB_COLOR
	uint size;
	if (!format)
		size = w * h;
	else
		size = w * h * format->bytesPerPixel;
#else
	uint size = w * h;
#endif

	if (cur->_size < size) {
		delete[] cur->_data;
		cur->_data = new byte[size];
		cur->_size = size;
	}

	if (buf && cur->_data)
		memcpy(cur->_data, buf, size);

	cur->_width = w;
	cur->_height = h;
	cur->_hotspotX = hotspotX;
	cur->_hotspotY = hotspotY;
	cur->_keycolor = keycolor;
	cur->_targetScale = targetScale;
#ifdef USE_RGB_COLOR
	if (format)
		cur->_format = *format;
	else
		cur->_format = Graphics::PixelFormat::createFormatCLUT8();
#endif

	g_system->setMouseCursor(cur->_data, w, h, hotspotX, hotspotY, keycolor, targetScale, format);
}

bool CursorManager::supportsCursorPalettes() {
	return false;
}

void CursorManager::disableCursorPalette(bool disable) {
	return;
}

void CursorManager::pushCursorPalette(const byte *colors, uint start, uint num) {
	return;
}

void CursorManager::popCursorPalette() {
	return;
}

void CursorManager::replaceCursorPalette(const byte *colors, uint start, uint num) {
	return;
}

} // End of namespace Graphics
