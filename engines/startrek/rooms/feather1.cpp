/* ScummVM - Graphic Adventure Engine3
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
	{ {ACTION_TICK, 1, 0, 0}, &Room::feather1Tick1 },
	{ {ACTION_TICK, 45, 0, 0}, &Room::feather1Tick45 },
	{ {ACTION_TICK, 85, 0, 0}, &Room::feather1Tick85 },
	{ {ACTION_TICK, 95, 0, 0}, &Room::feather1Tick95 },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0}, &Room::feather1QuetzecoatlDisappeared },
	{ {ACTION_GET, HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1GetRightVine },
	{ {ACTION_GET, OBJECT_LEFT_VINE, 0, 0}, &Room::feather1GetLeftVine },
	{ {ACTION_GET, HOTSPOT_ROCKS, 0, 0}, &Room::feather1GetRocks },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0}, &Room::feather1ReachedRocks },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0}, &Room::feather1PickedUpRocks },
	{ {ACTION_GET, OBJECT_SNAKE, 0, 0}, &Room::feather1GetSnake },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0}, &Room::feather1ReachedSnake },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::feather1Timer1Expired },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0}, &Room::feather1Timer2Expired },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0}, &Room::feather1PickedUpSnake },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::feather1Timer0Expired },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::feather1UseCommunicator },
	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_HOLE, 0}, &Room::feather1UseRockOnHole },
	{ {ACTION_FINISHED_WALKING, 9, 0, 0}, &Room::feather1ReachedHole },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0}, &Room::feather1Timer3Expired },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::feather1PutRockInHole },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_LEFT_VINE, 0}, &Room::feather1UseSnakeOnLeftVine },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_SPOCK, 0}, &Room::feather1UseSnakeOnSpock },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_KIRK, 0}, &Room::feather1UseSnakeOnKirk },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_MCCOY, 0}, &Room::feather1UseSnakeOnMccoy },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_REDSHIRT, 0}, &Room::feather1UseSnakeOnRedshirt },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_SNAKE, 0}, &Room::feather1UseRockOnSnake },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SNAKE, 0}, &Room::feather1UseSpockOnSnake },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_SNAKE, 0}, &Room::feather1UseMccoyOnSnake },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_SNAKE, 0}, &Room::feather1UseRedshirtOnSnake },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_HOLE, 0}, &Room::feather1UseSpockOnHole },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_HOLE, 0}, &Room::feather1UseMccoyOnHole },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_HOLE, 0}, &Room::feather1UseRedshirtOnHole },
	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_MOSS, 0}, &Room::feather1UseRockOnMoss },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_SPOCK, 0}, &Room::feather1UseRockOnSpock },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_MCCOY, 0}, &Room::feather1UseRockOnMccoy },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_REDSHIRT, 0}, &Room::feather1UseRockOnRedshirt },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_MOSS, 0}, &Room::feather1UseSpockOnMoss },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_MOSS, 0}, &Room::feather1UseMccoyOnMoss },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_MOSS, 0}, &Room::feather1UseRedshirtOnMoss },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_LEFT_VINE, 0}, &Room::feather1UseRockOnLeftVine },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::feather1ReadyToThrowRock1 },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::feather1ThrewRock1 },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0}, &Room::feather1ReadyToThrowRock2 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::feather1ThrewRock2 },

	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_LEFT_VINE,   0}, &Room::feather1UseSpockOnVine },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_RIGHT_VINE, 0}, &Room::feather1UseSpockOnVine },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_LEFT_VINE,   0}, &Room::feather1UseMccoyOnVine },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_RIGHT_VINE, 0}, &Room::feather1UseMccoyOnVine },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_LEFT_VINE,   0}, &Room::feather1UseRedshirtOnVine },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_RIGHT_VINE, 0}, &Room::feather1UseRedshirtOnVine },
	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_LEFT_VINE,   0}, &Room::feather1UseKirkOnVine },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_RIGHT_VINE, 0}, &Room::feather1UseKirkOnVine },
	{ {ACTION_FINISHED_WALKING,    3, 0,                0}, &Room::feather1ReachedVineToClimbUp },
	{ {ACTION_FINISHED_ANIMATION,  3, 0,                0}, &Room::feather1ClimbedUpVine },
	{ {ACTION_FINISHED_WALKING,   10, 0,                0}, &Room::feather1ReachedVineToClimbDown },
	{ {ACTION_FINISHED_ANIMATION, 10, 0,                0}, &Room::feather1ClimbedDownVine },

	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0}, &Room::feather1UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0}, &Room::feather1UsePhaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_VINE, 0}, &Room::feather1UseSTricorderOnRightVine },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SNAKE, 0}, &Room::feather1UseSTricorderOnSnake },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MOSS, 0}, &Room::feather1UseSTricorderOnMoss },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_HOLE, 0}, &Room::feather1UseSTricorderOnHole },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::feather1UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_ROCKS, 0}, &Room::feather1UseSTricorderOnRocks },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_LEFT_VINE,   0}, &Room::feather1UseMTricorderOnVine },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_RIGHT_VINE, 0}, &Room::feather1UseMTricorderOnVine },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_MOSS, 0}, &Room::feather1UseMTricorderOnMoss },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_HOLE, 0}, &Room::feather1UseMTricorderOnHole },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SNAKE, 0}, &Room::feather1UseMTricorderOnSnake },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0}, &Room::feather1UseMedkit },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::feather1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::feather1TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::feather1TalkToRedshirt },
	{ {ACTION_WALK, HOTSPOT_EXIT, 0, 0}, &Room::feather1WalkToExit },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::feather1LookAnywhere },
	{ {ACTION_LOOK, OBJECT_SNAKE, 0, 0}, &Room::feather1LookAtSnake },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_VINE, 0, 0}, &Room::feather1LookAtRightVine },
	{ {ACTION_LOOK, HOTSPOT_HOLE, 0, 0}, &Room::feather1LookAtHole },
	{ {ACTION_LOOK, HOTSPOT_MOSS, 0, 0}, &Room::feather1LookAtMoss },
	{ {ACTION_LOOK, HOTSPOT_ROCKS, 0, 0}, &Room::feather1LookAtRocks },
	{ {ACTION_LOOK, HOTSPOT_LIGHT, 0, 0}, &Room::feather1LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_EYES_1, 0, 0}, &Room::feather1LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_2, 0, 0}, &Room::feather1LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_3, 0, 0}, &Room::feather1LookAtEyes },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0}, &Room::feather1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::feather1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::feather1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::feather1LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_LEFT_VINE, 0, 0}, &Room::feather1LookAtLeftVine },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather1Tick1() {
	playVoc("FEA1LOOP");
	playMidiMusicTracks(27);

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
		showText(TX_SPEAKER_QUETZECOATL, 57, true);
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
	showDescription(8, true);
}

void Room::feather1GetLeftVine() {
	if (_awayMission->feather.vineState == 0)
		showDescription(9, true);
	else
		showDescription(8, true);
}

void Room::feather1GetRocks() {
	if (_roomVar.feather.crewEscaped[OBJECT_KIRK])
		showDescription(21, true);
	else if (_awayMission->feather.gotRock)
		showDescription(19, true);
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
	showDescription(22, true);
	_awayMission->feather.gotRock = true;
}

void Room::feather1GetSnake() {
	// BUG: Infinite score mechanism. Just keep trying and failing to get the snake.
	_awayMission->feather.missionScore++;

	if (!_roomVar.feather.crewEscaped[OBJECT_KIRK]) {
		walkCrewmanC(OBJECT_KIRK, 0x90, 0xbe, &Room::feather1ReachedSnake);
		_awayMission->disableInput = true;
	} else
		showDescription(21, true);
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
	showText(TX_SPEAKER_SPOCK, 23, true);
}

void Room::feather1UseRockOnHole() {
	if (_roomVar.feather.crewEscaped[OBJECT_KIRK])
		showDescription(21, true);
	else if (_awayMission->feather.holeBlocked)
		showDescription(5, true);
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
	showText(TX_SPEAKER_SPOCK, 37, true);
}

void Room::feather1UseSnakeOnSpock() {
	showText(TX_SPEAKER_SPOCK, 41, true);
	showText(TX_SPEAKER_KIRK,   1, true);
	showText(TX_SPEAKER_MCCOY, 18, true);
}

void Room::feather1UseSnakeOnKirk() {
	showText(TX_SPEAKER_SPOCK, 26, true);
}

void Room::feather1UseSnakeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 25 + FOLLOWUP_MESSAGE_OFFSET, true);
}

void Room::feather1UseSnakeOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 50, true);
}

void Room::feather1UseRockOnSnake() {
	if (!_awayMission->feather.gotSnake) {
		loadActorAnim2(OBJECT_SNAKE, "s5r1si");
		_awayMission->timers[0] = 80;
		_roomVar.feather.snakeInHole = true;
		showText(TX_SPEAKER_SPOCK, 34, true); // BUGFIX: speaker is Spock, not Stragey
	}
}

void Room::feather1UseSpockOnSnake() {
	showText(TX_SPEAKER_SPOCK, 35, true);
}

void Room::feather1UseMccoyOnSnake() {
	showText(TX_SPEAKER_MCCOY, 5, true);
}

void Room::feather1UseRedshirtOnSnake() {
	showText(TX_SPEAKER_STRAGEY, 46, true);
}

void Room::feather1UseSpockOnHole() {
	if (_roomVar.feather.snakeInHole)
		showText(TX_SPEAKER_SPOCK, 25, true);
	else
		showText(TX_SPEAKER_SPOCK, 27, true);
}

void Room::feather1UseMccoyOnHole() {
	if (_roomVar.feather.snakeInHole)
		showText(TX_SPEAKER_MCCOY, 4, true);
	else
		showText(TX_SPEAKER_MCCOY, 10, true);
}

void Room::feather1UseRedshirtOnHole() {
	if (_roomVar.feather.snakeInHole)
		showText(TX_SPEAKER_STRAGEY, 47, true);
	else
		showText(TX_SPEAKER_STRAGEY, 51, true);
}

void Room::feather1UseRockOnMoss() {
	showDescription(6, true);
}

void Room::feather1UseRockOnSpock() {
	showText(TX_SPEAKER_SPOCK, 32, true);
}

void Room::feather1UseRockOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::feather1UseRockOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 45, true);
}

void Room::feather1UseSpockOnMoss() {
	showText(TX_SPEAKER_SPOCK, 33, true);
}

void Room::feather1UseMccoyOnMoss() {
	showText(TX_SPEAKER_MCCOY, 15, true);
}

void Room::feather1UseRedshirtOnMoss() {
	showText(TX_SPEAKER_MCCOY, 49, true);
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

	showText(TX_SPEAKER_SPOCK, 42, true);
	showText(TX_SPEAKER_MCCOY, 17, true);
	showText(TX_SPEAKER_STRAGEY, 54, true);
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

	showText(TX_SPEAKER_SPOCK, 43, true);
	showText(TX_SPEAKER_MCCOY, 21, true);
	showText(TX_SPEAKER_STRAGEY, 53, true);
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
		showText(TX_SPEAKER_MCCOY, 13, true);
}

void Room::feather1UseRedshirtOnVine() {
	if (_awayMission->feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_REDSHIRT;
		feather1CrewmanClimbVine();
	} else
		showText(TX_SPEAKER_STRAGEY, 52, true);
}

void Room::feather1UseKirkOnVine() {
	if (_awayMission->feather.vineState == 2) {
		_roomVar.feather.crewmanClimbingVine = OBJECT_KIRK;
		feather1CrewmanClimbVine();
	} else
		showDescription(10, true);
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
	showText(TX_SPEAKER_SPOCK, 28, true);
}

void Room::feather1UseSTricorderOnRightVine() {
	spockScan(DIR_N, 39, false, true);
}

void Room::feather1UseSTricorderOnSnake() {
	spockScan(DIR_S, 29, false, true);
}

void Room::feather1UseSTricorderOnMoss() {
	spockScan(DIR_E, 38, false, true);
}

void Room::feather1UseSTricorderOnHole() {
	if (_roomVar.feather.snakeInHole)
		spockScan(DIR_E, 30, false, true);
	else
		spockScan(DIR_E, 31, false, true);
}

void Room::feather1UseSTricorderAnywhere() {
	spockScan(DIR_S, 24, false, true);
}

void Room::feather1UseSTricorderOnRocks() {
	spockScan(DIR_E, 3, false, true);
}

void Room::feather1UseMTricorderOnVine() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_E, 7, false, true);
}

void Room::feather1UseMTricorderOnMoss() {
	mccoyScan(DIR_E, 14, false, true);
}

void Room::feather1UseMTricorderOnHole() {
	if (_roomVar.feather.snakeInHole)
		mccoyScan(DIR_E, 11, false, true);
	else
		mccoyScan(DIR_E, 12, false, true);
}

void Room::feather1UseMTricorderOnSnake() {
	mccoyScan(DIR_S, 19, false, true);
	if (!_roomVar.feather.scannedSnake) {
		showText(TX_SPEAKER_SPOCK,   44, true);
		showText(TX_SPEAKER_STRAGEY, 55, true);
		showText(TX_SPEAKER_KIRK,    2, true);
		_roomVar.feather.scannedSnake = true;
	}
}

void Room::feather1UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 8, true);
}

void Room::feather1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 6, true);
}

void Room::feather1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 40, true);
}

void Room::feather1TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 48, true);
}

void Room::feather1WalkToExit() {
	walkCrewman(OBJECT_KIRK, 0x0a, 0x30);
}

void Room::feather1LookAnywhere() {
	showDescription(20, true);
}

void Room::feather1LookAtSnake() {
	showDescription(17, true);
}

void Room::feather1LookAtRightVine() {
	showDescription(15, true);
}

void Room::feather1LookAtHole() {
	showDescription(14, true);
}

void Room::feather1LookAtMoss() {
	showDescription(16, true);
}

void Room::feather1LookAtRocks() {
	showDescription(11, true);
}

void Room::feather1LookAtLight() {
	showDescription(13, true);
}

void Room::feather1LookAtEyes() {
	showDescription(0, true);
}

void Room::feather1LookAtKirk() {
	showDescription(1, true);
}

void Room::feather1LookAtSpock() {
	showDescription(4, true);
}

void Room::feather1LookAtMccoy() {
	showDescription(3, true);
}

void Room::feather1LookAtRedshirt() {
	showDescription(2, true);
}

void Room::feather1LookAtLeftVine() {
	if (_awayMission->feather.vineState == 0)
		showDescription(12, true);
	else
		showDescription(15, true);
}

}
