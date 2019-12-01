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
	showText(TX_SPEAKER_KIRK,  18, true);
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
			showText(TX_SPEAKER_SPOCK, 57, true);
	} else if (_awayMission->mudd.discoveredBase3System) {
		showText(TX_SPEAKER_SPOCK, 62, true);
	} else {
		showText(TX_SPEAKER_SPOCK, 61, true);
		showText(TX_SPEAKER_KIRK,  13, true);
		showText(TX_SPEAKER_SPOCK, 62, true);
		showText(TX_SPEAKER_MCCOY, 48, true);
		showText(TX_SPEAKER_SPOCK, 59, true);
		showText(TX_SPEAKER_KIRK,  31, true);
		showText(TX_SPEAKER_SPOCK, 60, true);

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

	showText(TX_SPEAKER_SPOCK, 58, true);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4ShowLeftConsoleMenu() {
	const TextRef choices[] = {
		TX_SPEAKER_SPOCK,
		38,
		39,
		40,
		41,
		TX_BLANK
	};

	int choice = showMultipleTexts(choices, true);

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

	default:
		break;
	}
}

void Room::mudd4SpockUsedSensors() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, 53, true);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4SpockUsedEngineering() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, 56, true);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4SpockUsedNavigation() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, 55, true);
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
		showText(TX_SPEAKER_KIRK, 06, true);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		walkCrewmanC(OBJECT_SPOCK, 0x6d, 0x9a, &Room::mudd4FinishedWalking3);
		_awayMission->disableInput = true;
	} else {
		if (!_roomVar.mudd.kirkUsingRightConsole)
			showText(TX_SPEAKER_KIRK, 49, true);
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

	showText(TX_SPEAKER_SPOCK, 52, true); // NOTE: why is Spock talking here?

	mudd4ShowRightConsoleMenu();
}

void Room::mudd4ShowRightConsoleMenu() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		2,
		7,
		3,	// BUGFIX: this used Spock's voice instead of Kirk's in original
		TX_BLANK
	};

	int choice = showMultipleTexts(choices, true);

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

	default:
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
		showText(TX_SPEAKER_KIRK, 04, true);
	mudd4ShowRightConsoleMenu();
}

void Room::mudd4KirkUsedCommunications() {
	_awayMission->disableInput = false;
	if (!_awayMission->mudd.knowAboutTorpedo) {
		showText(TX_SPEAKER_SPOCK, 54, true);
		mudd4ShowRightConsoleMenu();
	} else {
		if (!_awayMission->mudd.contactedEnterpriseFirstTime) {
			_awayMission->mudd.missionScore++;
			showText(TX_SPEAKER_KIRK,  25, true);
			showText(TX_SPEAKER_SCOTT, 2 + SCOTTY_MESSAGE_OFFSET, true);

			if (_awayMission->mudd.torpedoStatus == 1)
				showText(TX_SPEAKER_KIRK, 27, true);
			else
				showText(TX_SPEAKER_KIRK, 28, true);

			showText(TX_SPEAKER_SCOTT, 4 + SCOTTY_MESSAGE_OFFSET, true);

			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				24,
				33,
				22,	// ENHANCEMENT: This line was recorded twice (MUD4_022 and MUD4_023). It's used twice, so, might as well make use of the second recording.
				TX_BLANK
			};

			int choice = showMultipleTexts(choices, true);

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

			showText(TX_SPEAKER_KIRK,  20, true);
			showText(TX_SPEAKER_SCOTT, 3 + SCOTTY_MESSAGE_OFFSET, true);
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
			 9,
			16,
			TX_BLANK
		};

		showText(TX_SPEAKER_MUDD, 66, true);
		int choice = showMultipleTexts(choices, true);

		if (choice == 1) {
			// Copy of code at very bottom of function
			showText(TX_SPEAKER_MUDD, 74, true);
			showText(TX_SPEAKER_KIRK, 17, true);
			showText(TX_SPEAKER_MUDD, 72, true);
			showText(TX_SPEAKER_KIRK,  8, true);
			showText(TX_SPEAKER_MUDD, 68, true);
			showText(TX_SPEAKER_KIRK, 32, true);

			endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
		} else {
			// Threaten to arrest Mudd.
			if (_awayMission->mudd.muddErasedDatabase || _awayMission->mudd.databaseDestroyed)
				showText(TX_SPEAKER_KIRK, 34, true);
			if (_awayMission->mudd.discoveredLenseAndDegrimerFunction)
				showText(TX_SPEAKER_KIRK, 37, true);

			showText(TX_SPEAKER_KIRK, 26, true);

			if (_awayMission->mudd.muddErasedDatabase && _awayMission->mudd.databaseDestroyed) {
				// Mudd offers to copy the lost data to the enterprise computer. You don't
				// get any points for this outcome.

				// NOTE: This combination is probably impossible, making this unused?
				// (Either you give Mudd access to the database, or he destroys it.)

				showText(TX_SPEAKER_MUDD,  71, true);
				showText(TX_SPEAKER_KIRK,  36, true);
				showText(TX_SPEAKER_SPOCK, 29, true);
				showText(TX_SPEAKER_KIRK,  30, true);
			} else {
				// Mudd is forced to agree to give samples to a university.
				showText(TX_SPEAKER_MUDD, 64, true);
				showText(TX_SPEAKER_KIRK, 12, true);
				showText(TX_SPEAKER_MUDD, 70, true);
				showText(TX_SPEAKER_KIRK, 10, true);
				showText(TX_SPEAKER_MUDD, 67, true);
				showText(TX_SPEAKER_KIRK, 11, true);

				_awayMission->mudd.missionScore += 4;
				playMidiMusicTracks(30);

				showText(TX_SPEAKER_MUDD, 73, true);
				showText(TX_SPEAKER_KIRK, 35, true);
			}
			showText(TX_SPEAKER_MUDD, 69, true);
			showText(TX_SPEAKER_KIRK, 01, true);

			endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
		}
	} else {
		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			14,
			21,
			TX_BLANK
		};

		showText(TX_SPEAKER_MUDD, 65, true);
		int choice = showMultipleTexts(choices, true);

		if (choice == 1) {
			showText(TX_SPEAKER_MUDD, 74, true);
			showText(TX_SPEAKER_KIRK, 17, true);
		}
		showText(TX_SPEAKER_MUDD, 72, true);
		showText(TX_SPEAKER_KIRK,  8, true);
		showText(TX_SPEAKER_MUDD, 68, true);
		showText(TX_SPEAKER_KIRK, 32, true);

		endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
	}
}

