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
 */

#include "common/array.h"
#include "common/events.h"
#include "common/list.h"
#include "common/system.h"
#include "common/timer.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/mactextwindow.h"
#include "graphics/macgui/macmenu.h"

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
								  { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa }, // kPatternCheckers2
								  { 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22 }, // kPatternLightGray
								  { 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd }  // kPatternDarkGray
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
static const byte macCursorCrossHair[] = {
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
static const byte macCursorWatch[] = {
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
	1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
};
static const byte macCursorCrossBar[] = {
	0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
	1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
	1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1,
	1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1,
	1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1,
	0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void menuTimerHandler(void *refCon);

MacWindowManager::MacWindowManager() {
	_screen = 0;
	_screenCopy = 0;
	_lastId = 0;
	_activeWindow = -1;
	_needsRemoval = false;

	_mode = kWMModeNone;

	_menu = 0;
	_menuDelay = 0;
	_menuTimerActive = false;

	_engine = nullptr;
	_pauseEngineCallback = nullptr;

	_colorBlack = 0;
	_colorWhite = 2;

	_fullRefresh = true;

	for (int i = 0; i < ARRAYSIZE(fillPatterns); i++)
		_patterns.push_back(fillPatterns[i]);

	g_system->getPaletteManager()->setPalette(palette, 0, ARRAYSIZE(palette) / 3);

	_fontMan = new MacFontManager();

	CursorMan.replaceCursorPalette(palette, 0, ARRAYSIZE(palette) / 3);
	CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
	_cursorIsArrow = true;
	CursorMan.showMouse(true);
}

MacWindowManager::~MacWindowManager() {
	for (int i = 0; i < _lastId; i++)
		delete _windows[i];

	delete _fontMan;
	delete _screenCopy;

	g_system->getTimerManager()->removeTimerProc(&menuTimerHandler);
}

MacWindow *MacWindowManager::addWindow(bool scrollable, bool resizable, bool editable) {
	MacWindow *w = new MacWindow(_lastId, scrollable, resizable, editable, this);

	addWindowInitialized(w);

	setActive(getNextId());

	return w;
}

MacTextWindow *MacWindowManager::addTextWindow(const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler) {
	MacTextWindow *w = new MacTextWindow(this, font, fgcolor, bgcolor, maxWidth, textAlignment, menu, cursorHandler);

	addWindowInitialized(w);

	setActive(getNextId());

	return w;
}


void MacWindowManager::addWindowInitialized(MacWindow *macwindow) {
	_windows.push_back(macwindow);
	_windowStack.push_back(macwindow);
}

MacMenu *MacWindowManager::addMenu() {
	_menu = new MacMenu(getNextId(), _screen->getBounds(), this);

	_windows.push_back(_menu);

	return _menu;
}

void MacWindowManager::activateMenu() {
	if (!_menu)
		return;

	_menu->setVisible(true);
}

bool MacWindowManager::isMenuActive() {
	if (!_menu)
		return false;

	return _menu->isVisible();
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

void MacWindowManager::removeWindow(MacWindow *target) {
	_windowsToRemove.push_back(target);
	_needsRemoval = true;
}

void macDrawPixel(int x, int y, int color, void *data) {
	MacPlotData *p = (MacPlotData *)data;

	if (p->fillType > p->patterns->size())
		return;

	byte *pat = p->patterns->operator[](p->fillType - 1);

	if (p->thickness == 1) {
		if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
			uint xu = (uint)x; // for letting compiler optimize it
			uint yu = (uint)y;

			*((byte *)p->surface->getBasePtr(xu, yu)) =
				(pat[yu % 8] & (1 << (7 - xu % 8))) ?
					color : p->bgColor;
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
							color : p->bgColor;
				}
	}
}

void MacWindowManager::drawDesktop() {
	Common::Rect r(_screen->getBounds());

	MacPlotData pd(_screen, &_patterns, kPatternCheckers, 1, _colorWhite);

	Graphics::drawRoundRect(r, kDesktopArc, _colorBlack, true, macDrawPixel, &pd);

	g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _screen->w, _screen->h);
}

void MacWindowManager::draw() {
	assert(_screen);

	removeMarked();

	if (_fullRefresh && !(_mode & kWMModeNoDesktop))
		drawDesktop();

	for (Common::List<BaseMacWindow *>::const_iterator it = _windowStack.begin(); it != _windowStack.end(); it++) {
		BaseMacWindow *w = *it;
		if (w->draw(_screen, _fullRefresh)) {
			w->setDirty(false);

			Common::Rect clip(w->getDimensions().left - 2, w->getDimensions().top - 2, w->getDimensions().right - 2, w->getDimensions().bottom - 2);
			clip.clip(_screen->getBounds());
			clip.clip(Common::Rect(0, 0, g_system->getWidth() - 1, g_system->getHeight() - 1));

			if (!clip.isEmpty())
				g_system->copyRectToScreen(_screen->getBasePtr(clip.left, clip.top), _screen->pitch, clip.left, clip.top, clip.width(), clip.height());
		}
	}

	// Menu is drawn on top of everything and always
	if (_menu)
		_menu->draw(_screen, _fullRefresh);

	_fullRefresh = false;
}

static void menuTimerHandler(void *refCon) {
	MacWindowManager *wm = (MacWindowManager *)refCon;

	if (wm->_menuHotzone.contains(wm->_lastMousePos)) {
		wm->activateMenu();
		if (wm->_mode & kWMModalMenuMode) {
			if (!wm->_screenCopy)
				wm->_screenCopy = new ManagedSurface(*wm->_screen);	// Create a copy
			else
				*wm->_screenCopy = *wm->_screen;
			wm->pauseEngine(true);
		}
	}

	wm->_menuTimerActive = false;

	g_system->getTimerManager()->removeTimerProc(&menuTimerHandler);
}

bool MacWindowManager::processEvent(Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE)
		_lastMousePos = event.mouse;

	if (_menu && !_menu->isVisible()) {
		if ((_mode & kWMModeAutohideMenu) && event.type == Common::EVENT_MOUSEMOVE) {
			if (!_menuTimerActive && _menuHotzone.contains(event.mouse)) {
				_menuTimerActive = true;

				g_system->getTimerManager()->installTimerProc(&menuTimerHandler, _menuDelay, this, "menuWindowCursor");
			}
		}
	}

	// Menu gets events first for shortcuts and menu bar
	if (_menu && _menu->processEvent(event))
		return true;

	if (_activeWindow != -1) {
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
	}

	for (Common::List<BaseMacWindow *>::const_iterator it = _windowStack.end(); it != _windowStack.begin();) {
		it--;
		BaseMacWindow *w = *it;

		if (w->hasAllFocus() || (w->isEditable() && event.type == Common::EVENT_KEYDOWN) ||
				w->getDimensions().contains(event.mouse.x, event.mouse.y)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP)
				setActive(w->getId());

			return w->processEvent(event);
		}
	}

	return false;
}

