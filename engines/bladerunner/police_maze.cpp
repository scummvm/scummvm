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

#include "bladerunner/police_maze.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/police_maze_track.h"
#include "bladerunner/savefile.h"

namespace BladeRunner {

PoliceMaze::PoliceMaze(BladeRunnerEngine *vm) : _vm(vm) {
	reset();
}

PoliceMaze::~PoliceMaze() {
	reset();
}

bool PoliceMaze::init() {
	return true;
}

void PoliceMaze::save(SaveFile &f) {
	f.write(_tracksCount);
	f.write(_a2);
	f.write(_a3);
	for (int i = 0; i < 64; ++i) {
		_tracks[i]->save(f);
	}
}

void PoliceMaze::reset() {
	_tracksCount = 0;
	_a2 = 0;
	_a3 = 0;
	for (int i = 0; i < 64; ++i) {
		_tracks[i] = nullptr;
	}
	_a4 = 0;
	_a5 = 0;
}

} // End of namespace BladeRunner
