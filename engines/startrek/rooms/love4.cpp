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

#define OBJECT_DOOR 8
#define OBJECT_ROMULAN_1 9
#define OBJECT_ROMULAN_2 10
#define OBJECT_ROMULAN_3 11
#define OBJECT_ROMULAN_4 12

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_LADDER 0x21

namespace StarTrek {

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
		playMidiMusicTracks(1, -1);
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
		playSoundEffectIndex(SND_DOOR1);
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
	showDescription(TX_LOV4N012);
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
		mccoyScan(DIR_S, TX_LOV4_018, false);
	else
		mccoyScan(DIR_S, TX_LOV4_015, false);
}

void Room::love4UseMTricorderAnywhere() {
	if (_awayMission->love.romulansCured)
		mccoyScan(DIR_S, TX_LOV4_019, false);
	else if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		mccoyScan(DIR_S, TX_LOV4_020, false);
	else
		mccoyScan(DIR_S, TX_LOV4_021, false);
}

void Room::love4UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_LOV4_006, false);
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
