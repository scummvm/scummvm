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
		TX_MUD5_006,
		TX_MUD5_010,
		TX_BLANK
	};

	showText(TX_SPEAKER_MUDD, TX_MUD5_034);
	showMultipleTexts(choices);
	showText(TX_SPEAKER_MUDD, TX_MUD5_037);

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
	showText(TX_SPEAKER_KIRK,  TX_MUD5_003);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd5UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_MUD5_022, false);
}

void Room::mudd5UseSTricorderOnEngine() {
	spockScan(DIR_E, TX_MUD5_011, false);
}

void Room::mudd5UseSTricorderOnCrane() {
	spockScan(DIR_S, TX_MUD5_023, false);
}

void Room::mudd5UseSTricorderOnHatch() {
	spockScan(DIR_W, TX_MUD5_024, false);
	showText(TX_SPEAKER_KIRK, TX_MUD5_001);
}

void Room::mudd5UseSTricorderOnLifeSupportGenerator() {
	if (_awayMission->mudd.lifeSupportMalfunctioning)
		spockScan(DIR_W, TX_MUD5_019, false);
	else
		spockScan(DIR_W, TX_MUD5_021, false);
}

void Room::mudd5UseStunPhaserOnHatch() {
	showText(TX_SPEAKER_SPOCK, TX_MUD5_025);
	showText(TX_SPEAKER_MCCOY, TX_MUD5_015);
	showText(TX_SPEAKER_SPOCK, TX_MUD5_030);
}

void Room::mudd5UseKillPhaserOnHatch() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD5_039);
	showText(TX_SPEAKER_SPOCK,   TX_MUD5_027);
}

void Room::mudd5UseAnythingOnLifeSupportGenerator() {
	showText(TX_SPEAKER_MCCOY, TX_MUD5_014); // BUGFIX: speaker is McCoy, not none
}


void Room::mudd5UseDooverOnLifeSupportGenerator() {
	if (_awayMission->mudd.lifeSupportMalfunctioning) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0x57, 0xb4, &Room::mudd5KirkReachedLifeSupportGenerator);
	} else
		showText(TX_SPEAKER_MCCOY, TX_MUD5_014); // BUGFIX: speaker is McCoy, not none
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
	showText(TX_SPEAKER_KIRK,  TX_MUD5_007);
	showText(TX_SPEAKER_SPOCK, TX_MUD5_028);

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
	showText(TX_SPEAKER_MUDD, TX_MUD5_035);
	showText(TX_SPEAKER_KIRK, TX_MUD5_005);
	showText(TX_SPEAKER_MUDD, TX_MUD5_036);

	loadActorAnim2(OBJECT_MUDD, "s4ephc");
	_awayMission->disableInput = false;
}

void Room::mudd5LookAtHatch() {
	showDescription(TX_MUD5N011);
	showText(TX_SPEAKER_SPOCK, TX_MUD5_031);
	showText(TX_SPEAKER_KIRK,  TX_MUD5_009);
}

void Room::mudd5LookAtLifeSupportGenerator() {
	if (_awayMission->mudd.lifeSupportMalfunctioning)
		showDescription(TX_MUD5N008);
	else {
		showDescription(TX_MUD5N012);
		showText(TX_SPEAKER_SPOCK, TX_MUD5_026);
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
	showDescription(TX_MUD5N002);
}

void Room::mudd5LookAtSpock() {
	showDescription(TX_MUD5N005);
}

void Room::mudd5LookAtMccoy() {
	showDescription(TX_MUD5N004);
}

void Room::mudd5LookAtRedshirt() {
	showDescription(TX_MUD5N003);
}

void Room::mudd5LookAtDoor() {
	showDescription(TX_MUD5N010);
}

void Room::mudd5LookAtCrane() {
	showDescription(TX_MUD5N000);
}

void Room::mudd5LookAtEngine() {
	showDescription(TX_MUD5N006);
}

void Room::mudd5TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_MUD5_002);
	showText(TX_SPEAKER_SPOCK, TX_MUD5_032);
}

void Room::mudd5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_MUD5_033);
	showText(TX_SPEAKER_MCCOY, TX_MUD5_016);
}

void Room::mudd5TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_MUD5_017);
	showText(TX_SPEAKER_KIRK,  TX_MUD5_004);
}

void Room::mudd5TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD5_040);
	showText(TX_SPEAKER_MCCOY,   TX_MUD5_018);
}

void Room::mudd5UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_MUD5_012);
}

}
