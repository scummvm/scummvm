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

#ifndef MADS_CORE_CONCAT_H
#define MADS_CORE_CONCAT_H

#include "common/stream.h"

namespace MADS {
namespace MADSV2 {

#define CONCAT_ID_STRING                "MADSCONCAT 1.0\032"
#define CONCAT_ID_LENGTH                16
#define CONCAT_ID_CHECK                 10

#define CONCAT_EXT                      ".HAG"

#define CONCAT_MAX_FILES                750
#define CONCAT_MAX_OUT                  10

struct Concat {
	long file_offset;
	long file_size;
	char name[13];

	void load(Common::SeekableReadStream *src) {
		file_offset = src->readSint32LE();
		file_size = src->readSint32LE();
		src->read(name, 13);
		src->skip(1);
	}
	static uint record_size() {
		return 4 + 4 + 13 + 1;
	}
};

typedef Concat *ConcatPtr;

} // namespace MADSV2
} // namespace MADS

#endif
