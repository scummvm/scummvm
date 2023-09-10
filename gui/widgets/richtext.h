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

#ifndef GUI_WIDGETS_RICHTEXT_H
#define GUI_WIDGETS_RICHTEXT_H

#include "common/str.h"
#include "gui/widget.h"

namespace Graphics {
class MacText;
class ManagedSurface;
}

namespace GUI {

class ScrollBarWidget;

/* RichTextWidget */
class RichTextWidget : public Widget, public CommandSender {
protected:
	Graphics::MacText *_txtWnd = nullptr;
	Graphics::ManagedSurface *_surface = nullptr;
	Common::U32String _text;

	ScrollBarWidget *_verticalScroll;
	int16 _scrolledX, _scrolledY;
	int _mouseDownY = 0;
	int _mouseDownStartY = 0;
	int _innerMargin;
	int _scrollbarWidth;
	uint16 _limitH;
	int _textWidth;
	int _textHeight;

	Common::Path _imageArchive;

public:
	RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String());
	RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String());
	RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String());

	~RichTextWidget();

	void reflowLayout() override;
	void draw() override;

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleMouseWheel(int x, int y, int direction) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseMoved(int x, int y, int button) override;
	void handleTooltipUpdate(int x, int y) override;

	void markAsDirty() override;

	bool containsWidget(Widget *) const override;

	void setImageArchive(const Common::Path &fname) { _imageArchive = fname; }

protected:
	void init();
	void recalc();
	void drawWidget() override;
	void createWidget();
	Widget *findWidget(int x, int y) override;
};

} // End of namespace GUI

#endif
