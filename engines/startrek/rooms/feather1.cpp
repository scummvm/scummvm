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
	{ Action(ACTION_TICK, 1, 0, 0), &Room::feather1Tick1 },
	{ Action(ACTION_TICK, 45, 0, 0), &Room::feather1Tick45 },
	{ Action(ACTION_TICK, 85, 0, 0), &Room::feather1Tick85 },
	{ Action(ACTION_TICK, 95, 0, 0), &Room::feather1Tick95 },
	{ Action(ACTION_FINISHED_ANIMATION, 18, 0, 0), &Room::feather1QuetzecoatlDisappeared },
	{ Action(ACTION_GET, HOTSPOT_RIGHT_VINE, 0, 0), &Room::feather1GetRightVine },
	{ Action(ACTION_GET, OBJECT_LEFT_VINE, 0, 0), &Room::feather1GetLeftVine },
	{ Action(ACTION_GET, HOTSPOT_ROCKS, 0, 0), &Room::feather1GetRocks },
	{ Action(ACTION_FINISHED_WALKING, 6, 0, 0), &Room::feather1ReachedRocks },
	{ Action(ACTION_FINISHED_ANIMATION, 6, 0, 0), &Room::feather1PickedUpRocks },
	{ Action(ACTION_GET, OBJECT_SNAKE, 0, 0), &Room::feather1GetSnake },
	{ Action(ACTION_FINISHED_WALKING, 8, 0, 0), &Room::feather1ReachedSnake },
	{ Action(ACTION_TIMER_EXPIRED, 1, 0, 0), &Room::feather1Timer1Expired },
	{ Action(ACTION_TIMER_EXPIRED, 2, 0, 0), &Room::feather1Timer2Expired },
	{ Action(ACTION_FINISHED_ANIMATION, 8, 0, 0), &Room::feather1PickedUpSnake },
	{ Action(ACTION_TIMER_EXPIRED, 0, 0, 0), &Room::feather1Timer0Expired },
	{ Action(ACTION_USE, OBJECT_ICOMM, -1, 0), &Room::feather1UseCommunicator },
	{ Action(ACTION_USE, OBJECT_IROCK, HOTSPOT_HOLE, 0), &Room::feather1UseRockOnHole },
	{ Action(ACTION_FINISHED_WALKING, 9, 0, 0), &Room::feather1ReachedHole },
	{ Action(ACTION_TIMER_EXPIRED, 3, 0, 0), &Room::feather1Timer3Expired },
	{ Action(ACTION_FINISHED_ANIMATION, 9, 0, 0), &Room::feather1PutRockInHole },
	{ Action(ACTION_USE, OBJECT_ISNAKE, OBJECT_LEFT_VINE, 0), &Room::feather1UseSnakeOnLeftVine },
	{ Action(ACTION_USE, OBJECT_ISNAKE, OBJECT_SPOCK, 0), &Room::feather1UseSnakeOnSpock },
	{ Action(ACTION_USE, OBJECT_ISNAKE, OBJECT_KIRK, 0), &Room::feather1UseSnakeOnKirk },
	{ Action(ACTION_USE, OBJECT_ISNAKE, OBJECT_MCCOY, 0), &Room::feather1UseSnakeOnMccoy },
	{ Action(ACTION_USE, OBJECT_ISNAKE, OBJECT_REDSHIRT, 0), &Room::feather1UseSnakeOnRedshirt },
	{ Action(ACTION_USE, OBJECT_IROCK, OBJECT_SNAKE, 0), &Room::feather1UseRockOnSnake },
	{ Action(ACTION_USE, OBJECT_SPOCK, OBJECT_SNAKE, 0), &Room::feather1UseSpockOnSnake },
	{ Action(ACTION_USE, OBJECT_MCCOY, OBJECT_SNAKE, 0), &Room::feather1UseMccoyOnSnake },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, OBJECT_SNAKE, 0), &Room::feather1UseRedshirtOnSnake },
	{ Action(ACTION_USE, OBJECT_SPOCK, HOTSPOT_HOLE, 0), &Room::feather1UseSpockOnHole },
	{ Action(ACTION_USE, OBJECT_MCCOY, HOTSPOT_HOLE, 0), &Room::feather1UseMccoyOnHole },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_HOLE, 0), &Room::feather1UseRedshirtOnHole },
	{ Action(ACTION_USE, OBJECT_IROCK, HOTSPOT_MOSS, 0), &Room::feather1UseRockOnMoss },
	{ Action(ACTION_USE, OBJECT_IROCK, OBJECT_SPOCK, 0), &Room::feather1UseRockOnSpock },
	{ Action(ACTION_USE, OBJECT_IROCK, OBJECT_MCCOY, 0), &Room::feather1UseRockOnMccoy },
	{ Action(ACTION_USE, OBJECT_IROCK, OBJECT_REDSHIRT, 0), &Room::feather1UseRockOnRedshirt },
	{ Action(ACTION_USE, OBJECT_SPOCK, HOTSPOT_MOSS, 0), &Room::feather1UseSpockOnMoss },
	{ Action(ACTION_USE, OBJECT_MCCOY, HOTSPOT_MOSS, 0), &Room::feather1UseMccoyOnMoss },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_MOSS, 0), &Room::feather1UseRedshirtOnMoss },
	{ Action(ACTION_USE, OBJECT_IROCK, OBJECT_LEFT_VINE, 0), &Room::feather1UseRockOnLeftVine },
	{ Action(ACTION_FINISHED_WALKING, 1, 0, 0), &Room::feather1ReadyToThrowRock1 },
	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::feather1ThrewRock1 },
	{ Action(ACTION_FINISHED_WALKING, 2, 0, 0), &Room::feather1ReadyToThrowRock2 },
	{ Action(ACTION_FINISHED_ANIMATION, 2, 0, 0), &Room::feather1ThrewRock2 },

	{ Action(ACTION_USE, OBJECT_SPOCK,    OBJECT_LEFT_VINE,   0), &Room::feather1UseSpockOnVine },
	{ Action(ACTION_USE, OBJECT_SPOCK,    HOTSPOT_RIGHT_VINE, 0), &Room::feather1UseSpockOnVine },
	{ Action(ACTION_USE, OBJECT_MCCOY,    OBJECT_LEFT_VINE,   0), &Room::feather1UseMccoyOnVine },
	{ Action(ACTION_USE, OBJECT_MCCOY,    HOTSPOT_RIGHT_VINE, 0), &Room::feather1UseMccoyOnVine },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, OBJECT_LEFT_VINE,   0), &Room::feather1UseRedshirtOnVine },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_RIGHT_VINE, 0), &Room::feather1UseRedshirtOnVine },
	{ Action(ACTION_USE, OBJECT_KIRK,     OBJECT_LEFT_VINE,   0), &Room::feather1UseKirkOnVine },
	{ Action(ACTION_USE, OBJECT_KIRK,     HOTSPOT_RIGHT_VINE, 0), &Room::feather1UseKirkOnVine },
	{ Action(ACTION_FINISHED_WALKING,    3, 0,                0), &Room::feather1ReachedVineToClimbUp },
	{ Action(ACTION_FINISHED_ANIMATION,  3, 0,                0), &Room::feather1ClimbedUpVine },
	{ Action(ACTION_FINISHED_WALKING,   10, 0,                0), &Room::feather1ReachedVineToClimbDown },
	{ Action(ACTION_FINISHED_ANIMATION, 10, 0,                0), &Room::feather1ClimbedDownVine },

	{ Action(ACTION_USE, OBJECT_IPHASERS, -1, 0), &Room::feather1UsePhaser },
	{ Action(ACTION_USE, OBJECT_IPHASERK, -1, 0), &Room::feather1UsePhaser },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_VINE, 0), &Room::feather1UseSTricorderOnRightVine },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, OBJECT_SNAKE, 0), &Room::feather1UseSTricorderOnSnake },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MOSS, 0), &Room::feather1UseSTricorderOnMoss },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_HOLE, 0), &Room::feather1UseSTricorderOnHole },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, -1, 0), &Room::feather1UseSTricorderAnywhere },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_ROCKS, 0), &Room::feather1UseSTricorderOnRocks },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, OBJECT_LEFT_VINE,   0), &Room::feather1UseMTricorderOnVine },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_RIGHT_VINE, 0), &Room::feather1UseMTricorderOnVine },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_MOSS, 0), &Room::feather1UseMTricorderOnMoss },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_HOLE, 0), &Room::feather1UseMTricorderOnHole },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, OBJECT_SNAKE, 0), &Room::feather1UseMTricorderOnSnake },
	{ Action(ACTION_USE, OBJECT_IMEDKIT, -1, 0), &Room::feather1UseMedkit },
	{ Action(ACTION_TALK, OBJECT_MCCOY, 0, 0), &Room::feather1TalkToMccoy },
	{ Action(ACTION_TALK, OBJECT_SPOCK, 0, 0), &Room::feather1TalkToSpock },
	{ Action(ACTION_TALK, OBJECT_REDSHIRT, 0, 0), &Room::feather1TalkToRedshirt },
	{ Action(ACTION_WALK, HOTSPOT_EXIT, 0, 0), &Room::feather1WalkToExit },
	{ Action(ACTION_LOOK, -1, 0, 0), &Room::feather1LookAnywhere },
	{ Action(ACTION_LOOK, OBJECT_SNAKE, 0, 0), &Room::feather1LookAtSnake },
	{ Action(ACTION_LOOK, HOTSPOT_RIGHT_VINE, 0, 0), &Room::feather1LookAtRightVine },
	{ Action(ACTION_LOOK, HOTSPOT_HOLE, 0, 0), &Room::feather1LookAtHole },
	{ Action(ACTION_LOOK, HOTSPOT_MOSS, 0, 0), &Room::feather1LookAtMoss },
	{ Action(ACTION_LOOK, HOTSPOT_ROCKS, 0, 0), &Room::feather1LookAtRocks },
	{ Action(ACTION_LOOK, HOTSPOT_LIGHT, 0, 0), &Room::feather1LookAtLight },
	{ Action(ACTION_LOOK, HOTSPOT_EYES_1, 0, 0), &Room::feather1LookAtEyes },
	{ Action(ACTION_LOOK, HOTSPOT_EYES_2, 0, 0), &Room::feather1LookAtEyes },
	{ Action(ACTION_LOOK, HOTSPOT_EYES_3, 0, 0), &Room::feather1LookAtEyes },
	{ Action(ACTION_LOOK, OBJECT_KIRK, 0, 0), &Room::feather1LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_SPOCK, 0, 0), &Room::feather1LookAtSpock },
	{ Action(ACTION_LOOK, OBJECT_MCCOY, 0, 0), &Room::feather1LookAtMccoy },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::feather1LookAtRedshirt },
	{ Action(ACTION_LOOK, OBJECT_LEFT_VINE, 0, 0), &Room::feather1LookAtLeftVine },
};

