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

#define OBJECT_SERVICE_PANEL 8
#define OBJECT_INSULATION 9
#define OBJECT_WRENCH 10
#define OBJECT_GRATE 11
#define OBJECT_GAS_TANK 12

#define HOTSPOT_ENGINEERING_PANEL 0x20
#define HOTSPOT_MONITOR 0x21
#define HOTSPOT_SHAFT 0x22
#define HOTSPOT_TURBINES 0x23

namespace StarTrek {

extern const RoomAction love3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::love3Tick1 },
	{ {ACTION_TICK, 80, 0, 0, 0}, &Room::love3Tick80 },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0, 0}, &Room::love3TouchedWarp1 },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0}, &Room::love3Timer3Expired },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::love3LookAtServicePanel },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::love3LookAtMonitor },
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::love3LookAtWrench },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::love3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::love3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::love3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love3LookAtRedshirt },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::love3LookAnywhere },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::love3LookAtShaft },
	{ {ACTION_LOOK, 11, 0, 0, 0}, &Room::love3LookAtGrate },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::love3LookAtPanel },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::love3LookAtPanel },
	{ {ACTION_LOOK, 12, 0, 0, 0}, &Room::love3LookAtGasTank },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::love3LookAtTurbines },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::love3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::love3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::love3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love3TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::love3UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::love3UseSTricorderOnTurbines },
	{ {ACTION_USE, OBJECT_ISTRICOR, 9, 0, 0}, &Room::love3UseSTricorderOnInsulation },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::love3UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_KIRK, 8, 0, 0}, &Room::love3UseKirkOnServicePanel },
	{ {ACTION_USE, OBJECT_SPOCK, 8, 0, 0}, &Room::love3UseSpockOnServicePanel },
	{ {ACTION_USE, OBJECT_MCCOY, 8, 0, 0}, &Room::love3UseMccoyOnServicePanel },
	{ {ACTION_USE, OBJECT_REDSHIRT, 8, 0, 0}, &Room::love3UseRedshirtOnServicePanel },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::love3CrewmanReachedServicePanel },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::love3OpenedOrClosedServicePanel },

	{ {ACTION_USE, OBJECT_IWRENCH, 12, 0, 0}, &Room::love3UseWrenchOnGasTank },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::love3ReachedGasTankToUnscrew },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::love3ScrewedOrUnscrewedGasTank },

	{ {ACTION_USE, OBJECT_IWRENCH, OBJECT_IO2GAS, 0, 0}, &Room::love3UseWrenchOnO2OrH2Gas },
	{ {ACTION_USE, OBJECT_IWRENCH, OBJECT_IH2GAS, 0, 0}, &Room::love3UseWrenchOnO2OrH2Gas },

	{ {ACTION_USE, OBJECT_IWRENCH, OBJECT_IN2GAS, 0, 0}, &Room::love3UseWrenchOnN2Gas },
	{ {ACTION_FINISHED_WALKING, 15, 0, 0, 0}, &Room::love3ReachedGasTankToPutDown },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::love3PutN2TankBack },

	{ {ACTION_USE, OBJECT_IWRENCH, 0x20, 0, 0}, &Room::love3UseWrenchOnEngineeringPanel },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::love3ReachedEngineeringPanel },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::love3OpenedEngineeringPanel },

	{ {ACTION_USE, OBJECT_IWRENCH, 0x21, 0, 0}, &Room::love3UseWrenchOnMonitor },

	{ {ACTION_USE, OBJECT_IH2O, 0x21, 0, 0}, &Room::love3UseWaterOnMonitor },
	{ {ACTION_FINISHED_WALKING, 14, 0, 0, 0}, &Room::love3ReachedMonitorToUseWater },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::love3PouredWaterOnMonitor },

	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_KIRK, 11, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_SPOCK, 11, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_MCCOY, 11, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },
	{ {ACTION_USE, OBJECT_REDSHIRT, 11, 0, 0}, &Room::love3UseCrewmanOnEngineeringPanelOrGrate },

	{ {ACTION_USE, OBJECT_IWRENCH, 11, 0, 0}, &Room::love3UseWrenchOnGrate },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::love3ReachedGateWithWrench },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0, 0}, &Room::love3OpenedOrClosedGrate },

	{ {ACTION_USE, OBJECT_KIRK, 0x22, 0, 0}, &Room::love3UseCrewmanOnShaft },
	{ {ACTION_USE, OBJECT_SPOCK, 0x22, 0, 0}, &Room::love3UseCrewmanOnShaft },
	{ {ACTION_USE, OBJECT_MCCOY, 0x22, 0, 0}, &Room::love3UseCrewmanOnShaft },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x22, 0, 0}, &Room::love3UseCrewmanOnShaft },

	{ {ACTION_USE, OBJECT_IH2O, 0x22, 0, 0}, &Room::love3UseWaterOnShaft },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::love3ReachedShaftUsingWater },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0, 0}, &Room::love3PouredWaterDownShaft },

	{ {ACTION_USE, OBJECT_IN2O, 0x22, 0, 0}, &Room::love3UseNitrousOxideOnShaft },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::love3ReachedShaftUsingNitrousOxide },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::love3PouredNitrousOxideDownShaft },

	{ {ACTION_USE, OBJECT_INH3, 0x22, 0, 0}, &Room::love3UseAmmoniaOnShaft },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0, 0}, &Room::love3ReachedShaftUsingAmmonia },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0, 0}, &Room::love3PouredAmmoniaDownShaft },

	{ {ACTION_USE, OBJECT_IRLG, 0x22, 0, 0}, &Room::love3UseRomulanLaughingGasOnShaft },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0, 0}, &Room::love3ReachedShaftUsingRomulanLaughingGas },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0, 0}, &Room::love3PouredRomulanLaughingGasDownShaft },

	{ {ACTION_GET, 10, 0, 0, 0}, &Room::love3GetWrench },
	{ {ACTION_FINISHED_WALKING, 9, 0, 0, 0}, &Room::love3ReachedWrenchToGet },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0, 0}, &Room::love3PickedUpWrench },

	{ {ACTION_GET, 12, 0, 0, 0}, &Room::love3GetGasTank },
	{ {ACTION_FINISHED_WALKING, 10, 0, 0, 0}, &Room::love3ReachedGasTankToGet },

	{ {ACTION_USE, OBJECT_IANTIGRA, 12, 0, 0}, &Room::love3UseAntigravOnGasTank },
	{ {ACTION_FINISHED_WALKING, 11, 0, 0, 0}, &Room::love3ReachedGasTankUsingAntigrav },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0, 0}, &Room::love3PickedUpGasTank },

	{ {ACTION_GET, 9, 0, 0, 0}, &Room::love3GetInsulation },
	{ {ACTION_FINISHED_WALKING, 12, 0, 0, 0}, &Room::love3ReachedInsulationToGet },
	{ {ACTION_FINISHED_ANIMATION, 14, 0, 0, 0}, &Room::love3PickedUpInsulation },

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

