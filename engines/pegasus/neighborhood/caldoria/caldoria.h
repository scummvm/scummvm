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

const tAlternateID kAltCaldoriaNormal = 0;
const tAlternateID kAltCaldoriaRoofDoorBlown = 2;
const tAlternateID kAltCaldoriaSinclairDown = 3;

//	Room IDs.

const tRoomID kCaldoria00 = 1;
const tRoomID kCaldoria01 = 2;
const tRoomID kCaldoria02 = 3;
const tRoomID kCaldoria03 = 4;
const tRoomID kCaldoria04 = 5;
const tRoomID kCaldoria05 = 6;
const tRoomID kCaldoria06 = 7;
const tRoomID kCaldoria07 = 8;
const tRoomID kCaldoria08 = 9;
const tRoomID kCaldoria09 = 10;
const tRoomID kCaldoria10 = 11;
const tRoomID kCaldoriaToilet = 12;
const tRoomID kCaldoria11 = 13;
const tRoomID kCaldoria12 = 14;
const tRoomID kCaldoriaVidPhone = 15;
const tRoomID kCaldoriaReplicator = 16;
const tRoomID kCaldoriaDrawers = 17;
const tRoomID kCaldoria13 = 18;
const tRoomID kCaldoria14 = 19;
const tRoomID kCaldoria15 = 20;
const tRoomID kCaldoria16 = 21;
const tRoomID kCaldoria17 = 22;
const tRoomID kCaldoria18 = 23;
const tRoomID kCaldoria19 = 24;
const tRoomID kCaldoria20 = 25;
const tRoomID kCaldoria21 = 26;
const tRoomID kCaldoria22 = 27;
const tRoomID kCaldoria23 = 28;
const tRoomID kCaldoria24 = 29;
const tRoomID kCaldoria25 = 30;
const tRoomID kCaldoria26 = 31;
const tRoomID kCaldoria27 = 32;
const tRoomID kCaldoria28 = 33;
const tRoomID kCaldoria29 = 34;
const tRoomID kCaldoria30 = 35;
const tRoomID kCaldoria31 = 36;
const tRoomID kCaldoria32 = 37;
const tRoomID kCaldoria33 = 38;
const tRoomID kCaldoria34 = 39;
const tRoomID kCaldoria35 = 40;
const tRoomID kCaldoria36 = 41;
const tRoomID kCaldoria37 = 42;
const tRoomID kCaldoria38 = 43;
const tRoomID kCaldoria39 = 44;
const tRoomID kCaldoria40 = 45;
const tRoomID kCaldoria41 = 46;
const tRoomID kCaldoriaBinoculars = 47;
const tRoomID kCaldoria42 = 48;
const tRoomID kCaldoriaKiosk = 49;
const tRoomID kCaldoria44 = 50;
const tRoomID kCaldoria45 = 51;
const tRoomID kCaldoria46 = 52;
const tRoomID kCaldoria47 = 53;
const tRoomID kCaldoria48 = 54;
const tRoomID kCaldoria49 = 55;
const tRoomID kCaldoria50 = 56;
const tRoomID kCaldoria51 = 57;
const tRoomID kCaldoria52 = 58;
const tRoomID kCaldoria53 = 59;
const tRoomID kCaldoria54 = 60;
const tRoomID kCaldoria55 = 61;
const tRoomID kCaldoria56 = 62;
const tRoomID kCaldoriaDeathRoom = 0;

//	Hot Spot Activation IDs.

const tHotSpotActivationID kActivate4DClosed = 1;
const tHotSpotActivationID kActivate4DOpen = 2;
const tHotSpotActivationID kActivateMirrorReady = 3;
const tHotSpotActivationID kActivateStylistReady = 4;
const tHotSpotActivationID kActivateReplicatorReady = 5;
const tHotSpotActivationID kActivateOJOnThePad = 6;
const tHotSpotActivationID kActivateDrawersClosed = 7;
const tHotSpotActivationID kActivateRightOpen = 8;
const tHotSpotActivationID kActivateLeftOpen = 9;
const tHotSpotActivationID kActivateFocusedOnShip = 10;
const tHotSpotActivationID kActivateNotFocusedOnShip = 11;
const tHotSpotActivationID kActivateReadyForCard = 12;
const tHotSpotActivationID kActivateReadyToTransport = 13;
const tHotSpotActivationID kActivateRoofSlotEmpty = 14;
const tHotSpotActivationID kActivateZoomedOnSinclair = 15;

