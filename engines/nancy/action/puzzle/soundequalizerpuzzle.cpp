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

#include "engines/nancy/action/puzzle/soundequalizerpuzzle.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/scrollbar.h"

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/puzzledata.h"

namespace Nancy {
namespace Action {

class ViewportScrollbar : public UI::Scrollbar {
public:
	ViewportScrollbar(uint16 zOrder, const Common::Rect &srcBounds, Graphics::ManagedSurface &srcSurf, const Common::Point &topPosition, uint16 scrollDistance, bool isVertical = true) :
		Scrollbar(zOrder, srcBounds, srcSurf, topPosition, scrollDistance, isVertical) {}
	virtual ~ViewportScrollbar() = default;

	bool handleInput(NancyInput &input) {
		if (_screenPosition.contains(input.mousePos)) {
			input.input &= (~NancyInput::kRightMouseButtonUp);

			Scrollbar::handleInput(input);

			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			return true;
		}

		return false;
	}
};

SoundEqualizerPuzzle::~SoundEqualizerPuzzle() {
	for (auto *scrollbar : _sliders) {
		delete scrollbar;
	}
}

void SoundEqualizerPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	const VIEW *viewportData = (const VIEW *)g_nancy->getEngineData("VIEW");
	assert(viewportData);
	Common::Rect vpPos = viewportData->screenPosition;

	if (_puzzleState->sliderValues[0] == 255) {
		for (uint i = 0; i < 6; ++i) {
			_puzzleState->sliderValues[i] = _sliderInitialPositions[i];
		}
	}

	_sliders.resize(6);
	for (uint i = 0; i < 6; ++i) {
		Common::Point screenDest(_sliderX[i], _sliderYMax[i] - (_sliderSrc.height() / 2));
		screenDest.x += vpPos.left;
		screenDest.y += vpPos.top;
		_sliders[i] = new ViewportScrollbar(	8,
												_sliderSrc,
												_image,
												screenDest,
												_sliderYMin[i] - _sliderYMax[i]);
		_sliders[i]->init();
		_sliders[i]->setPosition((float)(100 - _puzzleState->sliderValues[i]) / 100);
	}
}

void SoundEqualizerPuzzle::registerGraphics() {
	for (auto *scrollbar : _sliders) {
		scrollbar->registerGraphics();
	}

	RenderActionRecord::registerGraphics();
}

void SoundEqualizerPuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = (SoundEqualizerPuzzleData *)NancySceneState.getPuzzleData(SoundEqualizerPuzzleData::getTag());
	assert(_puzzleState);

	uint16 difficulty = NancySceneState.getDifficulty();
	readFilename(stream, _imageName);

	readRect(stream, _buttonSrc);
	readRect(stream, _buttonDest);
	readRect(stream, _sliderSrc);

	_sliderX.resize(6);
	_sliderYMin.resize(6);
	_sliderYMax.resize(6);
	for (uint i = 0; i < 6; ++i) {
		_sliderX[i] = stream.readUint16LE();
		_sliderYMin[i] = stream.readUint16LE();
		_sliderYMax[i] = stream.readUint16LE();
	}

	readRect(stream, _lightSrc);

	_lightDests.resize(6);
	for (uint i = 0; i < 6; ++i) {
		readRectArray(stream, _lightDests[i], 10);
	}

	_sliderInitialPositions.resize(6);
	for (uint i = 0; i < 3; ++i) {
		// Only read the data for the current difficulty and skip over the rest
		if (i == difficulty) {
			for (uint j = 0; j < 6; ++j) {
				_sliderInitialPositions[j] = stream.readUint16LE();
			}
		} else {
			stream.skip(12);
		}
	}

	_sounds.resize(3);
	for (uint i = 0; i < 3; ++i) {
		_sounds[i].readNormal(stream);
	}

	_minVolume.resize(3);
	_maxVolume.resize(3);
	_minRate.resize(3);
	_maxRate.resize(3);

	for (uint i = 0; i < 3; ++i) {
		// Only read the data for the current difficulty and skip over the rest
		if (i == difficulty) {
			for (uint j = 0; j < 3; ++j) {
				_minVolume[j] = stream.readUint16LE();
				_maxVolume[j] = stream.readUint16LE();
				_minRate[j] = stream.readUint32LE();
				_maxRate[j] = stream.readUint32LE();
			}
		} else {
			stream.skip(12 * 3);
		}
	}

