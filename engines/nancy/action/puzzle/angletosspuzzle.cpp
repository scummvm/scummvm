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
#include "engines/nancy/action/puzzle/angletosspuzzle.h"

namespace Nancy {
namespace Action {

void AngleTossPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Draw the initial angle and power indicators.
	// The throw button sprite is NOT drawn here — the static background already shows the
	// idle button state. The sprite (data+0x4d) is the launched/pressed overlay, drawn
	// only when LAUNCH is clicked (mirroring the DAT_0059bfbd conditional in FUN_0044b1fa).
	_drawSurface.blitFrom(_image, _angleSprites[_curAngle], _angleDisplay);
	_drawSurface.blitFrom(_image, _powerSprites[_curPower], _powerDisplay);
}

void AngleTossPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	// data+0x21..0x2c: 6 × uint16.
	// _initialPower/_initialAngle: starting player position (copied to object+0x24/0x26 in original).
	// _numPowers/_numAngles: UI control bounds.
	// _targetPower/_targetAngle: the correct answer for this round (compared in FUN_0044a6be).
	_initialPower = stream.readUint16LE();
	_initialAngle = stream.readUint16LE();
	_numPowers    = stream.readUint16LE();
	_numAngles    = stream.readUint16LE();
	_targetPower  = stream.readUint16LE();
	_targetAngle  = stream.readUint16LE();

	// 22 rects — see header for full mapping.
	readRect(stream, _throwHotspot);					// Rect  0 — data+0x2d
	readRect(stream, _throwDisplay);					// Rect  1 — data+0x3d
	readRect(stream, _throwSprite);					// Rect  2 — data+0x4d
	readRect(stream, _aimLeftHotspot);				// Rect  3 — data+0x5d
	readRect(stream, _aimRightHotspot);				// Rect  4 — data+0x6d
	readRect(stream, _angleDisplay);					// Rect  5 — data+0x7d
	readRectArray(stream, _angleSprites, 5);		// Rects 6-10 — data+0x8d
	readRect(stream, _powerDisplay);					// Rect 11 — data+0xdd
	readRectArray(stream, _powerHotspots, 5);		// Rects 12-16 — data+0xed
	readRectArray(stream, _powerSprites, 5);		// Rects 17-21 — data+0x13d

	_powerSound.readNormal(stream);		// data+0x18d
	_squeakSound.readNormal(stream);	// data+0x1be
	_chainSound.readNormal(stream);		// data+0x1ef

	_throwSquidScene.readData(stream);	// data+0x220, ends at data+0x238

	// HACK: We've skipped some of the flag data at this point,
	// so go back to read them correctly
	_throwSquidScene._flag.label = kEvNoEvent;
	_throwSquidScene._flag.flag = 0;
	stream.seek(-3, SEEK_CUR);

	_powerTooStrongFlag = stream.readSint16LE();
	_powerTooWeakFlag = stream.readSint16LE();
	_angleTooLeftFlag = stream.readSint16LE();
	_angleTooRightFlag = stream.readSint16LE();
	_winFlag = stream.readSint16LE();

	_exitScene.readData(stream);		// data+0x240
	readRect(stream, _exitHotspot);		// data+0x259
}

