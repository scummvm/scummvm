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

#ifndef NANCY_DETECTION_H
#define NANCY_DETECTION_H

#include "engines/advancedDetector.h"

namespace Nancy {

enum GameType {
	kGameTypeNone = 0,
	kGameTypeVampire = 1,
	kGameTypeNancy1 = 2,
	kGameTypeNancy2 = 3,
	kGameTypeNancy3 = 4,
	kGameTypeNancy4 = 5,
	kGameTypeNancy5 = 6,
	kGameTypeNancy6 = 7
};

enum NancyGameFlags {
	GF_COMPRESSED = 1 << 0
};

struct NancyGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

enum NancyDebugChannels {
	kDebugEngine		= 1 << 0,
	kDebugActionRecord	= 1 << 1,
	kDebugScene			= 1 << 2
};

} // End of namespace Nancy

#endif // NANCY_DETECTION_H
