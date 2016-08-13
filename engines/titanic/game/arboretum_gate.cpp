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
	_winterOffStartFrame = 244;
	_winterOffEndFrame = 304;
	_springOffStartFrame = 122;
	_springOffEndFrame = 182;
	_summerOffStartFrame1 = 183;
	_summerOffEndFrame1 = 243;
	_summerOffStartFrame2 = 665;
	_summerOffEndFrame2 = 724;
	_autumnOffStartFrame1 = 61;
	_autumnOffEndFrame1 = 121;
	_autumnOffStartFrame2 = 0;
	_autumnOffEndFrame2 = 60;
	_winterOnStartFrame = 485;
	_winterOnEndFrame = 544;
	_springOnStartFrame = 425;
	_springOnEndFrame = 484;
	_summerOnStartFrame1 = 545;
	_summerOnEndFrame1 = 604;
	_summerOnStartFrame2 = 605;
	_summerOnEndFrame2 = 664;
	_autumnOnStartFrame1 = 305;
	_autumnOnEndFrame1 = 364;
	_autumnOnStartFrame2 = 365;
	_autumnOnEndFrame2 = 424;
}

void CArboretumGate::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_seasonNum, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_initialFrame, indent);
	file->writeNumberLine(_v3, indent);
	file->writeQuotedLine(_viewName1, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_winterOffStartFrame, indent);
	file->writeNumberLine(_winterOffEndFrame, indent);
	file->writeNumberLine(_springOffStartFrame, indent);
	file->writeNumberLine(_springOffEndFrame, indent);
	file->writeNumberLine(_summerOffStartFrame1, indent);
	file->writeNumberLine(_summerOffEndFrame1, indent);
	file->writeNumberLine(_summerOffStartFrame2, indent);
	file->writeNumberLine(_summerOffEndFrame2, indent);
	file->writeNumberLine(_autumnOffStartFrame1, indent);
	file->writeNumberLine(_autumnOffEndFrame1, indent);
	file->writeNumberLine(_autumnOffStartFrame2, indent);
	file->writeNumberLine(_autumnOffEndFrame2, indent);
	file->writeNumberLine(_winterOnStartFrame, indent);
	file->writeNumberLine(_winterOnEndFrame, indent);
	file->writeNumberLine(_springOnStartFrame, indent);
	file->writeNumberLine(_springOnEndFrame, indent);
	file->writeNumberLine(_summerOnStartFrame1, indent);
	file->writeNumberLine(_summerOnEndFrame1, indent);
	file->writeNumberLine(_summerOnStartFrame2, indent);
	file->writeNumberLine(_summerOnEndFrame2, indent);
	file->writeNumberLine(_autumnOnStartFrame1, indent);
	file->writeNumberLine(_autumnOnEndFrame1, indent);
	file->writeNumberLine(_autumnOnStartFrame2, indent);
	file->writeNumberLine(_autumnOnEndFrame2, indent);
	file->writeQuotedLine(_viewName2, indent);

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
	_winterOffStartFrame = file->readNumber();
	_winterOffEndFrame = file->readNumber();
	_springOffStartFrame = file->readNumber();
	_springOffEndFrame = file->readNumber();
	_summerOffStartFrame1 = file->readNumber();
	_summerOffEndFrame1 = file->readNumber();
	_summerOffStartFrame2 = file->readNumber();
	_summerOffEndFrame2 = file->readNumber();
	_autumnOffStartFrame1 = file->readNumber();
	_autumnOffEndFrame1 = file->readNumber();
	_autumnOffStartFrame2 = file->readNumber();
	_autumnOffEndFrame2 = file->readNumber();
	_winterOnStartFrame = file->readNumber();
	_winterOnEndFrame = file->readNumber();
	_springOnStartFrame = file->readNumber();
	_springOnEndFrame = file->readNumber();
	_summerOnStartFrame1 = file->readNumber();
	_summerOnEndFrame1 = file->readNumber();
	_summerOnStartFrame2 = file->readNumber();
	_summerOnEndFrame2 = file->readNumber();
	_autumnOnStartFrame1 = file->readNumber();
	_autumnOnEndFrame1 = file->readNumber();
	_autumnOnStartFrame2 = file->readNumber();
	_autumnOnEndFrame2 = file->readNumber();
	_viewName2 = file->readString();

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
		case SPRING:
			playMovie(_springOffStartFrame, _springOffEndFrame, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			break;

		case SUMMER:
			if (_v1) {
				playMovie(_summerOffStartFrame2, _summerOffEndFrame2, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_summerOffStartFrame1, _summerOffEndFrame1, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			}
			break;

		case AUTUMN:
			if (_v1) {
				playMovie(_autumnOffStartFrame2, _autumnOffEndFrame2, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_autumnOffStartFrame1, _autumnOffEndFrame1, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			}
			break;

		case WINTER:
			playMovie(_winterOffStartFrame, _winterOffEndFrame, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
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
		case SPRING:
			playMovie(_springOnStartFrame, _springOnEndFrame, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			break;

		case SUMMER:
			if (_v1) {
				playMovie(_summerOnStartFrame2, _summerOnEndFrame2, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_summerOnStartFrame1, _summerOnEndFrame1, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			}
			break;

		case AUTUMN:
			if (_v1) {
				playMovie(_autumnOnStartFrame2, _autumnOnEndFrame2, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			} else {
				playMovie(_autumnOnStartFrame1, _autumnOnEndFrame1, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
			}
			break;

		case WINTER:
			playMovie(_winterOnStartFrame, _winterOnEndFrame, MOVIE_GAMESTATE || MOVIE_NOTIFY_OBJECT);
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
		case SPRING:
			_initialFrame = _springOffStartFrame;
			break;

		case SUMMER:
			_initialFrame = _v1 ? _summerOffStartFrame2 : _summerOffStartFrame1;
			break;

		case AUTUMN:
			_initialFrame = _v1 ? _autumnOffStartFrame1 : _autumnOffStartFrame2;
			break;

		case WINTER:
			_initialFrame = _winterOffStartFrame;
			break;

		default:
			break;
		}

		loadFrame(_initialFrame);
	}

	return true;
}

} // End of namespace Titanic
