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

namespace Drascula {

void DrasculaEngine::cleanupString(char *string) {
	uint len = strlen(string);
	for (uint h = 0; h < len; h++)
		if (string[h] == (char)0xa7)
			string[h] = ' ';
}

void DrasculaEngine::converse(int index) {
	char fileName[20];
	sprintf(fileName, "op_%d.cal", index);
	_arj.open(fileName);
	if (!_arj.isOpen())
		error("missing data file %s", fileName);

	int size = _arj.size();
	int game1 = kDialogOptionUnselected, 
		game2 = kDialogOptionUnselected,
		game3 = kDialogOptionUnselected;
	char phrase1[78], phrase2[78], phrase3[78], phrase4[78];
	char sound1[13], sound2[13], sound3[13], sound4[13];
	int answer1, answer2, answer3;
	char buffer[256];

	breakOut = 0;

	withoutVerb();

	getStringFromLine(buffer, size, phrase1);
	getStringFromLine(buffer, size, phrase2);
	getStringFromLine(buffer, size, phrase3);
	getStringFromLine(buffer, size, phrase4);
	getStringFromLine(buffer, size, sound1);
	getStringFromLine(buffer, size, sound2);
	getStringFromLine(buffer, size, sound3);
	getStringFromLine(buffer, size, sound4);
	getIntFromLine(buffer, size, &answer1);
	getIntFromLine(buffer, size, &answer2);
	getIntFromLine(buffer, size, &answer3);

	_arj.close();

	if (currentChapter == 2 && !strcmp(fileName, "op_5.cal") && flags[38] == 1 && flags[33] == 1) {
		strcpy(phrase3, _text[_lang][405]);
		strcpy(sound3, "405.als");
		answer3 = 31;
	}

	if (currentChapter == 6 && !strcmp(fileName, "op_12.cal") && flags[7] == 1) {
		strcpy(phrase3, _text[_lang][273]);
		strcpy(sound3, "273.als");
		answer3 = 14;
	}

	if (currentChapter == 6 && !strcmp(fileName, "op_12.cal") && flags[10] == 1) {
		strcpy(phrase3, _text[_lang][274]);
		strcpy(sound3, "274.als");
		answer3 = 15;
	}

	cleanupString(phrase1);
	cleanupString(phrase2);
	cleanupString(phrase3);
	cleanupString(phrase4);

	loadPic("car.alg", backSurface);
	// TODO code here should limit y position for mouse in dialog menu,
	// but we can't implement this as there is lack in backend functionality
	// from 1(top) to 31
	color_abc(kColorLightGreen);

	while (breakOut == 0) {
		updateRoom();

		if (musicStatus() == 0 && roomMusic != 0) {
			if (currentChapter == 3 || currentChapter == 5) {
				playMusic(roomMusic);
			} else {	// chapters 1, 2, 4, 6
				if (flags[11] == 0)
					playMusic(roomMusic);
			}
		}

		updateEvents();

		print_abc_opc(phrase1, 2, game1);
		print_abc_opc(phrase2, 10, game2);
		print_abc_opc(phrase3, 18, game3);
		print_abc_opc(phrase4, 26, kDialogOptionUnselected);

		if (mouseY > 0 && mouseY < 9) {
			if (game1 == kDialogOptionClicked && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (game1 != kDialogOptionClicked && _color != kColorLightGreen)
				color_abc(kColorLightGreen);

			print_abc_opc(phrase1, 2, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				game1 = kDialogOptionClicked;
				talk(phrase1, sound1);
				response(answer1);
			}
		} else if (mouseY > 8 && mouseY < 17) {
			if (game2 == kDialogOptionClicked && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (game2 != kDialogOptionClicked && _color != kColorLightGreen)
				color_abc(kColorLightGreen);

			print_abc_opc(phrase2, 10, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				game2 = kDialogOptionClicked;
				talk(phrase2, sound2);
				response(answer2);
			}
		} else if (mouseY > 16 && mouseY < 25) {
			if (game3 == kDialogOptionClicked && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (game3 != kDialogOptionClicked && _color != kColorLightGreen)
				color_abc(kColorLightGreen);

			print_abc_opc(phrase3, 18, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				game3 = kDialogOptionClicked;
				talk(phrase3, sound3);
				response(answer3);
			}
		} else if (mouseY > 24 && mouseY < 33) {
			print_abc_opc(phrase4, 26, kDialogOptionSelected);

			if (leftMouseButton == 1) {
				delay(100);
				talk(phrase4, sound4);
				breakOut = 1;
			}
		} else if (_color != kColorLightGreen)
			color_abc(kColorLightGreen);

		updateScreen();
	} // while (breakOut == 0)

	if (currentChapter == 2)
		loadPic(menuBackground, backSurface);
	else
		loadPic(99, backSurface);
}

void DrasculaEngine::response(int function) {
	if (currentChapter == 1) {
		if (function >= 10 && function <= 12)
			talk_drunk(function - 9);
	} else if (currentChapter == 2) {
		if (function == 8)
			animation_8_2();
		else if (function == 9)
			animation_9_2();
		else if (function == 10)
			animation_10_2();
		else if (function == 15)
			animation_15_2();
		else if (function == 16)
			animation_16_2();
		else if (function == 17)
			animation_17_2();
		else if (function == 19)
			talk_vonBraun(5, kVonBraunDoor);
		else if (function == 20)
			animation_20_2();
		else if (function == 21)
			talk_vonBraun(6, kVonBraunDoor);
		else if (function == 23)
			animation_23_2();
		else if (function == 28)
			animation_28_2();
		else if (function == 29)
			animation_29_2();
		else if (function == 30)
			animation_30_2();
		else if (function == 31)
			animation_31_2();
	} else if (currentChapter == 3) {
		grr();
	} else if (currentChapter == 4) {
		if (function == 2)
			animation_2_4();
		else if (function == 3)
			animation_3_4();
		else if (function == 4)
			animation_4_4();
	} else if (currentChapter == 5) {
		if (function == 2)
			talk_bj(22);
		else if (function == 3)
			animation_3_5();
		else if (function == 6)
			animation_6_5();
		else if (function == 7)
			animation_7_5();
		else if (function == 8)
			animation_8_5();
		else if (function == 15)
			animation_15_5();
		else if (function == 16)
			talk_mus(8);
		else if (function == 17)
			talk_mus(9);
	} else if (currentChapter == 6) {
		if (function == 2)
			talk_drascula(24, 1);
		else if (function == 3)
			talk_drascula(24, 1);
		else if (function == 4)
			talk_drascula(25, 1);
		else if (function == 11)
			animation_11_6();
		else if (function == 12)
			animation_12_6();
		else if (function == 13)
			talk_bartender(15, 1);
		else if (function == 14)
			animation_14_6();
		else if (function == 15)
			talk_bartender(16, 1);
	}
}

} // End of namespace Drascula
