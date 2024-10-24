/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

extern const RoomAction demon6ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::demon6Tick1 },
	{ {ACTION_TICK, 30, 0, 0, 0}, &Room::demon6Tick30 },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::demon6SpockReachedComputer },

	{ {ACTION_WALK, 0x26, 0, 0, 0}, &Room::demon6WalkToDoor },
	{ {ACTION_WALK, 10, 0, 0, 0}, &Room::demon6WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::demon6TouchedDoorOpenTrigger },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::demon6DoorOpenedOrReachedDoor },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::demon6DoorOpenedOrReachedDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0, 0}, &Room::demon6UsePhaserOnStephen },
	{ {ACTION_USE, OBJECT_IPHASERS, 8, 0, 0}, &Room::demon6UsePhaserOnStephen },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x20, 0, 0}, &Room::demon6UsePhaserOnCase },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x20, 0, 0}, &Room::demon6UsePhaserOnCase },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::demon6LookAtWorkspace },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::demon6LookAtCase },
	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::demon6LookAtTable },
	{ {ACTION_LOOK, OBJECT_IMINERAL, 0, 0, 0}, &Room::demon6LookAtMineral },
	{ {ACTION_LOOK, OBJECT_ISHELLS, 0, 0, 0}, &Room::demon6LookAtShells },
	{ {ACTION_LOOK, OBJECT_ISKULL, 0, 0, 0}, &Room::demon6LookAtSkull },
	{ {ACTION_LOOK, OBJECT_IMETAL, 0, 0, 0}, &Room::demon6LookAtMetal },
	{ {ACTION_LOOK, OBJECT_IMETEOR, 0, 0, 0}, &Room::demon6LookAtMeteor },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::demon6LookAtMountain },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::demon6LookAtSynthesizer },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::demon6LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon6LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon6LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon6LookAtRedshirt },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::demon6LookAtComputer },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::demon6LookAtComputer },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::demon6LookAnywhere },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::demon6LookAtStephen },

	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon6TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon6TalkToSpock },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::demon6TalkToKirk },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon6TalkToRedshirt },
	{ {ACTION_TALK, 8, 0, 0, 0}, &Room::demon6TalkToStephen },
	{ {ACTION_USE, OBJECT_IBERRY, 8, 0, 0}, &Room::demon6UseBerryOnStephen },
	{ {ACTION_USE, OBJECT_IBERRY, 0x23, 0, 0}, &Room::demon6UseBerryOnSynthesizer },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::demon6MccoyReachedSynthesizer },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::demon6FinishedMakingHypoDytoxin },
	{ {ACTION_USE, OBJECT_IHAND, 0x22, 0, 0}, &Room::demon6UseHandOnWorkspace },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::demon6SpockReachedWorkspace },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::demon6SpockFinishedRepairingHand },
	{ {ACTION_USE, 0xff, 0x22, 0, 0}, &Room::demon6UseAnythingOnWorkspace },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0, 0}, &Room::demon6UseCrewmanOnCase },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0, 0}, &Room::demon6UseCrewmanOnCase },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0, 0}, &Room::demon6UseCrewmanOnCase },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0, 0}, &Room::demon6UseCrewmanOnCase },

	{ {ACTION_USE, OBJECT_KIRK, 0x21, 0, 0}, &Room::demon6UseKirkOnComputer },
	{ {ACTION_USE, OBJECT_KIRK, 9, 0, 0}, &Room::demon6UseKirkOnComputer },
	{ {ACTION_USE, OBJECT_MCCOY, 0x21, 0, 0}, &Room::demon6UseMccoyOnComputer },
	{ {ACTION_USE, OBJECT_MCCOY, 9, 0, 0}, &Room::demon6UseMccoyOnComputer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x21, 0, 0}, &Room::demon6UseRedshirtOnComputer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 9, 0, 0}, &Room::demon6UseRedshirtOnComputer },
	{ {ACTION_USE, OBJECT_SPOCK, 0x21, 0, 0}, &Room::demon6UseSpockOnComputer },
	{ {ACTION_USE, OBJECT_SPOCK, 9, 0, 0}, &Room::demon6UseSpockOnComputer },
	{ {ACTION_USE, OBJECT_IMINERAL, 8, 0, 0}, &Room::demon6UseMineralOnStephen },
	{ {ACTION_USE, OBJECT_ISHELLS, 8, 0, 0}, &Room::demon6UseShellsOnStephen },
	{ {ACTION_USE, OBJECT_IMETEOR, 8, 0, 0}, &Room::demon6UseMeteorOnStephen },
	{ {ACTION_USE, OBJECT_ISKULL, 8, 0, 0}, &Room::demon6UseSkullOnStephen },
	{ {ACTION_USE, OBJECT_IMETAL, 8, 0, 0}, &Room::demon6UseMetalOnStephen },
	{ {ACTION_USE, OBJECT_IHAND, 8, 0, 0}, &Room::demon6UseHandOnStephen },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0, 0}, &Room::demon6UseMTricoderOnStephen },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0, 0}, &Room::demon6UseSTricoderOnTable },
	{ {ACTION_USE, OBJECT_ISTRICOR, 9, 0, 0}, &Room::demon6UseSTricoderOnComputer },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::demon6UseSTricoderOnComputer },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IMINERAL, 0, 0}, &Room::demon6UseSTricoderOnMineral },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_ISHELLS, 0, 0}, &Room::demon6UseSTricoderOnShells },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_ISKULL, 0, 0}, &Room::demon6UseSTricoderOnSkull },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IMETAL, 0, 0}, &Room::demon6UseSTricoderOnMetal },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IMETEOR, 0, 0}, &Room::demon6UseSTricoderOnMeteor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::demon6UseSTricoderOnCase },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::demon6UseSTricoderOnSynthesizer },
	{ {ACTION_GET, 0x20, 0, 0, 0}, &Room::demon6GetCase },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::demon6KirkReachedCase },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum demon6TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
	TX_SPEAKER_STEPHEN,
	TX_DEM6_001, TX_DEM6_002, TX_DEM6_003, TX_DEM6_004, TX_DEM6_005,
	TX_DEM6_006, TX_DEM6_007, TX_DEM6_008, TX_DEM6_009, TX_DEM6_010,
	TX_DEM6_011, TX_DEM6_012, TX_DEM6_013, TX_DEM6_014, TX_DEM6_015,
	TX_DEM6_016, TX_DEM6_017, TX_DEM6_018, TX_DEM6_019, TX_DEM6_020,
	TX_DEM6_021, TX_DEM6_022, TX_DEM6_023, TX_DEM6_024, TX_DEM6_025,
	TX_DEM6_026, TX_DEM6_027, TX_DEM6_028, TX_DEM6_029, TX_DEM6_030,
	TX_DEM6_031, TX_DEM6_032, TX_DEM6_033, TX_DEM6_034, TX_DEM6_035,
	TX_DEM6_036, TX_DEM6_037, TX_DEM6_038, TX_DEM6_039, TX_DEM6_040,
	TX_DEM6_041, TX_DEM6_042, TX_DEM6_043, TX_DEM6_044, TX_DEM6_045,
	TX_DEM6_046, TX_DEM6_047, TX_DEM6_048, TX_DEM6_049, TX_DEM6_050,
	TX_DEM6_051, TX_DEM6_052, TX_DEM6_053, TX_DEM6N000, TX_DEM6N001,
	TX_DEM6N002, TX_DEM6N003, TX_DEM6N004, TX_DEM6N005, TX_DEM6N006,
	TX_DEM6N007, TX_DEM6N008, TX_DEM6N009, TX_DEM6N010, TX_DEM6N011,
	TX_DEM6N012, TX_DEM6N013, TX_DEM6N014, TX_DEM6N015, TX_DEM6N016,
	TX_DEM6N017, TX_DEM6N018, TX_DEM6N019, TX_DEM6N020, TX_DEM6N021,
	TX_DEM6N022, TX_DEM6N023, TX_DEM6N024, TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets demon6TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 4150, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 4161, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 4171, 0, 0, 0 },
	{ TX_SPEAKER_EVERTS, 4181, 0, 0, 0 },
	{ TX_SPEAKER_STEPHEN, 4195, 0, 0, 0 },
	{ TX_DEM6_001, 6283, 0, 0, 0 },
	{ TX_DEM6_002, 9373, 0, 0, 0 },
	{ TX_DEM6_003, 6387, 0, 0, 0 },
	{ TX_DEM6_004, 2427, 0, 0, 0 },
	{ TX_DEM6_005, 1927, 0, 0, 0 },
	{ TX_DEM6_006, 6339, 0, 0, 0 },
	{ TX_DEM6_007, 3239, 0, 0, 0 },
	{ TX_DEM6_008, 3668, 0, 0, 0 },
	{ TX_DEM6_009, 3057, 0, 0, 0 },
	{ TX_DEM6_010, 10137, 0, 0, 0 },
	{ TX_DEM6_011, 10040, 0, 0, 0 },
	{ TX_DEM6_012, 9812, 0, 0, 0 },
	{ TX_DEM6_013, 9601, 0, 0, 0 },
	{ TX_DEM6_014, 8244, 0, 0, 0 },
	{ TX_DEM6_015, 2549, 0, 0, 0 },
	{ TX_DEM6_016, 2942, 0, 0, 0 },
	{ TX_DEM6_017, 715, 0, 0, 0 },
	{ TX_DEM6_018, 1765, 0, 0, 0 },
	{ TX_DEM6_019, 6103, 0, 0, 0 },
	{ TX_DEM6_020, 8973, 0, 0, 0 },
	{ TX_DEM6_021, 9718, 0, 0, 0 },
	{ TX_DEM6_022, 5898, 0, 0, 0 },
	{ TX_DEM6_023, 10197, 0, 0, 0 },
	{ TX_DEM6_024, 5812, 0, 0, 0 },
	{ TX_DEM6_025, 9188, 0, 0, 0 },
	{ TX_DEM6_026, 1485, 0, 0, 0 },
	{ TX_DEM6_027, 1322, 0, 0, 0 },
	{ TX_DEM6_028, 613, 0, 0, 0 },
	{ TX_DEM6_029, 8443, 0, 0, 0 },
	{ TX_DEM6_030, 6431, 0, 0, 0 },
	{ TX_DEM6_031, 6524, 0, 0, 0 },
	{ TX_DEM6_032, 8881, 0, 0, 0 },
	{ TX_DEM6_033, 7758, 0, 0, 0 },
	{ TX_DEM6_034, 6613, 0, 0, 0 },
	{ TX_DEM6_035, 9475, 0, 0, 0 },
	{ TX_DEM6_036, 7210, 0, 0, 0 },
	{ TX_DEM6_037, 8785, 0, 0, 0 },
	{ TX_DEM6_038, 7315, 0, 0, 0 },
	{ TX_DEM6_039, 6930, 0, 0, 0 },
	{ TX_DEM6_040, 7122, 0, 0, 0 },
	{ TX_DEM6_041, 7566, 0, 0, 0 },
	{ TX_DEM6_042, 4216, 0, 0, 0 },
	{ TX_DEM6_043, 8493, 0, 0, 0 },
	{ TX_DEM6_044, 6213, 0, 0, 0 },
	{ TX_DEM6_045, 6701, 0, 0, 0 },
	{ TX_DEM6_046, 6735, 0, 0, 0 },
	{ TX_DEM6_047, 6761, 0, 0, 0 },
	{ TX_DEM6_048, 6791, 0, 0, 0 },
	{ TX_DEM6_049, 6837, 0, 0, 0 },
	{ TX_DEM6_050, 6868, 0, 0, 0 },
	{ TX_DEM6_051, 6667, 0, 0, 0 },
	{ TX_DEM6_052, 2655, 0, 0, 0 },
	{ TX_DEM6_053, 9413, 0, 0, 0 },
	{ TX_DEM6N000, 5105, 0, 0, 0 },
	{ TX_DEM6N001, 4691, 0, 0, 0 },
	{ TX_DEM6N002, 5174, 0, 0, 0 },
	{ TX_DEM6N003, 4592, 0, 0, 0 },
	{ TX_DEM6N004, 5252, 0, 0, 0 },
	{ TX_DEM6N005, 5058, 0, 0, 0 },
	{ TX_DEM6N006, 4498, 0, 0, 0 },
	{ TX_DEM6N007, 5323, 0, 0, 0 },
	{ TX_DEM6N008, 5424, 0, 0, 0 },
	{ TX_DEM6N009, 5594, 0, 0, 0 },
	{ TX_DEM6N010, 5662, 0, 0, 0 },
	{ TX_DEM6N011, 4909, 0, 0, 0 },
	{ TX_DEM6N012, 6074, 0, 0, 0 },
	{ TX_DEM6N013, 6188, 0, 0, 0 },
	{ TX_DEM6N014, 10359, 0, 0, 0 },
	{ TX_DEM6N015, 5501, 0, 0, 0 },
	{ TX_DEM6N016, 4871, 0, 0, 0 },
	{ TX_DEM6N017, 4960, 0, 0, 0 },
	{ TX_DEM6N018, 6006, 0, 0, 0 },
	{ TX_DEM6N019, 8372, 0, 0, 0 },
	{ TX_DEM6N020, 5750, 0, 0, 0 },
	{ TX_DEM6N021, 1139, 0, 0, 0 },
	{ TX_DEM6N022, 3812, 0, 0, 0 },
	{ TX_DEM6N023, 1687, 0, 0, 0 },
	{ TX_DEM6N024, 1036, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 8214, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText demon6Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::demon6Tick1() {
	playVoc("DEM6LOOP");
	if (_awayMission->demon.gotBerries || (!_awayMission->demon.field3e && _awayMission->demon.field37)) {
		loadActorAnim(8, "oldman", 0x5f, 0xb1, 0);
		_roomVar.demon.stephenInRoom = true;
	}
	loadActorAnim(10, "s0r6d2", 0xa0, 0x92, 0);
	loadActorAnim(9, "scrnan", 0, 0xc7, 0);
}

void Room::demon6Tick30() {
	if (!_awayMission->demon.gotBerries)
		return;
	if (_awayMission->demon.field3e)
		return;
	if (!_awayMission->demon.field37)
		return;
	if (_awayMission->demon.stephenWelcomedToStudy)
		return;

	showText(TX_SPEAKER_STEPHEN, TX_DEM6_042);
	_awayMission->demon.stephenWelcomedToStudy = true;
}

void Room::demon6SpockReachedComputer() {
	showText(TX_SPEAKER_SPOCK, TX_DEM6_014);
}

void Room::demon6WalkToDoor() {
	_awayMission->disableInput = true;
	_roomVar.demon.movingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x94, 4);
}

