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

void DrasculaEngine::converse(int index) {
	char fileName[20];
	sprintf(fileName, "op_%d.cal", index);
	uint h;
	int game1 = 1, game2 = 1, game3 = 1, game4 = 1;
	char phrase1[78];
	char phrase2[78];
	char phrase3[87];
	char phrase4[78];
	char sound1[13];
	char sound2[13];
	char sound3[13];
	char sound4[13];
	int answer1;
	int answer2;
	int answer3;
	int used1 = 0;
	int used2 = 0;
	int used3 = 0;
	char buffer[256];
	uint len;

	breakOut = 0;

	if (currentChapter == 5)
		withoutVerb();

	_arj.open(fileName);
	if (!_arj.isOpen()) {
		error("missing data file %s", fileName);
	}
	int size = _arj.size();

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

	len = strlen(phrase1);
	for (h = 0; h < len; h++)
		if (phrase1[h] == (char)0xa7)
			phrase1[h] = ' ';

	len = strlen(phrase2);
	for (h = 0; h < len; h++)
		if (phrase2[h] == (char)0xa7)
			phrase2[h] = ' ';

	len = strlen(phrase3);
	for (h = 0; h < len; h++)
		if (phrase3[h] == (char)0xa7)
			phrase3[h] = ' ';

	len = strlen(phrase4);
	for (h = 0; h < len; h++)
		if (phrase4[h] == (char)0xa7)
			phrase4[h] = ' ';

	loadPic("car.alg", backSurface);
	// TODO code here should limit y position for mouse in dialog menu,
	// but we can't implement this due lack backend functionality
	// from 1(top) to 31
	color_abc(kColorLightGreen);

	while (breakOut == 0) {
		updateRoom();

		if (currentChapter == 1 || currentChapter == 4 || currentChapter == 6) {
			if (musicStatus() == 0 && flags[11] == 0)
				playMusic(roomMusic);
		} else if (currentChapter == 2) {
			if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
				playMusic(roomMusic);
		} else if (currentChapter == 3 || currentChapter == 5) {
			if (musicStatus() == 0)
				playMusic(roomMusic);
		}

		updateEvents();

		if (mouseY > 0 && mouseY < 9) {
			if (used1 == 1 && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (used1 == 0 && _color != kColorLightGreen)
				color_abc(kColorLightGreen);
		} else if (mouseY > 8 && mouseY < 17) {
			if (used2 == 1 && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (used2 == 0 && _color != kColorLightGreen)
				color_abc(kColorLightGreen);
		} else if (mouseY > 16 && mouseY < 25) {
			if (used3 == 1 && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (used3 == 0 && _color != kColorLightGreen)
				color_abc(kColorLightGreen);
		} else if (_color != kColorLightGreen)
			color_abc(kColorLightGreen);

		if (mouseY > 0 && mouseY < 9)
			game1 = 2;
		else if (mouseY > 8 && mouseY < 17)
			game2 = 2;
		else if (mouseY > 16 && mouseY < 25)
			game3 = 2;
		else if (mouseY > 24 && mouseY < 33)
			game4 = 2;

		print_abc_opc(phrase1, 1, 2, game1);
		print_abc_opc(phrase2, 1, 10, game2);
		print_abc_opc(phrase3, 1, 18, game3);
		print_abc_opc(phrase4, 1, 26, game4);

		updateScreen();

		if ((leftMouseButton == 1) && (game1 == 2)) {
			delay(100);
			used1 = 1;
			talk(phrase1, sound1);
			if (currentChapter == 3)
				grr();
			else
				response(answer1);
		} else if ((leftMouseButton == 1) && (game2 == 2)) {
			delay(100);
			used2 = 1;
			talk(phrase2, sound2);
			if (currentChapter == 3)
				grr();
			else
				response(answer2);
		} else if ((leftMouseButton == 1) && (game3 == 2)) {
			delay(100);
			used3 = 1;
			talk(phrase3, sound3);
			if (currentChapter == 3)
				grr();
			else
				response(answer3);
		} else if ((leftMouseButton == 1) && (game4 == 2)) {
			delay(100);
			talk(phrase4, sound4);
			breakOut = 1;
		}

		if (leftMouseButton == 1) {
			delay(100);
			color_abc(kColorLightGreen);
		}

		game1 = (used1 == 0) ? 1 : 3;
		game2 = (used2 == 0) ? 1 : 3;
		game3 = (used3 == 0) ? 1 : 3;
		game4 = 1;
	} // while (breakOut == 0)

	if (currentChapter == 2)
		loadPic(menuBackground, backSurface);
	else
		loadPic(99, backSurface);
	if (currentChapter != 5)
		withoutVerb();
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
			animation_19_2();
		else if (function == 20)
			animation_20_2();
		else if (function == 21)
			animation_21_2();
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
	} else if (currentChapter == 4) {
		if (function == 2)
			animation_2_4();
		else if (function == 3)
			animation_3_4();
		else if (function == 4)
			animation_4_4();
	} else if (currentChapter == 5) {
		if (function == 2)
			animation_2_5();
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
			animation_16_5();
		else if (function == 17)
			animation_17_5();
	} else if (currentChapter == 6) {
		if (function == 2)
			animation_2_6();
		else if (function == 3)
			animation_3_6();
		else if (function == 4)
			animation_4_6();
		else if (function == 11)
			animation_11_6();
		else if (function == 12)
			animation_12_6();
		else if (function == 13)
			animation_13_6();
		else if (function == 14)
			animation_14_6();
		else if (function == 15)
			animation_15_6();
	}
}

} // End of namespace Drascula
