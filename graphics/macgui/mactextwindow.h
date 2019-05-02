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

#ifndef GRAPHICS_MACGUI_MACTEXTWINDOW_H
#define GRAPHICS_MACGUI_MACTEXTWINDOW_H

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"

namespace Graphics {

struct SelectedText {
	int startX, startY;
	int endX, endY;
	int startRow, startCol;
	int endRow, endCol;

	SelectedText() {
		startX = startY = -1;
		endX = endY = -1;
		startRow = startCol = -1;
		endRow = endCol = -1;
	}

	bool needsRender() {
		return startX != endX || startY != endY;
	}
};

class MacTextWindow : public MacWindow {
public:
	MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler = true);
	virtual ~MacTextWindow();

	virtual void resize(int w, int h);

	virtual bool processEvent(Common::Event &event);

	virtual bool draw(ManagedSurface *g, bool forceRedraw = false);

	void setTextWindowFont(const MacFont *macFont);
	const MacFont *getTextWindowFont();

	void appendText(Common::String str, const MacFont *macFont, bool skipAdd = false);
	void clearText();

	void undrawCursor();

	const Common::String getInput() { return _inputText; }
	void clearInput();
	void appendInput(Common::String str);

	Common::String getSelection(bool formatted = false, bool newlines = true);
	void clearSelection();
	Common::String cutSelection();
	const SelectedText *getSelectedText() { return &_selectedText; }

private:
	bool isCutAllowed();

	void scroll(int delta);

	void undrawInput();
	void drawInput();
	void drawSelection();
	void updateCursorPos();

	void startMarking(int x, int y);
	void updateTextSelection(int x, int y);

public:
	int _cursorX, _cursorY;
	bool _cursorState;

	bool _cursorDirty;
	Common::Rect *_cursorRect;
	bool _cursorOff;

	int _scrollPos;

private:
	MacText *_mactext;
	const MacFont *_font;
	const Font *_fontRef;

	ManagedSurface *_cursorSurface;

	bool _inTextSelection;
	SelectedText _selectedText;

	int _maxWidth;
	Common::String _inputText;
	uint _inputTextHeight;
	bool _inputIsDirty;

	MacMenu *_menu;
};

} // End of namespace Graphics

#endif