enum love3TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_FERRIS,
	TX_LOV0_011, TX_LOV0_012, TX_LOV0_013, TX_LOV0_014, TX_LOV0_015,
	TX_LOV0_016, TX_LOV0_017, TX_LOV0_019, TX_LOV0_020, TX_LOV0_021,
	TX_LOV0_022, TX_LOV0_025, TX_LOV0_026, TX_LOV0_029, TX_LOV0_033,
	TX_LOV0_039, TX_LOV0_040, TX_LOV0_041, TX_LOV0_042, TX_LOV0_043,
	TX_LOV0_045, TX_LOV0_046, TX_LOV0_047, TX_LOV0_048, TX_LOV0_049,
	TX_LOV0_050, TX_LOV0_101, TX_LOV0_102, TX_LOV0_103, TX_LOV0_104,
	TX_LOV0_105, TX_LOV0_106, TX_LOV0_107, TX_LOV0_124, TX_LOV3_001,
	TX_LOV3_002, TX_LOV3_003, TX_LOV3_004, TX_LOV3_005, TX_LOV3_006,
	TX_LOV3_007, TX_LOV3_008, TX_LOV3_009, TX_LOV3_010, TX_LOV3_011,
	TX_LOV3_012, TX_LOV3_013, TX_LOV3_014, TX_LOV3_015, TX_LOV3_016,
	TX_LOV3_017, TX_LOV3_018, TX_LOV3_019, TX_LOV3_020, TX_LOV3_021,
	TX_LOV3_022, TX_LOV3_023, TX_LOV3_024, TX_LOV3_025, TX_LOV3_026,
	TX_LOV5_015, TX_LOV5_019, TX_LOV5_027, TX_LOV5_030, TX_LOV5_038,
	TX_LOVA_100, TX_LOVA_F01, TX_LOVA_F02, TX_LOVA_F03, TX_LOVA_F04,
	TX_LOVA_F07, TX_LOVA_F08, TX_LOVA_F10, TX_LOVA_F54, TX_LOVA_F55,
	TX_MUD2_040, TX_MUD4_018, TX_TUG2_010, TX_VENA_F41, TX_BRIDU146,
	TX_GENER004, TX_LOV2N005, TX_LOV3N000, TX_LOV3N001, TX_LOV3N002,
	TX_LOV3N003, TX_LOV3N004, TX_LOV3N005, TX_LOV3N006, TX_LOV3N007,
	TX_LOV3N008, TX_LOV3N009, TX_LOV3N010, TX_LOV3N011, TX_LOV3N012,
	TX_LOV3N013, TX_LOV3N014, TX_LOV3N015, TX_LOV3N016, TX_LOV3N017,
	TX_LOV3N018, TX_LOV3N019, TX_LOV3N020, TX_LOV3N021, TX_LOV3N022,
	TX_LOV3N023, TX_LOV3N024, TX_LOV3N025, TX_LOV3N026, TX_LOV3N027,
	TX_LOV3N028, TX_LOV3N029, TX_LOV3N030, TX_LOV3N031, TX_LOV3N032,
	TX_LOV3NA08, TX_LOV3NA09, TX_LOV3NA20, TX_LOV3NA21, TX_LOV3NA22,
	TX_LOV3NA23, TX_LOV3NJ32, TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets love3TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 9959, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 9970, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 9980, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 9990, 0, 0, 0 },
	{ TX_LOV0_011, 12469, 0, 0, 0 },
	{ TX_LOV0_012, 12596, 0, 0, 0 },
	{ TX_LOV0_013, 12892, 0, 0, 0 },
	{ TX_LOV0_014, 12656, 0, 0, 0 },
	{ TX_LOV0_015, 12760, 0, 0, 0 },
	{ TX_LOV0_016, 12563, 0, 0, 0 },
	{ TX_LOV0_017, 13026, 0, 0, 0 },
	{ TX_LOV0_019, 12946, 0, 0, 0 },
	{ TX_LOV0_020, 12503, 0, 0, 0 },
	{ TX_LOV0_021, 12814, 0, 0, 0 },
	{ TX_LOV0_022, 12704, 0, 0, 0 },
	{ TX_LOV0_025, 13952, 0, 0, 0 },
	{ TX_LOV0_026, 13825, 0, 0, 0 },
	{ TX_LOV0_029, 13705, 0, 0, 0 },
	{ TX_LOV0_033, 13765, 0, 0, 0 },
	{ TX_LOV0_039, 13087, 0, 0, 0 },
	{ TX_LOV0_040, 13214, 0, 0, 0 },
	{ TX_LOV0_041, 13510, 0, 0, 0 },
	{ TX_LOV0_042, 13274, 0, 0, 0 },
	{ TX_LOV0_043, 13378, 0, 0, 0 },
	{ TX_LOV0_045, 13181, 0, 0, 0 },
	{ TX_LOV0_046, 13644, 0, 0, 0 },
	{ TX_LOV0_047, 13564, 0, 0, 0 },
	{ TX_LOV0_048, 13121, 0, 0, 0 },
	{ TX_LOV0_049, 13432, 0, 0, 0 },
	{ TX_LOV0_050, 13322, 0, 0, 0 },
	{ TX_LOV0_101, 14033, 0, 0, 0 },
	{ TX_LOV0_102, 14097, 0, 0, 0 },
	{ TX_LOV0_103, 14167, 0, 0, 0 },
	{ TX_LOV0_104, 14217, 0, 0, 0 },
	{ TX_LOV0_105, 14274, 0, 0, 0 },
	{ TX_LOV0_106, 14386, 0, 0, 0 },
	{ TX_LOV0_107, 14454, 0, 0, 0 },
	{ TX_LOV0_124, 13902, 0, 0, 0 },
	{ TX_LOV3_001, 1598, 0, 0, 0 },
	{ TX_LOV3_002, 11502, 0, 0, 0 },
	{ TX_LOV3_003, 11007, 0, 0, 0 },
	{ TX_LOV3_004, 11136, 0, 0, 0 },
	{ TX_LOV3_005, 11251, 0, 0, 0 },
	{ TX_LOV3_006, 2083, 0, 0, 0 },
	{ TX_LOV3_007, 14507, 0, 0, 0 },
	{ TX_LOV3_008, 1826, 0, 0, 0 },
	{ TX_LOV3_009, 10712, 0, 0, 0 },
	{ TX_LOV3_010, 684, 0, 0, 0 },
	{ TX_LOV3_011, 5079, 0, 0, 0 },
	{ TX_LOV3_012, 4619, 0, 0, 0 },
	{ TX_LOV3_013, 11334, 0, 0, 0 },
	{ TX_LOV3_014, 11378, 0, 0, 0 },
	{ TX_LOV3_015, 6046, 0, 0, 0 },
	{ TX_LOV3_016, 3156, 0, 0, 0 },
	{ TX_LOV3_017, 1696, 0, 0, 0 },
	{ TX_LOV3_018, 11891, 0, 0, 0 },
	{ TX_LOV3_019, 10794, 0, 0, 0 },
	{ TX_LOV3_020, 851, 0, 0, 0 },
	{ TX_LOV3_021, 3714, 0, 0, 0 },
	{ TX_LOV3_022, 2953, 0, 0, 0 },
	{ TX_LOV3_023, 4928, 0, 0, 0 },
	{ TX_LOV3_024, 1960, 0, 0, 0 },
	{ TX_LOV3_025, 5869, 0, 0, 0 },
	{ TX_LOV3_026, 10954, 0, 0, 0 },
	{ TX_LOV5_015, 7076, 0, 0, 0 },
	//{ TX_LOV5_015, 7255, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV5_019, 6998, 0, 0, 0 },
	{ TX_LOV5_027, 7511, 0, 0, 0 },
	{ TX_LOV5_030, 7415, 0, 0, 0 },
	{ TX_LOV5_038, 7466, 0, 0, 0 },
	{ TX_LOVA_100, 8620, 0, 0, 0 },
	{ TX_LOVA_F01, 8062, 0, 0, 0 },
	{ TX_LOVA_F02, 8134, 0, 0, 0 },
	{ TX_LOVA_F03, 8333, 0, 0, 0 },
	{ TX_LOVA_F04, 8248, 0, 0, 0 },
	{ TX_LOVA_F07, 8697, 0, 0, 0 },
	{ TX_LOVA_F08, 7686, 0, 0, 0 },
	//{ TX_LOVA_F08, 8401, 0, 0, 0 }, // ignore duplicate line
	{ TX_LOVA_F10, 8568, 0, 0, 0 },
	{ TX_LOVA_F54, 7842, 0, 0, 0 },
	{ TX_LOVA_F55, 7143, 0, 0, 0 },
	{ TX_MUD2_040, 8893, 0, 0, 0 },
	{ TX_MUD4_018, 9435, 0, 0, 0 },
	//{ TX_MUD4_018, 9266, 0, 0, 0 },	// ignore duplicate line
	//{ TX_MUD4_018, 9599, 0, 0, 0 },	// ignore duplicate line
	//{ TX_MUD4_018, 9102, 0, 0, 0 },	// ignore duplicate line
	{ TX_TUG2_010, 8498, 0, 0, 0 },
	{ TX_VENA_F41, 9389, 0, 0, 0 },
	//{ TX_VENA_F41, 9722, 0, 0, 0 },	// ignore duplicate line
	//{ TX_VENA_F41, 9225, 0, 0, 0 },	// ignore duplicate line
	//{ TX_VENA_F41, 9558, 0, 0, 0 },	// ignore duplicate line
	{ TX_BRIDU146, 9668, 0, 0, 0 },
	//{ TX_BRIDU146, 9335, 0, 0, 0 },	// ignore duplicate line
	//{ TX_BRIDU146, 9504, 0, 0, 0 },	// ignore duplicate line
	//{ TX_BRIDU146, 9171, 0, 0, 0 },	// ignore duplicate line
	{ TX_GENER004, 7991, 0, 0, 0 },
	{ TX_LOV2N005, 8756, 0, 0, 0 },
	//{ TX_LOV2N005, 8982, 0, 0, 0 },	// ignore duplicate line
	//{ TX_LOV2N005, 8836, 0, 0, 0 },	// ignore duplicate line
	{ TX_LOV3N000, 10223, 0, 0, 0 },	// typo: the audio file is wrong
	{ TX_LOV3N001, 4852, 0, 0, 0 },
	{ TX_LOV3N002, 10260, 0, 0, 0 },
	{ TX_LOV3N003, 10324, 0, 0, 0 },
	{ TX_LOV3N004, 10475, 0, 0, 0 },
	{ TX_LOV3N005, 10381, 0, 0, 0 },
	{ TX_LOV3N006, 1503, 0, 0, 0 },
	{ TX_LOV3N007, 5476, 0, 0, 0 },
	{ TX_LOV3N008, 10423, 0, 0, 0 },
	{ TX_LOV3N009, 4422, 0, 0, 0 },
	{ TX_LOV3N010, 5947, 0, 0, 0 },
	{ TX_LOV3N011, 4277, 0, 0, 0 },
	{ TX_LOV3N012, 2895, 0, 0, 0 },
	{ TX_LOV3N013, 3371, 0, 0, 0 },
	{ TX_LOV3N014, 5550, 0, 0, 0 },
	{ TX_LOV3N015, 3060, 0, 0, 0 },
	{ TX_LOV3N016, 12045, 0, 0, 0 },
	{ TX_LOV3N017, 1374, 0, 0, 0 },
	{ TX_LOV3N018, 4564, 0, 0, 0 },
	{ TX_LOV3N019, 1272, 0, 0, 0 },
	{ TX_LOV3N020, 2580, 0, 0, 0 },
	{ TX_LOV3N021, 11748, 0, 0, 0 },
	{ TX_LOV3N022, 2675, 0, 0, 0 },
	{ TX_LOV3N023, 2631, 0, 0, 0 },
	{ TX_LOV3N024, 10535, 0, 0, 0 },
	{ TX_LOV3N025, 12110, 0, 0, 0 },
	{ TX_LOV3N026, 10600, 0, 0, 0 },
	{ TX_LOV3N027, 11699, 0, 0, 0 },
	{ TX_LOV3N028, 11606, 0, 0, 0 },
	{ TX_LOV3N029, 11650, 0, 0, 0 },
	{ TX_LOV3N030, 3914, 0, 0, 0 },
	{ TX_LOV3N031, 11563, 0, 0, 0 },
	{ TX_LOV3N032, 4188, 0, 0, 0 },
	{ TX_LOV3NA08, 12359, 0, 0, 0 },
	{ TX_LOV3NA09, 12156, 0, 0, 0 },
	{ TX_LOV3NA20, 10123, 0, 0, 0 },
	{ TX_LOV3NA21, 1187, 0, 0, 0 },
	{ TX_LOV3NA22, 10002, 0, 0, 0 },
	{ TX_LOV3NA23, 1089, 0, 0, 0 },
	{ TX_LOV3NJ32, 5767, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 14528, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText love3Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::love3Tick1() {
	playVoc("LOV3LOOP");

	if (!_awayMission->love.wrenchTaken)
		loadActorAnim(OBJECT_WRENCH, "wrench", 0x10a, 0xb6, 0);

	if (_awayMission->love.grateRemoved)
		loadActorAnim(OBJECT_GRATE, "s3r4g3", 0, 0, 0);
	else
		loadActorAnim(OBJECT_GRATE, "s3r4g1", 0x116, 0x8f, 0);

	if (_awayMission->love.insulationOnGround)
		loadActorAnim(OBJECT_INSULATION, "s3r4p2", 0, 0, 0);

	if (!_awayMission->love.tookN2TankFromServicePanel)
		loadActorAnim(OBJECT_GAS_TANK, "r4n2", 0, 0, 0);

	if (_awayMission->love.servicePanelOpen)
		loadActorAnim(OBJECT_SERVICE_PANEL, "s3r4d1a", 1, 0, 0);
	else
		loadActorAnim(OBJECT_SERVICE_PANEL, "s3r4d2", 1, 0, 0);

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	_roomVar.love.cmnXPosToCureSpock = 0xb4;
	_roomVar.love.cmnYPosToCureSpock = 0xb7;
}

void Room::love3Tick80() {
	if (!_awayMission->love.visitedRoomWithRomulans && haveItem(OBJECT_IRLG)) {
		showText(TX_SPEAKER_MCCOY, TX_LOV3_010);
		showText(TX_SPEAKER_SPOCK, TX_LOV3_020);
	}
}

void Room::love3TouchedWarp1() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	_awayMission->timers[3] = 10;
}

