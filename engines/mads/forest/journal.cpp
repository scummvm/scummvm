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

#include "mads/forest/journal.h"
#include "mads/forest/extra.h"
#include "mads/forest/global.h"
#include "mads/forest/midi.h"
#include "mads/core/global.h"
#include "mads/core/kernel.h"
#include "mads/core/player.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

static int prior_room;

static void change_to_journal() {
	save_player();
	prior_room = room_id;
	new_room = 199;
}

static int get_prior_room() {
	return prior_room;
}

void display_journal() {
	open_interface(JOURNAL_FLY);
	change_to_journal();
}

void leave_journal() {
	close_interface(JOURNAL_FLY);
	global[g009] = 0;
	midi_stop();
	new_room = get_prior_room();
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
