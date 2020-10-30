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

#ifndef TINSEL_DETECTION_H
#define TINSEL_DETECTION_H

#include "engines/advancedDetector.h"

namespace Tinsel {

enum TinselGameID {
	GID_DW1 = 0,
	GID_DW2 = 1,
	GID_NOIR = 2
};

enum TinselGameFeatures {
	GF_SCNFILES = 1 << 0,
	GF_ENHANCED_AUDIO_SUPPORT = 1 << 1,
	GF_ALT_MIDI = 1 << 2,		// Alternate sequence in midi.dat file

	// The GF_USE_?FLAGS values specify how many country flags are displayed
	// in the subtitles options dialog.
	// None of these defined -> 1 language, in ENGLISH.TXT
	GF_USE_3FLAGS = 1 << 3,	// French, German, Spanish
	GF_USE_4FLAGS = 1 << 4,	// French, German, Spanish, Italian
	GF_USE_5FLAGS = 1 << 5	// All 5 flags
};

/**
 * The following is the ScummVM definitions of the various Tinsel versions:
 * TINSEL_V0 - This was an early engine version that was only used in the Discworld 1
 *			demo.
 * TINSEL_V1 - This was the engine version used by Discworld 1. Note that there were two
 *			major releases: an earlier version that used *.gra files, and a later one that
 *			used *.scn files, and contained certain script and engine bugfixes. In ScummVM,
 *			we treat both releases as 'Tinsel 1', since the engine fixes from the later
 *			version work equally well the earlier version data.
 * TINSEL_V2 - This is the engine used for the Discworld 2 game.
 */
enum TinselEngineVersion {
	TINSEL_V0 = 0,
	TINSEL_V1 = 1,
	TINSEL_V2 = 2,
	TINSEL_V3 = 3
};

struct TinselGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

} // End of namespace Tinsel

#endif // TINSEL_DETECTION_H
