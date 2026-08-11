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

#include "twine/parser/holomap.h"
#include "common/debug.h"
#include "common/stream.h"

namespace TwinE {

void TrajectoryData::reset() {
	_trajectories.clear();
}

bool TrajectoryData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
	reset();
	_trajectories.reserve(100); // this is the lba1 amount of trajectories
	while (stream.pos() < stream.size()) {
		Trajectory data;
		data.locationIdx = stream.readSint16LE();
		data.trajLocationIdx = stream.readSint16LE();
		data.vehicleIdx = stream.readSint16LE();
		data.angle.x = stream.readSint16LE();
		data.angle.y = stream.readSint16LE();
		data.angle.z = stream.readSint16LE();
		data.numAnimFrames = stream.readSint16LE();
		assert(data.numAnimFrames < ARRAYSIZE(data.positions));
		for (int32 i = 0; i < data.numAnimFrames; ++i) {
			data.positions[i].x = stream.readSint16LE();
			data.positions[i].y = stream.readSint16LE();
		}
		_trajectories.push_back(data);
	}
	return !stream.err();
}

} // End of namespace TwinE
