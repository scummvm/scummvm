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
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/core/game.h"

#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/portraits/portrait_se.h"
#include "ultima/nuvie/misc/u6_misc.h"

namespace Ultima {
namespace Nuvie {

bool PortraitSE::init() {
	Std::string filename;

	avatar_portrait_num = 0;

	width = 79;
	height = 85;

	config_get_path(config, "faces.lzc", filename);
	if (faces.open(filename, 4) == false) {
		ConsoleAddError("Opening " + filename);
		return false;
	}

	return true;
}

bool PortraitSE::load(NuvieIO *objlist) {
	// U6 only?
	objlist->seek(0x1c72);

	avatar_portrait_num = objlist->read1(); //read in the avatar portrait number from objlist.
	if (avatar_portrait_num > 0)
		avatar_portrait_num--;

	return true;
}

uint8 PortraitSE::get_portrait_num(Actor *actor) {
	uint8 num;

	if (actor == NULL)
		return NO_PORTRAIT_FOUND;

	num = actor->get_actor_num();

	return num;
}

U6Shape *PortraitSE::get_background_shape(Actor *actor) {
	U6Lib_n file;
	U6Shape *bg = new U6Shape();
	Std::string filename;
	config_get_path(config, "bkgrnd.lzc", filename);
	file.open(filename, 4, NUVIE_GAME_MD);
	unsigned char *temp_buf = file.get_item(get_background_shape_num(actor));
	bg->load(temp_buf + 8);
	free(temp_buf);

	return bg;
}

uint8 PortraitSE::get_background_shape_num(Actor *actor) {
	const struct {
		uint16 x;
		uint16 y;
		uint16 x1;
		uint16 y1;
		uint8 bg_num;
	} bg_lookup_tbl[27] = {
		{0x1E8, 0x208, 0x287, 0x25F, 0x4},
		{0x0E0, 0x260, 0x107, 0x287, 0xC},
		{0x140, 0xC0,  0x187, 0xEF,  0xC},
		{0x168, 0x218, 0x19F, 0x23F, 0xC},
		{0x290, 0x318, 0x2C7, 0x34F, 0xC},
		{0x188, 0x108, 0x1B7, 0x13F, 0xC},
		{0x3D0, 0x130, 0x3F7, 0x167, 0xC},
		{0x220, 0x120, 0x25F, 0x15F, 0xC},
		{0x228, 0x60,  0x25F, 0x8F,  0xC},
		{0x0F0, 0xA8,  0x307, 0x12F, 0x1},
		{0x1F8, 0x40,  0x257, 0x8F,  0x1},
		{0x1D8, 0x140, 0x207, 0x167, 0x1},
		{0x340, 0xE8,  0x3FF, 0x1DF, 0x1},
		{0x130, 0x130, 0x167, 0x1A7, 0x1},
		{0x168, 0x1D0, 0x209, 0x227, 0x1},
		{0x58,  0x268, 0x90,  0x2A8, 0x1},
		{0x118, 0x40,  0x13F, 0x77,  0x1},
		{0x280, 0x310, 0x29F, 0x327, 0x1},
		{0x160, 0xF8,  0x21F, 0x1A7, 0xB},
		{0x230, 0x160, 0x2C7, 0x1E7, 0xB},
		{0x100, 0x48,  0x15F, 0xA7,  0xB},
		{0x178, 0x100, 0x1C8, 0x12F, 0xB},
		{0x120, 0xB0,  0x13F, 0xDF,  0xB},
		{0x1A8, 0x78,  0x1EF, 0xB7,  0xB},
		{0x0A8, 0x238, 0x100, 0x27F, 0xB},
		{0x148, 0x228, 0x1BF, 0x287, 0xB},
		{0x0,   0x0,   0x3FF, 0x3FF, 0x2}
	};

	uint16 x = actor->get_x();
	uint16 y = actor->get_y();
	uint8 z = actor->get_z();

	if (z == 3) {
		return 10;
	} else if (z != 0) {
		return 9;
	}

	for (int i = 0; i < 27; i++) {
		if (x >= bg_lookup_tbl[i].x && y >= bg_lookup_tbl[i].y && x <= bg_lookup_tbl[i].x1 && y <= bg_lookup_tbl[i].y1) {
			if (bg_lookup_tbl[i].bg_num < 5) {
				GameClock *clock = Game::get_game()->get_clock();
				if (clock) {
					uint8 hour = clock->get_hour();
					if (hour >= 18 || hour < 5) {
						return bg_lookup_tbl[i].bg_num + 4;
					}
				}
			}
			return bg_lookup_tbl[i].bg_num;
		}
	}

	return 2;
}

unsigned char *PortraitSE::get_portrait_data(Actor *actor) {
	uint8 num = get_portrait_num(actor);
	if (num == NO_PORTRAIT_FOUND)
		return NULL;

	U6Shape *bg_shp = get_background_shape(actor);

	unsigned char *temp_buf = faces.get_item(num);
	if (!temp_buf)
		return NULL;
	U6Shape *p_shp = new U6Shape();
	p_shp->load(temp_buf + 8);
	free(temp_buf);

	//blit the background into the portrait while keeping the 1 pixel border.
	uint16 bg_w, bg_h;
	bg_shp->get_size(&bg_w, &bg_h);
	uint16 p_w, p_h;
	p_shp->get_size(&p_w, &p_h);
	unsigned char *bg_data = bg_shp->get_data();
	unsigned char *p_data = p_shp->get_data();
	p_data += p_w;
	p_data += 1;

	for (int i = 0; i < bg_h; i++) {
		for (int j = 0; j < bg_w; j++)
			if (p_data[j] == 255) {
				p_data[j] = bg_data[j];
			}
		p_data += p_w;
		bg_data += bg_w;
	}

	bg_data = (unsigned char *)malloc(p_w * p_h);
	memcpy(bg_data, p_shp->get_data(), p_w * p_h);

	delete bg_shp;
	delete p_shp;

	return bg_data;
}

} // End of namespace Nuvie
} // End of namespace Ultima
