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

#define OBJECT_8 8
#define OBJECT_BEAM 9
#define OBJECT_10 10
#define OBJECT_11 11
#define OBJECT_12 12
#define OBJECT_13 13
#define OBJECT_REDGEM1 14
#define OBJECT_REDGEM2 15
#define OBJECT_REDGEM3 16
#define OBJECT_GRNGEM1 17
#define OBJECT_GRNGEM2 18
#define OBJECT_GRNGEM3 19
#define OBJECT_BLUGEM1 20
#define OBJECT_BLUGEM2 21
#define OBJECT_BLUGEM3 22
#define OBJECT_VLICT 23

#define HOTSPOT_HOLE1 0x21
#define HOTSPOT_HOLE2 0x22
#define HOTSPOT_HOLE3 0x23
#define HOTSPOT_INTERFACE 0x24 // Yellow light

#define REDGEM1_X 0x82
#define REDGEM1_Y 0x98
#define REDGEM2_X 0x81
#define REDGEM2_Y 0x9e
#define REDGEM3_X 0x80
#define REDGEM3_Y 0xa5

#define GRNGEM1_X 0x97
#define GRNGEM1_Y 0x98
#define GRNGEM2_X 0x97
#define GRNGEM2_Y 0x9e
#define GRNGEM3_X 0x97
#define GRNGEM3_Y 0xa5

#define BLUGEM1_X 0xab
#define BLUGEM1_Y 0x98
#define BLUGEM2_X 0xac
#define BLUGEM2_Y 0x9e
#define BLUGEM3_X 0xad
#define BLUGEM3_Y 0xa5

#define COMBO(a,b,c) (holes[0] == a && holes[1] == b && holes[2] == c)

