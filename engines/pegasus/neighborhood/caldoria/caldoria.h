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

#ifndef PEGASUS_NEIGHBORHOOD_CALDORIA_CALDORIA_H
#define PEGASUS_NEIGHBORHOOD_CALDORIA_CALDORIA_H

#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

const TimeScale kCaldoriaMovieScale = 600;
const TimeScale kCaldoriaFramesPerSecond = 15;
const TimeScale kCaldoriaFrameDuration = 40;

//	Alternate IDs.

const AlternateID kAltCaldoriaNormal = 0;
const AlternateID kAltCaldoriaRoofDoorBlown = 2;
const AlternateID kAltCaldoriaSinclairDown = 3;

//	Room IDs.

const RoomID kCaldoria00 = 1;
const RoomID kCaldoria01 = 2;
const RoomID kCaldoria02 = 3;
const RoomID kCaldoria03 = 4;
const RoomID kCaldoria04 = 5;
const RoomID kCaldoria05 = 6;
const RoomID kCaldoria06 = 7;
const RoomID kCaldoria07 = 8;
const RoomID kCaldoria08 = 9;
const RoomID kCaldoria09 = 10;
const RoomID kCaldoria10 = 11;
const RoomID kCaldoriaToilet = 12;
const RoomID kCaldoria11 = 13;
const RoomID kCaldoria12 = 14;
const RoomID kCaldoriaVidPhone = 15;
const RoomID kCaldoriaReplicator = 16;
const RoomID kCaldoriaDrawers = 17;
const RoomID kCaldoria13 = 18;
const RoomID kCaldoria14 = 19;
const RoomID kCaldoria15 = 20;
const RoomID kCaldoria16 = 21;
const RoomID kCaldoria17 = 22;
const RoomID kCaldoria18 = 23;
const RoomID kCaldoria19 = 24;
const RoomID kCaldoria20 = 25;
const RoomID kCaldoria21 = 26;
const RoomID kCaldoria22 = 27;
const RoomID kCaldoria23 = 28;
const RoomID kCaldoria24 = 29;
const RoomID kCaldoria25 = 30;
const RoomID kCaldoria26 = 31;
const RoomID kCaldoria27 = 32;
const RoomID kCaldoria28 = 33;
const RoomID kCaldoria29 = 34;
const RoomID kCaldoria30 = 35;
const RoomID kCaldoria31 = 36;
const RoomID kCaldoria32 = 37;
const RoomID kCaldoria33 = 38;
const RoomID kCaldoria34 = 39;
const RoomID kCaldoria35 = 40;
const RoomID kCaldoria36 = 41;
const RoomID kCaldoria37 = 42;
const RoomID kCaldoria38 = 43;
const RoomID kCaldoria39 = 44;
const RoomID kCaldoria40 = 45;
const RoomID kCaldoria41 = 46;
const RoomID kCaldoriaBinoculars = 47;
const RoomID kCaldoria42 = 48;
const RoomID kCaldoriaKiosk = 49;
const RoomID kCaldoria44 = 50;
const RoomID kCaldoria45 = 51;
const RoomID kCaldoria46 = 52;
const RoomID kCaldoria47 = 53;
const RoomID kCaldoria48 = 54;
const RoomID kCaldoria49 = 55;
const RoomID kCaldoria50 = 56;
const RoomID kCaldoria51 = 57;
const RoomID kCaldoria52 = 58;
const RoomID kCaldoria53 = 59;
const RoomID kCaldoria54 = 60;
const RoomID kCaldoria55 = 61;
const RoomID kCaldoria56 = 62;
const RoomID kCaldoriaDeathRoom = 0;

//	Hot Spot Activation IDs.

const HotSpotActivationID kActivate4DClosed = 1;
const HotSpotActivationID kActivate4DOpen = 2;
const HotSpotActivationID kActivateMirrorReady = 3;
const HotSpotActivationID kActivateStylistReady = 4;
const HotSpotActivationID kActivateReplicatorReady = 5;
const HotSpotActivationID kActivateOJOnThePad = 6;
const HotSpotActivationID kActivateDrawersClosed = 7;
const HotSpotActivationID kActivateRightOpen = 8;
const HotSpotActivationID kActivateLeftOpen = 9;
const HotSpotActivationID kActivateFocusedOnShip = 10;
const HotSpotActivationID kActivateNotFocusedOnShip = 11;
const HotSpotActivationID kActivateReadyForCard = 12;
const HotSpotActivationID kActivateReadyToTransport = 13;
const HotSpotActivationID kActivateRoofSlotEmpty = 14;
const HotSpotActivationID kActivateZoomedOnSinclair = 15;

