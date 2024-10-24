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

// BUGFIX: some of the audio filenames in the text were incorrect? (see text.h)

#include "startrek/room.h"

#define OBJECT_CABINET 8
#define OBJECT_SYNTHESIZER_DOOR 9
#define OBJECT_DOOR 10
#define OBJECT_GAS_FEED 11
#define OBJECT_CAN1 12
#define OBJECT_CAN2 13
#define OBJECT_ANTIGRAV 14
#define OBJECT_SYNTHESIZER_OUTPUT 15
#define OBJECT_POLYBERYLCARBONATE 16
#define OBJECT_VIRUSSAMPLE 17
#define OBJECT_CURESAMPLE 18
#define OBJECT_CURE 19

#define HOTSPOT_SYNTHESIZER 0x20
#define HOTSPOT_ARDAK 0x21
#define HOTSPOT_DOOR 0x22
#define HOTSPOT_CANISTER_SLOT 0x23

namespace StarTrek {

extern const RoomAction love2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::love2Tick1 },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0, 0}, &Room::love2TouchedWarp1 },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0}, &Room::love2Timer3Expired },
	{ {ACTION_WALK, 10, 0, 0, 0}, &Room::love2WalkToDoor },
	{ {ACTION_WALK, 0x22, 0, 0, 0}, &Room::love2WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::love2TouchedHotspot0 },
	{ {ACTION_FINISHED_WALKING, 14, 0, 0, 0}, &Room::love2DoorReachedOrOpened },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::love2DoorReachedOrOpened },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::love2LookAtCabinet },
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::love2LookAtDoor },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::love2LookAtSynthesizer },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::love2LookAtAnywhere },
	{ {ACTION_LOOK, 14, 0, 0, 0}, &Room::love2LookAtAntigrav },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::love2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::love2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love2LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::love2LookAtKirk },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::love2LookAtArdak },
	{ {ACTION_LOOK, 15, 0, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 16, 0, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 18, 0, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 19, 0, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 17, 0, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::love2LookAtChamber },
	{ {ACTION_LOOK, 12, 0, 0, 0}, &Room::love2LookAtCan1 },
	{ {ACTION_LOOK, 13, 0, 0, 0}, &Room::love2LookAtCan2 },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::love2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::love2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::love2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love2TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::love2UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::love2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_KIRK, 8, 0, 0}, &Room::love2UseKirkOnCabinet },
	{ {ACTION_USE, OBJECT_SPOCK, 8, 0, 0}, &Room::love2UseSpockOnCabinet },
	{ {ACTION_USE, OBJECT_MCCOY, 8, 0, 0}, &Room::love2UseMccoyOnCabinet },
	{ {ACTION_USE, OBJECT_REDSHIRT, 8, 0, 0}, &Room::love2UseRedshirtOnCabinet },
	{ {ACTION_FINISHED_WALKING, 12, 0, 0, 0}, &Room::love2CrewmanReachedCabinet },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0, 0}, &Room::love2CrewmanAccessedCabinet },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::love2CrewmanOpenedOrClosedCabinet },
	{ {ACTION_USE, OBJECT_IWRENCH, 11, 0, 0}, &Room::love2UseWrenchOnGasFeed },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::love2ReachedGasFeed },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0, 0}, &Room::love2ChangedGasFeed },
	{ {ACTION_USE, OBJECT_IO2GAS, 0x23, 0, 0}, &Room::love2UseO2GasOnCanisterSlot },
	{ {ACTION_USE, OBJECT_IH2GAS, 0x23, 0, 0}, &Room::love2UseH2GasOnCanisterSlot },
	{ {ACTION_USE, OBJECT_IN2GAS, 0x23, 0, 0}, &Room::love2UseN2GasOnCanisterSlot },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::love2ReachedCanisterSlot },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0, 0}, &Room::love2PutCanisterInSlot1 },
	{ {ACTION_FINISHED_ANIMATION, 14, 0, 0, 0}, &Room::love2PutCanisterInSlot2 },
	{ {ACTION_USE, OBJECT_IANTIGRA, 12, 0, 0}, &Room::love2UseAntigravOnCanister1 },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::love2ReachedCanisterSlot1ToGet },
	{ {ACTION_FINISHED_ANIMATION, 15, 0, 0, 0}, &Room::love2TookCanister1 },
	{ {ACTION_USE, OBJECT_IANTIGRA, 13, 0, 0}, &Room::love2UseAntigravOnCanister2 },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::love2ReachedCanisterSlot2ToGet },
	{ {ACTION_FINISHED_ANIMATION, 16, 0, 0, 0}, &Room::love2TookCanister2 },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0, 0}, &Room::love2UseKirkOnSynthesizer },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0, 0}, &Room::love2UseSpockOnSynthesizer },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0, 0}, &Room::love2UseMccoyOnSynthesizer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0, 0}, &Room::love2UseRedshirtOnSynthesizer },
	{ {ACTION_FINISHED_WALKING, 13, 0, 0, 0}, &Room::love2CrewmanReachedSynthesizer },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0, 0}, &Room::love2CrewmanUsedSynthesizer },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0, 0}, &Room::love2SpockReachedGasFeeds },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0, 0}, &Room::love2SpockEnabledGasFeeds },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::love2SynthesizerDoorClosed },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::love2SynthesizerFinished },
	{ {ACTION_FINISHED_ANIMATION, 27, 0, 0, 0}, &Room::love2ClosedSynthesizerDoorMakingRLG },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0, 0}, &Room::love2ClosedSynthesizerDoorMakingCure },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::love2CureStartedSynthesizing },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0, 0}, &Room::love2CureSynthesized },
	{ {ACTION_USE, OBJECT_IPBC, 9, 0, 0}, &Room::love2UsePolyberylcarbonateOnSynthesizerDoor },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::love2KirkReachedSynthesizerWithPolyberylcarbonate },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0}, &Room::love2SynthesizerDoorOpenedWithPolyberylcarbonate },
	{ {ACTION_FINISHED_ANIMATION, 19, 0, 0, 0}, &Room::love2PutPolyberylcarbonateInSynthesizer },

	{ {ACTION_USE, OBJECT_IDISHES, 9, 0, 0}, &Room::love2UseVirusSampleOnSynthesizerDoor },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0, 0}, &Room::love2KirkReachedSynthesizerWithVirusSample },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::love2SynthesizerDoorOpenedWithVirusSample },
	{ {ACTION_FINISHED_ANIMATION, 20, 0, 0, 0}, &Room::love2PutVirusSampleInSynthesizer },

	{ {ACTION_USE, OBJECT_ISAMPLE, 9, 0, 0}, &Room::love2UseCureSampleOnSynthesizerDoor },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0, 0}, &Room::love2KirkReachedSynthesizerWithCureSample },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0, 0}, &Room::love2SynthesizerDoorOpenedWithCureSample },
	{ {ACTION_FINISHED_ANIMATION, 21, 0, 0, 0}, &Room::love2PutCureSampleInSynthesizer },

	{ {ACTION_USE, 0xff, 9, 0, 0}, &Room::love2UseAnythingOnSynthesizerDoor },
	{ {ACTION_USE, 0xff, 0x20, 0, 0}, &Room::love2UseAnythingOnSynthesizer },
	{ {ACTION_GET, 13, 0, 0, 0}, &Room::love2GetCanister },
	{ {ACTION_GET, 12, 0, 0, 0}, &Room::love2GetCanister },

	{ {ACTION_GET, 14, 0, 0, 0}, &Room::love2GetAntigrav },
	{ {ACTION_FINISHED_WALKING, 9, 0, 0, 0}, &Room::love2ReachedAntigrav },
	{ {ACTION_FINISHED_ANIMATION, 22, 0, 0, 0}, &Room::love2GotAntigrav },

	{ {ACTION_GET, 16, 0, 0, 0}, &Room::love2GetPolyberylcarbonate },
	{ {ACTION_FINISHED_WALKING, 18, 0, 0, 0}, &Room::love2ReachedPolyberylcarbonate },
	{ {ACTION_FINISHED_ANIMATION, 26, 0, 0, 0}, &Room::love2GotPolyberylcarbonate },

	{ {ACTION_GET, 17, 0, 0, 0}, &Room::love2GetDishes },
	{ {ACTION_FINISHED_WALKING, 17, 0, 0, 0}, &Room::love2ReachedDishes },
	{ {ACTION_FINISHED_ANIMATION, 25, 0, 0, 0}, &Room::love2GotDishes },

	{ {ACTION_GET, 18, 0, 0, 0}, &Room::love2GetSample },
	{ {ACTION_FINISHED_WALKING, 19, 0, 0, 0}, &Room::love2ReachedSample },
	{ {ACTION_FINISHED_ANIMATION, 28, 0, 0, 0}, &Room::love2GotSample },

	{ {ACTION_GET, 15, 0, 0, 0}, &Room::love2GetSynthesizerOutput },
	{ {ACTION_FINISHED_WALKING, 10, 0, 0, 0}, &Room::love2ReachedSynthesizerOutput },
	{ {ACTION_FINISHED_ANIMATION, 23, 0, 0, 0}, &Room::love2GotSynthesizerOutput },

	{ {ACTION_GET, 19, 0, 0, 0}, &Room::love2GetCure },
	{ {ACTION_FINISHED_WALKING, 11, 0, 0, 0}, &Room::love2ReachedCure },
	{ {ACTION_FINISHED_ANIMATION, 24, 0, 0, 0}, &Room::love2GotCure },

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

