/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Isometric level module - private header

#ifndef SAGA_ISOMAP_H_
#define SAGA_ISOMAP_H_

namespace Saga {

struct ISOTILE_ENTRY {
	int tile_h;
	int unknown01;
	size_t tile_offset;
	int unknown04;
	int unknown06;

};

#define SAGA_ISOTILE_ENTRY_LEN 8
#define SAGA_ISOTILE_WIDTH 32
#define SAGA_ISOTILE_BASEHEIGHT 15

#define SAGA_METATILE_W 8
#define SAGA_METATILE_H 8
#define SAGA_METATILE_SIZE 64

#define SAGA_METAMAP_W 16
#define SAGA_METAMAP_H 16
#define SAGA_METAMAP_SIZE 256

struct ISO_METATILE_ENTRY {
	int mtile_n;
	int unknown02;
	int unknown04;
	int unknown06;
	int tile_tbl[SAGA_METATILE_SIZE];
};

#define SAGA_METATILE_ENTRY_LEN 136

class Gfx;

class IsoMap {
public:
	IsoMap(Gfx *gfx);
	int loadTileset(const byte *, size_t);
	int loadMetaTileset(const byte *, size_t);
	int loadMetamap(const byte *mm_res_p, size_t mm_res_len);
	int draw(SURFACE *dst_s);
private:
	int drawTile(SURFACE *dst_s, uint16 tile_i, int tile_x, int tile_y);
	int drawMetaTile(SURFACE *dst_s, uint16 mtile_i, int mtile_x, int mtile_y);
	int drawMetamap(SURFACE *dst_s, int map_x, int map_y);
	int _init;
	int _tiles_loaded;
	const byte *_tileres_p;
	size_t _tileres_len;
	uint16 _tile_ct;
	ISOTILE_ENTRY *_tile_tbl;

	int _mtiles_loaded;
	const byte *_mtileres_p;
	size_t _mtileres_len;
	uint16 _mtile_ct;
	ISO_METATILE_ENTRY *_mtile_tbl;

	int _metamap_loaded;
	int _metamap_n;
	uint16 _metamap_tbl[SAGA_METAMAP_SIZE];
	const byte *_mm_res_p;
	size_t _mm_res_len;

	Gfx *_gfx;
};

} // End of namespace Saga

#endif
