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

extern const RoomAction mudd3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::mudd3Tick1 },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0},    &Room::mudd3UseCommunicator },
	{ {ACTION_LOOK, 0x20, 0, 0},              &Room::mudd3LookAtScreen },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0}, &Room::mudd3UseSTricorderOnScreen },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0},     &Room::mudd3UseKirkOnScreen },
	{ {ACTION_USE, OBJECT_SPOCK, 0x21, 0},    &Room::mudd3UseSpockOnSphere },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0},        &Room::mudd3Timer3Expired },
	{ {ACTION_USE, OBJECT_MCCOY, 0x21, 0},    &Room::mudd3UseMccoyOnSphere },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x21, 0}, &Room::mudd3UseRedshirtOnSphere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x21, 0}, &Room::mudd3UseMTricorderOnSphere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::mudd3UseSTricorderOnSphere },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0},     &Room::mudd3ReadyToHookUpTricorders },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0},        &Room::mudd3Timer1Expired },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0},        &Room::mudd3Timer2Expired },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0},        &Room::mudd3Timer4Expired },
	{ {ACTION_USE, OBJECT_IDISKS, 0x21, 0},   &Room::mudd3UseMemoryDiskOnSphere },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0}, &Room::muddaUseDegrimer },
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::muddaFiredAlienDevice },

	{ {ACTION_GET, 13, 0, 0}, &Room::mudd3GetRepairTool },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0},   &Room::mudd3ReachedRepairTool },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::mudd3PickedUpRepairTool },
	{ {ACTION_LOOK, 0x21, 0, 0},            &Room::mudd3LookAtSphere },
	{ {ACTION_WALK, 0x22, 0, 0},            &Room::mudd3WalkToNorthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0},    &Room::mudd3TouchedHotspot0 },
	{ {ACTION_WALK, 0x23, 0, 0},            &Room::mudd3WalkToEastDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0},    &Room::mudd3TouchedHotspot1 },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::mudd3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::mudd3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::mudd3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd3LookAtRedshirt },
	{ {ACTION_LOOK, 8,               0, 0}, &Room::mudd3LookAtMudd },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::mudd3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::mudd3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::mudd3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd3TalkToRedshirt },
	{ {ACTION_TALK, 8,               0, 0}, &Room::mudd3TalkToMudd },
	{ {ACTION_USE, OBJECT_IMEDKIT,  0xff, 0}, &Room::mudd3UseMedkit },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

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
	showText(TX_SPEAKER_KIRK,  3, true);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd3LookAtScreen() {
	// BUGFIX: the condition was flipped in the original; the more "vague" description
	// should be shown before the alien language is understood.
	if (_awayMission->mudd.translatedAlienLanguage)
		showText(TX_SPEAKER_SPOCK, 38, true);
	else
		showDescription(17, true);
}

void Room::mudd3UseSTricorderOnScreen() {
	if (_roomVar.mudd.tricordersUnavailable)
		showText(TX_SPEAKER_SPOCK, 39, true);
	else
		spockScan(DIR_S, 17, false, true);
}

// BUGFIX: Event was actually "use screen on kirk", which makes no sense.
void Room::mudd3UseKirkOnScreen() {
	if (!_awayMission->mudd.translatedAlienLanguage) {
		showDescription(11, true);
		showText(TX_SPEAKER_SPOCK, 37, true);
	}
}

void Room::mudd3UseSpockOnSphere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	loadActorStandAnim(OBJECT_SPOCK);

	if (_awayMission->mudd.computerDataErasedOrDestroyed) {
		if (!_awayMission->mudd.databaseDestroyed) {
			showDescription(0, true);
			showText(TX_SPEAKER_SPOCK, 52, true);
			showText(TX_SPEAKER_KIRK,  12, true);
			showText(TX_SPEAKER_MCCOY, 24, true);
		}
		showText(TX_SPEAKER_KIRK,   6, true);
		showText(TX_SPEAKER_SPOCK, 48, true);
		showText(TX_SPEAKER_KIRK,   8, true);
		showText(TX_SPEAKER_SPOCK, 51, true);
	} else if (_awayMission->mudd.translatedAlienLanguage) {
		if (_awayMission->mudd.accessedAlienDatabase)
			showText(TX_SPEAKER_SPOCK, 35, true);
		else {
			_awayMission->mudd.accessedAlienDatabase = true;

			showText(TX_SPEAKER_SPOCK, 54, true);
			showText(TX_SPEAKER_MCCOY, 32, true);
			showText(TX_SPEAKER_SPOCK, 50, true);
			showText(TX_SPEAKER_SPOCK, 49, true);
			showText(TX_SPEAKER_SPOCK, 46, true);
			showText(TX_SPEAKER_KIRK,   7, true);

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
		showText(TX_SPEAKER_SPOCK, 43, true);
		showText(TX_SPEAKER_MCCOY, 22, true);
		showText(TX_SPEAKER_SPOCK, 56, true);
		showText(TX_SPEAKER_KIRK,  15, true);
		showText(TX_SPEAKER_SPOCK, 47, true);
		_roomVar.mudd.suggestedUsingTricorders = true;
	} else {
		showText(TX_SPEAKER_SPOCK, 44, true);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
		walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	}
}

