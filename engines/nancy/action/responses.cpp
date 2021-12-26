/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// For whatever reason the games don't use the conditional dialogue code
// inside primary video, and instead have a bunch of functions that manually
// check hardcoded event flag IDs and extract strings embedded inside the exe.
// These tables contain the extracted data and offsets for the string data, but if
// there happen to be many versions of the same game it might be a better idea to just
// directly copy the dialogue strings in here.

struct ConditionalResponseDesc {
	byte characterID; // 0: Daryl, 1: Connie, 2: Hal, 3: Hulk
	uint fileOffset;
	uint16 sceneID;
	EventFlagDescription conditions[7];
};

struct GoodbyeDesc {
	byte characterID;
	uint fileOffset;
	uint16 sceneIDs[4];
};

struct HintDesc {
	byte characterID; // 0: Ned, 1: Bess, 2: George
	byte hintID;
	EventFlagDescription flagConditions[4];
	EventFlagDescription inventoryCondition[2];
};

static const uint nancy1ResponseBaseFileOffset = 0xB1FE0; // TODO there could be more than one version of the exe
static const uint nancy1HintOffsets[] = { 0xABB88, 0xAD760, 0xAF338 }; // Ned, Bess, George

#define EMPTY_DESC { -1, kFalse }

static const GoodbyeDesc nancy1Goodbyes[] = {
	// Daryl
	{
		0,
		0x11B0,
		{ 0xC94, 0xC95, 0xC96, 0xC97}
	},

	// Connie
	{
		1,
		0x11D8,
		{ 0xFC, 0x9D8, 0x9D9, 0x9DB }
	},

	// Hal
	{
		2,
		0x11FC,
		{ 0x1C3, 0x1C4, 0x1C5, 0x1C6}
	},

	// Hulk
	{
		3,
		0x1228,
		{ 0xCE2, 0xCE0, 0xCE2, 0xCE0 } // only two responses
	}
};