void Room::demon6TouchedDoorOpenTrigger() {
	if (!_roomVar.demon.movingToDoor)
		return;
	loadActorAnim(10, "s0r6d1", 0xa0, 0x92, 2);
	playSoundEffectIndex(kSfxDoor);
}

void Room::demon6DoorOpenedOrReachedDoor() {
	_roomVar.demon.doorCounter++;
	if (_roomVar.demon.doorCounter == 2)
		loadRoomIndex(0, 2);
}

void Room::demon6UsePhaserOnStephen() {
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_028);
}

void Room::demon6UsePhaserOnCase() {
	showText(TX_SPEAKER_MCCOY, TX_DEM6_017);
}

void Room::demon6LookAtWorkspace() {
	showDescription(TX_DEM6N003);
}

void Room::demon6LookAtCase() {
	showDescription(TX_DEM6N001);
}

void Room::demon6LookAtTable() {
	showDescription(TX_DEM6N007);
}

void Room::demon6LookAtMineral() {
	showDescription(TX_DEM6N016);
}

void Room::demon6LookAtShells() {
	showDescription(TX_DEM6N011);
}

void Room::demon6LookAtSkull() {
	showDescription(TX_DEM6N017);
}

void Room::demon6LookAtMetal() {
	showDescription(TX_DEM6N005);
}

