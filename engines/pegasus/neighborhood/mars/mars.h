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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PEGASUS_NEIGHBORHOOD_MARS_MARS_H
#define PEGASUS_NEIGHBORHOOD_MARS_MARS_H

#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/mars/reactor.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/shuttleenergymeter.h"

namespace Pegasus {

//	Element Coordinates

const tCoordType kUndoHiliteLeft = kNavAreaLeft + 140;
const tCoordType kUndoHiliteTop = kNavAreaTop + 36;

const tCoordType kCurrentGuessLeft = kNavAreaLeft + 146;
const tCoordType kCurrentGuessTop = kNavAreaTop + 90;

const tCoordType kReactorChoiceHiliteLeft = kNavAreaLeft + 116;
const tCoordType kReactorChoiceHiliteTop = kNavAreaTop + 158;

const tCoordType kReactorHistoryLeft = kNavAreaLeft + 302;
const tCoordType kReactorHistoryTop = kNavAreaTop + 39;

const tCoordType kAnswerLeft = kNavAreaLeft + 304;
const tCoordType kAnswerTop = kNavAreaTop + 180;

const tCoordType kShuttle1Left = 0;
const tCoordType kShuttle1Top = 0;

const tCoordType kShuttle2Left = 0;
const tCoordType kShuttle2Top = 96;

const tCoordType kShuttle3Left = 500;
const tCoordType kShuttle3Top = 96;

const tCoordType kShuttle4Left = 0;
const tCoordType kShuttle4Top = 320;

const tCoordType kShuttleWindowLeft = 140;
const tCoordType kShuttleWindowTop = 96;
const tCoordType kShuttleWindowWidth = 360;
const tCoordType kShuttleWindowHeight = 224;

const tCoordType kShuttleWindowMidH = (kShuttleWindowLeft * 2 + kShuttleWindowWidth) / 2;
const tCoordType kShuttleWindowMidV = (kShuttleWindowTop * 2 + kShuttleWindowHeight) / 2;

const tCoordType kShuttleLeftLeft = 0;
const tCoordType kShuttleLeftTop = 128;

const tCoordType kShuttleRightLeft = 506;
const tCoordType kShuttleRightTop = 128;

const tCoordType kShuttleLowerLeftLeft = 74;
const tCoordType kShuttleLowerLeftTop = 358;

const tCoordType kShuttleLowerRightLeft = 486;
const tCoordType kShuttleLowerRightTop = 354;

const tCoordType kShuttleCenterLeft = 260;
const tCoordType kShuttleCenterTop = 336;

const tCoordType kShuttleUpperLeftLeft = 30;
const tCoordType kShuttleUpperLeftTop = 32;

const tCoordType kShuttleUpperRightLeft = 506;
const tCoordType kShuttleUpperRightTop = 52;

const tCoordType kShuttleLeftEnergyLeft = 110;
const tCoordType kShuttleLeftEnergyTop = 186;

const tCoordType kShuttleRightEnergyLeft = 510;
const tCoordType kShuttleRightEnergyTop = 186;

const tCoordType kShuttleEnergyLeft = 186;
const tCoordType kShuttleEnergyTop = 60;
const tCoordType kShuttleEnergyWidth = 252;
const tCoordType kShuttleEnergyHeight = 22;

const tCoordType kPlanetStartLeft = kShuttleWindowLeft;
const tCoordType kPlanetStartTop = kShuttleWindowTop + kShuttleWindowHeight;

const tCoordType kPlanetStopLeft = kShuttleWindowLeft;
const tCoordType kPlanetStopTop = kShuttleWindowTop + kShuttleWindowHeight - 100;

const tCoordType kShuttleTractorLeft = kShuttleWindowLeft + 6;
const tCoordType kShuttleTractorTop = kShuttleWindowTop + 56;
const tCoordType kShuttleTractorWidth = 348;
const tCoordType kShuttleTractorHeight = 112;

const tCoordType kShuttleJunkLeft = kShuttleWindowLeft + 6;
const tCoordType kShuttleJunkTop = kShuttleWindowTop + 6;

const tDisplayOrder kShuttlePlanetOrder = kInterfaceLayer;
const tDisplayOrder kShuttleAlienShipOrder = kShuttlePlanetOrder + 1;
const tDisplayOrder kShuttleRobotShipOrder = kShuttleAlienShipOrder + 1;
const tDisplayOrder kShuttleTractorBeamMovieOrder = kShuttleRobotShipOrder + 1;
const tDisplayOrder kShuttleWeaponBackOrder = kShuttleTractorBeamMovieOrder + 1;
const tDisplayOrder kShuttleJunkOrder = kShuttleWeaponBackOrder + 1;
const tDisplayOrder kShuttleWeaponFrontOrder = kShuttleJunkOrder + 1;
const tDisplayOrder kShuttleTractorBeamOrder = kShuttleWeaponFrontOrder + 1;
const tDisplayOrder kShuttleHUDOrder = kShuttleTractorBeamOrder + 1;
const tDisplayOrder kShuttleBackgroundOrder = kShuttleHUDOrder + 1;
const tDisplayOrder kShuttleMonitorOrder = kShuttleBackgroundOrder + 1;
const tDisplayOrder kShuttleStatusOrder = kShuttleMonitorOrder + 1;

const TimeValue kShuttleSwingStart = 0;
const TimeValue kShuttleSwingStop = 5 * 600;

const TimeValue kCanyonChaseStart = kShuttleSwingStop;
const TimeValue kCanyonChaseStop = 60 * 600 + 43 * 600 + 14 * 40;

const TimeValue kLaunchTubeReachedTime = 60 * 600 + 38 * 600 - kCanyonChaseStart;
const TimeValue kCanyonChaseFinishedTime = kCanyonChaseStop - kCanyonChaseStart -
											kLaunchTubeReachedTime;

//	Left shuttle.

const TimeValue kShuttleLeftIntroStart = 0;
const TimeValue kShuttleLeftIntroStop = 400;

const TimeValue kShuttleLeftBlankTime = 400;

const TimeValue kShuttleLeftNormalTime = 440;

const TimeValue kShuttleLeftAutoTestTime = 480;

const TimeValue kShuttleLeftDamagedTime = 520;

const TimeValue kShuttleLeftDampingTime = 560;

const TimeValue kShuttleLeftGravitonTime = 600;

const TimeValue kShuttleLeftTractorTime = 640;

//	Right shuttle.

const TimeValue kShuttleRightIntroStart = 0;
const TimeValue kShuttleRightIntroStop = 400;

const TimeValue kShuttleRightDestroyedStart = 400;
const TimeValue kShuttleRightDestroyedStop = 840;

const TimeValue kShuttleRightBlankTime = 840;

const TimeValue kShuttleRightNormalTime = 880;

const TimeValue kShuttleRightDamagedTime = 920;

const TimeValue kShuttleRightTargetLockTime = 960;

const TimeValue kShuttleRightGravitonTime = 1000;

const TimeValue kShuttleRightOverloadTime = 1040;

//	Lower Left shuttle.

const TimeValue kShuttleLowerLeftCollisionTime = 0;

const TimeValue kShuttleLowerLeftTubeTime = 40;

const TimeValue kShuttleLowerLeftAutopilotTime = 80;

//	Lower Right shuttle.

const TimeValue kShuttleLowerRightOffTime = 0;

const TimeValue kShuttleLowerRightTrackingTime = 40;

const TimeValue kShuttleLowerRightTransportTime = 80;

const TimeValue kShuttleLowerRightTransportHiliteTime = 120;

//	Center shuttle.

const TimeValue kShuttleCenterBoardingTime = 0;

const TimeValue kShuttleCenterCheckTime = 40;

const TimeValue kShuttleCenterNavCompTime = 80;

const TimeValue kShuttleCenterCommTime = 120;

const TimeValue kShuttleCenterWeaponsTime = 160;

const TimeValue kShuttleCenterAllSystemsTime = 200;

const TimeValue kShuttleCenterSecureLooseTime = 240;

const TimeValue kShuttleCenterAutoTestTime = 280;

const TimeValue kShuttleCenterLaunchTime = 320;

const TimeValue kShuttleCenterEnterTubeTime = 360;

const TimeValue kShuttleCenterTargetSightedTime = 400;

const TimeValue kShuttleCenterVerifyingTime = 440;

const TimeValue kShuttleCenterScanningTime = 480;

const TimeValue kShuttleCenterSafeTime = 520;

//	Upper Left shuttle.

const TimeValue kShuttleUpperLeftDimTime = 0;

const TimeValue kShuttleUpperLeftDampingTime = 40;

const TimeValue kShuttleUpperLeftGravitonTime = 80;

const TimeValue kShuttleUpperLeftTractorTime = 120;

//	Upper Right shuttle.

const TimeValue kShuttleUpperRightLockedTime = 0;

const TimeValue kShuttleUpperRightArmedTime = 40;

const TimeValue kShuttleUpperRightAlienDestroyedTime = 80;

const TimeValue kShuttleUpperRightOverloadTime = 120;

const TimeValue kShuttleUpperRightTargetDestroyedTime = 160;

//	Shuttle distance

const int kShuttleDistance = 500;

const int kJunkMaxDistance = kShuttleDistance;
const int kJunkMinDistance = 40;

const int kEnergyBeamMaxDistance = kShuttleDistance;
const int kEnergyBeamMinDistance = 40;

const int kGravitonMaxDistance = kShuttleDistance;
const int kGravitonMinDistance = 40;


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
};

