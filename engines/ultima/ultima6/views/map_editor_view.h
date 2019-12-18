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

#ifndef ULTIMA6_VIEWS_MAP_EDITOR_VIEW_H
#define ULTIMA6_VIEWS_MAP_EDITOR_VIEW_H

#include "ultima/ultima6/views/view.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;
class GUI_Button;
class MapWindow;

class MapEditorView: public View {
private:
	Graphics::ManagedSurface *roof_tiles;
	MapWindow *map_window;
	uint16 selectedTile;
	uint16 tile_offset;
	GUI_Button *up_button;
	GUI_Button *down_button;
public:
	MapEditorView(Configuration *config);
	virtual ~MapEditorView();

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om);
	void Display(bool full_redraw);
	GUI_status KeyDown(Common::KeyState key);
	GUI_status MouseDown(int x, int y, int button);
	GUI_status MouseUp(int x, int y, int button);
	GUI_status MouseMotion(int x, int y, uint8 state);

	virtual void close_view();
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);
protected:
	void setTile(uint16 x, uint16 y, uint8 level);
	void toggleGrid();
private:
	void update_selected_tile_relative(sint32 rel_value);
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
