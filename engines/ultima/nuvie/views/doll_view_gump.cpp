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
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/container_view_gump.h"
#include "ultima/nuvie/views/doll_widget.h"
#include "ultima/nuvie/views/doll_view_gump.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

DollViewGump::DollViewGump(Configuration *cfg) : DraggableView(cfg),
	gump_button(NULL), combat_button(NULL), heart_button(NULL), party_button(NULL), inventory_button(NULL),
	doll_widget(NULL), font(NULL), actor(NULL), cursor_tile(NULL) {
	bg_image = NULL;
	actor_doll = NULL;
	is_avatar = false;
	show_cursor = true;
	cursor_pos = CURSOR_HEAD;
	cursor_xoff = 50;
	cursor_yoff = 16;
}

DollViewGump::~DollViewGump() {
	if (font)
		delete font;
	if (actor_doll)
		SDL_FreeSurface(actor_doll);
}

bool DollViewGump::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Actor *a, Font *f, Party *p, TileManager *tm, ObjManager *om) {
	View::init(x, y, f, p, tm, om);

	SetRect(area.left, area.top, 108, 136);

	actor = a;
	is_avatar = actor->is_avatar();
	cursor_tile = tile_manager->get_gump_cursor_tile();
	doll_widget = new DollWidget(config, this);
	doll_widget->init(actor, 26, 16, tile_manager, obj_manager);

	AddWidget(doll_widget);

	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string imagefile;
	Std::string path;

	Graphics::ManagedSurface *image, *image1;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;

	gump_button = loadButton(datadir, "gump", 0, 112);

	build_path(datadir, "left_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	left_button = new GUI_Button(this, 23, 7, image, image1, this);
	this->AddWidget(left_button);

	build_path(datadir, "right_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	right_button = new GUI_Button(this, 86, 7, image, image1, this);
	this->AddWidget(right_button);

	build_path(datadir, "doll", path);
	datadir = path;

	build_path(datadir, "doll_bg.bmp", imagefile);
	bg_image = SDL_LoadBMP(imagefile.c_str());

	set_bg_color_key(0, 0x70, 0xfc);

	build_path(datadir, "combat_btn_up.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(datadir, "combat_btn_down.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	combat_button = new GUI_Button(NULL, 23, 92, image, image1, this);
	this->AddWidget(combat_button);

	heart_button = loadButton(datadir, "heart", 23, 108);
	party_button = loadButton(datadir, "party", 47, 108);
	inventory_button = loadButton(datadir, "inventory", 71, 108);

	font = new GUI_Font(GUI_FONT_GUMP);
	font->setColoring(0x08, 0x08, 0x08, 0x80, 0x58, 0x30, 0x00, 0x00, 0x00);

	if (party->get_member_num(actor) < 0) {
		if (Game::get_game()->get_event()->using_control_cheat() == false)
			heart_button->Hide();
		left_button->Hide();
		right_button->Hide();
	}
	party_button->Hide();
	is_avatar = actor->is_avatar();
	ViewManager *vm = Game::get_game()->get_view_manager();
	if (is_avatar)
		actor_doll = vm->loadAvatarDollImage(actor_doll);
	else
		actor_doll = vm->loadCustomActorDollImage(actor_doll, actor->get_actor_num());
	setColorKey(actor_doll);

	return true;
}

void DollViewGump::setColorKey(Graphics::ManagedSurface *image) {
	if (image) {
		bg_color_key = SDL_MapRGB(image->format, 0xf1, 0x0f, 0xc4);
		SDL_SetColorKey(image, SDL_TRUE, bg_color_key);
	}
}

static const char combat_mode_tbl[][8] = {"COMMAND", "FRONT", "REAR", "FLANK", "BERSERK", "RETREAT", "ASSAULT"};
static const char combat_mode_tbl_se[][8] = {"COMMAND", "RANGED", "FLEE", "CLOSE"};
static const char combat_mode_tbl_md[][8] = {"COMMAND", "RANGED", "FLEE", "ATTACK"};

void DollViewGump::set_actor(Actor *a) {
	actor = a;
	if (actor) {
		is_avatar = actor->is_avatar();
		ViewManager *vm = Game::get_game()->get_view_manager();
		if (is_avatar)
			actor_doll = vm->loadAvatarDollImage(actor_doll);
		else
			actor_doll = vm->loadCustomActorDollImage(actor_doll, actor->get_actor_num());
		setColorKey(actor_doll);
	}

	if (doll_widget)
		doll_widget->set_actor(actor);
}

GUI_status DollViewGump::set_cursor_pos(gumpCursorPos pos) {
	cursor_pos = pos;
	switch (cursor_pos) {
	case CURSOR_LEFT:
		cursor_xoff = 18;
		cursor_yoff = 2;
		return GUI_YUM;
	case CURSOR_RIGHT:
		cursor_xoff = 82;
		cursor_yoff = 2;
		return GUI_YUM;
	case CURSOR_HEAD:
		cursor_xoff = 50;
		cursor_yoff = 16;
		return GUI_YUM;
	case CURSOR_NECK:
		cursor_xoff = 26;
		cursor_yoff = 24;
		return GUI_YUM;
	case CURSOR_RIGHT_HAND:
		cursor_xoff = 26;
		cursor_yoff = 40;
		return GUI_YUM;
	case CURSOR_CHEST:
		cursor_xoff = 74;
		cursor_yoff = 24;
		return GUI_YUM;
	case CURSOR_LEFT_HAND:
		cursor_xoff = 74;
		cursor_yoff = 40;
		return GUI_YUM;
	case CURSOR_RIGHT_RING:
		cursor_xoff = 26;
		cursor_yoff = 57;
		return GUI_YUM;
	case CURSOR_LEFT_RING:
		cursor_xoff = 74;
		cursor_yoff = 57;
		return GUI_YUM;
	case CURSOR_FEET:
		cursor_xoff = 50;
		cursor_yoff = 63;
		return GUI_YUM;
	case CURSOR_CHECK:
		cursor_xoff = 1;
		cursor_yoff = 111;
		return GUI_YUM;
	case CURSOR_COMBAT:
		cursor_xoff = 23;
		cursor_yoff = 92;
		return GUI_YUM;
	case CURSOR_HEART:
		cursor_xoff = 26;
		cursor_yoff = 109;
		return GUI_YUM;
	case CURSOR_PARTY:
		cursor_xoff = 50;
		cursor_yoff = 109;
		return GUI_YUM;
	case CURSOR_INVENTORY:
	default :
		cursor_xoff = 74;
		cursor_yoff = 109;
		return GUI_YUM;
	}
}

void DollViewGump::Display(bool full_redraw) {
//display_level_text();
//display_spell_list_text();
	Common::Rect dst;
	dst = area;
	dst.setWidth(108);
	dst.setHeight(136);
	SDL_BlitSurface(bg_image, NULL, surface, &dst);

	if (actor_doll) {
		dst.translate(45, 32);
		SDL_BlitSurface(actor_doll, NULL, surface, &dst);
	}

	uint8 w = font->getCenter(actor->get_name(), 58);
	font->textOut(screen->get_sdl_surface(), area.left + 29 + w, area.top + 7, actor->get_name());

	displayEquipWeight();

	DisplayChildren(full_redraw);
	displayCombatMode();
	if (show_cursor)
		screen->blit(area.left + cursor_xoff, area.top + cursor_yoff,
			(const unsigned char *)cursor_tile->data, 8, 16, 16, 16, true);
	update_display = false;
	screen->update(area.left, area.top, area.width(), area.height());

	return;
}

void DollViewGump::displayEquipWeight() {
	uint8 strength = actor->get_strength();
	unsigned int equip_weight = Game::get_game()->get_view_manager()->get_display_weight(actor->get_inventory_equip_weight());
	char string[4]; //nnn\0

	snprintf(string, 4, "%u", equip_weight);
	font->textOut(screen->get_sdl_surface(), area.left + ((equip_weight > 9) ? 59 : 64), area.top + 82, string);

	snprintf(string, 4, "%u", strength);
	font->textOut(screen->get_sdl_surface(), area.left + ((strength > 9) ? 76 : 81), area.top + 82, string);
}

void DollViewGump::displayCombatMode() {
	if (!actor->is_in_party() || party->get_member_num(actor) == 0)
		return;
	uint8 index = get_combat_mode_index(actor);
	const char *text;
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		text = combat_mode_tbl[index];
	else if (Game::get_game()->get_game_type() == NUVIE_GAME_MD)
		text = combat_mode_tbl_md[index];
	else // SE
		text = combat_mode_tbl_se[index];
	uint8 c = font->getCenter(text, 55);
	font->textOut(screen->get_sdl_surface(), area.left + 36 + c, area.top + 97, text);
}

void DollViewGump::left_arrow() {
	if (party->get_member_num(actor) < 0)
		return;
	uint8 party_mem_num = party->get_member_num(actor);
	if (party_mem_num > 0)
		party_mem_num--;
	else
		party_mem_num = party->get_party_size() - 1;

	set_actor(party->get_actor(party_mem_num));
}

void DollViewGump::right_arrow() {
	if (party->get_member_num(actor) < 0)
		return;
	set_actor(party->get_actor((party->get_member_num(actor) + 1) % party->get_party_size()));
}

GUI_status DollViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	Events *event = Game::get_game()->get_event();
	//close gump and return control to Magic class for clean up.
	if (event->get_mode() == ATTACK_MODE || caller == gump_button) {
		Game::get_game()->get_view_manager()->close_gump(this);
		return GUI_YUM;
	} else if (caller == right_button) {
		right_arrow();
	} else if (caller == left_button) {
		left_arrow();
	} else if (caller == inventory_button) {
		Game::get_game()->get_view_manager()->open_container_view(actor);
	} else if (caller == heart_button) {
		Game::get_game()->get_view_manager()->open_portrait_gump(actor);
	} else if (caller == combat_button) {
		activate_combat_button();
	} else if (caller == party_button) { // FIXME: What is this supposed to do?

	} else if (caller == doll_widget) {
		if (event->get_mode() != MOVE_MODE && event->get_mode() != EQUIP_MODE) {
			Obj *obj = (Obj *)data;
			event->select_view_obj(obj, actor);
			return GUI_YUM;
		}
	}

	return GUI_PASS;
}

GUI_status DollViewGump::moveCursorRelative(uint8 direction) {
	gumpCursorPos cursor_left = actor->is_in_party() ? CURSOR_LEFT : CURSOR_HEAD; // don't allow pickpocket or control cheat into arrow area
	gumpCursorPos cursor_right = actor->is_in_party() ? CURSOR_RIGHT : CURSOR_HEAD;
	gumpCursorPos cursor_party; // no party button yet so skip it
	gumpCursorPos cursor_heart; // not available in pickpocket mode

	if (!actor->is_in_party() && !Game::get_game()->get_event()->using_control_cheat()) {
		if (direction == NUVIE_DIR_SW || direction == NUVIE_DIR_W)
			cursor_heart = CURSOR_CHECK;
		else
			cursor_heart = CURSOR_INVENTORY;
	} else
		cursor_heart = CURSOR_HEART;

	if (direction == NUVIE_DIR_W || direction == NUVIE_DIR_SW)
		cursor_party = cursor_heart;
	else
		cursor_party = CURSOR_INVENTORY;

	switch (cursor_pos) {
	case CURSOR_LEFT:
		switch (direction) {
		case NUVIE_DIR_NE:
		case NUVIE_DIR_E:
			return set_cursor_pos(CURSOR_RIGHT);
		case NUVIE_DIR_SW:
		case NUVIE_DIR_S:
			return set_cursor_pos(CURSOR_NECK);
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_HEAD);
		default:
			return GUI_YUM;
		}
	case CURSOR_RIGHT:
		switch (direction) {
		case NUVIE_DIR_W:
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_LEFT);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_CHEST);
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_HEAD);
		default:
			return GUI_YUM;
		}
	case CURSOR_HEAD:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
			return set_cursor_pos(cursor_left);
		case NUVIE_DIR_NE:
			return set_cursor_pos(cursor_right);
		case NUVIE_DIR_W:
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_NECK);
		case NUVIE_DIR_E:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_CHEST);
		case NUVIE_DIR_S:
			return set_cursor_pos(CURSOR_FEET);
		default:
			return GUI_YUM;
		}
	case CURSOR_NECK:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
			return set_cursor_pos(cursor_left);
		case NUVIE_DIR_E:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_HEAD);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_RIGHT_HAND);
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_LEFT_HAND);
		default:
			return GUI_YUM;
		}
	case CURSOR_CHEST:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NE:
			return set_cursor_pos(cursor_right);
		case NUVIE_DIR_W:
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_HEAD);
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_RIGHT_HAND);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_LEFT_HAND);
		default:
			return GUI_YUM;
		}
	case CURSOR_RIGHT_HAND:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_NECK);
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_CHEST);
		case NUVIE_DIR_E:
			return set_cursor_pos(CURSOR_LEFT_HAND);
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_FEET);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_RIGHT_RING);
		default:
			return GUI_YUM;
		}
	case CURSOR_LEFT_HAND:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_CHEST);
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_HEAD);
		case NUVIE_DIR_W:
			return set_cursor_pos(CURSOR_RIGHT_HAND);
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_FEET);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_LEFT_RING);
		default:
			return GUI_YUM;
		}
	case CURSOR_RIGHT_RING:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_RIGHT_HAND);
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_LEFT_HAND);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_FEET);
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_COMBAT);
		case NUVIE_DIR_E:
			return set_cursor_pos(CURSOR_LEFT_RING);
		default:
			return GUI_YUM;
		}
	case CURSOR_LEFT_RING:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_LEFT_HAND);
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_RIGHT_HAND);
		case NUVIE_DIR_W:
			return set_cursor_pos(CURSOR_RIGHT_RING);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_FEET);
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_COMBAT);
		default:
			return GUI_YUM;
		}
	case CURSOR_FEET:
		switch (direction) {
		case NUVIE_DIR_N:
			return set_cursor_pos(CURSOR_HEAD);
		case NUVIE_DIR_W:
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_RIGHT_RING);
		case NUVIE_DIR_E:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_LEFT_RING);
		case NUVIE_DIR_S:
		case NUVIE_DIR_SW:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_COMBAT);
		default:
			return GUI_YUM;
		}
	case CURSOR_COMBAT:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
			return set_cursor_pos(CURSOR_RIGHT_RING);
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_FEET);
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_CHECK);
		case NUVIE_DIR_SE:
			return set_cursor_pos(cursor_party);
		case NUVIE_DIR_S:
			return set_cursor_pos(cursor_heart);
		default:
			return GUI_YUM;
		}
	case CURSOR_CHECK:
		switch (direction) {
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_COMBAT);
		case NUVIE_DIR_E:
		case NUVIE_DIR_SE:
			return set_cursor_pos(cursor_heart);
		default:
			return GUI_YUM;
		}
	case CURSOR_HEART:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_COMBAT);
		case NUVIE_DIR_W:
		case NUVIE_DIR_SW:
			return set_cursor_pos(CURSOR_CHECK);
		case NUVIE_DIR_E:
		case NUVIE_DIR_SE:
			return set_cursor_pos(cursor_party);
		default:
			return GUI_YUM;
		}
	case CURSOR_PARTY:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_COMBAT);
		case NUVIE_DIR_W:
		case NUVIE_DIR_SW:
			return set_cursor_pos(cursor_heart);
		case NUVIE_DIR_E:
		case NUVIE_DIR_SE:
			return set_cursor_pos(CURSOR_INVENTORY);
		default:
			return GUI_YUM;
		}
	case CURSOR_INVENTORY:
		switch (direction) {
		case NUVIE_DIR_N:
		case NUVIE_DIR_NW:
		case NUVIE_DIR_NE:
			return set_cursor_pos(CURSOR_COMBAT);
		case NUVIE_DIR_W:
		case NUVIE_DIR_SW:
			return set_cursor_pos(cursor_party);
		default:
			return GUI_YUM;
		}
	default:
		return GUI_YUM;
	}
}

