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

enum {
	kCursorHeight = 12
};

struct SelectedText {
	int startX, startY;
	int endX, endY;

	SelectedText() {
		startX = startY = 0;
		endX = endY = 0;
	}

	bool needsRender() {
		return startX != endX || startY != endY;
	}
};

class MacTextWindow : public MacWindow {
public:
	MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment);
	virtual ~MacTextWindow();

	virtual void resize(int w, int h);

	virtual bool processEvent(Common::Event &event);

	/**
	 * Similar to that described in BaseMacWindow.
	 * @param g See BaseMacWindow.
	 * @param forceRedraw If true, the borders are guarranteed to redraw.
	 */
	virtual bool draw(ManagedSurface *g, bool forceRedraw = false);

	void setTextWindowFont(const MacFont *macFont);
	const MacFont *getTextWindowFont();

	void appendText(Common::String str, const MacFont *macFont, bool skipAdd = false);
	void clearText();

	void setSelection(int selStartX, int selStartY, int selEndX, int selEndY);

	void undrawCursor();

	const Common::String getInput() { return _inputText; }
	void clearInput();

private:
	void undrawInput();
	void drawInput();
	void updateCursorPos();

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

	SelectedText _selectedText;

	int _maxWidth;
	Common::String _inputText;
	uint _inputTextHeight;
	bool _inputIsDirty;

};

} // End of namespace Graphics

#endif