namespace StarTrek {

extern const RoomAction trial5ActionList[] = {
	{ {ACTION_TICK,  1,     0, 0, 0}, &Room::trial5Tick1 },
	{ {ACTION_TICK, 60,     0, 0, 0}, &Room::trial5Tick60 },
	{ {ACTION_DONE_ANIM, 1, 0, 0, 0}, &Room::trial5VlictAppeared },
	{ {ACTION_USE, OBJECT_KIRK, HOTSPOT_INTERFACE, 0, 0}, &Room::trial5WalkToInterface },
	{ {ACTION_WALK, HOTSPOT_INTERFACE,          0, 0, 0}, &Room::trial5WalkToInterface },
	{ {ACTION_DONE_WALK, 2,                     0, 0, 0}, &Room::trial5ReachedInterface },
	{ {ACTION_DONE_WALK, 30,                    0, 0, 0}, &Room::trial5ReachedGem },
	{ {ACTION_DONE_ANIM, 31,                    0, 0, 0}, &Room::trial5PickedUpGem },

	{ {ACTION_GET, OBJECT_REDGEM1, 0, 0, 0}, &Room::trial5GetRedGem1 },
	{ {ACTION_GET, OBJECT_REDGEM2, 0, 0, 0}, &Room::trial5GetRedGem2 },
	{ {ACTION_GET, OBJECT_REDGEM3, 0, 0, 0}, &Room::trial5GetRedGem3 },
	{ {ACTION_GET, OBJECT_GRNGEM1, 0, 0, 0}, &Room::trial5GetGreenGem1 },
	{ {ACTION_GET, OBJECT_GRNGEM2, 0, 0, 0}, &Room::trial5GetGreenGem2 },
	{ {ACTION_GET, OBJECT_GRNGEM3, 0, 0, 0}, &Room::trial5GetGreenGem3 },
	{ {ACTION_GET, OBJECT_BLUGEM1, 0, 0, 0}, &Room::trial5GetBlueGem1 },
	{ {ACTION_GET, OBJECT_BLUGEM2, 0, 0, 0}, &Room::trial5GetBlueGem2 },
	{ {ACTION_GET, OBJECT_BLUGEM3, 0, 0, 0}, &Room::trial5GetBlueGem3 },

	{ {ACTION_LOOK, OBJECT_KIRK,       0, 0, 0}, &Room::trial5LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,      0, 0, 0}, &Room::trial5LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,      0, 0, 0}, &Room::trial5LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,   0, 0, 0}, &Room::trial5LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_BEAM,       0, 0, 0}, &Room::trial5LookAtBeam },
	{ {ACTION_LOOK, HOTSPOT_INTERFACE, 0, 0, 0}, &Room::trial5LookAtInterface },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_INTERFACE, 0, 0}, &Room::trial5UseSTricorderOnInterface },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_INTERFACE, 0, 0}, &Room::trial5UseMTricorderOnInterface },

	{ {ACTION_LOOK, OBJECT_REDGEM1, 0, 0, 0}, &Room::trial5LookAtRedGem1 },
	{ {ACTION_LOOK, OBJECT_REDGEM2, 0, 0, 0}, &Room::trial5LookAtRedGem2 },
	{ {ACTION_LOOK, OBJECT_REDGEM3, 0, 0, 0}, &Room::trial5LookAtRedGem3 },
	{ {ACTION_LOOK, OBJECT_GRNGEM1, 0, 0, 0}, &Room::trial5LookAtGreenGem1 },
	{ {ACTION_LOOK, OBJECT_GRNGEM2, 0, 0, 0}, &Room::trial5LookAtGreenGem2 },
	{ {ACTION_LOOK, OBJECT_GRNGEM3, 0, 0, 0}, &Room::trial5LookAtGreenGem3 },
	{ {ACTION_LOOK, OBJECT_BLUGEM1, 0, 0, 0}, &Room::trial5LookAtBlueGem1 },
	{ {ACTION_LOOK, OBJECT_BLUGEM2, 0, 0, 0}, &Room::trial5LookAtBlueGem2 },
	{ {ACTION_LOOK, OBJECT_BLUGEM3, 0, 0, 0}, &Room::trial5LookAtBlueGem3 },

	{ {ACTION_LOOK, HOTSPOT_HOLE1, 0, 0, 0}, &Room::trial5LookAtHole },
	{ {ACTION_LOOK, HOTSPOT_HOLE2, 0, 0, 0}, &Room::trial5LookAtHole },
	{ {ACTION_LOOK, HOTSPOT_HOLE3, 0, 0, 0}, &Room::trial5LookAtHole },

	{ {ACTION_TALK, OBJECT_KIRK,       0, 0, 0}, &Room::trial5TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,      0, 0, 0}, &Room::trial5TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,      0, 0, 0}, &Room::trial5TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,   0, 0, 0}, &Room::trial5TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0, 0}, &Room::trial5UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0, 0}, &Room::trial5UsePhaser },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::trial5UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::trial5UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ICOMM,    0xff, 0, 0}, &Room::trial5UseCommunicator },

	{ {ACTION_DONE_WALK, 3, 0, 0, 0},  &Room::trial5ReachedHoleToPutGem },
	{ {ACTION_DONE_ANIM, 32, 0, 0, 0}, &Room::trial5CrewmanBeamedOut },

	{ {ACTION_USE, OBJECT_IREDGEM1, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseRedGem1OnHole1 },
	{ {ACTION_USE, OBJECT_IREDGEM2, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseRedGem2OnHole1 },
	{ {ACTION_USE, OBJECT_IREDGEM3, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseRedGem3OnHole1 },
	{ {ACTION_USE, OBJECT_IGRNGEM1, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseGreenGem1OnHole1 },
	{ {ACTION_USE, OBJECT_IGRNGEM2, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseGreenGem2OnHole1 },
	{ {ACTION_USE, OBJECT_IGRNGEM3, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseGreenGem3OnHole1 },
	{ {ACTION_USE, OBJECT_IBLUGEM1, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseBlueGem1OnHole1 },
	{ {ACTION_USE, OBJECT_IBLUGEM2, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseBlueGem2OnHole1 },
	{ {ACTION_USE, OBJECT_IBLUGEM3, HOTSPOT_HOLE1, 0, 0}, &Room::trial5UseBlueGem3OnHole1 },

	{ {ACTION_USE, OBJECT_IREDGEM1, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseRedGem1OnHole2 },
	{ {ACTION_USE, OBJECT_IREDGEM2, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseRedGem2OnHole2 },
	{ {ACTION_USE, OBJECT_IREDGEM3, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseRedGem3OnHole2 },
	{ {ACTION_USE, OBJECT_IGRNGEM1, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseGreenGem1OnHole2 },
	{ {ACTION_USE, OBJECT_IGRNGEM2, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseGreenGem2OnHole2 },
	{ {ACTION_USE, OBJECT_IGRNGEM3, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseGreenGem3OnHole2 },
	{ {ACTION_USE, OBJECT_IBLUGEM1, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseBlueGem1OnHole2 },
	{ {ACTION_USE, OBJECT_IBLUGEM2, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseBlueGem2OnHole2 },
	{ {ACTION_USE, OBJECT_IBLUGEM3, HOTSPOT_HOLE2, 0, 0}, &Room::trial5UseBlueGem3OnHole2 },

	{ {ACTION_USE, OBJECT_IREDGEM1, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseRedGem1OnHole3 },
	{ {ACTION_USE, OBJECT_IREDGEM2, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseRedGem2OnHole3 },
	{ {ACTION_USE, OBJECT_IREDGEM3, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseRedGem3OnHole3 },
	{ {ACTION_USE, OBJECT_IGRNGEM1, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseGreenGem1OnHole3 },
	{ {ACTION_USE, OBJECT_IGRNGEM2, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseGreenGem2OnHole3 },
	{ {ACTION_USE, OBJECT_IGRNGEM3, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseGreenGem3OnHole3 },
	{ {ACTION_USE, OBJECT_IBLUGEM1, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseBlueGem1OnHole3 },
	{ {ACTION_USE, OBJECT_IBLUGEM2, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseBlueGem2OnHole3 },
	{ {ACTION_USE, OBJECT_IBLUGEM3, HOTSPOT_HOLE3, 0, 0}, &Room::trial5UseBlueGem3OnHole3 },

	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0}, &Room::trial5UseMedkitAnywhere },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum trial5TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_BENNIE, TX_SPEAKER_UHURA,
	TX_SPEAKER_BIALBI, TX_SPEAKER_VLICT, TX_SPEAKER_VOICE, TX_SPEAKER_LIGHT_OF_WAR,
	TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_SPEAKER_LIGHT_OF_TRAVEL,
	TX_TRI5_001, TX_TRI5_002, TX_TRI5_003, TX_TRI5_004, TX_TRI5_005,
	TX_TRI5_006, TX_TRI5_007, TX_TRI5_008, TX_TRI5_009, TX_TRI5_010,
	TX_TRI5_011, TX_TRI5_012, TX_TRI5_013, TX_TRI5_014, TX_TRI5_015,
	TX_TRI5_016, TX_TRI5_017, TX_TRI5_018, TX_TRI5_019, TX_TRI5_020,
	TX_TRI5_021, TX_TRI5_022, TX_TRI5_023, TX_TRI5_024, TX_TRI5_025,
	TX_TRI5_026, TX_TRI5_027, TX_TRI5_028, TX_TRI5_029, TX_TRI5_030,
	TX_TRI5_031, TX_TRI5_032, TX_TRI5_033, TX_TRI5_034, TX_TRI5_035,
	TX_TRI5_036, TX_TRI5_037, TX_TRI5_038, TX_TRI5_039, TX_TRI5_040,
	TX_TRI5_041, TX_TRI5_042, TX_TRI5_043, TX_TRI5_044, TX_TRI5_045,
	TX_TRI5_046, TX_TRI5_047, TX_TRI5_048, TX_TRI5_049, TX_TRI5_050,
	TX_TRI5_051, TX_TRI5_052, TX_TRI5N002, TX_TRI5N004, TX_TRI5N005,
	TX_TRI5N006, TX_TRI5N007, TX_TRI5N008, TX_TRI5N009, TX_TRI5N010,
	TX_TRI5N011, TX_TRI5N012, TX_TRI5N013, TX_TRI5N014, TX_TRI5N015,
	TX_TRI5N016, TX_TRI5N017, TX_TRI5N018, TX_TRI5N019, TX_TRI5N020,
	TX_TRI5N021
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets trial5TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 275, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 286, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 296, 0, 0, 0 },
	{ TX_SPEAKER_BENNIE, 306, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 320, 0, 0, 0 },
	{ TX_SPEAKER_BIALBI, 393, 0, 0, 0 },
	{ TX_SPEAKER_VLICT, 400, 0, 0, 0 },
	{ TX_SPEAKER_VOICE, 378, 0, 0, 0 },
	{ TX_SPEAKER_LIGHT_OF_WAR, 330, 0, 0, 0 },
	{ TX_SPEAKER_LIGHT_OF_KNOWLEDGE, 343, 0, 0, 0 },
	{ TX_SPEAKER_LIGHT_OF_TRAVEL, 362, 0, 0, 0 },
	{ TX_TRI5_001, 7210, 0, 0, 0 },
	{ TX_TRI5_002, 1082, 0, 0, 0 },
	{ TX_TRI5_003, 1947, 0, 0, 0 },
	{ TX_TRI5_004, 3104, 0, 0, 0 },
	{ TX_TRI5_005, 3664, 0, 0, 0 },
	{ TX_TRI5_006, 2153, 0, 0, 0 },
	{ TX_TRI5_007, 2591, 0, 0, 0 },
	{ TX_TRI5_008, 2043, 0, 0, 0 },
	{ TX_TRI5_009, 2488, 0, 0, 0 },
	{ TX_TRI5_010, 5768, 0, 0, 0 },
	{ TX_TRI5_011, 5944, 0, 0, 0 },
	{ TX_TRI5_012, 14859, 0, 0, 0 },
	{ TX_TRI5_013, 7481, 0, 0, 0 },
	{ TX_TRI5_014, 1010, 0, 0, 0 },
	{ TX_TRI5_015, 2409, 0, 0, 0 },
	{ TX_TRI5_016, 7317, 0, 0, 0 },
	{ TX_TRI5_017, 943, 0, 0, 0 },
	{ TX_TRI5_018, 1917, 0, 0, 0 },
	{ TX_TRI5_019, 2950, 0, 0, 0 },
	{ TX_TRI5_020, 1447, 0, 0, 0 },
	{ TX_TRI5_021, 3487, 0, 0, 0 },
	{ TX_TRI5_022, 1699, 0, 0, 0 },
	{ TX_TRI5_023, 2313, 0, 0, 0 },
	{ TX_TRI5_024, 4276, 0, 0, 0 },
	{ TX_TRI5_025, 3004, 0, 0, 0 },
	{ TX_TRI5_026, 3285, 0, 0, 0 },
	{ TX_TRI5_027, 1251, 0, 0, 0 },
	{ TX_TRI5_028, 3554, 0, 0, 0 },
	{ TX_TRI5_029, 1755, 0, 0, 0 },
	{ TX_TRI5_030, 3179, 0, 0, 0 },
	{ TX_TRI5_031, 1500, 0, 0, 0 },
	{ TX_TRI5_032, 4001, 0, 0, 0 },
	{ TX_TRI5_033, 3830, 0, 0, 0 },
	{ TX_TRI5_034, 2752, 0, 0, 0 },
	{ TX_TRI5_035, 4215, 0, 0, 0 },
	{ TX_TRI5_036, 13018, 0, 0, 0 },
	{ TX_TRI5_037, 13608, 0, 0, 0 },
	{ TX_TRI5_038, 13684, 0, 0, 0 },
	{ TX_TRI5_039, 13969, 0, 0, 0 },
	//{ TX_TRI5_039, 12698, 0, 0, 0 },	// Ignore duplicate line
	{ TX_TRI5_040, 13379, 0, 0, 0 },
	{ TX_TRI5_041, 13860, 0, 0, 0 },
	{ TX_TRI5_042, 14630, 0, 0, 0 },
	{ TX_TRI5_043, 14770, 0, 0, 0 },
	{ TX_TRI5_044, 14223, 0, 0, 0 },
	{ TX_TRI5_045, 14131, 0, 0, 0 },
	{ TX_TRI5_046, 14343, 0, 0, 0 },
	{ TX_TRI5_047, 13301, 0, 0, 0 },
	{ TX_TRI5_048, 13150, 0, 0, 0 },
	{ TX_TRI5_049, 13071, 0, 0, 0 },
	{ TX_TRI5_050, 12880, 0, 0, 0 },
	{ TX_TRI5_051, 7549, 0, 0, 0 },
	{ TX_TRI5_052, 897, 0, 0, 0 },
	{ TX_TRI5N002, 5594, 0, 0, 0 },
	{ TX_TRI5N004, 5661, 0, 0, 0 },
	{ TX_TRI5N005, 5313, 0, 0, 0 },
	{ TX_TRI5N006, 7119, 0, 0, 0 },
	{ TX_TRI5N007, 7905, 0, 0, 0 },
	{ TX_TRI5N008, 5447, 0, 0, 0 },
	{ TX_TRI5N009, 5381, 0, 0, 0 },
	{ TX_TRI5N010, 5519, 0, 0, 0 },
	{ TX_TRI5N011, 7659, 0, 0, 0 },
	{ TX_TRI5N012, 7778, 0, 0, 0 },
	{ TX_TRI5N013, 6041, 0, 0, 0 },
	{ TX_TRI5N014, 6157, 0, 0, 0 },
	{ TX_TRI5N015, 6273, 0, 0, 0 },
	{ TX_TRI5N016, 6746, 0, 0, 0 },
	{ TX_TRI5N017, 6865, 0, 0, 0 },
	{ TX_TRI5N018, 6984, 0, 0, 0 },
	{ TX_TRI5N019, 6389, 0, 0, 0 },
	{ TX_TRI5N020, 6508, 0, 0, 0 },
	{ TX_TRI5N021, 6627, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText trial5Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::trial5Tick1() {
	playVoc("TRI5LOOP");
	playMidiMusicTracks(MIDITRACK_31);

	loadActorAnim2(OBJECT_REDGEM1, "redgem", REDGEM1_X, REDGEM1_Y);
	loadActorAnim2(OBJECT_REDGEM2, "redgem", REDGEM2_X, REDGEM2_Y);
	loadActorAnim2(OBJECT_REDGEM3, "redgem", REDGEM3_X, REDGEM3_Y);
	loadActorAnim2(OBJECT_GRNGEM1, "grngem", GRNGEM1_X, GRNGEM1_Y);
	loadActorAnim2(OBJECT_GRNGEM2, "grngem", GRNGEM2_X, GRNGEM2_Y);
	loadActorAnim2(OBJECT_GRNGEM3, "grngem", GRNGEM3_X, GRNGEM3_Y);
	loadActorAnim2(OBJECT_BLUGEM1, "blugem", BLUGEM1_X, BLUGEM1_Y);
	loadActorAnim2(OBJECT_BLUGEM2, "blugem", BLUGEM2_X, BLUGEM2_Y);
	loadActorAnim2(OBJECT_BLUGEM3, "blugem", BLUGEM3_X, BLUGEM3_Y);
}

void Room::trial5Tick60() {
	showText(TX_SPEAKER_BENNIE, TX_TRI5_052);
	showText(TX_SPEAKER_SPOCK,  TX_TRI5_017);
	showText(TX_SPEAKER_MCCOY,  TX_TRI5_014);
	showText(TX_SPEAKER_KIRK,   TX_TRI5_002);
}

void Room::trial5VlictAppeared() {
	showText(TX_SPEAKER_BIALBI, TX_TRI5_027);
	showText(TX_SPEAKER_VLICT,  TX_TRI5_020);
	showText(TX_SPEAKER_BIALBI, TX_TRI5_031);
	showText(TX_SPEAKER_VLICT,  TX_TRI5_022);
	showText(TX_SPEAKER_BIALBI, TX_TRI5_029);
	showText(TX_SPEAKER_VLICT,  TX_TRI5_018);

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_TRI5_003, TX_TRI5_008, TX_TRI5_006,
		TX_END
	};
	int choice = showMultipleTexts(choices);

	if (choice == 0) { // "Do you want me to help you?"
		showText(TX_SPEAKER_VLICT, TX_TRI5_023);
		showText(TX_SPEAKER_MCCOY, TX_TRI5_015);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			TX_TRI5_009, TX_TRI5_007,
			TX_END
		};
		choice = showMultipleTexts(choices2);

		if (choice == 0)
			choice = 1;
		else if (choice == 1)
			choice = 2;
	}

	if (choice == 1) { // "Now you can rot as you deserve"
		showText(TX_SPEAKER_BIALBI, TX_TRI5_034);

		_awayMission->trial.missionEndMethod = 4;
		endMission(_awayMission->trial.missionScore, 1, 4);
	} else if (choice == 2) { // "I'll intervene if Quetzecoatl goes free"
		showText(TX_SPEAKER_VLICT, TX_TRI5_019);
		playMidiMusicTracks(MIDITRACK_28);

		showText(TX_SPEAKER_BIALBI, TX_TRI5_025);
		_awayMission->trial.missionScore += 4;
		showText(TX_SPEAKER_KIRK,   TX_TRI5_004);
		showText(TX_SPEAKER_BIALBI, TX_TRI5_030);
		showText(TX_SPEAKER_BIALBI, TX_TRI5_026);
		showText(TX_SPEAKER_VLICT,  TX_TRI5_021);
		showText(TX_SPEAKER_BIALBI, TX_TRI5_028);
		showText(TX_SPEAKER_KIRK,   TX_TRI5_005);
		showText(TX_SPEAKER_BIALBI, TX_TRI5_033);
		showText(TX_SPEAKER_BIALBI, TX_TRI5_032);

		_awayMission->trial.missionEndMethod = 3;
		endMission(1, 1, 3); // FIXME: Are these parameters correct?
	}
}

void Room::trial5WalkToInterface() {
	walkCrewmanC(OBJECT_KIRK, 0x106, 0x96, &Room::trial5ReachedInterface);
}

void Room::trial5ReachedInterface() {
	if (!_awayMission->trial.neuralInterfaceActive)
		showText(TX_SPEAKER_VOICE, TX_TRI5_035);
	else {
		showText(TX_SPEAKER_BIALBI, TX_TRI5_024);
		loadActorAnimC(OBJECT_VLICT, "vlict1", 0xbe, 0x91, &Room::trial5VlictAppeared);
		_awayMission->trial.missionScore += 5;
		playMidiMusicTracks(MIDITRACK_1);
	}
}

void Room::trial5GetGem(int16 item, int16 object) {
	_roomVar.trial.itemToUse = item;
	_roomVar.trial.objectToUse = object;

	_awayMission->disableInput = true;
	if (object == _awayMission->trial.holeContents[0]) {
		_awayMission->trial.holeContents[0] = 0;
		_awayMission->trial.neuralInterfaceActive = false;
		loadActorStandAnim(OBJECT_BEAM);
		walkCrewmanC(OBJECT_KIRK, 0x2c, 0x9f, &Room::trial5ReachedGem);
	} else if (object == _awayMission->trial.holeContents[1]) {
		_awayMission->trial.holeContents[1] = 0;
		_awayMission->trial.neuralInterfaceActive = false;
		loadActorStandAnim(OBJECT_BEAM);
		walkCrewmanC(OBJECT_KIRK, 0x22, 0x9f, &Room::trial5ReachedGem);
	} else if (object == _awayMission->trial.holeContents[2]) {
		_awayMission->trial.holeContents[2] = 0;
		_awayMission->trial.neuralInterfaceActive = false;
		loadActorStandAnim(OBJECT_BEAM);
		walkCrewmanC(OBJECT_KIRK, 0x3c, 0x9f, &Room::trial5ReachedGem);
	} else if (object == OBJECT_REDGEM1 || object == OBJECT_REDGEM2 || object == OBJECT_REDGEM3)
		walkCrewmanC(OBJECT_KIRK, 0x80, 0xaa, &Room::trial5ReachedGem);
	else if (object == OBJECT_GRNGEM1 || object == OBJECT_GRNGEM2 || object == OBJECT_GRNGEM3)
		walkCrewmanC(OBJECT_KIRK, 0x95, 0xaa, &Room::trial5ReachedGem);
	else if (object == OBJECT_BLUGEM1 || object == OBJECT_BLUGEM2 || object == OBJECT_BLUGEM3)
		walkCrewmanC(OBJECT_KIRK, 0xad, 0xaa, &Room::trial5ReachedGem);
}

void Room::trial5ReachedGem() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::trial5PickedUpGem);
}

void Room::trial5PickedUpGem() {
	giveItem(_roomVar.trial.itemToUse);
	loadActorStandAnim(_roomVar.trial.objectToUse);
	_awayMission->disableInput = false;
}

void Room::trial5GetRedGem1() {
	trial5GetGem(OBJECT_IREDGEM1, OBJECT_REDGEM1);
}

void Room::trial5GetRedGem2() {
	trial5GetGem(OBJECT_IREDGEM2, OBJECT_REDGEM2);
}

void Room::trial5GetRedGem3() {
	trial5GetGem(OBJECT_IREDGEM3, OBJECT_REDGEM3);
}

void Room::trial5GetGreenGem1() {
	trial5GetGem(OBJECT_IGRNGEM1, OBJECT_GRNGEM1);
}

void Room::trial5GetGreenGem2() {
	trial5GetGem(OBJECT_IGRNGEM2, OBJECT_GRNGEM2);
}

void Room::trial5GetGreenGem3() {
	trial5GetGem(OBJECT_IGRNGEM3, OBJECT_GRNGEM3);
}

void Room::trial5GetBlueGem1() {
	trial5GetGem(OBJECT_IBLUGEM1, OBJECT_BLUGEM1);
}

void Room::trial5GetBlueGem2() {
	trial5GetGem(OBJECT_IBLUGEM2, OBJECT_BLUGEM2);
}

void Room::trial5GetBlueGem3() {
	trial5GetGem(OBJECT_IBLUGEM3, OBJECT_BLUGEM3);
}

void Room::trial5LookAtKirk() {
	showDescription(TX_TRI5N005);
}

void Room::trial5LookAtSpock() {
	showDescription(TX_TRI5N009);
}

void Room::trial5LookAtMccoy() {
	showDescription(TX_TRI5N008);
}

void Room::trial5LookAtRedshirt() {
	showDescription(TX_TRI5N010);
}

void Room::trial5LookAtBeam() {
	showDescription(TX_TRI5N002);
}

void Room::trial5LookAtInterface() {
	showDescription(TX_TRI5N004);
}

void Room::trial5UseSTricorderOnInterface() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_TRI5_010, true);
}

void Room::trial5UseMTricorderOnInterface() {
	mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_TRI5_011, true);
}

void Room::trial5LookAtRedGem1() {
	showDescription(TX_TRI5N013);
}

void Room::trial5LookAtRedGem2() {
	showDescription(TX_TRI5N014);
}

void Room::trial5LookAtRedGem3() {
	showDescription(TX_TRI5N015);
}

void Room::trial5LookAtGreenGem1() {
	showDescription(TX_TRI5N019);
}

void Room::trial5LookAtGreenGem2() {
	showDescription(TX_TRI5N020);
}

void Room::trial5LookAtGreenGem3() {
	showDescription(TX_TRI5N021);
}

void Room::trial5LookAtBlueGem1() {
	showDescription(TX_TRI5N016);
}

void Room::trial5LookAtBlueGem2() {
	showDescription(TX_TRI5N017);
}

void Room::trial5LookAtBlueGem3() {
	showDescription(TX_TRI5N018);
}

void Room::trial5LookAtHole() {
	showDescription(TX_TRI5N006);
}

void Room::trial5TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_TRI5_001);
}

void Room::trial5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TRI5_016);
}

void Room::trial5TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_TRI5_013);
}

void Room::trial5TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, TX_TRI5_051);
}

void Room::trial5UsePhaser() {
	showDescription(TX_TRI5N011);
}

void Room::trial5UseMTricorderAnywhere() {
	// BUGFIX: Original had Spock doing the animation, not Mccoy
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, - 1, true);
	showDescription(TX_TRI5N012);
}

void Room::trial5UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, - 1, true);
	showDescription(TX_TRI5N012);
}

void Room::trial5UseCommunicator() {
	showDescription(TX_TRI5N007);
}

void Room::trial5UseGemOnHole(int16 item, int16 object, int16 hole) {
	const Common::Point positions[] = {
		Common::Point(0x2c, 0x9f),
		Common::Point(0x22, 0x9f),
		Common::Point(0x3c, 0x9f),
	};

	hole -= 1;

	_roomVar.trial.itemToUse = item;
	_roomVar.trial.objectToUse = object;
	_roomVar.trial.hole = hole;

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	walkCrewmanC(OBJECT_KIRK, positions[hole].x, positions[hole].y, &Room::trial5ReachedHoleToPutGem);
}

void Room::trial5ReachedHoleToPutGem() {
	const Common::Point holePositions[] = {
		Common::Point(0x2e, 0x8e),
		Common::Point(0x22, 0x95),
		Common::Point(0x3c, 0x95),
	};
	const char *const gemAnims[] = {
		"RedGem1", "RedGem2", "RedGem3",
		"GrnGem1", "GrnGem2", "GrnGem3",
		"BluGem1", "BluGem2", "BluGem3",
	};

	const int16 hole = _roomVar.trial.hole;
	const Common::Point &pos = holePositions[hole];
	const char *anim = gemAnims[_roomVar.trial.objectToUse - OBJECT_REDGEM1];

	loadActorAnim2(OBJECT_KIRK, "kusehn");

	if (_awayMission->trial.holeContents[hole] == 0) {
		_awayMission->trial.holeContents[hole] = _roomVar.trial.objectToUse;
		loadActorAnim2(_roomVar.trial.objectToUse, anim, pos.x, pos.y);
		loseItem(_roomVar.trial.itemToUse);

		// Look at the contents of the holes, decide what to do

		const int16 RED = 1;
		const int16 GREEN = 2;
		const int16 BLUE = 3;

		int16 holes[3];
		holes[0] = 0;
		holes[1] = 0;
		holes[2] = 0;

		for (int i = 0; i < 3; i++) {
			int16 c = _awayMission->trial.holeContents[i];
			if (c >= OBJECT_REDGEM1 && c <= OBJECT_REDGEM3)
				holes[i] = RED;
			else if (c >= OBJECT_GRNGEM1 && c <= OBJECT_GRNGEM3)
				holes[i] = GREEN;
			else if (c >= OBJECT_BLUGEM1 && c <= OBJECT_BLUGEM3)
				holes[i] = BLUE;
		}

		if COMBO(RED, RED, RED) {
			trial5ActivateLightOfWar();
			playMidiMusicTracks(MIDITRACK_1);
			showText(TX_SPEAKER_LIGHT_OF_WAR, TX_TRI5_050); // All ships in orbit destroyed
			showGameOverMenu();
		} else if COMBO(RED, RED, GREEN) {
			trial5RestrictedCombination1();
		} else if COMBO(RED, RED, BLUE) {
			trial5RestrictedCombination1();
		} else if COMBO(RED, GREEN, RED) {
			trial5RestrictedCombination1();
		} else if COMBO(RED, GREEN, GREEN) {
			trial5ActivateLightOfWar();
			showText(TX_SPEAKER_LIGHT_OF_WAR, TX_TRI5_036);
		} else if COMBO(RED, GREEN, BLUE) {
			trial5ActivateLightOfWar();
			showText(TX_SPEAKER_LIGHT_OF_WAR, TX_TRI5_048);
		} else if COMBO(RED, BLUE, RED) {
			trial5RestrictedCombination1();
		} else if COMBO(RED, BLUE, GREEN) {
			trial5RestrictedCombination1();
		} else if COMBO(RED, BLUE, BLUE) {
			trial5ActivateLightOfWar();
			showText(TX_SPEAKER_LIGHT_OF_WAR, TX_TRI5_049);
		} else if COMBO(GREEN, RED, RED) {
			trial5ActivateLightOfKnowledge();
			showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_047);
		} else if COMBO(GREEN, RED, GREEN) {
			trial5RestrictedCombination2();
		} else if COMBO(GREEN, RED, BLUE) {
			trial5RestrictedCombination2();
		} else if COMBO(GREEN, GREEN, RED) {
			trial5RestrictedCombination1();
		} else if COMBO(GREEN, GREEN, GREEN) {
			trial5ActivateLightOfKnowledge();
			playVoc("EFX12B");
			showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_040);
			showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_037);
			_awayMission->trial.neuralInterfaceActive = true;

			if (!_awayMission->trial.gotPointsForActivatingInterface) {
				_awayMission->trial.gotPointsForActivatingInterface = true;
				_awayMission->trial.missionScore += 3;
			}
		} else if COMBO(GREEN, GREEN, BLUE) {
			trial5RestrictedCombination1();
		} else if COMBO(GREEN, BLUE, RED) {
			trial5ActivateLightOfKnowledge();
			showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_041);
		} else if COMBO(GREEN, BLUE, GREEN) {
			trial5RestrictedCombination2();
		} else if COMBO(GREEN, BLUE, BLUE) {
			trial5ActivateLightOfKnowledge();
			showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_038);
		} else if COMBO(BLUE, RED, RED) {
			trial5ActivateLightOfTravel();
			showText(TX_SPEAKER_LIGHT_OF_TRAVEL, TX_TRI5_045);
		} else if COMBO(BLUE, RED, GREEN) {
			trial5DestinationNoLongerExists();
		} else if COMBO(BLUE, RED, BLUE) {
			trial5DestinationNoLongerExists();
		} else if COMBO(BLUE, GREEN, RED) {
			trial5ActivateLightOfTravel();
			showText(TX_SPEAKER_LIGHT_OF_TRAVEL, TX_TRI5_042);
		} else if COMBO(BLUE, GREEN, GREEN) {
			trial5ActivateLightOfTravel();
			showText(TX_SPEAKER_LIGHT_OF_TRAVEL, TX_TRI5_044);
		} else if COMBO(BLUE, GREEN, BLUE) {
			trial5DestinationNoLongerExists();
		} else if COMBO(BLUE, BLUE, RED) {
			trial5RestrictedCombination1();
		} else if COMBO(BLUE, BLUE, GREEN) {
			trial5RestrictedCombination1();
		} else if COMBO(BLUE, BLUE, BLUE) {
			trial5ActivateLightOfTravel();
			showText(TX_SPEAKER_LIGHT_OF_TRAVEL, TX_TRI5_046);

			_awayMission->disableInput = true;
			playSoundEffectIndex(kSfxTransporterDematerialize);

			loadActorAnimC(OBJECT_KIRK,  "kteled", -1, -1, &Room::trial5CrewmanBeamedOut);
			loadActorAnimC(OBJECT_SPOCK, "steled", -1, -1, &Room::trial5CrewmanBeamedOut);
			loadActorAnimC(OBJECT_MCCOY, "mteled", -1, -1, &Room::trial5CrewmanBeamedOut);
			if (!_awayMission->redshirtDead)
				loadActorAnimC(OBJECT_REDSHIRT, "rteled", -1, -1, &Room::trial5CrewmanBeamedOut);
		} else {
			// Some holes are empty
			loadActorStandAnim(OBJECT_BEAM);
		}
	}
}