void Room::demon6LookAtMeteor() {
	showDescription(TX_DEM6N000);
}

void Room::demon6LookAtMountain() {
	showDescription(TX_DEM6N002);
}

void Room::demon6LookAtSynthesizer() {
	showDescription(TX_DEM6N004);
}

void Room::demon6LookAtKirk() {
	showDescription(TX_DEM6N008);
}

void Room::demon6LookAtSpock() {
	showDescription(TX_DEM6N015);
}

void Room::demon6LookAtMccoy() {
	showDescription(TX_DEM6N009);
}

void Room::demon6LookAtRedshirt() {
	showDescription(TX_DEM6N010);
}

void Room::demon6LookAtComputer() {
	showDescription(TX_DEM6N006);
	if (!_awayMission->demon.lookedAtComputer) {
		_awayMission->demon.lookedAtComputer = true;
		_awayMission->demon.missionScore++;
	}
}

void Room::demon6LookAnywhere() {
	showDescription(TX_DEM6N024);
}

void Room::demon6LookAtStephen() {
	showDescription(TX_DEM6N021);
}

void Room::demon6TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_DEM6_020);
}

void Room::demon6TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM6_025);
}

void Room::demon6TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM6_002);
}

void Room::demon6TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM6_053);
}

void Room::demon6TalkToStephen() {
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_035);
}

