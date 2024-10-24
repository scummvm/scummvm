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

#define OBJECT_DEAD_GUY 8 // Unused
#define OBJECT_POWER_PACK 9
#define OBJECT_CABLES 10
#define OBJECT_DOOR 11
#define OBJECT_CONSOLE_LEFT 12
#define OBJECT_SPARKS_1 13
#define OBJECT_SPARKS_2 14
#define OBJECT_CONTROLS 15
#define OBJECT_SLIDER 16

#define HOTSPOT_SLIDER 0x20
#define HOTSPOT_CONTROLS 0x21
#define HOTSPOT_POWER_CIRCUIT 0x22
#define HOTSPOT_DOOR 0x23
#define HOTSPOT_TRANSPORTER 0x24

namespace StarTrek {

extern const RoomAction veng8ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::veng8Tick1 },

	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::veng8WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::veng8WalkToDoor },
	{ {ACTION_DONE_WALK, 8,       0, 0, 0}, &Room::veng8ReachedDoor },
	{ {ACTION_DONE_ANIM, 7,       0, 0, 0}, &Room::veng8DoorOpened },
	{ {ACTION_TIMER_EXPIRED, 0,   0, 0, 0}, &Room::veng8Timer0Expired },
	{ {ACTION_DONE_ANIM, 11,      0, 0, 0}, &Room::veng8SparkAnim1Done },
	{ {ACTION_TIMER_EXPIRED, 1,   0, 0, 0}, &Room::veng8Timer1Expired },
	{ {ACTION_DONE_ANIM, 12,      0, 0, 0}, &Room::veng8SparkAnim2Done },

	{ {ACTION_LOOK, OBJECT_POWER_PACK,     0, 0, 0}, &Room::veng8LookAtPowerPack },
	{ {ACTION_LOOK, OBJECT_DOOR,           0, 0, 0}, &Room::veng8LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,          0, 0, 0}, &Room::veng8LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_POWER_CIRCUIT, 0, 0, 0}, &Room::veng8LookAtPowerCircuits },
	{ {ACTION_LOOK, OBJECT_CONTROLS,       0, 0, 0}, &Room::veng8LookAtControls },
	{ {ACTION_LOOK, HOTSPOT_CONTROLS,      0, 0, 0}, &Room::veng8LookAtControls },
	{ {ACTION_LOOK, OBJECT_DEAD_GUY,       0, 0, 0}, &Room::veng8LookAtDeadGuy },
	{ {ACTION_LOOK, OBJECT_SLIDER,         0, 0, 0}, &Room::veng8LookAtSlider },
	{ {ACTION_LOOK, HOTSPOT_SLIDER,        0, 0, 0}, &Room::veng8LookAtSlider },
	{ {ACTION_LOOK, OBJECT_KIRK,           0, 0, 0}, &Room::veng8LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,          0, 0, 0}, &Room::veng8LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,          0, 0, 0}, &Room::veng8LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,       0, 0, 0}, &Room::veng8LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_TRANSPORTER,   0, 0, 0}, &Room::veng8LookAtTransporter },
	{ {ACTION_LOOK, 0xff,                  0, 0, 0}, &Room::veng8LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,           0, 0, 0}, &Room::veng8TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,          0, 0, 0}, &Room::veng8TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,          0, 0, 0}, &Room::veng8TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,       0, 0, 0}, &Room::veng8TalkToRedshirt },

	{ {ACTION_USE, OBJECT_KIRK,  OBJECT_CONTROLS,  0, 0}, &Room::veng8UseKirkOnControls },
	{ {ACTION_USE, OBJECT_KIRK,  HOTSPOT_CONTROLS, 0, 0}, &Room::veng8UseKirkOnControls },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_CONTROLS,  0, 0}, &Room::veng8UseSpockOnControls },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_CONTROLS, 0, 0}, &Room::veng8UseSpockOnControls },
	{ {ACTION_DONE_WALK, 13,                    0, 0, 0}, &Room::veng8SpockReachedControls },
	{ {ACTION_DONE_ANIM, 14,                    0, 0, 0}, &Room::veng8SpockUsedControls },

	{ {ACTION_USE, OBJECT_IFUSION, HOTSPOT_POWER_CIRCUIT, 0, 0}, &Room::veng8UsePowerPackOnPowerCircuit },
	{ {ACTION_DONE_WALK, 9,                            0, 0, 0}, &Room::veng8SpockReachedPowerCircuit },
	{ {ACTION_DONE_ANIM, 10,                           0, 0, 0}, &Room::veng8SpockConnectedPowerPack },
	{ {ACTION_DONE_ANIM, 15,                           0, 0, 0}, &Room::veng8TransporterPoweredUp },

	{ {ACTION_USE, OBJECT_KIRK,  OBJECT_SLIDER,  0, 0}, &Room::veng8UseKirkOnSlider },
	{ {ACTION_USE, OBJECT_KIRK,  HOTSPOT_SLIDER, 0, 0}, &Room::veng8UseKirkOnSlider },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SLIDER,  0, 0}, &Room::veng8UseSpockOnSlider },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_SLIDER, 0, 0}, &Room::veng8UseSpockOnSlider },
	{ {ACTION_DONE_WALK, 16,                  0, 0, 0}, &Room::veng8SpockReachedSlider },
	{ {ACTION_DONE_WALK, 1,                   0, 0, 0}, &Room::veng8RedshirtReachedTransporter },
	{ {ACTION_DONE_WALK, 4,                   0, 0, 0}, &Room::veng8RedshirtReachedTransporter },
	{ {ACTION_DONE_ANIM, 5,                   0, 0, 0}, &Room::veng8SpockPulledSliderToBeamOut },
	{ {ACTION_DONE_ANIM, 2,                   0, 0, 0}, &Room::veng8RedshirtBeamedOut },
	{ {ACTION_DONE_ANIM, 6,                   0, 0, 0}, &Room::veng8SpockPulledSliderToBeamIn },
	{ {ACTION_DONE_ANIM, 3,                   0, 0, 0}, &Room::veng8RedshirtBeamedIn },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_CONTROLS,  0, 0}, &Room::veng8UseSTricorderOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SLIDER,    0, 0}, &Room::veng8UseSTricorderOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_CONTROLS, 0, 0}, &Room::veng8UseSTricorderOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_SLIDER,   0, 0}, &Room::veng8UseSTricorderOnConsole },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum veng8TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_KIJE,
	TX_BRIDU146, TX_MUD4_018, TX_VEN2U093, TX_VEN2_028,
	TX_VEN2_066, TX_VEN2_098, TX_VEN4N010, TX_VEN5N002, TX_VEN5N004,
	TX_VEN6N007, TX_VEN8N000, TX_VEN8N001, TX_VEN8N003, TX_VEN8N004,
	TX_VEN8N005, TX_VEN8N006, TX_VEN8N007, TX_VEN8N008, TX_VEN8N009,
	TX_VEN8N010, TX_VEN8_001, TX_VEN8_002, TX_VEN8_003, TX_VEN8_004,
	TX_VEN8_005, TX_VEN8_006, TX_VEN8_007, TX_VEN8_008, TX_VEN8_009,
	TX_VEN8_010, TX_VEN8_011, TX_VEN8_012, TX_VEN8_013,
	TX_VEN8_014, TX_VEN8_015, TX_VEN8_016, TX_VEN8_018, TX_VEN8_020,
	TX_VEN8_021, TX_VEN8_023, TX_VEN8_024, TX_VEN8_025, TX_VEN8_026,
	TX_VEN8_027, TX_VEN8_028, TX_VEN8_029, TX_VEN8_030, TX_VEN8_031,
	TX_VEN8_032, TX_VEN8_033, TX_VEN8_034, TX_VEN8_035, TX_VEN8_036,
	TX_VEN8_037, TX_VEN8_038, TX_VEN8_039, TX_VEN8_040, TX_VEN8_041,
	TX_VEN8_042, TX_VENA_F32, TX_VENA_F34, TX_VENA_F40
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets veng8TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 7744, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 7755, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 7765, 0, 0, 0 },
	{ TX_SPEAKER_KIJE, 7779, 0, 0, 0 },
	{ TX_BRIDU146, 7146, 0, 0, 0 },
	{ TX_MUD4_018, 7077, 0, 0, 0 },
	//{ TX_MUD4_018, 7210, 0, 0, 0 },	// Skip duplicate text
	{ TX_VEN2U093, 7278, 0, 0, 0 },
	{ TX_VEN2_028, 7385, 0, 0, 0 },
	{ TX_VEN2_066, 7463, 0, 0, 0 },
	{ TX_VEN2_098, 6273, 0, 0, 0 },
	{ TX_VEN4N010, 6927, 0, 0, 0 },
	{ TX_VEN5N002, 1314, 0, 0, 0 },
	{ TX_VEN5N004, 1369, 0, 0, 0 },
	{ TX_VEN6N007, 6992, 0, 0, 0 },
	{ TX_VEN8N000, 1449, 0, 0, 0 },
	{ TX_VEN8N001, 1254, 0, 0, 0 },
	{ TX_VEN8N003, 1079, 0, 0, 0 },
	{ TX_VEN8N004, 695, 0, 0, 0 },
	{ TX_VEN8N005, 1621, 0, 0, 0 },
	{ TX_VEN8N006, 892, 0, 0, 0 },
	{ TX_VEN8N007, 990, 0, 0, 0 },
	{ TX_VEN8N008, 1173, 0, 0, 0 },
	{ TX_VEN8N009, 805, 0, 0, 0 },
	{ TX_VEN8N010, 1519, 0, 0, 0 },
	{ TX_VEN8_001, 2288, 0, 0, 0 },
	{ TX_VEN8_002, 1721, 0, 0, 0 },
	{ TX_VEN8_003, 2892, 0, 0, 0 },
	{ TX_VEN8_004, 4214, 0, 0, 0 },
	{ TX_VEN8_005, 1809, 0, 0, 0 },
	{ TX_VEN8_006, 1876, 0, 0, 0 },
	{ TX_VEN8_007, 5851, 0, 0, 0 },
	{ TX_VEN8_008, 5667, 0, 0, 0 },
	{ TX_VEN8_009, 5092, 0, 0, 0 },
	//{ TX_VEN8_009, 3845, 0, 0, 0 },	// Skip duplicate text
	{ TX_VEN8_010, 4795, 0, 0, 0 },
	{ TX_VEN8_011, 3584, 0, 0, 0 },
	{ TX_VEN8_012, 5371, 0, 0, 0 },
	{ TX_VEN8_013, 4576, 0, 0, 0 },
	{ TX_VEN8_014, 3169, 0, 0, 0 },
	{ TX_VEN8_015, 4458, 0, 0, 0 },
	{ TX_VEN8_016, 7815, 0, 0, 0 },
	{ TX_VEN8_018, 2443, 0, 0, 0 },
	{ TX_VEN8_020, 2215, 0, 0, 0 },
	{ TX_VEN8_021, 2391, 0, 0, 0 },
	{ TX_VEN8_023, 2628, 0, 0, 0 },
	{ TX_VEN8_024, 7890, 0, 0, 0 },
	{ TX_VEN8_025, 6123, 0, 0, 0 },
	{ TX_VEN8_026, 3070, 0, 0, 0 },
	{ TX_VEN8_027, 4359, 0, 0, 0 },
	{ TX_VEN8_028, 2130, 0, 0, 0 },
	{ TX_VEN8_029, 2050, 0, 0, 0 },
	{ TX_VEN8_030, 3263, 0, 0, 0 },
	{ TX_VEN8_031, 3429, 0, 0, 0 },
	{ TX_VEN8_032, 1986, 0, 0, 0 },
	{ TX_VEN8_033, 3717, 0, 0, 0 },
	{ TX_VEN8_034, 2576, 0, 0, 0 },
	{ TX_VEN8_035, 4984, 0, 0, 0 },
	{ TX_VEN8_036, 2682, 0, 0, 0 },
	{ TX_VEN8_037, 5258, 0, 0, 0 },
	{ TX_VEN8_038, 5770, 0, 0, 0 },
	{ TX_VEN8_039, 5455, 0, 0, 0 },
	{ TX_VEN8_040, 2812, 0, 0, 0 },
	{ TX_VEN8_041, 2757, 0, 0, 0 },
	{ TX_VEN8_042, 4668, 0, 0, 0 },
	{ TX_VENA_F32, 6503, 0, 0, 0 },
	{ TX_VENA_F34, 6773, 0, 0, 0 },
	{ TX_VENA_F40, 6368, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText veng8Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::veng8Tick1() {
	playVoc("VEN8LOOP");

	if (_awayMission->veng.powerPackPluggedIntoTransporter) {
		loadActorAnim(OBJECT_POWER_PACK, "s7r8pp", 0x10d, 0xa2);
		loadActorAnim(OBJECT_CABLES, "s7r8pl", 0x101, 0x97);
		loadActorAnim(OBJECT_CONSOLE_LEFT, "s7r8t2", 0x3c, 0x9f);
		loadActorAnim(OBJECT_CONTROLS, "s7r8q1", 0x35, 0xa0);
		loadActorAnim(OBJECT_SLIDER, "s7r8q2", 0x44, 0xa0);
	}

	loadActorAnim(OBJECT_DOOR, "s7r8d1c", 0x12c, 0xac);
	_awayMission->timers[0] = 47;
	_awayMission->timers[1] = 14;
}

void Room::veng8WalkToDoor() {
	walkCrewmanC(OBJECT_KIRK, 0x117, 0xb6, &Room::veng8ReachedDoor);
}

void Room::veng8ReachedDoor() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	loadActorAnimC(OBJECT_DOOR, "s7r8d1", 0x12c, 0xac, &Room::veng8DoorOpened);
}

