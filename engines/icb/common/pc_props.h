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

#ifndef ICB_PC_PROPS_H
#define ICB_PC_PROPS_H

#include "engines/icb/common/px_staticlayers.h" // for types + defines
#include "engines/icb/common/px_types.h"

namespace ICB {

#define PCPROP_SCHEMA 3
#define PCPROP_ID MKTAG('p', 'o', 'r', 'P')
#define PCSETFILE_ID MKTAG('t', 'n', 'i', 'm')

typedef struct _pcSetHeader {
	uint32 id;
	uint32 cameraOffset;
	uint32 lightOffset;
	uint32 propOffset;
	uint32 layerOffset;
	uint32 backgroundOffset;
} _pcSetHeader;

class pcPropRGBState {
private:
	uint16 *zPtrs[TILE_COUNT];
	uint16 *semiPtrs[TILE_COUNT];
	uint16 nLRBgTiles;
	uint16 nLRFgTiles;
	uint16 nHRBgTiles;
	uint16 nHRFgTiles;
	uint32 *palettePtr;
	uint16 bgLRSurfaceWidth;
	uint16 bgLRSurfaceHeight;
	uint16 fgLRSurfaceWidth;
	uint16 fgLRSurfaceHeight;
	uint16 bgHRSurfaceWidth;
	uint16 bgHRSurfaceHeight;
	uint16 fgHRSurfaceWidth;
	uint16 fgHRSurfaceHeight;
	uint8 *bgLRRleDataPtr;
	uint8 *fgLRRleDataPtr;
	uint8 *bgHRRleDataPtr;
	uint8 *fgHRRleDataPtr;
	LRECT *tileRects;
public:
	pcPropRGBState(uint8 *propBasePtr, uint32 dataOffset) {
		uint8 *ptr = propBasePtr + dataOffset;

		for (int i = 0; i < TILE_COUNT; i++) {
			zPtrs[i] = 0;
			if (uint32 offset = READ_LE_U32(ptr)) {
				zPtrs[i] = (uint16 *)(propBasePtr + offset);
			}
			ptr += 4;
		}

		for (int i = 0; i < TILE_COUNT; i++) {
			semiPtrs[i] = 0;
			if (uint32 offset = READ_LE_U32(ptr)) {
				semiPtrs[i] = (uint16 *)(propBasePtr + offset);
			}
			ptr += 4;
		}

		nLRBgTiles = READ_LE_U16(ptr);
		ptr += 2;

		nLRFgTiles = READ_LE_U16(ptr);
		ptr += 2;

		nHRBgTiles = READ_LE_U16(ptr);
		ptr += 2;

		nHRFgTiles = READ_LE_U16(ptr);
		ptr += 2;

		palettePtr = (uint32 *)(propBasePtr + READ_LE_U32(ptr));
		ptr += 4;

		bgLRSurfaceWidth = READ_LE_U16(ptr);
		ptr += 2;

		bgLRSurfaceHeight = READ_LE_U16(ptr);
		ptr += 2;

		fgLRSurfaceWidth = READ_LE_U16(ptr);
		ptr += 2;

		fgLRSurfaceHeight = READ_LE_U16(ptr);
		ptr += 2;

		bgHRSurfaceWidth = READ_LE_U16(ptr);
		ptr += 2;

		bgHRSurfaceHeight = READ_LE_U16(ptr);
		ptr += 2;

		fgHRSurfaceWidth = READ_LE_U16(ptr);
		ptr += 2;

		fgHRSurfaceHeight = READ_LE_U16(ptr);
		ptr += 2;

		bgLRRleDataPtr = propBasePtr + READ_LE_U32(ptr);
		ptr += 4;

		fgLRRleDataPtr = propBasePtr + READ_LE_U32(ptr);
		ptr += 4;

		bgHRRleDataPtr = propBasePtr + READ_LE_U32(ptr);
		ptr += 4;

		fgHRRleDataPtr = propBasePtr + READ_LE_U32(ptr);
		ptr += 4;

		tileRects = (LRECT *)ptr;
	}

