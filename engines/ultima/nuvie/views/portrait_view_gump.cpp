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
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/container_widget_gump.h"
#include "ultima/nuvie/views/portrait_view_gump.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

PortraitViewGump::PortraitViewGump(Configuration *cfg) : DraggableView(cfg) {
	portrait = NULL;
	font = NULL;
	gump_button = NULL;
	portrait_data = NULL;
	actor = NULL;
	cursor_tile = NULL;
	show_cursor = true;
	cursor_pos = CURSOR_CHECK;
	cursor_xoff = 1;
	cursor_yoff = 67;
}

PortraitViewGump::~PortraitViewGump() {
	if (font)
		delete font;
	if (portrait_data)
		free(portrait_data);
}

bool PortraitViewGump::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om, Portrait *por, Actor *a) {
	View::init(x, y, f, p, tm, om);

	SetRect(area.left, area.top, 188, 91);

	portrait = por;
	set_actor(a);

	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string imagefile;
	Std::string path;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;

	gump_button = loadButton(datadir, "gump", 0, 67);

	build_path(datadir, "portrait_bg.bmp", imagefile);
	bg_image = SDL_LoadBMP(imagefile.c_str());

	set_bg_color_key(0, 0x70, 0xfc);

	font = new GUI_Font(GUI_FONT_GUMP);
	font->setColoring(0x08, 0x08, 0x08, 0x80, 0x58, 0x30, 0x00, 0x00, 0x00);

	Graphics::ManagedSurface *image, *image1;

	build_path(datadir, "left_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	left_button = new GUI_Button(this, 23, 6, image, image1, this);
	this->AddWidget(left_button);

	build_path(datadir, "right_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	right_button = new GUI_Button(this, 166, 6, image, image1, this);
	this->AddWidget(right_button);

	if (party->get_member_num(actor) < 0) {
		left_button->Hide();
		right_button->Hide();
	}
	cursor_tile = tile_manager->get_gump_cursor_tile();

	return true;
}

void PortraitViewGump::set_actor(Actor *a) {
	actor = a;
	if (portrait_data)
		free(portrait_data);
	portrait_data = portrait->get_portrait_data(actor);
}

void PortraitViewGump::left_arrow() {
	if (party->get_member_num(actor) < 0)
		return;
	uint8 party_mem_num = party->get_member_num(actor);
	if (party_mem_num > 0)
		party_mem_num--;
	else
		party_mem_num = party->get_party_size() - 1;

	set_actor(party->get_actor(party_mem_num));
}

void PortraitViewGump::right_arrow() {
	if (party->get_member_num(actor) < 0)
		return;
	set_actor(party->get_actor((party->get_member_num(actor) + 1) % party->get_party_size()));
}

void PortraitViewGump::Display(bool full_redraw) {
	char buf[6]; //xxxxx\n
//display_level_text();
//display_spell_list_text();
	Common::Rect dst;
	dst = area;
	SDL_BlitSurface(bg_image, NULL, surface, &dst);

	DisplayChildren(full_redraw);
	screen->blit(area.left + 25, area.top + 17, portrait_data, 8, portrait->get_portrait_width(), portrait->get_portrait_height(), portrait->get_portrait_width(), false);

	int w, h;
	w = font->getCenter(actor->get_name(), 138);

	font->setColoring(0x08, 0x08, 0x08, 0x80, 0x58, 0x30, 0x00, 0x00, 0x00);

	font->textOut(screen->get_sdl_surface(), area.left + 29 + w, area.top + 6, actor->get_name());

	snprintf(buf, 5, "%d", actor->get_strength());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 170 - w, area.top + 18, buf);

	snprintf(buf, 5, "%d", actor->get_dexterity());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 170 - w, area.top + 27, buf);

	snprintf(buf, 5, "%d", actor->get_intelligence());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 170 - w, area.top + 36, buf);

	font->setColoring(0x6c, 0x00, 0x00, 0xbc, 0x34, 0x00, 0x00, 0x00, 0x00);

	snprintf(buf, 5, "%d", actor->get_magic());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 142 - w, area.top + 55, buf);

	snprintf(buf, 5, "%d", actor->get_maxmagic());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 170 - w, area.top + 55, buf);

	font->setColoring(0x00, 0x3c, 0x70, 0x74, 0x74, 0x74, 0x00, 0x00, 0x00);

	snprintf(buf, 5, "%d", actor->get_hp());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 142 - w, area.top + 64, buf);

	snprintf(buf, 5, "%d", actor->get_maxhp());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 170 - w, area.top + 64, buf);

	font->setColoring(0xa8, 0x28, 0x00, 0xa8, 0x54, 0x00, 0x00, 0x00, 0x00);

	snprintf(buf, 5, "%d", actor->get_level());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 142 - w, area.top + 73, buf);

	snprintf(buf, 5, "%d", actor->get_exp());
	font->textExtent(buf, &w, &h);
	font->textOut(screen->get_sdl_surface(), area.left + 170 - w, area.top + 73, buf);

	if (show_cursor)
		screen->blit(area.left + cursor_xoff, area.top + cursor_yoff, (const unsigned char *)cursor_tile->data, 8, 16, 16, 16, true);
	update_display = false;
	screen->update(area.left, area.top, area.width(), area.height());


	return;
}

