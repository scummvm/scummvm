/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

enum trial1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_UHURA, TX_SPEAKER_BENNIE,
	TX_SPEAKER_ENTITY,
	TX_TRI1_002, TX_TRI1_003, TX_TRI1_004, TX_TRI1_005, TX_TRI1_006,
	TX_TRI1_007, TX_TRI1_008, TX_TRI1_009, TX_TRI1_010, TX_TRI1_011,
	TX_TRI1_012, TX_TRI1_013, TX_TRI1_014, TX_TRI1_015, TX_TRI1_016,
	TX_TRI1_017, TX_TRI1_018, TX_TRI1_019, TX_TRI1_020, TX_TRI1_021,
	TX_TRI1_022, TX_TRI1_023, TX_TRI1_024, TX_TRI1_025, TX_TRI1_026,
	TX_TRI1_027, TX_TRI1_029, TX_TRI1_030, TX_TRI1_031, TX_TRI1_032,
	TX_TRI1_033, TX_TRI1_034, TX_TRI1_035, TX_TRI1_036, TX_TRI1_037,
	TX_TRI1_038, TX_TRI1_039, TX_TRI1_040, TX_TRI1_041, TX_TRI1_042,
	TX_TRI1_043, TX_TRI1_044, TX_TRI1_045, TX_TRI1_046, TX_TRI1_047,
	TX_TRI1_048, TX_TRI1_049, TX_TRI1_050, TX_TRI1_051, TX_TRI1_052,
	TX_TRI1_053, TX_TRI1_054, TX_TRI1_055, TX_TRI1_056, TX_TRI1_057,
	TX_TRI1_J00, TX_TRI1N000, TX_TRI1N001, TX_TRI1N002, TX_TRI1N003,
	TX_TRI1N004, TX_TRI1N005, TX_TRI1N006, TX_TRI1N007, TX_TRI1N008,
	TX_TRI1N009, TX_TRI1N011, TX_TRI1N012, TX_TRI1N013, TX_TRI1N014,
	TX_TRI1N015, TX_TRI1N016, TX_TRI1N017, TX_TRI1U068, TX_TRI1U079,
	TX_TRI1U080, TX_TRI1U086, TX_TRI1U090, TX_TRI1U095, TX_TRI1U098,
	TX_TRI1U101, TX_TRI1U102, TX_TRI1U106, TX_TRI1C001, TX_TRI1C002
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets trial1TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 275, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 286, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 296, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 320, 0, 0, 0 },
	{ TX_SPEAKER_BENNIE, 306, 0, 0, 0 },
	{ TX_SPEAKER_ENTITY, 330, 0, 0, 0 },
	{ TX_TRI1_002, 7154, 0, 0, 0 },
	{ TX_TRI1_003, 7575, 0, 0, 0 },
	{ TX_TRI1_004, 6149, 0, 0, 0 },
	{ TX_TRI1_005, 6076, 0, 0, 0 },
	{ TX_TRI1_006, 7100, 0, 0, 0 },
	{ TX_TRI1_007, 936, 0, 0, 0 },
	{ TX_TRI1_008, 7619, 0, 0, 0 },
	{ TX_TRI1_009, 6209, 0, 0, 0 },
	{ TX_TRI1_010, 1276, 0, 0, 0 },
	{ TX_TRI1_011, 6761, 0, 0, 0 },
	{ TX_TRI1_012, 5106, 0, 0, 0 },
	{ TX_TRI1_013, 5610, 0, 0, 0 },
	{ TX_TRI1_014, 5737, 0, 0, 0 },
	{ TX_TRI1_015, 10856, 0, 0, 0 },
	{ TX_TRI1_016, 7808, 0, 0, 0 },
	{ TX_TRI1_017, 4855, 0, 0, 0 },
	{ TX_TRI1_018, 7936, 0, 0, 0 },
	{ TX_TRI1_019, 3040, 0, 0, 0 },
	{ TX_TRI1_020, 8025, 0, 0, 0 },
	{ TX_TRI1_021, 4261, 0, 0, 0 },
	{ TX_TRI1_022, 4596, 0, 0, 0 },
	{ TX_TRI1_023, 4723, 0, 0, 0 },
	{ TX_TRI1_024, 4387, 0, 0, 0 },
	{ TX_TRI1_025, 4457, 0, 0, 0 },
	{ TX_TRI1_026, 11067, 0, 0, 0 },
	{ TX_TRI1_027, 9814, 0, 0, 0 },
	{ TX_TRI1_029, 8075, 0, 0, 0 },
	{ TX_TRI1_030, 8140, 0, 0, 0 },
	{ TX_TRI1_031, 8211, 0, 0, 0 },
	{ TX_TRI1_032, 865, 0, 0, 0 },
	{ TX_TRI1_033, 10751, 0, 0, 0 },
	{ TX_TRI1_034, 2991, 0, 0, 0 },
	{ TX_TRI1_035, 9017, 0, 0, 0 },
	{ TX_TRI1_036, 8488, 0, 0, 0 },
	{ TX_TRI1_037, 9853, 0, 0, 0 },
	{ TX_TRI1_038, 8412, 0, 0, 0 },
	{ TX_TRI1_039, 8292, 0, 0, 0 },
	{ TX_TRI1_040, 4132, 0, 0, 0 },
	{ TX_TRI1_041, 5347, 0, 0, 0 },
	{ TX_TRI1_042, 5488, 0, 0, 0 },
	{ TX_TRI1_043, 4978, 0, 0, 0 },
	{ TX_TRI1_044, 8651, 0, 0, 0 },
	{ TX_TRI1_045, 10642, 0, 0, 0 },
	{ TX_TRI1_046, 2905, 0, 0, 0 },
	{ TX_TRI1_047, 695, 0, 0, 0 },
	{ TX_TRI1_048, 3398, 0, 0, 0 },
	{ TX_TRI1_049, 9712, 0, 0, 0 },
	{ TX_TRI1_050, 9100, 0, 0, 0 },
	{ TX_TRI1_051, 9218, 0, 0, 0 },
	{ TX_TRI1_052, 9892, 0, 0, 0 },
	{ TX_TRI1_053, 3193, 0, 0, 0 },
	{ TX_TRI1_054, 9318, 0, 0, 0 },
	{ TX_TRI1_055, 1502, 0, 0, 0 },
	{ TX_TRI1_056, 648, 0, 0, 0 },
	{ TX_TRI1_057, 1140, 0, 0, 0 },
	{ TX_TRI1_J00, 2812, 0, 0, 0 },
	{ TX_TRI1N000, 2606, 0, 0, 0 },
	{ TX_TRI1N001, 2431, 0, 0, 0 },
	{ TX_TRI1N002, 2527, 0, 0, 0 },
	{ TX_TRI1N003, 1870, 0, 0, 0 },
	{ TX_TRI1N004, 10811, 0, 0, 0 },
	//{ TX_TRI1N004, 3577, 0, 0, 0 },		// ignore duplicate line
	{ TX_TRI1N005, 10082, 0, 0, 0 },
	{ TX_TRI1N006, 2249, 0, 0, 0 },
	{ TX_TRI1N007, 2714, 0, 0, 0 },
	{ TX_TRI1N008, 3629, 0, 0, 0 },
	{ TX_TRI1N009, 2340, 0, 0, 0 },
	{ TX_TRI1N011, 2154, 0, 0, 0 },
	{ TX_TRI1N012, 4065, 0, 0, 0 },
	{ TX_TRI1N013, 10207, 0, 0, 0 },
	{ TX_TRI1N014, 3932, 0, 0, 0 },
	{ TX_TRI1N015, 2076, 0, 0, 0 },
	{ TX_TRI1N016, 2004, 0, 0, 0 },
	{ TX_TRI1N017, 1938, 0, 0, 0 },
	{ TX_TRI1U068, 6299, 0, 0, 0 },
	{ TX_TRI1U079, 7248, 0, 0, 0 },
	{ TX_TRI1U080, 5936, 0, 0, 0 },
	{ TX_TRI1U086, 6358, 0, 0, 0 },
	{ TX_TRI1U090, 6011, 0, 0, 0 },
	{ TX_TRI1U095, 7738, 0, 0, 0 },
	{ TX_TRI1U098, 6613, 0, 0, 0 },
	{ TX_TRI1U101, 6947, 0, 0, 0 },
	{ TX_TRI1U102, 7379, 0, 0, 0 },
	{ TX_TRI1U106, 6444, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText trial1Texts[] = {
	{ TX_TRI1C001, Common::EN_ANY, "Zzzt! Sptttz! Zzzt! Tttt!" },
	{ TX_TRI1C002, Common::EN_ANY, "Clunk." },
	{ -1, Common::UNK_LANG, "" }
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
		showText(TX_SPEAKER_BENNIE, TX_TRI1_056);
		showText(TX_SPEAKER_SPOCK,  TX_TRI1_047);
		showText(TX_SPEAKER_MCCOY,  TX_TRI1_032);
		showText(TX_SPEAKER_KIRK,   TX_TRI1_007);
		showText(TX_SPEAKER_BENNIE, TX_TRI1_057);
		showText(TX_SPEAKER_KIRK,   TX_TRI1_010);
		showText(TX_SPEAKER_BENNIE, TX_TRI1_055);
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
	showDescription(TX_TRI1N003);
}

void Room::trial1LookAtSpock() {
	showDescription(TX_TRI1N017);
}

void Room::trial1LookAtMccoy() {
	showDescription(TX_TRI1N016);
}

void Room::trial1LookAtRedshirt() {
	showDescription(TX_TRI1N015);
}

void Room::trial1LookAtWall() {
	showDescription(TX_TRI1N011);
}

void Room::trial1LookAtFloor() {
	showDescription(TX_TRI1N006);
}

void Room::trial1LookAtRods() {
	showDescription(TX_TRI1N009);
}

void Room::trial1LookAtDoor() {
	showDescription(TX_TRI1N001);
}

void Room::trial1LookAtLock() {
	showDescription(TX_TRI1N002);
}

void Room::trial1LookAtEntity() {
	showDescription(TX_TRI1N000);
}

void Room::trial1LookAtMoltenRock() {
	showDescription(TX_TRI1N007);
}

void Room::trial1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_TRI1_J00);
}

