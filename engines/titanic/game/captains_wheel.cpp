/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/game/captains_wheel.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCaptainsWheel, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

CCaptainsWheel::CCaptainsWheel() : CBackground(),
		_stopEnabled(false), _actionNum(0), _fieldE8(0),
		_cruiseEnabled(false), _goEnabled(false), _fieldF4(0) {
}

void CCaptainsWheel::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_stopEnabled, indent);
	file->writeNumberLine(_actionNum, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_cruiseEnabled, indent);
	file->writeNumberLine(_goEnabled, indent);
	file->writeNumberLine(_fieldF4, indent);

	CBackground::save(file, indent);
}

void CCaptainsWheel::load(SimpleFile *file) {
	file->readNumber();
	_stopEnabled = file->readNumber();
	_actionNum = file->readNumber();
	_fieldE8 = file->readNumber();
	_cruiseEnabled = file->readNumber();
	_goEnabled = file->readNumber();
	_fieldF4 = file->readNumber();

	CBackground::load(file);
}

bool CCaptainsWheel::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_stopEnabled) {
		_stopEnabled = false;
		CTurnOff offMsg;
		offMsg.execute(this);
		playMovie(162, 168, 0);
	} else {
		playMovie(0, 8, MOVIE_NOTIFY_OBJECT);
	}

	return true;
}

bool CCaptainsWheel::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_stopEnabled) {
		_stopEnabled = false;
		CTurnOff offMsg;
		offMsg.execute(this);
		playMovie(162, 168, MOVIE_WAIT_FOR_FINISH);
	}

	return true;
}

bool CCaptainsWheel::ActMsg(CActMsg *msg) {
	if (msg->_action == "Spin") {
		if (_stopEnabled) {
			CTurnOn onMsg;
			onMsg.execute("RatchetySound");
			playMovie(8, 142, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
	} else if (msg->_action == "Honk") {
		if (_stopEnabled) {
			playMovie(150, 160, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
	} else if (msg->_action == "Go") {
		if (_stopEnabled) {
			_goEnabled = false;
			incTransitions();
			_stopEnabled = false;
			_actionNum = 1;

			CTurnOff offMsg;
			offMsg.execute(this);
			playMovie(162, 168, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
	} else if (msg->_action == "Cruise") {
		if (_stopEnabled) {
			incTransitions();
			_stopEnabled = false;
			_actionNum = 2;

			CTurnOff offMsg;
			offMsg.execute(this);
			playMovie(162, 168, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		}
	} else if (msg->_action == "SetDestin") {
		playSound(TRANSLATE("a#44.wav", "a#37.wav"));
		CSetVolumeMsg volumeMsg;
		volumeMsg._volume = 25;
		volumeMsg.execute("EngineSounds");
		CTurnOn onMsg;
		onMsg.execute("EngineSounds");
		_goEnabled = true;
	} else if (msg->_action == "ClearDestin") {
		_goEnabled = false;
	}

	return true;
}

bool CCaptainsWheel::TurnOff(CTurnOff *msg) {
	CSignalObject signalMsg;
	signalMsg._numValue = 0;

	static const char *const NAMES[8] = {
		"WheelSpin", "SeagullHorn", "WheelStopButt", "StopHotSpot",
		"WheelCruiseButt", "CruiseHotSpot", "WheelGoButt","GoHotSpot"
	};
	for (int idx = 0; idx < 8; ++idx)
		signalMsg.execute(NAMES[idx]);

	return true;
}

bool CCaptainsWheel::TurnOn(CTurnOn *msg) {
	CSignalObject signalMsg;
	signalMsg._numValue = 1;
	signalMsg.execute("WheelSpin");
	signalMsg.execute("SeagullHorn");

	if (_stopEnabled) {
		signalMsg.execute("WheelStopButt");
		signalMsg.execute("StopHotSpot");
	}

	if (_cruiseEnabled) {
		signalMsg.execute("WheelCruiseButt");
		signalMsg.execute("CruiseHotSpot");
	}

	if (_goEnabled) {
		signalMsg.execute("WheelGoButt");
		signalMsg.execute("GoHotSpot");
	}

	return true;
}

bool CCaptainsWheel::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 8) {
		_stopEnabled = true;
		CTurnOn onMsg;
		onMsg.execute(this);
	}

	if (msg->_endFrame == 142) {
		CTurnOff offMsg;
		offMsg.execute("RatchetySound");
	}

	if (msg->_endFrame == 168) {
		switch (_actionNum) {
		case 1: {
			CActMsg actMsg(starIsSolved() ? "GoEnd" : "Go");
			actMsg.execute("GoSequence");
			break;
		}

		case 2: {
			CActMsg actMsg("Cruise");
			actMsg.execute("CruiseSequence");
			break;
		}

		default:
			break;
		}

		_actionNum = 0;
	}

	return true;
}

} // End of namespace Titanic