void AngleTossPuzzle::execute() {
	switch (_state) {
	case kBegin:
		// Restore the player selection to the starting position for this round.
		_curPower = _initialPower;
		_curAngle = _initialAngle;

		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_powerSound);
		g_nancy->_sound->loadSound(_squeakSound);
		g_nancy->_sound->loadSound(_chainSound);

		// FUN_0044a526: clear all result/hint flags before the player starts.
		NancySceneState.setEventFlag(_powerTooStrongFlag, g_nancy->_false);
		NancySceneState.setEventFlag(_powerTooWeakFlag, g_nancy->_false);
		NancySceneState.setEventFlag(_angleTooLeftFlag, g_nancy->_false);
		NancySceneState.setEventFlag(_angleTooRightFlag, g_nancy->_false);
		NancySceneState.setEventFlag(_winFlag, g_nancy->_false);

		_state = kRun;
		break;

	case kRun:
		// Wait for the chain sound to finish, then evaluate the throw (FUN_0044a6be)
		// and always transition to _throwSquidScene so the animation plays.
		if (_isThrown && !g_nancy->_sound->isSoundPlaying(_chainSound)) {
			_isThrown = false;

			// FUN_0044a6be: set exactly one result flag based on how accurate the throw was.
			// The animation scene reads these flags to decide what to show.
			if (_curPower == _targetPower && _curAngle == _targetAngle) {
				// Exact match of power and angle — player wins round!
				NancySceneState.setEventFlag(_winFlag, g_nancy->_true);
			} else if (_curPower > _targetPower) {
				// Power too strong
				NancySceneState.setEventFlag(_powerTooStrongFlag, g_nancy->_true);
			} else if (_curPower < _targetPower) {
				// Power too weak
				NancySceneState.setEventFlag(_powerTooWeakFlag, g_nancy->_true);
			} else if (_curAngle > _targetAngle) {
				// Angle too far right
				NancySceneState.setEventFlag(_angleTooRightFlag, g_nancy->_true);
			} else if (_curAngle < _targetAngle) {
				// Angle too far left
				NancySceneState.setEventFlag(_angleTooLeftFlag, g_nancy->_true);
			}

			_state = kActionTrigger;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_powerSound);
		g_nancy->_sound->stopSound(_squeakSound);
		g_nancy->_sound->stopSound(_chainSound);

		if (_exitPressed) {
			_exitScene.execute();
		} else {
			_throwSquidScene.execute();
		}

		finishExecution();
		break;
	}
}

void AngleTossPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isThrown) {
		return;
	}

	// All rects are in viewport-local coordinates.
	Common::Point localMousePos = input.mousePos;
	Common::Rect vpPos = NancySceneState.getViewport().getScreenPosition();
	localMousePos -= Common::Point(vpPos.left, vpPos.top);

	// Exit button
	if (_exitHotspot.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitPressed = true;
			_state = kActionTrigger;
		}
		return;
	}

	// Aim left arrow — always show hotspot cursor; only act when not already at min
	if (_aimLeftHotspot.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (_curAngle > 0 && (input.input & NancyInput::kLeftMouseButtonUp)) {
			--_curAngle;
			_drawSurface.fillRect(_angleDisplay, _drawSurface.getTransparentColor());
			_drawSurface.blitFrom(_image, _angleSprites[_curAngle], _angleDisplay);
			g_nancy->_sound->playSound(_squeakSound);
			_needsRedraw = true;
		}
		return;
	}

	// Aim right arrow — always show hotspot cursor; only act when not already at max
	if (_aimRightHotspot.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (_curAngle + 1 < _numAngles && (input.input & NancyInput::kLeftMouseButtonUp)) {
			++_curAngle;
			_drawSurface.fillRect(_angleDisplay, _drawSurface.getTransparentColor());
			_drawSurface.blitFrom(_image, _angleSprites[_curAngle], _angleDisplay);
			g_nancy->_sound->playSound(_squeakSound);
			_needsRedraw = true;
		}
		return;
	}

	// Power-level buttons — direct selection (Whale = 0, Dolphin = 1, Trout = 2, Shrimp = 3, Fish Fry = 4)
	for (uint i = 0; i < _powerHotspots.size(); ++i) {
		if (_powerHotspots[i].contains(localMousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if ((input.input & NancyInput::kLeftMouseButtonUp) && _curPower != i) {
				_curPower = i;
				_drawSurface.fillRect(_powerDisplay, _drawSurface.getTransparentColor());
				_drawSurface.blitFrom(_image, _powerSprites[_curPower], _powerDisplay);
				g_nancy->_sound->playSound(_powerSound);
				_needsRedraw = true;
			}
			return;
		}
	}

	// LAUNCH button — hotspot is rect 0 (_throwHotspot), sprite is drawn at rect 1 (_throwDisplay)
	if (_throwHotspot.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			g_nancy->_sound->playSound(_chainSound);
			_isThrown = true;

			// Show the launched/pressed overlay sprite (DAT_0059bfbd != 0 path in FUN_0044b1fa).
			_drawSurface.blitFrom(_image, _throwSprite, _throwDisplay);
			_needsRedraw = true;
		}
		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
