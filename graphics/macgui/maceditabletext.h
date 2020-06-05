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

#ifndef GRAPHICS_MACGUI_MACEDITABLETEXT_H
#define GRAPHICS_MACGUI_MACEDITABLETEXT_H

#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwidget.h"

namespace Graphics {

class MacMenu;
class MacText;
class MacWidget;
class MacWindow;

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

class MacEditableText : public MacText, public MacWidget {
public:
	MacEditableText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0, uint16 border = 0, uint16 gutter = 0, uint16 boxShadow = 0, uint16 textShadow = 0);
	MacEditableText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::String &s, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment = kTextAlignLeft, int interlinear = 0, uint16 border = 0, uint16 gutter = 0, uint16 boxShadow = 0, uint16 textShadow = 0);
			// 0 pixels between the lines by default
	virtual ~MacEditableText();

	virtual void resize(int w, int h);

	virtual bool processEvent(Common::Event &event) override;

	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	virtual bool draw(bool forceRedraw = false) override;
	virtual void blit(ManagedSurface *g, Common::Rect &dest) override;

	void setAlignOffset(TextAlign align);
	Common::Point calculateOffset();
	virtual void setActive(bool active) override;

	void appendText(const Common::U32String &str, const MacFont *macFont, bool skipAdd = false);
	void appendText(const Common::String &str, const MacFont *macFont, bool skipAdd = false);
	void clearText();

	void undrawCursor();

	Common::U32String getSelection(bool formatted = false, bool newlines = true);
	void clearSelection();
	Common::U32String cutSelection();
	const SelectedText *getSelectedText() { return &_selectedText; }

	Common::U32String getEditedString();

private:
	void init();
	bool isCutAllowed();

	void scroll(int delta);

	void drawSelection();
	void updateCursorPos();

	void startMarking(int x, int y);
	void updateTextSelection(int x, int y);

public:
	int _cursorX, _cursorY;
	bool _cursorState;
	int _cursorRow, _cursorCol;

	bool _cursorDirty;
	Common::Rect *_cursorRect;
	bool _cursorOff;
	bool _selectable;

	int _scrollPos;

protected:
	Common::Point _alignOffset;

private:
	ManagedSurface *_cursorSurface;

	int _editableRow;

	bool _inTextSelection;
	SelectedText _selectedText;

	int _maxWidth;

	MacMenu *_menu;
};

} // End of namespace Graphics

#endif
