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
#include "common/util.h"
#include "hollywood/debug.h"
#include "hollywood/runaway/navigation.h"

namespace Hollywood {

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
	const int count = MAX(ABS(end.x - start.x), ABS(end.y - start.y));
	for (int i = 0; i <= count; ++i) {
		const int x = start.x + (end.x - start.x) * i / MAX(1, count);
		const int y = start.y + (end.y - start.y) * i / MAX(1, count);
		if (!walkable(x, y))
			return false;
	}
	return true;
}

void RunawayWalkMap::findPath(Common::Point position, Common::Point target, Common::Array<Common::Point> &path) const {
	path.clear();
	if (position.x < 0 || position.x >= _width || position.y < 0 || position.y >= _height ||
			_mask.size() != (uint)(_width * _height) || _maps.size() < 256)
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
		int next = i;
		while (next > 0 && clearPath(previous, reverse[next - 1]))
			--next;
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
	map.findPath(position(), target, _points);
	_index = 0;
}

void RunawayWalkPath::setTarget(Common::Point target) {
	clear();
	_points.push_back(target);
}

bool RunawayWalkPath::advance(float travel, Common::Point &actorPosition, byte &facing) {
	const float previousX = _x, previousY = _y;
	while (travel > 0 && active()) {
		const Common::Point target = _points[_index];
		const float dx = target.x - _x, dy = target.y - _y;
		const float distance = sqrtf(dx * dx + dy * dy);
		if (distance > 0) {
			if (ABS(dx) > ABS(dy) * 2)
				facing = dx > 0 ? 2 : 6;
			else if (ABS(dy) > ABS(dx) * 2)
				facing = dy > 0 ? 4 : 0;
			else
				facing = dy > 0 ? (dx > 0 ? 3 : 5) : (dx > 0 ? 1 : 7);
		}
		if (distance <= travel) {
			_x = target.x;
			_y = target.y;
			travel -= distance;
			++_index;
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
