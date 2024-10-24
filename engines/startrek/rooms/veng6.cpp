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

#define OBJECT_CABLE_1 8
#define OBJECT_CABLE_2 9
#define OBJECT_JUNCTION_BOX 10
#define OBJECT_JOURNAL 11
#define OBJECT_DOOR 12
#define OBJECT_DEAD_GUY 13
#define OBJECT_IMPULSE_ENGINES 14
#define OBJECT_CABINET 15

#define HOTSPOT_LEFT_PORT 0x20
#define HOTSPOT_RIGHT_PORT 0x21
#define HOTSPOT_OIL_CANISTER 0x22
#define HOTSPOT_CABINET 0x23
#define HOTSPOT_RIGHT_CONSOLE 0x24 // Fusion mixture regulator
#define HOTSPOT_IMPULSE_ENGINES 0x25
#define HOTSPOT_LEFT_CONSOLE 0x26 // Impulse engine control
#define HOTSPOT_DOOR 0x27

namespace StarTrek {

extern const RoomAction veng6ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::veng6Tick1 },

	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::veng6WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::veng6WalkToDoor },
	{ {ACTION_DONE_WALK, 2,       0, 0, 0}, &Room::veng6ReachedDoor },
	{ {ACTION_DONE_ANIM, 1,       0, 0, 0}, &Room::veng6DoorOpened },

	{ {ACTION_LOOK, HOTSPOT_LEFT_PORT,       0, 0, 0}, &Room::veng6LookAtLeftPort },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_PORT,      0, 0, 0}, &Room::veng6LookAtRightPort },
	{ {ACTION_LOOK, OBJECT_DOOR,             0, 0, 0}, &Room::veng6LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,            0, 0, 0}, &Room::veng6LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_OIL_CANISTER,    0, 0, 0}, &Room::veng6LookAtOilCanister },
	{ {ACTION_LOOK, OBJECT_CABINET,          0, 0, 0}, &Room::veng6LookAtCabinet },
	{ {ACTION_LOOK, HOTSPOT_CABINET,         0, 0, 0}, &Room::veng6LookAtCabinet },
	{ {ACTION_LOOK, OBJECT_CABLE_2,          0, 0, 0}, &Room::veng6LookAtCable },
	{ {ACTION_LOOK, OBJECT_CABLE_1,          0, 0, 0}, &Room::veng6LookAtCable },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_CONSOLE,   0, 0, 0}, &Room::veng6LookAtRightConsole },
	{ {ACTION_LOOK, OBJECT_IMPULSE_ENGINES,  0, 0, 0}, &Room::veng6LookAtImpulseEngines },
	{ {ACTION_LOOK, HOTSPOT_IMPULSE_ENGINES, 0, 0, 0}, &Room::veng6LookAtImpulseEngines },
	{ {ACTION_LOOK, HOTSPOT_LEFT_CONSOLE,    0, 0, 0}, &Room::veng6LookAtLeftConsole },
	{ {ACTION_LOOK, OBJECT_JUNCTION_BOX,     0, 0, 0}, &Room::veng6LookAtJunctionBox },
	{ {ACTION_LOOK, OBJECT_JOURNAL,          0, 0, 0}, &Room::veng6LookAtJournal },
	{ {ACTION_LOOK, OBJECT_KIRK,             0, 0, 0}, &Room::veng6LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,            0, 0, 0}, &Room::veng6LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,            0, 0, 0}, &Room::veng6LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,         0, 0, 0}, &Room::veng6LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_DEAD_GUY,         0, 0, 0}, &Room::veng6LookAtDeadGuy },
	{ {ACTION_LOOK, 0xff,                    0, 0, 0}, &Room::veng6LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,             0, 0, 0}, &Room::veng6TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,            0, 0, 0}, &Room::veng6TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,            0, 0, 0}, &Room::veng6TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,         0, 0, 0}, &Room::veng6TalkToRedshirt },

	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_LEFT_PORT,    0, 0}, &Room::veng6UseCable1OnLeftPort },
	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_LEFT_CONSOLE, 0, 0}, &Room::veng6UseCable1OnLeftPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_LEFT_PORT,    0, 0}, &Room::veng6UseCable2OnLeftPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_LEFT_CONSOLE, 0, 0}, &Room::veng6UseCable2OnLeftPort },
	{ {ACTION_DONE_WALK, 3,                           0, 0, 0}, &Room::veng6KirkOrSpockReachedLeftPort },
	{ {ACTION_DONE_WALK, 4,                           0, 0, 0}, &Room::veng6KirkOrSpockReachedLeftPort },
	{ {ACTION_DONE_ANIM, 5,                           0, 0, 0}, &Room::veng6AttachedLeftCable },

	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_RIGHT_PORT,    0, 0}, &Room::veng6UseCable1OnRightPort },
	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_RIGHT_CONSOLE, 0, 0}, &Room::veng6UseCable1OnRightPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_RIGHT_PORT,    0, 0}, &Room::veng6UseCable2OnRightPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_RIGHT_CONSOLE, 0, 0}, &Room::veng6UseCable2OnRightPort },
	{ {ACTION_DONE_WALK, 6,                           0, 0, 0}, &Room::veng6KirkOrSpockReachedRightPort },
	{ {ACTION_DONE_WALK, 7,                           0, 0, 0}, &Room::veng6KirkOrSpockReachedRightPort },
	{ {ACTION_DONE_ANIM, 8,                           0, 0, 0}, &Room::veng6AttachedRightCable },

	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_JUNCTION_BOX, 0, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_JUNCTION_BOX, 0, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_JUNCTION_BOX, 0, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_JUNCTION_BOX, 0, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_DONE_WALK, 9,                           0, 0, 0}, &Room::veng6SpockReachedJunctionBox },
	{ {ACTION_DONE_ANIM, 10,                          0, 0, 0}, &Room::veng6SpockAdjustedJunctionBox },

	{ {ACTION_USE, OBJECT_IHYPO, HOTSPOT_OIL_CANISTER, 0, 0}, &Room::veng6UseHypoOnOilCanister },
	{ {ACTION_DONE_WALK, 13,                        0, 0, 0}, &Room::veng6ReachedOilCanister },
	{ {ACTION_DONE_ANIM, 14,                        0, 0, 0}, &Room::veng6FilledHypoWithOil },

	{ {ACTION_USE, OBJECT_KIRK, OBJECT_CABINET, 0, 0}, &Room::veng6UseCabinet },
	{ {ACTION_DONE_WALK, 15,                 0, 0, 0}, &Room::veng6ReachedCabinet },
	{ {ACTION_DONE_ANIM, 16,                 0, 0, 0}, &Room::veng6OpenedOrClosedCabinet },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_IMPULSE_ENGINES, 0, 0}, &Room::veng6UseSTricorderOnImpulseEngines },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_CONSOLE,   0, 0}, &Room::veng6UseSTricorderOnRightConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_JUNCTION_BOX,     0, 0}, &Room::veng6UseSTricorderOnJunctionBox },
	{ {ACTION_USE, OBJECT_IFUSION,  HOTSPOT_LEFT_CONSOLE,    0, 0}, &Room::veng6UsePowerPackOnLeftConsole },
	{ {ACTION_GET, OBJECT_JUNCTION_BOX,                   0, 0, 0}, &Room::veng6GetJunctionBox },
	{ {ACTION_GET, HOTSPOT_OIL_CANISTER,                  0, 0, 0}, &Room::veng6GetOilCanister },
	{ {ACTION_GET, OBJECT_JOURNAL,                        0, 0, 0}, &Room::veng6GetEngineeringJournal },
	{ {ACTION_DONE_WALK, 11,                              0, 0, 0}, &Room::veng6ReachedEngineeringJournal },
	{ {ACTION_DONE_ANIM, 12,                              0, 0, 0}, &Room::veng6TookEngineeringJournal },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_DEAD_GUY, 0, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_DEAD_GUY, 0, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_DEAD_GUY, 0, 0}, &Room::vengaUseMccoyOnDeadGuy },

	// ENHANCEMENT
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IMPULSE_ENGINES, 0, 0}, &Room::veng6UseSTricorderOnImpulseEngines },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum veng6TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_KIJE, TX_SPEAKER_COMPUTER,
	TX_BRIDU146, TX_COMPA185, TX_DEM3_019, TX_MUD1N014, TX_MUD4_018,
	TX_VEN0N016, TX_VEN0_016, TX_VEN2U093, TX_VEN2_028,
	TX_VEN2_066, TX_VEN2_098, TX_VEN4N010, TX_VEN4_016, TX_VEN5N007,
	TX_VEN6N000, TX_VEN6N001, TX_VEN6N002, TX_VEN6N003, TX_VEN6N005,
	TX_VEN6N006, TX_VEN6N007, TX_VEN6N008, TX_VEN6N009, TX_VEN6N010,
	TX_VEN6N011, TX_VEN6N012, TX_VEN6N013, TX_VEN6N015, TX_VEN6N016,
	TX_VEN6N017, TX_VEN6N018, TX_VEN6N019, TX_VEN6N020, TX_VEN6N022,
	TX_VEN6N024, TX_VEN6N025, TX_VEN6_001, TX_VEN6_002, TX_VEN6_003,
	TX_VEN6_005, TX_VEN6_006, TX_VEN6_007, TX_VEN6_008, TX_VEN6_009,
	TX_VEN6_010, TX_VEN6_011, TX_VEN6_012, TX_VEN6_013, TX_VEN6_014,
	TX_VEN6_015, TX_VEN6_016, TX_VEN6_017, TX_VEN6_018, TX_VEN6_019,
	TX_VEN6_020, TX_VEN6_021, TX_VEN6_022, TX_VEN6_023, TX_VENA_F32,
	TX_VENA_F34, TX_VENA_F40
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets veng6TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 8894, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 8905, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 8915, 0, 0, 0 },
	{ TX_SPEAKER_KIJE, 8925, 0, 0, 0 },
	{ TX_SPEAKER_COMPUTER, 8937, 0, 0, 0 },
	{ TX_BRIDU146, 7831, 0, 0, 0 },
	{ TX_COMPA185, 4008, 0, 0, 0 },
	{ TX_DEM3_019, 8315, 0, 0, 0 },
	{ TX_MUD1N014, 6550, 0, 0, 0 },
	{ TX_MUD4_018, 7762, 0, 0, 0 },
	//{ TX_MUD4_018, 7895, 0, 0, 0 },	// Skip duplicate text
	{ TX_VEN0N016, 2156, 0, 0, 0 },
	{ TX_VEN0_016, 8552, 0, 0, 0 },
	//{ TX_VEN0_016, 8611, 0, 0, 0 },	// Skip duplicate text
	{ TX_VEN2U093, 7963, 0, 0, 0 },
	{ TX_VEN2_028, 8070, 0, 0, 0 },
	{ TX_VEN2_066, 8148, 0, 0, 0 },
	{ TX_VEN2_098, 6958, 0, 0, 0 },
	{ TX_VEN4N010, 7612, 0, 0, 0 },
	{ TX_VEN4_016, 8366, 0, 0, 0 },
	{ TX_VEN5N007, 775, 0, 0, 0 },
	{ TX_VEN6N000, 2099, 0, 0, 0 },
	{ TX_VEN6N001, 1781, 0, 0, 0 },
	{ TX_VEN6N002, 1999, 0, 0, 0 },
	{ TX_VEN6N003, 1873, 0, 0, 0 },
	{ TX_VEN6N005, 5276, 0, 0, 0 },
	{ TX_VEN6N006, 4887, 0, 0, 0 },
	{ TX_VEN6N007, 4998, 0, 0, 0 },
	{ TX_VEN6N007, 7677, 0, 0, 0 },
	{ TX_VEN6N008, 6615, 0, 0, 0 },
	{ TX_VEN6N009, 1334, 0, 0, 0 },
	{ TX_VEN6N010, 1253, 0, 0, 0 },
	{ TX_VEN6N011, 845, 0, 0, 0 },
	{ TX_VEN6N012, 1425, 0, 0, 0 },
	{ TX_VEN6N013, 1052, 0, 0, 0 },
	{ TX_VEN6N015, 976, 0, 0, 0 },
	{ TX_VEN6N016, 8975, 0, 0, 0 },
	{ TX_VEN6N018, 9063, 0, 0, 0 },
	{ TX_VEN6N017, 1139, 0, 0, 0 },
	{ TX_VEN6N019, 2207, 0, 0, 0 },
	{ TX_VEN6N020, 3318, 0, 0, 0 },
	{ TX_VEN6N022, 3710, 0, 0, 0 },
	{ TX_VEN6N024, 6803, 0, 0, 0 },
	{ TX_VEN6N025, 1536, 0, 0, 0 },
	{ TX_VEN6_001, 5699, 0, 0, 0 },
	{ TX_VEN6_002, 4457, 0, 0, 0 },
	{ TX_VEN6_003, 4062, 0, 0, 0 },
	{ TX_VEN6_005, 2298, 0, 0, 0 },
	{ TX_VEN6_006, 6407, 0, 0, 0 },
	{ TX_VEN6_007, 5830, 0, 0, 0 },
	{ TX_VEN6_008, 2674, 0, 0, 0 },
	{ TX_VEN6_009, 2457, 0, 0, 0 },
	{ TX_VEN6_010, 5433, 0, 0, 0 },
	{ TX_VEN6_011, 4737, 0, 0, 0 },
	{ TX_VEN6_012, 3881, 0, 0, 0 },
	{ TX_VEN6_013, 5972, 0, 0, 0 },
	{ TX_VEN6_014, 2534, 0, 0, 0 },
	{ TX_VEN6_015, 6196, 0, 0, 0 },
	{ TX_VEN6_016, 4297, 0, 0, 0 },
	{ TX_VEN6_017, 4105, 0, 0, 0 },
	{ TX_VEN6_018, 2781, 0, 0, 0 },
	{ TX_VEN6_019, 4596, 0, 0, 0 },
	{ TX_VEN6_020, 5500, 0, 0, 0 },
	{ TX_VEN6_021, 1616, 0, 0, 0 },
	{ TX_VEN6_022, 9197, 0, 0, 0 },
	{ TX_VEN6_023, 2954, 0, 0, 0 },
	{ TX_VENA_F32, 7188, 0, 0, 0 },
	{ TX_VENA_F34, 7458, 0, 0, 0 },
	{ TX_VENA_F40, 7053, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText veng6Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

#define DOOR_X 0x13e
#define DOOR_Y 0xa8

#define CABLE_1_X 0x5d
#define CABLE_1_Y 0x95

#define CABLE_2_X 0x107
#define CABLE_2_Y 0x94

#define IMPULSE_ENGINE_X 0x9e
#define IMPULSE_ENGINE_Y 0x59

#define CABINET_X 0x19
#define CABINET_Y 0x86

void Room::veng6Tick1() {
	loadActorAnim(OBJECT_DOOR, "s7r6d1c", DOOR_X, DOOR_Y);
	loadActorAnim(OBJECT_DEAD_GUY, "s7r6c1", 0x82, 0x5c);
	loadActorAnim(OBJECT_JUNCTION_BOX, "s7r6j1", 0x9a, 0xaf);

	if (_awayMission->veng.junctionCablesConnected & 1)
		loadActorAnim(OBJECT_CABLE_1, "s7r6cl", CABLE_1_X, CABLE_1_Y);
	if (_awayMission->veng.junctionCablesConnected & 2)
		loadActorAnim(OBJECT_CABLE_2, "s7r6cr", CABLE_2_X, CABLE_2_Y);

	if (_awayMission->veng.impulseEnginesOn) {
		loadActorAnim(OBJECT_IMPULSE_ENGINES, "s7r6i1", IMPULSE_ENGINE_X, IMPULSE_ENGINE_Y);
		// FIXME: doesn't loop
		playVoc("VEN6LON");
	} else
		playVoc("VEN6LOFF");

	if (!_awayMission->veng.tookEngineeringJournal)
		loadActorAnim(OBJECT_JOURNAL, "s7r6ej", 0x24, 0x82);

	loadActorAnim(OBJECT_CABINET, "s7r6p1c", CABINET_X, CABINET_Y);
	if (_awayMission->veng.engineeringCabinetOpen)
		loadActorAnim(OBJECT_CABINET, "s7r6p1o", CABINET_X, CABINET_Y);
}

void Room::veng6WalkToDoor() {
	walkCrewmanC(OBJECT_KIRK, 0x120, 0xad, &Room::veng6ReachedDoor);
}

void Room::veng6ReachedDoor() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	loadActorAnimC(OBJECT_DOOR, "s7r6d1", DOOR_X, DOOR_Y, &Room::veng6DoorOpened);
}

