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

#include "common/archive.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/xsheetfile.h"

namespace Nancy {

static const uint kXSheetFrameCountOffset = 0x22;
static const uint kXSheetNameSize = 33;
static const uint kXSheetCelRecordSize = 140;

bool XSheetFile::load(const Common::String &name) {
	Common::SeekableReadStream *xsheet = SearchMan.createReadStreamForMember(Common::Path(name));
	if (!xsheet) {
		warning("Could not open XSheet file %s", name.c_str());
		return false;
	}

	const uint16 signatureLength = g_nancy->getGameType() <= kGameTypeNancy11 ? 18 : 22;
	const Common::String signature = xsheet->readString('\0', signatureLength);
	if (signature != "XSHEET WayneSikes" && signature != "XSHEET HerInteractive") {
		warning("XSheet %s has an unknown signature", name.c_str());
		delete xsheet;
		return false;
	}

	xsheet->seek(kXSheetFrameCountOffset);
	numFrames = xsheet->readUint16LE();
	uint numTrees = kMaxTrees;

	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		// Nancy13 moved the names of the trees into the XSheet, in slots for
		// as many trees as there can be
		numTrees = MIN<uint>(xsheet->readUint16LE(), kMaxTrees);

		treeNames.resize(numTrees);
		for (Common::String &treeName : treeNames) {
			readFilename(*xsheet, treeName);
		}

		xsheet->skip((kMaxTrees - numTrees) * kXSheetNameSize);

		if (g_nancy->getGameType() >= kGameTypeNancy15) {
			// Nancy20 lines the rectangles up on four bytes
			if (g_nancy->getGameType() >= kGameTypeNancy20) {
				xsheet->skip(2);
			}

			readRectArray(*xsheet, treeRects, kMaxTrees);

			// Nancy17 leaves the right and bottom edge out of the rect
			if (g_nancy->getGameType() >= kGameTypeNancy17) {
				for (Common::Rect &rect : treeRects) {
					if (!rect.isEmpty()) {
						--rect.right;
						--rect.bottom;
					}
				}
			}
		}

		frameTiming = xsheet->readUint32LE();
	} else {
		xsheet->skip(2);
		frameTiming = xsheet->readUint16LE();
		xsheet->skip(2);
	}

	if (g_nancy->getGameType() >= kGameTypeNancy16) {
		// Every frame has a movie frame for each of the slots
		movieFrames.resize(numTrees, Common::Array<int32>(numFrames));
		for (uint frame = 0; frame < numFrames; ++frame) {
			for (uint tree = 0; tree < kMaxTrees; ++tree) {
				const int32 movieFrame = xsheet->readSint32LE();
				if (tree < numTrees) {
					movieFrames[tree][frame] = movieFrame;
				}
			}

			xsheet->skip(8);
		}
	} else {
		celNames.resize(numTrees, Common::Array<Common::Path>(numFrames));
		for (uint frame = 0; frame < numFrames; ++frame) {
			for (uint tree = 0; tree < numTrees; ++tree) {
				readFilename(*xsheet, celNames[tree][frame]);
			}

			xsheet->skip(kXSheetCelRecordSize - numTrees * kXSheetNameSize);
		}
	}

	delete xsheet;
	return true;
}

} // End of namespace Nancy
