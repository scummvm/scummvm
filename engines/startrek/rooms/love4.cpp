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

extern const RoomAction love4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::love4Tick1 },
	{ {ACTION_TICK, 10, 0, 0}, &Room::love4Tick10 },

	{ {ACTION_WALK, 8, 0, 0}, &Room::love4WalkToDoor },
	{ {ACTION_WALK, 0x20, 0, 0}, &Room::love4WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::love4TouchedHotspot0 },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::love4DoorOpenedOrReached },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::love4DoorOpenedOrReached },

	{ {ACTION_USE, OBJECT_KIRK, 0x21, 0}, &Room::love4UseKirkOnLadder },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0}, &Room::love4ReachedLadder },
	{ {ACTION_USE, OBJECT_IPHASERS,  9, 0}, &Room::love4UseStunPhaserOnRomulan },
	{ {ACTION_USE, OBJECT_IPHASERS, 10, 0}, &Room::love4UseStunPhaserOnRomulan },
	{ {ACTION_USE, OBJECT_IPHASERS, 11, 0}, &Room::love4UseStunPhaserOnRomulan },
	{ {ACTION_USE, OBJECT_IPHASERS, 12, 0}, &Room::love4UseStunPhaserOnRomulan },
	// NOTE: nothing for OBJECT_IPHASERK. There's an unused audio file which might fit.
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::love4LookAnywhere },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::love4LookAtLadder },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::love4LookAtDoor },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0}, &Room::love4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::love4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::love4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::love4LookAtRedshirt },
	{ {ACTION_LOOK,  9, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_LOOK, 10, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_LOOK, 11, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_LOOK, 12, 0, 0}, &Room::love4LookAtRomulan },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0}, &Room::love4TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::love4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::love4TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::love4TalkToSpock },
	{ {ACTION_TALK,  9, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_TALK, 10, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_TALK, 11, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_TALK, 12, 0, 0}, &Room::love4TalkToRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR,  9, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 10, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 11, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 12, 0}, &Room::love4UseMTricorderOnRomulan },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::love4UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::love4UseSTricorderAnywhere },

	{ {ACTION_USE, OBJECT_ICURE,  9, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_USE, OBJECT_ICURE, 10, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_USE, OBJECT_ICURE, 11, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_USE, OBJECT_ICURE, 12, 0}, &Room::love4UseCureOnRomulan },
	{ {ACTION_FINISHED_WALKING,   3, 0, 0}, &Room::love4MccoyReachedRomulan4 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::love4MccoyCuredRomulan4 },
	{ {ACTION_FINISHED_WALKING,   4, 0, 0}, &Room::love4MccoyReachedRomulan3 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0}, &Room::love4MccoyCuredRomulan3 },
	{ {ACTION_FINISHED_WALKING,   5, 0, 0}, &Room::love4MccoyReachedRomulan2 },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::love4MccoyCuredRomulan2 },
	{ {ACTION_FINISHED_WALKING,   6, 0, 0}, &Room::love4MccoyReachedRomulan1 },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0}, &Room::love4MccoyCuredRomulan1 },

	{ {ACTION_USE, OBJECT_IH2O,   9, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IH2O,  10, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IH2O,  11, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IH2O,  12, 0}, &Room::love4UseWaterOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT,  9, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT, 10, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT, 11, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_IMEDKIT, 12, 0}, &Room::love4UseMedkitOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE,  9, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE, 10, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE, 11, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, OBJECT_ISAMPLE, 12, 0}, &Room::love4UseCureSampleOnRomulan },
	{ {ACTION_USE, 0xff,  9, 0}, &Room::love4UseAnythingOnRomulan },
	{ {ACTION_USE, 0xff, 10, 0}, &Room::love4UseAnythingOnRomulan },
	{ {ACTION_USE, 0xff, 11, 0}, &Room::love4UseAnythingOnRomulan },
	{ {ACTION_USE, 0xff, 12, 0}, &Room::love4UseAnythingOnRomulan },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
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
	showText(TX_SPEAKER_MCCOY, 7, true);
}

