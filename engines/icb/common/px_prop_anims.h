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

#ifndef ICB_PX_PROP_ANIMS_H_INCLUDED
#define ICB_PX_PROP_ANIMS_H_INCLUDED

// Include headers needed by this file.
#include "engines/icb/common/px_common.h"

namespace ICB {

// These define the filenames for files containing prop animations.
// The PSX definition is in engine\p4_psx.h
#define PX_FILENAME_PROPANIMS "pxwgpropanims"

#ifndef PC_EXT_LINKED
#define PC_EXT_LINKED "linked"
#endif

#ifndef PSX_EXT_LINKED
#define PSX_EXT_LINKED "PSXlinked"
#endif

#ifdef PX_EXT_LINKED
#undef PX_EXT_LINKED
#endif

#define PX_EXT_LINKED PC_EXT_LINKED

#define VERSION_PXWGPROPANIMS 300

// This the animations for one prop.
typedef struct {
	uint16 num_anims; // The number of animations this prop has.
	uint16 anims[1];  // Array of file offsets to the entries for the animations.
} _animating_prop;

// This holds one animation and any associated barriers. Note that the offsets are relative to the
// start of this _animation_entry.
typedef struct {
	uint16 name;                  // File offset of the name of the animation.
	uint16 offset_barriers;       // Offset to an array of barrier indices (NULL if none).
	uint16 offset_heights;        // Offset to an array of PxReal height values (0 means there isn't one).
	uint8 num_frames;             // Number of frames in the animation.
	uint8 num_barriers_per_frame; // The number of barriers per frame (same for each frame and very often 1).
	uint8 frames[1];              // The frames for the animation.
} _animation_entry;

} // End of namespace ICB

#endif // #ifndef _PX_PROP_ANIMS_H_INCLUDED
