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

#define OBJECT_ENTITY 8
#define OBJECT_MOLTEN_ROCK 9
#define OBJECT_DOOR 10
#define OBJECT_THROWN_IRON_ROD 11

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_LOCK 0x21
#define HOTSPOT_RODS 0x22
#define HOTSPOT_WALL 0x23
#define HOTSPOT_FLOOR 0x24

namespace StarTrek {

extern const RoomAction trial1ActionList[] = {
	{ {ACTION_TICK, 1,  0, 0}, &Room::trial1Tick1 },
	{ {ACTION_TICK, 50, 0, 0}, &Room::trial1Tick50 },
	{ {ACTION_DONE_ANIM, 9, 0, 0}, &Room::trial1DoorOpened },

	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::trial1TouchedHotspot0 },
	{ {ACTION_DONE_ANIM, 1, 0, 0},       &Room::trial1KirkDied },

	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0}, &Room::trial1TouchedHotspot1 },

	{ {ACTION_GET, HOTSPOT_RODS, 0, 0}, &Room::trial1GetRod },
	{ {ACTION_DONE_WALK, 4, 0, 0},      &Room::trial1ReachedRod },
	{ {ACTION_DONE_ANIM, 5, 0, 0},      &Room::trial1PickedUpRod },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0},      &Room::trial1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0},     &Room::trial1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0},     &Room::trial1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0},  &Room::trial1LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_WALL, 0, 0},     &Room::trial1LookAtWall },
	{ {ACTION_LOOK, HOTSPOT_FLOOR, 0, 0},     &Room::trial1LookAtFloor },
	{ {ACTION_LOOK, HOTSPOT_RODS, 0, 0},     &Room::trial1LookAtRods },
	{ {ACTION_LOOK, HOTSPOT_DOOR, 0, 0},     &Room::trial1LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_LOCK, 0, 0},     &Room::trial1LookAtLock },
	{ {ACTION_LOOK, OBJECT_ENTITY, 0, 0},     &Room::trial1LookAtEntity },
	{ {ACTION_LOOK, OBJECT_MOLTEN_ROCK, 0, 0}, &Room::trial1LookAtMoltenRock },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0},      &Room::trial1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0},     &Room::trial1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0},     &Room::trial1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0},  &Room::trial1TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_ENTITY, 0, 0},  &Room::trial1TalkToEntity },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_KIRK,     0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_KIRK,     0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_SPOCK,    0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_SPOCK,    0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY,    0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_MCCOY,    0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_REDSHIRT, 0}, &Room::trial1UsePhaserOnCrewman },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_REDSHIRT, 0}, &Room::trial1UsePhaserOnCrewman },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_ENTITY, 0}, &Room::trial1UsePhaserOnEntity },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_ENTITY, 0}, &Room::trial1UsePhaserOnEntity },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_WALL, 0}, &Room::trial1UsePhaserOnWall },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_WALL, 0}, &Room::trial1UsePhaserOnWall },

	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_FLOOR, 0}, &Room::trial1UseStunPhaserOnFloor },
	{ {ACTION_DONE_WALK, 12, 0, 0}, &Room::trial1ReachedFloorToUseStunPhaser },
	{ {ACTION_DONE_ANIM, 13, 0, 0}, &Room::trial1DoneShootingFloorWithStunPhaser },

	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_FLOOR, 0}, &Room::trial1UseKillPhaserOnFloor },
	{ {ACTION_DONE_WALK, 14, 0, 0}, &Room::trial1ReachedFloorToUseKillPhaser },
	{ {ACTION_DONE_ANIM, 15, 0, 0}, &Room::trial1DoneShootingFloorWithKillPhaser },

	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_RODS, 0}, &Room::trial1UsePhaserOnRod },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_RODS, 0}, &Room::trial1UsePhaserOnRod },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_DOOR, 0}, &Room::trial1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_DOOR, 0}, &Room::trial1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_LOCK, 0}, &Room::trial1UsePhaserOnLock },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_LOCK, 0}, &Room::trial1UsePhaserOnLock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0}, &Room::trial1UseMTricorderOnKirk },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0}, &Room::trial1UseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0}, &Room::trial1UseMTricorderOnMccoy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::trial1UseMTricorderOnRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_ENTITY, 0}, &Room::trial1UseMTricorderOnEntity },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WALL, 0}, &Room::trial1UseSTricorderOnWall },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_ENTITY, 0}, &Room::trial1UseSTricorderOnEntity },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RODS, 0}, &Room::trial1UseSTricorderOnRods },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_FLOOR, 0}, &Room::trial1UseSTricorderOnFloor },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR, 0}, &Room::trial1UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LOCK, 0}, &Room::trial1UseSTricorderOnLock },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::trial1UseCommunicator },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_ENTITY, 0}, &Room::trial1UseMccoyOnEntity },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_LOCK, 0}, &Room::trial1UseMccoyOnLock },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_FLOOR, 0}, &Room::trial1UseMccoyOnFloor },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_ENTITY, 0}, &Room::trial1UseSpockOnEntity },

	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_LOCK, 0}, &Room::trial1UseSpockOnLock },
	{ {ACTION_DONE_WALK, 18, 0, 0},                &Room::trial1SpockReachedKeypad },
	{ {ACTION_DONE_ANIM, 19, 0, 0},                &Room::trial1SpockUsedKeypad },
	{ {ACTION_DONE_WALK, 20, 0, 0},                &Room::trial1SpockReachedKeypadWithExtraProgram },
	{ {ACTION_DONE_ANIM, 21, 0, 0},                &Room::trial1SpockUsedKeypadWithExtraProgram },

	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_FLOOR, 0},    &Room::trial1UseSpockOnFloor },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_ENTITY, 0}, &Room::trial1UseRedshirtOnEntity },

	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_LOCK, 0}, &Room::trial1UseRedshirtOnLock },
	{ {ACTION_DONE_WALK, 24, 0, 0},                   &Room::trial1RedshirtReachedKeypad },
	{ {ACTION_DONE_ANIM, 25, 0, 0},                   &Room::trial1RedshirtUsedKeypad },
	{ {ACTION_DONE_WALK, 26, 0, 0},                   &Room::trial1RedshirtReachedKeypadWithExtraProgram },
	{ {ACTION_DONE_ANIM, 27, 0, 0},                   &Room::trial1RedshirtUsedKeypadWithExtraProgram },

	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_FLOOR, 0}, &Room::trial1UseRedshirtOnFloor },
	{ {ACTION_USE, OBJECT_IWROD, OBJECT_MCCOY, 0},     &Room::trial1UseRodOnMccoy },
	{ {ACTION_USE, OBJECT_IWROD, OBJECT_SPOCK, 0},     &Room::trial1UseRodOnSpock },
	{ {ACTION_USE, OBJECT_IWROD, OBJECT_REDSHIRT, 0},  &Room::trial1UseRodOnRedshirt },
	{ {ACTION_USE, OBJECT_IWROD, HOTSPOT_FLOOR, 0},    &Room::trial1UseRodOnFloorOrWall },
	{ {ACTION_USE, OBJECT_IWROD, HOTSPOT_WALL, 0},     &Room::trial1UseRodOnFloorOrWall },

	{ {ACTION_USE, OBJECT_IWROD, OBJECT_ENTITY, 0},    &Room::trial1UseWoodRodOnEntity },
	{ {ACTION_DONE_WALK, 10, 0, 0},                    &Room::trial1ReachedPositionToThrowWoodRod },
	{ {ACTION_DONE_ANIM,  6, 0, 0},                    &Room::trial1DoneThrowingWoodRod },

	{ {ACTION_USE, OBJECT_IWROD, HOTSPOT_LOCK, 0},    &Room::trial1UseWoodRodOnLock },

	{ {ACTION_USE, OBJECT_IWROD, OBJECT_MOLTEN_ROCK, 0}, &Room::trial1UseWoodRodOnMoltenRock },
	{ {ACTION_DONE_WALK, 16, 0, 0},                      &Room::trial1ReachedMoltenRock },
	{ {ACTION_DONE_ANIM, 17, 0, 0},                      &Room::trial1DoneCoatingWoodRod },

	{ {ACTION_USE, OBJECT_IIROD, OBJECT_MCCOY, 0},     &Room::trial1UseRodOnMccoy },
	{ {ACTION_USE, OBJECT_IIROD, OBJECT_SPOCK, 0},     &Room::trial1UseRodOnSpock },
	{ {ACTION_USE, OBJECT_IIROD, OBJECT_REDSHIRT, 0},  &Room::trial1UseRodOnRedshirt },
	{ {ACTION_USE, OBJECT_IIROD, HOTSPOT_FLOOR, 0},    &Room::trial1UseRodOnFloorOrWall },
	{ {ACTION_USE, OBJECT_IIROD, HOTSPOT_WALL, 0},     &Room::trial1UseRodOnFloorOrWall },

	{ {ACTION_USE, OBJECT_IIROD, OBJECT_ENTITY, 0},    &Room::trial1UseIronRodOnEntity },
	{ {ACTION_DONE_WALK, 11, 0, 0},                    &Room::trial1ReachedPositionToThrowIronRod },
	{ {ACTION_DONE_ANIM,  7, 0, 0},                    &Room::trial1DoneThrowingIronRod },

	{ {ACTION_USE, OBJECT_IIROD, HOTSPOT_LOCK, 0},   &Room::trial1UseIronRodOnLock },
	{ {ACTION_GET, OBJECT_THROWN_IRON_ROD, 0, 0},    &Room::trial1GetThrownIronRod },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0},             &Room::trial1WalkToDoor },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},         &Room::trial1UseMedkit },

	// ENHANCEMENT: Implement actions for the door object, not just the hotspot
	{ {ACTION_LOOK, OBJECT_DOOR, 0, 0},              &Room::trial1LookAtDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_DOOR, 0}, &Room::trial1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_DOOR, 0}, &Room::trial1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DOOR, 0}, &Room::trial1UseSTricorderOnDoor },
	{ {ACTION_WALK, OBJECT_DOOR, 0, 0},              &Room::trial1WalkToDoor },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::trial1Tick1() {
	if (!_awayMission->trial.doorOpen) {
		_awayMission->disableInput = 2;
		loadActorAnim2(OBJECT_DOOR, "t1drc");
	}
	if (!_awayMission->trial.entityDefeated) {
		loadActorAnim2(OBJECT_ENTITY, "ec", 0x5f, 0xb1, 3); // No callback defined
		playVoc("TRI1LOOP");
	}
}

