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

#ifndef PEGASUS_CHASE_H
#define PEGASUS_CHASE_H

#include "pegasus/hotspot.h"
#include "pegasus/interaction.h"
#include "pegasus/notification.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

// Notification constants.

static const NotificationFlags kChaseEnteredBranchZone = 1;
static const NotificationFlags kChaseExitedBranchZone = kChaseEnteredBranchZone << 1;
static const NotificationFlags kChaseFinished = kChaseExitedBranchZone << 1;

// Time to choose a direction.

static const TimeValue kDecisionTime = 600 * 2;

class ChaseInteraction : public GameInteraction, public NotificationReceiver {
public:
	ChaseInteraction(const InteractionID, Neighborhood *, NotificationID, NotificationManager *);
	virtual ~ChaseInteraction() {}

	virtual void handleInput(const Input &, const Hotspot *);

protected:

	virtual void openInteraction();
	virtual void initInteraction() {}
	virtual void closeInteraction();

	virtual void receiveNotification(Notification *, const NotificationFlags);

	virtual void setUpBranch() {}
	virtual void branchLeft() {}
	virtual void branchRight() {}
	virtual void dontBranch() {}

	virtual void showControlsHint();
	virtual void hideControlsHint();

	Notification _chaseNotification;

	Notification *_neighborhoodNotification;

	Picture _steerPict;

private:

	bool _directionNeeded;
	TurnDirection _turnDirection;
};

} // End of namespace Pegasus

#endif
