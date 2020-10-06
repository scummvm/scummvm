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

#ifndef ICB_PX_FEATURES_H_INCLUDED
#define ICB_PX_FEATURES_H_INCLUDED

#include "engines/icb/common/px_common.h"

namespace ICB {

// These define the filenames for files containing floor maps.
#define PX_FILENAME_FEATURES_INFO "pxwgfeatures"
#ifndef PX_EXT_LINKED
#define PX_EXT_LINKED "linked"
#endif

#ifndef PSX_EXT_LINKED
#define PSX_EXT_LINKED "PSXlinked"
#endif

// This is the version for these files.  The engine checks this runtime to know that it is running with
// the correct version of file.
#define VERSION_PXWGFEATURES 200

// enum _feature_type
// Not entirely sure what these will be yet.  But here are a couple of suggestions.
enum _feature_type { ANIMATING = 0, FEATURE, OTHER };

// struct _feature_info
// This holds information about one prop.
typedef struct {
	PXreal x, y, z;     // Reference point for the prop.
	PXreal floor_y;     // Y projected down to the floor the prop is on.
	_feature_type type; // The type of the prop.
	PXfloat direction;  // 0 - 99 (maybe use -1 to indicate no direction).
} _feature_info;

} // End of namespace ICB

#endif // #ifndef _PX_FEATURES_H_INCLUDED
