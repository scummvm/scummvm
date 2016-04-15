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

#ifndef WAGE_GUI_H
#define WAGE_GUI_H

#include "common/str-array.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "common/rect.h"

#include "wage/macwindow.h"
#include "wage/macwindowmanager.h"

namespace Wage {

class Menu;

enum {
	kMenuHeight = 20,
	kMenuLeftMargin = 7,
	kMenuSpacing = 13,
	kMenuPadding = 16,
	kMenuDropdownPadding = 14,
	kMenuDropdownItemHeight = 16,
	kMenuItemHeight = 20,
	kDesktopArc = 7,
	kComponentsPadding = 10,
	kCursorHeight = 12
};

enum {
	kPatternSolid = 1,
	kPatternStripes = 2,
	kPatternCheckers = 3,
	kPatternCheckers2 = 4
};

class Gui {
public:
	Gui(WageEngine *engine);
	~Gui();

	void draw();
	void appendText(const char *str);
	void clearOutput();
	void mouseMove(int x, int y);
	void mouseDown(int x, int y);
	Designed *mouseUp(int x, int y);
	void drawInput();
	void setSceneDirty() { _sceneDirty = true; }
	const Graphics::Font *getFont(const char *name, Graphics::FontManager::FontUsage fallback);
	void regenCommandsMenu();
	void regenWeaponsMenu();
	void processMenuShortCut(byte flags, uint16 ascii);
	void pushArrowCursor();
	void popCursor();

	void actionCopy();
	void actionPaste();
	void actionUndo();
	void actionClear();
	void actionCut();
	void disableUndo();
	void disableAllMenus();
	void enableNewGameMenus();

	bool builtInFonts() { return _builtInFonts; }

private:
	void drawScene();
	void drawConsole();
	void undrawCursor();
	void drawDesktop();
	void paintBorder(Graphics::ManagedSurface *g, Common::Rect &r, WindowType windowType, int highlightedPart = kBorderNone,
						float scrollPos = 0.0, float scrollSize = 0.0);
	void renderConsole(Graphics::ManagedSurface *g, Common::Rect &r);
	void drawBox(Graphics::ManagedSurface *g, int x, int y, int w, int h);
	void fillRect(Graphics::ManagedSurface *g, int x, int y, int w, int h, int color = kColorBlack);
	void loadFonts();
	void flowText(Common::String &str);
	const Graphics::Font *getConsoleFont();
	const Graphics::Font *getTitleFont();
	void startMarking(int x, int y);
	int calcTextX(int x, int textLine);
	int calcTextY(int y);
	void updateTextSelection(int x, int y);

public:
	Graphics::ManagedSurface _screen;
	int _cursorX, _cursorY;
	bool _cursorState;
	Common::Rect _consoleTextArea;

	bool _builtInFonts;
	WageEngine *_engine;

	Patterns _patterns;

	bool _cursorDirty;
	Common::Rect _cursorRect;
	bool _cursorOff;

	bool _menuDirty;

private:
	Graphics::ManagedSurface _console;
	Menu *_menu;
	Scene *_scene;
	bool _sceneDirty;
	bool _consoleDirty;
	bool _bordersDirty;

	Common::StringArray _out;
	Common::StringArray _lines;
	uint _scrollPos;
	int _consoleLineHeight;
	uint _consoleNumLines;
	bool _consoleFullRedraw;

	Common::Rect _sceneArea;
	bool _sceneIsActive;
	bool _cursorIsArrow;

	bool _inTextSelection;
	int _selectionStartX;
	int _selectionStartY;
	int _selectionEndX;
	int _selectionEndY;

	Common::String _clipboard;
	Common::String _undobuffer;

	int _inputTextLineNum;

	MacWindowManager _wm;
	int _sceneWindowId;
	int _consoleWindowId;
};

} // End of namespace Wage

#endif