//	Hot Spot IDs.

const tHotSpotID kCa4DEnvironOpenSpotID = 5000;
const tHotSpotID kCa4DEnvironCloseSpotID = 5001;
const tHotSpotID kCa4DVisualSpotID = 5002;
const tHotSpotID kCa4DAudioSpotID = 5003;
const tHotSpotID kCa4DChoice1SpotID = 5004;
const tHotSpotID kCa4DChoice2SpotID = 5005;
const tHotSpotID kCa4DChoice3SpotID = 5006;
const tHotSpotID kCa4DChoice4SpotID = 5007;
const tHotSpotID kCaBathroomMirrorSpotID = 5008;
const tHotSpotID kCaHairStyle1SpotID = 5009;
const tHotSpotID kCaHairStyle2SpotID = 5010;
const tHotSpotID kCaHairStyle3SpotID = 5011;
const tHotSpotID kCaShowerSpotID = 5012;
const tHotSpotID kCaBathroomToiletSpotID = 5013;
const tHotSpotID kCaldoriaVidPhoneSpotID = 5014;
const tHotSpotID kCaldoriaReplicatorSpotID = 5015;
const tHotSpotID kCaldoriaDrawersSpotID = 5016;
const tHotSpotID kCaldoriaVidPhoneOutSpotID = 5017;
const tHotSpotID kCaBedroomVidPhoneActivationSpotID = 5018;
const tHotSpotID kCaldoriaReplicatorOutSpotID = 5019;
const tHotSpotID kCaldoriaMakeOJSpotID = 5020;
const tHotSpotID kCaldoriaMakeStickyBunsSpotID = 5021;
const tHotSpotID kCaldoriaOrangeJuiceSpotID = 5022;
const tHotSpotID kCaldoriaOrangeJuiceDropSpotID = 5023;
const tHotSpotID kCaldoriaDrawersOutSpotID = 5024;
const tHotSpotID kCaldoriaLeftDrawerOpenSpotID = 5025;
const tHotSpotID kCaldoriaRightDrawerOpenSpotID = 5026;
const tHotSpotID kCaldoriaKeyCardSpotID = 5027;
const tHotSpotID kCaldoriaLeftDrawerCloseSpotID = 5028;
const tHotSpotID kCaldoriaRightDrawerWithKeysCloseSpotID = 5029;
const tHotSpotID kCaldoriaRightDrawerNoKeysCloseSpotID = 5030;
const tHotSpotID kCaldoriaFourthFloorElevatorSpotID = 5031;
const tHotSpotID kCaldoria20DoorbellSpotID = 5032;
const tHotSpotID kCaldoria21DoorbellSpotID = 5033;
const tHotSpotID kCaldoria26DoorbellSpotID = 5034;
const tHotSpotID kCaldoriaFourthFloorElevator1 = 5035;
const tHotSpotID kCaldoriaFourthFloorElevator2 = 5036;
const tHotSpotID kCaldoriaFourthFloorElevator3 = 5037;
const tHotSpotID kCaldoriaFourthFloorElevator4 = 5038;
const tHotSpotID kCaldoriaFourthFloorElevator5 = 5039;
const tHotSpotID kCaldoriaGroundElevator1 = 5040;
const tHotSpotID kCaldoriaGroundElevator2 = 5041;
const tHotSpotID kCaldoriaGroundElevator3 = 5042;
const tHotSpotID kCaldoriaGroundElevator4 = 5043;
const tHotSpotID kCaldoriaGroundElevator5 = 5044;
const tHotSpotID kCaldoria29DoorbellSpotID = 5045;
const tHotSpotID kCaldoria34DoorbellSpotID = 5046;
const tHotSpotID kCaldoria35DoorbellSpotID = 5047;
const tHotSpotID kCaldoriaGroundElevatorSpotID = 5048;
const tHotSpotID kCaldoriaBinocularZoomInSpotID = 5049;
const tHotSpotID kCaldoriaBinocularsOutSpotID = 5050;
const tHotSpotID kCaldoriaZoomInOnShipSpotID = 5051;
const tHotSpotID kCaldoriaKioskSpotID = 5052;
const tHotSpotID kCaldoriaKioskOutSpotID = 5053;
const tHotSpotID kCaldoriaKioskInfoSpotID = 5054;
const tHotSpotID kCaldoriaGTCardDropSpotID = 5055;
const tHotSpotID kCaldoriaGTTokyoSpotID = 5056;
const tHotSpotID kCaldoriaGTTSASpotID = 5057;
const tHotSpotID kCaldoriaGTBeachSpotID = 5058;
const tHotSpotID kCaldoriaGTOtherSpotID = 5059;
const tHotSpotID kCaldoriaRoofElevator1 = 5060;
const tHotSpotID kCaldoriaRoofElevator2 = 5061;
const tHotSpotID kCaldoriaRoofElevator3 = 5062;
const tHotSpotID kCaldoriaRoofElevator4 = 5063;
const tHotSpotID kCaldoriaRoofElevator5 = 5064;
const tHotSpotID kCaldoriaRoofElevatorSpotID = 5065;
const tHotSpotID kCaldoriaRoofDoorSpotID = 5066;
const tHotSpotID kCaldoriaRoofCardDropSpotID = 5067;
const tHotSpotID kCaldoria53EastSinclairTargetSpotID = 5068;

