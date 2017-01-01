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
	_fieldE0(0), _fieldE4(0), _fieldE8(0), _fieldEC(0),
	_fieldF0(0), _fieldF4(0) {
}

void CCaptainsWheel::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_fieldF4, indent);

	CBackground::save(file, indent);
}

void CCaptainsWheel::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();
	_fieldF4 = file->readNumber();

	CBackground::load(file);
}

bool CCaptainsWheel::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_fieldE0) {
		_fieldE0 = false;
		CTurnOff offMsg;
		offMsg.execute(this);
		playMovie(162, 168, 0);
	} else {
		playMovie(0, 8, MOVIE_NOTIFY_OBJECT);
	}

	return true;
}

bool CCaptainsWheel::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_fieldE0) {
		_fieldE0 = false;
		CTurnOff offMsg;
		offMsg.execute(this);
		playMovie(162, 168, MOVIE_GAMESTATE);
	}

	return true;
}

bool CCaptainsWheel::ActMsg(CActMsg *msg) {
	if (msg->_action == "Spin") {
		if (_fieldE0) {
			CTurnOn onMsg;
			onMsg.execute("RatchetySound");
			playMovie(8, 142, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		}
	} else if (msg->_action == "Honk") {
		if (_fieldE0) {
			playMovie(150, 160, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		}
	} else if (msg->_action == "Go") {
		if (!_fieldE0) {
			incTransitions();
			_fieldE0 = false;
			_fieldE4 = 1;

			CTurnOff offMsg;
			offMsg.execute(this);
			playMovie(162, 168, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		}
	} else if (msg->_action == "Cruise") {
		if (_fieldE0) {
			incTransitions();
			_fieldE0 = false;
			_fieldE4 = 2;

			CTurnOff offMsg;
			offMsg.execute(this);
			playMovie(162, 168, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		}
	} else if (msg->_action == "SetDestin") {
		playSound("a#44.wav");
		CSetVolumeMsg volumeMsg;
		volumeMsg._volume = 25;
		volumeMsg.execute("EngineSounds");
		CTurnOn onMsg;
		onMsg.execute("EngineSounds");
		_fieldF0 = 1;
	} else if (msg->_action == "ClearDestin") {
		_fieldF0 = 0;
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

	if (_fieldE0) {
		signalMsg.execute("WheelStopButt");
		signalMsg.execute("StopHotSpot");
	}

	if (_fieldEC) {
		signalMsg.execute("WheelCruiseButt");
		signalMsg.execute("CruiseHotSpot");
	}

	if (_fieldF0) {
		signalMsg.execute("WheelGoButt");
		signalMsg.execute("GoHotSpot");
	}

	return true;
}

bool CCaptainsWheel::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 8) {
		_fieldE0 = true;
		CTurnOn onMsg;
		onMsg.execute(this);
	}

	if (msg->_endFrame == 142) {
		CTurnOff offMsg;
		offMsg.execute("RatchetySound");
	}

	if (msg->_endFrame == 168) {
		switch (_fieldE4) {
		case 1: {
			CActMsg actMsg(starFn2() ? "GoEnd" : "Go");
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

		_fieldE4 = 0;
	}

	return true;
}

} // End of namespace Titanic
