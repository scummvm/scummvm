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

#ifndef HOLLYWOOD_RUNAWAY_NAVIGATION_H
#define HOLLYWOOD_RUNAWAY_NAVIGATION_H

#include "common/array.h"
#include "common/rect.h"

namespace Hollywood {

byte runawayFacingTo(Common::Point position, Common::Point target);

class RunawayWalkMap {
public:
	static const uint kRouteDataSize = 21 * 21 * (20 * 4 + 19);
	RunawayWalkMap(int width, int height, const Common::Array<byte> &mask, const Common::Array<byte> &maps,
			const Common::Array<byte> *routes = nullptr) :
		_width(width), _height(height), _mask(mask), _maps(maps), _routes(routes) {}
	void findPath(Common::Point start, Common::Point target, Common::Array<Common::Point> &path) const;
	bool projectToFloor(Common::Point &target) const;

private:
	bool walkable(int x, int y) const;
	bool clearPath(Common::Point start, Common::Point end) const;
	byte nearestZone(Common::Point position) const;
	bool findZonePath(Common::Point start, Common::Point target, Common::Array<Common::Point> &path) const;
	int _width, _height;
	const Common::Array<byte> &_mask, &_maps;
	const Common::Array<byte> *_routes;
};

class RunawayWalkPath {
public:
	static const uint kFrameDuration = 60;
	explicit RunawayWalkPath(Common::Point position = Common::Point()) : _position(position) {}
	void reset(Common::Point position);
	void clear();
	void findPath(const RunawayWalkMap &map, Common::Point target);
	void setTarget(Common::Point target);
	const Common::Array<Common::Point> &points() const { return _points; }
	// Perspective scales at the starting position and each waypoint.
	void start(byte facing, byte arrivalFacing, const Common::Array<float> &scales, bool museum = false);
	// Advance the original 60 ms cels and return the number of foot contacts.
	uint advance(uint elapsed, Common::Point &position, byte &facing);
	bool active() const { return _index < _steps.size(); }
	byte frame() const { return _frame; }
	uint timeToNextFrame() const { return kFrameDuration - _elapsed; }

private:
	struct Step {
		Common::Point position;
		byte facing, frame;
		Step(Common::Point p, byte direction, byte cel) : position(p), facing(direction), frame(cel) {}
	};
	void appendFrame(Step &step, byte frame);
	void appendTurn(Step &step, byte facing);
	Common::Point _position;
	Common::Array<Common::Point> _points;
	Common::Array<Step> _steps;
	uint _index = 0, _elapsed = 0;
	byte _frame = 0;
	bool _footContact = false;
};

class RunawayCamera {
public:
	void reset() { _elapsed = 0; }
	static Common::Point followWalk(Common::Point camera, Common::Point previous, Common::Point position, int height, bool fast);
	Common::Point followIdle(Common::Point camera, Common::Point position, int height, uint elapsed);

private:
	uint _elapsed = 0;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_NAVIGATION_H