enum ShuttleWeaponSelection {
	kNoWeapon,
	kEnergyBeam,
	kGravitonCannon,
	kTractorBeam
};

const tRoomID kMars0A = 0;
const tRoomID kMars31South = 33;
const tRoomID kMars34 = 37;
const tRoomID kMars35 = 38;
const tRoomID kMars39 = 42;
const tRoomID kMars49 = 51;
const tRoomID kMars60 = 58;
const tRoomID kMarsMaze004 = 60;
const tRoomID kMarsMaze200 = 224;

class Mars : public Neighborhood {
friend void robotTimerExpiredFunction(FunctionPtr *, void *);
friend void lockThawTimerExpiredFunction(FunctionPtr *, void *);
friend void bombTimerExpiredFunction(FunctionPtr *, void *);
friend void bombTimerExpiredInGameFunction(FunctionPtr *, void *);
friend void airStageExpiredFunction(FunctionPtr *, void *);
friend void marsTimerFunction(FunctionPtr *, void *);

public:
	Mars(InputHandler *, PegasusEngine *);
	virtual ~Mars();

	void flushGameState();

	virtual uint16 getDateResID() const;

	virtual tAirQuality getAirQuality(const tRoomID);

	void checkAirMask();

	void showBigExplosion(const Common::Rect &, const tDisplayOrder);
	void showLittleExplosion(const Common::Rect &, const tDisplayOrder);
	void hitByJunk();
	void decreaseRobotShuttleEnergy(const int, Common::Point impactPoint);
	void setUpNextDropTime();

