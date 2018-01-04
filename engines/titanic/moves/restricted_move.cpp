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

#include "titanic/moves/restricted_move.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CRestrictedMove, CMovePlayerTo)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CRestrictedMove::CRestrictedMove() : CMovePlayerTo(), _classNum(0) {
}

void CRestrictedMove::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_classNum, indent);

	CMovePlayerTo::save(file, indent);
}

void CRestrictedMove::load(SimpleFile *file) {
	file->readNumber();
	_classNum = file->readNumber();

	CMovePlayerTo::load(file);
}

bool CRestrictedMove::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	int classNum = getPassengerClass();
	if (classNum <= _classNum) {
		// Okay to change to the given destination
		changeView(_destination);
	} else if (classNum != UNCHECKED) {
		petDisplayMessage(1, CLASS_NOT_ALLOWED_AT_DEST);
	} else if (compareRoomNameTo("EmbLobby")) {
		if (g_language != Common::DE_DEU)
			playSound("a#17.wav");
		petDisplayMessage(1, CHECK_IN_AT_RECEPTION);
	} else if (compareViewNameTo("Titania.Node 1.S")) {
		CProximity prox(Audio::Mixer::kSpeechSoundType);
		playSound(TRANSLATE("z#226.wav", "z#132.wav"), prox);
		changeView(_destination);
	}

	return true;
}

bool CRestrictedMove::EnterViewMsg(CEnterViewMsg *msg) {
	int classNum = getPassengerClass();
	bool flag = classNum <= _classNum;

	if (classNum == UNCHECKED) {
		if (compareRoomNameTo("EmbLobby"))
			flag = false;
		else if (compareViewNameTo("Titania.Node 1.S"))
			flag = true;
	}

	_cursorId = flag ? CURSOR_MOVE_FORWARD : CURSOR_INVALID;
	return true;
}

} // End of namespace Titanic
