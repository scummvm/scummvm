/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_NORAD_H
#define PEGASUS_NEIGHBORHOOD_NORAD_NORAD_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

// This is the code common to both Norad Alpha and Norad Delta

class Norad : public Neighborhood {
public:
	Norad(InputHandler *, PegasusEngine *owner, const Common::String &resName, const NeighborhoodID);
	~Norad() override {}

	void flushGameState() override;

	void start() override;

	virtual void getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID,
			HotSpotID &clawControlSpotID, HotSpotID &pinchClawSpotID,
			HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID,
			HotSpotID &moveClawLeftSpotID,HotSpotID &moveClawUpSpotID,
			HotSpotID &clawCCWSpotID, HotSpotID &clawCWSpotID, uint32 &, const uint32 *&) = 0;
	void checkAirMask() override;

	uint16 getDateResID() const override;

	GameInteraction *makeInteraction(const InteractionID) override;

	Common::Path getBriefingMovie() override;

	void pickedUpItem(Item *) override;

	virtual void playClawMonitorIntro() {}

	void doneWithPressureDoor();

protected:
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &) override;
	void cantOpenDoor(CanOpenDoorReason) override;
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant) override;
	void startExitMovie(const ExitTable::Entry &) override;
	void startZoomMovie(const ZoomTable::Entry &) override;
	void upButton(const Input &) override;
	void activateHotspots() override;

	void arriveAt(const RoomID, const DirectionConstant) override;
	virtual void arriveAtNoradElevator();
	virtual void arriveAtUpperPressureDoorRoom();
	virtual void arriveAtLowerPressureDoorRoom();
	virtual void arriveAtSubPlatformRoom();
	virtual void arriveAtSubControlRoom();
	void setUpAirMask();
	void receiveNotification(Notification *, const NotificationFlags) override;
	virtual bool playingAgainstRobot() { return false; }

	Notification _noradNotification;
	bool _doneWithPressureDoor;

	RoomID _elevatorUpRoomID;
	RoomID _elevatorDownRoomID;
	HotSpotID _elevatorUpSpotID;
	HotSpotID _elevatorDownSpotID;

	TimeBase _airMaskTimer;
	NotificationCallBack _airMaskCallBack;

	RoomID _subRoomEntryRoom1;
	DirectionConstant _subRoomEntryDir1;
	RoomID _subRoomEntryRoom2;
	DirectionConstant _subRoomEntryDir2;
	RoomID _upperPressureDoorRoom;
	RoomID _lowerPressureDoorRoom;

	HotSpotID _upperPressureDoorUpSpotID;
	HotSpotID _upperPressureDoorDownSpotID;
	HotSpotID _upperPressureDoorAbortSpotID;

	HotSpotID _lowerPressureDoorUpSpotID;
	HotSpotID _lowerPressureDoorDownSpotID;
	HotSpotID _lowerPressureDoorAbortSpotID;

	TimeValue _pressureSoundIn;
	TimeValue _pressureSoundOut;
	TimeValue _equalizeSoundIn;
	TimeValue _equalizeSoundOut;
	TimeValue _accessDeniedIn;
	TimeValue _accessDeniedOut;

	RoomID	 _platformRoom;
	RoomID _subControlRoom;
};

} // End of namespace Pegasus

#endif
