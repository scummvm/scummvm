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

#define OBJECT_DOOR1 8
#define OBJECT_DOOR2 9
#define OBJECT_DOOR3 10
#define OBJECT_FREEZER 11
#define OBJECT_CHAMBER 12
#define OBJECT_PHASERSHOT 13
#define OBJECT_DISH_IN_CHAMBER 14
#define OBJECT_BOTTLE 15

#define HOTSPOT_LADDER 0x20
#define HOTSPOT_DISTILLATOR 0x21
#define HOTSPOT_FREEZER 0x22
#define HOTSPOT_REPLICATOR 0x23
#define HOTSPOT_NOZZLE 0x24
#define HOTSPOT_LASER 0x25
#define HOTSPOT_DOOR3 0x26
#define HOTSPOT_DOOR1 0x27
#define HOTSPOT_DOOR2 0x28

namespace StarTrek {

extern const RoomAction love1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::love1Tick1 },

	{ {ACTION_WALK, 10, 0, 0, 0}, &Room::love1WalkToDoor3 },
	{ {ACTION_WALK, 0x26, 0, 0, 0}, &Room::love1WalkToDoor3 },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0, 0}, &Room::love1OpenDoor3 },
	{ {ACTION_FINISHED_WALKING, 15, 0, 0, 0}, &Room::love1ReachedDoor3 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::love1ReachedDoor3 },

	{ {ACTION_WALK, 8, 0, 0, 0}, &Room::love1WalkToDoor1 },
	{ {ACTION_WALK, 0x27, 0, 0, 0}, &Room::love1WalkToDoor1 },
	{ {ACTION_TOUCHED_HOTSPOT, 2, 0, 0, 0}, &Room::love1OpenDoor1 },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0, 0}, &Room::love1ReachedDoor1 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::love1ReachedDoor1 },

	{ {ACTION_WALK, 9, 0, 0, 0}, &Room::love1WalkToDoor2 },
	{ {ACTION_WALK, 0x28, 0, 0, 0}, &Room::love1WalkToDoor2 },
	{ {ACTION_TOUCHED_HOTSPOT, 3, 0, 0, 0}, &Room::love1OpenDoor2 },
	{ {ACTION_FINISHED_WALKING, 17, 0, 0, 0}, &Room::love1ReachedDoor2 },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::love1ReachedDoor2 },

	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::love1LookAtLaser },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::love1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::love1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::love1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love1LookAtRedshirt },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::love1LookAnywhere },
	{ {ACTION_LOOK, 15, 0, 0, 0}, &Room::love1LookAtNozzle },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::love1LookAtNozzle },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::love1LookAtLadder },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::love1LookAtDoor1Or2 },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::love1LookAtDoor1Or2 },
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::love1LookAtDoor3 },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::love1LookAtDistillator },
	{ {ACTION_LOOK, 14, 0, 0, 0}, &Room::love1LookAtChamber },
	{ {ACTION_LOOK, 12, 0, 0, 0}, &Room::love1LookAtChamber },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::love1LookAtReplicator },
	{ {ACTION_LOOK, 11, 0, 0, 0}, &Room::love1LookAtFreezer },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::love1LookAtFreezer },
	{ {ACTION_LOOK, OBJECT_IDISHES, 0, 0, 0}, &Room::love1LookAtDishes },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::love1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::love1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::love1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love1TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_IDISHES, 0, 0}, &Room::love1UseMTricorderOnDishes },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::love1UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::love1UseSTricorderOnReplicator },
	{ {ACTION_USE, OBJECT_ISTRICOR, 12, 0, 0}, &Room::love1UseSTricorderOnReplicator },
	{ {ACTION_USE, OBJECT_SPOCK, 0x25, 0, 0}, &Room::love1UseSTricorderOnLaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0, 0}, &Room::love1UseSTricorderOnLaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0, 0}, &Room::love1UseSTricorderOnFreezer },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::love1UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IDISHES, 0, 0}, &Room::love1UseSTricorderOnDishes },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::love1UseSTricorderOnDistillator },
	{ {ACTION_GET, 11, 0, 0, 0}, &Room::love1GetFreezer },
	{ {ACTION_GET, 0x22, 0, 0, 0}, &Room::love1GetFreezer },
	{ {ACTION_FINISHED_WALKING, 14, 0, 0, 0}, &Room::love1KirkReachedFreezer },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::love1KirkGotVirusCulture },
	{ {ACTION_GET, 14, 0, 0, 0}, &Room::love1GetFromChamber },
	{ {ACTION_GET, 12, 0, 0, 0}, &Room::love1GetFromChamber },
	{ {ACTION_GET, 0x23, 0, 0, 0}, &Room::love1GetFromChamber },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0, 0}, &Room::love1KirkReachedChamber },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0, 0}, &Room::love1KirkGotCureSample },
	{ {ACTION_GET, 15, 0, 0, 0}, &Room::love1GetFromNozzle },
	{ {ACTION_GET, 0x24, 0, 0, 0}, &Room::love1GetFromNozzle },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::love1KirkReachedNozzleToGet },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0, 0}, &Room::love1KirkGotBottleFromNozzle },
	{ {ACTION_USE, OBJECT_IN2O, 0x24, 0, 0}, &Room::love1UseN2OOnNozzle },
	{ {ACTION_USE, OBJECT_IH2O, 0x24, 0, 0}, &Room::love1UseH2OOnNozzle },
	{ {ACTION_USE, OBJECT_INH3, 0x24, 0, 0}, &Room::love1UseNH3OnNozzle },
	{ {ACTION_USE, OBJECT_IRLG, 0x24, 0, 0}, &Room::love1UseRLGOnNozzle },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::love1KirkReachedNozzleToPut },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0, 0}, &Room::love1KirkPutBottleInNozzle },
	{ {ACTION_USE, 0xff, 0x24, 0, 0}, &Room::love1UseAnthingOnNozzle },
	{ {ACTION_USE, OBJECT_SPOCK, 0x23, 0, 0}, &Room::love1UseSpockOnReplicator },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x23, 0, 0}, &Room::love1UseRedshirtOnReplicator },
	{ {ACTION_USE, OBJECT_MCCOY, 14, 0, 0}, &Room::love1UseMccoyOnReplicator },
	{ {ACTION_USE, OBJECT_MCCOY, 15, 0, 0}, &Room::love1UseMccoyOnReplicator },
	{ {ACTION_USE, OBJECT_MCCOY, 0x23, 0, 0}, &Room::love1UseMccoyOnReplicator },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::love1MccoyReachedReplicator },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0, 0}, &Room::love1MccoyUsedReplicator },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0, 0}, &Room::love1ChamberClosed },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0, 0}, &Room::love1ChamberOpened },
	{ {ACTION_USE, 0xff, 12, 0, 0}, &Room::love1UseAnythingOnChamber },
	{ {ACTION_USE, OBJECT_IDISHES, 12, 0, 0}, &Room::love1UseDishesOnChamber },
	{ {ACTION_USE, OBJECT_IDISHES, 0x23, 0, 0}, &Room::love1UseDishesOnChamber },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::love1KirkReachedChamberToPut },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::love1ChamberOpenedForDish },
	{ {ACTION_FINISHED_ANIMATION, 14, 0, 0, 0}, &Room::love1KirkPutDishInChamber },
	{ {ACTION_USE, OBJECT_IINSULAT, 0x21, 0, 0}, &Room::love1UseInsulationOnDistillator },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::love1KirkReachedDistillator },
	{ {ACTION_FINISHED_ANIMATION, 15, 0, 0, 0}, &Room::love1KirkGotPolyberylcarbonate },
	{ {ACTION_USE, OBJECT_KIRK, 0x22, 0, 0}, &Room::love1UseKirkOnFreezer },
	{ {ACTION_USE, OBJECT_KIRK, 11, 0, 0}, &Room::love1UseKirkOnFreezer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x22, 0, 0}, &Room::love1UseRedshirtOnFreezer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 11, 0, 0}, &Room::love1UseRedshirtOnFreezer },
	{ {ACTION_USE, OBJECT_SPOCK, 0x22, 0, 0}, &Room::love1UseSpockOnFreezer },
	{ {ACTION_USE, OBJECT_SPOCK, 11, 0, 0}, &Room::love1UseSpockOnFreezer },
	{ {ACTION_USE, OBJECT_MCCOY, 0x22, 0, 0}, &Room::love1UseMccoyOnFreezer },
	{ {ACTION_USE, OBJECT_MCCOY, 11, 0, 0}, &Room::love1UseMccoyOnFreezer },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0, 0}, &Room::love1CrewmanReachedFreezer },
	{ {ACTION_FINISHED_ANIMATION, 16, 0, 0, 0}, &Room::love1CrewmanOpenedOrClosedFreezer },
	{ {ACTION_USE, 0xff, 0x22, 0, 0}, &Room::love1UseAnythingOnFreezer },
	{ {ACTION_FINISHED_WALKING, 11, 0, 0, 0}, &Room::love1ReachedFreezerWithArbitraryItem },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0, 0}, &Room::love1FinishedUsingArbitraryItemOnFreezer },
	{ {ACTION_USE, 0xff, 0x23, 0, 0}, &Room::love1UseAnythingOnReplicator },
	{ {ACTION_FINISHED_WALKING, 12, 0, 0, 0}, &Room::love1ReachedReplicatorWithArbitraryItem },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0, 0}, &Room::love1FinishedUsingArbitraryItemOnReplicator },
	{ {ACTION_USE, 0xff, 0x21, 0, 0}, &Room::love1UseAnythingOnDistillator },
	{ {ACTION_FINISHED_WALKING, 13, 0, 0, 0}, &Room::love1ReachedDistillatorWithArbitraryItem },
	{ {ACTION_FINISHED_ANIMATION, 19, 0, 0, 0}, &Room::love1FinishedUsingArbitraryItemOnDistillator },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0, 0}, &Room::love1UseKirkOnLadder },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0, 0}, &Room::love1UseSpockOnLadder },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0, 0}, &Room::love1UseMccoyOnLadder },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0, 0}, &Room::love1UseRedshirtOnLadder },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::love1CrewmanReachedLadder },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::love1CrewmanDiedFromPhaser },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::love1TouchedHotspot0 },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum love1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_FERRIS,
	TX_LOV0_011, TX_LOV0_012, TX_LOV0_013, TX_LOV0_014, TX_LOV0_015,
	TX_LOV0_016, TX_LOV0_017, TX_LOV0_019, TX_LOV0_020, TX_LOV0_021,
	TX_LOV0_022, TX_LOV0_025, TX_LOV0_026, TX_LOV0_029, TX_LOV0_033,
	TX_LOV0_039, TX_LOV0_040, TX_LOV0_041, TX_LOV0_042, TX_LOV0_043,
	TX_LOV0_045, TX_LOV0_046, TX_LOV0_047, TX_LOV0_048, TX_LOV0_049,
	TX_LOV0_050, TX_LOV0_101, TX_LOV0_102, TX_LOV0_103, TX_LOV0_104,
	TX_LOV0_105, TX_LOV0_106, TX_LOV0_107, TX_LOV0_124, TX_LOV1_001,
	TX_LOV1_002, TX_LOV1_003, TX_LOV1_004, TX_LOV1_005, TX_LOV1_006,
	TX_LOV1_007, TX_LOV1_008, TX_LOV1_009, TX_LOV1_010, TX_LOV1_011,
	TX_LOV1_012, TX_LOV1_013, TX_LOV1_014, TX_LOV1_015, TX_LOV1_016,
	TX_LOV1_017, TX_LOV1_018, TX_LOV1_019, TX_LOV1_021, TX_LOV1_022,
	TX_LOV1_023, TX_LOV1_024, TX_LOV1_025, TX_LOV1_026, TX_LOV1_027,
	TX_LOV1_028, TX_LOV5_015, TX_LOV5_019, TX_LOV5_027, TX_LOV5_030,
	TX_LOV5_038, TX_LOVA_100, TX_LOVA_F01, TX_LOVA_F02, TX_LOVA_F03,
	TX_LOVA_F04, TX_LOVA_F07, TX_LOVA_F08, TX_LOVA_F10, TX_LOVA_F54,
	TX_LOVA_F55, TX_MUD2_040, TX_MUD4_018, TX_TUG2_010, TX_VENA_F41,
	TX_LOV1N000, TX_LOV1N001, TX_LOV1N002, TX_LOV1N003, TX_LOV1N004,
	TX_LOV1N005, TX_LOV1N006, TX_LOV1N007, TX_LOV1N008, TX_LOV1N009,
	TX_LOV1N010, TX_LOV1N011, TX_LOV1N012, TX_LOV1N013, TX_LOV1N014,
	TX_LOV1N015, TX_LOV1N016, TX_LOV1N017, TX_LOV1N018, TX_LOV1N019,
	TX_LOV1N020, TX_LOV1N021, TX_LOV1N022, TX_LOV1N023, TX_LOV1N034,
	TX_LOV1N035, TX_LOV2N005, TX_BRIDU146, TX_GENER004, TX_GET_BOTTLE_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets love1TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 11280, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 11291, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 11301, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 11311, 0, 0, 0 },
	{ TX_LOV0_011, 12397, 0, 0, 0 },
	{ TX_LOV0_012, 12524, 0, 0, 0 },
	{ TX_LOV0_013, 12820, 0, 0, 0 },
	{ TX_LOV0_014, 12584, 0, 0, 0 },
	{ TX_LOV0_015, 12688, 0, 0, 0 },
	{ TX_LOV0_016, 12491, 0, 0, 0 },
	{ TX_LOV0_017, 12954, 0, 0, 0 },
	{ TX_LOV0_019, 12874, 0, 0, 0 },
	{ TX_LOV0_020, 12431, 0, 0, 0 },
	{ TX_LOV0_021, 12742, 0, 0, 0 },
	{ TX_LOV0_022, 12632, 0, 0, 0 },
	{ TX_LOV0_025, 13880, 0, 0, 0 },
	{ TX_LOV0_026, 13753, 0, 0, 0 },
	{ TX_LOV0_029, 13633, 0, 0, 0 },
	{ TX_LOV0_033, 13693, 0, 0, 0 },
	{ TX_LOV0_039, 13015, 0, 0, 0 },
	{ TX_LOV0_040, 13142, 0, 0, 0 },
	{ TX_LOV0_041, 13438, 0, 0, 0 },
	{ TX_LOV0_042, 13202, 0, 0, 0 },
	{ TX_LOV0_043, 13306, 0, 0, 0 },
	{ TX_LOV0_045, 13109, 0, 0, 0 },
	{ TX_LOV0_046, 13572, 0, 0, 0 },
	{ TX_LOV0_047, 13492, 0, 0, 0 },
	{ TX_LOV0_048, 13049, 0, 0, 0 },
	{ TX_LOV0_049, 13360, 0, 0, 0 },
	{ TX_LOV0_050, 13250, 0, 0, 0 },
	{ TX_LOV0_101, 13961, 0, 0, 0 },
	{ TX_LOV0_102, 14025, 0, 0, 0 },
	{ TX_LOV0_103, 14095, 0, 0, 0 },
	{ TX_LOV0_104, 14145, 0, 0, 0 },
	{ TX_LOV0_105, 14202, 0, 0, 0 },
	{ TX_LOV0_106, 14314, 0, 0, 0 },
	{ TX_LOV0_107, 14382, 0, 0, 0 },
	{ TX_LOV0_124, 13830, 0, 0, 0 },
	{ TX_LOV1_001,  2668, 0, 0, 0 },
	{ TX_LOV1_002,  5066, 0, 0, 0 },
	{ TX_LOV1_003,  3804, 0, 0, 0 },
	{ TX_LOV1_004,  3901, 0, 0, 0 },
	{ TX_LOV1_005,  2812, 0, 0, 0 },
	{ TX_LOV1_006, 12236, 0, 0, 0 },
	{ TX_LOV1_007, 11998, 0, 0, 0 },
	{ TX_LOV1_008,  6774, 0, 0, 0 },
	{ TX_LOV1_009,  3507, 0, 0, 0 },
	{ TX_LOV1_010,  3389, 0, 0, 0 },
	{ TX_LOV1_011,  2940, 0, 0, 0 },
	{ TX_LOV1_012,  4991, 0, 0, 0 },
	{ TX_LOV1_013,  5598, 0, 0, 0 },
	{ TX_LOV1_014,  5709, 0, 0, 0 },
	{ TX_LOV1_015,  5330, 0, 0, 0 },
	{ TX_LOV1_016,  6618, 0, 0, 0 },
	{ TX_LOV1_017,  3246, 0, 0, 0 },
	{ TX_LOV1_018, 11433, 0, 0, 0 },
	{ TX_LOV1_019, 11560, 0, 0, 0 },
	{ TX_LOV1_021, 11620, 0, 0, 0 },
	{ TX_LOV1_022, 12328, 0, 0, 0 },
	{ TX_LOV1_023, 12144, 0, 0, 0 },
	{ TX_LOV1_024,  7186, 0, 0, 0 },
	{ TX_LOV1_025,  7248, 0, 0, 0 },
	{ TX_LOV1_026,  5176, 0, 0, 0 },
	{ TX_LOV1_027,  3073, 0, 0, 0 },
	{ TX_LOV1_028,  7353, 0, 0, 0 },
	{ TX_LOV5_015,  8435, 0, 0, 0 },
	//{ TX_LOV5_015,  8256, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV5_019,  8178, 0, 0, 0 },
	{ TX_LOV5_027,  8691, 0, 0, 0 },
	{ TX_LOV5_030,  8595, 0, 0, 0 },
	{ TX_LOV5_038,  8646, 0, 0, 0 },
	{ TX_LOVA_100,  9800, 0, 0, 0 },
	{ TX_LOVA_F01,  9242, 0, 0, 0 },
	{ TX_LOVA_F02,  9314, 0, 0, 0 },
	{ TX_LOVA_F03,  9513, 0, 0, 0 },
	{ TX_LOVA_F04,  9428, 0, 0, 0 },
	{ TX_LOVA_F07,  9877, 0, 0, 0 },
	{ TX_LOVA_F08,  8866, 0, 0, 0 },
	//{ TX_LOVA_F08,  9581, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOVA_F10,  9748, 0, 0, 0 },
	{ TX_LOVA_F54,  9022, 0, 0, 0 },
	{ TX_LOVA_F55,  8323, 0, 0, 0 },
	{ TX_MUD2_040, 10073, 0, 0, 0 },
	{ TX_MUD4_018, 10446, 0, 0, 0 },
	//{ TX_MUD4_018, 10779, 0, 0, 0 },	// ignore duplicate line
	//{ TX_MUD4_018, 10282, 0, 0, 0 },	// ignore duplicate line
	//{ TX_MUD4_018, 10615, 0, 0, 0 },	// ignore duplicate line
	{ TX_TUG2_010,  9678, 0, 0, 0 },
	{ TX_VENA_F41, 10569, 0, 0, 0 },
	{ TX_VENA_F41, 10405, 0, 0, 0 },
	//{ TX_VENA_F41, 10902, 0, 0, 0 },	// ignore duplicate line
	//{ TX_VENA_F41, 10738, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV1N000, 11684, 0, 0, 0 },
	{ TX_LOV1N001, 11864, 0, 0, 0 },
	{ TX_LOV1N002, 11926, 0, 0, 0 },
	{ TX_LOV1N003, 11766, 0, 0, 0 },
	{ TX_LOV1N004, 11802, 0, 0, 0 },
	{ TX_LOV1N005, 11333, 0, 0, 0 },
	{ TX_LOV1N006, 11381, 0, 0, 0 },
	{ TX_LOV1N007,  4720, 0, 0, 0 },
	{ TX_LOV1N008,  6479, 0, 0, 0 },
	{ TX_LOV1N009,  4452, 0, 0, 0 },
	{ TX_LOV1N010,  4093, 0, 0, 0 },
	{ TX_LOV1N011,  4548, 0, 0, 0 },
	{ TX_LOV1N012,  2547, 0, 0, 0 },
	{ TX_LOV1N013,  2113, 0, 0, 0 },
	{ TX_LOV1N014,  2038, 0, 0, 0 },
	{ TX_LOV1N015,  2239, 0, 0, 0 },
	{ TX_LOV1N016,  1914, 0, 0, 0 },
	{ TX_LOV1N017,  1782, 0, 0, 0 },
	{ TX_LOV1N018,  2478, 0, 0, 0 },
	{ TX_LOV1N019,  1399, 0, 0, 0 },
	{ TX_LOV1N020,  1686, 0, 0, 0 },
	{ TX_LOV1N021,  1576, 0, 0, 0 },
	{ TX_LOV1N022,  1498, 0, 0, 0 },
	{ TX_LOV1N023,  2393, 0, 0, 0 },
	{ TX_LOV1N034,  5974, 0, 0, 0 },
	{ TX_LOV1N035,  4313, 0, 0, 0 },
	{ TX_LOV2N005,  9936, 0, 0, 0 },
	//{ TX_LOV2N005, 10016, 0, 0, 0 },	// ignore duplicate line
	//{ TX_LOV2N005, 10162, 0, 0, 0 },	// ignore duplicate line
	{ TX_BRIDU146, 10848, 0, 0, 0 },
	//{ TX_BRIDU146, 10515, 0, 0, 0 },	// ignore duplicate line
	//{ TX_BRIDU146, 10684, 0, 0, 0 },	// ignore duplicate line
	//{ TX_BRIDU146, 10351, 0, 0, 0 },	// ignore duplicate line
	{ TX_GENER004,  9171, 0, 0, 0 },
	{ TX_GET_BOTTLE_ERROR, 4664, 0, 0, 0 },
	{          -1,  0,    0, 0, 0 }
};