void Room::love3Timer3Expired() {
	loadRoomIndex(1, 0);
}

void Room::love3LookAtServicePanel() {
	showDescription(TX_LOV3NA20);
}

void Room::love3LookAtMonitor() {
	showDescription(TX_LOV3N000); // BUGFIX: original didn't play the audio
}

void Room::love3LookAtWrench() {
	showDescription(TX_LOV3N002);
}

void Room::love3LookAtKirk() {
	showDescription(TX_LOV3N003);
}

void Room::love3LookAtMccoy() {
	showDescription(TX_LOV3N005);
}

void Room::love3LookAtSpock() {
	showDescription(TX_LOV3N008);
}

void Room::love3LookAtRedshirt() {
	showDescription(TX_LOV3N004);
}

void Room::love3LookAnywhere() {
	showDescription(TX_LOV3NA23);
}

void Room::love3LookAtShaft() {
	showDescription(TX_LOV3NA21);
}

void Room::love3LookAtGrate() {
	showDescription(TX_LOV3N019);
}

void Room::love3LookAtPanel() {
	if (_awayMission->love.insulationOnGround)
		showDescription(TX_LOV3N017);
	else
		showDescription(TX_LOV3NA22);
}

void Room::love3LookAtGasTank() {
	showDescription(TX_LOV3N006);
}

