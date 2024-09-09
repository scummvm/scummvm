/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/cursorman.h"

#include "common/rect.h"
#include "common/system.h"
#include "common/stack.h"

namespace Common {
DECLARE_SINGLETON(Graphics::CursorManager);
}

namespace Graphics {

static const int CURSOR_W = 12;
static const int CURSOR_H = 20;
static const byte ARROW_CURSOR[CURSOR_W * CURSOR_H] = {
	1,1,0,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,1,1,1,1,1,
	1,2,2,2,1,2,2,1,0,0,0,0,
	1,2,2,1,1,2,2,1,0,0,0,0,
	1,2,1,0,0,1,2,2,1,0,0,0,
	1,1,0,0,0,1,2,2,1,0,0,0,
	1,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,0,1,1,0,0,
};
static const byte CURSOR_PALETTE[] = { 0x80, 0x80, 0x80, 0, 0, 0, 0xff, 0xff, 0xff };

CursorManager::~CursorManager() {
	for (Common::Stack<Cursor *>::size_type i = 0; i < _cursorStack.size(); ++i)
		delete _cursorStack[i];
	_cursorStack.clear();
	for (Common::Stack<Palette *>::size_type i = 0; i < _cursorPaletteStack.size(); ++i)
		delete _cursorPaletteStack[i];
	_cursorPaletteStack.clear();
}

bool CursorManager::isVisible() {
	if (_cursorStack.empty())
		return false;
	return _cursorStack.top()->_visible;
}

bool CursorManager::showMouse(bool visible) {
	if (_cursorStack.empty())
		return false;
	if (_locked) {
		return false;
	}

	_cursorStack.top()->_visible = visible;

	// Should work, even if there's just a dummy cursor on the stack.
	return g_system->showMouse(visible);
}

void CursorManager::pushCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	PixelFormat pixelFormat;
	if (format)
		pixelFormat = *format;
	else
		pixelFormat = PixelFormat::createFormatCLUT8();

	Surface surf;
	// we won't touch 'buf' ...
	surf.init(w, h, w * pixelFormat.bytesPerPixel, const_cast<void *>(buf), pixelFormat);

	pushCursor(surf, hotspotX, hotspotY, keycolor, dontScale, mask);
}

void CursorManager::pushCursor(const Surface &surf, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const byte *mask) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorMask))
		mask = nullptr;

	Cursor *cur = new Cursor(surf, hotspotX, hotspotY, keycolor, dontScale, mask);

	cur->_visible = isVisible();
	_cursorStack.push(cur);

	g_system->setMouseCursor(cur->_surf.getPixels(), cur->_surf.w, cur->_surf.h, hotspotX, hotspotY, keycolor, dontScale, &cur->_surf.format, mask);
}

void CursorManager::popCursor() {
	if (_cursorStack.empty())
		return;

	Cursor *cur = _cursorStack.pop();
	delete cur;

	if (!_cursorStack.empty()) {
		cur = _cursorStack.top();
		g_system->setMouseCursor(cur->_surf.getPixels(), cur->_surf.w, cur->_surf.h, cur->_hotspotX, cur->_hotspotY, cur->_keycolor, cur->_dontScale, &cur->_surf.format, cur->_mask);
	} else {
		g_system->setMouseCursor(nullptr, 0, 0, 0, 0, 0);
	}

	g_system->showMouse(isVisible());
}


void CursorManager::popAllCursors() {
	while (!_cursorStack.empty()) {
		Cursor *cur = _cursorStack.pop();
		delete cur;
	}

	if (g_system->hasFeature(OSystem::kFeatureCursorPalette)) {
		while (!_cursorPaletteStack.empty()) {
			Palette *pal = _cursorPaletteStack.pop();
			delete pal;
		}
	}

	g_system->setMouseCursor(nullptr, 0, 0, 0, 0, 0);
	g_system->showMouse(isVisible());
}

void CursorManager::replaceCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask) {
	PixelFormat pixelFormat;
	if (format)
		pixelFormat = *format;
	else
		pixelFormat = PixelFormat::createFormatCLUT8();

	Surface surf;
	// we won't touch 'buf' ...
	surf.init(w, h, w * pixelFormat.bytesPerPixel, const_cast<void *>(buf), pixelFormat);

	replaceCursor(surf, hotspotX, hotspotY, keycolor, dontScale, mask);
}