extern const RoomText love1Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::love1Tick1() {
	playVoc("LOV1LOOP");

	if (_awayMission->love.freezerOpen)
		loadActorAnim(OBJECT_FREEZER, "s3r2d4o", 0x67, 0x8d, 0);

	if (_awayMission->love.chamberHasDish)
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5o", 0xb4, 0x75, 0);
	else
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5c", 0xb4, 0x75, 0);

	if (_awayMission->love.chamberHasDish)
		loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);

	switch (_awayMission->love.bottleInNozzle) {
	case BOTTLETYPE_N2O:
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle1");
		_roomVar.love.itemInNozzle = OBJECT_IN2O;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;
	case BOTTLETYPE_NH3:
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle2");
		_roomVar.love.itemInNozzle = OBJECT_INH3;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;
	case BOTTLETYPE_H2O:
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle3");
		_roomVar.love.itemInNozzle = OBJECT_IH2O;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;
	case BOTTLETYPE_RLG:
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle4");
		_roomVar.love.itemInNozzle = OBJECT_IRLG;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;

	case BOTTLETYPE_NONE:
	default:
		break;
	}

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	loadActorAnim(OBJECT_DOOR3, "s3r2d3a", 0xdb, 0x7e, 0);
	loadActorAnim(OBJECT_DOOR1, "s3r2d1a", 0, 0, 0);
	loadActorAnim(OBJECT_DOOR2, "s3r2d2a", 0, 0, 0);

	_roomVar.love.cmnXPosToCureSpock = 0x90;
	_roomVar.love.cmnYPosToCureSpock = 0xa7;
}

