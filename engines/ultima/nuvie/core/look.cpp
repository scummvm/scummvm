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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/core/look.h"

namespace Ultima {
namespace Nuvie {

Look::Look(Configuration *cfg)
	: look_data(NULL), desc_buf(NULL) {
	config = cfg;

	look_tbl[2047] = NULL;
	max_len = 0;
}

Look::~Look() {
	free(look_data);
	free(desc_buf);
}

bool Look::init() {
	Std::string filename;
	U6Lzw lzw;
	uint32 decomp_size;
	unsigned char *ptr;
	const char *s;
	uint16 i, j;
	unsigned int len;
	int game_type;
	NuvieIOFileRead look_file;

	config->value("config/GameType", game_type);

	switch (game_type) {
	case NUVIE_GAME_U6 :
		config_get_path(config, "look.lzd", filename);
		look_data = lzw.decompress_file(filename, decomp_size);
		if (look_data == NULL)
			return false;
		break;
	case NUVIE_GAME_MD :
	case NUVIE_GAME_SE :
		U6Lib_n lib_file;
		config_get_path(config, "look.lzc", filename);
		if (lib_file.open(filename, 4, game_type) == false)
			return false;
		look_data = lib_file.get_item(0);
		break;
	}

	ptr = look_data;
// i: current string pos, j: last string pos
	for (i = 0, j = 0; i < 2048;) {
		// get number of string
		i = ptr[0] + (ptr[1] << 8);

		if (i >= 2048)
			break;

		// store pointer to look_data buffer
		look_tbl[i] = s = reinterpret_cast<char *>(&ptr[2]);

		// update max_len
		len = strlen(s);
		if (max_len < len)
			max_len = len;

		ptr += len + 3;

		// fill all empty strings between current and last one
		for (; j <= i; j++)
			look_tbl[j] = s;
	}

// fill remaining strings with "Nothing"
	for (i = j; i < 2048; i++) {
		look_tbl[i] = look_tbl[0]; // nothing
	}

// allocate space for description buffer
	desc_buf = (char *)malloc(max_len + 1);
	if (desc_buf == NULL)
		return false;

	return true;
}

const char *Look::get_description(uint16 tile_num, bool *plural) {
	const char *desc;
	char c;
	uint16 i, j;
	uint16 len;
	bool has_plural = false;

	if (tile_num >= 2048)
		return NULL;

	desc = look_tbl[tile_num];

	len = strlen(desc);
 
	for (i = 0, j = 0; i < len;) {
		if (desc[i] == '\\' || desc[i] == '/') {
			has_plural = true;
			c = desc[i];
			for (i++; Common::isAlpha(desc[i]) && i < len; i++) {

				if ((*plural && c == '\\') || (!*plural && c == '/')) {
					desc_buf[j] = desc[i];
					j++;
				}
			}
		} else {
			desc_buf[j] = desc[i];
			i++;
			j++;
		}
	}

	desc_buf[j] = desc[i];

	*plural = has_plural; //we return if this string contained a plural form.

	return desc_buf;
}

bool Look::has_plural(uint16 tile_num) {
	const char *desc;

	if (tile_num >= 2048)
		return false;

	desc = look_tbl[tile_num];

	if (desc == NULL)
		return false;

	for (; *desc != '\0'; desc++) {
		if (*desc == '\\')
			return true;
	}

	return false;
}

uint16 Look::get_max_len() {
	return max_len;
}

void Look::print() {
	uint16 i;

	for (i = 0; i < 2048; i++) {
		DEBUG(0, LEVEL_DEBUGGING, "%04d :: %s\n", i, look_tbl[i]);
	}

	return;
}

} // End of namespace Nuvie
} // End of namespace Ultima
