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

#define OBJECT_DOOR 8
#define OBJECT_ROMULAN_1 9
#define OBJECT_ROMULAN_2 10
#define OBJECT_ROMULAN_3 11
#define OBJECT_ROMULAN_4 12

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_LADDER 0x21

namespace StarTrek {

extern const RoomAction love4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::love4Tick1 },
	{ {ACTION_TICK, 10, 0, 0, 0}, &Room::love4Tick10 },

	{ {ACTION_WALK, 8, 0, 0, 0}, &Room::love4WalkToDoor },
	{ {ACTION_WALK, 0x20, 0, 0, 0}, &Room::love4WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::love4TouchedHotspot0 },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::love4DoorOpenedOrReached },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::love4DoorOpenedOrReached },

	{ {ACTION_USE, OBJECT_KIRK, 0x21, 0, 0}, &Room::love4UseKirkOnLadder },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::love4ReachedLadder },
	{ {ACTION_USE, OBJECT_IPHASERS,  9, 0, 0}, &Room::love4UseStunPhaserOnRomulan },
	{ {ACTION_USE, OBJECT_IPHASERS, 10, 0, 0}, &Room::love4UseStunPhaserOnRomulan },
	{ {ACTION_USE, OBJECT_IPHASERS, 11, 0, 0}, &Room::love4UseStunPhaserOnRomulan },
	{ {ACTION_USE, OBJECT_IPHASERS, 12, 0, 0}, &Room::love4UseStunPhaserOnRomulan },
	// NOTE: nothing for OBJECT_IPHASERK. There's an unused audio file which might fit.
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::love4LookAnywhere },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::love4LookAtLadder },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::love4LookAtDoor },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::love4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::love4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::love4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love4LookAtRedshirt },
	{ {ACTION_LOOK,  9, 0, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_LOOK, 11, 0, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_LOOK, 12, 0, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::love4TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::love4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love4TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::love4TalkToSpock },
	{ {ACTION_TALK,  9, 0, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_TALK, 10, 0, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_TALK, 11, 0, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_TALK, 12, 0, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR,  9, 0, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 10, 0, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 11, 0, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 12, 0, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::love4UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::love4UseSTricorderAnywhere },

	{ {ACTION_USE, OBJECT_ICURE,  9, 0, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_USE, OBJECT_ICURE, 10, 0, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_USE, OBJECT_ICURE, 11, 0, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_USE, OBJECT_ICURE, 12, 0, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_FINISHED_WALKING,   3, 0, 0, 0}, &Room::love4MccoyReachedRomulan4 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::love4MccoyCuredRomulan4 },
	{ {ACTION_FINISHED_WALKING,   4, 0, 0, 0}, &Room::love4MccoyReachedRomulan3 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::love4MccoyCuredRomulan3 },
	{ {ACTION_FINISHED_WALKING,   5, 0, 0, 0}, &Room::love4MccoyReachedRomulan2 },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::love4MccoyCuredRomulan2 },
	{ {ACTION_FINISHED_WALKING,   6, 0, 0, 0}, &Room::love4MccoyReachedRomulan1 },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0}, &Room::love4MccoyCuredRomulan1 },

	{ {ACTION_USE, OBJECT_IH2O,   9, 0, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IH2O,  10, 0, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IH2O,  11, 0, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IH2O,  12, 0, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT,  9, 0, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT, 10, 0, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT, 11, 0, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT, 12, 0, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE,  9, 0, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE, 10, 0, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE, 11, 0, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE, 12, 0, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, 0xff,  9, 0, 0}, &Room::love4UseAnythingOnRomulan },
	{ {ACTION_USE, 0xff, 10, 0, 0}, &Room::love4UseAnythingOnRomulan },
	{ {ACTION_USE, 0xff, 11, 0, 0}, &Room::love4UseAnythingOnRomulan },
	{ {ACTION_USE, 0xff, 12, 0, 0}, &Room::love4UseAnythingOnRomulan },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum love4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_FERRIS,
	TX_LOV0_011, TX_LOV0_012, TX_LOV0_013, TX_LOV0_014, TX_LOV0_015,
	TX_LOV0_016, TX_LOV0_017, TX_LOV0_019, TX_LOV0_020, TX_LOV0_021,
	TX_LOV0_022, TX_LOV0_025, TX_LOV0_026, TX_LOV0_029, TX_LOV0_033,
	TX_LOV0_039, TX_LOV0_040, TX_LOV0_041, TX_LOV0_042, TX_LOV0_043,
	TX_LOV0_045, TX_LOV0_046, TX_LOV0_047, TX_LOV0_048, TX_LOV0_049,
	TX_LOV0_050, TX_LOV0_101, TX_LOV0_102, TX_LOV0_103, TX_LOV0_104,
	TX_LOV0_105, TX_LOV0_106, TX_LOV0_107, TX_LOV0_124, TX_LOV4_001,
	TX_LOV4_002, TX_LOV4_003, TX_LOV4_004, TX_LOV4_005, TX_LOV4_006,
	TX_LOV4_007, TX_LOV4_009, TX_LOV4_010, TX_LOV4_011, TX_LOV4_012,
	TX_LOV4_013, TX_LOV4_014, TX_LOV4_015, TX_LOV4_016, TX_LOV4_017,
	TX_LOV4_018, TX_LOV4_019, TX_LOV4_020, TX_LOV4_021, TX_LOV4_022,
	TX_LOV4_023, TX_LOV4_024, TX_LOV4_025, TX_LOV4_026, TX_LOV4_027,
	TX_LOV4_028, TX_LOV4_029, TX_LOV5_015, TX_LOV5_019, TX_LOV5_027,
	TX_LOV5_030, TX_LOV5_038, TX_LOVA_100, TX_LOVA_F01, TX_LOVA_F02,
	TX_LOVA_F03, TX_LOVA_F04, TX_LOVA_F07, TX_LOVA_F08, TX_LOVA_F10,
	TX_LOVA_F54, TX_LOVA_F55, TX_MUD2_040, TX_MUD4_018, TX_TUG2_010,
	TX_VENA_F41, TX_FEA5_008, TX_LOV2N005, TX_LOV4N000, TX_LOV4N001,
	TX_LOV4N002, TX_LOV4N003, TX_LOV4N004, TX_LOV4N005, TX_LOV4N006,
	TX_LOV4N007, TX_LOV4N008, TX_LOV4N009, TX_LOV4N010, TX_LOV4N011,
	TX_LOV4N013, TX_BRIDU146, TX_GENER004
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets love4TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 7266, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 7277, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 7287, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 7297, 0, 0, 0 },
	{ TX_LOV0_011, 8771, 0, 0, 0 },
	{ TX_LOV0_012, 8898, 0, 0, 0 },
	{ TX_LOV0_013, 9194, 0, 0, 0 },
	{ TX_LOV0_014, 8958, 0, 0, 0 },
	{ TX_LOV0_015, 9062, 0, 0, 0 },
	{ TX_LOV0_016, 8865, 0, 0, 0 },
	{ TX_LOV0_017, 9328, 0, 0, 0 },
	{ TX_LOV0_019, 9248, 0, 0, 0 },
	{ TX_LOV0_020, 8805, 0, 0, 0 },
	{ TX_LOV0_021, 9116, 0, 0, 0 },
	{ TX_LOV0_022, 9006, 0, 0, 0 },
	{ TX_LOV0_025, 10254, 0, 0, 0 },
	{ TX_LOV0_026, 10127, 0, 0, 0 },
	{ TX_LOV0_029, 10007, 0, 0, 0 },
	{ TX_LOV0_033, 10067, 0, 0, 0 },
	{ TX_LOV0_039, 9389, 0, 0, 0 },
	{ TX_LOV0_040, 9516, 0, 0, 0 },
	{ TX_LOV0_041, 9812, 0, 0, 0 },
	{ TX_LOV0_042, 9576, 0, 0, 0 },
	{ TX_LOV0_043, 9680, 0, 0, 0 },
	{ TX_LOV0_045, 9483, 0, 0, 0 },
	{ TX_LOV0_046, 9946, 0, 0, 0 },
	{ TX_LOV0_047, 9866, 0, 0, 0 },
	{ TX_LOV0_048, 9423, 0, 0, 0 },
	{ TX_LOV0_049, 9734, 0, 0, 0 },
	{ TX_LOV0_050, 9624, 0, 0, 0 },
	{ TX_LOV0_101, 10335, 0, 0, 0 },
	{ TX_LOV0_102, 10399, 0, 0, 0 },
	{ TX_LOV0_103, 10469, 0, 0, 0 },
	{ TX_LOV0_104, 10519, 0, 0, 0 },
	{ TX_LOV0_105, 10576, 0, 0, 0 },
	{ TX_LOV0_106, 10688, 0, 0, 0 },
	{ TX_LOV0_107, 10756, 0, 0, 0 },
	{ TX_LOV0_124, 10204, 0, 0, 0 },
	{ TX_LOV4_001, 2899, 0, 0, 0 },
	{ TX_LOV4_002, 1211, 0, 0, 0 },
	{ TX_LOV4_003, 8428, 0, 0, 0 },
	{ TX_LOV4_004, 3355, 0, 0, 0 },
	{ TX_LOV4_005, 1095, 0, 0, 0 },
	{ TX_LOV4_006, 8685, 0, 0, 0 },
	{ TX_LOV4_007, 7977, 0, 0, 0 },
	{ TX_LOV4_009, 3010, 0, 0, 0 },
	{ TX_LOV4_010, 8197, 0, 0, 0 },
	{ TX_LOV4_011, 3416, 0, 0, 0 },
	{ TX_LOV4_012, 3537, 0, 0, 0 },
	{ TX_LOV4_013, 2078, 0, 0, 0 },
	{ TX_LOV4_014, 7908, 0, 0, 0 },
	{ TX_LOV4_015, 1733, 0, 0, 0 },
	{ TX_LOV4_016, 10809, 0, 0, 0 },
	{ TX_LOV4_017, 8130, 0, 0, 0 },
	{ TX_LOV4_018, 1601, 0, 0, 0 },
	{ TX_LOV4_019, 1855, 0, 0, 0 },
	{ TX_LOV4_020, 8489, 0, 0, 0 },
	{ TX_LOV4_021, 8618, 0, 0, 0 },
	{ TX_LOV4_022, 1301, 0, 0, 0 },
	{ TX_LOV4_023, 2545, 0, 0, 0 },
	{ TX_LOV4_024, 1164, 0, 0, 0 },
	{ TX_LOV4_025, 7610, 0, 0, 0 },
	{ TX_LOV4_026, 2772, 0, 0, 0 },
	{ TX_LOV4_027, 8360, 0, 0, 0 },
	{ TX_LOV4_028, 8291, 0, 0, 0 },
	{ TX_LOV4_029, 3295, 0, 0, 0 },
	{ TX_LOV5_015, 4669, 0, 0, 0 },
	//{ TX_LOV5_015, 4490, 0, 0, 0 },
	{ TX_LOV5_019, 4412, 0, 0, 0 },
	{ TX_LOV5_027, 4925, 0, 0, 0 },
	{ TX_LOV5_030, 4829, 0, 0, 0 },
	{ TX_LOV5_038, 4880, 0, 0, 0 },
	{ TX_LOVA_100, 6034, 0, 0, 0 },
	{ TX_LOVA_F01, 5476, 0, 0, 0 },
	{ TX_LOVA_F02, 5548, 0, 0, 0 },
	{ TX_LOVA_F03, 5747, 0, 0, 0 },
	{ TX_LOVA_F04, 5662, 0, 0, 0 },
	{ TX_LOVA_F07, 6111, 0, 0, 0 },
	{ TX_LOVA_F08, 5100, 0, 0, 0 },
	//{ TX_LOVA_F08, 5815, 0, 0, 0 },
	{ TX_LOVA_F10, 5982, 0, 0, 0 },
	{ TX_LOVA_F54, 5256, 0, 0, 0 },
	{ TX_LOVA_F55, 4557, 0, 0, 0 },
	{ TX_FEA5_008, 8051, 0, 0, 0 },
	{ TX_MUD2_040, 6307, 0, 0, 0 },
	{ TX_MUD4_018, 7013, 0, 0, 0 },
	//{ TX_MUD4_018, 6680, 0, 0, 0 },
	//{ TX_MUD4_018, 6516, 0, 0, 0 },
	//{ TX_MUD4_018, 6849, 0, 0, 0 },
	{ TX_TUG2_010, 5912, 0, 0, 0 },
	{ TX_VENA_F41, 7136, 0, 0, 0 },
	//{ TX_VENA_F41, 6803, 0, 0, 0 },
	//{ TX_VENA_F41, 6972, 0, 0, 0 },
	//{ TX_VENA_F41, 6639, 0, 0, 0 },
	{ TX_LOV2N005, 6170, 0, 0, 0 },
	//{ TX_LOV2N005, 6250, 0, 0, 0 },
	//{ TX_LOV2N005, 6396, 0, 0, 0 },
	{ TX_LOV4N000, 8170, 0, 0, 0 },
	{ TX_LOV4N001, 7740, 0, 0, 0 },
	{ TX_LOV4N002, 7554, 0, 0, 0 },
	{ TX_LOV4N003, 7690, 0, 0, 0 },
	{ TX_LOV4N004, 3206, 0, 0, 0 },
	{ TX_LOV4N005, 2437, 0, 0, 0 },
	{ TX_LOV4N006, 7309, 0, 0, 0 },
	{ TX_LOV4N007, 1473, 0, 0, 0 },
	{ TX_LOV4N008, 7843, 0, 0, 0 },
	{ TX_LOV4N009, 7362, 0, 0, 0 },
	{ TX_LOV4N010, 7495, 0, 0, 0 },
	{ TX_LOV4N011, 7441, 0, 0, 0 },
	{ TX_LOV4N013, 2692, 0, 0, 0 },
	{ TX_BRIDU146, 6749, 0, 0, 0 },
	//{ TX_BRIDU146, 7082, 0, 0, 0 },
	//{ TX_BRIDU146, 6918, 0, 0, 0 },
	//{ TX_BRIDU146, 6585, 0, 0, 0 },
	{ TX_GENER004, 5405, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText love4Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::love4Tick1() {
	playVoc("LOV4LOOP");

	if (_awayMission->love.romulansCured) {
		loadActorAnim2(OBJECT_ROMULAN_1,  "s3r5r1b", 0x36, 0xb3, 0);
		loadActorAnim2(OBJECT_ROMULAN_2, "s3r5r2b", 0xb9, 0xbb, 0);
		loadActorAnim2(OBJECT_ROMULAN_3, "s3r5r3b", 0xef, 0xc4, 0);
		loadActorAnim2(OBJECT_ROMULAN_4, "s3r5r4b", 0x12a, 0xaa, 0);
	} else {
		loadActorAnim2(OBJECT_ROMULAN_1,  "s3r5r1a", 0x36, 0xb3, 0);
		loadActorAnim2(OBJECT_ROMULAN_2, "s3r5r2a", 0xb9, 0xbb, 0);
		loadActorAnim2(OBJECT_ROMULAN_3, "s3r5r3a", 0xef, 0xc4, 0);
		loadActorAnim2(OBJECT_ROMULAN_4, "s3r5r4a", 0x12a, 0xaa, 0);
	}

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	loadActorAnim(OBJECT_DOOR, "s3r5d1a", 0x90, 0x99, 0);

	_roomVar.love.cmnXPosToCureSpock = 0x6b;
	_roomVar.love.cmnYPosToCureSpock = 0xb2;
}

void Room::love4Tick10() {
	if (!_awayMission->love.visitedRoomWithRomulans) {
		playMidiMusicTracks(MIDITRACK_1);
		_awayMission->love.visitedRoomWithRomulans = true;
	}
}

void Room::love4WalkToDoor() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewmanC(OBJECT_KIRK, 0x91, 0x9c, &Room::love4DoorOpenedOrReached);
}