void Room::trial1Tick50() {
	if (!_awayMission->trial.doorOpen)
		_awayMission->disableInput = false;
	if (!_awayMission->trial.entityDefeated) {
		showText(TX_SPEAKER_BENNIE, 56, true);
		showText(TX_SPEAKER_SPOCK,  47, true);
		showText(TX_SPEAKER_MCCOY,  32, true);
		showText(TX_SPEAKER_KIRK,   7, true);
		showText(TX_SPEAKER_BENNIE, 57, true);
		showText(TX_SPEAKER_KIRK,   10, true);
		showText(TX_SPEAKER_BENNIE, 55, true);
	}
}

void Room::trial1DoorOpened() { // Called after Spock opens the door
	_awayMission->trial.doorOpen = true;
	loadActorStandAnim(OBJECT_DOOR);
}

void Room::trial1TouchedHotspot0() { // Kirk touched the energy thing
	if (!_awayMission->trial.entityDefeated) {
		// BUGFIX: Disable input, otherwise you can cancel the death
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "kkills", -1, -1, &Room::trial1KirkDied);
	}
}

void Room::trial1KirkDied() {
	showGameOverMenu();
}

void Room::trial1TouchedHotspot1() { // Entered the door
	if (_awayMission->trial.doorOpen) {
		_awayMission->disableInput = true;
		loadRoomIndex(2, 2);
	}
}


