/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

// BUGFIX: some of the audio filenames in the text were incorrect? (see text.h)

#include "startrek/room.h"

#define OBJECT_CABINET 8
#define OBJECT_SYNTHESIZER_DOOR 9
#define OBJECT_DOOR 10
#define OBJECT_GAS_FEED 11
#define OBJECT_CAN1 12
#define OBJECT_CAN2 13
#define OBJECT_ANTIGRAV 14
#define OBJECT_SYNTHESIZER_OUTPUT 15
#define OBJECT_POLYBERYLCARBONATE 16
#define OBJECT_VIRUSSAMPLE 17
#define OBJECT_CURESAMPLE 18
#define OBJECT_CURE 19

#define HOTSPOT_SYNTHESIZER 0x20
#define HOTSPOT_ARDAK 0x21
#define HOTSPOT_DOOR 0x22
#define HOTSPOT_CANISTER_SLOT 0x23

namespace StarTrek {

extern const RoomAction love2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::love2Tick1 },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0}, &Room::love2TouchedWarp1 },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0}, &Room::love2Timer3Expired },
	{ {ACTION_WALK, 10, 0, 0}, &Room::love2WalkToDoor },
	{ {ACTION_WALK, 0x22, 0, 0}, &Room::love2WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::love2TouchedHotspot0 },
	{ {ACTION_FINISHED_WALKING, 14, 0, 0}, &Room::love2DoorReachedOrOpened },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::love2DoorReachedOrOpened },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::love2LookAtCabinet },
	{ {ACTION_LOOK, 10, 0, 0}, &Room::love2LookAtDoor },
	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::love2LookAtSynthesizer },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::love2LookAtAnywhere },
	{ {ACTION_LOOK, 14, 0, 0}, &Room::love2LookAtAntigrav },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::love2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::love2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::love2LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0}, &Room::love2LookAtKirk },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::love2LookAtArdak },
	{ {ACTION_LOOK, 15, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 16, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 18, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 19, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 17, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 9, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 12, 0, 0}, &Room::love2LookAtCan1 },
	{ {ACTION_LOOK, 13, 0, 0}, &Room::love2LookAtCan2 },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0}, &Room::love2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::love2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::love2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::love2TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::love2UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::love2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_KIRK, 8, 0}, &Room::love2UseKirkOnCabinet },
	{ {ACTION_USE, OBJECT_SPOCK, 8, 0}, &Room::love2UseSpockOnCabinet },
	{ {ACTION_USE, OBJECT_MCCOY, 8, 0}, &Room::love2UseMccoyOnCabinet },
	{ {ACTION_USE, OBJECT_REDSHIRT, 8, 0}, &Room::love2UseRedshirtOnCabinet },
	{ {ACTION_FINISHED_WALKING, 12, 0, 0}, &Room::love2CrewmanReachedCabinet },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0}, &Room::love2CrewmanAccessedCabinet },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::love2CrewmanOpenedOrClosedCabinet },
	{ {ACTION_USE, OBJECT_IWRENCH, 11, 0}, &Room::love2UseWrenchOnGasFeed },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::love2ReachedGasFeed },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0}, &Room::love2ChangedGasFeed },
	{ {ACTION_USE, OBJECT_IO2GAS, 0x23, 0}, &Room::love2UseO2GasOnCanisterSlot },
	{ {ACTION_USE, OBJECT_IH2GAS, 0x23, 0}, &Room::love2UseH2GasOnCanisterSlot },
	{ {ACTION_USE, OBJECT_IN2GAS, 0x23, 0}, &Room::love2UseN2GasOnCanisterSlot },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0}, &Room::love2ReachedCanisterSlot },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0}, &Room::love2PutCanisterInSlot1 },
	{ {ACTION_FINISHED_ANIMATION, 14, 0, 0}, &Room::love2PutCanisterInSlot2 },
	{ {ACTION_USE, OBJECT_IANTIGRA, 12, 0}, &Room::love2UseAntigravOnCanister1 },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0}, &Room::love2ReachedCanisterSlot1ToGet },
	{ {ACTION_FINISHED_ANIMATION, 15, 0, 0}, &Room::love2TookCanister1 },
	{ {ACTION_USE, OBJECT_IANTIGRA, 13, 0}, &Room::love2UseAntigravOnCanister2 },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0}, &Room::love2ReachedCanisterSlot2ToGet },
	{ {ACTION_FINISHED_ANIMATION, 16, 0, 0}, &Room::love2TookCanister2 },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0}, &Room::love2UseKirkOnSynthesizer },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0}, &Room::love2UseSpockOnSynthesizer },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0}, &Room::love2UseMccoyOnSynthesizer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0}, &Room::love2UseRedshirtOnSynthesizer },
	{ {ACTION_FINISHED_WALKING, 13, 0, 0}, &Room::love2CrewmanReachedSynthesizer },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0}, &Room::love2CrewmanUsedSynthesizer },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0}, &Room::love2SpockReachedGasFeeds },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0}, &Room::love2SpockEnabledGasFeeds },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0}, &Room::love2SynthesizerDoorClosed },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::love2SynthesizerFinished },
	{ {ACTION_FINISHED_ANIMATION, 27, 0, 0}, &Room::love2ClosedSynthesizerDoorMakingRLG },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0}, &Room::love2ClosedSynthesizerDoorMakingCure },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::love2CureStartedSynthesizing },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0}, &Room::love2CureSynthesized },
	{ {ACTION_USE, OBJECT_IPBC, 9, 0}, &Room::love2UsePolyberylcarbonateOnSynthesizerDoor },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0}, &Room::love2KirkReachedSynthesizerWithPolyberylcarbonate },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0}, &Room::love2SynthesizerDoorOpenedWithPolyberylcarbonate },
	{ {ACTION_FINISHED_ANIMATION, 19, 0, 0}, &Room::love2PutPolyberylcarbonateInSynthesizer },

	{ {ACTION_USE, OBJECT_IDISHES, 9, 0}, &Room::love2UseVirusSampleOnSynthesizerDoor },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0}, &Room::love2KirkReachedSynthesizerWithVirusSample },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0}, &Room::love2SynthesizerDoorOpenedWithVirusSample },
	{ {ACTION_FINISHED_ANIMATION, 20, 0, 0}, &Room::love2PutVirusSampleInSynthesizer },

	{ {ACTION_USE, OBJECT_ISAMPLE, 9, 0}, &Room::love2UseCureSampleOnSynthesizerDoor },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0}, &Room::love2KirkReachedSynthesizerWithCureSample },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0}, &Room::love2SynthesizerDoorOpenedWithCureSample },
	{ {ACTION_FINISHED_ANIMATION, 21, 0, 0}, &Room::love2PutCureSampleInSynthesizer },

	{ {ACTION_USE, 0xff, 9, 0}, &Room::love2UseAnythingOnSynthesizerDoor },
	{ {ACTION_USE, 0xff, 0x20, 0}, &Room::love2UseAnythingOnSynthesizer },
	{ {ACTION_GET, 13, 0, 0}, &Room::love2GetCanister },
	{ {ACTION_GET, 12, 0, 0}, &Room::love2GetCanister },

	{ {ACTION_GET, 14, 0, 0}, &Room::love2GetAntigrav },
	{ {ACTION_FINISHED_WALKING, 9, 0, 0}, &Room::love2ReachedAntigrav },
	{ {ACTION_FINISHED_ANIMATION, 22, 0, 0}, &Room::love2GotAntigrav },

	{ {ACTION_GET, 16, 0, 0}, &Room::love2GetPolyberylcarbonate },
	{ {ACTION_FINISHED_WALKING, 18, 0, 0}, &Room::love2ReachedPolyberylcarbonate },
	{ {ACTION_FINISHED_ANIMATION, 26, 0, 0}, &Room::love2GotPolyberylcarbonate },

	{ {ACTION_GET, 17, 0, 0}, &Room::love2GetDishes },
	{ {ACTION_FINISHED_WALKING, 17, 0, 0}, &Room::love2ReachedDishes },
	{ {ACTION_FINISHED_ANIMATION, 25, 0, 0}, &Room::love2GotDishes },

	{ {ACTION_GET, 18, 0, 0}, &Room::love2GetSample },
	{ {ACTION_FINISHED_WALKING, 19, 0, 0}, &Room::love2ReachedSample },
	{ {ACTION_FINISHED_ANIMATION, 28, 0, 0}, &Room::love2GotSample },

	{ {ACTION_GET, 15, 0, 0}, &Room::love2GetSynthesizerOutput },
	{ {ACTION_FINISHED_WALKING, 10, 0, 0}, &Room::love2ReachedSynthesizerOutput },
	{ {ACTION_FINISHED_ANIMATION, 23, 0, 0}, &Room::love2GotSynthesizerOutput },

	{ {ACTION_GET, 19, 0, 0}, &Room::love2GetCure },
	{ {ACTION_FINISHED_WALKING, 11, 0, 0}, &Room::love2ReachedCure },
	{ {ACTION_FINISHED_ANIMATION, 24, 0, 0}, &Room::love2GotCure },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::love2Tick1() {
	playVoc("LOV2LOOP");

	char canName[10];

	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		strcpy(canName, "o2can");
		break;
	case CANTYPE_H2:
		strcpy(canName, "h2can");
		break;
	case CANTYPE_N2:
		strcpy(canName, "n2can");
		break;
	default:
		break;
	}

	if (_awayMission->love.canister1 >= 1 && _awayMission->love.canister1 <= 3)
		loadActorAnim(OBJECT_CAN1, canName, 0xa7, 0xae, 0);

	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		strcpy(canName, "o2can");
		break;
	case CANTYPE_H2:
		strcpy(canName, "h2can");
		break;
	case CANTYPE_N2:
		strcpy(canName, "n2can");
		break;
	default:
		break;
	}

	if (_awayMission->love.canister2 >= 1 && _awayMission->love.canister2 <= 3)
		loadActorAnim(OBJECT_CAN2, canName, 0xb1, 0xaf, 0);

	if (_awayMission->love.gasFeedOn)
		loadActorAnim(OBJECT_GAS_FEED, "s3r3vo", 0xac, 0x75, 0);
	else
		loadActorAnim(OBJECT_GAS_FEED, "s3r3vc", 0xac, 0x75, 0);

	if (_awayMission->love.cabinetOpen)
		loadActorAnim(OBJECT_CABINET, "s3r3d1o", 0x1e, 0xaf, 0);
	else
		loadActorAnim(OBJECT_CABINET, "s3r3d1c", 0x1e, 0xaf, 0);

	if (_awayMission->love.cabinetOpen && !haveItem(OBJECT_IANTIGRA))
		loadActorAnim(OBJECT_ANTIGRAV, "antigr", 0x1f, 0xa7, 0);

	if (_awayMission->love.synthesizerContents != 0)
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2o", 0x8a, 0x8d, 0);
	else
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2c", 0x8a, 0x8d, 0);

	bool valid = true;

	switch (_awayMission->love.synthesizerContents) {
	case SYNTHITEM_PBC:
		strcpy(_roomVar.love.chamberInputAnim, "pbcanm");
		_roomVar.love.chamberObject = OBJECT_POLYBERYLCARBONATE;
		break;
	case SYNTHITEM_VIRUS_SAMPLE:
		strcpy(_roomVar.love.chamberInputAnim, "dishes");
		_roomVar.love.chamberObject = OBJECT_VIRUSSAMPLE;
		break;
	case SYNTHITEM_CURE_SAMPLE:
		strcpy(_roomVar.love.chamberInputAnim, "dishes");
		_roomVar.love.chamberObject = OBJECT_CURESAMPLE;
		break;
	case SYNTHITEM_BOTTLE:
		switch (_awayMission->love.synthesizerBottleIndex) {
		case 1:
			strcpy(_roomVar.love.chamberOutputAnim, "btle1");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		case 2:
			strcpy(_roomVar.love.chamberOutputAnim, "btle2");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		case 3:
			strcpy(_roomVar.love.chamberOutputAnim, "btle3");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		case 4:
			strcpy(_roomVar.love.chamberOutputAnim, "btle4");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		default:
			strcpy(_roomVar.love.chamberOutputAnim, "cure");
			_roomVar.love.chamberObject = OBJECT_CURE;
			break;
		}
		break;
	default:
		valid = false;
		break;
	}

	if (valid) {
		if (_awayMission->love.synthesizerContents == 9)
			loadActorAnim2(_roomVar.love.chamberObject, _roomVar.love.chamberOutputAnim, 0x8a, 0x8b, 0);
		else
			loadActorAnim2(_roomVar.love.chamberObject, _roomVar.love.chamberInputAnim, 0x8a, 0x8b, 0);
	}

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	loadActorAnim(OBJECT_DOOR, "s3r3d3a", 0x12a, 0xb5, 0);

	_roomVar.love.cmnXPosToCureSpock = 0xc9;
	_roomVar.love.cmnYPosToCureSpock = 0xb7;
}

