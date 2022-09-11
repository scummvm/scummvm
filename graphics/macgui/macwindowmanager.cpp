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
 */

#include "common/array.h"
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

#include "image/bmp.h"

namespace Graphics {

static const byte palette[] = {
	0, 0, 0,           // Black
	0x80, 0x80, 0x80,  // Gray80
	0x88, 0x88, 0x88,  // Gray88
	0xee, 0xee, 0xee,  // GrayEE
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

static const byte cursorPalette[] = {
	0, 0, 0,
	0xff, 0xff, 0xff
};

static const byte macCursorArrow[] = {
	1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	1, 0, 1, 3, 3, 3, 3, 3, 3, 3, 3,
	1, 0, 0, 1, 3, 3, 3, 3, 3, 3, 3,
	1, 0, 0, 0, 1, 3, 3, 3, 3, 3, 3,
	1, 0, 0, 0, 0, 1, 3, 3, 3, 3, 3,
	1, 0, 0, 0, 0, 0, 1, 3, 3, 3, 3,
	1, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3,
	1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 0, 0, 1, 0, 0, 1, 3, 3, 3, 3,
	1, 0, 1, 3, 1, 0, 0, 1, 3, 3, 3,
	1, 1, 3, 3, 1, 0, 0, 1, 3, 3, 3,
	1, 3, 3, 3, 3, 1, 0, 0, 1, 3, 3,
	3, 3, 3, 3, 3, 1, 0, 0, 1, 3, 3,
	3, 3, 3, 3, 3, 3, 1, 1, 1, 3, 3
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
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};
static const byte macCursorWatch[] = {
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 0, 1, 1, 1, 1, 1, 1, 0, 1, 3,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 3,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 3,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
	0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 3,
	0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 3,
	3, 0, 1, 1, 1, 1, 1, 1, 0, 1, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3,
};
static const byte macCursorCrossBar[] = {
	3, 3, 3, 0, 0, 0, 0, 3, 3, 3, 3,
	3, 3, 3, 0, 1, 1, 0, 0, 3, 3, 3,
	3, 3, 3, 0, 1, 1, 0, 0, 3, 3, 3,
	3, 3, 3, 0, 1, 1, 0, 0, 3, 3, 3,
	0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 3,
	0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
	0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	3, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	3, 3, 3, 0, 1, 1, 0, 0, 3, 3, 3,
	3, 3, 3, 0, 1, 1, 0, 0, 3, 3, 3,
	3, 3, 3, 0, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 3, 3, 0, 0, 0, 0, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

static void menuTimerHandler(void *refCon);

MacWindowManager::MacWindowManager(uint32 mode, MacPatterns *patterns, Common::Language language) {
	_screen = nullptr;
	_screenCopy = nullptr;
	_desktopBmp = nullptr;
	_desktop = nullptr;
	_lastId = 0;
	_activeWindow = -1;
	_needsRemoval = false;

	_activeWidget = nullptr;
	_mouseDown = false;
	_hoveredWidget = nullptr;

	_mode = 0;
	_language = language;

	_menu = 0;
	_menuDelay = 0;
	_menuTimerActive = false;

	_engineP = nullptr;
	_engineR = nullptr;
	_redrawEngineCallback = nullptr;
	_screenCopyPauseToken = nullptr;

	_colorBlack = kColorBlack;
	_colorGray80 = kColorGray80;
	_colorGray88 = kColorGray88;
	_colorGrayEE = kColorGrayEE;
	_colorWhite = kColorWhite;
	_colorGreen = kColorGreen;
	_colorGreen2 = kColorGreen2;

	_fullRefresh = true;
	_inEditableArea = false;

	_hilitingWidget = false;

	if (mode & kWMMode32bpp)
		_pixelformat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	else
		_pixelformat = PixelFormat::createFormatCLUT8();

	if (patterns) {
		_patterns = *patterns;
	} else {
		for (int i = 0; i < ARRAYSIZE(fillPatterns); i++)
			_patterns.push_back(fillPatterns[i]);
	}

	// builtin pattern
	for (int i = 0; i < ARRAYSIZE(fillPatterns); i++)
		_builtinPatterns.push_back(fillPatterns[i]);

	g_system->getPaletteManager()->setPalette(palette, 0, ARRAYSIZE(palette) / 3);

	_paletteSize = ARRAYSIZE(palette) / 3;
	if (_paletteSize) {
		_palette = (byte *)malloc(_paletteSize * 3);
		memcpy(_palette, palette, _paletteSize * 3);
	}
	_paletteLookup.setPalette(_palette, _paletteSize);

	_fontMan = new MacFontManager(mode, language);

	_cursor = nullptr;
	_tempType = kMacCursorArrow;
	replaceCursor(kMacCursorArrow);
	CursorMan.showMouse(true);

	loadDataBundle();
	setDesktopMode(mode);
}

MacWindowManager::~MacWindowManager() {
	for (Common::HashMap<uint, BaseMacWindow *>::iterator it = _windows.begin(); it != _windows.end(); it++)
		delete it->_value;

	if (_palette)
		free(_palette);

	delete _fontMan;
	delete _screenCopy;

	if (_desktopBmp) {
		_desktopBmp->free();
		delete _desktopBmp;
	}
	delete _desktop;

	cleanupDataBundle();

	g_system->getTimerManager()->removeTimerProc(&menuTimerHandler);
}

void MacWindowManager::setDesktopMode(uint32 mode) {
	if (!(mode & Graphics::kWMNoScummVMWallpaper)) {
		if (!_mode || (_mode & Graphics::kWMNoScummVMWallpaper))
			loadDesktop();
	} else if (_desktopBmp) {
		_desktopBmp->free();
		_desktopBmp = nullptr;
	}

	_mode = mode;
}

void MacWindowManager::setScreen(ManagedSurface *screen) {
	_screen = screen;
	delete _screenCopy;
	_screenCopy = nullptr;

	if (_desktop)
		_desktop->free();
	else
		_desktop = new ManagedSurface();

	_desktop->create(_screen->w, _screen->h, _pixelformat);
	drawDesktop();
}

void MacWindowManager::setScreen(int w, int h) {
	if (_desktop)
		_desktop->free();
	else
		_desktop = new ManagedSurface();

	_screenDims = Common::Rect(w, h);
	_desktop->create(w, h, _pixelformat);
	drawDesktop();
}

int MacWindowManager::getWidth() {
	return _screenDims.width();
}

int MacWindowManager::getHeight() {
	return _screenDims.height();
}

void MacWindowManager::resizeScreen(int w, int h) {
	if (!_screen)
		error("MacWindowManager::resizeScreen(): Trying to creating surface on non-existing screen");
	_screenDims = Common::Rect(w, h);
	_screen->free();
	_screen->create(w, h, _pixelformat);
}

void MacWindowManager::setMode(uint32 mode) {
	_mode = mode;

	if (mode & kWMModeForceBuiltinFonts)
		_fontMan->forceBuiltinFonts();
}

void MacWindowManager::clearHandlingWidgets() {
	// pass a LBUTTONUP event to those widgets should clear those state
	Common::Event event;
	event.type = Common::EVENT_LBUTTONUP;
	event.mouse = _lastClickPos;
	processEvent(event);

	setActiveWidget(nullptr);
	_hoveredWidget = nullptr;
}

void MacWindowManager::setActiveWidget(MacWidget *widget) {
	if (_activeWidget == widget)
		return;

	if (_activeWidget)
		_activeWidget->setActive(false);

	_activeWidget = widget;

	if (_activeWidget)
		_activeWidget->setActive(true);
}

void MacWindowManager::clearWidgetRefs(MacWidget *widget) {
	if (widget == _hoveredWidget)
		_hoveredWidget = nullptr;

	if (widget == _activeWidget)
		_activeWidget = nullptr;
}

MacWindow *MacWindowManager::addWindow(bool scrollable, bool resizable, bool editable) {
	MacWindow *w = new MacWindow(_lastId, scrollable, resizable, editable, this);

	addWindowInitialized(w);

	setActiveWindow(getNextId());

	return w;
}

MacTextWindow *MacWindowManager::addTextWindow(const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler) {
	MacTextWindow *w = new MacTextWindow(this, font, fgcolor, bgcolor, maxWidth, textAlignment, menu, cursorHandler);

	addWindowInitialized(w);

	setActiveWindow(getNextId());

	return w;
}

MacTextWindow *MacWindowManager::addTextWindow(const Font *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler) {
	MacTextWindow *w = new MacTextWindow(this, font, fgcolor, bgcolor, maxWidth, textAlignment, menu, cursorHandler);

	addWindowInitialized(w);

	setActiveWindow(getNextId());

	return w;
}


void MacWindowManager::addWindowInitialized(MacWindow *macwindow) {
	_windows[macwindow->getId()] = macwindow;
	_windowStack.push_back(macwindow);
}

MacMenu *MacWindowManager::addMenu() {
	if (_menu) {
		_windows[_menu->getId()] = nullptr;
		delete _menu;
	}

	_menu = new MacMenu(getNextId(), getScreenBounds(), this);

	_windows[_menu->getId()] = _menu;

	return _menu;
}

MacMenu *MacWindowManager::getMenu() {
	if (_menu) {
		return _menu;
	}
	return nullptr;
}

void MacWindowManager::removeMenu() {
	if (_menu) {
		_windows[_menu->getId()] = nullptr;
		delete _menu;
		_menu = nullptr;
	}
}

void MacWindowManager::activateMenu() {
	if (!_menu || ((_mode & kWMModeAutohideMenu) && _menu->isVisible()))
		return;

	if (_mode & kWMModalMenuMode) {
		activateScreenCopy();
	}

	_menu->setVisible(true);
}

void MacWindowManager::activateScreenCopy() {
	if (_screen) {
		if (!_screenCopy)
			_screenCopy = new ManagedSurface(*_screen);	// Create a copy
		else
			*_screenCopy = *_screen;
	} else {
		Surface *surface = g_system->lockScreen();

		if (!_screenCopy) {
			_screenCopy = new ManagedSurface(_screenDims.width(), _screenDims.height());
		}

		_screenCopy->blitFrom(*surface);
		g_system->unlockScreen();
	}

	_screenCopyPauseToken = new PauseToken(pauseEngine());
}

void MacWindowManager::disableScreenCopy() {
	if (_screenCopyPauseToken) {
		_screenCopyPauseToken->clear();
		delete _screenCopyPauseToken;
		_screenCopyPauseToken = nullptr;
	}

	// add a check, we may not get the _screenCopy because we may not activate the menu
	if (!_screenCopy)
		return;

	if (_screen)
		*_screen = *_screenCopy; // restore screen

	g_system->copyRectToScreen(_screenCopy->getBasePtr(0, 0), _screenCopy->pitch, 0, 0, _screenCopy->w, _screenCopy->h);
}

void MacWindowManager::setMenuItemCheckMark(MacMenuItem *menuItem, bool checkMark) {
	if (_menu) {
		_menu->setCheckMark(menuItem, checkMark);
	} else {
		warning("MacWindowManager::setMenuItemCheckMark: wm doesn't have menu");
	}
}

void MacWindowManager::setMenuItemEnabled(MacMenuItem *menuItem, bool enabled) {
	if (_menu) {
		_menu->setEnabled(menuItem, enabled);
	} else {
		warning("MacWindowManager::setMenuItemEnabled: wm doesn't have menu");
	}
}

void MacWindowManager::setMenuItemName(MacMenuItem *menuItem, const Common::String &name) {
	if (_menu) {
		_menu->setName(menuItem, name);
	} else {
		warning("MacWindowManager::setMenuItemName: wm doesn't have menu");
	}
}

void MacWindowManager::setMenuItemAction(MacMenuItem *menuItem, int actionId) {
	if (_menu) {
		_menu->setAction(menuItem, actionId);
	} else {
		warning("MacWindowManager::setMenuItemAction: wm doesn't have menu");
	}
}

bool MacWindowManager::getMenuItemCheckMark(MacMenuItem *menuItem) {
	if (_menu) {
		return _menu->getCheckMark(menuItem);
	} else {
		warning("MacWindowManager::getMenuItemCheckMark: wm doesn't have menu");
		return false;
	}
}

bool MacWindowManager::getMenuItemEnabled(MacMenuItem *menuItem) {
	if (_menu) {
		return _menu->getEnabled(menuItem);
	} else {
		warning("MacWindowManager::getMenuItemEnabled: wm doesn't have menu");
		return false;
	}
}

Common::String MacWindowManager::getMenuItemName(MacMenuItem *menuItem) {
	if (_menu) {
		return _menu->getName(menuItem);
	} else {
		warning("MacWindowManager::getMenuItemName: wm doesn't have menu");
		return Common::String();
	}
}

int MacWindowManager::getMenuItemAction(MacMenuItem *menuItem) {
	if (_menu) {
		return _menu->getAction(menuItem);
	} else {
		warning("MacWindowManager::getMenuItemAction: wm doesn't have menu");
		return 0;
	}
}

// this is refer to how we deal U32String in splitString in mactext
// maybe we can optimize this specifically
Common::U32String stripFormat(const Common::U32String &str) {
	Common::U32String res, paragraph, tmp;
	// calc the size of str
	const Common::U32String::value_type *l = str.c_str();
	while (*l) {
		// split paragraph first
		paragraph.clear();
		while (*l) {
			if (*l == '\r') {
				l++;
				if (*l == '\n')
					l++;
				break;
			}
			if (*l == '\n') {
				l++;
				break;
			}
			paragraph += *l++;
		}
		const Common::U32String::value_type *s = paragraph.c_str();
		tmp.clear();
		while (*s) {
			if (*s == '\001') {
				s++;
				// if there are two \001, then we regard it as one character
				if (*s == '\001') {
					tmp += *s++;
				}
			} else if (*s == '\015') {	// binary format
				// we are skipping the formatting stuffs
				// this number 12, and the number 23, is the size of our format
				s += 12;
			} else if (*s == '\016') {	// human-readable format
				s += 23;
			} else {
				tmp += *s++;
			}
		}
		res += tmp;
		if (*l)
			res += '\n';
	}
	return res;
}

void MacWindowManager::setTextInClipboard(const Common::U32String &str) {
	_clipboard = str;
	g_system->setTextInClipboard(stripFormat(str));
}

// get the text size ignoring \n
int getPureTextSize(const Common::U32String &str, bool global) {
	const Common::U32String::value_type *l = str.c_str();
	int res = 0;
	if (global) {
		// if we are in global, then we have no format in str. thus, we ignore all \r \n
		while (*l) {
			if (*l != '\n' && *l != '\r')
				res++;
			l++;
		}
	} else {
		// if we are not in global, then we are using the wm clipboard, which use \n for new line
		// i think that if statement can be optimized to, like if (*l != '\n' && (!global || *l != '\r'))
		// but for the sake of readability, we keep codes here
		while (*l) {
			if (*l != '\n')
				res++;
			l++;
		}
	}
	return res;
}

Common::U32String MacWindowManager::getTextFromClipboard(const Common::U32String &format, int *size) {
	Common::U32String global_str = g_system->getTextFromClipboard();
	// str is what we need
	Common::U32String str;
	if (_clipboard.empty()) {
		// if wm clipboard is empty, then we use the global clipboard, which won't contain the format
		str = format + global_str;
		if (size)
			*size = getPureTextSize(global_str, true);
	} else {
		Common::U32String tmp = stripFormat(_clipboard);
		if (tmp == global_str) {
			// if the text is equal, then we use wm one which contains the format
			str = _clipboard;
			if (size)
				*size = getPureTextSize(tmp, false);
		} else {
			// otherwise, we prefer the global one
			str = format + global_str;
			if (size)
				*size = getPureTextSize(global_str, true);
		}
	}
	return str;
}

bool MacWindowManager::isMenuActive() {
	if (!_menu)
		return false;

	return _menu->isVisible();
}

void MacWindowManager::setActiveWindow(int id) {
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

MacWindow *MacWindowManager::findWindowAtPoint(int16 x, int16 y) {
	Common::List<Graphics::BaseMacWindow *>::iterator it;
	Graphics::MacWindow *win = nullptr;

	for (it = _windowStack.begin(); it != _windowStack.end(); it++) {
		if ((*it)->getDimensions().contains(x, y)) {
			win = reinterpret_cast<Graphics::MacWindow *>(*it);
		}
	}

	return win;
}

MacWindow *MacWindowManager::findWindowAtPoint(Common::Point point) {
	Common::List<Graphics::BaseMacWindow *>::iterator it;
	Graphics::MacWindow *win = nullptr;

	for (it = _windowStack.begin(); it != _windowStack.end(); it++) {
		if ((*it)->getDimensions().contains(point)) {
			win = reinterpret_cast<Graphics::MacWindow *>(*it);
		}
	}

	return win;
}

void MacWindowManager::removeWindow(MacWindow *target) {
	_windowsToRemove.push_back(target);
	_needsRemoval = true;
	_hoveredWidget = nullptr;

	if (target->getId() == _activeWindow)
		_activeWindow = -1;
}

template<typename T>
void macDrawPixel(int x, int y, int color, void *data) {
	MacPlotData *p = (MacPlotData *)data;

	if (p->fillType > p->patterns->size() || !p->fillType)
		return;

	byte *pat = p->patterns->operator[](p->fillType - 1);

	if (p->thickness == 1) {
		if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
			uint xu = (uint)x; // for letting compiler optimize it
			uint yu = (uint)y;

			*((T)p->surface->getBasePtr(xu, yu)) = p->invert ? ~(*((T)p->surface->getBasePtr(xu, yu))) :
				(pat[(yu + p->fillOriginY) % 8] & (1 << (7 - (xu + p->fillOriginX) % 8))) ? color : p->bgColor;

			if (p->mask)
				*((T)p->mask->getBasePtr(xu, yu)) = 0xff;
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
					*((T)p->surface->getBasePtr(xu, yu)) = p->invert ? ~(*((T)p->surface->getBasePtr(xu, yu))) :
						(pat[(yu + p->fillOriginY) % 8] & (1 << (7 + (xu - p->fillOriginX) % 8))) ? color : p->bgColor;

					if (p->mask)
						*((T)p->mask->getBasePtr(xu, yu)) = 0xff;
				}
	}
}

void macDrawInvertPixel(int x, int y, int color, void *data) {
	MacPlotData *p = (MacPlotData *)data;

	if (p->fillType > p->patterns->size() || !p->fillType)
		return;

	if (x >= 0 && x < p->surface->w && y >= 0 && y < p->surface->h) {
		uint xu = (uint)x; // for letting compiler optimize it
		uint yu = (uint)y;

		byte cur_color = *((byte *)p->surface->getBasePtr(xu, yu));
		// 0 represent black in default palette, and 4 represent white
		// if color is black, we invert it to white, otherwise, we invert it to black
		byte invert_color = 0;
		if (cur_color == 0) {
			invert_color = 4;
		}
		*((byte *)p->surface->getBasePtr(xu, yu)) = invert_color;

		if (p->mask)
			*((byte *)p->mask->getBasePtr(xu, yu)) = 0xff;
	}
}

MacDrawPixPtr MacWindowManager::getDrawPixel() {
	if (_pixelformat.bytesPerPixel == 1)
		return &macDrawPixel<byte *>;
	else
		return &macDrawPixel<uint32 *>;
}

// get the function of drawing invert pixel for default palette
MacDrawPixPtr MacWindowManager::getDrawInvertPixel() {
	if (_pixelformat.bytesPerPixel == 1)
		return &macDrawInvertPixel;
	warning("function of drawing invert pixel for default palette has not implemented yet");
	return nullptr;
}

void MacWindowManager::loadDesktop() {
	Common::SeekableReadStream *file = getFile("scummvm_background.bmp");
	if (!file)
		return;

	Image::BitmapDecoder bmpDecoder;
	Graphics::Surface *source;
	_desktopBmp = new Graphics::TransparentSurface();

	bmpDecoder.loadStream(*file);
	source = bmpDecoder.getSurface()->convertTo(_desktopBmp->getSupportedPixelFormat(), bmpDecoder.getPalette());

	_desktopBmp->copyFrom(*source);

	delete file;
	source->free();
	delete source;
}

void MacWindowManager::setDesktopColor(byte r, byte g, byte b) {
	_desktopBmp = new Graphics::TransparentSurface();
	uint32 color = TS_RGB(r, g, b);

	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::ManagedSurface *source = new Graphics::ManagedSurface();
	source->create(10, 10, requiredFormat_4byte);
	Common::Rect area = source->getBounds();
	source->fillRect(area, color);

	_desktopBmp->copyFrom(*source);
	source->free();
	delete source;
}

void MacWindowManager::drawDesktop() {
	if (_desktopBmp) {
		for (int i = 0; i < _desktop->w; ++i) {
			for (int j = 0; j < _desktop->h; ++j) {
				uint32 color = *(uint32 *)_desktopBmp->getBasePtr(i % _desktopBmp->w, j % _desktopBmp->h);
				if (_pixelformat.bytesPerPixel == 1) {
					byte r, g, b;
					_desktopBmp->format.colorToRGB(color, r, g, b);
					if (color > 0) {
						*((byte *)_desktop->getBasePtr(i, j)) = findBestColor(r, g, b);
					}
				} else {
					*((uint32 *)_desktop->getBasePtr(i, j)) = color;
				}
			}
		}
	} else {
		Common::Rect r(_desktop->getBounds());

		MacPlotData pd(_desktop, nullptr, &_patterns, kPatternCheckers, 0, 0, 1, _colorWhite);

		Graphics::drawRoundRect(r, kDesktopArc, _colorBlack, true, getDrawPixel(), &pd);
	}
}

void MacWindowManager::draw() {
	removeMarked();

	Common::Rect bounds = getScreenBounds();

	if (_fullRefresh) {
		if (!(_mode & kWMModeNoDesktop)) {
			Common::Rect screen = getScreenBounds();
			if (_desktop->w != screen.width() || _desktop->h != screen.height()) {
				_desktop->free();
				_desktop->create(screen.width(), screen.height(), _pixelformat);
				drawDesktop();
			}

			if (_screen) {
				_screen->blitFrom(*_desktop, Common::Point(0, 0));
				g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _screen->w, _screen->h);
			} else {
				_screenCopyPauseToken = new PauseToken(pauseEngine());
				g_system->copyRectToScreen(_desktop->getPixels(), _desktop->pitch, 0, 0, _desktop->w, _desktop->h);
			}
		}
		if (_redrawEngineCallback != nullptr)
			_redrawEngineCallback(_engineR);
	}

	Common::Array<Common::Rect> dirtyRects;
	for (Common::List<BaseMacWindow *>::const_iterator it = _windowStack.begin(); it != _windowStack.end(); it++) {
		BaseMacWindow *w = *it;
		if (!w->isVisible())
			continue;

		Common::Rect clip = w->getInnerDimensions();
		clip.clip(bounds);

		if (clip.isEmpty())
			continue;

		clip = w->getDimensions();
		clip.clip(bounds);

		if (clip.isEmpty())
			continue;

		bool forceRedraw = _fullRefresh;
		if (!forceRedraw && dirtyRects.size()) {
			for (Common::Array<Common::Rect>::iterator dirty = dirtyRects.begin(); dirty != dirtyRects.end(); dirty++) {
				if (clip.intersects(*dirty)) {
					forceRedraw = true;
					break;
				}
			}
		}

		if (!_screen) {
			if (w->isDirty() || forceRedraw) {
				w->draw(forceRedraw);

				Common::Rect outerDims = w->getDimensions();
				Common::Rect innerDims = w->getInnerDimensions();
				int adjWidth, adjHeight;

				if (w->isDirty() || forceRedraw) {
					w->draw(forceRedraw);

					adjustDimensions(clip, outerDims, adjWidth, adjHeight);

					if (_pixelformat.bytesPerPixel == 1) {
						Surface *surface = g_system->lockScreen();
						ManagedSurface *border = w->getBorderSurface();

						for (int y = 0; y < adjHeight; y++) {
							const byte *src = (const byte *)border->getBasePtr(clip.left - outerDims.left, y);
							byte *dst = (byte *)surface->getBasePtr(clip.left, y + clip.top);
							for (int x = 0; x < adjWidth; x++, src++, dst++)
									if (*src != _colorGreen2 && *src != _colorGreen)
										*dst = *src;
						}

						g_system->unlockScreen();
					} else {
						g_system->copyRectToScreen(w->getBorderSurface()->getBasePtr(MAX(clip.left - outerDims.left, 0), MAX(clip.top - outerDims.top, 0)), w->getBorderSurface()->pitch, clip.left, clip.top, adjWidth, adjHeight);
					}
				}

				adjustDimensions(clip, innerDims, adjWidth, adjHeight);
				g_system->copyRectToScreen(w->getWindowSurface()->getBasePtr(MAX(clip.left - innerDims.left, 0), MAX(clip.top - innerDims.top, 0)), w->getWindowSurface()->pitch,MAX(innerDims.left, (int16)0), MAX(innerDims.top, (int16)0), adjWidth, adjHeight);

				dirtyRects.push_back(clip);
			}

			if (_screenCopyPauseToken) {
				_screenCopyPauseToken->clear();
				delete _screenCopyPauseToken;
				_screenCopyPauseToken = nullptr;
			}
		} else if (w->draw(_screen, forceRedraw)) {
			w->setDirty(false);
			g_system->copyRectToScreen(_screen->getBasePtr(clip.left, clip.top), _screen->pitch, clip.left, clip.top, clip.width(), clip.height());
			dirtyRects.push_back(clip);
		}
	}

	// Menu is drawn on top of everything and always
	if (_menu && !(_mode & kWMModeFullscreen)) {
		if (_fullRefresh)
			_menu->draw(_screen, _fullRefresh);
		else {
			// add intersection check with menu
			bool menuRedraw = false;
			for (Common::Array<Common::Rect>::iterator dirty = dirtyRects.begin(); dirty != dirtyRects.end(); dirty++) {
				if (_menu->checkIntersects(*dirty)) {
					menuRedraw = true;
					break;
				}
			}
			_menu->draw(_screen, menuRedraw);
		}
	}

	_fullRefresh = false;
}

static void menuTimerHandler(void *refCon) {
	MacWindowManager *wm = (MacWindowManager *)refCon;

	if (wm->_menuHotzone.contains(wm->_lastMousePos)) {
		wm->activateMenu();
	}

	wm->_menuTimerActive = false;

	g_system->getTimerManager()->removeTimerProc(&menuTimerHandler);
}

bool MacWindowManager::processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		_lastMousePos = event.mouse;
		break;
	case Common::EVENT_LBUTTONDOWN:
		_mouseDown = true;
		_lastClickPos = event.mouse;
		break;
	case Common::EVENT_LBUTTONUP:
		_mouseDown = false;
		break;
	default:
		break;
	}

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
		if ((_windows[_activeWindow]->isEditable() && _windows[_activeWindow]->getType() == kWindowWindow &&
				 ((MacWindow *)_windows[_activeWindow])->getInnerDimensions().contains(event.mouse.x, event.mouse.y)) ||
				(_activeWidget && _activeWidget->isEditable() &&
				 _activeWidget->getDimensions().contains(event.mouse.x, event.mouse.y))) {
			if (getCursorType() != kMacCursorBeam) {
				_tempType = getCursorType();
				_inEditableArea = true;
				replaceCursor(kMacCursorBeam);
			}
		} else {
			// here, we use _inEditableArea is distinguish whether the current Beam cursor is set by director or ourself
			// if we are not in the editable area but we are drawing the Beam cursor, then the cursor is set by director, thus we don't replace it
			if (getCursorType() == kMacCursorBeam && _inEditableArea) {
				replaceCursor(_tempType, _cursor);
				_inEditableArea = false;
			}
		}
	}

	for (Common::List<BaseMacWindow *>::const_iterator it = _windowStack.end(); it != _windowStack.begin();) {
		it--;
		BaseMacWindow *w = *it;

		if (w->hasAllFocus() || (w->isEditable() && event.type == Common::EVENT_KEYDOWN) ||
				w->getDimensions().contains(event.mouse.x, event.mouse.y)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP)
				setActiveWindow(w->getId());

			return w->processEvent(event);
		}
	}

	return false;
}

