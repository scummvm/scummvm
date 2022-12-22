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

#ifndef GOB_RXYFILE_H
#define GOB_RXYFILE_H

#include "common/system.h"
#include "common/array.h"

namespace Common {
	class SeekableReadStream;
	class SeekableReadStreamEndian;
}

namespace Gob {

/** A RXY file, containing relative sprite coordinates.
 *
 *  Used in hardcoded "actiony" parts of gob games.
 */
class RXYFile {
public:
	struct Coordinates {
		uint16 left;
		uint16 top;
		uint16 right;
		uint16 bottom;
	};

	RXYFile(Common::SeekableReadStream &rxy);
	RXYFile(Common::SeekableReadStreamEndian &rxy);
	RXYFile(uint16 width, uint16 height);
	~RXYFile();

	uint size() const;

	uint16 getWidth () const;
	uint16 getHeight() const;

	uint16 getRealCount() const;

	const Coordinates &operator[](uint i) const;

	uint16 add(uint16 left, uint16 top, uint16 right, uint16 bottom);

private:
	typedef Common::Array<Coordinates> CoordArray;

	CoordArray _coords;

	uint16 _realCount;

	uint16 _width;
	uint16 _height;


	void load(Common::SeekableReadStreamEndian &rxy);
};

} // End of namespace Gob

#endif // GOB_RXYFILE_H
