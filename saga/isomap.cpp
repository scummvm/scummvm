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

// Isometric level module
#include "saga/saga.h"

#include "saga/game_mod.h"
#include "saga/gfx.h"

#include "saga/isomap.h"

namespace Saga {

IsoMap::IsoMap(Gfx *gfx) {
	_gfx = gfx;
	_init = 1;
	_tiles_loaded = 0;
}

int IsoMap::loadTileset(const byte *tileres_p, size_t tileres_len) {
	ISOTILE_ENTRY first_entry;
	ISOTILE_ENTRY *tile_tbl;

	uint16 i;

	assert((_init) && (!_tiles_loaded));
	assert((tileres_p != NULL) && (tileres_len > 0));

	MemoryReadStream readS(tileres_p, tileres_len);

	readS.readUint16LE(); // skip
	first_entry.tile_offset = readS.readUint16LE();

	_tile_ct = first_entry.tile_offset / SAGA_ISOTILE_ENTRY_LEN;

	readS.seek(0);

	tile_tbl = (ISOTILE_ENTRY *)malloc(_tile_ct * sizeof *tile_tbl);
	if (tile_tbl == NULL) {
		return MEM;
	}

	for (i = 0; i < _tile_ct; i++) {
		tile_tbl[i].tile_h = readS.readByte();
		tile_tbl[i].mask_rule = readS.readByte();
		tile_tbl[i].tile_offset = readS.readUint16LE();
		tile_tbl[i].terrain_mask = readS.readSint16LE();
		tile_tbl[i].mask = readS.readSint16LE();
	}

	_tiles_loaded = 1;
	_tile_tbl = tile_tbl;
	_tileres_p = tileres_p;
	_tileres_len = tileres_len;

	return SUCCESS;
}

int IsoMap::loadMetaTileset(const byte *mtileres_p, size_t mtileres_len) {
	ISO_METATILE_ENTRY *mtile_tbl;
	uint16 mtile_ct;
	uint16 ct;
	int i;

	assert(_init);
	assert((mtileres_p != NULL) && (mtileres_len > 0));

	MemoryReadStream readS(mtileres_p, mtileres_len);

	mtile_ct = mtileres_len / SAGA_METATILE_ENTRY_LEN;
	mtile_tbl = (ISO_METATILE_ENTRY *)malloc(mtile_ct * sizeof *mtile_tbl);
	if (mtile_tbl == NULL) {
		return MEM;
	}

	for (ct = 0; ct < mtile_ct; ct++) {
		mtile_tbl[ct].mtile_n = readS.readUint16LE();
		mtile_tbl[ct].height = readS.readSint16LE();
		mtile_tbl[ct].highest_pixel = readS.readSint16LE();
		mtile_tbl[ct].v_bits = readS.readByte();
		mtile_tbl[ct].u_bits = readS.readByte();
		for (i = 0; i < SAGA_METATILE_SIZE; i++) {
			mtile_tbl[ct].tile_tbl[i] = readS.readUint16LE();
		}
	}

	_mtile_ct = mtile_ct;
	_mtile_tbl = mtile_tbl;
	_mtileres_p = mtileres_p;
	_mtileres_len = mtileres_len;

	_mtiles_loaded = 1;

	return SUCCESS;
}

int IsoMap::loadMetamap(const byte *mm_res_p, size_t mm_res_len) {
	int i;

	MemoryReadStream readS(mm_res_p, mm_res_len);
	_metamap_n = readS.readSint16LE();

	for (i = 0; i < SAGA_METAMAP_SIZE; i++) {
		_metamap_tbl[i] = readS.readUint16LE();
	}

	_mm_res_p = mm_res_p;
	_mm_res_len = mm_res_len;
	_metamap_loaded = 1;

	return SUCCESS;
}

int IsoMap::draw(SURFACE *dst_s) {
	GAME_DISPLAYINFO disp_info;
	GAME_GetDisplayInfo(&disp_info);
	Rect iso_rect(disp_info.logical_w, disp_info.scene_h);
	_gfx->drawRect(dst_s, &iso_rect, 0);
	drawMetamap(dst_s, -1000, -500);

	return SUCCESS;
}

int IsoMap::drawMetamap(SURFACE *dst_s, int map_x, int map_y) {
	int meta_base_x = map_x;
	int meta_base_y = map_y;
	int meta_xi;
	int meta_yi;
	int meta_x;
	int meta_y;
	int meta_idx;

	for (meta_yi = SAGA_METAMAP_H - 1; meta_yi >= 0; meta_yi--) {
		meta_x = meta_base_x;
		meta_y = meta_base_y;
		for (meta_xi = SAGA_METAMAP_W - 1; meta_xi >= 0; meta_xi--) {
			meta_idx = meta_xi + (meta_yi * 16);
			drawMetaTile(dst_s, _metamap_tbl[meta_idx], meta_x, meta_y);
			meta_x += 128;
			meta_y += 64;
		}

		meta_base_x -= 128;
		meta_base_y += 64;
	}

	return SUCCESS;
}

int IsoMap::drawMetaTile(SURFACE *dst_s, uint16 mtile_i, int mtile_x, int mtile_y) {
	int tile_xi;
	int tile_yi;
	int tile_x;
	int tile_y;
	int tile_base_x;
	int tile_base_y;
	int tile_i;
	ISO_METATILE_ENTRY *mtile_p;
	assert(_init && _mtiles_loaded);

	if (mtile_i >= _mtile_ct) {
		return FAILURE;
	}

	mtile_p = &_mtile_tbl[mtile_i];

	tile_base_x = mtile_x;
	tile_base_y = mtile_y;

	for (tile_yi = SAGA_METATILE_H - 1; tile_yi >= 0; tile_yi--) {
		tile_y = tile_base_y;
		tile_x = tile_base_x;
		for (tile_xi = SAGA_METATILE_W - 1; tile_xi >= 0; tile_xi--) {
			tile_i = tile_xi + (tile_yi * SAGA_METATILE_W);
			drawTile(dst_s, mtile_p->tile_tbl[tile_i], tile_x, tile_y);
			tile_x += SAGA_ISOTILE_WIDTH / 2;
			tile_y += SAGA_ISOTILE_BASEHEIGHT / 2 + 1;
		}
		tile_base_x -= SAGA_ISOTILE_WIDTH / 2;
		tile_base_y += SAGA_ISOTILE_BASEHEIGHT / 2 + 1;
	}

	return SUCCESS;
}

int IsoMap::drawTile(SURFACE *dst_s, uint16 tile_i, int tile_x, int tile_y) {
	const byte *tile_p;
	const byte *read_p;
	byte *draw_p;
	int draw_x;
	int draw_y;
	int tile_h;
	int w_count = 0;
	int row;
	int bg_runct;
	int fg_runct;
	int ct;

	assert(_init && _tiles_loaded);

	if (tile_i >= _tile_ct) {
		return FAILURE;
	}

	/* temporary x clip */
	if (tile_x < 0) {
		return SUCCESS;
	}

	/* temporary x clip */
	if (tile_x >= 320 - 32) {
		return SUCCESS;
	}

	tile_p = _tileres_p + _tile_tbl[tile_i].tile_offset;
	tile_h = _tile_tbl[tile_i].tile_h;

	read_p = tile_p;
	draw_p = dst_s->buf + tile_x + (tile_y * dst_s->buf_pitch);

	draw_x = tile_x;
	draw_y = tile_y;

	if (tile_h > SAGA_ISOTILE_BASEHEIGHT) {
		draw_y = tile_y - (tile_h - SAGA_ISOTILE_BASEHEIGHT);
	}

	// temporary y clip
	if (draw_y < 0) {
		return SUCCESS;
	}

	for (row = 0; row < tile_h; row++) {
		draw_p = dst_s->buf + draw_x + ((draw_y + row) * dst_s->buf_pitch);
		w_count = 0;

		// temporary y clip
		if ((draw_y + row) >= 137) {
			return SUCCESS;
		}

		for (;;) {
			bg_runct = *read_p++;
			w_count += bg_runct;
			if (w_count >= SAGA_ISOTILE_WIDTH) {
				break;
			}

			draw_p += bg_runct;
			fg_runct = *read_p++;
			w_count += fg_runct;

			for (ct = 0; ct < fg_runct; ct++) {
				*draw_p++ = *read_p++;
			}
		}
	}

	return SUCCESS;
}

} // End of namespace Saga