//	Hot Spot IDs.

const HotSpotID kCa4DEnvironOpenSpotID = 5000;
const HotSpotID kCa4DEnvironCloseSpotID = 5001;
const HotSpotID kCa4DVisualSpotID = 5002;
const HotSpotID kCa4DAudioSpotID = 5003;
const HotSpotID kCa4DChoice1SpotID = 5004;
const HotSpotID kCa4DChoice2SpotID = 5005;
const HotSpotID kCa4DChoice3SpotID = 5006;
const HotSpotID kCa4DChoice4SpotID = 5007;
const HotSpotID kCaBathroomMirrorSpotID = 5008;
const HotSpotID kCaHairStyle1SpotID = 5009;
const HotSpotID kCaHairStyle2SpotID = 5010;
const HotSpotID kCaHairStyle3SpotID = 5011;
const HotSpotID kCaShowerSpotID = 5012;
const HotSpotID kCaBathroomToiletSpotID = 5013;
const HotSpotID kCaldoriaVidPhoneSpotID = 5014;
const HotSpotID kCaldoriaReplicatorSpotID = 5015;
const HotSpotID kCaldoriaDrawersSpotID = 5016;
const HotSpotID kCaldoriaVidPhoneOutSpotID = 5017;
const HotSpotID kCaBedroomVidPhoneActivationSpotID = 5018;
const HotSpotID kCaldoriaReplicatorOutSpotID = 5019;
const HotSpotID kCaldoriaMakeOJSpotID = 5020;
const HotSpotID kCaldoriaMakeStickyBunsSpotID = 5021;
const HotSpotID kCaldoriaOrangeJuiceSpotID = 5022;
const HotSpotID kCaldoriaOrangeJuiceDropSpotID = 5023;
const HotSpotID kCaldoriaDrawersOutSpotID = 5024;
const HotSpotID kCaldoriaLeftDrawerOpenSpotID = 5025;
const HotSpotID kCaldoriaRightDrawerOpenSpotID = 5026;
const HotSpotID kCaldoriaKeyCardSpotID = 5027;
const HotSpotID kCaldoriaLeftDrawerCloseSpotID = 5028;
const HotSpotID kCaldoriaRightDrawerWithKeysCloseSpotID = 5029;
const HotSpotID kCaldoriaRightDrawerNoKeysCloseSpotID = 5030;
const HotSpotID kCaldoriaFourthFloorElevatorSpotID = 5031;
const HotSpotID kCaldoria20DoorbellSpotID = 5032;
const HotSpotID kCaldoria21DoorbellSpotID = 5033;
const HotSpotID kCaldoria26DoorbellSpotID = 5034;
const HotSpotID kCaldoriaFourthFloorElevator1 = 5035;
const HotSpotID kCaldoriaFourthFloorElevator2 = 5036;
const HotSpotID kCaldoriaFourthFloorElevator3 = 5037;
const HotSpotID kCaldoriaFourthFloorElevator4 = 5038;
const HotSpotID kCaldoriaFourthFloorElevator5 = 5039;
const HotSpotID kCaldoriaGroundElevator1 = 5040;
const HotSpotID kCaldoriaGroundElevator2 = 5041;
const HotSpotID kCaldoriaGroundElevator3 = 5042;
const HotSpotID kCaldoriaGroundElevator4 = 5043;
const HotSpotID kCaldoriaGroundElevator5 = 5044;
const HotSpotID kCaldoria29DoorbellSpotID = 5045;
const HotSpotID kCaldoria34DoorbellSpotID = 5046;
const HotSpotID kCaldoria35DoorbellSpotID = 5047;
const HotSpotID kCaldoriaGroundElevatorSpotID = 5048;
const HotSpotID kCaldoriaBinocularZoomInSpotID = 5049;
const HotSpotID kCaldoriaBinocularsOutSpotID = 5050;
const HotSpotID kCaldoriaZoomInOnShipSpotID = 5051;
const HotSpotID kCaldoriaKioskSpotID = 5052;
const HotSpotID kCaldoriaKioskOutSpotID = 5053;
const HotSpotID kCaldoriaKioskInfoSpotID = 5054;
const HotSpotID kCaldoriaGTCardDropSpotID = 5055;
const HotSpotID kCaldoriaGTTokyoSpotID = 5056;
const HotSpotID kCaldoriaGTTSASpotID = 5057;
const HotSpotID kCaldoriaGTBeachSpotID = 5058;
const HotSpotID kCaldoriaGTOtherSpotID = 5059;
const HotSpotID kCaldoriaRoofElevator1 = 5060;
const HotSpotID kCaldoriaRoofElevator2 = 5061;
const HotSpotID kCaldoriaRoofElevator3 = 5062;
const HotSpotID kCaldoriaRoofElevator4 = 5063;
const HotSpotID kCaldoriaRoofElevator5 = 5064;
const HotSpotID kCaldoriaRoofElevatorSpotID = 5065;
const HotSpotID kCaldoriaRoofDoorSpotID = 5066;
const HotSpotID kCaldoriaRoofCardDropSpotID = 5067;
const HotSpotID kCaldoria53EastSinclairTargetSpotID = 5068;

