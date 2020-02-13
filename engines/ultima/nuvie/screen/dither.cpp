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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/screen/dither.h"

namespace Ultima {
namespace Nuvie {

//dither EGA to CGA
static const uint8 dither_cga_tbl[0x10] =   {0, 3, 3, 3, 13, 13, 13, 3, 3, 13, 15, 3, 13, 13, 15, 15};
//static const uint8 dither_cga_tbl[0x10] = {0,1,1,1,2 ,2 ,2 ,1,1,2 ,3 ,1,2 ,2 ,3, 3};

Dither::Dither(Configuration *cfg) {
	config = cfg;
	dither = NULL;
	mode = DITHER_NONE;

	set_mode();

	if (mode != DITHER_NONE)
		load_data();
}

Dither::~Dither() {
	if (dither)
		free(dither);
}

bool Dither::load_data() {
	Std::string path;
	NuvieIOFileRead file;

	config_get_path(config, "dither", path);
	if (file.open(path) == false)
		return false;//fixme better error handling

	dither = (uint8 *)malloc(0x200);
	if (dither == NULL)
		return false;

	file.readToBuf(dither, 0x200);

	file.close();

	return true;
}

void Dither::set_mode() {
	Std::string str_dither_mode;

	config->value("config/general/dither_mode", str_dither_mode);

	if (str_dither_mode == "none")
		mode = DITHER_NONE;
	else if (str_dither_mode == "cga") {
		mode = DITHER_CGA;
	} else if (str_dither_mode == "ega") {
		mode = DITHER_EGA;
	} else
		mode = DITHER_NONE;

	return;
}

bool Dither::dither_bitmap(unsigned char *src_buf, uint16 src_w, uint16 src_h, bool has_transparency) {
	uint8 pixel;

	if (!dither || mode == DITHER_NONE)
		return false;

	for (int y = 0; y < src_h; y++) {
		for (int x = 0; x < src_w; x++) {
			pixel = src_buf[y * src_w + x];

			if (has_transparency && pixel == 0xff)
				continue;

			if (pixel >= 0xe0 && pixel != 0xff)
				pixel &= 0xf;

			if (((x & 1) ^ (y & 1)) == 0)
				pixel = dither[0x100 + pixel];
			else
				pixel = dither[pixel];

			if (mode == DITHER_CGA)
				pixel = dither_cga_tbl[pixel];

			src_buf[y * src_w + x] = pixel;
		}
	}

	return true;
}

} // End of namespace Nuvie
} // End of namespace Ultima
