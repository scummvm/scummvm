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

int x_talk_dch[6] = {1, 25, 49, 73, 97, 121};
int x_talk_izq[6] = {145, 169, 193, 217, 241, 265};

void DrasculaEngine::talkInit(const char *filename) {
	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	playFile(filename);
}

bool DrasculaEngine::isTalkFinished(int* length) {
	byte key = getScan();
	if (key != 0)
		stopSound();
	if (soundIsActive())
		return false;

	return true;
}

// talker types:
// 0: talk_igor_dch
// 1: talk_igor_front
// 2: talk_igor_door
// 3: talk_igor_seated
// 4: talk_igor_wig
void DrasculaEngine::talk_igor(int index, int talkerType) {
	char filename[20];
	sprintf(filename, "I%i.als", index);
	const char *said = _texti[_lang][index];
	int x_talk0[8] = {  56,  82, 108, 134, 160, 186, 212, 238 };
	int x_talk1[8] = {  56,  86, 116, 146, 176, 206, 236, 266 };
	int x_talk3[4] = {  80, 102, 124, 146 };
	int x_talk4[4] = { 119, 158, 197, 236 };
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		if (talkerType == kIgorDch || talkerType == kIgorFront) {
			face = _rnd->getRandomNumber(7);
			copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
			updateRefresh_pre();
		} else if (talkerType == kIgorSeated || talkerType == kIgorWig) {
			face = _rnd->getRandomNumber(3);
			copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
			updateRefresh_pre();
		}
		
		if (talkerType == kIgorDch) {
			placeIgor();
			placeDrascula();
			copyBackground(igorX + 17, igorY, igorX + 17, igorY, 37, 24, drawSurface1, screenSurface);
			copyRect(x_talk0[face], 148, igorX + 17, igorY, 25, 24, frontSurface, screenSurface);
			updateRefresh();
			if (withVoices == 0)
				centerText(said, igorX + 26, igorY);
			updateScreen();
			pause(3);
		} else if (talkerType == kIgorFront) {
			if (currentChapter == 1 || currentChapter == 4)
				placeIgor();
			if (currentChapter == 1)
				placeDrascula();
			if (currentChapter == 1 || currentChapter == 6)
				copyBackground(igorX, igorY, igorX, igorY, 29, 25, drawSurface1, screenSurface);
			copyRect(x_talk1[face], 173, igorX, igorY, 29, 25, frontSurface, screenSurface);
			updateRefresh();
			if (withVoices == 0)
				centerText(said, igorX + 26, igorY);
			updateScreen();
			pause(3);
		} else if (talkerType == kIgorDoor) {
			updateRoom();
			if (withVoices == 0)
				centerText(said, 87, 66);
			updateScreen();
		} else if (talkerType == kIgorSeated || talkerType == kIgorWig) {
			if (talkerType == kIgorSeated)
				copyBackground(x_talk3[face], 109, 207, 92, 21, 23, drawSurface3, screenSurface);
			else
				copyBackground(x_talk4[face], 78, 199, 94, 38, 27, drawSurface3, screenSurface);
			moveCharacters();
			updateRefresh();
			if (withVoices == 0)
				centerText(said, 221, 102);

			updateScreen();
			pause(3);
		}
	} while	(!isTalkFinished(&length));

	if ((talkerType == kIgorFront && currentChapter == 6) || 
		 talkerType == kIgorDoor || talkerType == kIgorSeated || talkerType == kIgorWig) {
		updateRoom();
	}

	if (talkerType == kIgorDch || (talkerType == kIgorFront && currentChapter == 1)) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
	}
	updateScreen();
}