void Room::love3LookAtTurbines() {
	showDescription(TX_LOV3N026);
	showText(TX_SPEAKER_MCCOY, TX_LOV3_009);
	showText(TX_SPEAKER_SPOCK, TX_LOV3_019);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, TX_LOV3_026);
}

void Room::love3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV3_001);
}

void Room::love3TalkToSpock() {
	if (_awayMission->love.visitedRoomWithRomulans)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_003);
	else if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_017);
	else if (haveItem(OBJECT_IRLG))
		showText(TX_SPEAKER_SPOCK, TX_LOV3_004);
	else
		showText(TX_SPEAKER_SPOCK, TX_LOV3_003);
}

void Room::love3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV3_008);
}

void Room::love3TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV3_024);
}

void Room::love3UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV3_005, false);
	else
		mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_LOV3_006, false);
}

void Room::love3UseSTricorderOnTurbines() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_LOV3_014, false);
}

void Room::love3UseSTricorderOnInsulation() {
	if (_awayMission->love.insulationOnGround)
		spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_LOV3_002, false);
}

void Room::love3UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_LOV3_013, false);
}

void Room::love3UseKirkOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_KIRK;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseSpockOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_SPOCK;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseMccoyOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_MCCOY;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseRedshirtOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3CrewmanReachedServicePanel() {
	Common::String anim = getCrewmanAnimFilename(_roomVar.love.activeCrewman, "usehe");
	loadActorAnim2(_roomVar.love.activeCrewman, anim, -1, -1, 1);
}

