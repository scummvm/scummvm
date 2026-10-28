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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"

#include "engines/nancy/script/xsheet.h"

namespace Nancy {
namespace Script {

// The parts of an XSheet
static const uint kXSheetFrameCountOffset = 0x22;
static const uint kXSheetMaxTrees = 4;

bool XSheet::load(const Common::String &name) {
	clear();

	Common::SeekableReadStream *xsheet = SearchMan.createReadStreamForMember(Common::Path(name));
	if (!xsheet) {
		warning("Could not open XSheet file %s", name.c_str());
		return false;
	}

	xsheet->seek(kXSheetFrameCountOffset);
	_numFrames = xsheet->readUint16LE();
	const uint numTrees = MIN<uint>(xsheet->readUint16LE(), kXSheetMaxTrees);

	Common::Array<Common::String> treeNames(kXSheetMaxTrees);
	for (Common::String &treeName : treeNames) {
		readFilename(*xsheet, treeName);
	}

	// Nancy20 lines the rectangles up on four bytes
	if (g_nancy->getGameType() >= kGameTypeNancy20) {
		xsheet->skip(2);
	}

	// Where each part goes in the viewport; the right and bottom edge are left out
	Common::Array<Common::Rect> treeRects;
	readRectArray(*xsheet, treeRects, kXSheetMaxTrees);

	_framesPerSecond = xsheet->readUint32LE();

	for (uint i = 0; i < numTrees; ++i) {
		Tree *tree = new Tree();
		tree->frames.resize(_numFrames);
		_trees.push_back(tree);
	}

	// Every frame has a movie frame for each of the parts there can be
	for (uint frame = 0; frame < _numFrames; ++frame) {
		for (uint i = 0; i < kXSheetMaxTrees; ++i) {
			const int32 movieFrame = xsheet->readSint32LE();
			if (i < numTrees) {
				_trees[i]->frames[frame] = movieFrame;
			}
		}

		xsheet->skip(8);
	}

	delete xsheet;

	_name = name;
	_frame = 0;

	for (uint i = 0; i < numTrees; ++i) {
		Tree &tree = *_trees[i];
		Common::Rect dest = treeRects[i];
		if (!dest.isEmpty()) {
			--dest.right;
			--dest.bottom;
		}

		if (dest.isEmpty() || !tree.player.loadFile(Common::Path(treeNames[i]))) {
			warning("Could not load conversation movie %s of XSheet %s", treeNames[i].c_str(), name.c_str());
			clear();
			return false;
		}

		// The movies come with an alpha plane, which they are drawn by
		tree.dest = dest;
		tree.graphic.init();
		tree.graphic.moveTo(dest);
		tree.graphic.setTransparent(false);
		tree.graphic.setVisible(false);
		tree.graphic.registerGraphics();

		showMovieFrame(tree, (tree.frames.empty() || isTreeFreeRunning(tree)) ? 0 : tree.frames[0]);
	}

	return true;
}

void XSheet::clear() {
	for (Tree *tree : _trees) {
		delete tree;
	}

	_trees.clear();
	_name.clear();
	_numFrames = 0;
	_framesPerSecond = 0;
	_frame = 0;
}

bool XSheet::isTreeFreeRunning(const Tree &tree) const {
	return _frame < tree.frames.size() && tree.frames[_frame] == kFreeRunning;
}

// The movies are padded to a multiple of 16 pixels; the picture itself is as
// large as the place it goes to
void XSheet::drawTree(Tree &tree, const Graphics::Surface &frame) {
	const Common::Rect picture(MIN<int>(frame.w, tree.dest.width()), MIN<int>(frame.h, tree.dest.height()));
	GraphicsManager::copyToManaged(frame.getSubArea(picture), tree.graphic._drawSurface);
	tree.graphic.setNeedsRedraw(true);
}

void XSheet::showMovieFrame(Tree &tree, int32 movieFrame) {
	if (movieFrame == tree.shownFrame) {
		return;
	}

	const Graphics::Surface *frame = tree.player.decodeNextFrame(movieFrame);
	if (frame) {
		drawTree(tree, *frame);
		tree.shownFrame = movieFrame;
	}
}

void XSheet::showFrame(uint frame) {
	if (frame == _frame) {
		return;
	}

	_frame = frame;

	for (Tree *tree : _trees) {
		if (frame < tree->frames.size() && tree->frames[frame] != kFreeRunning) {
			showMovieFrame(*tree, tree->frames[frame]);
		}
	}
}

void XSheet::startFreeRunning() {
	for (Tree *tree : _trees) {
		// The parts that play by themselves keep their own time
		if (isTreeFreeRunning(*tree)) {
			tree->player.start();
		}
	}
}

void XSheet::updateFreeRunning() {
	for (Tree *tree : _trees) {
		if (isTreeFreeRunning(*tree) && tree->player.needsUpdate()) {
			const Graphics::Surface *movieFrame = tree->player.decodeNextFrame();
			if (movieFrame) {
				drawTree(*tree, *movieFrame);
			}
		}
	}
}

bool XSheet::isFreeRunningDone() const {
	for (const Tree *tree : _trees) {
		if (isTreeFreeRunning(*tree) && !tree->player.endOfVideo()) {
			return false;
		}
	}

	return true;
}

void XSheet::parkFreeRunning() {
	for (Tree *tree : _trees) {
		if (isTreeFreeRunning(*tree) && tree->player.getFrameCount() > 0) {
			const Graphics::Surface *movieFrame = tree->player.decodeNextFrame(tree->player.getFrameCount() - 1);
			if (movieFrame) {
				drawTree(*tree, *movieFrame);
			}
		}
	}
}

void XSheet::setVisible(bool isVisible) {
	for (Tree *tree : _trees) {
		tree->graphic.setVisible(isVisible);
	}
}

void XSheet::setZ(double z, const SceneContext &context) {
	for (uint i = 0; i < _trees.size(); ++i) {
		_trees[i]->graphic.setScriptZ(z + (i == 0 ? 1 : 0), context);
	}
}

} // End of namespace Script
} // End of namespace Nancy
