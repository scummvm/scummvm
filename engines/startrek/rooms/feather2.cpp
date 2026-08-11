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

#define HOTSPOT_EYES_1   0x20
#define HOTSPOT_EYES_2   0x21
#define HOTSPOT_EYES_3   0x22
#define HOTSPOT_BIG_TREE 0x23
#define HOTSPOT_TREES    0x24
#define HOTSPOT_VINES 0x25
#define HOTSPOT_LEFT_EXIT 0x26

namespace StarTrek {

extern const RoomAction feather2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0},                 &Room::feather2Tick1 },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0},    &Room::feather2UseCommunicator },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0, 0}, &Room::feather2UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0, 0}, &Room::feather2UsePhaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::feather2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::feather2UseMTricorderAnywhere },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0},      &Room::feather2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0},      &Room::feather2TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0},   &Room::feather2TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_VINES, 0, 0, 0},     &Room::feather2LookAtVines },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0},  &Room::feather2UseMedkit },
	{ {ACTION_WALK, HOTSPOT_LEFT_EXIT, 0, 0, 0}, &Room::feather2WalkToLeftExit },
	{ {ACTION_LOOK, HOTSPOT_EYES_1,    0, 0, 0}, &Room::feather2LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_2,    0, 0, 0}, &Room::feather2LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_3,    0, 0, 0}, &Room::feather2LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_BIG_TREE,  0, 0, 0}, &Room::feather2LookAtBigTree },
	{ {ACTION_LOOK, HOTSPOT_TREES,     0, 0, 0}, &Room::feather2LookAtTrees },
	{ {ACTION_LOOK, 0xff,              0, 0, 0}, &Room::feather2LookAnywhere },
	{ {ACTION_LOOK, OBJECT_KIRK,       0, 0, 0}, &Room::feather2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,      0, 0, 0}, &Room::feather2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,      0, 0, 0}, &Room::feather2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,   0, 0, 0}, &Room::feather2LookAtRedshirt },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum feather2TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_QUETZECOATL, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_STRAGEY,
	TX_FEA2_001, TX_FEA2_002, TX_FEA2_003, TX_FEA2_004, TX_FEA2_005,
	TX_FEA2_006, TX_FEA2_007, TX_FEA2_008, TX_FEA2_009, TX_FEA2_010,
	TX_FEA2N000, TX_FEA2N001, TX_FEA2N002, TX_FEA2N003, TX_FEA2N004,
	TX_FEA2N005, TX_FEA2N006, TX_FEA2N007, TX_FEA2N008
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets feather2TextOffsets[] = {
	{ TX_SPEAKER_KIRK,  965, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 988, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 998, 0, 0, 0 },
	{ TX_SPEAKER_QUETZECOATL, 976, 0, 0, 0 },
	{ TX_SPEAKER_STRAGEY, 1008, 0, 0, 0 },
	{ TX_FEA2_001, 465, 0, 0, 0 },
	{ TX_FEA2_002, 1827, 0, 0, 0 },
	{ TX_FEA2_003, 1713, 0, 0, 0 },
	{ TX_FEA2_004, 1894, 0, 0, 0 },
	{ TX_FEA2_005, 1390, 0, 0, 0 },
	{ TX_FEA2_006, 1624, 0, 0, 0 },
	{ TX_FEA2_007, 1747, 0, 0, 0 },
	{ TX_FEA2_008, 2003, 0, 0, 0 },
	{ TX_FEA2_009, 2112, 0, 0, 0 },
	{ TX_FEA2_010, 1253, 0, 0, 0 },
	{ TX_FEA2N000, 1209, 0, 0, 0 },
	{ TX_FEA2N001, 1461, 0, 0, 0 },
	{ TX_FEA2N002, 2172, 0, 0, 0 },
	{ TX_FEA2N003, 2479, 0, 0, 0 },
	{ TX_FEA2N004, 2365, 0, 0, 0 },
	{ TX_FEA2N005, 2252, 0, 0, 0 },
	{ TX_FEA2N006, 1021, 0, 0, 0 },
	{ TX_FEA2N007, 1091, 0, 0, 0 },
	{ TX_FEA2N008, 1521, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText feather2Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::feather2Tick1() {
	playVoc("FEA2LOOP");
	playMidiMusicTracks(MIDITRACK_27);
}

void Room::feather2UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_FEA2_006);
}

void Room::feather2UsePhaser() {
	// FIXME: Why does McCoy say "They're dead, Jim"?
	showText(TX_SPEAKER_MCCOY, TX_FEA2_003);
}

void Room::feather2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA2_007);
}

void Room::feather2UseMTricorderAnywhere() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_FEA2_002);
}

void Room::feather2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA2_004);
}

void Room::feather2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA2_008);
}

void Room::feather2TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA2_009);
}

void Room::feather2LookAtVines() {
	// NOTE: This might be unused? I can't find where HOTSPOT_VINES is supposed to be.
	showDescription(TX_FEA2N000);
	showText(TX_SPEAKER_STRAGEY, TX_FEA2_010);
	showText(TX_SPEAKER_MCCOY,   TX_FEA2_005);
}

void Room::feather2UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_FEA2_001);
}

void Room::feather2WalkToLeftExit() {
	walkCrewman(OBJECT_KIRK, 0x14, 0x96);
}

void Room::feather2LookAtEyes() {
	showDescription(TX_FEA2N001);
}

void Room::feather2LookAtBigTree() {
	showDescription(TX_FEA2N006);
}

void Room::feather2LookAtTrees() {
	showDescription(TX_FEA2N007);
}

void Room::feather2LookAnywhere() {
	showDescription(TX_FEA2N008);
}

void Room::feather2LookAtKirk() {
	showDescription(TX_FEA2N002);
}

void Room::feather2LookAtSpock() {
	showDescription(TX_FEA2N005);
}

void Room::feather2LookAtMccoy() {
	showDescription(TX_FEA2N004);
}

void Room::feather2LookAtRedshirt() {
	showDescription(TX_FEA2N003);
}

}
