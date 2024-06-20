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

#include "qdengine/core/qd_precomp.h"


namespace QDEngine {

XStream::XStream(bool handleErrors) {
	handleErrors_ = handleErrors;
	handler  = INVALID_HANDLE_VALUE;
	eofFlag  = 1;
	radix    = XS_DEFRADIX;
	digits   = XS_DEFDIGITS;
	extSize  = -1;
	extPos   = 0;
	ioError_ = false;
}

XStream::~XStream() {
	close();
}

XStream::XStream(const char* name, unsigned flags, bool handleErrors) {
	handleErrors_ = handleErrors;
	handler  = INVALID_HANDLE_VALUE;
	eofFlag  = 1;
	radix    = XS_DEFRADIX;
	digits   = XS_DEFDIGITS;
	extSize  = -1;
	extPos   = 0;
	ioError_ = false;
	open(name, flags);
}
} // namespace QDEngine