void Room::love2TouchedWarp1() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	_awayMission->timers[3] = 10;
}

void Room::love2Timer3Expired() {
	loadRoomIndex(0, 0);
}

void Room::love2WalkToDoor() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0x127, 0xba, 14);
}

// Triggers door opening
void Room::love2TouchedHotspot0() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR, "s3r3d3", 0x12a, 0xb5, 1);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2DoorReachedOrOpened() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(1, 1);
}

void Room::love2LookAtCabinet() {
	showDescription(27, true);
}

void Room::love2LookAtDoor() {
	showDescription(1, true);
}

void Room::love2LookAtSynthesizer() {
	showDescription(28, true);
}

void Room::love2LookAtAnywhere() {
	showDescription(26, true);
}

void Room::love2LookAtAntigrav() {
	showDescription(6, true);
}

void Room::love2LookAtMccoy() {
	showDescription(11, true);
}

void Room::love2LookAtSpock() {
	showDescription(14, true);
}

void Room::love2LookAtRedshirt() {
	showDescription(10, true);
}

void Room::love2LookAtKirk() {
	showDescription(9, true);
}

void Room::love2LookAtArdak() {
	showDescription(45, true);
	showText(TX_SPEAKER_SPOCK, 37, true);
}

void Room::love2LookAtChamber() {
	showDescription(29, true);
	showText(TX_SPEAKER_SPOCK, 35, true);
	showText(TX_SPEAKER_MCCOY, 19, true);
	showText(TX_SPEAKER_SPOCK, 33, true);
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::love2LookAtCan1() {
	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		showDescription(4, true);
		break;
	case CANTYPE_H2:
		showDescription(2, true);
		break;
	case CANTYPE_N2:
	default:
		showDescription(3, true);
		break;
	}
}

