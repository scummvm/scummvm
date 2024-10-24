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

// This room is confusing, because it has two hotspots directly adjacent to each other
// that deal with torpedoes, but they act differently.

// BUG: It's possible to swap from having shields up to some other system, by first
// putting shields up, then swapping to another system in the few seconds before the Elasi
// hail the ship.

// BUG: Once you have permission to power down the shields, attempting to power on the
// transporter gives you text along the lines of "the elasi would detect us lowering the
// shields" (even after lowering the shields).

#define OBJECT_DECK 8
#define OBJECT_STRICORDER 9
#define OBJECT_MTRICORDER 10
#define OBJECT_VIEWSCREEN 11
#define OBJECT_DAMAGE_DISPLAY_1 12
#define OBJECT_MAIN_COMPUTER 13
#define OBJECT_TORPEDO_BUTTON 14
#define OBJECT_IMPULSE_CONSOLE 15
#define OBJECT_MIDDLE_CONSOLE 16
#define OBJECT_RIGHT_COMPUTER_PANEL 17
#define OBJECT_DECK_IO_CONSOLE 18
#define OBJECT_CEILING_PANEL 19
#define OBJECT_LEFT_COMPUTER_PANEL 20
#define OBJECT_DAMAGE_DISPLAY_2 21

#define HOTSPOT_TORPEDO_BUTTON 0x20
#define HOTSPOT_TORPEDO_CONTROL 0x21
#define HOTSPOT_DECK_IO_SLOT 0x22
#define HOTSPOT_IMPULSE_CONSOLE 0x23
#define HOTSPOT_MAIN_COMPUTER 0x24
#define HOTSPOT_DECK_IO_CONSOLE 0x25
#define HOTSPOT_MIDDLE_CONSOLE 0x26
#define HOTSPOT_VIEWSCREEN 0x27
#define HOTSPOT_DAMAGE_DISPLAY 0x28

namespace StarTrek {

extern const RoomAction veng2ActionList[] = {
	{ {ACTION_TICK,          1, 0, 0, 0}, &Room::veng2Tick1 },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::veng2Timer0Expired },
	{ {ACTION_TICK,         40, 0, 0, 0}, &Room::veng2Timer0Expired },
	{ {ACTION_DONE_ANIM,     5, 0, 0, 0}, &Room::veng2ElasiShipDecloaked },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::veng2Timer2Expired },
	{ {ACTION_TOUCHED_WARP,  1, 0, 0, 0}, &Room::veng2TouchedDoor },

	{ {ACTION_LOOK, OBJECT_VIEWSCREEN,         0, 0, 0}, &Room::veng2LookAtViewscreen },
	{ {ACTION_LOOK, HOTSPOT_VIEWSCREEN,        0, 0, 0}, &Room::veng2LookAtViewscreen },
	{ {ACTION_LOOK, OBJECT_MIDDLE_CONSOLE,     0, 0, 0}, &Room::veng2LookAtMiddleConsole },
	{ {ACTION_LOOK, HOTSPOT_MIDDLE_CONSOLE,    0, 0, 0}, &Room::veng2LookAtMiddleConsole },
	{ {ACTION_LOOK, OBJECT_DAMAGE_DISPLAY_1,   0, 0, 0}, &Room::veng2LookAtDamageDisplay },
	{ {ACTION_LOOK, OBJECT_DAMAGE_DISPLAY_2,   0, 0, 0}, &Room::veng2LookAtDamageDisplay },
	{ {ACTION_LOOK, HOTSPOT_DAMAGE_DISPLAY,    0, 0, 0}, &Room::veng2LookAtDamageDisplay },
	{ {ACTION_LOOK, OBJECT_STRICORDER,         0, 0, 0}, &Room::veng2LookAtSTricorder },
	{ {ACTION_LOOK, OBJECT_MTRICORDER,         0, 0, 0}, &Room::veng2LookAtMTricorder },
	{ {ACTION_LOOK, OBJECT_TORPEDO_BUTTON,     0, 0, 0}, &Room::veng2LookAtTorpedoButton },
	{ {ACTION_LOOK, HOTSPOT_TORPEDO_BUTTON,    0, 0, 0}, &Room::veng2LookAtTorpedoButton },
	{ {ACTION_LOOK, HOTSPOT_TORPEDO_CONTROL,   0, 0, 0}, &Room::veng2LookAtTorpedoControl },
	{ {ACTION_LOOK, OBJECT_IMPULSE_CONSOLE,    0, 0, 0}, &Room::veng2LookAtImpulseConsole },
	{ {ACTION_LOOK, HOTSPOT_IMPULSE_CONSOLE,   0, 0, 0}, &Room::veng2LookAtImpulseConsole },
	{ {ACTION_LOOK, OBJECT_MAIN_COMPUTER,      0, 0, 0}, &Room::veng2LookAtMainComputer },
	{ {ACTION_LOOK, HOTSPOT_MAIN_COMPUTER,     0, 0, 0}, &Room::veng2LookAtMainComputer },
	{ {ACTION_LOOK, OBJECT_DECK,               0, 0, 0}, &Room::veng2LookAtRecordDeck },
	{ {ACTION_LOOK, OBJECT_DECK_IO_CONSOLE,    0, 0, 0}, &Room::veng2LookAtDeckIOConsole },
	{ {ACTION_LOOK, HOTSPOT_DECK_IO_CONSOLE,   0, 0, 0}, &Room::veng2LookAtDeckIOConsole },
	{ {ACTION_LOOK, HOTSPOT_DECK_IO_SLOT,      0, 0, 0}, &Room::veng2LookAtDeckIOConsole },
	{ {ACTION_LOOK, OBJECT_KIRK,               0, 0, 0}, &Room::veng2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,              0, 0, 0}, &Room::veng2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,              0, 0, 0}, &Room::veng2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,           0, 0, 0}, &Room::veng2LookAtRedshirt },
	{ {ACTION_LOOK, 0xff,                      0, 0, 0}, &Room::veng2LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,               0, 0, 0}, &Room::veng2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,              0, 0, 0}, &Room::veng2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,              0, 0, 0}, &Room::veng2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,           0, 0, 0}, &Room::veng2TalkToRedshirt },

	{ {ACTION_USE, OBJECT_ITECH, OBJECT_DECK_IO_CONSOLE,   0, 0}, &Room::veng2UseTechJournalOnDeckIOConsole },
	{ {ACTION_USE, OBJECT_ITECH, HOTSPOT_DECK_IO_CONSOLE,  0, 0}, &Room::veng2UseTechJournalOnDeckIOConsole },
	{ {ACTION_USE, OBJECT_ITECH, HOTSPOT_DECK_IO_SLOT,     0, 0}, &Room::veng2UseTechJournalOnDeckIOConsole },
	{ {ACTION_DONE_WALK, 22,                            0, 0, 0}, &Room::veng2ReachedDeckIOWithTechJournal },
	{ {ACTION_DONE_ANIM, 23,                            0, 0, 0}, &Room::veng2DoneReadingTechJournal },
	{ {ACTION_USE, OBJECT_IDECK, OBJECT_DECK_IO_CONSOLE,   0, 0}, &Room::veng2UseLogOnDeckIOConsole },
	{ {ACTION_USE, OBJECT_IDECK, HOTSPOT_DECK_IO_CONSOLE,  0, 0}, &Room::veng2UseLogOnDeckIOConsole },
	{ {ACTION_USE, OBJECT_IDECK, HOTSPOT_DECK_IO_SLOT,     0, 0}, &Room::veng2UseLogOnDeckIOConsole },
	{ {ACTION_DONE_WALK, 24,                            0, 0, 0}, &Room::veng2ReachedDeckIOWithLog },
	{ {ACTION_DONE_ANIM, 25,                            0, 0, 0}, &Room::veng2LoadedLog },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK,              0, 0}, &Room::veng2UseCommunicator },

	{ {ACTION_USE, OBJECT_KIRK, OBJECT_TORPEDO_BUTTON,     0, 0}, &Room::veng2UseKirkOnTorpedoButton },
	{ {ACTION_USE, OBJECT_KIRK, HOTSPOT_TORPEDO_BUTTON,    0, 0}, &Room::veng2UseKirkOnTorpedoButton },
	{ {ACTION_DONE_WALK, 18,                            0, 0, 0}, &Room::veng2UseKirkReachedTorpedoButton },
	{ {ACTION_DONE_ANIM, 19,                            0, 0, 0}, &Room::veng2KirkFiredTorpedo },
	{ {ACTION_DONE_ANIM, 7,                             0, 0, 0}, &Room::veng2TorpedoStruckElasiShip },
	{ {ACTION_DONE_ANIM, 6,                             0, 0, 0}, &Room::veng2ElasiRetreated },
	{ {ACTION_DONE_WALK, 1,                             0, 0, 0}, &Room::veng2CrewmanInPositionToBeamOut },
	{ {ACTION_DONE_WALK, 2,                             0, 0, 0}, &Room::veng2CrewmanInPositionToBeamOut },
	{ {ACTION_DONE_WALK, 3,                             0, 0, 0}, &Room::veng2CrewmanInPositionToBeamOut },
	{ {ACTION_DONE_WALK, 4,                             0, 0, 0}, &Room::veng2CrewmanInPositionToBeamOut },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_TORPEDO_BUTTON,   0, 0}, &Room::veng2UseSTricorderOnTorpedoButton }, // FIXME: hotspots?
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_MIDDLE_CONSOLE,   0, 0}, &Room::veng2UseSTricorderOnMiddleConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MIDDLE_CONSOLE,  0, 0}, &Room::veng2UseSTricorderOnMiddleConsole },

	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_TORPEDO_CONTROL, 0, 0}, &Room::veng2UseSTricorderOnTorpedoControl },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_TORPEDO_CONTROL, 0, 0}, &Room::veng2UseSTricorderOnTorpedoControl },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_TORPEDO_CONTROL, 0, 0}, &Room::veng2UseSTricorderOnTorpedoControl },

	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_IMPULSE_CONSOLE,  0, 0}, &Room::veng2UseImpulseConsole },
	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_IMPULSE_CONSOLE,  0, 0}, &Room::veng2UseImpulseConsole },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_IMPULSE_CONSOLE, 0, 0}, &Room::veng2UseImpulseConsole },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_IMPULSE_CONSOLE, 0, 0}, &Room::veng2UseImpulseConsole },
	{ {ACTION_DONE_WALK, 20,                              0, 0, 0}, &Room::veng2SpockReachedImpulseConsole },
	{ {ACTION_DONE_ANIM, 21,                              0, 0, 0}, &Room::veng2SpockUsedImpulseConsole },

	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_MAIN_COMPUTER,  0, 0}, &Room::veng2UseMainComputer },
	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_MAIN_COMPUTER,  0, 0}, &Room::veng2UseMainComputer },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_MAIN_COMPUTER, 0, 0}, &Room::veng2UseMainComputer },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_MAIN_COMPUTER, 0, 0}, &Room::veng2UseMainComputer },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_MAIN_COMPUTER,  0, 0}, &Room::veng2UseSTricorderOnMainComputer },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MAIN_COMPUTER, 0, 0}, &Room::veng2UseSTricorderOnMainComputer },
	{ {ACTION_DONE_WALK, 10,                            0, 0, 0}, &Room::veng2SpockReachedMainComputerToPutTricorder },
	{ {ACTION_DONE_ANIM, 11,                            0, 0, 0}, &Room::veng2AttachedSTricorderToComputer },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MAIN_COMPUTER,  0, 0}, &Room::veng2UseMTricorderOnMainComputer },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_MAIN_COMPUTER, 0, 0}, &Room::veng2UseMTricorderOnMainComputer },
	{ {ACTION_DONE_WALK, 12,                            0, 0, 0}, &Room::veng2MccoyReachedMainComputerToPutTricorder },
	{ {ACTION_DONE_ANIM, 13,                            0, 0, 0}, &Room::veng2AttachedMTricorderToComputer },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DECK_IO_CONSOLE, 0, 0}, &Room::veng2UseSTricorderOnDeckIOConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DECK_IO_SLOT,   0, 0}, &Room::veng2UseSTricorderOnDeckIOConsole },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IMPULSE_CONSOLE,  0, 0}, &Room::veng2UseSTricorderOnImpulseConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_IMPULSE_CONSOLE, 0, 0}, &Room::veng2UseSTricorderOnImpulseConsole },
	{ {ACTION_USE, OBJECT_IFUSION, HOTSPOT_MAIN_COMPUTER, 0, 0}, &Room::veng2UseFusionPackOnMainComputer },
	{ {ACTION_USE, OBJECT_IFUSION, OBJECT_MAIN_COMPUTER,  0, 0}, &Room::veng2UseFusionPackOnMainComputer },

	{ {ACTION_GET, OBJECT_DECK, 0, 0, 0}, &Room::veng2GetLogDeck },
	{ {ACTION_DONE_WALK, 8,     0, 0, 0}, &Room::veng2ReachedLogDeck },
	{ {ACTION_DONE_ANIM, 9,     0, 0, 0}, &Room::veng2PickedUpLogDeck },

	{ {ACTION_GET, OBJECT_STRICORDER, 0, 0, 0}, &Room::veng2GetSTricorder },
	{ {ACTION_DONE_WALK, 14,          0, 0, 0}, &Room::veng2ReachedSTricorderToGet },
	{ {ACTION_DONE_ANIM, 15,          0, 0, 0}, &Room::veng2PickedUpSTricorder },
	{ {ACTION_GET, OBJECT_MTRICORDER, 0, 0, 0}, &Room::veng2GetMTricorder },
	{ {ACTION_DONE_WALK, 16,          0, 0, 0}, &Room::veng2ReachedMTricorderToGet },
	{ {ACTION_DONE_ANIM, 17,          0, 0, 0}, &Room::veng2PickedUpMTricorder },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0, 0}, &Room::vengaLookAtHypo },

	// ENHANCEMENTs
	{ {ACTION_USE, OBJECT_KIRK, HOTSPOT_TORPEDO_CONTROL,     0, 0}, &Room::veng2UseKirkOnTorpedoButton },
	// TODO: uncomment
