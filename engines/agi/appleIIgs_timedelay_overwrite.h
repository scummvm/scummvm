/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef AGI_APPLEIIGS_DELAY_OVERWRITE_H
#define AGI_APPLEIIGS_DELAY_OVERWRITE_H

namespace Agi {

struct AgiAppleIIgsDelayOverwriteRoomEntry {
	int16 fromRoom;
	int16 toRoom;
	int16 timeDelayOverwrite; // time delay here is like on PC, so 0 - unlimited, 1 - 20 cycles, 2 - 10 cycles
};

struct AgiAppleIIgsDelayOverwriteGameEntry {
	uint32 gameId;
	int16  defaultTimeDelayOverwrite; // time delay here is like on PC, so 0 - unlimited, 1 - 20 cycles, 2 - 10 cycles
	const AgiAppleIIgsDelayOverwriteRoomEntry *roomTable;
};

static const AgiAppleIIgsDelayOverwriteRoomEntry appleIIgsDelayOverwriteMH1[] = {
	{ 153, 153,  2 }, // Intro w/ credits
	{ 104, 104,  2 }, // Intro cutscene
	{ 117, 117,  2 }, // Intro cutscene (ego waking up)
	// Room 124+125 are MAD rooms, those seem to work at a proper speed
	{  -1,  -1, -1 }
};

static const AgiAppleIIgsDelayOverwriteRoomEntry appleIIgsDelayOverwriteSQ2[] = {
	{   1,   1, -1 }, // Intro: space ship entering space port, don't touch speed
	{  -1,  -1, -1 }
};

static const AgiAppleIIgsDelayOverwriteGameEntry appleIIgsDelayOverwriteGameTable[] = {
	{ GID_BC,         2, nullptr }, // NEEDS TESTING
	{ GID_GOLDRUSH,   2, nullptr }, // NEEDS TESTING
	{ GID_KQ1,        2, nullptr }, // NEEDS TESTING
	// KQ2 seems to work fine at speed given by scripts (NEEDS FURTHER TESTING)
	{ GID_KQ3,        2, nullptr }, // NEEDS TESTING
	{ GID_KQ4,        2, nullptr }, // NEEDS TESTING
	{ GID_LSL1,       2, nullptr }, // Switch Larry 1 to 10 cycles per second (that's around PC Larry 1's "normal" speed
	{ GID_MH1,       -1, appleIIgsDelayOverwriteMH1 }, // NEEDS TESTING
	{ GID_MIXEDUP,    2, nullptr }, // NEEDS TESTING
	{ GID_PQ1,        2, nullptr }, // NEEDS TESTING
	{ GID_SQ1,        2, nullptr }, // NEEDS TESTING
	{ GID_SQ2,        2, appleIIgsDelayOverwriteSQ2 }, // NEEDS TESTING
	{ GID_AGIDEMO,   -1, nullptr }
};

} // End of namespace Agi

#endif /* AGI_APPLEIIGS_DELAY_OVERWRITE_H */
