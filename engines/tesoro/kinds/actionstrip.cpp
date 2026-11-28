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

#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/room.h"
#include "tesoro/kinds/actionstrip.h"
#include "tesoro/kinds/objectgroup.h"
#include "tesoro/world.h"

namespace Tesoro {

	// position constants for the action strip
enum { kHiddenY = 480, kRise = 85, kHoverReach = 35 };

void ActionStrip::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	InteractiveObject::init(record, roomSortOffset, levelPlayers, now, world, 0, 0);
	_slot = 0;
}

void ActionStrip::onAttach() {
	if (_level) {
		_pointer = _level->findObject(nameKey64("CURSOR"));
		_group = _level->findObject(nameKey64("ICONOS"));
		_square = _level->findObject(nameKey64("CUADRADO"));
	}
	if (_room)
		_room->insertSorted(this);
	_world->actionStripReady(this);
}

bool ActionStrip::isPicture(const Object *object) const {
	return object && (object->kindId() == 3 || object->kindId() == 4);
}

void ActionStrip::movePictures(int32 dy) {
	if (!_group)
		return;
	const Common::Array<Object *> &members = static_cast<ObjectGroup *>(_group)->members();
	for (uint i = 0; i < members.size(); ++i) {
		if (isPicture(members[i]))
			members[i]->setPosition(members[i]->x(), members[i]->y() + dy);
	}
}

bool ActionStrip::acceptsPointer() {
	if (!_world->actionStripAvailable())
		return false;

	const int32 now = _world->now();
	_world->setHoverText(nullptr);
	_anim.selectBank(0, 1, 1, false, false, now);
	if (_group) {
		const Common::Array<Object *> &members = static_cast<ObjectGroup *>(_group)->members();
		for (uint i = 0; i < members.size(); ++i) {
			if (isPicture(members[i]))
				members[i]->animation().selectBank(0, 0, 1, false, false, now);
		}
	}
	if (_square)
		_square->animation().selectBank(0, 0, 1, false, false, now);
	_hovered = nullptr;
	showHoverText();
	return true;
}

int ActionStrip::showHoverText() {
	if (!_square || !_group || (_square->animation().current().flags() & AnimPlayer::kPlaying))
		return 1;

	const int32 now = _world->now();
	const Common::Array<Object *> &members = static_cast<ObjectGroup *>(_group)->members();

	// Pictures above the screen slide down into view.
	if (!members.empty() && members[0] && members[0]->y() > kHiddenY) {
		for (uint i = 0; i < members.size(); ++i) {
			if (isPicture(members[i])) {
				members[i]->setPosition(members[i]->x(), members[i]->y() - kRise);
				members[i]->animation().selectBank(1, 0, 0, false, false, now);
			}
		}
	}

	_slot = -1;
	Object *found = nullptr;
	for (uint i = 0; i < members.size(); ++i) {
		if (!members[i])
			continue;
		const int32 d = members[i]->x() - _pointer->x();
		if ((d < 0 ? -d : d) < kHoverReach) {
			_slot = (int)i;
			found = members[i];
		}
	}

	if (found != _hovered) {
		if (found)
			found->animation().selectBank(1, 0, 3, false, false, now);
		if (_hovered)
			_hovered->animation().selectBank(2, (uint16)(3 - _hovered->animation().current().current()), 3, false, false, now);
		_hovered = found;
	}
	return 1;
}

int ActionStrip::clearHoverText() {
	if (!_group)
		return 1;

	const int32 now = _world->now();
	const Common::Array<Object *> &members = static_cast<ObjectGroup *>(_group)->members();
	if (!members.empty() && members[0] && members[0]->y() < kHiddenY)
		movePictures(kRise);

	for (uint i = 0; i < members.size(); ++i) {
		if (isPicture(members[i]))
			members[i]->animation().selectBank(0, 2, 3, false, false, now);
	}
	if (_square)
		_square->animation().selectBank(0, 2, 3, false, false, now);
	_anim.selectBank(0, 0, 0, false, false, now);
	return 1;
}

int ActionStrip::setVerbAndClick(int verb) {
	if (_slot >= 0) {
		_world->returnPointerItem();
		_world->cursorFrame(_slot, _slot);
	}
	return 1;
}

} // End of namespace Tesoro