//	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DECK_IO_CONSOLE, 0, 0}, &Room::veng2UseSTricorderOnDeckIOConsole },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum veng2TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_KIJE, TX_SPEAKER_SCOTT,
	TX_SPEAKER_CHEKOV, TX_SPEAKER_ELASI_CAPTAIN, TX_SPEAKER_PATTERSON, TX_SPEAKER_UHURA, TX_SPEAKER_ANDRADE,
	TX_SPEAKER_ELASI_WEAPONS_MASTER, TX_SPEAKER_ELASI_CREWMAN,
	TX_VEN2_001, TX_VEN2_002, TX_VEN2_003, TX_VEN2_004, TX_VEN2_005,
	TX_VEN2_006, TX_VEN2_007, TX_VEN2_008, TX_VEN2_009, TX_VEN2_010,
	TX_VEN2_011, TX_VEN2_012, TX_VEN2_013, TX_VEN2_014, TX_VEN2_015,
	TX_VEN2_016, TX_VEN2_017, TX_VEN2_018, TX_VEN2_019,	TX_VEN2_020,
	TX_VEN2_021, TX_VEN2_022, TX_VEN2_023, TX_VEN2_024, TX_VEN2_025,
	TX_VEN2_026, TX_VEN2_027, TX_VEN2_028, TX_VEN2_029, TX_VEN2_030,
	TX_VEN2_031, TX_VEN2_032, TX_VEN2_033, TX_VEN2_034, TX_VEN2_035,
	TX_VEN2_036, TX_VEN2_037, TX_VEN2_038, TX_VEN2_039, TX_VEN2_040,
	TX_VEN2_041, TX_VEN2_042, TX_VEN2_043, TX_VEN2_044, TX_VEN2_045,
	TX_VEN2_046, TX_VEN2_047, TX_VEN2_048, TX_VEN2_050, TX_VEN2_051,
	TX_VEN2_052, TX_VEN2_053, TX_VEN2_054, TX_VEN2_055, TX_VEN2_056,
	TX_VEN2_057, TX_VEN2_058, TX_VEN2_059, TX_VEN2_060, TX_VEN2_061,
	TX_VEN2_062, TX_VEN2_063, TX_VEN2_064, TX_VEN2_065, TX_VEN2_066,
	TX_VEN2_067, TX_VEN2_068, TX_VEN2_069, TX_VEN2_070, TX_VEN2_071,
	TX_VEN2_072, TX_VEN2_073, TX_VEN2_074, TX_VEN2_075, TX_VEN2_076,
	TX_VEN2_077, TX_VEN2_078, TX_VEN2_079, TX_VEN2_080, TX_VEN2_081,
	TX_VEN2_082, TX_VEN2_083, TX_VEN2_084, TX_VEN2_085, TX_VEN2_086,
	TX_VEN2_087, TX_VEN2_088, TX_VEN2_089, TX_VEN2_091, TX_VEN2_092,
	TX_VEN2_093, TX_VEN2_094, TX_VEN2_096, TX_VEN2_097, TX_VEN2_098,
	TX_VEN2_099, TX_VEN2_100, TX_VEN2_101, TX_VEN2_102, TX_VEN2_103,
	TX_VEN2_104, TX_VEN2_105, TX_VEN2_106, TX_VEN2_107, TX_VEN2_108,
	TX_VEN2_109, TX_VEN2_110, TX_VEN2_111, TX_VEN2_112, TX_VEN2_113,
	TX_VEN2_114, TX_VEN2_115, TX_VEN2_ELA, TX_VEN2_ENT, TX_VEN2_F28,
	TX_VEN2_F29, TX_VEN2_F35, TX_VEN2_S09, TX_VEN2_SHI, TX_VEN2_TRA,
	TX_VEN2_WEA, TX_VEN4N010, TX_VEN4_017, TX_VEN6N007, TX_VENA_F32,
	TX_VENA_F34, TX_VENA_F40, TX_MUD4_018, TX_VEN2N000, TX_VEN2N001,
	TX_VEN2N002, TX_VEN2N003, TX_VEN2N004, TX_VEN2N005, TX_VEN2N006,
	TX_VEN2N007, TX_VEN2N008, TX_VEN2N009, TX_VEN2N010, TX_VEN2N011,
	TX_VEN2N012, TX_VEN2N013, TX_VEN2N015, TX_VEN2N016, TX_VEN2N017,
	TX_VEN2N018, TX_VEN2N019, TX_VEN2N020, TX_VEN2N021, TX_VEN2N022,
	TX_VEN2N023, TX_VEN2N024, TX_VEN2N025, TX_VEN2N027, TX_VEN2N028,
	TX_VEN2N029, TX_VEN2N030, TX_VEN2N031, TX_VEN2N032, TX_VEN2N033,
	TX_VEN2N034, TX_VEN2N035, TX_VEN2N038, TX_VEN2N039, TX_VEN2U093,
	TX_VEN2U094, TX_VEN2_HAIL_ELASI, TX_VEN2_HAIL_ENT, TX_VEN2_CANCEL,
	TX_VEN2_090
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets veng2TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 20603, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 20614, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 20624, 0, 0, 0 },
	{ TX_SPEAKER_KIJE, 20719, 0, 0, 0 },
	{ TX_SPEAKER_SCOTT, 20634, 0, 0, 0 },
	{ TX_SPEAKER_CHEKOV, 20644, 0, 0, 0 },
	{ TX_SPEAKER_ELASI_CAPTAIN, 20731, 0, 0, 0 },
	{ TX_SPEAKER_PATTERSON, 20677, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 20658, 0, 0, 0 },
	{ TX_SPEAKER_ANDRADE, 20695, 0, 0, 0 },
	{ TX_SPEAKER_ELASI_WEAPONS_MASTER, 20745, 0, 0, 0 },
	{ TX_SPEAKER_ELASI_CREWMAN, 20766, 0, 0, 0 },
	{ TX_VEN2_001, 12332, 0, 0, 0 },
	{ TX_VEN2_002, 15643, 0, 0, 0 },
	{ TX_VEN2_003, 13474, 0, 0, 0 },
	{ TX_VEN2_003, 13295, 0, 0, 0 },
	{ TX_VEN2_004, 6710, 0, 0, 0 },
	{ TX_VEN2_005, 6633, 0, 0, 0 },
	{ TX_VEN2_006, 1862, 0, 0, 0 },
	{ TX_VEN2_007, 7242, 0, 0, 0 },
	{ TX_VEN2_008, 2468, 0, 0, 0 },
	{ TX_VEN2_009, 6954, 0, 0, 0 },
	{ TX_VEN2_010, 1756, 0, 0, 0 },
	{ TX_VEN2_011, 22026, 0, 0, 0 },
	{ TX_VEN2_012, 21967, 0, 0, 0 },
	{ TX_VEN2_013, 11534, 0, 0, 0 },
	{ TX_VEN2_014, 11088, 0, 0, 0 },
	{ TX_VEN2_015, 9563, 0, 0, 0 },
	{ TX_VEN2_015, 22250, 0, 0, 0 },
	{ TX_VEN2_016, 1372, 0, 0, 0 },
	{ TX_VEN2_017, 8039, 0, 0, 0 },
	{ TX_VEN2_018, 15028, 0, 0, 0 },
	{ TX_VEN2_019, 1051, 0, 0, 0 },
	{ TX_VEN2_020, 2081, 0, 0, 0 },
	{ TX_VEN2_021, 2626, 0, 0, 0 },
	{ TX_VEN2_022, 3046, 0, 0, 0 },
	{ TX_VEN2_023, 22533, 0, 0, 0 },
	{ TX_VEN2_024, 15934, 0, 0, 0 },
	{ TX_VEN2_025, 10100, 0, 0, 0 },
	{ TX_VEN2_026, 11332, 0, 0, 0 },
	{ TX_VEN2_027, 1576, 0, 0, 0 },
	{ TX_VEN2_028, 8928, 0, 0, 0 },
	{ TX_VEN2_029, 9654, 0, 0, 0 },
	{ TX_VEN2_029, 22325, 0, 0, 0 },
	{ TX_VEN2_030, 17649, 0, 0, 0 },
	{ TX_VEN2_031, 16748, 0, 0, 0 },
	{ TX_VEN2_032, 1198, 0, 0, 0 },
	{ TX_VEN2_033, 14498, 0, 0, 0 },
	{ TX_VEN2_034, 14871, 0, 0, 0 },
	{ TX_VEN2_035, 13922, 0, 0, 0 },
	{ TX_VEN2_036, 22919, 0, 0, 0 },
	{ TX_VEN2_037, 23214, 0, 0, 0 },
	{ TX_VEN2_038, 17250, 0, 0, 0 },
	{ TX_VEN2_039, 7091, 0, 0, 0 },
	{ TX_VEN2_040, 9261, 0, 0, 0 },
	{ TX_VEN2_041, 7154, 0, 0, 0 },
	{ TX_VEN2_042, 1139, 0, 0, 0 },
	{ TX_VEN2_043, 7956, 0, 0, 0 },
	{ TX_VEN2_044, 3004, 0, 0, 0 },
	{ TX_VEN2_045, 16134, 0, 0, 0 },
	{ TX_VEN2_046, 7637, 0, 0, 0 },
	{ TX_VEN2_047, 18668, 0, 0, 0 },
	{ TX_VEN2_047, 19102, 0, 0, 0 },
	{ TX_VEN2_048, 22718, 0, 0, 0 },
	{ TX_VEN2_050, 13181, 0, 0, 0 },
	{ TX_VEN2_051, 18177, 0, 0, 0 },
	{ TX_VEN2_052, 14307, 0, 0, 0 },
	{ TX_VEN2_053, 22827, 0, 0, 0 },
	{ TX_VEN2_054, 10390, 0, 0, 0 },
	{ TX_VEN2_055, 12792, 0, 0, 0 },
	{ TX_VEN2_056, 18054, 0, 0, 0 },
	{ TX_VEN2_057, 13554, 0, 0, 0 },
	{ TX_VEN2_058, 10271, 0, 0, 0 },
	{ TX_VEN2_059, 16593, 0, 0, 0 },
	{ TX_VEN2_060, 18322, 0, 0, 0 },
	{ TX_VEN2_061, 8589, 0, 0, 0 },
	{ TX_VEN2_062, 12484, 0, 0, 0 },
	{ TX_VEN2_063, 13376, 0, 0, 0 },
	{ TX_VEN2_064, 12699, 0, 0, 0 },
	{ TX_VEN2_065, 12924, 0, 0, 0 },
	{ TX_VEN2_066, 9124, 0, 0, 0 },
	{ TX_VEN2_067, 17887, 0, 0, 0 },
	{ TX_VEN2_068, 13011, 0, 0, 0 },
	{ TX_VEN2_069, 13740, 0, 0, 0 },
	{ TX_VEN2_070, 7834, 0, 0, 0 },
	{ TX_VEN2_071, 15497, 0, 0, 0 },
	{ TX_VEN2_072, 2934, 0, 0, 0 },
	{ TX_VEN2_073, 1295, 0, 0, 0 },
	{ TX_VEN2_074, 16244, 0, 0, 0 },
	{ TX_VEN2_075, 6898, 0, 0, 0 },
	{ TX_VEN2_076, 1100, 0, 0, 0 },
	{ TX_VEN2_077, 15137, 0, 0, 0 },
	{ TX_VEN2_078, 7533, 0, 0, 0 },
	{ TX_VEN2_079, 15779, 0, 0, 0 },
	{ TX_VEN2_080, 15999, 0, 0, 0 },
	{ TX_VEN2_081, 8104, 0, 0, 0 },
	{ TX_VEN2_082, 8295, 0, 0, 0 },
	{ TX_VEN2_083, 21010, 0, 0, 0 },
	{ TX_VEN2_084, 21263, 0, 0, 0 },
	{ TX_VEN2_085, 14671, 0, 0, 0 },
	{ TX_VEN2_086, 7302, 0, 0, 0 },
	{ TX_VEN2_087, 15414, 0, 0, 0 },
	{ TX_VEN2_088, 14226, 0, 0, 0 },
	{ TX_VEN2_089, 14584, 0, 0, 0 },
	{ TX_VEN2_091, 14077, 0, 0, 0 },
	{ TX_VEN2_092, 14813, 0, 0, 0 },
	{ TX_VEN2_093, 995, 0, 0, 0 },
	{ TX_VEN2_094, 7375, 0, 0, 0 },
	{ TX_VEN2_096, 16473, 0, 0, 0 },
	{ TX_VEN2_097, 21609, 0, 0, 0 },
	{ TX_VEN2_098, 19583, 0, 0, 0 },
	{ TX_VEN2_098, 8666, 0, 0, 0 },
	{ TX_VEN2_099, 2384, 0, 0, 0 },
	{ TX_VEN2_100, 9831, 0, 0, 0 },
	{ TX_VEN2_101, 11009, 0, 0, 0 },
	{ TX_VEN2_102, 2733, 0, 0, 0 },
	{ TX_VEN2_103, 1799, 0, 0, 0 },
	{ TX_VEN2_104, 10706, 0, 0, 0 },
	{ TX_VEN2_105, 2512, 0, 0, 0 },
	{ TX_VEN2_106, 11136, 0, 0, 0 },
	{ TX_VEN2_107, 22443, 0, 0, 0 },
	{ TX_VEN2_108, 9448, 0, 0, 0 },
	{ TX_VEN2_109, 1454, 0, 0, 0 },
	{ TX_VEN2_110, 22561, 0, 0, 0 },
	{ TX_VEN2_111, 1956, 0, 0, 0 },
	{ TX_VEN2_112, 2187, 0, 0, 0 },
	{ TX_VEN2_113, 1620, 0, 0, 0 },
	{ TX_VEN2_114, 9953, 0, 0, 0 },
	{ TX_VEN2_115, 11383, 0, 0, 0 },
	{ TX_VEN2_ELA, 22180, 0, 0, 0 },
	{ TX_VEN2_ENT, 22215, 0, 0, 0 },
	{ TX_VEN2_F28, 10925, 0, 0, 0 },
	{ TX_VEN2_F29, 10850, 0, 0, 0 },
	{ TX_VEN2_F35, 11997, 0, 0, 0 },
	{ TX_VEN2_S09, 11713, 0, 0, 0 },
	{ TX_VEN2_SHI, 21908, 0, 0, 0 },
	{ TX_VEN2_TRA, 21931, 0, 0, 0 },
	{ TX_VEN2_WEA, 21885, 0, 0, 0 },
	{ TX_VEN4_017, 6826, 0, 0, 0 },
	{ TX_VENA_F32, 19813, 0, 0, 0 },
	{ TX_VENA_F34, 20083, 0, 0, 0 },
	{ TX_VENA_F40, 19678, 0, 0, 0 },
	//{ TX_DEM6_015, 20930, 0, 0, 0 },
	{ TX_VEN2N000, 4402, 0, 0, 0 },
	{ TX_VEN2N001, 4341, 0, 0, 0 },
	{ TX_VEN2N002, 3318, 0, 0, 0 },
	{ TX_VEN2N003, 5479, 0, 0, 0 },
	{ TX_VEN2N004, 6403, 0, 0, 0 },
	{ TX_VEN2N005, 5275, 0, 0, 0 },
	{ TX_VEN2N006, 5363, 0, 0, 0 },
	{ TX_VEN2N007, 5177, 0, 0, 0 },
	{ TX_VEN2N008, 4856, 0, 0, 0 },
	{ TX_VEN2N009, 4957, 0, 0, 0 },
	{ TX_VEN2N009, 5067, 0, 0, 0 },
	{ TX_VEN2N010, 6159, 0, 0, 0 },
	{ TX_VEN2N011, 20781, 0, 0, 0 },
	{ TX_VEN2N012, 6331, 0, 0, 0 },
	{ TX_VEN2N013, 19468, 0, 0, 0 },
	{ TX_VEN2N015, 19034, 0, 0, 0 },
	{ TX_VEN2N016, 17487, 0, 0, 0 },
	{ TX_VEN2N017, 17096, 0, 0, 0 },
	{ TX_VEN2N018, 20823, 0, 0, 0 },
	{ TX_VEN2N019, 6259, 0, 0, 0 },
	{ TX_VEN2N020, 4671, 0, 0, 0 },
	{ TX_VEN2N021, 4553, 0, 0, 0 },
	{ TX_VEN2N022, 3192, 0, 0, 0 },
	{ TX_VEN2N023, 5574, 0, 0, 0 },
	{ TX_VEN2N024, 4064, 0, 0, 0 },
	{ TX_VEN2N025, 5766, 0, 0, 0 },
	{ TX_VEN2N027, 3963, 0, 0, 0 },
	{ TX_VEN2N027, 5673, 0, 0, 0 },
	{ TX_VEN2N028, 10216, 0, 0, 0 },
	{ TX_VEN2N029, 5898, 0, 0, 0 },
	{ TX_VEN2N030, 12131, 0, 0, 0 },
	{ TX_VEN2N030, 3479, 0, 0, 0 },
	{ TX_VEN2N031, 6071, 0, 0, 0 },
	{ TX_VEN2N031, 5983, 0, 0, 0 },
	{ TX_VEN2N032, 23477, 0, 0, 0 },
	{ TX_VEN2N032, 3824, 0, 0, 0 },
	{ TX_VEN2N033, 4173, 0, 0, 0 },
	{ TX_VEN2N033, 4257, 0, 0, 0 },
	{ TX_VEN2N034, 4778, 0, 0, 0 },
	{ TX_VEN2N035, 6492, 0, 0, 0 },
	{ TX_VEN2N035, 3689, 0, 0, 0 },
	{ TX_VEN2N038, 20875, 0, 0, 0 },
	{ TX_VEN2N039, 18582, 0, 0, 0 },
	{ TX_VEN2U093, 8821, 0, 0, 0 },
	{ TX_VEN2U094, 11259, 0, 0, 0 },
	{ TX_VEN4N010, 20237, 0, 0, 0 },
	{ TX_VEN6N007, 20302, 0, 0, 0 },
	{ TX_MUD4_018, 8758, 0, 0, 0 },
	{ TX_VEN2_HAIL_ELASI, 9004, 0, 0, 0 },
	{ TX_VEN2_HAIL_ENT, 9031, 0, 0, 0 },
	{ TX_VEN2_CANCEL, 9063, 0, 0, 0 },
	//TODO: TX_VEN2_090
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText veng2Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