enum love2TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_FERRIS, TX_SPEAKER_COMPUTER,
	TX_LOV0_011, TX_LOV0_012, TX_LOV0_013, TX_LOV0_014, TX_LOV0_015,
	TX_LOV0_016, TX_LOV0_017, TX_LOV0_019, TX_LOV0_020, TX_LOV0_021,
	TX_LOV0_022, TX_LOV0_025, TX_LOV0_026, TX_LOV0_029, TX_LOV0_033,
	TX_LOV0_039, TX_LOV0_040, TX_LOV0_041, TX_LOV0_042, TX_LOV0_043,
	TX_LOV0_045, TX_LOV0_046, TX_LOV0_047, TX_LOV0_048, TX_LOV0_049,
	TX_LOV0_050, TX_LOV0_101, TX_LOV0_102, TX_LOV0_103, TX_LOV0_104,
	TX_LOV0_105, TX_LOV0_106, TX_LOV0_107, TX_LOV0_124, TX_LOV2_001,
	TX_LOV2_002, TX_LOV2_003, TX_LOV2_004, TX_LOV2_005, TX_LOV2_006,
	TX_LOV2_007, TX_LOV2_008, TX_LOV2_009, TX_LOV2_010, TX_LOV2_011,
	TX_LOV2_012, TX_LOV2_014, TX_LOV2_015, TX_LOV2_016, TX_LOV2_017,
	TX_LOV2_018, TX_LOV2_019, TX_LOV2_020, TX_LOV2_021, TX_LOV2_022,
	TX_LOV2_023, TX_LOV2_024, TX_LOV2_025, TX_LOV2_026, TX_LOV2_027,
	TX_LOV2_028, TX_LOV2_029, TX_LOV2_030, TX_LOV2_031, TX_LOV2_032,
	TX_LOV2_033, TX_LOV2_034, TX_LOV2_035, TX_LOV2_036, TX_LOV2_037,
	TX_LOV2_038, TX_LOV2_039, TX_LOV2_040, TX_LOV2_041, TX_LOV2_043,
	TX_LOV5_015, TX_LOV5_019, TX_LOV5_027, TX_LOV5_030, TX_LOV5_038,
	TX_LOVA_100, TX_LOVA_F01, TX_LOVA_F02, TX_LOVA_F03, TX_LOVA_F04,
	TX_LOVA_F07, TX_LOVA_F08, TX_LOVA_F10, TX_LOVA_F54, TX_LOVA_F55,
	TX_LOV2N000, TX_LOV2N001, TX_LOV2N002, TX_LOV2N003, TX_LOV2N004,
	TX_LOV2N005, TX_LOV2N006, TX_LOV2N007, TX_LOV2N008, TX_LOV2N009,
	TX_LOV2N010, TX_LOV2N011, TX_LOV2N012, TX_LOV2N013, TX_LOV2N014,
	TX_LOV2N015, TX_LOV2N016, TX_LOV2N017, TX_LOV2N018, TX_LOV2N019,
	TX_LOV2N024, TX_LOV2N025, TX_LOV2N026, TX_LOV2N027, TX_LOV2N028,
	TX_LOV2N029, TX_LOV2N030, TX_LOV2N031, TX_LOV2N033, TX_LOV2N034,
	TX_LOV2N035, TX_LOV2N036, TX_LOV2N037, TX_LOV2N038, TX_LOV2N039,
	TX_LOV2N040, TX_LOV2N041, TX_LOV2N042, TX_LOV2N043, TX_LOV2N044,
	TX_LOV2N045, TX_LOV2N046, TX_LOV2N047, TX_LOV2N048, TX_LOV2N049,
	TX_LOV2N050, TX_LOV2N051, TX_MUD2_040, TX_MUD4_018, TX_TUG2_010,
	TX_VENA_F41, TX_BRIDU146, TX_GENER004
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets love2TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 14244, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 14255, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 14265, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 14275, 0, 0, 0 },
	{ TX_SPEAKER_COMPUTER, 14287, 0, 0, 0 },
	{ TX_LOV0_011, 16471, 0, 0, 0 },
	{ TX_LOV0_012, 16598, 0, 0, 0 },
	{ TX_LOV0_013, 16894, 0, 0, 0 },
	{ TX_LOV0_014, 16658, 0, 0, 0 },
	{ TX_LOV0_015, 16762, 0, 0, 0 },
	{ TX_LOV0_016, 16565, 0, 0, 0 },
	{ TX_LOV0_017, 17028, 0, 0, 0 },
	{ TX_LOV0_019, 16948, 0, 0, 0 },
	{ TX_LOV0_020, 16505, 0, 0, 0 },
	{ TX_LOV0_021, 16816, 0, 0, 0 },
	{ TX_LOV0_022, 16706, 0, 0, 0 },
	{ TX_LOV0_025, 17954, 0, 0, 0 },
	{ TX_LOV0_026, 17827, 0, 0, 0 },
	{ TX_LOV0_029, 17707, 0, 0, 0 },
	{ TX_LOV0_033, 17767, 0, 0, 0 },
	{ TX_LOV0_039, 17089, 0, 0, 0 },
	{ TX_LOV0_040, 17216, 0, 0, 0 },
	{ TX_LOV0_041, 17512, 0, 0, 0 },
	{ TX_LOV0_042, 17276, 0, 0, 0 },
	{ TX_LOV0_043, 17380, 0, 0, 0 },
	{ TX_LOV0_045, 17183, 0, 0, 0 },
	{ TX_LOV0_046, 17646, 0, 0, 0 },
	{ TX_LOV0_047, 17566, 0, 0, 0 },
	{ TX_LOV0_048, 17123, 0, 0, 0 },
	{ TX_LOV0_049, 17434, 0, 0, 0 },
	{ TX_LOV0_050, 17324, 0, 0, 0 },
	{ TX_LOV0_101, 18035, 0, 0, 0 },
	{ TX_LOV0_102, 18099, 0, 0, 0 },
	{ TX_LOV0_103, 18169, 0, 0, 0 },
	{ TX_LOV0_104, 18219, 0, 0, 0 },
	{ TX_LOV0_105, 18276, 0, 0, 0 },
	{ TX_LOV0_106, 18388, 0, 0, 0 },
	{ TX_LOV0_107, 18456, 0, 0, 0 },
	{ TX_LOV0_124, 17904, 0, 0, 0 },
	{ TX_LOV2_001, 1734, 0, 0, 0 },
	{ TX_LOV2_002, 6690, 0, 0, 0 },
	{ TX_LOV2_003, 5722, 0, 0, 0 },
	{ TX_LOV2_004, 5885, 0, 0, 0 },
	{ TX_LOV2_005, 4110, 0, 0, 0 },
	{ TX_LOV2_006, 4025, 0, 0, 0 },
	{ TX_LOV2_007, 8933, 0, 0, 0 },
	{ TX_LOV2_008, 15998, 0, 0, 0 },
	{ TX_LOV2_009, 15764, 0, 0, 0 },
	{ TX_LOV2_010, 4710, 0, 0, 0 },
	//{ TX_LOV2_010, 16437, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV2_011, 9129, 0, 0, 0 },
	{ TX_LOV2_012, 1916, 0, 0, 0 },
	{ TX_LOV2_014, 8848, 0, 0, 0 },
	{ TX_LOV2_015, 5377, 0, 0, 0 },
	{ TX_LOV2_016, 15386, 0, 0, 0 },
	{ TX_LOV2_017, 7699, 0, 0, 0 },
	{ TX_LOV2_018, 6248, 0, 0, 0 },
	{ TX_LOV2_019, 15259, 0, 0, 0 },
	{ TX_LOV2_020, 8083, 0, 0, 0 },
	{ TX_LOV2_021, 6856, 0, 0, 0 },
	{ TX_LOV2_022, 6402, 0, 0, 0 },
	{ TX_LOV2_023, 7371, 0, 0, 0 },
	{ TX_LOV2_024, 4468, 0, 0, 0 },
	{ TX_LOV2_025, 3226, 0, 0, 0 },
	//{ TX_LOV2_025, 3540, 0, 0, 0 },		// ignore duplicate line
	{ TX_LOV2_026, 2639, 0, 0, 0 },
	{ TX_LOV2_027, 16081, 0, 0, 0 },
	{ TX_LOV2_028, 6936, 0, 0, 0 },
	{ TX_LOV2_029, 15832, 0, 0, 0 },
	{ TX_LOV2_030, 6312, 0, 0, 0 },
	{ TX_LOV2_031, 6088, 0, 0, 0 },
	{ TX_LOV2_032, 5224, 0, 0, 0 },
	{ TX_LOV2_033, 15321, 0, 0, 0 },
	{ TX_LOV2_034, 7288, 0, 0, 0 },	// NOTE: has typo (dash instead of underscore)
	{ TX_LOV2_035, 14978, 0, 0, 0 },
	{ TX_LOV2_036, 5447, 0, 0, 0 },
	{ TX_LOV2_037, 15561, 0, 0, 0 },
	{ TX_LOV2_038, 6472, 0, 0, 0 },
	{ TX_LOV2_039, 7111, 0, 0, 0 },	// NOTE: has typo (dash instead of underscore)
	{ TX_LOV2_040, 9058, 0, 0, 0 },
	{ TX_LOV2_041, 15889, 0, 0, 0 },
	{ TX_LOV2_043, 5634, 0, 0, 0 },
	{ TX_LOV5_015, 11567, 0, 0, 0 },
	//{ TX_LOV5_015, 11388, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV5_019, 11310, 0, 0, 0 },
	{ TX_LOV5_027, 11823, 0, 0, 0 },
	{ TX_LOV5_030, 11727, 0, 0, 0 },
	{ TX_LOV5_038, 11778, 0, 0, 0 },
	{ TX_LOVA_100, 12932, 0, 0, 0 },
	{ TX_LOVA_F01, 12374, 0, 0, 0 },
	{ TX_LOVA_F02, 12446, 0, 0, 0 },
	{ TX_LOVA_F03, 12645, 0, 0, 0 },
	{ TX_LOVA_F04, 12560, 0, 0, 0 },
	{ TX_LOVA_F07, 13009, 0, 0, 0 },
	{ TX_LOVA_F08, 11998, 0, 0, 0 },
	//{ TX_LOVA_F08, 12713, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOVA_F10, 12880, 0, 0, 0 },
	{ TX_LOVA_F54, 12154, 0, 0, 0 },
	{ TX_LOVA_F55, 11455, 0, 0, 0 },
	{ TX_LOV2N000, 16147, 0, 0, 0 },
	{ TX_LOV2N001, 15741, 0, 0, 0 },
	{ TX_LOV2N002, 14608, 0, 0, 0 },
	{ TX_LOV2N003, 14647, 0, 0, 0 },
	{ TX_LOV2N004, 14569, 0, 0, 0 },
	{ TX_LOV2N005,  4653, 0, 0, 0 },
	//{ TX_LOV2N005, 13294, 0, 0, 0 },	// ignore duplicate line
	//{ TX_LOV2N005, 13148, 0, 0, 0 },	// ignore duplicate line
	//{ TX_LOV2N005, 13068, 0, 0, 0 },	// ignore duplicate line
	//{ TX_LOV2N005,  4410, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV2N006, 14300, 0, 0, 0 },
	{ TX_LOV2N007, 16217, 0, 0, 0 },
	{ TX_LOV2N008, 16186, 0, 0, 0 },
	{ TX_LOV2N009,  1467, 0, 0, 0 },
	{ TX_LOV2N010, 14780, 0, 0, 0 },
	{ TX_LOV2N011, 14686, 0, 0, 0 },
	{ TX_LOV2N012,  4334, 0, 0, 0 },
	{ TX_LOV2N013, 16249, 0, 0, 0 },
	{ TX_LOV2N014, 14728, 0, 0, 0 },
	{ TX_LOV2N015, 16281, 0, 0, 0 },
	{ TX_LOV2N016,  5555, 0, 0, 0 },
	{ TX_LOV2N017,  5147, 0, 0, 0 },
	{ TX_LOV2N018,  5812, 0, 0, 0 },
	{ TX_LOV2N019,  6013, 0, 0, 0 },
	{ TX_LOV2N024,  2955, 0, 0, 0 },
	{ TX_LOV2N025,  8231, 0, 0, 0 },
	//{ TX_LOV2N025,  8454, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV2N026, 14840, 0, 0, 0 },
	{ TX_LOV2N027, 14878, 0, 0, 0 },
	{ TX_LOV2N028, 14425, 0, 0, 0 },
	{ TX_LOV2N029, 14920, 0, 0, 0 },
	{ TX_LOV2N030,  3356, 0, 0, 0 },
	{ TX_LOV2N031,  3696, 0, 0, 0 },
	{ TX_LOV2N033, 16366, 0, 0, 0 },
	{ TX_LOV2N034,  2843, 0, 0, 0 },
	{ TX_LOV2N035,  3081, 0, 0, 0 },
	{ TX_LOV2N036,  9618, 0, 0, 0 },
	{ TX_LOV2N037,  9803, 0, 0, 0 },
	{ TX_LOV2N038, 10228, 0, 0, 0 },
	{ TX_LOV2N039, 10072, 0, 0, 0 },
	{ TX_LOV2N040,  9986, 0, 0, 0 },
	{ TX_LOV2N041, 10152, 0, 0, 0 },
	{ TX_LOV2N042,  9298, 0, 0, 0 },
	{ TX_LOV2N043, 10484, 0, 0, 0 },
	{ TX_LOV2N044,  9430, 0, 0, 0 },
	{ TX_LOV2N045, 15417, 0, 0, 0 },
	{ TX_LOV2N046,  8005, 0, 0, 0 },
	{ TX_LOV2N047,  7616, 0, 0, 0 },
	{ TX_LOV2N048,  7209, 0, 0, 0 },
	{ TX_LOV2N049,  7033, 0, 0, 0 },
	{ TX_LOV2N050,  6778, 0, 0, 0 },
	{ TX_LOV2N051,  6612, 0, 0, 0 },
	{ TX_MUD2_040, 13205, 0, 0, 0 },
	{ TX_MUD4_018, 13414, 0, 0, 0 },
	//{ TX_MUD4_018, 13911, 0, 0, 0 },	// ignore duplicate line
	//{ TX_MUD4_018, 13747, 0, 0, 0 },	// ignore duplicate line
	//{ TX_MUD4_018, 13578, 0, 0, 0 },	// ignore duplicate line
	{ TX_TUG2_010, 12810, 0, 0, 0 },
	{ TX_VENA_F41, 14034, 0, 0, 0 },
	//{ TX_VENA_F41, 13537, 0, 0, 0 },	// ignore duplicate line
	//{ TX_VENA_F41, 13870, 0, 0, 0 },	// ignore duplicate line
	//{ TX_VENA_F41, 13701, 0, 0, 0 },	// ignore duplicate line
	{ TX_BRIDU146, 13647, 0, 0, 0 },
	//{ TX_BRIDU146, 13980, 0, 0, 0 },	// ignore duplicate line
	//{ TX_BRIDU146, 13816, 0, 0, 0 },	// ignore duplicate line
	//{ TX_BRIDU146, 13483, 0, 0, 0 },	// ignore duplicate line
	{ TX_GENER004, 12303, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText love2Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::love2Tick1() {
	playVoc("LOV2LOOP");

	char canName[10];

	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		Common::strcpy_s(canName, "o2can");
		break;
	case CANTYPE_H2:
		Common::strcpy_s(canName, "h2can");
		break;
	case CANTYPE_N2:
		Common::strcpy_s(canName, "n2can");
		break;
	default:
		break;
	}

	if (_awayMission->love.canister1 >= 1 && _awayMission->love.canister1 <= 3)
		loadActorAnim(OBJECT_CAN1, canName, 0xa7, 0xae, 0);

	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		Common::strcpy_s(canName, "o2can");
		break;
	case CANTYPE_H2:
		Common::strcpy_s(canName, "h2can");
		break;
	case CANTYPE_N2:
		Common::strcpy_s(canName, "n2can");
		break;
	default:
		break;
	}

	if (_awayMission->love.canister2 >= 1 && _awayMission->love.canister2 <= 3)
		loadActorAnim(OBJECT_CAN2, canName, 0xb1, 0xaf, 0);

	if (_awayMission->love.gasFeedOn)
		loadActorAnim(OBJECT_GAS_FEED, "s3r3vo", 0xac, 0x75, 0);
	else
		loadActorAnim(OBJECT_GAS_FEED, "s3r3vc", 0xac, 0x75, 0);

	if (_awayMission->love.cabinetOpen)
		loadActorAnim(OBJECT_CABINET, "s3r3d1o", 0x1e, 0xaf, 0);
	else
		loadActorAnim(OBJECT_CABINET, "s3r3d1c", 0x1e, 0xaf, 0);

	if (_awayMission->love.cabinetOpen && !haveItem(OBJECT_IANTIGRA))
		loadActorAnim(OBJECT_ANTIGRAV, "antigr", 0x1f, 0xa7, 0);

	if (_awayMission->love.synthesizerContents != 0)
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2o", 0x8a, 0x8d, 0);
	else
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2c", 0x8a, 0x8d, 0);

	bool valid = true;

	switch (_awayMission->love.synthesizerContents) {
	case SYNTHITEM_PBC:
		Common::strcpy_s(_roomVar.love.chamberInputAnim, "pbcanm");
		_roomVar.love.chamberObject = OBJECT_POLYBERYLCARBONATE;
		break;
	case SYNTHITEM_VIRUS_SAMPLE:
		Common::strcpy_s(_roomVar.love.chamberInputAnim, "dishes");
		_roomVar.love.chamberObject = OBJECT_VIRUSSAMPLE;
		break;
	case SYNTHITEM_CURE_SAMPLE:
		Common::strcpy_s(_roomVar.love.chamberInputAnim, "dishes");
		_roomVar.love.chamberObject = OBJECT_CURESAMPLE;
		break;
	case SYNTHITEM_BOTTLE:
		switch (_awayMission->love.synthesizerBottleIndex) {
		case 1:
			Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle1");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		case 2:
			Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle2");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		case 3:
			Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle3");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		case 4:
			Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle4");
			_roomVar.love.chamberObject = OBJECT_SYNTHESIZER_OUTPUT;
			break;
		default:
			Common::strcpy_s(_roomVar.love.chamberOutputAnim, "cure");
			_roomVar.love.chamberObject = OBJECT_CURE;
			break;
		}
		break;
	default:
		valid = false;
		break;
	}

	if (valid) {
		if (_awayMission->love.synthesizerContents == 9)
			loadActorAnim2(_roomVar.love.chamberObject, _roomVar.love.chamberOutputAnim, 0x8a, 0x8b, 0);
		else
			loadActorAnim2(_roomVar.love.chamberObject, _roomVar.love.chamberInputAnim, 0x8a, 0x8b, 0);
	}

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	loadActorAnim(OBJECT_DOOR, "s3r3d3a", 0x12a, 0xb5, 0);

	_roomVar.love.cmnXPosToCureSpock = 0xc9;
	_roomVar.love.cmnYPosToCureSpock = 0xb7;
}

