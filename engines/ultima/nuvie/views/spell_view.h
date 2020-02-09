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

#ifndef NUVIE_VIEWS_SPELL_VIEW_H
#define NUVIE_VIEWS_SPELL_VIEW_H

#include "ultima/nuvie/views/draggable_view.h"
#include "ultima/nuvie/core/obj.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Actor;
class Font;
class U6Bmp;
class Spell;

class SpellView : public DraggableView {

	U6Bmp *background;
	bool all_spells_mode;

protected:
	bool event_mode; //this means we are reporting the spell_num back to the event class. Used by the enchant spell.

	Obj *spell_container;
	Actor *caster;
	uint16 caster_reagents[8];

	uint8 level;
	uint8 spell_num;

	sint16 cur_spells[16];

	uint8 num_spells_per_page;

public:
	SpellView(Configuration *cfg);
	~SpellView() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om);

	void set_spell_caster(Actor *actor, Obj *s_container, bool eventMode);
	sint16 get_selected_spell() {
		if (spell_container) {
			return spell_container->quality;
		} else return -1;
	}
	void Display(bool full_redraw) override;
	void PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y) override;
	void close_look();
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;

	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseMotion(int x, int y, uint8 state) override {
		return GUI_YUM;
	}
	GUI_status MouseEnter(uint8 state) override {
		return GUI_YUM;
	}
	GUI_status MouseLeave(uint8 state) override {
		return GUI_YUM;
	}
	GUI_status MouseClick(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseDouble(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseDelayed(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseHeld(int x, int y, Shared::MouseButton button) override {
		return GUI_YUM;
	}
	GUI_status MouseWheel(sint32 x, sint32 y) override;

protected:

	void event_mode_select_spell();

	void add_command_icons(Screen *tmp_screen, void *view_manager);
	void hide_buttons();
	void show_buttons();
	void update_buttons();

	void move_left();
	void move_right();
	GUI_status move_up();
	GUI_status move_down();

	void set_prev_level();
	void set_next_level();

	virtual uint8 fill_cur_spell_list();
	sint8 get_selected_index();

	void display_level_text();
	void display_spell_list_text();
	void display_spell_text(Spell *spell, uint16 line_num, uint8 selected_spell);
	void show_spell_description();
	GUI_status cancel_spell();
	uint16 get_available_spell_count(Spell *s);

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
