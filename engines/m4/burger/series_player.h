
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
#include "m4/burger/core/play_break.h"

namespace M4 {
namespace Burger {

// Flags for series_stream_with_breaks and series_play_with_breaks
#define DIGI_LOOP       1024 // these must be more than the series play flags
#define DIGI_STOP       2048 

struct seriesPlayer {
	const seriesPlayBreak *break_list = nullptr;	// Holds the list of breaks for digi plays
	seriesPlayBreak  current_break;			// Current play break
	machine *series_machine = nullptr;		// Series' playback machine
	machine *shadow_machine = nullptr;		// Shadow series' playback machine
	int32  series = 0;						// Holds the series handle upon loading so it can be unloaded at the end
	int32  shadow_series = 0;				// Holds the series handle for the shadow
	int32  index = 0;						// Holds which line of the list currently waiting for a play break
	const char *name = nullptr;				// Name of series to play
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