#define STRICORDER_POS_X 0x45
#define STRICORDER_POS_Y 0x71

#define MTRICORDER_POS_X 0x55
#define MTRICORDER_POS_Y 0x67

#define MAIN_COMPUTER_X 0x4f
#define MAIN_COMPUTER_Y 0x71

#define VIEWSCREEN_X 0x9e
#define VIEWSCREEN_Y 0x63

#define DAMAGE_DISPLAY_1_X 0xfa
#define DAMAGE_DISPLAY_1_Y 0x64

#define TORPEDO_BUTTON_X 0x97
#define TORPEDO_BUTTON_Y 0x74

void Room::veng2Tick1() {
	if (!_awayMission->veng.tookRecordDeckFromAuxilaryControl)
		loadActorAnim2(OBJECT_DECK, "s7r2d1", 0xee, 0x8c);

	if (!haveItem(OBJECT_ISTRICOR))
		loadActorAnim2(OBJECT_STRICORDER, "s7r2t1", STRICORDER_POS_X, STRICORDER_POS_Y);

	if (!haveItem(OBJECT_IMTRICOR))
		loadActorAnim2(OBJECT_MTRICORDER, "s7r2t2", MTRICORDER_POS_X, MTRICORDER_POS_Y);

	if (_awayMission->veng.tricordersPluggedIntoComputer == 3)
		loadActorAnim2(OBJECT_MAIN_COMPUTER, "s7r2t3", MAIN_COMPUTER_X, MAIN_COMPUTER_Y);

	if (_awayMission->veng.elasiShipDecloaked)
		loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2u2", VIEWSCREEN_X, VIEWSCREEN_Y);

	if (_awayMission->veng.poweredSystem == 2)
		loadActorAnim2(OBJECT_DAMAGE_DISPLAY_1, "s7r2sh2", DAMAGE_DISPLAY_1_X, DAMAGE_DISPLAY_1_Y);

	loadActorAnim2(OBJECT_TORPEDO_BUTTON, "s7r2pl", TORPEDO_BUTTON_X, TORPEDO_BUTTON_Y);
	if (_awayMission->veng.torpedoLoaded)
		loadActorAnim2(OBJECT_TORPEDO_BUTTON, "s7r2pm", TORPEDO_BUTTON_X, TORPEDO_BUTTON_Y);

	if (!_awayMission->veng.impulseEnginesOn) {
		loadActorAnim2(OBJECT_IMPULSE_CONSOLE, "s7r2o1", 0x0b, 0x87);
		loadActorAnim2(OBJECT_LEFT_COMPUTER_PANEL, "s7r2o2", 0x36, 0x63);
		loadActorAnim2(OBJECT_RIGHT_COMPUTER_PANEL, "s7r2o3", 0x58, 0x5f);
		loadActorAnim2(OBJECT_MIDDLE_CONSOLE, "s7r2o4", 0x9e, 0x7b);
		loadActorAnim2(OBJECT_DAMAGE_DISPLAY_2, "s7r2o5", 0x112, 0x66);
		loadActorAnim2(OBJECT_DECK_IO_CONSOLE, "s7r2o6", 0x131, 0x86);
		loadActorAnim2(OBJECT_CEILING_PANEL, "s7r2o7", 0x9b, 0x14);

		// FIXME: this doesn't loop
		playVoc("VEN2LOFF");
	} else
		playVoc("VEN2LON");
}