void Room::trial1GetRod() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xe9, 0xa9, &Room::trial1ReachedRod);
}

void Room::trial1ReachedRod() {
	loadActorAnimC(OBJECT_KIRK, "t1pkup", 0xe9, 0xa9, &Room::trial1PickedUpRod);
	giveItem(OBJECT_IWROD);
	if (!_awayMission->trial.gotPointsForGettingRod) {
		_awayMission->trial.gotPointsForGettingRod = true;
		_awayMission->trial.missionScore += 1;
	}
}

void Room::trial1PickedUpRod() {
	_awayMission->disableInput = false;
}

void Room::trial1LookAtKirk() {
	showDescription(3, true);
}

void Room::trial1LookAtSpock() {
	showDescription(17, true);
}

void Room::trial1LookAtMccoy() {
	showDescription(16, true);
}

void Room::trial1LookAtRedshirt() {
	showDescription(15, true);
}

void Room::trial1LookAtWall() {
	showDescription(11, true);
}

void Room::trial1LookAtFloor() {
	showDescription(6, true);
}

void Room::trial1LookAtRods() {
	showDescription(9, true);
}

void Room::trial1LookAtDoor() {
	showDescription(1, true);
}

void Room::trial1LookAtLock() {
	showDescription(2, true);
}

void Room::trial1LookAtEntity() {
	showDescription(0, true);
}

