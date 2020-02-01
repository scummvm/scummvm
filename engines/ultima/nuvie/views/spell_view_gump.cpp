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
#include "ultima/nuvie/core/magic.h"
#include "ultima/nuvie/views/spell_view_gump.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/files/nuvie_bmp_file.h"

namespace Ultima {
namespace Nuvie {

#define NEWMAGIC_BMP_W 144
#define NEWMAGIC_BMP_H 82

SpellViewGump::SpellViewGump(Configuration *cfg) : SpellView(cfg) {
	num_spells_per_page = 10;
	bg_image = NULL;
	gump_button = NULL;
	font = NULL;
	selected_spell = -1;
}

SpellViewGump::~SpellViewGump() {
	delete font;
}

bool SpellViewGump::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om) {
	View::init(x, y, f, p, tm, om);

	SetRect(area.left, area.top, 162, 108);

	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string imagefile;
	Std::string path;

	Graphics::ManagedSurface *image, *image1;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;

	build_path(datadir, "gump_btn_up.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(datadir, "gump_btn_down.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	gump_button = new GUI_Button(NULL, 0, 9, image, image1, this);
	this->AddWidget(gump_button);

	build_path(datadir, "spellbook", path);
	datadir = path;

	build_path(datadir, "spellbook_left_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str()); //we load this twice as they are freed in ~GUI_Button()
	image1 = SDL_LoadBMP(imagefile.c_str());

	left_button = new GUI_Button(this, 27, 4, image, image1, this);
	this->AddWidget(left_button);

	build_path(datadir, "spellbook_right_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	right_button = new GUI_Button(this, 132, 4, image, image1, this);
	this->AddWidget(right_button);

	font = new GUI_Font(GUI_FONT_GUMP);
	font->setColoring(0x7c, 0x00, 0x00, 0xd0, 0x70, 0x00, 0x00, 0x00, 0x00);

	return true;
}


uint8 SpellViewGump::fill_cur_spell_list() {
	uint8 count = SpellView::fill_cur_spell_list();

	//load spell images
	uint8 i;
	char filename[24]; // spellbook_spell_xxx.bmp\0
	Std::string datadir = GUI::get_gui()->get_data_dir();
	Std::string path;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;
	build_path(datadir, "spellbook", path);
	datadir = path;

	Std::string imagefile;

	SDL_FreeSurface(bg_image);

	//build_path(datadir, "", spellbookdir);

	build_path(datadir, "spellbook_bg.bmp", imagefile);
	bg_image = bmp.getSdlSurface32(imagefile);
	if (bg_image == NULL) {
		DEBUG(0, LEVEL_ERROR, "Failed to load spellbook_bg.bmp from '%s' directory\n", datadir.c_str());
		return count;
	}

	set_bg_color_key(0, 0x70, 0xfc);

	for (i = 0; i < count; i++) {
		sprintf(filename, "spellbook_spell_%03d.bmp", cur_spells[i]);
		build_path(datadir, filename, imagefile);
		Graphics::ManagedSurface *spell_image = bmp.getSdlSurface32(imagefile);
		if (spell_image == NULL) {
			DEBUG(0, LEVEL_ERROR, "Failed to load %s from '%s' directory\n", filename, datadir.c_str());
		} else {
			Common::Rect dst;

			uint8 base = (level - 1) * 16;
			uint8 spell = cur_spells[i] - base;

			dst.left = ((spell < 5) ? 25 : 88);
			dst.top = 18 + (spell % 5) * 14;
			dst.setWidth(58);
			dst.setHeight(13);

			SDL_BlitSurface(spell_image, NULL, bg_image, &dst);
			SDL_FreeSurface(spell_image);
			printSpellQty(cur_spells[i], dst.left + ((spell < 5) ? 50 : 48), dst.top);
		}
	}

	loadCircleString(datadir);

	return count;
}

void SpellViewGump::loadCircleString(Std::string datadir) {
	Std::string imagefile;
	char filename[7]; // n.bmp\0

	sprintf(filename, "%d.bmp", level);
	build_path(datadir, filename, imagefile);

	Graphics::ManagedSurface *s = bmp.getSdlSurface32(imagefile);
	if (s != NULL) {
		Common::Rect dst;
		dst = Common::Rect(70, 7, 74, 13);
		SDL_BlitSurface(s, NULL, bg_image, &dst);
	}

	switch (level) {
	case 1 :
		loadCircleSuffix(datadir, "st.bmp");
		break;
	case 2 :
		loadCircleSuffix(datadir, "nd.bmp");
		break;
	case 3 :
		loadCircleSuffix(datadir, "rd.bmp");
		break;
	default:
		break;
	}
}

void SpellViewGump::loadCircleSuffix(Std::string datadir, Std::string image) {
	Std::string imagefile;

	build_path(datadir, image, imagefile);
	Graphics::ManagedSurface *s = bmp.getSdlSurface32(imagefile);
	if (s != NULL) {
		Common::Rect dst;
		dst = Common::Rect(75, 7, 82, 13);
		SDL_BlitSurface(s, NULL, bg_image, &dst);
	}
}


void SpellViewGump::printSpellQty(uint8 spellNum, uint16 x, uint16 y) {
	Magic *m = Game::get_game()->get_magic();
	char num_str[4];

	Spell *spell = m->get_spell((uint8)spellNum);

	uint16 qty = get_available_spell_count(spell);
	snprintf(num_str, 3, "%d", qty);

	if (qty < 10)
		x += 5;

	font->textOut(bg_image, x, y, num_str);
}

void SpellViewGump::Display(bool full_redraw) {
//display_level_text();
//display_spell_list_text();
	Common::Rect dst;
	dst = area;
	dst.setWidth(162);
	dst.setHeight(108);
	SDL_BlitSurface(bg_image, NULL, surface, &dst);

	DisplayChildren(full_redraw);

	sint16 spell = get_selected_spell();

	if (spell < 0)
		spell = 0;

	spell = spell % 16;
	screen->fill(248, area.left + ((spell < 5) ? 75 : 136), area.top + 18 + 7 + (spell % 5) * 14, 10, 1);

	update_display = false;
	screen->update(area.left, area.top, area.width(), area.height());

	return;
}

GUI_status SpellViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	//close gump and return control to Magic class for clean up.
	if (caller == gump_button) {
		if (Game::get_game()->get_event()->is_looking_at_spellbook())
			close_look();
		else
			close_spellbook();
		return GUI_YUM;
	} else if (caller == left_button) {
		move_left();
		return GUI_YUM;
	} else if (caller == right_button) {
		move_right();
		return GUI_YUM;
	}

	return GUI_PASS;
}

void SpellViewGump::close_spellbook() {
	Game::get_game()->get_event()->close_spellbook();
}

sint16 SpellViewGump::getSpell(int x, int y) {
	int localy = y - area.top;
	int localx = x - area.left;

	localy += 3; //align the pointer in the center of the crosshair cursor.
	localx += 3;

	if (localy < 21 || localy > 88 || localx < 28 || localx > 148) {
		return -1;
	}

	uint8 spell = (level - 1) * 16;


	if (localx >= 89)
		spell += 5;

	spell += (localy - 20) / 14;

	for (uint8 i = 0; cur_spells[i] != -1 && i < 16; i++) {
		if (cur_spells[i] == spell) {
			return spell;
		}
	}

	return -1;
}

GUI_status SpellViewGump::MouseWheel(sint32 x, sint32 y) {
	if (y > 0) {
		move_left();
	} else if (y < 0) {
		move_right();
	}
	return GUI_YUM;
}

GUI_status SpellViewGump::MouseDown(int x, int y, Shared::MouseButton button) {
	if (button == Shared::BUTTON_RIGHT) {
		close_spellbook();
		return GUI_YUM;
	}

	sint16 clicked_spell = getSpell(x, y);

	if (clicked_spell != -1) {
		selected_spell = clicked_spell;
		return GUI_YUM;
	}

	bool can_target = true; // maybe put this check into GUI_widget
	if (HitRect(x, y)) {
		if (bg_image) {
			uint32 pixel = sdl_getpixel(bg_image, x - area.left, y - area.top);
			if (pixel != bg_color_key)
				can_target = false;
		} else
			can_target = false;
	}

	if (can_target) {
		Events *event = Game::get_game()->get_event();
		if (event->is_looking_at_spellbook()) {
			close_spellbook();
			return GUI_YUM;
		}
		event->target_spell(); //Simulate a global key down event.
		if (event->get_mode() == INPUT_MODE)
			Game::get_game()->get_map_window()->select_target(x, y);
		if (event->get_mode() != MOVE_MODE)
			close_spellbook();
		return GUI_YUM;
	}

	return DraggableView::MouseDown(x, y, button);
}

GUI_status SpellViewGump::MouseUp(int x, int y, Shared::MouseButton button) {
	sint16 spell = getSpell(x, y);

	if (spell != -1 && spell == selected_spell) {
		spell_container->quality = spell;
		if (Game::get_game()->get_event()->is_looking_at_spellbook())
			show_spell_description();
		else if (event_mode) {
			event_mode_select_spell();
		} else {
			//Simulate a global key down event.
			Game::get_game()->get_event()->target_spell();
		}

		return GUI_YUM;
	}


	return DraggableView::MouseUp(x, y, button);
}

} // End of namespace Nuvie
} // End of namespace Ultima