void Room::demon6UseBerryOnStephen() {
	if (_awayMission->demon.knowAboutHypoDytoxin) {
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_027);
	} else {
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_026);
	}
}

void Room::demon6UseBerryOnSynthesizer() {
	if (!_awayMission->demon.knowAboutHypoDytoxin)
		return;
	walkCrewman(OBJECT_MCCOY, 0x104, 0xa5, 1);
}

void Room::demon6MccoyReachedSynthesizer() {
	loadActorAnim2(OBJECT_MCCOY, "musemn", -1, -1, 1);
	playVoc("LD1REPLI");
}

void Room::demon6FinishedMakingHypoDytoxin() {
	playMidiMusicTracks(MIDITRACK_NONE);
	showDescription(TX_DEM6N023);
	showText(TX_SPEAKER_MCCOY, TX_DEM6_018);

	giveItem(OBJECT_IDETOXIN);
	_awayMission->demon.madeHypoDytoxin = true;
	loseItem(OBJECT_IBERRY);

	if (!_awayMission->demon.gavePointsForDytoxin) {
		_awayMission->demon.gavePointsForDytoxin = true;
		_awayMission->demon.missionScore++;
	}
}

void Room::demon6UseHandOnWorkspace() {
	if (_awayMission->demon.repairedHand) {
		showText(TX_SPEAKER_SPOCK, TX_DEM6N018); // FIXME
	} else {
		_awayMission->disableInput = true;
		showText(TX_SPEAKER_KIRK, TX_DEM6_005);
		walkCrewman(OBJECT_SPOCK, 0xb3, 0xbb, 2);
	}
}