void Room::trial1LookAtMoltenRock() {
	showDescription(7, true);
}

void Room::trial1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_TRI1_J00);
}

void Room::trial1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 46, true);
	showText(TX_SPEAKER_MCCOY, 34, true);
}

void Room::trial1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 19, true);
}

void Room::trial1TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, 53, true);
}

void Room::trial1TalkToEntity() {
	showText(TX_SPEAKER_ENTITY, TX_TRI1C001);
}

void Room::trial1UsePhaserOnCrewman() {
	showText(TX_SPEAKER_BENNIE, 48, true);
}

void Room::trial1UsePhaserOnEntity() {
	showDescription(4, true);
}

void Room::trial1UsePhaserOnWall() {
	showDescription(8, true);
}


void Room::trial1UseStunPhaserOnFloor() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewmanC(OBJECT_KIRK, 0xca, 0xbc, &Room::trial1ReachedFloorToUseStunPhaser);
}

void Room::trial1ReachedFloorToUseStunPhaser() {
	playSoundEffectIndex(kSfxPhaser);
	loadActorAnimC(OBJECT_KIRK, "t1mlts", 0xca, 0xbc, &Room::trial1DoneShootingFloorWithStunPhaser);
}

void Room::trial1DoneShootingFloorWithStunPhaser() {
	_awayMission->disableInput = false;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);
}


void Room::trial1UseKillPhaserOnFloor() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewmanC(OBJECT_KIRK, 0xca, 0xbc, &Room::trial1ReachedFloorToUseKillPhaser);
}

void Room::trial1ReachedFloorToUseKillPhaser() {
	playSoundEffectIndex(kSfxPhaser);
	loadActorAnimC(OBJECT_KIRK, "t1mltk", 0xca, 0xbc, &Room::trial1DoneShootingFloorWithKillPhaser);
}

void Room::trial1DoneShootingFloorWithKillPhaser() {
	showDescription(14, true);
	loadActorAnim2(OBJECT_MOLTEN_ROCK, "t1mltd", 0xca, 0xbc);

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);

	// BUG: Infinite score mechanism
	_awayMission->trial.missionScore += 1;
	_awayMission->disableInput = false;
}

void Room::trial1UsePhaserOnRod() {
	showDescription(12, true);
}

void Room::trial1UsePhaserOnDoor() {
	showText(TX_SPEAKER_SPOCK, 40, true);
}

void Room::trial1UsePhaserOnLock() {
	showText(TX_SPEAKER_MCCOY, 21, true);
}

void Room::trial1UseMTricorderOnKirk() {
	mccoyScan(DIR_S, 24, false, true);
}

void Room::trial1UseMTricorderOnSpock() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_S, 25, false, true);
}

void Room::trial1UseMTricorderOnMccoy() {
	mccoyScan(DIR_S, 22, false, true);
}