void Room::love4LookAnywhere() {
	showDescription(9, true);
}

void Room::love4LookAtLadder() {
	showDescription(10, true);
}

void Room::love4LookAtDoor() {
	showDescription(8, true);
}

void Room::love4LookAtKirk() {
	showDescription(2, true);
}

void Room::love4LookAtMccoy() {
	// BUGFIX: original game plays audio "LOV4/LOV4_025". This is mccoy saying something.
	// It doesn't match up with the actual text, which is the narrator saying that mccoy
	// is thinking it.
	// Not sure if this was an intentional decision, but there is another unused audio
	// file which matches with the text more accurately, so I'm using that instead.
	showDescription(12, true);
}

void Room::love4LookAtSpock() {
	showDescription(03, true);
}

void Room::love4LookAtRedshirt() {
	showDescription(01, true);
}

void Room::love4LookAtRomulan() {
	if (!_awayMission->love.romulansCured)
		showDescription(06, true);
	else
		showDescription(11, true);
}

void Room::love4TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  05, true);
	showText(TX_SPEAKER_MCCOY, 24, true);
	showText(TX_SPEAKER_KIRK,  02, true);
}

void Room::love4TalkToMccoy() {
	if (_awayMission->love.romulansCured)
		showText(TX_SPEAKER_MCCOY, 22, true);
	else
		showText(TX_SPEAKER_MCCOY, 10, true);
}

void Room::love4TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, 28, true);
}

void Room::love4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 27, true);
	showText(TX_SPEAKER_KIRK,   3, true);
}

void Room::love4TalkToRomulan() {
	if (_awayMission->love.romulansCured)
		showDescription(7, true); // BUGFIX: original didn't play audio
}

void Room::love4UseMTricorderOnRomulan() {
	if (_awayMission->love.romulansCured)
		mccoyScan(DIR_S, 18, false, true);
	else
		mccoyScan(DIR_S, 15, false, true);
}

void Room::love4UseMTricorderAnywhere() {
	if (_awayMission->love.romulansCured)
		mccoyScan(DIR_S, 19, false, true);
	else if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		mccoyScan(DIR_S, 20, false, true);
	else
		mccoyScan(DIR_S, 21, false, true);
}

void Room::love4UseSTricorderAnywhere() {
	spockScan(DIR_S, 6, false, true);
}


// Mccoy walks around to all romulans, giving each the cure
void Room::love4UseCureOnRomulan() {
	if (_awayMission->love.romulansCured)
		showText(TX_SPEAKER_MCCOY, 13, true);
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

	showDescription(05, true);
	if (!_roomVar.love.gaveWaterToRomulans)
		showText(TX_SPEAKER_MCCOY, 23, true);

	_awayMission->disableInput = false;
}


void Room::love4UseWaterOnRomulan() {
	// BUGFIX: If the romulans are unconscious, you can't use water on them.
	// In the original, you could use water on them, but there would be no corresponding
	// narration, you'd get no points for it, and you'd lose the water anyway.
	if (!_awayMission->love.romulansCured)
		showText(TX_SPEAKER_MCCOY, 9, true);
	else {
		_roomVar.love.gaveWaterToRomulans = true;
		if (_awayMission->love.romulansCured) {
			showDescription(13, true);
			showText(TX_SPEAKER_MCCOY, 26, true);
			showText(TX_SPEAKER_KIRK, 1, true);
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
			showText(TX_SPEAKER_MCCOY, 9, true);
		else {
			showDescription(4, true);
			if (!_awayMission->redshirtDead) {
				showText(TX_SPEAKER_FERRIS, 29, true);
				showText(TX_SPEAKER_KIRK,    4, true);
			}
			showText(TX_SPEAKER_MCCOY, 11, true);
			loseItem(OBJECT_IH2O);
		}
	}
}

void Room::love4UseMedkitOnRomulan() {
	showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::love4UseCureSampleOnRomulan() {
	showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::love4UseAnythingOnRomulan() {
	showText(TX_SPEAKER_MCCOY, 17, true);
}

}
