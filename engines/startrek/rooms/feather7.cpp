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

namespace StarTrek {

extern const RoomAction feather7ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0},       &Room::feather7Tick1 },
	{ {ACTION_TICK, 40, 0, 0},      &Room::feather7Tick40 },
	{ {ACTION_DONE_WALK, 1, 0, 0},  &Room::feather7KirkReachedSeat },
	{ {ACTION_DONE_WALK, 2, 0, 0},  &Room::feather7SpockReachedSeat },
	{ {ACTION_DONE_WALK, 3, 0, 0},  &Room::feather7MccoyReachedSeat },
	{ {ACTION_DONE_ANIM, 4, 0, 0},  &Room::feather7QuetzecoatlReachedSeat },
	{ {ACTION_DONE_ANIM, 1, 0, 0},  &Room::feather7KirkSatDown },
	{ {ACTION_DONE_ANIM, 2, 0, 0},  &Room::feather7ReadyToBeamOut },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather7Tick1() {
	playVoc("FEA7LOOP");
	playMidiMusicTracks(33);
	_awayMission->disableInput = 2;
	loadActorAnim(OBJECT_QUETZECOATL, "s5r7qn", 0x106, 0x98);
}

void Room::feather7Tick40() {
	if (_awayMission->feather.diedFromStalactites)
		showText(TX_SPEAKER_QUETZECOATL, 17, true);

	showText(TX_SPEAKER_QUETZECOATL, 22, true);

	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_QUETZECOATL, 21, true);

	// BUGFIX: Show this even if redshirt isn't dead (he wishes you wouldn't have knocked
	// out Tlaoxac)
	if (_awayMission->feather.knockedOutTlaoxac)
		showText(TX_SPEAKER_QUETZECOATL, 23, true);

	showText(TX_SPEAKER_QUETZECOATL, 27, true);

	walkCrewmanC(OBJECT_KIRK, 0x6c, 0x93, &Room::feather7KirkReachedSeat);
	walkCrewmanC(OBJECT_SPOCK, 0xbb, 0x8c, &Room::feather7SpockReachedSeat);
	walkCrewmanC(OBJECT_MCCOY, 0x8d, 0x8c, &Room::feather7MccoyReachedSeat);
	loadActorAnimC(OBJECT_QUETZECOATL, "s5r7qw", -1, -1, &Room::feather7QuetzecoatlReachedSeat);
}

void Room::feather7KirkReachedSeat() {
	loadActorAnimC(OBJECT_KIRK, "s5r7ks", -1, -1, &Room::feather7KirkSatDown);
}

void Room::feather7SpockReachedSeat() {
	loadActorAnim2(OBJECT_SPOCK, "s5r7ss");
}

void Room::feather7MccoyReachedSeat() {
	loadActorAnim2(OBJECT_MCCOY, "s5r7ms");
}

void Room::feather7QuetzecoatlReachedSeat() {
	loadActorAnim2(OBJECT_QUETZECOATL, "s5r7qn", 0x97, 0x98);
}

void Room::feather7KirkSatDown() {
	const TextRef choices1[] = {
		TX_SPEAKER_KIRK,
		5, 6, 10,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices1, true);

	if (choice == 0) {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, 20, true);
	} else if (choice == 1)
		showText(TX_SPEAKER_QUETZECOATL, 19, true);
	else
		showText(TX_SPEAKER_QUETZECOATL, 16, true);

	showText(TX_SPEAKER_QUETZECOATL, 29, true);

	const TextRef choices2[] = {
		TX_SPEAKER_KIRK,
		7, 4, 12,
		TX_BLANK
	};
	choice = showMultipleTexts(choices2, true);

	if (choice == 0) {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, 30, true);
	} else if (choice == 1)
		showText(TX_SPEAKER_QUETZECOATL, 18, true);
	else {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, 26, true);
	}

	showText(TX_SPEAKER_SPOCK, 14, true);
	showText(TX_SPEAKER_QUETZECOATL, 24, true);

	const TextRef choices3[] = {
		TX_SPEAKER_KIRK,
		8, 9, 11,
		TX_BLANK
	};
	choice = showMultipleTexts(choices3, true);

	if (choice == 0) {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, 25, true);
	} else if (choice == 1)
		showText(TX_SPEAKER_QUETZECOATL, 28, true);

	showText(TX_SPEAKER_QUETZECOATL, 15, true);
	showText(TX_SPEAKER_MCCOY, 13, true);
	showText(TX_SPEAKER_KIRK, 3, true);

	loadActorAnimC(OBJECT_KIRK, "s5r7ku", -1, -1, &Room::feather7ReadyToBeamOut);
	loadActorAnim2(OBJECT_SPOCK, "s5r7su");
	loadActorAnim2(OBJECT_MCCOY, "s5r7mu");
}

void Room::feather7ReadyToBeamOut() {
	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_KIRK, 2, true);
	else {
		_awayMission->feather.missionScore += 1;
		showText(TX_SPEAKER_KIRK, 1, true);
	}

	if (!_roomVar.feather.insultedQuetzecoatl)
		_awayMission->feather.missionScore += 2;
	_awayMission->feather.missionScore += 4;

	loadActorAnim2(OBJECT_QUETZECOATL, "s5r7qt");
	endMission(_awayMission->feather.missionScore, 0x13, 0);
}

}
