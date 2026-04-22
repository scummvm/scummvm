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

#ifndef MADS_CORE_COPY_H
#define MADS_CORE_COPY_H

#include "common/stream.h"
#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define COPY_SUCCEED            0
#define COPY_FAIL               1
#define COPY_ESCAPE             2

#define COPY_TRIES_ALLOWED      2

#define COPY_LENGTH             4096
#define COPY_CHECK              256


struct CopyProt {
	byte manual;
	int16 page;
	int16 line;
	int16 word_number;
	char say[20];

	static constexpr int SIZE = 1 + 2 + 2 + 2 + 20;
	void load(Common::SeekableReadStream *src);
};

extern int global_copy_verify();

} // namespace MADSV2
} // namespace MADS

#endif
