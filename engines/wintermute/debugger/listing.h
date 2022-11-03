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

#ifndef LISTING_H_
#define LISTING_H_

#include "common/array.h"
#include "common/str.h"

namespace Wintermute {

struct ListingLine {
	uint number;
	Common::String text;
};

class Listing {
public:
	virtual ~Listing() {};
	/**
	 * @brief get the listing length (in lines)
	 */
	virtual uint getLength() const = 0;
	/**
	 * @brief return a specific line from a listing
	 * @param n line number
	 */
	virtual Common::String getLine(uint n) = 0;
	/**
	 * @brief shorthand to get a lump of lines instead of calling getLine a number of times
	 * Generally you won't need to redefine these
	 */
	virtual Common::Array<ListingLine> getLines(uint centre, uint before, uint after);
	virtual Common::Array<ListingLine> getLines(uint beginning, uint end);
};

} // End of namespace Wintermute

#endif /* LISTING_H_ */
