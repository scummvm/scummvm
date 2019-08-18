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
};

extern const int feather7NumActions = ARRAYSIZE(feather7ActionList);


void Room::feather7Tick1() {
	playVoc("FEA7LOOP");
	playMidiMusicTracks(33);
	_awayMission->disableInput = 2;
	loadActorAnim(OBJECT_QUETZECOATL, "s5r7qn", 0x106, 0x98);
}

void Room::feather7Tick40() {
	if (_awayMission->feather.diedFromStalactites)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_017);

	showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_022);

	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_021);

	// BUGFIX: Show this even if redshirt isn't dead (he wishes you wouldn't have knocked
	// out Tlaoxac)
	if (_awayMission->feather.knockedOutTlaoxac)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_023);

	showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_027);

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
		TX_FEA7_005, TX_FEA7_006, TX_FEA7_010,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices1);

	if (choice == 0) {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_020);
	} else if (choice == 1)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_019);
	else
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_016);

	showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_029);

	const TextRef choices2[] = {
		TX_SPEAKER_KIRK,
		TX_FEA7_007, TX_FEA7_004, TX_FEA7_012,
		TX_BLANK
	};
	choice = showMultipleTexts(choices2);

	if (choice == 0) {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_030);
	} else if (choice == 1)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_018);
	else {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_026);
	}

	showText(TX_SPEAKER_SPOCK, TX_FEA7_014);
	showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_024);

	const TextRef choices3[] = {
		TX_SPEAKER_KIRK,
		TX_FEA7_008, TX_FEA7_009, TX_FEA7_011,
		TX_BLANK
	};
	choice = showMultipleTexts(choices3);

	if (choice == 0) {
		_roomVar.feather.insultedQuetzecoatl = true;
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_025);
	} else if (choice == 1)
		showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_028);

	showText(TX_SPEAKER_QUETZECOATL, TX_FEA7_015);
	showText(TX_SPEAKER_MCCOY, TX_FEA7_013);
	showText(TX_SPEAKER_KIRK, TX_FEA7_003);

	loadActorAnimC(OBJECT_KIRK, "s5r7ku", -1, -1, &Room::feather7ReadyToBeamOut);
	loadActorAnim2(OBJECT_SPOCK, "s5r7su");
	loadActorAnim2(OBJECT_MCCOY, "s5r7mu");
}

void Room::feather7ReadyToBeamOut() {
	if (_awayMission->redshirtDead)
		showText(TX_SPEAKER_KIRK, TX_FEA7_002);
	else {
		_awayMission->feather.missionScore += 1;
		showText(TX_SPEAKER_KIRK, TX_FEA7_001);
	}

	if (!_roomVar.feather.insultedQuetzecoatl)
		_awayMission->feather.missionScore += 2;
	_awayMission->feather.missionScore += 4;

	loadActorAnim2(OBJECT_QUETZECOATL, "s5r7qt");
	endMission(_awayMission->feather.missionScore, 0x13, 0);
}

}
