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

#ifndef NANCY_ACTION_SETPLAYERCLOCK_H
#define NANCY_ACTION_SETPLAYERCLOCK_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Action record implementing an alarm clock. First used in nancy3
class SetPlayerClock : public RenderActionRecord {
public:
	enum AlarmState { kTimeMode, kAlarmMode, kWait };
	SetPlayerClock() : RenderActionRecord(7) {}
	virtual ~SetPlayerClock();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "SetPlayerClock"; }
	bool isViewportRelative() const override { return true; }

	void drawTime(uint16 hours, uint16 minutes);

	Common::String _imageName;

	Common::Rect _minutesDest;
	Common::Rect _hoursDest;
	Common::Rect _AMPMDest;
	Common::Rect _timeButtonDest;
	Common::Rect _alarmButtonDest;
	Common::Rect _setButtonDest;
	Common::Rect _cancelButtonDest;
	Common::Rect _upButtonDest;
	Common::Rect _downButtonDest;
	Common::Rect _modeLightDest;

	Common::Array<Common::Rect> _minutesSrc;
	Common::Array<Common::Rect> _hoursSrc;
	Common::Rect _AMSrc;
	Common::Rect _PMSrc;
	Common::Rect _timeButtonSrc;
	Common::Rect _alarmButtonSrc;
	Common::Rect _setButtonSrc;
	Common::Rect _cancelButtonSrc;
	Common::Rect _upButtonSrc;
	Common::Rect _downButtonSrc;
	Common::Rect _timeLightSrc;
	Common::Rect _alarmLightSrc;

	SoundDescription _buttonSound;
	SceneChangeWithFlag _alarmSetScene;
	uint16 _alarmSoundDelay;
	SoundDescription _alarmRingSound; // NO SOUND in MHM
	SceneChangeWithFlag _exitScene;

	Graphics::ManagedSurface _image;

	int8 _lastDrawnHours = -1;
	int8 _lastDrawnMinutes = -1;
	int8 _alarmHours = -1;
	bool _clearButton = true;
	Time _sceneChangeTime;

	AlarmState _alarmState = kTimeMode;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SETPLAYERCLOCK_H
