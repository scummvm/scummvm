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

#ifndef NANCY_ACTION_ANALOGCLOCK_H
#define NANCY_ACTION_ANALOGCLOCK_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 alarm clock (AR 196). The hands follow the player clock, and are frames of
// their own movies. Clicking the alarm hand moves it on by an hour; pressing the alarm
// button then advances the player clock to the next time the alarm hour comes around.
class AnalogClock : public PuzzleRecord {
public:
	AnalogClock() : PuzzleRecord(7) {}
	virtual ~AnalogClock() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "AnalogClock"; }

	// The alarm is disabled without a value table entry for it
	static const int16 kNoAlarm = -4;

	enum HandType {
		kHourHand,
		kMinuteHand,
		kAlarmHand,
		kNumHands
	};

	struct Hand {
		Common::Rect dest;
		Common::Path movieName;

		MoviePlayer movie;
		Graphics::ManagedSurface frameSurface;
		int frame = -1;
	};

	bool hasAlarm() const { return _alarmIndex != kNoAlarm; }
	int16 getAlarmHour() const;
	void drawHand(Hand &hand, int frame);
	void pressAlarmButton();

	void redraw();

	// -- File data --
	int16 _alarmIndex = kNoAlarm;		// value table entry holding the alarm hour
	Hand _hands[kNumHands];
	uint16 _alarmHandCursor = 0;
	RandomSoundBlock _alarmHandSound;
	RandomSoundBlock _tickSound;
	Common::Rect _buttonDownSrc;
	Common::Rect _buttonDest;
	uint16 _buttonCursor = 0;
	RandomSoundBlock _buttonSoundBlock;

	// -- Runtime state --
	int _minuteFrame = 0;
	bool _isButtonPressed = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ANALOGCLOCK_H
