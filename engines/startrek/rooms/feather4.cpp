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

#define HOTSPOT_EYES     0x20
#define HOTSPOT_MUSHROOM 0x21
#define HOTSPOT_FERN     0x22
#define HOTSPOT_LIGHT_1  0x23
#define HOTSPOT_LIGHT_2  0x24
#define HOTSPOT_LIGHT_3  0x25

namespace StarTrek {

extern const RoomAction feather4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0},                            &Room::feather4Tick1 },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0},               &Room::feather4UseCommunicator },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0, 0},            &Room::feather4UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0, 0},            &Room::feather4UsePhaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_1, 0, 0}, &Room::feather4UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_2, 0, 0}, &Room::feather4UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_3, 0, 0}, &Room::feather4UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MUSHROOM, 0, 0}, &Room::feather4UseSTricorderOnMushroom },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_FERN, 0, 0},    &Room::feather4UseSTricorderOnFern },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0},            &Room::feather4UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_1, 0, 0}, &Room::feather4UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_2, 0, 0}, &Room::feather4UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_3, 0, 0}, &Room::feather4UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_MUSHROOM, 0, 0}, &Room::feather4UseMTricorderOnMushroom },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0},            &Room::feather4UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0},             &Room::feather4UseMedkitAnywhere },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0},     &Room::feather4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0},     &Room::feather4TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0},  &Room::feather4TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_EYES, 0, 0, 0},     &Room::feather4LookAtEyes },
	{ {ACTION_LOOK, 0xff, 0, 0, 0},             &Room::feather4LookAnywhere },
	{ {ACTION_LOOK, HOTSPOT_MUSHROOM, 0, 0, 0}, &Room::feather4LookAtMushroom },
	{ {ACTION_LOOK, HOTSPOT_FERN, 0, 0, 0},     &Room::feather4LookAtFern },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_1, 0, 0, 0},  &Room::feather4LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_2, 0, 0, 0},  &Room::feather4LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_3, 0, 0, 0},  &Room::feather4LookAtLight },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0},      &Room::feather4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0},     &Room::feather4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0},     &Room::feather4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0},  &Room::feather4LookAtRedshirt },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum feather4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_STRAGEY, TX_SPEAKER_QUETZECOATL,
	TX_FEA4_001, TX_FEA4_002, TX_FEA4_003, TX_FEA4_004, TX_FEA4_006,
	TX_FEA4_007, TX_FEA4_008, TX_FEA4_009, TX_FEA4_010, TX_FEA4_011,
	TX_FEA4_012, TX_FEA4_013, TX_FEA4N000, TX_FEA4N001, TX_FEA4N002,
	TX_FEA4N003, TX_FEA4N004, TX_FEA4N005, TX_FEA4N006, TX_FEA4N007,
	TX_FEA4N008
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets feather4TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 1205, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 1228, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 1238, 0, 0, 0 },
	{ TX_SPEAKER_STRAGEY, 1248, 0, 0, 0 },
	{ TX_SPEAKER_QUETZECOATL, 1216, 0, 0, 0 },
	{ TX_FEA4_001, 2525, 0, 0, 0 },
	{ TX_FEA4_002, 700, 0, 0, 0 },
	{ TX_FEA4_003, 2674, 0, 0, 0 },
	{ TX_FEA4_004, 1779, 0, 0, 0 },
	{ TX_FEA4_006, 1729, 0, 0, 0 },
	{ TX_FEA4_007, 1945, 0, 0, 0 },
	{ TX_FEA4_008, 1667, 0, 0, 0 },
	{ TX_FEA4_009, 2768, 0, 0, 0 },
	{ TX_FEA4_010, 2458, 0, 0, 0 },
	{ TX_FEA4_011, 1607, 0, 0, 0 },
	{ TX_FEA4_012, 2567, 0, 0, 0 },
	{ TX_FEA4_013, 2298, 0, 0, 0 },
	{ TX_FEA4N000, 1324, 0, 0, 0 },
	{ TX_FEA4N001, 1261, 0, 0, 0 },
	{ TX_FEA4N002, 1370, 0, 0, 0 },
	{ TX_FEA4N003, 1426, 0, 0, 0 },
	{ TX_FEA4N004, 2045, 0, 0, 0 },
	{ TX_FEA4N005, 2235, 0, 0, 0 },
	{ TX_FEA4N006, 2175, 0, 0, 0 },
	{ TX_FEA4N007, 2123, 0, 0, 0 },
	{ TX_FEA4N008, 1576, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText feather4Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::feather4Tick1() {
	playVoc("FEA4LOOP");
	playMidiMusicTracks(MIDITRACK_27);
}

void Room::feather4UseCommunicator() {
	showText(TX_SPEAKER_MCCOY, TX_FEA4N008);
}

void Room::feather4UsePhaser() {
	showText(TX_SPEAKER_SPOCK, TX_FEA4_011);
}

void Room::feather4UseSTricorderOnLight() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_FEA4_010);
}

void Room::feather4UseSTricorderOnMushroom() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_FEA4_012);
}

void Room::feather4UseSTricorderOnFern() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_FEA4_009);
}

void Room::feather4UseSTricorderAnywhere() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_FEA4_008);
}

void Room::feather4UseMTricorderOnLight() {
	mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_FEA4_001);
}

void Room::feather4UseMTricorderOnMushroom() {
	mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_FEA4_003);
}

void Room::feather4UseMTricorderAnywhere() {
	mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_FEA4_006);
}

void Room::feather4UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_FEA4_002);
}

void Room::feather4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA4_004);
}

void Room::feather4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA4_007);
}

void Room::feather4TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA4_013);
}

void Room::feather4LookAtEyes() {
	showDescription(TX_FEA4N003);
}

void Room::feather4LookAnywhere() {
	showDescription(TX_FEA4N008);
}

void Room::feather4LookAtMushroom() {
	showDescription(TX_FEA4N001);
}

void Room::feather4LookAtFern() {
	showDescription(TX_FEA4N000);
}

void Room::feather4LookAtLight() {
	showDescription(TX_FEA4N002);
}

void Room::feather4LookAtKirk() {
	showDescription(TX_FEA4N004);
}

void Room::feather4LookAtSpock() {
	showDescription(TX_FEA4N007);
}

void Room::feather4LookAtMccoy() {
	showDescription(TX_FEA4N006);
}

void Room::feather4LookAtRedshirt() {
	showDescription(TX_FEA4N005);
}

}
