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

#include "titanic/game/arboretum_gate.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CArboretumGate, CBackground)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TurnOn)
END_MESSAGE_MAP()

int CArboretumGate::_v1;
int CArboretumGate::_initialFrame;
int CArboretumGate::_v3;

CArboretumGate::CArboretumGate() : CBackground() {
	_viewName1 = "NULL";
	_viewName2 = "NULL";
	_seasonNum = 0;
	_fieldF0 = 0;
	_startFrameSpringOff = 244;
	_endFrameSpringOff = 304;
	_startFrameSummerOff = 122;
	_endFrameSummerOff = 182;
	_startFrameAutumnOff1 = 183;
	_endFrameAutumnOff1 = 243;
	_startFrameAutumnOff2 = 665;
	_endFrameAutumnOff2 = 724;
	_startFrameWinterOff1 = 61;
	_endFrameWinterOff1 = 121;
	_startFrameWinterOff2 = 0;
	_endFrameWinterOff2 = 60;
	_startFrameSpringOn = 485;
	_endFrameSpringOn = 544;
	_startFrameSummerOn = 425;
	_endFrameSummerOn = 484;
	_startFrameAutumnOn1 = 545;
	_endFrameAutumnOn1 = 604;
	_startFrameAutumnOn2 = 605;
	_endFrameAutumnOn2 = 664;
	_startFrameWinterOn1 = 305;
	_endFrameWinterOn1 = 364;
	_startFrameWinterOn2 = 365;
	_endFrameWinterOn2 = 424;

	// German specific fields
	_field160 = _field164 = _field168 = _field16C = 0;
	_field170 = _field174 = _field178 = _field17C = 0;
	_field180 = _field184 = _field188 = _field18C = 0;
	_field190 = _field194 = _field198 = _field19C = 0;
	_field1A0 = _field1A4 = _field1A8 = _field1AC = 0;
	_field1B0 = _field1B4 = _field1B8 = _field1BC = 0;
}

void CArboretumGate::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_seasonNum, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_initialFrame, indent);
	file->writeNumberLine(_v3, indent);
	file->writeQuotedLine(_viewName1, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_startFrameSpringOff, indent);
	file->writeNumberLine(_endFrameSpringOff, indent);
	file->writeNumberLine(_startFrameSummerOff, indent);
	file->writeNumberLine(_endFrameSummerOff, indent);
	file->writeNumberLine(_startFrameAutumnOff1, indent);
	file->writeNumberLine(_endFrameAutumnOff1, indent);
	file->writeNumberLine(_startFrameAutumnOff2, indent);
	file->writeNumberLine(_endFrameAutumnOff2, indent);
	file->writeNumberLine(_startFrameWinterOff1, indent);
	file->writeNumberLine(_endFrameWinterOff1, indent);
	file->writeNumberLine(_startFrameWinterOff2, indent);
	file->writeNumberLine(_endFrameWinterOff2, indent);
	file->writeNumberLine(_startFrameSpringOn, indent);
	file->writeNumberLine(_endFrameSpringOn, indent);
	file->writeNumberLine(_startFrameSummerOn, indent);
	file->writeNumberLine(_endFrameSummerOn, indent);
	file->writeNumberLine(_startFrameAutumnOn1, indent);
	file->writeNumberLine(_endFrameAutumnOn1, indent);
	file->writeNumberLine(_startFrameAutumnOn2, indent);
	file->writeNumberLine(_endFrameAutumnOn2, indent);
	file->writeNumberLine(_startFrameWinterOn1, indent);
	file->writeNumberLine(_endFrameWinterOn1, indent);
	file->writeNumberLine(_startFrameWinterOn2, indent);
	file->writeNumberLine(_endFrameWinterOn2, indent);
	file->writeQuotedLine(_viewName2, indent);

	if (g_vm->isGerman()) {
		file->writeNumberLine(_field160, indent);
		file->writeNumberLine(_field164, indent);
		file->writeNumberLine(_field168, indent);
		file->writeNumberLine(_field16C, indent);
		file->writeNumberLine(_field170, indent);
		file->writeNumberLine(_field174, indent);
		file->writeNumberLine(_field178, indent);
		file->writeNumberLine(_field17C, indent);
		file->writeNumberLine(_field180, indent);
		file->writeNumberLine(_field184, indent);
		file->writeNumberLine(_field188, indent);
		file->writeNumberLine(_field18C, indent);
		file->writeNumberLine(_field190, indent);
		file->writeNumberLine(_field194, indent);
		file->writeNumberLine(_field198, indent);
		file->writeNumberLine(_field19C, indent);
		file->writeNumberLine(_field1A0, indent);
		file->writeNumberLine(_field1A4, indent);
		file->writeNumberLine(_field1A8, indent);
		file->writeNumberLine(_field1AC, indent);
		file->writeNumberLine(_field1B0, indent);
		file->writeNumberLine(_field1B4, indent);
		file->writeNumberLine(_field1B8, indent);
		file->writeNumberLine(_field1BC, indent);
	}

	CBackground::save(file, indent);
}

