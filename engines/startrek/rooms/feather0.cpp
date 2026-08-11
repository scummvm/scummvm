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

#define OBJECT_QUETZECOATL 8

#define HOTSPOT_MOON 0x20
#define HOTSPOT_LOG 0x21
#define HOTSPOT_HUT 0x22
#define HOTSPOT_EYES_1 0x23
#define HOTSPOT_EYES_2 0x24
#define HOTSPOT_EYES_3 0x25
#define HOTSPOT_EYES_4 0x26
#define HOTSPOT_EYES_5 0x27
#define HOTSPOT_EYES_6 0x28
#define HOTSPOT_EYES_7 0x29
#define HOTSPOT_EYES_8 0x2a
#define HOTSPOT_EYES_9 0x2b
#define HOTSPOT_TREE 0x2c

namespace StarTrek {

extern const RoomAction feather0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0},  &Room::feather0Tick1 },
	{ {ACTION_TICK, 60, 0, 0, 0}, &Room::feather0Tick60 },
	{ {ACTION_TALK, 8, 0, 0, 0},  &Room::feather0TalkToQuetzecoatl },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0},                &Room::feather0Timer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0},                &Room::feather0Timer1Expired },
	{ {ACTION_USE, OBJECT_IPHASERS, 8,            0, 0}, &Room::feather0UsePhaserOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_IPHASERK, 8,            0, 0}, &Room::feather0UsePhaserOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY, 0, 0}, &Room::feather0UsePhaserOnMccoy },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY, 0, 0}, &Room::feather0UsePhaserOnMccoy },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff,          0, 0}, &Room::feather0UseMedkit },
	{ {ACTION_LOOK, 8,    0, 0, 0}, &Room::feather0LookAtQuetzecoatl },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::feather0LookAtMoon },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::feather0LookAtLog },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::feather0LookAtHut },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::feather0LookAnywhere },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x26, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x27, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x28, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x29, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x2a, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x2b, 0, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x2c, 0, 0, 0}, &Room::feather0LookAtTree },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::feather0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::feather0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::feather0LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::feather0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::feather0TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::feather0TalkToRedshirt },
	{ {ACTION_USE, OBJECT_ISTRICOR,  8, 0, 0}, &Room::feather0UseSTricorderOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::feather0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR,  8, 0, 0}, &Room::feather0UseMTricorderOnQuetzecoatl },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum feather0TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_QUETZECOATL, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_STRAGEY,
	TX_FEA0_002, TX_FEA0_003, TX_FEA0_004, TX_FEA0_005, TX_FEA0_006,
	TX_FEA0_007, TX_FEA0_008, TX_FEA0_009, TX_FEA0_010, TX_FEA0_011,
	TX_FEA0_012, TX_FEA0_013, TX_FEA0_014, TX_FEA0_015, TX_FEA0_016,
	TX_FEA0_017, TX_FEA0_018, TX_FEA0_019, TX_FEA0_020, TX_FEA0_021,
	TX_FEA0_022, TX_FEA0_023, TX_FEA0_024, TX_FEA0_025, TX_FEA0_026,
	TX_FEA0_027, TX_FEA0_028, TX_FEA0_029, TX_FEA0_030, TX_FEA0_031,
	TX_FEA0_032, TX_FEA0_033, TX_FEA0_101, TX_FEA0_108, TX_FEA0N000,
	TX_FEA0N001, TX_FEA0N002, TX_FEA0N003, TX_FEA0N004, TX_FEA0N005,
	TX_FEA0N006, TX_FEA0N007, TX_FEA0N008, TX_FEA0N009, TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets feather0TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 1775, 0, 0, 0 },
	{ TX_SPEAKER_QUETZECOATL, 1786, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 1798, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 1808, 0, 0, 0 },
	{ TX_SPEAKER_STRAGEY, 1818, 0, 0, 0 },
	{ TX_FEA0_002, 5547, 0, 0, 0 },
	{ TX_FEA0_003, 5324, 0, 0, 0 },
	{ TX_FEA0_004, 4335, 0, 0, 0 },
	{ TX_FEA0_005, 5094, 0, 0, 0 },
	{ TX_FEA0_006, 3553, 0, 0, 0 },
	{ TX_FEA0_007, 3670, 0, 0, 0 },
	{ TX_FEA0_008, 5870, 0, 0, 0 },
	{ TX_FEA0_009, 4232, 0, 0, 0 },
	{ TX_FEA0_010, 4586, 0, 0, 0 },
	{ TX_FEA0_011, 3786, 0, 0, 0 },
	{ TX_FEA0_012, 5671, 0, 0, 0 },
	{ TX_FEA0_013, 4963, 0, 0, 0 },
	{ TX_FEA0_014, 5764, 0, 0, 0 },
	{ TX_FEA0_015, 3203, 0, 0, 0 },
	{ TX_FEA0_016, 3382, 0, 0, 0 },
	{ TX_FEA0_017, 2597, 0, 0, 0 },
	{ TX_FEA0_018, 2949, 0, 0, 0 },
	{ TX_FEA0_019, 6135, 0, 0, 0 },
	{ TX_FEA0_020, 3083, 0, 0, 0 },
	{ TX_FEA0_021, 2664, 0, 0, 0 },
	{ TX_FEA0_022, 2793, 0, 0, 0 },
	{ TX_FEA0_023, 3034, 0, 0, 0 },
	{ TX_FEA0_024, 5986, 0, 0, 0 },
	{ TX_FEA0_025, 3343, 0, 0, 0 },
	{ TX_FEA0_026, 4911, 0, 0, 0 },
	{ TX_FEA0_027, 5250, 0, 0, 0 },
	{ TX_FEA0_028, 3466, 0, 0, 0 },
	{ TX_FEA0_029, 5204, 0, 0, 0 },
	{ TX_FEA0_030, 4444, 0, 0, 0 },
	{ TX_FEA0_031, 4007, 0, 0, 0 },
	{ TX_FEA0_032, 3894, 0, 0, 0 },
	{ TX_FEA0_033, 4124, 0, 0, 0 },
	{ TX_FEA0_101, 4711, 0, 0, 0 },
	{ TX_FEA0_108,  830, 0, 0, 0 },
	{ TX_FEA0N000, 1830, 0, 0, 0 },
	{ TX_FEA0N001, 1950, 0, 0, 0 },
	{ TX_FEA0N002, 2477, 0, 0, 0 },
	{ TX_FEA0N003, 1890, 0, 0, 0 },
	{ TX_FEA0N004, 2383, 0, 0, 0 },
	{ TX_FEA0N005, 2425, 0, 0, 0 },
	{ TX_FEA0N006, 2021, 0, 0, 0 },
	{ TX_FEA0N007, 2158, 0, 0, 0 },
	{ TX_FEA0N008, 2076, 0, 0, 0 },
	{ TX_FEA0N009, 2538, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 6222, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText feather0Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::feather0Tick1() {
	playVoc("FEA0LOOP");
	loadActorAnim(OBJECT_QUETZECOATL, "s5r0qb", 0xbe, 0xa6);
}

void Room::feather0Tick60() {
	playMidiMusicTracks(MIDITRACK_33);
}

void Room::feather0TalkToQuetzecoatl() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_FEA0_006,
		TX_FEA0_007,
		TX_FEA0_011,
		TX_END
	};

	showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_028);
	int choice = showMultipleTexts(choices);
	bool alreadyAngered = false;

	if (choice == 0) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_009,
			TX_FEA0_004,
			TX_END
		};
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_032);
		choice = showMultipleTexts(choices0);
	} else if (choice == 1) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_002,
			TX_FEA0_012,
			TX_END
		};
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_031);
		choice = showMultipleTexts(choices0);
	} else if (choice == 2) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_014,
			TX_FEA0_008,
			TX_END
		};
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_033);
		choice = showMultipleTexts(choices0);

		if (choice == 0) {
			alreadyAngered = true;
			showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_024);
		}
	} else
		showDescription(TX_DIALOG_ERROR);

	if (!alreadyAngered) {
		if (choice == 0)
			showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_030);
		else // choice == 1
			showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_101);
		showText(TX_SPEAKER_KIRK, TX_FEA0_010);
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_026);

		const TextRef choices1[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_013,
			TX_FEA0_005,
			TX_END
		};
		choice = showMultipleTexts(choices1);

		if (choice == 0) {
			showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_029);
			showText(TX_SPEAKER_KIRK, TX_FEA0_003);
		}

		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_027);
	}

	// Quetzecoatl ultimately warps the crew away no matter what
	_awayMission->disableInput = true;
	loadActorAnim(OBJECT_QUETZECOATL, "s5r0qt");
	playMidiMusicTracks(MIDITRACK_1);
	_awayMission->timers[0] = 50;
	_awayMission->timers[1] = 64;
}

