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

#ifndef NANCY_SCRIPT_V5_RECORDNODE_H
#define NANCY_SCRIPT_V5_RECORDNODE_H

#include "common/str.h"

namespace Nancy {
namespace Script {

// Nancy26 makes every record a node of a tree: it is enabled, visible and
// paused by its own say and by what the Transformers above it say, and can be
// see-through. Messages reach it by its names. Records of the other games
// have none of this.
struct RecordNodeState {
	bool localEnabled = true;
	bool localVisible = true;
	bool localPaused = false;
	double localAlpha = 1.0;
	double drawnAlpha = 1.0;	// what the picture was last drawn with
	double keyZ = 0.0;
	int drawRank = -1;			// the place among the records of the scene

	Common::String localName;
	Common::String uniqueAlias;	// "AR<number>", which no other record has

	// "active" stands for enabled, visible and not paused
	bool isActive() const { return localEnabled && localVisible && !localPaused; }
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_RECORDNODE_H