void Room::trial5ActivateLightOfWar() {
	playSoundEffectIndex(kSfxTransporterEnergize);
	loadActorAnim2(OBJECT_BEAM, "redlit", 0, 0);
}

void Room::trial5ActivateLightOfKnowledge() {
	playSoundEffectIndex(kSfxTransporterEnergize);
	loadActorAnim2(OBJECT_BEAM, "grnlit", 0, 0);
}

void Room::trial5ActivateLightOfTravel() {
	playSoundEffectIndex(kSfxTransporterEnergize);
	loadActorAnim2(OBJECT_BEAM, "blulit", 0, 0);
}

void Room::trial5RestrictedCombination1() {
	// NOTE: The only difference between "trial5RestrictedCombination1" and
	// "trial5RestrictedCombination2" is that this one doesn't show the light of
	// knowledge.
	// It's inconsistent, but I guess we can chalk it up to the aliens being mysterious...
	playSoundEffectIndex(kSfxTransporterEnergize);
	showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_039);
}

void Room::trial5RestrictedCombination2() {
	trial5ActivateLightOfKnowledge();
	showText(TX_SPEAKER_LIGHT_OF_KNOWLEDGE, TX_TRI5_039);
}

void Room::trial5DestinationNoLongerExists() {
	trial5ActivateLightOfTravel();
	showText(TX_SPEAKER_LIGHT_OF_TRAVEL, TX_TRI5_043);
}