extern const int feather1NumActions = sizeof(feather1ActionList) / sizeof(RoomAction);


void Room::feather1Tick1() {
	playVoc("FEA1LOOP");
	playMidiMusicTracks(27);

	if (_vm->_awayMission.feather.vineState == 0)
		_vm->_awayMission.disableInput = true;

	if (!_vm->_awayMission.feather.gotSnake)
		loadActorAnim(OBJECT_SNAKE, "s5r1so", 0x9c, 0xc1);
	if (_vm->_awayMission.feather.vineState == 0)
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
	if (_vm->_awayMission.feather.vineState == 0) {
		playVoc("LD3MAGAP");
		loadActorAnim(OBJECT_QUETZECOATL, "s5r1qa", 0xb4, 0x32);
	}
}

void Room::feather1Tick85() {
	if (_vm->_awayMission.feather.vineState == 0)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA1_057);
}

void Room::feather1Tick95() {
	if (_vm->_awayMission.feather.vineState == 0) {
		playVoc("LD3MAGDI");
		loadActorAnimC(OBJECT_QUETZECOATL, "s5r1qd", -1, -1, &Room::feather1QuetzecoatlDisappeared);
	}
}

void Room::feather1QuetzecoatlDisappeared() {
	_vm->_awayMission.disableInput = false;
}