void Room::love3OpenedOrClosedServicePanel() {
	if (_awayMission->love.servicePanelOpen)
		loadActorAnim2(OBJECT_SERVICE_PANEL, "s3r4d1b", 1, 0, 0);
	else
		loadActorAnim2(OBJECT_SERVICE_PANEL, "s3r4d1", 1, 0, 0);

	playVoc("SMADOOR1");
	_awayMission->love.servicePanelOpen = !_awayMission->love.servicePanelOpen;

	if (_awayMission->love.servicePanelOpen) {
		if (_awayMission->love.tookN2TankFromServicePanel)
			showDescription(TX_LOV3N023);
		else
			showDescription(TX_LOV3N022);
	} else
		showDescription(TX_LOV3N020);
}

// Use wrench on gas tank, to screw or unscrew it in position
void Room::love3UseWrenchOnGasTank() {
	walkCrewman(OBJECT_KIRK, 0x10e, 0xae, 2);
}

void Room::love3ReachedGasTankToUnscrew() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 2);
	playVoc("MUR3E1S");
}

void Room::love3ScrewedOrUnscrewedGasTank() {
	if (!_awayMission->love.gasTankUnscrewed) {
		showDescription(TX_LOV3N012);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_FERRIS, TX_LOV3_022);
		_awayMission->love.gasTankUnscrewed = true;
	} else {
		showDescription(TX_LOV3N015);
		_awayMission->love.gasTankUnscrewed = false;
	}
}


