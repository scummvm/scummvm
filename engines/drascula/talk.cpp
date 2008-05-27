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

void DrasculaEngine::talk_igor_dch(const char *said, const char *filename) {
	int x_talk[8] = { 56, 82, 108, 134, 160, 186, 212, 238 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	pon_igor();
	pon_dr();
	copyBackground(x_igor + 17, y_igor, x_igor + 17, y_igor, 37, 24, dir_dibujo1, dir_zona_pantalla);

	copyRect(x_talk[cara], 148, x_igor + 17, y_igor, 25, 24, dir_hare_frente, dir_zona_pantalla);

	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, x_igor + 26, y_igor);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_igor();
	pon_dr();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_dr_izq(const char *said, const char *filename) {
	int x_talk[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(RED);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	pon_igor();
	pon_dr();
	if (num_ejec == 6)
		pon_hare();

	copyBackground(x_dr, y_dr, x_dr, y_dr, 38, 31, dir_dibujo1, dir_zona_pantalla);
	if (num_ejec == 6)
		copyRect(x_talk[cara], 90, x_dr, y_dr, 38, 31, dir_dibujo2, dir_zona_pantalla);
	else
		copyRect(x_talk[cara], 90, x_dr, y_dr, 38, 31, dir_hare_fondo, dir_zona_pantalla);

	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, x_dr + 19, y_dr);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_igor();
	pon_dr();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_dr_dch(const char *said, const char *filename) {
	int x_talk[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(RED);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();

	pon_igor();
	pon_dr();
	if (num_ejec == 6)
		pon_hare();

	copyBackground(x_dr, y_dr, x_dr, y_dr, 45, 31, dir_dibujo1, dir_zona_pantalla);
	if (num_ejec != 6)
		copyRect(x_talk[cara], 58, x_dr + 7, y_dr, 38, 31, dir_hare_fondo, dir_zona_pantalla);
	else
		copyRect(x_talk[cara], 58, x_dr + 7, y_dr, 38, 31, dir_dibujo2, dir_zona_pantalla);

	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, x_dr + 19, y_dr);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	if (num_ejec == 6)
		updateRoom(); 

	pon_igor();
	pon_dr();
	if (num_ejec == 6)
		pon_hare();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_solo(const char *said, const char *filename) {
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	if (num_ejec == 1)
		color_abc(color_solo);
	else if (num_ejec == 4)
		color_abc(RED);

	talkInit(filename);

	if (num_ejec == 6)
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

bucless:

	if (withVoices == 0) {
		if (num_ejec == 1)
			centra_texto(said, 156, 90);
		else if (num_ejec == 6)
			centra_texto(said, 213, 72);
		else if (num_ejec == 5)
			centra_texto(said, 173, 92);
	}
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}
	if (num_ejec == 6) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}
}

void DrasculaEngine::talk_igor_frente(const char *said, const char *filename) {
	int x_talk[8] = { 56, 86, 116, 146, 176, 206, 236, 266 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	if (num_ejec == 1 || num_ejec == 4)
		pon_igor();
	if (num_ejec == 1)
		pon_dr();
	if (num_ejec == 1 || num_ejec == 6)
		copyBackground(x_igor, y_igor, x_igor, y_igor, 29, 25, dir_dibujo1, dir_zona_pantalla);
	copyRect(x_talk[cara], 173, x_igor, y_igor, 29, 25, dir_hare_frente, dir_zona_pantalla);

	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, x_igor + 26, y_igor);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	if (num_ejec == 6) {
		updateRoom();
	}

	if (num_ejec == 1) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
	}
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_tabernero(const char *said, const char *filename) {
	int x_talk[9] = { 1, 23, 45, 67, 89, 111, 133, 155, 177 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(MAROON);

	talkInit(filename);

bucless:

	if (num_ejec == 1) {
		if (musicStatus() == 0)
			playMusic(roomMusic);
	} else if (num_ejec == 2) {
		if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
			playMusic(roomMusic);
	}

	cara = _rnd->getRandomNumber(8);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 2, 121, 44, 21, 24, dir_hare_dch, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 132, 45);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete(sku);
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_bj(const char *said, const char *filename) {
	int x_talk[5] = { 64, 92, 120, 148, 176 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:
	if (num_ejec != 5) {
		cara = _rnd->getRandomNumber(4);

		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

		updateRefresh_pre();

		copyBackground(x_bj + 2, y_bj - 1, x_bj + 2, y_bj - 1, 27, 40,
					dir_dibujo1, dir_zona_pantalla);

		copyRect(x_talk[cara], 99, x_bj + 2, y_bj - 1, 27, 40,
					dir_dibujo3, dir_zona_pantalla);
		pon_hare();
		updateRefresh();

		if (withVoices == 0)
			centra_texto(said, x_bj + 7, y_bj);

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		pause(3);
	} else {
		updateRoom();

		if (withVoices == 0)
			centra_texto(said, 93, 80);

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk(int index) {
	char name[100];
	sprintf(name, "%i.als", index);
	talk(_text[_lang][index], name);
}

void DrasculaEngine::talk(const char *said, const char *filename) {
	int suma_1_pixel = 0;
	if (num_ejec != 2)
		suma_1_pixel = 1;

	int y_mask_talk = 170;
	int x_talk_dch[6] = { 1, 25, 49, 73, 97, 121 };
	int x_talk_izq[6] = { 145, 169, 193, 217, 241, 265 };
	int cara;
	int longitud = strlen(said);

	if (num_ejec == 6) {
		if (flags[0] == 0 && roomNumber == 102) {
			talk_pen(said, filename);
			return;
		}
		if (flags[0] == 0 && roomNumber == 58) {
			talk_pen2(said, filename);
			return;
		}
	}

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	if (num_ejec != 2) {
		if (factor_red[hare_y + alto_hare] == 100)
			suma_1_pixel = 0;
	}

	if (num_ejec == 4) {
		if (roomNumber == 24 || flags[29] == 0) {
			color_abc(YELLOW);
		}
	} else {
		color_abc(YELLOW);
	}

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(5);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();
	if (num_ejec == 2)
		copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, ancho_hare, alto_talk - 1, dir_zona_pantalla, dir_dibujo3);
	else
		copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
				(int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]),
				dir_zona_pantalla, dir_dibujo3);

	pon_hare();

	if (num_ejec == 2) {
		if (!strcmp(fondo_y_menu, "99.alg") || !strcmp(fondo_y_menu, "994.alg"))
			copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, ancho_hare, alto_talk - 1, dir_dibujo3, dir_zona_pantalla);
	} else {
		copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
				(int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]),
				dir_dibujo3, dir_zona_pantalla);
	}

	if (sentido_hare == 0) {
		if (num_ejec == 2)
			copyRect(x_talk_izq[cara], y_mask_talk, hare_x + 8, hare_y - 1, ancho_talk, alto_talk,
						dir_hare_dch, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_izq[cara], y_mask_talk, hare_x + (int)((8.0f / 100) * factor_red[hare_y + alto_hare]),
						hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);

		updateRefresh();
	} else if (sentido_hare == 1) {
		if (num_ejec == 2)
			copyRect(x_talk_dch[cara], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk,
					dir_hare_dch, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_dch[cara], y_mask_talk, hare_x + (int)((12.0f / 100) * factor_red[hare_y + alto_hare]),
					hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		updateRefresh();
	} else if (sentido_hare == 2) {
		if (num_ejec == 2)
			copyRect(x_talk_izq[cara], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk,
					dir_hare_frente, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_izq[cara], y_mask_talk,
						suma_1_pixel + hare_x + (int)((12.0f / 100) * factor_red[hare_y + alto_hare]),
						hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
		updateRefresh();
	} else if (sentido_hare == 3) {
		if (num_ejec == 2)
			copyRect(x_talk_dch[cara], y_mask_talk, hare_x + 8, hare_y, ancho_talk, alto_talk,
					dir_hare_frente, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_dch[cara], y_mask_talk,
						suma_1_pixel + hare_x + (int)((8.0f / 100) * factor_red[hare_y + alto_hare]),
						hare_y, ancho_talk,alto_talk, factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
		updateRefresh();
	}

	if (withVoices == 0)
		centra_texto(said, hare_x, hare_y);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (num_ejec == 1 && key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (num_ejec == 1) {
		if (musicStatus() == 0 && flags[11] == 0 && corta_musica == 0)
			playMusic(roomMusic);
	}
}

void DrasculaEngine::talk_pianista(const char *said, const char *filename) {
	int x_talk[4] = { 97, 145, 193, 241 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(3);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 139, 228, 112, 47, 60,
				dir_hare_dch, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 221, 128);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_borracho(const char *said, const char *filename) {
	int x_talk[8] = { 1, 21, 41, 61, 81, 101, 121, 141 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	if (num_ejec == 1) {
		loadPic("an11y13.alg");
		decompressPic(dir_hare_frente, 1);
	}

	flags[13] = 1;

bebiendo:

	if (flags[12] == 1) {
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		goto bebiendo;
	}

	color_abc(DARK_GREEN);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 29, 177, 50, 19, 19, dir_hare_frente, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 181, 54);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	flags[13] = 0;
	if (num_ejec == 1) {
		loadPic("96.alg");
		decompressPic(dir_hare_frente, 1);
	}

	if (num_ejec == 1) {
		if (musicStatus() == 0 && flags[11] == 0)
			playMusic(roomMusic);
	} else if (num_ejec == 2) {
		if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
			playMusic(roomMusic);
	}
}

void DrasculaEngine::talk_vb(int index) {
	char name[100];
	sprintf(name, "VB%i.als", index);
	talk_vb(_textvb[_lang][index], name);
}

void DrasculaEngine::talk_vb(const char *said, const char *filename) {
	int x_talk[6] = {1, 27, 53, 79, 105, 131};
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(VON_BRAUN);

	talkInit(filename);

	copyBackground(vb_x + 5, 64, OBJWIDTH + 1, 0, 25, 27, dir_dibujo1, dir_dibujo3);

bucless:

	if (sentido_vb == 1) {
		cara = _rnd->getRandomNumber(5);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

		pon_hare();
		pon_vb();

		copyBackground(OBJWIDTH + 1, 0, vb_x + 5, 64, 25, 27, dir_dibujo3, dir_zona_pantalla);
		copyRect(x_talk[cara], 34, vb_x + 5, 64, 25, 27, dir_hare_frente, dir_zona_pantalla);
		updateRefresh();
	}

	if (withVoices == 0)
		centra_texto(said, vb_x, 66);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_vbpuerta(int index) {
	char name[100];
	sprintf(name, "VB%i.als", index);
	talk_vb(_textvb[_lang][index], name);
}

void DrasculaEngine::talk_vbpuerta(const char *said, const char *filename) {
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(VON_BRAUN);

	talkInit(filename);

bucless:

	updateRoom();
	if (withVoices == 0)
		centra_texto(said, 150, 80);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_ciego(const char *said, const char *filename, const char *sincronia) {
	byte *num_cara;
	int p = 0;
	int pos_ciego[6];
	int longitud = strlen(said);

	color_abc(VON_BRAUN);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	talkInit(filename);

	pos_ciego[1] = 2;
	pos_ciego[2] = 73;
	pos_ciego[3] = 1;
	pos_ciego[4] = 126;
	pos_ciego[5] = 149;

bucless:
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pos_ciego[5] = 149;
	char c = toupper(sincronia[p]);

	if (c == '0' || c == '2' || c == '4' || c == '6')
		pos_ciego[0] = 1;
	else
		pos_ciego[0] = 132;

	if (c == '0')
		num_cara = dir_dibujo3;
	else if (c == '1')
		num_cara = dir_dibujo3;
	else if (c == '2')
		num_cara = dir_hare_dch;
	else if (c == '3')
		num_cara = dir_hare_dch;
	else if (c == '4')
		num_cara = dir_hare_fondo;
	else if (c == '5')
		num_cara = dir_hare_fondo;
	else {
		num_cara = dir_hare_frente;
		pos_ciego[5] = 146;
	}

	copyRectClip( pos_ciego, num_cara, dir_zona_pantalla);

	if (withVoices == 0)
		centra_texto(said, 310, 71);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(2);
	p++;

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}
}

void DrasculaEngine::talk_hacker(const char *said, const char *filename) {
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	color_abc(YELLOW);

	talkInit(filename);

bucless:
	if (withVoices == 0)
		centra_texto(said, 156, 170);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	key = 0;
}

void DrasculaEngine::talk_lobo(const char *said, const char *filename) {
	int x_talk[9] = {52, 79, 106, 133, 160, 187, 214, 241, 268};
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(RED);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(8);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 136, 198, 81, 26, 24, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 203, 78);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_mus(const char *said, const char *filename) {
	int x_talk[8] = { 16, 35, 54, 73, 92, 111, 130, 149};
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 156, 190, 64, 18, 24, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 197, 64);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
	if (longitud > 0)
		goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_pen(const char *said, const char *filename) {
	int x_talk[8] = {112, 138, 164, 190, 216, 242, 268, 294};
	int cara;
	int longitud = strlen(said);

	flags[1] = 1;

	updateRoom();
	copyRect(44, 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(YELLOW);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(7);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyRect(x_talk[cara], 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);

	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 160, 105);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	flags[1] = 0;
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_pen2(const char *said, const char *filename) {
	int x_talk[5]={122, 148, 174, 200, 226};
	int cara;
	int longitud = strlen(said);

	flags[1] = 1;

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(YELLOW);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(4);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 171, 173, 116, 25, 28, dir_dibujo3, dir_zona_pantalla);

	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 195, 107);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	flags[1] = 0;
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_taber2(const char *said, const char *filename) {
	int x_talk[6] = {1, 23, 45, 67, 89, 111};
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(MAROON);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(5);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 130, 151, 43, 21, 24, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 132, 45);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_bj_cama(const char *said, const char *filename) {
	int x_talk[5] = {51, 101, 151, 201, 251};
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(4);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(65, 103, 65, 103, 49, 38, dir_dibujo1, dir_zona_pantalla);
	copyRect(x_talk[cara], 105, 65, 103, 49, 38, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 104, 102);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_htel(const char *said, const char *filename) {
	char *num_cara;
	int x_talk[3] = {1, 94, 187};
	int cara, pantalla;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(YELLOW);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(2);
	pantalla = _rnd->getRandomNumber(2);

	if (cara == 0 && pantalla == 0)
		num_cara = (char *)dir_dibujo3;
	else if (pantalla == 1)
		num_cara = (char *)dir_hare_frente;
	else
		num_cara = (char *)dir_hare_fondo;

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	copyBackground(x_talk[cara], 1, 45, 24, 92, 108, (byte *)num_cara, dir_zona_pantalla);

	if (withVoices == 0)
		centra_texto(said, 90, 50);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_sinc(const char *said, const char *filename, const char *sincronia) {
	int suma_1_pixel = 1;
	int y_mask_talk = 170;
	int x_talk_dch[6] = {1, 25, 49, 73, 97, 121};
	int x_talk_izq[6] = {145, 169, 193, 217, 241, 265};
	int p, cara = 0;
	int longitud = strlen(said);

	color_abc(YELLOW);

	if (num_ejec == 1) {
		if (factor_red[hare_y + alto_hare] == 100)
			suma_1_pixel = 0;
	}

	p = 0;

	talkInit(filename);

bucless:

	if (sincronia[p] == '0')
		cara = 0;
	if (sincronia[p] == '1')
		cara = 1;
	if (sincronia[p] == '2')
		cara = 2;
	if (sincronia[p] == '3')
		cara = 3;
	if (sincronia[p] == '4')
		cara = 4;
	if (sincronia[p] == '5')
		cara = 5;
	if (sincronia[p] == '6')
		cara = 6;
	if (sincronia[p] == '7')
		cara = 7;

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();
	if (num_ejec == 2)
		copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, ancho_hare, alto_talk - 1, dir_zona_pantalla, dir_dibujo3);
	else
		copyBackground(hare_x, hare_y, OBJWIDTH + 1, 0, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
				(int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]), dir_zona_pantalla, dir_dibujo3);
	pon_hare();
	if (num_ejec == 2) {
		if (alto_hare != 56)
			copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, ancho_hare, alto_talk - 1, dir_dibujo3, dir_zona_pantalla);
	} else
		copyBackground(OBJWIDTH + 1, 0, hare_x, hare_y, (int)(((float)ancho_hare / 100) * factor_red[hare_y + alto_hare]),
				(int)(((float)(alto_talk - 1) / 100) * factor_red[hare_y + alto_hare]), dir_dibujo3, dir_zona_pantalla);

	if (sentido_hare == 0) {
		if (num_ejec == 2)
			copyRect(x_talk_izq[cara], y_mask_talk, hare_x + 8, hare_y - 1, ancho_talk, alto_talk, dir_hare_dch, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_izq[cara], y_mask_talk, (int)(hare_x + (8.0f / 100) * factor_red[hare_y + alto_hare]),
							hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		updateRefresh();
	} else if (sentido_hare == 1) {
		if (num_ejec == 2)
			copyRect(x_talk_dch[cara], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk, dir_hare_dch, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_dch[cara], y_mask_talk, (int)(hare_x + (12.0f / 100) * factor_red[hare_y + alto_hare]),
							hare_y, ancho_talk, alto_talk, factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		updateRefresh();
	} else if (sentido_hare == 2) {
		if (num_ejec == 2)
			copyRect(x_talk_izq[cara], y_mask_talk, hare_x + 12, hare_y, ancho_talk, alto_talk, dir_hare_frente, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_izq[cara], y_mask_talk,
						(int)(suma_1_pixel + hare_x + (12.0f / 100) * factor_red[hare_y + alto_hare]), hare_y,
						ancho_talk, alto_talk, factor_red[hare_y + alto_hare], dir_hare_frente, dir_zona_pantalla);
		updateRefresh();
	} else if (sentido_hare == 3) {
		if (num_ejec == 2)
			copyRect(x_talk_dch[cara], y_mask_talk, hare_x + 8, hare_y, ancho_talk, alto_talk, dir_hare_frente, dir_zona_pantalla);
		else
			reduce_hare_chico(x_talk_dch[cara], y_mask_talk,
						(int)(suma_1_pixel + hare_x + (8.0f / 100) * factor_red[hare_y + alto_hare]), hare_y,
						ancho_talk, alto_talk, factor_red[hare_y + alto_hare], dir_hare_frente, dir_zona_pantalla);
		updateRefresh();
	}

	if (withVoices == 0)
		centra_texto(said, hare_x, hare_y);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	p++;
	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	if (num_ejec == 1 && musicStatus() == 0 && flags[11] == 0)
		playMusic(roomMusic);
	if (num_ejec == 2 && musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
		playMusic(roomMusic);
}

void DrasculaEngine::talk_baul(const char *said, const char *filename) {
	int cara = 0, cara_antes;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	cara_antes = flags[19];

	color_abc(MAROON);

	talkInit(filename);

bucless:

	if (cara == 1)
		cara = 0;
	else
		cara = 1;

	flags[19] = cara;
	updateRoom();

	if (withVoices == 0)
		centra_texto(said, 263, 69);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(4);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	flags[19] = cara_antes;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_igorpuerta(const char *said, const char *filename) {
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	updateRoom();
	if (withVoices == 0)
		centra_texto(said, 87, 66);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_igor_sentado(const char *said, const char *filename) {
	int x_talk[4] = { 80, 102, 124, 146 };
	int cara;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	cara = _rnd->getRandomNumber(3);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 109, 207, 92, 21, 23, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 221, 102);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::talk_igor_peluca(const char *said, const char *filename) {
	int x_talk[4] = {119, 158, 197, 236};
	int cara = 0;
	int longitud = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

	color_abc(WHITE);

	talkInit(filename);

bucless:

	_rnd->getRandomNumber(3);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();

	copyBackground(x_talk[cara], 78, 199, 94, 38, 27, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	updateRefresh();

	if (withVoices == 0)
		centra_texto(said, 221, 102);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1){
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud -= 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

} // End of namespace Drascula
