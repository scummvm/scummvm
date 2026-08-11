/* ScummVM - Graphic Adventure Engine3
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

#define OBJECT_QUETZECOATL 8
#define OBJECT_LEFT_VINE 9
#define OBJECT_SNAKE 10
#define OBJECT_THROWN_ROCK 11
#define OBJECT_ROCK_IN_HOLE 12

#define HOTSPOT_ROCKS  0x20
#define HOTSPOT_HOLE   0x21
#define HOTSPOT_MOSS   0x22
#define HOTSPOT_RIGHT_VINE   0x23
#define HOTSPOT_LIGHT  0x24
#define HOTSPOT_EYES_1 0x25
#define HOTSPOT_EYES_2 0x26
#define HOTSPOT_EYES_3 0x27
#define HOTSPOT_EXIT   0x28

// Positions where crewmen walk to when climbing up or down the vine
#define VINE_TOP_X 0xcc
#define VINE_TOP_Y 0x30
#define VINE_BOT_X 0xc3
#define VINE_BOT_Y 0xc7

namespace StarTrek {

extern const RoomAction feather1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::feather1Tick1 },
	{ {ACTION_TICK, 45, 0, 0, 0}, &Room::feather1Tick45 },
	{ {ACTION_TICK, 85, 0, 0, 0}, &Room::feather1Tick85 },
	{ {ACTION_TICK, 95, 0, 0, 0}, &Room::feather1Tick95 },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0, 0}, &Room::feather1QuetzecoatlDisappeared },
	{ {ACTION_GET, HOTSPOT_RIGHT_VINE, 0, 0, 0}, &Room::feather1GetRightVine },
	{ {ACTION_GET, OBJECT_LEFT_VINE, 0, 0, 0}, &Room::feather1GetLeftVine },
	{ {ACTION_GET, HOTSPOT_ROCKS, 0, 0, 0}, &Room::feather1GetRocks },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::feather1ReachedRocks },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::feather1PickedUpRocks },
	{ {ACTION_GET, OBJECT_SNAKE, 0, 0, 0}, &Room::feather1GetSnake },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0, 0}, &Room::feather1ReachedSnake },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::feather1Timer1Expired },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::feather1Timer2Expired },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0, 0}, &Room::feather1PickedUpSnake },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::feather1Timer0Expired },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::feather1UseCommunicator },
	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_HOLE, 0, 0}, &Room::feather1UseRockOnHole },
	{ {ACTION_FINISHED_WALKING, 9, 0, 0, 0}, &Room::feather1ReachedHole },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0}, &Room::feather1Timer3Expired },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::feather1PutRockInHole },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_LEFT_VINE, 0, 0}, &Room::feather1UseSnakeOnLeftVine },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_SPOCK, 0, 0}, &Room::feather1UseSnakeOnSpock },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_KIRK, 0, 0}, &Room::feather1UseSnakeOnKirk },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_MCCOY, 0, 0}, &Room::feather1UseSnakeOnMccoy },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_REDSHIRT, 0, 0}, &Room::feather1UseSnakeOnRedshirt },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_SNAKE, 0, 0}, &Room::feather1UseRockOnSnake },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SNAKE, 0, 0}, &Room::feather1UseSpockOnSnake },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_SNAKE, 0, 0}, &Room::feather1UseMccoyOnSnake },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_SNAKE, 0, 0}, &Room::feather1UseRedshirtOnSnake },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_HOLE, 0, 0}, &Room::feather1UseSpockOnHole },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_HOLE, 0, 0}, &Room::feather1UseMccoyOnHole },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_HOLE, 0, 0}, &Room::feather1UseRedshirtOnHole },
	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_MOSS, 0, 0}, &Room::feather1UseRockOnMoss },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_SPOCK, 0, 0}, &Room::feather1UseRockOnSpock },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_MCCOY, 0, 0}, &Room::feather1UseRockOnMccoy },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_REDSHIRT, 0, 0}, &Room::feather1UseRockOnRedshirt },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_MOSS, 0, 0}, &Room::feather1UseSpockOnMoss },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_MOSS, 0, 0}, &Room::feather1UseMccoyOnMoss },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_MOSS, 0, 0}, &Room::feather1UseRedshirtOnMoss },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_LEFT_VINE, 0, 0}, &Room::feather1UseRockOnLeftVine },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::feather1ReadyToThrowRock1 },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::feather1ThrewRock1 },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::feather1ReadyToThrowRock2 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::feather1ThrewRock2 },

	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_LEFT_VINE,   0, 0}, &Room::feather1UseSpockOnVine },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1UseSpockOnVine },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_LEFT_VINE,   0, 0}, &Room::feather1UseMccoyOnVine },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1UseMccoyOnVine },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_LEFT_VINE,   0, 0}, &Room::feather1UseRedshirtOnVine },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1UseRedshirtOnVine },
	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_LEFT_VINE,   0, 0}, &Room::feather1UseKirkOnVine },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1UseKirkOnVine },
	{ {ACTION_FINISHED_WALKING,    3, 0,                0, 0}, &Room::feather1ReachedVineToClimbUp },
	{ {ACTION_FINISHED_ANIMATION,  3, 0,                0, 0}, &Room::feather1ClimbedUpVine },
	{ {ACTION_FINISHED_WALKING,   10, 0,                0, 0}, &Room::feather1ReachedVineToClimbDown },
	{ {ACTION_FINISHED_ANIMATION, 10, 0,                0, 0}, &Room::feather1ClimbedDownVine },

	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0, 0}, &Room::feather1UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0, 0}, &Room::feather1UsePhaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1UseSTricorderOnRightVine },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SNAKE, 0, 0}, &Room::feather1UseSTricorderOnSnake },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MOSS, 0, 0}, &Room::feather1UseSTricorderOnMoss },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_HOLE, 0, 0}, &Room::feather1UseSTricorderOnHole },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::feather1UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_ROCKS, 0, 0}, &Room::feather1UseSTricorderOnRocks },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_LEFT_VINE,   0, 0}, &Room::feather1UseMTricorderOnVine },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1UseMTricorderOnVine },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_MOSS, 0, 0}, &Room::feather1UseMTricorderOnMoss },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_HOLE, 0, 0}, &Room::feather1UseMTricorderOnHole },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SNAKE, 0, 0}, &Room::feather1UseMTricorderOnSnake },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0}, &Room::feather1UseMedkit },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::feather1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::feather1TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::feather1TalkToRedshirt },
	{ {ACTION_WALK, HOTSPOT_EXIT, 0, 0, 0}, &Room::feather1WalkToExit },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::feather1LookAnywhere },
	{ {ACTION_LOOK, OBJECT_SNAKE, 0, 0, 0}, &Room::feather1LookAtSnake },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_VINE, 0, 0, 0}, &Room::feather1LookAtRightVine },
	{ {ACTION_LOOK, HOTSPOT_HOLE, 0, 0, 0}, &Room::feather1LookAtHole },
	{ {ACTION_LOOK, HOTSPOT_MOSS, 0, 0, 0}, &Room::feather1LookAtMoss },
	{ {ACTION_LOOK, HOTSPOT_ROCKS, 0, 0, 0}, &Room::feather1LookAtRocks },
	{ {ACTION_LOOK, HOTSPOT_LIGHT, 0, 0, 0}, &Room::feather1LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_EYES_1, 0, 0, 0}, &Room::feather1LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_2, 0, 0, 0}, &Room::feather1LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_3, 0, 0, 0}, &Room::feather1LookAtEyes },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::feather1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::feather1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::feather1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::feather1LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_LEFT_VINE, 0, 0, 0}, &Room::feather1LookAtLeftVine },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum feather1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_QUETZECOATL, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_STRAGEY,
	TX_FEA1_001, TX_FEA1_002, TX_FEA1_003, TX_FEA1_004, TX_FEA1_005,
	TX_FEA1_006, TX_FEA1_007, TX_FEA1_008, TX_FEA1_009, TX_FEA1_010,
	TX_FEA1_011, TX_FEA1_012, TX_FEA1_013, TX_FEA1_014, TX_FEA1_015,
	TX_FEA1_017, TX_FEA1_018, TX_FEA1_019, TX_FEA1_020, TX_FEA1_021,
	TX_FEA1_022, TX_FEA1_023, TX_FEA1_024, TX_FEA1_025, TX_FEA1_026,
	TX_FEA1_027, TX_FEA1_028, TX_FEA1_029, TX_FEA1_030, TX_FEA1_031,
	TX_FEA1_032, TX_FEA1_033, TX_FEA1_034, TX_FEA1_035, TX_FEA1_037,
	TX_FEA1_038, TX_FEA1_039, TX_FEA1_040, TX_FEA1_041, TX_FEA1_042,
	TX_FEA1_043, TX_FEA1_044, TX_FEA1_045, TX_FEA1_046, TX_FEA1_047,
	TX_FEA1_048, TX_FEA1_049, TX_FEA1_050, TX_FEA1_051, TX_FEA1_052,
	TX_FEA1_053, TX_FEA1_054, TX_FEA1_055, TX_FEA1_056, TX_FEA1_057,
	TX_FEA1_A46, TX_FEA1_F25, TX_FEA1N000, TX_FEA1N001, TX_FEA1N002,
	TX_FEA1N003, TX_FEA1N004, TX_FEA1N005, TX_FEA1N006, TX_FEA1N007,
	TX_FEA1N008, TX_FEA1N009, TX_FEA1N010, TX_FEA1N011, TX_FEA1N012,
	TX_FEA1N013, TX_FEA1N014, TX_FEA1N015, TX_FEA1N016, TX_FEA1N017,
	TX_FEA1N018, TX_FEA1N019, TX_FEA1N020, TX_FEA1N021, TX_FEA1N022
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets feather1TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 4412, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 4435, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 4445, 0, 0, 0 },
	{ TX_SPEAKER_QUETZECOATL, 4423, 0, 0, 0 },
	{ TX_SPEAKER_STRAGEY, 4455, 0, 0, 0 },
	{ TX_FEA1_001, 10730, 0, 0, 0 },
	{ TX_FEA1_002, 9907, 0, 0, 0 },
	{ TX_FEA1_003, 10929, 0, 0, 0 },
	{ TX_FEA1_004, 6487, 0, 0, 0 },
	{ TX_FEA1_005, 6143, 0, 0, 0 },
	{ TX_FEA1_006, 7970, 0, 0, 0 },
	{ TX_FEA1_007, 9933, 0, 0, 0 },
	{ TX_FEA1_008, 3761, 0, 0, 0 },
	{ TX_FEA1_009, 4949, 0, 0, 0 },
	{ TX_FEA1_010, 6430, 0, 0, 0 },
	{ TX_FEA1_011, 9554, 0, 0, 0 },
	{ TX_FEA1_012, 9519, 0, 0, 0 },
	{ TX_FEA1_013, 5288, 0, 0, 0 },
	{ TX_FEA1_014, 9462, 0, 0, 0 },
	{ TX_FEA1_015, 6775, 0, 0, 0 },
	{ TX_FEA1_017, 5616, 0, 0, 0 },
	{ TX_FEA1_018, 10805, 0, 0, 0 },
	{ TX_FEA1_019, 9608, 0, 0, 0 },
	{ TX_FEA1_020, 10217, 0, 0, 0 },
	{ TX_FEA1_021, 5876, 0, 0, 0 },
	{ TX_FEA1_022, 9983, 0, 0, 0 },
	{ TX_FEA1_023, 4693, 0, 0, 0 },
	{ TX_FEA1_023, 4693, 0, 0, 0 },
	{ TX_FEA1_024, 10984, 0, 0, 0 },
	{ TX_FEA1_025, 6379, 0, 0, 0 },
	{ TX_FEA1_026, 7037, 0, 0, 0 },
	{ TX_FEA1_027, 6312, 0, 0, 0 },
	{ TX_FEA1_028, 8659, 0, 0, 0 },
	{ TX_FEA1_029, 8901, 0, 0, 0 },
	{ TX_FEA1_030, 9168, 0, 0, 0 },
	{ TX_FEA1_031, 9092, 0, 0, 0 },
	{ TX_FEA1_032, 4842, 0, 0, 0 },
	{ TX_FEA1_033, 6704, 0, 0, 0 },
	{ TX_FEA1_034, 5992, 0, 0, 0 },
	{ TX_FEA1_035, 6055, 0, 0, 0 },
	{ TX_FEA1_037, 5146, 0, 0, 0 },
	{ TX_FEA1_038, 9268, 0, 0, 0 },
	{ TX_FEA1_039, 8771, 0, 0, 0 },
	{ TX_FEA1_040, 8095, 0, 0, 0 },
	{ TX_FEA1_041, 10597, 0, 0, 0 },
	{ TX_FEA1_042, 5503, 0, 0, 0 },
	{ TX_FEA1_043, 5770, 0, 0, 0 },
	{ TX_FEA1_044, 9705, 0, 0, 0 },
	{ TX_FEA1_045, 4988, 0, 0, 0 },
	{ TX_FEA1_046, 6218, 0, 0, 0 },
	{ TX_FEA1_047, 6618, 0, 0, 0 },
	{ TX_FEA1_048, 8173, 0, 0, 0 },
	{ TX_FEA1_049, 6859, 0, 0, 0 },
	{ TX_FEA1_049, 6859, 0, 0, 0 },
	{ TX_FEA1_050, 7132, 0, 0, 0 },
	{ TX_FEA1_051, 6579, 0, 0, 0 },
	{ TX_FEA1_052, 5409, 0, 0, 0 },
	{ TX_FEA1_053, 5948, 0, 0, 0 },
	{ TX_FEA1_054, 5696, 0, 0, 0 },
	{ TX_FEA1_055, 9851, 0, 0, 0 },
	{ TX_FEA1_056, 10123, 0, 0, 0 },
	{ TX_FEA1_057, 4468, 0, 0, 0 },
	{ TX_FEA1_A46, 5235, 0, 0, 0 },
	{ TX_FEA1_F25, 6966, 0, 0, 0 },
	{ TX_FEA1N000, 4782, 0, 0, 0 },
	{ TX_FEA1N001, 10345, 0, 0, 0 },
	{ TX_FEA1N002, 10538, 0, 0, 0 },
	{ TX_FEA1N003, 10482, 0, 0, 0 },
	{ TX_FEA1N004, 10429, 0, 0, 0 },
	{ TX_FEA1N005, 5056, 0, 0, 0 },
	{ TX_FEA1N006, 6661, 0, 0, 0 },
	{ TX_FEA1N007, 8511, 0, 0, 0 },
	{ TX_FEA1N008, 8454, 0, 0, 0 },
	{ TX_FEA1N009, 8409, 0, 0, 0 },
	{ TX_FEA1N010, 5100, 0, 0, 0 },
	{ TX_FEA1N011, 7824, 0, 0, 0 },
	{ TX_FEA1N012, 7295, 0, 0, 0 },
	{ TX_FEA1N013, 7905, 0, 0, 0 },
	{ TX_FEA1N014, 7650, 0, 0, 0 },
	{ TX_FEA1N015, 7409, 0, 0, 0 },
	{ TX_FEA1N016, 7758, 0, 0, 0 },
	{ TX_FEA1N017, 7542, 0, 0, 0 },
	{ TX_FEA1N018, 8586, 0, 0, 0 },
	{ TX_FEA1N019, 8370, 0, 0, 0 },
	{ TX_FEA1N020, 7467, 0, 0, 0 },
	{ TX_FEA1N021, 8262, 0, 0, 0 },
	{ TX_FEA1N022, 8317, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText feather1Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::feather1Tick1() {
	playVoc("FEA1LOOP");
	playMidiMusicTracks(MIDITRACK_27);

	if (_awayMission->feather.vineState == 0)
		_awayMission->disableInput = true;

	if (!_awayMission->feather.gotSnake)
		loadActorAnim(OBJECT_SNAKE, "s5r1so", 0x9c, 0xc1);
	if (_awayMission->feather.vineState == 0)
		loadActorAnim(OBJECT_LEFT_VINE, "s5r1v0", 0xa0, 0x23);
	else {
		_roomVar.feather.crewEscaped[OBJECT_KIRK] = true;
		_roomVar.feather.crewEscaped[OBJECT_SPOCK] = true;
		_roomVar.feather.crewEscaped[OBJECT_MCCOY] = true;
		_roomVar.feather.crewEscaped[OBJECT_REDSHIRT] = true;
		loadActorAnim(OBJECT_LEFT_VINE, "s5r1v4", 0xa0, 0x23);
	}
}

void Room::feather1Tick45() {
	if (_awayMission->feather.vineState == 0) {
		playVoc("LD3MAGAP");
		loadActorAnim(OBJECT_QUETZECOATL, "s5r1qa", 0xb4, 0x32);
	}
}

void Room::feather1Tick85() {
	if (_awayMission->feather.vineState == 0)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA1_057);
}

void Room::feather1Tick95() {
	if (_awayMission->feather.vineState == 0) {
		playVoc("LD3MAGDI");
		loadActorAnimC(OBJECT_QUETZECOATL, "s5r1qd", -1, -1, &Room::feather1QuetzecoatlDisappeared);
	}
}

void Room::feather1QuetzecoatlDisappeared() {
	_awayMission->disableInput = false;
}

void Room::feather1GetRightVine() {
	showDescription(TX_FEA1N008);
}

void Room::feather1GetLeftVine() {
	if (_awayMission->feather.vineState == 0)
		showDescription(TX_FEA1N009);
	else
		showDescription(TX_FEA1N008);
}

void Room::feather1GetRocks() {
	if (_roomVar.feather.crewEscaped[OBJECT_KIRK])
		showDescription(TX_FEA1N021);
	else if (_awayMission->feather.gotRock)
		showDescription(TX_FEA1N019);
	else {
		walkCrewmanC(OBJECT_KIRK, 0x90, 0xb6, &Room::feather1ReachedRocks);
		_awayMission->disableInput = true;
	}
}

void Room::feather1ReachedRocks() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather1PickedUpRocks);
}

void Room::feather1PickedUpRocks() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IROCK);
	showDescription(TX_FEA1N022);
	_awayMission->feather.gotRock = true;
}

void Room::feather1GetSnake() {
	// BUG: Infinite score mechanism. Just keep trying and failing to get the snake.
	_awayMission->feather.missionScore++;

	if (!_roomVar.feather.crewEscaped[OBJECT_KIRK]) {
		walkCrewmanC(OBJECT_KIRK, 0x90, 0xbe, &Room::feather1ReachedSnake);
		_awayMission->disableInput = true;
	} else
		showDescription(TX_FEA1N021);
}

void Room::feather1ReachedSnake() {
	// NOTE: There are two pieces of unused text that could fit here.
	// TX_FEA1N007: "The snake darts back into its hole"
	// TX_FEA1N018: "With nowhere for the snake to go, you capture it"

	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather1PickedUpSnake);
	if (_awayMission->feather.holeBlocked)
		_awayMission->timers[1] = 20;
	else
		_awayMission->timers[2] = 6;
}

void Room::feather1Timer1Expired() {
	giveItem(OBJECT_ISNAKE);
	loadActorStandAnim(OBJECT_SNAKE);
	_awayMission->feather.gotSnake = true;
}

void Room::feather1Timer2Expired() { // Snake retreats into hole
	loadActorAnim2(OBJECT_SNAKE, "s5r1si");
	_awayMission->timers[0] = 80;
	_roomVar.feather.snakeInHole = true;
}

void Room::feather1PickedUpSnake() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
}

void Room::feather1Timer0Expired() { // Snake comes back out of hole
	loadActorAnim2(OBJECT_SNAKE, "s5r1so");
	_roomVar.feather.snakeInHole = false;
}

void Room::feather1UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_023);
}

void Room::feather1UseRockOnHole() {
	if (_roomVar.feather.crewEscaped[OBJECT_KIRK])
		showDescription(TX_FEA1N021);
	else if (_awayMission->feather.holeBlocked)
		showDescription(TX_FEA1N005);
	else {
		_awayMission->feather.missionScore++;
		walkCrewmanC(OBJECT_KIRK, 0xa3, 0xb6, &Room::feather1ReachedHole);
		_awayMission->disableInput = true;
		if (_roomVar.feather.snakeInHole)
			_awayMission->timers[0] = 12;
	}
}

void Room::feather1ReachedHole() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather1PutRockInHole);
	_awayMission->timers[3] = 18;
}

void Room::feather1Timer3Expired() {
	loadActorAnim(OBJECT_ROCK_IN_HOLE, "s5r1rk", 0xad, 0xba);
	_awayMission->feather.holeBlocked = true;
}

void Room::feather1PutRockInHole() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
}

void Room::feather1UseSnakeOnLeftVine() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_037);
}

void Room::feather1UseSnakeOnSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_041);
	showText(TX_SPEAKER_KIRK,  TX_FEA1_001);
	showText(TX_SPEAKER_MCCOY, TX_FEA1_018);
}

void Room::feather1UseSnakeOnKirk() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_026);
}

void Room::feather1UseSnakeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_F25);
}

void Room::feather1UseSnakeOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_050);
}

void Room::feather1UseRockOnSnake() {
	if (!_awayMission->feather.gotSnake) {
		loadActorAnim2(OBJECT_SNAKE, "s5r1si");
		_awayMission->timers[0] = 80;
		_roomVar.feather.snakeInHole = true;
		showText(TX_SPEAKER_SPOCK, TX_FEA1_034); // BUGFIX: speaker is Spock, not Stragey
	}
}

void Room::feather1UseSpockOnSnake() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_035);
}

void Room::feather1UseMccoyOnSnake() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_005);
}

void Room::feather1UseRedshirtOnSnake() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_046);
}

void Room::feather1UseSpockOnHole() {
	if (_roomVar.feather.snakeInHole)
		showText(TX_SPEAKER_SPOCK, TX_FEA1_025);
	else
		showText(TX_SPEAKER_SPOCK, TX_FEA1_027);
}

void Room::feather1UseMccoyOnHole() {
	if (_roomVar.feather.snakeInHole)
		showText(TX_SPEAKER_MCCOY, TX_FEA1_004);
	else
		showText(TX_SPEAKER_MCCOY, TX_FEA1_010);
}

void Room::feather1UseRedshirtOnHole() {
	if (_roomVar.feather.snakeInHole)
		showText(TX_SPEAKER_STRAGEY, TX_FEA1_047);
	else
		showText(TX_SPEAKER_STRAGEY, TX_FEA1_051);
}

void Room::feather1UseRockOnMoss() {
	showDescription(TX_FEA1N006);
}

void Room::feather1UseRockOnSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_032);
}

void Room::feather1UseRockOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_009);
}

void Room::feather1UseRockOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_045);
}

void Room::feather1UseSpockOnMoss() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_033);
}

void Room::feather1UseMccoyOnMoss() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_015);
}

void Room::feather1UseRedshirtOnMoss() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_049);
}

void Room::feather1UseRockOnLeftVine() {
	if (_awayMission->feather.vineState == 0) {
		_awayMission->feather.vineState++;
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x7a, 0xb6, &Room::feather1ReadyToThrowRock1);
	} else if (_awayMission->feather.vineState == 1) {
		_awayMission->feather.vineState++;
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0xa2, 0xb9, &Room::feather1ReadyToThrowRock2);
	}
}

void Room::feather1ReadyToThrowRock1() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kt", -1, -1, &Room::feather1ThrewRock1);
	loadActorAnim2(OBJECT_LEFT_VINE, "s5r1v2");
	loadActorAnim(OBJECT_THROWN_ROCK, "s5r1ru", 0x81, 0x6d);
	playVoc("THROWROC");
}

void Room::feather1ThrewRock1() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);

	showText(TX_SPEAKER_SPOCK, TX_FEA1_042);
	showText(TX_SPEAKER_MCCOY, TX_FEA1_017);
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_054);
}

void Room::feather1ReadyToThrowRock2() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kt", -1, -1, &Room::feather1ThrewRock2);
	loadActorAnim2(OBJECT_LEFT_VINE, "s5r1v3");
	loadActorAnim(OBJECT_THROWN_ROCK, "s5r1ru", 0xa3, 0x70);
	playVoc("THROWROC");
}

void Room::feather1ThrewRock2() {
	_awayMission->feather.missionScore += 6;
	loadActorStandAnim(OBJECT_KIRK);
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_SPOCK, TX_FEA1_043);
	showText(TX_SPEAKER_MCCOY, TX_FEA1_021);
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_053);
}


void Room::feather1UseSpockOnVine() {
	if (_awayMission->feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_SPOCK;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_SPOCK, TX_FEA1_A46);
}

void Room::feather1UseMccoyOnVine() {
	if (_awayMission->feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_MCCOY;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_MCCOY, TX_FEA1_013);
}

void Room::feather1UseRedshirtOnVine() {
	if (_awayMission->feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_REDSHIRT;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_STRAGEY, TX_FEA1_052);
}

void Room::feather1UseKirkOnVine() {
	if (_awayMission->feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_KIRK;
		feather1CrewmanClimbVine();
	} else
		showDescription(TX_FEA1N010);
}

// This was refactored, due to the similarity of the code for each crewman. Originally, the
// following 5 vine-climbing functions were repeated for each crewman.
void Room::feather1CrewmanClimbVine() {
	_awayMission->disableInput = true;
	if (_roomVar.feather.crewEscaped[_roomVar.feather.crewmanClimbingVine])
		walkCrewmanC(_roomVar.feather.crewmanClimbingVine, VINE_TOP_X, VINE_TOP_Y,
		             &Room::feather1ReachedVineToClimbDown);
	else
		walkCrewmanC(_roomVar.feather.crewmanClimbingVine, VINE_BOT_X, VINE_BOT_Y,
		             &Room::feather1ReachedVineToClimbUp);
}

void Room::feather1ReachedVineToClimbUp() {
	const char *crew = "ksmr";
	Common::String anim = "s5r1_c";
	anim.setChar(crew[_roomVar.feather.crewmanClimbingVine], 4);
	loadActorAnimC(_roomVar.feather.crewmanClimbingVine, anim, -1, -1, &Room::feather1ClimbedUpVine);
}

void Room::feather1ClimbedUpVine() {
	_awayMission->disableInput = false;
	_roomVar.feather.crewEscaped[_roomVar.feather.crewmanClimbingVine] = true;

	Common::String anim = getCrewmanAnimFilename(_roomVar.feather.crewmanClimbingVine, "stndw");
	loadActorAnim2(_roomVar.feather.crewmanClimbingVine, anim, VINE_TOP_X, VINE_TOP_Y);

	switch (_roomVar.feather.crewmanClimbingVine) {
	case OBJECT_SPOCK:
		walkCrewman(OBJECT_SPOCK, 0xaf, 0x30);
		break;
	case OBJECT_MCCOY:
		walkCrewman(OBJECT_MCCOY, 0xbe, 0x30);
		break;
	case OBJECT_REDSHIRT:
		walkCrewman(OBJECT_REDSHIRT, 0xca, 0x30);
		break;
	case OBJECT_KIRK:
		walkCrewman(OBJECT_KIRK, 0xa0, 0x30);
		break;
	default:
		break;
	}
}

void Room::feather1ReachedVineToClimbDown() {
	const char *crew = "ksmr";
	Common::String anim = "s5r1_d";
	anim.setChar(crew[_roomVar.feather.crewmanClimbingVine], 4);
	loadActorAnimC(_roomVar.feather.crewmanClimbingVine, anim, VINE_BOT_X, VINE_BOT_Y, &Room::feather1ClimbedDownVine);
}

void Room::feather1ClimbedDownVine() {
	_awayMission->disableInput = false;
	_roomVar.feather.crewEscaped[_roomVar.feather.crewmanClimbingVine] = false;

	Common::String anim = getCrewmanAnimFilename(_roomVar.feather.crewmanClimbingVine, "stndw");
	loadActorAnim2(_roomVar.feather.crewmanClimbingVine, anim, VINE_BOT_X, VINE_BOT_Y);

	switch (_roomVar.feather.crewmanClimbingVine) {
	case OBJECT_SPOCK:
		walkCrewman(OBJECT_SPOCK, 0x68, 0xbe);
		break;
	case OBJECT_MCCOY:
		walkCrewman(OBJECT_MCCOY, 0x55, 0xb4);
		break;
	case OBJECT_REDSHIRT:
		walkCrewman(OBJECT_REDSHIRT, 0xdc, 0xc6);
		break;
	case OBJECT_KIRK:
		walkCrewman(OBJECT_KIRK, 0x87, 0xba);
		break;
	default:
		break;
	}
}

void Room::feather1UsePhaser() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_028);
}

void Room::feather1UseSTricorderOnRightVine() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_FEA1_039);
}

void Room::feather1UseSTricorderOnSnake() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA1_029);
}

void Room::feather1UseSTricorderOnMoss() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_FEA1_038);
}

void Room::feather1UseSTricorderOnHole() {
	if (_roomVar.feather.snakeInHole)
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_FEA1_030);
	else
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_FEA1_031);
}

void Room::feather1UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA1_024);
}

void Room::feather1UseSTricorderOnRocks() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_FEA1_003);
}

void Room::feather1UseMTricorderOnVine() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_FEA1_007);
}

void Room::feather1UseMTricorderOnMoss() {
	mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_FEA1_014);
}

void Room::feather1UseMTricorderOnHole() {
	if (_roomVar.feather.snakeInHole)
		mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_FEA1_011);
	else
		mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_FEA1_012);
}

void Room::feather1UseMTricorderOnSnake() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_FEA1_019);
	if (!_roomVar.feather.scannedSnake) {
		showText(TX_SPEAKER_SPOCK,   TX_FEA1_044);
		showText(TX_SPEAKER_STRAGEY, TX_FEA1_055);
		showText(TX_SPEAKER_KIRK,    TX_FEA1_002);
		_roomVar.feather.scannedSnake = true;
	}
}

void Room::feather1UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_008);
}

void Room::feather1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA1_006);
}

void Room::feather1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_040);
}

void Room::feather1TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_048);
}

void Room::feather1WalkToExit() {
	walkCrewman(OBJECT_KIRK, 0x0a, 0x30);
}

void Room::feather1LookAnywhere() {
	showDescription(TX_FEA1N020);
}

void Room::feather1LookAtSnake() {
	showDescription(TX_FEA1N017);
}

void Room::feather1LookAtRightVine() {
	showDescription(TX_FEA1N015);
}

void Room::feather1LookAtHole() {
	showDescription(TX_FEA1N014);
}

void Room::feather1LookAtMoss() {
	showDescription(TX_FEA1N016);
}

void Room::feather1LookAtRocks() {
	showDescription(TX_FEA1N011);
}

void Room::feather1LookAtLight() {
	showDescription(TX_FEA1N013);
}

void Room::feather1LookAtEyes() {
	showDescription(TX_FEA1N000);
}

void Room::feather1LookAtKirk() {
	showDescription(TX_FEA1N001);
}

void Room::feather1LookAtSpock() {
	showDescription(TX_FEA1N004);
}

void Room::feather1LookAtMccoy() {
	showDescription(TX_FEA1N003);
}

void Room::feather1LookAtRedshirt() {
	showDescription(TX_FEA1N002);
}

void Room::feather1LookAtLeftVine() {
	if (_awayMission->feather.vineState == 0)
		showDescription(TX_FEA1N012);
	else
		showDescription(TX_FEA1N015);
}

}