void Room::love2LookAtCan2() {
	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		showDescription(4, true);
		break;
	case CANTYPE_H2:
		showDescription(2, true);
		break;
	case CANTYPE_N2:
	default:
		showDescription(3, true);
		break;
	}
}

void Room::love2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::love2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 29, true);
}

void Room::love2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::love2TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, 41, true);
}

void Room::love2UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, 8, false, true);
	else
		mccoyScan(DIR_S, 12, false);
}

void Room::love2UseSTricorderAnywhere() {
	spockScan(DIR_S, 27, false, true);
}

void Room::love2UseKirkOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_KIRK;
	walkCrewman(OBJECT_KIRK, 0x2b, 0xbb, 12);
}

void Room::love2UseSpockOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_SPOCK;
	walkCrewman(OBJECT_SPOCK, 0x2b, 0xbb, 12);
}

void Room::love2UseMccoyOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_MCCOY;
	walkCrewman(OBJECT_MCCOY, 0x2b, 0xbb, 12);
}

void Room::love2UseRedshirtOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_REDSHIRT;
	walkCrewman(OBJECT_REDSHIRT, 0x2b, 0xbb, 12);
}

void Room::love2CrewmanReachedCabinet() {
	Common::String anim = getCrewmanAnimFilename(_roomVar.love.chamberObject, "usehw");
	loadActorAnim2(_roomVar.love.chamberObject, anim, -1, -1, 11);
}

