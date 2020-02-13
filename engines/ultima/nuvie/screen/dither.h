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

#ifndef NUVIE_SCREEN_DITHER_H
#define NUVIE_SCREEN_DITHER_H

namespace Ultima {
namespace Nuvie {

class Configuration;

#define DITHER_NO_TRANSPARENCY false

// Dither modes..

#define DITHER_NONE 0
#define DITHER_CGA  1
#define DITHER_EGA  2
#define DITHER_HRC  3 //FIXME add this mode.

class Dither {
	Configuration *config;
	uint8 *dither;
	uint8 mode;

public:

	Dither(Configuration *cfg);
	~Dither();
	uint8 get_mode() {
		return mode;
	}
	bool dither_bitmap(unsigned char *src_buf, uint16 src_w, uint16 src_h, bool has_transparency);

protected:

	bool load_data();
	void set_mode();

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
