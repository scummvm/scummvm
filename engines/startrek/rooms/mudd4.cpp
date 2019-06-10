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

#define OBJECT_VIEWSCREEN 8
#define OBJECT_ALIENDV 9
#define OBJECT_REPAIR_TOOL 10
#define OBJECT_11 11

#define HOTSPOT_SCREEN 0x20
#define HOTSPOT_CONSOLE 0x21
#define HOTSPOT_LEFT_CONSOLE 0x22
#define HOTSPOT_RIGHT_CONSOLE 0x23
#define HOTSPOT_EAST_DOOR 0x24
#define HOTSPOT_WEST_DOOR 0x25

namespace StarTrek {

void Room::mudd4Tick1() {
	playVoc("MUD4LOOP");

	if (!_awayMission->mudd.tookRepairTool)
		loadActorAnim(OBJECT_REPAIR_TOOL, "s4crdv", 0xc9, 0x8e);

	if (_awayMission->mudd.viewScreenEnabled)
		loadActorAnim(OBJECT_VIEWSCREEN, "s4crvo", 0xa5, 0x76);
}

void Room::mudd4UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD4_018);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd4Timer2Expired() {
	playSoundEffectIndex(SND_07);
}

void Room::mudd4Timer3Expired() {
	playSoundEffectIndex(SND_TRANSENE);
}

void Room::mudd4UseSpockOnLeftConsole() {
	_roomVar.mudd.usingLeftConsole = true;
	mudd4UseSpockOnConsole();
}

void Room::mudd4UseSpockOnRightConsole() {
	_roomVar.mudd.usingLeftConsole = false;
	mudd4UseSpockOnConsole();
}

void Room::mudd4UseSpockOnConsole() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorStandAnim(OBJECT_SPOCK);

	if (_awayMission->mudd.translatedAlienLanguage) {
		if (!_roomVar.mudd.usingLeftConsole)
			showText(TX_SPEAKER_SPOCK, TX_MUD4_057);
	} else if (_awayMission->mudd.discoveredBase3System) {
		showText(TX_SPEAKER_SPOCK, TX_MUD4_062);
	} else {
		showText(TX_SPEAKER_SPOCK, TX_MUD4_061);
		showText(TX_SPEAKER_KIRK,  TX_MUD4_013);
		showText(TX_SPEAKER_SPOCK, TX_MUD4_062);
		showText(TX_SPEAKER_MCCOY, TX_MUD4_048);
		showText(TX_SPEAKER_SPOCK, TX_MUD4_059);
		showText(TX_SPEAKER_KIRK,  TX_MUD4_031);
		showText(TX_SPEAKER_SPOCK, TX_MUD4_060);

		if (!_awayMission->mudd.discoveredBase3System) {
			_awayMission->mudd.discoveredBase3System = true;
			_awayMission->mudd.missionScore++;
		}
	}

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	walkCrewmanC(OBJECT_SPOCK, 0x6d, 0x9a, &Room::mudd4SpockReachedChair);
	_awayMission->disableInput = true;
}

void Room::mudd4SpockReachedChair() {
	if (_roomVar.mudd.usingLeftConsole && _awayMission->mudd.translatedAlienLanguage)
		loadActorAnimC(OBJECT_SPOCK, "s4crss", -1, -1, &Room::mudd4SpockSatInChair);
	else
		_awayMission->disableInput = false;
}

void Room::mudd4SpockSatInChair() {
	loadActorAnim2(OBJECT_SPOCK, "s4crsr", 0x55, 0x98);

	showText(TX_SPEAKER_SPOCK, TX_MUD4_058);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4ShowLeftConsoleMenu() {
	const TextRef choices[] = {
		TX_SPEAKER_SPOCK,
		TX_MUD4_038,
		TX_MUD4_039,
		TX_MUD4_040,
		TX_MUD4_041,
		TX_BLANK
	};

	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0: // Sensors
		loadActorAnimC(OBJECT_SPOCK, "s4crsr", 0x55, 0x98, &Room::mudd4SpockUsedSensors);
		_awayMission->timers[2] = 5;
		break;

	case 1: // Navigation
		loadActorAnimC(OBJECT_SPOCK, "s4crsl", 0x55, 0x98, &Room::mudd4SpockUsedNavigation);
		_awayMission->timers[2] = 5;
		break;

	case 2: // Engineering
		loadActorAnimC(OBJECT_SPOCK, "s4crsr", 0x55, 0x98, &Room::mudd4SpockUsedEngineering);
		_awayMission->timers[2] = 5;
		break;

	case 3: // Done
		_awayMission->disableInput = false;
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		walkCrewman(OBJECT_SPOCK, 0x69, 0xb7);
		break;
	}
}

