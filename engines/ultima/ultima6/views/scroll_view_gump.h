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

#ifndef ULTIMA6_VIEWS_SCROLL_VIEW_GUMP_H
#define ULTIMA6_VIEWS_SCROLL_VIEW_GUMP_H

#include "ultima/ultima6/views/draggable_view.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class ScrollWidgetGump;

class ScrollViewGump : public DraggableView {

	ScrollWidgetGump *scroll_widget;

public:
	ScrollViewGump(Configuration *cfg);
	~ScrollViewGump();

	bool init(Screen *tmp_screen, void *view_manager, Font *f, Party *p, TileManager *tm, ObjManager *om, Std::string text_string);

	void Display(bool full_redraw);

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);

	GUI_status MouseDown(int x, int y, Shared::MouseButton button);
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) {
		return GUI_YUM;
	}
	GUI_status KeyDown(const Common::KeyState &key);

protected:

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
