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

#ifndef GRAPHICS_MACGUI_MACWINDOWMANAGER_H
#define GRAPHICS_MACGUI_MACWINDOWMANAGER_H

#include "common/hashmap.h"
#include "common/list.h"
#include "common/mutex.h"
#include "common/stack.h"
#include "common/events.h"

#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"

#include "engines/engine.h"

namespace Common {
class Archive;
}

namespace Graphics {

namespace MacGUIConstants {
enum {
	kDesktopArc = 7
};

enum {
	kColorBlack = 0,
	kColorGray80 = 1,
	kColorGray88 = 2,
	kColorGrayEE = 3,
	kColorWhite = 4,
	kColorGreen = 5,
	kColorGreen2 = 6,
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

enum MacCursorType {
	kMacCursorArrow,
	kMacCursorBeam,
	kMacCursorCrossHair,
	kMacCursorCrossBar,
	kMacCursorWatch,
	kMacCursorCustom,
	kMacCursorOff
};

enum {
	kWMModeNone         		= 0,
	kWMModeNoDesktop    		= (1 << 0),
	kWMModeAutohideMenu 		= (1 << 1),
	kWMModalMenuMode 			= (1 << 2),
	kWMModeForceBuiltinFonts	= (1 << 3),
	kWMModeUnicode				= (1 << 4),
	kWMModeManualDrawWidgets	= (1 << 5),
	kWMModeFullscreen			= (1 << 6),
	kWMModeButtonDialogStyle	= (1 << 7),
	kWMMode32bpp				= (1 << 8),
	kWMNoScummVMWallpaper		= (1 << 9),
	kWMModeWin95				= (1 << 10),
	kWMModeForceMacFontsInWin95 = (1 << 11), // Enforce Mac font for languages which don't have glyphs in ms_sans_serif.ttf
	kWMModeNoCursorOverride     = (1 << 12),
	kWMModeForceMacBorder       = (1 << 13),
};

}
using namespace MacGUIConstants;

class Cursor;

class ManagedSurface;

class MacCursor;
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
	bool invert;

	MacPlotData(Graphics::ManagedSurface *s, Graphics::ManagedSurface *m, MacPatterns *p, uint f, int fx, int fy, int t, uint bg, bool inv = false) :
		surface(s), mask(m), patterns(p), fillType(f), fillOriginX(fx), fillOriginY(fy), thickness(t), bgColor(bg), invert(inv) {
	}
};

struct ZoomBox {
	Common::Rect start;
	Common::Rect end;
	Common::Array<Common::Rect> last;
	int delay;
	int step;
	uint32 startTime;
	uint32 nextTime;
};

typedef void (* MacDrawPixPtr)(int, int, int, void *);

/**
 * A manager class to handle window creation, destruction,
 * drawing, moving and event handling.
 */
class MacWindowManager {
public:
	MacWindowManager(uint32 mode = 0, MacPatterns *patterns = nullptr, Common::Language language = Common::UNK_LANG);
	~MacWindowManager();

	MacDrawPixPtr getDrawPixel();
	MacDrawPixPtr getDrawInvertPixel();

	/**
	 * Mutator to indicate the surface onto which the desktop will be drawn.
	 * Note that this method should be called as soon as the WM is created.
	 * @param screen Surface on which the desktop will be drawn.
	 */
	void setScreen(ManagedSurface *screen);

	/**
	 * Mutator to indicate the dimensions of the desktop, when a backing surface is not used.
	 * Note that this method should be called as soon as the WM is created.
	 * @param screen Surface on which the desktop will be drawn.
	 */
	void setScreen(int w, int h);

	int getWidth();

	int getHeight();

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
	MacTextWindow *addTextWindow(const Font *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler = true);
	void resizeScreen(int w, int h);

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
	void addMenu(int id, MacMenu *menu);

	void removeMenu();
	void activateMenu();

	void activateScreenCopy();
	void disableScreenCopy();

	bool isMenuActive();
	void setDesktopMode(uint32 mode);

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
	 * Return Top Window containing a point
	 * @param x x coordinate of point
	 * @param y y coordiante of point
	 */
	MacWindow *findWindowAtPoint(int16 x, int16 y);
	/**
	 * Return Top Window containing a point
	 * @param point Point
	 */
	MacWindow *findWindowAtPoint(Common::Point point);

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

	/**
	 * Similar to setActiveWidget but in this case no action including animation
	 * hover, etc can work until a window is locked.
	 * Anything outside this window will not respond to user.
	 * @param widget Pointer to the widget to lock, nullptr for no widget
	 */
	void setLockedWidget(MacWidget *widget);

	/**
	 * Sets a background window, which is always active, this window cannot be
	 * deactivated by clicking outside it, ie it is always in background.
	 * @param window Pointer to the widget to background, nullptr for no widget
	 */
	void setBackgroundWindow(MacWindow *window);

	MacPatterns  &getBuiltinPatterns() { return _builtinPatterns; }

	MacWidget *getActiveWidget() { return _activeWidget; }
	MacWidget *getLockedWidget() { return _lockedWidget; }

	Common::Rect getScreenBounds() { return _screen ? _screen->getBounds() : _screenDims; }

	void clearWidgetRefs(MacWidget *widget);

	void printWMMode(int debuglevel = 0);

private:
	void replaceCursorType(MacCursorType type);

public:
	MacCursorType getCursorType() const;

	void pushCursor(MacCursorType type, Cursor *cursor = nullptr);
	void replaceCursor(MacCursorType type, Cursor *cursor = nullptr);

