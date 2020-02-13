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

#ifndef NUVIE_VIEWS_VIEW_H
#define NUVIE_VIEWS_VIEW_H

#include "ultima/nuvie/gui/widgets/gui_widget.h"

namespace Ultima {
namespace Nuvie {

GUI_status viewLeftButtonCallback(void *data);
GUI_status viewRightButtonCallback(void *data);

class Configuration;
class TileManager;
class Screen;
class Font;
class ObjManager;
class Party;
class GUI_Button;
class Actor;

class View: public GUI_Widget {
protected:
	Configuration *config;

	GUI_Button *left_button, *right_button, *actor_button, *party_button, *inventory_button;

	Font *font;
	TileManager *tile_manager;
	ObjManager *obj_manager;
	Party *party;

	uint8 cur_party_member;
	uint8 bg_color;
	bool new_ui_mode;

public:

	View(Configuration *cfg);
	~View() override;

	bool init(uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om);

	virtual bool set_party_member(uint8 party_member);
	uint8 get_party_member_num() {
		return cur_party_member;
	}
	bool next_party_member();
	bool prev_party_member();
	void fill_md_background(uint8 color, const Common::Rect &r);
	void set_combat_mode(Actor *actor);
	uint8 get_combat_mode_index(Actor *actor);

	virtual void close_view() {}

protected:

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
	GUI_Button *loadButton(Std::string dir, Std::string name, uint16 x, uint16 y);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