void Room::love1WalkToDoor3() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xd9, 0x81, 15);
}

void Room::love1OpenDoor3() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR3, "s3r2d3", 0xdb, 0x7e, 2);
		playSoundEffectIndex(kSfxDoor);
	}
}

// Door 3 opened, or door 3 reached (both must occur before transition happens)
void Room::love1ReachedDoor3() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(3, 1);
}

void Room::love1WalkToDoor1() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0x42, 0x97, 16);
}

void Room::love1OpenDoor1() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR1, "s3r2d1", 0, 0, 3);
		playSoundEffectIndex(kSfxDoor);
	}
}

// Door 1 opened, or door 1 reached
void Room::love1ReachedDoor1() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(0, 2);
}

void Room::love1WalkToDoor2() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0x79, 0x85, 17);
}

void Room::love1OpenDoor2() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR2, "s3r2d2", 0, 0, 4);
		playSoundEffectIndex(kSfxDoor);
	}
}

// Door 2 opened, or door 2 reached
void Room::love1ReachedDoor2() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(2, 0);
}

void Room::love1LookAtLaser() {
	showDescription(TX_LOV1N000);
}

void Room::love1LookAtKirk() {
	showDescription(TX_LOV1N003);
}

void Room::love1LookAtSpock() {
	showDescription(TX_LOV1N004);
}

