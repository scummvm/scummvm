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
	MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, int padding = 0);
	MacTextWindow(MacWindowManager *wm, const Font *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, int padding = 0);
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
	void setMarkdownText(const Common::U32String &str);

	void setEditable(bool editable) { _editable = editable; _mactext->setEditable(editable); }
	void setActive(bool active) override { MacWindow::setActive(active); if (_editable) _mactext->setActive(active); }
	void setSelectable(bool selectable) { _selectable = selectable; }

	const Common::U32String &getInput() { return _inputText; }
	void clearInput();
	void appendInput(const Common::U32String &str);
	void appendInput(const Common::String &str);

	Common::U32String getSelection(bool formatted = false, bool newlines = true);
	void clearSelection() { _mactext->clearSelection(); }
	Common::U32String cutSelection() { return _mactext->cutSelection(); }
	const SelectedText *getSelectedText() { return _mactext->getSelectedText(); }
	uint32 getTextColor() { return _mactext->getTextColor(); }
	uint32 getTextColor(int start, int end) { return _mactext->getTextColor(start, end); }
	void setTextColor(uint32 color, int start, int end) { return _mactext->setTextColor(color, start, end); }
	int getTextFont() { return _mactext->getTextFont(); }
	int getTextFont(int start, int end) { return _mactext->getTextFont(start, end); }
	int getTextSlant() { return _mactext->getTextSlant(); }
	int getTextSlant(int start, int end) { return _mactext->getTextSlant(start, end); }
	int getTextHeight() { return _mactext->getTextHeight(); }
	int getLineHeight(int line) { return _mactext->getLineHeight(line); }
	Common::U32String getTextChunk(int startRow, int startCol, int endRow, int endCol, bool formatted = false, bool newlines = true) { return _mactext->getTextChunk(startRow, startCol, endRow, endCol, formatted, newlines); }
	Common::U32String getPlainText() { return _mactext->getPlainText(); }
	Common::U32String getEditedString() { return _mactext->getEditedString(); };
	void enforceTextFont(uint16 fontId) { return _mactext->enforceTextFont(fontId); }
	void setTextFont(uint16 fontId, int start, int end) { return _mactext->setTextFont(fontId, start, end); }
	void enforceTextSlant(int textSlant) { return _mactext->enforceTextSlant(textSlant); }
	void setTextSlant(int textSlant, int start, int end) { return _mactext->setTextSlant(textSlant, start, end); }
	int getRowCount() { return _mactext->getRowCount(); }
	int getLineSpacing() { return _mactext->getLineSpacing(); }
	int getTextSize() { return _mactext->getTextSize(); }
	int getTextSize(int start, int end) { return _mactext->getTextSize(start, end); }
	void setTextSize(int textSize) { return _mactext->setTextSize(textSize); }
	void setTextSize(int textSize, int start, int end) { return _mactext->setTextSize(textSize, start, end); }

	int getMouseLine(int x, int y);

	virtual void setBorderColor(uint32 color) override { _mactext->setBorderColor(color); }
	/**
	 * if we want to draw the text which color is not black, then we need to set _textColorRGB
	 * @param rgb text color you want to draw
	 */
	void setTextColorRGB (uint32 rgb) { _textColorRGB = rgb; }

private:
	void init();

	void calcScrollBar();

	void undrawInput();
	void drawInput();
	void drawSelection();

	void calcWin95Scroll(int &scrollAreaTop, int &scrollAreaHeight, int &barY, int &barHeight);

public:
	bool _editable;
	bool _selectable;

private:
	MacText *_mactext;
	const MacFont *_font;
	const Font *_fontRef;

	int _maxWidth;
	Common::U32String _inputText;
	uint _inputTextHeight;
	bool _inputIsDirty;

	WindowClick _scrollDirection;
	uint32 _nextScrollTime;
	uint32 _scrollDelay;
	uint32 _nextWheelEventTime;
	WindowClick _clickedScrollPart;

	// dragging bar for win95
	bool _isDragging;
	int _dragStartY;
	int _dragStartScrollPos;

	MacMenu *_menu;

	int _bgcolor;
	int _textColorRGB;
};

} // End of namespace Graphics

#endif