void Room::veng2Timer0Expired() { // Elasi decloak if they haven't already
	if (_awayMission->veng.tricordersPluggedIntoComputer == 3 && _awayMission->veng.impulseEnginesOn) {
		if (!_awayMission->veng.elasiShipDecloaked) {
			_awayMission->veng.elasiShipDecloaked = true;
			playVoc("UNCLOAK");
			loadActorAnimC(OBJECT_VIEWSCREEN, "s7r2u1", VIEWSCREEN_X, VIEWSCREEN_Y, &Room::veng2ElasiShipDecloaked);
			_awayMission->disableInput = true;
		} else if (!_awayMission->veng.elasiHailedRepublic && _awayMission->veng.poweredSystem == 2)
			_awayMission->timers[2] = 45; // Timer until Elasi hail
	}
}

void Room::veng2ElasiShipDecloaked() {
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_KIJE,  TX_VEN2_093);
	showText(TX_SPEAKER_KIRK,  TX_VEN2_019);
	showText(TX_SPEAKER_SPOCK, TX_VEN2_076);
	showText(TX_SPEAKER_MCCOY, TX_VEN2_042);
	if (_awayMission->veng.poweredSystem != 2) {
		showText(TX_SPEAKER_SPOCK, TX_VEN2_032);
		_awayMission->veng.counterUntilElasiBoardWithShieldsDown = 1800;
	} else
		veng2Timer2Expired();
}