void Room::veng8DoorOpened() {
	walkCrewman(OBJECT_KIRK, 0x134, 0xb6);
}

void Room::veng8Timer0Expired() { // Create sparks from a console in the ceiling
	loadActorAnimC(OBJECT_SPARKS_1, "s7r8s1", 0, 0, &Room::veng8SparkAnim1Done);
}

void Room::veng8SparkAnim1Done() { // Create sparks every 128 ticks
	_awayMission->timers[0] = 128;
}

void Room::veng8Timer1Expired() { // Should create sparks, but they're not visible anywhere?
	loadActorAnimC(OBJECT_SPARKS_2, "s7r8s2", 0, 0, &Room::veng8SparkAnim2Done);
}

void Room::veng8SparkAnim2Done() {
	_awayMission->timers[1] = 32;
}

void Room::veng8LookAtPowerPack() {
	showDescription(TX_VEN8N004);
}

void Room::veng8LookAtDoor() {
	showDescription(TX_VEN8N009);
}

void Room::veng8LookAtPowerCircuits() {
	showDescription(TX_VEN8N006);
}

void Room::veng8LookAtControls() {
	showDescription(TX_VEN8N007);
}

void Room::veng8LookAtDeadGuy() {
	showDescription(TX_VEN8N003); // Unused, since the dead guy object isn't drawn
}

