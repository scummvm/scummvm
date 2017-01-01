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

#include "titanic/game/season_background.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSeasonBackground, CBackground)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CSeasonBackground::CSeasonBackground() : CBackground(),
	_seasonNum(SEASON_SUMMER), _flag(false), _defaultFrame(46), _unused(0) {
}

void CSeasonBackground::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_seasonNum, indent);
	file->writeNumberLine(_flag, indent);
	file->writeNumberLine(_defaultFrame, indent);
	file->writeNumberLine(_unused, indent);

	CBackground::save(file, indent);
}

void CSeasonBackground::load(SimpleFile *file) {
	file->readNumber();
	_seasonNum = (Season)file->readNumber();
	_flag = file->readNumber();
	_defaultFrame = file->readNumber();
	_unused = file->readNumber();

	CBackground::load(file);
}

bool CSeasonBackground::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(_defaultFrame);
	return true;
}

bool CSeasonBackground::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_seasonNum = (Season)(((int)_seasonNum + 1) % 4);

	switch (_seasonNum) {
	case SEASON_SUMMER:
		playMovie(0, 45, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
		_defaultFrame = 45;
		break;

	case SEASON_AUTUMN:
		if (_flag) {
			playMovie(232, 278, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_defaultFrame = 278;
		} else {
			playMovie(45, 91, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_defaultFrame = 91;
		}
		break;

	case SEASON_WINTER:
		if (_flag) {
			playMovie(278, 326, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_defaultFrame = 326;
		} else {
			CStatusChangeMsg changeMsg;
			changeMsg._newStatus = 0;
			changeMsg.execute("PickUpSpeechCentre");
			playMovie(91, 139, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_defaultFrame = 139;
		}
		break;

	case SEASON_SPRING:
		if (_flag) {
			playMovie(326, 417, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_defaultFrame = 417;
		} else {
			playMovie(139, 228, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			_defaultFrame = 228;
		}
		break;

	default:
		break;
	}

	return true;
}

bool CSeasonBackground::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == _defaultFrame) {
		CTurnOn onMsg;
		onMsg.execute("SeasonalAdjust");
	}

	if (msg->_endFrame == 91 && !_flag) {
		CStatusChangeMsg changeMsg;
		changeMsg.execute("PickUpSpeechCentre");
	}

	return true;
}

bool CSeasonBackground::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsSpeechCentre") {
		loadFrame(278);
		_defaultFrame = 278;
		_flag = true;
	}

	return true;
}

} // End of namespace Titanic