void Room::veng6DoorOpened() {
	walkCrewman(OBJECT_KIRK, 0x136, 0xaa);
}

void Room::veng6LookAtLeftPort() {
	showDescription(TX_VEN6N018);
}

void Room::veng6LookAtRightPort() {
	showDescription(TX_VEN6N016);
}

void Room::veng6LookAtDoor() {
	showDescription(TX_VEN5N007);
}

void Room::veng6LookAtOilCanister() {
	showDescription(TX_VEN6N011);
}

void Room::veng6LookAtCabinet() {
	showDescription(TX_VEN6N015);
}

void Room::veng6LookAtCable() {
	showDescription(TX_VEN6N013);
}

void Room::veng6LookAtRightConsole() {
	showDescription(TX_VEN6N017);
}

void Room::veng6LookAtImpulseEngines() {
	showDescription(TX_VEN6N010);
}

void Room::veng6LookAtLeftConsole() {
	showDescription(TX_VEN6N009);
}

void Room::veng6LookAtJunctionBox() {
	showDescription(TX_VEN6N012);
}

void Room::veng6LookAtJournal() {
	showDescription(TX_VEN6N025);
	showText(TX_SPEAKER_SPOCK, TX_VEN6_021);
}

void Room::veng6LookAtKirk() {
	showDescription(TX_VEN6N001);
}

