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

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
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

// Xsheet

void XsheetRecord::readTable(int tableIndex) {
	lua_State *L = getLua();
	lua_getfield(L, tableIndex, "xsheet");
	if (!lua_isstring(L, -1)) {
		luaL_error(L, "Field %s must be a string", "xsheet");
	}

	_xsheetName = lua_tostring(L, -1);
	lua_pop(L, 1);

	load();
}

// An XSheet is only loaded when it is another one, and there is none to load
// for an empty name
void XsheetRecord::load() {
	if (_xsheetName.empty() || _xsheetName.equalsIgnoreCase(_xsheet.getName())) {
		return;
	}

	if (!_xsheet.load(_xsheetName)) {
		return;
	}

	// The counter keeps its time, and goes at the pace of the new XSheet
	_framesPerSecond = _xsheet.getFramesPerSecond();
	if (_framesPerSecond > 0.0) {
		_frame = MAX<int>(0, (int)(_time * _framesPerSecond));
		if (_lastFrame > 0) {
			_hasEnded = _frame > _lastFrame;
			_frame = MIN(_frame, _lastFrame);
		}
	}

	const int lastFrame = (int)_xsheet.getNumFrames() - 1;
	if (lastFrame > 0) {
		_lastFrame = lastFrame;
		setFrame(MIN(_frame, _lastFrame));
	} else {
		_lastFrame = 0;
	}

	onZChanged();
	updateGraphics();
}

void XsheetRecord::setFrame(int frame) {
	if (frame == _frame || _framesPerSecond == 0.0) {
		return;
	}

	_frame = MAX(frame, 0);
	if (_lastFrame > 0) {
		_frame = MIN(frame, _lastFrame);
		_hasEnded = _frame == _lastFrame;
	}

	_time = _frame / _framesPerSecond;
}

void XsheetRecord::advance(double seconds) {
	_time += seconds;
	if (_framesPerSecond == 0.0) {
		return;
	}

	_frame = MAX<int>(0, (int)(_time * _framesPerSecond));
	if (_lastFrame > 0) {
		_hasEnded = _frame > _lastFrame;
		_frame = MIN(_frame, _lastFrame);
	}
}

void XsheetRecord::run(double sceneTime, double elapsedTime) {
	if (!_isPaused) {
		advance(elapsedTime);
	}

	_xsheet.showFrame((uint)_frame);

	if (_hasEnded && !_isPaused) {
		_isPaused = true;
		callFunction("OnPause");
	}
}

// Restarting goes back to the first frame, and loads an XSheet that was handed
// over in the meantime
bool XsheetRecord::prepareReset() {
	setFrame(0);
	_xsheet.showFrame(0);
	_isPaused = false;
	load();
	return true;
}

bool XsheetRecord::skipSpeech() {
	if (!isActive() || !hasStarted() || _isPaused || _lastFrame <= 0 || !isVoicePlaying()) {
		return false;
	}

	setFrame(_lastFrame);
	_xsheet.showFrame((uint)_frame);
	return true;
}

void XsheetRecord::onZChanged() {
	_xsheet.setZ(getZ(), getContext());
}

void XsheetRecord::updateGraphics() {
	_xsheet.setVisible(isActive() && hasStarted());
}

bool XsheetRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "paused") {
		lua_pushboolean(L, _isPaused);
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "lastFrame") {
		// One before the last frame of the counter, as in the original
		lua_pushinteger(L, _lastFrame - 1);
	} else if (key == "xsheet") {
		lua_pushstring(L, _xsheetName.c_str());
	} else {
		return false;
	}

	return true;
}

bool XsheetRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "paused") {
		_isPaused = lua_toboolean(L, 3) != 0;
	} else if (key == "frame") {
		setFrame((int)luaL_checkinteger(L, 3));
		_xsheet.showFrame((uint)_frame);
	} else if (key == "xsheet") {
		_xsheetName = luaL_checkstring(L, 3);
		if (!hasRunOnce()) {
			load();
		}
	} else {
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
