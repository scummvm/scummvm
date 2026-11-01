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

#ifndef NANCY_SCRIPT_TIMER_H
#define NANCY_SCRIPT_TIMER_H

#include "engines/nancy/script/record.h"

namespace Nancy {
namespace Script {

// The frame counter of the original. It goes through a range of frames at a
// rate, once per play, for a number of plays. The current frame follows the
// elapsed time, and a play is over once the time goes past the last frame.
// Inside, frames are counted from the first frame of the range. A last frame
// of -1 means that a play never ends.
class PlayCounter {
public:
	void setRange(int firstFrame, int lastFrame, int plays, int maxPlays);
	void advance(double seconds);
	void restart();

	void setRate(double framesPerSecond);
	void setFrame(int frame);
	void setPlays(int plays);

	double getRate() const { return _rate; }
	int getFrame() const { return _frame + _firstFrame; }
	double getPosition() const { return _position + _firstFrame; }
	int getFirstFrame() const { return _firstFrame; }
	int getLastFrame() const { return _lastFrame + _firstFrame; }
	int getNumFrames() const { return _lastFrame == -1 ? 0 : _lastFrame + 1; }
	int getPlays() const { return _plays; }
	int getMaxPlays() const { return _maxPlays; }

	// The time all plays take together, and the time that has gone by in the
	// plays so far
	double getDuration() const;
	double getElapsed() const;

	// Whether all plays are over. A counter without a maximum number of plays
	// never ends.
	bool hasEnded() const { return _hasEnded && _maxPlays != 0 && _plays == _maxPlays; }

private:
	void startPlay(int lastFrame);
	void advancePlay(double seconds);
	double getPlayDuration() const;

	double _time = 0.0;			// of the current play
	double _leftover = 0.0;		// time that has not made a whole frame yet
	double _rate = 0.0;
	double _period = 0.0;
	int _frame = 0;
	double _position = 0.0;		// the frame, plus the fraction of it gone by
	int _lastFrame = -1;
	bool _hasEnded = false;		// the current play

	int _firstFrame = -1;
	int _plays = 1;
	int _maxPlays = 0;			// 0 for no end
};

// Nancy24: a record that waits, and calls its OnDone when it is over. A timer
// is over after a fixed duration, after a random duration between minDuration
// and maxDuration, or once its frame counter has finished all its plays.
// Scripts can start a timer over with new values, by passing them to its Reset
// or Restart.
class TimerRecord : public Record {
public:
	TimerRecord(SceneContext &context);

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	bool prepareReset() override;
	bool isPaused() const override { return _isPaused; }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	bool hasDuration() const { return _minDuration != -1.0 || _maxDuration != -1.0; }

	// Picks the duration, and sets the counter to a single frame that lasts
	// that long
	void pickDuration();

	PlayCounter _counter;
	double _minDuration = -1.0;
	double _maxDuration = -1.0;
	bool _isPaused = false;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_TIMER_H
