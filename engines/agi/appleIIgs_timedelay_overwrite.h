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
	int16 activePictureNr; // resource number of current active background picture
	int16 timeDelayOverwrite; // time delay here is like on PC
	// so 0 - unlimited, 1 - 20 cycles, 2 - 10 cycles, -1 means do not touch speed set by scripts
	bool onlyWhenPlayerNotInControl; // only sets spee, when play is not in control
};

struct AgiAppleIIgsDelayOverwriteGameEntry {
	uint32 gameId;
	int16  defaultTimeDelayOverwrite; // time delay here is like on PC
	// so 0 - unlimited, 1 - 20 cycles, 2 - 10 cycles, -1 means do not touch speed set by scripts
	const AgiAppleIIgsDelayOverwriteRoomEntry *roomTable; // pointer to room table (optional)
};

static const AgiAppleIIgsDelayOverwriteRoomEntry appleIIgsDelayOverwriteKQ4[] = {
	{ 120, 121, -1, -1, true }, // Part of the intro: Graham gets his hat, throws it and breaks down, don't touch speed (3 is set)
	{ 128, 128, -1, -1, true }, // Part of the intro: first actual room for gameplay, but during intro, don't touch speed (3 is set)
	{  92,  92, -1, -1, true }, // Part of caught by gargoyle w/ Lolotte cutscene (3 is set)
	// room 54 sets the speed for a short time to 3 right after entering "clean" command. It doesn't seem to hurt that we switch it down to 2
	// room 92 is dual use, part of cutscenes, part of gameplay, that's why we only stop touching it, when player is not in control
	{ 135, 135, -1, -1, true }, // Part of ending cutscene. Don't touch speed (3 is set)
	{  -1,  -1, -1, -1, false }
};

static const AgiAppleIIgsDelayOverwriteRoomEntry appleIIgsDelayOverwriteMH1[] = {
	//{ 153, 153, -1,  2, false }, // Intro w/ credits
	//{ 104, 104, -1,  2, false }, // Intro cutscene
	//{ 117, 117, -1,  2, false }, // Intro cutscene (ego waking up)
	{ 114, 114, -1, -1, false }, // interactive MAD map
	{ 124, 125, -1, -1, false }, // MAD during intro (tracking), seem to work properly at given speed
	{ 132, 133, -1, -1, false }, // MAD day 2 intro (tracking)
	{ 137, 137, 17, -1, false }, // Night Club 4th arcade game - game sets speed to 7, needs to run that slow to be playable
	{ 137, 137, -1,  4, false }, // Night Club first few arcade games - game sets speed to 0, we need to fix it
	{ 115, 116, -1, -1, false }, // MAD day 3 intro (tracking)
	{ 148, 148, -1, -1, false }, // day 3: arcade sequence under pawn shop (game sets speed to 6)
	{ 103, 103, -1, -1, false }, // MAD day 4 intro (tracking)
	{ 105, 105, -1, -1, false }, // day 4 tracking mini game right at the start (game sets speed to 3)
	{ 107, 107, -1, -1, false }, // MAD day 4 intro (tracking)
	{ 112, 112, -1, -1, false }, // MAD day 4 intro (tracking)
	{  -1,  -1, -1, -1, false }
};

static const AgiAppleIIgsDelayOverwriteRoomEntry appleIIgsDelayOverwriteMIXEDUP[] = {
	{   1,   1, -1,  3, true }, // Jack and Jill cutscene	(issue 11210)
	{  -1,  -1, -1, -1, false }
};


static const AgiAppleIIgsDelayOverwriteRoomEntry appleIIgsDelayOverwriteSQ2[] = {
	{   1,   1, -1, -1, false }, // Intro: space ship entering space port, don't touch speed
	{  -1,  -1, -1, -1, false }
};

static const AgiAppleIIgsDelayOverwriteGameEntry appleIIgsDelayOverwriteGameTable[] = {
	{ GID_BC,         2, nullptr }, // sets the speed at the start and doesn't modify it
	{ GID_GOLDRUSH,   2, nullptr },
	{ GID_KQ1,        2, nullptr },
	// KQ2 seems to work fine at speed given by scripts
	{ GID_KQ3,        2, nullptr },
	{ GID_KQ4,        2, appleIIgsDelayOverwriteKQ4 },
	{ GID_LSL1,       2, nullptr }, // Switch Larry 1 to 10 cycles per second (that's around PC Larry 1's "normal" speed
	{ GID_MH1,        2, appleIIgsDelayOverwriteMH1 },
	{ GID_MIXEDUP,    2, appleIIgsDelayOverwriteMIXEDUP },
	{ GID_PQ1,        2, nullptr },
	{ GID_SQ1,        2, nullptr }, // completed, no issues using these settings
	{ GID_SQ2,        2, appleIIgsDelayOverwriteSQ2 },
	{ GID_AGIDEMO,   -1, nullptr }
};

} // End of namespace Agi

#endif /* AGI_APPLEIIGS_DELAY_OVERWRITE_H */
