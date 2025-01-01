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

#ifndef MEDIASTATION_DEBUG_CHANNELS_H
#define MEDIASTATION_DEBUG_CHANNELS_H

// This is a convenience so we don't have to include
// two files in every translation unit where we want
// debugging support.
#include "common/debug.h"

namespace MediaStation {

// TODO: Finish comments that describe the various debug levels
enum DebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,

	// Level 5: Decompiled Script Lines
	// Level 7: Instruction Types & Opcodes
	kDebugScript,
	kDebugEvents,

	// Level 9: Individual Datums
	kDebugLoading
};

} // End of namespace MediaStation

#endif