void Room::veng8LookAtSlider() {
	showDescription(TX_VEN8N008);
}

void Room::veng8LookAtKirk() {
	showDescription(TX_VEN8N001);
}

void Room::veng8LookAtSpock() {
	// NOTE: Potential alternative is VEN8N011
	showDescription(TX_VEN5N002);
}

void Room::veng8LookAtMccoy() {
	// NOTE: Potential alternative is VEN8N002 (though it doesn't fit very well with this
	// room, so perhaps the circumstances of this room changed in development)
	showDescription(TX_VEN5N004);
}

void Room::veng8LookAtRedshirt() {
	showDescription(TX_VEN8N000);
}

void Room::veng8LookAtTransporter() {
	showDescription(TX_VEN8N010);
}

void Room::veng8LookAnywhere() {
	showDescription(TX_VEN8N005);
}

void Room::veng8TalkToKirk() {
	if (_awayMission->veng.torpedoLoaded)
		showText(TX_SPEAKER_KIRK, TX_VEN8_002);
	else if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_KIRK, TX_VEN8_005);
	else
		showText(TX_SPEAKER_KIRK, TX_VEN8_006);
}

void Room::veng8TalkToSpock() {
	if (_awayMission->veng.torpedoLoaded)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_032);
	else if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_029);
	else
		showText(TX_SPEAKER_SPOCK, TX_VEN8_028);
}

