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

#define HOTSPOT_EYES     0x20
#define HOTSPOT_MUSHROOM 0x21
#define HOTSPOT_FERN     0x22
#define HOTSPOT_LIGHT_1  0x23
#define HOTSPOT_LIGHT_2  0x24
#define HOTSPOT_LIGHT_3  0x25

namespace StarTrek {

extern const RoomAction feather4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0},                            &Room::feather4Tick1 },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0},               &Room::feather4UseCommunicator },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0},            &Room::feather4UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0},            &Room::feather4UsePhaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_1, 0}, &Room::feather4UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_2, 0}, &Room::feather4UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_3, 0}, &Room::feather4UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_MUSHROOM, 0}, &Room::feather4UseSTricorderOnMushroom },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_FERN, 0},    &Room::feather4UseSTricorderOnFern },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0},            &Room::feather4UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_1, 0}, &Room::feather4UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_2, 0}, &Room::feather4UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_3, 0}, &Room::feather4UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_MUSHROOM, 0}, &Room::feather4UseMTricorderOnMushroom },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0},            &Room::feather4UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},             &Room::feather4UseMedkitAnywhere },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0},     &Room::feather4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0},     &Room::feather4TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0},  &Room::feather4TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_EYES, 0, 0},     &Room::feather4LookAtEyes },
	{ {ACTION_LOOK, 0xff, 0, 0},             &Room::feather4LookAnywhere },
	{ {ACTION_LOOK, HOTSPOT_MUSHROOM, 0, 0}, &Room::feather4LookAtMushroom },
	{ {ACTION_LOOK, HOTSPOT_FERN, 0, 0},     &Room::feather4LookAtFern },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_1, 0, 0},  &Room::feather4LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_2, 0, 0},  &Room::feather4LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_3, 0, 0},  &Room::feather4LookAtLight },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0},      &Room::feather4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0},     &Room::feather4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0},     &Room::feather4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0},  &Room::feather4LookAtRedshirt },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather4Tick1() {
	playVoc("FEA4LOOP");
	playMidiMusicTracks(27);
}

void Room::feather4UseCommunicator() {
	showText(TX_SPEAKER_MCCOY, 5, true); // BUGFIX: Original played wrong audio file (TX_FEA4N008)
}

void Room::feather4UsePhaser() {
	showText(TX_SPEAKER_SPOCK, 11, true);
}

void Room::feather4UseSTricorderOnLight() {
	spockScan(DIR_W, 10, true);
}

void Room::feather4UseSTricorderOnMushroom() {
	spockScan(DIR_W, 12, true);
}

void Room::feather4UseSTricorderOnFern() {
	spockScan(DIR_W, 9, false, true);
}

void Room::feather4UseSTricorderAnywhere() {
	spockScan(DIR_W, 8, false, true);
}

void Room::feather4UseMTricorderOnLight() {
	mccoyScan(DIR_W, 1, false, true);
}

void Room::feather4UseMTricorderOnMushroom() {
	mccoyScan(DIR_W, 3, false, true);
}

void Room::feather4UseMTricorderAnywhere() {
	mccoyScan(DIR_W, 6, false, true);
}

void Room::feather4UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, 2, true);
}

void Room::feather4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 4, true);
}

void Room::feather4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 7, true);
}

void Room::feather4TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 13, true);
}

void Room::feather4LookAtEyes() {
	showDescription(3, true);
}

void Room::feather4LookAnywhere() {
	showDescription(8, true);
}

void Room::feather4LookAtMushroom() {
	showDescription(1, true);
}

void Room::feather4LookAtFern() {
	showDescription(0, true);
}

void Room::feather4LookAtLight() {
	showDescription(2, true);
}

void Room::feather4LookAtKirk() {
	showDescription(4, true);
}

void Room::feather4LookAtSpock() {
	showDescription(7, true);
}

void Room::feather4LookAtMccoy() {
	showDescription(6, true);
}

void Room::feather4LookAtRedshirt() {
	showDescription(5, true);
}

}
