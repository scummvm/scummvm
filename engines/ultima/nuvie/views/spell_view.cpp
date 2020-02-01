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
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/u6_bmp.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/views/doll_widget.h"
#include "ultima/nuvie/views/inventory_widget.h"
#include "ultima/nuvie/views/spell_view.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/core/magic.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

static const char circle_num_tbl[][8] = {"1ST", "2ND", "3RD", "4TH", "5TH", "6TH", "7TH", "8TH"};
static const int obj_n_reagent[8] = {OBJ_U6_MANDRAKE_ROOT, OBJ_U6_NIGHTSHADE, OBJ_U6_BLACK_PEARL, OBJ_U6_BLOOD_MOSS, OBJ_U6_SPIDER_SILK, OBJ_U6_GARLIC, OBJ_U6_GINSENG, OBJ_U6_SULFUROUS_ASH};

#define NEWMAGIC_BMP_W 144
#define NEWMAGIC_BMP_H 82

SpellView::SpellView(Configuration *cfg) : DraggableView(cfg) {
	spell_container = NULL;
	background = NULL;
	level = 1;
	all_spells_mode = false;
	spell_num = 0;
	event_mode = false;
	num_spells_per_page = 8;
	caster = NULL;
}

SpellView::~SpellView() {
	if (background) {
		delete background;
	}
}

bool SpellView::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om) {
	View::init(x, y, f, p, tm, om);

	SetRect(area.left, area.top, NEWMAGIC_BMP_W, NEWMAGIC_BMP_H + 16);
	string filename;

	config_get_path(config, "newmagic.bmp", filename);
	background = new U6Bmp();
	if (background->load(filename) == false)
		return false;

	add_command_icons(tmp_screen, view_manager);

	return true;
}


void SpellView::PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y) {
	GUI_Widget::PlaceOnScreen(s, dm, x, y);
}

void SpellView::set_spell_caster(Actor *actor, Obj *s_container, bool eventMode) {
	caster = actor;
	spell_container = s_container;
	event_mode = eventMode;

	for (int shift = 0; shift < 8; shift++) {
		caster_reagents[shift] = caster->inventory_count_object(obj_n_reagent[shift]);
	}

	level = (spell_container->quality / 16) + 1;
	spell_num = spell_container->quality - (16 * level);


	if (Game::get_game()->has_unlimited_casting() || spell_container->find_in_container(OBJ_U6_SPELL, MAGIC_ALL_SPELLS, OBJ_MATCH_QUALITY))
		all_spells_mode = true;
	else
		all_spells_mode = false;

	fill_cur_spell_list();
	update_buttons();
	Game::get_game()->set_mouse_pointer(1); // crosshairs
}

void SpellView::Display(bool full_redraw) {
	if (full_redraw || update_display) {
		screen->fill(bg_color, area.left, area.top + NEWMAGIC_BMP_H, area.width(), area.height() - NEWMAGIC_BMP_H);

		screen->blit(area.left, area.top, background->get_data(), 8,  NEWMAGIC_BMP_W, NEWMAGIC_BMP_H, NEWMAGIC_BMP_W, true);
	}

	display_level_text();
	display_spell_list_text();

	DisplayChildren(full_redraw);
#if 1 // FIXME: This shouldn't need to be in the loop
	update_buttons(); // It doesn't seem to hurt speed though
	screen->update(area.left, area.top, area.width(), area.height());
#else
	if (full_redraw || update_display) {
		update_display = false;
		screen->update(area.left, area.top, area.width(), area.height());
	}
#endif
	return;
}

uint8 SpellView::fill_cur_spell_list() {
	Magic *m = Game::get_game()->get_magic();
	int j = 0;
	for (int i = 0; i < 16; i++) {
		cur_spells[i] = -1;

		if (m->get_spell((level - 1) * 16 + i) != NULL && (all_spells_mode || spell_container->find_in_container(OBJ_U6_SPELL, (level - 1) * 16 + i, OBJ_MATCH_QUALITY)))
			cur_spells[j++] = (level - 1) * 16 + i;
	}

	return j;
}

sint8 SpellView::get_selected_index() {
	for (uint8 i = 0; i < 16; i++) {
		if (cur_spells[i] == spell_container->quality) {
			return (sint8)i;
		}
	}

	return -1;
}

void SpellView::set_prev_level() {
	if (level == 1)
		return;

	uint8 old_level = level;


	uint8 num_spells = 0;
	for (; num_spells == 0;) {
		level--;

		if (level == 0)
			break;

		num_spells = fill_cur_spell_list();
	}

	if (num_spells == 0) {
		level = old_level;
		fill_cur_spell_list();
	}

	if (num_spells > num_spells_per_page)
		spell_container->quality = cur_spells[num_spells_per_page];
	else {
		spell_container->quality = cur_spells[0];
	}
}

