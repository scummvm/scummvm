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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "igor/igor.h"

namespace Igor {

const RoomDataOffsets IgorEngine::PART_00_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 79, 96 },
	{ 103, 347, 3503, 247, 323, 88 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_01_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 }, // no area-transition table
	{ 4, 15 },
	{ 19, 103, 3155, 209, 179, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 } // no room-specific dialogue matrix
};

const RoomDataOffsets IgorEngine::PART_02_ROOM_DATA_OFFSETS = {
	{ 125, 5, 20, 4 }, // five-area transition table
	{ 259, 280 },
	{ 289, 565, 4193, 473, 549, 96 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 } // no room-specific dialogue matrix
};

const RoomDataOffsets IgorEngine::PART_04_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 5, 18 },             // walk points and facing positions of the room objects
	{ 23, 231, 3247, 127, 203, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_05_ROOM_DATA_OFFSETS = {
	{ 125, 5, 20, 4 },
	{ 253, 262 },
	{ 265, 437, 3313, 329, 405, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_06_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 75, 84 },
	{ 87, 259, 3135, 151, 227, 80 },
	{
		135, 30, 5158, 100, 60,
		240,
		15458,
		15518,
		35
	}
};

const RoomDataOffsets IgorEngine::PART_07_ROOM_DATA_OFFSETS = {
	{ 80, 4, 12, 3 },
	{ 151, 168 },
	{ 175, 419, 3575, 319, 395, 88 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 } // no room-specific question/reply matrix
};

const RoomDataOffsets IgorEngine::PART_08_ROOM_DATA_OFFSETS = {
	{ 80, 4, 12, 3 },
	{ 149, 162 },
	{ 167, 375, 3391, 271, 347, 84 },
	{
		165, 30,
		5188, 100,
		90,
		270,
		15488,
		15548,
		100
	}
};

const RoomDataOffsets IgorEngine::PART_09_ROOM_DATA_OFFSETS = {
	{ 80, 4, 12, 3 },
	{ 149, 162 },
	{ 167, 375, 3391, 271, 347, 84 },
	{ 378, 29, 5237, 46, 120, 480, 10029, 10087, 46 }
};

const RoomDataOffsets IgorEngine::PART_10_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 77, 90 },
	{ 95, 303, 3319, 199, 275, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_11_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 35, 52 },
	{ 59, 309, 3255, 203, 279, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_12_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 75, 84 },
	{ 87, 259, 3135, 151, 227, 80 },
	{
		-85, 15, 2478, 25, 30,
		60,
		5230,
		5260,
		25
	}
};

const RoomDataOffsets IgorEngine::PART_13_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 31, 40 },
	{ 43, 215, 3091, 107, 183, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_14_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 32, 43 },
	{ 47, 237, 3183, 131, 207, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_15_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 3, 12 },
	{ 15, 187, 3063, 79, 155, 80 },
	{ 45, 22, 3756, 66, 60, 180, 10588, 10632, 66 }
};

