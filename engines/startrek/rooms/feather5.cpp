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

#define OBJECT_MONSTER 8
#define OBJECT_THROWN_FERN 10

// The log in the middle of this room has odd "hitboxes". The top half is hotspot 0x20,
// while the bottom half is hotspot 0x22. You need to use the redshirt on the top half to
// cross over. Not sure why this is.

#define HOTSPOT_LEFT_EXIT 0x20
#define HOTSPOT_FERN 0x21
#define HOTSPOT_LOG 0x22
#define HOTSPOT_WATER 0x23
#define HOTSPOT_INSECTS_1 0x24
#define HOTSPOT_INSECTS_2 0x25
#define HOTSPOT_CAVE 0x26

namespace StarTrek {

extern const RoomAction feather5ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0},                            &Room::feather5Tick1 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0},                 &Room::feather5TouchedHotspot0 },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_FERN, 0},       &Room::feather5UseSpockOnFern },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_FERN, 0},       &Room::feather5UseMccoyOnFern },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_FERN, 0},    &Room::feather5UseRedshirtOnFern },

	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_LEFT_EXIT, 0}, &Room::feather5UseRedshirtOnLeftExit },
	{ {ACTION_DONE_WALK, 8, 0, 0},                         &Room::feather5RedshirtReachedLog },
	{ {ACTION_DONE_ANIM, 8, 0, 0},                         &Room::feather5RedshirtDeathFinished },

	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_LEFT_EXIT, 0},    &Room::feather5UseSpockOnLeftExit },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_LEFT_EXIT, 0},    &Room::feather5UseMccoyOnLeftExit },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_KIRK, 0},         &Room::feather5UseSnakeOnKirk },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_SPOCK, 0},        &Room::feather5UseSnakeOnSpock },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_MCCOY, 0},        &Room::feather5UseSnakeOnMccoy },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_REDSHIRT, 0},     &Room::feather5UseSnakeOnRedshirt },
	{ {ACTION_USE, OBJECT_ISNAKE, HOTSPOT_WATER, 0},       &Room::feather5UseSnakeOnWater },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_KIRK, 0},         &Room::feather5UseKnifeOnKirk },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_SPOCK, 0},        &Room::feather5UseKnifeOnSpock },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_MCCOY, 0},        &Room::feather5UseKnifeOnMccoy },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_REDSHIRT, 0},     &Room::feather5UseKnifeOnRedshirt },
	{ {ACTION_USE, OBJECT_IKNIFE, HOTSPOT_WATER, 0},       &Room::feather5UseKnifeOnWater },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_MONSTER, 0},      &Room::feather5UseKnifeOnMonster },
	{ {ACTION_GET, HOTSPOT_FERN, 0, 0},                    &Room::feather5GetFern },

	{ {ACTION_USE, OBJECT_IKNIFE, HOTSPOT_FERN, 0},        &Room::feather5UseKnifeOnFern },
	{ {ACTION_DONE_WALK, 1, 0, 0},                         &Room::feather5ReachedFern },
	{ {ACTION_DONE_ANIM, 1, 0, 0},                         &Room::feather5PickedUpFern },

	{ {ACTION_USE, OBJECT_IFERN, OBJECT_MONSTER, 0},       &Room::feather5UseFernOnMonster },
	{ {ACTION_USE, OBJECT_IFERN, HOTSPOT_WATER, 0},        &Room::feather5UseFernOnMonster },
	{ {ACTION_DONE_WALK, 2, 0, 0},                         &Room::feather5ReachedPositionToUseFern },
	{ {ACTION_DONE_ANIM, 2, 0, 0},                         &Room::feather5DoneThrowingFern },
	{ {ACTION_DONE_ANIM, 3, 0, 0},                         &Room::feather5FernFellIntoWater },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0},                     &Room::feather5Timer1Expired },

	{ {ACTION_USE, OBJECT_IROCK, OBJECT_SPOCK, 0},         &Room::feather5UseRockOnSpock },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_MCCOY, 0},         &Room::feather5UseRockOnMccoy },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_REDSHIRT, 0},      &Room::feather5UseRockOnRedshirt },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_THROWN_FERN, 0},   &Room::feather5UseCrewmanOnThrownFern },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_THROWN_FERN, 0},   &Room::feather5UseCrewmanOnThrownFern },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_THROWN_FERN, 0}, &Room::feather5UseCrewmanOnThrownFern },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_MONSTER, 0},       &Room::feather5UseSpockOnMonster },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_MONSTER, 0},       &Room::feather5UseMccoyOnMonster },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_MONSTER, 0},    &Room::feather5UseRedshirtOnMonster },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0},              &Room::feather5UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0},              &Room::feather5UsePhaser },
	{ {ACTION_LOOK, 0xff, 0, 0},                           &Room::feather5LookAnywhere },
	{ {ACTION_LOOK, HOTSPOT_LOG, 0, 0},                    &Room::feather5LookAtLog },
	{ {ACTION_LOOK, HOTSPOT_WATER, 0, 0},                  &Room::feather5LookAtWater },
	{ {ACTION_LOOK, HOTSPOT_FERN, 0, 0},                   &Room::feather5LookAtFern },
	{ {ACTION_LOOK, HOTSPOT_INSECTS_1, 0, 0},              &Room::feather5LookAtInsects },
	{ {ACTION_LOOK, HOTSPOT_INSECTS_2, 0, 0},              &Room::feather5LookAtInsects },
	{ {ACTION_LOOK, HOTSPOT_CAVE, 0, 0},                   &Room::feather5LookAtCave },
	{ {ACTION_LOOK, OBJECT_MONSTER, 0, 0},                 &Room::feather5LookAtMonster },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0},                   &Room::feather5LookAtSpock },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0},                    &Room::feather5LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0},                   &Room::feather5LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0},                &Room::feather5LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0},                   &Room::feather5TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0},                   &Room::feather5TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0},                &Room::feather5TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_WATER, 0},     &Room::feather5UseMTricorderOnWater },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MONSTER, 0},    &Room::feather5UseMTricorderOnMonster },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_MONSTER, 0},    &Room::feather5UseSTricorderOnMonster },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0},              &Room::feather5UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WATER, 0},     &Room::feather5UseSTricorderOnWater },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_FERN, 0},      &Room::feather5UseSTricorderOnFern },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},               &Room::feather5UseMedkitAnywhere },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather5Tick1() {
	playVoc("FEA5LOOP");
	playMidiMusicTracks(29);

	if (!_awayMission->feather.waterMonsterRetreated)
		loadActorAnim(OBJECT_MONSTER, "s5r5os", 0x4e, 0xa4);
	else
		loadMapFile("feath5b");
}

