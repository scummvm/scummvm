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

#include "graphics/paletteman.h"

#include "common/system.h"
#include "common/stack.h"

DECLARE_SINGLETON(Graphics::PaletteManager);

namespace Graphics {

static bool g_initialized = false;

PaletteManager::PaletteManager() {
	if (!g_initialized) {
		g_initialized = true;
		_cursorPaletteStack.clear();
	}
}

void PaletteManager::pushCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return;

	Palette *pal = new Palette(colors, start, num);
	_cursorPaletteStack.push(pal);

	if (num)
		g_system->setCursorPalette(colors, start, num);
	else
		g_system->disableCursorPalette(true);
}

void PaletteManager::popCursorPalette() {
	if (!g_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return;

	if (_cursorPaletteStack.empty())
		return;

	Palette *pal;

	pal = _cursorPaletteStack.pop();
	delete pal;

	if (_cursorPaletteStack.empty()) {
		g_system->disableCursorPalette(true);
		return;
	}

	pal = _cursorPaletteStack.top();

	if (pal->_num)
		g_system->setCursorPalette(pal->_colors, pal->_start, pal->_num);
	else
		g_system->disableCursorPalette(true);
}

void PaletteManager::replaceCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return;

	if (_cursorPaletteStack.empty()) {
		pushCursorPalette(colors, start, num);
		return;
	}

	Palette *pal = _cursorPaletteStack.top();
	uint size = 4 * num;

	if (pal->_size < size) {
		// Could not re-use the old buffer. Create a new one.
		delete pal->_colors;
		pal->_colors = new byte[size];
		pal->_size = size;
	}

	pal->_start = start;
	pal->_num = num;

	if (num) {
		memcpy(pal->_colors, colors, 4 * num);
		g_system->setCursorPalette(pal->_colors, pal->_start, pal->_num);
	} else {
		g_system->disableCursorPalette(true);
	}
}

} // End of namespace Graphics