void MacWindowManager::adjustDimensions(const Common::Rect &clip, const Common::Rect &dims, int &adjWidth, int &adjHeight) {
	int wOffset, hOffset;

	wOffset = clip.left - dims.left;
	adjWidth = dims.width();
	if (wOffset > 0) {
		adjWidth -= wOffset;
	} else if (dims.right > getScreenBounds().right) {
		adjWidth -= (dims.right - getScreenBounds().right);
	}

	hOffset = clip.top - dims.top;
	adjHeight = dims.height();
	if (hOffset > 0) {
		adjHeight -= hOffset;
	} else if (dims.bottom > getScreenBounds().bottom) {
		adjHeight -= (dims.bottom - getScreenBounds().bottom);
	}
}

void MacWindowManager::removeMarked() {
	if (!_needsRemoval) return;

	Common::List<BaseMacWindow *>::const_iterator it;
	for (it = _windowsToRemove.begin(); it != _windowsToRemove.end(); it++) {
		removeFromStack(*it);
		removeFromWindowList(*it);
		delete *it;
		_activeWindow = -1;
		_fullRefresh = true;
	}
	_windowsToRemove.clear();
	_needsRemoval = false;

	// Do we need compact lastid?
	_lastId = 0;
	for (Common::HashMap<uint, BaseMacWindow *>::iterator lit = _windows.begin(); lit != _windows.end(); lit++) {
		if (lit->_key >= (uint)_lastId)
			_lastId = lit->_key + 1;
	}
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
	// _windows.erase(target->getId()); // Is applicable?
	for (Common::HashMap<uint, BaseMacWindow *>::iterator it = _windows.begin(); it != _windows.end(); it++) {
		if (it->_value == target) {
			_windows.erase(it);
			break;
		}
	}
}


