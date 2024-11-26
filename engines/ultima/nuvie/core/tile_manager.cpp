/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/files/nuvie_bmp_file.h"
#include "ultima/nuvie/files/u6_lib_n.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/screen/dither.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/look.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/gui/gui.h"

namespace Ultima {
namespace Nuvie {

#define NUM_ORIGINAL_TILES 2048

static const char article_tbl[][5] = {"", "a ", "an ", "the "};

static const uint16 U6_ANIM_SRC_TILE[32] = {0x16, 0x16, 0x1a, 0x1a, 0x1e, 0x1e, 0x12, 0x12,
											0x1a, 0x1e, 0x16, 0x12, 0x16, 0x1a, 0x1e, 0x12,
											0x1a, 0x1e, 0x1e, 0x12, 0x12, 0x16, 0x16, 0x1a,
											0x12, 0x16, 0x1e, 0x1a, 0x1a, 0x1e, 0x12, 0x16
										   };

//static const uint16 U6_WALL_TYPES[1][2] = {{156,176}};

static const Tile gump_cursor = {
	0,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	0,
	//uint8 qty;
	//uint8 flags;

	0,
	0,
	0,

	{
		15, 15, 15, 15, 255, 255, 255, 255, 255, 255, 255, 255, 15, 15, 15, 15,
		15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 15,
		15, 15, 15, 15, 255, 255, 255, 255, 255, 255, 255, 255, 15, 15, 15, 15
	}
};

TileManager::TileManager(const Configuration *cfg) : desc_buf(nullptr), config(cfg),
		look(nullptr), game_counter(0), rgame_counter(0), extendedTiles(nullptr),
		numTiles(NUM_ORIGINAL_TILES) {
	memset(tileindex, 0, sizeof(tileindex));
	memset(tile, 0, sizeof(tile));
	memset(&animdata, 0, sizeof animdata);

	config->value("config/GameType", game_type);
}

TileManager::~TileManager() {
// remove tiles
	free(desc_buf);
	delete look;
	if (extendedTiles) {
		free(extendedTiles);
	}
}

bool TileManager::loadTiles() {
	Common::Path maptiles_path, masktype_path, path;
	NuvieIOFileRead objtiles_vga;
	NuvieIOFileRead tileindx_vga;
	NuvieIOFileRead file;
	U6Lib_n lib_file;
	U6Lzw *lzw;
	uint32 tile_offset;

	unsigned char *tile_data = nullptr;
	uint32 maptiles_size = 0;
	uint32 objtiles_size;

	unsigned char *masktype = nullptr;
	uint32 masktype_size;
	uint16 i;

	Dither *dither;

	dither = Game::get_game()->get_dither();


	config_get_path(config, "maptiles.vga", maptiles_path);
	config_get_path(config, "masktype.vga", masktype_path);

	lzw = new U6Lzw();

	switch (game_type) {
	case NUVIE_GAME_U6 :
		tile_data = lzw->decompress_file(maptiles_path, maptiles_size);
		if (tile_data == nullptr) {
			ConsoleAddError("Decompressing " + maptiles_path.toString());
			return false;
		}

		masktype = lzw->decompress_file(masktype_path, masktype_size);
		if (masktype == nullptr) {
			ConsoleAddError("Decompressing " + masktype_path.toString());
			return false;
		}
		break;
	case NUVIE_GAME_MD :
	case NUVIE_GAME_SE :
		if (lib_file.open(maptiles_path, 4, game_type) == false) {
			ConsoleAddError("Opening " + maptiles_path.toString());
			return false;
		}
		maptiles_size = lib_file.get_item_size(0);

		tile_data = lib_file.get_item(0);
		lib_file.close();

		if (lib_file.open(masktype_path, 4, game_type) == false) {
			ConsoleAddError("Opening " + masktype_path.toString());
			return false;
		}
		//masktype_size = lib_file.get_item_size(0);

		masktype = lib_file.get_item(0);
		lib_file.close();
		break;
	}

	if (tile_data == nullptr) {
		ConsoleAddError("Loading maptiles.vga");
		return false;
	}

	if (masktype == nullptr) {
		ConsoleAddError("Loading masktype.vga");
		return false;
	}

	config_get_path(config, "objtiles.vga", path);
	if (objtiles_vga.open(path) == false) {
		ConsoleAddError("Opening " + path.toString());
		return false;
	}

	objtiles_size = objtiles_vga.get_size();

	tile_data = (unsigned char *)nuvie_realloc(tile_data, maptiles_size + objtiles_size);

	objtiles_vga.readToBuf(&tile_data[maptiles_size], objtiles_size);

	config_get_path(config, "tileindx.vga", path);

	if (tileindx_vga.open(path) == false) {
		ConsoleAddError("Opening " + path.toString());
		return false;
	}

	for (i = 0; i < 2048; i++) {
		tile_offset = tileindx_vga.read2() * 16;
		tile[i].tile_num = i;

		tile[i].transparent = false;

		switch (masktype[i]) {
		case U6TILE_TRANS :
			tile[i].transparent = true;
			memcpy(tile[i].data, &tile_data[tile_offset], 256);
			break;

		case U6TILE_PLAIN :
			memcpy(tile[i].data, &tile_data[tile_offset], 256);
			break;

		case U6TILE_PBLCK :
			tile[i].transparent = true;
			decodePixelBlockTile(&tile_data[tile_offset], i);
			break;
		}

		dither->dither_bitmap(tile[i].data, 16, 16, tile[i].transparent);

		tileindex[i] = i; //set all tile indexs to default value. this is changed in update() for animated tiles
	}

	loadAnimData();
	loadTileFlag();

	free(masktype);
	free(tile_data);

	look = new Look(config);
	if (look->init() == false) {
		ConsoleAddError("Initialising Look Class");
		return false;
	}

	desc_buf = (char *)malloc(look->get_max_len() + 6); // add space for "%03d \n\0" or "the \n\0"
	if (desc_buf == nullptr) {
		ConsoleAddError("Allocating desc_buf");
		return false;
	}

	loadAnimMask();

#ifdef TILEMANAGER_DEBUG

	look->print();

	DEBUG(0, LEVEL_DEBUGGING, "Dumping tile flags:");
	for (i = 0; i < 2048; i++) {
		bool plural;
		DEBUG(1, LEVEL_DEBUGGING, "%04d : ", i);
		print_b(LEVEL_DEBUGGING, tile[i].flags1);
		DEBUG(1, LEVEL_DEBUGGING, " ");
		print_b(LEVEL_DEBUGGING, tile[i].flags2);
		DEBUG(1, LEVEL_DEBUGGING, " ");
		print_b(LEVEL_DEBUGGING, tile[i].flags3);
		DEBUG(1, LEVEL_DEBUGGING, " %s\n", look->get_description(i, &plural));
	}
#endif

	delete lzw;

	return true;
}

Tile *TileManager::get_tile(uint16 tile_num) {
	if (tile_num < NUM_ORIGINAL_TILES) {
		return &tile[tileindex[tile_num]];
	}

	return get_extended_tile(tile_num);
}

Tile *TileManager::get_anim_base_tile(uint16 tile_num) {
	return &tile[tileindex[U6_ANIM_SRC_TILE[tile_num - 16] / 2]];
}

Tile *TileManager::get_original_tile(uint16 tile_num) {
	if (tile_num < NUM_ORIGINAL_TILES) {
		return &tile[tile_num];
	}

	return get_extended_tile(tile_num);
}


Tile *TileManager::get_extended_tile(uint16 tile_num) {
	if (tile_num <= numTiles) {
		return &extendedTiles[tile_num - 2048];
	}
	return &tile[0];
}

// set entry in tileindex[] to tile num
void TileManager::set_tile_index(uint16 tile_index, uint16 tile_num) {
	tileindex[tile_index] = tile_num;
}


void TileManager::set_anim_loop(uint16 tile_num, sint8 loopc, uint8 loop) {
	for (uint32 i = 0; i < 32; i++)
		if (animdata.tile_to_animate[i] == tile_num) {
			animdata.loop_count[i] = loopc;
			animdata.loop[i] = loop;
		}
}


const char *TileManager::lookAtTile(uint16 tile_num, uint16 qty, bool show_prefix) {
	const char *desc;
	bool plural;
	Tile *tileP;

	tileP = get_original_tile(tile_num);

	if (qty > 1)
		plural = true;
	else
		plural = false;

	desc = look->get_description(tileP->tile_num, &plural);
	if (show_prefix == false)
		return desc;

	if (qty > 0 &&
	        (plural || Game::get_game()->get_game_type() == NUVIE_GAME_SE))
		Common::sprintf_s(desc_buf, look->get_max_len() + 6, "%u %s", qty, desc);
	else
		Common::sprintf_s(desc_buf, look->get_max_len() + 6, "%s%s", article_tbl[tileP->article_n], desc);

	DEBUG(0, LEVEL_DEBUGGING, "%s (%x): flags1:", desc_buf, tile_num);
	print_b(LEVEL_INFORMATIONAL, tileP->flags1);
	DEBUG(1, LEVEL_DEBUGGING, " f2:");
	print_b(LEVEL_INFORMATIONAL, tileP->flags2);
	DEBUG(1, LEVEL_DEBUGGING, " f3:");
	print_b(LEVEL_INFORMATIONAL, tileP->flags3);
	DEBUG(1, LEVEL_DEBUGGING, "\n");

	return desc_buf;
}

bool TileManager::tile_is_stackable(uint16 tile_num) {
	return look->has_plural(tile_num); // luteijn: FIXME, doesn't take into account Zu Ylem, Silver Snake Venom, and possibly other stackables that don't have a plural defined.
}

void TileManager::update() {
	uint16 i;
	uint16 current_anim_frame = 0;
	uint16 prev_tileindex;
	uint8 current_hour = Game::get_game()->get_clock()->get_hour();
	static sint8 last_hour = -1;

// cycle animated tiles

	for (i = 0; i < animdata.number_of_tiles_to_animate; i++) {
		if (animdata.loop_count[i] != 0) {
			if (animdata.loop[i] == 0) // get next frame
				current_anim_frame = (game_counter & animdata.and_masks[i]) >> animdata.shift_values[i];
			else if (animdata.loop[i] == 1) // get previous frame
				current_anim_frame = (rgame_counter & animdata.and_masks[i]) >> animdata.shift_values[i];
			prev_tileindex = tileindex[animdata.tile_to_animate[i]];
			tileindex[animdata.tile_to_animate[i]] = tileindex[animdata.first_anim_frame[i] + current_anim_frame];
			// loop complete if back to first frame (and not infinite loop)
			if (animdata.loop_count[i] > 0
			        && tileindex[animdata.tile_to_animate[i]] != prev_tileindex
			        && tileindex[animdata.tile_to_animate[i]] == tileindex[animdata.first_anim_frame[i]])
				--animdata.loop_count[i];
		} else // not animating
			tileindex[animdata.tile_to_animate[i]] = tileindex[animdata.first_anim_frame[i]];
	}

	if (Game::get_game()->anims_paused() == false) { // update counter
		if (game_counter == 65535)
			game_counter = 0;
		else
			game_counter++;
		if (rgame_counter == 0)
			rgame_counter = 65535;
		else
			rgame_counter--;
	}
// cycle time-based animations
	if (current_hour != last_hour)
		update_timed_tiles(current_hour);
	last_hour = current_hour;
}


bool TileManager::loadTileFlag() {
	Common::Path filename;
	NuvieIOFileRead file;
	uint16 i;

	config_get_path(config, "tileflag", filename);

	if (file.open(filename) == false)
		return false;

	for (i = 0; i < 2048; i++) {
		tile[i].flags1 = file.read1();
		if (tile[i].flags1 & 0x2)
			tile[i].passable = false;
		else
			tile[i].passable = true;

		if (tile[i].flags1 & 0x1)
			tile[i].water = true;
		else
			tile[i].water = false;

		if (tile[i].flags1 & 0x8)
			tile[i].damages = true;
		else
			tile[i].damages = false;
	}

	for (i = 0; i < 2048; i++) {
		tile[i].flags2 = file.read1();
		if (tile[i].flags2 & 0x10)
			tile[i].toptile = true;
		else
			tile[i].toptile = false;

		if ((tile[i].flags2 & 0x4) || (tile[i].flags2 & 0x8))
			tile[i].boundary = true;
		else
			tile[i].boundary = false;


		if (tile[i].flags2 & 0x40)
			tile[i].dbl_height = true;
		else
			tile[i].dbl_height = false;

		if (tile[i].flags2 & 0x80)
			tile[i].dbl_width = true;
		else
			tile[i].dbl_width = false;
	}

	file.seek(0x1400);

	for (i = 0; i < 2048; i++) { // '', 'a', 'an', 'the'
		tile[i].flags3 = file.read1();
		tile[i].article_n = (tile[i].flags3 & 0xC0) >> 6;
	}

	return true;
}

bool TileManager::loadAnimData() {
	Common::Path filename;
	NuvieIOFileRead file;
	int gameType;
	config->value("config/GameType", gameType);
	config_get_path(config, "animdata", filename);

	if (file.open(filename) == false)
		return false;

	if (file.get_size() != 194)
		return false;

	animdata.number_of_tiles_to_animate = file.read2();

	for (int i = 0; i < 32; i++) {
		animdata.tile_to_animate[i] = file.read2();
	}

	for (int i = 0; i < 32; i++) {
		animdata.first_anim_frame[i] = file.read2();
	}

	for (int i = 0; i < 32; i++) {
		animdata.and_masks[i] = file.read1();
	}

	for (int i = 0; i < 32; i++) {
		animdata.shift_values[i] = file.read1();
	}

	for (int i = 0; i < 32; i++) { // FIXME: any data on which tiles don't start as animated?
		animdata.loop[i] = 0; // loop forwards
		if ((gameType == NUVIE_GAME_U6 &&
		        (animdata.tile_to_animate[i] == 862 // Crank
		         || animdata.tile_to_animate[i] == 1009 // Crank
		         || animdata.tile_to_animate[i] == 1020)) // Chain
		        || (gameType == NUVIE_GAME_MD
		            && ((animdata.tile_to_animate[i] >= 1 && animdata.tile_to_animate[i] <= 4) // cistern
		                || (animdata.tile_to_animate[i] >= 16 && animdata.tile_to_animate[i] <= 23) // canal
		                || (animdata.tile_to_animate[i] >= 616 && animdata.tile_to_animate[i] <= 627) // watch --pu62 lists as 416-427
		                || animdata.tile_to_animate[i] == 1992
		                || animdata.tile_to_animate[i] == 1993
		                || animdata.tile_to_animate[i] == 1980
		                || animdata.tile_to_animate[i] == 1981)))

			animdata.loop_count[i] = 0; // don't start animated
		else
			animdata.loop_count[i] = -1; // infinite animation
	}

	return true;
}

void TileManager::decodePixelBlockTile(const unsigned char *tile_data, uint16 tile_num) {
	uint8 len;
	uint16 disp;
	uint8 x;
	const unsigned char *ptr;
	unsigned char *data_ptr;

// num_blocks = tile_data[0];

	ptr = &tile_data[1];

	data_ptr = tile[tile_num].data;

	memset(data_ptr, 0xff, 256); //set all pixels to transparent.

	for (;;) {
		disp = (ptr[0] + (ptr[1] << 8));

		x = disp % 160 + (disp >= 1760 ? 160 : 0);

		len = ptr[2];

		if (len == 0)
			break;

		data_ptr += x;

		memcpy(data_ptr, &ptr[3], len);

		data_ptr += len;

		ptr += (3 + len);
	}

	return;
}


bool TileManager::loadAnimMask() {
	Common::Path filename;
	U6Lzw lzw;
	uint16 i;
	unsigned char *animmask;
	unsigned char *mask_ptr;
	uint32 animmask_size;

	unsigned char *tile_data;
	uint16 bytes2clear;
	uint16 displacement;
	int gameType;

	config->value("config/GameType", gameType);
	if (gameType != NUVIE_GAME_U6)               //only U6 has animmask.vga
		return true;

	config_get_path(config, "animmask.vga", filename);

	animmask = lzw.decompress_file(filename, animmask_size);

	if (animmask == nullptr)
		return false;

	for (i = 0; i < 32; i++) { // Make the 32 tiles from index 16 onwards transparent with data from animmask.vga
		tile_data = tile[16 + i].data;
		tile[16 + i].transparent = true;

		mask_ptr = animmask + i * 64;
		bytes2clear = mask_ptr[0];

		if (bytes2clear != 0)
			memset(tile_data, 0xff, bytes2clear);

		tile_data += bytes2clear;
		mask_ptr++;

		displacement = mask_ptr[0];
		bytes2clear = mask_ptr[1];

		mask_ptr += 2;

		for (; displacement != 0 && bytes2clear != 0; mask_ptr += 2) {
			tile_data += displacement;

			memset(tile_data, 0xff, bytes2clear);
			tile_data += bytes2clear;

			displacement = mask_ptr[0];
			bytes2clear = mask_ptr[1];
		}
	}

	free(animmask);

	return true;
}


/* Update tiles for timed-based animations.
 */
void TileManager::update_timed_tiles(uint8 hour) {
	uint16 new_tile;
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
		// sundials
		if (hour >= 5 && hour <= 6)
			new_tile = 328;
		else if (hour >= 7 && hour <= 8)
			new_tile = 329;
		else if (hour >= 9 && hour <= 10)
			new_tile = 330;
		else if (hour >= 11 && hour <= 12)
			new_tile = 331;
		else if (hour >= 13 && hour <= 14)
			new_tile = 332;
		else if (hour >= 15 && hour <= 16)
			new_tile = 333;
		else if (hour >= 17 && hour <= 18)
			new_tile = 334;
		else if (hour >= 19 && hour <= 20)
			new_tile = 335;
		else // 9pm to 5am
			new_tile = 861;
		set_tile_index(861, new_tile);
	}

}

void TileManager::set_anim_first_frame(uint16 anim_number, uint16 new_start_tile_num) {
	if (anim_number < animdata.number_of_tiles_to_animate) {
		animdata.first_anim_frame[anim_number] = new_start_tile_num;
	}
}

/* Returns tile rotated about the center by `rotate' degrees. (8-bit; clipped to
 * standard 16x16 size) It must be deleted after use.
 * **Fixed-point rotate function taken from the SDL Graphics Extension library
 * (SGE) (c)1999-2003 Anders Lindstr�m, licensed under LGPL v2+.**
 */
Tile *TileManager::get_rotated_tile(const Tile *tileP, float rotate, uint8 src_y_offset) {
	Tile *new_tile = new Tile(*tileP); // retain properties of original tileP
	get_rotated_tile(tileP, new_tile, rotate, src_y_offset);

	return new_tile;
}

void TileManager::get_rotated_tile(const Tile *tileP, Tile *dest_tile, float rotate, uint8 src_y_offset) {
	unsigned char tile_data[256];

	memset(&dest_tile->data, 255, 256); // fill output with transparent color

	int32 dy, sx, sy;
	int16 rx, ry;
	uint16 px = 8, py = 8; // rotate around these coordinates
	uint16 xmin = 0, xmax = 15, ymin = 0, ymax = 15; // size
	uint16 sxmin = xmin, sxmax = xmax, symin = ymin, symax = ymax;
	uint16 qx = 8, qy = 8; // ?? don't remember

	float theta = float(rotate * M_PI / 180.0); /* Convert to radians.  */

	int32 const stx = int32((sin(theta)) * 8192.0);
	int32 const ctx = int32((cos(theta)) * 8192.0);
	int32 const sty = int32((sin(theta)) * 8192.0);
	int32 const cty = int32((cos(theta)) * 8192.0);
	int32 const mx = int32(px * 8192.0);
	int32 const my = int32(py * 8192.0);

	int32 const dx = xmin - qx;
	int32 const ctdx = ctx * dx;
	int32 const stdx = sty * dx;

	int32 const src_pitch = 16;
	int32 const dst_pitch = 16;
	uint8 const *src_row = (uint8 const *)&tileP->data;
	uint8 *dst_pixels = (uint8 *)&dest_tile->data;
	uint8 *dst_row;

	if (src_y_offset > 0 && src_y_offset < 16) { //shift source down before rotating. This is used by bolt and arrow tiles.
		memset(&tile_data, 255, 256);
		memcpy(&tile_data[src_y_offset * 16], &tileP->data, 256 - (src_y_offset * 16));
		src_row = (uint8 *)&tile_data;
	}

	for (uint32 y = ymin; y < ymax; y++) {
		dy = y - qy;

		sx = int32(ctdx  + stx * dy + mx); /* Compute source anchor points */
		sy = int32(cty * dy - stdx  + my);

		/* Calculate pointer to dst surface */
		dst_row = (uint8 *)dst_pixels + y * dst_pitch;

		for (uint32 x = xmin; x < xmax; x++) {
			rx = int16(sx >> 13); /* Convert from fixed-point */
			ry = int16(sy >> 13);

			/* Make sure the source pixel is actually in the source image. */
			if ((rx >= sxmin) && (rx <= sxmax) && (ry >= symin) && (ry <= symax))
				*(dst_row + x) = *(src_row + ry * src_pitch + rx);

			sx += ctx;  /* Incremental transformations */
			sy -= sty;
		}
	}


	//memcpy(&dest_tile->data, &tile_data, 256); // replace data

	return;
}


#if 0 /* old */
Tile *TileManager::get_rotated_tile(Tile *tile, float rotate) {
	float angle = (rotate != 0) ? (rotate * M_PI) / 180.0 : 0; // radians
	const float mul_x1 = cos(angle);  // | x1  y1 |
	const float mul_y1 = sin(angle);  // | x2  y2 |
	const float mul_x2 = -mul_y1;
	const float mul_y2 = mul_x1;
	unsigned char tile_data[256];
	unsigned char *input = (unsigned char *)&tile->data, *output;

	memset(&tile_data, 255, 256); // fill output with transparent color

	for (sint8 y = -8; y < 8; y++) { // scan input pixels
		for (sint8 x = -8; x < 8; x++) {
			sint8 rx = (sint8)rint((x * mul_x1) + (y * mul_x2)); // calculate target pixel
			sint8 ry = (sint8)rint((x * mul_y1) + (y * mul_y2));
			if (rx >= -8 && rx <= 7 && ry >= -8 && ry <= 7) {
				output = (unsigned char *)&tile_data;
				output += (ry + 8) * 16;
				output[rx + 8] = input[x + 8]; // copy
				if (rx <= 6) output[rx + 8 + 1] = input[x + 8]; // copy again to adjacent pixel
			}
		}
		input += 16; // next line
	}

	Tile *new_tile = new Tile(*tile); // retain properties of original tile
	memcpy(&new_tile->data, &tile_data, 256); // replace data
	return new_tile;
}
#endif

Tile *TileManager::get_cursor_tile() {
	Tile *cursor_tile = nullptr;
	switch (game_type) {
	case NUVIE_GAME_U6 :
		cursor_tile = get_tile(365);
		break;

	case NUVIE_GAME_MD :
		cursor_tile = get_tile(265);
		break;

	case NUVIE_GAME_SE :
		cursor_tile = get_tile(381);
		break;
	}

	return cursor_tile;
}

Tile *TileManager::get_use_tile() {
	Tile *use_tile = nullptr;
	switch (game_type) {
	case NUVIE_GAME_U6 :
		use_tile = get_tile(364);
		break;

	case NUVIE_GAME_MD :
		use_tile = get_tile(264);
		break;

	case NUVIE_GAME_SE :
		use_tile = get_tile(380);
		break;
	}

	return use_tile;
}

const Tile *TileManager::get_gump_cursor_tile() {
	return &gump_cursor;
}

Tile *TileManager::loadCustomTiles(const Common::Path &filename, bool overwrite_tiles, bool copy_tileflags, uint16 tile_num_start_offset) {
	NuvieBmpFile bmp;

	if (bmp.load(filename) == false) {
		return nullptr;
	}

	unsigned char *tile_data = bmp.getRawIndexedData();

	uint16 w = bmp.getWidth();
	uint16 h = bmp.getHeight();
	uint16 pitch = w;

	if (w % 16 != 0 || h % 16 != 0) {
		return nullptr;
	}

	w = w / 16;
	h = h / 16;

	uint16 num_tiles = w * h;

	Tile *newTilePtr = nullptr;
	Tile *origTile = nullptr;
	if (overwrite_tiles) {
		newTilePtr = get_original_tile(tile_num_start_offset);
	} else {
		newTilePtr = addNewTiles(num_tiles);
	}

	if (copy_tileflags) {
		origTile = get_tile(tile_num_start_offset);
	}

	Tile *t = newTilePtr;

	Dither *dither = Game::get_game()->get_dither();

	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			unsigned char *data = tile_data + (y * 16 * pitch) + (x * 16);
			for (uint16 i = 0; i < 16; i++) {
				memcpy(&t->data[i * 16], data, 16);
				data += pitch;
			}

			if (origTile) {
				copyTileMetaData(t, origTile);
				origTile++;
			}
			dither->dither_bitmap(t->data, 16, 16, t->transparent);
			t++;
		}
	}

	return newTilePtr;
}

