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

#define OBJECT_MUDD 8
#define OBJECT_LIFE_SUPPORT_GENERATOR 9
#define OBJECT_DOOR 10
#define OBJECT_ALIENDV 11

#define HOTSPOT_HATCH 0x20
#define HOTSPOT_ENGINE 0x21
#define HOTSPOT_DOOR 0x22
#define HOTSPOT_CRANE 0x23

const int16 GENERATOR_X = 0x54;
const int16 GENERATOR_Y = 0xb0;

namespace StarTrek {

extern const RoomAction mudd5ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::mudd5Tick1 },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0},        &Room::mudd5Timer1Expired },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0},        &Room::mudd5Timer2Expired },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0},      &Room::mudd5UseCommunicator },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0},   &Room::mudd5UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::mudd5UseSTricorderOnEngine },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0}, &Room::mudd5UseSTricorderOnCrane },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0}, &Room::mudd5UseSTricorderOnHatch },
	{ {ACTION_USE, OBJECT_ISTRICOR, 9,    0}, &Room::mudd5UseSTricorderOnLifeSupportGenerator },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x20, 0}, &Room::mudd5UseStunPhaserOnHatch },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x20, 0}, &Room::mudd5UseKillPhaserOnHatch },
	{ {ACTION_USE, 0xff,            9,    0}, &Room::mudd5UseAnythingOnLifeSupportGenerator },
	{ {ACTION_USE, OBJECT_IDOOVER,  9,    0}, &Room::mudd5UseDooverOnLifeSupportGenerator },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0},     &Room::mudd5KirkReachedLifeSupportGenerator },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0},        &Room::mudd5KirkTimer4Expired },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0},   &Room::mudd5KirkRepairedLifeSupportGenerator },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0},        &Room::mudd5KirkTimer3Expired },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::muddaFiredAlienDevice },
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0}, &Room::muddaUseDegrimer },

	{ {ACTION_LOOK, 0x20, 0, 0},         &Room::mudd5LookAtHatch },
	{ {ACTION_LOOK, 9,    0, 0},         &Room::mudd5LookAtLifeSupportGenerator },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::mudd5TouchedHotspot0 },
	{ {ACTION_WALK, 0x22, 0, 0},         &Room::mudd5WalkToDoor },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0},     &Room::mudd5LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0},    &Room::mudd5LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0},    &Room::mudd5LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd5LookAtRedshirt },
	{ {ACTION_LOOK, 0x22, 0, 0},            &Room::mudd5LookAtDoor },
	{ {ACTION_LOOK, 0x23, 0, 0},            &Room::mudd5LookAtCrane },
	{ {ACTION_LOOK, 0x21, 0, 0},            &Room::mudd5LookAtEngine },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0},     &Room::mudd5TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0},    &Room::mudd5TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0},    &Room::mudd5TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd5TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},  &Room::mudd5UseMedkit },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::mudd5Tick1() {
	playVoc("MUD5LOOP");

	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
	if (_awayMission->mudd.lifeSupportMalfunctioning) {
		playMidiMusicTracks(19);
		loadActorAnim(OBJECT_LIFE_SUPPORT_GENERATOR, "s4epls", GENERATOR_X, GENERATOR_Y);
	} else {
		loadActorAnim(OBJECT_LIFE_SUPPORT_GENERATOR, "s4eplo", GENERATOR_X, GENERATOR_Y);
		_awayMission->mudd.numTimesEnteredRoom5++;
		// BUG: this event can become permanently unavailable? (test)
		if (_awayMission->mudd.numTimesEnteredRoom5 == 2 && !_awayMission->mudd.muddUnavailable && !_awayMission->mudd.repairedLifeSupportGenerator) {
			_awayMission->mudd.muddUnavailable = true;
			_awayMission->mudd.numTimesEnteredRoom5 = 1;
			_awayMission->disableInput = 2;
			playMidiMusicTracks(3);
			loadActorAnim(OBJECT_MUDD, "s4ephh", 0x0e, 0xa7);
			_awayMission->timers[1] = 112;
			_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
		}
	}
}

void Room::mudd5Timer1Expired() { // Mudd enters room through hatch
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		6, true,
		10, true,
		TX_BLANK
	};

	showText(TX_SPEAKER_MUDD, 34, true);
	showMultipleTexts(choices);
	showText(TX_SPEAKER_MUDD, 37, true);

	loadActorAnim2(OBJECT_MUDD, "s4ephc");
	_awayMission->timers[2] = 140;
}

void Room::mudd5Timer2Expired() { // Life-support generator starts acting up
	_awayMission->disableInput = false;
	_awayMission->mudd.lifeSupportMalfunctioning = true;
	playMidiMusicTracks(19);
	loadActorAnim(OBJECT_LIFE_SUPPORT_GENERATOR, "s4epls", GENERATOR_X, GENERATOR_Y);
}

void Room::mudd5UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  3, true);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd5UseSTricorderAnywhere() {
	spockScan(DIR_S, 22, false, true);
}

