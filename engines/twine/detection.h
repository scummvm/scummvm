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

#ifndef TWINE_DETECTION_H
#define TWINE_DETECTION_H

namespace TwinE {

enum TwineGameType {
	GType_LBA = 1,
	GType_LBA2 = 2,
	// slideshow demo of lba1
	GType_LBASHOW = 3
};

enum TwineFeatureFlags {
	TF_VERSION_EUROPE = (1 << 0),
	TF_VERSION_USA = (1 << 1),
	TF_VERSION_CUSTOM = (1 << 2),
	TF_USE_GIF = (1 << 3),
	TF_DOTEMU_ENHANCED = (1 << 4),
	TF_MOD = (1 << 5)
};

} // End of namespace TwinE

#endif // TWINE_DETECTION_H