void Room::love2TouchedWarp1() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	_awayMission->timers[3] = 10;
}

void Room::love2Timer3Expired() {
	loadRoomIndex(0, 0);
}

void Room::love2WalkToDoor() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0x127, 0xba, 14);
}

// Triggers door opening
void Room::love2TouchedHotspot0() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR, "s3r3d3", 0x12a, 0xb5, 1);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2DoorReachedOrOpened() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(1, 1);
}

void Room::love2LookAtCabinet() {
	showDescription(TX_LOV2N027);
}

void Room::love2LookAtDoor() {
	showDescription(TX_LOV2N001);
}

void Room::love2LookAtSynthesizer() {
	showDescription(TX_LOV2N028);
}

void Room::love2LookAtAnywhere() {
	showDescription(TX_LOV2N026);
}

void Room::love2LookAtAntigrav() {
	showDescription(TX_LOV2N006);
}

void Room::love2LookAtMccoy() {
	showDescription(TX_LOV2N011);
}

void Room::love2LookAtSpock() {
	showDescription(TX_LOV2N014);
}

void Room::love2LookAtRedshirt() {
	showDescription(TX_LOV2N010);
}

void Room::love2LookAtKirk() {
	showDescription(TX_LOV2N009);
}

void Room::love2LookAtArdak() {
	showDescription(TX_LOV2N045);
	showText(TX_SPEAKER_SPOCK, TX_LOV2_037);
}

