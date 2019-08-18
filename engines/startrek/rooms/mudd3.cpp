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
#define OBJECT_9 9
#define OBJECT_ALIEN_IMAGE 10
#define OBJECT_ALIENDV 11
#define OBJECT_NORTH_DOOR 12
#define OBJECT_REPAIR_TOOL 13

#define HOTSPOT_SCREEN 0x20
#define HOTSPOT_SPHERE 0x21
#define HOTSPOT_NORTH_DOOR 0x22
#define HOTSPOT_EAST_DOOR 0x23

namespace StarTrek {

// BUG-ish: trying to scan the sphere while someone is behind it causes it to scan that
// object instead.

void Room::mudd3Tick1() {
	playVoc("MUD3LOOP");

	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;

	if (_awayMission->mudd.databaseDestroyed) {
		loadActorAnim(OBJECT_9, "s4lbpb", 0xa2, 0x9f);

		// FIXME: is this supposed to be in this if statement?
		if (!_awayMission->mudd.tookRepairTool)
			loadActorAnim(OBJECT_REPAIR_TOOL, "s4lbdv", 0xb7, 0xa8);
	}

	_awayMission->mudd.muddInDatabaseRoom = false;

	if (!_awayMission->mudd.muddVisitedDatabaseRoom && _awayMission->mudd.translatedAlienLanguage && !_awayMission->mudd.muddUnavailable) {
		_awayMission->mudd.muddVisitedDatabaseRoom = true;
		loadActorAnim(OBJECT_MUDD, "s4lbhs", 0xa2, 0x9f);
		playMidiMusicTracks(3);
		_awayMission->mudd.muddInDatabaseRoom = true;
		_awayMission->timers[2] = 10;
	}
}

void Room::mudd3UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD3_003);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd3LookAtScreen() {
	// BUGFIX: the condition was flipped in the original; the more "vague" description
	// should be shown before the alien language is understood.
	if (_awayMission->mudd.translatedAlienLanguage)
		showText(TX_SPEAKER_SPOCK, TX_MUD3_038);
	else
		showDescription(TX_MUD3N017);
}

void Room::mudd3UseSTricorderOnScreen() {
	if (_roomVar.mudd.tricordersUnavailable)
		showText(TX_SPEAKER_SPOCK, TX_MUD3_039);
	else
		spockScan(DIR_S, TX_MUD3_017, false);
}

// BUGFIX: Event was actually "use screen on kirk", which makes no sense.
void Room::mudd3UseKirkOnScreen() {
	if (!_awayMission->mudd.translatedAlienLanguage) {
		showDescription(TX_MUD3N011);
		showText(TX_SPEAKER_SPOCK, TX_MUD3_037);
	}
}

