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

#include "m4/burger/stream_break.h"
#include "m4/platform/sound/digi.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

#define _GB(X) _G(streamBreak)._##X

static void set_next_series_stream_break() {
	_GB(my_stream_break) = &_GB(my_stream_break_list)[_GB(my_stream_break_index)];

	// While there is a positive frame to break on...
	while (_GB(my_stream_break)->frame >= 0) {
		// If variable is null or variable = value then this break is used
		// Note: if Variable != value this break is skipped
		if ((!_GB(my_stream_break)->variable) || (*(_GB(my_stream_break)->variable) == _GB(my_stream_break)->value)) {
			series_stream_break_on_frame(_GB(my_stream_viewer), _GB(my_stream_break)->frame, gSERIES_STREAM_BREAK);
			break;
		}
		++_GB(my_stream_break_index);	 // go to the next break in list
		_GB(my_stream_break) = &_GB(my_stream_break_list)[_GB(my_stream_break_index)]; // grab the info from the list
	}
}

void handle_series_stream_break() {
	int32 thisFrame = _GB(my_stream_break)->frame;

	// Handle all breaks for this frame number (only one series break can occur on each frame)
	// There can be any number of things to do once we've broken on a particular frame.

	while (thisFrame == _GB(my_stream_break)->frame) {

		if ((!_GB(my_stream_break)->variable) || (*(_GB(my_stream_break)->variable) == _GB(my_stream_break)->value)) { // if there is a variable check if equal value

			if (_GB(my_stream_break)->flags & DIGI_STOP) {	 // Should we do a digi_stop?
				digi_stop(_GB(my_stream_break)->channel);
			}

			if (_GB(my_stream_break)->sound) {  // Is there is a sound to play?
				if (_GB(my_stream_break)->flags & DIGI_LOOP) {  // Should it be looped?
					digi_play_loop(_GB(my_stream_break)->sound, _GB(my_stream_break)->channel, _GB(my_stream_break)->volume, _GB(my_stream_break)->trigger);
				} else {
					digi_play(_GB(my_stream_break)->sound, _GB(my_stream_break)->channel, _GB(my_stream_break)->volume, _GB(my_stream_break)->trigger);
				}
			} else {
				// No sound to play, so see if there is a trigger to dispatch
				if (_GB(my_stream_break)->trigger != NO_TRIGGER) {
					kernel_trigger_dispatch(kernel_trigger_create(_GB(my_stream_break)->trigger));
				}
			}
		}

		++_GB(my_stream_break_index);		// Go to next break in list
		_GB(my_stream_break) = &_GB(my_stream_break_list)[_GB(my_stream_break_index)];
	}

	if (_GB(my_stream_break)->frame >= 0) {  // If we aren't at end of list yet, set up next stream break
		set_next_series_stream_break();
	}
}

void series_stream_with_breaks(const seriesStreamBreak list[], const char *name, int32 framerate, frac16 depth, int32 trigger) {
	_GB(my_stream_break_index) = 0;
	_GB(my_stream_break_list) = list;
	_GB(my_stream_viewer) = series_stream(name, framerate, depth, trigger);
	set_next_series_stream_break();
}

void digi_preload_stream_breaks(const seriesStreamBreak list[]) {
	int32 i = 0;

	while (list[i].frame >= 0) {	// While there is a legitimate frame to break on
		// If variable is null or variable = value then this break is used
		if ((!list[i].variable) || (*(list[i].variable) == list[i].value)) {
			if (list[i].sound) {	  // If there is a sound to load for this break
				if (!digi_preload(list[i].sound)) {		 // Check to see if it loaded	ok
					term_message("**** Couldn't load: %s.RAW", list[i].sound);
				}
			}
		}

		++i;  // Next in list please
	}
}

void digi_unload_stream_breaks(const seriesStreamBreak list[]) {
	int32 i = 0;

	while (list[i].frame >= 0) {
		if (list[i].sound) {	  // if there is a sound to unload
			digi_unload(list[i].sound);	// unload it
		}
		++i; // next in list please
	}
}

} // namespace Burger
} // namespace M4
