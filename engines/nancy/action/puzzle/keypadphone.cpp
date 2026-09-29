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
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/keypadphone.h"

namespace Nancy {
namespace Action {

static const uint32 kMsPerTick = 20;
static const float kTicksPerSecond = 50.0f;

void KeypadPhone::readScene(Common::SeekableReadStream &stream, SceneChangeWithFlag &scene) {
	scene._sceneChange.sceneID = stream.readUint16LE();
	scene._sceneChange.frameID = stream.readUint16LE();
	scene._sceneChange.continueSceneSound = kContinueSceneSound;
	scene._flag.label = stream.readSint16LE();
	scene._flag.flag = stream.readByte();
}

// Numbers are written with dashes and spaces; only the keys on the keypad count
Common::String KeypadPhone::keepDialable(const Common::String &text) {
	Common::String result;
	for (char c : text) {
		if (Common::isDigit(c) || c == '*' || c == '#') {
			result += c;
		}
	}

	return result;
}

void KeypadPhone::readData(Common::SeekableReadStream &stream) {
	_maxDigits = stream.readUint32LE();
	_noClassWaitTicks = (int32)(stream.readFloatLE() * kTicksPerSecond);
	_classWaitTicks = (int32)(stream.readFloatLE() * kTicksPerSecond);

	_numbers.resize(stream.readUint16LE());
	for (Number &number : _numbers) {
		Common::String digits;
		readFilename(stream, digits);
		number.digits = keepDialable(digits);
		number.condition.label = stream.readSint16LE();
		number.condition.flag = stream.readByte();
		readScene(stream, number.scene);
	}

	_buttonCursor = stream.readUint16LE();
	readFilename(stream, _imageName);
	_pressedTicks = (int32)(stream.readFloatLE() * kTicksPerSecond);

	_buttons.resize(stream.readUint16LE());
	for (Button &button : _buttons) {
		button.key = stream.readByte();
		readRect(stream, button.src);
		readRect(stream, button.dest);
		button.sound.readData(stream);
	}

	_classes.resize(stream.readUint16LE());
	for (NumberClass &numberClass : _classes) {
		readFilename(stream, numberClass.prefix);
		numberClass.minExtraDigits = stream.readSint32LE();
		numberClass.maxExtraDigits = stream.readSint32LE();
		readScene(stream, numberClass.scene);
	}

	_dialToneBlock.readData(stream);
	readExitHotspots(stream, _exits);
}

void KeypadPhone::init() {
	initViewportSurface();
	loadImage();

	_lastTickTime = g_nancy->getTotalPlayTime();
	_dialTone = playSoundBlock(_dialToneBlock);
}

// A class matches digits that start with its prefix and have the right amount after it
const KeypadPhone::NumberClass *KeypadPhone::findClass() const {
	for (const NumberClass &numberClass : _classes) {
		int32 extra = (int32)_dialed.size() - (int32)numberClass.prefix.size();
		if (_dialed.hasPrefix(numberClass.prefix) && extra >= numberClass.minExtraDigits && extra <= numberClass.maxExtraDigits) {
			return &numberClass;
		}
	}

	return nullptr;
}

void KeypadPhone::pressButton(const Button &button) {
	_dialed += button.key;

	// Enhancement: the original keeps the dial tone going until the phone changes scene
	g_nancy->_sound->stopSound(_dialTone);
	playSoundBlock(button.sound);

	// The number gets dialed once the player stops pressing keys, sooner when it
	// already looks complete
	if (_dialed.size() >= _maxDigits) {
		_dialWaitTicks = 1;
	} else {
		_dialWaitTicks = findClass() ? _classWaitTicks : _noClassWaitTicks;
	}

	_pressedButton = &button;
	_pressedTicksLeft = _pressedTicks;
}

void KeypadPhone::dial() {
	const Number *emptyNumber = nullptr;
	const Number *dialedNumber = nullptr;
	for (const Number &number : _numbers) {
		if (number.digits.empty()) {
			emptyNumber = &number;
		} else if (number.digits == _dialed) {
			dialedNumber = &number;
		}
	}

	const NumberClass *numberClass = findClass();
	if (dialedNumber && (dialedNumber->condition.label == kEvNoEvent ||
			NancySceneState.getEventFlag(dialedNumber->condition.label, dialedNumber->condition.flag))) {
		_endScene = dialedNumber->scene;
	} else if (numberClass) {
		_endScene = numberClass->scene;
	} else if (emptyNumber) {
		_endScene = emptyNumber->scene;
	}

	g_nancy->_sound->stopSound(_dialTone);
	_hasDialed = true;
}

void KeypadPhone::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	if (_pressedButton && _pressedTicksLeft > 0) {
		_drawSurface.blitFrom(_image, _pressedButton->src, _pressedButton->dest.origin());
	}

	_needsRedraw = true;
}

void KeypadPhone::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();
		int32 ticks = (now - _lastTickTime) / kMsPerTick;
		_lastTickTime += ticks * kMsPerTick;

		if (ticks > 0) {
			_pressedTicksLeft = MAX<int32>(_pressedTicksLeft - ticks, 0);

			if (_dialWaitTicks > 0) {
				_dialWaitTicks = MAX<int32>(_dialWaitTicks - ticks, 0);
				if (_dialWaitTicks == 0) {
					dial();
				}
			}
		}

		redraw();

		if (_hasDialed) {
			_state = kActionTrigger;
		}

		break;
	}
	case kActionTrigger:
		if (_hasDialed) {
			_endScene.execute();
		} else {
			g_nancy->_sound->stopSound(_dialTone);
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void KeypadPhone::handleInput(NancyInput &input) {
	if (_state != kRun || _hasDialed) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();
	bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	for (const ExitHotspot &exit : _exits) {
		if (!exit.hotspot.contains(pos)) {
			continue;
		}

		setDataCursor(exit.cursorType);
		if (clicked) {
			_exitScene._sceneChange = exit.scene;
			_exitScene._flag = exit.flag;
			_state = kActionTrigger;
			input.eatMouseInput();
		}

		return;
	}

	for (const Button &button : _buttons) {
		if (!button.dest.contains(pos)) {
			continue;
		}

		setDataCursor(_buttonCursor);
		if (clicked) {
			pressButton(button);
			input.eatMouseInput();
		}

		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