void Room::trial1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TRI1_046);
	showText(TX_SPEAKER_MCCOY, TX_TRI1_034);
}

void Room::trial1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_TRI1_019);
}

void Room::trial1TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, TX_TRI1_053);
}

void Room::trial1TalkToEntity() {
	showText(TX_SPEAKER_ENTITY, TX_TRI1C001);
}

void Room::trial1UsePhaserOnCrewman() {
	showText(TX_SPEAKER_BENNIE, TX_TRI1_048);
}

void Room::trial1UsePhaserOnEntity() {
	showDescription(TX_TRI1N004);
}

void Room::trial1UsePhaserOnWall() {
	showDescription(TX_TRI1N008);
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
	showDescription(TX_TRI1N014);
	loadActorAnim2(OBJECT_MOLTEN_ROCK, "t1mltd", 0xca, 0xbc);

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);

	// BUG: Infinite score mechanism
	_awayMission->trial.missionScore += 1;
	_awayMission->disableInput = false;
}

void Room::trial1UsePhaserOnRod() {
	showDescription(TX_TRI1N012);
}

void Room::trial1UsePhaserOnDoor() {
	showText(TX_SPEAKER_SPOCK, TX_TRI1_040);
}

void Room::trial1UsePhaserOnLock() {
	showText(TX_SPEAKER_MCCOY, TX_TRI1_021);
}