// Triggers the door opening
void Room::love4TouchedHotspot0() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnimC(OBJECT_DOOR, "s3r5d1", 0x90, 0x99, &Room::love4DoorOpenedOrReached);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love4DoorOpenedOrReached() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(5, 1);
}

void Room::love4UseKirkOnLadder() {
	walkCrewmanC(OBJECT_KIRK, 0xf6, 0xac, &Room::love4ReachedLadder);
}

void Room::love4ReachedLadder() {
	loadRoomIndex(1, 2);
}

void Room::love4UseStunPhaserOnRomulan() {
	showText(TX_SPEAKER_MCCOY, TX_LOV4_007);
}

void Room::love4LookAnywhere() {
	showDescription(TX_LOV4N009);
}

void Room::love4LookAtLadder() {
	showDescription(TX_LOV4N010);
}

void Room::love4LookAtDoor() {
	showDescription(TX_LOV4N008);
}

void Room::love4LookAtKirk() {
	showDescription(TX_LOV4N002);
}

void Room::love4LookAtMccoy() {
	// BUGFIX: original game plays audio "LOV4/LOV4_025". This is mccoy saying something.
	// It doesn't match up with the actual text, which is the narrator saying that mccoy
	// is thinking it.
	// Not sure if this was an intentional decision, but there is another unused audio
	// file which matches with the text more accurately, so I'm using that instead.
	//showDescription(TX_LOV4N012);	// TODO
}

