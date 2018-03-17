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

#include "bladerunner/police_maze_track.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/savefile.h"

namespace BladeRunner {

PoliceMazeTrack::PoliceMazeTrack(BladeRunnerEngine *vm) : _vm(vm) {
	reset();
}

PoliceMazeTrack::~PoliceMazeTrack() {
	reset();
}

void PoliceMazeTrack::save(SaveFile &f) {
	f.write(_isPresent);
	f.write(_itemId);
	f.write(_count);
	f.write(_dataIndex);
	f.write(_a6);
	f.write(_a7);
	f.write(_pointIndex);
	f.write(_a9);
	f.write(_rotating);
	f.write(_maxAngle);
	f.write(_angleChange);
	f.write(_a13);

	for (int i = 0; i < 100; ++i) {
		f.write(_points[i]);
	}

	f.write(_a4);
	f.write(_a5);
 }

void PoliceMazeTrack::reset() {
	_isPresent   = false;
	_itemId      = -1;
	_count       =  0;
	_data        =  0;
	_dataIndex   =  0;
	_a4          =  0;
	_a5          =  0;
	_time        =  0;
	_a6          =  0;
	_a7          =  0;
	_pointIndex  =  0;
	_a9          =  0;
	_rotating    =  0;
	_maxAngle    =  0;
	_angleChange =  0;
	_a13         =  1;
}

} // End of namespace BladeRunner
