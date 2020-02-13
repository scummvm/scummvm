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

#ifndef NUVIE_CORE_CURSOR_H
#define NUVIE_CORE_CURSOR_H

#include "ultima/shared/std/string.h"
#include "ultima/shared/std/containers.h"


namespace Ultima {
namespace Nuvie {

class Configuration;
class Screen;
class U6Shape;

typedef struct {
	uint16 point_x, point_y; // hotspot
	unsigned char *shapedat;
	uint16 w, h;
} MousePointer;


/* Contains all mouse pointers, with hotspot and draw methods that work on the
 * active cursor.
 */
class Cursor {
	friend class Screen;
	Screen *screen;
	Configuration *config;
	sint32 cur_x, cur_y; // location on screen, unused normally
	Std::vector<MousePointer *> cursors; // pointer list
	uint8 cursor_id; // which pointer is active

	unsigned char *cleanup; // restore image behind cursor
	Common::Rect cleanup_area;
	Common::Rect update_area; // clear & display are updated at once (avoid flicker)

	bool hidden;

	uint16 screen_w, screen_h;

	void add_update(uint16 x, uint16 y, uint16 w, uint16 h);
	inline void fix_position(MousePointer *ptr, int &px, int &py);
	void save_backing(uint32 px, uint32 py, uint32 w, uint32 h);

public:
	Cursor();
	~Cursor()                              {
		unload_all();
	}
	bool init(Configuration *c, Screen *s, nuvie_game_t game_type);
	uint32 load_all(Std::string filename, nuvie_game_t game_type);
	void unload_all();
	bool set_pointer(uint8 ptr_num);

	void reset_position()           {
		cur_x = -1;
		cur_y = -1;
	}
	void move(uint32 px, uint32 py) {
		cur_x = px;
		cur_y = py;
	}
	void hide()                     {
		hidden = true;
		clear();
		update();
	}
	void show()                     {
		hidden = false;
	}

	void get_hotspot(uint16 &x, uint16 &y) {
		x = cursors[cursor_id]->point_x;
		y = cursors[cursor_id]->point_y;
	}
	bool display()                         {
		return (display(cur_x, cur_y));
	}
	bool display(int px, int py);
	void clear();
	void update();

	bool is_visible() {
		return !hidden;
	}
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
