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

#ifndef NUVIE_VIEWS_PORTRAIT_VIEW_H
#define NUVIE_VIEWS_PORTRAIT_VIEW_H

#include "ultima/nuvie/views/view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class Portrait;
class Screen;
class Font;
class ObjManager;
class Party;
class Player;
class Actor;
class DollWidget;
class U6Shape;

class PortraitView : public View {

	nuvie_game_t gametype;

	uint8 cur_actor_num;
	Portrait *portrait;
	Std::string *name_string;

	unsigned char *portrait_data;
	U6Shape *bg_data;
	uint8 portrait_width;
	uint8 portrait_height;

	bool waiting; // waiting for input, then will return to previous view
	bool show_cursor; // indicate waiting for input
// uint16 cursor_x, cursor_y;

	DollWidget *doll_widget;
	bool display_doll;

public:
	PortraitView(Configuration *cfg);
	~PortraitView() override;

	bool init(uint16 x, uint16 y, Font *f, Party *p, Player *player, TileManager *tm, ObjManager *om, Portrait *port);
	void Display(bool full_redraw) override;
	GUI_status HandleEvent(const Common::Event *event) override;

	bool set_portrait(Actor *actor, const char *name);
	void set_show_cursor(bool state) {
		show_cursor = state;
	}
	void set_waiting(bool state);
	bool get_waiting()     {
		return (waiting);
	}

protected:

	void display_name(uint16 y_offset);

private:
	void load_background(const char *filename, uint8 lib_offset);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
