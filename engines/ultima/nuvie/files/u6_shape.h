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

#ifndef NUVIE_FILES_U6_SHAPE_H
#define NUVIE_FILES_U6_SHAPE_H

/*
 * ==========
 *  Includes
 * ==========
 */
#include "ultima/shared/std/string.h"


namespace Ultima {
namespace Nuvie {

class U6Lib_n;
class Configuration;

/*
 * ==================
 *  Class definition
 * ==================
 *
 * U6Shape can load Ultima VI shape files and return the shapes
 * stored into these files either as a Graphics::ManagedSurface or as raw data.
 */
class U6Shape {
private:
	uint16 hotx, hoty;

protected:
	unsigned char *raw;
	uint16 width, height;

public:
	U6Shape();
	virtual ~U6Shape();

	bool init(uint16 w, uint16 h, uint16 hx = 0, uint16 hy = 0);
	virtual bool load(Std::string filename);
	bool load(U6Lib_n *file, uint32 index);
	virtual bool load(unsigned char *buf);
	bool load_from_lzc(Std::string filename, uint32 idx, uint32 sub_idx);
	bool load_WoU_background(Configuration *config, nuvie_game_t game_type);

	unsigned char *get_data();
	Graphics::ManagedSurface *get_shape_surface();
	bool get_hot_point(uint16 *x, uint16 *y);
	bool get_size(uint16 *w, uint16 *h);

	void draw_line(uint16 sx, uint16 sy, uint16 ex, uint16 ey, uint8 color);
	bool blit(U6Shape *shp, uint16 x, uint16 y);
	void fill(uint8 color);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
