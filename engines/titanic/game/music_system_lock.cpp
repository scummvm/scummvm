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

#include "titanic/game/music_system_lock.h"
#include "titanic/core/room_item.h"
#include "titanic/carry/carry.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMusicSystemLock, CDropTarget)
	ON_MESSAGE(DropObjectMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CMusicSystemLock::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value, indent);
	CDropTarget::save(file, indent);
}

void CMusicSystemLock::load(SimpleFile *file) {
	file->readNumber();
	_value = file->readNumber();
	CDropTarget::load(file);
}

bool CMusicSystemLock::DropObjectMsg(CDropObjectMsg *msg) {
	CTreeItem *key = msg->_item->findByName("Music System Key");
	if (key) {
		setVisible(true);
		playMovie(MOVIE_NOTIFY_OBJECT);
	}

	return false;
}

bool CMusicSystemLock::MovieEndMsg(CMovieEndMsg *msg) {
	CTreeItem *phonograph = findRoom()->findByName("Restaurant Phonograph");

	// Toggle the locked status of the music system
	CQueryPhonographState queryMsg;
	queryMsg.execute(phonograph);
	CLockPhonographMsg lockMsg(!queryMsg._value);
	lockMsg.execute(phonograph, nullptr, MSGFLAG_SCAN);

	setVisible(false);
	return true;
}

} // End of namespace Titanic
