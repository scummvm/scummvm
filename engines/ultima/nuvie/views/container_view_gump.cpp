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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/fonts/font_manager.h"
#include "ultima/nuvie/views/container_widget_gump.h"
#include "ultima/nuvie/views/container_view_gump.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

#define CONTAINER_WIDGET_OFFSET 29
#define CHECK_X 0

ContainerViewGump::ContainerViewGump(Configuration *cfg) : DraggableView(cfg) {
	bg_image = NULL;
	gump_button = NULL;
	up_arrow_button = NULL;
	down_arrow_button = NULL;
	doll_button = NULL;
	left_arrow_button = NULL;
	right_arrow_button = NULL;
	container_widget = NULL;
	font = NULL;
	actor = NULL;
	container_obj = NULL;
}

ContainerViewGump::~ContainerViewGump() {
}

bool ContainerViewGump::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om, Obj *container_obj_type) {
	View::init(x, y, f, p, tm, om);

	//actor = p->get_actor(p->get_leader()); don't have party leader as default, get owner of container obj or leave NULL (moved to init_container_type)

	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string imagefile;
	Std::string path;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;

	init_container_type(datadir, container_obj_type);

	set_bg_color_key(0, 0x70, 0xfc);

	//font = new GUI_Font(GUI_FONT_GUMP);
	//font->setColoring( 0x08, 0x08, 0x08, 0x80, 0x58, 0x30, 0x00, 0x00, 0x00);
	font = f;

	return true;
}

void ContainerViewGump::init_container_type(Std::string datadir, Obj *obj_type) {


	if (obj_type != NULL) {
		if (obj_type->is_in_inventory())
			actor = obj_type->get_actor_holding_obj();
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
			if (obj_type->obj_n == OBJ_U6_CHEST)
				return init_chest(datadir);
			else if (obj_type->obj_n == OBJ_U6_CRATE)
				return init_crate(datadir);
			else if (obj_type->obj_n == OBJ_U6_BARREL)
				return init_barrel(datadir);
			else if (obj_type->obj_n == OBJ_U6_DEAD_GARGOYLE)
				return init_corpse(datadir, "corpse_gargoyle_bg.bmp");
			else if (obj_type->obj_n == OBJ_U6_DEAD_BODY
			         || obj_type->obj_n == OBJ_U6_GRAVE || obj_type->obj_n == OBJ_U6_REMAINS)
				return init_corpse(datadir, "corpse_body_bg.bmp");
			else if (obj_type->obj_n == OBJ_U6_DEAD_CYCLOPS)
				return init_corpse(datadir, "corpse_cyclops_bg.bmp");
			else if (obj_type->obj_n == OBJ_U6_DEAD_ANIMAL || obj_type->obj_n == OBJ_U6_MOUSE
			         || obj_type->obj_n == OBJ_U6_MONGBAT || obj_type->obj_n == OBJ_U6_DRAKE
			         || obj_type->obj_n == OBJ_U6_REAPER)
				return init_corpse(datadir, "corpse_animal_bg.bmp");
		}
	}

	return init_backpack(datadir, obj_type ? !obj_type->is_in_inventory() : true);
}

void ContainerViewGump::init_backpack(Std::string datadir, bool extend_area_w) {
	Std::string imagefile, path;
	uint8 check_y = 27;
	gump_button = loadButton(datadir, "gump", CHECK_X, check_y);

	build_path(datadir, "container", path);
	datadir = path;

	up_arrow_button = loadButton(datadir, "cont_up", 83, 35);
	down_arrow_button = loadButton(datadir, "cont_down", 83, 66);

	build_path(datadir, "backpack_bg.bmp", imagefile);

	bg_image = SDL_LoadBMP(imagefile.c_str());

	doll_button = loadButton(datadir, "cont_doll", area.left + 18, area.top + bg_image->h);
	left_arrow_button = loadButton(datadir, "cont_left", area.left + 18 + 11, area.top + bg_image->h);
	right_arrow_button = loadButton(datadir, "cont_right", area.left + 18 + 22, area.top + bg_image->h);

	SetRect(area.left, area.top, bg_image->w, bg_image->h + 16); //111, 101);

	container_widget = new ContainerWidgetGump(config, this);
	container_widget_y_offset = CONTAINER_WIDGET_OFFSET;
	container_widget->init(actor, 21, container_widget_y_offset, 4, 3, tile_manager, obj_manager, font, CHECK_X, check_y);

	AddWidget(container_widget);
	if (extend_area_w) // text extends beyond the gump
		area.right += 4;
}