void Room::veng8TalkToMccoy() {
	if (_awayMission->veng.torpedoLoaded) {
		showText(TX_SPEAKER_MCCOY, TX_VEN8_020);
		showText(TX_SPEAKER_KIRK,  TX_VEN8_001);
	} else if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_MCCOY, TX_VEN8_021);
	else
		showText(TX_SPEAKER_MCCOY, TX_VEN8_018);
}

void Room::veng8TalkToRedshirt() {
	if (_awayMission->veng.torpedoLoaded) {
		showText(TX_SPEAKER_KIJE,  TX_VEN8_034);
		showText(TX_SPEAKER_MCCOY, TX_VEN8_023);
		showText(TX_SPEAKER_KIJE,  TX_VEN8_036);
	} else if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_KIJE, TX_VEN8_041);
	else
		showText(TX_SPEAKER_KIJE, TX_VEN8_040);
}

void Room::veng8UseKirkOnControls() {
	showText(TX_SPEAKER_KIRK, TX_VEN8_003);
	veng8UseSpockOnControls();
}

void Room::veng8UseSpockOnControls() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_SPOCK, 0x37, 0xc4, &Room::veng8SpockReachedControls);
}

void Room::veng8SpockReachedControls() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng8SpockUsedControls);
}

void Room::veng8SpockUsedControls() {
	_awayMission->disableInput = false;

	if (!_awayMission->veng.powerPackPluggedIntoTransporter && !_awayMission->veng.impulseEnginesOn)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_026);
	else if (!_awayMission->veng.powerPackPluggedIntoTransporter && _awayMission->veng.poweredSystem != 3)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_014);
	else if (!_awayMission->veng.examinedTorpedoControl)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_030);
	else {
		showText(TX_SPEAKER_SPOCK, TX_VEN8_031);
		_awayMission->veng.setTransporterCoordinates = true;
	}
}