void Room::feather1GetRightVine() {
	showText(TX_FEA1N008);
}

void Room::feather1GetLeftVine() {
	if (_vm->_awayMission.feather.vineState == 0)
		showText(TX_FEA1N009);
	else
		showText(TX_FEA1N008);
}

void Room::feather1GetRocks() {
	if (_roomVar.feather.crewEscaped[OBJECT_KIRK])
		showText(TX_FEA1N021);
	else if (_vm->_awayMission.feather.gotRock)
		showText(TX_FEA1N019);
	else {
		walkCrewmanC(OBJECT_KIRK, 0x90, 0xb6, &Room::feather1ReachedRocks);
		_vm->_awayMission.disableInput = true;
	}
}

void Room::feather1ReachedRocks() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather1PickedUpRocks);
}

void Room::feather1PickedUpRocks() {
	_vm->_awayMission.disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IROCK);
	showText(TX_FEA1N022);
	_vm->_awayMission.feather.gotRock = true;
}

void Room::feather1GetSnake() {
	// BUG: Infinite score mechanism. Just keep trying and failing to get the snake.
	_vm->_awayMission.feather.missionScore++;

	if (!_roomVar.feather.crewEscaped[OBJECT_KIRK]) {
		walkCrewmanC(OBJECT_KIRK, 0x90, 0xbe, &Room::feather1ReachedSnake);
		_vm->_awayMission.disableInput = true;
	} else
		showText(TX_FEA1N021);
}

