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

void DrasculaEngine::talkInit(const char *filename) {
	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}
}

bool DrasculaEngine::isTalkFinished(int* length) {
	byte key = getScan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (soundIsActive())
			return false;
		delete(sku);
		sku = 0;
		ctvd_terminate();
	} else {
		length -= 2;
		if (length > 0)
			return false;
	}

	return true;
}

void DrasculaEngine::talk_igor_dch(int index) {
	char name[20];
	sprintf(name, "I%i.als", index);
	talk_igor_dch(_texti[_lang][index], name);
}

void DrasculaEngine::talk_igor_dch(const char *said, const char *filename) {
	int x_talk[8] = { 56, 82, 108, 134, 160, 186, 212, 238 };
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(7);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		placeIgor();
		placeDrascula();
		copyBackground(igorX + 17, igorY, igorX + 17, igorY, 37, 24, drawSurface1, screenSurface);

		copyRect(x_talk[face], 148, igorX + 17, igorY, 25, 24, frontSurface, screenSurface);

		updateRefresh();

		if (withVoices == 0)
			centerText(said, igorX + 26, igorY);

		updateScreen();

		pause(3);

	} while	(!isTalkFinished(&length));

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	placeIgor();
	placeDrascula();
	updateScreen();
}

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
			pon_hare();

		copyBackground(x_dr, y_dr, x_dr, y_dr, 38 + offset, 31, drawSurface1, screenSurface);
		if (currentChapter == 6)
			copyRect(x_talk[face], offset2, x_dr + offset, y_dr, 38, 31, drawSurface2, screenSurface);
		else
			copyRect(x_talk[face], offset2, x_dr + offset, y_dr, 38, 31, backSurface, screenSurface);

		updateRefresh();

		if (withVoices == 0)
			centerText(said, x_dr + 19, y_dr);

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
		pon_hare();

	updateScreen();
}

void DrasculaEngine::talk_dr_dch(int index) {
	talk_drascula(index, 1);
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

void DrasculaEngine::talk_igor_front(int index) {
	char name[20];
	sprintf(name, "I%i.als", index);
	talk_igor_front(_texti[_lang][index], name);
}

void DrasculaEngine::talk_igor_front(const char *said, const char *filename) {
	int x_talk[8] = { 56, 86, 116, 146, 176, 206, 236, 266 };
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(7);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		if (currentChapter == 1 || currentChapter == 4)
			placeIgor();
		if (currentChapter == 1)
			placeDrascula();
		if (currentChapter == 1 || currentChapter == 6)
			copyBackground(igorX, igorY, igorX, igorY, 29, 25, drawSurface1, screenSurface);
		copyRect(x_talk[face], 173, igorX, igorY, 29, 25, frontSurface, screenSurface);

		updateRefresh();

		if (withVoices == 0)
			centerText(said, igorX + 26, igorY);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	if (currentChapter == 6) {
		updateRoom();
	}

	if (currentChapter == 1) {
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		placeIgor();
		placeDrascula();
	}
	updateScreen();
}

void DrasculaEngine::talk_bartender(int index) {
	char name[20];
	sprintf(name, "t%i.als", index);
	talk_bartender(_textt[_lang][index], name);
}

void DrasculaEngine::talk_bartender(const char *said, const char *filename) {
	int x_talk[9] = { 1, 23, 45, 67, 89, 111, 133, 155, 177 };
	int face;
	int length = strlen(said);

	color_abc(kColorMaroon);

	talkInit(filename);

	do {
		if (currentChapter == 1) {
			if (musicStatus() == 0)
				playMusic(roomMusic);
		} else if (currentChapter == 2) {
			if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
				playMusic(roomMusic);
		}

		face = _rnd->getRandomNumber(8);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 2, 121, 44, 21, 24, extraSurface, screenSurface);
		pon_hare();
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
	char name[20];
	sprintf(name, "BJ%i.als", index);
	talk_bj(_textbj[_lang][index], name);
}

void DrasculaEngine::talk_bj(const char *said, const char *filename) {
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

			copyBackground(x_bj + 2, y_bj - 1, x_bj + 2, y_bj - 1, 27, 40,
						   drawSurface1, screenSurface);

			copyRect(x_talk[face], 99, x_bj + 2, y_bj - 1, 27, 40,
					 drawSurface3, screenSurface);
			pon_hare();
			updateRefresh();

			if (withVoices == 0)
				centerText(said, x_bj + 7, y_bj);

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
	int suma_1_pixel = 0;
	if (currentChapter != 2)
		suma_1_pixel = 1;

	int y_mask_talk = 170;
	int x_talk_dch[6] = { 1, 25, 49, 73, 97, 121 };
	int x_talk_izq[6] = { 145, 169, 193, 217, 241, 265 };
	int face;
	int length = strlen(said);

	if (currentChapter == 6) {
		if (flags[0] == 0 && roomNumber == 102) {
			talk_pen(said, filename);
			return;
		}
		if (flags[0] == 0 && roomNumber == 58) {
			talk_pen2(said, filename);
			return;
		}
	}

	if (currentChapter != 2) {
		if (factor_red[hare_y + alto_hare] == 100)
			suma_1_pixel = 0;
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
			copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, ancho_hare, alto_talk - 1, screenSurface, drawSurface3);
		else
			copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
					   (int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]),
						   screenSurface, drawSurface3);

		pon_hare();

		if (currentChapter == 2) {
			if (!strcmp(menuBackground, "99.alg") || !strcmp(menuBackground, "994.alg"))
				copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, ancho_hare, alto_talk - 1, drawSurface3, screenSurface);
		} else {
			copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
					   (int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]),
						   drawSurface3, screenSurface);
		}

		if (sentido_hare == 0) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, hare_x + 8, hare_y - 1, ancho_talk, alto_talk,
						extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk, hare_x + (int)((8.0f / 100) * factor_red[hare_y + alto_hare]),
						hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare],
						extraSurface, screenSurface);

			updateRefresh();
		} else if (sentido_hare == 1) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk,
					extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk, hare_x + (int)((12.0f / 100) * factor_red[hare_y + alto_hare]),
					hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare], extraSurface, screenSurface);
			updateRefresh();
		} else if (sentido_hare == 2) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk,
					frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk,
						suma_1_pixel + hare_x + (int)((12.0f / 100) * factor_red[hare_y + alto_hare]),
						hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare],
						frontSurface, screenSurface);
			updateRefresh();
		} else if (sentido_hare == 3) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, hare_x + 8, hare_y, ancho_talk, alto_talk,
					frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk,
						suma_1_pixel + hare_x + (int)((8.0f / 100) * factor_red[hare_y + alto_hare]),
						hare_y, ancho_talk,alto_talk, factor_red[hare_y + alto_hare],
						frontSurface, screenSurface);
			updateRefresh();
		}

		if (withVoices == 0)
			centerText(said, hare_x, hare_y);

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
	char name[20];
	sprintf(name, "P%i.als", index);
	talk(_textp[_lang][index], name);
}