//	Extra sequence IDs.

const ExtraID kCaldoriaWakeUpView1 = 0;
const ExtraID kCaldoria00WakeUp1 = 1;
const ExtraID kCaldoria00WakeUp2 = 2;
const ExtraID kCaldoria00SitDown = 3;
const ExtraID k4DEnvironOpenToINN = 4;
const ExtraID k4DINNInterruption = 5;
const ExtraID k4DINNIntro = 6;
const ExtraID k4DINNMarkJohnson = 7;
const ExtraID k4DINNMeganLove = 8;
const ExtraID k4DINNFadeOut = 9;
const ExtraID k4DEnvironOpenFromINN = 10;
const ExtraID k4DEnvironOpen = 11;
const ExtraID k4DEnvironOpenView = 12;
const ExtraID k4DEnvironClose = 13;
const ExtraID k4DIslandLoop = 14;
const ExtraID k4DDesertLoop = 15;
const ExtraID k4DMountainLoop = 16;
const ExtraID k4DIsland1ToIsland0 = 17;
const ExtraID k4DIsland2ToIsland0 = 18;
const ExtraID k4DIsland0ToDesert0 = 19;
const ExtraID k4DIsland1ToDesert0 = 20;
const ExtraID k4DIsland2ToDesert0 = 21;
const ExtraID k4DIsland0ToMountain0 = 22;
const ExtraID k4DIsland1ToMountain0 = 23;
const ExtraID k4DIsland2ToMountain0 = 24;
const ExtraID k4DDesert0ToIsland0 = 25;
const ExtraID k4DDesert1ToIsland0 = 26;
const ExtraID k4DDesert2ToIsland0 = 27;
const ExtraID k4DDesert0ToMountain0 = 28;
const ExtraID k4DDesert1ToMountain0 = 29;
const ExtraID k4DDesert2ToMountain0 = 30;
const ExtraID k4DMountain0ToIsland0 = 31;
const ExtraID k4DMountain1ToIsland0 = 32;
const ExtraID k4DMountain2ToIsland0 = 33;
const ExtraID k4DMountain0ToDesert0 = 34;
const ExtraID k4DMountain1ToDesert0 = 35;
const ExtraID k4DMountain2ToDesert0 = 36;
const ExtraID kCaBathroomGreeting = 37;
const ExtraID kCaBathroomBodyFat = 38;
const ExtraID kCaBathroomStylistIntro = 39;
const ExtraID kCaBathroomRetrothrash = 40;
const ExtraID kCaBathroomRetrothrashReturn = 41;
const ExtraID kCaBathroomGeoWave = 42;
const ExtraID kCaBathroomGeoWaveReturn = 43;
const ExtraID kCaBathroomAgencyStandard = 44;
const ExtraID kCaldoriaShowerTitle = 45;
const ExtraID kCaldoriaShowerButton = 46;
const ExtraID kCaldoriaShowerDown = 47;
const ExtraID kCaldoriaShowerUp = 48;
const ExtraID kCaBedroomVidPhone = 49;
const ExtraID kCaBedroomMessage1 = 50;
const ExtraID kCaBedroomMessage2 = 51;
const ExtraID kCreateOrangeJuice = 52;
const ExtraID kDisposeOrangeJuice = 53;
const ExtraID kReplicatorNorthViewWithOJ = 54;
const ExtraID kLeftDrawerOpen = 55;
const ExtraID kLeftDrawerClose = 56;
const ExtraID kRightDrawerOpenWithKeys = 57;
const ExtraID kRightDrawerCloseWithKeys = 58;
const ExtraID kRightDrawerOpenNoKeys = 59;
const ExtraID kRightDrawerCloseNoKeys = 60;
const ExtraID kRightDrawerOpenViewWithKeys = 61;
const ExtraID kRightDrawerOpenViewNoKeys = 62;
const ExtraID kCaldoria16ElevatorUp = 63;
const ExtraID kCaldoria16ElevatorDown = 64;
const ExtraID kCaldoria16SouthViewWithElevator = 65;
const ExtraID kCaldoria20Doorbell = 66;
const ExtraID kCaldoria21Doorbell = 67;
const ExtraID kCaldoria26Doorbell = 68;
const ExtraID kCaldoriaFourthToGround = 69;
const ExtraID kCaldoriaRoofToFourth = 70;
const ExtraID kCaldoriaRoofToGround = 71;
const ExtraID kCaldoriaGroundToFourth = 72;
const ExtraID kCaldoriaGroundToRoof = 73;
const ExtraID kCaldoriaFourthToRoof = 74;
const ExtraID kCaldoria29Doorbell = 75;
const ExtraID kCaldoria34Doorbell = 76;
const ExtraID kCaldoria35Doorbell = 77;
const ExtraID kBinocularsZoomInOnShip = 78;
const ExtraID kCaldoriaKioskVideo = 79;
const ExtraID kCaldoriaTransporterArrowLoop = 80;
const ExtraID kArriveAtCaldoriaFromTSA = 81;
const ExtraID kCaGTOtherChoice = 82;
const ExtraID kCaGTCardSwipe = 83;
const ExtraID kCaGTSelectTSA = 84;
const ExtraID kCaGTFryTheFly = 85;
const ExtraID kCaGTGoToTSA = 86;
const ExtraID kCaGTSelectBeach = 87;
const ExtraID kCaGTGoToBeach = 88;
const ExtraID kCaGTArriveAtBeach = 89;
const ExtraID kCaGTSelectTokyo = 90;
const ExtraID kCaGTGoToTokyo = 91;
const ExtraID kCaGTArriveAtTokyo = 92;
const ExtraID kCa48NorthRooftopClosed = 93;
const ExtraID kCa48NorthExplosion = 94;
const ExtraID kCa48NorthExplosionDeath = 95;
const ExtraID kCa49NorthVoiceAnalysis = 96;
const ExtraID kCa50SinclairShoots = 97;
const ExtraID kCa53EastZoomToSinclair = 98;
const ExtraID kCa53EastDeath2 = 99;
const ExtraID kCa53EastShootSinclair = 100;
const ExtraID kCa53EastZoomOutFromSinclair = 101;
const ExtraID kCa54SouthDeath = 102;
const ExtraID kCaldoria56BombStage1 = 103;
const ExtraID kCaldoria56BombStage2 = 104;
const ExtraID kCaldoria56BombStage3 = 105;
const ExtraID kCaldoria56BombStage4 = 106;
const ExtraID kCaldoria56BombStage5 = 107;
const ExtraID kCaldoria56BombStage6 = 108;
const ExtraID kCaldoria56BombStage7 = 109;
const ExtraID kCaldoria56BombExplodes = 110;