void MacWindowManager::removeMarked() {
	if (!_needsRemoval) return;

	Common::List<BaseMacWindow *>::const_iterator it;
	for (it = _windowsToRemove.begin(); it != _windowsToRemove.end(); it++) {
		removeFromStack(*it);
		removeFromWindowList(*it);
		delete *it;
		_activeWindow = 0;
		_fullRefresh = true;
	}
	_windowsToRemove.clear();
	_needsRemoval = false;
	_lastId = _windows.size();
}

void MacWindowManager::removeFromStack(BaseMacWindow *target) {
	Common::List<BaseMacWindow *>::iterator stackIt;
	for (stackIt = _windowStack.begin(); stackIt != _windowStack.end(); stackIt++) {
		if (*stackIt == target) {
			stackIt = _windowStack.erase(stackIt);
			stackIt--;
		}
	}
}

void MacWindowManager::removeFromWindowList(BaseMacWindow *target) {
	int size = _windows.size();
	int ndx = 0;
	for (int i = 0; i < size; i++) {
		if (_windows[i] == target) {
			ndx = i;
		}
	}
	_windows.remove_at(ndx);
}

/////////////////
// Cursor stuff
/////////////////
void MacWindowManager::pushArrowCursor() {
	CursorMan.pushCursor(macCursorArrow, 11, 16, 1, 1, 3);
}

void MacWindowManager::pushBeamCursor() {
	CursorMan.pushCursor(macCursorBeam, 11, 16, 1, 1, 3);
}

void MacWindowManager::pushCrossHairCursor() {
	CursorMan.pushCursor(macCursorCrossHair, 11, 16, 1, 1, 3);
}

void MacWindowManager::pushCrossBarCursor() {
	CursorMan.pushCursor(macCursorCrossBar, 11, 16, 1, 1, 3);
}

void MacWindowManager::pushWatchCursor() {
	CursorMan.pushCursor(macCursorWatch, 11, 16, 1, 1, 3);
}

void MacWindowManager::popCursor() {
	CursorMan.popCursor();
}

///////////////////
// Palette stuff
///////////////////
void MacWindowManager::passPalette(const byte *pal, uint size) {
	const byte *p = pal;

	_colorWhite = -1;
	_colorBlack = -1;

	// Search pure white and black colors
	for (uint i = 0; i < size; i++) {
		if (_colorWhite == -1 && p[0] == 0xff && p[1] == 0xff && p[2] == 0xff)
			_colorWhite = i;


		if (_colorBlack == -1 && p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00)
			_colorBlack = i;

		p += 3;
	}

	if (_colorWhite != -1 && _colorBlack != -1)
		return;

	// We did not find some color. Let's find closest approximations
	float darkest = 1000.0f, brightest = -1.0f;
	int di = -1, bi = -1;
	p = pal;

	for (uint i = 0; i < size; i++) {
		float gray = p[0] * 0.3f + p[1] * 0.59f + p[2] * 0.11f;

		if (darkest > gray) {
			darkest = gray;
			di = i;
		}

		if (brightest < gray) {
			brightest = gray;
			bi = i;
		}

		p += 3;
	}

	_colorWhite = bi;
	_colorBlack = di;
}

void MacWindowManager::pauseEngine(bool pause) {
	if (_engine && _pauseEngineCallback) {
		_pauseEngineCallback(_engine, pause);
	}
}

void MacWindowManager::setEnginePauseCallback(void *engine, void (*pauseCallback)(void *, bool)) {
	_engine = engine;
	_pauseEngineCallback = pauseCallback;
}

} // End of namespace Graphics