void DrasculaEngine::talk_pianist(const char *said, const char *filename) {
	int x_talk[4] = { 97, 145, 193, 241 };
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(3);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 139, 228, 112, 47, 60,
					   extraSurface, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 221, 128);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_drunk(int index) {
	char name[20];
	sprintf(name, "B%i.als", index);
	talk_drunk(_textb[_lang][index], name);
}

void DrasculaEngine::talk_drunk(const char *said, const char *filename) {
	int x_talk[8] = { 1, 21, 41, 61, 81, 101, 121, 141 };
	int face;
	int length = strlen(said);

	if (currentChapter == 1) {
		loadAndDecompressPic("an11y13.alg", frontSurface, 1);
	}

	flags[13] = 1;

	while (flags[12] == 1) {
		updateRoom();
		updateScreen();
	}

	color_abc(kColorDarkGreen);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(7);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 29, 177, 50, 19, 19, frontSurface, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 181, 54);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();

	flags[13] = 0;
	if (currentChapter == 1) {
		loadAndDecompressPic("96.alg", frontSurface, 1);
	}

	if (currentChapter == 1) {
		if (musicStatus() == 0 && flags[11] == 0)
			playMusic(roomMusic);
	} else if (currentChapter == 2) {
		if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
			playMusic(roomMusic);
	}
}

void DrasculaEngine::talk_vb(int index) {
	char name[20];
	sprintf(name, "VB%i.als", index);
	talk_vb(_textvb[_lang][index], name);
}

