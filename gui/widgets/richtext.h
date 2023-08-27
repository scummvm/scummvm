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
class RichTextWidget : public Widget {
protected:
	Graphics::MacText *_txtWnd = nullptr;
	Graphics::ManagedSurface *_surface = nullptr;
	Common::U32String _text;

	ScrollBarWidget *_verticalScroll;
	int16 _scrolledX, _scrolledY;
	int _scrollbarWidth;
	uint16 _limitH;
	uint32 _reflowCmd;

public:
	RichTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String());
	RichTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String());
	RichTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String());

	void reflowLayout() override;
	void draw() override;

protected:
	void init();
	void recalc();
	void drawWidget() override;
	void createWidget();
};

} // End of namespace GUI

#endif
