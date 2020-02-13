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

#ifndef NUVIE_VIEWS_MAP_EDITOR_VIEW_H
#define NUVIE_VIEWS_MAP_EDITOR_VIEW_H

#include "ultima/nuvie/views/view.h"

namespace Ultima {
namespace Nuvie {

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
	~MapEditorView() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om);
	void Display(bool full_redraw) override;
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override;

	void close_view() override;
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
protected:
	void setTile(uint16 x, uint16 y, uint8 level);
	void toggleGrid();
private:
	void update_selected_tile_relative(sint32 rel_value);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