void SpellView::set_next_level() {
	if (level == 8)
		return;

	uint8 old_level = level;


	uint8 num_spells = 0;
	for (; num_spells == 0;) {
		level++;

		if (level == 9)
			break;

		num_spells = fill_cur_spell_list();
	}

	if (num_spells == 0) {
		level = old_level;
		fill_cur_spell_list();
	} else
		spell_container->quality = cur_spells[0];
}

void SpellView::move_left() {
	sint8 index = get_selected_index();
	if (index < 0)
		index = 0;

	if (index >= num_spells_per_page) {
		spell_container->quality = cur_spells[0];
	} else {
		set_prev_level();
	}

	update_buttons();
	update_display = true;
}

void SpellView::move_right() {
	sint8 index = get_selected_index();
	if (index < 0)
		index = 0;

	if (index >= num_spells_per_page || cur_spells[num_spells_per_page] == -1) {
		set_next_level();
	} else {
		spell_container->quality = cur_spells[num_spells_per_page];
	}

	update_buttons();
	update_display = true;
}

GUI_status SpellView::move_up() {
	sint8 index = get_selected_index();

	if (index > 0 && index != num_spells_per_page) {
		spell_container->quality = cur_spells[index - 1];
		update_display = true;
	} else
		move_left();
	return GUI_YUM;
}

GUI_status SpellView::move_down() {
	sint8 index = get_selected_index();

	if (index != -1 && index < 15 && index != (num_spells_per_page - 1)) {
		if (cur_spells[index + 1] != -1) {
			spell_container->quality = cur_spells[index + 1];
			update_display = true;
		} else
			move_right();
	} else
		move_right();
	return GUI_YUM;
}

void SpellView::display_level_text() {
	font->drawString(screen, circle_num_tbl[level - 1], area.left + 96 + 8, area.top + NEWMAGIC_BMP_H);
	font->drawString(screen, "level", area.left + 96, area.top + NEWMAGIC_BMP_H + 8);
	return;
}

void SpellView::display_spell_list_text() {
	Magic *m = Game::get_game()->get_magic();

	sint8 index = get_selected_index();

	if (index >= num_spells_per_page)
		index = num_spells_per_page;
	else
		index = 0;

	for (uint8 i = 0; i < num_spells_per_page; i++) {
		sint16 spellNum = cur_spells[i + index];
		if (spellNum != -1) {
			Spell *spell = m->get_spell((uint8)spellNum);

			display_spell_text(spell, i, spell_container->quality);
		}
	}
}

void SpellView::display_spell_text(Spell *spell, uint16 line_num, uint8 selected_spell) {
	char num_str[4];
	line_num++;

	font->drawString(screen, spell->name, area.left + 16, area.top + (line_num * 8));
	snprintf(num_str, 3, "%d", get_available_spell_count(spell));
	font->drawString(screen, num_str, area.left + NEWMAGIC_BMP_W - 24, area.top + (line_num * 8));

	if (spell->num == selected_spell)
		font->drawChar(screen, 26, area.left + 8, area.top + (line_num * 8));

	return;
}

uint16 SpellView::get_available_spell_count(Spell *s) {
	if (s->reagents == 0) // Help and Armageddon
		return 1;
	if (Game::get_game()->has_unlimited_casting())
		return 99;
	sint32 min_reagents = -1;
	for (int shift = 0; shift < 8; shift++) {
		if (1 << shift & s->reagents) {
			if (min_reagents == -1 || caster_reagents[shift] < min_reagents)
				min_reagents = caster_reagents[shift];
		}
	}

	if (min_reagents == -1)
		min_reagents = 0;

	return (uint16)min_reagents;
}

void SpellView::add_command_icons(Screen *tmp_screen, void *view_manager) {
	Tile *tile;
	Graphics::ManagedSurface *button_image;
	Graphics::ManagedSurface *button_image2;

	tile = tile_manager->get_tile(412); //left arrow icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	left_button = new GUI_Button(this, 2 * 16, NEWMAGIC_BMP_H, button_image, button_image2, this);
	this->AddWidget(left_button);

	tile = tile_manager->get_tile(413); //right arrow icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	right_button = new GUI_Button(this, 3 * 16, NEWMAGIC_BMP_H, button_image, button_image2, this);
	this->AddWidget(right_button);

}

void SpellView::event_mode_select_spell() {
	sint16 spellNum = get_selected_spell();
	Game::get_game()->get_event()->select_spell_num(spellNum);
	release_focus();
}

/* Move the cursor around
 */
GUI_status SpellView::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case NORTH_KEY:
		return move_up();
	case SOUTH_KEY:
		return move_down();
	case WEST_KEY:
	case PREVIOUS_PARTY_MEMBER_KEY:
		move_left();
		break;
	case EAST_KEY:
	case NEXT_PARTY_MEMBER_KEY:
		move_right();
		break;
	case HOME_KEY:
// TODO - add going to first viable page
		break;
	case END_KEY:
// TODO - add going to last viable page
		break;
	case DO_ACTION_KEY:
		if (Game::get_game()->get_event()->is_looking_at_spellbook()) {
			show_spell_description();
			return GUI_YUM;
		}
		if (event_mode) {
			event_mode_select_spell();
			return GUI_YUM;
		}

		return GUI_PASS;
	case CANCEL_ACTION_KEY:
		return cancel_spell();
	case TOGGLE_CURSOR_KEY :

		break;

	default:
		return GUI_PASS;
	}
	return (GUI_YUM);
}

GUI_status SpellView::cancel_spell() {
	Events *event = Game::get_game()->get_event();
	if (event->is_looking_at_spellbook()) {
		close_look();
		return GUI_YUM;
	} else if (event_mode) {
		event->select_spell_num(-1);
		release_focus();
		return GUI_YUM;
	}
	event->set_mode(CAST_MODE);
	event->cancelAction();
	return GUI_YUM;
}

GUI_status SpellView::MouseWheel(sint32 x, sint32 y) {
	if (y > 0)
		return move_up();
	if (y < 0)
		return move_down();

	return GUI_YUM;
}

GUI_status SpellView::MouseDown(int x, int y, Shared::MouseButton button) {
	y -= area.top;
	x -= area.left;
	Events *event = Game::get_game()->get_event();
	bool selecting_spell_target, canceling_spell, doing_nothing;
	if (Game::get_game()->is_original_plus()) {
		if (Game::get_game()->is_original_plus_full_map())
			selecting_spell_target = (x < -7 || y > 194);
		else
			selecting_spell_target = (x < -7);
		canceling_spell = (x > 1 && (y > 101 || x > 137));
		doing_nothing = ((x > -8 && x < 16) || (x > -8 && (y < 8 || (y > 71 && y < 195))));
	} else {
		selecting_spell_target = (x < 0 && y > 0 && y < 162);
		canceling_spell = (x > 1 && (y > 101 || x > 137));
		doing_nothing = (y < 8 || y > 71 || x < 16 || x > 134);
	}

	if (button == Shared::BUTTON_RIGHT)
		return cancel_spell();

	if (selecting_spell_target) { // cast selected spell on the map
		if (event->is_looking_at_spellbook()) {
			close_look();
			return GUI_YUM;
		}

		event->target_spell();
		if (event->get_mode() == INPUT_MODE) {
			y += area.top;
			x += area.left;
			Game::get_game()->get_map_window()->select_target(x, y);
		}
		return GUI_YUM;
	}
	if (canceling_spell) // cancel spell
		return cancel_spell();
	if (doing_nothing) // do nothing
		return GUI_YUM;
// selecting spell index

	sint8 index = get_selected_index();

	if (index >= num_spells_per_page)
		index = num_spells_per_page;
	else
		index = 0;
	y = (y / num_spells_per_page) - 1;
	//printf("x = %d, y = %d index=%d\n", x, y, index);

	if (cur_spells[index + y] != -1) {
		spell_container->quality = cur_spells[index + y];
		update_display = true;
		if (event->is_looking_at_spellbook())
			show_spell_description();
		else if (event_mode)
			event_mode_select_spell();
		else
			Game::get_game()->get_event()->target_spell();
	}

	return GUI_YUM;
}

void SpellView::hide_buttons() {
	if (left_button) left_button->Hide();
	if (right_button) right_button->Hide();
}

void SpellView::show_buttons() {
	if (left_button) left_button->Show();
	if (right_button) right_button->Show();
}

void SpellView::update_buttons() {
	show_buttons();
	sint8 index = get_selected_index();

	if (level == 1 && index <= (num_spells_per_page - 1) && left_button)
		left_button->Hide();

	uint8 old_level = level;
	uint8 num_spells = 0;
	for (; num_spells == 0;) {
		level++;
		if (level == 9)
			break;
		num_spells = fill_cur_spell_list();
	}
	level = old_level;
	fill_cur_spell_list();

	if (right_button && ((level < 8 && num_spells == 0) || level == 8)
	        && cur_spells[num_spells_per_page * (1 + index / num_spells_per_page)] == -1)
		right_button->Hide();
}

void SpellView::close_look() {
	Game::get_game()->get_event()->set_looking_at_spellbook(false);
	Game::get_game()->get_scroll()->display_prompt();
	Game::get_game()->get_view_manager()->close_spell_mode();
	Game::get_game()->get_event()->endAction();
}

void SpellView::show_spell_description() {
	if (get_selected_index() != -1) {
		uint8 index = (level - 1) * 16 + get_selected_index();
		Game::get_game()->get_magic()->show_spell_description(index);
	}
	close_look();
}

GUI_status SpellView::callback(uint16 msg, GUI_CallBack *caller, void *data) {

	if (caller == left_button) {
		move_left();
		return GUI_YUM;
	} else if (caller == right_button) {
		move_right();
		return GUI_YUM;
	}

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