void CArboretumGate::load(SimpleFile *file) {
	file->readNumber();
	_seasonNum = file->readNumber();
	_v1 = file->readNumber();
	_initialFrame = file->readNumber();
	_v3 = file->readNumber();
	_viewName1 = file->readString();
	_fieldF0 = file->readNumber();
	_startFrameSpringOff = file->readNumber();
	_endFrameSpringOff = file->readNumber();
	_startFrameSummerOff = file->readNumber();
	_endFrameSummerOff = file->readNumber();
	_startFrameAutumnOff1 = file->readNumber();
	_endFrameAutumnOff1 = file->readNumber();
	_startFrameAutumnOff2 = file->readNumber();
	_endFrameAutumnOff2 = file->readNumber();
	_startFrameWinterOff1 = file->readNumber();
	_endFrameWinterOff1 = file->readNumber();
	_startFrameWinterOff2 = file->readNumber();
	_endFrameWinterOff2 = file->readNumber();
	_startFrameSpringOn = file->readNumber();
	_endFrameSpringOn = file->readNumber();
	_startFrameSummerOn = file->readNumber();
	_endFrameSummerOn = file->readNumber();
	_startFrameAutumnOn1 = file->readNumber();
	_endFrameAutumnOn1 = file->readNumber();
	_startFrameAutumnOn2 = file->readNumber();
	_endFrameAutumnOn2 = file->readNumber();
	_startFrameWinterOn1 = file->readNumber();
	_endFrameWinterOn1 = file->readNumber();
	_startFrameWinterOn2 = file->readNumber();
	_endFrameWinterOn2 = file->readNumber();
	_viewName2 = file->readString();

	if (g_vm->isGerman()) {
		_field160 = file->readNumber();
		_field164 = file->readNumber();
		_field168 = file->readNumber();
		_field16C = file->readNumber();
		_field170 = file->readNumber();
		_field174 = file->readNumber();
		_field178 = file->readNumber();
		_field17C = file->readNumber();
		_field180 = file->readNumber();
		_field184 = file->readNumber();
		_field188 = file->readNumber();
		_field18C = file->readNumber();
		_field190 = file->readNumber();
		_field194 = file->readNumber();
		_field198 = file->readNumber();
		_field19C = file->readNumber();
		_field1A0 = file->readNumber();
		_field1A4 = file->readNumber();
		_field1A8 = file->readNumber();
		_field1AC = file->readNumber();
		_field1B0 = file->readNumber();
		_field1B4 = file->readNumber();
		_field1B8 = file->readNumber();
		_field1BC = file->readNumber();
	}

	CBackground::load(file);
}

bool CArboretumGate::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_seasonNum = (_seasonNum + 1) % 4;
	return true;
}

