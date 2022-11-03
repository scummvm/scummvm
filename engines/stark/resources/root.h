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

#ifndef STARK_RESOURCES_ROOT_H
#define STARK_RESOURCES_ROOT_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * The top level element of the game resource tree.
 *
 * Contains all the levels.
 */
class Root : public Object {
public:
	static const Type::ResourceType TYPE = Type::kRoot;

	Root(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Root();

protected:
	void printData() override;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ROOT_H
