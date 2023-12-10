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

#include "m4/burger/core/play_break.h"
#include "m4/platform/sound/digi.h"
#include "m4/burger/vars.h"
#include "m4/platform/sound/digi.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {

static void set_next_series_play_break(int32 slot) {
	seriesPlayer *player = &_G(seriesPlayers)[slot];

	player->current_break = player->break_list[player->index];	// Grab the info from the list
	player->digi_trigger = false;	// Assume for now gSERIES_PLAY_BREAK trigger will come from series_ play not digi_play

	while (player->current_break.firstFrame >= 0) {
		// If there is a positive frame to play from (or to in the case of SERIES_BACKWARDS)

		// if variable is null or variable = value then this break is valid
		if ((!player->current_break.variable) || (*(player->current_break.variable) == player->current_break.value)) {

			// Are the first and last frame numbers backwards?
			if ((player->current_break.lastFrame != -1) && (player->current_break.firstFrame > player->current_break.lastFrame)) {
				player->current_break.flags = player->current_break.flags | SERIES_BACKWARD;	// make sure SERIES_BACKWARD is in flags
				int32 temp = player->current_break.firstFrame;									// switch the first and last frame numbers
				player->current_break.firstFrame = player->current_break.lastFrame;
				player->current_break.lastFrame = temp;
			}

			if (player->current_break.flags & DIGI_STOP) {
				// Should we do a digi_stop?
				digi_stop(player->current_break.channel);
			}

			// Is there is a sound to play?
			if (player->current_break.sound) {
				// Should it be looped?
				if (player->current_break.flags & DIGI_LOOP) {
					digi_play_loop(player->current_break.sound, player->current_break.channel, player->current_break.volume, player->current_break.trigger);

				} else {
					// There is a sound to play and it is not looping (finite) 
					if (_G(seriesPlayers)[slot].current_break.loopCount == -1) {	// Check if series is to loop as long as sound plays (loopCount = -1)
						_G(seriesPlayers)[slot].digi_trigger = true;	// Remember that the gSERIES_PLAY_BREAK will be a sound trigger
						digi_play(player->current_break.sound, player->current_break.channel, player->current_break.volume, kSERIES_PLAY_BREAK_0 + slot);
					} else {
						// Use the play break trigger 
						digi_play(player->current_break.sound, player->current_break.channel, player->current_break.volume, player->current_break.trigger);
					}
				}
			} else {
				// No sound to play so see if there is a trigger to dispatch
				if (player->current_break.trigger != NO_TRIGGER) {
					kernel_trigger_dispatchx(kernel_trigger_create(player->current_break.trigger));
				}
			}

			// Play the series
			if (player->digi_trigger) {
				// Don't ask for the gSERIES_PLAY_BREAK trigger
				player->series_machine = series_play(player->name, player->depth, player->current_break.flags, NO_TRIGGER,
					player->framerate, player->current_break.loopCount,
					player->scale, player->x, player->y,
					player->current_break.firstFrame, player->current_break.lastFrame);
			} else { // ask for the gSERIES_PLAY_BREAK trigger
				player->series_machine = series_play(player->name, player->depth, player->current_break.flags, (int16)slot + kSERIES_PLAY_BREAK_0,
					player->framerate, player->current_break.loopCount,
					player->scale, player->x, player->y,
					player->current_break.firstFrame, player->current_break.lastFrame);
			}

			// play the shadow series if necessary
			if (player->with_shadow) {
				player->shadow_machine = series_play(player->shadow_name, player->depth + 1, player->current_break.flags, NO_TRIGGER,
					player->framerate, player->current_break.loopCount,
					player->scale, player->x, player->y,
					player->current_break.firstFrame, player->current_break.lastFrame);
			}
			++player->index;	 // set up index to the next break in list
			break; // we have handled a break so don't look any farther

		} else {	// variable didn't equate to value so keep looking
			++player->index;	 // go to the next break in list
			player->current_break = player->break_list[player->index]; // grab the info from the list
		}
	}

	if (player->current_break.firstFrame < 0) {
		// There were no more breaks, so unload series/sounds and send off trigger
		player->in_use = false;
		series_unload(player->series);
		if (player->with_shadow)
			series_unload(player->shadow_series);
		if (player->preload_sounds)
			digi_unload_play_breaks(player->break_list);

		kernel_trigger_dispatchx(player->trigger);
	}
}

void handle_series_play_break(int32 slot) {
	seriesPlayer *player = &_G(seriesPlayers)[slot];

	if (player->digi_trigger) { // if this was caused by a digi_trigger 
		kernel_terminate_machine(player->series_machine);
		if (player->with_shadow) {
			kernel_terminate_machine(player->shadow_machine);
		}
	}
	set_next_series_play_break(slot);
}

int32 series_play_with_breaks(const seriesPlayBreak list[], const char *name, frac16 depth, int32 trigger, uint32 flags,
	int32 framerate, int32 scale, int32 x, int32 y) {
	int32 slot;

	// look for a free player slot (MAX_SERIES_PLAYERS)
	for (slot = 0; slot < MAX_SERIES_PLAYERS; ++slot) {
		if (!(_G(seriesPlayers)[slot].in_use))	break;
	}

	if (slot >= MAX_SERIES_PLAYERS) {
		// No free slots
		term_message("**** all seriesPlayers are in use!");
		return -1;
	}

	seriesPlayer *player = &_G(seriesPlayers)[slot];

	player->in_use = true;
	player->index = 0;
	player->break_list = list;
	player->name = name;
	player->depth = depth;
	player->trigger = kernel_trigger_create(trigger);
	player->framerate = framerate;
	player->scale = scale;
	player->x = x;
	player->y = y;
	player->with_shadow = (bool)(flags & WITH_SHADOW);
	player->preload_sounds = (bool)(flags & PRELOAD_SOUNDS);

	player->series = series_load(name, -1, nullptr);	// load sprite series			 									  

	// Check if shadow sprites should be loaded
	if (player->with_shadow) {
		// Make shadow name by adding an "s" to the series name
		Common::strcpy_s(player->shadow_name, name);
		Common::strcat_s(player->shadow_name, "s\0");
		player->shadow_series = series_load(player->shadow_name, -1, nullptr);
	} else {
		player->shadow_name[0] = '\0';
	}

	if (player->preload_sounds) {		 // check if sounds are to be preloaded
		digi_preload_play_breaks(list);
	}

	set_next_series_play_break(slot);
	term_message("series_play_with_breaks:%s  shadow:%s  preload_sounds:%d  slot:%d",
		player->name, player->shadow_name, player->preload_sounds, slot);
	return slot;
}

void digi_preload_play_breaks(const seriesPlayBreak list[], bool loadAll) {
	int32 i = 0;

	// While there is a legitimate frame to break on
	while (list[i].firstFrame >= 0) {
		// If variable is null or variable = value then this break is used
		if ((!list[i].variable) || (*(list[i].variable) == list[i].value) || loadAll) {
			// If there is a sound to load	for this break
			if (list[i].sound) {
				if (!digi_preload(list[i].sound)) {
					term_message("**** Couldn't load: %s.RAW", list[i].sound);
				}
			}
		}

		++i;  // Next in list please
	}
}

void digi_unload_play_breaks(const seriesPlayBreak list[]) {
	int32 i = 0;

	while (list[i].firstFrame >= 0) {
		// If there is a sound to unload
		if (list[i].sound) {
			digi_unload(list[i].sound);	// Unload it
		}
		++i;	// Next in list please
	}
}

} // namespace Burger
} // namespace M4