void MacWindowManager::addZoomBox(ZoomBox *box) {
	_zoomBoxes.push_back(box);
}

void MacWindowManager::renderZoomBox(bool redraw) {
	if (!_zoomBoxes.size())
		return;

	ZoomBox *box = _zoomBoxes.front();
	uint32 t = g_system->getMillis();

	MacPlotData pd(_screen, nullptr, &getPatterns(), Graphics::kPatternCheckers, 0, 0, 1, 0, true);

	// Undraw the previous boxes
	if (box->last.size() != 0) {
		for (uint i = 0; i < box->last.size(); i++) {
			Common::Rect r = box->last.remove_at(i);
			zoomBoxInner(r, pd);
		}
	}

	if (box->nextTime > t)
		return;

	const int numSteps = 14;
	// We have 15 steps in total, and we have flying rectange
	// from switching 3/4 frames

	int start, end;
	// Determine, how many rectangles and what are their numbers
	if (box->step <= 5) {
		start = 1;
		end = box->step - 1;
	} else {
		start = box->step - 4;
		end = MIN(start + 3 - box->step % 2, 7);
	}

	for (int i = start; i <= end; i++) {
		Common::Rect r(box->start.left   + (box->end.left   - box->start.left)   * i / 8,
					   box->start.top    + (box->end.top    - box->start.top)    * i / 8,
					   box->start.right  + (box->end.right  - box->start.right)  * i / 8,
					   box->start.bottom + (box->end.bottom - box->start.bottom) * i / 8);

		zoomBoxInner(r, pd);
		box->last.push_back(r);
	}

	box->step++;
	box->nextTime = box->startTime + 1000 * box->step * box->delay / 60;

	if (redraw) {
		g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _screen->getBounds().width(), _screen->getBounds().height()); // zoomBox
	}

	if (box->step >= numSteps) {
		delete _zoomBoxes[0];
		_zoomBoxes.remove_at(0);
	}
}

