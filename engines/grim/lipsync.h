/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_LIPSYNC_H
#define GRIM_LIPSYNC_H

#include "common/str.h"

#include "engines/grim/object.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class LipSync : public Object {
public:
	LipSync(const Common::String &filename, Common::SeekableReadStream *data);
	~LipSync();

	int getAnim(int pos);
	bool isValid() { return _numEntries > 0; }
	const Common::String &getFilename() const { return _fname; };

private:
	struct LipEntry {
		uint16 frame;
		uint16 anim;
	};

	struct PhonemeAnim {
		uint16 phoneme;
		uint16 anim;
	};

	LipEntry *_entries;
	int _numEntries;

	static const PhonemeAnim _animTable[];
	static const int _animTableSize;
	Common::String _fname;
};

} // end of namespace Grim

#endif

