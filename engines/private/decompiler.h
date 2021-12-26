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
// Heavily based on code by jdieguez

#ifndef PRIVATE_DECOMPILER_H
#define PRIVATE_DECOMPILER_H

#include "common/array.h"
#include "common/debug.h"
#include "common/str.h"

namespace Private {

class Decompiler {
public:
	Decompiler(char *buf, uint32 fileSize, bool mac = false);
	Common::String getResult() const;

private:
	void decompile(Common::Array<byte> &buffer, bool mac);
	Common::String _result;
};

} // namespace Private

#endif