// Finished looking at alien image
void Room::mudd3Timer3Expired() {
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_MCCOY, 27, true);
	showText(TX_SPEAKER_SPOCK, 53, true);
	showText(TX_SPEAKER_MCCOY, 28, true);
	showText(TX_SPEAKER_MCCOY, 26, true);
}

void Room::mudd3UseMccoyOnSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	loadActorStandAnim(OBJECT_MCCOY);

	showText(TX_SPEAKER_MCCOY, 21, true);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0xec, 0x84);
}

void Room::mudd3UseRedshirtOnSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
	loadActorStandAnim(OBJECT_REDSHIRT);

	showText(TX_SPEAKER_BUCHERT, 68, true); // BUGFIX: original played McCoy's audio by mistake
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	walkCrewman(OBJECT_REDSHIRT, 0xec, 0x84);
}

void Room::mudd3UseMTricorderOnSphere() {
	if (_roomVar.mudd.tricordersUnavailable)
		showText(TX_SPEAKER_SPOCK, 39, true); // BUGFIX: speaker is Spock
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
			showText(TX_SPEAKER_MCCOY, 21, true);
			_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
			walkCrewman(OBJECT_MCCOY, 0xec, 0x84);
		}
	}
}

void Room::mudd3UseSTricorderOnSphere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	loadActorStandAnim(OBJECT_SPOCK);

	if (_roomVar.mudd.tricordersUnavailable)
		showText(TX_SPEAKER_SPOCK, 39, true); // BUGFIX: speaker is Spock
	else if (_awayMission->mudd.translatedAlienLanguage || (_awayMission->mudd.discoveredBase3System && !_roomVar.mudd.suggestedUsingTricorders)) {
		showText(TX_SPEAKER_SPOCK, 41, true);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
		walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	} else if (_awayMission->mudd.discoveredBase3System && _roomVar.mudd.suggestedUsingTricorders) { // Hooking up tricorders
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
		walkCrewmanC(OBJECT_MCCOY, 0xca, 0x88, &Room::mudd3ReadyToHookUpTricorders);
	} else { // No idea how to access it
		showText(TX_SPEAKER_SPOCK, 42, true);
		walkCrewman(OBJECT_SPOCK, 0xcd, 0x87);
	}
}

void Room::mudd3ReadyToHookUpTricorders() {
	showText(TX_SPEAKER_SPOCK, 18, true);

	_roomVar.mudd.tricordersUnavailable = true;
	_awayMission->timers[1] = 50;

	loadActorAnim2(OBJECT_SPOCK, "sscans");
	playSoundEffectIndex(kSfxTricorder);
}

void Room::mudd3Timer1Expired() { // Tricorders ready for use again
	showText(TX_SPEAKER_SPOCK, 40, true);
	showText(TX_SPEAKER_SPOCK, 34, true);

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
		10,
		9,
		TX_BLANK
	};

	_awayMission->mudd.computerDataErasedOrDestroyed = true;

	showText(TX_SPEAKER_MUDD, 65, true);
	int choice = showMultipleTexts(choices, true);

	if (choice == 0) { // Allow him to access the database (he ends up erasing it)
		showText(TX_SPEAKER_MUDD, 66, true);
		_awayMission->mudd.muddErasedDatabase = true;

		// ENHANCEMENT: Add a few lines to make it clear that Mudd erased the databanks.
		// Otherwise, the end of the mission when you confront Mudd doesn't make sense
		// unless the player happened to try accessing the database again. Also, if you
		// talk to the crew, they berate him for no apparent reason if this isn't clear.
		showDescription(0, true);
		showText(TX_SPEAKER_MCCOY, 31, true);

	} else { // Don't allow it (he destroys it by accident)
		showText(TX_SPEAKER_MUDD, 64, true);
		_awayMission->timers[4] = 98;
		_awayMission->disableInput = 2;
		playMidiMusicTracks(26);
		loadActorAnim(OBJECT_MUDD, "s4lbhb", 0xa2, 0x9f);
	}
}