void Room::mudd4SpockUsedSensors() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_MUD4_053);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4SpockUsedEngineering() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_MUD4_056);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4SpockUsedNavigation() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_MUD4_055);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4FinishedWalking3() { // Spock reaches console after attempting to use Kirk on one
	_awayMission->disableInput = false;
	mudd4UseSpockOnLeftConsole();
}

void Room::mudd4UseKirkOnRightConsole() {
	_roomVar.mudd.kirkUsingRightConsole = true;
	mudd4UseKirkOnConsole();
}

void Room::mudd4UseKirkOnLeftConsole() {
	_roomVar.mudd.kirkUsingRightConsole = false;
	mudd4UseKirkOnConsole();
}

void Room::mudd4UseKirkOnConsole() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	loadActorStandAnim(OBJECT_KIRK);

	if (!_awayMission->mudd.translatedAlienLanguage) {
		showText(TX_SPEAKER_KIRK, TX_MUD4_006);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		walkCrewmanC(OBJECT_SPOCK, 0x6d, 0x9a, &Room::mudd4FinishedWalking3);
		_awayMission->disableInput = true;
	} else {
		if (!_roomVar.mudd.kirkUsingRightConsole)
			showText(TX_SPEAKER_KIRK, TX_MUD4_049);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0x7e, 0x98, &Room::mudd4KirkReachedRightConsole);
		_awayMission->disableInput = true;
	}
}

void Room::mudd4KirkReachedRightConsole() {
	if (_roomVar.mudd.kirkUsingRightConsole && _awayMission->mudd.translatedAlienLanguage)
		loadActorAnimC(OBJECT_KIRK, "s4crks", -1, -1, &Room::mudd4KirkSatInChair);
	else
		_awayMission->disableInput = false;
}

void Room::mudd4KirkSatInChair() {
	loadActorAnim2(OBJECT_KIRK, "s4crkl", 0x94, 0x98);
	_awayMission->timers[2] = 5;
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_SPOCK, TX_MUD4_052); // NOTE: why is Spock talking here?

	mudd4ShowRightConsoleMenu();
}

void Room::mudd4ShowRightConsoleMenu() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_MUD4_002,
		TX_MUD4_007,
		TX_MUD4_003, // BUGFIX: this used Spock's voice instead of Kirk's in original
		TX_BLANK
	};

	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0: // Communications
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "s4crkr", 0x94, 0x98, &Room::mudd4KirkUsedCommunications);
		break;

	case 1: // View Screen
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "s4crkl", 0x94, 0x98, &Room::mudd4KirkUsedViewScreen);
		break;

	case 2: // Done
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0x83, 0xb6);
		break;
	}
}

void Room::mudd4KirkUsedViewScreen() {
	if (!_awayMission->mudd.viewScreenEnabled) {
		_awayMission->mudd.viewScreenEnabled = true;
		playVoc("SE2BIGAS");
		loadActorAnim(OBJECT_VIEWSCREEN, "s4crvs", 0xa5, 0x76);
		_awayMission->timers[3] = 5;
		_awayMission->timers[1] = 50;
		if (!_awayMission->mudd.gotPointsForEnablingViewscreen) {
			_awayMission->mudd.gotPointsForEnablingViewscreen = true;
			_awayMission->mudd.missionScore++;
		}
	} else {
		_awayMission->mudd.viewScreenEnabled = false;
		loadActorAnim(OBJECT_VIEWSCREEN, "s4crvf", 0xa5, 0x76);
		_awayMission->timers[3] = 5;
		_awayMission->timers[1] = 30;
	}
}

void Room::mudd4Timer1Expired() {
	_awayMission->disableInput = false;
	if (_awayMission->mudd.viewScreenEnabled)
		showText(TX_SPEAKER_KIRK, TX_MUD4_004);
	mudd4ShowRightConsoleMenu();
}

