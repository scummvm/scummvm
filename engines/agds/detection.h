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

#ifndef DETECTION_H
#define DETECTION_H

namespace AGDS {

enum AGDSGameFlag {
	AGDS_2511 = (1 << 0),
	AGDS_2299 = (1 << 1),
};

static constexpr int kAGDSVersionDemo2283 = 2283;
static constexpr int kAGDSVersionBlackMirror2296 = 2296;
static constexpr int kAGDSVersionBlackMirror2299 = 2299;
static constexpr int kAGDSVersionNibiru2511 = 2511;

} // End of namespace AGDS

#endif // DETECTION_H
