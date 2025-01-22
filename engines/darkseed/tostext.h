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

#ifndef DARKSEED_TOSTEXT_H
#define DARKSEED_TOSTEXT_H

#include "common/array.h"
#include "common/file.h"
#include "common/str.h"

namespace Darkseed {

class TosText {
	Common::U32StringArray _textArray;
	uint16 _numEntries = 0;

public:
	bool load();

	uint16 getNumEntries() const;
	const Common::U32String &getText(uint16 textIndex);

private:
	Common::U32String loadString(Common::File &file, uint16 index) const;
};

} // namespace Darkseed

#endif // DARKSEED_TOSTEXT_H