void Room::veng6LookAtSpock() {
	showDescription(TX_VEN6N003);
}

void Room::veng6LookAtMccoy() {
	showDescription(TX_VEN6N002);
}

void Room::veng6LookAtRedshirt() {
	showDescription(TX_VEN6N000);
}

void Room::veng6LookAtDeadGuy() {
	showDescription(TX_VEN0N016);
}

void Room::veng6LookAnywhere() {
	showDescription(TX_VEN6N019);
}

void Room::veng6TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_VEN6_005);
	showText(TX_SPEAKER_MCCOY, TX_VEN6_009);
}

void Room::veng6TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_VEN6_014);
}

void Room::veng6TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN6_008);
	showText(TX_SPEAKER_SPOCK, TX_VEN6_018);
}

void Room::veng6TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, TX_VEN6_023);
}

void Room::veng6UseCable1OnLeftPort() {
	veng6UseCableOnLeftPort(OBJECT_ICABLE1);
}

void Room::veng6UseCable2OnLeftPort() {
	veng6UseCableOnLeftPort(OBJECT_ICABLE2);
}

void Room::veng6UseCableOnLeftPort(byte cable) {
	_roomVar.veng.cableInUse = cable;

	if (!(_awayMission->veng.junctionCablesConnected & 1)) {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x91, 0xb4, &Room::veng6KirkOrSpockReachedLeftPort);
		walkCrewmanC(OBJECT_KIRK,  0x5c, 0xa0, &Room::veng6KirkOrSpockReachedLeftPort);
	}
}