void Room::mudd5UseSTricorderOnEngine() {
	spockScan(DIR_E, 11, false, true);
}

void Room::mudd5UseSTricorderOnCrane() {
	spockScan(DIR_S, 23, false, true);
}

void Room::mudd5UseSTricorderOnHatch() {
	spockScan(DIR_W, 24, false, true);
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::mudd5UseSTricorderOnLifeSupportGenerator() {
	if (_awayMission->mudd.lifeSupportMalfunctioning)
		spockScan(DIR_W, 19, false, true);
	else
		spockScan(DIR_W, 21, false, true);
}

void Room::mudd5UseStunPhaserOnHatch() {
	showText(TX_SPEAKER_SPOCK, 25, true);
	showText(TX_SPEAKER_MCCOY, 15, true);
	showText(TX_SPEAKER_SPOCK, 30, true);
}

void Room::mudd5UseKillPhaserOnHatch() {
	showText(TX_SPEAKER_BUCHERT, 39, true);
	showText(TX_SPEAKER_SPOCK,   27, true);
}

void Room::mudd5UseAnythingOnLifeSupportGenerator() {
	showText(TX_SPEAKER_MCCOY, 14, true); // BUGFIX: speaker is McCoy, not none
}


void Room::mudd5UseDooverOnLifeSupportGenerator() {
	if (_awayMission->mudd.lifeSupportMalfunctioning) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0x57, 0xb4, &Room::mudd5KirkReachedLifeSupportGenerator);
	} else
		showText(TX_SPEAKER_MCCOY, 14, true); // BUGFIX: speaker is McCoy, not none
}

void Room::mudd5KirkReachedLifeSupportGenerator() {
	loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::mudd5KirkRepairedLifeSupportGenerator);
	_awayMission->timers[4] = 20;
}

void Room::mudd5KirkTimer4Expired() {
	playMidiMusicTracks(-1);
	loadActorAnim2(OBJECT_LIFE_SUPPORT_GENERATOR, "s4eplo");
	_awayMission->mudd.repairedLifeSupportGenerator = true;
	_awayMission->mudd.missionScore += 2;
}

void Room::mudd5KirkRepairedLifeSupportGenerator() {
	showText(TX_SPEAKER_KIRK,  7, true);
	showText(TX_SPEAKER_SPOCK, 28, true);

	_awayMission->mudd.lifeSupportMalfunctioning = false;
	_awayMission->mudd.muddUnavailable = false;
	playMidiMusicTracks(3);
	loadActorAnim(OBJECT_MUDD, "s4ephh", 0x0e, 0xa7);
	_awayMission->disableInput = true;

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	loadActorStandAnim(OBJECT_KIRK);

	_awayMission->timers[3] = 110;
}

void Room::mudd5KirkTimer3Expired() { // Mudd popped back in after repairing generator
	showText(TX_SPEAKER_MUDD, 35, true);
	showText(TX_SPEAKER_KIRK,  5, true);
	showText(TX_SPEAKER_MUDD, 36, true);

	loadActorAnim2(OBJECT_MUDD, "s4ephc");
	_awayMission->disableInput = false;
}

void Room::mudd5LookAtHatch() {
	showDescription(11, true);
	showText(TX_SPEAKER_SPOCK, 31, true);
	showText(TX_SPEAKER_KIRK,   9, true);
}

void Room::mudd5LookAtLifeSupportGenerator() {
	if (_awayMission->mudd.lifeSupportMalfunctioning)
		showDescription(8, true);
	else {
		showDescription(12, true);
		showText(TX_SPEAKER_SPOCK, 26, true);
	}
}

void Room::mudd5TouchedHotspot0() { // Triggers door
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
		loadActorAnim(OBJECT_DOOR, "s4epdo", 0x85, 0x74);
	}
}

void Room::mudd5WalkToDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x85, 0x74);
}

void Room::mudd5LookAtKirk() {
	showDescription(2, true);
}

void Room::mudd5LookAtSpock() {
	showDescription(5, true);
}

void Room::mudd5LookAtMccoy() {
	showDescription(4, true);
}

void Room::mudd5LookAtRedshirt() {
	showDescription(3, true);
}

void Room::mudd5LookAtDoor() {
	showDescription(10, true);
}

void Room::mudd5LookAtCrane() {
	showDescription(0, true);
}

void Room::mudd5LookAtEngine() {
	showDescription(6, true);
}

void Room::mudd5TalkToKirk() {
	showText(TX_SPEAKER_KIRK,   2, true);
	showText(TX_SPEAKER_SPOCK, 32, true);
}

void Room::mudd5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 33, true);
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::mudd5TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 17, true);
	showText(TX_SPEAKER_KIRK,   4, true);
}

void Room::mudd5TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, 40, true);
	showText(TX_SPEAKER_MCCOY,   18, true);
}

void Room::mudd5UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 12, true);
}

}
