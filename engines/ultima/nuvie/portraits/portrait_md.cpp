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
#include "ultima/nuvie/portraits/portrait_md.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/script/script.h"

namespace Ultima {
namespace Nuvie {

bool PortraitMD::init() {
	Std::string filename;

	avatar_portrait_num = 0;

	width = 76;
	height = 83;

	config_get_path(config, "mdfaces.lzc", filename);
	if (faces.open(filename, 4) == false) {
		ConsoleAddError("Opening " + filename);
		return false;
	}

	return true;
}

bool PortraitMD::load(NuvieIO *objlist) {
	objlist->seek(OBJLIST_OFFSET_MD_GENDER);

	avatar_portrait_num = objlist->read1() == 0 ? 1 : 0; //read in the avatar portrait number from objlist.
//if(avatar_portrait_num > 0)
//  avatar_portrait_num--;

	return true;
}

uint8 PortraitMD::get_portrait_num(Actor *actor) {
	if (actor == NULL)
		return NO_PORTRAIT_FOUND;

	uint8 num = Game::get_game()->get_script()->call_get_portrait_number(actor);
	if (num != NO_PORTRAIT_FOUND)
		num++;
	return num;
}

unsigned char *PortraitMD::get_portrait_data(Actor *actor) {
	uint8 num = get_portrait_num(actor);
	if (num == NO_PORTRAIT_FOUND)
		return NULL;

	U6Shape *bg_shp = get_background_shape(num);

	unsigned char *temp_buf = faces.get_item(num);
	if (!temp_buf)
		return NULL;
	U6Shape *p_shp = new U6Shape();
	p_shp->load(temp_buf + 8);
	free(temp_buf);

	uint16 w, h;
	bg_shp->get_size(&w, &h);
	unsigned char *bg_data = bg_shp->get_data();
	unsigned char *p_data = p_shp->get_data();
	for (int i = 0; i < w * h; i++) {
		if (p_data[i] != 255) {
			bg_data[i] = p_data[i];
		}
	}

	p_data = (unsigned char *)malloc(w * h);
	memcpy(p_data, bg_data, w * h);

	delete bg_shp;
	delete p_shp;

	return p_data;
}

U6Shape *PortraitMD::get_background_shape(uint8 actor_num) {
	U6Lib_n file;
	U6Shape *bg = new U6Shape();
	Std::string filename;
	config_get_path(config, "mdback.lzc", filename);
	file.open(filename, 4, NUVIE_GAME_MD);
	unsigned char *temp_buf = file.get_item(get_background_shape_num(actor_num));
	bg->load(temp_buf + 8);
	free(temp_buf);

	return bg;
}

uint8 PortraitMD::get_background_shape_num(uint8 actor_num) {
	const uint8 bg_tbl[] = {
		0x22, 0x17, 0x50, 0x0, 0x0, 0x0, 0x0, 0x0, 0x56, 0x27, 0x0, 0x0, 0x55, 0x45,
		0x70, 0x0, 0x53, 0x25, 0x45, 0x15, 0x17, 0x37, 0x45, 0x32, 0x24,
		0x53, 0x21, 0x42, 0x13, 0x66, 0x61, 0x20, 0x67, 0x23, 0x15, 0x60,
		0x0, 0x0, 0x0, 0x0, 0x37, 0x45, 0x32, 0x24, 0x75, 0x73, 0x50, 0x12,
		0x51, 0x2, 0x65, 0x61, 0x45, 0x46, 0x31, 0x0, 0x24, 0x77, 0x6, 0x50,
		0
	};

	actor_num--;

	//FIXME add logic word_4115A  return 0;

	if (actor_num > 121) {
		return NUVIE_RAND() % 7;
	}

	uint8 v = bg_tbl[actor_num / 2];

	if ((actor_num & 1) == 0) {
		return v >> 4;
	}

	return v & 0xf;
}

} // End of namespace Nuvie
} // End of namespace Ultima