void Room::mudd3UseSpockOnSphere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	loadActorStandAnim(OBJECT_SPOCK);

	if (_awayMission->mudd.computerDataErasedOrDestroyed) {
		if (!_awayMission->mudd.databaseDestroyed) {
			showDescription(TX_MUD3N000);
			showText(TX_SPEAKER_SPOCK, TX_MUD3_052);
			showText(TX_SPEAKER_KIRK,  TX_MUD3_012);
			showText(TX_SPEAKER_MCCOY, TX_MUD3_024);
		}
		showText(TX_SPEAKER_KIRK,  TX_MUD3_006);
		showText(TX_SPEAKER_SPOCK, TX_MUD3_048);
		showText(TX_SPEAKER_KIRK,  TX_MUD3_008);
		showText(TX_SPEAKER_SPOCK, TX_MUD3_051);
	} else if (_awayMission->mudd.translatedAlienLanguage) {
		if (_awayMission->mudd.accessedAlienDatabase)
			showText(TX_SPEAKER_SPOCK, TX_MUD3_035);
		else {
			_awayMission->mudd.accessedAlienDatabase = true;

			showText(TX_SPEAKER_SPOCK, TX_MUD3_054);
			showText(TX_SPEAKER_MCCOY, TX_MUD3_032);
			showText(TX_SPEAKER_SPOCK, TX_MUD3_050);
			showText(TX_SPEAKER_SPOCK, TX_MUD3_049);
			showText(TX_SPEAKER_SPOCK, TX_MUD3_046);
			showText(TX_SPEAKER_KIRK,  TX_MUD3_007);

			_awayMission->mudd.missionScore++;
			playVoc("LSHAIANS");
			loadActorAnim(OBJECT_ALIEN_IMAGE, "s4lbap", 0x32, 0x64);

			_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
			_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
			_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
			_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
			loadActorStandAnim(OBJECT_KIRK);
			loadActorStandAnim(OBJECT_SPOCK);
			loadActorStandAnim(OBJECT_MCCOY);
			loadActorStandAnim(OBJECT_REDSHIRT);

			_awayMission->disableInput = true;
			_awayMission->timers[3] = 40;
		}
	} else if (_awayMission->mudd.discoveredBase3System) {
		showText(TX_SPEAKER_SPOCK, TX_MUD3_043);
		showText(TX_SPEAKER_MCCOY, TX_MUD3_022);
		showText(TX_SPEAKER_SPOCK, TX_MUD3_056);
		showText(TX_SPEAKER_KIRK,  TX_MUD3_015);
		showText(TX_SPEAKER_SPOCK, TX_MUD3_047);
		_roomVar.mudd.suggestedUsingTricorders = true;
	} else {
		showText(TX_SPEAKER_SPOCK, TX_MUD3_044);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
		walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	}
}

// Finished looking at alien image
void Room::mudd3Timer3Expired() {
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_MCCOY, TX_MUD3_027);
	showText(TX_SPEAKER_SPOCK, TX_MUD3_053);
	showText(TX_SPEAKER_MCCOY, TX_MUD3_028);
	showText(TX_SPEAKER_MCCOY, TX_MUD3_026);
}

void Room::mudd3UseMccoyOnSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	loadActorStandAnim(OBJECT_MCCOY);

	showText(TX_SPEAKER_MCCOY, TX_MUD3_021);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0xec, 0x84);
}

void Room::mudd3UseRedshirtOnSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
	loadActorStandAnim(OBJECT_REDSHIRT);

	showText(TX_SPEAKER_BUCHERT, TX_MUD3_068); // BUGFIX: original played McCoy's audio by mistake
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	walkCrewman(OBJECT_REDSHIRT, 0xec, 0x84);
}

void Room::mudd3UseMTricorderOnSphere() {
	if (_roomVar.mudd.tricordersUnavailable)
		showText(TX_SPEAKER_SPOCK, TX_MUD3_039); // BUGFIX: speaker is Spock
	else {
		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
		loadActorStandAnim(OBJECT_MCCOY);
		if (!_awayMission->mudd.translatedAlienLanguage && _roomVar.mudd.suggestedUsingTricorders && _awayMission->mudd.discoveredBase3System) {
			_awayMission->disableInput = true;

			_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
			walkCrewman(OBJECT_MCCOY, 0xac, 0x88);
			_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
			walkCrewmanC(OBJECT_SPOCK, 0xa5, 0x8c, &Room::mudd3ReadyToHookUpTricorders);
		} else {
			showText(TX_SPEAKER_MCCOY, TX_MUD3_021);
			_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
			walkCrewman(OBJECT_MCCOY, 0xec, 0x84);
		}
	}
}

void Room::mudd3UseSTricorderOnSphere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	loadActorStandAnim(OBJECT_SPOCK);

	if (_roomVar.mudd.tricordersUnavailable)
		showText(TX_SPEAKER_SPOCK, TX_MUD3_039); // BUGFIX: speaker is Spock
	else if (_awayMission->mudd.translatedAlienLanguage || (_awayMission->mudd.discoveredBase3System && !_roomVar.mudd.suggestedUsingTricorders)) {
		showText(TX_SPEAKER_SPOCK, TX_MUD3_041);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
		walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	} else if (_awayMission->mudd.discoveredBase3System && _roomVar.mudd.suggestedUsingTricorders) { // Hooking up tricorders
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
		walkCrewmanC(OBJECT_MCCOY, 0xca, 0x88, &Room::mudd3ReadyToHookUpTricorders);
	} else { // No idea how to access it
		showText(TX_SPEAKER_SPOCK, TX_MUD3_042);
		walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	}
}