void Room::love3UseWrenchOnO2OrH2Gas() {
	showText(TX_SPEAKER_SPOCK, TX_LOV3_016);
}


// Use wrench on N2 gas = putting it from the inventory back to the service panel
void Room::love3UseWrenchOnN2Gas() {
	walkCrewman(OBJECT_KIRK, 0x10e, 0xae, 15);
}

void Room::love3ReachedGasTankToPutDown() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 3);
	playVoc("MUR3E1S");
}

void Room::love3PutN2TankBack() {
	showDescription(TX_LOV3N013);
	_awayMission->love.tookN2TankFromServicePanel = false;
	loseItem(OBJECT_IN2GAS);
	loadActorAnim(OBJECT_GAS_TANK, "r4n2", 0, 0, 0);
}


// Use wrench on engineering panel, insulation comes out
void Room::love3UseWrenchOnEngineeringPanel() {
	walkCrewman(OBJECT_KIRK, 0x37, 0xb4, 3);
}

void Room::love3ReachedEngineeringPanel() {
	loadActorAnim2(OBJECT_KIRK, "kuselw", -1, -1, 4);
	playVoc("MUR3E1S");
}

void Room::love3OpenedEngineeringPanel() {
	if (!_awayMission->love.insulationOnGround) {
		playVoc("MADR4E4A");
		loadActorAnim(OBJECT_INSULATION, "s3r4p1", 0, 0, 0);
		_awayMission->love.insulationOnGround = true;
	}
	showDescription(TX_LOV3N021);
}