void DrasculaEngine::talk_vb(const char *said, const char *filename) {
	int x_talk[6] = {1, 27, 53, 79, 105, 131};
	int face;
	int length = strlen(said);

	color_abc(kColorBrown);

	talkInit(filename);

	copyBackground(vb_x + 5, 64, OBJWIDTH + 1, 0, 25, 27, drawSurface1, drawSurface3);

	do {
		if (sentido_vb == 1) {
			face = _rnd->getRandomNumber(5);
			copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

			pon_hare();
			pon_vb();

			copyBackground(OBJWIDTH + 1, 0, vb_x + 5, 64, 25, 27, drawSurface3, screenSurface);
			copyRect(x_talk[face], 34, vb_x + 5, 64, 25, 27, frontSurface, screenSurface);
			updateRefresh();
		}

		if (withVoices == 0)
			centerText(said, vb_x, 66);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
	if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_vbpuerta(int index) {
	char name[20];
	sprintf(name, "VB%i.als", index);
	talk_vb(_textvb[_lang][index], name);
}

void DrasculaEngine::talk_vbpuerta(const char *said, const char *filename) {
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

void DrasculaEngine::talk_blind(const char *said, const char *filename, const char *sincronia) {
	byte *num_cara;
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
		char c = toupper(sincronia[p]);

		if (c == '0' || c == '2' || c == '4' || c == '6')
			pos_blind[0] = 1;
		else
			pos_blind[0] = 132;

		if (c == '0' || c == '1')
			num_cara = drawSurface3;
		else if (c == '2' || c == '3')
			num_cara = extraSurface;
		else if (c == '4' || c == '5')
			num_cara = backSurface;
		else {
			num_cara = frontSurface;
			pos_blind[5] = 146;
		}

		copyRectClip( pos_blind, num_cara, screenSurface);

		if (withVoices == 0)
			centerText(said, 310, 71);

		updateScreen();
		pause(2);
		p++;
	} while (!isTalkFinished(&length));
}

void DrasculaEngine::talk_hacker(const char *said, const char *filename) {
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

void DrasculaEngine::talk_wolf(int index) {
	char name[20];
	sprintf(name, "L%i.als", index);
	talk_wolf(_textl[_lang][index], name);
}

void DrasculaEngine::talk_wolf(const char *said, const char *filename) {
	int x_talk[9] = {52, 79, 106, 133, 160, 187, 214, 241, 268};
	int face;
	int length = strlen(said);

	color_abc(kColorRed);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(8);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 136, 198, 81, 26, 24, drawSurface3, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 203, 78);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_mus(int index) {
	char name[20];
	sprintf(name, "E%i.als", index);
	talk_mus(_texte[_lang][index], name);
}

void DrasculaEngine::talk_mus(const char *said, const char *filename) {
	int x_talk[8] = { 16, 35, 54, 73, 92, 111, 130, 149};
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(7);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 156, 190, 64, 18, 24, drawSurface3, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 197, 64);

		updateScreen();

		pause(3);

	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_pen(const char *said, const char *filename) {
	int x_talk[8] = {112, 138, 164, 190, 216, 242, 268, 294};
	int face;
	int length = strlen(said);

	flags[1] = 1;

	updateRoom();
	copyRect(44, 145, 145, 105, 25, 29, drawSurface3, screenSurface);
	updateScreen();

	color_abc(kColorYellow);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(7);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyRect(x_talk[face], 145, 145, 105, 25, 29, drawSurface3, screenSurface);

		updateRefresh();

		if (withVoices == 0)
			centerText(said, 160, 105);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	flags[1] = 0;
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateRefresh_pre();
	updateScreen();
}

void DrasculaEngine::talk_pen2(const char *said, const char *filename) {
	int x_talk[5]={122, 148, 174, 200, 226};
	int face;
	int length = strlen(said);

	flags[1] = 1;

	color_abc(kColorYellow);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(4);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 171, 173, 116, 25, 28, drawSurface3, screenSurface);

		updateRefresh();

		if (withVoices == 0)
			centerText(said, 195, 107);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	flags[1] = 0;
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateRefresh_pre();
	updateScreen();
}

void DrasculaEngine::talk_taber2(int index) {
	char name[20];
	sprintf(name, "t%i.als", index);
	talk_taber2(_textt[_lang][index], name);
}

void DrasculaEngine::talk_taber2(const char *said, const char *filename) {
	int x_talk[6] = {1, 23, 45, 67, 89, 111};
	int face;
	int length = strlen(said);

	color_abc(kColorMaroon);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(5);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 130, 151, 43, 21, 24, drawSurface3, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 132, 45);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_bj_bed(int index) {
	char name[20];
	sprintf(name, "BJ%i.als", index);
	talk_bj_bed(_textbj[_lang][index], name);
}

void DrasculaEngine::talk_bj_bed(const char *said, const char *filename) {
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
		pon_hare();
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
	char name[20];
	sprintf(name, "%i.als", index);
	talk_htel(_text[_lang][index], name);
}

void DrasculaEngine::talk_htel(const char *said, const char *filename) {
	char *num_cara;
	int x_talk[3] = {1, 94, 187};
	int face, pantalla;
	int length = strlen(said);

	color_abc(kColorYellow);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(2);
		pantalla = _rnd->getRandomNumber(2);

		if (face == 0 && pantalla == 0)
			num_cara = (char *)drawSurface3;
		else if (pantalla == 1)
			num_cara = (char *)frontSurface;
		else
			num_cara = (char *)backSurface;

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		copyBackground(x_talk[face], 1, 45, 24, 92, 108, (byte *)num_cara, screenSurface);

		if (withVoices == 0)
			centerText(said, 90, 50);

		updateScreen();
		pause(3);
	} while (!isTalkFinished(&length));

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();
}

void DrasculaEngine::talk_sinc(const char *said, const char *filename, const char *sincronia) {
	int suma_1_pixel = 1;
	int y_mask_talk = 170;
	int x_talk_dch[6] = {1, 25, 49, 73, 97, 121};
	int x_talk_izq[6] = {145, 169, 193, 217, 241, 265};
	int p, face = 0;
	int length = strlen(said);

	color_abc(kColorYellow);

	if (currentChapter == 1) {
		if (factor_red[hare_y + alto_hare] == 100)
			suma_1_pixel = 0;
	}

	p = 0;

	talkInit(filename);

	do {
		face = atoi(&sincronia[p]);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();
		if (currentChapter == 2)
			copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, ancho_hare, alto_talk - 1, screenSurface, drawSurface3);
		else
			copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
				(int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]), screenSurface, drawSurface3);
		pon_hare();
		if (currentChapter == 2) {
			if (alto_hare != 56)
				copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, ancho_hare, alto_talk - 1, drawSurface3, screenSurface);
		} else
			copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
				(int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]), drawSurface3, screenSurface);

		if (sentido_hare == 0) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, hare_x + 8, hare_y - 1, ancho_talk, alto_talk, extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk, (int)(hare_x + (8.0f / 100) * factor_red[hare_y + alto_hare]),
							hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare], extraSurface, screenSurface);
			updateRefresh();
		} else if (sentido_hare == 1) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk, extraSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk, (int)(hare_x + (12.0f / 100) * factor_red[hare_y + alto_hare]),
							hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare], extraSurface, screenSurface);
			updateRefresh();
		} else if (sentido_hare == 2) {
			if (currentChapter == 2)
				copyRect(x_talk_izq[face], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk, frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_izq[face], y_mask_talk,
						(int)(suma_1_pixel + hare_x + (12.0f / 100) * factor_red[hare_y + alto_hare]), hare_y,
						ancho_talk, alto_talk, factor_red[hare_y + alto_hare], frontSurface, screenSurface);
			updateRefresh();
		} else if (sentido_hare == 3) {
			if (currentChapter == 2)
				copyRect(x_talk_dch[face], y_mask_talk, hare_x + 8, hare_y, ancho_talk, alto_talk, frontSurface, screenSurface);
			else
				reduce_hare_chico(x_talk_dch[face], y_mask_talk,
						(int)(suma_1_pixel + hare_x + (8.0f / 100) * factor_red[hare_y + alto_hare]), hare_y,
						ancho_talk, alto_talk, factor_red[hare_y + alto_hare], frontSurface, screenSurface);
			updateRefresh();
		}

		if (withVoices == 0)
			centerText(said, hare_x, hare_y);

		updateScreen();

		p++;
		pause(3);
	} while (!isTalkFinished(&length));

	if (currentChapter == 1 && musicStatus() == 0 && flags[11] == 0)
		playMusic(roomMusic);
	if (currentChapter == 2 && musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_baul(const char *said, const char *filename) {
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

void DrasculaEngine::talk_igor_door(int index) {
	char name[20];
	sprintf(name, "I%i.als", index);
	talk_igor_door(_texti[_lang][index], name);
}

void DrasculaEngine::talk_igor_door(const char *said, const char *filename) {
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		updateRoom();
		if (withVoices == 0)
			centerText(said, 87, 66);
		updateScreen();
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_igor_seated(int index) {
	char name[20];
	sprintf(name, "I%i.als", index);
	talk_igor_seated(_texti[_lang][index], name);
}

void DrasculaEngine::talk_igor_seated(const char *said, const char *filename) {
	int x_talk[4] = { 80, 102, 124, 146 };
	int face;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(3);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 109, 207, 92, 21, 23, drawSurface3, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 221, 102);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_igor_wig(int index) {
	char name[20];
	sprintf(name, "I%i.als", index);
	talk_igor_wig(_texti[_lang][index], name);
}

void DrasculaEngine::talk_igor_wig(const char *said, const char *filename) {
	int x_talk[4] = {119, 158, 197, 236};
	int face = 0;
	int length = strlen(said);

	color_abc(kColorWhite);

	talkInit(filename);

	do {
		face = _rnd->getRandomNumber(3);

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

		updateRefresh_pre();

		copyBackground(x_talk[face], 78, 199, 94, 38, 27, drawSurface3, screenSurface);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centerText(said, 221, 102);

		updateScreen();

		pause(3);
	} while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::talk_dr_grande(const char *said, const char *filename) {
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

} // End of namespace Drascula
