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

#define OBJECT_VLICT 8
#define OBJECT_GUARD 9
#define OBJECT_QUETZECOATL 10

namespace StarTrek {

extern const RoomAction trial4ActionList[] = {
	{ {ACTION_TICK, 1,  0, 0}, &Room::trial4Tick1 },
	{ {ACTION_TICK, 60, 0, 0}, &Room::trial4Tick60 },
};

extern const int trial4NumActions = ARRAYSIZE(trial4ActionList);


void Room::trial4Tick1() {
	playVoc("TRI4LOOP");

	loadActorAnim2(OBJECT_VLICT, "vlict1", 0x9f, 0x48);
	loadActorAnim2(OBJECT_GUARD, "kgstnd", 0xdc, 0x6a);
	loadActorAnim2(OBJECT_QUETZECOATL, "qstand", 0x10e, 0xaa);

	playMidiMusicTracks(MIDITRACK_32, -1);
}

void Room::trial4Tick60() {
	showText(TX_SPEAKER_VLICT, TX_TRI4_061);

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_TRI4_022, TX_TRI4_039, TX_TRI4_032,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices);

	if (choice == 0) { // "You were trying to kill us"
		showText(TX_SPEAKER_VLICT, TX_TRI4_064);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			TX_TRI4_033, TX_TRI4_009, TX_TRI4_002,
			TX_BLANK
		};
		showMultipleTexts(choices2); // choice doesn't matter

		showText(TX_SPEAKER_VLICT, TX_TRI4_062);
		showText(TX_SPEAKER_KIRK,  TX_TRI4_014);
		showText(TX_SPEAKER_VLICT, TX_TRI4_066);
		showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_043);

		const TextRef choices3[] = {
			TX_SPEAKER_KIRK,
			TX_TRI4_029, TX_TRI4_020, TX_TRI4_030,
			TX_BLANK
		};
		choice = showMultipleTexts(choices3);

		if (choice == 0) { // "Then you have my deepest respect"
			// Nothing more
		} else if (choice == 1) { // "Make it a painless death"
			showText(TX_SPEAKER_VLICT, TX_TRI4_063);
		} else if (choice == 2) { // "I hope you chose the right time"
			showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_040);
		}
	} else if (choice == 1) { // "Use of guile is honorable?"
		showText(TX_SPEAKER_VLICT, TX_TRI4_060);
		showText(TX_SPEAKER_KIRK,  TX_TRI4_013);
		showText(TX_SPEAKER_VLICT, TX_TRI4_072);
		showText(TX_SPEAKER_KIRK,  TX_TRI4_015);
		showText(TX_SPEAKER_VLICT, TX_TRI4_071);
		showText(TX_SPEAKER_KIRK,  TX_TRI4_005);
		showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_052);
		showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_041);
		showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_051);
		showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_053);
		showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_050);
		showText(TX_SPEAKER_VLICT, TX_TRI4_070);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			TX_TRI4_019, TX_TRI4_025, TX_TRI4_008,
			TX_BLANK
		};
		choice = showMultipleTexts(choices2);

		if (choice == 1) { // Threatened Vlict
			showText(TX_SPEAKER_VLICT, TX_TRI4_069);
			showText(TX_SPEAKER_KIRK,  TX_TRI4_004);
		} // else, nothing more
	} else if (choice == 2) { // "You didn't say I couldn't use my ship"
		showText(TX_SPEAKER_VLICT, TX_TRI4_058);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			TX_TRI4_027, TX_TRI4_034, TX_TRI4_018,
			TX_BLANK
		};
		choice = showMultipleTexts(choices2);

		if (choice == 0) { // "Let's stop playing games"
			showText(TX_SPEAKER_VLICT, TX_TRI4_075);
			showText(TX_SPEAKER_KIRK,  TX_TRI4_017);
			showText(TX_SPEAKER_QUETZECOATL,  TX_TRI4_042);
			showText(TX_SPEAKER_VLICT, TX_TRI4_054);

			const TextRef choices3[] = {
				TX_SPEAKER_KIRK,
				TX_TRI4_028, TX_TRI4_021, TX_TRI4_031,
				TX_BLANK
			};
			choice = showMultipleTexts(choices3);

			if (choice == 0 || choice == 1) {
				showText(TX_SPEAKER_VLICT, TX_TRI4_068);
				showText(TX_SPEAKER_MCCOY, TX_TRI4_036);
			} else if (choice == 2) {
				showText(TX_SPEAKER_VLICT, TX_TRI4_056);
				showText(TX_SPEAKER_KIRK, TX_TRI4_001);
			}
		} else if (choice == 1) { // "You lost, release Quetzecoatl"
			showText(TX_SPEAKER_VLICT, TX_TRI4_076);
			showText(TX_SPEAKER_MCCOY, TX_TRI4_037);
			showText(TX_SPEAKER_VLICT, TX_TRI4_073);
			showText(TX_SPEAKER_KIRK,  TX_TRI4_011);
			showText(TX_SPEAKER_VLICT, TX_TRI4_059);
		} else if (choice == 2) { // "Nature beat me to it"
			showText(TX_SPEAKER_VLICT, TX_TRI4_065);
			showText(TX_SPEAKER_KIRK,  TX_TRI4_016);
			showText(TX_SPEAKER_VLICT, TX_TRI4_074);

			const TextRef choices3[] = {
				TX_SPEAKER_KIRK,
				TX_TRI4_026, TX_TRI4_024, TX_TRI4_023,
				TX_BLANK
			};
			choice = showMultipleTexts(choices3);

			if (choice == 0) { // "The empire will learn you betrayed them"
				showText(TX_SPEAKER_VLICT, TX_TRI4_057);
				showText(TX_SPEAKER_SPOCK, TX_TRI4_038);
				showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_044);
				showText(TX_SPEAKER_KIRK, TX_TRI4_006);
				showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_048);
			} else if (choice == 1) { // "You didn't give him a fair trial"
				showText(TX_SPEAKER_VLICT,       TX_TRI4_055);
				showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_045);
				showText(TX_SPEAKER_KIRK,        TX_TRI4_003);
				showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_047);
				showText(TX_SPEAKER_KIRK,        TX_TRI4_010);
			} else if (choice == 2) { // "How can a liar like you say 'honor'"
				showText(TX_SPEAKER_VLICT,       TX_TRI4_067);
				showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_046);
				showText(TX_SPEAKER_KIRK,        TX_TRI4_007);
				showText(TX_SPEAKER_QUETZECOATL, TX_TRI4_049);
			}
		}
	}

	showText(TX_SPEAKER_KIRK, TX_TRI4_012);
	_awayMission->trial.missionEndMethod = 2;
	endMission(_awayMission->trial.missionScore, _awayMission->trial.field2b, 2);
}

}