void Room::demon6SpockReachedWorkspace() {
	loadActorAnim(OBJECT_SPOCK, "spokop", 0xb3, 0xbb, 3);
}

void Room::demon6SpockFinishedRepairingHand() {
	showText(TX_SPEAKER_SPOCK, TX_DEM6_024);
	_awayMission->demon.repairedHand = true;
	_awayMission->demon.missionScore += 2;
	loadActorStandAnim(OBJECT_SPOCK);
	_awayMission->disableInput = false;
}

// FIXME: doesn't work
void Room::demon6UseAnythingOnWorkspace() {
	showDescription(TX_DEM6N020);
}

void Room::demon6StephenIsInsulted() {
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_030);
	_roomVar.demon.insultedStephenRecently = true;
	_awayMission->demon.insultedStephen = true;
}

void Room::demon6StephenDescribesItemsInCase() {
	while (true) {
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_051);
		TextRef choices2[] = { TX_SPEAKER_STEPHEN, TX_DEM6_045, TX_DEM6_046, TX_DEM6_047, TX_DEM6_048, TX_DEM6_049, TX_DEM6_050, TX_END };
		TextRef choice = showMultipleTexts(choices2);

		switch (choice) {
		case 0:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_040);
			break;
		case 1:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_033);
			break;
		case 2:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_036);
			break;
		case 3:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_038);
			break;
		case 4:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_039);
			break;
		case 5:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_041);
			_roomVar.demon.caseOpened = true;
			return;
		default:
			showDescription(TX_DIALOG_ERROR);
		}
	}
}