void Room::feather1ReachedSnake() {
	// NOTE: There are two pieces of unused text that could fit here.
	// TX_FEA1N007: "The snake darts back into its hole"
	// TX_FEA1N018: "With nowhere for the snake to go, you capture it"

	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather1PickedUpSnake);
	if (_vm->_awayMission.feather.holeBlocked)
		_vm->_awayMission.timers[1] = 20;
	else
		_vm->_awayMission.timers[2] = 6;
}

void Room::feather1Timer1Expired() {
	giveItem(OBJECT_ISNAKE);
	loadActorStandAnim(OBJECT_SNAKE);
	_vm->_awayMission.feather.gotSnake = true;
}

void Room::feather1Timer2Expired() { // Snake retreats into hole
	loadActorAnim2(OBJECT_SNAKE, "s5r1si");
	_vm->_awayMission.timers[0] = 80;
	_roomVar.feather.snakeInHole = true;
}

void Room::feather1PickedUpSnake() {
	_vm->_awayMission.disableInput = false;
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
		showText(TX_FEA1N021);
	else if (_vm->_awayMission.feather.holeBlocked)
		showText(TX_FEA1N005);
	else {
		_vm->_awayMission.feather.missionScore++;
		walkCrewmanC(OBJECT_KIRK, 0xa3, 0xb6, &Room::feather1ReachedHole);
		_vm->_awayMission.disableInput = true;
		if (_roomVar.feather.snakeInHole)
			_vm->_awayMission.timers[0] = 12;
	}
}

void Room::feather1ReachedHole() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather1PutRockInHole);
	_vm->_awayMission.timers[3] = 18;
}

void Room::feather1Timer3Expired() {
	loadActorAnim(OBJECT_ROCK_IN_HOLE, "s5r1rk", 0xad, 0xba);
	_vm->_awayMission.feather.holeBlocked = true;
}

void Room::feather1PutRockInHole() {
	_vm->_awayMission.disableInput = false;
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
	if (!_vm->_awayMission.feather.gotSnake) {
		loadActorAnim2(OBJECT_SNAKE, "s5r1si");
		_vm->_awayMission.timers[0] = 80;
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
	showText(TX_FEA1N006);
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
	if (_vm->_awayMission.feather.vineState == 0) {
		_vm->_awayMission.feather.vineState++;
		_vm->_awayMission.disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x7a, 0xb6, &Room::feather1ReadyToThrowRock1);
	} else if (_vm->_awayMission.feather.vineState == 1) {
		_vm->_awayMission.feather.vineState++;
		_vm->_awayMission.disableInput = true;
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
	_vm->_awayMission.disableInput = false;
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
	_vm->_awayMission.feather.missionScore += 6;
	loadActorStandAnim(OBJECT_KIRK);
	_vm->_awayMission.disableInput = false;

	showText(TX_SPEAKER_SPOCK, TX_FEA1_043);
	showText(TX_SPEAKER_MCCOY, TX_FEA1_021);
	showText(TX_SPEAKER_STRAGEY, TX_FEA1_053);
}


void Room::feather1UseSpockOnVine() {
	if (_vm->_awayMission.feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_SPOCK;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_SPOCK, TX_FEA1_A46);
}

void Room::feather1UseMccoyOnVine() {
	if (_vm->_awayMission.feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_MCCOY;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_MCCOY, TX_FEA1_013);
}

void Room::feather1UseRedshirtOnVine() {
	if (_vm->_awayMission.feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_REDSHIRT;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_STRAGEY, TX_FEA1_052);
}

void Room::feather1UseKirkOnVine() {
	if (_vm->_awayMission.feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_KIRK;
		feather1CrewmanClimbVine();
	} else
		showText(TX_FEA1N010);
}

// This was refactored, due to the similarity of the code for each crewman. Originally, the
// following 5 vine-climbing functions were repeated for each crewman.
void Room::feather1CrewmanClimbVine() {
	_vm->_awayMission.disableInput = true;
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
	_vm->_awayMission.disableInput = false;
	_roomVar.feather.crewEscaped[_roomVar.feather.crewmanClimbingVine] = true;

	Common::String anim = _vm->getCrewmanAnimFilename(_roomVar.feather.crewmanClimbingVine, "stndw");
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
	}
}