void Room::feather5TouchedHotspot0() { // Approached the log
	if (!_awayMission->feather.waterMonsterRetreated) {
		showText(TX_SPEAKER_SPOCK, 5, true);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_STRAGEY, 33, true);
	}
}

void Room::feather5UseSpockOnFern() {
	showText(TX_SPEAKER_SPOCK, 4, true);
}

void Room::feather5UseMccoyOnFern() {
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::feather5UseRedshirtOnFern() {
	showText(TX_SPEAKER_STRAGEY, 35, true);
}

void Room::feather5UseRedshirtOnLeftExit() {
	if (!_awayMission->feather.waterMonsterRetreated) {
		showText(TX_SPEAKER_STRAGEY, 34, true);
		_awayMission->disableInput = true;
		loadMapFile("feath5b");
		walkCrewmanC(OBJECT_REDSHIRT, 0x8f, 0x64, &Room::feather5RedshirtReachedLog);
	}
}

void Room::feather5RedshirtReachedLog() {
	playMidiMusicTracks(2);
	playVoc("TENTICL2");
	loadActorAnim(OBJECT_MONSTER, "s5r5oh", 0x4e, 0xa4);
	loadActorAnimC(OBJECT_REDSHIRT, "s5r5ol", 0x8f, 0x64, &Room::feather5RedshirtDeathFinished);
	_awayMission->feather.waterMonsterRetreated = true;
	_awayMission->redshirtDead = true;
}

void Room::feather5RedshirtDeathFinished() {
	loadActorStandAnim(OBJECT_REDSHIRT);
	_awayMission->disableInput = false;
}

void Room::feather5UseSpockOnLeftExit() {
	if (!_awayMission->feather.waterMonsterRetreated)
		showText(TX_SPEAKER_SPOCK, 2, true);
}

void Room::feather5UseMccoyOnLeftExit() {
	if (!_awayMission->feather.waterMonsterRetreated)
		showText(TX_SPEAKER_MCCOY, 15, true);
}

void Room::feather5UseSnakeOnKirk() {
	showText(TX_SPEAKER_KIRK, 10, true);
}

void Room::feather5UseSnakeOnSpock() {
	showText(TX_SPEAKER_SPOCK, 32, true);
}

void Room::feather5UseSnakeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 20, true);
}

void Room::feather5UseSnakeOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 40, true);
}

void Room::feather5UseSnakeOnWater() {
	showDescription(11, true);
	loseItem(OBJECT_ISNAKE);
}

void Room::feather5UseKnifeOnKirk() {
	// ENHANCEMENT: Originally showed TX_DEM0_020 here ("Jim, are you out of your mind?")
	// However, there is an unused audio file for text that otherwise only appears in the
	// mac version. (DOS floppy version has yet another variation for this text.)
	// We'll use that unused audio file here...
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::feather5UseKnifeOnSpock() {
	showText(TX_SPEAKER_SPOCK, 29, true);
}

void Room::feather5UseKnifeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 8, true);
}

void Room::feather5UseKnifeOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 39, true);
}

void Room::feather5UseKnifeOnWater() {
	showText(TX_SPEAKER_MCCOY, 18, true);
}

void Room::feather5UseKnifeOnMonster() {
	showText(TX_SPEAKER_SPOCK, 6, true);
}

void Room::feather5GetFern() {
	showDescription(8, true);
}

void Room::feather5UseKnifeOnFern() {
	if (_awayMission->feather.gotFern)
		showDescription(15, true);
	else {
		walkCrewmanC(OBJECT_KIRK, 0x106, 0x8a, &Room::feather5ReachedFern);
		_awayMission->disableInput = true;
	}
}