//	Extra sequence IDs.

const tExtraID kCaldoriaWakeUpView1 = 0;
const tExtraID kCaldoria00WakeUp1 = 1;
const tExtraID kCaldoria00WakeUp2 = 2;
const tExtraID kCaldoria00SitDown = 3;
const tExtraID k4DEnvironOpenToINN = 4;
const tExtraID k4DINNInterruption = 5;
const tExtraID k4DINNIntro = 6;
const tExtraID k4DINNMarkJohnson = 7;
const tExtraID k4DINNMeganLove = 8;
const tExtraID k4DINNFadeOut = 9;
const tExtraID k4DEnvironOpenFromINN = 10;
const tExtraID k4DEnvironOpen = 11;
const tExtraID k4DEnvironOpenView = 12;
const tExtraID k4DEnvironClose = 13;
const tExtraID k4DIslandLoop = 14;
const tExtraID k4DDesertLoop = 15;
const tExtraID k4DMountainLoop = 16;
const tExtraID k4DIsland1ToIsland0 = 17;
const tExtraID k4DIsland2ToIsland0 = 18;
const tExtraID k4DIsland0ToDesert0 = 19;
const tExtraID k4DIsland1ToDesert0 = 20;
const tExtraID k4DIsland2ToDesert0 = 21;
const tExtraID k4DIsland0ToMountain0 = 22;
const tExtraID k4DIsland1ToMountain0 = 23;
const tExtraID k4DIsland2ToMountain0 = 24;
const tExtraID k4DDesert0ToIsland0 = 25;
const tExtraID k4DDesert1ToIsland0 = 26;
const tExtraID k4DDesert2ToIsland0 = 27;
const tExtraID k4DDesert0ToMountain0 = 28;
const tExtraID k4DDesert1ToMountain0 = 29;
const tExtraID k4DDesert2ToMountain0 = 30;
const tExtraID k4DMountain0ToIsland0 = 31;
const tExtraID k4DMountain1ToIsland0 = 32;
const tExtraID k4DMountain2ToIsland0 = 33;
const tExtraID k4DMountain0ToDesert0 = 34;
const tExtraID k4DMountain1ToDesert0 = 35;
const tExtraID k4DMountain2ToDesert0 = 36;
const tExtraID kCaBathroomGreeting = 37;
const tExtraID kCaBathroomBodyFat = 38;
const tExtraID kCaBathroomStylistIntro = 39;
const tExtraID kCaBathroomRetrothrash = 40;
const tExtraID kCaBathroomRetrothrashReturn = 41;
const tExtraID kCaBathroomGeoWave = 42;
const tExtraID kCaBathroomGeoWaveReturn = 43;
const tExtraID kCaBathroomAgencyStandard = 44;
const tExtraID kCaldoriaShowerTitle = 45;
const tExtraID kCaldoriaShowerButton = 46;
const tExtraID kCaldoriaShowerDown = 47;
const tExtraID kCaldoriaShowerUp = 48;
const tExtraID kCaBedroomVidPhone = 49;
const tExtraID kCaBedroomMessage1 = 50;
const tExtraID kCaBedroomMessage2 = 51;
const tExtraID kCreateOrangeJuice = 52;
const tExtraID kDisposeOrangeJuice = 53;
const tExtraID kReplicatorNorthViewWithOJ = 54;
const tExtraID kLeftDrawerOpen = 55;
const tExtraID kLeftDrawerClose = 56;
const tExtraID kRightDrawerOpenWithKeys = 57;
const tExtraID kRightDrawerCloseWithKeys = 58;
const tExtraID kRightDrawerOpenNoKeys = 59;
const tExtraID kRightDrawerCloseNoKeys = 60;
const tExtraID kRightDrawerOpenViewWithKeys = 61;
const tExtraID kRightDrawerOpenViewNoKeys = 62;
const tExtraID kCaldoria16ElevatorUp = 63;
const tExtraID kCaldoria16ElevatorDown = 64;
const tExtraID kCaldoria16SouthViewWithElevator = 65;
const tExtraID kCaldoria20Doorbell = 66;
const tExtraID kCaldoria21Doorbell = 67;
const tExtraID kCaldoria26Doorbell = 68;
const tExtraID kCaldoriaFourthToGround = 69;
const tExtraID kCaldoriaRoofToFourth = 70;
const tExtraID kCaldoriaRoofToGround = 71;
const tExtraID kCaldoriaGroundToFourth = 72;
const tExtraID kCaldoriaGroundToRoof = 73;
const tExtraID kCaldoriaFourthToRoof = 74;
const tExtraID kCaldoria29Doorbell = 75;
const tExtraID kCaldoria34Doorbell = 76;
const tExtraID kCaldoria35Doorbell = 77;
const tExtraID kBinocularsZoomInOnShip = 78;
const tExtraID kCaldoriaKioskVideo = 79;
const tExtraID kCaldoriaTransporterArrowLoop = 80;
const tExtraID kArriveAtCaldoriaFromTSA = 81;
const tExtraID kCaGTOtherChoice = 82;
const tExtraID kCaGTCardSwipe = 83;
const tExtraID kCaGTSelectTSA = 84;
const tExtraID kCaGTFryTheFly = 85;
const tExtraID kCaGTGoToTSA = 86;
const tExtraID kCaGTSelectBeach = 87;
const tExtraID kCaGTGoToBeach = 88;
const tExtraID kCaGTArriveAtBeach = 89;
const tExtraID kCaGTSelectTokyo = 90;
const tExtraID kCaGTGoToTokyo = 91;
const tExtraID kCaGTArriveAtTokyo = 92;
const tExtraID kCa48NorthRooftopClosed = 93;
const tExtraID kCa48NorthExplosion = 94;
const tExtraID kCa48NorthExplosionDeath = 95;
const tExtraID kCa49NorthVoiceAnalysis = 96;
const tExtraID kCa50SinclairShoots = 97;
const tExtraID kCa53EastZoomToSinclair = 98;
const tExtraID kCa53EastDeath2 = 99;
const tExtraID kCa53EastShootSinclair = 100;
const tExtraID kCa53EastZoomOutFromSinclair = 101;
const tExtraID kCa54SouthDeath = 102;
const tExtraID kCaldoria56BombStage1 = 103;
const tExtraID kCaldoria56BombStage2 = 104;
const tExtraID kCaldoria56BombStage3 = 105;
const tExtraID kCaldoria56BombStage4 = 106;
const tExtraID kCaldoria56BombStage5 = 107;
const tExtraID kCaldoria56BombStage6 = 108;
const tExtraID kCaldoria56BombStage7 = 109;
const tExtraID kCaldoria56BombExplodes = 110;

