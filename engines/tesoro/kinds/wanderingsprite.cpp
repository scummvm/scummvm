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

#include "common/util.h"

#include "tesoro/core/camera.h"
#include "tesoro/core/display.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/room.h"
#include "tesoro/kinds/wanderingsprite.h"
#include "tesoro/world.h"

namespace Tesoro {

void WanderingSprite::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	SpriteObject::init(record, roomSortOffset, levelPlayers, now);
	_world = &world;
	for (uint i = 0; i < 12; ++i)
		_anchorName[i] = (char)record.dataByte(2 + i); // name of the object it anchors to
	_anchorName[12] = '\0';
	_pathLow = (int8)record.dataByte(0xe);
	_pathHigh = (int8)record.dataByte(0xf);
	_minWait = record.dataByte(0x12) | (record.dataByte(0x13) << 8); // minimum wait time before animation triggers
	_maxWait = record.dataByte(0x14) | (record.dataByte(0x15) << 8); // maximum wait time before animation triggers
	_stamp = now;
	_wait = (int32)_world->randomNumber(_minWait, _maxWait);
	_travelling = false;
	_flags |= kVisible;
}

void WanderingSprite::onAttach() {
	_anchor = _level ? _level->findObject(nameKey64(_anchorName)) : nullptr;
	if (_room)
		_room->insertSorted(this);
}

const AnimStep *WanderingSprite::pathStep() const {
	if (!_anchor || _anchor->animation().bankCount() == 0)
		return nullptr;
	const AnimBank *bank = _anchor->animation().current().bank();
	return bank && _path < bank->stepCount() ? &bank->step(_path) : nullptr;
}

static int16 pointX(const AnimStep &step, uint i) { return (int16)(step.entries[i] & 0xffff); }
static int16 pointY(const AnimStep &step, uint i) { return (int16)(step.entries[i] >> 16); }

static int32 segmentLength(const AnimStep &step, int32 i) {
	if (i < 0 || (uint)(i + 1) >= step.entries.size())
		return 0;
	const int32 dx = pointX(step, i + 1) - pointX(step, i);
	const int32 dy = pointY(step, i + 1) - pointY(step, i);
	return (int32)sqrt((double)(dx * dx + dy * dy));
}

void WanderingSprite::arrive(const AnimStep &path) {
	_travelling = false;
	_x = _anchor->x() + (path.entries.empty() ? 0 : pointX(path, 0));
	_y = _anchor->y() + (path.entries.empty() ? 0 : pointY(path, 0));
	_wait = (int32)_world->randomNumber(_minWait, _maxWait);
}

void WanderingSprite::updateAnimation(int32 now) {
	_scale = 0x100;
	AnimPlayer &player = _anim.current();

	if (!_travelling) {
		player.clearMovement();
		const int32 elapsed = (now - _stamp) / 1000;
		if (elapsed - _wait > 5) {
			_stamp = now;
			_wait = (int32)_world->randomNumber(_minWait, _maxWait);
			return;
		}
		if (elapsed < _wait)
			return;

		_covered = 0;
		_travelling = true;
		_stamp = now;
		_point = 0;
		_path = _world->randomNumber((uint32)(int32)_pathLow, (uint32)(int32)_pathHigh);
		const AnimStep *path = pathStep();
		if (!path || path->entries.empty()) {
			if (!_warned)
				warning("WanderingSprite: the anchor has no path %u", _path);
			_warned = true;
			_travelling = false;
			return;
		}
		_x = _anchor->x() + pointX(*path, 0);
		_y = _anchor->y() + pointY(*path, 0);
		_anim.selectBank(0, 0, -1, true, false, now);
		return;
	}

	const AnimStep *path = pathStep();
	if (!path)
		return;

	player.advance(now);
	const int32 mx = player.movementX(), my = player.movementY();
	const int32 travelled = (int32)sqrt((double)(mx * mx + my * my));

	int32 length = segmentLength(*path, _point);
	int32 end = _covered + length;
	while (end <= travelled) {
		if ((int32)path->entries.size() - 2 <= _point) {
			arrive(*path);
			return;
		}
		++_point;
		_covered = end;
		length = segmentLength(*path, _point);
		end += length;
	}

	const int32 span = end - _covered;
	if (span == 0 || (uint)(_point + 1) >= path->entries.size()) {
		if (!_warned)
			warning("WanderingSprite: a path segment of length zero");
		_warned = true;
		return;
	}

	const int32 along = travelled - _covered;
	const int32 dx = pointX(*path, _point + 1) - pointX(*path, _point);
	const int32 dy = pointY(*path, _point + 1) - pointY(*path, _point);
	player.setMovement((int16)((along * dx) / span + pointX(*path, _point) - pointX(*path, 0)),
	                   (int16)((along * dy) / span + pointY(*path, _point) - pointY(*path, 0)));

	if ((uint)(_point + 1) < path->scales.size()) {
		const int32 s0 = path->scales[_point];
		const int32 s1 = path->scales[_point + 1];
		_scale = ((s1 - s0) * along) / span + s0;
	}
}

void WanderingSprite::draw(const Camera &camera, Display &display) {
	AnimPlayer &player = _anim.current();
	const AnimStep &step = player.bank()->step(player.current());
	if (!step.hasImage())
		return;

	const int32 x = (((int32)step.imageX * _scale) >> 8) - camera.scaledX() + _x + 320 + player.movementX();
	const int32 y = camera.scaledY() - (((int32)step.imageY * _scale) >> 8) - _y - player.movementY() + 240;
	display.queueImageScaled(x, y, (uint16)_sortValue, player.bank()->image(step.imageIndex), _scale << 8, _scale << 8, 0);
}

} // End of namespace Tesoro
