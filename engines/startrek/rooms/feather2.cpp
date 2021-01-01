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

#define HOTSPOT_EYES_1   0x20
#define HOTSPOT_EYES_2   0x21
#define HOTSPOT_EYES_3   0x22
#define HOTSPOT_BIG_TREE 0x23
#define HOTSPOT_TREES    0x24
#define HOTSPOT_VINES 0x25
#define HOTSPOT_LEFT_EXIT 0x26

namespace StarTrek {

extern const RoomAction feather2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0},                 &Room::feather2Tick1 },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0},    &Room::feather2UseCommunicator },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0}, &Room::feather2UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0}, &Room::feather2UsePhaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::feather2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::feather2UseMTricorderAnywhere },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0},      &Room::feather2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0},      &Room::feather2TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0},   &Room::feather2TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_VINES, 0, 0},     &Room::feather2LookAtVines },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},  &Room::feather2UseMedkit },
	{ {ACTION_WALK, HOTSPOT_LEFT_EXIT, 0, 0}, &Room::feather2WalkToLeftExit },
	{ {ACTION_LOOK, HOTSPOT_EYES_1,    0, 0}, &Room::feather2LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_2,    0, 0}, &Room::feather2LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_EYES_3,    0, 0}, &Room::feather2LookAtEyes },
	{ {ACTION_LOOK, HOTSPOT_BIG_TREE,  0, 0}, &Room::feather2LookAtBigTree },
	{ {ACTION_LOOK, HOTSPOT_TREES,     0, 0}, &Room::feather2LookAtTrees },
	{ {ACTION_LOOK, 0xff,              0, 0}, &Room::feather2LookAnywhere },
	{ {ACTION_LOOK, OBJECT_KIRK,       0, 0}, &Room::feather2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,      0, 0}, &Room::feather2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,      0, 0}, &Room::feather2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,   0, 0}, &Room::feather2LookAtRedshirt },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather2Tick1() {
	playVoc("FEA2LOOP");
	playMidiMusicTracks(27);
}

void Room::feather2UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, 6, true);
}

void Room::feather2UsePhaser() {
	// FIXME: Why does McCoy say "They're dead, Jim"?
	showText(TX_SPEAKER_MCCOY, 3, true);
}

void Room::feather2UseSTricorderAnywhere() {
	spockScan(DIR_S, 7, true);
}

void Room::feather2UseMTricorderAnywhere() {
	// ENHANCEMENT: Original didn't play tricorder sound, etc
	mccoyScan(DIR_S, 2, true);
}

void Room::feather2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 4, true);
}

void Room::feather2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 8, true);
}

void Room::feather2TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 9, true);
}

void Room::feather2LookAtVines() {
	// NOTE: This might be unused? I can't find where HOTSPOT_VINES is supposed to be.
	showDescription(0, true);
	showText(TX_SPEAKER_STRAGEY, 10, true);
	showText(TX_SPEAKER_MCCOY,   5, true);
}

void Room::feather2UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 1, true);
}

void Room::feather2WalkToLeftExit() {
	walkCrewman(OBJECT_KIRK, 0x14, 0x96);
}

void Room::feather2LookAtEyes() {
	showDescription(1, true);
}

void Room::feather2LookAtBigTree() {
	showDescription(6, true);
}

void Room::feather2LookAtTrees() {
	showDescription(7, true);
}

void Room::feather2LookAnywhere() {
	showDescription(8, true);
}

void Room::feather2LookAtKirk() {
	showDescription(2, true);
}

void Room::feather2LookAtSpock() {
	showDescription(5, true);
}

void Room::feather2LookAtMccoy() {
	showDescription(4, true);
}

void Room::feather2LookAtRedshirt() {
	showDescription(3, true);
}

}
