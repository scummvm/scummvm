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
#include "graphics/surface.h"
#include "common/rect.h"

namespace Wage {

class Menu;

enum WindowType {
	kWindowScene,
	kWindowConsole
};

enum {
	kMenuHeight = 20,
	kMenuLeftMargin = 7,
	kMenuSpacing = 13,
	kMenuPadding = 16,
	kMenuDropdownPadding = 12,
	kMenuDropdownItemHeight = 15,
	kMenuItemHeight = 20,
	kBorderWidth = 17,
	kDesktopArc = 7,
	kComponentsPadding = 10,
	kCursorHeight = 12
};

class Gui {
public:
	Gui(WageEngine *engine);
	~Gui();

	void draw();
	void appendText(Common::String &str);
	void clearOutput();
	void mouseMove(int x, int y);
	void mouseClick(int x, int y);
	Designed *getClickTarget(int x, int y);
	void drawInput();
	void setSceneDirty() { _sceneDirty = true; }
	const Graphics::Font *getFont(const char *name, Graphics::FontManager::FontUsage fallback);

private:
	void paintBorder(Graphics::Surface *g, Common::Rect &r, WindowType windowType);
	void renderConsole(Graphics::Surface *g, Common::Rect &r);
	void drawBox(Graphics::Surface *g, int x, int y, int w, int h);
	void fillRect(Graphics::Surface *g, int x, int y, int w, int h);
	void loadFonts();
	void flowText(Common::String &str);
	const Graphics::Font *getConsoleFont();
	const Graphics::Font *getTitleFont();

public:
	Graphics::Surface _screen;
	int _cursorX, _cursorY;
	bool _cursorState;
	Common::Rect _consoleTextArea;
	bool _cursorOff;

	bool _builtInFonts;
	WageEngine *_engine;

private:
	Graphics::Surface _console;
	Menu *_menu;
	Scene *_scene;
	bool _sceneDirty;
	bool _consoleDirty;
	bool _bordersDirty;
	bool _menuDirty;

	Common::StringArray _out;
	Common::StringArray _lines;
	uint _scrollPos;
	int _consoleLineHeight;
	uint _consoleNumLines;
	bool _consoleFullRedraw;

	Common::Rect _sceneArea;
	bool _sceneIsActive;
	bool _cursorIsArrow;
};

} // End of namespace Wage

#endif
