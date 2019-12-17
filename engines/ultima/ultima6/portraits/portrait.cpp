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

#include "nuvieDefs.h"

#include "Configuration.h"
#include "Console.h"
#include "NuvieIOFile.h"
#include "U6Shape.h"
#include "Dither.h"
#include "Game.h"

#include "ActorManager.h"
#include "Actor.h"

#include "Portrait.h"
#include "PortraitU6.h"
#include "PortraitMD.h"
#include "PortraitSE.h"

#include "U6Lzw.h"
#include "U6misc.h"
#include "U6Lib_n.h"

#include "U6objects.h"

namespace Ultima {
namespace Ultima6 {

Portrait *newPortrait(nuvie_game_t gametype, Configuration *cfg) {
	// Correct portrait class for each game
	switch (gametype) {
	case NUVIE_GAME_U6 :
		return (Portrait *) new PortraitU6(cfg);
		break;
	case NUVIE_GAME_MD :
		return (Portrait *) new PortraitMD(cfg);
		break;
	case NUVIE_GAME_SE :
		return (Portrait *) new PortraitSE(cfg);
		break;
	}
	return NULL;
}


Portrait::Portrait(Configuration *cfg) {
	config = cfg;
	avatar_portrait_num = 0;
	width = 0;
	height = 0;
}

uint8 Portrait::get_avatar_portrait_num() {
	return get_portrait_num(Game::get_game()->get_actor_manager()->get_avatar());
}


unsigned char *Portrait::get_wou_portrait_data(U6Lib_n *lib, uint8 num) {
// MD/SE
	U6Shape *shp;
	unsigned char *shp_data;
	NuvieIOBuffer shp_buf;
	U6Lib_n shp_lib;
	unsigned char *new_portrait;
	uint16 portrait_w;
	uint16 portrait_h;

	shp_data = lib->get_item(num, NULL);
	shp_buf.open(shp_data, lib->get_item_size(num), NUVIE_BUF_NOCOPY);

	if (shp_buf.get_size() == 0) { // no portrait at that index
		free(shp_data);
		return (NULL);
	}
	shp = new U6Shape();
	shp_lib.open(&shp_buf, 4, NUVIE_GAME_SE);
	shp->load(&shp_lib, 0);
	shp->get_size(&portrait_w, &portrait_h);
	new_portrait = (unsigned char *)malloc(portrait_w * portrait_h);
	memcpy(new_portrait, shp->get_data(), portrait_w * portrait_h);
	//new_portrait=shp->get_data(); // probably need to copy here

	delete shp;
	shp_lib.close();
	free(shp_data);

	return new_portrait;
}

} // End of namespace Ultima6
} // End of namespace Ultima
