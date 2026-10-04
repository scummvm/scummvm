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
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/dragsurfacepuzzle.h"

namespace Nancy {
namespace Action {

void DragSurfacePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	// The view is as large as its place on screen, so the rects are read as they are
	readExclusiveRect(stream, _src);
	readExclusiveRect(stream, _dest);
	readFilename(stream, _saveName);
	_hoverCursor = stream.readUint16LE();
	_dragCursor = stream.readUint16LE();

	readActionZoneArray(stream, _zones, true);
	readExitHotspot(stream);
}

void DragSurfacePuzzle::init() {
	_drawSurface.create(_dest.width(), _dest.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(_dest);

	loadImage();

	// Back to where the picture was left
	const DragSurfaceData *data = (const DragSurfaceData *)NancySceneState.getPuzzleData(DragSurfaceData::getTag());
	assert(data);

	if (!_saveName.empty() && data->viewOrigins.contains(_saveName)) {
		_src.moveTo(data->viewOrigins[_saveName]);
	}

	redraw();
}

void DragSurfacePuzzle::saveView() {
	if (_saveName.empty()) {
		return;
	}

	DragSurfaceData *data = (DragSurfaceData *)NancySceneState.getPuzzleData(DragSurfaceData::getTag());
	assert(data);
	data->viewOrigins[_saveName] = _src.origin();
}

void DragSurfacePuzzle::redraw() {
	_drawSurface.blitFrom(_image, _src, Common::Point());
	_needsRedraw = true;
}

bool DragSurfacePuzzle::isInZone(const ActionZone &zone, const Common::Point &picturePos) const {
	// A zone can depend on an event flag
	if (zone.val49 != -1 && !NancySceneState.getEventFlag(zone.val49, zone.val4b)) {
		return false;
	}

	return zone.rect.contains(picturePos);
}

void DragSurfacePuzzle::dragTo(const Common::Point &pos) {
	// The picture follows the mouse along each direction it has room in
	int dx = _lastDragPos.x - pos.x;
	int dy = _lastDragPos.y - pos.y;

	if (dx != 0 && _src.left + dx >= 0 && _src.right + dx < _image.w) {
		_src.translate(dx, 0);
		_lastDragPos.x = pos.x;
	}

	if (dy != 0 && _src.top + dy >= 0 && _src.bottom + dy < _image.h) {
		_src.translate(0, dy);
		_lastDragPos.y = pos.y;
	}

	redraw();
}

void DragSurfacePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested || _zoneSceneRequested) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		if (_zoneSceneRequested) {
			NancySceneState.changeScene(_zoneScene);
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void DragSurfacePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested || _zoneSceneRequested) {
		return;
	}

	const bool isReleased = input.input & NancyInput::kLeftMouseButtonUp;
	if (!(input.input & (NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonHeld)) && !isReleased) {
		_isMouseDown = false;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (!_isMouseDown && hoverExitHotspot(input)) {
		if (isReleased) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	if (!_dest.contains(pos)) {
		return;
	}

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		_isMouseDown = true;
		_hasDragged = false;
		_pressPos = pos;
		_lastDragPos = pos;
	}

	if (_isMouseDown && !isReleased) {
		if (!_hasDragged && (ABS(pos.x - _pressPos.x) > kDragThreshold || ABS(pos.y - _pressPos.y) > kDragThreshold)) {
			_hasDragged = true;
		}

		if (_hasDragged) {
			dragTo(pos);
			setDataCursor(_dragCursor);
			return;
		}
	}

	const bool clicked = isReleased && _isMouseDown && !_hasDragged;
	if (isReleased) {
		if (_hasDragged) {
			saveView();
		}

		_isMouseDown = false;
		_hasDragged = false;
	}

	Common::Point picturePos = pos - _dest.origin() + _src.origin();

	bool hasZoneCursor = false;
	for (const ActionZone &zone : _zones) {
		if (!isInZone(zone, picturePos)) {
			continue;
		}

		if (zone.type == kZoneCursor && !hasZoneCursor) {
			setDataCursor(zone.tailId, zone.tailFlag != 0);
			hasZoneCursor = true;
		} else if (zone.type == kZoneUnknown13 && clicked && zone.specialEffectId != kNoSceneNancy17) {
			// These change the scene once they are clicked
			playSoundBlock(zone._sound);
			_zoneScene.sceneID = zone.specialEffectId;
			_zoneScene.frameID = 0;
			_zoneScene.continueSceneSound = kContinueSceneSound;
			_zoneSceneRequested = true;
			input.eatMouseInput();
			return;
		}
	}

	if (!hasZoneCursor) {
		setDataCursor(_hoverCursor);
	}
}

} // End of namespace Action
} // End of namespace Nancy
