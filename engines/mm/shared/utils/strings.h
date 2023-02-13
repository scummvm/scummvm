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

#ifndef MM_UTILS_STRINGS_H
#define MM_UTILS_STRINGS_H

#include "common/str.h"
#include "common/str-array.h"

namespace MM {

extern Common::String capitalize(const Common::String &str);
extern Common::String camelCase(const Common::String &str);
extern Common::String uppercase(const Common::String &str);
extern Common::String searchAndReplace(const Common::String &str,
	const Common::String &find, const Common::String &replace);
extern int strToInt(const Common::String &str);
extern int hexToInt(const Common::String &str);
extern Common::StringArray splitLines(const Common::String &str);

} // namespace MM

#endif
