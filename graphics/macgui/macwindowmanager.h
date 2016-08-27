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

#ifndef GRAPHICS_MACGUI_MACWINDOWMANAGER_H
#define GRAPHICS_MACGUI_MACWINDOWMANAGER_H

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
	kColorGreen2 = 4,
	kColorCount
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

/**
 * A manager class to handle window creation, destruction,
 * drawing, moving and event handling.
 */
class MacWindowManager {
public:
	MacWindowManager();
	~MacWindowManager();

	/**
	 * Mutator to indicate the surface onto which the desktop will be drawn.
	 * Note that this method should be called as soon as the WM is created.
	 * @param screen Surface on which the desktop will be drawn.
	 */
	void setScreen(ManagedSurface *screen) { _screen = screen; }
	/**
	 * Accessor method to check the presence of built-in fonts.
	 * @return True if there are bult-in fonts.
	 */
	bool hasBuiltInFonts() { return _builtInFonts; }
	/**
	 * Retrieve a font from the available ones.
	 * @param name Name of the desired font.
	 * @param fallback Fallback policy in case the desired font isn't there.
	 * @return The requested font or the fallback.
	 */
	const Font *getFont(const char *name, FontManager::FontUsage fallback);

	/**
	 * Return font name from standard ID
	 * @param id ID of the font
	 * @param size size of the font
	 * @return the font name or NULL if ID goes beyond the mapping
	 */
	const char *getFontName(int id, int size);

	/**
	 * Create a window with the given parameters.
	 * Note that this method allocates the necessary memory for the window.
	 * @param scrollable True if the window has to be scrollable.
	 * @param resizable True if the window can be resized.
	 * @param editable True if the window can be edited.
	 * @return Pointer to the newly created window.
	 */
	MacWindow *addWindow(bool scrollable, bool resizable, bool editable);
	/**
	 * Add the menu to the desktop.
	 * Note that the returned menu is empty, and therefore must be filled
	 * afterwards.
	 * @return Pointer to a new empty menu.
	 */
	Menu *addMenu();
	/**
	 * Set the desired window state to active.
	 * @param id ID of the window that has to be set to active.
	 */
	void setActive(int id);
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
	void setFullRefresh(bool redraw) { _fullRefresh = true; }

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
	void drawFilledRoundRect(ManagedSurface *surface, Common::Rect &rect, int arc, int color);

	void pushArrowCursor();
	void popCursor();

private:
	void drawDesktop();
	void loadFonts();

	void removeMarked();
	void removeFromStack(BaseMacWindow *target);
	void removeFromWindowList(BaseMacWindow *target);

private:
	ManagedSurface *_screen;

	Common::List<BaseMacWindow *> _windowStack;
	Common::Array<BaseMacWindow *> _windows;

	Common::List<BaseMacWindow *> _windowsToRemove;
	bool _needsRemoval;

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
