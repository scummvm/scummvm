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

#include "titanic/game/little_lift_button.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLittleLiftButton, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CLittleLiftButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value, indent);
	CBackground::save(file, indent);
}

void CLittleLiftButton::load(SimpleFile *file) {
	file->readNumber();
	_value = file->readNumber();
	CBackground::load(file);
}

bool CLittleLiftButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	playMovie(MOVIE_NOTIFY_OBJECT);
	playSound("z#60.wav");
	return true;
}

bool CLittleLiftButton::MovieEndMsg(CMovieEndMsg *msg) {
	changeView("SecClassLittleLift.Node 1.N");

	CRoomItem *room = getRoom();
	if (room) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = _value;
		statusMsg.execute(room, nullptr, MSGFLAG_SCAN);
	}

	return true;
}

} // End of namespace Titanic
