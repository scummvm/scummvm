/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BURIED_AIDATA_H
#define BURIED_AIDATA_H

#include "buried/navdata.h"

namespace Buried {

enum {
	// Types
	AI_COMMENT_TYPE_INFORMATION           = (1 << 0),
	AI_COMMENT_TYPE_HELP                  = (1 << 1),
	AI_COMMENT_TYPE_SPONTANEOUS           = (1 << 2),
	AI_COMMENT_TYPE_OTHER                 = (1 << 3),

	// Flags
	AI_COMMENT_FLAG_SPECIAL_LOGIC         = (1 << 4),
	AI_STATUS_FLAG_NON_BASE_DERIVED       = (1 << 5),
	AI_DEPENDENCY_FLAG_NON_BASE_DERIVED_A = (1 << 6),
	AI_DEPENDENCY_CHECK_FOR_MINIMUM_A     = (1 << 7),
	AI_DEPENDENCY_FLAG_NON_BASE_DERIVED_B = (1 << 8),
	AI_DEPENDENCY_CHECK_FOR_MINIMUM_B     = (1 << 9),
	AI_COMMENT_DISABLE_IN_WALKTHROUGH     = (1 << 10)
};

// commentID - Environment relative comment ID number
// commentFlags - Flags definit this comment. These include type and special logic.
// dependencyFlagOffset - Offset to dependency flag
// dependencyValue - Maximum value for dependency condition to be true
// statusFlagOffset - Offset to status flag to be incremented when this comment is played
struct AIComment {
	Location location;
	uint16 commentID;
	uint16 commentFlags;
	uint16 dependencyFlagOffsetA;
	uint16 dependencyValueA;
	uint16 dependencyFlagOffsetB;
	uint16 dependencyValueB;
	uint16 statusFlagOffset;
};

} // End of namespace Buried

#endif