void Room::love2CrewmanAccessedCabinet() {
	if (_awayMission->love.cabinetOpen)
		loadActorAnim2(OBJECT_CABINET, "s3r3d4", 0x1e, 0xaf, 2);
	else
		loadActorAnim2(OBJECT_CABINET, "s3r3d1", 0x1e, 0xaf, 2);

	playSoundEffectIndex(kSfxDoor);

	if (_awayMission->love.cabinetOpen)
		loadActorStandAnim(OBJECT_ANTIGRAV);

	_awayMission->love.cabinetOpen = !_awayMission->love.cabinetOpen;
}

void Room::love2CrewmanOpenedOrClosedCabinet() {
	if (_awayMission->love.cabinetOpen && !haveItem(OBJECT_IANTIGRA))
		loadActorAnim(OBJECT_ANTIGRAV, "antigr", 0x1f, 0xa7, 0);
}

void Room::love2UseWrenchOnGasFeed() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 1);
}

void Room::love2ReachedGasFeed() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 12);
	playVoc("MUR3E1S");
}

void Room::love2ChangedGasFeed() {
	if (_awayMission->love.gasFeedOn) {
		_awayMission->love.gasFeedOn = false;
		loadActorAnim2(OBJECT_GAS_FEED, "s3r3v2", 0xac, 0x75, 0);
		showDescription(7, true);
	} else {
		_awayMission->love.gasFeedOn = true;
		loadActorAnim2(OBJECT_GAS_FEED, "s3r3v1", 0xac, 0x75, 0);
		showDescription(8, true);
	}
}