void Room::veng6KirkOrSpockReachedLeftPort() {
	if (++_roomVar.veng.kirkAndSpockReadyToAttachLeftCable == 2) {
		loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::veng6AttachedLeftCable);
		loadActorAnim(OBJECT_SPOCK, "suseln");
		playVoc("EFX4");
	}
}

void Room::veng6AttachedLeftCable() {
	loadActorAnim(OBJECT_CABLE_1, "s7r6cl", CABLE_1_X, CABLE_1_Y);
	showDescription(TX_VEN6N020);
	_awayMission->veng.junctionCablesConnected |= 1;
	loseItem(_roomVar.veng.cableInUse);
	_awayMission->disableInput = false;
}

void Room::veng6UseCable1OnRightPort() {
	veng6UseCableOnRightPort(OBJECT_ICABLE1);
}

void Room::veng6UseCable2OnRightPort() {
	veng6UseCableOnRightPort(OBJECT_ICABLE2);
}

void Room::veng6UseCableOnRightPort(byte cable) {
	_roomVar.veng.cableInUse = cable;

	if (!(_awayMission->veng.junctionCablesConnected & 2)) {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0xac, 0xb4, &Room::veng6KirkOrSpockReachedRightPort);
		walkCrewmanC(OBJECT_KIRK, 0x107, 0xa0, &Room::veng6KirkOrSpockReachedRightPort);
	}
}