void Room::veng2Timer2Expired() { // Elasi hail the Enterprise if they haven't already
	if (_awayMission->veng.elasiHailedRepublic)
		return;
	_awayMission->veng.elasiHailedRepublic = true;

	showText(TX_SPEAKER_SPOCK, TX_VEN2_073);
	showText(TX_SPEAKER_KIRK,  TX_VEN2_016);
	loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2p1", VIEWSCREEN_X, VIEWSCREEN_Y);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_109);
	showText(TX_SPEAKER_KIRK,          TX_VEN2_027);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_113);
	showText(TX_SPEAKER_KIRK,          TX_VEN2_010);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_103);
	showText(TX_SPEAKER_KIRK,          TX_VEN2_006);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_111);
	showText(TX_SPEAKER_KIRK,          TX_VEN2_020);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_112);

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_VEN2_012, TX_VEN2_011,
		TX_END
	};

	int choice = showMultipleTexts(choices);

	if (choice == 0) {
		showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_099);
		showText(TX_SPEAKER_KIRK,          TX_VEN2_008);
	}

	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_105);
	showText(TX_SPEAKER_KIRK,          TX_VEN2_021);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_102);
	showText(TX_SPEAKER_SPOCK,         TX_VEN2_072);
	showText(TX_SPEAKER_MCCOY,         TX_VEN2_044);
	showText(TX_SPEAKER_KIRK,          TX_VEN2_022);

	loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2u2", VIEWSCREEN_X, VIEWSCREEN_Y);
	_awayMission->veng.counterUntilElasiAttack = 27000;
	_awayMission->veng.countdownStarted = true;
}

void Room::veng2TouchedDoor() {
	playSoundEffectIndex(kSfxDoor);
	_awayMission->rdfStillDoDefaultAction = true;
}

void Room::veng2LookAtViewscreen() {
	if (!_awayMission->veng.elasiShipDecloaked)
		showDescription(TX_VEN2N022);
	else
		showDescription(TX_VEN2N002);
}

void Room::veng2LookAtMiddleConsole() {
	showDescription(TX_VEN2N030);
}

void Room::veng2LookAtDamageDisplay() {
	if (!_awayMission->veng.impulseEnginesOn)
		showDescription(TX_VEN2N035);
	else
		showDescription(TX_VEN2N032);

	// NOTE: There were originally 2 more cases, where "poweredSystem == 2" or otherwise. For
	// the case where poweredSystem == 2, it played TX_VEN2N032, but with mismatching text.
}

void Room::veng2LookAtSTricorder() {
	showDescription(TX_VEN2N027);
}

void Room::veng2LookAtMTricorder() {
	showDescription(TX_VEN2N024);
}

