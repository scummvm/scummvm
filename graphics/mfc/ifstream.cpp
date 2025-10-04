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

#include "common/file.h"
#include "graphics/mfc/ifstream.h"
#include "graphics/mfc/global_functions.h"

namespace Graphics {
namespace MFC {

ifstream::ifstream() {
}

ifstream::ifstream(const char *filename, ios::openmode mode) {
	open(filename, mode);
}

void ifstream::open(const char *filename, ios::openmode mode) {
	_file = OpenFile(filename);
	if (!_file)
		error("Could not open - %s", filename);
}

bool ifstream::is_open() const {
	return _file;
}

void ifstream::close() {
	delete _file;
	_file = nullptr;
}

ifstream &ifstream::getline(char *buffer, size_t count) {
	char c;
	_cCount = 0;

	while (!_file->eos()) {
		c = _file->readByte();

		// Check for end of line characters
		if (c == '\n')
			break;
		if (c == '\r') {
			c = _file->readByte();
			assert(c == '\n');
			break;
		}

		*buffer++ = c;
		++_cCount;
	}

	*buffer = '\0';
	return *this;
}

ifstream &ifstream::read(char *buffer, size_t count) {
	_cCount = _file->read(buffer, count);
	return *this;
}

size_t ifstream::gcount() const {
	return _cCount;
}

bool ifstream::good() const {
	return !_file->err();
}

bool ifstream::eof() const {
	return _file->eos();
}

bool ifstream::ifstream::fail() const {
	return _file->err();
}

bool ifstream::ifstream::bad() const {
	return _file->err();
}

size_t ifstream::tellg() {
	return _file->pos();
}

ifstream &ifstream::seekg(size_t pos) {
	_file->seek(pos);
	return *this;
}
ifstream &ifstream::seekg(int32 off, int dir) {
	_file->seek(off, dir);
	return *this;
}

} // namespace MFC
} // namespace Graphics