void Room::love2LookAtChamber() {
	showDescription(TX_LOV2N029);
	showText(TX_SPEAKER_SPOCK, TX_LOV2_035);
	showText(TX_SPEAKER_MCCOY, TX_LOV2_019);
	showText(TX_SPEAKER_SPOCK, TX_LOV2_033);
	showText(TX_SPEAKER_MCCOY, TX_LOV2_016);
}

void Room::love2LookAtCan1() {
	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		showDescription(TX_LOV2N004);
		break;
	case CANTYPE_H2:
		showDescription(TX_LOV2N002);
		break;
	case CANTYPE_N2:
	default:
		showDescription(TX_LOV2N003);
		break;
	}
}

void Room::love2LookAtCan2() {
	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		showDescription(TX_LOV2N004);
		break;
	case CANTYPE_H2:
		showDescription(TX_LOV2N002);
		break;
	case CANTYPE_N2:
	default:
		showDescription(TX_LOV2N003);
		break;
	}
}

void Room::love2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV2_001);
}

void Room::love2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_LOV2_029);
}

void Room::love2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV2_009);
}

void Room::love2TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV2_041);
}

void Room::love2UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV2_008, false);
	else
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV2_012, false);
}

void Room::love2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_LOV2_027, false);
}

void Room::love2UseKirkOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_KIRK;
	walkCrewman(OBJECT_KIRK, 0x2b, 0xbb, 12);
}

