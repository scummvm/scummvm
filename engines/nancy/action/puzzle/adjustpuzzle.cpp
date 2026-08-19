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

#include "common/util.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/adjustpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void AdjustPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);		// 0x1c0
	_field3d = stream.readSint16LE();		// 0x3d

	int16 numPieces = stream.readSint16LE();
	_pieces.resize(numPieces);				// 0x3f
	for (int16 i = 0; i < numPieces; ++i) {
		Piece &piece = _pieces[i];
		for (int j = 0; j < 4; ++j) {
			readRect(stream, piece.rects[j]);
		}
		int16 numSubRects = stream.readSint16LE();
		readRectArray(stream, piece.subRects, numSubRects);
		piece.initialState = stream.readByte();
		readRect(stream, piece.boundRect);
	}

	_pieceSound.readData(stream);			// 0x4f

	readRect(stream, _rectA5);				// 0xa5
	readRect(stream, _rectB5);				// 0xb5

	_testSound.readData(stream);			// 0xc5

	readFilename(stream, _adjustName);		// 0x11b
	if (!_adjustName.empty()) {
		_hasAdjustRect = true;
		readRect(stream, _adjustRect);		// 0x11f
		_adjustSound.readData(stream);		// 0x12f
	}

	_field18a = stream.readSint16LE();		// 0x18a

	int16 numOverlays = stream.readSint16LE();
	_overlayNames.resize(numOverlays);		// 0x18c
	for (int16 i = 0; i < numOverlays; ++i) {
		readFilename(stream, _overlayNames[i]);
	}

	int16 numRows = stream.readSint16LE();
	_matrix.resize(numRows);				// 0x1b0
	for (int16 i = 0; i < numRows; ++i) {
		MatrixRow &row = _matrix[i];
		row.cols.resize(numPieces);
		for (int16 j = 0; j < numPieces; ++j) {
			row.cols[j] = stream.readSint16LE();
		}
		row.result = stream.readByte();
	}

	_defaultResult = stream.readByte();		// 0x1c4
	_perfectResult = stream.readByte();		// 0x1c5

	Outcome *outcomes[] = { &_winScene, &_loseScene };
	for (Outcome *outcome : outcomes) {
		outcome->sceneID = stream.readSint16LE();
		outcome->frameID = stream.readSint16LE();
		outcome->flag.label = stream.readSint16LE();
		outcome->flag.flag = stream.readByte();
		outcome->sound.readData(stream);
	}

	// Trailing count-prefixed array of 23-byte give-up hotspots
	// {Rect, uint16 cursorType, uint16 sceneID, int16 flagLabel, byte flagValue}.
	// The exit always jumps to the scene's first frame.
	int16 numExitZones = stream.readSint16LE();
	for (int16 i = 0; i < numExitZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 flagLabel = stream.readSint16LE();
		byte flagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			_exitScene.frameID = 0;
			_exitFlag.label = flagLabel;
			_exitFlag.flag = flagValue;
		}
	}
}

void AdjustPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	_overlayImages.resize(_overlayNames.size());
	for (uint i = 0; i < _overlayNames.size(); ++i) {
		if (!_overlayNames[i].empty()) {
			g_nancy->_resource->loadImage(_overlayNames[i], _overlayImages[i]);
			_overlayImages[i].setTransparentColor(_drawSurface.getTransparentColor());
		}
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i].state = _pieces[i].initialState;
	}

	_resultIndex = 0;
	_showResult = false;
	_solved = false;
	_lost = false;
	_outcomeApplied = false;

	redraw();
}

