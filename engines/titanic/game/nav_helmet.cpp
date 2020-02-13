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
#include "titanic/star_control/star_control.h"
#include "titanic/translation.h"

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
	file->writeNumberLine(_helmetOn, indent);
	CGameObject::save(file, indent);
}

void CNavHelmet::load(SimpleFile *file) {
	file->readNumber();
	_helmetOn = file->readNumber();
	CGameObject::load(file);
}

bool CNavHelmet::MovieEndMsg(CMovieEndMsg *msg) {
	CPetControl *pet = getPetControl();
	assert(pet);

	if (_helmetOn && pet->isAreaUnlocked()) {
		setVisible(false);

		pet->setArea(PET_STARFIELD);
		petDisplayMessage(1, ADJUST_VIEWING_APPARATUS);
		pet->incAreaLocks();

		starFn(STAR_SHOW);
		starFn(STAR_STEREO_PAIR_ON);
	}

	return true;
}

bool CNavHelmet::EnterViewMsg(CEnterViewMsg *msg) {
	petSetRemoteTarget();
	loadFrame(120);
	return true;
}

bool CNavHelmet::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	return true;
}

bool CNavHelmet::PETHelmetOnOffMsg(CPETHelmetOnOffMsg *msg) {
	CPetControl *pet = getPetControl();

	if (_helmetOn) {
		_helmetOn = false;
		setVisible(true);
		starFn(STAR_HIDE);
		playMovie(61, 120, MOVIE_NOTIFY_OBJECT);
		playSound(TRANSLATE("a#47.wav", "a#40.wav"));
		playSound(TRANSLATE("a#48.wav", "a#41.wav"));

		if (pet) {
			pet->decAreaLocks();
			pet->setArea(PET_REMOTE);
		}

		decTransitions();
	} else {
		incTransitions();
		_helmetOn = true;
		setVisible(true);
		playMovie(0, 60, MOVIE_NOTIFY_OBJECT);
		playSound(TRANSLATE("a#48.wav", "a#41.wav"));
		playSound(TRANSLATE("a#47.wav", "a#40.wav"));
	}

	return true;
}

bool CNavHelmet::PETPhotoOnOffMsg(CPETPhotoOnOffMsg *msg) {
	if (_helmetOn)
		starFn(STAR_TOGGLE_HOME_PHOTO);

	return true;
}

bool CNavHelmet::PETStarFieldLockMsg(CPETStarFieldLockMsg *msg) {
	if (_helmetOn) {
		CPetControl *pet = getPetControl();
		CStarControl *starControl = nullptr;
		bool isStarFieldMode = false;

		if (pet)
			starControl = pet->getStarControl();

		if (starControl)
			isStarFieldMode = starControl->isStarFieldMode();

		if (isStarFieldMode) {
			// locking and unlocking only in starfield
			// It already does this without the conditional
			// but now it will also not play the sounds in
			// photoview
			if (msg->_value) {
				playSound(TRANSLATE("a#6.wav", "a#58.wav"));
				starFn(LOCK_STAR);
			} else {
				playSound(TRANSLATE("a#5.wav", "a#57.wav"));
				starFn(UNLOCK_STAR);
			}
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
