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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/dotconnectpuzzle.h"

namespace Nancy {
namespace Action {

void DotConnectPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	for (int i = 0; i < kNumDots; ++i)
		readRect(stream, _dotSrcRects[i]);
	for (int i = 0; i < kNumDots; ++i)
		readRect(stream, _dotHighlightSrcRects[i]);

	_lineColorR = stream.readByte();
	_lineColorG = stream.readByte();
	_lineColorB = stream.readByte();

	_lineThickness = stream.readSint16LE();

	for (int i = 0; i < kNumEdges; ++i) {
		_solution[i].a = stream.readSint32LE();
		_solution[i].b = stream.readSint32LE();
	}

	_clickSound.readNormal(stream);
	_firstLineHint.readNormal(stream);
	_startHint.readNormal(stream);
	_tooManyLinesSound.readNormal(stream);
	_allCoveredSound.readNormal(stream);

	_winScene.readData(stream);
	stream.skip(2);
	_winFlag.label = stream.readSint16LE();
	_winFlag.flag  = stream.readByte();
	_winDelaySec = stream.readUint16LE();
	_winSound.readNormal(stream);

	_exitScene.readData(stream);
	stream.skip(2);
	_exitFlag.label = stream.readSint16LE();
	_exitFlag.flag  = stream.readByte();

	readRect(stream, _exitHotspot);
}

void DotConnectPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	for (int i = 0; i < kNumDots; ++i)
		_isActiveDot[i] = false;
	for (int i = 0; i < kNumEdges; ++i) {
		if (_solution[i].a >= 0 && _solution[i].a < kNumDots)
			_isActiveDot[_solution[i].a] = true;
		if (_solution[i].b >= 0 && _solution[i].b < kNumDots)
			_isActiveDot[_solution[i].b] = true;
	}

	_drawn.clear();
	_currentTip = -1;
	_subState = kPlaying;
	_firstLineHintPlayed = false;
	_startHintPlayed = false;
	_allCoveredPlayed = false;
	_tooManyPlayed = false;

	redraw();
}

void DotConnectPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through

	case kRun:
		switch (_subState) {
		case kPlaying:
			break;
		case kWaitWinDelay:
			if (g_system->getMillis() >= _winDelayEndTime) {
				if (_winSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_winSound);
					g_nancy->_sound->playSound(_winSound);
					_subState = kWaitWinSound;
				} else {
					_subState = kExitToWin;
				}
			}
			break;
		case kPlayWinSound:
			_subState = kWaitWinSound;
			break;
		case kWaitWinSound:
			if (!g_nancy->_sound->isSoundPlaying(_winSound)) {
				g_nancy->_sound->stopSound(_winSound);
				_subState = kExitToWin;
			}
			break;
		case kExitToWin:
		case kExitToCancel:
			_state = kActionTrigger;
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_clickSound);
		g_nancy->_sound->stopSound(_firstLineHint);
		g_nancy->_sound->stopSound(_startHint);
		g_nancy->_sound->stopSound(_tooManyLinesSound);
		g_nancy->_sound->stopSound(_allCoveredSound);
		g_nancy->_sound->stopSound(_winSound);
		if (_subState == kExitToWin) {
			if (_winFlag.label != -1)
				NancySceneState.setEventFlag(_winFlag);
			if (_winScene.sceneID != kNoScene)
				NancySceneState.changeScene(_winScene);
		} else {
			if (_exitFlag.label != -1)
				NancySceneState.setEventFlag(_exitFlag);
			if (_exitScene.sceneID != kNoScene)
				NancySceneState.changeScene(_exitScene);
		}
		finishExecution();
		break;
	}
}

void DotConnectPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _subState != kPlaying)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	if (!_exitHotspot.isEmpty() && _exitHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_subState = kExitToCancel;
		}
		return;
	}

	for (int i = 0; i < kNumDots; ++i) {
		if (_dotSrcRects[i].isEmpty() || !_dotSrcRects[i].contains(mouseVP))
			continue;
		if (dotAlreadyUsed(i))
			continue;
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp)
			onDotClicked(i);
		return;
	}
}

// The current tip stays clickable so it can act as an undo handle.
bool DotConnectPuzzle::dotAlreadyUsed(int dot) const {
	if (dot == _currentTip)
		return false;
	for (uint i = 0; i < _drawn.size(); ++i) {
		if (_drawn[i].a == dot || _drawn[i].b == dot)
			return true;
	}
	return false;
}