void Room::demon6UseCrewmanOnCase() {
	if (_roomVar.demon.stephenInRoom) {
		if (_roomVar.demon.insultedStephenRecently)
			return;
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_044);
		TextRef choices1[] = {TX_SPEAKER_KIRK, TX_DEM6_001, TX_DEM6_006, TX_DEM6_003, TX_END};
		TextRef choice = showMultipleTexts(choices1);

		if (choice == 0) {
			demon6StephenIsInsulted();
		} else if (choice == 1) {
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_034);
			demon6StephenDescribesItemsInCase();
		} else if (choice == 2) {
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_031);
			TextRef choices3[] = {TX_SPEAKER_KIRK, TX_DEM6_001, TX_DEM6_006, TX_END};
			choice = showMultipleTexts(choices3);

			if (choice == 0)
				demon6StephenIsInsulted();
			else if (choice == 1)
				demon6StephenDescribesItemsInCase();
			else
				showDescription(TX_DIALOG_ERROR);
		} else {
			showDescription(TX_DIALOG_ERROR);
		}
	} else {
		showDescription(TX_DEM6N012);
		showText(TX_SPEAKER_MCCOY, TX_DEM6_019);
	}
}

void Room::demon6UseKirkOnComputer() {
	showText(TX_SPEAKER_KIRK, TX_DEM6_004);
	demon6UseSpockOnComputer();
}

void Room::demon6UseMccoyOnComputer() {
	showText(TX_SPEAKER_MCCOY, TX_DEM6_015);
}

void Room::demon6UseRedshirtOnComputer() {
	showText(TX_SPEAKER_EVERTS, TX_DEM6_052);
}

void Room::demon6UseSpockOnComputer() {
	walkCrewman(OBJECT_SPOCK, 0x3d, 0xc5, 3);
}

void Room::demon6UseMineralOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~8;
	demon6ReturnItemToStephen(OBJECT_IMINERAL);
}

void Room::demon6UseShellsOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~2;
	demon6ReturnItemToStephen(OBJECT_ISHELLS);
}

void Room::demon6UseMeteorOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~4;
	demon6ReturnItemToStephen(OBJECT_IMETEOR);
}

void Room::demon6UseSkullOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~16;
	demon6ReturnItemToStephen(OBJECT_ISKULL);
}

void Room::demon6UseMetalOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~1;
	demon6ReturnItemToStephen(OBJECT_IMETAL);
}

void Room::demon6ReturnItemToStephen(int item) {
	loseItem(item);
	showDescription(TX_DEM6N019);
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_029);
}

void Room::demon6UseHandOnStephen() {
	if (_awayMission->demon.repairedHand)
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_037);
	else
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_043);
}

void Room::demon6UseMTricoderOnStephen() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM6_016);
}

void Room::demon6UseSTricoderOnTable() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_009);
}

void Room::demon6UseSTricoderOnComputer() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_007);
}

void Room::demon6UseSTricoderOnMineral() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_013);
}

void Room::demon6UseSTricoderOnShells() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_021);
}

void Room::demon6UseSTricoderOnSkull() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_012);
}

void Room::demon6UseSTricoderOnMetal() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_011);
}

void Room::demon6UseSTricoderOnMeteor() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_010);
}

void Room::demon6UseSTricoderOnCase() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_023);
}

void Room::demon6UseSTricoderOnSynthesizer() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_008);
}

void Room::demon6GetCase() {
	if (!_roomVar.demon.caseOpened)
		showDescription(TX_DEM6N022);
	else
		walkCrewman(OBJECT_KIRK, 0xff, 0xba, 5);
}

void Room::demon6KirkReachedCase() {
	const int items[] = { OBJECT_IMETAL, OBJECT_ISHELLS, OBJECT_IMETEOR, OBJECT_IMINERAL, OBJECT_ISKULL };
	byte ret = 0x1f ^ demon6ShowCase(_awayMission->demon.itemsTakenFromCase ^ 0x1f);
	_awayMission->demon.itemsTakenFromCase = ret;

	const int *i = items;
	while (ret != 0) {
		if (ret & 1)
			giveItem(*i);
		i++;
		ret >>= 1;
	}
}