void MacWindowManager::zoomBoxInner(Common::Rect &r, Graphics::MacPlotData &pd) {
	Graphics::drawLine(r.left,  r.top,    r.right, r.top,    0xff, getDrawPixel(), &pd);
	Graphics::drawLine(r.right, r.top,    r.right, r.bottom, 0xff, getDrawPixel(), &pd);
	Graphics::drawLine(r.left,  r.bottom, r.right, r.bottom, 0xff, getDrawPixel(), &pd);
	Graphics::drawLine(r.left,  r.top,    r.left,  r.bottom, 0xff, getDrawPixel(), &pd);
}

/////////////////
// Cursor stuff
/////////////////
void MacWindowManager::replaceCursorType(MacCursorType type) {
	if (_cursorTypeStack.empty())
		_cursorTypeStack.push(type);
	else
		_cursorTypeStack.top() = type;
}

MacCursorType MacWindowManager::getCursorType() const {
	if (_cursorTypeStack.empty())
		return kMacCursorOff;

	return _cursorTypeStack.top();
}

void MacWindowManager::pushCursor(MacCursorType type, Cursor *cursor) {
	switch (type) {
	case kMacCursorOff:
		CursorMan.pushCursor(nullptr, 0, 0, 0, 0, 0);
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorArrow:
		CursorMan.pushCursor(macCursorArrow, 11, 16, 1, 1, 3);
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorBeam:
		CursorMan.pushCursor(macCursorBeam, 11, 16, 1, 1, 3);
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorCrossHair:
		CursorMan.pushCursor(macCursorCrossHair, 11, 16, 1, 1, 3);
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorCrossBar:
		CursorMan.pushCursor(macCursorCrossBar, 11, 16, 1, 1, 3);
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorWatch:
		CursorMan.pushCursor(macCursorWatch, 11, 16, 1, 1, 3);
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorCustom:
		if (!cursor) {
			warning("MacWindowManager::pushCursor(): Custom cursor signified but not provided");
			return;
		}

		pushCustomCursor(cursor);
	}

	_cursorTypeStack.push(type);
}

