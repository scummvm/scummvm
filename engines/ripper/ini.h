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

#ifndef RIPPER_INI_H
#define RIPPER_INI_H

#include "common/str.h"

namespace Common {
class INIFile;
}

namespace Ripper {

Common::String getIniString(const Common::INIFile &ini, const char *section,
	const char *key, const Common::String &fallback = Common::String());
int getIniInt(const Common::INIFile &ini, const char *section,
	const char *key, int fallback = 0);
bool readIniUint(const Common::INIFile &ini, const Common::String &section,
	const Common::String &key, uint &value);

} // End of namespace Ripper

#endif // RIPPER_INI_H