void Room::veng2LookAtTorpedoButton() {
	showDescription(TX_VEN2N033);
}

// TODO: Consider merging "veng2LookAtTorpedoButton" with "veng2LookAtTorpedoControl"
// (and the same for use actions)
void Room::veng2LookAtTorpedoControl() {
	showDescription(TX_VEN2N033);

	if (!_awayMission->veng.impulseEnginesOn)
		showDescription(TX_VEN2N001); // Unused, since it can't be selected when impulse is off?
	else if (!_awayMission->veng.torpedoLoaded) {
		showDescription(TX_VEN2N000);
		if (!_awayMission->veng.elasiShipDecloaked)
			showText(TX_SPEAKER_SPOCK, TX_VEN2_036);
		else
			showText(TX_SPEAKER_SPOCK, TX_VEN2_037);
		_awayMission->veng.examinedTorpedoControl = true;
	} else if (!_awayMission->veng.firedTorpedo)
		showDescription(TX_VEN2N021);
	else
		showDescription(TX_VEN2N020); // Unused, since after firing the torpedo, the mission ends
}

void Room::veng2LookAtImpulseConsole() {
	showDescription(TX_VEN2N034);

	if (!_awayMission->veng.impulseEnginesOn)
		showDescription(TX_VEN2N008);
	else if (_awayMission->veng.tricordersPluggedIntoComputer != 3)
		showDescription(TX_VEN2N009);
	else if (_awayMission->veng.poweredSystem == 0)
		showDescription(TX_VEN2N009);
	else if (_awayMission->veng.poweredSystem == 1)
		showDescription(TX_VEN2N007);
	else if (_awayMission->veng.poweredSystem == 2)
		showDescription(TX_VEN2N005);
	else
		showDescription(TX_VEN2N006);
}

void Room::veng2LookAtMainComputer() {
	if (_awayMission->veng.tricordersPluggedIntoComputer == 0)
		showDescription(TX_VEN2N003);
	else if (_awayMission->veng.tricordersPluggedIntoComputer == 1) // Med tricorder plugged in
		showDescription(TX_VEN2N023);
	else if (_awayMission->veng.tricordersPluggedIntoComputer == 2) // Sci tricorder plugged in
		showDescription(TX_VEN2N027);
	else // Both
		showDescription(TX_VEN2N025);
}

void Room::veng2LookAtRecordDeck() {
	showDescription(TX_VEN2N029);
}

void Room::veng2LookAtDeckIOConsole() {
	showDescription(TX_VEN2N031);
}

void Room::veng2LookAtKirk() {
	showDescription(TX_VEN2N010);
}

void Room::veng2LookAtSpock() {
	showDescription(TX_VEN2N019);
}

void Room::veng2LookAtMccoy() {
	showDescription(TX_VEN2N012);
}

void Room::veng2LookAtRedshirt() {
	showDescription(TX_VEN2N004);
}

void Room::veng2LookAnywhere() {
	showDescription(TX_VEN2N035);
}

void Room::veng2TalkToKirk() {
	if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_KIRK, TX_VEN2_004);
	else
		showText(TX_SPEAKER_KIRK, TX_VEN2_005);
}

void Room::veng2TalkToSpock() {
	if (_awayMission->veng.countdownStarted) {
		showText(TX_SPEAKER_SPOCK, TX_VEN2_075);
		showText(TX_SPEAKER_KIRK,  TX_VEN2_009);
	} else
		showText(TX_SPEAKER_SPOCK, TX_VEN4_017);
}

void Room::veng2TalkToMccoy() {
	if (_awayMission->veng.countdownStarted) {
		showText(TX_SPEAKER_MCCOY, TX_VEN2_041);
		showText(TX_SPEAKER_KIRK,  TX_VEN2_007);
	} else
		showText(TX_SPEAKER_MCCOY, TX_VEN2_039);
}

void Room::veng2TalkToRedshirt() {
	if (_awayMission->veng.countdownStarted) {
		showText(TX_SPEAKER_KIJE,  TX_VEN2_094);
		showText(TX_SPEAKER_SPOCK, TX_VEN2_078);
	} else
		showText(TX_SPEAKER_KIJE, TX_VEN2_086);
}

void Room::veng2UseTechJournalOnDeckIOConsole() {
	if (_awayMission->veng.readEngineeringJournal)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_046);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x112, 0x98, &Room::veng2ReachedDeckIOWithTechJournal);
	}
}

void Room::veng2ReachedDeckIOWithTechJournal() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng2DoneReadingTechJournal);
}

void Room::veng2DoneReadingTechJournal() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_VEN2_070);
	showText(TX_SPEAKER_MCCOY, TX_VEN2_043);
	showText(TX_SPEAKER_KIRK,  TX_VEN2_017);
	showText(TX_SPEAKER_SPOCK, TX_VEN2_081);
	_awayMission->veng.readEngineeringJournal = true;
}

void Room::veng2UseLogOnDeckIOConsole() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x112, 0x98, &Room::veng2ReachedDeckIOWithLog);
}

void Room::veng2ReachedDeckIOWithLog() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng2LoadedLog);
}

void Room::veng2LoadedLog() {
	playVoc("COMPUTER");
	_awayMission->disableInput = false;
	_awayMission->veng.captainsLogIndex++;

	if (_awayMission->veng.captainsLogIndex == 1)
		showText(TX_SPEAKER_PATTERSON, TX_VEN2_082);
	else if (_awayMission->veng.captainsLogIndex == 2)
		showText(TX_SPEAKER_PATTERSON, TX_VEN2_083);
	else if (_awayMission->veng.captainsLogIndex == 3)
		showText(TX_SPEAKER_PATTERSON, TX_VEN2_084);
	else if (_awayMission->veng.captainsLogIndex == 4) {
		showText(TX_SPEAKER_ANDRADE, TX_VEN2_097);
		_awayMission->veng.readAllLogs = true;
		showText(TX_SPEAKER_SPOCK, TX_VEN2_061);
	} else
		showText(TX_SPEAKER_SPOCK, TX_VEN2_061);
}

void Room::veng2UseCommunicator() {
	if (_awayMission->veng.toldElasiToBeamOver)
		showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_098);
	else if (_awayMission->veng.countdownStarted) {
		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_VEN2_HAIL_ELASI, TX_VEN2_HAIL_ENT, TX_VEN2_CANCEL,
			TX_END
		};
		int choice = showMultipleTexts(choices);

		if (choice == 0) { // Hail Elasi
			if (!_awayMission->veng.torpedoLoaded)
				showText(TX_SPEAKER_MCCOY, TX_VEN2_040);
			else {
				loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2p1", VIEWSCREEN_X, VIEWSCREEN_Y);
				showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_108);

				const TextRef choices2[] = {
					TX_SPEAKER_KIRK,
					TX_VEN2_015, TX_VEN2_029,
					TX_END
				};
				choice = showMultipleTexts(choices2);

				if (choice == 0) { // "We don't have it yet"
					showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_100);
					loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2u2", VIEWSCREEN_X, VIEWSCREEN_Y);
				} else { // "You'll have to beam over"
					showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_114);
					showText(TX_SPEAKER_KIRK, TX_VEN2_025);
					loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2u2", VIEWSCREEN_X, VIEWSCREEN_Y);
					_awayMission->veng.counterUntilElasiNagToDisableShields = 1800;
					_awayMission->veng.toldElasiToBeamOver = true;
				}
			}
		} else if (choice == 1) // Hail enterprise
			showText(TX_SPEAKER_SPOCK, TX_VEN2_066);
	} else if (_awayMission->veng.elasiShipDecloaked) {
		showText(TX_SPEAKER_SPOCK, TX_VEN2_066);
	} else {
		showText(TX_SPEAKER_KIRK,  TX_MUD4_018);
		showText(TX_SPEAKER_UHURA, TX_VEN2U093);
		showText(TX_SPEAKER_KIRK,  TX_VEN2_028);
	}
}