void TileManager::copyTileMetaData(Tile *dest, Tile *src) {
	dest->passable = src->passable;
	dest->water = src->water;
	dest->toptile = src->toptile;
	dest->dbl_width = src->dbl_width;
	dest->dbl_height = src->dbl_height;
	dest->transparent = src->transparent;
	dest->boundary = src->boundary;
	dest->damages = src->damages;
	dest->article_n = src->article_n;

	dest->flags1 = src->flags1;
	dest->flags2 = src->flags2;
	dest->flags3 = src->flags3;
}

Tile *TileManager::addNewTiles(uint16 num_tiles) {
	Tile *tileDataPtr = (Tile *)realloc(extendedTiles, sizeof(Tile) * (numTiles - NUM_ORIGINAL_TILES + num_tiles));
	if (tileDataPtr != nullptr) {
		extendedTiles = tileDataPtr;
	}

	tileDataPtr += (numTiles - NUM_ORIGINAL_TILES);

	Tile *t = tileDataPtr;
	for (uint16 i = 0; i < num_tiles; i++, t++) {
		t->tile_num = numTiles + i;
	}

	numTiles += num_tiles;

	return tileDataPtr;
}

void TileManager::freeCustomTiles() {
	if (extendedTiles) {
		free(extendedTiles);
		extendedTiles = nullptr;
		numTiles = NUM_ORIGINAL_TILES;
	}
}

