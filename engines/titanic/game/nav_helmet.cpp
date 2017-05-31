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

#include "titanic/game/nav_helmet.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CNavHelmet, CGameObject)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(PETHelmetOnOffMsg)
	ON_MESSAGE(PETPhotoOnOffMsg)
	ON_MESSAGE(PETStarFieldLockMsg)
	ON_MESSAGE(PETSetStarDestinationMsg)
END_MESSAGE_MAP()

void CNavHelmet::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CNavHelmet::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CNavHelmet::MovieEndMsg(CMovieEndMsg *msg) {
	if (_flag) {
		setVisible(false);

		CPetControl *pet = getPetControl();
		if (pet) {
			pet->setArea(PET_STARFIELD);
			petDisplayMessage(1, ADJUST_VIEWING_APPARATUS);
			pet->incAreaLocks();
		}

		starFn(STAR_SHOW);
		starFn(STAR_12);
	}

	return true;
}

bool CNavHelmet::EnterViewMsg(CEnterViewMsg *msg) {
	petSetRemoteTarget();
	return true;
}

bool CNavHelmet::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	return true;
}

bool CNavHelmet::PETHelmetOnOffMsg(CPETHelmetOnOffMsg *msg) {
	CPetControl *pet = getPetControl();

	if (_flag) {
		_flag = false;
		setVisible(true);
		starFn(STAR_HIDE);
		playMovie(61, 120, MOVIE_NOTIFY_OBJECT);
		playSound("a#47.wav");
		playSound("a#48.wav");

		if (pet) {
			pet->decAreaLocks();
			pet->setArea(PET_REMOTE);
		}

		decTransitions();
	} else {
		incTransitions();
		_flag = true;
		setVisible(true);
		playMovie(0, 60, MOVIE_NOTIFY_OBJECT);
		playSound("a#48.wav");
		playSound("a#47.wav");
	}

	return true;
}

bool CNavHelmet::PETPhotoOnOffMsg(CPETPhotoOnOffMsg *msg) {
	if (_flag)
		starFn(STAR_TOGGLE_MODE);

	return true;
}

bool CNavHelmet::PETStarFieldLockMsg(CPETStarFieldLockMsg *msg) {
	if (_flag) {
		if (msg->_value) {
			playSound("a#6.wav");
			starFn(LOCK_STAR);
		} else {
			playSound("a#5.wav");
			starFn(UNLOCK_STAR);
		}
	}

	return true;
}

bool CNavHelmet::PETSetStarDestinationMsg(CPETSetStarDestinationMsg *msg) {
	CActMsg actMsg("SetDestin");
	actMsg.execute("CaptainsWheel");
	return true;
}

} // End of namespace Titanic