void Room::veng6KirkOrSpockReachedRightPort() {
	if (++_roomVar.veng.kirkAndSpockReadyToAttachRightCable == 2) {
		loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::veng6AttachedRightCable);
		loadActorAnim(OBJECT_SPOCK, "suseln");
		playVoc("EFX4");
	}
}

void Room::veng6AttachedRightCable() {
	loadActorAnim(OBJECT_CABLE_2, "s7r6cr", CABLE_2_X, CABLE_2_Y);
	showDescription(TX_VEN6N022);
	_awayMission->veng.junctionCablesConnected |= 2;
	loseItem(_roomVar.veng.cableInUse);
	_awayMission->disableInput = false;
}

void Room::veng6UseCrewmanOnJunctionBox() {
	if (_awayMission->veng.impulseEnginesOn)
		showText(TX_SPEAKER_SPOCK, TX_VEN6_012);
	else if ((_awayMission->veng.junctionCablesConnected & 3) == 3) {
		showText(TX_SPEAKER_COMPUTER, TX_COMPA185);
		showText(TX_SPEAKER_KIRK, TX_VEN6_003);
		if (!_awayMission->veng.readEngineeringJournal)
			showText(TX_SPEAKER_SPOCK, TX_VEN6_017);
		else {
			showText(TX_SPEAKER_SPOCK, TX_VEN6_016);
			showText(TX_SPEAKER_KIRK,  TX_VEN6_002);
			walkCrewmanC(OBJECT_SPOCK, 0x99, 0xb4, &Room::veng6SpockReachedJunctionBox);
		}
	} else
		showText(TX_SPEAKER_SPOCK, TX_VEN6_011);
}