void Room::love3UseWrenchOnMonitor() {
	showText(TX_SPEAKER_SPOCK, TX_LOV3_018);
}


void Room::love3UseWaterOnMonitor() {
	walkCrewmanC(OBJECT_KIRK, 0xc8, 0xb2, &Room::love3ReachedMonitorToUseWater);
}

void Room::love3ReachedMonitorToUseWater() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::love3PouredWaterOnMonitor);
}

void Room::love3PouredWaterOnMonitor() {
	// BUGFIX: the speaker should be Lt. Ferris. In the original, the speaker wasn't
	// specified.
	// However, there is no substitute text for when Lt. Ferris is dead.
	showText(TX_SPEAKER_FERRIS, TX_LOV3_021);

	loseItem(OBJECT_IH2O);
}


void Room::love3UseCrewmanOnEngineeringPanelOrGrate() {
	showDescription(TX_LOV3N030);
}


void Room::love3UseWrenchOnGrate() {
	walkCrewmanC(OBJECT_KIRK, 0xf6, 0xaa, &Room::love3ReachedGateWithWrench);
	_awayMission->disableInput = true;
}

void Room::love3ReachedGateWithWrench() {
	if (_awayMission->love.grateRemoved)
		loadActorAnimC(OBJECT_KIRK, "s3r4g2a", -1, -1, &Room::love3OpenedOrClosedGrate);
	else
		loadActorAnimC(OBJECT_KIRK, "s3r4g2",  -1, -1, &Room::love3OpenedOrClosedGrate);
	loadActorStandAnim(OBJECT_GRATE);
	playVoc("MUR3E1S");
}

void Room::love3OpenedOrClosedGrate() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);

	if (!_awayMission->love.grateRemoved) {
		_awayMission->love.grateRemoved = true;
		loadActorAnim(OBJECT_GRATE, "s3r4g3", 0, 0, 0);
		showDescription(TX_LOV3N016);
		if (!_awayMission->love.gotPointsForOpeningGrate) {
			_awayMission->love.gotPointsForOpeningGrate = true;
			_awayMission->love.missionScore++;
		}
	} else {
		loadActorAnim(OBJECT_GRATE, "s3r4g1", 0x116, 0x8f, 0);
		_awayMission->love.grateRemoved = false;
		showDescription(TX_LOV3N032);
	}
}


void Room::love3UseCrewmanOnShaft() {
	showDescription(TX_LOV3N011);
}


void Room::love3UseWaterOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingWater);
}

void Room::love3ReachedShaftUsingWater() {
	if (!_awayMission->love.grateRemoved)
		showDescription(TX_LOV3N025);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredWaterDownShaft);
}

void Room::love3PouredWaterDownShaft() {
	showDescription(TX_LOV3N009);
	loseItem(OBJECT_IH2O);
}


void Room::love3UseNitrousOxideOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingNitrousOxide);
}

void Room::love3ReachedShaftUsingNitrousOxide() {
	if (!_awayMission->love.grateRemoved)
		showDescription(TX_LOV3N025);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredNitrousOxideDownShaft);
}

