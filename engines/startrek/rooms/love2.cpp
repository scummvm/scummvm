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
	playSoundEffectIndex(SND_DOOR1);
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
		playSoundEffectIndex(SND_DOOR1);
	}
}

void Room::love2DoorReachedOrOpened() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(1, 1);
}

void Room::love2LookAtCabinet() {
	showDescription(TX_LOV2N027);
}

void Room::love2LookAtDoor() {
	showDescription(TX_LOV2N001);
}

void Room::love2LookAtSynthesizer() {
	showDescription(TX_LOV2N028);
}

void Room::love2LookAtAnywhere() {
	showDescription(TX_LOV2N026);
}

void Room::love2LookAtAntigrav() {
	showDescription(TX_LOV2N006);
}

void Room::love2LookAtMccoy() {
	showDescription(TX_LOV2N011);
}

void Room::love2LookAtSpock() {
	showDescription(TX_LOV2N014);
}

void Room::love2LookAtRedshirt() {
	showDescription(TX_LOV2N010);
}

void Room::love2LookAtKirk() {
	showDescription(TX_LOV2N009);
}

void Room::love2LookAtArdak() {
	showDescription(TX_LOV2N045);
	showText(TX_SPEAKER_SPOCK, TX_LOV2_037);
}

void Room::love2LookAtChamber() {
	showDescription(TX_LOV2N029);
	showText(TX_SPEAKER_SPOCK, TX_LOV2_035);
	showText(TX_SPEAKER_MCCOY, TX_LOV2_019);
	showText(TX_SPEAKER_SPOCK, TX_LOV2_033);
	showText(TX_SPEAKER_MCCOY, TX_LOV2_016);
}

void Room::love2LookAtCan1() {
	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		showDescription(TX_LOV2N004);
		break;
	case CANTYPE_H2:
		showDescription(TX_LOV2N002);
		break;
	case CANTYPE_N2:
	default:
		showDescription(TX_LOV2N003);
		break;
	}
}

void Room::love2LookAtCan2() {
	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		showDescription(TX_LOV2N004);
		break;
	case CANTYPE_H2:
		showDescription(TX_LOV2N002);
		break;
	case CANTYPE_N2:
	default:
		showDescription(TX_LOV2N003);
		break;
	}
}

void Room::love2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV2_001);
}

void Room::love2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_LOV2_029);
}

void Room::love2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV2_009);
}

void Room::love2TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV2_041);
}

void Room::love2UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, TX_LOV2_008, false);
	else
		// BUGFIX: should be TX_LOV2_012, but the audio file is missing. Using equivalent
		// audio from another room.
		mccoyScan(DIR_S, TX_LOV1_010, false);
}

void Room::love2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_LOV2_027, false);
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

	playSoundEffectIndex(SND_DOOR1);

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
		showDescription(TX_LOV2N007);
	} else {
		_awayMission->love.gasFeedOn = true;
		loadActorAnim2(OBJECT_GAS_FEED, "s3r3v1", 0xac, 0x75, 0);
		showDescription(TX_LOV2N008);
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
		showText(TX_SPEAKER_SPOCK, TX_LOV2_026);
	else {
		if (_awayMission->love.canister1 == CANTYPE_NONE)
			loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 13);
		else if (_awayMission->love.canister2 == CANTYPE_NONE)
			loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 14);
		else
			showDescription(TX_LOV2N024);
	}
}

void Room::love2PutCanisterInSlot1() {
	playVoc("SE6FOOD");
	loadActorAnim(OBJECT_CAN1, _roomVar.love.canisterAnim, 0xa7, 0xae, 0);
	_awayMission->love.canister1 = _roomVar.love.canisterType;
	showDescription(TX_LOV2N034);
	loseItem(_roomVar.love.canisterItem);
}

void Room::love2PutCanisterInSlot2() {
	playVoc("SE6FOOD");
	loadActorAnim(OBJECT_CAN2, _roomVar.love.canisterAnim, 0xb1, 0xaf, 0);
	_awayMission->love.canister2 = _roomVar.love.canisterType;
	showDescription(TX_LOV2N035);
	loseItem(_roomVar.love.canisterItem);
}

void Room::love2UseAntigravOnCanister1() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 4);
}