bool CArboretumGate::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsSpeechCentre") {
		_v1 = 1;
		CVisibleMsg visibleMsg(true);
		visibleMsg.execute("SpCtrOverlay");
	} else if (msg->_action == "ExitLFrozen") {
		if (_v3) {
			_viewName2 = "FrozenArboretum.Node 2.W";
			CTurnOn onMsg;
			onMsg.execute(this);
		} else {
			changeView("FrozenArboretum.Node 2.W");
		}
	} else if (msg->_action == "ExitRFrozen") {
		if (_v3) {
			_viewName2 = "FrozenArboretum.Node 2.E";
			CTurnOn onMsg;
			onMsg.execute(this);
		} else {
			changeView("FrozenArboretum.Node 2.E");
		}
	} else if (msg->_action == "ExitLNormal") {
		if (_v3) {
			_viewName2 = "Arboretum.Node 2.W";
			CTurnOn onMsg;
			onMsg.execute(this);
		} else {
			changeView("Arboretum.Node 2.W");
		}
	} else if (msg->_action == "ExitRNormal") {
		if (_v3) {
			_viewName2 = "Arboretum.Node 2.E";
			CTurnOn onMsg;
			onMsg.execute(this);
		}
		else {
			changeView("Arboretum.Node 2.E");
		}
	}

	return true;
}

bool CArboretumGate::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(!_v3);

	if (_viewName1 != "NULL") {
		changeView(_viewName1);
	} else if (_viewName2 != "NULL") {
		changeView(_viewName2);
		_viewName2 = "NULL";
	}

	return true;
}

bool CArboretumGate::LeaveViewMsg(CLeaveViewMsg *msg) {
	return false;
}

bool CArboretumGate::TurnOff(CTurnOff *msg) {
	if (!_v3) {
		switch (_seasonNum) {
		case SEASON_SUMMER:
			playMovie(_startFrameSummerOff, _endFrameSummerOff, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			break;

		case SEASON_AUTUMN:
			if (_v1) {
				playMovie(_startFrameAutumnOff2, _endFrameAutumnOff2, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_startFrameAutumnOff1, _endFrameAutumnOff1, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			}
			break;

		case SEASON_WINTER:
			if (_v1) {
				playMovie(_startFrameWinterOff2, _endFrameWinterOff2, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_startFrameWinterOff1, _endFrameWinterOff1, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			}
			break;

		case SEASON_SPRING:
			playMovie(_startFrameSpringOff, _endFrameSpringOff, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			break;

		default:
			break;
		}

		_v3 = 1;
		CArboretumGateMsg gateMsg;
		gateMsg.execute("Arboretum", nullptr, MSGFLAG_SCAN);
	}

	return true;
}

bool CArboretumGate::TurnOn(CTurnOn *msg) {
	if (_v3) {
		CArboretumGateMsg gateMsg(0);
		gateMsg.execute("Arboretum");
		setVisible(true);

		switch (_seasonNum) {
		case SEASON_SUMMER:
			playMovie(_startFrameSummerOn, _endFrameSummerOn, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			break;

		case SEASON_AUTUMN:
			if (_v1) {
				playMovie(_startFrameAutumnOn2, _endFrameAutumnOn2, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_startFrameAutumnOn1, _endFrameAutumnOn1, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			}
			break;

		case SEASON_WINTER:
			if (_v1) {
				playMovie(_startFrameWinterOn2, _endFrameWinterOn2, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_startFrameWinterOn1, _endFrameWinterOn1, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			}
			break;

		case SEASON_SPRING:
			playMovie(_startFrameSpringOn, _endFrameSpringOn, MOVIE_GAMESTATE | MOVIE_NOTIFY_OBJECT);
			break;

		default:
			break;
		}

		_v3 = 0;
	}

	return true;
}

bool CArboretumGate::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_v3) {
		CTurnOff offMsg;
		offMsg.execute(this);
	}

	return true;
}

bool CArboretumGate::EnterViewMsg(CEnterViewMsg *msg) {
	if (!_v3) {
		switch (_seasonNum) {
		case SEASON_SUMMER:
			_initialFrame = _startFrameSummerOff;
			break;

		case SEASON_AUTUMN:
			_initialFrame = _v1 ? _startFrameAutumnOff2 : _startFrameAutumnOff1;
			break;

		case SEASON_WINTER:
			_initialFrame = _v1 ? _startFrameWinterOff1 : _startFrameWinterOff2;
			break;

		case SEASON_SPRING:
			_initialFrame = _startFrameSpringOff;
			break;

		default:
			break;
		}

		loadFrame(_initialFrame);
	}

	return true;
}

} // End of namespace Titanic
