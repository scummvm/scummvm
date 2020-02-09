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

#ifndef PEGASUS_NEIGHBORHOOD_WSC_WSC_H
#define PEGASUS_NEIGHBORHOOD_WSC_WSC_H

#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/wsc/moleculebin.h"

namespace Pegasus {

static const DisplayOrder kWSCMoleculeBinOrder = kMonitorLayer;
static const DisplayOrder kWSCMoleculesMovieOrder = kWSCMoleculeBinOrder + 1;

static const RoomID kWSC01 = 0;
static const RoomID kWSC02Morph = 2;
static const RoomID kWSC02Messages = 3;
static const RoomID kWSC62 = 62;

class WSC : public Neighborhood {
public:
	WSC(InputHandler *, PegasusEngine *);
	~WSC() override {}

	void flushGameState() override;

	uint16 getDateResID() const override;

	bool okayToJump() override;

	void checkContinuePoint(const RoomID, const DirectionConstant) override;

	bool inSynthesizerGame();

	bool canSolve() override;
	void doSolve() override;

	void prepareForAIHint(const Common::String &) override;
	void cleanUpAfterAIHint(const Common::String &) override;

	void init() override;
	void start() override;

protected:
	enum {
		kWSCDraggingAntidoteFlag,

		kWSCPrivateLabMessagesOpenFlag,
		kWSCPrivateInterruptedMorphFlag,
		kWSCPrivateInMoleculeGameFlag,
		kWSCPrivateSinclairOfficeOpenFlag,
		kWSCPrivateOfficeLogOpenFlag,
		kWSCPrivate58SouthOpenFlag,
		kWSCPrivateClickedCatwalkCableFlag,
		kWSCPrivateRobotHeadOpenFlag,

		kWSCPrivateSeenPeopleAt17WestFlag,
		kWSCPrivateSeenPeopleAt19NorthFlag,
		kWSCPrivateSeenPeopleAt21SouthFlag,
		kWSCPrivateSeenPeopleAt24SouthFlag,
		kWSCPrivateSeenPeopleAt34EastFlag,
		kWSCPrivateSeenPeopleAt36WestFlag,
		kWSCPrivateSeenPeopleAt38NorthFlag,
		kWSCPrivateSeenPeopleAt46SouthFlag,
		kWSCPrivateSeenPeopleAt49NorthFlag,
		kWSCPrivateSeenPeopleAt73WestFlag,

		kWSCPrivateNeedPeopleAt17WestFlag,
		kWSCPrivateNeedPeopleAt21SouthFlag,
		kWSCPrivateNeedPeopleAt24SouthFlag,
		kWSCPrivateNeedPeopleAt34EastFlag,
		kWSCPrivateNeedPeopleAt36WestFlag,
		kWSCPrivateNeedPeopleAt38NorthFlag,
		kWSCPrivateNeedPeopleAt46SouthFlag,
		kWSCPrivateNeedPeopleAt49NorthFlag,
		kWSCPrivateNeedPeopleAt73WestFlag,

		kWSCPrivateGotRetScanChipFlag,
		kWSCPrivateGotMapChipFlag,
		kWSCPrivateGotOpticalChipFlag,

		kNumWSCPrivateFlags
	};

	void arriveAt(const RoomID, const DirectionConstant) override;
	void turnTo(const DirectionConstant) override;
	void receiveNotification(Notification *, const NotificationFlags) override;
	void dropItemIntoRoom(Item *, Hotspot *) override;
	void clickInHotspot(const Input &, const Hotspot *) override;
	TimeValue getViewTime(const RoomID, const DirectionConstant) override;
	void getZoomEntry(const HotSpotID, ZoomTable::Entry &) override;
	CanMoveForwardReason canMoveForward(ExitTable::Entry &entry) override;
	void cantMoveThatWay(CanMoveForwardReason reason) override;
	CanTurnReason canTurn(TurnDirection turn, DirectionConstant &nextDir) override;
	void zoomTo(const Hotspot *hotspot) override;
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *) override;
	void setUpMoleculeGame();
	void nextMoleculeGameLevel();
	void startMoleculeGameLevel();
	void moleculeGameClick(const HotSpotID);
	void loadAmbientLoops() override;
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &) override;
	void cantOpenDoor(CanOpenDoorReason) override;
	void pickedUpItem(Item *) override;
	void doorOpened() override;
	void startExtraSequence(const ExtraID, const NotificationFlags, const InputBits) override;
	void getExtraEntry(const uint32, ExtraTable::Entry &) override;
	void takeItemFromRoom(Item *item) override;
	void checkPeopleCrossing();
	void turnLeft() override;
	void turnRight() override;
	void moveForward() override;
	Hotspot *getItemScreenSpot(Item *, DisplayElement *) override;
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant) override;
	void getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) override;
	void getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) override;
	void bumpIntoWall() override;
	void activateHotspots() override;
	void setUpAIRules() override;
	Common::String getBriefingMovie() override;
	Common::String getEnvScanMovie() override;
	uint getNumHints() override;
	Common::String getHintMovie(uint) override;
	void closeDoorOffScreen(const RoomID, const DirectionConstant) override;
	void setUpPoison();
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &) override;
	void timerExpired(const uint32) override;

	Common::String getSoundSpotsName() override;
	Common::String getNavMovieName() override;

	FlagsArray<byte, kNumWSCPrivateFlags> _privateFlags;
	const Hotspot *_cachedZoomSpot;
	MoleculeBin _moleculeBin;
	int32 _moleculeGameLevel, _numCorrect;
	Movie _moleculesMovie;
	uint32 _levelArray[6];
	Common::Rational _energyDrainRate;
	Sprite *_argonSprite;
};

} // End of namespace Pegasus

#endif
