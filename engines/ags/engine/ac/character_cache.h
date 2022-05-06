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

#ifndef AGS_ENGINE_AC_CHARACTER_CACHE_H
#define AGS_ENGINE_AC_CHARACTER_CACHE_H

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

// stores cached info about the character
struct CharacterCache {
	Shared::Bitmap *image = nullptr;
	int sppic = 0;
	int scaling = 0;
	int inUse = 0;
	short tintredwas = 0, tintgrnwas = 0, tintbluwas = 0, tintamntwas = 0;
	short lightlevwas = 0, tintlightwas = 0;
	// no mirroredWas is required, since the code inverts the sprite number
};

} // namespace AGS3

#endif