	uint16 *GetZTileTable(int t) { return zPtrs[t]; }
	uint16 *GetSemiTileTable(int t) { return semiPtrs[t]; }
	uint16 GetLRBgTileQty() { return nLRBgTiles; }
	uint16 GetLRFgTileQty() { return nLRFgTiles; }
	uint16 GetHRBgTileQty() { return nHRBgTiles; }
	uint16 GetHRFgTileQty() { return nHRFgTiles; }
	uint32 *GetPalette() { return palettePtr; }
	uint16 GetLRBgSurfaceWidth() { return bgLRSurfaceWidth; }
	uint16 GetLRBgSurfaceHeight() { return bgLRSurfaceHeight; }
	uint16 GetLRFgSurfaceWidth() { return fgLRSurfaceWidth; }
	uint16 GetLRFgSurfaceHeight() { return fgLRSurfaceHeight; }
	uint16 GetHRBgSurfaceWidth() { return bgHRSurfaceWidth; }
	uint16 GetHRBgSurfaceHeight() { return bgHRSurfaceHeight; }
	uint16 GetHRFgSurfaceWidth() { return fgHRSurfaceWidth; }
	uint16 GetHRFgSurfaceHeight() { return fgHRSurfaceHeight; }
	uint8 *GetLRBgRlePtr() { return bgLRRleDataPtr; }
	uint8 *GetLRFgRlePtr() { return fgLRRleDataPtr; }
	uint8 *GetHRBgRlePtr() { return bgHRRleDataPtr; }
	uint8 *GetHRFgRlePtr() { return fgHRRleDataPtr; }
	LRECT *GetTileRects() { return tileRects; }
};

class pcPropRGB {
      private:
	char name[32];
	uint32 stateQty;
	pcPropRGBState **states;

public:
	pcPropRGB(uint8 *propBasePtr, uint32 dataOffset) {
		uint8 *ptr = propBasePtr + dataOffset;

		memcpy(name, ptr, 32);
		ptr += 32;

		stateQty = READ_LE_U32(ptr);
		ptr += 4;

		states = new pcPropRGBState *[stateQty];
		for (uint32 i = 0; i < stateQty; i++) {
			states[i] = new pcPropRGBState(propBasePtr, READ_LE_U32(ptr));
			ptr += 4;
		}
	}

	~pcPropRGB() {
		for (uint32 i = 0; i < stateQty; i++) {
			delete states[i];
		}
		delete[] states;
		states = 0;
	}

	const char *GetName() const { return name; }
	uint32 GetStateQty() const { return stateQty; }
	pcPropRGBState *GetState(uint32 s) { return states[s]; }
};

class pcPropFile {
private:
	uint32 id;
	uint32 schema;
	uint32 mapping;
	uint32 propQty;
	pcPropRGB **props;

public:
	pcPropFile(uint8 *propData) {
		uint8 *ptr = propData;

		id = READ_LE_U32(ptr);
		ptr += 4;

		schema = READ_LE_U32(ptr);
		ptr += 4;

		mapping = READ_LE_U32(ptr);
		ptr += 4;

		propQty = READ_LE_U32(ptr);
		ptr += 4;

		props = new pcPropRGB *[propQty];
		for (uint32 i = 0; i < propQty; i++) {
			props[i] = new pcPropRGB(propData, READ_LE_U32(ptr));
			ptr += 4;
		}
	}

	~pcPropFile() {
		for (uint32 i = 0; i < propQty; i++) {
			delete props[i];
		}
		delete[] props;
		props = 0;
	}

	uint32 GetId() const { return id; }
	uint32 GetPropQty() const { return propQty; }
	pcPropRGB *GetProp(uint32 p) { return props[p]; }
	uint32 GetSchema() const {
		if (id != PCPROP_ID)
			return 0;
		else
			return schema;
	}
};

} // End of namespace ICB

#endif // #ifndef PC_PROPS_H
