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

#ifndef BLADERUNNER_UI_SCROLLBOX_H
#define BLADERUNNER_UI_SCROLLBOX_H

#include "bladerunner/color.h"
#include "bladerunner/ui/ui_component.h"

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace BladeRunner {

typedef void UIScrollBoxClickedCallback(void *callbackData, void *source, int lineData, int mouseButton);

class UIScrollBox : public UIComponent {
	static const int kLineHeight = 10;
	static const Color256 k3DFrameColors[];
	static const Color256 kTextBackgroundColors[];
	static const Color256 kTextColors1[];
	static const Color256 kTextColors2[];
	static const Color256 kTextColors3[];
	static const Color256 kTextColors4[];

	struct Line {
		Common::String text;
		int    lineData;
		int    flags;
		uint32 checkboxFrame;
	};

	int                   _selectedLineState;
	int                   _scrollUpButtonState;
	int                   _scrollDownButtonState;
	int                   _scrollAreaUpState;
	int                   _scrollAreaDownState;
	int                   _scrollBarState;

	int                   _hoveredLine;
	int                   _selectedLineIndex;

	bool                  _scrollUpButtonHover;
	bool                  _scrollDownButtonHover;
	bool                  _scrollAreaUpHover;
	bool                  _scrollAreaDownHover;
	bool                  _scrollBarHover;

	bool                  _mouseButton;

	UIScrollBoxClickedCallback   *_lineSelectedCallback;
	void                         *_callbackData;

	bool                  _isVisible;
	int                   _style;
	bool                  _center;

	uint32                _timeLastScroll;
	uint32                _timeLastCheckbox;
	uint32                _timeLastHighlight;

	int                   _highlightFrame;

	Common::Rect          _rect;
	Common::Rect          _scrollBarRect;

	int                   _lineCount;
	int                   _maxLineCount;
	Common::Array<Line *> _lines;
	int                   _maxLinesVisible;
	int                   _firstLineVisible;

	bool                  _mouseOver;

public:
	UIScrollBox(BladeRunnerEngine *vm,
	            UIScrollBoxClickedCallback *lineSelectedCallback,
	            void *callbackData,
	            int maxLineCount,
	            int style,
	            bool center,
	            Common::Rect rect,
	            Common::Rect scrollBarRect);

	~UIScrollBox() override;

	void draw(Graphics::Surface &surface) override;

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool alternateButton) override;
	void handleMouseUp(bool alternateButton) override;
	void handleMouseScroll(int direction) override;

	void show();
	void hide();
	bool isVisible();
	bool hasFocus();

	void setBoxTop(int top);
	void setBoxLeft(int left);
	void setBoxWidth(uint16 width);
	void setScrollbarTop(int top);
	void setScrollbarLeft(int left);
	void setScrollbarWidth(uint16 width);

	int    getBoxLeft();
	uint16 getBoxWidth();


	void clearLines();
	void addLine(const Common::String &text, int lineData, int flags);
	void addLine(const char *text, int lineData, int flags);
	void sortLines();

	int getSelectedLineData();
	Common::String getLineText(int lineData);
	int getMaxLinesVisible();
	int getLineCount();

	void checkAll();
	void uncheckAll();
	void toggleCheckBox(int lineData);

	bool hasLine(int lineData);

	void resetHighlight(int lineData);
	void setFlags(int lineData, int flags);
	void resetFlags(int lineData, int flags);

private:
	static int sortFunction(const void *line1, const void *line2);

	void draw3DFrame(Graphics::Surface &surface, Common::Rect rect, bool pressed, int style);

	void scrollUp();
	void scrollDown();

	int findLine(int lineData);
};

} // End of namespace BladeRunner

#endif
