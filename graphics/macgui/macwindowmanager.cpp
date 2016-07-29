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

#include "common/array.h"
#include "common/events.h"
#include "common/list.h"
#include "common/unzip.h"
#include "common/system.h"
#include "common/stream.h"

#include "graphics/cursorman.h"
#include "graphics/fonts/bdf.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "wage/macwindow.h"
#include "wage/macmenu.h"

namespace Graphics {

static const byte palette[] = {
	0, 0, 0,           // Black
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff,  // White
	0x00, 0xff, 0x00,  // Green
	0x00, 0xcf, 0x00   // Green2
};

static byte fillPatterns[][8] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, // kPatternSolid
								  { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 }, // kPatternStripes
								  { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 }, // kPatternCheckers
								  { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa }  // kPatternCheckers2
};

static const byte macCursorArrow[] = {
	2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 0, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 0, 0, 2, 3, 3, 3, 3, 3, 3, 3,
	2, 0, 0, 0, 2, 3, 3, 3, 3, 3, 3,
	2, 0, 0, 0, 0, 2, 3, 3, 3, 3, 3,
	2, 0, 0, 0, 0, 0, 2, 3, 3, 3, 3,
	2, 0, 0, 0, 0, 0, 0, 2, 3, 3, 3,
	2, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3,
	2, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2,
	2, 0, 0, 2, 0, 0, 2, 3, 3, 3, 3,
	2, 0, 2, 3, 2, 0, 0, 2, 3, 3, 3,
	2, 2, 3, 3, 2, 0, 0, 2, 3, 3, 3,
	2, 3, 3, 3, 3, 2, 0, 0, 2, 3, 3,
	3, 3, 3, 3, 3, 2, 0, 0, 2, 3, 3,
	3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 3
};

static const byte macCursorBeam[] = {
	0, 0, 3, 3, 3, 0, 0, 3, 3, 3, 3,
	3, 3, 0, 3, 0, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 0, 3, 0, 3, 3, 3, 3, 3, 3,
	0, 0, 3, 3, 3, 0, 0, 3, 3, 3, 3,
};

MacWindowManager::MacWindowManager() {
    _screen = 0;
    _lastId = 0;
    _activeWindow = -1;

	_menu = 0;

	_fullRefresh = true;

	_builtInFonts = true;

	for (int i = 0; i < ARRAYSIZE(fillPatterns); i++)
		_patterns.push_back(fillPatterns[i]);

	loadFonts();

	g_system->getPaletteManager()->setPalette(palette, 0, ARRAYSIZE(palette) / 3);

	CursorMan.replaceCursorPalette(palette, 0, ARRAYSIZE(palette) / 3);
	CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
	_cursorIsArrow = true;
	CursorMan.showMouse(true);
}

MacWindowManager::~MacWindowManager() {
    for (int i = 0; i < _lastId; i++)
        delete _windows[i];
}

MacWindow *MacWindowManager::addWindow(bool scrollable, bool resizable, bool editable) {
	MacWindow *w = new MacWindow(_lastId, scrollable, resizable, editable, this);

    _windows.push_back(w);
    _windowStack.push_back(w);

    setActive(_lastId);

    _lastId++;

    return w;
}

Wage::Menu *MacWindowManager::addMenu() {
	_menu = new Wage::Menu(_lastId, _screen->getBounds(), this);

	_windows.push_back(_menu);

	_lastId++;

	return _menu;
}

void MacWindowManager::setActive(int id) {
    if (_activeWindow == id)
        return;

    if (_activeWindow != -1)
        _windows[_activeWindow]->setActive(false);

    _activeWindow = id;

    _windows[id]->setActive(true);

    _windowStack.remove(_windows[id]);
    _windowStack.push_back(_windows[id]);

    _fullRefresh = true;
}

struct PlotData {
	Graphics::ManagedSurface *surface;
	Patterns *patterns;
	uint fillType;
	int thickness;

	PlotData(Graphics::ManagedSurface *s, Patterns *p, int f, int t) :
		surface(s), patterns(p), fillType(f), thickness(t) {}
};

static void drawPixel(int x, int y, int color, void *data) {
	PlotData *p = (PlotData *)data;

	if (p->fillType > p->patterns->size())
		return;

	byte *pat = p->patterns->operator[](p->fillType - 1);

	if (p->thickness == 1) {
		if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
			uint xu = (uint)x; // for letting compiler optimize it
			uint yu = (uint)y;

			*((byte *)p->surface->getBasePtr(xu, yu)) =
				(pat[yu % 8] & (1 << (7 - xu % 8))) ?
					color : kColorWhite;
		}
	} else {
		int x1 = x;
		int x2 = x1 + p->thickness;
		int y1 = y;
		int y2 = y1 + p->thickness;

		for (y = y1; y < y2; y++)
			for (x = x1; x < x2; x++)
				if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
					uint xu = (uint)x; // for letting compiler optimize it
					uint yu = (uint)y;
					*((byte *)p->surface->getBasePtr(xu, yu)) =
						(pat[yu % 8] & (1 << (7 - xu % 8))) ?
							color : kColorWhite;
				}
	}
}

