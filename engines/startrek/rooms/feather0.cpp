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
		TX_FEA0_006,
		TX_FEA0_007,
		TX_FEA0_011,
		TX_BLANK
	};

	showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_028);
	int choice = showMultipleTexts(choices);
	bool alreadyAngered = false;

	if (choice == 0) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_009,
			TX_FEA0_004,
			TX_BLANK
		};
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_032);
		choice = showMultipleTexts(choices0);
	} else if (choice == 1) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_002,
			TX_FEA0_012,
			TX_BLANK
		};
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA0_031);
		choice = showMultipleTexts(choices0);
	} else if (choice == 2) {
		const TextRef choices0[] = {
			TX_SPEAKER_KIRK,
			TX_FEA0_014,
			TX_FEA0_008,
			TX_BLANK
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
			TX_BLANK
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
	spockScan(DIR_S, TX_FEA0_020, false);
}

void Room::feather0UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_FEA0_019, false);
}

void Room::feather0UseMTricorderOnQuetzecoatl() {
	mccoyScan(DIR_S, TX_FEA0_015, false);
}

}