//	Caldoria interactions.

const tInteractionID kCaldoria4DInteractionID = 0;
const tInteractionID kCaldoriaBombInteractionID = 1;
const tInteractionID kCaldoriaMessagesInteractionID = 2;
const tInteractionID kCaldoriaMirrorInteractionID = 3;

//	Caldoria:

const tDisplayOrder kVidPhoneOrder = kMonitorLayer;
const tDisplayOrder k4DSpritesOrder = kMonitorLayer;
const tDisplayOrder kCaldoriaMessagesOrder = kMonitorLayer;
const tDisplayOrder kCaldoriaElevatorOrder = kMonitorLayer;
const tDisplayOrder kCaldoriaA05LightLoopOrder = kMonitorLayer;
const tDisplayOrder kCaldoriaA07LightLoopOrder = kMonitorLayer;
const tDisplayOrder kCaldoriaBombGridOrder = kMonitorLayer;
const tDisplayOrder kCaldoriaBombTimerOrder = kCaldoriaBombGridOrder + 1;

/////////////////////////////////////////////
//
//	Caldoria

const tCoordType kCaldoriaVidPhoneLeft = kNavAreaLeft + 105;
const tCoordType kCaldoriaVidPhoneTop = kNavAreaTop + 28;

const tCoordType kCaldoria4DSpritesLeft = kNavAreaLeft + 10;
const tCoordType kCaldoria4DSpritesTop = kNavAreaTop + 142;