void Room::trial1UseMTricorderOnRedshirt() {
	mccoyScan(DIR_S, 23, false, true);
}

void Room::trial1UseMTricorderOnEntity() {
	mccoyScan(DIR_S, 17, false, true);
}

void Room::trial1UseSTricorderOnWall() {
	spockScan(DIR_S, 43, false, true);
}

void Room::trial1UseSTricorderOnEntity() {
	spockScan(DIR_S, 12, false, true);

	// BUG: infinite score mechanism
	_awayMission->trial.missionScore += 1;
}

void Room::trial1UseSTricorderOnRods() {
	spockScan(DIR_S, 41, false, true);
}

void Room::trial1UseSTricorderOnFloor() {
	spockScan(DIR_S, 42, false, true);

	// BUG: infinite score mechanism
	_awayMission->trial.missionScore += 1;
}

void Room::trial1UseSTricorderOnDoor() {
	spockScan(DIR_S, 13, false, true);
}

void Room::trial1UseSTricorderOnLock() {
	spockScan(DIR_S, 14, false, true);

	if (!_awayMission->trial.scannedLock) {
		_awayMission->trial.scannedLock = true;
		// BUGFIX: Moved this into the if statement (used to be an infinite score mechanism)
		_awayMission->trial.missionScore += 1;
	}
}

void Room::trial1UseCommunicator() {
	if (_awayMission->trial.forceFieldDown) {
		showText(TX_SPEAKER_UHURA, 90, true);

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			5, 4, 9,
			TX_BLANK
		};
		int choice = showMultipleTexts(choices, true);

		if (choice == 0) { // "Beam us back to the enterprise"
			_awayMission->trial.missionEndMethod = 1;
			endMission(1, 1, 1);
		} else if (choice == 1) { // "Beam us to Vlict's position"
			showText(TX_SPEAKER_UHURA, 80, true);
			_awayMission->disableInput = true;
			loadRoomIndex(4, 4);
		} // Else don't transport anywhere
	} else { // Force field still up
		if (_awayMission->trial.uhuraAnalyzedCode)
			showText(TX_SPEAKER_UHURA, 68, true);
		else {
			showText(TX_SPEAKER_UHURA, 86, true);
			showText(TX_SPEAKER_UHURA, 106, true);
			showText(TX_SPEAKER_UHURA, 98, true);

			if (_awayMission->trial.scannedLock) {
				showText(TX_SPEAKER_KIRK,  11, true);
				showText(TX_SPEAKER_UHURA, 101, true);

				const TextRef choices[] = {
					TX_SPEAKER_KIRK,
					6, 2,
					TX_BLANK
				};
				int choice = showMultipleTexts(choices, true);

				if (choice == 0) { // Don't analyze the anomolous program
					showText(TX_SPEAKER_UHURA, 79, true);
					_awayMission->trial.doorCodeBehaviour = 2;
					_awayMission->trial.uhuraAnalyzedCode = true;
					_awayMission->trial.missionScore += 1;
				} else { // Analyze the anomolous program
					showText(TX_SPEAKER_UHURA, 102, true);

					const TextRef choices2[] = {
						TX_SPEAKER_KIRK,
						3, 8,
						TX_BLANK
					};
					choice = showMultipleTexts(choices2, true);

					if (choice == 0) { // Activate the program
						showText(TX_SPEAKER_UHURA, 95, true);
						_awayMission->trial.doorCodeBehaviour = 5;
						_awayMission->trial.uhuraAnalyzedCode = true;
						_awayMission->trial.missionScore += 3;
					} else { // Don't activate, only open the door
						showText(TX_SPEAKER_UHURA, 79, true);
						_awayMission->trial.doorCodeBehaviour = 2;
						_awayMission->trial.uhuraAnalyzedCode = true;
						_awayMission->trial.missionScore += 1;
					}
				}
			}
		}
	}
}