void Room::mudd3Timer4Expired() {
	_awayMission->mudd.databaseDestroyed = true;

	showText(TX_SPEAKER_MUDD, 63, true);
	showText(TX_SPEAKER_MCCOY, 31, true);

	_awayMission->disableInput = false;
}

void Room::mudd3UseMemoryDiskOnSphere() {
	// ENHANCEMENT: Turn to face south
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);

	if (_awayMission->mudd.databaseDestroyed)
		showDescription(14, true);
	else if (_awayMission->mudd.translatedAlienLanguage && !_awayMission->mudd.muddErasedDatabase) {
		showDescription(20, true);
		if (!_awayMission->mudd.gotPointsForDownloadingData) {
			_awayMission->mudd.missionScore += 3;
			_awayMission->mudd.gotPointsForDownloadingData = true;
		}
	} else
		showDescription(19, true);
}


void Room::mudd3GetRepairTool() {
	if (_awayMission->mudd.tookRepairTool)
		showDescription(18, true); // NOTE: unused, since the object disappears, can't be selected again
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

	showDescription(15, true);
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
		showDescription(5, true);
	else
		showDescription(4, true);
}

void Room::mudd3LookAtSpock() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(12, true);
	else
		showDescription( 2, true);
}

void Room::mudd3LookAtMccoy() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(10, true);
	else
		showDescription( 9, true);
}

void Room::mudd3LookAtRedshirt() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showDescription(7, true);
	else
		showDescription(6, true);
}

void Room::mudd3LookAtMudd() {
	showDescription(3, true);
}

void Room::mudd3TalkToKirk() {
	if (!_awayMission->mudd.muddInDatabaseRoom) {
		showText(TX_SPEAKER_KIRK,  14, true);
		showText(TX_SPEAKER_SPOCK, 57, true);
		showText(TX_SPEAKER_MCCOY, 25, true);
	} else
		showText(TX_SPEAKER_KIRK,  2, true);
}

void Room::mudd3TalkToSpock() {
	if (!_awayMission->mudd.muddInDatabaseRoom) {
		showText(TX_SPEAKER_SPOCK, 55, true);
		showText(TX_SPEAKER_MCCOY, 33, true);
		showText(TX_SPEAKER_KIRK,  11, true);
	} else
		showText(TX_SPEAKER_SPOCK, 19, true);
}

void Room::mudd3TalkToMccoy() {
	if (!_awayMission->mudd.muddInDatabaseRoom)
		showText(TX_SPEAKER_MCCOY, 23, true);
	else {
		showText(TX_SPEAKER_MCCOY, 30, true);
		showText(TX_SPEAKER_MUDD,  62, true);
	}
}

void Room::mudd3TalkToRedshirt() {
	if (!_awayMission->mudd.muddInDatabaseRoom) {
		showText(TX_SPEAKER_BUCHERT, 70, true);
		showText(TX_SPEAKER_KIRK,    16, true);
		showText(TX_SPEAKER_BUCHERT, 71, true);
	} else
		showText(TX_SPEAKER_BUCHERT, 67, true);
}

void Room::mudd3TalkToMudd() {
	// ENHANCEMENT: the 2nd part of the if condition is new; whether he physically
	// destroys the database or does through software, he should give this reaction.
	if (_awayMission->mudd.databaseDestroyed || _awayMission->mudd.muddErasedDatabase) {
		showText(TX_SPEAKER_MUDD,    60, true);
		showText(TX_SPEAKER_MCCOY,   29, true);
		showText(TX_SPEAKER_MUDD,    61, true);
		showText(TX_SPEAKER_KIRK,    13, true);
		showText(TX_SPEAKER_MUDD,    58, true);
		showText(TX_SPEAKER_SPOCK,   TX_DEM0N009);
		showText(TX_SPEAKER_BUCHERT, 69, true);
		showText(TX_SPEAKER_MUDD,    59, true);
		showText(TX_SPEAKER_KIRK,     5, true);
	}
}

void Room::mudd3UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 20, true);
}

}
