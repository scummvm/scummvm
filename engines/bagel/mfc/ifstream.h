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

#ifndef BAGEL_MFC_IFSTREAM_H
#define BAGEL_MFC_IFSTREAM_H

#include "common/file.h"

namespace Bagel {
namespace MFC {

namespace ios {
enum openmode {
	in = 1 << 0,
	binary = 1 << 1
};
} // namespace ios

class ifstream {
private:
	Common::SeekableReadStream *_file = nullptr;
	size_t _cCount = 0;

public:
	ifstream();
	ifstream(const char *filename, ios::openmode mode = ios::in);
	virtual ~ifstream() {
		close();
	}

	virtual void open(const char *filename, ios::openmode mode = ios::in);
	virtual bool is_open() const;
	virtual void close();

	virtual ifstream &getline(char *buffer, size_t count);
	virtual ifstream &read(char *buffer, size_t count);
	virtual size_t gcount() const;

	virtual bool good() const;
	virtual bool eof() const;
	virtual bool fail() const;
	virtual bool bad() const;

	// Positioning
	virtual size_t tellg();
	virtual ifstream &seekg(size_t pos);
	virtual ifstream &seekg(int32 off, int dir);
};

} // namespace MFC
} // namespace Bagel

#endif