void Room::veng8UsePowerPackOnPowerCircuit() {
	if (!_awayMission->veng.powerPackPluggedIntoTransporter) {
		_awayMission->disableInput = true;
		showText(TX_SPEAKER_KIRK,  TX_VEN8_011);
		showText(TX_SPEAKER_SPOCK, TX_VEN8_033);
		showText(TX_SPEAKER_KIRK,  TX_VEN8_009);
		walkCrewmanC(OBJECT_SPOCK, 0xfa, 0xa4, &Room::veng8SpockReachedPowerCircuit);
	}
}

void Room::veng8SpockReachedPowerCircuit() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng8SpockConnectedPowerPack);
}

void Room::veng8SpockConnectedPowerPack() {
	playVoc("MTHTRNSP");

	loadActorAnim(OBJECT_POWER_PACK, "s7r8pp", 0x10d, 0xa2);
	loadActorAnim(OBJECT_CABLES, "s7r8pl", 0x101, 0x97);
	loadActorAnimC(OBJECT_CONSOLE_LEFT, "s7r8t1", 0x3c, 0x9f, &Room::veng8TransporterPoweredUp);

	walkCrewman(OBJECT_SPOCK, 0xd4, 0xb2);
	_awayMission->veng.powerPackPluggedIntoTransporter = true;
	loseItem(OBJECT_IFUSION);
	_awayMission->disableInput = false;
}

