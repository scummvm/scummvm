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

#include "m4/burger/other.h"
#include "m4/core/errors.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {

static void other_fade_me_out(int32 trigger) {
	pal_fade_init(&_G(master_palette)[0], 0, 255, 0, 30, trigger);
}

void other_resurrect_player() {
	KernelTriggerType old_mode;

	if (!g_engine->autosaveExists())
		error_show(FL, 'Burg', "Couldn't resume game");

	_G(kernel).restore_slot = 0;
	old_mode = _G(kernel).trigger_mode;

	_G(kernel).trigger_mode = KT_DAEMON;
	other_fade_me_out(32001);

	_G(kernel).trigger_mode = old_mode;
	player_set_commands_allowed(false);
}

} // namespace Burger
} // namespace M4