// Talker type 0: talk_dr_izq, 1: talk_dr_dch
void DrasculaEngine::talk_drascula(int index, int talkerType) {
	const char *said = _textd[_lang][index];
	char filename[20];
	sprintf(filename, "d%i.als", index);
	int x_talk[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int face;
	int length = strlen(said);
	int offset = (talkerType == 0) ? 0 : 7;
	int offset2 = (talkerType == 0) ? 90 : 58;

	color_abc(kColorRed);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(7);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		placeIgor();
		placeDrascula();
		if (currentChapter == 6)
			moveCharacters();

		copyBackground(drasculaX, drasculaY, drasculaX, drasculaY, 38 + offset, 31, drawSurface1, screenSurface);
		if (currentChapter == 6)
			copyRect(x_talk[face], offset2, drasculaX + offset, drasculaY, 38, 31, drawSurface2, screenSurface);
		else
			copyRect(x_talk[face], offset2, drasculaX + offset, drasculaY, 38, 31, backSurface, screenSurface);

		updateRefresh();

		if (withVoices == 0)
			centerText(said, drasculaX + 19, drasculaY);

		updateScreen();

		pause(3);

	} while (!isTalkFinished(&length));

	if (talkerType == 0)
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	if (talkerType == 1 && currentChapter == 6)
		updateRoom();

	placeIgor();
	placeDrascula();

	if (talkerType == 1 && currentChapter == 6)
		moveCharacters();

	updateScreen();
}

void DrasculaEngine::talk_solo(const char *said, const char *filename) {
	int length = strlen(said);

	if (currentChapter == 1)
		color_abc(color_solo);
	else if (currentChapter == 4)
		color_abc(kColorRed);

	talkInit(filename);

	if (currentChapter == 6)
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	do {
		if (withVoices == 0) {
			if (currentChapter == 1)
				centerText(said, 156, 90);
			else if (currentChapter == 6)
				centerText(said, 213, 72);
			else if (currentChapter == 5)
				centerText(said, 173, 92);
		}
		updateScreen();
	} while (!isTalkFinished(&length));

	if (currentChapter == 6) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateScreen();
	}
}

