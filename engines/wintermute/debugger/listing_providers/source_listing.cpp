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

#include "source_listing.h"

namespace Wintermute {

SourceListing::SourceListing(const Common::Array<Common::String> &strings) : _strings(strings) {}

SourceListing::~SourceListing() {}

uint SourceListing::getLength() const {
	return _strings.size();
}

Common::String SourceListing::getLine(uint n) {
	uint index = n - 1; // Line numbers start from 1, arrays from 0
	/*
	 * Clients should not ask for a line number that
	 * is not in the source file.
	 * 0 is undefined, n - 1 is undefined.
	 * It is easy for the client to check that n > 0
	 * and n < getLength(), so it should just not happen.
	 * We return '^', after vim, to misbehaving clients.
	 */
	if (n == 0) {
		return Common::String("^");
	}
	if (index < getLength()) {
		return _strings[index];
	} else {
		return Common::String("^");
	}
}

} // End of namespace Wintermute