void Room::feather5ReachedFern() {
	loadActorAnimC(OBJECT_KIRK, "s5r5kf", -1, -1, &Room::feather5PickedUpFern);
}

void Room::feather5PickedUpFern() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IFERN);
	_awayMission->feather.gotFern = true;
}


void Room::feather5UseFernOnMonster() {
	// BUG: Infinite score mechanism. Just keep throwing ferns into the water.
	_awayMission->feather.missionScore += 2;
	walkCrewmanC(OBJECT_KIRK, 0x106, 0x8a, &Room::feather5ReachedPositionToUseFern);
	_awayMission->disableInput = true;
}

void Room::feather5ReachedPositionToUseFern() {
	loadActorAnimC(OBJECT_KIRK, "s5r5kt", -1, -1, &Room::feather5DoneThrowingFern);
	loadActorAnimC(OBJECT_THROWN_FERN, "s5r5pt", 0xe6, 0x52, &Room::feather5FernFellIntoWater);
}

void Room::feather5DoneThrowingFern() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	loseItem(OBJECT_IFERN);
	_awayMission->feather.gotFern = false;
}

void Room::feather5FernFellIntoWater() {
	if (!_awayMission->feather.waterMonsterRetreated) {
		_awayMission->feather.waterMonsterRetreated = true;
		_awayMission->timers[1] = 20;
		loadActorStandAnim(OBJECT_MONSTER);
		loadMapFile("feath5b");
	}
}

void Room::feather5Timer1Expired() { // Spock explains that the creature retreated
	showText(TX_SPEAKER_SPOCK, 25, true);
}


void Room::feather5UseRockOnSpock() {
	showText(TX_SPEAKER_SPOCK, 27, true);
}

void Room::feather5UseRockOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 19, true);
}

void Room::feather5UseRockOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 38, true);
}

void Room::feather5UseCrewmanOnThrownFern() {
	// This might be unused? The object in question (OBJECT_THROWN_FERN) shouldn't be
	// visible while player control is active.
	showDescription(3, true);
}

void Room::feather5UseSpockOnMonster() {
	showText(TX_SPEAKER_SPOCK, 3, true);
}

void Room::feather5UseMccoyOnMonster() {
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::feather5UseRedshirtOnMonster() {
	showText(TX_SPEAKER_STRAGEY, 37, true);
}

void Room::feather5UsePhaser() {
	showText(TX_SPEAKER_SPOCK, 28, true);
}

void Room::feather5LookAnywhere() {
	showDescription(12, true);
}

void Room::feather5LookAtLog() {
	showDescription(7, true);
}

void Room::feather5LookAtWater() {
	showDescription(9, true);
}

void Room::feather5LookAtFern() {
	showDescription(14, true);
}

void Room::feather5LookAtInsects() {
	showDescription(5, true);
}

void Room::feather5LookAtCave() {
	showDescription(13, true);
}

void Room::feather5LookAtMonster() {
	showDescription(0, true);
}

void Room::feather5LookAtSpock() {
	showDescription(6, true);
}

void Room::feather5LookAtKirk() {
	if (!_awayMission->feather.waterMonsterRetreated)
		showDescription(1, true);
	else // ENHANCEMENT: Fall back to default behaviour instead of doing nothing
		_awayMission->rdfStillDoDefaultAction = true;
}

void Room::feather5LookAtMccoy() {
	if (!_awayMission->feather.waterMonsterRetreated)
		showDescription(2, true);
	else // ENHANCEMENT: Fall back to default behaviour instead of doing nothing
		_awayMission->rdfStillDoDefaultAction = true;
}

void Room::feather5LookAtRedshirt() {
	if (!_awayMission->feather.waterMonsterRetreated)
		showDescription(4, true);
	else // ENHANCEMENT: Fall back to default behaviour instead of doing nothing
		_awayMission->rdfStillDoDefaultAction = true;
}

void Room::feather5TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 7, true);
}

void Room::feather5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 22, true);
}

void Room::feather5TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 36, true);
}

void Room::feather5UseMTricorderOnWater() {
	if (_awayMission->feather.waterMonsterRetreated)
		mccoyScan(DIR_W, 11, false, true);
	else
		mccoyScan(DIR_W, 14, false, true);
}

void Room::feather5UseMTricorderOnMonster() {
	mccoyScan(DIR_W, 13, false, true);
}

void Room::feather5UseSTricorderOnMonster() {
	spockScan(DIR_W, 24, false, true);
}

void Room::feather5UseSTricorderAnywhere() {
	spockScan(DIR_W, 26, false, true);
}

void Room::feather5UseSTricorderOnWater() {
	if (_awayMission->feather.waterMonsterRetreated)
		spockScan(DIR_W, 31, false, true);
	else
		spockScan(DIR_W, 30, false, true);
}

void Room::feather5UseSTricorderOnFern() {
	spockScan(DIR_N, 23, false, true);
}

void Room::feather5UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, 10, true);
}

}