	Common::String getBriefingMovie();
	Common::String getEnvScanMovie();
	uint getNumHints();
	Common::String getHintMovie(uint);

	virtual void shieldOn();
	virtual void shieldOff();

	void checkContinuePoint(const tRoomID, const tDirectionConstant);

	void setSoundFXLevel(const uint16);

	bool canSolve();
	void doSolve();

	bool inColorMatchingGame();

	// TODO: Space chase functions

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
	
	void init();
	void start();
	void setUpAIRules();
	void arriveAt(const tRoomID, const tDirectionConstant);
	void takeItemFromRoom(Item *);
	void dropItemIntoRoom(Item *, Hotspot *);
	void activateHotspots();
	void activateOneHotspot(HotspotInfoTable::Entry &, Hotspot *);
	void clickInHotspot(const Input &, const Hotspot *);
	tInputBits getInputFilter();
	
	TimeValue getViewTime(const tRoomID, const tDirectionConstant);
	void getZoomEntry(const tHotSpotID, ZoomTable::Entry &);
	void findSpotEntry(const tRoomID, const tDirectionConstant, tSpotFlags, SpotTable::Entry &);
	tCanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void openDoor();
	void closeDoorOffScreen(const tRoomID, const tDirectionConstant);
	int16 getStaticCompassAngle(const tRoomID, const tDirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);
	void turnTo(const tDirectionConstant);
	void receiveNotification(Notification *, const tNotificationFlags);
	void doorOpened();
	void setUpReactorEnergyDrain();
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void lockThawed();
	void robotTiredOfWaiting();
	
	void setUpReactorLevel1();
	void setUpNextReactorLevel();
	void makeColorSequence();
	void doUndoOneGuess();
	void doReactorGuess(int32 guess);
	void bombExplodesInGame();
	void didntFindBomb();
	tCanMoveForwardReason canMoveForward(ExitTable::Entry &);
	void cantMoveThatWay(tCanMoveForwardReason);
	void moveForward();
	void bumpIntoWall();
	void turnLeft();
	void turnRight();
	void airStageExpired();
	void loadAmbientLoops();
	void checkAirlockDoors();
	void pickedUpItem(Item *item);
	void cantOpenDoor(tCanOpenDoorReason);
	void launchMaze007Robot();
	void launchMaze015Robot();
	void launchMaze101Robot();
	void launchMaze104Robot();
	void launchMaze133Robot();
	void launchMaze136Robot();
	void launchMaze184Robot();
	void timerExpired(const uint32);
	void spotCompleted();

	void doCanyonChase(void);
	void startMarsTimer(TimeValue, TimeScale, MarsTimerCode);
	void marsTimerExpired(MarsTimerEvent &);
	void throwAwayMarsShuttle();

	// TODO: Space chase functions

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
	
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
	RobotShip _robotShip;
	ScalingMovie _explosions;
	NotificationCallBack _explosionCallBack;

	// TODO: Space chase variables
};

} // End of namespace Pegasus

#endif
