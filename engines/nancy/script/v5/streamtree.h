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

#ifndef NANCY_SCRIPT_V5_STREAMTREE_H
#define NANCY_SCRIPT_V5_STREAMTREE_H

#include "common/scummsys.h"

namespace Nancy {
namespace Script {

// Nancy26: a stream is a node above the records of its scenes. Messages to its
// name enable, disable, pause and lock it, which its records take on.
//
// The streams of the viewport and the ones of the UI make two trees, the UI on
// top. In each, streams get drawn in the order of their z, the ones begun later
// in front of the ones with the same z.
struct StreamNodeState {
	bool isEnabled = true;
	bool isVisible = true;
	bool isPaused = false;

	bool isInViewport = true;
	double z = 0.0;
	uint32 sequence = 0;	// when it was begun

	// Where the stream comes in the order all the streams get drawn in, from
	// the bottom. Everything the stream draws goes in the range of z orders of
	// that place, so it ends up above the streams before it.
	int32 band = 0;
};

// Nancy26 draws every stream as a whole over the ones before it, so each gets a
// band of z orders of its own, with the background at the bottom. These give
// the z order of a script z, and of the background, in a band.
uint16 getBandZOrder(double z, int32 band);
uint16 getBandBackgroundZOrder(int32 band);

// The scene works out the place of each of its records in the order of the
// tree they make; this is the script z that keeps a record at its place
double getRankZ(int rank);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_STREAMTREE_H