void Room::veng2UseKirkOnTorpedoButton() {
	if (_awayMission->veng.poweredSystem != 1)
		showDescription(TX_VEN2N028);
	else if (!_awayMission->veng.torpedoLoaded)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_058);
	else if (!_awayMission->veng.elasiShieldsDown)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_054); // Probably unused; weapons can't be powered while elasi shields are up?
	else { // Fire on the defenseless Elasi
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x90, 0x95, &Room::veng2UseKirkReachedTorpedoButton);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	}
}

void Room::veng2UseKirkReachedTorpedoButton() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng2KirkFiredTorpedo);
}

void Room::veng2KirkFiredTorpedo() {
	playVoc("DBLWHAM2");
	loadActorAnimC(OBJECT_VIEWSCREEN, "s7r2ph", 0, 0, &Room::veng2TorpedoStruckElasiShip);
}

void Room::veng2TorpedoStruckElasiShip() {
	loadActorAnim2(OBJECT_VIEWSCREEN, "s7r2p1", VIEWSCREEN_X, VIEWSCREEN_Y);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_104);
	showText(TX_SPEAKER_ELASI_WEAPONS_MASTER, TX_VEN2_F29);
	showText(TX_SPEAKER_ELASI_CREWMAN, TX_VEN2_F28);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_101);
	showText(TX_SPEAKER_KIRK, TX_VEN2_014);
	showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_106);
	loadActorAnimC(OBJECT_VIEWSCREEN, "s7r2rc", 0, 0, &Room::veng2ElasiRetreated);
}

void Room::veng2ElasiRetreated() {
	showText(TX_SPEAKER_UHURA,  TX_VEN2U094);
	showText(TX_SPEAKER_KIRK,   TX_VEN2_026);
	showText(TX_SPEAKER_CHEKOV, TX_VEN2_115);
	showText(TX_SPEAKER_KIRK,   TX_VEN2_013);
	showText(TX_SPEAKER_SCOTT,  TX_VEN2_S09);
	_awayMission->veng.firedTorpedo = true;

	walkCrewmanC(OBJECT_KIRK,     0xec,  0x9d, &Room::veng2CrewmanInPositionToBeamOut);
	walkCrewmanC(OBJECT_SPOCK,    0xd5,  0x96, &Room::veng2CrewmanInPositionToBeamOut);
	walkCrewmanC(OBJECT_MCCOY,    0x101, 0x95, &Room::veng2CrewmanInPositionToBeamOut);
	walkCrewmanC(OBJECT_REDSHIRT, 0xec,  0x8c, &Room::veng2CrewmanInPositionToBeamOut);
}

void Room::veng2CrewmanInPositionToBeamOut() {
	if (++_roomVar.veng.numCrewmenReadyToBeamOut == 4) {
		_awayMission->veng.missionScore = 16;
		if (_awayMission->veng.field68)
			_awayMission->veng.missionScore += 1;
		if (_awayMission->veng.getPointsForHealingBrittany)
			_awayMission->veng.missionScore += 1;
		if (_awayMission->veng.readAllLogs)
			_awayMission->veng.missionScore += 1;
		if (_awayMission->veng.field6b)
			_awayMission->veng.missionScore += 1;
		if (_awayMission->veng.field6c)
			_awayMission->veng.missionScore += 1;

		endMission(_awayMission->veng.missionScore, 21, 0);
	}
}

void Room::veng2UseSTricorderOnTorpedoButton() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_VEN2_F35); // ENHANCEMENT: Use spockScan (don't just show text)
}

void Room::veng2UseSTricorderOnMiddleConsole() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, -1);
	showDescription(TX_VEN2N030); // This is a narration, not Spock speaking
}

void Room::veng2UseSTricorderOnTorpedoControl() {
	showText(TX_SPEAKER_KIRK, TX_VEN2_001);
	spockScan(DIR_N, TX_SPEAKER_SPOCK, -1); // FIXME: delete this since Spock might not have his tricorder?

	if (_awayMission->veng.poweredSystem != 1) { // No power to weapons
		if (!_awayMission->veng.scannedMainComputer)
			showText(TX_SPEAKER_SPOCK, TX_VEN2_062);
		else
			showText(TX_SPEAKER_SPOCK, TX_VEN2_064);
	} else if (_awayMission->veng.tricordersPluggedIntoComputer != 3)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_055);
#if 0
	// FIXME: Unused, since the same condition is above. Fix needed?
	else if (_awayMission->veng.poweredSystem != 1)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_065);
#endif
	else if (!_awayMission->veng.torpedoLoaded) {
		showText(TX_SPEAKER_SPOCK, TX_VEN2_068);
		if (!_awayMission->veng.elasiShipDecloaked)
			showText(TX_SPEAKER_SPOCK, TX_VEN2_036);
		else
			showText(TX_SPEAKER_SPOCK, TX_VEN2_037);
		_awayMission->veng.examinedTorpedoControl = true;
		// TODO: consider making it easier to trigger the above flag? (using spock on it
		// won't work once the elasi are there, since you can't power the system; you need
		// to look at it instead.)
	} else
		showText(TX_SPEAKER_SPOCK, TX_VEN2_050);
}

void Room::veng2UseImpulseConsole() {
	if (!_awayMission->veng.impulseEnginesOn) {
		showText(TX_SPEAKER_KIRK,  TX_VEN2_003);
		showText(TX_SPEAKER_SPOCK, TX_VEN2_063);
	} else if (_awayMission->veng.tricordersPluggedIntoComputer != 3) {
		showText(TX_SPEAKER_KIRK,  TX_VEN2_003);
		showText(TX_SPEAKER_SPOCK, TX_VEN2_057);
	} else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x22, 0x9b, &Room::veng2SpockReachedImpulseConsole);
	}
}

void Room::veng2SpockReachedImpulseConsole() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng2SpockUsedImpulseConsole);
}

void Room::veng2PowerWeapons() {
	if (_awayMission->veng.poweredSystem == 2) {
		playVoc("LD6BMOFF");
		loadActorAnim2(OBJECT_DAMAGE_DISPLAY_1, "s7r2sh3", DAMAGE_DISPLAY_1_X, DAMAGE_DISPLAY_1_Y);
	}
	_awayMission->veng.poweredSystem = 1;
	showText(TX_SPEAKER_KIJE, TX_VEN2_088);
	if (_awayMission->veng.toldElasiToBeamOver) {
		showText(TX_SPEAKER_SPOCK, TX_VEN2_052);
		_awayMission->veng.elasiShieldsDown = true;
		_awayMission->veng.counterUntilElasiBoardWithInvitation = 900;
	}
	if (_awayMission->veng.elasiShipDecloaked && !_awayMission->veng.elasiHailedRepublic) {
		showText(TX_SPEAKER_SPOCK, TX_VEN2_033);
		_awayMission->veng.counterUntilElasiBoardWithShieldsDown = 1800;
	}
}

