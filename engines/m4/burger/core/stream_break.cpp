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

#include "m4/burger/core/stream_break.h"
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
			series_stream_break_on_frame(_GB(my_stream_viewer), _GB(my_stream_break)->frame, kSERIES_STREAM_BREAK);
			break;
		}

		// Go to the next break in list
		++_GB(my_stream_break_index);
		_GB(my_stream_break) = &_GB(my_stream_break_list)[_GB(my_stream_break_index)];
	}
}

void handle_series_stream_break() {
	int32 thisFrame = _GB(my_stream_break)->frame;
	auto &streamBreak = _GB(my_stream_break);

	// Handle all breaks for this frame number (only one series break can occur on each frame)
	// There can be any number of things to do once we've broken on a particular frame.
	while (thisFrame == streamBreak->frame) {
		// If there is a variable check if equal value
		if ((!streamBreak->variable) || (*(streamBreak->variable) == streamBreak->value)) {
			// Should we do a digi_stop?
			if (streamBreak->flags & DIGI_STOP) {
				digi_stop(streamBreak->channel);
			}

			// Is there is a sound to play?
			if (streamBreak->sound) {
				// Should it be looped?
				if (streamBreak->flags & DIGI_LOOP) {
					digi_play_loop(streamBreak->sound, streamBreak->channel, streamBreak->volume, streamBreak->trigger);
				} else {
					digi_play(streamBreak->sound, streamBreak->channel, streamBreak->volume, streamBreak->trigger);
				}
			} else {
				// No sound to play
				// See if it's a volume channel
				if (streamBreak->channel)
					digi_change_volume(streamBreak->channel, streamBreak->volume);

				// Is there is a trigger to dispatch
				if (streamBreak->trigger != NO_TRIGGER) {
					kernel_trigger_dispatchx(kernel_trigger_create(streamBreak->trigger));
				}
			}
		}

		// Go to next break in list
		++_GB(my_stream_break_index);
		streamBreak = &_GB(my_stream_break_list)[_GB(my_stream_break_index)];
	}

	// If we aren't at end of list yet, set up next stream break
	if (streamBreak->frame >= 0)
		set_next_series_stream_break();
}

machine *series_stream_with_breaks(const seriesStreamBreak list[], const char *name, int32 framerate, frac16 depth, int32 trigger) {
	_GB(my_stream_break_index) = 0;
	_GB(my_stream_break_list) = list;
	_GB(my_stream_viewer) = series_stream(name, framerate, depth, trigger);
	set_next_series_stream_break();

	return _GB(my_stream_viewer);
}

void digi_preload_stream_breaks(const seriesStreamBreak list[]) {
	int32 i = 0;

	// While there is a legitimate frame to break on
	while (list[i].frame >= 0) {
		// If variable is null or variable = value then this break is used
		if ((!list[i].variable) || (*(list[i].variable) == list[i].value)) {
			// Is there is a sound to load for this break
			if (list[i].sound) {
				digi_read_another_chunk();

				// Load the sound
				if (!digi_preload(list[i].sound)) {
					term_message("Preload stream breaks failed: %s", list[i].sound);
				}
			}
		}

		++i;  // Next in list please
	}
}

void digi_unload_stream_breaks(const seriesStreamBreak list[]) {
	int32 i = 0;

	while (list[i].frame >= 0) {
		// Unload any sounds
		if (list[i].sound)
			digi_unload(list[i].sound);

		++i;
	}
}

void palette_prep_for_stream() {
	gr_pal_clear(&_G(master_palette)[0]);			// Clear out colours in palette 
	pal_fade_set_start(&_G(master_palette)[0], 0);	// Set fade to zero percent instantly (0 ticks)
}

} // namespace Burger
} // namespace M4