const tCoordType kCaldoriaMessageLeft = kNavAreaLeft + 202;
const tCoordType kCaldoriaMessageTop = kNavAreaTop + 26;

const tCoordType kCaldoriaElevatorLeft = kNavAreaLeft + 407;
const tCoordType kCaldoriaElevatorTop = kNavAreaTop + 138;

const tCoordType kCaldoriaA05LightLoopLeft = kNavAreaLeft + 213;
const tCoordType kCaldoriaA05LightLoopTop = kNavAreaTop + 215;

const tCoordType kCaldoriaA07LightLoopLeft = kNavAreaLeft + 414;
const tCoordType kCaldoriaA07LightLoopTop = kNavAreaTop + 215;

const tCoordType kCaldoriaGunSpriteLeft = kNavAreaLeft + 276;
const tCoordType kCaldoriaGunSpriteTop = kNavAreaTop + 115;

const tCoordType kCaldoria11MessageLoopLeft = kNavAreaLeft + 135;
const tCoordType kCaldoria11MessageLoopTop = kNavAreaTop + 214;

const tCoordType kCaldoria12MessageLoopLeft = kNavAreaLeft + 209;
const tCoordType kCaldoria12MessageLoopTop = kNavAreaTop + 170;

const tCoordType kCaldoria13MessageLoopLeft = kNavAreaLeft + 480;
const tCoordType kCaldoria13MessageLoopTop = kNavAreaTop + 191;

const tCoordType kCaldoria14MessageLoopLeft = kNavAreaLeft + 248;
const tCoordType kCaldoria14MessageLoopTop = kNavAreaTop + 191;

const tCoordType kCaldoria48CardBombLoopLeft = kNavAreaLeft + 337;
const tCoordType kCaldoria48CardBombLoopTop = kNavAreaTop + 205;

const tCoordType kCaldoriaBombGridLeft = kNavAreaLeft + 290;
const tCoordType kCaldoriaBombGridTop = kNavAreaTop + 58;

const tCoordType kCaldoriaBombTimerLeft = kNavAreaLeft + 58;
const tCoordType kCaldoriaBombTimerTop = kNavAreaTop + 204;

//	Caldoria display IDs.

