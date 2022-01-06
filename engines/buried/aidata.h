/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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


struct AICommentInfo {
	byte timeZone;
	byte environment;
	const char *filePath;
};

// timeZone, environment, filePath
static const AICommentInfo s_aiCommentInfo[] {
	// Castle
	{ 1,  1, "CASTLE/CGTT"  }, { 1,  2, "CASTLE/CGTS"  }, { 1,  3, "CASTLE/CGMW"  },
	{ 1,  4, "CASTLE/CGMB"  }, { 1,  5, "CASTLE/CGBS"  }, { 1,  6, "CASTLE/CGKC"  },
	{ 1,  7, "CASTLE/CGST"  }, { 1,  8, "CASTLE/CGKS"  }, { 1,  9, "CASTLE/CGSR"  },
	{ 1, 10, "CASTLE/CGTR"  },
	// Mayan
	{ 2,  1, "MAYAN/MYTP"   }, { 2,  2, "MAYAN/MYMC"   }, { 2,  3, "MAYAN/MYWG"   },
	{ 2,  4, "MAYAN/MYWT"   }, { 2,  5, "MAYAN/MYAG"   }, { 2,  6, "MAYAN/MYDG"   },
	// Future Apartment
	{ 4,  1, "FUTAPT/FAKI"  }, { 4,  2, "FUTAPT/FAER"  }, { 4,  3, "FUTAPT/FAMN"  },
	// Da Vinci
	{ 5,  1, "DAVINCI/DSPT" }, { 5,  2, "DAVINCI/DSCT" }, { 5,  3, "DAVINCI/DSGD" },
	{ 5,  4, "DAVINCI/DSWS" }, { 5,  5, "DAVINCI/DSCY" },
	// Space Station
	{ 6,  1, "AILAB/AIHW"   }, { 6,  2, "AILAB/AICR"   }, { 6,  3, "AILAB/AIDB"   },
	{ 6,  4, "AILAB/AISC"   }, { 6,  5, "AILAB/AINX"   }, { 6,  6, "AILAB/AIIC"   },
	{ 6,  7, "AILAB/AISW"   }, { 6,  8, "AILAB/AIMR"   }, { 6, 10, "AILAB/AIHW"   },	// there is no 9
	{ 0,  0, "" }
};

} // End of namespace Buried

#endif
