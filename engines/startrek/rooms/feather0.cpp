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
	{ {ACTION_TICK, 1, 0, 0},  &Room::feather0Tick1 },
	{ {ACTION_TICK, 60, 0, 0}, &Room::feather0Tick60 },
	{ {ACTION_TALK, 8, 0, 0},  &Room::feather0TalkToQuetzecoatl },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0},                &Room::feather0Timer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0},                &Room::feather0Timer1Expired },
	{ {ACTION_USE, OBJECT_IPHASERS, 8,            0}, &Room::feather0UsePhaserOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_IPHASERK, 8,            0}, &Room::feather0UsePhaserOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY, 0}, &Room::feather0UsePhaserOnMccoy },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY, 0}, &Room::feather0UsePhaserOnMccoy },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff,          0}, &Room::feather0UseMedkit },
	{ {ACTION_LOOK, 8,    0, 0}, &Room::feather0LookAtQuetzecoatl },
	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::feather0LookAtMoon },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::feather0LookAtLog },
	{ {ACTION_LOOK, 0x22, 0, 0}, &Room::feather0LookAtHut },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::feather0LookAnywhere },
	{ {ACTION_LOOK, 0x23, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x24, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x25, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x26, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x27, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x28, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x29, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x2a, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x2b, 0, 0}, &Room::feather0LookAtEyes },
	{ {ACTION_LOOK, 0x2c, 0, 0}, &Room::feather0LookAtTree },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::feather0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::feather0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::feather0LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::feather0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::feather0TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::feather0TalkToRedshirt },
	{ {ACTION_USE, OBJECT_ISTRICOR,  8, 0}, &Room::feather0UseSTricorderOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::feather0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR,  8, 0}, &Room::feather0UseMTricorderOnQuetzecoatl },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather0Tick1() {
	playVoc("FEA0LOOP");
	loadActorAnim(OBJECT_QUETZECOATL, "s5r0qb", 0xbe, 0xa6);
}

void Room::feather0Tick60() {
	playMidiMusicTracks(33);
}

void Room::feather0TalkToQuetzecoatl() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		6,
		7,
		11,
		TX_BLANK
	};

	showText(TX_SPEAKER_QUETZECOATL, 28, true);
	int choice = showMultipleTexts(choices, true);
	bool alreadyAngered = false;

	if (choice == 0) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			9,
			4,
			TX_BLANK
		};
		showText(TX_SPEAKER_QUETZECOATL, 32, true);
		choice = showMultipleTexts(choices0, true);
	} else if (choice == 1) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			2,
			12,
			TX_BLANK
		};
		showText(TX_SPEAKER_QUETZECOATL, 31, true);
		choice = showMultipleTexts(choices0, true);
	} else if (choice == 2) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			14,
			8,
			TX_BLANK
		};
		showText(TX_SPEAKER_QUETZECOATL, 33, true);
		choice = showMultipleTexts(choices0, true);

		if (choice == 0) {
			alreadyAngered = true;
			showText(TX_SPEAKER_QUETZECOATL, 24, true);
		}
	} else
		showDescription(TX_DIALOG_ERROR);

	if (!alreadyAngered) {
		if (choice == 0)
			showText(TX_SPEAKER_QUETZECOATL, 30, true);
		else // choice == 1
			showText(TX_SPEAKER_QUETZECOATL, 101, true);
		showText(TX_SPEAKER_KIRK, 10, true);
		showText(TX_SPEAKER_QUETZECOATL, 26, true);

		const TextRef choices1[] = {
			TX_SPEAKER_KIRK,
			13,
			5,
			TX_BLANK
		};
		choice = showMultipleTexts(choices1, true);

		if (choice == 0) {
			showText(TX_SPEAKER_QUETZECOATL, 29, true);
			showText(TX_SPEAKER_KIRK, 3, true);
		}

		showText(TX_SPEAKER_QUETZECOATL, 27, true);
	}

	// Quetzecoatl ultimately warps the crew away no matter what
	_awayMission->disableInput = true;
	loadActorAnim(OBJECT_QUETZECOATL, "s5r0qt");
	playMidiMusicTracks(1);
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
	showText(TX_SPEAKER_QUETZECOATL, 25, true);
}

void Room::feather0UsePhaserOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::feather0UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 108, true);
}

void Room::feather0LookAtQuetzecoatl() {
	showDescription(8, true);
}

void Room::feather0LookAtMoon() {
	showDescription(7, true);
}

void Room::feather0LookAtLog() {
	showDescription(6, true);
}

void Room::feather0LookAtHut() {
	showDescription(1, true);
}

void Room::feather0LookAnywhere() {
	showDescription(9, true);
}

void Room::feather0LookAtEyes() {
	showDescription(0, true);
}

void Room::feather0LookAtTree() {
	showDescription(3, true);
}

void Room::feather0LookAtMccoy() {
	showDescription(4, true);
}

void Room::feather0LookAtSpock() {
	showDescription(5, true);
}

void Room::feather0LookAtRedshirt() {
	showDescription(2, true);
}

void Room::feather0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 17, true);
	showText(TX_SPEAKER_SPOCK, 21, true);
}

void Room::feather0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 22, true);
	showText(TX_SPEAKER_MCCOY, 18, true);
}

void Room::feather0TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, 23, true);
}

void Room::feather0UseSTricorderOnQuetzecoatl() {
	spockScan(DIR_S, 20, false, true);
}

void Room::feather0UseSTricorderAnywhere() {
	spockScan(DIR_S, 19, false, true);
}

void Room::feather0UseMTricorderOnQuetzecoatl() {
	mccoyScan(DIR_S, 15, false, true);
}

}
