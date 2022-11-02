/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-2013 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pegasus/chase.h"

namespace Pegasus {

// Notification constants.

static const NotificationFlags kChaseNotificationFlags = kChaseEnteredBranchZone |
															kChaseExitedBranchZone |
															kChaseFinished;

ChaseInteraction::ChaseInteraction(const InteractionID interactionID, Neighborhood *handler,
	const NotificationID notificationID, NotificationManager *manager) : GameInteraction(interactionID, handler),
	_chaseNotification(notificationID, manager), _steerPict(kNoDisplayElement), _directionNeeded(false),
	_turnDirection(kNoTurn) {

	// HACK HACK HACK HACK HACK
	// TRIPLE SUPER ULTRA HACK
	// This avoids a nasty optimization bug I have been unable to track down
	// after days and days of searching. Let's just ship the game already.
	setNextHandler(nullptr);

	_neighborhoodNotification = handler->getNeighborhoodNotification();
}

void ChaseInteraction::openInteraction() {
	_steerPict.initFromPICTFile("Images/Interface/steer.pict", true);

	_chaseNotification.notifyMe(this, kChaseNotificationFlags, kChaseNotificationFlags);

	_neighborhoodNotification->notifyMe(this, kDelayCompletedFlag | kSpotSoundCompletedFlag,
										kDelayCompletedFlag | kSpotSoundCompletedFlag);
}

void ChaseInteraction::closeInteraction() {
	_steerPict.hide();
	_steerPict.deallocateSurface();

	_chaseNotification.cancelNotification(this);

	_neighborhoodNotification->cancelNotification(this);
}

void ChaseInteraction::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if (notification == &_chaseNotification)
		switch (flags) {
		case kChaseEnteredBranchZone:
			_directionNeeded = true;
			showControlsHint();
			setUpBranch();
			break;
		case kChaseExitedBranchZone:
			switch (_turnDirection) {
			case kTurnLeft:
				branchLeft();
				break;
			case kTurnRight:
				branchRight();
				break;
			case kNoTurn:
				dontBranch();
				break;
			default:
				break;
			}
			hideControlsHint();
			_turnDirection = kNoTurn;
			_directionNeeded = false;
			break;
		case kChaseFinished:
			_owner->requestDeleteCurrentInteraction();
			break;
		default:
			break;
		}
}

void ChaseInteraction::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (input.anyDirectionInput()) {
		if (_directionNeeded) {
			if (input.leftButtonAnyDown())
				_turnDirection = kTurnLeft;
			else if (input.rightButtonAnyDown())
				_turnDirection = kTurnRight;
		}
	} else {
		this->InputHandler::handleInput(input, cursorSpot);
	}
}

void ChaseInteraction::showControlsHint() {
	_steerPict.show();
}

void ChaseInteraction::hideControlsHint() {
	_steerPict.hide();
}

} // End of namespace Pegasus
