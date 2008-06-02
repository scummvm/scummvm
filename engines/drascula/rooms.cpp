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
 * $URL$
 * $Id$
 *
 */

#include "drascula/drascula.h"
#include "drascula/staticdata.h"

namespace Drascula {

bool DrasculaEngine::roomParse(RoomTalkAction* roomActions, int fl) {
	bool didAction = false;

	for (int i = 0; i < ARRAYSIZE(roomActions); i++) {
		if (roomActions[i].num == currentChapter ||
			roomActions[i].num == -1) {
			if (roomActions[i].action == pickedObject ||
				roomActions[i].action == kVerbDefault) {
				if (roomActions[i].objectID == fl ||
					roomActions[i].objectID == -1) {
					talk(roomActions[i].speechID);
					didAction = true;
					break;
				}
			}
		}
	}

	return didAction;
}

void DrasculaEngine::room_0() {
	static const int mirar_t[3] = {100, 101, 54};
	static const int poder_t[6] = {11, 109, 111, 110, 115, 116};

	if (roomParse(room0Actions, -1))
		return;

	// non-default actions
	if (currentChapter == 2 || currentChapter == 4 ||
		currentChapter == 5 || currentChapter == 6) {
		if (pickedObject == kVerbLook) {
			talk(mirar_t[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (pickedObject == kVerbPick) {
			talk(poder_t[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else {
			talk(poder_t[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	}
}

void DrasculaEngine::room_1(int fl) {
	if (roomParse(room1Actions, fl))
		return;

	if (pickedObject == kVerbLook && fl == 118) {
		talk(1);
		pause(10);
		talk(2);
	} else if (pickedObject == kVerbLook && fl == 120 && flags[8] == 0)
		talk(14);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_3(int fl) {
	if (roomParse(room3Actions, fl))
		return;

	if (pickedObject == kVerbTalk && fl == 129) {
		talk(23);
		pause(6);
		talk_sinc(_text[_lang][50], "50.als", "11111111111144432554433");
	} else if (pickedObject == kVerbTalk && fl == 133) {
		talk_sinc(_text[_lang][322], "322.als", "13333334125433333333");
		updateRoom();
		updateScreen();
		pause(25);
		talk(33);
	} else if (pickedObject == kVerbLook && fl == 165) {
		talk(149);
		talk(150);
	} else if (pickedObject == kVerbPick && fl == 165) {
		copyBackground(0, 0, 0,0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyRect(44, 1, hare_x, hare_y, 41, 70, drawSurface2, screenSurface);
		updateRefresh();
		updateScreen();
		pause(4);
		pickObject(10);
		flags[3] = 1;
		visible[8] = 0;
	} else if (pickedObject == 14 && fl == 166 && flags[37] == 0) {
		animation_7_2();
		pickObject(8);
	} else if (pickedObject == 14 && fl == 166 && flags[37] == 1)
		talk(323);
	else if (pickedObject == kVerbTalk && fl == 211) {
		talk(185);
		talk(186);
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_4(int fl) {
	if (roomParse(room4Actions, fl))
		return;

	if (pickedObject == kVerbMove && fl == 189 && flags[34] == 0) {
		talk(327);
		pickObject(13);
		flags[34] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_5(int fl) {
	if (roomParse(room5Actions, fl))
		return;

	if (pickedObject == kVerbLook && fl == 136 && flags[8] == 0)
		talk(14);
	else if (pickedObject == 10 && fl == 136) {
		animation_5_2();
		removeObject(10);
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_6(int fl){
	if (roomParse(room6Actions, fl))
		return;

	if (pickedObject == kVerbLook && fl==144) {
		talk(41);
		talk(42);
	} else if (pickedObject == kVerbOpen && fl == 138)
		openDoor(0, 1);
	else if (pickedObject == kVerbClose && fl == 138)
		closeDoor(0, 1);
	else if (pickedObject == kVerbOpen && fl == 143 && flags[2] == 0) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyRect(228, 102, hare_x + 5, hare_y - 1, 47, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(10);
		playSound(3);
		flags[2] = 1;
		updateRoom();
		updateScreen();
		finishSound();
	} else if (pickedObject == kVerbClose && fl == 143 && flags[2] == 1) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		flags[2] = 0;
		updateRefresh_pre();
		copyRect(228, 102, hare_x + 5, hare_y - 1, 47, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(5);
		playSound(4);
		updateRoom();
		updateScreen();
		finishSound();
	} else if (pickedObject == kVerbOpen && fl == 139 && flags[1] == 0) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyRect(267, 1, hare_x - 14, hare_y - 2, 52, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(19);
		playSound(3);
		flags[1] = 1;
		visible[4] = 1;
		visible[2] = 0;
		updateRoom();
		updateScreen();
		finishSound();
	} else if (pickedObject == kVerbPick && fl == 140) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyRect(267, 1, hare_x - 14, hare_y - 2, 52, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(19);
		pickObject(9);
		visible[4] = 0;
		flags[10] = 1;
	} else if (pickedObject == kVerbOpen && fl == 140)
		hasAnswer = 1;
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_7(int fl){
	if (pickedObject == kVerbLook && fl == 169)
		talk(44);
	else if (pickedObject == kVerbLook && fl == 164)
		talk(35);
	else if (pickedObject == kVerbPick && fl == 190) {
		pickObject(17);
		flags[35] = 1;
		visible[3] = 0;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_8(int fl) {
	if (pickedObject == kVerbLook && fl == 147 && flags[7] == 0) {
		talk(58);
		pickObject(15);
		flags[7] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else if (pickedObject == kVerbLook && fl == 147)
		talk(59);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_9(int fl){
	if (pickedObject == kVerbLook && fl == 150)
		talk(35);
	else if (pickedObject == kVerbTalk && fl == 150)
		talk(6);
	else if (pickedObject == kVerbLook && fl == 51)
		talk(60);
	else if (pickedObject == kVerbTalk && fl == 51 && flags[4] == 0)
		animation_4_2();
	else if (pickedObject == kVerbTalk && fl == 51 && flags[4] == 1)
		animation_33_2();
	else if (pickedObject == 7 && fl == 51) {
		animation_6_2();
		removeObject(7);
		pickObject(14);}
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_12(int fl){
	if (roomParse(room12Actions, fl))
		return;

	if (pickedObject == kVerbOpen && fl == 156)
		openDoor(16, 4);
	else if (pickedObject == kVerbClose && fl == 156)
		closeDoor(16, 4);
	else
		hasAnswer = 0;
}

bool DrasculaEngine::room_13(int fl) {
	if (pickedObject == kVerbLook && fl == 51) {
		talk(411);
		sentido_hare = 3;
		talk(412);
		strcpy(objName[1], "yoda");
	} else if (pickedObject == kVerbTalk && fl == 51)
		converse("op_7.cal");
	else if (pickedObject == 19 && fl == 51)
		animation_1_3();
	else if (pickedObject == 9 && fl == 51) {
		animation_2_3();
		return true;
	} else
		hasAnswer = 0;
	return false;
}

void DrasculaEngine::room_14(int fl) {
	if (roomParse(room14Actions, fl))
		return;

	if (pickedObject == kVerbTalk && fl == 54 && flags[39] == 0)
		animation_12_2();
	else if (pickedObject == kVerbTalk && fl == 54 && flags[39] == 1)
		talk(109);
	else if (pickedObject == 12 && fl == 54)
		animation_26_2();
	else if (pickedObject == kVerbTalk && fl == 52 && flags[5] == 0)
		animation_11_2();
	else if (pickedObject == kVerbTalk && fl == 52 && flags[5] == 1)
		animation_36_2();
	else if (pickedObject == kVerbTalk && fl == 53)
		animation_13_2();
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_15(int fl) {
	if (pickedObject == kVerbTalk && fl == 188)
		talk(333);
	else if (pickedObject == kVerbLook && fl == 188)
		talk(334);
	else if (pickedObject == 19 && fl == 188 && flags[27] == 0)
		talk(335);
	else if (pickedObject == 19 && fl == 188 && flags[27] == 1) {
		talk(336);
		sentido_hare = 3;
		talk(337);
		talk_sinc(_text[_lang][46], "46.als", "4442444244244");
		sentido_hare = 1;
	} else if (pickedObject == 18 && fl == 188 && flags[26] == 0) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		copyRect(133, 135, hare_x + 6, hare_y, 39, 63, drawSurface3, screenSurface);
		updateScreen();
		playSound(8);
		finishSound();
		talk(338);
		flags[27] = 0;
		pickObject(19);
		removeObject(18);
	} else if (pickedObject == kVerbMove && fl == 188 && flags[27] == 0) {
		animation_34_2();
		talk(339);
		pickObject(16);
		flags[26] = 1;
		flags[27] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else if (pickedObject == kVerbLook && fl == 205)
		talk(172);
	else if (pickedObject == kVerbLook && fl == 206)
		talk(173);
	else if (pickedObject == kVerbMove && fl == 206)
		talk(174);
	else if (pickedObject == kVerbOpen && fl == 206)
		talk(174);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_16(int fl) {
	if (pickedObject == kVerbTalk && fl == 163)
	  talk(6);
	else if (pickedObject == kVerbOpen && fl == 163)
		openDoor(17, 0);
	else if (pickedObject == kVerbClose && fl == 163)
		closeDoor(17, 0);
	else if (pickedObject == kVerbLook && fl == 183)
		talk(340);
	else if (pickedObject == kVerbTalk && fl == 183) {
		talk(341);
		pause(10);
		talk_sinc(_text[_lang][50], "50.als", "11111111111144432554433");
		pause(3);
		talk_baul(83);
	} else if (pickedObject == kVerbOpen && fl == 183) {
		openDoor(19, NO_DOOR);
		if (flags[20] == 0) {
			flags[20] = 1;
			sentido_hare = 3;
			updateRoom();
			updateScreen();
			talk(342);
			pickObject(22);
		}
	} else if (pickedObject == kVerbClose && fl == 183)
		closeDoor(19, NO_DOOR);
	else if (pickedObject == kVerbLook && fl == 185)
		talk(37);
	else if (pickedObject == kVerbPick && fl == 185)
		talk(7);
	else if (pickedObject == kVerbMove && fl == 185)
		talk(7);
	else if (pickedObject == kVerbTalk && fl == 185)
		talk(38);
	else if (pickedObject == kVerbLook && fl == 187) {
		talk(343);
		sentido_hare = 3;
		updateRoom();
		updateScreen();
		talk(344);
	} else if (pickedObject == kVerbTalk && fl == 187)
		talk(345);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_17(int fl) {
	if (pickedObject == kVerbLook && fl == 177)
		talk(35);
	else if (pickedObject == kVerbTalk && fl == 177 && flags[18] == 0)
		talk(6);
	else if (pickedObject == kVerbTalk && fl == 177 && flags[18] == 1)
		animation_18_2();
	else if (pickedObject == kVerbOpen && fl == 177 && flags[18] == 1)
		talk(346);
	else if (pickedObject == kVerbOpen && fl == 177 && flags[14] == 0 && flags[18] == 0)
		animation_22_2();
	else if (pickedObject == kVerbOpen && fl == 177 && flags[14] == 1)
		openDoor(15, 1);
	else if (pickedObject == kVerbClose && fl == 177 && flags[14] == 1)
		closeDoor(15, 1);
	else if (pickedObject == 11 && fl == 50 && flags[22] == 0) {
		talk(347);
		flags[29] = 1;
		pickObject(23);
		removeObject(11);
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_18(int fl) {
	if (pickedObject == kVerbTalk && fl == 55 && flags[36] == 0)
		animation_24_2();
	else if (pickedObject == kVerbTalk && fl == 55 && flags[36] == 1)
		talk(109);
	else if (pickedObject == kVerbLook && fl == 181)
		talk(348);
	else if (pickedObject == kVerbPick && fl == 182) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyRect(44, 1, hare_x, hare_y, 41, 70, drawSurface2, screenSurface);
		updateRefresh();
		updateScreen();
		pause(4);
		pickObject(12);
		visible[2] = 0;
		flags[28] = 1;
	} else if (pickedObject == kVerbLook && fl == 182) {
		talk(154);
	} else if (fl == 55 && flags[38] == 0 && flags[33] == 0) {
		if (pickedObject == 8 || pickedObject == 13 || pickedObject == 15 ||
			pickedObject == 16 || pickedObject == 17)
			talk(349);
	} else if (fl == 55 && flags[38] == 1 && flags[33] == 1) {
		if (pickedObject == 8 || pickedObject == 13 || pickedObject == 15 ||
			pickedObject == 16 || pickedObject == 17)
			animation_24_2();
	}
	else if (pickedObject == 11 && fl == 50 && flags[22] == 0) {
		sentido_hare = 3;
		updateRoom();
		updateScreen();
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyRect(1, 1, hare_x - 1, hare_y + 3, 42, 67, drawSurface2, screenSurface);
		updateRefresh();
		updateScreen();
		pause(6);
		talk(347);
		flags[29] = 1;
		pickObject(23);
		removeObject(11);
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_19(int fl) {
	if (pickedObject == kVerbLook && fl == 214)
		talk(191);
	else
		hasAnswer = 0;
}

bool DrasculaEngine::room_21(int fl) {
	if (pickedObject == kVerbOpen && fl == 101 && flags[28] == 0)
		talk(419);
	else if (pickedObject == kVerbOpen && fl == 101 && flags[28] == 1)
		openDoor(0, 1);
	else if (pickedObject == kVerbClose && fl == 101)
		closeDoor(0, 1);
	else if(pickedObject == kVerbPick && fl == 141) {
		pickObject(19);
		visible[2] = 0;
		flags[10] = 1;
	} else if(pickedObject == 7 && fl == 101) {
		flags[28] = 1;
		openDoor(0, 1);
		withoutVerb();
	} else if (pickedObject == 21 && fl == 179) {
		animation_9_4();
		return true;
	} else
		hasAnswer = 0;

	return false;
}

void DrasculaEngine::room_22(int fl) {
	if (pickedObject == kVerbPick && fl == 140)
		talk(7);
	else if (pickedObject == 11 && fl == 140) {
		pickObject(18);
		visible[1] = 0;
		flags[24] = 1;
	} else if (pickedObject == 22 && fl == 52) {
		animate("up.bin",14);
		flags[26]=1;
		playSound(1);
		hiccup(14);
		finishSound();
		withoutVerb();
		removeObject(22);
		updateVisible();
		sentido_hare = 3;
		talk(499);
		talk(500);
	} else if (pickedObject == kVerbLook && fl == 52)
		talk(497);
	else if (pickedObject == kVerbTalk && fl == 52)
		talk(498);
	else if (pickedObject == kVerbPick && fl == 180 && flags[26] == 0)
		talk(420);
	else if (pickedObject == kVerbPick && fl == 180 && flags[26] == 1) {
		pickObject(7);
		visible[3] = 0;
		flags[27] = 1;
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_23(int fl) {
	if (pickedObject == kVerbOpen && fl == 103) {
		openDoor(0, 0);
		updateVisible();
	} else if(pickedObject == kVerbClose && fl == 103) {
		closeDoor(0, 0);
		updateVisible();
	} else if(pickedObject == kVerbOpen && fl == 104)
		openDoor(1, 1);
	else if(pickedObject == kVerbClose && fl == 104)
		closeDoor(1, 1);
	else if(pickedObject == kVerbPick && fl == 142) {
		pickObject(8);
		visible[2] = 0;
		flags[11] = 1;
		if (flags[22] == 1 && flags[14] == 1)
			flags[18] = 1;
		if (flags[18] == 1)
			animation_6_4();
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_24(int fl) {
	if (pickedObject == kVerbOpen && fl == 105)
		openDoor(1, 0);
	else if (pickedObject == kVerbClose && fl == 105)
		closeDoor(1, 0);
	else if (pickedObject == kVerbOpen && fl == 106)
		openDoor(2, 1);
	else if (pickedObject == kVerbClose && fl == 106)
		closeDoor(2, 1);
	else if (pickedObject == kVerbLook && fl == 151)
		talk(461);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_26(int fl) {
	if (pickedObject == kVerbOpen && fl == 107 && flags[30] == 0)
		openDoor(2, 0);
	else if (pickedObject == kVerbOpen && fl == 107 && flags[30] == 1)
		talk(421);
	else if (pickedObject == kVerbClose && fl == 107)
		closeDoor(2, 0);
	else if (pickedObject == 10 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == 8 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == 12 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == 16 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == kVerbPick && fl == 143 && flags[18] == 1) {
		lleva_al_hare(260, 180);
		pickObject(10);
		visible[1] = 0;
		flags[12] = 1;
		closeDoor(2, 0);
		sentido_hare = 2;
		talk_igor(27, 2);
		flags[30] = 1;
		talk_igor(28, 2);
		lleva_al_hare(153, 180);
	} else if (pickedObject == kVerbPick && fl == 143 && flags[18] == 0) {
		lleva_al_hare(260, 180);
		copyBackground(80, 78, 199, 94, 38, 27, drawSurface3, screenSurface);
		updateScreen(199, 94, 199, 94, 38, 27, screenSurface);
		pause(3);
		talk_igor(25, 4);
		lleva_al_hare(153, 180);
	} else if (pickedObject == kVerbTalk && fl == 51)
		animation_1_4();
	else if (pickedObject == kVerbOpen && fl == 167)
		talk(467);
	else if (pickedObject == kVerbLook && fl == 164)
		talk(470);
	else if (pickedObject == kVerbOpen && fl == 164)
		talk(471);
	else if (pickedObject == kVerbLook && fl == 163)
		talk(472);
	else if (pickedObject == kVerbPick && fl == 163)
		talk(473);
	else if (pickedObject == kVerbLook && fl == 165)
		talk(474);
	else if (pickedObject == kVerbLook && fl == 168)
		talk(476);
	else if (pickedObject == kVerbPick && fl == 168)
		talk(477);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_27(int fl) {
	if (pickedObject == kVerbOpen && fl == 110)
		openDoor(6, 1);
	else if (pickedObject == kVerbClose && fl == 110)
		closeDoor(6, 1);
	else if (pickedObject == kVerbOpen && fl == 116 && flags[23] == 0)
		talk(419);
	else if (pickedObject == kVerbOpen && fl == 116 && flags[23] == 1)
		openDoor(5, 3);
	else if (pickedObject == 17 && fl == 116) {
		flags[23] = 1;
		openDoor(5,3);
		withoutVerb();
	} else if (pickedObject == kVerbLook && fl == 175)
		talk(429);
	else if (fl == 150)
		talk(460);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_29(int fl) {
	if (pickedObject == kVerbOpen && fl == 114)
		openDoor(4, 1);
	else if (pickedObject == kVerbClose && fl == 114)
		closeDoor(4, 1);
	else if (pickedObject == kVerbLook && fl == 152)
		talk(463);
	else if (pickedObject == kVerbOpen && fl == 152)
		talk(464);
	else if (pickedObject == kVerbLook && fl == 153)
		talk(465);
	else if (pickedObject == kVerbPick && fl == 154)
		talk(466);
	else if (pickedObject == kVerbOpen && fl == 156)
		talk(467);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_30(int fl) {
	if (pickedObject == kVerbOpen && fl == 115)
		openDoor(4, 0);
	else if (pickedObject == kVerbClose && fl == 115)
		closeDoor(4, 0);
	else if (pickedObject == kVerbOpen && fl == 144 && flags[19] == 0)
		talk(422);
	else if (pickedObject == kVerbOpen && fl == 144 && flags[19] == 1 && flags[22] == 1)
		openDoor(16, 1);
	else if (pickedObject == kVerbOpen && fl == 144 && flags[19] == 1 && flags[22] == 0) {
		openDoor(16, 1);
		talk(423);
		flags[22] = 1;
		pickObject(12);
		if (flags[11] == 1 && flags[14] == 1)
			flags[18] = 1;
		if (flags[18] == 1)
			animation_6_4();
	} else if (pickedObject == kVerbClose && fl == 144)
		closeDoor(16, 1);
	else if (pickedObject == 13 && fl == 144) {
		talk(424);
		flags[19] = 1;
	} else if (pickedObject == kVerbOpen && fl == 157)
		talk(468);
	else if (pickedObject == kVerbLook && fl == 158)
		talk(469);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_31(int fl) {
	if (pickedObject == kVerbPick && fl == 145) {
		pickObject(11);
		visible[1] = 0;
		flags[13] = 1;
	} else if (pickedObject == kVerbOpen && fl == 117)
		openDoor(5, 0);
	else if (pickedObject == kVerbClose && fl == 117)
		closeDoor(5, 0);
	else if (pickedObject == kVerbLook && fl == 161)
		talk(470);
	else if (pickedObject == kVerbOpen && fl == 161)
		talk(471);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_34(int fl) {
	if (pickedObject == kVerbMove && fl == 146)
		animation_8_4();
	else if (pickedObject == kVerbLook && fl == 146)
		talk(458);
	else if (pickedObject == kVerbPick && fl == 146)
		talk(459);
	else if (pickedObject == kVerbOpen && fl == 120 && flags[25] == 1)
		openDoor(8, 2);
	else if (pickedObject == kVerbOpen && fl == 120 && flags[25] == 0) {
		openDoor(8, 2);
		sentido_hare = 3;
		talk(425);
		pickObject(14);
		flags[25] = 1;
	} else if (pickedObject == kVerbClose && fl == 120)
		closeDoor(8, 2);
	else
		hasAnswer=0;
}

void DrasculaEngine::room_35(int fl) {
	if (pickedObject == kVerbPick && fl == 148) {
		pickObject(16);
		visible[2] = 0;
		flags[14] = 1;
	if (flags[11] == 1 && flags[22] == 1)
		flags[18] = 1;
	if (flags[18] == 1)
		animation_6_4();
	} else if (pickedObject == kVerbPick && fl == 147) {
		talk(426);
		pickObject(15);
		visible[1] = 0;
		flags[15] = 1;
		flags[17] = 1;
		updateVisible();
	} else if (pickedObject == kVerbPick && fl == 149) {
		pickObject(13);
		visible[3] = 0;
		flags[17] = 0;
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_44(int fl) {
	if (pickedObject == kVerbLook && fl == 172)
		talk(428);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_49(int fl){
	if (pickedObject == kVerbTalk && fl ==51)
		converse("op_9.cal");
	else if (pickedObject == kVerbLook && fl == 51)
		talk(132);
	else if ((pickedObject == 8 && fl == 51) || (pickedObject == 8 && fl == 203))
		animation_5_5();
	else if (pickedObject == kVerbLook && fl == 200)
		talk(133);
	else if (pickedObject == kVerbTalk && fl == 200)
		talk(134);
	else if (pickedObject == kVerbLook && fl == 201)
		talk(135);
	else if (pickedObject == kVerbLook && fl == 203)
		talk(137);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_53(int fl) {
	if (pickedObject == kVerbPick && fl == 120) {
		pickObject(16);
		visible[3] = 0;
	} else if (pickedObject == kVerbLook && fl == 121)
		talk(128);
	else if (pickedObject == kVerbLook && fl == 209)
		talk(129);
	else if (pickedObject == kVerbMove && fl == 123)
		animation_11_5();
	else if (pickedObject == kVerbLook && fl == 52)
		talk(447);
	else if (pickedObject == kVerbTalk && fl == 52)
		talk(131);
	else if (pickedObject == 12 && fl == 52)
		animation_10_5();
	else if (pickedObject == 15 && fl == 52)
		animation_9_5();
	else if (pickedObject == 16 && fl == 121) {
		flags[2] = 1;
		withoutVerb();
		updateVisible();
	} else if (pickedObject == 16) {
		talk(439);
		withoutVerb();
		visible[3] = 1;
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_54(int fl) {
	if ((pickedObject == kVerbTalk && fl == 118) || (pickedObject == kVerbLook && fl == 118 && flags[0] == 0))
		animation_1_5();
	else if (pickedObject == kVerbLook && fl == 118 && flags[0]==1)
		talk(124);
	else if (pickedObject == kVerbLook && fl == 53)
		talk(127);
	else if (pickedObject == kVerbTalk && fl == 53 && flags[14] == 0) {
		talk(288);
		flags[12] = 1;
		pause(10);
		talk_mus(1);
		talk(289);
		talk_mus(2);
		talk_mus(3);
		converse("op_10.cal");
		flags[12] = 0;
		flags[14] = 1;
	} else if (pickedObject == kVerbTalk && fl == 53 && flags[14] == 1)
		talk(109);
	else if (pickedObject == kVerbPick && fl == 9999 && flags[13] == 0) {
		pickObject(8);
		flags[13] = 1;
		talk_mus(10);
		updateVisible();
	} else if (pickedObject == kVerbOpen && fl == 119)
		talk(125);
	else if (pickedObject == kVerbLook && fl == 119)
		talk(126);
	else if (pickedObject == 10 && fl == 119) {
		pause(4);
		talk(436);
		withoutVerb();
		removeObject(10);
	} else
		hasAnswer = 0;
}

void DrasculaEngine::room_55(int fl) {
	if (pickedObject == kVerbPick && fl == 122) {
		pickObject(12);
		flags[8] = 1;
		updateVisible();
	} else if (pickedObject == kVerbLook && fl == 122)
		talk(138);
	else if (pickedObject == kVerbLook && fl == 204)
		talk(139);
	else if (pickedObject == kVerbLook && fl == 205)
		talk(140);
	else if (fl == 206) {
		playSound(11);
		animate("det.bin", 17);
		finishSound();
		lleva_al_hare(hare_x - 3, hare_y + alto_hare + 6);
	} else
		hasAnswer = 0;
}

bool DrasculaEngine::room_56(int fl) {
	if (pickedObject == kVerbOpen && fl == 124) {
		animation_14_5();
		return true;
	} else if (pickedObject == kVerbLook && fl == 124)
		talk(450);
	else if (pickedObject == kVerbOpen && fl == 207)
		talk(141);
	else if (pickedObject == kVerbLook && fl == 208)
		talk(142);
	else
		hasAnswer = 0;

	return false;
}

void DrasculaEngine::room_58(int fl) {
	if (pickedObject == kVerbMove && fl == 103)
		animation_7_6();
	else if (pickedObject == kVerbLook && fl == 104)
		talk(454);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_59(int fl) {
	if ((pickedObject == kVerbTalk && fl == 51) || (pickedObject == kVerbLook && fl == 51)) {
		flags[9] = 1;
		talk(259);
		talk_bj_bed(13);
		talk(263);
		talk_bj_bed(14);
		pause(40);
		talk(264);
		talk_bj_bed(15);
		talk(265);
		flags[9] = 0;
		if (flags[11] == 0) {
			playSound(12);
			delay(40);
			finishSound();
			delay(10);
			lleva_al_hare(174, 168);
			sentido_hare = 2;
			updateRoom();
			updateScreen();
			pause(40);
			playSound(12);
			pause(19);
			stopSound();
			hare_se_ve = 0;
			updateRoom();
			copyRect(101, 34, hare_x - 4, hare_y - 1, 37, 70, drawSurface3, screenSurface);
			copyBackground(0, 0, 0, 0, 320, 200, screenSurface, drawSurface1);
			updateScreen();
			hare_se_ve = 1;
			clearRoom();
			loadPic("tlef0.alg", drawSurface1, COMPLETE_PAL);
			loadPic("tlef1.alg", drawSurface3, 1);
			loadPic("tlef2.alg", frontSurface, 1);
			loadPic("tlef3.alg", backSurface, 1);
			talk_htel(240);

			color_abc(kColorBrown);
			talk_solo(_textvb[_lang][58], "VB58.als");
			talk_htel(241);
			color_abc(kColorBrown);
			talk_solo(_textvb[_lang][59], "VB59.als");
			talk_htel(242);
			color_abc(kColorBrown);
			talk_solo(_textvb[_lang][60], "VB60.als");
			talk_htel(196);
			color_abc(kColorBrown);
			talk_solo(_textvb[_lang][61],"VB61.als");
			talk_htel(244);
			color_abc(kColorBrown);
			talk_solo(_textvb[_lang][62], "VB62.als");
			clearRoom();
			loadPic("aux59.alg", drawSurface3, 1);
			loadPic("96.alg", frontSurface, COMPLETE_PAL);
			loadPic("99.alg", backSurface, 1);
			loadPic("59.alg", drawSurface1, HALF_PAL);
			sentido_hare = 3;
			talk(245);
			withoutVerb();
			flags[11] = 1;
		}
	} else
		hasAnswer = 0;
}

bool DrasculaEngine::room_60(int fl) {
	if (pickedObject == kVerbMove && fl == 112)
		animation_10_6();
	else if (pickedObject == kVerbLook && fl == 112)
		talk(440);
	else if (pickedObject == kVerbTalk && fl == 52) {
		talk(266);
		talk_bartender(1, 1);
		converse("op_12.cal");
		withoutVerb();
		pickedObject = 0;
	} else if (pickedObject == kVerbTalk && fl == 115)
		talk(455);
	else if (pickedObject == kVerbTalk && fl == 56)
		talk(455);
	else if (pickedObject == kVerbLook && fl == 114)
		talk(167);
	else if (pickedObject == kVerbLook && fl == 113)
		talk(168);
	else if (pickedObject == kVerbPick && fl == 113)
		talk(170);
	else if (pickedObject == kVerbMove && fl == 113)
		talk(170);
	else if (pickedObject == kVerbTalk && fl == 113)
		talk(169);
	else if (pickedObject == 21 && fl == 56)
		animation_18_6();
	else if (pickedObject == 9 && fl == 56 && flags[6] == 1) {
		animation_9_6();
		return true;
	} else if (pickedObject == 9 && fl == 56 && flags[6] == 0) {
		animate("cnf.bin", 14);
		talk(455);
	} else
		hasAnswer = 0;

	return false;
}

void DrasculaEngine::room_61(int fl) {
	if (pickedObject == kVerbLook && fl == 116)
		talk(172);
	else if (pickedObject == kVerbLook && fl == 117)
		talk(173);
	else if (pickedObject == kVerbMove && fl == 117)
		talk(174);
	else if (pickedObject == kVerbOpen && fl == 117)
		talk(174);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_62(int fl) {
	if (pickedObject == kVerbTalk && fl == 53)
		converse("op_13.cal");
	else if (pickedObject == kVerbTalk && fl == 52 && flags[0] == 0)
		animation_3_1();
	else if (pickedObject == kVerbTalk && fl == 52 && flags[0] == 1)
		talk(109);
	else if (pickedObject == kVerbTalk && fl == 54)
		animation_4_1();
	else if (pickedObject == kVerbLook && fl == 100)
		talk(168);
	else if (pickedObject == kVerbTalk && fl == 100)
		talk(169);
	else if (pickedObject == kVerbPick && fl == 100)
		talk(170);
	else if (pickedObject == kVerbLook && fl == 101)
		talk(171);
	else if (pickedObject == kVerbLook && fl == 102)
		talk(167);
	else if (pickedObject == kVerbLook && fl == 103)
		talk(166);
	else hasAnswer = 0;
}

void DrasculaEngine::room_63(int fl) {
	if (pickedObject == kVerbLook && fl == 110)
		talk(172);
	else if (pickedObject == kVerbLook && fl == 109)
		talk(173);
	else if (pickedObject == kVerbMove && fl == 109)
		talk(174);
	else if (pickedObject == kVerbLook && fl == 108)
		talk(334);
	else if (pickedObject == kVerbTalk && fl == 108)
		talk(333);
	else
		hasAnswer = 0;
}

void DrasculaEngine::room_pendulum(int fl) {
	if (pickedObject == kVerbLook && fl == 100)
		talk(452);
	else if (pickedObject == kVerbLook && fl == 101)
		talk(123);
	else if (pickedObject == kVerbPick && fl == 101)
		pickObject(20);
	else if (pickedObject == 20 && fl == 100)
		animation_6_6();
	else if (pickedObject == kVerbPick || pickedObject == kVerbOpen)
		talk(453);
	else
		hasAnswer = 0;
}

void DrasculaEngine::updateRefresh() {
	if (currentChapter == 1) {
		if (roomNumber == 63)
			update_63();
		else if (roomNumber == 62)
			update_62();
	} else if (currentChapter == 2) {
		if (roomNumber == 3)
			update_3();
		else if (roomNumber == 2)
			update_2();
		else if (roomNumber == 4)
			update_4();
		else if (roomNumber == 5)
			update_5();
		else if (roomNumber == 15)
			update_15();
		else if (roomNumber == 17)
			update_17();
		else if (roomNumber == 18)
			update_18();
		else if (roomNumber == 10)
			showMap();
	} else if (currentChapter == 3) {
		if (roomNumber == 20)
			update_20();
		else if (roomNumber == 13)
			update_13();
	} else if (currentChapter == 4) {
		if (roomNumber == 29)
			update_29();
		else if (roomNumber == 26)
			update_26();
		else if (roomNumber == 27)
			update_27();
		else if (roomNumber == 31)
			update_31();
		else if (roomNumber == 34)
			update_34();
		else if (roomNumber == 35)
			update_35();
	} else if (currentChapter == 5) {
		if (roomNumber == 45)
			showMap();
		else if (roomNumber == 50)
			update_50();
		else if (roomNumber == 57)
			update_57();
	} else if (currentChapter == 6) {
		if (roomNumber == 60)
			update_60();
		else if (roomNumber == 61)
			update_61();
		else if (roomNumber == 58)
			update_58();
	}
}

void DrasculaEngine::updateRefresh_pre() {
	if (currentChapter == 1) {
		if (roomNumber == 62)
			update_62_pre();
		else if (roomNumber == 16)
			placeBJ();
	} else if (currentChapter == 2) {
		if (roomNumber == 1)
			update_1_pre();
		else if (roomNumber == 3)
			update_3_pre();
		else if (roomNumber == 5)
			update_5_pre();
		else if (roomNumber == 6)
			update_6_pre();
		else if (roomNumber == 7)
			update_7_pre();
		else if (roomNumber == 9)
			update_9_pre();
		else if (roomNumber == 12)
			update_12_pre();
		else if (roomNumber == 14)
			update_14_pre();
		else if (roomNumber == 16)
			update_16_pre();
		else if (roomNumber == 17)
			update_17_pre();
		else if (roomNumber == 18)
			update_18_pre();
	} else if (currentChapter == 3) {
		// nothing
	} else if (currentChapter == 4) {
		if (roomNumber == 21)
			update_21_pre();
		else if (roomNumber == 22)
			update_22_pre();
		else if (roomNumber == 23)
			update_23_pre();
		else if (roomNumber == 24)
			update_24_pre();
		else if (roomNumber == 26)
			update_26_pre();
		else if (roomNumber == 27)
			update_27_pre();
		else if (roomNumber == 29)
			update_29_pre();
		else if (roomNumber == 30)
			update_30_pre();
		else if (roomNumber == 31)
			update_31_pre();
		else if (roomNumber == 34)
			update_34_pre();
		else if (roomNumber == 35)
			update_35_pre();
	} else if (currentChapter == 5) {
		if (roomNumber == 49)
			update_49_pre();
		else if (roomNumber == 53)
			update_53_pre();
		else if (roomNumber == 54)
			update_54_pre();
		else if (roomNumber == 56)
			update_56_pre();
	} else if (currentChapter == 6) {
		if (roomNumber == 102)
			update_pendulum();
		else if (roomNumber == 58)
			update_58_pre();
		else if (roomNumber == 59)
			update_59_pre();
		else if (roomNumber == 60)
			update_60_pre();
	}
}

void DrasculaEngine::update_1_pre() {
	int cambio_col_antes = changeColor;

	if (hare_x > 98 && hare_x < 153)
		changeColor = 1;
	else
		changeColor = 0;

	if (cambio_col_antes != changeColor && changeColor == 1)
		hare_oscuro();
	if (cambio_col_antes != changeColor && changeColor == 0)
		hare_claro();

	if (flags[8] == 0)
		copyBackground(2, 158, 208, 67, 27, 40, drawSurface3, screenSurface);
}

void DrasculaEngine::update_2(){
	int pos_murci[6];
	int difference;
	int murci_x[] = {0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190,
					0, 48, 96, 144, 192, 240,
					30, 88, 146, 204, 262,
					88, 146, 204, 262,
					88, 146, 204, 262};

	int murci_y[] = {179, 179, 179, 179, 179, 179, 179, 179,
					158, 158, 158, 158, 158, 158, 158, 158,
					137, 137, 137, 137, 137, 137,
					115, 115, 115, 115, 115, 115,
					78, 78, 78, 78, 78,
					41, 41, 41, 41,
					4, 4, 4, 4};

	if (frame_bat == 41)
		frame_bat = 0;

	pos_murci[0] = murci_x[frame_bat];
	pos_murci[1] = murci_y[frame_bat];

	if (frame_bat < 22) {
		pos_murci[4] = 37;
		pos_murci[5] = 21;
	} else if (frame_bat > 27) {
		pos_murci[4] = 57;
		pos_murci[5] = 36;
	} else {
		pos_murci[4] = 47;
		pos_murci[5] = 22;
	}

	pos_murci[2] = 239;
	pos_murci[3] = 19;

	copyRectClip(pos_murci, drawSurface3, screenSurface);
	difference = getTime() - conta_blind_vez;
	if (difference >= 6) {
		frame_bat++;
		conta_blind_vez = getTime();
	}

	copyRect(29, 37, 58, 114, 57, 39, drawSurface3, screenSurface);
	showMap();
}

void DrasculaEngine::update_3_pre() {
	if (flags[3] == 1)
		copyBackground(258, 110, 85, 44, 23, 53, drawSurface3, screenSurface);
}

void DrasculaEngine::update_3() {
	if (hare_y + alto_hare < 118)
		copyRect(129, 110, 194, 36, 126, 88, drawSurface3, screenSurface);
	copyRect(47, 57, 277, 143, 43, 50, drawSurface3, screenSurface);
}

void DrasculaEngine::update_4() {
	int cambio_col_antes = changeColor;
	if (hare_x > 190)
		changeColor = 1;
	else
		changeColor = 0;

	if (cambio_col_antes != changeColor && changeColor == 1)
		hare_oscuro();
	if (cambio_col_antes != changeColor && changeColor == 0)
		hare_claro();
}

void DrasculaEngine::update_5_pre(){
	if (flags[8] == 0)
		copyBackground(256, 152, 208, 67, 27, 40, drawSurface3, screenSurface);
}

void DrasculaEngine::update_5() {
	copyRect(114, 130, 211, 87, 109, 69, drawSurface3, screenSurface);
}

void DrasculaEngine::update_6_pre() {
	int cambio_col_antes = changeColor;

	if ((hare_x > 149 && hare_y + alto_hare > 160 && hare_x < 220 && hare_y + alto_hare < 188) ||
			(hare_x > 75 && hare_y + alto_hare > 183 && hare_x < 145))
		changeColor = 0;
	else
		changeColor = 1;

	if (cambio_col_antes != changeColor && changeColor == 1)
		hare_oscuro();
	if (cambio_col_antes != changeColor && changeColor == 0)
		hare_claro();

	if (flags[1] == 0)
		copyBackground(97, 117, 34, 148, 36, 31, drawSurface3, screenSurface);
	if (flags[0] == 0)
		copyBackground(3, 103, 185, 69, 23, 76, drawSurface3, screenSurface);
	if (flags[2] == 0)
		copyBackground(28, 100, 219, 72, 64, 97, drawSurface3, screenSurface);
}

void DrasculaEngine::update_7_pre() {
	if (flags[35] == 0)
		copyBackground(1, 72, 158, 162, 19, 12, drawSurface3, screenSurface);
}

void DrasculaEngine::update_9_pre() {
	int blind_x[] = {26, 68, 110, 152, 194, 236, 278, 26, 68};
	int blind_y[] = {51, 51, 51, 51, 51, 51, 51, 127, 127};
	int difference;

	copyRect(blind_x[frame_blind], blind_y[frame_blind], 122, 57, 41, 72, drawSurface3, screenSurface);
	if (flags[9] == 0) {
		difference = getTime() - conta_blind_vez;
		if (difference >= 11) {
			frame_blind++;
			conta_blind_vez = getTime();
		}
		if (frame_blind == 9)
			frame_blind = 0;
	} else
		frame_blind = 3;
}

void DrasculaEngine::update_12_pre() {
	if (flags[16] == 0)
		copyBackground(1, 131, 106, 117, 55, 68, drawSurface3, screenSurface);
}

void DrasculaEngine::update_13() {
	if (hare_x > 55 && flags[3] == 0)
		animation_6_3();
	if (flags[1] == 0)
		copyRect(185, 110, 121, 65, 67, 88, drawSurface3, screenSurface);
	if (flags[2] == 0)
		copyRect(185, 21, 121, 63, 67, 88, drawSurface3, screenSurface);
	copyRect(3, 127, 99, 102, 181, 71, drawSurface3, screenSurface);
}

void DrasculaEngine::update_14_pre() {
	int candles_y[] = {158, 172, 186};
	int cirio_x[] = {14, 19, 24};
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31};
	int drunk_x[] = {1, 42, 83, 124, 165, 206, 247, 1};
	int difference;

	copyBackground(123, candles_y[frame_candles], 142, 14, 39, 13, drawSurface3, screenSurface);
	copyBackground(cirio_x[frame_candles], 146, 311, 80, 4, 8, drawSurface3, screenSurface);

	if (blinking == 5)
		copyBackground(1, 149, 127, 52, 9, 5, drawSurface3, screenSurface);
	if (hare_x > 101 && hare_x < 155)
		copyBackground(31, 138, 178, 51, 18, 16, drawSurface3, screenSurface);
	if (flags[11] == 0)
		copyBackground(pianista_x[frame_piano], 157, 245, 130, 29, 42, drawSurface3, screenSurface);
	else if (flags[5] == 0)
		copyBackground(145, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	else
		copyBackground(165, 140, 229, 117, 43, 59, drawSurface3, screenSurface);

	if (flags[12] == 1)
		copyBackground(drunk_x[frame_drunk], 82, 170, 50, 40, 53, drawSurface3, screenSurface);
	difference = getTime() - conta_blind_vez;
	if (difference > 6) {
		if (flags[12] == 1) {
			frame_drunk++;
			if (frame_drunk == 8) {
				frame_drunk = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		frame_candles++;
		if (frame_candles == 3)
			frame_candles = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		blinking = _rnd->getRandomNumber(10);
		conta_blind_vez = getTime();
	}
}

void DrasculaEngine::update_15() {
	copyRect(1, 154, 83, 122, 131, 44, drawSurface3, screenSurface);
}

void DrasculaEngine::update_16_pre() {
	if (flags[17] == 0)
		copyBackground(1, 103, 24, 72, 33, 95, drawSurface3, screenSurface);
	if (flags[19] == 1)
		copyBackground(37, 151, 224, 115, 56, 47, drawSurface3, screenSurface);
}

void DrasculaEngine::update_17_pre() {
	if (flags[15] == 1)
		copyBackground(1, 135, 108, 65, 44, 63, drawSurface3, screenSurface);
}

void DrasculaEngine::update_17() {
	copyRect(48, 135, 78, 139, 80, 30, drawSurface3, screenSurface);
}

void DrasculaEngine::update_18_pre() {
	int difference;
	int snore_x[] = {95, 136, 95, 136, 95, 95, 95, 95, 136, 95, 95, 95, 95, 95, 95, 95};
	int snore_y[] = {18, 18, 56, 56, 94, 94, 94, 94, 94, 18, 18, 18, 18, 18, 18, 18};

	if (flags[21] == 0) {
		copyBackground(1, 69, 120, 58, 56, 61, drawSurface3, screenSurface);
		copyBackground(snore_x[frame_snore], snore_y[frame_snore], 124, 59, 40, 37, drawSurface3, screenSurface);
	} else
		pon_vb();

	difference = getTime() - conta_blind_vez;
	if (difference > 9) {
		frame_snore++;
		if (frame_snore == 16)
			frame_snore = 0;
		conta_blind_vez = getTime();
	}
}

void DrasculaEngine::update_18() {
	if (flags[24] == 1)
		copyRect(177, 1, 69, 29, 142, 130, drawSurface3, screenSurface);
	copyRect(105, 132, 109, 108, 196, 65, drawSurface3, screenSurface);
}

void DrasculaEngine::update_20() {
	copyRect(1, 137, 106, 121, 213, 61, drawSurface3, screenSurface);
}

void DrasculaEngine::update_21_pre() {
	if (flags[0] == 1)
		copyBackground(2, 171, 84, 126, 17, 26, drawSurface3, screenSurface);

	if (flags[10] == 1)
		copyBackground(20, 163, 257, 149, 14, 34, drawSurface3, screenSurface);
}

void DrasculaEngine::update_22_pre() {
	if (flags[24] == 1)
		copyBackground(2, 187, 107, 106, 62, 12, drawSurface3, screenSurface);

	if (flags[27] == 0)
		copyBackground(32, 181, 203, 88, 13, 5, drawSurface3, screenSurface);

	if (flags[26] == 0)
		copyBackground(2, 133, 137, 83, 29, 53, drawSurface3, screenSurface);
	else
		copyBackground(65, 174, 109, 145, 55, 25, drawSurface3, screenSurface);
}

void DrasculaEngine::update_23_pre() {
	if (flags[11] == 1 && flags[0] == 0)
		copyBackground(87, 171, 237, 110, 20, 28, drawSurface3, screenSurface);

	if (flags[0] == 1)
		copyBackground(29, 126, 239, 94, 57, 73, drawSurface3, screenSurface);

	if (flags[1] == 1)
		copyRect(1, 135, 7, 94, 27, 64, drawSurface3, screenSurface);
}

void DrasculaEngine::update_24_pre() {
	if (flags[1] == 1)
		copyBackground(1, 163, 225, 124, 12, 36, drawSurface3, screenSurface);

	if (flags[2] == 1)
		copyBackground(14, 153, 30, 107, 23, 46, drawSurface3, screenSurface);
}

void DrasculaEngine::update_26_pre() {
	int difference;

	if (flags[2] == 1)
		copyBackground(1, 130, 87, 44, 50, 69, drawSurface3, screenSurface);

	if (flags[12] == 1)
		copyBackground(52, 177, 272, 103, 27, 22, drawSurface3, screenSurface);

	if (flags[18] == 0)
		copyBackground(80, 133, 199, 95, 50, 66, drawSurface3, screenSurface);

	if (blinking == 5 && flags[18] == 0)
		copyBackground(52, 172, 226, 106, 3, 4, drawSurface3, screenSurface);

	difference = getTime() - conta_blind_vez;
	if (difference >= 10) {
		blinking = _rnd->getRandomNumber(10);
		conta_blind_vez = getTime();
	}

	if (flags[20] == 1)
		copyBackground(182, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
}

void DrasculaEngine::update_26() {
	if (flags[29] == 1)
		copyRect(93, 1, hare_x, hare_y, 45, 78, backSurface, screenSurface);

	copyRect(233, 107, 17, 102, 66, 92, drawSurface3, screenSurface);
}

void DrasculaEngine::update_27_pre() {
	if (flags[5] == 1)
		copyRect(1, 175, 59, 109, 17, 24, drawSurface3, screenSurface);

	if (flags[6] == 1)
		copyRect(19, 177, 161, 103, 18, 22, drawSurface3, screenSurface);
}

void DrasculaEngine::update_27() {
	copyRect(38, 177, 103, 171, 21, 22, drawSurface3, screenSurface);
	copyRect(60, 162, 228, 156, 18, 37, drawSurface3, screenSurface);
}

void DrasculaEngine::update_29_pre() {
	if (flags[4] == 1)
		copyBackground(12, 113, 247, 49, 41, 84, drawSurface3, screenSurface);
}

void DrasculaEngine::update_29() {
	copyRect(1, 180, 150, 126, 10, 17, drawSurface3, screenSurface);
}

void DrasculaEngine::update_30_pre() {
	if (flags[4] == 1)
		copyBackground(1, 148, 148, 66, 35, 51, drawSurface3, screenSurface);

	if (flags[16] == 1)
		copyBackground(37, 173, 109, 84, 20, 26, drawSurface3, screenSurface);
}

void DrasculaEngine::update_31_pre() {
	if (flags[13] == 1)
		copyBackground(1, 163, 116, 41, 61, 36, drawSurface3, screenSurface);

	if (flags[5] == 1)
		copyBackground(1, 78, 245, 63, 30, 84, drawSurface3, screenSurface);
}

void DrasculaEngine::update_31() {
	copyRect(63, 190, 223, 157, 17, 9, drawSurface3, screenSurface);
}

void DrasculaEngine::update_34_pre() {
	if (flags[7] == 1)
		copyBackground(99, 127, 73, 41, 79, 72, drawSurface3, screenSurface);

	if (flags[8] == 1)
		copyBackground(36, 129, 153, 41, 62, 65, drawSurface3, screenSurface);
}

void DrasculaEngine::update_34() {
	copyRect(5, 171, 234, 126, 29, 23, drawSurface3, screenSurface);
}

void DrasculaEngine::update_35_pre() {
	if (flags[14] == 1)
		copyBackground(1, 86, 246, 65, 68, 87, drawSurface3, screenSurface);

	if (flags[17] == 0 && flags[15] == 1)
		copyBackground(111, 150, 118, 52, 40, 23, drawSurface3, screenSurface);

	if (flags[17] == 1)
		copyBackground(70, 150, 118, 52, 40, 23, drawSurface3, screenSurface);
}

void DrasculaEngine::update_35() {
	copyRect(1, 174, 54, 152, 195, 25, drawSurface3, screenSurface);
}


void DrasculaEngine::update_49_pre() {
	if (flags[6] == 0)
		copyBackground(2, 136, 176, 81, 49, 62, drawSurface3, screenSurface);
}

void DrasculaEngine::update_50() {
	copyRect(4, 153, 118, 95, 67, 44, drawSurface3, screenSurface);
}

void DrasculaEngine::update_53_pre() {
	if (flags[1] == 0)
		copyRect(2, 113, 205, 50, 38, 86, drawSurface3, screenSurface);
	if (flags[2] == 0)
		copyBackground(41, 159, 27, 117, 25, 40, drawSurface3, screenSurface);
	if (flags[9] == 1)
		copyBackground(67, 184, 56, 93, 32, 15, drawSurface3, screenSurface);
}

void DrasculaEngine::update_54_pre() {
	if (flags[5] == 1)
		copyBackground(168, 156, 187, 111, 7, 11, drawSurface3, screenSurface);
	if (flags[12] == 1)
		copyBackground(16, 156, 190, 64, 18, 24, drawSurface3, screenSurface);
}

void DrasculaEngine::update_56_pre() {
	if (flags[10] == 0)
		copyBackground(2, 126, 42, 67, 57, 67, drawSurface3, screenSurface);
	if (flags[11] == 1)
		copyBackground(60, 160, 128, 97, 103, 38, drawSurface3, screenSurface);
}

void DrasculaEngine::update_57() {
	copyRect(7, 113, 166, 61, 62, 82, drawSurface3, screenSurface);
}

void DrasculaEngine::update_58_pre() {
	if (flags[0] == 0)
		copyBackground(1, 156, 143, 120, 120, 43, drawSurface3, screenSurface);
	if (flags[1] == 2)
		copyRect(252, 171, 173, 116, 25, 28, drawSurface3, screenSurface);
	if (flags[1] == 0 && flags[0] == 0)
		copyRect(278, 171, 173, 116, 25, 28, drawSurface3, screenSurface);
	if (flags[2] == 0) {
		placeIgor();
		placeDrascula();
	}
	if (flags[3] == 1)
		copyRect(1, 29, 204, 0, 18, 125, drawSurface3, screenSurface);
	if (flags[8] == 1)
		copyBackground(20, 60, 30, 64, 46, 95, drawSurface3, screenSurface);
}

void DrasculaEngine::update_58() {
	if (hare_se_ve == 1)
		copyRect(67, 139, 140, 147, 12, 16, drawSurface3, screenSurface);
}

void DrasculaEngine::update_59_pre() {
	if (flags[4] == 0)
		copyRect(1, 146, 65, 106, 83, 40, drawSurface3, screenSurface);
	if (flags[9] == 1) {
		copyBackground(65, 103, 65, 103, 49, 38, drawSurface1, screenSurface);
		copyRect(1, 105, 65, 103, 49, 38, drawSurface3, screenSurface);
	}
}

void DrasculaEngine::update_60_pre() {
	int candles_y[] = {158, 172, 186};
	int difference;

	if (flags[5] == 0)
		placeDrascula();

	copyBackground(123, candles_y[frame_candles], 142, 14, 39, 13, drawSurface3, screenSurface);

	if (flag_tv == 1)
		copyBackground(114, 158, 8, 30, 8, 23, drawSurface3, screenSurface);

	difference = getTime() - conta_blind_vez;
	blinking = _rnd->getRandomNumber(7);
	if (blinking == 5 && flag_tv == 0)
		flag_tv = 1;
	else if (blinking == 5 && flag_tv == 1)
		flag_tv = 0;
	if (difference > 6) {
		frame_candles++;
		if (frame_candles == 3)
			frame_candles = 0;
		conta_blind_vez = getTime();
	}
}

void DrasculaEngine::update_60() {
	if (hare_y - 10 < y_dr && flags[5] == 0)
		placeDrascula();
}

void DrasculaEngine::update_61() {
	copyRect(1, 154, 83, 122, 131, 44, drawSurface3, screenSurface);
}

void DrasculaEngine::update_62_pre() {
	int candles_y[] = { 158, 172, 186 };
	int cirio_x[] = { 14, 19, 24 };
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31 };
	int drunk_x[] = {1, 42, 83, 124, 165, 206, 247, 1 };
	int difference;

	copyBackground(123, candles_y[frame_candles], 142, 14, 39, 13, drawSurface3, screenSurface);
	copyBackground(cirio_x[frame_candles], 146, 311, 80, 4, 8, drawSurface3, screenSurface);

	if (blinking == 5)
		copyBackground(1, 149, 127, 52, 9, 5, drawSurface3, screenSurface);

	if (hare_x > 101 && hare_x < 155)
		copyBackground(31, 138, 178, 51, 18, 16, drawSurface3, screenSurface);

	if (flags[11] == 0)
		copyBackground(pianista_x[frame_piano], 157, 245, 130, 29, 42, drawSurface3, screenSurface);
	else if (flags[5] == 0)
		copyBackground(145, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	else
		copyBackground(165, 140, 229, 117, 43, 59, drawSurface3, screenSurface);

	if (flags[12] == 1)
		copyBackground(drunk_x[frame_drunk], 82, 170, 50, 40, 53, drawSurface3, screenSurface);

	difference = getTime() - conta_blind_vez;
	if (difference > 6) {
		if (flags[12] == 1) {
			frame_drunk++;
			if (frame_drunk == 8) {
				frame_drunk = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		frame_candles++;
		if (frame_candles == 3)
			frame_candles = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		blinking = _rnd->getRandomNumber(10);
		conta_blind_vez = getTime();
	}
}

void DrasculaEngine::update_62() {
	int drunk_x[] = { 1, 42, 83, 124, 165, 206, 247, 1 };

	copyRect(1, 1, 0, 0, 62, 142, drawSurface2, screenSurface);

	if (hare_y + alto_hare < 89) {
		copyRect(205, 1, 180, 9, 82, 80, drawSurface3, screenSurface);
		copyBackground(drunk_x[frame_drunk], 82, 170, 50, 40, 53, drawSurface3, screenSurface);
	}
}

void DrasculaEngine::update_63() {
	copyRect(1, 154, 83, 122, 131, 44, drawSurface3, screenSurface);
}

void DrasculaEngine::update_pendulum() {
	int pendulum_x[] = {40, 96, 152, 208, 264, 40, 96, 152, 208, 208, 152, 264, 40, 96, 152, 208, 264};
	int difference;

	if (frame_pen <= 4)
		pendulumSurface = drawSurface3;
	else if (frame_pen <= 11)
		pendulumSurface = extraSurface;
	else
		pendulumSurface = frontSurface;

	copyBackground(pendulum_x[frame_pen], 19, 152, 0, 55, 125, pendulumSurface, screenSurface);

	if (flags[1] == 2)
		copyRect(18, 145, 145, 105, 25, 29, drawSurface3, screenSurface);

	if (flags[1] == 0)
		copyRect(44, 145, 145, 105, 25, 29, drawSurface3, screenSurface);

	difference = getTime() - conta_blind_vez;
	if (difference > 8) {
		frame_pen++;
		if (frame_pen == 17)
			frame_pen = 0;
		conta_blind_vez = getTime();
	}
}

bool DrasculaEngine::checkFlag(int fl) {
	characterMoved = 0;
	updateRoom();
	updateScreen();

	hasAnswer = 1;

	if (menuScreen == 1) {
		if (currentChapter == 1) {
			if (pickedObject == kVerbLook && fl == 28)
				talk(328);
		} else if (currentChapter == 2) {
			if ((pickedObject == kVerbLook && fl == 22 && flags[23] == 0)
					|| (pickedObject == kVerbOpen && fl == 22 && flags[23] == 0)) {
				talk(164);
				flags[23] = 1;
				withoutVerb();
				addObject(7);
				addObject(18);
			} else if (pickedObject == kVerbLook && fl == 22 && flags[23] == 1)
				talk(307);
			else if (pickedObject == kVerbLook && fl == 28)
				talk(328);
			else if (pickedObject == kVerbLook && fl == 7)
				talk(143);
			else if (pickedObject == kVerbTalk && fl == 7)
				talk(144);
			else if (pickedObject == kVerbLook && fl == 8)
				talk(145);
			else if (pickedObject == kVerbTalk && fl == 8)
				talk(146);
			else if (pickedObject == kVerbLook && fl == 9)
				talk(147);
			else if (pickedObject == kVerbTalk && fl == 9)
				talk(148);
			else if (pickedObject == kVerbLook && fl == 10)
				talk(151);
			else if (pickedObject == kVerbLook && fl == 11)
				talk(152);
			else if (pickedObject == kVerbTalk && fl == 11)
				talk(153);
			else if (pickedObject == kVerbLook && fl == 12)
				talk(154);
			else if (pickedObject == kVerbLook && fl == 13)
				talk(155);
			else if (pickedObject == kVerbLook && fl == 14)
				talk(157);
			else if (pickedObject == kVerbLook && fl == 15)
				talk(58);
			else if (pickedObject == kVerbLook && fl == 16)
				talk(158);
			else if (pickedObject == kVerbLook && fl == 17)
				talk(159);
			else if (pickedObject == kVerbLook && fl == 18)
				talk(160);
			else if (pickedObject == kVerbLook && fl == 19)
				talk(161);
			else if (pickedObject == kVerbLook && fl == 20)
				talk(162);
			else if (pickedObject == kVerbLook && fl == 23)
				talk(152);
			else
				hasAnswer = 0;
		} else if (currentChapter == 3) {
			if (pickedObject == kVerbLook && fl == 22)
				talk(307);
			else if (pickedObject == kVerbLook && fl == 28)
				talk(328);
			else if (pickedObject == kVerbLook && fl == 7)
				talk(143);
			else if (pickedObject == kVerbTalk && fl == 7)
				talk(144);
			else if (pickedObject == kVerbLook && fl == 8)
				talk(145);
			else if (pickedObject == kVerbTalk && fl == 8)
				talk(146);
			else if (pickedObject == kVerbLook && fl == 9)
				talk(147);
			else if (pickedObject == kVerbTalk && fl == 9)
				talk(148);
			else if (pickedObject == kVerbLook && fl == 10)
				talk(151);
			else if (pickedObject == kVerbLook && fl == 11)
				talk(152);
			else if (pickedObject == kVerbTalk && fl == 11)
				talk(153);
			else if (pickedObject == kVerbLook && fl == 12)
				talk(154);
			else if (pickedObject == kVerbLook && fl == 13)
				talk(155);
			else if (pickedObject == kVerbLook && fl == 14)
				talk(157);
			else if (pickedObject == kVerbLook && fl == 15)
				talk(58);
			else if (pickedObject == kVerbLook && fl == 16)
				talk(158);
			else if (pickedObject == kVerbLook && fl == 17)
				talk(159);
			else if (pickedObject == kVerbLook && fl == 18)
				talk(160);
			else if (pickedObject == kVerbLook && fl == 19)
				talk(161);
			else if (pickedObject == kVerbLook && fl == 20)
				talk(162);
			else if (pickedObject == kVerbLook && fl == 23)
				talk(152);
			else
				hasAnswer = 0;
		} else if (currentChapter == 4) {
			if ((pickedObject == 18 && fl == 19) || (pickedObject == 19 && fl == 18)) {
				withoutVerb();
				chooseObject(21);
				removeObject(18);
				removeObject(19);
			} else if ((pickedObject == 14 && fl == 19) || (pickedObject == 19 && fl == 14))
				talk(484);
			else if (pickedObject == kVerbLook && fl == 28)
				talk(328);
			else if (pickedObject == kVerbLook && fl == 7)
				talk(478);
			else if (pickedObject == kVerbLook && fl == 8)
				talk(480);
			else if (pickedObject == kVerbLook && fl == 9) {
				talk(482);
				talk(483);
			} else if (pickedObject == kVerbLook && fl == 10)
				talk(485);
			else if (pickedObject == kVerbLook && fl == 11)
				talk(488);
			else if (pickedObject == kVerbLook && fl == 12)
				talk(486);
			else if (pickedObject == kVerbLook && fl == 13)
				talk(490);
			else if (pickedObject == kVerbLook && fl == 14)
				talk(122);
			else if (pickedObject == kVerbLook && fl == 15)
				talk(117);
			else if (pickedObject == kVerbTalk && fl == 15)
				talk(118);
			else if (pickedObject == kVerbOpen && fl == 15)
				talk(119);
			else if (pickedObject == kVerbLook && fl == 16)
				talk(491);
			else if (pickedObject == kVerbLook && fl == 17)
				talk(478);
			else if (pickedObject == kVerbLook && fl == 18)
				talk(493);
			else if (pickedObject == kVerbLook && fl == 19) {
				talk(494);
				talk(495);
			} else if (pickedObject == kVerbLook && fl == 20)
				talk(162);
			else if (pickedObject == kVerbLook && fl == 21)
				talk(496);
			else if (pickedObject == kVerbLook && fl == 22)
				talk(161);
			else
				hasAnswer = 0;
		} else if (currentChapter == 5) {
			if (pickedObject == kVerbLook && fl == 28)
				talk(328);
			else if (pickedObject == kVerbLook && fl == 7)
				talk(478);
			else if (pickedObject == kVerbLook && fl == 8)
				talk(120);
			else if (pickedObject == kVerbLook && fl == 9) {
				talk(482);
				talk(483);
			} else if (pickedObject == kVerbLook && fl == 11)
				talk(488);
			else if (pickedObject == kVerbLook && fl == 13)
				talk(490);
			else if (pickedObject == kVerbLook && fl == 14)
				talk(121);
			else if (pickedObject == kVerbLook && fl == 15)
				talk(117);
			else if (pickedObject == kVerbTalk && fl == 15)
				talk(118);
			else if (pickedObject == kVerbOpen && fl == 15)
				talk(119);
			else if (pickedObject == kVerbLook && fl == 17)
				talk(478);
			else if (pickedObject == kVerbLook && fl == 20)
				talk(162);
			else
				hasAnswer = 0;
		} else if (currentChapter == 6) {
			if (pickedObject == kVerbLook && fl == 28)
				talk(328);
			else if (pickedObject == kVerbLook && fl == 9) {
				talk(482);
				talk(483);
			} else if (pickedObject == kVerbLook && fl == 20)
				talk(123);
			else if (pickedObject == kVerbLook && fl == 21)
				talk(441);
			else
				hasAnswer = 0;
		}
	} else {
		if (currentChapter == 1) {
			if (pickedObject == kVerbLook && fl == 50)
				talk(308);
			else if (pickedObject == kVerbOpen && fl == 50)
				talk(310);
			else if (pickedObject == kVerbClose && fl == 50)
				talk(311);
			else if (pickedObject == kVerbMove && fl == 50)
				talk(312);
			else if (pickedObject == kVerbPick && fl == 50)
				talk(313);
			else if (pickedObject == kVerbTalk && fl == 50)
				talk(314);
			else if (roomNumber == 62)
				room_62(fl);
			else if (roomNumber == 63)
				room_63(fl);
			else
				hasAnswer = 0;
		} else if (currentChapter == 2) {
			if (pickedObject == kVerbLook && fl == 50)
				talk(308);
			else if (pickedObject == kVerbOpen && fl == 50)
				talk(310);
			else if (pickedObject == kVerbClose && fl == 50)
				talk(311);
			else if (pickedObject == kVerbMove && fl == 50)
				talk(312);
			else if (pickedObject == kVerbPick && fl == 50)
				talk(313);
			else if (pickedObject == kVerbTalk && fl == 50)
				talk(314);
			// Note: the original check was strcmp(num_room, "18.alg")
			else if (pickedObject == 11 && fl == 50 && flags[22] == 0 && roomNumber != 18)
				talk(315);
			else if (pickedObject == 13 && fl == 50)
				talk(156);
			else if (pickedObject == 20 && fl == 50)
				talk(163);
			else if (roomNumber == 1)
				room_1(fl);
			else if (roomNumber == 3)
				room_3(fl);
			else if (roomNumber == 4)
				room_4(fl);
			else if (roomNumber == 5)
				room_5(fl);
			else if (roomNumber == 6)
				room_6(fl);
			else if (roomNumber == 7)
				room_7(fl);
			else if (roomNumber == 8)
				room_8(fl);
			else if (roomNumber == 9)
				room_9(fl);
			else if (roomNumber == 12)
				room_12(fl);
			else if (roomNumber == 14)
				room_14(fl);
			else if (roomNumber == 15)
				room_15(fl);
			else if (roomNumber == 16)
				room_16(fl);
			else if (roomNumber == 17)
				room_17(fl);
			else if (roomNumber == 18)
				room_18(fl);
			else if (roomNumber == 19)
				room_19(fl);
			else
				hasAnswer = 0;
		} else if (currentChapter == 3) {
			if (pickedObject == kVerbLook && fl == 50)
				talk(309);
			else if (pickedObject == kVerbOpen && fl == 50)
				talk(310);
			else if (pickedObject == kVerbClose && fl == 50)
				talk(311);
			else if (pickedObject == kVerbMove && fl == 50)
				talk(312);
			else if (pickedObject == kVerbPick && fl == 50)
				talk(313);
			else if (pickedObject == kVerbTalk && fl == 50)
				talk(314);
			else if (roomNumber == 13) {
				if (room_13(fl))
					return true;
			} else
				hasAnswer = 0;
		} else if (currentChapter == 4) {
			if (roomNumber == 28)
				talk(178);
			else if (pickedObject == kVerbLook && fl == 50)
				talk(309);
			else if (pickedObject == kVerbOpen && fl == 50)
				talk(310);
			else if (pickedObject == kVerbClose && fl == 50)
				talk(311);
			else if (pickedObject == kVerbMove && fl == 50)
				talk(312);
			else if (pickedObject == kVerbPick && fl == 50)
				talk(313);
			else if (pickedObject == kVerbTalk && fl == 50)
				talk(314);
			else if (pickedObject == 8 && fl == 50 && flags[18] == 0)
				talk(481);
			else if (pickedObject == 9 && fl == 50)
				talk(484);
			else if (pickedObject == 12 && fl == 50 && flags[18] == 0)
				talk(487);
			else if (pickedObject == 20 && fl == 50)
				talk(487);
			else if (roomNumber == 21) {
				if (room_21(fl))
					return true;
			} else if (roomNumber == 22)
				room_22(fl);
			else if (roomNumber == 23)
				room_23(fl);
			else if (roomNumber == 24)
				room_24(fl);
			else if (roomNumber == 26)
				room_26(fl);
			else if (roomNumber == 27)
				room_27(fl);
			else if (roomNumber == 29)
				room_29(fl);
			else if (roomNumber == 30)
				room_30(fl);
			else if (roomNumber == 31)
				room_31(fl);
			else if (roomNumber == 34)
				room_34(fl);
			else if (roomNumber == 35)
				room_35(fl);
			else if (roomNumber == 44)
				room_44(fl);
			else
				hasAnswer = 0;
		} else if (currentChapter == 5) {
			if (pickedObject == kVerbLook && fl == 50)
				talk("Cuanto mas me miro, mas me gusto", "54.als");
			else if (pickedObject == kVerbOpen && fl == 50)
				talk("y luego como me cierro", "19.als");
			else if (pickedObject == kVerbClose && fl == 50)
				talk("Tendre que abrirme primero no", "19.als");
			else if (pickedObject == kVerbMove && fl == 50)
				talk("Estoy bien donde estoy", "19.als");
			else if (pickedObject == kVerbPick && fl == 50)
				talk("Ya me tengo", "11.als");
			else if (pickedObject == kVerbTalk && fl == 50)
				talk("hola yo", "16.als");
			else if (pickedObject == 20 && fl == 50)
				talk(487);
			else if (roomNumber == 49)
				room_49(fl);
			else if (roomNumber == 53)
				room_53(fl);
			else if (roomNumber == 54)
				room_54(fl);
			else if (roomNumber == 55)
				room_55(fl);
			else if (roomNumber == 56) {
				if (room_56(fl))
					return true;
			} else
				hasAnswer = 0;
		} else if (currentChapter == 6) {
			if (pickedObject == kVerbLook && fl == 50 && flags[0] == 1)
				talk(308);
			else if (pickedObject == kVerbLook && fl == 50 && flags[0] == 0)
				talk(310 );
			else if (pickedObject == kVerbOpen && fl == 50)
				talk(310 );
			else if (pickedObject == kVerbClose && fl == 50)
				talk(311 );
			else if (pickedObject == kVerbMove && fl == 50)
				talk(312 );
			else if (pickedObject == kVerbPick && fl == 50)
				talk(313 );
			else if (pickedObject == kVerbTalk && fl == 50)
				talk(314 );
			else if (roomNumber == 102)
				room_pendulum(fl);
			else if (roomNumber == 58)
				room_58(fl);
			else if (roomNumber == 59)
				room_59(fl);
			else if (roomNumber == 60) {
				if (room_60(fl))
					return true;
			} else if (roomNumber == 61)
				room_61(fl);
			else
				hasAnswer = 0;
		}
	}
	if (hasAnswer == 0 && hasName == 1)
		room_0();
	else if (hasAnswer == 0 && menuScreen == 1)
		room_0();

	return false;
}

} // End of namespace Drascula