void Room::love2UseO2GasOnCanisterSlot() {
	_roomVar.love.canisterType = CANTYPE_O2;
	strcpy(_roomVar.love.canisterAnim, "o2can");
	_roomVar.love.canisterItem = OBJECT_IO2GAS;
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 3);
}

void Room::love2UseH2GasOnCanisterSlot() {
	_roomVar.love.canisterType = CANTYPE_H2;
	strcpy(_roomVar.love.canisterAnim, "h2can");
	_roomVar.love.canisterItem = OBJECT_IH2GAS;
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 3);
}

void Room::love2UseN2GasOnCanisterSlot() {
	_roomVar.love.canisterType = CANTYPE_N2;
	strcpy(_roomVar.love.canisterAnim, "n2can");
	_roomVar.love.canisterItem = OBJECT_IN2GAS;
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 3);
}

void Room::love2ReachedCanisterSlot() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, 26, true);
	else {
		if (_awayMission->love.canister1 == CANTYPE_NONE)
			loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 13);
		else if (_awayMission->love.canister2 == CANTYPE_NONE)
			loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 14);
		else
			showDescription(24, true);
	}
}

void Room::love2PutCanisterInSlot1() {
	playVoc("SE6FOOD");
	loadActorAnim(OBJECT_CAN1, _roomVar.love.canisterAnim, 0xa7, 0xae, 0);
	_awayMission->love.canister1 = _roomVar.love.canisterType;
	showDescription(34, true);
	loseItem(_roomVar.love.canisterItem);
}

void Room::love2PutCanisterInSlot2() {
	playVoc("SE6FOOD");
	loadActorAnim(OBJECT_CAN2, _roomVar.love.canisterAnim, 0xb1, 0xaf, 0);
	_awayMission->love.canister2 = _roomVar.love.canisterType;
	showDescription(35, true);
	loseItem(_roomVar.love.canisterItem);
}

void Room::love2UseAntigravOnCanister1() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 4);
}

void Room::love2ReachedCanisterSlot1ToGet() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, 25, true);
	else {
		loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 15);
		// BUGFIX: original game only played this for canister 2
		playVoc("SE3PLBAT");
	}
}

void Room::love2TookCanister1() {
	showDescription(30, true);
	loadActorStandAnim(OBJECT_CAN1);

	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		giveItem(OBJECT_IO2GAS);
		break;
	case CANTYPE_H2:
		giveItem(OBJECT_IH2GAS);
		break;
	case CANTYPE_N2:
	default:
		giveItem(OBJECT_IN2GAS);
		break;
	}

	_awayMission->love.canister1 = CANTYPE_NONE;
}

void Room::love2UseAntigravOnCanister2() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 5);
}

void Room::love2ReachedCanisterSlot2ToGet() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, 25, true);
	else {
		loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 16);
		playVoc("SE3PLBAT");
	}
}

void Room::love2TookCanister2() {
	showDescription(31, true);
	loadActorStandAnim(OBJECT_CAN2);

	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		giveItem(OBJECT_IO2GAS);
		break;
	case CANTYPE_H2:
		giveItem(OBJECT_IH2GAS);
		break;
	case CANTYPE_N2:
	default:
		giveItem(OBJECT_IN2GAS);
		break;
	}

	_awayMission->love.canister2 = CANTYPE_NONE;
}