void Room::trial5CrewmanBeamedOut() {
	if (!_awayMission->trial.gotPointsForBeamingOut) {
		_awayMission->trial.gotPointsForBeamingOut = true;
		_awayMission->trial.missionScore += 1;
	}
	loadRoomIndex(4, 4);
}


void Room::trial5UseRedGem1OnHole1() {
	trial5UseGemOnHole(OBJECT_IREDGEM1, OBJECT_REDGEM1, 1);
}

void Room::trial5UseRedGem2OnHole1() {
	trial5UseGemOnHole(OBJECT_IREDGEM2, OBJECT_REDGEM2, 1);
}

void Room::trial5UseRedGem3OnHole1() {
	trial5UseGemOnHole(OBJECT_IREDGEM3, OBJECT_REDGEM3, 1);
}

void Room::trial5UseGreenGem1OnHole1() {
	trial5UseGemOnHole(OBJECT_IGRNGEM1, OBJECT_GRNGEM1, 1);
}

void Room::trial5UseGreenGem2OnHole1() {
	trial5UseGemOnHole(OBJECT_IGRNGEM2, OBJECT_GRNGEM2, 1);
}

void Room::trial5UseGreenGem3OnHole1() {
	trial5UseGemOnHole(OBJECT_IGRNGEM3, OBJECT_GRNGEM3, 1);
}