void Room::love1LookAtMccoy() {
	showDescription(TX_LOV1N001);
}

void Room::love1LookAtRedshirt() {
	showDescription(TX_LOV1N002);
}

void Room::love1LookAnywhere() {
	showDescription(TX_LOV1N019);
}

void Room::love1LookAtNozzle() {
	showDescription(TX_LOV1N022);
}

void Room::love1LookAtLadder() {
	showDescription(TX_LOV1N021);
}

void Room::love1LookAtDoor1Or2() {
	showDescription(TX_LOV1N020);
}

void Room::love1LookAtDoor3() {
	showDescription(TX_LOV1N017);
}

void Room::love1LookAtDistillator() {
	showDescription(TX_LOV1N016);
}

void Room::love1LookAtChamber() {
	if (_awayMission->love.chamberHasCure)
		showDescription(TX_LOV1N013);
	else
		showDescription(TX_LOV1N014);
}

void Room::love1LookAtReplicator() {
	showDescription(TX_LOV1N015);
}

void Room::love1LookAtFreezer() {
	if (_awayMission->love.freezerOpen)
		showDescription(TX_LOV1N023);
	else
		showDescription(TX_LOV1N018);
}

void Room::love1LookAtDishes() {
	showDescription(TX_LOV1N012);
}

void Room::love1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV1_001);
}