// Returns the piece whose +/- control is under the cursor, and sets delta to
// +1 (increment) or -1 (decrement). Returns -1 if none. rects[1]/rects[3] are the
// on-screen down/up arrow hotspots (rects[0]/rects[2] are the arrow sprites in the
// image sheet).
int AdjustPuzzle::pieceControlAtCursor(const Common::Point &mousePos, int &delta) const {
	for (uint i = 0; i < _pieces.size(); ++i) {
		const Piece &piece = _pieces[i];
		if (!piece.rects[3].isEmpty() &&
				NancySceneState.getViewport().convertViewportToScreen(piece.rects[3]).contains(mousePos)) {
			delta = 1;	// up arrow
			return (int)i;
		}
		if (!piece.rects[1].isEmpty() &&
				NancySceneState.getViewport().convertViewportToScreen(piece.rects[1]).contains(mousePos)) {
			delta = -1;	// down arrow
			return (int)i;
		}
	}
	return -1;
}

byte AdjustPuzzle::evaluateResult() const {
	for (uint r = 0; r < _matrix.size(); ++r) {
		const MatrixRow &row = _matrix[r];
		bool match = true;
		for (uint j = 0; j < _pieces.size() && j < row.cols.size(); ++j) {
			if (row.cols[j] != _pieces[j].state) {
				match = false;
				break;
			}
		}
		if (match) {
			return row.result;
		}
	}
	return _defaultResult;
}

void AdjustPuzzle::runTest() {
	_resultIndex = evaluateResult();
	_showResult = true;

	if (_resultIndex == _perfectResult) {
		_solved = true;
	} else if (_loseScene.sceneID != kNoScene) {
		// A lose scene is configured (kNoScene means "stay and let the player retry").
		_lost = true;
	}

	redraw();
}

void AdjustPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	if (_showResult && !_solved && _resultIndex >= 1 && (uint)(_resultIndex - 1) < _overlayImages.size()) {
		// Result screen: the matched result overlay (a full-frame _TXT overlay).
		const Graphics::ManagedSurface &overlay = _overlayImages[_resultIndex - 1];
		_drawSurface.blitFrom(overlay, Common::Rect(overlay.w, overlay.h), Common::Point(0, 0));
	} else {
		// Levels run 1..5; level 1 is the default (from the scene background), and
		// levels 2..5 show the sheet sprite subRects[level - 2] at the piece's
		// on-screen bounding rect.
		for (const Piece &piece : _pieces) {
			int idx = piece.state - 2;
			if (piece.state == 1 || idx < 0 || (uint)idx >= piece.subRects.size()) {
				continue;
			}
			_drawSurface.blitFrom(_image, piece.subRects[idx],
				Common::Point(piece.boundRect.left, piece.boundRect.top));
		}
	}

	_needsRedraw = true;
}

void AdjustPuzzle::applyOutcome(const Outcome &outcome) {
	SceneChangeDescription desc;
	desc.sceneID = outcome.sceneID;
	desc.frameID = outcome.frameID;
	NancySceneState.changeScene(desc);
	NancySceneState.setEventFlag(outcome.flag);
}

void AdjustPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved || _lost) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// Give-up hotspot: leave the puzzle.
	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		if (click) {
			_exitRequested = true;
		}
		input.eatMouseInput();
		return;
	}

	int delta = 0;
	int piece = pieceControlAtCursor(input.mousePos, delta);
	if (piece >= 0) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (click) {
			// Levels run from 1 (default) up to 1 + the number of level sprites.
			int maxState = 1 + (int)_pieces[piece].subRects.size();
			_pieces[piece].state = CLIP<int>(_pieces[piece].state + delta, 1, maxState);
			_showResult = false;
			redraw();
		}
		input.eatMouseInput();
		return;
	}

	// The "test" control: evaluate the current settings.
	if (!_rectB5.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_rectB5).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (click) {
			runTest();
		}
		input.eatMouseInput();
	}
}

void AdjustPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		break;
	case kRun:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			NancySceneState.changeScene(_exitScene);
			break;
		}
		if ((_solved || _lost) && !_outcomeApplied) {
			_outcomeApplied = true;
			applyOutcome(_solved ? _winScene : _loseScene);
		}
		break;
	default:
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
