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

#ifndef ICB_PSX_PROPS_H
#define ICB_PSX_PROPS_H

#include "engines/icb/gfx/psx_pcdefines.h"

#define PSXPL_SCHEMA 4
#define PSXPL_ID "PLF"

#include "engines/icb/gfx/psx_zlayers.h"
#include "engines/icb/gfx/psx_bg.h"
#include "engines/icb/common/px_bitmap_psx.h"

namespace ICB {

// schema 0 file format is
/*
   uint32 schema;
   uint32 propQty;
   uint32 cmQty;
   uint32 cmOffsets[nCms]; // cmOffsets[nCms];
   propcmStruct props[propQty];
   {
      char* name;
      uint32 stateQty;
    }
   // This is the actual construction matrix and where the cmOffsets point to
   cmStruct cm[cmQty];
   {
      uint32 propQty; // currently always 1 but is likely to change
      uint32 propNameOffset;
      uint32 width; // how many bits in the CM table
      uint32 stateQty; // how many states
      uint32 statePtrs[stateQty]; // points to a psxPropLayers struct
      {
         int16  xs, xe;
         int16  ys, ye;
            uint16  zs, ze;
            uint16  w, h;
            uint32  n;
            uint32  zoverlayOffset; // relative bytes from state start to zoverlay
            uint32  zOffset; // relative bytes from state start to z data;
            uint16  palette[256];
            uint8 rgbData[w*h]
            uint8   zdata[w*h];
            psxZOverlay layers[n]
            {
            short xs, xe;
                short ys, ye;
                u_short zs, ze;
                u_short dZScale, dZShift;
            }
        }
   }
*/

// schema 1 file format is
/*
   char id[4];
   uint32 schema;
   uint32 propQty;
   uint32 cmQty;
   uint32 cmOffsets[nCms]; // cmOffsets[nCms];
   propcmStruct props[propQty];
   {
      char* name;
      uint32 stateQty;
    }
   // This is the actual construction matrix and where the cmOffsets point to
   cmStruct cm[cmQty];
   {
      uint32 propQty; // currently always 1 but is likely to change
      uint32 propNameOffset;
      uint32 width; // how many bits in the CM table
      uint32 stateQty; // how many states
      uint32 statePtrs[stateQty]; // points to a psxPropLayers struct
      {
         int16  xs, xe;
         int16  ys, ye;
            uint16  zs, ze;
            uint16  w, h;
            uint32  n;
            uint32  zoverlayOffset; // relative bytes from state start to zoverlay
            uint32  zOffset; // relative bytes from state start to z data;
            _pxPSXBitmap bitmap; // compress PSX Bitmap format inc. palette
            uint8   zdata[w*h];
            psxZOverlay layers[n]
            {
            short xs, xe;
                short ys, ye;
                u_short zs, ze;
                u_short dZScale, dZShift;
            }
        }
   }
*/

// schema 2 file format is
/*
   char id[4];
   uint32 schema;
   uint32 propQty;
   uint32 propOffsets[propQty]; // propOffsets[propQty];
   psxPropInfo propInfo[propQty];
   {
      char* name;
      uint32 stateQty;
   }
   psxProp props[propQty];
   {
      uint32 stateQty; // how many states
      uint32 propNameOffset;
      uint32 statePtrs[stateQty]; // points to a psxPropState struct
      psxPropState propStates[stateQty];
      {
         int16  xs, xe;
         int16  ys, ye;
            uint16 zs, ze;
            uint16 w, h;
            uint16 nPropRGBsprites;
            uint16 nBgRGBsprites;
            _pxPSXBitmap propBitmap; // compressed PSX Bitmap inc. palette
        }
   }
*/

// schema 3 file format is
/*
   char id[4];
   uint32 schema;
   uint32 propQty;
   uint32 propOffsets[propQty]; // propOffsets[propQty];
   psxPropInfo propInfo[propQty];
   {
      char* name;
      uint32 stateQty;
   }
   psxProp props[propQty];
   {
      uint32 stateQty; // how many states
      uint32 propNameOffset;
      uint32 statePtrs[stateQty]; // points to a psxPropState struct
      psxPropState propStates[stateQty];
      {
         int16  xs, xe;
         int16  ys, ye;
            uint16 zs, ze;
            uint16 w, h;
            uint16 nPropRGBsprites;
            uint16 nBgRGBsprites;
         uint32 zMicroOffset;
         uint32 propBitmapOffset;
         psxZMacroFrag zMacro[nPropRGBsprites];
         psxZMicroFrag zMicro[nMicroFrags];
            _pxPSXBitmap propBitmap; // compressed PSX Bitmap inc. palette
        }
   }
*/

// schema 4 file format is
/*
   char id[4];
   uint32 schema;
   uint32 propQty;
   uint32 propOffsets[propQty]; // propOffsets[propQty];
   psxPropInfo propInfo[propQty];
   {
      char* name;
      uint32 stateQty;
   }
   psxProp props[propQty];
   {
      uint32 stateQty; // how many states
      uint32 propNameOffset;
      uint32 statePtrs[stateQty]; // points to a psxPropState struct
      psxPropState propStates[stateQty];
      {
         int16  xs, xe;
         int16  ys, ye;
            uint16 zs, ze;
            uint16 w, h;

         uint16 nPropZsprites;
         uint16 nPropFGZsprites;
         uint16 nPropBGZsprites;
         uint16 padding;

            uint16 nPropRGBsprites;
            uint16 nBgRGBsprites;
         uint32 zMicroOffset;
         uint32 propBitmapOffset;
         psxZMacroFrag zMacro[nPropRGBsprites];
         psxZMicroFrag zMicro[nMicroFrags];
            _pxPSXBitmap propBitmap; // compressed PSX Bitmap inc. palette
        }
   }
*/

// z information about the macroscopic fragments
// (which are the RGB tiles within the psxBitmap structure)
// plus hooks into the microscopic z-data information
//
// These are currently a maximum of 64x64 big
typedef struct psxZMacroFrag {
	uint32 realzs; // in real z-coordinates e.g. not culled z-data for prop drawing order
	uint16 zs, ze;
	uint16 microStart;
	uint16 nMicro;
} psxZMacroFrag;

// z information about the microscopic fragments
// (which are the RGB tiles split into smaller tiles (very simple binary sub-division) until the z-range
// within the tile is within a tolerance - and so the microscopic fragment can be drawn at a single z value
//
// These microscopic tiles use x,y relative to the macroscopic fragment to save on bit storage for x,y
// e.g. gets it down to two int's instead of 3 !
typedef struct psxZMicroFrag {
	uint16 zs, ze; // store zs & ze just because otherwise we have a short of padding !
	uint8 xoff, yoff;
	uint8 w, h;
} psxZMicroFrag;

// The most basic information about a prop which current is just its name
typedef struct psxPropInfo { char *name; } psxPropInfo;

// This structure is overlaid ontop of the prop file which is read
// in from disc/CD
typedef struct psxProp {
	uint32 propNameOffset; // pointer to where the name is stored
	uint32 stateQty;       // how many states
	uint32 statePtrs[1];   // statePtrs[stateQty]
} psxProp;

typedef struct psxPropState {
	int16 xs, xe;
	int16 ys, ye;
	uint16 zs, ze;
	uint16 w, h;

	uint16 nPropZsprites;
	uint16 nPropFGZsprites;
	uint16 nPropBGZsprites;
	uint16 padding;

	uint16 nPropRGBsprites;
	uint16 nBgRGBsprites;

	uint32 zMicroOffset;
	uint32 propBitmapOffset;
	psxZMacroFrag zMacro[1];

	inline psxZMacroFrag *GetZMacroFragPtr(void);
	inline psxZMicroFrag *GetZMicroFragPtr(void);
	inline _pxPSXBitmap *GetPropBitmapPtr(void);

	psxPropState() { ; }

private:
	// Make '=' and copy constructor private to stop accidental assignment.
	psxPropState(const psxPropState &) { ; }
	void operator=(const psxPropState &) { ; }

} psxPropState;

inline psxZMacroFrag *psxPropState::GetZMacroFragPtr(void) { return zMacro; }

inline psxZMicroFrag *psxPropState::GetZMicroFragPtr(void) {
	psxZMicroFrag *zMicro = (psxZMicroFrag *)((uint8 *)this + zMicroOffset);
	return zMicro;
}

inline _pxPSXBitmap *psxPropState::GetPropBitmapPtr(void) {
	_pxPSXBitmap *propBitmap = (_pxPSXBitmap *)((uint8 *)this + propBitmapOffset);
	return propBitmap;
}

// Note these cannot be well represented by a C struct
// as they are full of variable length arrays
typedef struct psxPLfile {
	char id[4];
	uint32 schema;
	uint32 propQty;
	uint32 propOffsets[1];

	inline psxProp *GetProp(uint32 prop);
	inline uint32 GetPropStateQty(uint32 prop);
	inline char *GetPropName(uint32 prop);
	inline psxPropState *GetPropState(uint32 prop, uint32 state);
} psxPLfile;

inline psxProp *psxPLfile::GetProp(uint32 prop) { return (psxProp *)((uint8 *)id + propOffsets[prop]); }

inline uint32 psxPLfile::GetPropStateQty(uint32 prop) {
	psxProp *pProp = GetProp(prop);
	return pProp->stateQty;
}

inline char *psxPLfile::GetPropName(uint32 prop) {
	psxProp *pProp = GetProp(prop);
	return (id + pProp->propNameOffset);
}

inline psxPropState *psxPLfile::GetPropState(uint32 prop, uint32 state) {
	psxProp *pProp = GetProp(prop);
	return (psxPropState *)((uint8 *)id + pProp->statePtrs[state]);
}

} // End of namespace ICB

#endif // #ifndef PSX_PROPS_H