void DrasculaEngine::talk_bartender(int index, int talkerType) {
	char filename[20];
	sprintf(filename, "t%i.als", index);
	const char *said;

	// Line 82 is a special case
	if (index != 82)
		said = _textt[_lang][index];
	else {
		sprintf(filename, "d%i.als", index);
		said = _textd[_lang][index];
	}

	int x_talk[9] = { 1, 23, 45, 67, 89, 111, 133, 155, 177 };
	int face;
	int length = strlen(said);

	color_abc(kColorMaroon);

	talkInit(filename);

	do {
		if (talkerType == 0) {
			if (currentChapter == 1) {
				if (musicStatus() == 0)
					playMusic(roomMusic);
			} else if (currentChapter == 2) {
				if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
					playMusic(roomMusic);
			}

			face = _rnd->getRandomNumber(8);
		} else {
			face = _rnd->getRandomNumber(5);
		}

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		if (talkerType == 0)
			copyBackground(x_talk[face], 2, 121, 44, 21, 24, extraSurface, screenSurface);
		else
			copyBackground(x_talk[face], 130, 151, 43, 21, 24, drawSurface3, screenSurface);
		moveCharacters();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 132, 45);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_bj(int index) {
	char filename[20];
	sprintf(filename, "BJ%i.als", index);
	const char *said = _textbj[_lang][index];
	int x_talk[5] = { 64, 92, 120, 148, 176 };
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		if (currentChapter != 5) {
			face = _rnd->getRandomNumber(4);

			copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

			updateRefresh_pre();

			copyBackground(bjX + 2, bjY - 1, bjX + 2, bjY - 1, 27, 40,
						   drawSurface1, screenSurface);

			copyRect(x_talk[face], 99, bjX + 2, bjY - 1, 27, 40,
					 drawSurface3, screenSurface);
			moveCharacters();
			updateRefresh();

			if (withVoices == 0)
				centerText(said, bjX + 7, bjY);

			updateScreen();

			pause(3);
		} else {
			updateRoom();

			if (withVoices == 0)
				centerText(said, 93, 80);

			updateScreen();
		}
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk(int index) {
	char name[20];
	sprintf(name, "%i.als", index);
	talk(_text[_lang][index], name);
}

void DrasculaEngine::talk(const char *said, const char *filename) {
	int talkOffset = 0;
	if (currentChapter != 2)
		talkOffset = 1;

	int y_mask_talk = 170;
	int face;
	int length = strlen(said);

	if (currentChapter == 6) {
		if (flags[0] == 0 && roomNumber == 102) {
			talk_pen(said, filename, 0);
			return;
		}
		if (flags[0] == 0 && roomNumber == 58) {
			talk_pen(said, filename, 1);
			return;
		}
	}

	if (currentChapter != 2) {
		if (factor_red[curY + curHeight] == 100)
			talkOffset = 0;
	}

	if (currentChapter == 4) {
		if (roomNumber == 24 || flags[29] == 0) {
			color_abc(kColorYellow);
		}
	} else {
		color_abc(kColorYellow);
	}

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(5);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();
		if (currentChapter == 2)
			copyBackground(curX, curY, OBJWIDTH + 1, 0, curWidth, talkHeight - 1, screenSurface, drawSurface3);
		else
			copyBackground(curX, curY, OBJWIDTH + 1, 0, (int)(((float)curWidth / 100) * factor_red[curY + curHeight]),
					   (int)(((float)(talkHeight - 1) / 100) * factor_red[curY + curHeight]),
						   screenSurface, drawSurface3);

		moveCharacters();

		if (currentChapter == 2) {
			if (!strcmp(menuBackground, "99.alg") || !strcmp(menuBackground, "994.alg"))
				copyBackground(OBJWIDTH + 1, 0, curX, curY, curWidth, talkHeight - 1, drawSurface3, screenSurface);
		} else {
			copyBackground(OBJWIDTH + 1, 0, curX, curY, (int)(((float)curWidth / 100) * factor_red[curY + curHeight]),
					   (int)(((float)(talkHeight - 1) / 100) * factor_red[curY + curHeight]),
						   drawSurface3, screenSurface);
		}

		if (trackProtagonist == 0) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, curX + 8, curY - 1, talkWidth, talkHeight,
						extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk, curX + (int)((8.0f / 100) * factor_red[curY + curHeight]),
						curY, talkWidth, talkHeight, factor_red[curY + curHeight],
						extraSurface, screenSurface);

			updateRefresh();
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, curX + 12, curY, talkWidth, talkHeight,
					extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk, curX + (int)((12.0f / 100) * factor_red[curY + curHeight]),
					curY, talkWidth, talkHeight, factor_red[curY + curHeight], extraSurface, screenSurface);
			updateRefresh();
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, curX + 12, curY, talkWidth, talkHeight,
					frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk,
						talkOffset + curX + (int)((12.0f / 100) * factor_red[curY + curHeight]),
						curY, talkWidth, talkHeight, factor_red[curY + curHeight],
						frontSurface, screenSurface);
			updateRefresh();
		} else if (trackProtagonist == 3) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, curX + 8, curY, talkWidth, talkHeight,
					frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk,
						talkOffset + curX + (int)((8.0f / 100) * factor_red[curY + curHeight]),
						curY, talkWidth,talkHeight, factor_red[curY + curHeight],
						frontSurface, screenSurface);
			updateRefresh();
		}

		if (withVoices == 0)
			centerText(said, curX, curY);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();

	if (currentChapter == 1) {
		if (musicStatus() == 0 && flags[11] == 0 && musicStopped == 0)
			playMusic(roomMusic);
	}
}

void DrasculaEngine::talk_pianist(int index) {
	char filename[20];
	sprintf(filename, "P%i.als", index);
	const char* said = _textp[_lang][index];
	int x_talk[4] = { 97, 145, 193, 241 };
	int coords[7] = { 139, 228, 112, 47, 60, 221, 128 };

	color_abc(kColorWhite);
	talk_generic(said, filename, x_talk, 4, coords);
}