void Room::love2ReachedCanisterSlot1ToGet() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, TX_LOV2_025);
	else {
		loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 15);
		// BUGFIX: original game only played this for canister 2
		playVoc("SE3PLBAT");
	}
}

void Room::love2TookCanister1() {
	showDescription(TX_LOV2N030);
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
		showText(TX_SPEAKER_SPOCK, TX_LOV2_025);
	else {
		loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 16);
		playVoc("SE3PLBAT");
	}
}

void Room::love2TookCanister2() {
	showDescription(TX_LOV2N031);
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
				showText(TX_SPEAKER_SPOCK, TX_LOV2_005);
				walkCrewman(OBJECT_SPOCK, 0xa8, 0xb7, 16);
			} else
				showText(TX_SPEAKER_SPOCK, TX_LOV2_006);
		} else {
			_roomVar.love.cb++;
			showDescription(TX_LOV2N012);
		}
	}
}


void Room::love2SpockReachedGasFeeds() {
	loadActorAnim2(OBJECT_SPOCK, "susehn", -1, -1, 18);
}

void Room::love2SpockEnabledGasFeeds() {
	_awayMission->love.gasFeedOn = true;

	loadActorAnim2(OBJECT_GAS_FEED, "s3r3v1", 0xac, 0x75, 0);
	showDescription(TX_LOV2N008);
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
					playSoundEffectIndex(SND_DOOR1);
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Wet goo
					showDescription(TX_LOV2N051);
					showText(TX_SPEAKER_KIRK, TX_LOV2_002);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					goto closeSynthesizerDoor;

				case SYNTHITEM_CURE_SAMPLE: // Wet goo
wetGooFailure:
					showDescription(TX_LOV2N050);
					showText(TX_SPEAKER_MCCOY, TX_LOV2_021);
					loadActorStandAnim(OBJECT_CURESAMPLE);
					goto closeSynthesizerDoor;

				case SYNTHITEM_BOTTLE: // Nothing happens
bottleFailure:
					showText(TX_SPEAKER_SPOCK, TX_LOV2_028);
					break;

				case SYNTHITEM_NONE: // Water
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_H2O;
					strcpy(_roomVar.love.chamberOutputAnim, "btle3");
produceBottle:
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(SND_DOOR1);
					break;

closeSynthesizerDoor:
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(SND_DOOR1);
					_awayMission->love.synthesizerContents = 0;
					break;
				}
			} else if (c1 == CANTYPE_H2 && c2 == CANTYPE_N2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Inert matter
inertMatterFailure:
					showDescription(TX_LOV2N049);
					showText(TX_SPEAKER_SPOCK, TX_LOV2_039); // BUGFIX: original didn't play audio
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					goto closeSynthesizerDoor;

				case SYNTHITEM_VIRUS_SAMPLE: // Colorless goo (with useful information about virus in ammonia)
					showDescription(TX_LOV2N048);
					showText(TX_SPEAKER_SPOCK, TX_LOV2_034); // BUGFIX: original didn't play audio
					showText(TX_SPEAKER_MCCOY, TX_LOV2_023);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					goto closeSynthesizerDoor;

				case SYNTHITEM_CURE_SAMPLE: // Cure
					loadActorStandAnim(OBJECT_CURESAMPLE);
					loadActorAnim2(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 8); // -> love2ClosedSynthesizerDoorMakingCure
					playSoundEffectIndex(SND_DOOR1);
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					goto bottleFailure;

				case SYNTHITEM_NONE: // Ammonia
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_NH3;
					strcpy(_roomVar.love.chamberOutputAnim, "btle2");
					goto produceBottle;
				}
			} else if (c1 == CANTYPE_O2 && c2 == CANTYPE_N2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Inert matter
					goto inertMatterFailure;

				case SYNTHITEM_VIRUS_SAMPLE: // Wet goo
					showDescription(TX_LOV2N047);
					showText(TX_SPEAKER_MCCOY, TX_LOV2_017);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					goto closeSynthesizerDoor;

				case SYNTHITEM_CURE_SAMPLE: // Wet goo
					goto wetGooFailure;

				case SYNTHITEM_BOTTLE: // Nothing happens
					goto bottleFailure;

				case SYNTHITEM_NONE: // Laughing gas
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_N2O;
					strcpy(_roomVar.love.chamberOutputAnim, "btle1");
					goto produceBottle;
				}
			}
		} else {
			playVoc("EFX24");
			showDescription(TX_LOV2N005);
			showText(TX_SPEAKER_MCCOY, TX_LOV2_010);
		}
	} else {
		playVoc("EFX24");
		showDescription(TX_LOV2N005);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_024);
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
		showDescription(TX_LOV2N017);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_032);
		showText(TX_SPEAKER_MCCOY, TX_LOV2_015);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_036);
		break;

	case BOTTLETYPE_NH3:
		showDescription(TX_LOV2N016);
		if (!_awayMission->redshirtDead) {
			showText(TX_SPEAKER_FERRIS, TX_LOV2_043);
			showText(TX_SPEAKER_KIRK, TX_LOV2_003);
		}
		break;

	case BOTTLETYPE_H2O:
		showDescription(TX_LOV2N018);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_004);
		break;

	case BOTTLETYPE_RLG:
	default:
		showDescription(TX_LOV2N019);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_031);
		showText(TX_SPEAKER_MCCOY, TX_LOV2_018);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_030);
		showText(TX_SPEAKER_MCCOY, TX_LOV2_022);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_038);
		break;
	}
}