void Room::trial5UseBlueGem1OnHole1() {
	trial5UseGemOnHole(OBJECT_IBLUGEM1, OBJECT_BLUGEM1, 1);
}

void Room::trial5UseBlueGem2OnHole1() {
	trial5UseGemOnHole(OBJECT_IBLUGEM2, OBJECT_BLUGEM2, 1);
}

void Room::trial5UseBlueGem3OnHole1() {
	trial5UseGemOnHole(OBJECT_IBLUGEM3, OBJECT_BLUGEM3, 1);
}


void Room::trial5UseRedGem1OnHole2() {
	trial5UseGemOnHole(OBJECT_IREDGEM1, OBJECT_REDGEM1, 2);
}

void Room::trial5UseRedGem2OnHole2() {
	trial5UseGemOnHole(OBJECT_IREDGEM2, OBJECT_REDGEM2, 2);
}

void Room::trial5UseRedGem3OnHole2() {
	trial5UseGemOnHole(OBJECT_IREDGEM3, OBJECT_REDGEM3, 2);
}

void Room::trial5UseGreenGem1OnHole2() {
	trial5UseGemOnHole(OBJECT_IGRNGEM1, OBJECT_GRNGEM1, 2);
}

void Room::trial5UseGreenGem2OnHole2() {
	trial5UseGemOnHole(OBJECT_IGRNGEM2, OBJECT_GRNGEM2, 2);
}

