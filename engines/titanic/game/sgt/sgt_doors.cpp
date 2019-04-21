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

#include "titanic/game/sgt/sgt_doors.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSGTDoors, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(LeaveRoomMsg)
END_MESSAGE_MAP()

void CSGTDoors::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value1, indent);
	file->writeNumberLine(_open, indent);

	CGameObject::save(file, indent);
}

void CSGTDoors::load(SimpleFile *file) {
	file->readNumber();
	_value1 = file->readNumber();
	_open = file->readNumber();

	CGameObject::load(file);
}

bool CSGTDoors::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	_open = true;
	CPetControl *pet = getPetControl();

	if (pet) {
		int roomNum = pet->getRoomsRoomNum();
		static const int START_FRAMES[7] = { 0, 26, 30, 34, 38, 42, 46 };
		static const int END_FRAMES[7] = { 12, 29, 33, 37, 41, 45, 49 };

		if (pet->getRoomsSublevel() == 1)
			playMovie(START_FRAMES[roomNum], END_FRAMES[roomNum],
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		else
			playMovie(0, 12, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	}

	return true;
}

bool CSGTDoors::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CSGTDoors::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(!_open);
	return true;
}

bool CSGTDoors::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	setVisible(true);
	_open = false;
	CPetControl *pet = getPetControl();

	if (pet) {
		int roomNum = pet->getRoomsRoomNum();
		static const int START_FRAMES[7] = { 12, 69, 65, 61, 57, 53, 49 };
		static const int END_FRAMES[7] = { 25, 72, 68, 64, 60, 56, 52 };

		if (pet->getRoomsSublevel() == 1)
			playMovie(START_FRAMES[roomNum], END_FRAMES[roomNum],
				MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		else
			playMovie(12, 25, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	}

	return true;
}

} // End of namespace Titanic
