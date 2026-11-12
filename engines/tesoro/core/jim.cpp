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

#include "tesoro/core/jim.h"

#include "tesoro/core/display.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/world.h"

namespace Tesoro {

static const int kLevelPriority = 0x14;

int Jim::update() {
	_scale = _world.hasWalkArea() ? _world.scaleAt(_x, _y) : 0x100;
	return 1;
}

void Jim::init(const Common::Array<AnimPlayer> &levelPlayers) {
	_flags = kActive | kVisible | kCollidable | kHasAnimation | kHitter;
	_sortValue = 0x23;
	_nameKey = nameKey64("JIM");
	_x = 0x15e;
	_y = 0x14;
	_level = nullptr;
	_room = nullptr;

	byte indices[kBanks];
	for (uint i = 0; i < kBanks; ++i)
		indices[i] = (byte)i;
	_anim.init(levelPlayers, indices, kBanks);

	// Every bank starts on its first frame; the four walking banks get one point each.
	for (uint i = 0; i < kBanks; ++i)
		_anim.player(i).setRange(0, 0, false, false, _world.now());
	for (uint i = kWalkUp; i <= kWalkRight; ++i)
		_anim.player(i).bank()->makeSinglePoints();

	_facing = 3;
	showFacing(3);
}

void Jim::enterLevel(Level *level, Room *room, int32 x, int32 y) {
	if (_level) {
		_level->movedObjects().clear();
		removeFromList(_level->objects(), this);
		_room->removeObject(this);
		_world.removeFromMainList(_level);
	}

	_level = level;
	_room = room;
	level->objects().push_back(this);
	level->movedObjects().clear();
	room->insertSorted(this);

	const int32 now = _world.now();
	level->camera().moveTo(_x, _y, 0, now);
	level->camera().setTarget(this);
	_world.insertInMainList(level, kLevelPriority);

	_x = _previousX = x;
	_y = _previousY = y;
	level->camera().moveTo(x, y, 0, now);
	update();
}

void Jim::leaveLevel() {
	if (!_level)
		return;
	_level->movedObjects().clear();
	removeFromList(_level->objects(), this);
	_room->removeObject(this);
	_level = nullptr;
	_room = nullptr;
}

void Jim::showFacing(int facing) {
	_anim.selectBank(kStanding, facing - 1, facing - 1, false, false, _world.now());
}

void Jim::updateBounds(int32 now) {
	Object::updateBounds(now);
	const int32 scale = _scale;
	_hitRect.x1 = (_hitRect.x1 * scale) >> 8;
	_hitRect.y1 = (_hitRect.y1 * scale) >> 8;
	_hitRect.x2 = (_hitRect.x2 * scale) >> 8;
	_hitRect.y2 = (_hitRect.y2 * scale) >> 8;
}

void Jim::draw(const Camera &camera, Display &display) {
	AnimPlayer &player = _anim.current();
	const AnimStep &seq = player.bank()->step(player.current());
	if (!seq.hasImage())
		return;

	const int32 scale = _scale;
	const int32 x = ((seq.imageX * scale) >> 8) - camera.scaledX() + 0x140 + _x;
	const int32 y = ((camera.scaledY() - ((seq.imageY * scale) >> 8)) - _y) + 0xf0;
	display.queueImageScaled(x, y, _sortValue, player.bank()->image(seq.imageIndex), scale << 8, scale << 8, 0);
}

} // End of namespace Tesoro
