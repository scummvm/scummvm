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

#ifndef DUNE_DETECTION_H
#define DUNE_DETECTION_H

#include "engines/advancedDetector.h"

namespace Dune {

// A channel is a bit of its own, so that any of them can be asked for on
// their own.
enum DuneDebugChannels {
	kDebugResource = 1 << 0,
	kDebugGraphics = 1 << 1,
	kDebugScript   = 1 << 2,
	kDebugSound    = 1 << 3,
	kDebugVideo    = 1 << 4
};

// The game data either sits loose in the game directory (floppy release) or
// inside the DUNE.DAT container (CD release).  Both hold the same resources.
enum DuneGameFlags {
	GF_CD = 1 << 0
};

extern const ADGameDescription gameDescriptions[];

} // End of namespace Dune

#endif // DUNE_DETECTION_H
