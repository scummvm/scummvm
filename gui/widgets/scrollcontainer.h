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

#ifndef GUI_WIDGETS_SCROLLCONTAINER_H
#define GUI_WIDGETS_SCROLLCONTAINER_H

#include "gui/widget.h"
#include "common/str.h"
#include "scrollbar.h"

namespace GUI {

class ScrollContainerWidget: public Widget, public CommandSender {
	ScrollBarWidget *_verticalScroll;
	int16 _scrolledX, _scrolledY;
	uint16 _limitH;
	uint32 _reflowCmd;
	ThemeEngine::WidgetBackground _backgroundType;
	Common::String _dialogName;

	void recalc();

public:
	ScrollContainerWidget(GuiObject *boss, int x, int y, int w, int h, uint32 reflowCmd = 0);
	ScrollContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &dialogName, uint32 reflowCmd = 0);
	~ScrollContainerWidget() override;

	void init();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void reflowLayout() override;

	bool containsWidget(Widget *) const override;

	Common::Rect getClipRect() const override;

	void setBackgroundType(ThemeEngine::WidgetBackground backgroundType);

	void handleMouseWheel(int x, int y, int direction) override;

	// We overload getChildY to make sure child widgets are positioned correctly.
	// Essentially this compensates for the space taken up by the tab title header.
	int16	getChildX() const override;
	int16	getChildY() const override;
	uint16	getWidth() const override;
	uint16	getHeight() const override;

protected:
	void drawWidget() override;

	Widget *findWidget(int x, int y) override;
};

} // End of namespace GUI

#endif
