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
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::trial4Tick1() {
	playVoc("TRI4LOOP");

	loadActorAnim2(OBJECT_VLICT, "vlict1", 0x9f, 0x48);
	loadActorAnim2(OBJECT_GUARD, "kgstnd", 0xdc, 0x6a);
	loadActorAnim2(OBJECT_QUETZECOATL, "qstand", 0x10e, 0xaa);

	playMidiMusicTracks(MIDITRACK_32, -1);
}

void Room::trial4Tick60() {
	showText(TX_SPEAKER_VLICT, 61, true);

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		22, 39, 32,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices, true);

	if (choice == 0) { // "You were trying to kill us"
		showText(TX_SPEAKER_VLICT, 64, true);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			33, 9, 2,
			TX_BLANK
		};
		showMultipleTexts(choices2, true); // choice doesn't matter

		showText(TX_SPEAKER_VLICT, 62, true);
		showText(TX_SPEAKER_KIRK,  14, true);
		showText(TX_SPEAKER_VLICT, 66, true);
		showText(TX_SPEAKER_QUETZECOATL, 43, true);

		const TextRef choices3[] = {
			TX_SPEAKER_KIRK,
			29, 20, 30,
			TX_BLANK
		};
		choice = showMultipleTexts(choices3, true);

		if (choice == 0) { // "Then you have my deepest respect"
			// Nothing more
		} else if (choice == 1) { // "Make it a painless death"
			showText(TX_SPEAKER_VLICT, 63, true);
		} else if (choice == 2) { // "I hope you chose the right time"
			showText(TX_SPEAKER_QUETZECOATL, 40, true);
		}
	} else if (choice == 1) { // "Use of guile is honorable?"
		showText(TX_SPEAKER_VLICT, 60, true);
		showText(TX_SPEAKER_KIRK,  13, true);
		showText(TX_SPEAKER_VLICT, 72, true);
		showText(TX_SPEAKER_KIRK,  15, true);
		showText(TX_SPEAKER_VLICT, 71, true);
		showText(TX_SPEAKER_KIRK,  05, true);
		showText(TX_SPEAKER_QUETZECOATL, 52, true);
		showText(TX_SPEAKER_QUETZECOATL, 41, true);
		showText(TX_SPEAKER_QUETZECOATL, 51, true);
		showText(TX_SPEAKER_QUETZECOATL, 53, true);
		showText(TX_SPEAKER_QUETZECOATL, 50, true);
		showText(TX_SPEAKER_VLICT, 70, true);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			19, 25, 8,
			TX_BLANK
		};
		choice = showMultipleTexts(choices2, true);

		if (choice == 1) { // Threatened Vlict
			showText(TX_SPEAKER_VLICT, 69, true);
			showText(TX_SPEAKER_KIRK,  04, true);
		} // else, nothing more
	} else if (choice == 2) { // "You didn't say I couldn't use my ship"
		showText(TX_SPEAKER_VLICT, 58, true);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			27, 34, 18,
			TX_BLANK
		};
		choice = showMultipleTexts(choices2, true);

		if (choice == 0) { // "Let's stop playing games"
			showText(TX_SPEAKER_VLICT, 75, true);
			showText(TX_SPEAKER_KIRK,  17, true);
			showText(TX_SPEAKER_QUETZECOATL,  42, true);
			showText(TX_SPEAKER_VLICT, 54, true);

			const TextRef choices3[] = {
				TX_SPEAKER_KIRK,
				28, 21, 31,
				TX_BLANK
			};
			choice = showMultipleTexts(choices3, true);

			if (choice == 0 || choice == 1) {
				showText(TX_SPEAKER_VLICT, 68, true);
				showText(TX_SPEAKER_MCCOY, 36, true);
			} else if (choice == 2) {
				showText(TX_SPEAKER_VLICT, 56, true);
				showText(TX_SPEAKER_KIRK, 01, true);
			}
		} else if (choice == 1) { // "You lost, release Quetzecoatl"
			showText(TX_SPEAKER_VLICT, 76, true);
			showText(TX_SPEAKER_MCCOY, 37, true);
			showText(TX_SPEAKER_VLICT, 73, true);
			showText(TX_SPEAKER_KIRK,  11, true);
			showText(TX_SPEAKER_VLICT, 59, true);
		} else if (choice == 2) { // "Nature beat me to it"
			showText(TX_SPEAKER_VLICT, 65, true);
			showText(TX_SPEAKER_KIRK,  16, true);
			showText(TX_SPEAKER_VLICT, 74, true);

			const TextRef choices3[] = {
				TX_SPEAKER_KIRK,
				26, 24, 23,
				TX_BLANK
			};
			choice = showMultipleTexts(choices3, true);

			if (choice == 0) { // "The empire will learn you betrayed them"
				showText(TX_SPEAKER_VLICT, 57, true);
				showText(TX_SPEAKER_SPOCK, 38, true);
				showText(TX_SPEAKER_QUETZECOATL, 44, true);
				showText(TX_SPEAKER_KIRK, 06, true);
				showText(TX_SPEAKER_QUETZECOATL, 48, true);
			} else if (choice == 1) { // "You didn't give him a fair trial"
				showText(TX_SPEAKER_VLICT,       55, true);
				showText(TX_SPEAKER_QUETZECOATL, 45, true);
				showText(TX_SPEAKER_KIRK,        03, true);
				showText(TX_SPEAKER_QUETZECOATL, 47, true);
				showText(TX_SPEAKER_KIRK,        10, true);
			} else if (choice == 2) { // "How can a liar like you say 'honor'"
				showText(TX_SPEAKER_VLICT,       67, true);
				showText(TX_SPEAKER_QUETZECOATL, 46, true);
				showText(TX_SPEAKER_KIRK,        07, true);
				showText(TX_SPEAKER_QUETZECOATL, 49, true);
			}
		}
	}

	showText(TX_SPEAKER_KIRK, 12, true);
	_awayMission->trial.missionEndMethod = 2;
	endMission(_awayMission->trial.missionScore, _awayMission->trial.field2b, 2);
}

}