void Room::feather1ReachedVineToClimbDown() {
	const char *crew = "ksmr";
	Common::String anim = "s5r1_d";
	anim.setChar(crew[_roomVar.feather.crewmanClimbingVine], 4);
	loadActorAnimC(_roomVar.feather.crewmanClimbingVine, anim, VINE_BOT_X, VINE_BOT_Y, &Room::feather1ClimbedDownVine);
}

void Room::feather1ClimbedDownVine() {
	_vm->_awayMission.disableInput = false;
	_roomVar.feather.crewEscaped[_roomVar.feather.crewmanClimbingVine] = false;

	Common::String anim = _vm->getCrewmanAnimFilename(_roomVar.feather.crewmanClimbingVine, "stndw");
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
	}
}

void Room::feather1UsePhaser() {
	showText(TX_SPEAKER_SPOCK, TX_FEA1_028);
}

void Room::feather1UseSTricorderOnRightVine() {
	spockScan(DIR_N, TX_FEA1_039);
}

void Room::feather1UseSTricorderOnSnake() {
	spockScan(DIR_S, TX_FEA1_029);
}

void Room::feather1UseSTricorderOnMoss() {
	spockScan(DIR_E, TX_FEA1_038);
}

void Room::feather1UseSTricorderOnHole() {
	if (_roomVar.feather.snakeInHole)
		spockScan(DIR_E, TX_FEA1_030);
	else
		spockScan(DIR_E, TX_FEA1_031);
}

void Room::feather1UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_FEA1_024);
}

void Room::feather1UseSTricorderOnRocks() {
	spockScan(DIR_E, TX_FEA1_003);
}

void Room::feather1UseMTricorderOnVine() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_E, TX_FEA1_007);
}

void Room::feather1UseMTricorderOnMoss() {
	mccoyScan(DIR_E, TX_FEA1_014);
}

void Room::feather1UseMTricorderOnHole() {
	if (_roomVar.feather.snakeInHole)
		mccoyScan(DIR_E, TX_FEA1_011);
	else
		mccoyScan(DIR_E, TX_FEA1_012);
}

void Room::feather1UseMTricorderOnSnake() {
	mccoyScan(DIR_S, TX_FEA1_019);
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
	showText(TX_FEA1N020);
}

void Room::feather1LookAtSnake() {
	showText(TX_FEA1N017);
}

void Room::feather1LookAtRightVine() {
	showText(TX_FEA1N015);
}

void Room::feather1LookAtHole() {
	showText(TX_FEA1N014);
}

void Room::feather1LookAtMoss() {
	showText(TX_FEA1N016);
}

void Room::feather1LookAtRocks() {
	showText(TX_FEA1N011);
}

void Room::feather1LookAtLight() {
	showText(TX_FEA1N013);
}

void Room::feather1LookAtEyes() {
	showText(TX_FEA1N000);
}

void Room::feather1LookAtKirk() {
	showText(TX_FEA1N001);
}

void Room::feather1LookAtSpock() {
	showText(TX_FEA1N004);
}

void Room::feather1LookAtMccoy() {
	showText(TX_FEA1N003);
}

void Room::feather1LookAtRedshirt() {
	showText(TX_FEA1N002);
}

void Room::feather1LookAtLeftVine() {
	if (_vm->_awayMission.feather.vineState == 0)
		showText(TX_FEA1N012);
	else
		showText(TX_FEA1N015);
}

}