void Room::love2UseKirkOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_KIRK;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2UseSpockOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_SPOCK;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2UseMccoyOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_MCCOY;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2UseRedshirtOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2CrewmanReachedSynthesizer() {
	Common::String anim = getCrewmanAnimFilename(_roomVar.love.chamberObject, "usemn");
	loadActorAnim2(_roomVar.love.chamberObject, anim, -1, -1, 17);
}

void Room::love2CrewmanUsedSynthesizer() {
	if (_awayMission->love.gasFeedOn) {
		love2UseSynthesizer();
	} else {
		if (_roomVar.love.cb == 3) {
			if (haveItem(OBJECT_IWRENCH)) {
				showText(TX_SPEAKER_SPOCK, 5, true);
				walkCrewman(OBJECT_SPOCK, 0xa8, 0xb7, 16);
			} else
				showText(TX_SPEAKER_SPOCK, 6, true);
		} else {
			_roomVar.love.cb++;
			showDescription(12, true);
		}
	}
}


void Room::love2SpockReachedGasFeeds() {
	loadActorAnim2(OBJECT_SPOCK, "susehn", -1, -1, 18);
}

void Room::love2SpockEnabledGasFeeds() {
	_awayMission->love.gasFeedOn = true;

	loadActorAnim2(OBJECT_GAS_FEED, "s3r3v1", 0xac, 0x75, 0);
	showDescription(8, true);
	walkCrewman(OBJECT_SPOCK, 0xc6, 0xbb, 0);

	love2UseSynthesizer();
}

// Synthesizer takes the input and produces some output
void Room::love2UseSynthesizer() {
	if (_awayMission->love.canister1 != 0) {
		if (_awayMission->love.canister2 != 0) {
			int c1 = _awayMission->love.canister1;
			int c2 = _awayMission->love.canister2;
			if (c1 > c2) {
				int tmp = c1;
				c1 = c2;
				c2 = tmp;
			}

			if (c1 == CANTYPE_O2 && c2 == CANTYPE_H2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Romulan Laughing Gas
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", -1, -1, 27); // -> love2ClosedSynthesizerDoorMakingRLG
					playSoundEffectIndex(kSfxDoor);
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Wet goo
					showDescription(51, true);
					showText(TX_SPEAKER_KIRK, 2, true);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_CURE_SAMPLE: // Wet goo
					showDescription(50, true);
					showText(TX_SPEAKER_MCCOY, 21, true);
					loadActorStandAnim(OBJECT_CURESAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					showText(TX_SPEAKER_SPOCK, 28, true);
					break;

				case SYNTHITEM_NONE: // Water
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_H2O;
					strcpy(_roomVar.love.chamberOutputAnim, "btle3");
					// Produce bottle
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(kSfxDoor);
					break;
				}
			} else if (c1 == CANTYPE_H2 && c2 == CANTYPE_N2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Inert matter
					showDescription(49, true);
					showText(TX_SPEAKER_SPOCK, 39, true); // BUGFIX: original didn't play audio
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Colorless goo (with useful information about virus in ammonia)
					showDescription(48, true);
					showText(TX_SPEAKER_SPOCK, 34, true); // BUGFIX: original didn't play audio
					showText(TX_SPEAKER_MCCOY, 23, true);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_CURE_SAMPLE: // Cure
					loadActorStandAnim(OBJECT_CURESAMPLE);
					loadActorAnim2(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 8); // -> love2ClosedSynthesizerDoorMakingCure
					playSoundEffectIndex(kSfxDoor);
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					showText(TX_SPEAKER_SPOCK, 28, true);
					break;

				case SYNTHITEM_NONE: // Ammonia
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_NH3;
					strcpy(_roomVar.love.chamberOutputAnim, "btle2");
					// Produce bottle
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(kSfxDoor);
					break;
				}
			} else if (c1 == CANTYPE_O2 && c2 == CANTYPE_N2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Inert matter
					showDescription(49, true);
					showText(TX_SPEAKER_SPOCK, 39, true); // BUGFIX: original didn't play audio
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Wet goo
					showDescription(47, true);
					showText(TX_SPEAKER_MCCOY, 17, true);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_CURE_SAMPLE: // Wet goo
					showDescription(50, true);
					showText(TX_SPEAKER_MCCOY, 21, true);
					loadActorStandAnim(OBJECT_CURESAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					showText(TX_SPEAKER_SPOCK, 28, true);
					break;

				case SYNTHITEM_NONE: // Laughing gas
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_N2O;
					strcpy(_roomVar.love.chamberOutputAnim, "btle1");
					// Produce bottle
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(kSfxDoor);
					break;
				}
			}
		} else {
			playVoc("EFX24");
			showDescription(5, true);
			showText(TX_SPEAKER_MCCOY, 10, true);
		}
	} else {
		playVoc("EFX24");
		showDescription(5, true);
		showText(TX_SPEAKER_SPOCK, 24, true);
	}
}