void Room::veng6SpockReachedJunctionBox() {
	loadActorAnimC(OBJECT_SPOCK, "suseln", -1, -1, &Room::veng6SpockAdjustedJunctionBox);
}

void Room::veng6SpockAdjustedJunctionBox() {
	showText(TX_SPEAKER_SPOCK, TX_VEN6_019);
	loadActorAnim(OBJECT_IMPULSE_ENGINES, "s7r6i1", IMPULSE_ENGINE_X, IMPULSE_ENGINE_Y);
	_awayMission->veng.impulseEnginesOn = true;
	stopAllVocSounds();
	playVoc("VEN6LON");
}

void Room::veng6UseHypoOnOilCanister() {
	if (_awayMission->veng.oilInHypo)
		showDescription(TX_VEN6N006);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x3c, 0x9e, &Room::veng6ReachedOilCanister);
	}
}

void Room::veng6ReachedOilCanister() {
	loadActorAnimC(OBJECT_KIRK, "kusehw", -1, -1, &Room::veng6FilledHypoWithOil);
}

void Room::veng6FilledHypoWithOil() {
	_awayMission->disableInput = false;
	showDescription(TX_VEN6N007);
	_awayMission->veng.oilInHypo = true;
}

void Room::veng6UseCabinet() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x36, 0x9f, &Room::veng6ReachedCabinet);
}

void Room::veng6ReachedCabinet() {
	loadActorAnimC(OBJECT_KIRK, "kusehw", -1, -1, &Room::veng6OpenedOrClosedCabinet);
}

void Room::veng6OpenedOrClosedCabinet() {
	_awayMission->disableInput = false;

	if (_awayMission->veng.engineeringCabinetOpen) {
		loadActorAnim(OBJECT_CABINET, "s7r6p2", CABINET_X, CABINET_Y);
		playSoundEffectIndex(kSfxDoor);
		_awayMission->veng.engineeringCabinetOpen = false;
	} else {
		loadActorAnim(OBJECT_CABINET, "s7r6p1", CABINET_X, CABINET_Y);
		playSoundEffectIndex(kSfxDoor);
		_awayMission->veng.engineeringCabinetOpen = true;
		if (!_awayMission->veng.tookEngineeringJournal)
			showDescription(TX_VEN6N005);
	}
}

void Room::veng6UseSTricorderOnImpulseEngines() {
	if (_awayMission->veng.impulseEnginesOn)
		spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN6_010);
	else {
		spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN6_020);
		showText(TX_SPEAKER_KIRK,  TX_VEN6_001);
		showText(TX_SPEAKER_SPOCK, TX_VEN6_022);
	}
}

void Room::veng6UseSTricorderOnRightConsole() {
	if (_awayMission->veng.impulseEnginesOn)
		spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN6_013);
	else
		spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN6_007);
}

void Room::veng6UseSTricorderOnJunctionBox() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN6_015);
}

void Room::veng6UsePowerPackOnLeftConsole() {
	showText(TX_SPEAKER_SPOCK, TX_VEN6_006);
}

void Room::veng6GetJunctionBox() {
	showDescription(TX_MUD1N014);
}

void Room::veng6GetOilCanister() {
	showDescription(TX_VEN6N008);
}

void Room::veng6GetEngineeringJournal() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x36, 0x9f, &Room::veng6ReachedEngineeringJournal);
}

void Room::veng6ReachedEngineeringJournal() {
	loadActorAnimC(OBJECT_KIRK, "kusemw", -1, -1, &Room::veng6TookEngineeringJournal);
}

void Room::veng6TookEngineeringJournal() {
	loadActorStandAnim(OBJECT_JOURNAL);
	_awayMission->disableInput = false;
	showDescription(TX_VEN6N024);
	_awayMission->veng.tookEngineeringJournal = true;
	giveItem(OBJECT_ITECH);
}

}
