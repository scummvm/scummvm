/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_STATIC_LAYERS_H
#define ICB_PX_STATIC_LAYERS_H

#include "engines/icb/common/px_common.h"

namespace ICB {

#define PCLAYER_SCHEMA 1
#define PCLAYER_ID MKTAG('r', 'y', 'a', 'L')

#define TILE_WIDTH 64
#define TILE_HEIGHT 48
#define TILES_WIDE (640 / TILE_WIDTH)
#define TILES_HIGH (480 / TILE_HEIGHT)
#define TILE_COUNT (TILES_WIDE * TILES_HIGH)

class pcStaticLayers {
      private:
	char id[4];
	uint32 schema;
	uint32 mapping;
	uint32 tilePtrs[TILE_COUNT]; // 10 x 10 array of tiles (null means an empty tile)
	uint32 semiPtrs[TILE_COUNT]; // 10 x 10 array of tiles (null means an empty tile)

	uint8 *DataStart() { return (uint8 *)id; }

      public:
	uint16 *GetSemiTileTable(int idx) { return semiPtrs[idx] ? (uint16 *)(DataStart() + semiPtrs[idx]) : 0; }

	uint16 *GetTileTable(int idx) { return tilePtrs[idx] ? (uint16 *)(DataStart() + tilePtrs[idx]) : 0; }

	uint32 GetSchema() {
		if (READ_LE_U32(id) != PCLAYER_ID)
			return 0;
		else
			return schema;
	}
	pcStaticLayers () {
		(void)mapping; // shutup warning
	}
};

} // End of namespace ICB

#endif // _PX_STATIC_LAYERS_H