void Room::mudd3ReadyToHookUpTricorders() {
	showText(TX_SPEAKER_SPOCK, TX_MUD3_018);

	_roomVar.mudd.tricordersUnavailable = true;
	_awayMission->timers[1] = 50;

	loadActorAnim2(OBJECT_SPOCK, "sscans");
	playSoundEffectIndex(SND_TRICORDER);
}

void Room::mudd3Timer1Expired() { // Tricorders ready for use again
	showText(TX_SPEAKER_SPOCK, TX_MUD3_040);
	showText(TX_SPEAKER_SPOCK, TX_MUD3_034);

	_awayMission->disableInput = false;
	_roomVar.mudd.tricordersUnavailable = false;
	_roomVar.mudd.suggestedUsingTricorders = false;
	_awayMission->mudd.translatedAlienLanguage = true;
	_awayMission->mudd.missionScore++;

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	walkCrewman(OBJECT_MCCOY, 0xec, 0x84);
}

// Harry Mudd stumbled into the room
void Room::mudd3Timer2Expired() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_MUD3_010,
		TX_MUD3_009,
		TX_BLANK
	};

	_awayMission->mudd.computerDataErasedOrDestroyed = true;

	showText(TX_SPEAKER_MUDD, TX_MUD3_065);
	int choice = showMultipleTexts(choices);

	if (choice == 0) { // Allow him to access the database (he ends up erasing it)
		showText(TX_SPEAKER_MUDD, TX_MUD3_066);
		_awayMission->mudd.muddErasedDatabase = true;

		// ENHANCEMENT: Add a few lines to make it clear that Mudd erased the databanks.
		// Otherwise, the end of the mission when you confront Mudd doesn't make sense
		// unless the player happened to try accessing the database again. Also, if you
		// talk to the crew, they berate him for no apparent reason if this isn't clear.
		showDescription(TX_MUD3N000);
		showText(TX_SPEAKER_MCCOY, TX_MUD3_031);

	} else { // Don't allow it (he destroys it by accident)
		showText(TX_SPEAKER_MUDD, TX_MUD3_064);
		_awayMission->timers[4] = 98;
		_awayMission->disableInput = 2;
		playMidiMusicTracks(26);
		loadActorAnim(OBJECT_MUDD, "s4lbhb", 0xa2, 0x9f);
	}
}

void Room::mudd3Timer4Expired() {
	_awayMission->mudd.databaseDestroyed = true;

	showText(TX_SPEAKER_MUDD, TX_MUD3_063);
	showText(TX_SPEAKER_MCCOY, TX_MUD3_031);

	_awayMission->disableInput = false;
}

void Room::mudd3UseMemoryDiskOnSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);

	if (_awayMission->mudd.databaseDestroyed)
		showDescription(TX_MUD3N014);
	else if (_awayMission->mudd.translatedAlienLanguage && !_awayMission->mudd.muddErasedDatabase) {
		showDescription(TX_MUD3N020);
		if (!_awayMission->mudd.gotPointsForDownloadingData) {
			_awayMission->mudd.missionScore += 3;
			_awayMission->mudd.gotPointsForDownloadingData = true;
		}
	} else
		showDescription(TX_MUD3N019);
}


void Room::mudd3GetRepairTool() {
	if (_awayMission->mudd.tookRepairTool)
		showDescription(TX_MUD3N018); // NOTE: unused, since the object disappears, can't be selected again
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0xba, 0xc1, &Room::mudd3ReachedRepairTool);
	}
}

void Room::mudd3ReachedRepairTool() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::mudd3PickedUpRepairTool);
}