void Room::love4LookAtSpock() {
	showDescription(TX_LOV4N003);
}

void Room::love4LookAtRedshirt() {
	showDescription(TX_LOV4N001);
}

void Room::love4LookAtRomulan() {
	if (!_awayMission->love.romulansCured)
		showDescription(TX_LOV4N006);
	else
		showDescription(TX_LOV4N011);
}

void Room::love4TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_LOV4_005);
	showText(TX_SPEAKER_MCCOY, TX_LOV4_024);
	showText(TX_SPEAKER_KIRK,  TX_LOV4_002);
}

void Room::love4TalkToMccoy() {
	if (_awayMission->love.romulansCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV4_022);
	else
		showText(TX_SPEAKER_MCCOY, TX_LOV4_010);
}

void Room::love4TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV4_028);
}

void Room::love4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_LOV4_027);
	showText(TX_SPEAKER_KIRK,  TX_LOV4_003);
}

void Room::love4TalkToRomulan() {
	if (_awayMission->love.romulansCured)
		showDescription(TX_LOV4N007); // BUGFIX: original didn't play audio
}

void Room::love4UseMTricorderOnRomulan() {
	if (_awayMission->love.romulansCured)
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV4_018, false);
	else
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV4_015, false);
}

void Room::love4UseMTricorderAnywhere() {
	if (_awayMission->love.romulansCured)
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV4_019, false);
	else if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV4_020, false);
	else
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV4_021, false);
}