void Room::mudd4KirkUsedCommunications() {
	_awayMission->disableInput = false;
	if (!_awayMission->mudd.knowAboutTorpedo) {
		showText(TX_SPEAKER_SPOCK, TX_MUD4_054);
		mudd4ShowRightConsoleMenu();
	} else {
		if (!_awayMission->mudd.contactedEnterpriseFirstTime) {
			_awayMission->mudd.missionScore++;
			showText(TX_SPEAKER_KIRK,  TX_MUD4_025);
			showText(TX_SPEAKER_SCOTT, TX_MUD4_S02);

			if (_awayMission->mudd.torpedoStatus == 1)
				showText(TX_SPEAKER_KIRK, TX_MUD4_027);
			else
				showText(TX_SPEAKER_KIRK, TX_MUD4_028);

			showText(TX_SPEAKER_SCOTT, TX_MUD4_S04);

			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				TX_MUD4_024,
				TX_MUD4_033,
				TX_MUD4_022, // ENHANCEMENT: This line was recorded twice (MUD4_022 and MUD4_023). It's used twice, so, might as well make use of the second recording.
				TX_BLANK
			};

			int choice = showMultipleTexts(choices);

			if (choice == 1) {
				endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
			} else if (choice == 0) {
				_awayMission->mudd.contactedEnterpriseFirstTime = true;
			} else { // choice == 2
				mudd4TalkWithMuddAtMissionEnd();
			}
		} else {
			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				TX_MUD4_019,
				TX_MUD4_A29,
				TX_MUD4_023,
				TX_BLANK
			};

			showText(TX_SPEAKER_KIRK,  TX_MUD4_020);
			showText(TX_SPEAKER_SCOTT, TX_MUD4_S03);
			int choice = showMultipleTexts(choices);

			if (choice == 1) {
				endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
			} else if (choice == 2) {
				mudd4TalkWithMuddAtMissionEnd();
			}
		}
	}
}

void Room::mudd4TalkWithMuddAtMissionEnd() {
	if (_awayMission->mudd.muddUnavailable)
		return;
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
		walkCrewmanC(OBJECT_KIRK, 0x83, 0xb4, &Room::mudd4KirkReachedPositionToTalkToMudd);

		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
		walkCrewman(OBJECT_SPOCK, 0x69, 0xb7);

		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
		loadActorStandAnim(OBJECT_MCCOY);

		_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
		loadActorStandAnim(OBJECT_REDSHIRT);

		playMidiMusicTracks(3);
		loadActorAnim(OBJECT_11, "s4crhw", 0x13f, 0xc7);
	}
}

void Room::mudd4KirkReachedPositionToTalkToMudd() {
	_awayMission->disableInput = false;

	if (_awayMission->mudd.discoveredLenseAndDegrimerFunction
	        || _awayMission->mudd.muddErasedDatabase
	        || _awayMission->mudd.databaseDestroyed
	        || !_awayMission->mudd.accessedAlienDatabase) { // NOTE: why this last line? Test this...
		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_MUD4_009,
			TX_MUD4_016,
			TX_BLANK
		};

		showText(TX_SPEAKER_MUDD, TX_MUD4_066);
		int choice = showMultipleTexts(choices);

		if (choice == 1) {
			// Copy of code at very bottom of function
			showText(TX_SPEAKER_MUDD, TX_MUD4_074);
			showText(TX_SPEAKER_KIRK, TX_MUD4_017);
			showText(TX_SPEAKER_MUDD, TX_MUD4_072);
			showText(TX_SPEAKER_KIRK, TX_MUD4_008);
			showText(TX_SPEAKER_MUDD, TX_MUD4_068);
			showText(TX_SPEAKER_KIRK, TX_MUD4_032);

			endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
		} else {
			// Threaten to arrest Mudd.
			if (_awayMission->mudd.muddErasedDatabase || _awayMission->mudd.databaseDestroyed)
				showText(TX_SPEAKER_KIRK, TX_MUD4_034);
			if (_awayMission->mudd.discoveredLenseAndDegrimerFunction)
				showText(TX_SPEAKER_KIRK, TX_MUD4_037);

			showText(TX_SPEAKER_KIRK, TX_MUD4_026);

			if (_awayMission->mudd.muddErasedDatabase && _awayMission->mudd.databaseDestroyed) {
				// Mudd offers to copy the lost data to the enterprise computer. You don't
				// get any points for this outcome.

				// NOTE: This combination is probably impossible, making this unused?
				// (Either you give Mudd access to the database, or he destroys it.)

				showText(TX_SPEAKER_MUDD,  TX_MUD4_071);
				showText(TX_SPEAKER_KIRK,  TX_MUD4_036);
				showText(TX_SPEAKER_SPOCK, TX_MUD4_029);
				showText(TX_SPEAKER_KIRK,  TX_MUD4_030);
			} else {
				// Mudd is forced to agree to give samples to a university.
				showText(TX_SPEAKER_MUDD, TX_MUD4_064);
				showText(TX_SPEAKER_KIRK, TX_MUD4_012);
				showText(TX_SPEAKER_MUDD, TX_MUD4_070);
				showText(TX_SPEAKER_KIRK, TX_MUD4_010);
				showText(TX_SPEAKER_MUDD, TX_MUD4_067);
				showText(TX_SPEAKER_KIRK, TX_MUD4_011);

				_awayMission->mudd.missionScore += 4;
				playMidiMusicTracks(30);

				showText(TX_SPEAKER_MUDD, TX_MUD4_073);
				showText(TX_SPEAKER_KIRK, TX_MUD4_035);
			}
			showText(TX_SPEAKER_MUDD, TX_MUD4_069);
			showText(TX_SPEAKER_KIRK, TX_MUD4_001);

			endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
		}
	} else {
		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_MUD4_014,
			TX_MUD4_021,
			TX_BLANK
		};

		showText(TX_SPEAKER_MUDD, TX_MUD4_065);
		int choice = showMultipleTexts(choices);

		if (choice == 1) {
			showText(TX_SPEAKER_MUDD, TX_MUD4_074);
			showText(TX_SPEAKER_KIRK, TX_MUD4_017);
		}
		showText(TX_SPEAKER_MUDD, TX_MUD4_072);
		showText(TX_SPEAKER_KIRK, TX_MUD4_008);
		showText(TX_SPEAKER_MUDD, TX_MUD4_068);
		showText(TX_SPEAKER_KIRK, TX_MUD4_032);

		endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
	}
}