void TileManager::exportTilesetToBmpFile(const Common::Path &filename, bool fixupU6Shoreline) {
	NuvieBmpFile bmp;

	unsigned char pal[256 * 4];

	Game::get_game()->get_palette()->loadPaletteIntoBuffer(pal);

	//Magic background colour
	pal[255 * 4] = 0;
	pal[255 * 4 + 1] = 0xdf;
	pal[255 * 4 + 2] = 0xfc;

	bmp.initNewBlankImage(32 * 16, 64 * 16, pal);

	unsigned char *data = bmp.getRawIndexedData();

	for (uint8 i = 0; i < 64; i++) {
		for (uint8 j = 0; j < 32; j++) {
			if (fixupU6Shoreline && game_type == NUVIE_GAME_U6 && (i * 32 + j) >= 16 && (i * 32 + j) < 48) { //lay down the base tile for shoreline tiles
				writeBmpTileData(&data[i * 16 * 512 + j * 16], get_anim_base_tile(i * 32 + j), false);
				writeBmpTileData(&data[i * 16 * 512 + j * 16], &tile[tileindex[i * 32 + j]], true);
			} else {
				writeBmpTileData(&data[i * 16 * 512 + j * 16], &tile[tileindex[i * 32 + j]], false);
			}
		}
	}
	bmp.save(filename);
}

void TileManager::writeBmpTileData(unsigned char *data, const Tile *t, bool transparent) {
	for (uint8 y = 0; y < 16; y++) {
		for (uint8 x = 0; x < 16; x++) {
			if (!transparent || t->data[y * 16 + x] != 255) {
				data[x] = t->data[y * 16 + x];
			}
		}
		data += 512;
	}
}

void TileManager::anim_play_repeated(uint8 anim_index) {
	if (anim_index < get_number_of_animations()) {
		animdata.loop_count[anim_index] = -1; // infinite animation
	}
}

void TileManager::anim_stop_playing(uint8 anim_index) {
	if (anim_index < get_number_of_animations()) {
		animdata.loop_count[anim_index] = 0; // stop animation
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