void Room::love2ClosedSynthesizerDoorMakingRLG() {
	_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_RLG;
	strcpy(_roomVar.love.chamberOutputAnim, "btle4");
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
	playSoundEffectIndex(SND_DOOR1);
}

// Synthesizer door closed, using N2 + O2 on cure sample
void Room::love2ClosedSynthesizerDoorMakingCure() {
	loadActorAnim2(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 9); // -> love2CureStartedSynthesizing
}

void Room::love2CureStartedSynthesizing() {
	loadActorAnim(OBJECT_CURE, "cure", 0x8a, 0x8b, 10); // -> love2CureSynthesized
}

void Room::love2CureSynthesized() {
	showDescription(TX_LOV2N046);
	showText(TX_SPEAKER_MCCOY, TX_LOV2_020);
}


// Put polycarbonate in synthesizer
void Room::love2UsePolyberylcarbonateOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 6);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithPolyberylcarbonate() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(TX_LOV2N025);
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 5);
		playSoundEffectIndex(SND_DOOR1);
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
		showDescription(TX_LOV2N025);
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 6);
		playSoundEffectIndex(SND_DOOR1);
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
		showDescription(TX_LOV2N025); // BUGFIX: original didn't play audio
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 7);
		playSoundEffectIndex(SND_DOOR1);
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
	showText(TX_SPEAKER_MCCOY, TX_LOV2_014);
}

void Room::love2UseAnythingOnSynthesizer() {
	showText(TX_SPEAKER_SPOCK, TX_LOV2_007);
}

void Room::love2GetCanister() {
	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_MCCOY, TX_LOV2_011);
	else
		showText(TX_SPEAKER_FERRIS, TX_LOV2_040);
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
	showDescription(TX_LOV2N042);
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
	showDescription(TX_LOV2N044);
	giveItem(OBJECT_IPBC);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(SND_DOOR1);
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
	showDescription(TX_LOV2N036); // FIXME: text doesn't match audio
	giveItem(OBJECT_IDISHES);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(SND_DOOR1);
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
	showDescription(TX_LOV2N037);
	giveItem(OBJECT_ISAMPLE);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(SND_DOOR1);
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
		showDescription(TX_LOV2N040);
		giveItem(OBJECT_IN2O);
		break;

	case BOTTLETYPE_NH3:
		showDescription(TX_LOV2N039);
		giveItem(OBJECT_INH3);
		break;

	case BOTTLETYPE_H2O:
		showDescription(TX_LOV2N041);
		giveItem(OBJECT_IH2O);
		break;

	case BOTTLETYPE_RLG:
	default:
		showDescription(TX_LOV2N038);
		if (!_awayMission->love.gotTLDH) {
			_awayMission->love.gotTLDH = true;
			_awayMission->love.missionScore += 4;
		}
		giveItem(OBJECT_IRLG);
		break;
	}

	loadActorStandAnim(OBJECT_SYNTHESIZER_OUTPUT);
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(SND_DOOR1);

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
	showDescription(TX_LOV2N043);

	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(SND_DOOR1);
	_awayMission->love.synthesizerContents = 0;
}

}