void Room::trial1UseMTricorderOnKirk() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_TRI1_024, true);
}

void Room::trial1UseMTricorderOnSpock() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_TRI1_025, true);
}

void Room::trial1UseMTricorderOnMccoy() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_TRI1_022, true);
}

void Room::trial1UseMTricorderOnRedshirt() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_TRI1_023, true);
}

void Room::trial1UseMTricorderOnEntity() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_TRI1_017, true);
}

void Room::trial1UseSTricorderOnWall() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_TRI1_043, true);
}

void Room::trial1UseSTricorderOnEntity() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_TRI1_012, true);

	// BUG: infinite score mechanism
	_awayMission->trial.missionScore += 1;
}

void Room::trial1UseSTricorderOnRods() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_TRI1_041, true);
}

void Room::trial1UseSTricorderOnFloor() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_TRI1_042, true);

	// BUG: infinite score mechanism
	_awayMission->trial.missionScore += 1;
}

void Room::trial1UseSTricorderOnDoor() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_TRI1_013, true);
}

void Room::trial1UseSTricorderOnLock() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_TRI1_014, true);

	if (!_awayMission->trial.scannedLock) {
		_awayMission->trial.scannedLock = true;
		// BUGFIX: Moved this into the if statement (used to be an infinite score mechanism)
		_awayMission->trial.missionScore += 1;
	}
}

