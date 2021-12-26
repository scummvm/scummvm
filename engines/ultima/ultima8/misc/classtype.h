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

#ifndef ULTIMA8_MISC_CLASSTYPE_H
#define ULTIMA8_MISC_CLASSTYPE_H

namespace Ultima {
namespace Ultima8 {

// This is 'type' used to differentiate each class.
// The class name is used primarily to define entities in game saves
struct RunTimeClassType {
	const char *_className;
	inline bool operator == (const RunTimeClassType &other) const {
		return this == &other;
	}
};

#define ENABLE_RUNTIME_CLASSTYPE() \
	static const RunTimeClassType ClassType; \
	const RunTimeClassType & GetClassType() const override { return ClassType; }

#define ENABLE_RUNTIME_CLASSTYPE_BASE() \
	static const RunTimeClassType ClassType; \
	virtual const RunTimeClassType & GetClassType() const { return ClassType; }

#define DEFINE_RUNTIME_CLASSTYPE_CODE(Classname) \
	const RunTimeClassType Classname::ClassType = { #Classname };

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