void Room::mudd3PickedUpRepairTool() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_REPAIR_TOOL);
	_awayMission->mudd.tookRepairTool = true;
	_awayMission->mudd.missionScore++;
	giveItem(OBJECT_IDOOVER);
}

void Room::mudd3LookAtSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);

	showDescription(TX_MUD3N015);
}

void Room::mudd3WalkToNorthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xb8, 0x70);
}

void Room::mudd3TouchedHotspot0() { // Triggers north door
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
		loadActorAnim(OBJECT_NORTH_DOOR, "s4lbdo", 0xba, 0x6f);
	}
}

void Room::mudd3WalkToEastDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x121, 0x81);
}

void Room::mudd3TouchedHotspot1() {
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
	}
}

void Room::mudd3LookAtKirk() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(TX_MUD3N005);
	else
		showDescription(TX_MUD3N004);
}

void Room::mudd3LookAtSpock() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(TX_MUD3N012);
	else
		showDescription(TX_MUD3N002);
}

void Room::mudd3LookAtMccoy() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(TX_MUD3N010);
	else
		showDescription(TX_MUD3N009);
}

void Room::mudd3LookAtRedshirt() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(TX_MUD3N007);
	else
		showDescription(TX_MUD3N006);
}

void Room::mudd3LookAtMudd() {
	showDescription(TX_MUD3N003);
}

void Room::mudd3TalkToKirk() {
	if (!_awayMission->mudd.muddInDatabaseRoom) {
		showText(TX_SPEAKER_KIRK,  TX_MUD3_014);
		showText(TX_SPEAKER_SPOCK, TX_MUD3_057);
		showText(TX_SPEAKER_MCCOY, TX_MUD3_025);
	} else
		showText(TX_SPEAKER_KIRK, TX_MUD3_002);
}

void Room::mudd3TalkToSpock() {
	if (!_awayMission->mudd.muddInDatabaseRoom) {
		showText(TX_SPEAKER_SPOCK, TX_MUD3_055);
		showText(TX_SPEAKER_MCCOY, TX_MUD3_033);
		showText(TX_SPEAKER_KIRK,  TX_MUD3_011);
	} else
		showText(TX_SPEAKER_SPOCK, TX_MUD3_019);
}

void Room::mudd3TalkToMccoy() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showText(TX_SPEAKER_MCCOY, TX_MUD3_023);
	else {
		showText(TX_SPEAKER_MCCOY, TX_MUD3_030);
		showText(TX_SPEAKER_MUDD,  TX_MUD3_062);
	}
}

void Room::mudd3TalkToRedshirt() {
	if (!_awayMission->mudd.muddInDatabaseRoom) {
		showText(TX_SPEAKER_BUCHERT, TX_MUD3_070);
		showText(TX_SPEAKER_KIRK,    TX_MUD3_016);
		showText(TX_SPEAKER_BUCHERT, TX_MUD3_071);
	} else
		showText(TX_SPEAKER_BUCHERT, TX_MUD3_067);
}

void Room::mudd3TalkToMudd() {
	// ENHANCEMENT: the 2nd part of the if condition is new; whether he physically
	// destroys the database or does through software, he should give this reaction.
	if (_awayMission->mudd.databaseDestroyed || _awayMission->mudd.muddErasedDatabase) {
		showText(TX_SPEAKER_MUDD,    TX_MUD3_060);
		showText(TX_SPEAKER_MCCOY,   TX_MUD3_029);
		showText(TX_SPEAKER_MUDD,    TX_MUD3_061);
		showText(TX_SPEAKER_KIRK,    TX_MUD3_013);
		showText(TX_SPEAKER_MUDD,    TX_MUD3_058);
		showText(TX_SPEAKER_SPOCK,   TX_MUD3C001);
		showText(TX_SPEAKER_BUCHERT, TX_MUD3_069);
		showText(TX_SPEAKER_MUDD,    TX_MUD3_059);
		showText(TX_SPEAKER_KIRK,    TX_MUD3_005);
	}
}

void Room::mudd3UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_MUD3_020);
}

}
