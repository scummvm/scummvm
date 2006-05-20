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
		_cursorPaletteStack.clear();
	}
}

void PaletteManager::pushCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return;

	Palette *pal = new Palette;

	pal->colors = new byte[4 * num];
	pal->start = start;
	pal->num = num;
	memcpy(pal->colors, colors, 4 * num);

	_cursorPaletteStack.push(pal);

	g_system->setCursorPalette(colors, start, num);
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
	g_system->setCursorPalette(pal->colors, pal->start, pal->num);
}

void PaletteManager::replaceCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorHasPalette))
		return;

	if (_cursorPaletteStack.empty()) {
		pushCursorPalette(colors, start, num);
		return;
	}

	Palette *pal = _cursorPaletteStack.pop();

	delete pal->colors;
	pal->colors = new byte[4 * num];
	pal->start = start;
	pal->num = num;
}

} // End of namespace Graphics
