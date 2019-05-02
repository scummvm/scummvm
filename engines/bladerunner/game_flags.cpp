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

#include "bladerunner/savefile.h"

#include "common/debug.h"

namespace BladeRunner {

GameFlags::GameFlags()
	: _flags(nullptr), _flagCount(0) {
}

GameFlags::~GameFlags() {
	delete[] _flags;
}

void GameFlags::clear() {
	for (int i = 0; i <= _flagCount; ++i) {
		reset(i);
	}
}

void GameFlags::setFlagCount(int count) {
	assert(count > 0);

	_flagCount = count;
	_flags = new uint32[count / 32 + 1]();

	clear();
}

void GameFlags::set(int flag) {
	assert(flag >= 0 && flag <= _flagCount);

	_flags[flag / 32] |= (1 << (flag % 32));
}

void GameFlags::reset(int flag) {
	assert(flag >= 0 && flag <= _flagCount);

	_flags[flag / 32] &= ~(1 << (flag % 32));
}

bool GameFlags::query(int flag) const {
	//debug("GameFlags::query(%d): %d", flag, !!(flags[flag / 32] & (1 << (flag % 32))));
	assert(flag >= 0 && flag <= _flagCount);

	return !!(_flags[flag / 32] & (1 << (flag % 32)));
}

void GameFlags::save(SaveFileWriteStream &f) {
	for (int i = 0; i != _flagCount / 32 + 1; ++i) {
		f.writeUint32LE(_flags[i]);
	}
}

void GameFlags::load(SaveFileReadStream &f) {
	for (int i = 0; i != _flagCount / 32 + 1; ++i) {
		_flags[i] = f.readUint32LE();
	}
}

} // End of namespace BladeRunner