void Room::mudd4UseMccoyOnConsole() {
	// NOTE: This audio was recorded twice (TX_MUD4_045, and TX_MUD4_044 which is unused)
	showText(TX_SPEAKER_MCCOY, TX_MUD4_045);

	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x7d, 0xc3);
}

void Room::mudd4UseRedshirtOnConsole() {
	// Floppy version shows two different pieces of text here, but in the CD version,
	// there's no difference
	if (!_awayMission->mudd.translatedAlienLanguage)
		showText(TX_SPEAKER_BUCHERT, TX_MUD4_075);
	else
		showText(TX_SPEAKER_BUCHERT, TX_MUD4_075);

	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	walkCrewman(OBJECT_REDSHIRT, 0x2a, 0xbb);
}

void Room::mudd4UseSTricorderOnRepairTool() {
	spockScan(DIR_E, TX_MUD4_051, false);
}

void Room::mudd4UseSTricorderOnConsole() {
	spockScan(DIR_N, TX_MUD4_042, false);
}

void Room::mudd4UseSTricorderOnViewscreen() {
	spockScan(DIR_N, TX_MUD4_050, false);
}

void Room::mudd4UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_MUD4_043);
}

void Room::mudd4GetRepairTool() {
	if (_awayMission->mudd.tookRepairTool)
		showDescription(TX_MUD4N012); // NOTE: unused, since the object disappears, can't be selected again
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0xcb, 0x9e, &Room::mudd4ReachedRepairTool);
	}
}

void Room::mudd4ReachedRepairTool() {
	loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::mudd4PickedUpRepairTool);
}

void Room::mudd4PickedUpRepairTool() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_REPAIR_TOOL);
	_awayMission->mudd.tookRepairTool = true;
	_awayMission->mudd.missionScore++;
	giveItem(OBJECT_IDOOVER);
}

void Room::mudd4LookAtConsole() {
	showDescription(TX_MUD4N000);
}

void Room::mudd4LookAtViewscreen() {
	if (_awayMission->mudd.viewScreenEnabled)
		showDescription(TX_MUD4N008);
	else
		showDescription(TX_MUD4N013);
}

void Room::mudd4LookAtKirk() {
	showDescription(TX_MUD4N003);
}

void Room::mudd4LookAtSpock() {
	showDescription(TX_MUD4N006);
}

void Room::mudd4LookAtMccoy() {
	showDescription(TX_MUD4N001);
}

void Room::mudd4LookAtRedshirt() {
	showDescription(TX_MUD4N004);
}

void Room::mudd4LookAtRepairTool() {
	showDescription(TX_MUD4N011);
}

void Room::mudd4TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_MUD4_005);
}

void Room::mudd4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_MUD4_063);
	showText(TX_SPEAKER_KIRK,  TX_MUD4_015);
	showText(TX_SPEAKER_MCCOY, TX_MUD4_047);
}

void Room::mudd4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_MUD4_046);
}

void Room::mudd4TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD4_076);
}

void Room::mudd4WalkToEastDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xf0, 0xc7);
}

void Room::mudd4WalkToWestDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x50, 0xc7);
}

void Room::mudd4TouchedHotspot0() { // Trigger door sound
	if (_roomVar.mudd.walkingToDoor)
		playVoc("SMADOOR3");
}

}