void Room::love4UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_LOV4_006, false);
}


// Mccoy walks around to all romulans, giving each the cure
void Room::love4UseCureOnRomulan() {
	if (_awayMission->love.romulansCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV4_013);
	else {
		_awayMission->love.missionScore += 2;
		_awayMission->love.romulansCured = true;
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_MCCOY, 0x10e, 0xb1, &Room::love4MccoyReachedRomulan4);
	}
}

void Room::love4MccoyReachedRomulan4() {
	loadActorAnimC(OBJECT_MCCOY, "museme", -1, -1, &Room::love4MccoyCuredRomulan4);
}

void Room::love4MccoyCuredRomulan4() {
	loadActorAnim2(OBJECT_ROMULAN_4, "s3r5r4", 0x12a, 0xaa);
	walkCrewmanC(OBJECT_MCCOY, 0xeb, 0xc4, &Room::love4MccoyReachedRomulan3);
}

void Room::love4MccoyReachedRomulan3() {
	loadActorAnimC(OBJECT_MCCOY, "musele", -1, -1, &Room::love4MccoyCuredRomulan3);
}

void Room::love4MccoyCuredRomulan3() {
	loadActorAnim2(OBJECT_ROMULAN_3, "s3r5r3", 0xef, 0xc4);
	walkCrewmanC(OBJECT_MCCOY, 0xb9, 0xc3, &Room::love4MccoyReachedRomulan2);
}