	void pushCustomCursor(const byte *data, int w, int h, int hx, int hy, int transcolor);
	void replaceCustomCursor(const byte *data, int w, int h, int hx, int hy, int transcolor);
	void pushCustomCursor(const Graphics::Cursor *cursor);
	void popCursor();

	PauseToken pauseEngine();

	void setMode(uint32 mode);

	void setEngine(Engine *engine);
	void setEngineRedrawCallback(void *engine, void (*redrawCallback)(void *engine));

	void passPalette(const byte *palette, uint size);
	template <typename T> void decomposeColor(uint32 color, byte &r, byte &g, byte &b);
	uint32 findBestColor(byte cr, byte cg, byte cb);
	uint32 findBestColor(uint32 color);
	void setDesktopColor(byte, byte, byte);

	byte inverter(byte src);

	const byte *getPalette() { return _paletteLookup.getPalette().data; }
	uint getPaletteSize() { return _paletteLookup.getPalette().size; }

	void renderZoomBox(bool redraw = false);
	void addZoomBox(ZoomBox *box);

	void removeMarked();

	void loadDataBundle();
	void cleanupDataBundle();
	void cleanupDesktopBmp();

	BorderOffsets getBorderOffsets(uint32 windowType);
	Common::SeekableReadStream *getBorderFile(uint32 windowType, uint32 flags);
	Common::SeekableReadStream *getFile(const Common::Path &filename);

	void setTextInClipboard(const Common::U32String &str);
	/**
	 * get text from WM clipboard or the global clipboard
	 * @param size will change to the length of real text in clipboard
	 * @return the text in clipboard, which may contained the format
	 */
	Common::U32String getTextFromClipboard(const Common::U32String &format = Common::U32String(), int *size = nullptr);

	/**
	 * reset events for current widgets. i.e. we reset those variables which are used for handling events for macwidgets.
	 * e.g. we clear the active widget, set mouse down false, clear the hoveredWidget
	 * this function should be called when we are going to other level to handling events. thus wm may not handle events correctly.
	 */
	void clearHandlingWidgets();

	void setMenuItemCheckMark(MacMenuItem *menuItem, bool checkMark);
	void setMenuItemEnabled(MacMenuItem *menuItem, bool enabled);
	void setMenuItemName(MacMenuItem *menuItem, const Common::String &name);
	void setMenuItemAction(MacMenuItem *menuItem, int actionId);

	bool getMenuItemCheckMark(MacMenuItem *menuItem);
	bool getMenuItemEnabled(MacMenuItem *menuItem);
	Common::String getMenuItemName(MacMenuItem *menuItem);
	int getMenuItemAction(MacMenuItem *menuItem);
	MacMenu *getMenu();
	MacMenu *getMenu(int id);

public:
	MacFontManager *_fontMan;
	uint32 _mode;
	Common::Language _language;

	Common::Point _lastClickPos;
	Common::Point _lastMousePos;
	Common::Rect _menuHotzone;

	bool _menuTimerActive;
	bool _mouseDown;

	uint32 _colorBlack, _colorGray80, _colorGray88, _colorGrayEE, _colorWhite, _colorGreen, _colorGreen2;

	MacWidget *_hoveredWidget;

	// we use it to indicate whether we are clicking the hilite-able widget.
	// In list style button mode, we will highlight the subsequent buttons only when we've clicked the hilite-able button initially
	bool _hilitingWidget;

private:
	void loadDesktop();
	void drawDesktop();

	void removeFromStack(BaseMacWindow *target);
	void removeFromWindowList(BaseMacWindow *target);

	void zoomBoxInner(Common::Rect &r, Graphics::MacPlotData &pd);
	bool haveZoomBox() { return !_zoomBoxes.empty(); }

	void adjustDimensions(const Common::Rect &clip, const Common::Rect &dims, int &adjWidth, int &adjHeight);

public:
	Surface *_desktopBmp;
	ManagedSurface *_desktop;
	PixelFormat _pixelformat;

	ManagedSurface *_screen;
	ManagedSurface *_screenCopy;
	Common::Rect _screenDims;

private:
	Common::Mutex _mutex;
	Common::List<BaseMacWindow *> _windowStack;
	Common::HashMap<uint, BaseMacWindow *> _windows;

	Common::List<BaseMacWindow *> _windowsToRemove;
	bool _needsRemoval;

	int _lastId;
	int _activeWindow;

	bool _fullRefresh;

	bool _inEditableArea;

	MacPatterns _patterns;
	MacPatterns _builtinPatterns;

	MacMenu *_menu;
	uint32 _menuDelay;

	Engine *_engineP;
	void *_engineR;
	void (*_redrawEngineCallback)(void *engine);

	MacCursorType _tempType = kMacCursorArrow;
	Common::Stack<MacCursorType> _cursorTypeStack;
	Cursor *_cursor = nullptr;

	MacWidget *_activeWidget;
	MacWidget *_lockedWidget;
	MacWindow *_backgroundWindow;

	PauseToken *_screenCopyPauseToken;

	Common::Array<ZoomBox *> _zoomBoxes;
	Graphics::PaletteLookup _paletteLookup;
	Common::HashMap<uint, uint> _invertColorHash;

	Common::Archive *_dataBundle;

	Common::U32String _clipboard;
};

const Common::U32String::value_type *readHex(uint16 *res, const Common::U32String::value_type *s, int len);

} // End of namespace Graphics

#endif