void DotConnectPuzzle::onDotClicked(int dot) {
	if (_clickSound.name != "NO SOUND") {
		g_nancy->_sound->loadSound(_clickSound);
		g_nancy->_sound->playSound(_clickSound);
	}

	if (!_startHintPlayed) {
		_startHintPlayed = true;
		if (_startHint.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_startHint);
			g_nancy->_sound->playSound(_startHint);
		}
	}

	if (_currentTip == -1) {
		_currentTip = (int16)dot;
		redraw();
		return;
	}

	if (dot == _currentTip) {
		if (_drawn.empty()) {
			_currentTip = -1;
		} else {
			const Edge &last = _drawn.back();
			int16 other = (last.a == _currentTip) ? (int16)last.b : (int16)last.a;
			_drawn.pop_back();
			_currentTip = other;
		}
		redraw();
		return;
	}

	Edge e;
	e.a = _currentTip;
	e.b = dot;
	_drawn.push_back(e);
	_currentTip = (int16)dot;

	if (_drawn.size() == 1 && !_firstLineHintPlayed) {
		const Edge &d = _drawn[0];
		bool matchesFirst = (d.a == _solution[0].a && d.b == _solution[0].b);
		bool matchesLastReversed = (d.a == _solution[kNumEdges - 1].b &&
		                            d.b == _solution[kNumEdges - 1].a);
		if (matchesFirst || matchesLastReversed) {
			_firstLineHintPlayed = true;
			if (_firstLineHint.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_firstLineHint);
				g_nancy->_sound->playSound(_firstLineHint);
			}
		}
	}

	if (_drawn.size() == 16 && !_tooManyPlayed) {
		_tooManyPlayed = true;
		if (_tooManyLinesSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_tooManyLinesSound);
			g_nancy->_sound->playSound(_tooManyLinesSound);
		}
	}

	checkWin();
	redraw();
}

void DotConnectPuzzle::checkWin() {
	if ((int)_drawn.size() != kNumEdges)
		return;

	bool allMatch = true;
	for (int i = 0; i < kNumEdges; ++i) {
		const Edge &d = _drawn[i];
		const Edge &fwd = _solution[i];
		const Edge &rev = _solution[kNumEdges - 1 - i];
		bool matchFwd = (d.a == fwd.a && d.b == fwd.b);
		bool matchRev = (d.a == rev.b && d.b == rev.a);
		if (!matchFwd && !matchRev) {
			allMatch = false;
			break;
		}
	}

	if (allMatch) {
		_subState = kWaitWinDelay;
		_winDelayEndTime = g_system->getMillis() + (uint32)_winDelaySec * 1000;
		return;
	}

	if (!_allCoveredPlayed) {
		bool covered[kNumDots] = {};
		for (uint i = 0; i < _drawn.size(); ++i) {
			if (_drawn[i].a >= 0 && _drawn[i].a < kNumDots) covered[_drawn[i].a] = true;
			if (_drawn[i].b >= 0 && _drawn[i].b < kNumDots) covered[_drawn[i].b] = true;
		}
		for (int i = 0; i < kNumDots; ++i) {
			if (_isActiveDot[i] && !covered[i])
				return;
		}
		_allCoveredPlayed = true;
		if (_allCoveredSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_allCoveredSound);
			g_nancy->_sound->playSound(_allCoveredSound);
		}
	}
}

void DotConnectPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// The symbols themselves live on the underlying scene background; the
	// _dotSrcRects values are screen positions (not coordinates inside the
	// overlay image), so the puzzle never blits them from the overlay.

	uint32 color = _drawSurface.format.RGBToColor(_lineColorR, _lineColorG, _lineColorB);
	int thick = MAX<int>(1, _lineThickness);
	for (uint i = 0; i < _drawn.size(); ++i) {
		const Edge &e = _drawn[i];
		if (e.a < 0 || e.a >= kNumDots || e.b < 0 || e.b >= kNumDots)
			continue;
		Common::Point ca((_dotSrcRects[e.a].left + _dotSrcRects[e.a].right) / 2,
		                 (_dotSrcRects[e.a].top  + _dotSrcRects[e.a].bottom) / 2);
		Common::Point cb((_dotSrcRects[e.b].left + _dotSrcRects[e.b].right) / 2,
		                 (_dotSrcRects[e.b].top  + _dotSrcRects[e.b].bottom) / 2);
		for (int dy = -thick; dy <= thick; ++dy) {
			for (int dx = -thick; dx <= thick; ++dx) {
				_drawSurface.drawLine(ca.x + dx, ca.y + dy, cb.x + dx, cb.y + dy, color);
			}
		}
	}

	// Punch line pixels back out at every dot's screen rect so the scene's
	// pre-rendered symbols remain visible through any line passing over them.
	for (int i = 0; i < kNumDots; ++i) {
		if (!_dotSrcRects[i].isEmpty())
			_drawSurface.fillRect(_dotSrcRects[i], _drawSurface.getTransparentColor());
	}

	// Overlay the active tip with its highlight sprite from the overlay image.
	if (_currentTip >= 0 && _currentTip < kNumDots &&
	    !_dotHighlightSrcRects[_currentTip].isEmpty()) {
		const Common::Rect &dst = _dotSrcRects[_currentTip];
		_drawSurface.blitFrom(_image, _dotHighlightSrcRects[_currentTip],
			Common::Point(dst.left, dst.top));
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