void Room::veng8TransporterPoweredUp() {
	loadActorAnim(OBJECT_CONTROLS, "s7r8q1", 0x35, 0xa0);
	loadActorAnim(OBJECT_SLIDER, "s7r8q2", 0x44, 0xa0);
}

void Room::veng8UseKirkOnSlider() {
	showText(TX_SPEAKER_KIRK, TX_VEN8_004);
	veng8UseSpockOnSlider();
}

void Room::veng8UseSpockOnSlider() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_SPOCK, 0x37, 0xc4, &Room::veng8SpockReachedSlider);
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
}

void Room::veng8SpockReachedSlider() {
	_awayMission->disableInput = false;

	if (!_awayMission->veng.powerPackPluggedIntoTransporter && !_awayMission->veng.impulseEnginesOn)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_027);
	else if (!_awayMission->veng.powerPackPluggedIntoTransporter && _awayMission->veng.poweredSystem != 3)
		showText(TX_SPEAKER_SPOCK, TX_VEN8_015);
	else if (!_awayMission->veng.setTransporterCoordinates) {
		walkCrewman(OBJECT_SPOCK, 0x98, 0xb6);
		showText(TX_SPEAKER_SPOCK, TX_VEN8_013);
	} else if (_awayMission->veng.torpedoLoaded)
		showText(TX_SPEAKER_KIJE, TX_VEN8_042);
	else {
		_awayMission->disableInput = true;
		showText(TX_SPEAKER_KIRK, TX_VEN8_010);
		showText(TX_SPEAKER_KIJE, TX_VEN8_035);
		walkCrewmanC(OBJECT_REDSHIRT, 0xc8, 0x7f, &Room::veng8RedshirtReachedTransporter);
		_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
	}
}

void Room::veng8RedshirtReachedTransporter() {
	showText(TX_SPEAKER_KIRK, TX_VEN8_009);
	loadActorAnimC(OBJECT_SPOCK, "susehn", -1, -1, &Room::veng8SpockPulledSliderToBeamOut);
	playSoundEffectIndex(kSfxTransporterEnergize);
}

void Room::veng8SpockPulledSliderToBeamOut() {
	loadActorAnimC(OBJECT_REDSHIRT, "rteled", -1, -1, &Room::veng8RedshirtBeamedOut);
	playSoundEffectIndex(kSfxTransporterDematerialize);
	_awayMission->redshirtDead = true;
}

void Room::veng8RedshirtBeamedOut() {
	loadActorStandAnim(OBJECT_REDSHIRT);
	showText(TX_SPEAKER_KIJE, TX_VEN8_037);
	showText(TX_SPEAKER_KIRK, TX_VEN8_012);
	showText(TX_SPEAKER_KIJE, TX_VEN8_039);
	showText(TX_SPEAKER_KIRK, TX_VEN8_008);
	showText(TX_SPEAKER_KIJE, TX_VEN8_038);
	showText(TX_SPEAKER_KIRK, TX_VEN8_007);
	loadActorAnimC(OBJECT_SPOCK, "susehn", -1, -1, &Room::veng8SpockPulledSliderToBeamIn);
	playSoundEffectIndex(kSfxTransporterEnergize);
}

void Room::veng8SpockPulledSliderToBeamIn() {
	_awayMission->redshirtDead = false;
	loadActorAnimC(OBJECT_REDSHIRT, "rtele", 0xc8, 0x81, &Room::veng8RedshirtBeamedIn);
	playSoundEffectIndex(kSfxTransporterMaterialize);
}

void Room::veng8RedshirtBeamedIn() {
	walkCrewman(OBJECT_REDSHIRT, 0xba, 0xb2);
	_awayMission->disableInput = false;
	_awayMission->veng.torpedoLoaded = true;
}

void Room::veng8UseSTricorderOnConsole() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN8_025);
}

}