void Room::trial5UseGreenGem3OnHole2() {
	trial5UseGemOnHole(OBJECT_IGRNGEM3, OBJECT_GRNGEM3, 2);
}

void Room::trial5UseBlueGem1OnHole2() {
	trial5UseGemOnHole(OBJECT_IBLUGEM1, OBJECT_BLUGEM1, 2);
}

void Room::trial5UseBlueGem2OnHole2() {
	trial5UseGemOnHole(OBJECT_IBLUGEM2, OBJECT_BLUGEM2, 2);
}

void Room::trial5UseBlueGem3OnHole2() {
	trial5UseGemOnHole(OBJECT_IBLUGEM3, OBJECT_BLUGEM3, 2);
}


void Room::trial5UseRedGem1OnHole3() {
	trial5UseGemOnHole(OBJECT_IREDGEM1, OBJECT_REDGEM1, 3);
}

void Room::trial5UseRedGem2OnHole3() {
	trial5UseGemOnHole(OBJECT_IREDGEM2, OBJECT_REDGEM2, 3);
}

void Room::trial5UseRedGem3OnHole3() {
	trial5UseGemOnHole(OBJECT_IREDGEM3, OBJECT_REDGEM3, 3);
}

void Room::trial5UseGreenGem1OnHole3() {
	trial5UseGemOnHole(OBJECT_IGRNGEM1, OBJECT_GRNGEM1, 3);
}

void Room::trial5UseGreenGem2OnHole3() {
	trial5UseGemOnHole(OBJECT_IGRNGEM2, OBJECT_GRNGEM2, 3);
}

void Room::trial5UseGreenGem3OnHole3() {
	trial5UseGemOnHole(OBJECT_IGRNGEM3, OBJECT_GRNGEM3, 3);
}

void Room::trial5UseBlueGem1OnHole3() {
	trial5UseGemOnHole(OBJECT_IBLUGEM1, OBJECT_BLUGEM1, 3);
}

void Room::trial5UseBlueGem2OnHole3() {
	trial5UseGemOnHole(OBJECT_IBLUGEM2, OBJECT_BLUGEM2, 3);
}

void Room::trial5UseBlueGem3OnHole3() {
	trial5UseGemOnHole(OBJECT_IBLUGEM3, OBJECT_BLUGEM3, 3);
}


void Room::trial5UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_TRI5_012);
}

}