void ContainerViewGump::init_chest(Std::string datadir) {
	Std::string imagefile, path;
	uint8 check_y = 56;
	gump_button = loadButton(datadir, "gump", CHECK_X, check_y);

	build_path(datadir, "container", path);
	datadir = path;

	up_arrow_button = loadButton(datadir, "cont_up", 85, 31);
	down_arrow_button = loadButton(datadir, "cont_down", 85, 47);

	build_path(datadir, "chest_bg.bmp", imagefile);

	bg_image = SDL_LoadBMP(imagefile.c_str());

	SetRect(area.left, area.top, bg_image->w, bg_image->h + 16); //111, 101);

	container_widget = new ContainerWidgetGump(config, this);
	container_widget_y_offset = CONTAINER_WIDGET_OFFSET - 1;
	container_widget->init(actor, 21, container_widget_y_offset, 4, 2, tile_manager, obj_manager, font, CHECK_X, check_y);

	AddWidget(container_widget);
}

void ContainerViewGump::init_crate(Std::string datadir) {
	Std::string imagefile, path;
	uint8 check_y = 63;
	gump_button = loadButton(datadir, "gump", CHECK_X, check_y);

	build_path(datadir, "container", path);
	datadir = path;

	up_arrow_button = loadButton(datadir, "cont_up", 100, 15);
	down_arrow_button = loadButton(datadir, "cont_down", 100, 46);

	build_path(datadir, "crate_bg.bmp", imagefile);

	bg_image = SDL_LoadBMP(imagefile.c_str());

	SetRect(area.left, area.top, bg_image->w, bg_image->h);

	container_widget = new ContainerWidgetGump(config, this);
	container_widget_y_offset = 10;
	container_widget->init(actor, 21, container_widget_y_offset, 5, 3, tile_manager, obj_manager, font, CHECK_X, check_y);

	AddWidget(container_widget);
}

void ContainerViewGump::init_barrel(Std::string datadir) {
	Std::string imagefile, path;
	uint8 check_y = 55;
	gump_button = loadButton(datadir, "gump", CHECK_X, check_y);

	build_path(datadir, "container", path);
	datadir = path;

	up_arrow_button = loadButton(datadir, "cont_up", 102, 28);
	down_arrow_button = loadButton(datadir, "cont_down", 102, 42);

	build_path(datadir, "barrel_bg.bmp", imagefile);

	bg_image = SDL_LoadBMP(imagefile.c_str());

	SetRect(area.left, area.top, bg_image->w, bg_image->h);

	container_widget = new ContainerWidgetGump(config, this);
	container_widget_y_offset = 24;
	container_widget->init(actor, 38, container_widget_y_offset, 4, 2, tile_manager, obj_manager, font, CHECK_X, check_y);

	AddWidget(container_widget);
}

void ContainerViewGump::init_corpse(Std::string datadir, Std::string bg_filename) {
	Std::string imagefile, path;
	uint8 check_y = 25;
	gump_button = loadButton(datadir, "gump", CHECK_X, check_y);

	build_path(datadir, "container", path);
	datadir = path;

	up_arrow_button = loadButton(datadir, "cont_up", 67, 28);
	down_arrow_button = loadButton(datadir, "cont_down", 67, 78);

	build_path(datadir, bg_filename, imagefile);

	bg_image = SDL_LoadBMP(imagefile.c_str());

	SetRect(area.left, area.top, bg_image->w, bg_image->h);

	container_widget = new ContainerWidgetGump(config, this);
	container_widget_y_offset = 26;
	container_widget->init(actor, 20, container_widget_y_offset, 3, 4, tile_manager, obj_manager, font, CHECK_X, check_y);

	AddWidget(container_widget);
}
void ContainerViewGump::set_actor(Actor *a) {
	actor = a;
	container_obj = NULL;
	container_widget->set_actor(a);
	if (doll_button)
		doll_button->Show();
	if (party->get_member_num(a) >= 0) {
		if (left_arrow_button)
			left_arrow_button->Show();
		if (right_arrow_button)
			right_arrow_button->Show();
	} else {
		if (left_arrow_button)
			left_arrow_button->Hide();
		if (right_arrow_button)
			right_arrow_button->Hide();
	}
}

void ContainerViewGump::set_container_obj(Obj *o) {
	container_obj = o;
	container_widget->set_container(container_obj);
	if (doll_button)
		doll_button->Hide();
	if (left_arrow_button)
		left_arrow_button->Hide();
	if (right_arrow_button)
		right_arrow_button->Hide();
}

