/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_DELTA_NORADDELTA_H
#define PEGASUS_NEIGHBORHOOD_NORAD_DELTA_NORADDELTA_H

#include "pegasus/neighborhood/norad/norad.h"

namespace Pegasus {

class NoradDelta : public Norad {
public:
	NoradDelta(InputHandler *, PegasusEngine *);
	~NoradDelta() override {}

	void init() override;

	void start() override;

	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &) override;

	void finishedGlobeGame();

	GameInteraction *makeInteraction(const InteractionID) override;

	void playClawMonitorIntro() override;

	void getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID, HotSpotID &clawControlSpotID,
			HotSpotID &pinchClawSpotID, HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID,
			HotSpotID &moveClawLeftSpotID, HotSpotID &moveClawUpSpotID, HotSpotID &clawCCWSpotID,
			HotSpotID &clawCWSpotID, uint32 &, const uint32 *&) override;

	void playerBeatRobotWithClaw();
	void playerBeatRobotWithDoor();

	void loadAmbientLoops() override;

	void setUpAIRules() override;
	Common::String getEnvScanMovie() override;
	uint getNumHints() override;
	Common::String getHintMovie(uint) override;
	void closeDoorOffScreen(const RoomID, const DirectionConstant) override;

	void checkContinuePoint(const RoomID, const DirectionConstant) override;

	bool canSolve() override;
	void doSolve() override;

	void setSoundFXLevel(const uint16) override;

	void doorOpened() override;

protected:
	enum {
		kNoradPrivateArrivedFromSubFlag,
		kNoradPrivateFinishedGlobeGameFlag,
		kNoradPrivateRobotHeadOpenFlag,
		kNoradPrivateGotShieldChipFlag,
		kNoradPrivateGotOpticalChipFlag,
		kNoradPrivateGotRetScanChipFlag,
		kNumNoradPrivateFlags
	};

	static const uint32 _noradDeltaClawExtras[22];

	void getExitEntry(const RoomID, const DirectionConstant, ExitTable::Entry &) override;
	void getZoomEntry(const HotSpotID, ZoomTable::Entry &) override;
	void arriveAt(const RoomID, const DirectionConstant) override;
	void arriveAtNorad68West();
	void arriveAtNorad79West();
	TimeValue getViewTime(const RoomID, const DirectionConstant) override;
	void openDoor() override;
	void cantMoveThatWay(CanMoveForwardReason) override;
	void activateHotspots() override;
	void clickInHotspot(const Input &, const Hotspot *) override;
	void receiveNotification(Notification *, const NotificationFlags) override;
	void pickedUpItem(Item *item) override;
	void takeItemFromRoom(Item *item) override;
	void dropItemIntoRoom(Item *item, Hotspot *) override;
	Hotspot *getItemScreenSpot(Item *, DisplayElement *) override;

	bool playingAgainstRobot() override;

	void failRetinalScan();
	void succeedRetinalScan();
	void getDoorEntry(const RoomID, const DirectionConstant, DoorTable::Entry &) override;

	void bumpIntoWall() override;

	FlagsArray<byte, kNumNoradPrivateFlags> _privateFlags;

	Common::String getSoundSpotsName() override;
	Common::String getNavMovieName() override;
};

} // End of namespace Pegasus

#endif
