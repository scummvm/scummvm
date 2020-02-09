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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_MARS_H
#define PEGASUS_NEIGHBORHOOD_MARS_MARS_H

#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/energybeam.h"
#include "pegasus/neighborhood/mars/gravitoncannon.h"
#include "pegasus/neighborhood/mars/planetmover.h"
#include "pegasus/neighborhood/mars/reactor.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/shuttleenergymeter.h"
#include "pegasus/neighborhood/mars/shuttlehud.h"
#include "pegasus/neighborhood/mars/spacejunk.h"
#include "pegasus/neighborhood/mars/tractorbeam.h"

namespace Pegasus {

class InventoryItem;
class Mars;

enum MarsTimerCode {
	kMarsLaunchTubeReached,
	kMarsCanyonChaseFinished,
	kMarsSpaceChaseFinished // Player ran out of time...
};

struct MarsTimerEvent {
	Mars *mars;
	MarsTimerCode event;

	void fire();
};

enum ShuttleWeaponSelection {
	kNoWeapon,
	kEnergyBeam,
	kGravitonCannon,
	kTractorBeam
};

class Mars : public Neighborhood {
friend struct MarsTimerEvent;
public:
	Mars(InputHandler *, PegasusEngine *);
	~Mars() override;

	void flushGameState() override;

	uint16 getDateResID() const override;

	AirQuality getAirQuality(const RoomID) override;

	void checkAirMask() override;

	void showBigExplosion(const Common::Rect &, const DisplayOrder);
	void showLittleExplosion(const Common::Rect &, const DisplayOrder);
	void hitByJunk();
	void decreaseRobotShuttleEnergy(const int, Common::Point impactPoint);
	void setUpNextDropTime();

	Common::String getBriefingMovie() override;
	Common::String getEnvScanMovie() override;
	uint getNumHints() override;
	Common::String getHintMovie(uint) override;

	void shieldOn() override;
	void shieldOff() override;

	void checkContinuePoint(const RoomID, const DirectionConstant) override;

	void setSoundFXLevel(const uint16) override;

	bool canSolve() override;
	void doSolve() override;

	bool inColorMatchingGame();

protected:
	enum {
		kMarsPrivatePodStorageOpenFlag,
		kMarsPrivatePodTurnLeftFlag,
		kMarsPrivatePodTurnRightFlag,
		kMarsPrivateRobotTiredOfWaitingFlag,
		kMarsPrivatePlatformZoomedInFlag,
		kMarsPrivateBombExposedFlag,
		kMarsPrivateDraggingBombFlag,
		kMarsPrivateInSpaceChaseFlag,
		kMarsPrivateGotMapChipFlag,
		kMarsPrivateGotOpticalChipFlag,
		kMarsPrivateGotShieldChipFlag,
		kNumMarsPrivateFlags
	};

	void init() override;
	void start() override;
	void setUpAIRules() override;
	void arriveAt(const RoomID, const DirectionConstant) override;
	void takeItemFromRoom(Item *) override;
	void dropItemIntoRoom(Item *, Hotspot *) override;
	void activateHotspots() override;
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *) override;
	void clickInHotspot(const Input &, const Hotspot *) override;
	InputBits getInputFilter() override;

	TimeValue getViewTime(const RoomID, const DirectionConstant) override;
	void getZoomEntry(const HotSpotID, ZoomTable::Entry &) override;
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &) override;
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &) override;
	void openDoor() override;
	void closeDoorOffScreen(const RoomID, const DirectionConstant) override;
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant) override;
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &) override;
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &) override;
	void turnTo(const DirectionConstant) override;
	void receiveNotification(Notification *, const NotificationFlags) override;
	void doorOpened() override;
	void setUpReactorEnergyDrain();
	Hotspot *getItemScreenSpot(Item *, DisplayElement *) override;
	void lockThawed();
	void robotTiredOfWaiting();

	void setUpReactorLevel1();
	void setUpNextReactorLevel();
	void makeColorSequence();
	void doUndoOneGuess();
	void doReactorGuess(int32 guess);
	void bombExplodesInGame();
	void didntFindBomb();
	CanMoveForwardReason canMoveForward(ExitTable::Entry &) override;
	void cantMoveThatWay(CanMoveForwardReason) override;
	void moveForward() override;
	void bumpIntoWall() override;
	void turnLeft() override;
	void turnRight() override;
	void airStageExpired();
	void loadAmbientLoops() override;
	void checkAirlockDoors();
	void pickedUpItem(Item *item) override;
	void cantOpenDoor(CanOpenDoorReason) override;
	void launchMaze007Robot();
	void launchMaze015Robot();
	void launchMaze101Robot();
	void launchMaze104Robot();
	void launchMaze133Robot();
	void launchMaze136Robot();
	void launchMaze184Robot();
	void timerExpired(const uint32) override;
	void spotCompleted() override;

	void doCanyonChase(void);
	void startMarsTimer(TimeValue, TimeScale, MarsTimerCode);
	void marsTimerExpired(MarsTimerEvent &);
	void throwAwayMarsShuttle();
	void startUpFromFinishedSpaceChase();
	void startUpFromSpaceChase();
	void transportToRobotShip();
	void spaceChaseClick(const Input &, const HotSpotID);
	void updateCursor(const Common::Point, const Hotspot *) override;

	Common::String getSoundSpotsName() override;
	Common::String getNavMovieName() override;

	InventoryItem *_attackingItem;
	FuseFunction _bombFuse;
	FuseFunction _noAirFuse;
	FuseFunction _utilityFuse;
	FlagsArray<byte, kNumMarsPrivateFlags> _privateFlags;
	uint _reactorStage, _nextGuess;
	int32 _currentGuess[3];
	ReactorGuess _guessObject;
	Picture _undoPict;
	ReactorHistory _guessHistory;
	ReactorChoiceHighlight _choiceHighlight;

	Picture _shuttleInterface1;
	Picture _shuttleInterface2;
	Picture _shuttleInterface3;
	Picture _shuttleInterface4;
	Movie _canyonChaseMovie;

	MarsTimerEvent _marsEvent;

	Movie _leftShuttleMovie;
	Movie _rightShuttleMovie;
	Movie _lowerLeftShuttleMovie;
	Movie _lowerRightShuttleMovie;
	Movie _centerShuttleMovie;
	Movie _upperLeftShuttleMovie;
	Movie _upperRightShuttleMovie;
	Movie _leftDamageShuttleMovie;
	Movie _rightDamageShuttleMovie;
	ShuttleEnergyMeter _shuttleEnergyMeter;
	Movie _planetMovie;
	PlanetMover _planetMover;
	RobotShip _robotShip;
	ShuttleHUD _shuttleHUD;
	TractorBeam _tractorBeam;
	SpaceJunk _junk;
	EnergyBeam _energyBeam;
	GravitonCannon _gravitonCannon;
	Hotspot _energyChoiceSpot;
	Hotspot _gravitonChoiceSpot;
	Hotspot _tractorChoiceSpot;
	Hotspot _shuttleViewSpot;
	Hotspot _shuttleTransportSpot;
	ShuttleWeaponSelection _weaponSelection;
	ScalingMovie _explosions;
	NotificationCallBack _explosionCallBack;
};

} // End of namespace Pegasus

#endif