void MacWindowManager::replaceCursor(MacCursorType type, Cursor *cursor) {
	switch (type) {
	case kMacCursorOff:
		CursorMan.replaceCursor(nullptr, 0, 0, 0, 0, 0);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorArrow:
		CursorMan.replaceCursor(macCursorArrow, 11, 16, 1, 1, 3);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorBeam:
		CursorMan.replaceCursor(macCursorBeam, 11, 16, 1, 1, 3);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorCrossHair:
		CursorMan.replaceCursor(macCursorCrossHair, 11, 16, 1, 1, 3);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorCrossBar:
		CursorMan.replaceCursor(macCursorCrossBar, 11, 16, 1, 1, 3);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorWatch:
		CursorMan.replaceCursor(macCursorWatch, 11, 16, 1, 1, 3);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		break;
	case kMacCursorCustom:
		if (!cursor) {
			warning("MacWindowManager::replaceCursor(): Custom cursor signified but not provided");
			return;
		}

		CursorMan.replaceCursor(cursor);
		break;
	}

	replaceCursorType(type);
}

void MacWindowManager::pushCustomCursor(const byte *data, int w, int h, int hx, int hy, int transcolor) {
	CursorMan.pushCursor(data, w, h, hx, hy, transcolor);
	CursorMan.pushCursorPalette(cursorPalette, 0, 2);
	_cursorTypeStack.push(kMacCursorCustom);
}