	_solveChannelID = stream.readUint16LE();

	for (uint i = 0; i < 3; ++i) {
		// Only read the data for the current difficulty and skip over the rest
		if (i == difficulty) {
			_solveMinVolume = stream.readUint16LE();
			_solveMaxVolume = stream.readUint16LE();
			_solveMinRate = stream.readUint32LE();
			_solveMaxRate = stream.readUint32LE();
		} else {
			stream.skip(12);
		}
	}

	_exitScene.readData(stream);
	stream.skip(2);
	_exitSound.readNormal(stream);

	_solveFlag.label = stream.readSint16LE();
	_solveFlag.flag = stream.readByte();
}

void SoundEqualizerPuzzle::execute() {
	switch(_state) {
	case kBegin:
		_puzzleState = (SoundEqualizerPuzzleData *)NancySceneState.getPuzzleData(SoundEqualizerPuzzleData::getTag());
		assert(_puzzleState);

		init();
		registerGraphics();

		for (uint i = 0; i < 3; ++i) {
			g_nancy->_sound->loadSound(_sounds[i]);
			g_nancy->_sound->playSound(_sounds[i]);
		}

		for (uint i = 0; i < 6; ++i) {
			updateSlider(i);
		}

		NancySceneState.setNoHeldItem();

		_state = kRun;
		break;
	case kRun:
		break;
	case kActionTrigger:
		if (g_nancy->_sound->isSoundPlaying(_exitSound)) {
			return;
		}

		for (uint i = 0; i < 3; ++i) {
			g_nancy->_sound->stopSound(_sounds[i]);
		}

		NancySceneState.changeScene(_exitScene);
		finishExecution();
	}
}

void SoundEqualizerPuzzle::handleInput(NancyInput &input) {
	if (_state == kActionTrigger) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		return;
	} else if (_state == kBegin) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_buttonDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			// Exit button pressed
			_drawSurface.blitFrom(_image, _buttonSrc, _buttonDest);
			_needsRedraw = true;

			g_nancy->_sound->loadSound(_exitSound);
			g_nancy->_sound->playSound(_exitSound);

			_state = kActionTrigger;
			return;
		}
	} else {
		for (uint i = 0; i < 6; ++i) {
			if (_sliders[i]->handleInput(input)) {
				updateSlider(i);
				break;
			}
		}
	}
}

void SoundEqualizerPuzzle::updateSlider(uint sliderID) {
	float sliderVal = 1 - _sliders[sliderID]->getPos();
	_puzzleState->sliderValues[sliderID] = sliderVal * 100;

	if (sliderID < 3) {
		// First three sliders change pitch, except when the slider
		// controls the "solve" sound; in that case it does nothing
		if (sliderID != _solveChannelID) {
			g_nancy->_sound->setRate(_sounds[sliderID],
				_minRate[sliderID] + (_maxRate[sliderID] - _minRate[sliderID]) * sliderVal);
		}
	} else {
		// Other three sliders change volume;
		// "solve" sound slider behaves as an on/off switch
		if (sliderID - 3 != _solveChannelID) {
			g_nancy->_sound->setVolume(_sounds[sliderID - 3],
				_minVolume[sliderID - 3] + (_maxVolume[sliderID - 3] - _minVolume[sliderID - 3]) * sliderVal);
		} else {
			if (sliderVal * 100 >= _solveMinVolume && sliderVal * 100 <= _solveMaxVolume) {
				g_nancy->_sound->setVolume(_sounds[sliderID - 3], _maxVolume[sliderID - 3]);

				// Since the rate for the "solve" sound never actually changes,
				// we only need the volume to be correct.
				NancySceneState.setEventFlag(_solveFlag);
			} else {
				g_nancy->_sound->setVolume(_sounds[sliderID - 3], _minVolume[sliderID - 3]);
			}
		}
	}

	uint numLights = sliderVal * 10;

	if (numLights < 10) {
		Common::Rect clear = _lightDests[sliderID][numLights];
		clear.extend(_lightDests[sliderID][9]);

		_drawSurface.fillRect(clear, _drawSurface.getTransparentColor());

		_needsRedraw = true;
	}

	for (uint i = 0; i < numLights; ++i) {
		_drawSurface.blitFrom(_image, _lightSrc, _lightDests[sliderID][i]);

		_needsRedraw = true;
	}
}

} // End of namespace Action
} // End of namespace Nancy