void Room::love3PouredNitrousOxideDownShaft() {
	showDescription(TX_LOV3N018);
	showText(TX_SPEAKER_SPOCK, TX_LOV3_012);
	loseItem(OBJECT_IN2O);
}


void Room::love3UseAmmoniaOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingAmmonia);
}

void Room::love3ReachedShaftUsingAmmonia() {
	if (!_awayMission->love.grateRemoved)
		showDescription(TX_LOV3N025);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredAmmoniaDownShaft);
}

void Room::love3PouredAmmoniaDownShaft() {
	if (!_awayMission->love.romulansUnconsciousFromVirus) {
		showDescription(TX_LOV3N001);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_FERRIS, TX_LOV3_023);
	}

	loseItem(OBJECT_INH3);
}


void Room::love3UseRomulanLaughingGasOnShaft() {
	if (_awayMission->love.gotPointsForGassingRomulans) {
		// BUG-ish: you could get the points for this by starting the action, then
		// canceling it before Kirk reaches the shaft.
		_awayMission->love.gotPointsForGassingRomulans = true;
		_awayMission->love.missionScore += 6;
	}
	if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_011);
	else {
		walkCrewmanC(OBJECT_KIRK, 0xf6, 0xa4, &Room::love3ReachedShaftUsingRomulanLaughingGas);
	}
}

void Room::love3ReachedShaftUsingRomulanLaughingGas() {
	if (!_awayMission->love.grateRemoved)
		showDescription(TX_LOV3N025);
	else {
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredRomulanLaughingGasDownShaft);
		playVoc("EFX24");
	}
}

void Room::love3PouredRomulanLaughingGasDownShaft() {
	loseItem(OBJECT_IRLG);
	_awayMission->love.romulansUnconsciousFromLaughingGas = true;

	if (_awayMission->love.romulansUnconsciousFromVirus) {
		showDescription(TX_LOV3NA08);
		_awayMission->love.romulansUnconsciousFromLaughingGas = false;
	} else {
		showDescription(TX_LOV3NA09);
		playVoc("ROMULANL");
	}
}


// Get the wrench on the ground
void Room::love3GetWrench() {
	walkCrewmanC(OBJECT_KIRK, 0x104, 0xb6, &Room::love3ReachedWrenchToGet);
}

void Room::love3ReachedWrenchToGet() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::love3PickedUpWrench);
}

void Room::love3PickedUpWrench() {
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IWRENCH);
	loadActorStandAnim(OBJECT_WRENCH);
	_awayMission->love.wrenchTaken = true;
	showDescription(TX_LOV3N031);
}


// Get the N2 gas tank in the service panel (fails due to weight)
void Room::love3GetGasTank() {
	walkCrewmanC(OBJECT_KIRK, 0x121, 0xb7, &Room::love3ReachedGasTankToGet);
}

void Room::love3ReachedGasTankToGet() {
	if (_awayMission->love.gasTankUnscrewed)
		showDescription(TX_LOV3N007);
	else
		showDescription(TX_LOV3N014);
}


// Use antigrav unit N2 gas tank in the service panel (succeeds)
void Room::love3UseAntigravOnGasTank() {
	walkCrewmanC(OBJECT_KIRK, 0x10e, 0xae, &Room::love3ReachedGasTankUsingAntigrav);
}

void Room::love3ReachedGasTankUsingAntigrav() {
	if (_awayMission->love.gasTankUnscrewed) {
		loadActorAnimC(OBJECT_KIRK, "kusehe", -1, -1, &Room::love3PickedUpGasTank);
		playVoc("SE3PLBAT");
	} else {
		showDescription(TX_LOV3N010);
		showText(TX_SPEAKER_SPOCK, TX_LOV3_015);
	}
}

void Room::love3PickedUpGasTank() {
	giveItem(OBJECT_IN2GAS);
	loadActorStandAnim(OBJECT_GAS_TANK);
	_awayMission->love.tookN2TankFromServicePanel = true;

	showDescription(TX_LOV3NJ32);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, TX_LOV3_025);
}

void Room::love3GetInsulation() {
	walkCrewmanC(OBJECT_KIRK, 0x32, 0xbe, &Room::love3ReachedInsulationToGet);
}

void Room::love3ReachedInsulationToGet() {
	if (!_awayMission->love.field3c)
		loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::love3PickedUpInsulation);
}

void Room::love3PickedUpInsulation() {
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IINSULAT);
	showDescription(TX_LOV3N029);
}

}