void Room::mudd4UseMccoyOnConsole() {
	// NOTE: This audio was recorded twice (45, true, and 44, true which is unused)
	showText(TX_SPEAKER_MCCOY, 45, true);

	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x7d, 0xc3);
}

void Room::mudd4UseRedshirtOnConsole() {
	// Floppy version shows two different pieces of text here, but in the CD version,
	// there's no difference
	if (!_awayMission->mudd.translatedAlienLanguage)
		showText(TX_SPEAKER_BUCHERT, 75, true);
	else
		showText(TX_SPEAKER_BUCHERT, 75, true);

	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	walkCrewman(OBJECT_REDSHIRT, 0x2a, 0xbb);
}

void Room::mudd4UseSTricorderOnRepairTool() {
	spockScan(DIR_E, 51, false, true);
}

void Room::mudd4UseSTricorderOnConsole() {
	spockScan(DIR_N, 42, false, true);
}

void Room::mudd4UseSTricorderOnViewscreen() {
	spockScan(DIR_N, 50, false, true);
}

void Room::mudd4UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 43, true);
}

void Room::mudd4GetRepairTool() {
	if (_awayMission->mudd.tookRepairTool)
		showDescription(12, true); // NOTE: unused, since the object disappears, can't be selected again
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
	showDescription(0, true);
}

void Room::mudd4LookAtViewscreen() {
	if (_awayMission->mudd.viewScreenEnabled)
		showDescription(8, true);
	else
		showDescription(13, true);
}

void Room::mudd4LookAtKirk() {
	showDescription(3, true);
}

void Room::mudd4LookAtSpock() {
	showDescription(6, true);
}

void Room::mudd4LookAtMccoy() {
	showDescription(1, true);
}

void Room::mudd4LookAtRedshirt() {
	showDescription(4, true);
}

void Room::mudd4LookAtRepairTool() {
	showDescription(11, true);
}

void Room::mudd4TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 05, true);
}

void Room::mudd4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 63, true);
	showText(TX_SPEAKER_KIRK,  15, true);
	showText(TX_SPEAKER_MCCOY, 47, true);
}

void Room::mudd4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 46, true);
}

void Room::mudd4TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, 76, true);
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
