
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

#ifndef M4_BURGER_STREAM_BREAK_H
#define M4_BURGER_STREAM_BREAK_H

#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {
namespace Burger {

struct seriesStreamBreak {
	int32 frame;
	const char *sound;
	int32 channel;
	int32 volume;
	int32 trigger;
	uint32 flags;
	const int32 *variable;
	int32 value;
};
#define STREAM_BREAK_END {  -1,    nullptr, 0,   0, NO_TRIGGER, 0, nullptr, 0 }

// Flags for series_stream_with_breaks and series_play_with_breaks
#define DIGI_LOOP       1024 // these must be more than the series play flags
#define DIGI_STOP       2048 

struct StreamBreak_Globals {
	int32 _my_stream_break_index = 0;	// Holds which line of the list currently waiting for a stream break
	const seriesStreamBreak *_my_stream_break_list = nullptr;	// Holds the list of breaks for digi plays
	const seriesStreamBreak *_my_stream_break = nullptr;
	machine *_my_stream_viewer = nullptr;
};

machine *series_stream_with_breaks(const seriesStreamBreak list[], const char *name, int32 framerate, frac16 depth, int32 trigger);
void digi_preload_stream_breaks(const seriesStreamBreak list[]);
void digi_unload_stream_breaks(const seriesStreamBreak list[]);
void handle_series_stream_break();

/**
 * Palette_prep_for_stream is called just before series_stream() for a full screen animation.
 * It clears the palette, (debug: sets ui colors) and sets up a palette fade up from 0%
 */
void palette_prep_for_stream();

} // namespace Burger
} // namespace M4

#endif
