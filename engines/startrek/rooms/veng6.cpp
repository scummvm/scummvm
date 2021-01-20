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
	{ {ACTION_TICK, 1, 0, 0}, &Room::veng6Tick1 },

	{ {ACTION_WALK, OBJECT_DOOR,  0, 0}, &Room::veng6WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::veng6WalkToDoor },
	{ {ACTION_DONE_WALK, 2,       0, 0}, &Room::veng6ReachedDoor },
	{ {ACTION_DONE_ANIM, 1,       0, 0}, &Room::veng6DoorOpened },

	{ {ACTION_LOOK, HOTSPOT_LEFT_PORT,       0, 0}, &Room::veng6LookAtLeftPort },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_PORT,      0, 0}, &Room::veng6LookAtRightPort },
	{ {ACTION_LOOK, OBJECT_DOOR,             0, 0}, &Room::veng6LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,            0, 0}, &Room::veng6LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_OIL_CANISTER,    0, 0}, &Room::veng6LookAtOilCanister },
	{ {ACTION_LOOK, OBJECT_CABINET,          0, 0}, &Room::veng6LookAtCabinet },
	{ {ACTION_LOOK, HOTSPOT_CABINET,         0, 0}, &Room::veng6LookAtCabinet },
	{ {ACTION_LOOK, OBJECT_CABLE_2,          0, 0}, &Room::veng6LookAtCable },
	{ {ACTION_LOOK, OBJECT_CABLE_1,          0, 0}, &Room::veng6LookAtCable },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_CONSOLE,   0, 0}, &Room::veng6LookAtRightConsole },
	{ {ACTION_LOOK, OBJECT_IMPULSE_ENGINES,  0, 0}, &Room::veng6LookAtImpulseEngines },
	{ {ACTION_LOOK, HOTSPOT_IMPULSE_ENGINES, 0, 0}, &Room::veng6LookAtImpulseEngines },
	{ {ACTION_LOOK, HOTSPOT_LEFT_CONSOLE,    0, 0}, &Room::veng6LookAtLeftConsole },
	{ {ACTION_LOOK, OBJECT_JUNCTION_BOX,     0, 0}, &Room::veng6LookAtJunctionBox },
	{ {ACTION_LOOK, OBJECT_JOURNAL,          0, 0}, &Room::veng6LookAtJournal },
	{ {ACTION_LOOK, OBJECT_KIRK,             0, 0}, &Room::veng6LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,            0, 0}, &Room::veng6LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,            0, 0}, &Room::veng6LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,         0, 0}, &Room::veng6LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_DEAD_GUY,         0, 0}, &Room::veng6LookAtDeadGuy },
	{ {ACTION_LOOK, 0xff,                    0, 0}, &Room::veng6LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,             0, 0}, &Room::veng6TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,            0, 0}, &Room::veng6TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,            0, 0}, &Room::veng6TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,         0, 0}, &Room::veng6TalkToRedshirt },

	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_LEFT_PORT,    0}, &Room::veng6UseCable1OnLeftPort },
	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_LEFT_CONSOLE, 0}, &Room::veng6UseCable1OnLeftPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_LEFT_PORT,    0}, &Room::veng6UseCable2OnLeftPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_LEFT_CONSOLE, 0}, &Room::veng6UseCable2OnLeftPort },
	{ {ACTION_DONE_WALK, 3,                           0, 0}, &Room::veng6KirkOrSpockReachedLeftPort },
	{ {ACTION_DONE_WALK, 4,                           0, 0}, &Room::veng6KirkOrSpockReachedLeftPort },
	{ {ACTION_DONE_ANIM, 5,                           0, 0}, &Room::veng6AttachedLeftCable },

	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_RIGHT_PORT,    0}, &Room::veng6UseCable1OnRightPort },
	{ {ACTION_USE, OBJECT_ICABLE1, HOTSPOT_RIGHT_CONSOLE, 0}, &Room::veng6UseCable1OnRightPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_RIGHT_PORT,    0}, &Room::veng6UseCable2OnRightPort },
	{ {ACTION_USE, OBJECT_ICABLE2, HOTSPOT_RIGHT_CONSOLE, 0}, &Room::veng6UseCable2OnRightPort },
	{ {ACTION_DONE_WALK, 6,                           0, 0}, &Room::veng6KirkOrSpockReachedRightPort },
	{ {ACTION_DONE_WALK, 7,                           0, 0}, &Room::veng6KirkOrSpockReachedRightPort },
	{ {ACTION_DONE_ANIM, 8,                           0, 0}, &Room::veng6AttachedRightCable },

	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_JUNCTION_BOX, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_JUNCTION_BOX, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_JUNCTION_BOX, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_JUNCTION_BOX, 0}, &Room::veng6UseCrewmanOnJunctionBox },
	{ {ACTION_DONE_WALK, 9,                           0, 0}, &Room::veng6SpockReachedJunctionBox },
	{ {ACTION_DONE_ANIM, 10,                          0, 0}, &Room::veng6SpockAdjustedJunctionBox },

	{ {ACTION_USE, OBJECT_IHYPO, HOTSPOT_OIL_CANISTER, 0}, &Room::veng6UseHypoOnOilCanister },
	{ {ACTION_DONE_WALK, 13,                        0, 0}, &Room::veng6ReachedOilCanister },
	{ {ACTION_DONE_ANIM, 14,                        0, 0}, &Room::veng6FilledHypoWithOil },

	{ {ACTION_USE, OBJECT_KIRK, OBJECT_CABINET, 0}, &Room::veng6UseCabinet },
	{ {ACTION_DONE_WALK, 15,                 0, 0}, &Room::veng6ReachedCabinet },
	{ {ACTION_DONE_ANIM, 16,                 0, 0}, &Room::veng6OpenedOrClosedCabinet },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_IMPULSE_ENGINES, 0}, &Room::veng6UseSTricorderOnImpulseEngines },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_CONSOLE,   0}, &Room::veng6UseSTricorderOnRightConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_JUNCTION_BOX,     0}, &Room::veng6UseSTricorderOnJunctionBox },
	{ {ACTION_USE, OBJECT_IFUSION,  HOTSPOT_LEFT_CONSOLE,    0}, &Room::veng6UsePowerPackOnLeftConsole },
	{ {ACTION_GET, OBJECT_JUNCTION_BOX,                   0, 0}, &Room::veng6GetJunctionBox },
	{ {ACTION_GET, HOTSPOT_OIL_CANISTER,                  0, 0}, &Room::veng6GetOilCanister },
	{ {ACTION_GET, OBJECT_JOURNAL,                        0, 0}, &Room::veng6GetEngineeringJournal },
	{ {ACTION_DONE_WALK, 11,                              0, 0}, &Room::veng6ReachedEngineeringJournal },
	{ {ACTION_DONE_ANIM, 12,                              0, 0}, &Room::veng6TookEngineeringJournal },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },

	// ENHANCEMENT
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IMPULSE_ENGINES, 0}, &Room::veng6UseSTricorderOnImpulseEngines },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
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
	showDescription(18, true);
}