//	Caldoria interactions.

const InteractionID kCaldoria4DInteractionID = 0;
const InteractionID kCaldoriaBombInteractionID = 1;
const InteractionID kCaldoriaMessagesInteractionID = 2;
const InteractionID kCaldoriaMirrorInteractionID = 3;

//	Caldoria:

const DisplayOrder kVidPhoneOrder = kMonitorLayer;
const DisplayOrder k4DSpritesOrder = kMonitorLayer;
const DisplayOrder kCaldoriaMessagesOrder = kMonitorLayer;
const DisplayOrder kCaldoriaElevatorOrder = kMonitorLayer;
const DisplayOrder kCaldoriaA05LightLoopOrder = kMonitorLayer;
const DisplayOrder kCaldoriaA07LightLoopOrder = kMonitorLayer;
const DisplayOrder kCaldoriaBombGridOrder = kMonitorLayer;
const DisplayOrder kCaldoriaBombTimerOrder = kCaldoriaBombGridOrder + 1;

/////////////////////////////////////////////
//
//	Caldoria

const CoordType kCaldoriaVidPhoneLeft = kNavAreaLeft + 105;
const CoordType kCaldoriaVidPhoneTop = kNavAreaTop + 28;

const CoordType kCaldoria4DSpritesLeft = kNavAreaLeft + 10;
const CoordType kCaldoria4DSpritesTop = kNavAreaTop + 142;