void Room::veng2SpockUsedImpulseConsole() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_VEN2_069);

	// Choose where to transfer power
	const TextRef choices[] = {
		TX_EMPTY,
		TX_VEN2_WEA, TX_VEN2_SHI, TX_VEN2_TRA,
		TX_END
	};
	int choice = showMultipleTexts(choices);

	if (choice == 0) { // Weapons
		if (_awayMission->veng.toldElasiToBeamOver)
			veng2PowerWeapons();
		else if (_awayMission->veng.countdownStarted)
			showText(TX_SPEAKER_SPOCK, TX_VEN2_035);
		else if (_awayMission->veng.poweredSystem == 1) // Weapons already powered
			showText(TX_SPEAKER_KIJE, TX_VEN2_091);
		else
			veng2PowerWeapons();
	} else if (choice == 1) { // Shields
		if (_awayMission->veng.poweredSystem == 2) // Shields already powered
			showText(TX_SPEAKER_KIJE, TX_VEN2_089);
		else if (_awayMission->veng.toldElasiToBeamOver)
			showText(TX_SPEAKER_KIJE, TX_VEN2_085);
		else {
			loadActorAnim2(OBJECT_DAMAGE_DISPLAY_1, "s7r2sh", DAMAGE_DISPLAY_1_X, DAMAGE_DISPLAY_1_Y);
			_awayMission->veng.poweredSystem = 2;
			if (_awayMission->veng.elasiShipDecloaked)
				_awayMission->timers[2] = 45; // Timer until the elasi hail the ship
			showText(TX_SPEAKER_KIJE, TX_VEN2_092);
		}
	} else if (choice == 2) { // Transporter
		if (_awayMission->veng.countdownStarted) {
			showText(TX_SPEAKER_SPOCK, TX_VEN2_034);
			if (_awayMission->veng.havePowerPack) {
				showText(TX_SPEAKER_KIRK,  TX_VEN2_018);
				showText(TX_SPEAKER_SPOCK, TX_VEN2_077);
			}
		} else if (_awayMission->veng.poweredSystem == 3) // Transporter already powered
			showText(TX_SPEAKER_KIJE, TX_VEN2_090); // BUGFIX: audio didn't play properly
		else {
			if (_awayMission->veng.poweredSystem == 2) {
				playVoc("LD6BMOFF"); // ENHANCEMENT: make this consistent with the weapons code
				loadActorAnim2(OBJECT_DAMAGE_DISPLAY_1, "s7r2sh3", DAMAGE_DISPLAY_1_X, DAMAGE_DISPLAY_1_Y);
			}
			_awayMission->veng.poweredSystem = 3;
			showText(TX_SPEAKER_KIJE, TX_VEN2_087);
			if (_awayMission->veng.elasiShipDecloaked && !_awayMission->veng.elasiHailedRepublic) {
				showText(TX_SPEAKER_SPOCK, TX_VEN2_071);
				_awayMission->veng.counterUntilElasiBoardWithShieldsDown = 1800;
			}
		}
	}

}

void Room::veng2UseMainComputer() {
	showText(TX_SPEAKER_KIRK, TX_VEN2_002);
	if (_awayMission->veng.tricordersPluggedIntoComputer != 3) {
		spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_VEN2_079);
		showText(TX_SPEAKER_KIRK,  TX_VEN2_024);
		showText(TX_SPEAKER_SPOCK, TX_VEN2_080);
		showText(TX_SPEAKER_MCCOY, TX_VEN2_045);
		showText(TX_SPEAKER_SPOCK, TX_VEN2_074);
		showText(TX_SPEAKER_KIJE,  TX_VEN2_096);
		_awayMission->veng.scannedMainComputer = true;
	} else
		showText(TX_SPEAKER_SPOCK, TX_VEN2_059);
}

void Room::veng2UseSTricorderOnMainComputer() {
	if (!_awayMission->veng.scannedMainComputer)
		veng2UseMainComputer();
	else if (!_awayMission->veng.impulseEnginesOn)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_031);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x52, 0x84, &Room::veng2SpockReachedMainComputerToPutTricorder);
	}
}

void Room::veng2SpockReachedMainComputerToPutTricorder() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng2AttachedSTricorderToComputer);
}

void Room::veng2AttachedSTricorderToComputer() {
	playVoc("MUR4E1");
	_awayMission->disableInput = false;
	if (_awayMission->veng.tricordersPluggedIntoComputer != 1) // Mccoy's isn't in yet
		_awayMission->veng.tricordersPluggedIntoComputer = 2;
	else {
		_awayMission->veng.tricordersPluggedIntoComputer = 3;
		_awayMission->timers[0] = 40; // Timer until Elasi appear
	}

	loseItem(OBJECT_ISTRICOR);
	loadActorAnim2(OBJECT_STRICORDER, "s7r2t1", STRICORDER_POS_X, STRICORDER_POS_Y);

	showDescription(TX_VEN2N017);

	walkCrewman(OBJECT_SPOCK, 0x5f, 0xaa);
	if (_awayMission->veng.tricordersPluggedIntoComputer == 3) // Both tricorders plugged in
		veng2BothTricordersPluggedIn();
}

void Room::veng2UseMTricorderOnMainComputer() {
	if (!_awayMission->veng.impulseEnginesOn)
		showText(TX_SPEAKER_MCCOY, TX_VEN2_038);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x52, 0x84, &Room::veng2MccoyReachedMainComputerToPutTricorder);
	}
}

void Room::veng2MccoyReachedMainComputerToPutTricorder() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng2AttachedMTricorderToComputer);
}

void Room::veng2AttachedMTricorderToComputer() {
	playVoc("MUR4E1");
	_awayMission->disableInput = false;
	if (_awayMission->veng.tricordersPluggedIntoComputer != 2) // Spock's isn't in yet
		_awayMission->veng.tricordersPluggedIntoComputer = 1;
	else {
		_awayMission->veng.tricordersPluggedIntoComputer = 3;
		_awayMission->timers[0] = 40; // Timer until Elasi appear
	}

	loseItem(OBJECT_IMTRICOR);
	loadActorAnim2(OBJECT_MTRICORDER, "s7r2t2", MTRICORDER_POS_X, MTRICORDER_POS_Y);

	showDescription(TX_VEN2N016);

	walkCrewman(OBJECT_SPOCK, 0x5f, 0xaa);
	if (_awayMission->veng.tricordersPluggedIntoComputer == 3) // Both tricorders plugged in
		veng2BothTricordersPluggedIn();
}

void Room::veng2BothTricordersPluggedIn() {
	loadActorAnim2(OBJECT_MAIN_COMPUTER, "s7r2t3", MAIN_COMPUTER_X, MAIN_COMPUTER_Y);
	showText(TX_SPEAKER_SPOCK, TX_VEN2_030);
}

void Room::veng2UseSTricorderOnDeckIOConsole() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_VEN2_067);
}

void Room::veng2UseSTricorderOnImpulseConsole() {
	if (!_awayMission->veng.impulseEnginesOn)
		spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_VEN2_056);
	else
		spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_VEN2_051);
}

void Room::veng2UseFusionPackOnMainComputer() {
	showText(TX_SPEAKER_SPOCK, TX_VEN2_060);
}

void Room::veng2GetLogDeck() {
	walkCrewmanC(OBJECT_KIRK, 0xe7, 0x93, &Room::veng2ReachedLogDeck);
	_awayMission->disableInput = true;
}

void Room::veng2ReachedLogDeck() {
	loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::veng2PickedUpLogDeck);
}

void Room::veng2PickedUpLogDeck() {
	loadActorStandAnim(OBJECT_DECK);
	_awayMission->disableInput = false;
	_awayMission->veng.tookRecordDeckFromAuxilaryControl = true;
	showDescription(TX_VEN2N039);
	giveItem(OBJECT_IDECK);
}

void Room::veng2GetSTricorder() {
	if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_047);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x52, 0x84, &Room::veng2ReachedSTricorderToGet);
	}
}

void Room::veng2ReachedSTricorderToGet() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng2PickedUpSTricorder);
}

void Room::veng2PickedUpSTricorder() {
	_awayMission->disableInput = false;
	_awayMission->veng.tricordersPluggedIntoComputer &= ~2;
	giveItem(OBJECT_ISTRICOR);
	loadActorStandAnim(OBJECT_STRICORDER);
	loadActorStandAnim(OBJECT_MAIN_COMPUTER);
	walkCrewman(OBJECT_SPOCK, 0x5f, 0xaa);
	showDescription(TX_VEN2N015);
}

void Room::veng2GetMTricorder() {
	if (_awayMission->veng.countdownStarted)
		showText(TX_SPEAKER_SPOCK, TX_VEN2_047);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x52, 0x84, &Room::veng2ReachedMTricorderToGet);
	}
}

void Room::veng2ReachedMTricorderToGet() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::veng2PickedUpMTricorder);
}

void Room::veng2PickedUpMTricorder() {
	_awayMission->disableInput = false;
	_awayMission->veng.tricordersPluggedIntoComputer &= ~1;
	giveItem(OBJECT_IMTRICOR);
	loadActorStandAnim(OBJECT_MTRICORDER);
	loadActorStandAnim(OBJECT_MAIN_COMPUTER);
	walkCrewman(OBJECT_SPOCK, 0x5f, 0xaa);
	showDescription(TX_VEN2N013);
}

}
