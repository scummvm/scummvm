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

#ifndef PEGASUS_NEIGHBORHOOD_TSA_FULLTSA_H
#define PEGASUS_NEIGHBORHOOD_TSA_FULLTSA_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

class RipTimer : public IdlerAnimation {
public:
	RipTimer(const DisplayElementID id) : IdlerAnimation(id) {}
	~RipTimer() override {}

	void initImage();
	void releaseImage();

	void draw(const Common::Rect &) override;

protected:
	void timeChanged(const TimeValue) override;

	CoordType _middle;
	Surface _timerImage;
};

//	Room IDs.

static const RoomID kTSA00 = 0;
static const RoomID kTSA22Red = 28;
static const RoomID kTSA37 = 42;

class FullTSA : public Neighborhood {
public:
	FullTSA(InputHandler *, PegasusEngine *);
	~FullTSA() override {}

	void init() override;

	void start() override;

	uint16 getDateResID() const override;

	void flushGameState() override;

	void checkContinuePoint(const RoomID, const DirectionConstant) override;

	bool canSolve() override;
	void doSolve() override;

	void updateCursor(const Common::Point, const Hotspot *) override;

protected:
	enum {
		kTSAPrivateLogReaderOpenFlag,
		kTSAPrivateKeyVaultOpenFlag,
		kTSAPrivateChipVaultOpenFlag,
		kTSAPrivatePlayingLeftComparisonFlag,
		kTSAPrivatePlayingRightComparisonFlag,
		kTSAPrivateSeenRobotWarningFlag,
		kNumTSAPrivateFlags
	};

	Common::String getBriefingMovie() override;
	Common::String getEnvScanMovie() override;
	uint getNumHints() override;
	Common::String getHintMovie(uint) override;
	void loadAmbientLoops() override;
	void clickInHotspot(const Input &, const Hotspot *) override;

	int16 getStaticCompassAngle(const RoomID, const DirectionConstant) override;
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *spot) override;
	void activateHotspots() override;
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &) override;
	void dropItemIntoRoom(Item *, Hotspot *) override;
	void downButton(const Input &) override;
	void startDoorOpenMovie(const TimeValue, const TimeValue) override;
	TimeValue getViewTime(const RoomID, const DirectionConstant) override;
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &) override;
	void turnTo(const DirectionConstant) override;
	CanMoveForwardReason canMoveForward(ExitTable::Entry &) override;
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &) override;
	void bumpIntoWall() override;
	void initializeTBPMonitor(const int, const ExtraID);
	void playTBPMonitor();
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &) override;
	Hotspot *getItemScreenSpot(Item *, DisplayElement *) override;
	void openDoor() override;
	void turnRight() override;
	void turnLeft() override;
	void closeDoorOffScreen(const RoomID, const DirectionConstant) override;
	void playExtraMovie(const ExtraTable::Entry &, const NotificationFlags, const InputBits interruptionInput) override;
	void handleInput(const Input &, const Hotspot *) override;
	void arriveAtTSA25Red();
	void startUpComparisonMonitor();
	void shutDownComparisonMonitor();
	void initializeComparisonMonitor(const int, const ExtraID);
	void playLeftComparison();
	void playRightComparison();
	void startRobotGame();
	void setOffRipAlarm();
	uint getHistoricalLogIndex();
	void startUpRobotMonitor();
	void shutDownRobotMonitor();
	void pickedUpItem(Item *item) override;
	void arriveFromPrehistoric();

	void arriveFromNorad();
	void arriveFromMars();
	void arriveFromWSC();

	InputBits getInputFilter() override;
	void arriveAt(const RoomID, const DirectionConstant) override;
	void initializePegasusButtons(bool);
	void releaseSprites();
	void showMainJumpMenu();
	void arriveAtTSA37();
	void receiveNotification(Notification *, const NotificationFlags) override;
	void checkRobotLocations(const RoomID, const DirectionConstant);
	void getExtraEntry(const uint32, ExtraTable::Entry &) override;

	Sprite _sprite1, _sprite2, _sprite3;
	FuseFunction _utilityFuse;
	RipTimer _ripTimer;

	FlagsArray<byte, kNumTSAPrivateFlags> _privateFlags;

	Common::String getNavMovieName() override;
	Common::String getSoundSpotsName() override;

	void dieUncreatedInTSA();
};

} // End of namespace Pegasus

#endif