const CoordType kCaldoriaMessageLeft = kNavAreaLeft + 202;
const CoordType kCaldoriaMessageTop = kNavAreaTop + 26;

const CoordType kCaldoriaElevatorLeft = kNavAreaLeft + 407;
const CoordType kCaldoriaElevatorTop = kNavAreaTop + 138;

const CoordType kCaldoriaA05LightLoopLeft = kNavAreaLeft + 213;
const CoordType kCaldoriaA05LightLoopTop = kNavAreaTop + 215;

const CoordType kCaldoriaA07LightLoopLeft = kNavAreaLeft + 414;
const CoordType kCaldoriaA07LightLoopTop = kNavAreaTop + 215;

const CoordType kCaldoriaGunSpriteLeft = kNavAreaLeft + 276;
const CoordType kCaldoriaGunSpriteTop = kNavAreaTop + 115;

const CoordType kCaldoria11MessageLoopLeft = kNavAreaLeft + 135;
const CoordType kCaldoria11MessageLoopTop = kNavAreaTop + 214;

const CoordType kCaldoria12MessageLoopLeft = kNavAreaLeft + 209;
const CoordType kCaldoria12MessageLoopTop = kNavAreaTop + 170;

const CoordType kCaldoria13MessageLoopLeft = kNavAreaLeft + 480;
const CoordType kCaldoria13MessageLoopTop = kNavAreaTop + 191;

const CoordType kCaldoria14MessageLoopLeft = kNavAreaLeft + 248;
const CoordType kCaldoria14MessageLoopTop = kNavAreaTop + 191;

const CoordType kCaldoria48CardBombLoopLeft = kNavAreaLeft + 337;
const CoordType kCaldoria48CardBombLoopTop = kNavAreaTop + 205;

const CoordType kCaldoriaBombGridLeft = kNavAreaLeft + 290;
const CoordType kCaldoriaBombGridTop = kNavAreaTop + 58;

const CoordType kCaldoriaBombTimerLeft = kNavAreaLeft + 58;
const CoordType kCaldoriaBombTimerTop = kNavAreaTop + 204;

//	Caldoria display IDs.

const DisplayElementID kCaldoriaVidPhoneID = kNeighborhoodDisplayID;
const DisplayElementID kCaldoria4DSpritesID = kCaldoriaVidPhoneID + 1;
const DisplayElementID kCaldoriaMessagesID = kCaldoria4DSpritesID + 1;
const DisplayElementID kCaldoriaUtilityID = kCaldoriaMessagesID + 1;
const DisplayElementID kCaldoriaBombGridID = kCaldoriaUtilityID + 1;
const DisplayElementID kCaldoriaBombTimerID = kCaldoriaBombGridID + 1;

const TimeValue kCaldoria4DBlankChoiceIn = 29730;
const TimeValue kCaldoria4DBlankChoiceOut = 33910;

class Caldoria;

class SinclairCallBack : public TimeBaseCallBack {
public:
	SinclairCallBack(Caldoria *);
	~SinclairCallBack() {}

protected:
	virtual void callBack();

	Caldoria *_caldoria;
};