void Room::veng6LookAtRightPort() {
	showDescription(16, true);
}

void Room::veng6LookAtDoor() {
	showDescription(TX_VEN5N007);
}

void Room::veng6LookAtOilCanister() {
	showDescription(11, true);
}

void Room::veng6LookAtCabinet() {
	showDescription(15, true);
}

void Room::veng6LookAtCable() {
	showDescription(13, true);
}

void Room::veng6LookAtRightConsole() {
	showDescription(17, true);
}

void Room::veng6LookAtImpulseEngines() {
	showDescription(10, true);
}

void Room::veng6LookAtLeftConsole() {
	showDescription(9, true);
}

void Room::veng6LookAtJunctionBox() {
	showDescription(12, true);
}

void Room::veng6LookAtJournal() {
	showDescription(25, true);
	showText(TX_SPEAKER_SPOCK, 21, true);
}

void Room::veng6LookAtKirk() {
	showDescription(1, true);
}

void Room::veng6LookAtSpock() {
	showDescription(3, true);
}

void Room::veng6LookAtMccoy() {
	showDescription(2, true);
}

void Room::veng6LookAtRedshirt() {
	showDescription(0, true);
}

void Room::veng6LookAtDeadGuy() {
	showDescription(TX_VEN0N016);
}

void Room::veng6LookAnywhere() {
	showDescription(19, true);
}

