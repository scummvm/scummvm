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

#include "common/debug.h"
#include "common/endian.h"
#include "common/util.h"
#include "hollywood/debug.h"
#include "hollywood/runaway/navigation.h"

namespace Hollywood {

namespace {

const uint kWalkZoneCount = 21;
const uint kZonePointCount = 20;
const uint kZoneRouteLength = 19;
const uint kZonePointBytes = kWalkZoneCount * kWalkZoneCount * kZonePointCount * 4;

} // End of anonymous namespace

byte runawayFacingTo(Common::Point position, Common::Point target) {
	const int dx = target.x - position.x;
	const int dy = target.y - position.y;
	// Native 0x433480 uses six standing directions, with a 50-degree slope boundary.
	if (!dx || ABS(dy) > ABS(dx) * 1.191754) {
		return dy > 0 ? 4 : 0;
	}
	return dy < 0 ? (dx > 0 ? 1 : 7) : (dx > 0 ? 3 : 5);
}

bool RunawayWalkMap::walkable(int x, int y) const {
	return x >= 0 && x < _width && y >= 0 && y < _height && _maps[_mask[y * _width + x]] != 0;
}

bool RunawayWalkMap::clearPath(Common::Point start, Common::Point end) const {
	const int count = MAX(1, MAX(ABS(end.x - start.x), ABS(end.y - start.y)));
	// Match the rounded actor position in RunawayWalkPath.
	for (int i = 0; i <= count; ++i) {
		const int x = (start.x * count + (end.x - start.x) * i + count / 2) / count;
		const int y = (start.y * count + (end.y - start.y) * i + count / 2) / count;
		if (!walkable(x, y))
			return false;
	}
	return true;
}

bool RunawayWalkMap::projectToFloor(Common::Point &target) const {
	if (_width <= 0 || _height <= 0 || _mask.size() != (uint)(_width * _height) || _maps.size() < 256)
		return false;
	target.x = CLIP<int>(target.x, 0, _width - 1);
	target.y = CLIP<int>(target.y, 0, _height - 1);
	// Search down/up like the original. Empty columns at the scene's edges
	// use the nearest column containing floor, never an unpainted destination.
	for (int distance = 0; distance < _width; ++distance) {
		for (int side = 0; side < (distance ? 2 : 1); ++side) {
			const int x = target.x + (side ? distance : -distance);
			if (x < 0 || x >= _width)
				continue;
			for (int y = target.y; y < _height; ++y) {
				if (walkable(x, y)) {
					target = Common::Point(x, y);
					return true;
				}
			}
			for (int y = target.y - 1; y >= 0; --y) {
				if (walkable(x, y)) {
					target = Common::Point(x, y);
					return true;
				}
			}
		}
	}
	return false;
}

byte RunawayWalkMap::nearestZone(Common::Point position) const {
	if (walkable(position.x, position.y))
		return _maps[_mask[position.y * _width + position.x]];
	// A walk can be interrupted in a gap within a zone, and some approach
	// positions lie just outside the painted floor.
	int bestDistance = 0x7fffffff;
	byte zone = 0;
	for (int y = 0; y < _height; ++y) {
		for (int x = 0; x < _width; ++x) {
			const byte candidate = _maps[_mask[y * _width + x]];
			if (!candidate || candidate >= kWalkZoneCount)
				continue;
			const int distance = (x - position.x) * (x - position.x) + (y - position.y) * (y - position.y);
			if (distance < bestDistance) {
				bestDistance = distance;
				zone = candidate;
			}
		}
	}
	return zone;
}

bool RunawayWalkMap::findZonePath(Common::Point position, Common::Point target, Common::Array<Common::Point> &path) const {
	if (_routes->size() != kRouteDataSize)
		return false;
	byte zone = nearestZone(position);
	const byte targetZone = nearestZone(target);
	if (!zone || zone >= kWalkZoneCount || !targetZone || targetZone >= kWalkZoneCount)
		return false;
	const byte *route = _routes->data() + kZonePointBytes + (zone * kWalkZoneCount + targetZone) * kZoneRouteLength;
	for (uint step = 0; zone != targetZone && step < kZoneRouteLength; ++step) {
		const byte nextZone = route[step];
		if (!nextZone || nextZone >= kWalkZoneCount)
			return false;
		const byte *points = _routes->data() + (zone * kWalkZoneCount + nextZone) * kZonePointCount * 4;
		int bestDistance = 0x7fffffff;
		Common::Point next;
		for (uint i = 0; i < kZonePointCount; ++i) {
			const uint x = READ_LE_UINT16(points + i * 4), y = READ_LE_UINT16(points + i * 4 + 2);
			if (x == 0xffff)
				break;
			if (x >= (uint)_width || y >= (uint)_height)
				return false;
			const Common::Point point(x, y);
			int distance = position.sqrDist(point);
			// The final boundary also accounts for the destination (native 0x4332f0).
			if (nextZone == targetZone)
				distance += target.sqrDist(point);
			if (distance < bestDistance) {
				bestDistance = distance;
				next = point;
			}
		}
		if (bestDistance == 0x7fffffff)
			return false;
		path.push_back(next);
		position = next;
		zone = nextZone;
	}
	if (zone != targetZone)
		return false;
	path.push_back(target);
	return true;
}

void RunawayWalkMap::findPath(Common::Point position, Common::Point target, Common::Array<Common::Point> &path) const {
	path.clear();
	if (_mask.size() != (uint)(_width * _height) || _maps.size() < 256)
		return;
	// Original zone routes can cross intentional gaps in the floor mask.
	if (_routes && findZonePath(position, target, path))
		return;
	path.clear();
	if (position.x < 0 || position.x >= _width || position.y < 0 || position.y >= _height)
		return;
	// Search only the connected floor. Using pixel resolution preserves the
	// one-pixel-wide approach to the plug, which a coarse navigation grid loses.
	Common::Array<int32> parent;
	Common::Array<int> queue;
	parent.resize(_mask.size());
	for (uint i = 0; i < parent.size(); ++i)
		parent[i] = -1;
	const int start = position.y * _width + position.x;
	parent[start] = start;
	queue.push_back(start);
	int closest = start, bestDistance = 0x7fffffff;
	const int dx[] = {1, -1, 0, 0, 1, 1, -1, -1};
	const int dy[] = {0, 0, 1, -1, 1, -1, 1, -1};
	for (uint head = 0; head < queue.size(); ++head) {
		const int pixel = queue[head], x = pixel % _width, y = pixel / _width;
		const int distance = (x - target.x) * (x - target.x) + (y - target.y) * (y - target.y);
		if (distance < bestDistance) {
			closest = pixel;
			bestDistance = distance;
			if (!distance)
				break;
		}
		for (uint direction = 0; direction < 8; ++direction) {
			const int nx = x + dx[direction], ny = y + dy[direction], next = ny * _width + nx;
			if (walkable(nx, ny) && parent[next] == -1) {
				parent[next] = pixel;
				queue.push_back(next);
			}
		}
	}
	Common::Array<Common::Point> reverse;
	for (int pixel = closest; pixel != start; pixel = parent[pixel])
		reverse.push_back(Common::Point(pixel % _width, pixel / _width));
	Common::Point previous = position;
	for (int i = (int)reverse.size() - 1; i >= 0;) {
		// A blocked intermediate pixel does not rule out a clear longer segment.
		int next = 0;
		while (next < i && !clearPath(previous, reverse[next]))
			++next;
		previous = reverse[next];
		path.push_back(previous);
		i = next - 1;
	}
	debugC(1, kDebugScene, "Runaway: walk to %d,%d, reachable %d,%d, %u waypoints",
		target.x, target.y, closest % _width, closest / _width, path.size());
}

void RunawayWalkPath::reset(Common::Point position) {
	clear();
	_x = position.x;
	_y = position.y;
}

void RunawayWalkPath::findPath(const RunawayWalkMap &map, Common::Point target) {
	clear();
	map.findPath(position(), target, _points);
}

void RunawayWalkPath::setTarget(Common::Point target) {
	clear();
	_points.push_back(target);
}

Common::Point RunawayWalkPath::position() const {
	return Common::Point((int)roundf(_x), (int)roundf(_y));
}

bool RunawayWalkPath::advance(float travel, Common::Point &actorPosition, byte &facing) {
	const float previousX = _x, previousY = _y;
	while (travel > 0 && active()) {
		const Common::Point target = _points[_index];
		const float dx = target.x - _x, dy = target.y - _y;
		const float distance = sqrtf(dx * dx + dy * dy);
		if (distance > 0) {
			// Rounding of the remaining distance must not turn Brian mid-segment.
			if (_segmentFacing == 0xff) {
				if (ABS(dx) > ABS(dy) * 2)
					_segmentFacing = dx > 0 ? 2 : 6;
				else if (ABS(dy) > ABS(dx) * 2)
					_segmentFacing = dy > 0 ? 4 : 0;
				else
					_segmentFacing = dy > 0 ? (dx > 0 ? 3 : 5) : (dx > 0 ? 1 : 7);
			}
			facing = _segmentFacing;
		}
		if (distance <= travel) {
			_x = target.x;
			_y = target.y;
			travel -= distance;
			++_index;
			_segmentFacing = 0xff;
		} else {
			_x += dx * travel / distance;
			_y += dy * travel / distance;
			travel = 0;
		}
	}
	actorPosition = position();
	return _x != previousX || _y != previousY;
}

} // End of namespace Hollywood
