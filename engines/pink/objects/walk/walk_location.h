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
#ifndef PINK_WALK_LOCATION_H
#define PINK_WALK_LOCATION_H

#include "pink/utils.h"

namespace Pink {

class WalkLocation : public NamedObject {
public:
	void deserialize(Archive &archive) override;
	void toConsole() const override;
	Common::StringArray &getNeigbors() { return _neighbors;}

private:
	StringArray _neighbors;
};

} // End of namespace Pink

#endif