void Room::trial1UseCommunicator() {
	if (_awayMission->trial.forceFieldDown) {
		showText(TX_SPEAKER_UHURA, TX_TRI1U090);

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_TRI1_005, TX_TRI1_004, TX_TRI1_009,
			TX_END
		};
		int choice = showMultipleTexts(choices);

		if (choice == 0) { // "Beam us back to the enterprise"
			_awayMission->trial.missionEndMethod = 1;
			endMission(1, 1, 1);
		} else if (choice == 1) { // "Beam us to Vlict's position"
			showText(TX_SPEAKER_UHURA, TX_TRI1U080);
			_awayMission->disableInput = true;
			loadRoomIndex(4, 4);
		} // Else don't transport anywhere
	} else { // Force field still up
		if (_awayMission->trial.uhuraAnalyzedCode)
			showText(TX_SPEAKER_UHURA, TX_TRI1U068);
		else {
			showText(TX_SPEAKER_UHURA, TX_TRI1U086);
			showText(TX_SPEAKER_UHURA, TX_TRI1U106);
			showText(TX_SPEAKER_UHURA, TX_TRI1U098);

			if (_awayMission->trial.scannedLock) {
				showText(TX_SPEAKER_KIRK,  TX_TRI1_011);
				showText(TX_SPEAKER_UHURA, TX_TRI1U101);

				const TextRef choices[] = {
					TX_SPEAKER_KIRK,
					TX_TRI1_006, TX_TRI1_002,
					TX_END
				};
				int choice = showMultipleTexts(choices);

				if (choice == 0) { // Don't analyze the anomalous program
					showText(TX_SPEAKER_UHURA, TX_TRI1U079);
					_awayMission->trial.doorCodeBehaviour = 2;
					_awayMission->trial.uhuraAnalyzedCode = true;
					_awayMission->trial.missionScore += 1;
				} else { // Analyze the anomalous program
					showText(TX_SPEAKER_UHURA, TX_TRI1U102);

					const TextRef choices2[] = {
						TX_SPEAKER_KIRK,
						TX_TRI1_003, TX_TRI1_008,
						TX_END
					};
					choice = showMultipleTexts(choices2);

					if (choice == 0) { // Activate the program
						showText(TX_SPEAKER_UHURA, TX_TRI1U095);
						_awayMission->trial.doorCodeBehaviour = 5;
						_awayMission->trial.uhuraAnalyzedCode = true;
						_awayMission->trial.missionScore += 3;
					} else { // Don't activate, only open the door
						showText(TX_SPEAKER_UHURA, TX_TRI1U079);
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
	showText(TX_SPEAKER_MCCOY, TX_TRI1_016);
}

void Room::trial1UseMccoyOnLock() {
	if (!_awayMission->trial.entityDefeated)
		showText(TX_SPEAKER_MCCOY, TX_TRI1_018);
	else if (_awayMission->trial.doorCodeBehaviour == 0)
		showText(TX_SPEAKER_MCCOY, TX_TRI1_020);
	else
		showText(TX_SPEAKER_MCCOY, TX_TRI1_029);
}

void Room::trial1UseMccoyOnFloor() {
	showText(TX_SPEAKER_MCCOY, TX_TRI1_031);
}

void Room::trial1UseSpockOnEntity() {
	showText(TX_SPEAKER_SPOCK, TX_TRI1_039);
}


void Room::trial1UseSpockOnLock() {
	if (_awayMission->trial.doorOpen)
		showText(TX_SPEAKER_SPOCK, TX_TRI1_038);
	else if (!_awayMission->trial.entityDefeated)
		showText(TX_SPEAKER_SPOCK, TX_TRI1_036);
	else if (_awayMission->trial.doorCodeBehaviour == 0)
		showText(TX_SPEAKER_SPOCK, TX_TRI1_044);
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
	showText(TX_SPEAKER_SPOCK, TX_TRI1_035);
}

void Room::trial1UseRedshirtOnEntity() {
	showText(TX_SPEAKER_BENNIE, TX_TRI1_050);
}


void Room::trial1UseRedshirtOnLock() {
	if (_awayMission->trial.doorOpen)
		return;
	else if (!_awayMission->trial.entityDefeated)
		showText(TX_SPEAKER_BENNIE, TX_TRI1_051);
	else if (_awayMission->trial.doorCodeBehaviour == 0)
		showText(TX_SPEAKER_BENNIE, TX_TRI1_054);
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
	showText(TX_SPEAKER_BENNIE, TX_TRI1_049);
}

void Room::trial1UseRodOnMccoy() {
	// ENHANCEMENT: This was originally only voiced with the wooden rod, not the iron rod.
	// (This applies to the Spock and Redshirt functions below, too.)
	showText(TX_SPEAKER_MCCOY, TX_TRI1_027);
}

void Room::trial1UseRodOnSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TRI1_037);
}

void Room::trial1UseRodOnRedshirt() {
	showText(TX_SPEAKER_BENNIE, TX_TRI1_052);
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
	showDescription(TX_TRI1N005);
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
	showDescription(TX_TRI1N013);
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
	showText(TX_SPEAKER_SPOCK, TX_TRI1_045);
	showText(TX_SPEAKER_MCCOY, TX_TRI1_033);

	_awayMission->trial.missionScore += 1;
}


void Room::trial1UseIronRodOnLock() {
	showDescription(TX_TRI1N004);
}

void Room::trial1GetThrownIronRod() {
	showText(TX_SPEAKER_SPOCK, TX_TRI1_015);
}

void Room::trial1WalkToDoor() {
	if (_awayMission->trial.doorOpen)
		walkCrewman(OBJECT_KIRK, 0x1a, 0xac, 28);
}

void Room::trial1UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_TRI1_026);
}

}
