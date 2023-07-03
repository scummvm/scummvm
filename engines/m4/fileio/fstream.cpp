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

#include "m4/fileio/fstream.h"
#include "m4/vars.h"

namespace M4 {

bool f_stream_Init() {
	_G(firstStream) = nullptr;
	_G(lastStream) = nullptr;
	return true;
}

StreamFile::StreamFile(const Common::String &filename) {
	if (!_file.open(filename))
		error("Could not open - %s", filename.c_str());
}

int32 StreamFile::read(Handle bufferHandle, int32 n) {
	return _file.read(bufferHandle, n);
}

bool StreamFile::seek(uint32 n) {
	return _file.seek(n);
}

bool StreamFile::seek_ahead(uint32 n) {
	return _file.skip(n);
}

uint32 StreamFile::get_pos() {
	return _file.pos();
}

} // namespace M4
