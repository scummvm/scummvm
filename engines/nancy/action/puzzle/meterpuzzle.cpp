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
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/meterpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void MeterPuzzle::readData(Common::SeekableReadStream &stream) {
	_mode = stream.readSint16LE();		// 0x00
	_modeParam = stream.readSint16LE();	// 0x02
	_modeValue = stream.readSint32LE();	// 0x04

	readFilename(stream, _animName);	// 0x08

	_videoFormat = stream.readUint16LE();	// 0x29
	_value = stream.readSint16LE();			// 0x2b
	_firstFrame = stream.readSint16LE();	// 0x2d
	_lastFrame = stream.readSint16LE();		// 0x2f

	readRect(stream, _srcRect);			// 0x31
	readRect(stream, _destRect);		// 0x41
}

void MeterPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	_animation.loadFile(_animName);

	_displayedValue = -1;
	redraw();
}

double MeterPuzzle::sampleFraction() const {
	if (_mode == kTimer) {
		// The elapsed time of software timer _modeParam
		if (_modeParam < 0 || (uint)_modeParam >= TimerData::kNumTimers) {
			return 0.0;
		}

		TimerData *timerData = (TimerData *)NancySceneState.getPuzzleData(TimerData::getTag());
		if (!timerData) {
			return 0.0;
		}

		const TimerData::Timer &timer = timerData->timers[_modeParam];
		int32 fullScale = _modeValue;
		if (fullScale == -1) {
			// Full scale is the time of the timer's first one-shot trigger
			fullScale = 1;
			for (const TimerData::Trigger &trigger : timer.triggers) {
				if (trigger.type == TimerData::Trigger::kOneShot) {
					if (trigger.durationMs > 0) {
						fullScale = trigger.durationMs;
					}
					break;
				}
			}
		}

		return fullScale ? (double)timer.currentTimeMs / fullScale : 0.0;
	}

	// Modes 1/2 track value-table entry _modeParam
	int32 value = _value;
	if (_modeParam != 0xff) {
		TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
		if (table) {
			int16 tableValue = table->getValue(_modeParam);
			value = tableValue == kNoTableValue ? 0 : tableValue;	// unset reads as empty
		}
	}

	return _modeValue ? (double)value / _modeValue : 0.0;
}

int MeterPuzzle::computeFrame() const {
	int frameCount = _animation.getFrameCount();
	if (frameCount <= 0) {
		return 0;
	}

	int frame = (int)((double)frameCount * sampleFraction() + 0.5);
	return CLIP(frame, 0, frameCount - 1);
}

void MeterPuzzle::redraw() {
	if (!_animation.isVideoLoaded()) {
		return;
	}

	int frame = computeFrame();
	if (frame == _displayedValue) {
		return;
	}

	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	_animation.goToFrame(frame);
	// _srcRect is unused
	_animation.drawFrame(_drawSurface, Common::Point(_destRect.left, _destRect.top));

	_displayedValue = frame;
	_needsRedraw = true;
}

void MeterPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		// Repaint when the tracked value's frame changes.
		redraw();
		break;
	default:
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