void MacWindowManager::drawDesktop() {
	Common::Rect r(_screen->getBounds());

	PlotData pd(_screen, &_patterns, kPatternCheckers, 1);

	Graphics::drawRoundRect(r, kDesktopArc, kColorBlack, true, drawPixel, &pd);

	g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _screen->w, _screen->h);
}

void MacWindowManager::draw() {
    assert(_screen);

	if (_fullRefresh)
		drawDesktop();

    for (Common::List<BaseMacWindow *>::const_iterator it = _windowStack.begin(); it != _windowStack.end(); it++) {
		BaseMacWindow *w = *it;
        if (w->draw(_screen, _fullRefresh)) {
            w->setDirty(false);

			Common::Rect clip(w->getDimensions().left - 2, w->getDimensions().top - 2, w->getDimensions().right - 2, w->getDimensions().bottom - 2);
			clip.clip(_screen->getBounds());

            g_system->copyRectToScreen(_screen->getBasePtr(clip.left, clip.top), _screen->pitch, clip.left, clip.top, clip.width(), clip.height());
        }
    }

	// Menu is drawn on top of everything and always
	if (_menu)
		_menu->draw(_screen, _fullRefresh);

    _fullRefresh = false;
}

bool MacWindowManager::processEvent(Common::Event &event) {
	// Menu gets events first fir shortcuts and menu bar
	if (_menu && _menu->processEvent(event))
		return true;

    if (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN &&
            event.type != Common::EVENT_LBUTTONUP)
        return false;

	if (_windows[_activeWindow]->isEditable() && _windows[_activeWindow]->getType() == kWindowWindow &&
			((MacWindow *)_windows[_activeWindow])->getInnerDimensions().contains(event.mouse.x, event.mouse.y)) {
		if (_cursorIsArrow) {
			CursorMan.replaceCursor(macCursorBeam, 11, 16, 3, 8, 3);
			_cursorIsArrow = false;
		}
	} else {
		if (_cursorIsArrow == false) {
			CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
			_cursorIsArrow = true;
		}
	}

    for (Common::List<BaseMacWindow *>::const_iterator it = _windowStack.end(); it != _windowStack.begin();) {
        it--;
		BaseMacWindow *w = *it;

        if (w->hasAllFocus() || w->getDimensions().contains(event.mouse.x, event.mouse.y)) {
            if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP)
                setActive(w->getId());

            return w->processEvent(event);
        }
    }

    return false;
}

//////////////////////
// Font stuff
//////////////////////
void MacWindowManager::loadFonts() {
	Common::Archive *dat;

	dat = Common::makeZipArchive("classicmacfonts.dat");

	if (!dat) {
		warning("Could not find classicmacfonts.dat. Falling back to built-in fonts");
		_builtInFonts = true;

		return;
	}

	Common::ArchiveMemberList list;
	dat->listMembers(list);

	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getName());

		Graphics::BdfFont *font = Graphics::BdfFont::loadFont(*stream);

		delete stream;

		Common::String fontName = (*it)->getName();

		// Trim the .bdf extension
		for (int i = fontName.size() - 1; i >= 0; --i) {
			if (fontName[i] == '.') {
				while ((uint)i < fontName.size()) {
					fontName.deleteLastChar();
				}
				break;
			}
		}

		FontMan.assignFontToName(fontName, font);

		debug(2, " %s", fontName.c_str());
	}

	_builtInFonts = false;

	delete dat;
}

const Graphics::Font *MacWindowManager::getFont(const char *name, Graphics::FontManager::FontUsage fallback) {
	const Graphics::Font *font = 0;

	if (!_builtInFonts) {
		font = FontMan.getFontByName(name);

		if (!font)
			warning("Cannot load font %s", name);
	}

	if (_builtInFonts || !font)
		font = FontMan.getFontByUsage(fallback);

	return font;
}

/////////////////
// Cursor stuff
/////////////////
void MacWindowManager::pushArrowCursor() {
	CursorMan.pushCursor(macCursorArrow, 11, 16, 1, 1, 3);
}

void MacWindowManager::popCursor() {
	CursorMan.popCursor();
}


} // End of namespace Graphics