void Room::trial1UseMccoyOnEntity() {
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::trial1UseMccoyOnLock() {
	if (!_awayMission->trial.entityDefeated)
		showText(TX_SPEAKER_MCCOY, 18, true);
	else if (_awayMission->trial.doorCodeBehaviour == 0)
		showText(TX_SPEAKER_MCCOY, 20, true);
	else
		showText(TX_SPEAKER_MCCOY, 29, true);
}

void Room::trial1UseMccoyOnFloor() {
	showText(TX_SPEAKER_MCCOY, 31, true);
}

void Room::trial1UseSpockOnEntity() {
	showText(TX_SPEAKER_SPOCK, 39, true);
}


void Room::trial1UseSpockOnLock() {
	if (_awayMission->trial.doorOpen)
		showText(TX_SPEAKER_SPOCK, 38, true);
	else if (!_awayMission->trial.entityDefeated)
		showText(TX_SPEAKER_SPOCK, 36, true);
	else if (_awayMission->trial.doorCodeBehaviour == 0)
		showText(TX_SPEAKER_SPOCK, 44, true);
	else if (_awayMission->trial.doorCodeBehaviour == 2)
		walkCrewmanC(OBJECT_SPOCK, 0x1f, 0xb8, &Room::trial1SpockReachedKeypad);
	else
		walkCrewmanC(OBJECT_SPOCK, 0x1f, 0xb8, &Room::trial1SpockReachedKeypadWithExtraProgram);
}

void Room::trial1SpockReachedKeypad() { // Spock opens the door
	playSoundEffectIndex(kSfxButton);
	loadActorAnimC(OBJECT_SPOCK, "susemw", -1, -1, &Room::trial1SpockUsedKeypad);
}

void Room::trial1SpockUsedKeypad() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	loadActorStandAnim(OBJECT_SPOCK);
	playSoundEffectIndex(kSfxDoor);
	loadActorAnimC(OBJECT_DOOR, "t1drco", -1, -1, &Room::trial1DoorOpened);
	_awayMission->trial.doorOpen = true;
}

void Room::trial1SpockReachedKeypadWithExtraProgram() { // Spock activates the unknown program
	playSoundEffectIndex(kSfxButton);
	loadActorAnimC(OBJECT_SPOCK, "susemw", -1, -1, &Room::trial1SpockUsedKeypadWithExtraProgram);
}

void Room::trial1SpockUsedKeypadWithExtraProgram() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	loadActorStandAnim(OBJECT_SPOCK);

	_awayMission->disableInput = true;
	loadRoomIndex(5, 5);
}


void Room::trial1UseSpockOnFloor() {
	showText(TX_SPEAKER_SPOCK, 35, true);
}

void Room::trial1UseRedshirtOnEntity() {
	showText(TX_SPEAKER_BENNIE, 50, true);
}


void Room::trial1UseRedshirtOnLock() {
	if (_awayMission->trial.doorOpen)
		return;
	else if (!_awayMission->trial.entityDefeated)
		showText(TX_SPEAKER_BENNIE, 51, true);
	else if (_awayMission->trial.doorCodeBehaviour == 0)
		showText(TX_SPEAKER_BENNIE, 54, true);
	else if (_awayMission->trial.doorCodeBehaviour == 2)
		walkCrewmanC(OBJECT_REDSHIRT, 0x1f, 0xb8, &Room::trial1RedshirtReachedKeypad);
	else
		walkCrewmanC(OBJECT_REDSHIRT, 0x1f, 0xb8, &Room::trial1RedshirtReachedKeypadWithExtraProgram);
}

void Room::trial1RedshirtReachedKeypad() { // Redshirt opens the lock
	playSoundEffectIndex(kSfxButton);
	loadActorAnimC(OBJECT_REDSHIRT, "rusemw", -1, -1, &Room::trial1RedshirtUsedKeypad);
}

void Room::trial1RedshirtUsedKeypad() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	loadActorStandAnim(OBJECT_REDSHIRT);
	playSoundEffectIndex(kSfxDoor);
	loadActorAnimC(OBJECT_DOOR, "t1drco", -1, -1, &Room::trial1DoorOpened);
	_awayMission->trial.doorOpen = true;
}

