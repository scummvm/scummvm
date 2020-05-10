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
 */

#ifndef GRAPHICS_MACGUI_MACWINDOWMANAGER_H
#define GRAPHICS_MACGUI_MACWINDOWMANAGER_H

#include "common/hashmap.h"
#include "common/list.h"
#include "common/events.h"

#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/macgui/macwindow.h"

#include "engines/engine.h"

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
	kColorGreen2 = 4,
	kColorCount
};

enum {
	kPatternSolid = 1,
	kPatternStripes = 2,
	kPatternCheckers = 3,
	kPatternCheckers2 = 4,
	kPatternLightGray = 5,
	kPatternDarkGray = 6
};

enum {
	kWMModeNone         	= 0,
	kWMModeNoDesktop    	= (1 << 0),
	kWMModeAutohideMenu 	= (1 << 1),
	kWMModalMenuMode 		= (1 << 2),
	kWMModeForceBuiltinFonts= (1 << 3),
	kWMModeUnicode			= (1 << 4),
	kWMModeManualDrawWidgets= (1 << 5)
};

}
using namespace MacGUIConstants;

class Cursor;

class ManagedSurface;

class MacMenu;
class MacTextWindow;
class MacWidget;

class MacFont;

class MacFontManager;

typedef Common::Array<byte *> MacPatterns;

struct MacPlotData {
	Graphics::ManagedSurface *surface;
	Graphics::ManagedSurface *mask;
	MacPatterns *patterns;
	uint fillType;
	int fillOriginX;
	int fillOriginY;
	int thickness;
	uint bgColor;

	MacPlotData(Graphics::ManagedSurface *s, Graphics::ManagedSurface *m, MacPatterns *p, uint f, int fx, int fy, int t, uint bg) :
		surface(s), mask(m), patterns(p), fillType(f), fillOriginX(fx), fillOriginY(fy), thickness(t), bgColor(bg) {
	}
};

void macDrawPixel(int x, int y, int color, void *data);

/**
 * A manager class to handle window creation, destruction,
 * drawing, moving and event handling.
 */
class MacWindowManager {
public:
	MacWindowManager(uint32 mode = 0);
	~MacWindowManager();

	/**
	 * Mutator to indicate the surface onto which the desktop will be drawn.
	 * Note that this method should be called as soon as the WM is created.
	 * @param screen Surface on which the desktop will be drawn.
	 */
	void setScreen(ManagedSurface *screen) { _screen = screen; delete _screenCopy; _screenCopy = nullptr; }
	/**
	 * Create a window with the given parameters.
	 * Note that this method allocates the necessary memory for the window.
	 * @param scrollable True if the window has to be scrollable.
	 * @param resizable True if the window can be resized.
	 * @param editable True if the window can be edited.
	 * @return Pointer to the newly created window.
	 */
	MacWindow *addWindow(bool scrollable, bool resizable, bool editable);
	MacTextWindow *addTextWindow(const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler = true);

	/**
	 * Adds a window that has already been initialized to the registry.
	 * Like addWindow, but this doesn't create/allocate the Window.
	 * @param macWindow the window to be added to the registry
	 */
	void addWindowInitialized(MacWindow *macwindow);
	/**
	 * Returns the last allocated id
	 * @return last allocated window id
	 */
	int getLastId() { return _lastId; }
	/**
	 * Returns the next available id and the increments the internal counter.
	 * @return next (new) window id that can be used
	 */
	int getNextId() { return _lastId++; }
	/**
	 * Add the menu to the desktop.
	 * Note that the returned menu is empty, and therefore must be filled
	 * afterwards.
	 * @return Pointer to a new empty menu.
	 */
	MacMenu *addMenu();

	void activateMenu();

	void activateScreenCopy();
	void disableScreenCopy();

	bool isMenuActive();

	/**
	 * Set hot zone where menu appears (works only with autohide menu)
	 */
	void setMenuHotzone(const Common::Rect &rect) { _menuHotzone = rect; }

	/**
	 * Set delay in milliseconds when menu appears (works only with autohide menu)
	 */
	void setMenuDelay(int delay) { _menuDelay = delay; }

	/**
	 * Set the desired window state to active.
	 * @param id ID of the window that has to be set to active.
	 */
	void setActiveWindow(int id);

	/**
	 * Mark a window for removal.
	 * Note that the window data will be destroyed.
	 * @param target Window to be removed.
	 */
	void removeWindow(MacWindow *target);

	/**
	 * Mutator to indicate that the entire desktop must be refreshed.
	 * @param redraw Currently unused.
	 */
	void setFullRefresh(bool redraw) { _fullRefresh = redraw; }

	/**
	 * Method to draw the desktop into the screen,
	 * It will take into accout the contents set as dirty.
	 * Note that this method does not refresh the screen,
	 * g_system must be called separately.
	 */
	void draw();

	/**
	 * Method to process the events from the engine.
	 * Most often this method will be called from the engine's GUI, and
	 * will send the event to the relevant windows for them to process.
	 * @param event The event to be processed.
	 * @return True if the event was processed.
	 */
	bool processEvent(Common::Event &event);

	/**
	 * Accessor to retrieve an arbitrary window.
	 * @param id The id of the desired window.
	 * @return Pointer to the requested window, if it exists.
	 */
	BaseMacWindow *getWindow(int id) { return _windows[id]; }

	/**
	 * Retrieve the patterns used to fill surfaces.
	 * @return A MacPatterns object reference with the patterns.
	 */
	MacPatterns &getPatterns() { return _patterns; }

	/**
	 * Sets an active widget, typically the one which steals the input
	 * It also sends deactivation message to the previous one
	 * @param widget Pointer to the widget to activate, nullptr for no widget
	 */
	void setActiveWidget(MacWidget *widget);

	MacWidget *getActiveWidget() { return _activeWidget; }

	void pushArrowCursor();
	void pushBeamCursor();
	void pushCrossHairCursor();
	void pushCrossBarCursor();
	void pushWatchCursor();
	void pushCustomCursor(const byte *data, int w, int h, int hx, int hy, int transcolor);
	void pushCustomCursor(const Graphics::Cursor *cursor);
	void popCursor();

	PauseToken pauseEngine();

	void setMode(uint32 mode);

	void setEnginePauseCallback(Engine *engine);
	void setEngineRedrawCallback(void *engine, void (*redrawCallback)(void *engine));

	void passPalette(const byte *palette, uint size);
	uint findBestColor(byte cr, byte cg, byte cb);

public:
	MacFontManager *_fontMan;
	uint32 _mode;

	Common::Point _lastMousePos;
	Common::Rect _menuHotzone;

	bool _menuTimerActive;

	int _colorBlack, _colorWhite;

private:
	void drawDesktop();

	void removeMarked();
	void removeFromStack(BaseMacWindow *target);
	void removeFromWindowList(BaseMacWindow *target);

public:
	ManagedSurface *_screen;
	ManagedSurface *_screenCopy;

private:
	Common::List<BaseMacWindow *> _windowStack;
	Common::HashMap<uint, BaseMacWindow *> _windows;

	Common::List<BaseMacWindow *> _windowsToRemove;
	bool _needsRemoval;

	int _lastId;
	int _activeWindow;

	bool _fullRefresh;

	MacPatterns _patterns;
	byte *_palette;
	uint _paletteSize;

	MacMenu *_menu;
	uint32 _menuDelay;

	Engine *_engineP;
	void *_engineR;
	void (*_redrawEngineCallback)(void *engine);

	bool _cursorIsArrow;

	MacWidget *_activeWidget;

	PauseToken _screenCopyPauseToken;
};

} // End of namespace Graphics

#endif