int Room::demon6ShowCase(int visible) {
	const Common::Point spritePositions[] = {
		Common::Point(0x29, 0x80),
		Common::Point(0x58, 0x7d),
		Common::Point(0x82, 0x7d),
		Common::Point(0xae, 0x79),
		Common::Point(0xe1, 0x7d)
	};

	_vm->_gfx->fadeoutScreen();
	_vm->_gfx->pushSprites();

	_vm->_gfx->setBackgroundImage("stlcase");
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->clearPri();

	Sprite sprites[5];

	for (int i = 0; i < 5; i++) {
		if (!(visible & (0x10 >> i)))
			continue;
		_vm->_gfx->addSprite(&sprites[i]);
		sprites[i].pos.x = spritePositions[i].x;
		sprites[i].pos.y = spritePositions[i].y;
		sprites[i].drawPriority = 2;
		sprites[i].bitmapChanged = true;
		sprites[i].setBitmap(loadBitmapFile(Common::String::format("stlitem%d", i)));
	}

	Sprite buttonSprite;

	// BUGFIX: use draw mode 2 so the entire button is clickable (not just non-transparent parts)
	buttonSprite.drawMode = 2;

	buttonSprite.pos.x = 0xe1;
	buttonSprite.pos.y = 0x19;
	buttonSprite.drawPriority = 2;
	buttonSprite.bitmapChanged = true;
	buttonSprite.setBitmap(loadBitmapFile("donebutt"));

	_vm->_gfx->addSprite(&buttonSprite);
	_vm->_gfx->forceDrawAllSprites();
	_vm->_gfx->fadeinScreen();

	visible = demon6ShowCaseProcessInput(sprites, &buttonSprite, visible);

	_vm->_gfx->fadeoutScreen();
	_vm->_gfx->popSprites();
	_vm->_gfx->loadPri(_vm->getScreenName());
	_vm->_gfx->setBackgroundImage(_vm->getScreenName());
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->forceDrawAllSprites();

	return visible;
}

int Room::demon6ShowCaseProcessInput(Sprite *sprites, Sprite *buttonSprite, int visible) {
	bool exitLoop = false;

	while (!exitLoop) {
		TrekEvent event;
		if (!_vm->popNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_TICK:
			_vm->_gfx->drawAllSprites();
			break;

		case TREKEVENT_LBUTTONDOWN: {
			Sprite *clickedSprite = _vm->_gfx->getSpriteAt(_vm->_gfx->getMousePos());
			if (clickedSprite == buttonSprite)
				exitLoop = true;
			else if (clickedSprite != nullptr)
				visible = demon6ShowCaseProcessSelection(sprites, clickedSprite, visible);
			break;
		}

		case TREKEVENT_RBUTTONDOWN:
			exitLoop = true;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				exitLoop = true;
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1: {
				Sprite *clickedSprite = _vm->_gfx->getSpriteAt(_vm->_gfx->getMousePos());
				if (clickedSprite == buttonSprite)
					exitLoop = true;
				else if (clickedSprite != nullptr)
					visible = demon6ShowCaseProcessSelection(sprites, clickedSprite, visible);
				break;
			}

			case Common::KEYCODE_F2:
				exitLoop = true;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	return visible;
}

int Room::demon6ShowCaseProcessSelection(Sprite *sprites, Sprite *clickedSprite, int visible) {
	while (clickedSprite->pos.y < SCREEN_HEIGHT) { // Move the selected item down and off the screen.
		clickedSprite->drawPriority = 8;
		clickedSprite->bitmapChanged = true;
		_vm->_gfx->drawAllSprites();

		// WORKAROUND: original game had no bound on how fast the items move
		// off the screen. Here I bind it to the tick event.
		// (This was probably the intended behaviour since the original game
		// does check the clock cycle, but doesn't do a proper comparison with
		// it.)
		TrekEvent event;
		do {
			if (!_vm->popNextEvent(&event))
				continue;
		} while (event.type != TREKEVENT_TICK);
		clickedSprite->pos.y++;
	}

	clickedSprite->dontDrawNextFrame();
	_vm->_gfx->drawAllSprites();
	_vm->_gfx->delSprite(clickedSprite);
	delete clickedSprite->bitmap;
	clickedSprite->bitmap = nullptr;
	visible ^= (0x10 >> (clickedSprite - sprites));

	return visible;
}

}