void MacWindowManager::replaceCustomCursor(const byte *data, int w, int h, int hx, int hy, int transcolor) {
	CursorMan.replaceCursor(data, w, h, hx, hy, transcolor);
	CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
	replaceCursorType(kMacCursorCustom);
}

void MacWindowManager::pushCustomCursor(const Graphics::Cursor *cursor) {
	CursorMan.pushCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(),
	                     cursor->getHotspotY(), cursor->getKeyColor());

	if (cursor->getPalette())
		CursorMan.pushCursorPalette(cursor->getPalette(), cursor->getPaletteStartIndex(), cursor->getPaletteCount());
	else
		CursorMan.pushCursorPalette(cursorPalette, 0, 2);

	_cursorTypeStack.push(kMacCursorCustom);
}

void MacWindowManager::popCursor() {
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
	_cursorTypeStack.pop();
}

///////////////////
// Palette stuff
///////////////////
#define LOOKUPCOLOR(x) _color ## x = findBestColor(palette[kColor ## x * 3], palette[kColor ## x  * 3 + 1], palette[kColor ## x * 3 + 2]);

void MacWindowManager::passPalette(const byte *pal, uint size) {
	if (_palette)
		free(_palette);

	if (size) {
		_palette = (byte *)malloc(size * 3);
		memcpy(_palette, pal, size * 3);
	}
	_paletteSize = size;

	_paletteLookup.setPalette(pal, size);

	LOOKUPCOLOR(White);
	LOOKUPCOLOR(Gray80);
	LOOKUPCOLOR(Gray88);
	LOOKUPCOLOR(GrayEE);
	LOOKUPCOLOR(Black);
	LOOKUPCOLOR(Green);
	LOOKUPCOLOR(Green2);

	drawDesktop();
	setFullRefresh(true);
}

