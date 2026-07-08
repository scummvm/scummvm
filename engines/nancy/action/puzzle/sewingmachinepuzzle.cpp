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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/sewingmachinepuzzle.h"

namespace Nancy {
namespace Action {

void SewingMachinePuzzle::readData(Common::SeekableReadStream &stream) {
	// 87-byte PuzzleBase header blob.
	readFilename(stream, _imageName);	// blob 0x00
	readRect(stream, _rects[0]);		// blob 0x21
	readRect(stream, _rects[1]);		// blob 0x31
	_directionVector.x = stream.readSint32LE();	// blob 0x41
	_directionVector.y = stream.readSint32LE();	// blob 0x45
	_extentVector.x = stream.readSint32LE();	// blob 0x49
	_extentVector.y = stream.readSint32LE();	// blob 0x4d
	for (int i = 0; i < 3; ++i) {
		_params[i] = stream.readSint16LE();		// blob 0x51 / 0x53 / 0x55
	}

	_soundBlock.readData(stream);

	readActionZoneArray(stream, _zones);
}

void SewingMachinePuzzle::classifyZones() {
	for (uint i = 0; i < _zones.size(); ++i) {
		const ActionZone &z = _zones[i];
		switch (z.type) {
		case 0x0b:	// the needle line: touching it plays a stitch cue and sets a flag
			_collisionZone = i;
			break;
		case 0x0c:	// bottom trigger: its SpecialEffect carries the win scene + fade
			_triggerZones.push_back(i);
			if (z.specialEffectId >= 1000 && _winScene.sceneID == kNoScene) {
				_winScene.sceneID = z.specialEffectId;
				_winEventFlag = z.val49;
				if (z.hasSpecialEffect) {
					_winHasFade = true;
					_winFadeType = z.seType;
					_winFadeTotalTime = z.seTotalTime;
					_winFadeToBlackTime = z.seFadeToBlackTime;
					_winFadeRect = z.seRect;
				}
			}
			break;
		case 0x14:	// play-area boundary
			_boundaryZone = i;
			break;
		default:
			break;
		}
	}
}

void SewingMachinePuzzle::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

void SewingMachinePuzzle::drawCloth() {
	// The cloth is a tall strip; the fixed needle (drawn by the scene) sits over it.
	// The scene shows through wherever the strip does not cover.
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	_drawSurface.blitFrom(_image, _clothPos);
	_needsRedraw = true;
}

void SewingMachinePuzzle::feedCloth(const Common::Point &delta) {
	Common::Point newPos(CLIP<int>(_clothPos.x + delta.x, -_maxSteer, _maxSteer),
		CLIP<int>(_clothPos.y + delta.y, -_maxFeed, 0));

	int moved = ABS(newPos.x - _clothPos.x) + ABS(newPos.y - _clothPos.y);
	_clothPos = newPos;

	if (moved == 0) {
		// A pause resets the stitch stroke, matching the original's hysteresis.
		_strokeDistance = 0.0;
		return;
	}

	_strokeDistance += moved;
	drawCloth();

	// A stitch cue (needle sound) fires each time the cloth advances past the threshold.
	if (_strokeDistance >= _params[1] && _collisionZone >= 0) {
		_strokeDistance = 0.0;
		playSoundBlock(_zones[_collisionZone]._sound);
	}

	// The seam is finished once the cloth has been fed all the way through.
	if (_maxFeed > 0 && _clothPos.y <= -_maxFeed && !_solved) {
		_solved = true;
		_state = kActionTrigger;
	}
}

void SewingMachinePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Feed scrolls the whole strip past the viewport; steer lets the cloth wiggle
	// horizontally to follow the seam under the needle (range is a tunable guess).
	_maxFeed = MAX(0, _image.h - _drawSurface.h);
	_maxSteer = 100;
}

void SewingMachinePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		classifyZones();
		playSoundBlock(_soundBlock);	// start the sewing-machine ambience
		drawCloth();
		_state = kRun;
		break;
	case kRun:
		break;
	case kActionTrigger:
		// Seam finished: raise the win flag, then cross-dissolve to the scene the
		// trigger zone's SpecialEffect points at.
		if (_winEventFlag != -1) {
			NancySceneState.setEventFlag(_winEventFlag, g_nancy->_true);
		}
		if (_winScene.sceneID >= 1000 && _winScene.sceneID != kNoScene) {
			if (_winHasFade) {
				NancySceneState.specialEffect(_winFadeType, _winFadeTotalTime, _winFadeToBlackTime, _winFadeRect);
			}
			NancySceneState.changeScene(_winScene);
		}
		finishExecution();
		break;
	}
}

void SewingMachinePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		_dragging = true;
		_lastDragPos = input.mousePos;
	}

	if (_dragging && (input.input & NancyInput::kLeftMouseButtonHeld)) {
		feedCloth(input.mousePos - _lastDragPos);
		_lastDragPos = input.mousePos;
	}

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		_dragging = false;
		_strokeDistance = 0.0;
	}
}

} // End of namespace Action
} // End of namespace Nancy
