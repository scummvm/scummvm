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

#include "titanic/game/bridge_door.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBridgeDoor, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CBridgeDoor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CBridgeDoor::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CBridgeDoor::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	setVisible(true);
	playMovie(0, 6, 0);
	changeView("Titania.Node 12.N");

	return true;
}

bool CBridgeDoor::StatusChangeMsg(CStatusChangeMsg *msg) {
	setVisible(true);
	playMovie(7, 0, MOVIE_NOTIFY_OBJECT);
	return true;
}

bool CBridgeDoor::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	return true;
}

} // End of namespace Titanic
