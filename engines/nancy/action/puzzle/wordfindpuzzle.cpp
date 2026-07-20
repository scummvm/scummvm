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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/wordfindpuzzle.h"

namespace Nancy {
namespace Action {

WordFindPuzzleData *WordFindPuzzle::getPuzzleData() const {
	return (WordFindPuzzleData *)NancySceneState.getPuzzleData(WordFindPuzzleData::getTag());
}

void WordFindPuzzle::readData(Common::SeekableReadStream &stream) {
	// 82-byte base header.
	_cursorType = stream.readUint16LE();	// 0x00
	stream.skip(4);							// 0x02
	_cellGapX = stream.readUint16LE();		// 0x06
	_cellGapY = stream.readUint16LE();		// 0x08
	stream.skip(0x20);						// 0x0a
	_solveScene.sceneID = stream.readUint16LE();	// 0x2a - shown once every word is found
	stream.skip(3);							// 0x2c
	readFilename(stream, _solutionImageName);	// 0x2f
	uint16 numWords = stream.readUint16LE();	// 0x50

	_words.resize(numWords);
	for (uint i = 0; i < numWords; ++i) {
		Word &w = _words[i];
		readFilename(stream, w.gridImageName);
		readFilename(stream, w.overlayImageName);
		readFilename(stream, w.animName);

		uint16 coordCount = stream.readUint16LE();
		w.answerCoords.resize(coordCount);
		for (uint16 j = 0; j < coordCount; ++j) {
			w.answerCoords[j].x = stream.readSint16LE();
			w.answerCoords[j].y = stream.readSint16LE();
		}

		w.eventFlagLabel = stream.readSint16LE();
		uint16 rectCount = stream.readUint16LE();
		readRectArray(stream, w.letterRects, rectCount);
	}

	// The shared 23-byte exit-hotspot record.
	int16 numZones = stream.readSint16LE();
	for (int16 i = 0; i < numZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 exitFlagLabel = stream.readSint16LE();
		byte exitFlagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			// The field after the scene id is a flag label (set on give-up), not a frame.
			_exitScene.frameID = 0;
			_exitFlag.label = exitFlagLabel;
			_exitFlag.flag = exitFlagValue;
		}
	}

	_sounds.resize(4);
	for (uint i = 0; i < 4; ++i) {
		_sounds[i].readData(stream);
	}
}

Common::Point WordFindPuzzle::gridCoord(const Common::Rect &rect) const {
	// The grid is anchored at the very first word's first letter (a global origin in the
	// original), with the pitch being a letter's size plus the header's gap.
	if (_words.empty() || _words[0].letterRects.empty()) {
		return Common::Point(0, 0);
	}

	// The original derives the pitch from the raw (inclusive) rect: cellGap + (right - left).
	// readRect() has already made our rects exclusive (++right/++bottom), so subtract that
	// back out, otherwise the pitch is one pixel too large and the integer division lands on
	// the wrong row/column for every cell past the origin.
	const Common::Rect &origin = _words[0].letterRects[0];
	int pitchX = _cellGapX + (origin.right - origin.left - 1);
	int pitchY = _cellGapY + (origin.bottom - origin.top - 1);
	if (pitchX <= 0) {
		pitchX = 1;
	}
	if (pitchY <= 0) {
		pitchY = 1;
	}

	return Common::Point((rect.left - origin.left) / pitchX, (rect.top - origin.top) / pitchY);
}

bool WordFindPuzzle::isAdjacent(const Common::Point &a, const Common::Point &b) const {
	int dx = ABS(a.x - b.x);
	int dy = ABS(a.y - b.y);
	return (dx + dy) == 1;
}

int WordFindPuzzle::letterAtCursor(const Common::Point &mousePos) const {
	if ((uint)_currentWord >= _words.size()) {
		return -1;
	}

	const Common::Array<Common::Rect> &rects = _words[_currentWord].letterRects;
	for (uint i = 0; i < rects.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(rects[i]).contains(mousePos)) {
			return (int)i;
		}
	}
	return -1;
}

int WordFindPuzzle::numSelected() const {
	int count = 0;
	for (uint i = 0; i < _selected.size(); ++i) {
		if (_selected[i]) {
			++count;
		}
	}
	return count;
}

bool WordFindPuzzle::chainMatchesAnswer() const {
	// A word is spelled correctly when every selected letter's grid cell is one of the
	// word's answer cells (and the counts already match).
	const Word &w = _words[_currentWord];
	for (uint i = 0; i < _selected.size(); ++i) {
		if (!_selected[i]) {
			continue;
		}

		Common::Point c = gridCoord(w.letterRects[i]);
		bool found = false;
		for (uint j = 0; j < w.answerCoords.size(); ++j) {
			if (w.answerCoords[j] == c) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}
	}
	return true;
}

void WordFindPuzzle::loadWord() {
	if ((uint)_currentWord >= _words.size()) {
		_allFound = true;
		return;
	}

	const Word &w = _words[_currentWord];
	_gridImage.free();
	_overlayImage.free();
	g_nancy->_resource->loadImage(w.gridImageName, _gridImage);
	if (!w.overlayImageName.empty()) {
		g_nancy->_resource->loadImage(w.overlayImageName, _overlayImage);
	}

	_selected.clear();
	_selected.resize(w.letterRects.size(), false);
}

void WordFindPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	// The active word carries over from earlier visits to this puzzle.
	WordFindPuzzleData *data = getPuzzleData();
	_currentWord = data ? data->currentWord : 0;
	if (_currentWord < 0) {
		_currentWord = 0;
	}

	_allFound = ((uint)_currentWord >= _words.size());
	if (!_allFound) {
		loadWord();
	}

	NancySceneState.setNoHeldItem();
	redraw();
	registerGraphics();
}

void WordFindPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	if (!_allFound && (uint)_currentWord < _words.size() && !_gridImage.empty()) {
		_drawSurface.blitFrom(_gridImage, Common::Point(0, 0));

		// Show the overlay art at each selected letter (the overlay image lines up with the
		// grid, so a selected cell's region is its highlighted version).
		const Word &w = _words[_currentWord];
		for (uint i = 0; i < _selected.size(); ++i) {
			if (_selected[i] && !_overlayImage.empty()) {
				const Common::Rect &r = w.letterRects[i];
				if (r.left >= 0 && r.top >= 0 && r.right <= _overlayImage.w && r.bottom <= _overlayImage.h) {
					_drawSurface.blitFrom(_overlayImage, r, Common::Point(r.left, r.top));
				}
			}
		}

		// While the "found" animation plays it is drawn on top of the grid.
		if (_playingMovie && _movie.isVideoLoaded()) {
			_movie.drawFrame(_drawSurface, Common::Point(0, 0));
		}
	}

	_needsRedraw = true;
}

void WordFindPuzzle::startFoundAnimation() {
	const Word &w = _words[_currentWord];
	if (!w.animName.empty() && _movie.loadFile(w.animName) && _movie.getFrameCount() > 0) {
		_movie.playRange(0, _movie.getFrameCount() - 1);
		_playingMovie = true;
		redraw();
	} else {
		// No animation available: just resolve the word.
		finishCurrentWord();
	}
}

void WordFindPuzzle::finishCurrentWord() {
	const Word &w = _words[_currentWord];
	if (w.eventFlagLabel != -1) {
		NancySceneState.setEventFlag(w.eventFlagLabel, g_nancy->_true);
	}

	++_currentWord;
	WordFindPuzzleData *data = getPuzzleData();
	if (data) {
		data->currentWord = _currentWord;
	}

	if ((uint)_currentWord >= _words.size()) {
		_allFound = true;
	} else {
		loadWord();
	}

	redraw();
}

void WordFindPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun: {
		if (_allFound) {
			_state = kActionTrigger;
			break;
		}

		// While the found-word animation is playing, no input is taken; when it ends the
		// word's event flag is set and the puzzle advances.
		if (_playingMovie) {
			if (_movie.update()) {
				_movie.drawFrame(_drawSurface, Common::Point(0, 0));
				_needsRedraw = true;
			}

			if (!_movie.isRangePlaying()) {
				_playingMovie = false;
				_movie.close();
				finishCurrentWord();
			}

			break;
		}

		const Word &w = _words[_currentWord];
		if (numSelected() == (int)w.answerCoords.size() && !w.answerCoords.empty()) {
			if (chainMatchesAnswer()) {
				startFoundAnimation();
			} else {
				// A wrong path: clear it so the player can try again.
				for (uint i = 0; i < _selected.size(); ++i) {
					_selected[i] = false;
				}
				redraw();
			}
		}

		break;
	}
	case kActionTrigger: {
		if (!_allFound) {
			NancySceneState.setEventFlag(_exitFlag);
		}
		const SceneChangeDescription &sc = _allFound ? _solveScene : _exitScene;
		if (sc.sceneID != kNoScene) {
			NancySceneState.changeScene(sc);
		}

		finishExecution();
		break;
	}
	}
}

void WordFindPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _allFound || _playingMovie) {
		return;
	}

	int letter = letterAtCursor(input.mousePos);
	if (letter >= 0) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_cursorType, true);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			const Word &w = _words[_currentWord];
			Common::Point c = gridCoord(w.letterRects[letter]);

			if (_selected[letter]) {
				// The clicked letter is always removed; removing one from the middle of the chain
				// (adjacent to 2+ selected letters) breaks it, so the whole selection is reset.
				int neighbors = 0;
				for (uint i = 0; i < _selected.size(); ++i) {
					if (_selected[i] && i != (uint)letter && isAdjacent(c, gridCoord(w.letterRects[i]))) {
						++neighbors;
					}
				}
				_selected[letter] = false;
				if (neighbors >= 2) {
					for (uint i = 0; i < _selected.size(); ++i) {
						_selected[i] = false;
					}
				}
			} else {
				// Extend the chain: valid as the first letter, or next to at least one already-
				// selected letter (a winding word can touch two of them).
				int neighbors = 0;
				for (uint i = 0; i < _selected.size(); ++i) {
					if (_selected[i] && isAdjacent(c, gridCoord(w.letterRects[i]))) {
						++neighbors;
					}
				}
				if (numSelected() == 0 || neighbors >= 1) {
					_selected[letter] = true;
					if (!_sounds.empty()) {
						SoundDescription desc;
						desc.name = _sounds[0].names.empty() ? "" : _sounds[0].names[0];
						desc.channelID = _sounds[0].channel;
						desc.numLoops = 1;
						desc.volume = _sounds[0].volume;
						if (!desc.name.empty() && desc.name != "NO SOUND") {
							g_nancy->_sound->loadSound(desc);
							g_nancy->_sound->playSound(desc);
						}
					}
				}
			}

			redraw();
		}

		input.eatMouseInput();
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
