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

#ifndef TWINE_PARSER_PARSER_H
#define TWINE_PARSER_PARSER_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "twine/shared.h"

namespace TwinE {

class Parser {
protected:
	int _hqrIndex = -1;
	virtual void reset() {}
public:
	virtual ~Parser() {
		reset();
	}
	virtual bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) = 0;

	bool loadFromBuffer(const uint8 *buf, uint32 size, bool lba1);
	bool loadFromHQR(const char *name, int index, bool lba1);

	int hqrIndex() const {
		return _hqrIndex;
	}

	inline bool loadFromHQR(const TwineResource &resource, bool lba1) {
		return loadFromHQR(resource.hqr, resource.index, lba1);
	}
};

} // End of namespace TwinE

#endif