void Room::love1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_LOV1_005);
}

void Room::love1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_011);
}

void Room::love1TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV1_027);
}

void Room::love1UseMTricorderOnDishes() {
	mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_LOV1_017, false);
	// TODO: only works in this room, despite being inventory item?
}

void Room::love1UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV1_009, false);
	else
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV1_010, false);
}

void Room::love1UseSTricorderOnReplicator() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_LOV1_022, false);
}

void Room::love1UseSTricorderOnLaser() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_LOV1_006, false);
}

void Room::love1UseSTricorderOnFreezer() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_LOV1_023, false);
}

void Room::love1UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_LOV1_003, false);
}

void Room::love1UseSTricorderOnDishes() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_LOV1_004, false);
	// TODO: only works in this room, despite being inventory item?
}

void Room::love1UseSTricorderOnDistillator() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_LOV1_007, false);
}

void Room::love1GetFreezer() {
	walkCrewman(OBJECT_KIRK, 0x71, 0x8e, 14);
}

void Room::love1KirkReachedFreezer() {
	if (_awayMission->love.freezerOpen)
		loadActorAnim2(OBJECT_KIRK, "kusehw", -1, -1, 9);
	else
		showDescription(TX_LOV1N010);
}

void Room::love1KirkGotVirusCulture() {
	giveItem(OBJECT_IDISHES);
	showDescription(TX_LOV1N006);
}

