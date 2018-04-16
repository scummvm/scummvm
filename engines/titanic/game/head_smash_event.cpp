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

#include "titanic/game/head_smash_event.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHeadSmashEvent, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CHeadSmashEvent::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CBackground::save(file, indent);
}

void CHeadSmashEvent::load(SimpleFile *file) {
	file->readNumber();
	CBackground::load(file);
}

bool CHeadSmashEvent::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayToEnd") {
		setVisible(true);
		playMovie(MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	}

	return true;
}

bool CHeadSmashEvent::MovieEndMsg(CMovieEndMsg *msg) {
	changeView("CreatorsChamber.Node 1.W");
	return true;
}

} // End of namespace Titanic