void ContainerViewGump::Display(bool full_redraw) {
//display_level_text();
//display_spell_list_text();
	Common::Rect dst;
	dst = area;
	SDL_BlitSurface(bg_image, NULL, surface, &dst);

	DisplayChildren(full_redraw);

	if (actor) {
		font->drawString(screen, actor->get_name(), area.left + 18, area.top + 2, 15, 15);
		display_inventory_weight();
	}
	update_display = false;
	screen->update(area.left, area.top, area.width(), area.height());

	return;
}

void ContainerViewGump::display_inventory_weight() {
	uint8 strength = actor->get_strength();
	unsigned int equip_weight = Game::get_game()->get_view_manager()->get_display_weight(actor->get_inventory_weight());
	char string[11]; //I:nnn/nnns\0

	snprintf(string, 10, "I:%u/%us", equip_weight, strength * 2);
	font->drawString(screen, string, area.left + (container_obj ? 18 : 18 + 34), area.top + bg_image->h + 2, 15, 15);
}

void ContainerViewGump::left_arrow() {
	sint8 party_mem_num = party->get_member_num(actor);
	if (party_mem_num >= 0) {
		if (party_mem_num > 0)
			party_mem_num--;
		else
			party_mem_num = party->get_party_size() - 1;

		set_actor(party->get_actor(party_mem_num));
		force_full_redraw_if_needed();
	}
}

void ContainerViewGump::right_arrow() {
	set_actor(party->get_actor((party->get_member_num(actor) + 1) % party->get_party_size()));
	force_full_redraw_if_needed();
}

GUI_status ContainerViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	//close gump and return control to Magic class for clean up.
	if (caller == gump_button) {
		Game::get_game()->get_view_manager()->close_gump(this);
		return GUI_YUM;
	} else if (caller == down_arrow_button) {
		container_widget->down_arrow();
		return GUI_YUM;
	} else if (caller == up_arrow_button) {
		container_widget->up_arrow();
		return GUI_YUM;
	} else if (doll_button && caller == doll_button) {
		Game::get_game()->get_view_manager()->open_doll_view(actor);
		return GUI_YUM;
	} else if (left_arrow_button && caller == left_arrow_button) {
		left_arrow();
		return GUI_YUM;
	} else if (right_arrow_button && caller == right_arrow_button) {
		right_arrow();
		return GUI_YUM;
	}

	return GUI_PASS;
}

GUI_status ContainerViewGump::KeyDown(const Common::KeyState &key) {
	if (left_arrow_button && left_arrow_button->Status() == WIDGET_VISIBLE) { // okay to change member number
		KeyBinder *keybinder = Game::get_game()->get_keybinder();
		ActionType a = keybinder->get_ActionType(key);

		switch (keybinder->GetActionKeyType(a)) {
		case NEXT_PARTY_MEMBER_KEY:
			right_arrow();
			return GUI_YUM;
		case PREVIOUS_PARTY_MEMBER_KEY:
			left_arrow();
			return GUI_YUM;
		case HOME_KEY:
			set_actor(party->get_actor(0));
			force_full_redraw_if_needed();
			return GUI_YUM;
		case END_KEY:
			set_actor(party->get_actor(party->get_party_size() - 1));
			force_full_redraw_if_needed();
			return GUI_YUM;
		default:
			break;
		}
	}
	/* moved into container widget
	switch(key.keycode)
	    {
	        case Common::KEYCODE_RETURN:
	        case Common::KEYCODE_KP_ENTER:

	            return GUI_YUM;
	        default:
	            break;
	    }
	*/
	return container_widget->KeyDown(key);
}

GUI_status ContainerViewGump::MouseWheel(sint32 x, sint32 y) {
	int xpos, ypos;
	screen->get_mouse_location(&xpos, &ypos);
	ypos -= area.top;

	if (ypos >= container_widget_y_offset && ypos < container_widget_y_offset + container_widget->H()) {
		if (y > 0) {
			container_widget->up_arrow();
		} else if (y < 0) {
			container_widget->down_arrow();
		}
	} else {
		if (is_actor_container() && party->get_member_num(actor) >= 0) {
			if (y > 0) {
				left_arrow();
			} else if (y < 0) {
				right_arrow();
			}
		}
	}
	return GUI_YUM;
}

GUI_status ContainerViewGump::MouseDown(int x, int y, Shared::MouseButton button) {
	return DraggableView::MouseDown(x, y, button);
}

GUI_status ContainerViewGump::MouseUp(int x, int y, Shared::MouseButton button) {
	return DraggableView::MouseUp(x, y, button);
}

} // End of namespace Nuvie
} // End of namespace Ultima
