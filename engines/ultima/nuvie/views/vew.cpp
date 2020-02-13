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
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/view.h"
#include "ultima/nuvie/actors/actor.h"

#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/gui/gui_button.h"

namespace Ultima {
namespace Nuvie {

View::View(Configuration *cfg) : GUI_Widget(NULL, 0, 0, 0, 0) {
	config = cfg;
	new_ui_mode = false;
	left_button = NULL;
	font = NULL;
	tile_manager = NULL;
	right_button = NULL;
	obj_manager = NULL;
	party = NULL;
	party_button = NULL;
	inventory_button = NULL;
	actor_button = NULL;
	bg_color = 0;
	cur_party_member = 0;
}

View::~View() {
}

bool View::init(uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om) {
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		GUI_Widget::Init(NULL, x, y, 136, 96);
	else if (Game::get_game()->get_game_type() == NUVIE_GAME_SE)
		GUI_Widget::Init(NULL, x + 7, y - 2, 132, 113);
	else
		GUI_Widget::Init(NULL, x + 8, y - 4, 128, 118);
	Hide();
	font = f;
	party = p;
	tile_manager = tm;
	obj_manager = om;

	set_party_member(0);

	bg_color = Game::get_game()->get_palette()->get_bg_color();


	new_ui_mode = Game::get_game()->is_new_style();


	return true;
}

bool View::set_party_member(uint8 party_member) {
	uint16 size = party->get_party_size();

	if (party_member < size) {
		cur_party_member = party_member;

		if (left_button && right_button) {
			if (party_member == 0)
				left_button->Hide();
			else
				left_button->Show();

			if (party_member == size - 1)
				right_button->Hide();
			else
				right_button->Show();
		}

		Redraw();
		return true;
	}

	return false;
}

bool View::next_party_member() {
	if ((cur_party_member + 1) < party->get_party_size())
		return set_party_member(cur_party_member + 1);

	return false;
}

bool View::prev_party_member() {
	if (cur_party_member != 0)
		return set_party_member(cur_party_member - 1);

	return false;
}

void View::fill_md_background(uint8 color, const Common::Rect &r) {
	Screen *scr = Game::get_game()->get_screen();
	scr->fill(color, r.left + 1, r.top + 9, 1, 1); // left pillar
	scr->fill(color, r.left + 2, r.top + 6, 1, 4); // left pillar
	scr->fill(color, r.left + 3, r.top + 3, 1, 11); // left pillar
	scr->fill(color, r.left + r.width() - 2, r.top + 9, 1, 1); // right pillar
	scr->fill(color, r.left + r.width() - 3, r.top + 6, 1, 4); // right pillar
	scr->fill(color, r.left + r.width() - 4, r.top + 3, 1, 11); // right pillar
	scr->fill(color, r.left + 4, r.top, r.width() - 8, 15); // top center
	scr->fill(color, r.left, r.top + 14, r.width(), r.height() - 14); // bottom
	//scr->fill(15, r.left, r.top, r.width(), r.height()); // bottom

}

void View::set_combat_mode(Actor *actor) {
	uint8 combat_mode = actor->get_combat_mode();
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
		combat_mode++;
		if (combat_mode > ACTOR_WT_ASSAULT)
			combat_mode = ACTOR_WT_PLAYER;
	} else {
		if (combat_mode == ACTOR_WT_PLAYER)
			combat_mode = ACTOR_WT_RANGED;
		else if (combat_mode == ACTOR_WT_RANGED)
			combat_mode = ACTOR_WT_RETREAT;
		else if (combat_mode == ACTOR_WT_RETREAT)
			combat_mode = ACTOR_WT_ASSAULT;
		else if (combat_mode == ACTOR_WT_ASSAULT)
			combat_mode = ACTOR_WT_PLAYER;
	}
	actor->set_combat_mode(combat_mode);
}

uint8 View::get_combat_mode_index(Actor *actor) {
	uint8 combat_mode = actor->get_combat_mode();
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		return (combat_mode - 2);
	else {
		uint8 combat_mode_index = 0;
		if (combat_mode == ACTOR_WT_PLAYER)
			combat_mode_index = 0;
		else if (combat_mode == ACTOR_WT_RANGED)
			combat_mode_index = 1;
		else if (combat_mode == ACTOR_WT_RETREAT)
			combat_mode_index = 2;
		else if (combat_mode == ACTOR_WT_ASSAULT)
			combat_mode_index = 3;

		return combat_mode_index;
	}
}

GUI_status View::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	ViewManager *view_manager;

	if (caller == left_button) {
		prev_party_member();
		return GUI_YUM;
	}

	if (caller == right_button) {
		next_party_member();
		return GUI_YUM;
	}

	if (caller == actor_button) {
		view_manager = (ViewManager *)data;
		view_manager->set_actor_mode();
		return GUI_YUM;
	}

	if (caller == party_button) {
		view_manager = (ViewManager *)data;
		view_manager->set_party_mode();
		return GUI_YUM;
	}

	if (caller == inventory_button) {
		view_manager = (ViewManager *)data;
		view_manager->set_inventory_mode();
		return GUI_YUM;
	}

	return GUI_PASS;
}

GUI_Button *View::loadButton(Std::string dir, Std::string name, uint16 x, uint16 y) {
	GUI_Button *button;
	Std::string imagefile;
	Std::string path;

	Graphics::ManagedSurface *image, *image1;
	build_path(dir, name + "_btn_up.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(dir, name + "_btn_down.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	button = new GUI_Button(NULL, x, y, image, image1, this);
	this->AddWidget(button);
	return button;
}

} // End of namespace Nuvie
} // End of namespace Ultima
