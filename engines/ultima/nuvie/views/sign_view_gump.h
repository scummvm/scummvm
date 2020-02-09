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

#ifndef NUVIE_VIEWS_SIGN_VIEW_GUMP_H
#define NUVIE_VIEWS_SIGN_VIEW_GUMP_H

#include "ultima/nuvie/views/draggable_view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Font;
//class BMPFont;

class SignViewGump : public DraggableView {

	Font *font;
	char *sign_text;

public:
	SignViewGump(Configuration *cfg);
	~SignViewGump() override;

	bool init(Screen *tmp_screen, void *view_manager, Font *f, Party *p, TileManager *tm, ObjManager *om, const char *text_string, uint16 length);

	void Display(bool full_redraw) override;

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status KeyDown(const Common::KeyState &key) override;

protected:

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
