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

#include "listing.h"
#include "common/array.h"

namespace Wintermute {

Common::Array<ListingLine> Listing::getLines(uint begin, uint end) {
	assert(begin <= end);
	Common::Array<ListingLine> ret;
	for (uint i = begin; i <= end; i++) {
		ListingLine listingline;
		listingline.number = i;
		listingline.text = getLine(i);
		ret.push_back(listingline);
	}
	return ret;
}

Common::Array<ListingLine> Listing::getLines(uint centre, uint before, uint after) {
	uint begin = MAX(centre - before, (uint)1); // Line numbers start from 1
	uint end = MIN(centre + after, (uint)(getLength() - 1)); // Line numbers start from 1
	return getLines(begin, end);
}

} // End of namespace Wintermute