static const ConditionalResponseDesc nancy1ConditionalResponses[] = {
	// Daryl
	{
		0,
		0x840,
		0x7C,
		{
			{ 0x1D, kTrue },
			{ 0x39, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x804,
		0x7F,
		{
			{ 0x13, kTrue },
			{ 0x37, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x7BC,
		0x81,
		{
			{ 0xB, kTrue },
			{ 0x38, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x750,
		0x83,
		{
			{ 0, kTrue },
			{ 1, kFalse },
			{ 0x6B, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x6F4,
		0x84,
		{
			{ 0x64, kTrue },
			{ 0x1E, kFalse },
			{ 0x14, kFalse },
			{ 0xC, kFalse },
			{ 0x6C, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x5EC,
		0x86,
		{
			{ 0x6D, kFalse },
			{ 0x6, kTrue },
			{ 0x8, kTrue },
			{ 0x5E, kTrue },
			{ 0x17, kTrue },
			{ 0x24, kTrue },
			{ 0x9, kTrue }
		}
	},

	{
		0,
		0x554,
		0x8B,
		{
			{ 0x6E, kFalse },
			{ 0x24, kTrue },
			{ 0x9, kTrue },
			{ 0x5E, kFalse },
			{ 0x8, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x4F0,
		0x8D,
		{
			{ 0x6F, kFalse },
			{ 0x5E, kTrue },
			{ 0x24, kTrue },
			{ 0x9, kTrue },
			{ 0x8, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x458,
		0x8F,
		{
			{ 0x70, kFalse },
			{ 0x24, kTrue },
			{ 0x9, kTrue },
			{ 0x6, kTrue },
			{ 0x8, kTrue },
			{ 0x5E, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x3BC,
		0x90,
		{
			{ 0x71, kFalse },
			{ 0x5E, kTrue },
			{ 0x24, kFalse },
			{ 0x8, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x320,
		0x91,
		{
			{ 0x72, kFalse },
			{ 0x5E, kTrue },
			{ 0x8, kTrue },
			{ 0x6, kTrue },
			{ 0x24, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x2AC,
		0x92,
		{
			{ 0x73, kFalse },
			{ 0x8, kTrue },
			{ 0x6, kTrue },
			{ 0x5E, kFalse },
			{ 0x24, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x1F0,
		0x96,
		{
			{ 0x74, kFalse },
			{ 0x1D, kTrue },
			{ 0x13, kTrue },
			{ 0xB, kTrue },
			{ 0x5E, kFalse },
			{ 0x24, kFalse },
			{ 0x8, kFalse }
		}
	},

	{
		0,
		0x190,
		0x97,
		{
			{ 0x27, kFalse },
			{ 0x5, kTrue },
			EMPTY_DESC
		}
	},

	{
		0,
		0xF0,
		0x9C,
		{
			{ 0x28, kTrue },
			{ 0x75, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x94,
		0x93,
		{
			{ 0xC, kFalse },
			{ 0x6, kTrue },
			{ 0x76, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0x58,
		0x94,
		{
			{ 0x14, kFalse },
			{ 0x4, kTrue },
			{ 0x77, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		0,
		0x95,
		{
			{ 0x1E, kFalse },
			{ 0x63, kTrue },
			{ 0x78, kFalse },
			EMPTY_DESC
		}
	},

	// Connie
	{
		1,
		0xBE4,
		0xE9,
		{
			{ 0x1D, kTrue },
			{ 0x18, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0xB8C,
		0xEA,
		{
			{ 0x1F, kTrue },
			{ 0x19, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0xB54,
		0xEB,
		{
			{ 0xB, kTrue },
			{ 0x1A, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0xB14,
		0xEC,
		{
			{ 0x26, kTrue },
			{ 0x1C, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0xABC,
		0xED,
		{
			{ 0, kTrue },
			{ 1, kFalse },
			{ 0x79, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0xA00,
		0xEE,
		{
			{ 2, kTrue },
			{ 3, kTrue },
			{ 0x17, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0x6F4,
		0xEF,
		{
			{ 0x64, kTrue },
			{ 0x16, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0x968,
		0xF0,
		{
			{ 0x5, kTrue },
			{ 0x14, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		0x8C8,
		0xF5,
		{
			{ 0x28, kTrue },
			EMPTY_DESC
		}
	},

	{
		1,
		0x884,
		0xE7,
		{
			{ 0xD, kTrue },
			{ 0x5E, kFalse },
			EMPTY_DESC
		}
	},

	// Hal
	{
		2,
		0xED0,
		0x1B3,
		{
			{ 0x1D, kTrue },
			{ 0x11, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0x804,
		0x1B5,
		{
			{ 0x13, kTrue },
			{ 0xE, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0xE74,
		0x1B6,
		{
			{ 0x1B, kTrue },
			{ 0xF, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0xE2C,
		0x1B7,
		{
			{ 0x26, kTrue },
			{ 0x10, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0xDD4,
		0x1B9,
		{
			{ 0, kTrue },
			{ 1, kFalse },
			{ 0x68, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0xD48,
		0x1BA,
		{
			{ 0, kTrue },
			{ 1, kFalse },
			{ 0x20, kTrue },
			{ 0x69, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0x6F4,
		0x1BB,
		{
			{ 0x6A, kFalse },
			{ 0x64, kTrue },
			{ 0x5, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0xCC8,
		0x1BC,
		{
			{ 0x8, kTrue },
			{ 0x6, kTrue },
			{ 0xC, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		0xC2C,
		0x1BE,
		{
			{ 0x28, kTrue },
			EMPTY_DESC
		}
	},

	// Hulk
	{
		3,
		0x1164,
		0x14D,
		{
			{ 0x13, kTrue },
			{ 0x3A, kFalse },
			EMPTY_DESC
		}
	},

	{
		3,
		0xB54,
		0x150,
		{
			{ 0xB, kTrue },
			{ 0x25, kFalse },
			EMPTY_DESC
		}
	},

	{
		3,
		0x10D8,
		0x153,
		{
			{ 0x12, kTrue },
			{ 0x21, kFalse },
			EMPTY_DESC
		}
	},

	{

		3,
		0xE2C,
		0x154,
		{
			{ 0x26, kTrue },
			{ 0x22, kFalse },
			EMPTY_DESC
		}
	},

	{
		3,
		0x108C,
		0x155,
		{
			{ 0, kTrue },
			{ 1, kFalse },
			{ 0x66, kFalse },
			EMPTY_DESC
		}
	},

	{
		3,
		0x6F4,
		0x156,
		{
			{ 0x67, kFalse },
			{ 0x64, kTrue },
			EMPTY_DESC
		}
	},

	{
		3,
		0x1028,
		0x157,
		{
			{ 0x63, kTrue },
			{ 0x24, kFalse },
			EMPTY_DESC
		}
	},

	{
		3,
		0xFB0,
		0x158,
		{
			{ 0x5, kTrue },
			{ 0x1E, kFalse },
			EMPTY_DESC
		}
	},

	{
		3,
		0xF10,
		0x159,
		{
			{ 0x28, kTrue },
			EMPTY_DESC
		}
	}
};

static const HintDesc nancy1Hints[] {
	// Ned
	{
		0,
		1,
		{
			{ 0, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		0,
		2,
		{
			{ 0, kTrue },
			{ 1, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		0,
		3,
		{
			{ 1, kFalse },
			EMPTY_DESC
		},
		{
			{ 3, kFalse },
			EMPTY_DESC
		}
	},

	{
		0,
		4,
		{
			{ 0x55, kFalse },
			EMPTY_DESC
		},
		{
			{ 3, kTrue },
			EMPTY_DESC
		}
	},

	{
		0,
		5,
		{
			{ 0x55, kTrue },
			{ 0x56, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		0,
		6,
		{
			{ 0x57, kFalse },
			{ 0x56, kTrue },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		0,
		8,
		{
			{ 0xA, kTrue },
			{ 0x3B, kTrue },
			EMPTY_DESC
		},
		{
			{ 7, kFalse },
			EMPTY_DESC
		}
	},

	// Bess
	{
		1,
		1,
		{
			{ 0x57, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		1,
		2,
		{
			{ 0x57, kTrue },
			{ 0x3C, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		1,
		3,
		{
			{ 0x5A, kFalse },
			{ 0x3C, kTrue },
			{ 0x56, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		1,
		4,
		{
			{ 0x5A, kTrue },
			{ 0x56, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		1,
		6,
		{
			{ 0x5A, kFalse },
			{ 0x3C, kTrue },
			{ 0x56, kTrue },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		1,
		7,
		{
			{ 0x59, kTrue },
			{ 0xA, kFalse },
			EMPTY_DESC
		},
		{
			{ 0, kFalse },
			EMPTY_DESC
		}
	},

	{
		1,
		8,
		{
			{ 0xA, kTrue },
			{ 0x3B, kTrue },
			EMPTY_DESC
		},
		{
			{ 0, kTrue },
			{ 7, kFalse }
		}
	},

	{
		1,
		9,
		{
			{ 0x59, kFalse },
			{ 0xA, kTrue },
			{ 0x3B, kTrue },
			EMPTY_DESC
		},
		{
			{ 7, kFalse },
			EMPTY_DESC
		}
	},

	// George
	{
		2,
		0xA,
		{
			{ 0x4A, kTrue },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		2,
		1,
		{
			{ 0x5B, kFalse },
			EMPTY_DESC
		},
		{
			EMPTY_DESC
		}
	},

	{
		2,
		2,
		{
			{ 0x5B, kTrue },
			EMPTY_DESC
		},
		{
			{ 9, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		3,
		{
			{ 0x5B, kTrue },
			{ 0x5C, kFalse },
			{ 0x5D, kFalse },
			EMPTY_DESC
		},
		{
			{ 9, kTrue },
			EMPTY_DESC
		}
	},

	{
		2,
		4,
		{
			{ 0x5B, kTrue },
			{ 0x5C, kTrue },
			{ 0x5D, kFalse },
			EMPTY_DESC
		},
		{
			{ 9, kFalse },
			EMPTY_DESC
		}
	},

	{
		2,
		5,
		{
			{ 0x5B, kTrue },
			{ 0x5C, kTrue },
			{ 0x5D, kTrue },
			{ 0x3B, kFalse }
		},
		{
			{ 9, kTrue },
			EMPTY_DESC
		}
	},

	{
		2,
		6,
		{
			{ 0xA, kFalse },
			{ 0x3B, kTrue },
			EMPTY_DESC
		},
		{
			{ 9, kTrue },
			EMPTY_DESC
		}
	},

	{
		2,
		7,
		{
			{ 0x3B, kTrue },
			{ 0xA, kTrue },
			EMPTY_DESC
		},
		{
			{ 7, kFalse },
			EMPTY_DESC
		}
	}
};

} // End of namespace Action
} // End of namespace Nancy
