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

#define HOTSPOT_BENCH 0x23
#define HOTSPOT_WINDOW 0x25
#define HOTSPOT_CENTER 0x26

namespace StarTrek {

extern const RoomAction trial0ActionList[] = {
	{ {ACTION_TICK, 1,  0, 0}, &Room::trial0Tick1 },
	{ {ACTION_TICK, 40, 0, 0}, &Room::trial0Tick40 },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::trial0Timer0Expired },

	{ {ACTION_WALK, HOTSPOT_CENTER, 0, 0}, &Room::trial0WalkToRoomCenter },
	{ {ACTION_DONE_WALK, 1, 0, 0},         &Room::trial0ReachedRoomCenter },

	{ {ACTION_LOOK, HOTSPOT_WINDOW,     0, 0}, &Room::trial0LookAtWindow },
	{ {ACTION_LOOK, OBJECT_VLICT,       0, 0}, &Room::trial0LookAtVlict },
	{ {ACTION_LOOK, OBJECT_GUARD,       0, 0}, &Room::trial0LookAtGuard },
	{ {ACTION_LOOK, HOTSPOT_BENCH,      0, 0}, &Room::trial0LookAtBench },
	{ {ACTION_LOOK, HOTSPOT_CENTER,     0, 0}, &Room::trial0LookAtSeal },
	{ {ACTION_LOOK, OBJECT_KIRK,        0, 0}, &Room::trial0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,       0, 0}, &Room::trial0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,       0, 0}, &Room::trial0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,    0, 0}, &Room::trial0LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_QUETZECOATL, 0, 0}, &Room::trial0LookAtQuetzecoatl },
	{ {ACTION_TALK, OBJECT_VLICT,       0, 0}, &Room::trial0TalkToVlict },
	{ {ACTION_TALK, OBJECT_GUARD,       0, 0}, &Room::trial0TalkToGuard },
	{ {ACTION_TALK, OBJECT_QUETZECOATL, 0, 0}, &Room::trial0TalkToQuetzecoatl },
	{ {ACTION_TALK, OBJECT_MCCOY,       0, 0}, &Room::trial0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK,       0, 0}, &Room::trial0TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT,    0, 0}, &Room::trial0TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0}, &Room::trial0UsePhaserOrRock },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0}, &Room::trial0UsePhaserOrRock },
	{ {ACTION_USE, OBJECT_IROCK,    0xff, 0}, &Room::trial0UsePhaserOrRock },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_VLICT, 0}, &Room::trial0UseSnakeOnVlict },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_GUARD, 0}, &Room::trial0UseSnakeOnGuard },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_QUETZECOATL, 0}, &Room::trial0UseMTricorderOnQuetzecoatl },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_VLICT, 0}, &Room::trial0UseMTricorderOnVlict },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::trial0UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WINDOW, 0}, &Room::trial0UseSTricorderOnWindow },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::trial0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::trial0UseCommunicator },

	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_GUARD, 0}, &Room::trial0UseCrewmanOnKlingon },
	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_GUARD, 0}, &Room::trial0UseCrewmanOnKlingon },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_GUARD, 0}, &Room::trial0UseCrewmanOnKlingon },
	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_VLICT, 0}, &Room::trial0UseCrewmanOnKlingon },
	{ {ACTION_USE, OBJECT_KIRK,     OBJECT_VLICT, 0}, &Room::trial0UseCrewmanOnKlingon },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_VLICT, 0}, &Room::trial0UseCrewmanOnKlingon },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_VLICT, 0}, &Room::trial0UseMccoyOnVlict },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_GUARD, 0}, &Room::trial0UseMccoyOnGuard },
	{ {ACTION_USE, OBJECT_IMEDKIT,  0xff, 0}, &Room::trial0UseMedkitAnywhere },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::trial0Tick1() {
	playVoc("TRI0LOOP");
	_awayMission->timers[0] = getRandomWordInRange(180, 600);
	_awayMission->trial.field2b = 0x1d;
	loadActorAnim2(OBJECT_VLICT, "vlict1", 0x9f, 0x48);
	loadActorAnim2(OBJECT_GUARD, "kgstnd", 0xdc, 0x6a);
	loadActorAnim2(OBJECT_QUETZECOATL, "qteleg", 0x10e, 0xaa);
}

