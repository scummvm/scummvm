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
#include "common/math.h"
#include "tetraedge/te/te_frame_anim.h"

namespace Tetraedge {

TeFrameAnim::TeFrameAnim() : _nbFrames(0), _frameRate(25.0f), _reversed(false), _minFrame(0),
_numFramesToShow(-1), _startTime(0), _endTime(FLT_MAX), _loopCount(0), _lastFrameShown(-1) {
}

void TeFrameAnim::update(double millis) {
	int minFrame = MIN(_minFrame, _nbFrames);
	int maxFrame = MIN(_minFrame + _numFramesToShow, _nbFrames);
	double frameNo = _frameRate * millis / 1000.0;

	int loopsDone;
	int framesToPlay = maxFrame - minFrame;
	if (framesToPlay <= 0 && _nbFrames > 0)
		framesToPlay = _nbFrames;

	int frameToShow;
	if (framesToPlay == 0) {
		frameToShow = 0;
		loopsDone = -1;
	} else if (framesToPlay > 0) {
		loopsDone = (int)((int)frameNo / framesToPlay);
		if (!_reversed) {
			frameToShow = (int)frameNo % framesToPlay + minFrame;
		} else {
			frameToShow = (maxFrame - 1) - (int)frameNo % framesToPlay;
		}
	} else {
		// else, we don't know the total frames.. just keep asking for higher.
		loopsDone = -1;
		frameToShow = (int)frameNo;
	}

	if (_loopCount == -1 || loopsDone < _loopCount) {
		if (frameToShow == _lastFrameShown)
			return;

		_lastFrameShown = frameToShow;
		if (_frameChangedSignal.call()) {
			// got signal that we'd finished.
			if (_nbFrames == 0)
				_nbFrames = frameToShow;
		}
	} else {
		stop();
		_onFinishedSignal.call();
	}
}

} // end namespace Tetraedge
