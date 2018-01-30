/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/game_flags.h"

#include "bladerunner/game_constants.h"

#include "common/debug.h"

namespace BladeRunner {

GameFlags::GameFlags()
	: _flags(nullptr), _flagCount(0) {
}

GameFlags::~GameFlags() {
	delete[] _flags;
}

void GameFlags::setFlagCount(int count) {
	assert(count > 0);

	_flagCount = count;
	_flags = new uint32[count / 32 + 1];

	for (int i = 0; i <= _flagCount; ++i)
		reset(i);
}

void GameFlags::set(int flag) {
#if BLADERUNNER_DEBUG_CONSOLE
	debug("GameFlags::set(%d)", flag);
#endif

	assert(flag >= 0 && flag <= _flagCount);

	_flags[flag / 32] |= (1 << (flag % 32));
}

void GameFlags::reset(int flag) {
#if BLADERUNNER_DEBUG_CONSOLE
	debug("GameFlags::reset(%d)", flag);
#endif

	assert(flag >= 0 && flag <= _flagCount);

	_flags[flag / 32] &= ~(1 << (flag % 32));
}

bool GameFlags::query(int flag) const {
	//debug("GameFlags::query(%d): %d", flag, !!(flags[flag / 32] & (1 << (flag % 32))));
	assert(flag >= 0 && flag <= _flagCount);

	return !!(_flags[flag / 32] & (1 << (flag % 32)));
}

} // End of namespace BladeRunner