uint32 MacWindowManager::findBestColor(byte cr, byte cg, byte cb) {
	if (_pixelformat.bytesPerPixel == 4)
		return _pixelformat.RGBToColor(cr, cg, cb);

	return _paletteLookup.findBestColor(cr, cg, cb);
}

template <>
void MacWindowManager::decomposeColor<uint32>(uint32 color, byte &r, byte &g, byte &b) {
	_pixelformat.colorToRGB(color, r, g, b);
}

template <>
void MacWindowManager::decomposeColor<byte>(uint32 color, byte& r, byte& g, byte& b) {
	r = *(_palette + 3 * (byte)color + 0);
	g = *(_palette + 3 * (byte)color + 1);
	b = *(_palette + 3 * (byte)color + 2);
}

uint32 MacWindowManager::findBestColor(uint32 color) {
	if (_pixelformat.bytesPerPixel == 4)
		return color;

	byte r, g, b;
	decomposeColor<byte>(color, r, g, b);
	return _paletteLookup.findBestColor(r, g, b);
}

byte MacWindowManager::inverter(byte src) {
	if (_invertColorHash.contains(src))
		return _invertColorHash[src];

	if (_pixelformat.bytesPerPixel == 1) {
		byte r, g, b;
		decomposeColor<byte>(src, r, g, b);
		r = ~r;
		g = ~g;
		b = ~b;
		_invertColorHash[src] = findBestColor(r, g, b);
	} else {
		uint32 alpha = _pixelformat.ARGBToColor(255, 0, 0, 0);
		_invertColorHash[src] = ~(src & ~alpha) | alpha;
	}
	return _invertColorHash[src];
}