void CursorManager::replaceCursor(const Surface &surf, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const byte *mask) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorMask))
		mask = nullptr;

	if (_cursorStack.empty()) {
		pushCursor(surf, hotspotX, hotspotY, keycolor, dontScale, mask);
		return;
	}

	Cursor *cur = _cursorStack.top();

	const uint size = surf.w * surf.h * surf.format.bytesPerPixel;

	if (cur->_size < size) {
		// Don't use Surface::create() here because that doesn't guarantee
		// linearity of the surface buffer (i.e. pitch must be the same as
		// width * bytesPerPixel).
		delete[] (byte *)cur->_surf.getPixels();
		cur->_surf.setPixels(new byte[size]);
		cur->_size = size;
	}

	cur->_surf.pitch = surf.w * surf.format.bytesPerPixel;
	cur->_surf.w = surf.w;
	cur->_surf.h = surf.h;
	cur->_surf.format = surf.format;

	if (surf.getPixels() && cur->_surf.getPixels())
		cur->_surf.copyRectToSurface(surf, 0, 0, Common::Rect(surf.w, surf.h));

	delete[] cur->_mask;
	cur->_mask = nullptr;

	if (mask) {
		cur->_mask = new byte[surf.w * surf.h];
		memcpy(cur->_mask, mask, surf.w * surf.h);
	}

	cur->_hotspotX = hotspotX;
	cur->_hotspotY = hotspotY;
	cur->_keycolor = keycolor;
	cur->_dontScale = dontScale;

	g_system->setMouseCursor(cur->_surf.getPixels(), surf.w, surf.h, hotspotX, hotspotY, keycolor, dontScale, &cur->_surf.format, mask);
}

void CursorManager::replaceCursor(const Graphics::Cursor *cursor) {
	replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(),
				  cursor->getHotspotY(), cursor->getKeyColor(), false, nullptr, cursor->getMask());

	if (cursor->getPalette())
		replaceCursorPalette(cursor->getPalette(), cursor->getPaletteStartIndex(), cursor->getPaletteCount());
}

bool CursorManager::supportsCursorPalettes() {
	return g_system->hasFeature(OSystem::kFeatureCursorPalette);
}

void CursorManager::disableCursorPalette(bool disable) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	if (_cursorPaletteStack.empty())
		return;

	Palette *pal = _cursorPaletteStack.top();
	pal->_disabled = disable;

	g_system->setFeatureState(OSystem::kFeatureCursorPalette, !disable);
}

void CursorManager::pushCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	Palette *pal = new Palette(colors, start, num);
	_cursorPaletteStack.push(pal);

	if (num)
		g_system->setCursorPalette(colors, start, num);
	else
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
}

void CursorManager::popCursorPalette() {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	if (_cursorPaletteStack.empty())
		return;

	Palette *pal = _cursorPaletteStack.pop();
	delete pal;

	if (_cursorPaletteStack.empty()) {
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
		return;
	}

	pal = _cursorPaletteStack.top();

	if (pal->_num && !pal->_disabled)
		g_system->setCursorPalette(pal->_data, pal->_start, pal->_num);
	else
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
}

void CursorManager::replaceCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	if (_cursorPaletteStack.empty()) {
		pushCursorPalette(colors, start, num);
		return;
	}

	Palette *pal = _cursorPaletteStack.top();
	uint size = 3 * num;

	if (pal->_size < size) {
		// Could not re-use the old buffer. Create a new one.
		delete[] pal->_data;
		pal->_data = new byte[size];
		pal->_size = size;
	}

	pal->_start = start;
	pal->_num = num;

	if (num) {
		memcpy(pal->_data, colors, size);
		g_system->setCursorPalette(pal->_data, pal->_start, pal->_num);
	} else {
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
	}
}

void CursorManager::lock(bool locked) {
	_locked = locked;
}

void CursorManager::setDefaultArrowCursor(bool push) {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	if (push) {
		pushCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);
		pushCursor(ARROW_CURSOR, CURSOR_W, CURSOR_H, 0, 0, 0, true, &format);
	} else {
		replaceCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);
		replaceCursor(ARROW_CURSOR, CURSOR_W, CURSOR_H, 0, 0, 0, true, &format);
	}
}

CursorManager::Cursor::Cursor(const Surface &surf, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const byte *mask) {
	const uint32 keycolor_mask = (((uint32) -1) >> (sizeof(uint32) * 8 - surf.format.bytesPerPixel * 8));
	_keycolor = keycolor & keycolor_mask;
	_size = surf.w * surf.h * surf.format.bytesPerPixel;

	// make sure that the width * bytesPerPixel == pitch
	_surf.init(surf.w, surf.h, surf.w * surf.format.bytesPerPixel, new byte[_size], surf.format);
	if (surf.getPixels() && _surf.getPixels())
		_surf.copyRectToSurface(surf, 0, 0, Common::Rect(surf.w, surf.h));

	if (mask) {
		_mask = new byte[surf.w * surf.h];
		if (_mask)
			memcpy(_mask, mask, surf.w * surf.h);
	} else
		_mask = nullptr;

	_hotspotX = hotspotX;
	_hotspotY = hotspotY;
	_dontScale = dontScale;
	_visible = false;
}

CursorManager::Cursor::~Cursor() {
	delete[] (byte *)_surf.getPixels();
	delete[] _mask;
}

CursorManager::Palette::Palette(const byte *colors, uint start, uint num) {
	_start = start;
	_num = num;
	_size = 3 * num;

	if (num) {
		_data = new byte[_size];
		memcpy(_data, colors, _size);
	} else {
		_data = NULL;
	}

	_disabled = false;
}

CursorManager::Palette::~Palette() {
	delete[] _data;
}

} // End of namespace Graphics
