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

#include "m4/adv_r/adv_control.h"
#include "m4/core/errors.h"
#include "m4/mem/memman.h"
#include "m4/vars.h"

namespace M4 {

bool kernel_section_startup() {
	_G(game).previous_section = _G(game).section_id;
	_G(game).section_id = _G(game).new_section;

	return true;
}

void player_set_commands_allowed(bool t_or_f) {
	_G(set_commands_allowed_since_last_checked) = true;
	_G(player).comm_allowed = t_or_f;

	if (t_or_f) {
		// OK to do something
		mouse_set_sprite(kArrowCursor);
		g_vars->getInterface()->cancel_sentence();

		track_hotspots_refresh();

	} else {
		// Hour glass
		g_vars->getInterface()->showWaitCursor();
	}
}

void track_hotspots_refresh() {
	warning("TODO: track_hotspots_refresh");
}

} // End of namespace M4