PauseToken MacWindowManager::pauseEngine() {
	return _engineP->pauseEngine();
}

void MacWindowManager::setEngine(Engine *engine) {
	_engineP = engine;
}

void MacWindowManager::setEngineRedrawCallback(void *engine, void (*redrawCallback)(void *)) {
	_engineR = engine;
	_redrawEngineCallback = redrawCallback;
}

void MacWindowManager::printWMMode(int debuglevel) {
	Common::String out;

	if (_mode & kWMModeNoDesktop)
		out += "kWMModeNoDesktop";
	else
		out += "!kWMModeNoDesktop";

	if (_mode & kWMModeAutohideMenu)
		out += " kWMModeAutohideMenu";

	if (_mode & kWMModalMenuMode)
		out += " kWMModalMenuMode";

	if (_mode & kWMModeForceBuiltinFonts)
		out += " kWMModeForceBuiltinFonts";

	if (_mode & kWMModeUnicode)
		out += " kWMModeUnicode";

	if (_mode & kWMModeManualDrawWidgets)
		out += " kWMModeManualDrawWidgets";

	if (_mode & kWMModeFullscreen)
		out += " kWMModeFullscreen";
	else
		out += " !kWMModeFullscreen";

	if (_mode & kWMModeButtonDialogStyle)
		out += " kWMModeButtonDialogStyle";

	if (_mode & kWMMode32bpp)
		out += " kWMMode32bpp";
	else
		out += " !kWMMode32bpp";

	if (_mode & kWMNoScummVMWallpaper)
		out += " kWMNoScummVMWallpaper";

	if (_mode & kWMModeWin95)
		out += " kWMModeWin95";

	debug(debuglevel, "WM mode: %s", out.c_str());
}

} // End of namespace Graphics
