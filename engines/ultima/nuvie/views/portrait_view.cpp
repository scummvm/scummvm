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

#include "ultima/shared/std/string.h"

#include "ultima/nuvie/core/nuvie_defs.h"

#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_shape.h"

#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/views/doll_widget.h"
#include "ultima/nuvie/views/portrait_view.h"
#include "ultima/nuvie/views/sun_moon_strip_widget.h"

namespace Ultima {
namespace Nuvie {

PortraitView::PortraitView(Configuration *cfg) : View(cfg) {
	portrait_data = NULL;
	portrait = NULL;
	bg_data = NULL;
	name_string = new string;
	show_cursor = false;
	doll_widget = NULL;
	waiting = false;
	display_doll = false;
	cur_actor_num = 0;
	gametype = get_game_type(cfg);

//FIXME: Portraits in SE/MD are different size than in U6! 79x85 76x83
	switch (gametype) {
	case NUVIE_GAME_U6:
		portrait_width = 56;
		portrait_height = 64;
		break;
	case NUVIE_GAME_SE:
		portrait_width = 79;
		portrait_height = 85;
		break;
	case NUVIE_GAME_MD:
		portrait_width = 76;
		portrait_height = 83;
		break;
	}
}

PortraitView::~PortraitView() {
	if (portrait_data != NULL)
		free(portrait_data);

	if (bg_data != NULL)
		delete bg_data;

	delete name_string;
}

bool PortraitView::init(uint16 x, uint16 y, Font *f, Party *p, Player *player, TileManager *tm, ObjManager *om, Portrait *port) {
	View::init(x, y, f, p, tm, om);

	portrait = port;

	doll_widget = new DollWidget(config, this);
	doll_widget->init(NULL, 0, 16, tile_manager, obj_manager, true);

	AddWidget(doll_widget);
	doll_widget->Hide();

	if (gametype == NUVIE_GAME_U6) {
		SunMoonStripWidget *sun_moon_widget = new SunMoonStripWidget(player, tile_manager);
		sun_moon_widget->init(-8, -2);
		AddWidget(sun_moon_widget);
	} else if (gametype == NUVIE_GAME_MD) {
		load_background("mdscreen.lzc", 1);
	} else if (gametype == NUVIE_GAME_SE) {
		load_background("bkgrnd.lzc", 0);
	}

	return true;
}


void PortraitView::load_background(const char *f, uint8 lib_offset) {
	U6Lib_n file;
	bg_data = new U6Shape();
	Std::string path;
	config_get_path(config, f, path);
	file.open(path, 4, gametype);
	unsigned char *temp_buf = file.get_item(lib_offset);
	bg_data->load(temp_buf + 8);
	free(temp_buf);
}

void PortraitView::Display(bool full_redraw) {

	if (Game::get_game()->is_new_style() || Game::get_game()->is_original_plus_full_map())
		screen->fill(bg_color, area.left, area.top, area.width(), area.height());
	if (portrait_data != NULL/* && (full_redraw || update_display)*/) {
		update_display = false;
		if (gametype == NUVIE_GAME_U6) {
			if (display_doll)
				screen->blit(area.left + 72, area.top + 16, portrait_data, 8, portrait_width, portrait_height, portrait_width, false);
			else
				screen->blit(area.left + (area.width() - portrait_width) / 2, area.top + (area.height() - portrait_height) / 2, portrait_data, 8, portrait_width, portrait_height, portrait_width, true);
			display_name(80);
		} else if (gametype == NUVIE_GAME_MD) {
			uint16 w, h;
			bg_data->get_size(&w, &h);
			screen->blit(area.left, area.top - 2, bg_data->get_data(), 8, w, h, w, true);

			screen->blit(area.left + (area.width() - portrait_width) / 2, area.top + 6, portrait_data, 8, portrait_width, portrait_height, portrait_width, true);
			display_name(100);
		} else if (gametype == NUVIE_GAME_SE) {
			uint16 w, h;
			bg_data->get_size(&w, &h);
			screen->blit(area.left, area.top, bg_data->get_data(), 8, w, h, w, true);

			screen->blit(area.left + (area.width() - portrait_width) / 2 + 1, area.top + 1, portrait_data, 8, portrait_width, portrait_height, portrait_width, true);
			display_name(98);
		}
	}
	if (show_cursor && gametype == NUVIE_GAME_U6) { // FIXME: should we be using scroll's drawCursor?
		screen->fill(bg_color, area.left, area.top + area.height() - 8, 8, 8);
		Game::get_game()->get_scroll()->drawCursor(area.left, area.top + area.height() - 8);
	}
	DisplayChildren(full_redraw);
	screen->update(area.left, area.top, area.width(), area.height());
}

bool PortraitView::set_portrait(Actor *actor, const char *name) {
	if (Game::get_game()->is_new_style())
		this->Show();
	cur_actor_num = actor->get_actor_num();
	int doll_x_offset = 0;

	if (portrait_data != NULL)
		free(portrait_data);

	portrait_data = portrait->get_portrait_data(actor);

	if (gametype == NUVIE_GAME_U6 && actor->has_readied_objects()) {
		if (portrait_data == NULL)
			doll_x_offset = 34;

		doll_widget->MoveRelativeToParent(doll_x_offset, 16);

		display_doll = true;
		doll_widget->Show();
		doll_widget->set_actor(actor);
	} else {
		display_doll = false;
		doll_widget->Hide();
		doll_widget->set_actor(NULL);

		if (portrait_data == NULL)
			return false;
	}

	if (name == NULL)
		name = actor->get_name();
	if (name == NULL)
		name_string->assign("");  // just in case
	else
		name_string->assign(name);

	if (screen)
		screen->fill(bg_color, area.left, area.top, area.width(), area.height());

	Redraw();
	return true;
}

void PortraitView::display_name(uint16 y_offset) {
	const char *name;

	name = name_string->c_str();

	font->drawString(screen, name, area.left + (area.width() - strlen(name) * 8) / 2, area.top + y_offset);

	return;
}


/* On any input return to previous status view if waiting.
 * Returns true if event was used.
 */
GUI_status PortraitView::HandleEvent(const Common::Event *event) {
	if (waiting && (
			event->type == Common::EVENT_LBUTTONDOWN
			|| event->type == Common::EVENT_RBUTTONDOWN
			|| event->type == Common::EVENT_MBUTTONDOWN
			|| event->type == Common::EVENT_KEYDOWN)
	) {
		if (Game::get_game()->is_new_style())
			this->Hide();
		else // FIXME revert to previous status view
			Game::get_game()->get_view_manager()->set_inventory_mode();
		// Game::get_game()->get_scroll()->set_input_mode(false);
		Game::get_game()->get_scroll()->message("\n");
		set_waiting(false);
		return (GUI_YUM);
	}
	return (GUI_PASS);
}


/* Start/stop waiting for input to continue, and (for now) steal cursor from
 * MsgScroll.
 */
void PortraitView::set_waiting(bool state) {
	if (state == true && display_doll == false && portrait_data == NULL) { // don't wait for nothing
		if (Game::get_game()->is_new_style())
			this->Hide();
		return;
	}
	waiting = state;
	set_show_cursor(waiting);
	Game::get_game()->get_scroll()->set_show_cursor(!waiting);
	Game::get_game()->get_gui()->lock_input(waiting ? this : NULL);
}

} // End of namespace Nuvie
} // End of namespace Ultima
