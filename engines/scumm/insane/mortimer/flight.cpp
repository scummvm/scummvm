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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/mortimer/flight.h"

#include "common/endian.h"
#include "common/util.h"
#include "common/tokenizer.h"

namespace Scumm {

MortimerFlight::MortimerFlight(bool isDemo) : _isDemo(isDemo), _initialInteractive(true), _interactive(true),
	_inBranch(false), _steering(kWidth / 2, kHeight / 2), _collisionRadius(25) {
	clearProgress();
	reset();
}

bool MortimerFlight::load(Common::SeekableReadStream &stream, bool interactive, bool continuation) {
	_initialInteractive = interactive;
	return MortimerMovie::load(stream, true, continuation);
}

void MortimerFlight::clearProgress() {
	_rescuedCount = _rescuedSpecies = 0;
	_weapon = kRevitascope;
	memset(_hitTargets, 0, sizeof(_hitTargets));
	context() = MortimerMovieContext();
}

void MortimerFlight::clearHostiles() {
	// Retail hornet hits also suppress their salt projectiles.
	memset(_hitTargets + 800, 0, 100 * sizeof(_hitTargets[0]));
	memset(_hitTargets + 1000, 0, 400 * sizeof(_hitTargets[0]));
}

void MortimerFlight::reset() {
	MortimerCutscene::reset();
	if (_isDemo)
		clearProgress();
	_nextFlight.clear();
	_interactive = _initialInteractive;
	_targets.clear();
	_obstacles.clear();
	_inBranch = false;
}

bool MortimerFlight::advance(bool renderFrame) {
	_obstacles.clear();
	for (uint i = 0; i < _targets.size(); ++i)
		_targets[i].active = false;
	return MortimerMovie::advance(renderFrame);
}

int MortimerFlight::objectScale(byte codec) const {
	return _interactive ? 1 : MortimerMovie::objectScale(codec);
}

bool MortimerFlight::targetHit(uint16 id) const {
	const uint index = id >= 9000 ? id - 8000 : id;
	return index < ARRAYSIZE(_hitTargets) && _hitTargets[index];
}

void MortimerFlight::hitTarget(uint16 id) {
	const uint index = id >= 9000 ? id - 8000 : id;
	if (index < ARRAYSIZE(_hitTargets))
		_hitTargets[index] = true;
}

bool MortimerFlight::targetsRemain(const Common::String &list) const {
	Common::StringTokenizer lines(list, "\r\n\x1a");
	uint count = 0;
	while (!lines.empty() && count < 100) {
		const Common::String line = lines.nextToken();
		uint first = 0, last = 0;
		const int values = sscanf(line.c_str(), "%u - %u", &first, &last);
		if (values < 1 || first > 9399)
			continue;
		if (values == 1)
			last = first;
		for (uint id = first; id <= MIN(last, 9399U) && count < 100; ++id, ++count)
			if (!targetHit(id))
				return true;
	}
	return false;
}

MortimerFlight::Target *MortimerFlight::findTarget(uint16 id) {
	for (uint i = 0; i < _targets.size(); ++i)
		if (_targets[i].id == id)
			return &_targets[i];
	return nullptr;
}

void MortimerFlight::handleAction(const byte *data, uint32 size) {
	MortimerCutscene::handleAction(data, size);
	const uint16 code = READ_LE_UINT16(data), flags = READ_LE_UINT16(data + 2);
	const uint16 mode = READ_LE_UINT16(data + 4), id = READ_LE_UINT16(data + 6);
	if (code >= 2 && code <= 4 && mode == 1) {
		const uint16 toggle = READ_LE_UINT16(data + 8);
		if (id >= 2000 && id < 9000)
			return;
		if (id >= 1000 && id < 2000) {
			Target *target = findTarget(id - 1000);
			if (target)
				target->remapToggle = toggle;
			const uint duration = target && target->hitWeapon == kSlimeGun ? 7 : 11;
			setToggle(toggle, !target || !target->hit || frame() - target->rescuedAt >= duration);
			return;
		}
		Target *target = findTarget(id);
		if (!target && code == 2) {
			Target fresh;
			fresh.id = id;
			fresh.toggle = toggle;
			fresh.remapToggle = 0xffff;
			fresh.rescuedAt = frame() - 1000;
			fresh.hitWeapon = kRevitascope;
			fresh.active = false;
			fresh.hit = targetHit(id);
			// Salt belongs to the hornet whose ID is 100 lower. Destroying a
			// hornet also disables its subsequent salt objects (SHELL 407c5f).
			if (id >= 800 && id < 900) {
				fresh.hit = fresh.hit || targetHit(id - 100);
			}
			_targets.push_back(fresh);
			target = &_targets.back();
		}
		if (!target)
			return;
		target->toggle = toggle;
		setToggle(toggle, target->hit);
		if (code == 4) {
			const int x = READ_LE_INT16(data + 10), y = READ_LE_INT16(data + 12);
			const int w = READ_LE_INT16(data + 14), h = READ_LE_INT16(data + 16);
			target->rect = Common::Rect(CLIP<int>(x, 0, kWidth), CLIP<int>(y, 0, kHeight),
				CLIP<int>(x + MAX(0, w), 0, kWidth), CLIP<int>(y + MAX(0, h), 0, kHeight));
			target->active = !target->rect.isEmpty();
		} else if (code == 3) {
			target->active = false;
		}
	} else if (code == 4 && mode == 2) {
		const int x = READ_LE_INT16(data + 10), y = READ_LE_INT16(data + 12);
		const int w = READ_LE_INT16(data + 14), h = READ_LE_INT16(data + 16);
		_inBranch = _steering.x >= x && _steering.x <= x + w && _steering.y >= y && _steering.y <= y + h;
	} else if (code == 5 && (flags == 13 || flags == 14)) {
		Obstacle obstacle;
		obstacle.outside = flags == 13;
		for (uint i = 0; i < 4; ++i)
			obstacle.corners[i] = Common::Point(READ_LE_INT16(data + 12 + i * 4), READ_LE_INT16(data + 14 + i * 4));
		_obstacles.push_back(obstacle);
	} else if (code == 6 && (flags == 9 || flags == 10)) {
		// SHELL 407c5f calls 40b844/40b7ce at the scripted weapon changes.
		_weapon = flags == 9 ? kSlimeGun : kRevitascope;
	} else if (code == 6 && (flags == 11 || flags == 25)) {
		_interactive = flags == 11;
	} else if (code == 8 && flags == 46 && size >= 21 && id / 1000 != 7) {
		Common::String text((const char *)data + 18, size - 21);
		if (id / 1000 == 11) {
			context().conditions[MortimerMovieContext::kHostilesRemain] = targetsRemain(text);
			_weapon = context().conditions[MortimerMovieContext::kHostilesRemain] ? kSlimeGun : kRevitascope;
		} else if (id / 1000 == 12) {
			context().conditions[MortimerMovieContext::kAnimalsRemain] = targetsRemain(text);
		} else if ((_isDemo || _inBranch) && !text.hasPrefix("@riddles")) {
			// The demo has a fixed route. TITLE.SAN gives its destination
			// before the full-frame branch rectangle, so it cannot use the
			// retail steering predicate at this point in the frame.
			_nextFlight = text;
			for (uint i = 0; i < _nextFlight.size(); ++i)
				if (_nextFlight[i] == '\\')
					_nextFlight.setChar('/', i);
		}
	}
}

MortimerFlight::ShotResult MortimerFlight::shoot(const Common::Point &point) {
	// Later objects are drawn on top of earlier ones.
	for (int i = int(_targets.size()) - 1; i >= 0; --i) {
		Target &target = _targets[i];
		if (!target.active || target.hit || !target.rect.contains(point))
			continue;
		const uint species = target.id >= 9000 && target.id < 9400 ? 7 : target.id / 100;
		if (!target.id || species > 9 || species == 8 ||
			(species < 7 && _weapon != kRevitascope) || (species == 7 && _weapon != kSlimeGun))
			continue;
		target.hit = true;
		hitTarget(target.id);
		target.rescuedAt = frame();
		target.hitWeapon = _weapon;
		setToggle(target.toggle, true);
		if (target.remapToggle != 0xffff)
			setToggle(target.remapToggle, false);
		if (species < 7) {
			++_rescuedCount;
			_rescuedSpecies |= 1 << species;
		} else if (species == 7) {
			const uint16 saltId = target.id % 100 + 800;
			hitTarget(saltId);
			Target *salt = findTarget(saltId);
			if (salt) {
				salt->hit = true;
				setToggle(salt->toggle, true);
			}
		}
		render();
		return species == 9 ? kShellRestored : species == 7 ? kHornetHit : kAnimalRescued;
	}
	return kMiss;
}

bool MortimerFlight::hitsObstacle(const Common::Point &point) const {
	const int radius = _collisionRadius;
	const Common::Rect footprint(point.x - radius, point.y - radius, point.x + radius, point.y + radius);
	for (uint i = 0; i < _targets.size(); ++i) {
		const Target &target = _targets[i];
		if (target.id >= 800 && target.id < 900 && target.active && !target.hit && target.rect.intersects(footprint))
			return true;
	}
	for (uint n = 0; n < _obstacles.size(); ++n) {
		const Obstacle &obstacle = _obstacles[n];
		if (obstacle.outside) {
			int centerX = 0, centerY = 0;
			for (uint i = 0; i < 4; ++i) {
				centerX += obstacle.corners[i].x;
				centerY += obstacle.corners[i].y;
			}
			for (uint i = 0; i < 4; ++i) {
				const Common::Point &a = obstacle.corners[i], &b = obstacle.corners[(i + 1) % 4];
				const int dx = a.y - b.y, dy = b.x - a.x;
				const int side = dx * (centerX - 4 * a.x) + dy * (centerY - 4 * a.y);
				const int distance = dx * (point.x - a.x) + dy * (point.y - a.y);
				if ((side < 0 ? -distance : distance) < radius * (ABS(dx) + ABS(dy)))
					return true;
			}
			continue;
		}
		// Separating axes also handle vertical edges and degenerate quads
		// without the divisions used by the original executable.
		bool separated = false;
		for (uint axis = 0; axis < 6 && !separated; ++axis) {
			int dx = axis == 4 ? 1 : 0, dy = axis == 5 ? 1 : 0;
			if (axis < 4) {
				const Common::Point &a = obstacle.corners[axis], &b = obstacle.corners[(axis + 1) % 4];
				dx = a.y - b.y;
				dy = b.x - a.x;
			}
			int64 minimum = int64(dx) * obstacle.corners[0].x + int64(dy) * obstacle.corners[0].y;
			int64 maximum = minimum;
			for (uint i = 1; i < 4; ++i) {
				const int64 projection = int64(dx) * obstacle.corners[i].x + int64(dy) * obstacle.corners[i].y;
				minimum = MIN(minimum, projection);
				maximum = MAX(maximum, projection);
			}
			const int64 center = int64(dx) * point.x + int64(dy) * point.y;
			const int64 extent = radius * (int64(ABS(dx)) + ABS(dy));
			separated = center + extent < minimum || center - extent > maximum;
		}
		if (!separated)
			return true;
	}
	return false;
}

} // End of namespace Scumm