void Room::love2SynthesizerDoorClosed() {
	playVoc("LD1SCAN");
	loadActorAnim(OBJECT_SYNTHESIZER_OUTPUT, _roomVar.love.chamberOutputAnim, 0x8a, 0x8d, 4); // -> love2SynthesizerFinished
	_awayMission->love.synthesizerContents = SYNTHITEM_BOTTLE;
}

// Final product of synthesizer is produced
void Room::love2SynthesizerFinished() {
	switch (_awayMission->love.synthesizerBottleIndex) {
	case BOTTLETYPE_N2O:
		showDescription(17, true);
		showText(TX_SPEAKER_SPOCK, 32);
		showText(TX_SPEAKER_MCCOY, 15);
		showText(TX_SPEAKER_SPOCK, 36);
		break;

	case BOTTLETYPE_NH3:
		showDescription(16, true);
		if (!_awayMission->redshirtDead) {
			showText(TX_SPEAKER_FERRIS, 43, true);
			showText(TX_SPEAKER_KIRK, 3, true);
		}
		break;

	case BOTTLETYPE_H2O:
		showDescription(18, true);
		showText(TX_SPEAKER_SPOCK, 4, true);
		break;

	case BOTTLETYPE_RLG:
	default:
		showDescription(19, true);
		showText(TX_SPEAKER_SPOCK, 31, true);
		showText(TX_SPEAKER_MCCOY, 18, true);
		showText(TX_SPEAKER_SPOCK, 30, true);
		showText(TX_SPEAKER_MCCOY, 22, true);
		showText(TX_SPEAKER_SPOCK, 38, true);
		break;
	}
}

void Room::love2ClosedSynthesizerDoorMakingRLG() {
	_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_RLG;
	strcpy(_roomVar.love.chamberOutputAnim, "btle4");
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
	playSoundEffectIndex(kSfxDoor);
}

// Synthesizer door closed, using N2 + O2 on cure sample
void Room::love2ClosedSynthesizerDoorMakingCure() {
	loadActorAnim2(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 9); // -> love2CureStartedSynthesizing
}

void Room::love2CureStartedSynthesizing() {
	loadActorAnim(OBJECT_CURE, "cure", 0x8a, 0x8b, 10); // -> love2CureSynthesized
}

void Room::love2CureSynthesized() {
	showDescription(46, true);
	showText(TX_SPEAKER_MCCOY, 20, true);
}


// Put polycarbonate in synthesizer
void Room::love2UsePolyberylcarbonateOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 6);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithPolyberylcarbonate() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(25, true);
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 5);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2SynthesizerDoorOpenedWithPolyberylcarbonate() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 19);
}

void Room::love2PutPolyberylcarbonateInSynthesizer() {
	loadActorAnim(OBJECT_POLYBERYLCARBONATE, "pbcanm", 0x8a, 0x8b, 0);
	loseItem(OBJECT_IPBC);
	_awayMission->love.synthesizerContents = SYNTHITEM_PBC;
}


// Put dish (virus sample) in synthesizer
void Room::love2UseVirusSampleOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 7);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithVirusSample() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(25, true);
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 6);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2SynthesizerDoorOpenedWithVirusSample() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 20);
}