const tDisplayElementID kCaldoriaVidPhoneID = kNeighborhoodDisplayID;
const tDisplayElementID kCaldoria4DSpritesID = kCaldoriaVidPhoneID + 1;
const tDisplayElementID kCaldoriaMessagesID = kCaldoria4DSpritesID + 1;
const tDisplayElementID kCaldoriaUtilityID = kCaldoriaMessagesID + 1;
const tDisplayElementID kCaldoriaBombGridID = kCaldoriaUtilityID + 1;
const tDisplayElementID kCaldoriaBombTimerID = kCaldoriaBombGridID + 1;

const TimeValue kCaldoria4DBlankChoiceIn = 29730;
const TimeValue kCaldoria4DBlankChoiceOut = 33910;

class Caldoria : public Neighborhood {
friend void doorBombTimerExpiredFunction(FunctionPtr *, void *);
friend void sinclairTimerExpiredFunction(FunctionPtr *, void *);

public:
	Caldoria(InputHandler *, PegasusEngine *);
	virtual ~Caldoria();
	
	virtual uint16 getDateResID() const;
	
	void pickedUpItem(Item *);
	
	virtual GameInteraction *makeInteraction(const tInteractionID);
	
	virtual Common::String getBriefingMovie();
	virtual Common::String getEnvScanMovie();
	virtual uint getNumHints();
	virtual Common::String getHintMovie(uint);
	void loadAmbientLoops();
	bool wantsCursor();
	void flushGameState();

	void checkContinuePoint(const tRoomID, const tDirectionConstant);

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
	TimeValue getViewTime(const tRoomID, const tDirectionConstant);
	void findSpotEntry(const tRoomID, const tDirectionConstant, tSpotFlags, SpotTable::Entry &);
	void startSpotOnceOnly(TimeValue, TimeValue);
	void startExitMovie(const ExitTable::Entry &);
	void startZoomMovie(const ZoomTable::Entry &);
	void startDoorOpenMovie(const TimeValue, const TimeValue);
	void startTurnPush(const tTurnDirection, const TimeValue, const tDirectionConstant);
	void bumpIntoWall();
	int16 getStaticCompassAngle(const tRoomID, const tDirectionConstant);
	void getExitCompassMove(const ExitTable::Entry &, FaderMoveSpec &);
	void getZoomCompassMove(const ZoomTable::Entry &, FaderMoveSpec &);
	void getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &);
	void spotCompleted();
	void arriveAt(const tRoomID, const tDirectionConstant);
	void arriveAtCaldoria00();
	void arriveAtCaldoriaToilet();
	void arriveAtCaldoria44();
	void arriveAtCaldoria49();
	void arriveAtCaldoria56();
	void arriveAtCaldoriaDeath();
	void turnTo(const tDirectionConstant);
	void zoomTo(const Hotspot *);
	void downButton(const Input &);
	void receiveNotification(Notification *, const tNotificationFlags);
	tInputBits getInputFilter();
	void activateHotspots();
	void clickInHotspot(const Input &, const Hotspot *);
	void newInteraction(const tInteractionID);
	
	void clickOnDoorbell(const tHotSpotID);
	
	Hotspot *getItemScreenSpot(Item *, DisplayElement *);
	void dropItemIntoRoom(Item *, Hotspot *);
	void takeElevator(uint, uint);
	void updateElevatorMovie();
	void openElevatorMovie();
	void emptyOJGlass();
	void closeDoorOffScreen(const tRoomID, const tDirectionConstant);
	void doorBombTimerExpired();
	void sinclairTimerExpired();
	void checkSinclairShootsOS();
	void setUpSinclairLoops();
	void zoomToSinclair();
	void playEndMessage();
	void checkInterruptSinclair();
	
	tCanOpenDoorReason canOpenDoor(DoorTable::Entry &);
	void doorOpened();
	
	void updateCursor(const Common::Point, const Hotspot *);

	FlagsArray<uint16, kNumCaldoriaPrivateFlags> _privateFlags;

	const Hotspot *_zoomOutSpot;
	
	FuseFunction _utilityFuse;

	long _sinclairLoopCount;
	long _numSinclairLoops;

	Sprite *_gunSprite;

	Common::String getSoundSpotsName();
	Common::String getNavMovieName();
};

} // End of namespace Pegasus

#endif