void Room::love2UseSpockOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_SPOCK;
	walkCrewman(OBJECT_SPOCK, 0x2b, 0xbb, 12);
}

void Room::love2UseMccoyOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_MCCOY;
	walkCrewman(OBJECT_MCCOY, 0x2b, 0xbb, 12);
}

void Room::love2UseRedshirtOnCabinet() {
	_roomVar.love.chamberObject = OBJECT_REDSHIRT;
	walkCrewman(OBJECT_REDSHIRT, 0x2b, 0xbb, 12);
}

void Room::love2CrewmanReachedCabinet() {
	Common::String anim = getCrewmanAnimFilename(_roomVar.love.chamberObject, "usehw");
	loadActorAnim2(_roomVar.love.chamberObject, anim, -1, -1, 11);
}

void Room::love2CrewmanAccessedCabinet() {
	if (_awayMission->love.cabinetOpen)
		loadActorAnim2(OBJECT_CABINET, "s3r3d4", 0x1e, 0xaf, 2);
	else
		loadActorAnim2(OBJECT_CABINET, "s3r3d1", 0x1e, 0xaf, 2);

	playSoundEffectIndex(kSfxDoor);

	if (_awayMission->love.cabinetOpen)
		loadActorStandAnim(OBJECT_ANTIGRAV);

	_awayMission->love.cabinetOpen = !_awayMission->love.cabinetOpen;
}

