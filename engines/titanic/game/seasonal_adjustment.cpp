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

#include "titanic/game/seasonal_adjustment.h"
#include "titanic/core/project_item.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSeasonalAdjustment, CBackground)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

void CSeasonalAdjustment::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_switching, indent);
	file->writeNumberLine(_enabled, indent);

	CBackground::save(file, indent);
}

void CSeasonalAdjustment::load(SimpleFile *file) {
	file->readNumber();
	_switching = file->readNumber();
	_enabled = file->readNumber();

	CBackground::load(file);
}

bool CSeasonalAdjustment::StatusChangeMsg(CStatusChangeMsg *msg) {
	CChangeSeasonMsg changeMsg;
	switch (stateGetSeason()) {
	case SEASON_SUMMER:
		changeMsg._season = "Summer";
		break;
	case SEASON_AUTUMN:
		changeMsg._season = "Autumn";
		break;
	case SEASON_WINTER:
		changeMsg._season = "Winter";
		break;
	case SEASON_SPRING:
		changeMsg._season = "Spring";
		break;
	default:
		break;
	}

	changeMsg.execute(getRoot(), nullptr, MSGFLAG_SCAN);
	return true;
}

bool CSeasonalAdjustment::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CSeasonalAdjustment::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	playSound(TRANSLATE("z#42.wav", "z#743.wav"));
	if (!_enabled) {
		petDisplayMessage(1, SEASONAL_SWITCH_NOT_WORKING);
	} else if (!_switching) {
		playMovie(0, 6, MOVIE_NOTIFY_OBJECT);
		playMovie(6, 18, 0);
	}

	return true;
}

bool CSeasonalAdjustment::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 6) {
		stateChangeSeason();
		CStatusChangeMsg changeMsg;
		changeMsg.execute(this);
		CTurnOff offMsg;
		offMsg.execute(this);
		offMsg.execute("LeftPanExit");
		offMsg.execute("RightPanExit");
	}

	return true;
}

bool CSeasonalAdjustment::TurnOn(CTurnOn *msg) {
	if (_switching) {
		_switching = false;
		CTurnOn onMsg;
		onMsg.execute("LeftPanExit");
		onMsg.execute("RightPanExit");
	}

	return true;
}

bool CSeasonalAdjustment::TurnOff(CTurnOff *msg) {
	_switching = true;
	return true;
}

bool CSeasonalAdjustment::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsSpeechCentre") {
		msg->execute("SeasonBackground");
		msg->execute("ArbGate");
	} else if (msg->_action == "EnableObject") {
		_enabled = true;
	} else if (msg->_action == "DisableObject") {
		_enabled = false;
	}

	return true;
}

} // End of namespace Titanic
