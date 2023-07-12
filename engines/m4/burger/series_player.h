
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

#ifndef M4_BURGER_SERIES_PLAYER_H
#define M4_BURGER_SERIES_PLAYER_H

#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {
namespace Burger {

struct seriesStreamBreak {
	int32 frame = 0;
	const char *sound = nullptr;
	int32 channel = 0;
	int32 volume = 0;
	int32 trigger = 0;
	uint32 flags = 0;
	const int32 *variable = nullptr;
	int32 value = 0;
};

// Used as last line of seriesStreamBreak arrays
#define SERIES_STREAM_BREAK_END -1,nullptr,0,0,NO_TRIGGER,nullptr,nullptr,nullptr

struct seriesPlayBreak {
	int32 firstFrame = 0;
	int32 lastFrame = 0;
	char *sound = nullptr;
	int32 channel = 0;
	int32 volume = 0;
	int32 trigger = 0;
	uint32 flags = 0;
	int32 loopCount = 0;
	int32 *variable = nullptr;
	int32 value = 0;
};
#define WITH_SHADOW     1       // Flag for seriesPlayBreak
#define PRELOAD_SOUNDS  2       // Flag for seriesPlayBreak
#define SERIES_PLAY_BREAK_END -1,-1,nullptr,0,0,NO_TRIGGER,nullptr,0,nullptr,nullptr        // used as last line of seriesPlayBreak arrays

struct seriesPlayer {
	seriesPlayBreak *break_list = nullptr;	// Holds the list of breaks for digi plays
	seriesPlayBreak  current_break;			// Current play break
	machine *series_machine = nullptr;		// Series' playback machine
	machine *shadow_machine = nullptr;		// Shadow series' playback machine
	int32  series = 0;						// Holds the series handle upon loading so it can be unloaded at the end
	int32  shadow_series = 0;				// Holds the series handle for the shadow
	int32  index = 0;						// Holds which line of the list currently waiting for a play break
	char *name = nullptr;					// Name of series to play
	char   shadow_name[80] = { 0 };
	int32  framerate = 0;
	int32  trigger = 0;
	frac16 depth = 0;
	int32  scale = 0;
	int32  x = 0;
	int32  y = 0;
	bool   with_shadow = false;				// True if a shadow is to be played in unison with series
	bool   preload_sounds = false;			// True if you want to preload and unload the sounds automatically
	bool   digi_trigger = false;			// True if gSERIES_PLAY_BREAK trigger has come from a digi_play and not a series_play
	bool   in_use = false;					// Stops two calls from happening at the same time

	/**
	 * Clear the player
	 */
	void clear();
};
#define MAX_SERIES_PLAYERS 3

class SeriesPlayers {
private:
	seriesPlayer _players[MAX_SERIES_PLAYERS];
public:
	seriesPlayer &operator[](uint idx) {
		assert(idx < MAX_SERIES_PLAYERS);
		return _players[idx];
	}
	const seriesPlayer &operator[](uint idx) const {
		assert(idx < MAX_SERIES_PLAYERS);
		return _players[idx];
	}

	/**
	 * Clear the players
	 */
	void clear();
};

} // namespace Burger
} // namespace M4

#endif