void Room::love2CrewmanOpenedOrClosedCabinet() {
	if (_awayMission->love.cabinetOpen && !haveItem(OBJECT_IANTIGRA))
		loadActorAnim(OBJECT_ANTIGRAV, "antigr", 0x1f, 0xa7, 0);
}

void Room::love2UseWrenchOnGasFeed() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 1);
}

void Room::love2ReachedGasFeed() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 12);
	playVoc("MUR3E1S");
}

void Room::love2ChangedGasFeed() {
	if (_awayMission->love.gasFeedOn) {
		_awayMission->love.gasFeedOn = false;
		loadActorAnim2(OBJECT_GAS_FEED, "s3r3v2", 0xac, 0x75, 0);
		showDescription(TX_LOV2N007);
	} else {
		_awayMission->love.gasFeedOn = true;
		loadActorAnim2(OBJECT_GAS_FEED, "s3r3v1", 0xac, 0x75, 0);
		showDescription(TX_LOV2N008);
	}
}

void Room::love2UseO2GasOnCanisterSlot() {
	_roomVar.love.canisterType = CANTYPE_O2;
	Common::strcpy_s(_roomVar.love.canisterAnim, "o2can");
	_roomVar.love.canisterItem = OBJECT_IO2GAS;
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 3);
}

void Room::love2UseH2GasOnCanisterSlot() {
	_roomVar.love.canisterType = CANTYPE_H2;
	Common::strcpy_s(_roomVar.love.canisterAnim, "h2can");
	_roomVar.love.canisterItem = OBJECT_IH2GAS;
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 3);
}

void Room::love2UseN2GasOnCanisterSlot() {
	_roomVar.love.canisterType = CANTYPE_N2;
	Common::strcpy_s(_roomVar.love.canisterAnim, "n2can");
	_roomVar.love.canisterItem = OBJECT_IN2GAS;
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 3);
}

void Room::love2ReachedCanisterSlot() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, TX_LOV2_026);
	else {
		if (_awayMission->love.canister1 == CANTYPE_NONE)
			loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 13);
		else if (_awayMission->love.canister2 == CANTYPE_NONE)
			loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 14);
		else
			showDescription(TX_LOV2N024);
	}
}

void Room::love2PutCanisterInSlot1() {
	playVoc("SE6FOOD");
	loadActorAnim(OBJECT_CAN1, _roomVar.love.canisterAnim, 0xa7, 0xae, 0);
	_awayMission->love.canister1 = _roomVar.love.canisterType;
	showDescription(TX_LOV2N034);
	loseItem(_roomVar.love.canisterItem);
}

void Room::love2PutCanisterInSlot2() {
	playVoc("SE6FOOD");
	loadActorAnim(OBJECT_CAN2, _roomVar.love.canisterAnim, 0xb1, 0xaf, 0);
	_awayMission->love.canister2 = _roomVar.love.canisterType;
	showDescription(TX_LOV2N035);
	loseItem(_roomVar.love.canisterItem);
}

void Room::love2UseAntigravOnCanister1() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 4);
}

void Room::love2ReachedCanisterSlot1ToGet() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, TX_LOV2_025);
	else {
		loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 15);
		// BUGFIX: original game only played this for canister 2
		playVoc("SE3PLBAT");
	}
}

void Room::love2TookCanister1() {
	showDescription(TX_LOV2N030);
	loadActorStandAnim(OBJECT_CAN1);

	switch (_awayMission->love.canister1) {
	case CANTYPE_O2:
		giveItem(OBJECT_IO2GAS);
		break;
	case CANTYPE_H2:
		giveItem(OBJECT_IH2GAS);
		break;
	case CANTYPE_N2:
	default:
		giveItem(OBJECT_IN2GAS);
		break;
	}

	_awayMission->love.canister1 = CANTYPE_NONE;
}

void Room::love2UseAntigravOnCanister2() {
	walkCrewman(OBJECT_KIRK, 0xa8, 0xb7, 5);
}

void Room::love2ReachedCanisterSlot2ToGet() {
	if (_awayMission->love.gasFeedOn)
		showText(TX_SPEAKER_SPOCK, TX_LOV2_025);
	else {
		loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 16);
		playVoc("SE3PLBAT");
	}
}

void Room::love2TookCanister2() {
	showDescription(TX_LOV2N031);
	loadActorStandAnim(OBJECT_CAN2);

	switch (_awayMission->love.canister2) {
	case CANTYPE_O2:
		giveItem(OBJECT_IO2GAS);
		break;
	case CANTYPE_H2:
		giveItem(OBJECT_IH2GAS);
		break;
	case CANTYPE_N2:
	default:
		giveItem(OBJECT_IN2GAS);
		break;
	}

	_awayMission->love.canister2 = CANTYPE_NONE;
}

void Room::love2UseKirkOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_KIRK;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2UseSpockOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_SPOCK;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2UseMccoyOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_MCCOY;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2UseRedshirtOnSynthesizer() {
	_roomVar.love.chamberObject = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love.chamberObject, 0x46, 0xae, 13);
}

void Room::love2CrewmanReachedSynthesizer() {
	Common::String anim = getCrewmanAnimFilename(_roomVar.love.chamberObject, "usemn");
	loadActorAnim2(_roomVar.love.chamberObject, anim, -1, -1, 17);
}

void Room::love2CrewmanUsedSynthesizer() {
	if (_awayMission->love.gasFeedOn) {
		love2UseSynthesizer();
	} else {
		if (_roomVar.love.cb == 3) {
			if (haveItem(OBJECT_IWRENCH)) {
				showText(TX_SPEAKER_SPOCK, TX_LOV2_005);
				walkCrewman(OBJECT_SPOCK, 0xa8, 0xb7, 16);
			} else
				showText(TX_SPEAKER_SPOCK, TX_LOV2_006);
		} else {
			_roomVar.love.cb++;
			showDescription(TX_LOV2N012);
		}
	}
}


