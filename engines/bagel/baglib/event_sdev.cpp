/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/baglib/event_sdev.h"
#include "bagel/baglib/pda.h"
#include "bagel/baglib/sound_object.h"

namespace Bagel {

bool CBagEventSDev::_evalTurnEventsFl;

ErrorCode CBagEventSDev::attach() {
	ErrorCode errCode = CBagStorageDev::attach();

	// Set the firstpaint flag and attach objects to allow
	// for immediate run objects to run
	if (m_bFirstPaint) {
		m_bFirstPaint = false;
		AttachActiveObjects();
	}

	return errCode;

}

ErrorCode CBagEventSDev::evaluateExpressions() {
	// If a Zelda movie is playing, don't execute the event world
	if (CBagPDA::IsMoviePlaying())
		return ERR_NONE;

	ErrorCode errCode = ERR_NONE;
	int count = GetObjectCount();
	for (int i = 0; i < count; ++i) {

		CBagObject *posObj = GetObjectByPos(i);
		if (posObj != nullptr) {
			// Find the local Expression objects
			// This code says... only evaluate if we're in an if statement, this must be wrong.
			if (posObj->GetExpression() == nullptr || posObj->GetExpression()->evaluate(posObj->IsNegative())) {
				if (!posObj->isAttached()) {
					posObj->setActive();
					posObj->attach();
				}
				if (posObj->IsImmediateRun())
					posObj->runObject();

			} else if (posObj->isAttached() && (posObj->GetType() != SOUNDOBJ || !((CBagSoundObject *)posObj)->IsPlaying())) {
				posObj->setActive(false);
				posObj->detach();
			}
		} else
			errCode = ERR_FFIND;
	}

	return errCode;
}

ErrorCode CBagTurnEventSDev::evaluateExpressions() {
	// If a zelda movie is playing, don't execute the turncount world
	if (CBagPDA::IsMoviePlaying()) {
		return ERR_UNKNOWN;
	}

	ErrorCode errCode = ERR_NONE;
	int count = GetObjectCount();
	for (int i = 0; i < count; ++i) {
		CBagObject *posObj = GetObjectByPos(i);
		if (posObj != nullptr) {
			// Find the local Expression objects
			// This code says... only evaluate if we're in an if statement, this must be wrong.
			if (posObj->GetExpression() == nullptr || posObj->GetExpression()->evaluate(posObj->IsNegative())) {
				if (!posObj->isAttached()) {
					posObj->setActive();
					posObj->attach();
				}
				if (posObj->IsImmediateRun())
					posObj->runObject();
			} else if (posObj->isAttached() && (posObj->GetType() != SOUNDOBJ || !((CBagSoundObject *)posObj)->IsPlaying())) {
				posObj->setActive(false);
				posObj->detach();
			}
		} else
			errCode = ERR_FFIND;
	}

	return errCode;
}

} // namespace Bagel
