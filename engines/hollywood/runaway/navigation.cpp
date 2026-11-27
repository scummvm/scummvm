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
	// Match the integer actor positions in RunawayWalkPath.
	for (int i = 0; i <= count; ++i) {
		const int x = start.x + (end.x - start.x) * i / count;
		const int y = start.y + (end.y - start.y) * i / count;
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
	_position = position;
}

void RunawayWalkPath::clear() {
	_points.clear();
	_steps.clear();
	_index = _elapsed = _frame = 0;
	_footContact = false;
}

void RunawayWalkPath::findPath(const RunawayWalkMap &map, Common::Point target) {
	map.findPath(_position, target, _points);
	if (_points.empty())
		_points.push_back(_position);
}

void RunawayWalkPath::setTarget(Common::Point target) {
	_points.clear();
	_points.push_back(target);
}

void RunawayWalkPath::appendFrame(Step &step, byte frame) {
	step.frame = frame;
	_steps.push_back(step);
}

void RunawayWalkPath::appendTurn(Step &step, byte facing) {
	// Native 0x5a53f8 takes the shortest turn, clockwise for a half turn.
	const int direction = (facing + 8 - step.facing) % 8 <= 4 ? 1 : -1;
	while (step.facing != facing) {
		step.facing = (step.facing + 8 + direction) % 8;
		_steps.push_back(step);
	}
}

void RunawayWalkPath::start(byte facing, byte arrivalFacing, const Common::Array<float> &scales, bool museum) {
	assert(facing < 8 && (arrivalFacing < 8 || arrivalFacing == 0xff));
	assert(scales.size() == _points.size() + 1);
	_steps.clear();
	_index = _elapsed = 0;
	Step step(_position, facing, _frame);
	// Native strides per animation cel, clockwise from north (full game 0x40eac0).
	static const byte strides[] = {8, 11, 19, 11, 8, 11, 19, 11};
	for (uint i = 0; i < _points.size(); ++i) {
		const Common::Point from = step.position, target = _points[i];
		const bool last = i + 1 == _points.size();
		const float dx = target.x - from.x, dy = target.y - from.y;
		if (from == target) {
			if (!last)
				continue;
			if (!step.frame) {
				if (arrivalFacing != 0xff && arrivalFacing != step.facing) {
					appendFrame(step, 13);
					appendFrame(step, 14);
					appendTurn(step, arrivalFacing);
					appendFrame(step, 0);
				}
				continue;
			}
		}
		// Keep the native 10/50-degree walking sectors fixed for each segment.
		byte direction = step.facing;
		if (ABS(dy) < ABS(dx) * 0.176327f)
			direction = dx > 0 ? 2 : 6;
		else if (ABS(dy) > ABS(dx) * 1.191754f)
			direction = dy > 0 ? 4 : 0;
		else if (dx || dy)
			direction = dy > 0 ? (dx > 0 ? 3 : 5) : (dx > 0 ? 1 : 7);

		// Native 0x42f4e0 also joins a new path from an interrupted transition.
		if (!step.frame) {
			appendFrame(step, 13);
			appendFrame(step, 14);
			appendFrame(step, 11);
		} else if (step.frame == 13 || step.frame == 14) {
			if (step.frame == 13)
				appendFrame(step, 14);
			appendFrame(step, 11);
		} else if (step.frame == 15 || step.frame == 16) {
			if (step.frame == 16)
				appendFrame(step, 15);
			appendFrame(step, 3);
		} else if (step.frame == 17 || step.frame == 18) {
			if (step.frame == 18)
				appendFrame(step, 17);
			appendFrame(step, 9);
		}
		appendTurn(step, direction);
		// Native 0x42f3d0 uses the mean perspective height at the endpoints.
		// The museum demo (0x433930) measures diagonal walks along X.
		const float distance = direction == 0 || direction == 4 ? ABS(dy) :
			museum || direction == 2 || direction == 6 ? ABS(dx) : sqrtf(dx * dx + dy * dy);
		const float scale = MAX<float>(1.0f / 455, (scales[i] + scales[i + 1]) / 2);
		const float count = distance / (strides[direction] * scale);
		int frames = (int)count;
		if (count - frames > 0.5f)
			++frames;
		frames = MAX(1, frames);
		if (last) {
			// Finish on gait cel 3 or 9, which joins the two stopping poses.
			const byte endFrame = 1 + (step.frame + frames - 1) % 12;
			int adjustment = 3 - endFrame % 6;
			if (frames + adjustment <= 0)
				adjustment += 6;
			frames += adjustment;
		}
		for (int j = 1; j <= frames; ++j) {
			step.position = Common::Point(from.x + (int)(dx * j / frames), from.y + (int)(dy * j / frames));
			appendFrame(step, 1 + step.frame % 12);
		}
		if (!last)
			continue;
		if (arrivalFacing != 0xff)
			appendTurn(step, arrivalFacing);
		const byte stopFrame = step.frame == 3 ? 15 : 17;
		appendFrame(step, stopFrame);
		appendFrame(step, stopFrame + 1);
		if (step.facing == 2 || step.facing == 6)
			step.facing = step.facing == 2 ? 3 : 5;
		appendFrame(step, 0);
	}
	if (_steps.empty())
		_frame = 0;
}

uint RunawayWalkPath::advance(uint elapsed, Common::Point &position, byte &facing) {
	_elapsed += elapsed;
	uint footsteps = 0;
	while (_elapsed >= kFrameDuration && active()) {
		_elapsed -= kFrameDuration;
		const Step &step = _steps[_index++];
		if (step.position != _position || step.facing != facing || step.frame != _frame) {
			// Native 0x431950 sounds cels 6, 12 and 0, suppressing adjacent contacts.
			if (!_footContact && (step.frame == 6 || step.frame == 12 || step.frame == 0)) {
				++footsteps;
				_footContact = true;
			} else {
				_footContact = false;
			}
		}
		position = _position = step.position;
		facing = step.facing;
		_frame = step.frame;
	}
	return footsteps;
}

static int cameraStep(int height) {
	return height > 341 ? 4 : height > 227 ? 2 : 1;
}

Common::Point RunawayCamera::followWalk(Common::Point camera, Common::Point previous, Common::Point position, int height, bool fast) {
	// Native 0x42fee0 follows each walking cel, outside a nine-pixel center band.
	const int dx = position.x - previous.x, dy = position.y - previous.y;
	const int x = position.x - camera.x, y = position.y - height / 2 - camera.y;
	const int step = cameraStep(height);
	if ((dx > 0 && x >= 517) || (dx < 0 && x < 508))
		camera.x += fast ? dx : dx > 0 ? step : -step;
	if ((dy > 0 && y >= 305) || (dy < 0 && y < 296))
		camera.y += fast ? dy : dy > 0 ? step : -step;
	return camera;
}

Common::Point RunawayCamera::followIdle(Common::Point camera, Common::Point position, int height, uint elapsed) {
	// Native 0x434c50 continues smooth scrolling every 10 ms after walking stops.
	_elapsed += elapsed;
	const int step = MAX(2, cameraStep(height));
	while (_elapsed >= 10) {
		_elapsed -= 10;
		const int x = position.x - camera.x, y = position.y - height / 2 - camera.y;
		if (x >= 517)
			camera.x += step;
		else if (x < 508)
			camera.x -= step;
		if (y >= 305)
			camera.y += step;
		else if (y < 296)
			camera.y -= step;
	}
	return camera;
}

} // End of namespace Hollywood