void Room::veng6TalkToKirk() {
	showText(TX_SPEAKER_KIRK,   5, true);
	showText(TX_SPEAKER_MCCOY,  9, true);
}

void Room::veng6TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 14, true);
}

void Room::veng6TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY,  8, true);
	showText(TX_SPEAKER_SPOCK, 18, true);
}

void Room::veng6TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, 23, true);
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
	showDescription(20, true);
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
	showDescription(22, true);
	_awayMission->veng.junctionCablesConnected |= 2;
	loseItem(_roomVar.veng.cableInUse);
	_awayMission->disableInput = false;
}

void Room::veng6UseCrewmanOnJunctionBox() {
	if (_awayMission->veng.impulseEnginesOn)
		showText(TX_SPEAKER_SPOCK, 12, true);
	else if ((_awayMission->veng.junctionCablesConnected & 3) == 3) {
		showText(TX_SPEAKER_COMPUTER, TX_COMPA185);
		showText(TX_SPEAKER_KIRK,  3, true);
		if (!_awayMission->veng.readEngineeringJournal)
			showText(TX_SPEAKER_SPOCK, 17, true);
		else {
			showText(TX_SPEAKER_SPOCK, 16, true);
			showText(TX_SPEAKER_KIRK,   2, true);
			walkCrewmanC(OBJECT_SPOCK, 0x99, 0xb4, &Room::veng6SpockReachedJunctionBox);
		}
	} else
		showText(TX_SPEAKER_SPOCK, 11, true);
}

void Room::veng6SpockReachedJunctionBox() {
	loadActorAnimC(OBJECT_SPOCK, "suseln", -1, -1, &Room::veng6SpockAdjustedJunctionBox);
}

void Room::veng6SpockAdjustedJunctionBox() {
	showText(TX_SPEAKER_SPOCK, 19, true);
	loadActorAnim(OBJECT_IMPULSE_ENGINES, "s7r6i1", IMPULSE_ENGINE_X, IMPULSE_ENGINE_Y);
	_awayMission->veng.impulseEnginesOn = true;
	stopAllVocSounds();
	playVoc("VEN6LON");
}

void Room::veng6UseHypoOnOilCanister() {
	if (_awayMission->veng.oilInHypo)
		showDescription(6, true);
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
	showDescription(7, true);
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
			showDescription(5, true);
	}
}

void Room::veng6UseSTricorderOnImpulseEngines() {
	if (_awayMission->veng.impulseEnginesOn)
		spockScan(DIR_S, 10, false, true);
	else {
		spockScan(DIR_S, 20, false, true);
		showText(TX_SPEAKER_KIRK,   1, true);
		showText(TX_SPEAKER_SPOCK, 22, true);
	}
}

void Room::veng6UseSTricorderOnRightConsole() {
	if (_awayMission->veng.impulseEnginesOn)
		spockScan(DIR_S, 13, false, true);
	else
		spockScan(DIR_S,  7, false, true);
}

void Room::veng6UseSTricorderOnJunctionBox() {
	spockScan(DIR_S, 15, false, true);
}

void Room::veng6UsePowerPackOnLeftConsole() {
	showText(TX_SPEAKER_SPOCK,  6, true);
}

void Room::veng6GetJunctionBox() {
	showDescription(TX_MUD1N014);
}

void Room::veng6GetOilCanister() {
	showDescription(8, true);
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
	showDescription(24, true);
	_awayMission->veng.tookEngineeringJournal = true;
	giveItem(OBJECT_ITECH);
}

}
