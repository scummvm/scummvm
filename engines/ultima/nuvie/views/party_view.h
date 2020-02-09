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

#ifndef NUVIE_VIEWS_PARTY_VIEW_H
#define NUVIE_VIEWS_PARTY_VIEW_H

#include "ultima/nuvie/views/view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Font;
class Party;
class Player;
class Actor;
class SunMoonStripWidget;

class PartyView : public View {

	Player *player;
	void *view_manager;
	uint16 row_offset; // first party member displayed
	SunMoonStripWidget *sun_moon_widget;

public:
	PartyView(Configuration *cfg);
	~PartyView() override;

	bool init(void *vm, uint16 x, uint16 y, Font *f, Party *p, Player *pl, TileManager *tm, ObjManager *om);
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override {
		return (GUI_YUM);
	}
	GUI_status MouseWheel(sint32 x, sint32 y) override;
	bool drag_accept_drop(int x, int y, int message, void *data) override;
	void drag_perform_drop(int x, int y, int message, void *data) override;
	void Display(bool full_redraw) override;
	void update() {
		update_display = true;
	}
	void display_sun_moon_strip();
	void set_party_view_targeting(bool val) {
		party_view_targeting = val;
	}

protected:

	void display_surface_strip();
	void display_dungeon_strip();
	void display_arrows();
	void display_sun_moon(Tile *tile, uint8 pos);
	void display_sun(uint8 hour, uint8 minute, bool eclipse);
	void display_moons(uint8 day, uint8 hour, uint8 minute = 0);

	bool up_arrow();
	bool down_arrow();
	bool party_view_targeting;

	Actor *get_actor(int x, int y);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