GUI_status PortraitViewGump::set_cursor_pos(gumpCursorPos pos) {
	if (party->get_member_num(actor) < 0)
		return GUI_YUM;
	cursor_pos = pos;
	if (cursor_pos == CURSOR_CHECK) {
		cursor_xoff = 1;
		cursor_yoff = 67;
	} else if (cursor_pos == CURSOR_LEFT) {
		cursor_xoff = 18;
		cursor_yoff = 1;
	} else {
		cursor_xoff = 162;
		cursor_yoff = 1;
	}
	return GUI_YUM;
}

GUI_status PortraitViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	//close gump and return control to Magic class for clean up.
	if (caller == gump_button) {
		Game::get_game()->get_view_manager()->close_gump(this);
		return GUI_YUM;
	} else if (caller == left_button) {
		left_arrow();
	} else if (caller == right_button) {
		right_arrow();
	}

	return GUI_PASS;
}

GUI_status PortraitViewGump::KeyDown(const Common::KeyState &key) {
//	I was checking for numlock but probably don't need to
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case SOUTH_WEST_KEY:
	case SOUTH_KEY:
		return set_cursor_pos(CURSOR_CHECK);
	case NORTH_KEY:
		if (cursor_pos == CURSOR_CHECK)
			return set_cursor_pos(CURSOR_LEFT);
		return GUI_YUM;
	case NORTH_WEST_KEY:
	case WEST_KEY:
		if (cursor_pos == CURSOR_RIGHT)
			return set_cursor_pos(CURSOR_LEFT);
		return set_cursor_pos(CURSOR_CHECK);
	case NORTH_EAST_KEY:
	case SOUTH_EAST_KEY:
	case EAST_KEY:
		if (cursor_pos == CURSOR_CHECK)
			return set_cursor_pos(CURSOR_LEFT);
		return set_cursor_pos(CURSOR_RIGHT);
	case DO_ACTION_KEY:
		if (cursor_pos == CURSOR_CHECK)
			Game::get_game()->get_view_manager()->close_gump(this);
		else if (cursor_pos == CURSOR_LEFT)
			left_arrow();
		else
			right_arrow();
		return GUI_YUM;
	case NEXT_PARTY_MEMBER_KEY:
		right_arrow();
		return GUI_YUM;
	case PREVIOUS_PARTY_MEMBER_KEY:
		left_arrow();
		return GUI_YUM;
	case HOME_KEY:
		if (party->get_member_num(actor) >= 0)
			set_actor(party->get_actor(0));
		return GUI_YUM;
	case END_KEY:
		if (party->get_member_num(actor) >= 0)
			set_actor(party->get_actor(party->get_party_size() - 1));
		return GUI_YUM;
	default:
		break;
	}
	return GUI_PASS;
}

GUI_status PortraitViewGump::MouseWheel(sint32 x, sint32 y) {
	if (y > 0) {
		left_arrow();
	} else if (y < 0) {
		right_arrow();
	}
	return GUI_YUM;
}

GUI_status PortraitViewGump::MouseDown(int x, int y, Shared::MouseButton button) {
	return DraggableView::MouseDown(x, y, button);
}

} // End of namespace Nuvie
} // End of namespace Ultima