void Room::love1GetFromChamber() {
	if (!_awayMission->love.chamberHasDish)
		showDescription(TX_LOV1N009);
	else
		walkCrewman(OBJECT_KIRK, 0xb5, 0x8c, 8);
}

void Room::love1KirkReachedChamber() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 10);
}

void Room::love1KirkGotCureSample() {
	if (_awayMission->love.chamberHasCure) {
		giveItem(OBJECT_ISAMPLE);
		showDescription(TX_LOV1N035);

		// BUGFIX: after removing the cure, unset this variable.
		// Otherwise, any normal dish inserted afterward automagically becomes a cure.
		_awayMission->love.chamberHasCure = false;
	} else {
		giveItem(OBJECT_IDISHES);
		showDescription(TX_LOV1N006);
	}

	loadActorStandAnim(OBJECT_DISH_IN_CHAMBER);
	loadActorAnim2(OBJECT_CHAMBER, "s3r2d6", 0xb4, 0x75, 0);
	playSoundEffectIndex(kSfxDoor);
	_awayMission->love.chamberHasDish = false;
}

void Room::love1GetFromNozzle() {
	walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 2);
}

void Room::love1KirkReachedNozzleToGet() {
	if (_awayMission->love.bottleInNozzle == 0)
		showDescription(TX_LOV1N011);
	else
		loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 11);
}

