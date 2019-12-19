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

#ifndef ULTIMA6_VIEWS_DOLL_WIDGET_H
#define ULTIMA6_VIEWS_DOLL_WIDGET_H

#include "ultima/ultima6/gui/gui_widget.h"
#include "ultima/ultima6/core/tile_manager.h"
#include "ultima/ultima6/core/obj_manager.h"
#include "ultima/ultima6/views/inventory_message.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;
class Actor;
class U6Shape;

class DollWidget : public GUI_Widget {

	Configuration *config;
	TileManager *tile_manager;
	ObjManager *obj_manager;

	Actor *actor;
	bool use_new_dolls, old_use_new_dolls;

	Obj *selected_obj, *unready_obj;

	uint8 bg_color;
	bool is_in_portrait_view;
	bool need_to_free_tiles;
	Tile *empty_tile, *blocked_tile;

	U6Shape *md_doll_shp;
	Graphics::ManagedSurface *actor_doll, *doll_bg;

public:
	DollWidget(Configuration *cfg, GUI_CallBack *callback = NULL);
	~DollWidget();

	bool init(Actor *a, uint16 x, uint16 y, TileManager *tm, ObjManager *om, bool in_portrat_view = false);
	void free_doll_shapes();
	void setColorKey(Graphics::ManagedSurface *image);
	void set_actor(Actor *a);
	void Display(bool full_redraw);

	virtual GUI_status MouseDown(int x, int y, MouseButton button);
	virtual GUI_status MouseUp(int x, int y, MouseButton button);
	virtual GUI_status MouseMotion(int x, int y, uint8 state);
	GUI_status MouseDouble(int x, int y, MouseButton button);
	GUI_status MouseClick(int x, int y, MouseButton button);
	GUI_status MouseDelayed(int x, int y, MouseButton button);

	void drag_drop_success(int x, int y, int message, void *data);
	void drag_drop_failed(int x, int y, int message, void *data);

	bool drag_accept_drop(int x, int y, int message, void *data);
	void drag_perform_drop(int x, int y, int message, void *data);

	void drag_draw(int x, int y, int message, void *data);

	Common::Rect *get_item_hit_rect(uint8 location);

protected:

	GUI_CallBack *callback_object; // object-selected callback

	void display_doll();
	void display_old_doll();
	void display_new_doll();
	void display_readied_object(uint8 location, uint16 x, uint16 y, Actor *actor, Tile *emptyTile);

private:

	void load_md_doll_shp();
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