void Room::feather0Timer0Expired() {
	playVoc("SE2BIGEN");

	loadActorAnim2(OBJECT_KIRK, "ktele");
	loadActorAnim2(OBJECT_SPOCK, "stele");
	loadActorAnim2(OBJECT_MCCOY, "mtele");
	loadActorAnim2(OBJECT_REDSHIRT, "rtele");
}

void Room::feather0Timer1Expired() {
	_awayMission->disableInput = false;
	loadRoomIndex(1, 5);
}

void Room::feather0UsePhaserOnQuetzecoatl() {
	showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_025);
}

void Room::feather0UsePhaserOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA0_016);
}

void Room::feather0UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_FEA0_108);
}

void Room::feather0LookAtQuetzecoatl() {
	showDescription(TX_FEA0N008);
}

void Room::feather0LookAtMoon() {
	showDescription(TX_FEA0N007);
}

void Room::feather0LookAtLog() {
	showDescription(TX_FEA0N006);
}

void Room::feather0LookAtHut() {
	showDescription(TX_FEA0N001);
}

void Room::feather0LookAnywhere() {
	showDescription(TX_FEA0N009);
}

void Room::feather0LookAtEyes() {
	showDescription(TX_FEA0N000);
}

void Room::feather0LookAtTree() {
	showDescription(TX_FEA0N003);
}

void Room::feather0LookAtMccoy() {
	showDescription(TX_FEA0N004);
}

void Room::feather0LookAtSpock() {
	showDescription(TX_FEA0N005);
}

void Room::feather0LookAtRedshirt() {
	showDescription(TX_FEA0N002);
}

void Room::feather0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA0_017);
	showText(TX_SPEAKER_SPOCK, TX_FEA0_021);
}

void Room::feather0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA0_022);
	showText(TX_SPEAKER_MCCOY, TX_FEA0_018);
}

void Room::feather0TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA0_023);
}

void Room::feather0UseSTricorderOnQuetzecoatl() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA0_020, false);
}

void Room::feather0UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA0_019, false);
}

void Room::feather0UseMTricorderOnQuetzecoatl() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_FEA0_015, false);
}

}
