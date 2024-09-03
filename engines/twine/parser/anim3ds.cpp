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

#include "twine/parser/anim3ds.h"
#include "common/debug.h"

namespace TwinE {

bool Anim3DSData::loadFromStream(Common::SeekableReadStream &stream, bool lba1) {
	assert(!lba1);

	const int n = (int)stream.size() / 8;
	debug("preload %i anim3ds entries", n);
	for (int i = 0; i < n; ++i) {
		T_ANIM_3DS anim;
		stream.read(anim.Name, sizeof(anim.Name));
		anim.Deb = stream.readSint16LE();
		anim.Fin = stream.readSint16LE();
		_anims.push_back(anim);
	}
	return !stream.err();
}

} // namespace TwinE