void Room::love4MccoyReachedRomulan2() {
	loadActorAnimC(OBJECT_MCCOY, "musemn", -1, -1, &Room::love4MccoyCuredRomulan2);
}

void Room::love4MccoyCuredRomulan2() {
	loadActorAnim2(OBJECT_ROMULAN_2, "s3r5r2", 0xb9, 0xbb);
	walkCrewmanC(OBJECT_MCCOY, 0x36, 0xba, &Room::love4MccoyReachedRomulan1);
}

void Room::love4MccoyReachedRomulan1() {
	loadActorAnimC(OBJECT_MCCOY, "musemn", -1, -1, &Room::love4MccoyCuredRomulan1);
}

void Room::love4MccoyCuredRomulan1() {
	loadActorAnim2(OBJECT_ROMULAN_1, "s3r5r1", 0x36, 0xb3, 0);

	showDescription(TX_LOV4N005);
	if (!_roomVar.love.gaveWaterToRomulans)
		showText(TX_SPEAKER_MCCOY, TX_LOV4_023);

	_awayMission->disableInput = false;
}


void Room::love4UseWaterOnRomulan() {
	// BUGFIX: If the romulans are unconscious, you can't use water on them.
	// In the original, you could use water on them, but there would be no corresponding
	// narration, you'd get no points for it, and you'd lose the water anyway.
	if (!_awayMission->love.romulansCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV4_009);
	else {
		_roomVar.love.gaveWaterToRomulans = true;
		if (_awayMission->love.romulansCured) {
			showDescription(TX_LOV4N013);
			showText(TX_SPEAKER_MCCOY, TX_LOV4_026);
			showText(TX_SPEAKER_KIRK, TX_LOV4_001);
			if (!_awayMission->love.gotPointsForHydratingRomulans) {
				_awayMission->love.gotPointsForHydratingRomulans = true;
				_awayMission->love.missionScore += 2;
			}
		}

		loseItem(OBJECT_IH2O);
	}

	// Note the following unused block of code, an alternative implementation of the
	// function, resembling the version in love5. If they succumbed to the virus from
	// taking too long, McCoy doesn't let you hydrate them, saying it's dangerous.
	// Otherwise, the romulans get up even without receiving the cure. (At least the
	// narration says they do.)
	// These events don't make too much sense, probably cut in the original release, but
	// they did get voice acted anyway.
	if (false) {
		if (_awayMission->love.romulansUnconsciousFromVirus)
			showText(TX_SPEAKER_MCCOY, TX_LOV4_009);
		else {
			showDescription(TX_LOV4N004);
			if (!_awayMission->redshirtDead) {
				showText(TX_SPEAKER_FERRIS, TX_LOV4_029);
				showText(TX_SPEAKER_KIRK,   TX_LOV4_004);
			}
			showText(TX_SPEAKER_MCCOY, TX_LOV4_011);
			loseItem(OBJECT_IH2O);
		}
	}
}

void Room::love4UseMedkitOnRomulan() {
	showText(TX_SPEAKER_MCCOY, TX_LOV4_012);
}

void Room::love4UseCureSampleOnRomulan() {
	showText(TX_SPEAKER_MCCOY, TX_LOV4_014);
}

void Room::love4UseAnythingOnRomulan() {
	showText(TX_SPEAKER_MCCOY, TX_LOV4_017);
}

}
