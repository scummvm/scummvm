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

namespace Scumm {

MortimerFlight::MortimerFlight() : _initialInteractive(true), _interactive(true) {
	reset();
}

bool MortimerFlight::load(Common::SeekableReadStream &stream, bool interactive) {
	_initialInteractive = interactive;
	return MortimerMovie::load(stream);
}

void MortimerFlight::reset() {
	MortimerMovie::reset();
	_rescuedCount = _rescuedSpecies = 0;
	_nextFlight.clear();
	_weapon = kRevitascope;
	_interactive = _initialInteractive;
	_targets.clear();
	_obstacles.clear();
	memset(_toggles, 0, sizeof(_toggles));
}

bool MortimerFlight::advance() {
	_obstacles.clear();
	for (uint i = 0; i < _targets.size(); ++i)
		_targets[i].active = false;
	return MortimerMovie::advance();
}

int MortimerFlight::objectScale(byte codec) const {
	return _interactive ? 1 : MortimerMovie::objectScale(codec);
}

bool MortimerFlight::skipObject(uint16 a, uint16 b) const {
	return a != 0xffff && (b ? _toggles[a] != _toggles[b] : _toggles[a]);
}

MortimerFlight::Target *MortimerFlight::findTarget(uint16 id) {
	for (uint i = 0; i < _targets.size(); ++i)
		if (_targets[i].id == id)
			return &_targets[i];
	return nullptr;
}

void MortimerFlight::handleAction(const byte *data, uint32 size) {
	MortimerMovie::handleAction(data, size);
	const uint16 code = READ_LE_UINT16(data), flags = READ_LE_UINT16(data + 2);
	const uint16 mode = READ_LE_UINT16(data + 4), id = READ_LE_UINT16(data + 6);
	if (code >= 2 && code <= 4 && mode == 1) {
		const uint16 toggle = READ_LE_UINT16(data + 8);
		if (id == 2000) {
			// Offscreen control object: the remap SKIPs compare against this
			// false toggle, rather than against the individual stone objects.
			_toggles[toggle] = false;
			return;
		}
		if (id >= 1000) {
			Target *target = findTarget(id - 1000);
			if (target)
				target->remapToggle = toggle;
			// Keep the remapped silhouette for the duration of its NUT effect.
			const uint duration = target && target->hitWeapon == kSlimeGun ? 7 : 11;
			_toggles[toggle] = !target || !target->hit || frame() - target->rescuedAt >= duration;
			return;
		}
		Target *target = findTarget(id);
		if (!target && code == 2) {
			Target fresh;
			fresh.id = id;
			fresh.toggle = toggle;
			fresh.remapToggle = 0xffff;
			fresh.rescuedAt = 0;
			fresh.hitWeapon = kRevitascope;
			fresh.active = fresh.hit = false;
			// Salt belongs to the hornet whose ID is 100 lower. Destroying a
			// hornet also disables its subsequent salt objects (SHELL 407c5f).
			if (id >= 800 && id < 900) {
				const Target *hornet = findTarget(id - 100);
				fresh.hit = hornet && hornet->hit;
			}
			_targets.push_back(fresh);
			target = &_targets.back();
		}
		if (!target)
			return;
		target->toggle = toggle;
		_toggles[toggle] = target->hit;
		if (code == 4) {
			const int x = READ_LE_INT16(data + 10), y = READ_LE_INT16(data + 12);
			const int w = READ_LE_INT16(data + 14), h = READ_LE_INT16(data + 16);
			target->rect = Common::Rect(CLIP<int>(x, 0, kWidth), CLIP<int>(y, 0, kHeight),
				CLIP<int>(x + MAX(0, w), 0, kWidth), CLIP<int>(y + MAX(0, h), 0, kHeight));
			target->active = !target->rect.isEmpty();
		} else if (code == 3) {
			target->active = false;
		}
	} else if (code == 2 && mode == 3) {
		_toggles[READ_LE_UINT16(data + 8)] = !_toggles[READ_LE_UINT16(data + 10)];
	} else if (code == 5 && flags == 14 && size >= 28) {
		Obstacle obstacle;
		for (uint i = 0; i < 4; ++i)
			obstacle.corners[i] = Common::Point(READ_LE_INT16(data + 12 + i * 4), READ_LE_INT16(data + 14 + i * 4));
		_obstacles.push_back(obstacle);
	} else if (code == 6 && (flags == 9 || flags == 10)) {
		// SHELL 407c5f calls 40b844/40b7ce at the scripted weapon changes.
		_weapon = flags == 9 ? kSlimeGun : kRevitascope;
	} else if (code == 6 && (flags == 11 || flags == 25)) {
		_interactive = flags == 11;
	} else if (code == 8 && flags == 46) {
		_nextFlight = Common::String((const char *)data + 18, size - 21);
		for (uint i = 0; i < _nextFlight.size(); ++i)
			if (_nextFlight[i] == '\\')
				_nextFlight.setChar('/', i);
	}
}

MortimerFlight::ShotResult MortimerFlight::shoot(const Common::Point &point) {
	// Later objects are drawn on top of earlier ones.
	for (int i = int(_targets.size()) - 1; i >= 0; --i) {
		Target &target = _targets[i];
		if (!target.active || target.hit || !target.rect.contains(point))
			continue;
		const uint species = target.id / 100;
		if (!target.id || species > 9 || species == 8 ||
			(species < 7 && _weapon != kRevitascope) || (species == 7 && _weapon != kSlimeGun))
			continue;
		target.hit = true;
		target.rescuedAt = frame();
		target.hitWeapon = _weapon;
		_toggles[target.toggle] = true;
		if (target.remapToggle != 0xffff)
			_toggles[target.remapToggle] = false;
		if (species < 7) {
			++_rescuedCount;
			_rescuedSpecies |= 1 << species;
		} else if (species == 7) {
			Target *salt = findTarget(target.id + 100);
			if (salt) {
				salt->hit = true;
				_toggles[salt->toggle] = true;
			}
		}
		render();
		return species == 9 ? kShellRestored : species == 7 ? kHornetHit : kAnimalRescued;
	}
	return kMiss;
}

bool MortimerFlight::hitsObstacle(const Common::Point &point) const {
	// SHELL 40e390 uses a 50 x 50 footprint in movie coordinates.
	const Common::Rect footprint(point.x - 25, point.y - 25, point.x + 25, point.y + 25);
	for (uint i = 0; i < _targets.size(); ++i) {
		const Target &target = _targets[i];
		if (target.id >= 800 && target.id < 900 && target.active && !target.hit && target.rect.intersects(footprint))
			return true;
	}
	for (uint n = 0; n < _obstacles.size(); ++n) {
		const Obstacle &obstacle = _obstacles[n];
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
			const int64 radius = 25 * (int64(ABS(dx)) + ABS(dy));
			separated = center + radius < minimum || center - radius > maximum;
		}
		if (!separated)
			return true;
	}
	return false;
}

} // End of namespace Scumm