class Caldoria : public Neighborhood {
friend class SinclairCallBack;
friend void doorBombTimerExpiredFunction(FunctionPtr *, void *);
friend void sinclairTimerExpiredFunction(FunctionPtr *, void *);

public:
	Caldoria(InputHandler *, PegasusEngine *);
	virtual ~Caldoria();
	
	virtual uint16 getDateResID() const;
	
	void pickedUpItem(Item *);
	
	virtual GameInteraction *makeInteraction(const InteractionID);
	
	virtual Common::String getBriefingMovie();
	virtual Common::String getEnvScanMovie();
	virtual uint getNumHints();
	virtual Common::String getHintMovie(uint);
	void loadAmbientLoops();
	bool wantsCursor();
	void flushGameState();

	void checkContinuePoint(const RoomID, const DirectionConstant);

protected:
	enum {
		kCaldoriaPrivate4DSystemOpenFlag,
		kCaloriaPrivateLeftDrawerOpenFlag,
		kCaldoriaPrivateRightDrawerOpenFlag,
		kCaldoriaPrivateReadyToShootFlag,
		kCaldoriaPrivateZoomingToBombFlag,
		kCaldoriaPrivateCanOpenElevatorDoorFlag,
		kCaldoriaPrivateSinclairTimerExpiredFlag,
		kCaldoriaPrivateSeen13CarFlag,
		kCaldoriaPrivateSeen14CarFlag,
		kCaldoriaPrivateSeen18CarFlag,
		kCaldoriaPrivateSeen23CarFlag,
		kCaldoriaPrivateSeen33CarFlag,
		kCaldoriaPrivateSeen36CarFlag,
		kCaldoriaPrivateSeen41NorthCarFlag,
		kCaldoriaPrivateSeen41EastCarFlag,
		kCaldoriaPrivateSeen41WestCarFlag,
		kNumCaldoriaPrivateFlags
	};
	
	void init();
	void start();
	
	void setUpRoofTop();

	void setUpAIRules();
	void doAIRecalibration();
	TimeValue getViewTime(const RoomID, const DirectionConstant);
	void findSpotEntry(const RoomID, const DirectionConstant, SpotFlags, SpotTable::Entry &);
	void startSpotOnceOnly(TimeValue, TimeValue);
	void startExitMovie(const ExitTable::Entry &);
	void startZoomMovie(const ZoomTable::Entry &);
	void startDoorOpenMovie(const TimeValue, const TimeValue);
	void startTurnPush(const TurnDirection, const TimeValue, const DirectionConstant);
	void bumpIntoWall();
	int16 getStaticCompassAngle(const RoomID, const DirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	void getZoomCompassMove(const ZoomTable::Entry &, FaderMoveSpec &);
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);
	void spotCompleted();
	void arriveAt(const RoomID, const DirectionConstant);
	void arriveAtCaldoria00();
	void arriveAtCaldoriaToilet();
	void arriveAtCaldoria44();
	void arriveAtCaldoria49();
	void arriveAtCaldoria56();
	void arriveAtCaldoriaDeath();
	void turnTo(const DirectionConstant);
	void zoomTo(const Hotspot *);
	void downButton(const Input &);
	void receiveNotification(Notification *, const NotificationFlags);
	InputBits getInputFilter();
	void activateHotspots();
	void clickInHotspot(const Input &, const Hotspot *);
	void newInteraction(const InteractionID);
	
	void clickOnDoorbell(const HotSpotID);
	
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void dropItemIntoRoom(Item *, Hotspot *);
	void takeElevator(uint, uint);
	void updateElevatorMovie();
	void openElevatorMovie();
	void emptyOJGlass();
	void closeDoorOffScreen(const RoomID, const DirectionConstant);
	void doorBombTimerExpired();
	void sinclairTimerExpired();
	void checkSinclairShootsOS();
	void setUpSinclairLoops();
	void zoomToSinclair();
	void playEndMessage();
	void checkInterruptSinclair();
	
	CanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void doorOpened();
	
	void updateCursor(const Common::Point, const Hotspot *);

	FlagsArray<uint16, kNumCaldoriaPrivateFlags> _privateFlags;

	const Hotspot *_zoomOutSpot;
	
	FuseFunction _utilityFuse;

	long _sinclairLoopCount;
	long _numSinclairLoops;

	Sprite *_gunSprite;

	SinclairCallBack _sinclairInterrupt;

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