void Room::trial0Tick40() {
	showText(TX_SPEAKER_VLICT, 58, true);
}

void Room::trial0Timer0Expired() { // Doesn't do anything?
	_awayMission->timers[0] = getRandomWordInRange(180, 600);
}

void Room::trial0WalkToRoomCenter() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	walkCrewmanC(OBJECT_KIRK, 0xa0, 0xaa, &Room::trial0ReachedRoomCenter);
}

void Room::trial0ReachedRoomCenter() {
	bool gaveUp = false;

	_awayMission->disableInput = false;
	showText(TX_SPEAKER_VLICT, 53, true);

	const TextRef choices1[] = {
		TX_SPEAKER_KIRK,
		19, 16, 21,
		TX_BLANK
	};
	showMultipleTexts(choices1, true);

	showText(TX_SPEAKER_VLICT, 54, true);
	showText(TX_SPEAKER_KIRK,   4, true);
	showText(TX_SPEAKER_VLICT, 47, true);

	const TextRef choices2[] = {
		TX_SPEAKER_KIRK,
		11, 18, 15,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices2, true);

	if (choice == 0) { // Kirk intervenes as a warrior
		showText(TX_SPEAKER_VLICT, 43, true);
		showText(TX_SPEAKER_KIRK,   2, true);
		showText(TX_SPEAKER_VLICT, 56, true);
		showText(TX_SPEAKER_VLICT, 61, true);
		showText(TX_SPEAKER_KIRK,   1, true);
		showText(TX_SPEAKER_VLICT, 60, true);
		_awayMission->disableInput = true;
		loadRoomIndex(1, 4);
	} else if (choice == 1) { // "This trial is a mockery"
		showText(TX_SPEAKER_VLICT,        48, true);
		showText(TX_SPEAKER_QUETZECOATL,  41, true);
		showText(TX_SPEAKER_VLICT,        50, true);
		showText(TX_SPEAKER_QUETZECOATL,  39, true);
		showText(TX_SPEAKER_VLICT,        59, true);
		showText(TX_SPEAKER_QUETZECOATL,  40, true);
		showText(TX_SPEAKER_VLICT,        46, true);

		const TextRef choices3[] = {
			TX_SPEAKER_KIRK,
			23, 6, 10,
			TX_BLANK
		};
		choice = showMultipleTexts(choices3, true);

		if (choice == 0 || choice == 1) {
			showText(TX_SPEAKER_VLICT, 52, true);

			const TextRef choices4[] = {
				TX_SPEAKER_KIRK,
				17, 20, 8,
				TX_BLANK
			};
			choice = showMultipleTexts(choices4, true);

			if (choice == 0 || choice == 1) {
				showText(TX_SPEAKER_VLICT, 57, true);
				showText(TX_SPEAKER_KIRK,   3, true);
				showText(TX_SPEAKER_VLICT, 51, true);

				const TextRef choices5[] = {
					TX_SPEAKER_KIRK,
					9, 14, 13,
					TX_BLANK
				};
				choice = showMultipleTexts(choices5, true);

				if (choice == 0 || choice == 1) {
					showText(TX_SPEAKER_VLICT, 55, true);
					_awayMission->disableInput = true;
					loadRoomIndex(1, 4);
				} else { // choice == 2
					showText(TX_SPEAKER_VLICT, 45, true);
					_awayMission->disableInput = true;
					loadRoomIndex(1, 4);
				}
			} else // choice == 2
				gaveUp = true;
		} else // choice == 2
			gaveUp = true;
	} else // choice == 2
		gaveUp = true;

	if (gaveUp) {
		playMidiMusicTracks(2, -1);
		showText(TX_SPEAKER_VLICT, 49, true);
		showText(TX_SPEAKER_VLICT, 44, true);
		_awayMission->trial.missionEndMethod = 0;

		// FIXME: Are these parameters in the right order?
		endMission(_awayMission->trial.missionScore, _awayMission->trial.field2b, 0);
	}
}