void Room::love2SpockReachedGasFeeds() {
	loadActorAnim2(OBJECT_SPOCK, "susehn", -1, -1, 18);
}

void Room::love2SpockEnabledGasFeeds() {
	_awayMission->love.gasFeedOn = true;

	loadActorAnim2(OBJECT_GAS_FEED, "s3r3v1", 0xac, 0x75, 0);
	showDescription(TX_LOV2N008);
	walkCrewman(OBJECT_SPOCK, 0xc6, 0xbb, 0);

	love2UseSynthesizer();
}

// Synthesizer takes the input and produces some output
void Room::love2UseSynthesizer() {
	if (_awayMission->love.canister1 != 0) {
		if (_awayMission->love.canister2 != 0) {
			int c1 = _awayMission->love.canister1;
			int c2 = _awayMission->love.canister2;
			if (c1 > c2) {
				int tmp = c1;
				c1 = c2;
				c2 = tmp;
			}

			if (c1 == CANTYPE_O2 && c2 == CANTYPE_H2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Romulan Laughing Gas
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", -1, -1, 27); // -> love2ClosedSynthesizerDoorMakingRLG
					playSoundEffectIndex(kSfxDoor);
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Wet goo
					showDescription(TX_LOV2N051);
					showText(TX_SPEAKER_KIRK, TX_LOV2_002);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_CURE_SAMPLE: // Wet goo
					showDescription(TX_LOV2N050);
					showText(TX_SPEAKER_MCCOY, TX_LOV2_021);
					loadActorStandAnim(OBJECT_CURESAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					showText(TX_SPEAKER_SPOCK, TX_LOV2_028);
					break;

				case SYNTHITEM_NONE: // Water
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_H2O;
					Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle3");
					// Produce bottle
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(kSfxDoor);
					break;
				}
			} else if (c1 == CANTYPE_H2 && c2 == CANTYPE_N2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Inert matter
					showDescription(TX_LOV2N049);
					showText(TX_SPEAKER_SPOCK, TX_LOV2_039); // BUGFIX: original didn't play audio
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Colorless goo (with useful information about virus in ammonia)
					showDescription(TX_LOV2N048);
					showText(TX_SPEAKER_SPOCK, TX_LOV2_034); // BUGFIX: original didn't play audio
					showText(TX_SPEAKER_MCCOY, TX_LOV2_023);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_CURE_SAMPLE: // Cure
					loadActorStandAnim(OBJECT_CURESAMPLE);
					loadActorAnim2(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 8); // -> love2ClosedSynthesizerDoorMakingCure
					playSoundEffectIndex(kSfxDoor);
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					showText(TX_SPEAKER_SPOCK, 28);
					break;

				case SYNTHITEM_NONE: // Ammonia
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_NH3;
					Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle2");
					// Produce bottle
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(kSfxDoor);
					break;
				}
			} else if (c1 == CANTYPE_O2 && c2 == CANTYPE_N2) {
				switch (_awayMission->love.synthesizerContents) {
				case SYNTHITEM_PBC: // Inert matter
					showDescription(TX_LOV2N049);
					showText(TX_SPEAKER_SPOCK, TX_LOV2_039); // BUGFIX: original didn't play audio
					loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_VIRUS_SAMPLE: // Wet goo
					showDescription(TX_LOV2N047);
					showText(TX_SPEAKER_MCCOY, TX_LOV2_017);
					loadActorStandAnim(OBJECT_VIRUSSAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_CURE_SAMPLE: // Wet goo
					showDescription(TX_LOV2N050);
					showText(TX_SPEAKER_MCCOY, TX_LOV2_021);
					loadActorStandAnim(OBJECT_CURESAMPLE);
					// Close synthesizer door
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
					playSoundEffectIndex(kSfxDoor);
					_awayMission->love.synthesizerContents = 0;
					break;

				case SYNTHITEM_BOTTLE: // Nothing happens
					showText(TX_SPEAKER_SPOCK, 28);
					break;

				case SYNTHITEM_NONE: // Laughing gas
				default:
					_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_N2O;
					Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle1");
					// Produce bottle
					loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
					playSoundEffectIndex(kSfxDoor);
					break;
				}
			}
		} else {
			playVoc("EFX24");
			showDescription(TX_LOV2N005);
			showText(TX_SPEAKER_MCCOY, TX_LOV2_010);
		}
	} else {
		playVoc("EFX24");
		showDescription(TX_LOV2N005);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_024);
	}
}

void Room::love2SynthesizerDoorClosed() {
	playVoc("LD1SCAN");
	loadActorAnim(OBJECT_SYNTHESIZER_OUTPUT, _roomVar.love.chamberOutputAnim, 0x8a, 0x8d, 4); // -> love2SynthesizerFinished
	_awayMission->love.synthesizerContents = SYNTHITEM_BOTTLE;
}

// Final product of synthesizer is produced
void Room::love2SynthesizerFinished() {
	switch (_awayMission->love.synthesizerBottleIndex) {
	case BOTTLETYPE_N2O:
		showDescription(TX_LOV2N017);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_032);
		showText(TX_SPEAKER_MCCOY, TX_LOV2_015);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_036);
		break;

	case BOTTLETYPE_NH3:
		showDescription(TX_LOV2N016);
		if (!_awayMission->redshirtDead) {
			showText(TX_SPEAKER_FERRIS, TX_LOV2_043);
			showText(TX_SPEAKER_KIRK, TX_LOV2_003);
		}
		break;

	case BOTTLETYPE_H2O:
		showDescription(TX_LOV2N018);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_004);
		break;

	case BOTTLETYPE_RLG:
	default:
		showDescription(TX_LOV2N019);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_031);
		showText(TX_SPEAKER_MCCOY, TX_LOV2_018);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_030);
		showText(TX_SPEAKER_MCCOY, TX_LOV2_022);
		showText(TX_SPEAKER_SPOCK, TX_LOV2_038);
		break;
	}
}

void Room::love2ClosedSynthesizerDoorMakingRLG() {
	_awayMission->love.synthesizerBottleIndex = BOTTLETYPE_RLG;
	Common::strcpy_s(_roomVar.love.chamberOutputAnim, "btle4");
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 3); // -> love2SynthesizerDoorClosed
	playSoundEffectIndex(kSfxDoor);
}