void DrasculaEngine::talk_drunk(int index) {
	char filename[20];
	sprintf(filename, "B%i.als", index);
	const char *said = _textb[_lang][index];
	int x_talk[8] = { 1, 21, 41, 61, 81, 101, 121, 141 };
	int coords[7] = { 29, 177, 50, 19, 19, 181, 54 };

	if (currentChapter == 1)
		loadPic("an11y13.alg", frontSurface);

	flags[13] = 1;

	while (flags[12] == 1) {
		updateRoom();
		updateScreen();
	}

	color_abc(kColorDarkGreen);

	talk_generic(said, filename, x_talk, 8, coords);

	flags[13] = 0;
	if (currentChapter == 1)
		loadPic("96.alg", frontSurface);

	if (currentChapter == 1) {
		if (musicStatus() == 0 && flags[11] == 0)
			playMusic(roomMusic);
	} else if (currentChapter == 2) {
		if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
			playMusic(roomMusic);
	}
}

void DrasculaEngine::talk_vonBraun(int index) {
	char filename[20];
	sprintf(filename, "VB%i.als", index);
	const char *said = _textvb[_lang][index];
	int x_talk[6] = {1, 27, 53, 79, 105, 131};
	int face;
	int length = strlen(said);

	color_abc(kColorBrown);

	talkInit(filename);

	copyBackground(vonBraunX + 5, 64, OBJWIDTH + 1, 0, 25, 27, drawSurface1, drawSurface3);

	do {
		if (trackVonBraun == 1) {
			face = _rnd->getRandomNumber(5);
			copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

			moveCharacters();
			moveVonBraun();

			copyBackground(OBJWIDTH + 1, 0, vonBraunX + 5, 64, 25, 27, drawSurface3, screenSurface);
			copyRect(x_talk[face], 34, vonBraunX + 5, 64, 25, 27, frontSurface, screenSurface);
			updateRefresh();
		}

		if (withVoices == 0)
			centerText(said, vonBraunX, 66);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
	if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_vonBraunpuerta(int index) {
	char filename[20];
	sprintf(filename, "VB%i.als", index);
	const char *said = _textvb[_lang][index];
	int length = strlen(said);

	color_abc(kColorBrown);

	talkInit(filename);

	do {
		updateRoom();

		if (withVoices == 0)
			centerText(said, 150, 80);

		updateScreen();
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
	if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_blind(int index) {
	// Special case: a second array is used for sync here
	// Also, the blind man's texts in the first array and his
	// voice files start from 58, not 1
	char filename[20];
	sprintf(filename, "d%i.als", index + TEXTD_START - 1);
	const char *said = _textd[_lang][index + TEXTD_START - 1];
	const char *syncChar = _textd1[_lang][index - 1];

	byte *faceBuffer;
	int p = 0;
	int pos_blind[6];
	int length = strlen(said);

	color_abc(kColorBrown);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	talkInit(filename);

	pos_blind[1] = 2;
	pos_blind[2] = 73;
	pos_blind[3] = 1;
	pos_blind[4] = 126;
	pos_blind[5] = 149;

	do {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		pos_blind[5] = 149;
		char c = toupper(syncChar[p]);

		if (c == '0' || c == '2' || c == '4' || c == '6')
			pos_blind[0] = 1;
		else
			pos_blind[0] = 132;

		if (c == '0' || c == '1')
			faceBuffer = drawSurface3;
		else if (c == '2' || c == '3')
			faceBuffer = extraSurface;
		else if (c == '4' || c == '5')
			faceBuffer = backSurface;
		else {
			faceBuffer = frontSurface;
			pos_blind[5] = 146;
		}

		copyRectClip( pos_blind, faceBuffer, screenSurface);

		if (withVoices == 0)
			centerText(said, 310, 71);

		updateScreen();
		pause(2);
		p++;
	} while (!isTalkFinished(&length));
}

void DrasculaEngine::talk_hacker(int index) {
	char filename[20];
	sprintf(filename, "d%i.als", index);
	const char *said = _textd[_lang][index];
	int length = strlen(said);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();

	color_abc(kColorYellow);

	talkInit(filename);

	do {
		if (withVoices == 0)
			centerText(said, 156, 170);
		updateScreen();
	} while (!isTalkFinished(&length));
}

void DrasculaEngine::talk_werewolf(int index) {
	char filename[20];
	sprintf(filename, "L%i.als", index);
	const char *said = _textl[_lang][index];
	int x_talk[9] = {52, 79, 106, 133, 160, 187, 214, 241, 268};
	int coords[7] = { 136, 198, 81, 26, 24, 203, 78 };

	color_abc(kColorRed);
	talk_generic(said, filename, x_talk, 9, coords);
}

void DrasculaEngine::talk_mus(int index) {
	char filename[20];
	sprintf(filename, "E%i.als", index);
	const char *said = _texte[_lang][index];
	int x_talk[8] = { 16, 35, 54, 73, 92, 111, 130, 149};
	int coords[7] = { 156, 190, 64, 18, 24, 197, 64 };

	color_abc(kColorWhite);
	talk_generic(said, filename, x_talk, 8, coords);
}

void DrasculaEngine::talk_pen(const char *said, const char *filename, int talkerType) {
	int x_talk[8] = {112, 138, 164, 190, 216, 242, 268, 294};
	int x_talk2[5] = {122, 148, 174, 200, 226};
	int face;
	int length = strlen(said);

	flags[1] = 1;

	if (talkerType == 0) {
		updateRoom();
		copyRect(44, 145, 145, 105, 25, 29, drawSurface3, screenSurface);
		updateScreen();
	}

	color_abc(kColorYellow);

	talkInit(filename);

	do {
		if (talkerType == 0)
			face = _rnd->getRandomNumber(7);
		else
			face = _rnd->getRandomNumber(4);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();

		if (talkerType == 0)
			copyRect(x_talk[face], 145, 145, 105, 25, 29, drawSurface3, screenSurface);
		else
			copyBackground(x_talk2[face], 171, 173, 116, 25, 28, drawSurface3, screenSurface);

		updateRefresh();

		if (withVoices == 0) {
			if (talkerType == 0)
				centerText(said, 160, 105);
			else
				centerText(said, 195, 107);
		}

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	flags[1] = 0;
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateRefresh_pre();
	updateScreen();
}

void DrasculaEngine::talk_bj_bed(int index) {
	char filename[20];
	sprintf(filename, "BJ%i.als", index);
	const char *said = _textbj[_lang][index];
	int x_talk[5] = {51, 101, 151, 201, 251};
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(4);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(65, 103, 65, 103, 49, 38, drawSurface1, screenSurface);
		copyRect(x_talk[face], 105, 65, 103, 49, 38, drawSurface3, screenSurface);
		moveCharacters();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 104, 102);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_htel(int index) {
	char filename[20];
	sprintf(filename, "%i.als", index);
	const char *said = _text[_lang][index];
	char *faceBuffer;
	int x_talk[3] = {1, 94, 187};
	int face, curScreen;
	int length = strlen(said);

	color_abc(kColorYellow);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(2);
		curScreen = _rnd->getRandomNumber(2);

		if (face == 0 && curScreen == 0)
			faceBuffer = (char *)drawSurface3;
		else if (curScreen == 1)
			faceBuffer = (char *)frontSurface;
		else
			faceBuffer = (char *)backSurface;

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		copyBackground(x_talk[face], 1, 45, 24, 92, 108, (byte *)faceBuffer, screenSurface);

		if (withVoices == 0)
			centerText(said, 90, 50);

		updateScreen();
		pause(3);
	} while (!isTalkFinished(&length));

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();
}

void DrasculaEngine::talk_sync(const char *said, const char *filename, const char *syncChar) {
	int talkOffset = 1;
	int y_mask_talk = 170;
	int p, face = 0;
	int length = strlen(said);
	char buf[2];

	color_abc(kColorYellow);

	if (currentChapter == 1) {
		if (factor_red[curY + curHeight] == 100)
			talkOffset = 0;
	}

	p = 0;

	talkInit(filename);

	do {
		strncpy(buf, &syncChar[p], 1);
		face = atoi(buf);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();
		if (currentChapter == 2)
			copyBackground(curX, curY, OBJWIDTH + 1, 0, curWidth, talkHeight - 1, screenSurface, drawSurface3);
		else
			copyBackground(curX, curY, OBJWIDTH + 1, 0, (int)(((float)curWidth / 100) * factor_red[curY + curHeight]),
				(int)(((float)(talkHeight - 1) / 100) * factor_red[curY + curHeight]), screenSurface, drawSurface3);
		moveCharacters();
		if (currentChapter == 2) {
			if (curHeight != 56)
				copyBackground(OBJWIDTH + 1, 0, curX, curY, curWidth, talkHeight - 1, drawSurface3, screenSurface);
		} else
			copyBackground(OBJWIDTH + 1, 0, curX, curY, (int)(((float)curWidth / 100) * factor_red[curY + curHeight]),
				(int)(((float)(talkHeight - 1) / 100) * factor_red[curY + curHeight]), drawSurface3, screenSurface);

		if (trackProtagonist == 0) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, curX + 8, curY - 1, talkWidth, talkHeight, extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk, (int)(curX + (8.0f / 100) * factor_red[curY + curHeight]),
							curY, talkWidth, talkHeight, factor_red[curY + curHeight], extraSurface, screenSurface);
			updateRefresh();
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, curX + 12, curY, talkWidth, talkHeight, extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk, (int)(curX + (12.0f / 100) * factor_red[curY + curHeight]),
							curY, talkWidth, talkHeight, factor_red[curY + curHeight], extraSurface, screenSurface);
			updateRefresh();
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, curX + 12, curY, talkWidth, talkHeight, frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk,
						(int)(talkOffset + curX + (12.0f / 100) * factor_red[curY + curHeight]), curY,
						talkWidth, talkHeight, factor_red[curY + curHeight], frontSurface, screenSurface);
			updateRefresh();
		} else if (trackProtagonist == 3) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, curX + 8, curY, talkWidth, talkHeight, frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk,
						(int)(talkOffset + curX + (8.0f / 100) * factor_red[curY + curHeight]), curY,
						talkWidth, talkHeight, factor_red[curY + curHeight], frontSurface, screenSurface);
			updateRefresh();
		}

		if (withVoices == 0)
			centerText(said, curX, curY);

		updateScreen();

		p++;
		pause(3);
	} while (!isTalkFinished(&length));

	if (currentChapter == 1 && musicStatus() == 0 && flags[11] == 0)
		playMusic(roomMusic);
	if (currentChapter == 2 && musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_baul(int index) {
	char filename[20];
	sprintf(filename, "d%i.als", index);
	const char *said = _text[_lang][index];
	int face = 0, cara_antes;
	int length = strlen(said);

	cara_antes = flags[19];

	color_abc(kColorMaroon);

	talkInit(filename);

	do {
		face = (face == 1) ? 0 : 1;

		flags[19] = face;
		updateRoom();

		if (withVoices == 0)
			centerText(said, 263, 69);

		updateScreen();

		pause(4);
	} while (!isTalkFinished(&length));

	flags[19] = cara_antes;
	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_dr_grande(int index) {
	char filename[20];
	sprintf(filename, "D%i.als", index);
	const char *said = _textd[_lang][index];
	int x_talk[4] = {47, 93, 139, 185};
	int face;
	int l = 0;
	int length = strlen(said);

	color_abc(kColorRed);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(3);
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		copyBackground(interf_x[l] + 24, interf_y[l], 0, 45, 39, 31, drawSurface2, screenSurface);
		copyBackground(x_talk[face], 1, 171, 68, 45, 48, drawSurface2, screenSurface);
		l++;
		if (l == 7)
			l = 0;

		if (withVoices == 0)
			centerText(said, 191, 69);

		updateScreen();

		pause(3);

		byte key = getScan();
		if (key == Common::KEYCODE_ESCAPE)
			term_int = 1;
	} while (!isTalkFinished(&length));
}

void DrasculaEngine::talk_generic(const char* said, const char* filename, int* faces, int faceCount, int* coords) {
	int face;
	int length = strlen(said);
	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(faceCount - 1);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		updateRefresh_pre();
		copyBackground(faces[face], coords[0], coords[1], coords[2], 
						coords[3], coords[4], drawSurface3, screenSurface);
		moveCharacters();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, coords[5], coords[6]);

		updateScreen();

		pause(3);

	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

} // End of namespace Drascula