void Room::love1KirkGotBottleFromNozzle() {
	switch (_awayMission->love.bottleInNozzle) {
	case BOTTLETYPE_N2O:
		giveItem(OBJECT_IN2O);
		break;
	case BOTTLETYPE_NH3:
		giveItem(OBJECT_INH3);
		break;
	case BOTTLETYPE_H2O:
		giveItem(OBJECT_IH2O);
		break;
	case BOTTLETYPE_RLG:
		giveItem(OBJECT_IRLG);
		break;
	default:
		showDescription(TX_GET_BOTTLE_ERROR);
		break;
	}

	_awayMission->love.bottleInNozzle = BOTTLETYPE_NONE;
	loadActorStandAnim(OBJECT_BOTTLE);
	showDescription(TX_LOV1N007);
	_roomVar.love.itemInNozzle = 0;
}

void Room::love1UseN2OOnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_IN2O;
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle1");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_N2O;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1UseH2OOnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_IH2O;
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle3");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_H2O;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1UseNH3OnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_INH3;
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle2");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_NH3;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1UseRLGOnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_IRLG;
		Common::strcpy_s(_roomVar.love.bottleAnimation, "btle4");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_RLG;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1KirkReachedNozzleToPut() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 12);
}

void Room::love1KirkPutBottleInNozzle() {
	loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
	loseItem(_roomVar.love.itemInNozzle);
}

void Room::love1UseAnthingOnNozzle() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_012);
}

void Room::love1UseSpockOnReplicator() {
	showText(TX_SPEAKER_SPOCK, TX_LOV1_002);
}

void Room::love1UseRedshirtOnReplicator() {
	showText(TX_SPEAKER_FERRIS, TX_LOV1_026);
}

void Room::love1UseMccoyOnReplicator() {
	walkCrewman(OBJECT_MCCOY, 0xb1, 0x8c, 4);
}

void Room::love1MccoyReachedReplicator() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		showText(TX_SPEAKER_MCCOY, TX_LOV1_021);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else if (!_awayMission->love.chamberHasDish) {
		showText(TX_SPEAKER_MCCOY, TX_LOV1_015);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else
		loadActorAnim2(OBJECT_MCCOY, "musehn", -1, -1, 13);
}

void Room::love1MccoyUsedReplicator() {
	if (_roomVar.love.itemInNozzle == OBJECT_INH3) {
		loadActorStandAnim(OBJECT_DISH_IN_CHAMBER);
		loadActorAnim2(OBJECT_CHAMBER, "s3r2d6", 0xb4, 0x75, 7);
		playSoundEffectIndex(kSfxDoor);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_LOV1_019);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	}
}

void Room::love1ChamberClosed() {
	loadActorAnim2(OBJECT_CHAMBER, "s3r2d5", 0xb4, 0x75, 8);
	playSoundEffectIndex(kSfxDoor);
}

void Room::love1ChamberOpened() {
	loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);
	showText(TX_SPEAKER_MCCOY, TX_LOV1_018);
	_awayMission->love.chamberHasCure = true;
}

void Room::love1UseAnythingOnChamber() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_013);
}

void Room::love1UseDishesOnChamber() {
	walkCrewman(OBJECT_KIRK, 0xb3, 0x8c, 5);
}

void Room::love1KirkReachedChamberToPut() {
	if (_awayMission->love.chamberHasDish)
		showText(TX_SPEAKER_MCCOY, TX_LOV1_014); // TODO: test
	else {
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5", 0xb4, 0x75, 1);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love1ChamberOpenedForDish() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 14);
}

void Room::love1KirkPutDishInChamber() {
	loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);
	loseItem(OBJECT_IDISHES);
	_awayMission->love.chamberHasDish = true;
}

void Room::love1UseInsulationOnDistillator() {
	walkCrewman(OBJECT_KIRK, 0xbe, 0xc1, 6);
}

void Room::love1KirkReachedDistillator() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 15);
	playVoc("LD1PROCE");
}

void Room::love1KirkGotPolyberylcarbonate() {
	// Result of using insulation on distillator
	showDescription(TX_LOV1N034);
	if (!_awayMission->love.gotPolyberylcarbonate) {
		_awayMission->love.gotPolyberylcarbonate = true;
		_awayMission->love.missionScore++;
	}

	giveItem(OBJECT_IPBC);
	loseItem(OBJECT_IINSULAT);
}

