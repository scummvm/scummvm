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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef GRAPHICS_MACWINDOWMANAGER_H
#define GRAPHICS_MACWINDOWMANAGER_H

#include "common/array.h"
#include "common/list.h"
#include "common/events.h"
#include "common/archive.h"

#include "graphics/fontman.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {	

namespace MacGUIConstants {
	enum {
		kDesktopArc = 7
	};

	enum {
		kColorBlack = 0,
		kColorGray = 1,
		kColorWhite = 2,
		kColorGreen = 3,
		kColorGreen2 = 4
	};

	enum {
		kPatternSolid = 1,
		kPatternStripes = 2,
		kPatternCheckers = 3,
		kPatternCheckers2 = 4
	};
}
using namespace MacGUIConstants;

class ManagedSurface;

class Menu;

typedef Common::Array<byte *> MacPatterns;

class MacWindowManager {
public:
	MacWindowManager();
	~MacWindowManager();

	void setScreen(ManagedSurface *screen) { _screen = screen; }
	bool hasBuiltInFonts() { return _builtInFonts; }
	const Font *getFont(const char *name, FontManager::FontUsage fallback);

	MacWindow *addWindow(bool scrollable, bool resizable, bool editable);
	Menu *addMenu();
	void setActive(int id);

	void setFullRefresh(bool redraw) { _fullRefresh = true; }

	void draw();

	bool processEvent(Common::Event &event);

	BaseMacWindow *getWindow(int id) { return _windows[id]; }

	MacPatterns &getPatterns() { return _patterns; }
	void drawFilledRoundRect(ManagedSurface *surface, Common::Rect &rect, int arc, int color);

	void pushArrowCursor();
	void popCursor();

private:
	void drawDesktop();
	void loadFonts();

private:
	ManagedSurface *_screen;

	Common::List<BaseMacWindow *> _windowStack;
	Common::Array<BaseMacWindow *> _windows;

	int _lastId;
	int _activeWindow;

	bool _fullRefresh;

	MacPatterns _patterns;

	Menu *_menu;

	bool _builtInFonts;
	bool _cursorIsArrow;
};

} // End of namespace Graphics

#endif
