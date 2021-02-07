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

#if 0
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

enum trial4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets trial4TextOffsets[] = {
	//{ TX_SPEAKER_KIRK, 1064, 0 },
	//{ TX_SPEAKER_MCCOY, 1075, 0 },
	//{ TX_SPEAKER_SPOCK, 1085, 0 },
	{          -1, 0,    0 }
};

extern const RoomText trial4Texts[] = {
    { -1, Common::UNK_LANG, "" }
};

void Room::trial4Tick1() {
	playVoc("TRI4LOOP");

	loadActorAnim2(OBJECT_VLICT, "vlict1", 0x9f, 0x48);
	loadActorAnim2(OBJECT_GUARD, "kgstnd", 0xdc, 0x6a);
	loadActorAnim2(OBJECT_QUETZECOATL, "qstand", 0x10e, 0xaa);

	playMidiMusicTracks(MIDITRACK_32, -1);
}

void Room::trial4Tick60() {
	showText(TX_SPEAKER_VLICT, 61);

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		22, 39, 32,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices);

	if (choice == 0) { // "You were trying to kill us"
		showText(TX_SPEAKER_VLICT, 64);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			33, 9, 2,
			TX_BLANK
		};
		showMultipleTexts(choices2); // choice doesn't matter

		showText(TX_SPEAKER_VLICT, 62);
		showText(TX_SPEAKER_KIRK,  14);
		showText(TX_SPEAKER_VLICT, 66);
		showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 43);

		const TextRef choices3[] = {
			TX_SPEAKER_KIRK,
			29, 20, 30,
			TX_BLANK
		};
		choice = showMultipleTexts(choices3);

		if (choice == 0) { // "Then you have my deepest respect"
			// Nothing more
		} else if (choice == 1) { // "Make it a painless death"
			showText(TX_SPEAKER_VLICT, 63);
		} else if (choice == 2) { // "I hope you chose the right time"
			showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 40);
		}
	} else if (choice == 1) { // "Use of guile is honorable?"
		showText(TX_SPEAKER_VLICT, 60);
		showText(TX_SPEAKER_KIRK,  13);
		showText(TX_SPEAKER_VLICT, 72);
		showText(TX_SPEAKER_KIRK,  15);
		showText(TX_SPEAKER_VLICT, 71);
		showText(TX_SPEAKER_KIRK,  05);
		showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 52);
		showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 41);
		showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 51);
		showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 53);
		showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 50);
		showText(TX_SPEAKER_VLICT, 70);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			19, 25, 8,
			TX_BLANK
		};
		choice = showMultipleTexts(choices2);

		if (choice == 1) { // Threatened Vlict
			showText(TX_SPEAKER_VLICT, 69);
			showText(TX_SPEAKER_KIRK,  04);
		} // else, nothing more
	} else if (choice == 2) { // "You didn't say I couldn't use my ship"
		showText(TX_SPEAKER_VLICT, 58);

		const TextRef choices2[] = {
			TX_SPEAKER_KIRK,
			27, 34, 18,
			TX_BLANK
		};
		choice = showMultipleTexts(choices2);

		if (choice == 0) { // "Let's stop playing games"
			showText(TX_SPEAKER_VLICT, 75);
			showText(TX_SPEAKER_KIRK,  17);
			showText(TX_SPEAKER_QUETZECOATL_GLOBAL,  42);
			showText(TX_SPEAKER_VLICT, 54);

			const TextRef choices3[] = {
				TX_SPEAKER_KIRK,
				28, 21, 31,
				TX_BLANK
			};
			choice = showMultipleTexts(choices3);

			if (choice == 0 || choice == 1) {
				showText(TX_SPEAKER_VLICT, 68);
				showText(TX_SPEAKER_MCCOY, 36);
			} else if (choice == 2) {
				showText(TX_SPEAKER_VLICT, 56);
				showText(TX_SPEAKER_KIRK, 01);
			}
		} else if (choice == 1) { // "You lost, release Quetzecoatl"
			showText(TX_SPEAKER_VLICT, 76);
			showText(TX_SPEAKER_MCCOY, 37);
			showText(TX_SPEAKER_VLICT, 73);
			showText(TX_SPEAKER_KIRK,  11);
			showText(TX_SPEAKER_VLICT, 59);
		} else if (choice == 2) { // "Nature beat me to it"
			showText(TX_SPEAKER_VLICT, 65);
			showText(TX_SPEAKER_KIRK,  16);
			showText(TX_SPEAKER_VLICT, 74);

			const TextRef choices3[] = {
				TX_SPEAKER_KIRK,
				26, 24, 23,
				TX_BLANK
			};
			choice = showMultipleTexts(choices3);

			if (choice == 0) { // "The empire will learn you betrayed them"
				showText(TX_SPEAKER_VLICT, 57);
				showText(TX_SPEAKER_SPOCK, 38);
				showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 44);
				showText(TX_SPEAKER_KIRK, 06);
				showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 48);
			} else if (choice == 1) { // "You didn't give him a fair trial"
				showText(TX_SPEAKER_VLICT,       55);
				showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 45);
				showText(TX_SPEAKER_KIRK,        03);
				showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 47);
				showText(TX_SPEAKER_KIRK,        10);
			} else if (choice == 2) { // "How can a liar like you say 'honor'"
				showText(TX_SPEAKER_VLICT,       67);
				showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 46);
				showText(TX_SPEAKER_KIRK,        07);
				showText(TX_SPEAKER_QUETZECOATL_GLOBAL, 49);
			}
		}
	}

	showText(TX_SPEAKER_KIRK, 12);
	_awayMission->trial.missionEndMethod = 2;
	endMission(_awayMission->trial.missionScore, _awayMission->trial.field2b, 2);
}

}
#endif