void Room::trial1RedshirtReachedKeypadWithExtraProgram() {
	playSoundEffectIndex(kSfxButton);
	loadActorAnimC(OBJECT_REDSHIRT, "rusemw", -1, -1, &Room::trial1RedshirtUsedKeypadWithExtraProgram);
}

void Room::trial1RedshirtUsedKeypadWithExtraProgram() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	loadActorStandAnim(OBJECT_REDSHIRT);

	_awayMission->disableInput = true;
	loadRoomIndex(5, 5);
}


void Room::trial1UseRedshirtOnFloor() {
	showText(TX_SPEAKER_BENNIE, 49, true);
}

void Room::trial1UseRodOnMccoy() {
	// ENHANCEMENT: This was originally only voiced with the wooden rod, not the iron rod.
	// (This applies to the Spock and Redshirt functions below, too.)
	showText(TX_SPEAKER_MCCOY, 27, true);
}

void Room::trial1UseRodOnSpock() {
	showText(TX_SPEAKER_SPOCK, 37, true);
}

void Room::trial1UseRodOnRedshirt() {
	showText(TX_SPEAKER_BENNIE, 52, true);
}

void Room::trial1UseRodOnFloorOrWall() {
	showDescription(TX_TRI1C002);
}


void Room::trial1UseWoodRodOnEntity() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xe9, 0xad, &Room::trial1ReachedPositionToThrowWoodRod);
	loseItem(OBJECT_IWROD);
}

void Room::trial1ReachedPositionToThrowWoodRod() {
	loadActorAnimC(OBJECT_KIRK, "kthrwr", 0xe9, 0xad, &Room::trial1DoneThrowingWoodRod);
}

void Room::trial1DoneThrowingWoodRod() {
	_awayMission->disableInput = false;
}


void Room::trial1UseWoodRodOnLock() {
	showDescription(5, true);
}


void Room::trial1UseWoodRodOnMoltenRock() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xe4, 0xc4, &Room::trial1ReachedMoltenRock);
}

void Room::trial1ReachedMoltenRock() {
	loadActorAnimC(OBJECT_KIRK, "t1dip", 0xe4, 0xc4, &Room::trial1DoneCoatingWoodRod);
}

void Room::trial1DoneCoatingWoodRod() {
	_awayMission->disableInput = false;
	showDescription(13, true);
	loseItem(OBJECT_IWROD);
	giveItem(OBJECT_IIROD);

	if (!_awayMission->trial.gotPointsForCoatingRodWithIron) {
		_awayMission->trial.gotPointsForCoatingRodWithIron = true;
		_awayMission->trial.missionScore += 1;
	}
}


void Room::trial1UseIronRodOnEntity() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xe9, 0xac, &Room::trial1ReachedPositionToThrowIronRod);
	_awayMission->trial.entityDefeated = true;
	loseItem(OBJECT_IIROD);
}

void Room::trial1ReachedPositionToThrowIronRod() {
	loadActorAnimC(OBJECT_KIRK, "kthri", 0xe9, 0xac, &Room::trial1DoneThrowingIronRod);
	loadActorStandAnim(OBJECT_ENTITY);
}

void Room::trial1DoneThrowingIronRod() {
	loadActorAnim(OBJECT_THROWN_IRON_ROD, "t1irod", 0xe9, 0xac, 8); // No callback defined

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	loadActorStandAnim(OBJECT_KIRK);

	_awayMission->disableInput = false;
	stopAllVocSounds();
	showText(TX_SPEAKER_SPOCK, 45, true);
	showText(TX_SPEAKER_MCCOY, 33, true);

	_awayMission->trial.missionScore += 1;
}


void Room::trial1UseIronRodOnLock() {
	showDescription(4, true);
}

void Room::trial1GetThrownIronRod() {
	showText(TX_SPEAKER_SPOCK, 15, true);
}

void Room::trial1WalkToDoor() {
	if (_awayMission->trial.doorOpen)
		walkCrewman(OBJECT_KIRK, 0x1a, 0xac, 28);
}

void Room::trial1UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 26, true);
}

}