GUI_status DollViewGump::KeyDown(const Common::KeyState &key) {
// I was restricting numpad keys when in numlock but there shouldn't be any needed number input
//	bool numlock = (key.flags & Common::KBD_NUM); // SDL doesn't get the proper num lock state in Windows
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case SOUTH_WEST_KEY:
		return moveCursorRelative(NUVIE_DIR_SW);
	case SOUTH_EAST_KEY:
		return moveCursorRelative(NUVIE_DIR_SE);
	case NORTH_WEST_KEY:
		return moveCursorRelative(NUVIE_DIR_NW);
	case NORTH_EAST_KEY:
		return moveCursorRelative(NUVIE_DIR_NE);
	case NORTH_KEY:
		return moveCursorRelative(NUVIE_DIR_N);
	case SOUTH_KEY:
		return moveCursorRelative(NUVIE_DIR_S);
	case WEST_KEY:
		return moveCursorRelative(NUVIE_DIR_W);
	case EAST_KEY:
		return moveCursorRelative(NUVIE_DIR_E);
	case NEXT_PARTY_MEMBER_KEY:
		right_arrow();
		return GUI_YUM;
	case PREVIOUS_PARTY_MEMBER_KEY:
		left_arrow();
		return GUI_YUM;
	case HOME_KEY:
		set_actor(party->get_actor(0));
		return GUI_YUM;
	case END_KEY:
		set_actor(party->get_actor(party->get_party_size() - 1));
		return GUI_YUM;
	case DO_ACTION_KEY: {
		Events *event = Game::get_game()->get_event();
		bool in_party = party->get_member_num(actor) >= 0;
		if (event->get_mode() == ATTACK_MODE || cursor_pos == CURSOR_CHECK) {
			Game::get_game()->get_view_manager()->close_gump(this);
		} else if (cursor_pos == CURSOR_LEFT) {
			left_arrow();
		} else if (cursor_pos == CURSOR_RIGHT) {
			right_arrow();
		} else if (cursor_pos == CURSOR_COMBAT) {
			activate_combat_button();
		} else if (cursor_pos == CURSOR_HEART) {
			if (in_party || event->using_control_cheat())
				Game::get_game()->get_view_manager()->open_portrait_gump(actor);
		} else if (cursor_pos == CURSOR_PARTY) {
			if (in_party) {
			}
		} else if (cursor_pos == CURSOR_INVENTORY) {
			Game::get_game()->get_view_manager()->open_container_view(actor);
		} else {
			Obj *obj = actor->inventory_get_readied_object((uint8)cursor_pos);
			if (event->get_mode() == MOVE_MODE || event->get_mode() == EQUIP_MODE) {
				if (obj)
					event->unready(obj);
			} else
				event->select_view_obj(obj, actor);
		}
		return GUI_YUM;
	}
	default:
		break;
	}
	return GUI_PASS;
}

void DollViewGump::activate_combat_button() {
	Events *event = Game::get_game()->get_event();
	if (actor->is_in_party() && party->get_member_num(actor) != 0) {
		set_combat_mode(actor);
		update_display = true;
	} else if (event->get_mode() != INPUT_MODE && event->get_mode() != CAST_MODE
	           && event->get_mode() != ATTACK_MODE)
		event->newAction(COMBAT_MODE);
}

GUI_status DollViewGump::MouseWheel(sint32 x, sint32 y) {
	if (y > 0) {
		left_arrow();
	} else if (y < 0) {
		right_arrow();
	}
	return GUI_YUM;
}

GUI_status DollViewGump::MouseDown(int x, int y, Shared::MouseButton button) {
	return DraggableView::MouseDown(x, y, button);
}

GUI_status DollViewGump::MouseUp(int x, int y, Shared::MouseButton button) {
	return DraggableView::MouseUp(x, y, button);
}

} // End of namespace Nuvie
} // End of namespace Ultima
