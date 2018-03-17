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

#ifndef BLADERUNNER_POLICE_MAZE_H
#define BLADERUNNER_POLICE_MAZE_H

namespace BladeRunner {

class BladeRunnerEngine;
class PoliceMazeTrack;
class SaveFile;

class PoliceMaze {
	BladeRunnerEngine *_vm;

	PoliceMazeTrack *_tracks[64];
	int              _tracksCount;
	int              _a2;
	int              _a3;
	int              _a4;
	int              _a5;

public:
	PoliceMaze(BladeRunnerEngine *vm);
	~PoliceMaze();

	bool init();

	void save(SaveFile &f);
	void reset();
};

} // End of namespace BladeRunner

#endif