void Room::love2PutVirusSampleInSynthesizer() {
	loadActorAnim(OBJECT_VIRUSSAMPLE, "dishes", 0x8a, 0x8b, 0);
	loseItem(OBJECT_IDISHES);
	_awayMission->love.synthesizerContents = SYNTHITEM_VIRUS_SAMPLE;
}


// Put cure sample in synthesizer
void Room::love2UseCureSampleOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 8);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithCureSample() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(25, true); // BUGFIX: original didn't play audio
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 7);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2SynthesizerDoorOpenedWithCureSample() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 21);
}

void Room::love2PutCureSampleInSynthesizer() {
	loadActorAnim(OBJECT_CURESAMPLE, "dishes", 0x8a, 0x8b, 0);
	loseItem(OBJECT_ISAMPLE);
	_awayMission->love.synthesizerContents = SYNTHITEM_CURE_SAMPLE;
}



void Room::love2UseAnythingOnSynthesizerDoor() {
	showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::love2UseAnythingOnSynthesizer() {
	showText(TX_SPEAKER_SPOCK, 7, true);
}

void Room::love2GetCanister() {
	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_MCCOY, 11, true);
	else
		showText(TX_SPEAKER_FERRIS, 40, true);
}


// Get antigrav from cabinet
void Room::love2GetAntigrav() {
	walkCrewman(OBJECT_KIRK, 0x37, 0xb4, 9);
}

void Room::love2ReachedAntigrav() {
	loadActorAnim2(OBJECT_KIRK, "kusehw", -1, -1, 22);
}

void Room::love2GotAntigrav() {
	loadActorStandAnim(OBJECT_ANTIGRAV);
	giveItem(OBJECT_IANTIGRA);
	showDescription(42, true);
}


// Get polyberylcarbonate from synthesizer
void Room::love2GetPolyberylcarbonate() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 18);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedPolyberylcarbonate() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 26);
}

void Room::love2GotPolyberylcarbonate() {
	loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
	showDescription(44, true);
	giveItem(OBJECT_IPBC);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
}


// Get virus sample from synthesizer
void Room::love2GetDishes() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 17);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedDishes() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 25);
}

void Room::love2GotDishes() {
	loadActorStandAnim(OBJECT_VIRUSSAMPLE);
	showDescription(36, true); // FIXME: text doesn't match audio
	giveItem(OBJECT_IDISHES);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
}


// Get cure sample from synthesizer
void Room::love2GetSample() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 19);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedSample() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 28);
}

void Room::love2GotSample() {
	loadActorStandAnim(OBJECT_CURESAMPLE);
	showDescription(37, true);
	giveItem(OBJECT_ISAMPLE);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
}


// Get synthesizer output
void Room::love2GetSynthesizerOutput() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 10);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedSynthesizerOutput() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 23);
}

void Room::love2GotSynthesizerOutput() {
	switch (_awayMission->love.synthesizerBottleIndex) {
	case BOTTLETYPE_N2O:
		showDescription(40, true);
		giveItem(OBJECT_IN2O);
		break;

	case BOTTLETYPE_NH3:
		showDescription(39, true);
		giveItem(OBJECT_INH3);
		break;

	case BOTTLETYPE_H2O:
		showDescription(41, true);
		giveItem(OBJECT_IH2O);
		break;

	case BOTTLETYPE_RLG:
	default:
		showDescription(38, true);
		if (!_awayMission->love.gotTLDH) {
			_awayMission->love.gotTLDH = true;
			_awayMission->love.missionScore += 4;
		}
		giveItem(OBJECT_IRLG);
		break;
	}

	loadActorStandAnim(OBJECT_SYNTHESIZER_OUTPUT);
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);

	_awayMission->love.synthesizerContents = 0;
	_awayMission->love.synthesizerBottleIndex = 0;
}

// Get the cure
void Room::love2GetCure() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 11);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedCure() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 24);
}

void Room::love2GotCure() {
	if (!_awayMission->love.gotCure) {
		_awayMission->love.gotCure = true;
		_awayMission->love.missionScore += 5;
	}

	giveItem(OBJECT_ICURE);
	loadActorStandAnim(OBJECT_CURE);
	showDescription(43, true);

	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
	_awayMission->love.synthesizerContents = 0;
}

}