void Room::love1UseKirkOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0xa0;
	_roomVar.love.crewmanUsingFreezerRetX = 0x96;
	_roomVar.love.crewmanUsingDevice = OBJECT_KIRK;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1UseRedshirtOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0x89;
	_roomVar.love.crewmanUsingFreezerRetX = 0x95;
	_roomVar.love.crewmanUsingDevice = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1UseSpockOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0xa9;
	_roomVar.love.crewmanUsingFreezerRetX = 0x61;
	_roomVar.love.crewmanUsingDevice = OBJECT_SPOCK;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1UseMccoyOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0x98;
	_roomVar.love.crewmanUsingFreezerRetX = 0xbf;
	_roomVar.love.crewmanUsingDevice = OBJECT_MCCOY;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1CrewmanReachedFreezer() {
	Common::String useAnim = getCrewmanAnimFilename(_roomVar.love.crewmanUsingDevice, "usehw");

	loadActorAnim2(_roomVar.love.crewmanUsingDevice, useAnim, -1, -1, 16);
}

void Room::love1CrewmanOpenedOrClosedFreezer() {
	if (_awayMission->love.freezerOpen)
		loadActorAnim(OBJECT_FREEZER, "s3r2d4a", 0x67, 0x8d, 0);
	else
		loadActorAnim(OBJECT_FREEZER, "s3r2d4", 0x67, 0x8d, 0);

	playSoundEffectIndex(kSfxDoor);
	_awayMission->love.freezerOpen = !_awayMission->love.freezerOpen;

	walkCrewman(_roomVar.love.crewmanUsingDevice, _roomVar.love.crewmanUsingFreezerRetX, _roomVar.love.crewmanUsingFreezerRetY, 0);
	_awayMission->disableInput = false;
}

void Room::love1UseAnythingOnFreezer() {
	walkCrewman(OBJECT_KIRK, 0x6f, 0x8e, 11);
}

void Room::love1ReachedFreezerWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kusemw", -1, -1, 17);
}

void Room::love1FinishedUsingArbitraryItemOnFreezer() {
	showDescription(TX_LOV1N008);
}

void Room::love1UseAnythingOnReplicator() {
	walkCrewman(OBJECT_KIRK, 0xb5, 0x8c, 12);
}

void Room::love1ReachedReplicatorWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 18);
}

void Room::love1FinishedUsingArbitraryItemOnReplicator() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_016);
}

void Room::love1UseAnythingOnDistillator() {
	walkCrewman(OBJECT_KIRK, 0xc2, 0xb2, 13);
}

void Room::love1ReachedDistillatorWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kuseme", -1, -1, 19);
}

void Room::love1FinishedUsingArbitraryItemOnDistillator() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_008);
}

void Room::love1UseKirkOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_KIRK;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1UseSpockOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_SPOCK;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1UseMccoyOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_MCCOY;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1UseRedshirtOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_REDSHIRT;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1CrewmanReachedLadder() {
	const int32 speakers[] = {
		TX_SPEAKER_KIRK,
		TX_SPEAKER_SPOCK,
		TX_SPEAKER_MCCOY,
		TX_SPEAKER_FERRIS
	};

	if (_awayMission->love.romulansUnconsciousFromLaughingGas || _awayMission->love.romulansUnconsciousFromVirus) // Romulans unconscious
		loadRoomIndex(4, 3);
	else { // Romulans still conscious, they shoot you
		loadActorAnim(OBJECT_PHASERSHOT, "s3r2s2", 0xf3, 0x89, 0);
		playSoundEffectIndex(kSfxPhaser);

		Common::String anim = getCrewmanAnimFilename(_roomVar.love.crewmanUsingDevice, "killw");
		loadActorAnim(_roomVar.love.crewmanUsingDevice, anim, 0x102, 0x89, 6);
		_roomVar.love.dyingSpeaker = speakers[_roomVar.love.crewmanUsingDevice];
	}
}

void Room::love1CrewmanDiedFromPhaser() {
	if (_roomVar.love.crewmanUsingDevice == OBJECT_REDSHIRT) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_FERRIS, TX_LOV1_024);
		_awayMission->redshirtDead = true;
	} else {
		showText(_roomVar.love.dyingSpeaker, TX_LOV1_025);
		showGameOverMenu();
	}
}

// Romulans fire a "warning shot" when you go to the right part of the room.
void Room::love1TouchedHotspot0() {
	if (_awayMission->love.romulansUnconsciousFromLaughingGas || _awayMission->love.romulansUnconsciousFromVirus)
		return;
	loadActorAnim(OBJECT_PHASERSHOT, "s3r2s1", 0xf3, 0x89, 0);
	playSoundEffectIndex(kSfxPhaser);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, TX_LOV1_028);
}


}