// The Spanish floppy room has no dialogue data (Tobias isnt a character)
const RoomDataOffsets IgorEngine::PART_15_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 4, 15 },
	{ 19, 209, 3155, 103, 179, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_16_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 29, 34 },
	{ 35, 171, 2907, 59, 135, 76 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_17_ROOM_DATA_OFFSETS = {
	{ 320, 8, 56, 7 },
	{ 835, 844 },
	{ 847, 1019, 3895, 911, 987, 80 },
	{ -174, 5, 257, 1, 30, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_18_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 34, 49 },
	{ 55, 281, 3367, 179, 255, 86 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_19_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 33, 46 },
	{ 51, 259, 3359, 155, 231, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// This room doesnt exist in the English floppy onwards
const RoomDataOffsets IgorEngine::PART_20_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 4, 15 },
	{ 19, 209, 3155, 103, 179, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_21_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 5, 18 },
	{ 23, 231, 3247, 127, 203, 84 },
	{ -85, 10, 1658, 50, 30, 90, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_21_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 33, 46 },
	{ 51, 259, 3275, 155, 231, 84 },
	{ -85, 10, 1658, 50, 30, 90, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_22_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 1, 6 },
	{ 7, 143, 2879, 31, 107, 76 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_23_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 4, 15 },
	{ 19, 209, 3155, 103, 179, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_24_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 5, 18 },
	{ 23, 235, 3111, 127, 203, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_28_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 33, 46 },
	{ 51, 259, 3275, 155, 231, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_23_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 31, 40 },
	{ 43, 215, 3091, 107, 183, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_24_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 31, 40 },
	{ 43, 215, 3091, 107, 183, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_25_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 31, 40 },
	{ 43, 215, 3091, 107, 183, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_25_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 31, 40 },
	{ 43, 215, 3091, 107, 183, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_26_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 3, 12 },
	{ 15, 187, 3063, 79, 155, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_26_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 32, 43 },
	{ 47, 237, 3183, 131, 207, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_27_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 3, 12 },
	{ 15, 187, 3063, 79, 155, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_27_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 6, 21 },
	{ 27, 253, 3339, 151, 227, 86 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_28_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 78, 93 },
	{ 99, 325, 3411, 223, 299, 86 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_30_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 75, 84 },
	{ 87, 259, 3135, 151, 227, 80 },
	{
		-90,
		9,
		1489,
		11,
		30,
		90,
		2711,
		2729,
		11
	}
};

const RoomDataOffsets IgorEngine::PART_30_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 3, 12 },
	{ 15, 187, 3063, 79, 155, 80 },
	{
		-90,
		9,
		1489,
		11,
		30,
		90,
		2711,
		2729,
		11
	}
};

const RoomDataOffsets IgorEngine::PART_31_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 79, 96 },
	{ 103, 351, 3367, 247, 323, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_31_FLOPPY_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 32, 43 },
	{ 47, 237, 3183, 131, 207, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_32_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 4, 15 },
	{ 19, 209, 3155, 103, 179, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_33_ROOM_DATA_OFFSETS = {
	{ 180, 6, 30, 5 },
	{ 405, 426 },
	{ 435, 715, 4011, 619, 695, 92 },
	{ -46, 9, 1533, 19, 60, 120, 3571, 3589, 19 }
};

const RoomDataOffsets IgorEngine::PART_34_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 }, // walk paths are built directly by the room
	{ 4, 15 },
	{ 19, 211, 3087, 103, 179, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_35_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 }, // walk paths are built directly by the room
	{ 5, 18 },
	{ 23, 233, 3179, 127, 203, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_36_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 77, 90 },
	{ 95, 303, 3319, 199, 275, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_37_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 75, 84 },
	{ 87, 259, 3135, 151, 227, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_50_ROOM_DATA_OFFSETS = {
	{ 245, 7, 42, 6 },
	{ 589, 594 },
	{ 595, 731, 3467, 619, 695, 76 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_68_ROOM_DATA_OFFSETS = {
	{ 80, 4, 12, 3 },
	{ 148, 159 },
	{ 163, 355, 3231, 247, 323, 80 },
	{ -75, 21, 3472, 100, 30, 60, 13772, 13814, 100 }
};

const RoomDataOffsets IgorEngine::PART_69_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 76, 87 },
	{ 91, 283, 3159, 175, 251, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_70_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 3, 12 },
	{ 15, 187, 3063, 79, 155, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_71_ROOM_DATA_OFFSETS = {
	{ 0, 0, 0, 0 },
	{ 5, 18 },
	{ 23, 231, 3247, 127, 203, 84 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_72_ROOM_DATA_OFFSETS = {
	{ 45, 3, 6, 2 },
	{ 76, 87 },
	{ 91, 281, 3227, 175, 251, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_76_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 32, 43 },
	{ 47, 237, 3183, 131, 207, 82 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_77_ROOM_DATA_OFFSETS = {
	{ 320, 8, 56, 7 },
	{ 835, 844 },
	{ 847, 1019, 3895, 911, 987, 80 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const RoomDataOffsets IgorEngine::PART_81_ROOM_DATA_OFFSETS = {
	{ 20, 2, 2, 1 },
	{ 32, 43 },
	{ 47, 237, 3183, 131, 207, 82 },
	{ -125, 10, 1618, 50, 30, 30, 6818, 6838, 50 }
};

} // End of namespace Igor
