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
/*
 Description:   
 
    Isometric level module - private header

 Notes: 
*/

#ifndef SAGA_ISOMAP_H_
#define SAGA_ISOMAP_H_

namespace Saga {

typedef struct R_ISOTILE_ENTRY_tag {

	int tile_h;
	int unknown01;
	size_t tile_offset;
	int unknown04;
	int unknown06;

} R_ISOTILE_ENTRY;

#define SAGA_ISOTILE_ENTRY_LEN 8
#define SAGA_ISOTILE_WIDTH 32
#define SAGA_ISOTILE_BASEHEIGHT 15

#define SAGA_METATILE_W 8
#define SAGA_METATILE_H 8
#define SAGA_METATILE_SIZE 64

#define SAGA_METAMAP_W 16
#define SAGA_METAMAP_H 16
#define SAGA_METAMAP_SIZE 256

typedef struct R_ISO_METATILE_ENTRY_tag {

	int mtile_n;
	int unknown02;
	int unknown04;
	int unknown06;

	int tile_tbl[SAGA_METATILE_SIZE];

} R_ISO_METATILE_ENTRY;

#define SAGA_METATILE_ENTRY_LEN 136

typedef struct R_ISOMAP_MODULE_tag {

	int init;

	int tiles_loaded;
	const uchar *tileres_p;
	size_t tileres_len;
	uint tile_ct;
	R_ISOTILE_ENTRY *tile_tbl;

	int mtiles_loaded;
	const uchar *mtileres_p;
	size_t mtileres_len;
	uint mtile_ct;
	R_ISO_METATILE_ENTRY *mtile_tbl;

	int metamap_loaded;
	int metamap_n;
	uint metamap_tbl[SAGA_METAMAP_SIZE];
	const uchar *mm_res_p;
	size_t mm_res_len;

} R_ISOMAP_MODULE;

int ISOMAP_DrawTile(R_SURFACE * dst_s, uint tile_i, int tile_x, int tile_y);

int
ISOMAP_DrawMetaTile(R_SURFACE * dst_s, uint mtile_i, int mtile_x, int mtile_y);

int ISOMAP_DrawMetamap(R_SURFACE * dst_s, int map_x, int map_y);

} // End of namespace Saga

#endif				/* SAGA_ISOMAP_H_ */