void Room::trial0LookAtWindow() {
	showDescription(7, true);
}

void Room::trial0LookAtVlict() {
	showDescription(1, true);
}

void Room::trial0LookAtGuard() {
	showDescription(0, true);
}

void Room::trial0LookAtBench() {
	showDescription(6, true);
}

void Room::trial0LookAtSeal() {
	showDescription(8, true);
}

void Room::trial0LookAtKirk() {
	showDescription(4, true);
}

void Room::trial0LookAtSpock() {
	showDescription(5, true);
}

void Room::trial0LookAtMccoy() {
	showDescription(2, true);
}

void Room::trial0LookAtRedshirt() {
	showDescription(3, true);
}

void Room::trial0LookAtQuetzecoatl() {
	showDescription(9, true);
}

void Room::trial0TalkToVlict() {
	showText(TX_SPEAKER_VLICT, 62, true);
}

void Room::trial0TalkToGuard() {
	showText(TX_SPEAKER_KLINGON_GUARD, 24 + FOLLOWUP_MESSAGE_OFFSET, true);
	showText(TX_SPEAKER_BENNIE, 65, true);
	showText(TX_SPEAKER_KIRK,    7, true);
	showText(TX_SPEAKER_BENNIE, 64, true);
	showText(TX_SPEAKER_KIRK,   22, true);
	showText(TX_SPEAKER_BENNIE, 66, true);
}

void Room::trial0TalkToQuetzecoatl() {
	showText(TX_SPEAKER_QUETZECOATL, 42, true);
}

void Room::trial0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 31, true);
}

void Room::trial0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 25, true);
}

void Room::trial0TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, 63, true);
}

void Room::trial0UsePhaserOrRock() {
	showText(TX_SPEAKER_SPOCK, 26, true);
	showText(TX_SPEAKER_MCCOY, 36, true);
}

void Room::trial0UseSnakeOnVlict() {
	// Unused, since the snake item doesn't carry over to the courtroom
	showText(TX_SPEAKER_VLICT, TX_TRI4_076);
}

void Room::trial0UseSnakeOnGuard() {
	// Unused, since the snake item doesn't carry over to the courtroom
	showText(TX_SPEAKER_KLINGON_GUARD, 24 + FOLLOWUP_MESSAGE_OFFSET, true);
}

void Room::trial0UseMTricorderOnQuetzecoatl() {
	mccoyScan(DIR_S, 29, false, true);
}

void Room::trial0UseMTricorderOnVlict() {
	mccoyScan(DIR_S, 30, false, true);
}

void Room::trial0UseMTricorderAnywhere() {
	mccoyScan(DIR_S, 35, false, true);
}

void Room::trial0UseSTricorderOnWindow() {
	spockScan(DIR_S, 37, false, true);
}

void Room::trial0UseSTricorderAnywhere() {
	// ENHANCEMENT: Originally didn't play tricorder sound, etc
	spockScan(DIR_S, 38, false, true);
}

void Room::trial0UseCommunicator() {
	showText(TX_SPEAKER_UHURA, 75, true);
}

void Room::trial0UseCrewmanOnKlingon() {
	showText(TX_SPEAKER_SPOCK, 24, true);
}

void Room::trial0UseMccoyOnVlict() {
	showText(TX_SPEAKER_MCCOY, 27, true);
}

void Room::trial0UseMccoyOnGuard() {
	showText(TX_SPEAKER_MCCOY, 28, true);
}

void Room::trial0UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, 34, true);
}

}
