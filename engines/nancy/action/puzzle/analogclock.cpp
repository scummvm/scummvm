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

#include "common/archive.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/analogclock.h"

namespace Nancy {
namespace Action {

void AnalogClock::readData(Common::SeekableReadStream &stream) {
	_alarmIndex = stream.readSint16LE();

	for (Hand &hand : _hands) {
		readRect(stream, hand.dest);
		readFilename(stream, hand.movieName);
	}

	// The alarm needs both its movie and its value table entry
	if (_hands[kAlarmHand].movieName.empty() || !hasAlarm()) {
		_alarmIndex = kNoAlarm;
	}

	_alarmHandCursor = stream.readUint16LE();
	_alarmHandSound.readData(stream);
	_tickSound.readData(stream);

	readFilename(stream, _imageName);
	readRect(stream, _buttonDownSrc);
	readRect(stream, _buttonDest);
	_buttonCursor = stream.readUint16LE();

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_buttonSoundBlock.readData(stream);

	readExitHotspot(stream);
}

void AnalogClock::init() {
	initViewportSurface();

	// The game data lacks the pressed button's image
	if (SearchMan.hasFile(_imageName) || SearchMan.hasFile(_imageName.append(".cif"))) {
		loadImage();
	}

	for (uint i = 0; i < kNumHands; ++i) {
		if (i != kAlarmHand || hasAlarm()) {
			_hands[i].movie.loadFile(_hands[i].movieName);
		}
	}

	redraw();
}

int16 AnalogClock::getAlarmHour() const {
	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	assert(table);
	return table->getValue(_alarmIndex);
}

void AnalogClock::drawHand(Hand &hand, int frame) {
	if (!hand.movie.isVideoLoaded()) {
		return;
	}

	if (frame != hand.frame) {
		const Graphics::Surface *decoded = hand.movie.decodeNextFrame(frame);
		if (!decoded) {
			return;
		}

		GraphicsManager::copyToManaged(*decoded, hand.frameSurface);
		hand.frameSurface.setTransparentColor(_drawSurface.getTransparentColor());
		hand.frame = frame;
	}

	_drawSurface.blitFrom(hand.frameSurface, hand.dest.origin());
}

// Advances the player clock to the next time the alarm hour comes around
void AnalogClock::pressAlarmButton() {
	uint minutes = NancySceneState.getPlayerTimeMinutes();
	int hour = minutes / 60;
	int alarmHour = getAlarmHour();
	if (alarmHour <= hour) {
		alarmHour += 12 + ((hour - alarmHour) / 12) * 12;
	}

	int32 advance = (alarmHour - hour) * 60 - (int32)(minutes % 60);
	NancySceneState.setPlayerTime((uint32)advance * 60000, kRelativeClockBump);

	_isButtonPressed = true;
	_solveSound = playSoundBlock(_buttonSoundBlock);
}

void AnalogClock::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	uint minutes = NancySceneState.getPlayerTimeMinutes();
	Hand &hourHand = _hands[kHourHand];
	Hand &minuteHand = _hands[kMinuteHand];
	if (hourHand.movie.isVideoLoaded()) {
		float hours = (minutes / 60) % 12 + (minutes % 60) / 60.0f;
		drawHand(hourHand, (int)(hourHand.movie.getFrameCount() * hours / 12.0f));
	}

	drawHand(minuteHand, _minuteFrame);

	if (hasAlarm()) {
		drawHand(_hands[kAlarmHand], getAlarmHour());
	}

	if (_isButtonPressed && !_image.empty()) {
		_drawSurface.blitFrom(_image, _buttonDownSrc, _buttonDest.origin());
	}

	_needsRedraw = true;
}

void AnalogClock::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_isButtonPressed) {
			if (!isSolveSoundPlaying()) {
				_state = kActionTrigger;
			}

			break;
		}

		// The minute hand ticks audibly
		Hand &minuteHand = _hands[kMinuteHand];
		if (minuteHand.movie.isVideoLoaded()) {
			int frame = (int)(minuteHand.movie.getFrameCount() * (NancySceneState.getPlayerTimeMinutes() % 60) / 60.0f);
			if (frame != _minuteFrame) {
				playSoundBlock(_tickSound);
				_minuteFrame = frame;
			}
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_solveScene.execute();
		}

		finishExecution();
		break;
	}
}

void AnalogClock::handleInput(NancyInput &input) {
	if (_state != kRun || _isButtonPressed || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	if (!hasAlarm()) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();
	bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	if (_hands[kAlarmHand].dest.contains(pos)) {
		setDataCursor(_alarmHandCursor);
		if (clicked) {
			// Each click moves the alarm on by an hour
			TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
			assert(table);
			table->setValue(_alarmIndex, (getAlarmHour() + 1) % 12);
			playSoundBlock(_alarmHandSound);
			input.eatMouseInput();
		}
	} else if (_buttonDest.contains(pos)) {
		setDataCursor(_buttonCursor);
		if (clicked) {
			pressAlarmButton();
			input.eatMouseInput();
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
