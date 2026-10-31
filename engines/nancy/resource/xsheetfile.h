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

#ifndef NANCY_RESOURCE_XSHEETFILE_H
#define NANCY_RESOURCE_XSHEETFILE_H

#include "common/array.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/str.h"

namespace Nancy {

// An XSheet puts a character in a conversation together from parts, its trees:
// cels, or movies from Nancy15 on. For each of its frames, it names the cel or
// the movie frame that every tree shows.
struct XSheetFile {
	static const uint kMaxTrees = 4;

	// Nancy13 and up; earlier games name the trees in their conversations
	Common::Array<Common::String> treeNames;

	// Nancy15 and up: where the trees that are movies go, one for each slot
	Common::Array<Common::Rect> treeRects;

	// Milliseconds a frame up to Nancy18, frames a second from Nancy19 on
	uint32 frameTiming = 0;

	uint numFrames = 0;

	// For each tree, the cel of every frame before Nancy16, and the movie frame
	// of every frame from Nancy16 on
	Common::Array<Common::Array<Common::Path>> celNames;
	Common::Array<Common::Array<int32>> movieFrames;

	bool load(const Common::String &name);
};

} // End of namespace Nancy

#endif // NANCY_RESOURCE_XSHEETFILE_H
