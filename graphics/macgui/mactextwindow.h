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

#ifndef GRAPHICS_MACGUI_MACTEXTWINDOW_H
#define GRAPHICS_MACGUI_MACTEXTWINDOW_H

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"

namespace Graphics {

class MacTextWindow : public MacWindow {
public:
	MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler = true);
	MacTextWindow(MacWindowManager *wm, const Font *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, bool cursorHandler = true);
	virtual ~MacTextWindow();

	virtual void resize(int w, int h) override;
	void setDimensions(const Common::Rect &r) override;

	virtual bool processEvent(Common::Event &event) override;

	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	virtual bool draw(bool forceRedraw = false) override;
	virtual void blit(ManagedSurface *g, Common::Rect &dest) override;

	void setTextWindowFont(const MacFont *macFont);
	const MacFont *getTextWindowFont();

	void appendText(const Common::U32String &str, const MacFont *macFont = nullptr, bool skipAdd = false);
	void appendText(const Common::String &str, const MacFont *macFont = nullptr, bool skipAdd = false);
	void clearText();

	void setEditable(bool editable) { _editable = editable; _mactext->setEditable(editable); }
	void setSelectable(bool selectable) { _selectable = selectable; }

	void undrawCursor();

	const Common::U32String &getInput() { return _inputText; }
	void clearInput();
	void appendInput(const Common::U32String &str);
	void appendInput(const Common::String &str);

	Common::U32String getSelection(bool formatted = false, bool newlines = true);
	void clearSelection();
	Common::U32String cutSelection();
	const SelectedText *getSelectedText() { return &_selectedText; }
	int getTextHeight() { return _mactext->getTextHeight(); }
	int getMouseLine(int x, int y);

	/**
	 * if we want to draw the text which color is not black, then we need to set _textColorRGB
	 * @param rgb text color you want to draw
	 */
	void setTextColorRGB (uint32 rgb) { _textColorRGB = rgb; }

private:
	void init(bool cursorHandler);
	bool isCutAllowed();

	void scroll(int delta);
	void calcScrollBar();

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
	bool _editable;
	bool _selectable;

	int _scrollPos;

private:
	MacText *_mactext;
	const MacFont *_font;
	const Font *_fontRef;

	ManagedSurface *_cursorSurface;

	bool _inTextSelection;
	SelectedText _selectedText;

	int _maxWidth;
	Common::U32String _inputText;
	uint _inputTextHeight;
	bool _inputIsDirty;

	MacMenu *_menu;

	int _bgcolor;
	int _textColorRGB;
};

} // End of namespace Graphics

#endif
