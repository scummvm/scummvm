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

#include "titanic/npcs/callbot.h"
#include "titanic/core/room_item.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCallBot, CGameObject)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CCallBot::CCallBot() : CGameObject(), _enabled(0) {
}

void CCallBot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_npcName, indent);
	file->writeNumberLine(_enabled, indent);

	CGameObject::save(file, indent);
}

void CCallBot::load(SimpleFile *file) {
	file->readNumber();
	_npcName = file->readString();
	_enabled = file->readNumber();

	CGameObject::load(file);
}

bool CCallBot::TurnOn(CTurnOn *msg) {
	_enabled = true;
	return true;
}

bool CCallBot::EnterViewMsg(CEnterViewMsg *msg) {
	if (_enabled) {
		CRoomItem *room = getRoom();

		if (room) {
			CGameState &gs = getGameManager()->_gameState;
			gs.setMode(GSMODE_CUTSCENE);

			CSummonBotQueryMsg queryMsg;
			queryMsg._npcName = _npcName;
			if (queryMsg.execute(room))
				petOnSummonBot(_npcName, 0);

			gs.setMode(GSMODE_INTERACTIVE);
		}

		_enabled = false;
	}

	return true;
}

} // End of namespace Titanic
