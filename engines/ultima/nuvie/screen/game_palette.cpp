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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/screen/dither.h"
#include "ultima/nuvie/screen/game_palette.h"

namespace Ultima {
namespace Nuvie {

GamePalette::GamePalette(Screen *s, Configuration *cfg) {
	screen = s;
	config = cfg;

	palette = (uint8 *)malloc(768);
	memset(palette, 0, 768);

	this->loadPalette();

	set_palette();

	counter = 0;
}

GamePalette::~GamePalette() {
	free(palette);
}

void GamePalette::set_palette() {
	screen->set_palette(palette);
}

bool GamePalette::loadPalette() {
	uint16 i, j;
	Std::string filename;
	NuvieIOFileRead file;
	unsigned char *buf;
	uint8 *pal_ptr;
	Std::string game_name, game_id, pal_name;
	uint8 dither_mode;

	config->value("config/GameName", game_name);
	config->value("config/GameID", game_id);

	pal_name.assign(game_id);
	pal_name.append("pal");

	config_get_path(config, pal_name, filename);

	if (file.open(filename) == false) {
		DEBUG(0, LEVEL_ERROR, "loading palette.\n");
		return false;
	}

	buf = file.readAll();

	pal_ptr = palette;

	for (i = 0, j = 0; i < 256; i++, j += 3) {
		pal_ptr[0] = buf[j] << 2;
		pal_ptr[1] = buf[j + 1] << 2;
		pal_ptr[2] = buf[j + 2] << 2;
		pal_ptr += 3;
	}

	free(buf);

	/*
	    printf("GIMP Palette\nName: SE\n#\n");
	    for (int i = 0; i < 0x100; i++)
	    {
	        for (int j = 0; j < 3; j++)
	        {
	            printf("% 3d ", palette[i*3+j]);
	        }
	        printf(" untitled\n");
	    }
	*/
	dither_mode = Game::get_game()->get_dither()->get_mode();
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
		if (dither_mode == DITHER_NONE)
			bg_color = 0x31;
		else
			bg_color = 0xf;
	} else if (Game::get_game()->get_game_type() == NUVIE_GAME_MD)
		bg_color = 220;
	else // SE
		bg_color = 72;
	return true;
}

bool GamePalette::loadPaletteIntoBuffer(unsigned char *pal) {
	uint16 i, j;
	Std::string filename;
	NuvieIOFileRead file;
	unsigned char *buf;
	uint8 *pal_ptr;
	Std::string game_name, game_id, pal_name;

	config->value("config/GameName", game_name);
	config->value("config/GameID", game_id);

	pal_name.assign(game_id);
	pal_name.append("pal");

	config_get_path(config, pal_name, filename);

	if (file.open(filename) == false) {
		DEBUG(0, LEVEL_ERROR, "loading palette.\n");
		return false;
	}

	buf = file.readAll();

	pal_ptr = pal;

	for (i = 0, j = 0; i < 256; i++, j += 3) {
		pal_ptr[0] = buf[j] << 2;
		pal_ptr[1] = buf[j + 1] << 2;
		pal_ptr[2] = buf[j + 2] << 2;
		pal_ptr[3] = 0;
		pal_ptr += 4;
	}

	free(buf);

	return true;
}

void GamePalette::rotatePalette() {
	if (Game::get_game()->anims_paused())
		return;

	screen->rotate_palette(0xe0, 8); // Fires, braziers, candles
	screen->rotate_palette(0xe8, 8); // BluGlo[tm] magical items

	if (counter == 0) {
		screen->rotate_palette(0xf0, 4); //
		screen->rotate_palette(0xf4, 4); // Kitchen Cauldrons
		screen->rotate_palette(0xf8, 4); // Poison Field
		counter = 1;
	} else
		counter = 0;

}

} // End of namespace Nuvie
} // End of namespace Ultima
