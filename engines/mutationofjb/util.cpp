/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mutationofjb/util.h"

#include "common/str.h"
#include "common/translation.h"

#include "engines/engine.h"

namespace MutationOfJB {

void reportFileMissingError(const char *fileName) {
	const char *msg = _s("Unable to locate the '%s' engine data file");
	Common::U32String errorMessage = Common::U32String::format(_(msg), fileName);
	GUIErrorMessage(errorMessage);
	warning(msg, fileName);
}

Common::String toUpperCP895(const Common::String &str) {
	static const byte conversionTable[] = {
		0x00, 0x9A, 0x90, 0x85, 0x8E, 0x00, 0x00, 0x80, 0x89, 0x00, 0x00, 0x00, 0x9C, 0x8A, 0x00, 0x00, /* 0x80-0x8F */
		0x00, 0x92, 0x00, 0xA7, 0x99, 0x00, 0xA6, 0x00, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, /* 0x90-0x9F */
		0x8F, 0x8B, 0x95, 0x97, 0xA5, 0x00, 0x00, 0x00, 0x9B, 0x9E, 0xAB, 0x00                          /* 0xA0-0xAB */
	};

	Common::String ret = str;
	for (Common::String::iterator it = ret.begin(); it != ret.end(); ++it) {
		const byte cp895Byte = reinterpret_cast<const byte &>(*it);
		if (cp895Byte < 0x80) {
			*it = static_cast<char>(toupper(*it));
		} else if (cp895Byte <= 0xAB) {
			byte newChar = conversionTable[cp895Byte - 0x80];
			if (newChar != 0) {
				reinterpret_cast<byte &>(*it) = newChar;
			}
		}
	}
	return ret;
}

}
