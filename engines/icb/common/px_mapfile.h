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

// NOTES

// 1.   A session has a number of maps.  These maps may, though don't have to, correspond to the
//		slices in the walkgrid data.  Each map is one item in a px_linkeddatafile.  The data structures
//		below describe the internal structure of this.

// 2.   Each entry in the px_linkeddatafile is simply a uint32 count of the number of objects in
//		this level, followed by the objects themselves, which are all one of the types listed below.

// 3.   All x,y,z coordinates are divided by two before storage, so we can fit them in a byte.  Thus
//		the map is stored half-scale.

// Make sure this header gets included only once.
#ifndef ICB_PX_MAPFILE_H_INCLUDED
#define ICB_PX_MAPFILE_H_INCLUDED

// Include headers needed by this file.
#include "engines/icb/common/px_common.h"

namespace ICB {

// This defines a schema version number for the data.  If the format is changed then this number should
// be changed to prevent the engine running with out-of-date data.
#define REMORA_MAP_FILE_VERSION 100

// These define extensions and names for these files.
#if !defined(REMORA_MAP_FILENAME)
#define REMORA_MAP_FILENAME "remora_map"
#endif

#if !defined(REMORA_MAP_PC_EXT)
#define REMORA_MAP_PC_EXT "rmf_pc"
#endif

#if !defined(REMORA_MAP_PSX_EXT)
#define REMORA_MAP_PSX_EXT "rmf_psx"
#endif

#if !defined(PX_MAP_EXT)
#if defined(_PSX)
#define PX_MAP_EXT REMORA_MAP_PSX_EXT
#else
#define PX_MAP_EXT REMORA_MAP_PC_EXT
#endif
#endif

// This defines the 'knowledge levels' that items can have.  Basically, the idea is that an object
// will only get drawn in the game if the knowledge level for the player is greater than or equal
// to the level stored for the object.
#define REMORA_MAP_MAX_KNOWLEDGE_LEVEL 3

// These are definitions for the object types.  These are packed in with the 'level' indicator.  So
// if you had a line that should be displayed only when the player's knowledge is at level 4 (i.e. the
// player is currently in a location), then it would have the following code:
// 0x40 | 0x02 = 0x42.
enum MapObjectType { REMORA_MAP_RECTANGLE = 0, REMORA_MAP_FILLED_RECTANGLE, REMORA_MAP_LINE, REMORA_MAP_TEXT, REMORA_MAP_POLYGON, REMORA_MAP_NUM_OBJECTS };

// These macros allow access to the two halves of the packed field mentioned previously.
#define GET_KNOWLEDGE_LEVEL(x) ((uint32)((x & 0xf0) >> 4))
#define GET_OBJECT_TYPE(x) ((MapObjectType)(x & 0x0f))

// type _map_colour_point
// This holds a point with a colour.  This is not used in the objects that have a fixed number
// of points because we lose 3 padding bytes per point.
typedef struct {
	uint8 nX, nY;
	uint8 nR, nG, nB;
	uint8 nPad1;
	uint8 nPad2;
	uint8 nPad3;
} _map_colour_point;

// type _map_rectangle
// An outline rectangle - object type 0.
typedef struct {
	uint32 nLocationNameHash; // Hash of the location this object is part of.
	uint8 nLevelAndType;      // See note above for how to construct this field.
	uint8 nX1, nY1;           // Top-left corner of the rectangle.
	uint8 nR1, nG1, nB1;      // Colour for top-left corner.
	uint8 nX2, nY2;           // Bottom-right corner of the rectangle.
	uint8 nR2, nG2, nB2;      // Colour for bottom-right corner.
	uint8 nWidth;             // Width to draw rectangle line.
} _map_rectangle;

// type _map_filled_rectangle
// A filled rectangle - object type 1.
typedef struct {
	uint32 nLocationNameHash; // Hash of the location this object is part of.
	uint8 nLevelAndType;      // See note above for how to construct this field.
	uint8 nX1, nY1;           // Top-left corner of the rectangle.
	uint8 nR1, nG1, nB1;      // Colour for top-left corner.
	uint8 nX2, nY2;           // Bottom-right corner of the rectangle.
	uint8 nR2, nG2, nB2;      // Colour for bottom-right corner.
	uint8 nPad1;
} _map_filled_rectangle;

// type _map_line
// A line - object type 2.
typedef struct {
	uint32 nLocationNameHash; // Hash of the location this object is part of.
	uint8 nLevelAndType;      // See note above for how to construct this field.
	uint8 nX1, nY1;           // Top-left corner of the rectangle.
	uint8 nR1, nG1, nB1;      // Colour for top-left corner.
	uint8 nX2, nY2;           // Bottom-right corner of the rectangle.
	uint8 nR2, nG2, nB2;      // Colour for bottom-right corner.
	uint8 nWidth;             // Width to draw rectangle line.
} _map_line;

// type _map_text
// Some text to be displayed - object type 3.
typedef struct {
	uint32 nLocationNameHash; // Hash of the location this object is part of.
	uint8 nLevelAndType;      // See note above for how to construct this field.
	uint8 nX, nY;             // Bottom-left point to start drawing text.
	uint8 nR, nG, nB;         // Colour.
	uint8 nPad1;
	uint8 nPad2;
	uint32 nTextHash; // Hash of the text.
} _map_text;

// type _map_polygon
// A filled rectangle - object type 4.
typedef struct {
	uint32 nLocationNameHash; // Hash of the location this object is part of.
	uint8 nLevelAndType;      // See note above for how to construct this field.
	uint8 nNumPoints;         // How many points it has.
	uint8 m_nPad1;
	uint8 m_nPad2;
	_map_colour_point pPoints[1]; // The points for the polygon.
} _map_polygon;

// This function works out the size of a polygon structure in the file because there are variable.
inline uint32 _map_polygon_sizeof(const _map_polygon *pPoly) { return (sizeof(_map_polygon) + (pPoly->nNumPoints - 1) * sizeof(_map_colour_point)); }

} // End of namespace ICB

#endif // #if !defined(_PX_MAPFILE_H_INCLUDED)
