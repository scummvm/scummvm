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

#ifndef TESORO_CORE_OBJECTGROUP_H
#define TESORO_CORE_OBJECTGROUP_H

#include "common/array.h"
#include "tesoro/core/object.h"

namespace Tesoro {

/**
 * Object kind 2: a named list of other objects of the same level.
 * Used to group the action icons and moved them together as a unit.
 */
class ObjectGroup : public Object {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers);

	const Common::Array<Object *> &members() const { return _members; }

protected:
	void onAttach() override;

private:
	Common::Array<uint64> _keys;
	Common::Array<Object *> _members;
};

} // End of namespace Tesoro

#endif
