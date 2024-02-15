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

#include "m4/burger/core/release_trigger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

#define _GT(X) _G(releaseTrigger)._##X

void release_trigger_on_digi_state(int32 trigger, int32 checkChannels, int32 matchValue) {
	_GT(released_digi_trigger) = kernel_trigger_create(trigger);
	_GT(check_channels) = checkChannels;
	_GT(match_value) = matchValue;

	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(kRELEASE_TRIGGER_DIGI_CHECK);
	_G(kernel).trigger_mode = oldMode;
}

void release_trigger_digi_check() {
	uint32 i;
	uint32 digi_state = 0;

	// Set channel bits in digi_state that a sound is presently busy in each channel
	for (i = 0; i < 3; ++i) {
		if (digi_play_state(i + 1))	 	// Check if a sound is playing
			digi_state |= (1 << i); 	// Set the bit that corresponds with this channel 
	}

	term_message("%d %d %d", digi_state, _GT(check_channels), _GT(match_value));

	// AND this with the channels we care about
	digi_state &= _GT(check_channels);
	term_message("               %d %d %d", digi_state, _GT(check_channels), _GT(match_value));

	// Compare this with the match values 
	if (digi_state == _GT(match_value)) {
		kernel_trigger_dispatchx(_GT(released_digi_trigger));
	} else {
		kernel_timing_trigger(TENTH_SECOND, kRELEASE_TRIGGER_DIGI_CHECK);
	}
}

} // namespace Burger
} // namespace M4
