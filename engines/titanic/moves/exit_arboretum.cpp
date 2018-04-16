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

#include "titanic/moves/exit_arboretum.h"
#include "titanic/game/seasonal_adjustment.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CExitArboretum, CMovePlayerTo)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

CExitArboretum::CExitArboretum() : CMovePlayerTo(),
		_seasonNum(SEASON_SUMMER), _exitDirection(0), _enabled(true) {
}

void CExitArboretum::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_seasonNum, indent);
	file->writeNumberLine(_exitDirection, indent);
	file->writeNumberLine(_enabled, indent);

	CMovePlayerTo::save(file, indent);
}

void CExitArboretum::load(SimpleFile *file) {
	file->readNumber();
	_seasonNum = (Season)file->readNumber();
	_exitDirection = file->readNumber();
	_enabled = file->readNumber();

	CMovePlayerTo::load(file);
}

bool CExitArboretum::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_enabled) {
		CActMsg actMsg;
		if (_seasonNum == SEASON_WINTER) {
			switch (_exitDirection) {
			case 0:
				actMsg._action = "ExitLFrozen";
				break;
			case 1:
				actMsg._action = "ExitRFrozen";
				break;
			default:
				break;
			}
		} else {
			switch (_exitDirection) {
			case 0:
				actMsg._action = "ExitLNormal";
				break;
			case 1:
				actMsg._action = "ExitRNormal";
				break;
			default:
				break;
			}
		}

		actMsg.execute("ArbGate");
	}

	return true;
}

bool CExitArboretum::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_seasonNum = (Season)(((int)_seasonNum + 1) % 4);
	return true;
}

bool CExitArboretum::TurnOn(CTurnOn *msg) {
	_enabled = true;
	return true;
}

bool CExitArboretum::TurnOff(CTurnOff *msg) {
	_enabled = false;
	return true;
}

} // End of namespace Titanic
