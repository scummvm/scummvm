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
#include "m4/globals.h"

namespace M4 {

void m4FirstRun() {
#if TODO
	add_hot_keys();

	// restore a game?

	if (!_G(kernel).restore_game)
		_G(interface).show();

	_G(kernel).going = true;
#endif
}

void m4SectionStartup() {
	// TODO
}

void m4LastRun() {
	// TODO
}

void m4SceneLoad() {
	// TODO
}

void m4EndScene() {
	// TODO
}

} // End of namespace M4
