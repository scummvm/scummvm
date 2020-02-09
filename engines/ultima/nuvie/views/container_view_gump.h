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

#ifndef NUVIE_VIEWS_CONTAINER_VIEW_GUMP_H
#define NUVIE_VIEWS_CONTAINER_VIEW_GUMP_H

#include "ultima/nuvie/views/draggable_view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Actor;
class Font;
class U6Bmp;
class Font;
class ContainerWidgetGump;

class ContainerViewGump : public DraggableView {

	GUI_Button *gump_button;
	GUI_Button *up_arrow_button;
	GUI_Button *down_arrow_button;
	GUI_Button *left_arrow_button;
	GUI_Button *right_arrow_button;
	GUI_Button *doll_button;

	ContainerWidgetGump *container_widget;
	uint16 container_widget_y_offset;

	Font *font;

	Actor *actor;
	Obj *container_obj;

public:
	ContainerViewGump(Configuration *cfg);
	~ContainerViewGump() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om, Obj *container_obj_type);

	void Display(bool full_redraw) override;

	void set_actor(Actor *a);
	Actor *get_actor() {
		return actor;
	}
	void set_container_obj(Obj *o);
	Obj *get_container_obj() {
		return container_obj;
	}

	bool is_actor_container() {
		return (container_obj == NULL);
	}

	GUI_status KeyDown(const Common::KeyState &key) override;

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override {
		return DraggableView::MouseMotion(x, y, state);
	}
	GUI_status MouseWheel(sint32 x, sint32 y) override;
	void MoveRelative(int dx, int dy) override {
		return DraggableView::MoveRelative(dx, dy);
	}


	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
protected:

	void init_container_type(Std::string datadir, Obj *obj_type);
	void init_backpack(Std::string datadir, bool extend_area_w);
	void init_chest(Std::string datadir);
	void init_crate(Std::string datadir);
	void init_barrel(Std::string datadir);
	void init_corpse(Std::string datadir, Std::string bg_filename);
	void display_inventory_weight();

	void left_arrow();
	void right_arrow();

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
