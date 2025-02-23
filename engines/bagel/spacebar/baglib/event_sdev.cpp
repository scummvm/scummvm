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

#include "bagel/spacebar/baglib/event_sdev.h"
#include "bagel/spacebar/baglib/pda.h"
#include "bagel/spacebar/baglib/sound_object.h"

namespace Bagel {
namespace SpaceBar {

bool CBagEventSDev::_evalTurnEventsFl;

ErrorCode CBagEventSDev::attach() {
	const ErrorCode errorCode = CBagStorageDev::attach();

	// Set the firstpaint flag and attach objects to allow
	// for immediate run objects to run
	if (_bFirstPaint) {
		_bFirstPaint = false;
		attachActiveObjects();
	}

	return errorCode;

}

ErrorCode CBagEventSDev::evaluateExpressions() {
	// If a Zelda movie is playing, don't execute the event world
	if (CBagPDA::isMoviePlaying())
		return ERR_NONE;

	ErrorCode errorCode = ERR_NONE;
	const int count = getObjectCount();
	for (int i = 0; i < count; ++i) {

		CBagObject *posObj = getObjectByPos(i);
		if (posObj != nullptr) {
			// Find the local Expression objects
			// This code says... only evaluate if we're in an if statement, this must be wrong.
			if (posObj->getExpression() == nullptr || posObj->getExpression()->evaluate(posObj->isNegative())) {
				if (!posObj->isAttached()) {
					posObj->setActive();
					posObj->attach();
				}
				if (posObj->isImmediateRun())
					posObj->runObject();

			} else if (posObj->isAttached() && (posObj->getType() != SOUND_OBJ || !((CBagSoundObject *)posObj)->isPlaying())) {
				posObj->setActive(false);
				posObj->detach();
			}
		} else
			errorCode = ERR_FFIND;
	}

	return errorCode;
}

ErrorCode CBagTurnEventSDev::evaluateExpressions() {
	// If a zelda movie is playing, don't execute the turncount world
	if (CBagPDA::isMoviePlaying()) {
		return ERR_UNKNOWN;
	}

	ErrorCode errorCode = ERR_NONE;
	const int count = getObjectCount();
	for (int i = 0; i < count; ++i) {
		CBagObject *posObj = getObjectByPos(i);
		if (posObj != nullptr) {
			// Find the local Expression objects
			// This code says... only evaluate if we're in an if statement, this must be wrong.
			if (posObj->getExpression() == nullptr || posObj->getExpression()->evaluate(posObj->isNegative())) {
				if (!posObj->isAttached()) {
					posObj->setActive();
					posObj->attach();
				}
				if (posObj->isImmediateRun())
					posObj->runObject();
			} else if (posObj->isAttached() && (posObj->getType() != SOUND_OBJ || !((CBagSoundObject *)posObj)->isPlaying())) {
				posObj->setActive(false);
				posObj->detach();
			}
		} else
			errorCode = ERR_FFIND;
	}

	return errorCode;
}

} // namespace SpaceBar
} // namespace Bagel
