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

#ifndef WAGE_MENU_H
#define WAGE_MENU_H

namespace Wage {

struct MenuItem;
struct MenuSubItem;

enum {
	kFontStyleBold = 1,
	kFontStyleItalic = 2,
	kFontStyleUnderline = 4,
	kFontStyleOutline = 8,
	kFontStyleShadow = 16,
	kFontStyleCondensed = 32,
	kFontStyleExtended = 64
};

enum {
	kMenuAbout = 0,
	kMenuFile = 1,
	kMenuEdit = 2,
	kMenuCommands = 3,
	kMenuWeapons = 4
};

enum {
	kMenuActionAbout,
	kMenuActionNew,
	kMenuActionOpen,
	kMenuActionClose,
	kMenuActionSave,
	kMenuActionSaveAs,
	kMenuActionRevert,
	kMenuActionQuit,

	kMenuActionUndo,
	kMenuActionCut,
	kMenuActionCopy,
	kMenuActionPaste,
	kMenuActionClear,

	kMenuActionCommand
};

class Menu : public BaseMacWindow {
public:
	Menu(int id, const Common::Rect &bounds, MacWindowManager *wm, Gui *gui);
	~Menu();

	bool draw(Graphics::ManagedSurface *g, bool forceRedraw = false);
	bool processEvent(Common::Event &event);

	void regenCommandsMenu();
	void regenWeaponsMenu();
	void enableCommand(int menunum, int action, bool state);
	void disableAllMenus();

	void setActive(bool active) { _menuActivated = active; }
	bool hasAllFocus() { return _menuActivated; }

	Common::Rect _bbox;

private:
	Gui *_gui;
	Graphics::ManagedSurface _screen;
	Graphics::ManagedSurface _screenCopy;
	Graphics::ManagedSurface _tempSurface;

private:
	const Graphics::Font *getMenuFont();
	const char *getAcceleratorString(MenuSubItem *item, const char *prefix);
	int calculateMenuWidth(MenuItem *menu);
	void calcMenuBounds(MenuItem *menu);
	void renderSubmenu(MenuItem *menu);
	void createCommandsMenu(MenuItem *menu);
	void createWeaponsMenu(MenuItem *menu);
	void executeCommand(MenuSubItem *subitem);

	bool keyEvent(Common::Event &event);
	bool mouseClick(int x, int y);
	bool mouseRelease(int x, int y);
	bool mouseMove(int x, int y);

	bool processMenuShortCut(byte flags, uint16 ascii);

	Common::Array<MenuItem *> _items;
	MenuItem *_weapons;
	MenuItem *_commands;

	const Graphics::Font *_font;

	bool _menuActivated;

	int _activeItem;
	int _activeSubItem;
};

} // End of namespace Wage

#endif
