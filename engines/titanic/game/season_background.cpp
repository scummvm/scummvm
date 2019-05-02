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
#include "titanic/translation.h"

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
		playMovie(0, TRANSLATE(45, 49), MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		_defaultFrame = TRANSLATE(45, 49);
		break;

	case SEASON_AUTUMN:
		if (_flag) {
			playMovie(TRANSLATE(232, 49), TRANSLATE(278, 98),
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_defaultFrame = TRANSLATE(278, 98);
		} else {
			playMovie(TRANSLATE(45, 196), TRANSLATE(91, 245),
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_defaultFrame = TRANSLATE(91, 245);
		}
		break;

	case SEASON_WINTER:
		if (_flag) {
			playMovie(TRANSLATE(278, 98), TRANSLATE(326, 147),
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_defaultFrame = TRANSLATE(326, 147);
		} else {
			CStatusChangeMsg changeMsg;
			changeMsg._newStatus = 0;
			changeMsg.execute("PickUpSpeechCentre");
			playMovie(TRANSLATE(91, 245), TRANSLATE(139, 294),
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_defaultFrame = TRANSLATE(139, 294);
		}
		break;

	case SEASON_SPRING:
		if (_flag) {
			playMovie(TRANSLATE(326, 147), TRANSLATE(417, 195),
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_defaultFrame = TRANSLATE(417, 195);
		} else {
			playMovie(TRANSLATE(139, 294), TRANSLATE(228, 342),
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			_defaultFrame = TRANSLATE(228, 342);
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

	if (msg->_endFrame == TRANSLATE(91, 245) && !_flag) {
		CStatusChangeMsg changeMsg;
		changeMsg.execute("PickUpSpeechCentre");
	}

	return true;
}

bool CSeasonBackground::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsSpeechCentre") {
		_defaultFrame = TRANSLATE(278, 98);
		loadFrame(_defaultFrame);
		_flag = true;
	}

	return true;
}

} // End of namespace Titanic
