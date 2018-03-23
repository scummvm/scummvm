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

#include "bladerunner/bladerunner.h"

#include "bladerunner/police_maze.h"

namespace BladeRunner {

PoliceMaze::PoliceMaze(BladeRunnerEngine *vm) {
	_vm = vm;

	reset();

	for (int i = 0; i < kNumMazeTracks; i++) {
		_tracks[i] = new PoliceMazeTargetTrack;
	}
}

PoliceMaze::~PoliceMaze() {
	for (int i = 0; i < kNumMazeTracks; i++) {
		delete _tracks[i];
	}

	reset();
}

void PoliceMaze::reset() {
	_isActive = false;
	_needAnnouncement = false;
	_announcementRead = false;

	for (int i = 0; i < kNumMazeTracks; i++) {
		_tracks[i] = 0;
	}

	_pm_var1 = 0;
	_pm_var2 = 0;
}

void PoliceMaze::activate() {
	_needAnnouncement = true;
	_announcementRead = false;
}

void PoliceMaze::setPauseState(bool state) {
	warning("PoliceMaze::setPauseState(%d)", state);
}

void PoliceMaze::tick() {
}

PoliceMazeTargetTrack::PoliceMazeTargetTrack() {
	reset();
}

PoliceMazeTargetTrack::~PoliceMazeTargetTrack() {
}

void PoliceMazeTargetTrack::reset() {
	_isPresent = 0;
	_itemId = -1;
	_count = 0;
	_data = 0;
	_dataIndex = 0;
	_pmt_var1 = 0;
	_pmt_var2 = 0;
	_time = 0;
	_pmt_var3 = 0;
	_pmt_var4 = 0;
	_pointIndex = 0;
	_pmt_var5 = 0;
	_rotating = 0;
	_maxAngle = 0;
	_angleChange = 0;
	_visible = true;
}

void PoliceMazeTargetTrack::add(int trackId, float startX, float startY, float startZ, float endX, float endY, float endZ, int count, void *list, bool a11) {
	warning("PoliceMazeTargetTrack::add(%d, %f, %f, %f, %f, %f, %f, %d, %p, %d)", trackId,  startX,  startY,  startZ,  endX,  endY,  endZ,  count,  (void *)list, a11);
}


} // End of namespace BladeRunner