// Synthesizer door closed, using N2 + O2 on cure sample
void Room::love2ClosedSynthesizerDoorMakingCure() {
	loadActorAnim2(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 9); // -> love2CureStartedSynthesizing
}

void Room::love2CureStartedSynthesizing() {
	loadActorAnim(OBJECT_CURE, "cure", 0x8a, 0x8b, 10); // -> love2CureSynthesized
}

void Room::love2CureSynthesized() {
	showDescription(TX_LOV2N046);
	showText(TX_SPEAKER_MCCOY, TX_LOV2_020);
}


// Put polycarbonate in synthesizer
void Room::love2UsePolyberylcarbonateOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 6);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithPolyberylcarbonate() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(TX_LOV2N025);
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 5);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2SynthesizerDoorOpenedWithPolyberylcarbonate() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 19);
}

void Room::love2PutPolyberylcarbonateInSynthesizer() {
	loadActorAnim(OBJECT_POLYBERYLCARBONATE, "pbcanm", 0x8a, 0x8b, 0);
	loseItem(OBJECT_IPBC);
	_awayMission->love.synthesizerContents = SYNTHITEM_PBC;
}


// Put dish (virus sample) in synthesizer
void Room::love2UseVirusSampleOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 7);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithVirusSample() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(TX_LOV2N025);
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 6);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2SynthesizerDoorOpenedWithVirusSample() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 20);
}

void Room::love2PutVirusSampleInSynthesizer() {
	loadActorAnim(OBJECT_VIRUSSAMPLE, "dishes", 0x8a, 0x8b, 0);
	loseItem(OBJECT_IDISHES);
	_awayMission->love.synthesizerContents = SYNTHITEM_VIRUS_SAMPLE;
}


// Put cure sample in synthesizer
void Room::love2UseCureSampleOnSynthesizerDoor() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 8);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2KirkReachedSynthesizerWithCureSample() {
	if (_awayMission->love.synthesizerContents != 0)
		showDescription(TX_LOV2N025); // BUGFIX: original didn't play audio
	else {
		loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d2", 0x8a, 0x8d, 7);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love2SynthesizerDoorOpenedWithCureSample() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 21);
}

void Room::love2PutCureSampleInSynthesizer() {
	loadActorAnim(OBJECT_CURESAMPLE, "dishes", 0x8a, 0x8b, 0);
	loseItem(OBJECT_ISAMPLE);
	_awayMission->love.synthesizerContents = SYNTHITEM_CURE_SAMPLE;
}



void Room::love2UseAnythingOnSynthesizerDoor() {
	showText(TX_SPEAKER_MCCOY, TX_LOV2_014);
}

void Room::love2UseAnythingOnSynthesizer() {
	showText(TX_SPEAKER_SPOCK, TX_LOV2_007);
}

void Room::love2GetCanister() {
	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_MCCOY, TX_LOV2_011);
	else
		showText(TX_SPEAKER_FERRIS, TX_LOV2_040);
}


// Get antigrav from cabinet
void Room::love2GetAntigrav() {
	walkCrewman(OBJECT_KIRK, 0x37, 0xb4, 9);
}

void Room::love2ReachedAntigrav() {
	loadActorAnim2(OBJECT_KIRK, "kusehw", -1, -1, 22);
}

void Room::love2GotAntigrav() {
	loadActorStandAnim(OBJECT_ANTIGRAV);
	giveItem(OBJECT_IANTIGRA);
	showDescription(TX_LOV2N042);
}


// Get polyberylcarbonate from synthesizer
void Room::love2GetPolyberylcarbonate() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 18);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedPolyberylcarbonate() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 26);
}

void Room::love2GotPolyberylcarbonate() {
	loadActorStandAnim(OBJECT_POLYBERYLCARBONATE);
	showDescription(TX_LOV2N044);
	giveItem(OBJECT_IPBC);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
}


// Get virus sample from synthesizer
void Room::love2GetDishes() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 17);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedDishes() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 25);
}

void Room::love2GotDishes() {
	loadActorStandAnim(OBJECT_VIRUSSAMPLE);
	showDescription(TX_LOV2N036); // FIXME: text doesn't match audio
	giveItem(OBJECT_IDISHES);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
}


// Get cure sample from synthesizer
void Room::love2GetSample() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 19);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedSample() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 28);
}

void Room::love2GotSample() {
	loadActorStandAnim(OBJECT_CURESAMPLE);
	showDescription(TX_LOV2N037);
	giveItem(OBJECT_ISAMPLE);
	_awayMission->love.synthesizerContents = 0;
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
}


// Get synthesizer output
void Room::love2GetSynthesizerOutput() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 10);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedSynthesizerOutput() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 23);
}

void Room::love2GotSynthesizerOutput() {
	switch (_awayMission->love.synthesizerBottleIndex) {
	case BOTTLETYPE_N2O:
		showDescription(TX_LOV2N040);
		giveItem(OBJECT_IN2O);
		break;

	case BOTTLETYPE_NH3:
		showDescription(TX_LOV2N039);
		giveItem(OBJECT_INH3);
		break;

	case BOTTLETYPE_H2O:
		showDescription(TX_LOV2N041);
		giveItem(OBJECT_IH2O);
		break;

	case BOTTLETYPE_RLG:
	default:
		showDescription(TX_LOV2N038);
		if (!_awayMission->love.gotTLDH) {
			_awayMission->love.gotTLDH = true;
			_awayMission->love.missionScore += 4;
		}
		giveItem(OBJECT_IRLG);
		break;
	}

	loadActorStandAnim(OBJECT_SYNTHESIZER_OUTPUT);
	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);

	_awayMission->love.synthesizerContents = 0;
	_awayMission->love.synthesizerBottleIndex = 0;
}

// Get the cure
void Room::love2GetCure() {
	walkCrewman(OBJECT_KIRK, 0x85, 0xad, 11);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love2ReachedCure() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 24);
}

void Room::love2GotCure() {
	if (!_awayMission->love.gotCure) {
		_awayMission->love.gotCure = true;
		_awayMission->love.missionScore += 5;
	}

	giveItem(OBJECT_ICURE);
	loadActorStandAnim(OBJECT_CURE);
	showDescription(TX_LOV2N043);

	loadActorAnim(OBJECT_SYNTHESIZER_DOOR, "s3r3d5", 0x8a, 0x8d, 0);
	playSoundEffectIndex(kSfxDoor);
	_awayMission->love.synthesizerContents = 0;
}

}
