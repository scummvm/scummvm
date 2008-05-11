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

static const int interf_x[] ={ 1, 65, 129, 193, 1, 65, 129 };
static const int interf_y[] ={ 51, 51, 51, 51, 83, 83, 83 };

void DrasculaEngine::animation_1_1() {
	int l, l2, p;
	int pos_pixel[6];

	while (term_int == 0) {
		playMusic(29);
		fliplay("logoddm.bin", 9);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(600);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		delay(340);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		playMusic(26);
		delay(500);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fliplay("logoalc.bin", 8);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("cielo.alg");
		decompressPic(dir_zona_pantalla, 256);
		Negro();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(2);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(900);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_abc(RED);
		centra_texto("Transilvanya, 1993 d.c.", 160, 100);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1000);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1200);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		fliplay("scrollb.bin", 9);

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		comienza_sound("s5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("scr2.bin", 17);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fin_sound_corte();
		anima("scr3.bin", 17);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("cielo2.alg");
		decompressPic(dir_zona_pantalla, 256);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		FundeAlNegro(1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		loadPic("96.alg");
		decompressPic(dir_hare_frente, COMPLETA);
		loadPic("103.alg");
		decompressPic(dir_dibujo1, MEDIA);
		loadPic("104.alg");
		decompressPic(dir_dibujo3, 1);
		loadPic("aux104.alg");
		decompressPic(dir_dibujo2, 1);

		playMusic(4);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(400);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		for (l2 = 0; l2 < 3; l2++)
			for (l = 0; l < 7; l++) {
				copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
				copyBackground(interf_x[l], interf_y[l], 156, 45, 63, 31, dir_dibujo2, dir_zona_pantalla);
				updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
				if (getscan() == Common::KEYCODE_ESCAPE) {
					term_int = 1;
					break;
				}
				pause(3);
			}
			if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
				break;

		l2 = 0; p = 0;
		pos_pixel[3] = 45;
		pos_pixel[4] = 63;
		pos_pixel[5] = 31;

		for (l = 0; l < 180; l++) {
			copyBackground(0, 0, 320 - l, 0, l, 200, dir_dibujo3, dir_zona_pantalla);
			copyBackground(l, 0, 0, 0, 320 - l, 200, dir_dibujo1, dir_zona_pantalla);

			pos_pixel[0] = interf_x[l2];
			pos_pixel[1] = interf_y[l2];
			pos_pixel[2] = 156 - l;

			copyRectClip(pos_pixel, dir_dibujo2, dir_zona_pantalla);
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			p++;
			if (p == 6) {
				p = 0;
				l2++;
			}
			if (l2 == 7)
				l2 = 0;
			if (getscan() == Common::KEYCODE_ESCAPE) {
				term_int = 1;
				break;
			}
		}
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);

		talk_dr_grande(_textd[_lang][1], "D1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		clearRoom();

		loadPic("100.alg");
		decompressPic(dir_dibujo1, MEDIA);
		loadPic("auxigor.alg");
		decompressPic(dir_hare_frente, 1);
		loadPic("auxdr.alg");
		decompressPic(dir_hare_fondo, 1);
		sentido_dr = 0;
		x_dr = 129;
		y_dr = 95;
		sentido_igor = 1;
		x_igor = 66;
		y_igor = 97;

		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_igor_dch(_texti[_lang][8], "I8.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_dr_izq(_textd[_lang][2], "d2.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(_textd[_lang][3], "d3.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		color_solo = RED;
		loadPic("plan1.alg");
		decompressPic(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(10);
		talk_solo(_textd[_lang][4],"d4.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("plan1.alg");
		decompressPic(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_solo(_textd[_lang][5], "d5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("plan2.alg");
		decompressPic(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(20);
		talk_solo(_textd[_lang][6], "d6.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("plan3.alg");
		decompressPic(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(20);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_textd[_lang][7], "d7.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("plan3.alg");
		decompressPic(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_solo(_textd[_lang][8], "d8.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("100.alg");
		decompressPic(dir_dibujo1, MEDIA);
		MusicFadeout();
		stopMusic();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(_texti[_lang][9], "I9.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(_textd[_lang][9], "d9.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(_texti[_lang][10], "I10.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		playMusic(11);
		talk_dr_izq(_textd[_lang][10], "d10.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("rayo1.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		comienza_sound("s5.als");
		anima("rayo2.bin", 15);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("frel2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("frel.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("frel.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fin_sound_corte();
		clearRoom();
		Negro();
		playMusic(23);
		FundeDelNegro(0);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 1;
		talk_igor_dch(_texti[_lang][1], "I1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_dch(_textd[_lang][11], "d11.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(1);
		sentido_dr = 0;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_dr_izq(_textd[_lang][12], "d12.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(1);
		sentido_dr = 1;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_igor_dch(_texti[_lang][2], "I2.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pause(13);
		talk_dr_dch(_textd[_lang][13],"d13.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(1);
		sentido_dr = 0;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_dr_izq(_textd[_lang][14], "d14.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(_texti[_lang][3], "I3.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(_textd[_lang][15], "d15.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(_texti[_lang][4], "I4.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(_textd[_lang][16], "d16.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(_texti[_lang][5], "I5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_igor = 3;
		talk_dr_izq(_textd[_lang][17], "d17.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pause(18);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_frente(_texti[_lang][6], "I6.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		FundeAlNegro(0);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		playMusic(2);
		pause(5);
		fliplay("intro.bin", 12);
		term_int = 1;
	}
	clearRoom();
	loadPic("96.alg");
	decompressPic(dir_hare_frente, COMPLETA);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::talk_dr_grande(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;
	int x_talk[4] = {47, 93, 139, 185};
	int cara;
	int l = 0;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	color_abc(RED);

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

bucless:

	cara = _rnd->getRandomNumber(3);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	copyBackground(interf_x[l] + 24, interf_y[l], 0, 45, 39, 31, dir_dibujo2, dir_zona_pantalla);
	copyBackground(x_talk[cara], 1, 171, 68, 45, 48, dir_dibujo2, dir_zona_pantalla);
	l++;
	if (l == 7)
		l = 0;

	if (con_voces == 0)
		centra_texto(dicho, 191, 69);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
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
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}
}

void DrasculaEngine::animation_2_1() {
	int l;

	lleva_al_hare(231, 91);
	hare_se_ve = 0;

	term_int = 0;

	for (;;) {
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		anima("ag.bin", 14);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		loadPic("an11y13.alg");
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		talk_tabernero(_textt[_lang][22], "T22.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		loadPic("97.alg");
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		pause(4);
		comienza_sound("s1.als");
		hipo(18);
		fin_sound();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		clearRoom();
		stopMusic();
		corta_musica = 1;
		memset(dir_zona_pantalla, 0, 64000);
		color_solo = WHITE;
		pause(80);

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_textbj[_lang][1], "BJ1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("bj.alg");
		decompressPic(dir_zona_pantalla, MEDIA);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		Negro();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = YELLOW;
		talk_solo(_text[_lang][214], "214.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		loadPic("16.alg");
		decompressPic(dir_dibujo1, MEDIA);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("auxbj.alg");
		decompressPic(dir_dibujo3, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		strcpy(num_room, "16.alg");

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		for (l = 0; l < 200; l++)
			factor_red[l] = 99;
		x_bj = 170;
		y_bj = 90;
		sentido_bj = 0;
		hare_x = 91;
		hare_y = 95;
		sentido_hare = 1;
		hare_se_ve = 1;
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		loadPic("97g.alg");
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		anima("lev.bin", 15);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		lleva_al_hare(100 + ancho_hare / 2, 99 + alto_hare);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		hare_x = 100;
		hare_y = 95;

		talk_bj(_textbj[_lang][2], "BJ2.als");
		talk(_text[_lang][215], "215.als");
		talk_bj(_textbj[_lang][3], "BJ3.als");
		talk(_text[_lang][216], "216.als");
		talk_bj(_textbj[_lang][4], "BJ4.als");
		talk_bj(_textbj[_lang][5], "BJ5.als");
		talk_bj(_textbj[_lang][6], "BJ6.als");
		talk(_text[_lang][217], "217.als");
		talk_bj(_textbj[_lang][7], "BJ7.als");
		talk(_text[_lang][218], "218.als");
		talk_bj(_textbj[_lang][8], "BJ8.als");
		talk(_text[_lang][219], "219.als");
		talk_bj(_textbj[_lang][9], "BJ9.als");
		talk(_text[_lang][220], "220.als");
		talk(_text[_lang][221], "221.als");
		talk_bj(_textbj[_lang][10], "BJ10.als");
		talk(_text[_lang][222], "222.als");
		anima("gaf.bin", 15);
		anima("bjb.bin", 14);
		playMusic(9);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("97.alg");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pause(120);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_text[_lang][223], "223.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = WHITE;
		updateRoom();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(110);
		talk_solo(_textbj[_lang][11], "BJ11.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pause(118);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lleva_al_hare(132, 97 + alto_hare);
		pause(60);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk(_text[_lang][224], "224.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk_bj(_textbj[_lang][12], "BJ12.als");
		lleva_al_hare(157, 98 + alto_hare);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("bes.bin", 16);
		playMusic(11);
		anima("rap.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 3;
		strcpy(num_room, "no_bj.alg");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pause(8);
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk(_text[_lang][225], "225.als");
		pause(76);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk(_text[_lang][226], "226.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(30);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		talk(_text[_lang][227],"227.als");
		FundeAlNegro(0);
		break;
	}
}

void DrasculaEngine::animation_3_1() {
	loadPic("an11y13.alg");
	decompressPic(dir_hare_dch, 1);

	talk(_text[_lang][192], "192.als");
	talk_tabernero(_textt[_lang][1], "t1.als");
	talk(_text[_lang][193], "193.als");
	talk_tabernero(_textt[_lang][2], "t2.als");
	talk(_text[_lang][194], "194.als");
	talk_tabernero(_textt[_lang][3], "t3.als");
	talk(_text[_lang][195], "195.als");
	talk_tabernero(_textt[_lang][4], "t4.als");
	talk(_text[_lang][196], "196.als");
	talk_tabernero(_textt[_lang][5], "t5.als");
	talk_tabernero(_textt[_lang][6], "t6.als");
	talk(_text[_lang][197], "197.als");
	talk_tabernero(_textt[_lang][7], "t7.als");
	talk(_text[_lang][198], "198.als");
	talk_tabernero(_textt[_lang][8], "t8.als");
	talk(_text[_lang][199], "199.als");
	talk_tabernero(_textt[_lang][9], "t9.als");
	talk(_text[_lang][200], "200.als");
	talk(_text[_lang][201], "201.als");
	talk(_text[_lang][202], "202.als");

	flags[0] = 1;

	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_4_1() {
	loadPic("an12.alg");
	decompressPic(dir_hare_dch, 1);

	talk(_text[_lang][205],"205.als");

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	updateScreen(228,112, 228,112, 47,60, dir_zona_pantalla);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianista(_textp[_lang][1], "p1.als");
	talk(_text[_lang][206], "206.als");
	talk_pianista(_textp[_lang][2], "p2.als");
	talk(_text[_lang][207], "207.als");
	talk_pianista(_textp[_lang][3], "p3.als");
	talk(_text[_lang][208], "208.als");
	talk_pianista(_textp[_lang][4], "p4.als");
	talk(_text[_lang][209], "209.als");

	flags[11] = 0;
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_1_2() {
	lleva_al_hare(178, 121);
	lleva_al_hare(169, 135);
}

void DrasculaEngine::animation_2_2() {
	int n, x=0;

	sentido_hare = 0;
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_hare();
	updateRefresh();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	loadPic("an2_1.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("an2_2.alg");
	decompressPic(dir_hare_dch, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	copyBackground(1, 1, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		updateScreen(201,87, 201,87, 50,52, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 55, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++){
		x++;
		copyBackground(x, 109, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	x = 0;
	comienza_sound("s2.als");

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		updateScreen(201,87, 201,87, 50,52, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 55, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}
	x = 0;

	for (n = 0; n < 2; n++) {
		x++;
		copyBackground(x, 109, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	fin_sound();

	pause (4);

	comienza_sound("s1.als");
	hipo_sin_nadie(12);
	fin_sound();
}

void DrasculaEngine::animation_3_2() {
	lleva_al_hare(163, 106);
	lleva_al_hare(287, 101);
	sentido_hare = 0;
}

void DrasculaEngine::animation_4_2() {
	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(_textd[_lang][56], "d56.als");
	pause(8);

	clearRoom();
	loadPic("ciego1.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("ciego2.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("ciego3.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("ciego4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("ciego5.alg");
	decompressPic(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(10);

	talk_ciego(_textd[_lang][68], "d68.als", "44472225500022227555544444664447222550002222755554444466");
	pause(5);
	talk_hacker(_textd[_lang][57], "d57.als");
	pause(6);
	talk_ciego(_textd[_lang][69],"d69.als","444722255000222275555444446655033336666664464402256555005504450005446");
	pause(4);
	talk_hacker(_textd[_lang][58],"d58.als");
	talk_ciego(_textd[_lang][70],"d70.als", "4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046");
	delay(14);
	talk_hacker(_textd[_lang][59],"d59.als");
	talk_ciego(_textd[_lang][71],"d71.als", "550330227556444744446660004446655544444722255000222275555444446644444");
	talk_hacker(_textd[_lang][60],"d60.als");
	talk_ciego(_textd[_lang][72],"d72.als", "55033022755644455550444744400044504447222550002222755554444466000");
	talk_hacker(_textd[_lang][61],"d61.als");
	talk_ciego(_textd[_lang][73],"d73.als", "55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446");
	talk_hacker(_textd[_lang][62],"d62.als");
	talk_ciego(_textd[_lang][74],"d74.als", "55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666");
	talk_hacker(_textd[_lang][63],"d63.als");
	talk_ciego(_textd[_lang][75],"d75.als", "44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555");
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	_system->delayMillis(1);
	talk_hacker(_textd[_lang][64], "d64.als");
	talk_ciego(_textd[_lang][76], "d76.als", "5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(14);

	clearRoom();

	playMusic(musica_room);
	loadPic("9.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("aux9.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	sin_verbo();

	flags[9] = 0;
	flags[4] = 1;
}

void DrasculaEngine::animation_8_2() {
	talk_pianista(_textp[_lang][6], "P6.als");
	talk(_text[_lang][358], "358.als");
	talk_pianista(_textp[_lang][7], "P7.als");
	talk_pianista(_textp[_lang][8], "P8.als");
}

void DrasculaEngine::animation_9_2() {
	talk_pianista(_textp[_lang][9], "P9.als");
	talk_pianista(_textp[_lang][10], "P10.als");
	talk_pianista(_textp[_lang][11], "P11.als");
}

void DrasculaEngine::animation_10_2() {
	talk_pianista(_textp[_lang][12], "P12.als");
	talk(_text[_lang][361], "361.als");
	pause(40);
	talk_pianista(_textp[_lang][13], "P13.als");
	talk(_text[_lang][362], "362.als");
	talk_pianista(_textp[_lang][14], "P14.als");
	talk(_text[_lang][363], "363.als");
	talk_pianista(_textp[_lang][15], "P15.als");
	talk(_text[_lang][364], "364.als");
	talk_pianista(_textp[_lang][16], "P16.als");
}

void DrasculaEngine::animation_14_2() {
	int n, pos_cabina[6];
	int l = 0;

	loadPic("an14_2.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an14_1.alg");

	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = -160;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;

	for (n = -160; n <= 0; n = n + 5 + l) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		pon_hare();
		pon_vb();
		pos_cabina[3] = n;
		copyRectClip(pos_cabina, dir_hare_fondo, dir_zona_pantalla);
		updateRefresh();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		l = l + 1;
	}

	flags[24] = 1;

	decompressPic(dir_dibujo1, 1);

	comienza_sound("s7.als");
	hipo(15);

	fin_sound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_15_2() {
	talk_borracho(_textb[_lang][8], "B8.als");
	pause(7);
	talk_borracho(_textb[_lang][9], "B9.als");
	talk_borracho(_textb[_lang][10], "B10.als");
	talk_borracho(_textb[_lang][11], "B11.als");
}

void DrasculaEngine::animation_16_2() {
	int l;

	talk_borracho(_textb[_lang][12], "B12.als");
	talk(_text[_lang][371], "371.als");

	clearRoom();

	playMusic(32);
	int key = getscan();
	if (key != 0)
		goto asco;

	color_abc(DARK_GREEN);

	loadPic("his1.alg");
	decompressPic(dir_dibujo1, MEDIA);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(_texthis[_lang][1], 180, 180);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);
	key = getscan();
	if (key != 0)
		goto asco;

	clearRoom();
	loadPic("his2.alg");
	decompressPic(dir_dibujo1, MEDIA);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(_texthis[_lang][2], 180, 180);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);
	key = getscan();
	if (key != 0)
		goto asco;

	clearRoom();
	loadPic("his3.alg");
	decompressPic(dir_dibujo1, MEDIA);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(_texthis[_lang][3], 180, 180);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);

	clearRoom();
	loadPic("his4_1.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("his4_2.alg");
	decompressPic(dir_dibujo3, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo3, dir_zona_pantalla);
	centra_texto(_texthis[_lang][1], 180, 180);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	for (l = 1; l < 200; l++) {
		copyBackground(0, 0, 0, l, 320, 200 - l, dir_dibujo3, dir_zona_pantalla);
		copyBackground(0, 200 - l, 0, 0, 320, l, dir_dibujo1, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		key = getscan();
		if (key != 0)
			goto asco;
	}

	pause(5);
	FundeAlNegro(2);
	clearRoom();

asco:
	loadPic(roomDisk);
	decompressPic(dir_dibujo3, 1);
	loadPic(num_room);
	decompressPic(dir_dibujo1, MEDIA);
	Negro();
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	if (musica_room != 0)
		playMusic(musica_room);
	else
		stopMusic();
}

void DrasculaEngine::animation_17_2() {
	talk_borracho(_textb[_lang][13], "B13.als");
	talk_borracho(_textb[_lang][14], "B14.als");
	flags[40] = 1;
}

void DrasculaEngine::animation_19_2() {
	talk_vbpuerta(_textvb[_lang][5], "VB5.als");
}

void DrasculaEngine::animation_20_2() {
	talk_vbpuerta(_textvb[_lang][7], "VB7.als");
	talk_vbpuerta(_textvb[_lang][8], "VB8.als");
	talk(_text[_lang][383], "383.als");
	talk_vbpuerta(_textvb[_lang][9], "VB9.als");
	talk(_text[_lang][384], "384.als");
	talk_vbpuerta(_textvb[_lang][10], "VB10.als");
	talk(_text[_lang][385], "385.als");
	talk_vbpuerta(_textvb[_lang][11], "VB11.als");
	if (flags[23] == 0) {
		talk(_text[_lang][350], "350.als");
		talk_vbpuerta(_textvb[_lang][57], "VB57.als");
	} else {
		talk(_text[_lang][386], "386.als");
		talk_vbpuerta(_textvb[_lang][12], "VB12.als");
		flags[18] = 0;
		flags[14] = 1;
		abre_puerta(15, 1);
		sal_de_la_habitacion(1);
		animation_23_2();
		sal_de_la_habitacion(0);
		flags[21] = 0;
		flags[24] = 0;
		sentido_vb = 1;
		vb_x = 120;

		rompo_y_salgo = 1;
	}
}

void DrasculaEngine::animation_21_2() {
	talk_vbpuerta(_textvb[_lang][6], "VB6.als");
}

void DrasculaEngine::animation_23_2() {
	loadPic("an24.alg");
	decompressPic(dir_hare_frente, 1);

	flags[21] = 1;

	if (flags[25] == 0) {
		talk_vb(_textvb[_lang][13], "VB13.als");
		talk_vb(_textvb[_lang][14], "VB14.als");
		pause(10);
		talk(_text[_lang][387], "387.als");
	}

	talk_vb(_textvb[_lang][15], "VB15.als");
	lleva_vb(42);
	sentido_vb = 1;
	talk_vb(_textvb[_lang][16], "VB16.als");
	sentido_vb = 2;
	lleva_al_hare(157, 147);
	lleva_al_hare(131, 149);
	sentido_hare = 0;
	animation_14_2();
	if (flags[25] == 0)
		talk_vb(_textvb[_lang][17], "VB17.als");
	pause(8);
	sentido_vb = 1;
	talk_vb(_textvb[_lang][18], "VB18.als");

	if (flags[29] == 0)
		animation_23_anexo();
	else
		animation_23_anexo2();

	sentido_vb = 2;
	animation_25_2();
	lleva_vb(99);

	if (flags[29] == 0) {
		talk_vb(_textvb[_lang][19], "VB19.als");
		if (flags[25] == 0) {
			talk_vb(_textvb[_lang][20],"VB20.als");
			if (resta_objeto(7) == 0)
				flags[30] = 1;
			if (resta_objeto(18) == 0)
				flags[31] = 1;
			if (resta_objeto(19) == 0)
				flags[32] = 1;
		}
		talk_vb(_textvb[_lang][21], "VB21.als");
	} else
		animation_27_2();

	flags[25] = 1;
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_23_anexo() {
	int n, p_x = hare_x + 2, p_y = hare_y - 3;
	int x[] = {1, 38, 75, 112, 75, 112, 75, 112, 149, 112, 149, 112, 149, 186, 223, 260,
				1, 38, 75, 112, 149, 112, 149, 112, 149, 112, 149, 186, 223, 260, 260, 260, 260, 223};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 76, 76, 76, 76, 76, 76, 76,
				76, 76, 76, 76, 76, 76, 76, 1, 1, 1, 1};

	loadPic("an23.alg");
	decompressPic(dir_hare_fondo, 1);

	for (n = 0; n < 34; n++) {
		copyRect(p_x, p_y, p_x, p_y, 36, 74, dir_dibujo1, dir_zona_pantalla);
		copyRect(x[n], y[n], p_x, p_y, 36, 74, dir_hare_fondo, dir_zona_pantalla);
		updateRefresh();
		updateScreen(p_x, p_y, p_x, p_y, 36, 74, dir_zona_pantalla);
		pause(5);
	}

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_23_anexo2() {
	int n, p_x = hare_x + 4, p_y = hare_y;
	int x[] = {1, 35, 69, 103, 137, 171, 205, 239, 273, 1, 35, 69, 103, 137};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 73, 73, 73, 73, 73};

	pause(50);

	loadPic("an23_2.alg");
	decompressPic(dir_hare_fondo, 1);

	for (n = 0; n < 14; n++) {
		copyRect(p_x, p_y, p_x, p_y, 33, 71, dir_dibujo1, dir_zona_pantalla);
		copyRect(x[n], y[n], p_x, p_y, 33, 71, dir_hare_fondo, dir_zona_pantalla);
		updateRefresh();
		updateScreen(p_x,p_y, p_x,p_y, 33,71, dir_zona_pantalla);
		pause(5);
	}

	loadPic("99.alg");
	decompressPic(dir_hare_fondo,1);
}

void DrasculaEngine::animation_25_2() {
	int n, pos_cabina[6];

	loadPic("an14_2.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("18.alg");
	decompressPic(dir_dibujo1, 1);

	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = 0;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;

	flags[24] = 0;

	comienza_sound("s6.als");

	for (n = 0; n >= -160; n = n - 8) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

		updateRefresh_pre();
		pon_hare();
		pon_vb();

		pos_cabina[3] = n;

		copyRectClip(pos_cabina, dir_hare_fondo, dir_zona_pantalla);

		updateRefresh();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	fin_sound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_27_2() {
	flags[22] = 1;

	sin_verbo();
	resta_objeto(23);
	suma_objeto(11);

	talk_vb(_textvb[_lang][23], "VB23.als");
	talk_vb(_textvb[_lang][24], "VB24.als");
	if (flags[30] == 1)
		suma_objeto(7);
	if (flags[31] == 1)
		suma_objeto(18);
	if (flags[32] == 1)
		suma_objeto(19);
	talk_vb(_textvb[_lang][25],"VB25.als");
	talk_vb(_textvb[_lang][26],"VB26.als");
}

void DrasculaEngine::animation_28_2() {
	talk_vb(_textvb[_lang][27], "VB27.als");
	talk_vb(_textvb[_lang][28], "VB28.als");
	talk_vb(_textvb[_lang][29], "VB29.als");
	talk_vb(_textvb[_lang][30], "VB30.als");
}

void DrasculaEngine::animation_29_2() {
	if (flags[33] == 0) {
		talk_vb(_textvb[_lang][32], "VB32.als");
		talk(_text[_lang][398], "398.als");
		talk_vb(_textvb[_lang][33], "VB33.als");
		talk(_text[_lang][399], "399.als");
		talk_vb(_textvb[_lang][34], "VB34.als");
		talk_vb(_textvb[_lang][35], "VB35.als");
		talk(_text[_lang][400], "400.als");
		talk_vb(_textvb[_lang][36], "VB36.als");
		talk_vb(_textvb[_lang][37], "VB37.als");
		talk(_text[_lang][386], "386.als");
		talk_vb(_textvb[_lang][38], "VB38.als");
		talk_vb(_textvb[_lang][39], "VB39.als");
		talk(_text[_lang][401], "401.als");
		talk_vb(_textvb[_lang][40], "VB40.als");
		talk_vb(_textvb[_lang][41], "VB41.als");
		flags[33] = 1;
	} else
		talk_vb(_textvb[_lang][43], "VB43.als");

	talk(_text[_lang][402], "402.als");
	talk_vb(_textvb[_lang][42], "VB42.als");

	if (flags[38] == 0) {
		talk(_text[_lang][403], "403.als");
		rompo_y_salgo = 1;
	} else
		talk(_text[_lang][386], "386.als");
}

void DrasculaEngine::animation_30_2() {
	talk_vb(_textvb[_lang][31], "VB31.als");
	talk(_text[_lang][396], "396.als");
}

void DrasculaEngine::animation_31_2() {
	talk_vb(_textvb[_lang][44], "VB44.als");
	lleva_vb(-50);
	pause(15);
	lleva_al_hare(159, 140);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	sentido_hare = 2;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(78);
	sentido_hare = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(22);
	talk(_text[_lang][406], "406.als");
	lleva_vb(98);
	talk_vb(_textvb[_lang][45], "VB45.als");
	talk_vb(_textvb[_lang][46], "VB46.als");
	talk_vb(_textvb[_lang][47], "VB47.als");
	talk(_text[_lang][407], "407.als");
	talk_vb(_textvb[_lang][48], "VB48.als");
	talk_vb(_textvb[_lang][49], "VB49.als");
	talk(_text[_lang][408], "408.als");
	talk_vb(_textvb[_lang][50], "VB50.als");
	talk_vb(_textvb[_lang][51], "VB51.als");
	talk(_text[_lang][409], "409.als");
	talk_vb(_textvb[_lang][52], "VB52.als");
	talk_vb(_textvb[_lang][53], "VB53.als");
	pause(12);
	talk_vb(_textvb[_lang][54], "VB54.als");
	talk_vb(_textvb[_lang][55], "VB55.als");
	talk(_text[_lang][410], "410.als");
	talk_vb(_textvb[_lang][56], "VB56.als");

	rompo_y_salgo = 1;

	flags[38] = 0;
	flags[36] = 1;
	sin_verbo();
	resta_objeto(8);
	resta_objeto(13);
	resta_objeto(15);
	resta_objeto(16);
	resta_objeto(17);
	suma_objeto(20);
}

void DrasculaEngine::animation_35_2() {
	int n, x = 0;

	lleva_al_hare(96, 165);
	lleva_al_hare(79, 165);

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	loadPic("an35_1.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an35_2.alg");
	decompressPic(dir_hare_frente, 1);

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 70, 90, 46, 80, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(70,90, 70,90, 46,80,dir_zona_pantalla);
		x = x + 46;
		pause(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 82, 70, 90, 46, 80, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(70, 90, 70, 90, 46, 80, dir_zona_pantalla);
		x = x + 46;
		pause(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 70, 90, 46, 80, dir_hare_frente, dir_zona_pantalla);
		updateScreen(70, 90, 70, 90, 46, 80, dir_zona_pantalla);

		x = x + 46;

		pause(3);
	}

	x = 0;
	for (n = 0; n < 2; n++) {
		x++;
		copyBackground(x, 82, 70, 90, 46, 80, dir_hare_frente, dir_zona_pantalla);
		updateScreen(70, 90, 70,90, 46, 80,dir_zona_pantalla);
		x = x + 46;
		pause(3);
	}

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(19);

	comienza_sound("s1.als");
	hipo_sin_nadie(18);
	fin_sound();

	pause(10);

	FundeAlNegro(2);
}

void DrasculaEngine::animation_1_3() {
	talk(_text[_lang][413], "413.als");
	grr();
	pause(50);
	talk(_text[_lang][414], "414.als");
}

void DrasculaEngine::animation_2_3() {
	flags[0] = 1;
	playMusic(13);
	animation_3_3();
	playMusic(13);
	animation_4_3();
	flags[1] = 1;
	updateRoom();
	updateScreen(120, 0, 120, 0, 200, 200, dir_zona_pantalla);
	animation_5_3();
	flags[0] = 0;
	flags[1] = 1;

	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);

	lleva_al_hare(332, 127);
}

void DrasculaEngine::animation_3_3() {
	int n, x = 0;
	int px = hare_x - 20, py = hare_y - 1;

	loadPic("an2y_1.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("an2y_2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("an2y_3.alg");
	decompressPic(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}
}

void DrasculaEngine::animation_4_3() {
	int n, x = 0;
	int px = 120, py = 63;

	loadPic("any_1.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("any_2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("any_3.alg");
	decompressPic(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++){
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, px, py, 77, 89, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 91, px, py, 77, 89, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, px, py, 77, 89, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 91, px, py, 77, 89, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, px, py, 77, 89, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 91, px, py, 77, 89, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pause(3);
	}
}

void DrasculaEngine::animation_5_3() {
	int n, x = 0;
	int px = hare_x - 20, py = hare_y - 1;

	loadPic("an3y_1.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("an3y_2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("an3y_3.alg");
	decompressPic(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px,py, px,py, 71,72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pause(3);
	}
}

void DrasculaEngine::animation_6_3() {
	int frame = 0, px = 112, py = 62;
	int yoda_x[] = { 3 ,82, 161, 240, 3, 82 };
	int yoda_y[] = { 3, 3, 3, 3, 94, 94 };

	hare_se_mueve = 0;
	flags[3] = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	flags[1] = 0;

	loadPic("an4y.alg");
	decompressPic(dir_hare_frente, 1);

	for (frame = 0; frame < 6; frame++) {
		pause(3);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(yoda_x[frame], yoda_y[frame], px, py,	78, 90,	dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 78, 90, dir_zona_pantalla);
	}

	flags[2] = 1;

	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::animation_rayo() {
	loadPic("anr_1.alg");
	decompressPic(dir_hare_frente, MEDIA);
	loadPic("anr_2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("anr_3.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("anr_4.alg");
	decompressPic(dir_dibujo1, 1);
	loadPic("anr_5.alg");
	decompressPic(dir_dibujo3, 1);

	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_frente);

	pause(50);

	comienza_sound("s5.als");

	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_dch);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_fondo);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo1);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_fondo);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo3);
	pause(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_frente);
	fin_sound();
}

void DrasculaEngine::animation_2_4() {
	talk_igor_sentado(_texti[_lang][16], "I16.als");
	talk(_text[_lang][278], "278.als");
	talk_igor_sentado(_texti[_lang][17], "I17.als");
	talk(_text[_lang][279], "279.als");
	talk_igor_sentado(_texti[_lang][18], "I18.als");
}

void DrasculaEngine::animation_3_4() {
	talk_igor_sentado(_texti[_lang][19], "I19.als");
	talk_igor_sentado(_texti[_lang][20], "I20.als");
	talk(_text[_lang][281], "281.als");
}

void DrasculaEngine::animation_4_4() {
	talk(_text[_lang][287], "287.als");
	talk_igor_sentado(_texti[_lang][21], "I21.als");
	talk(_text[_lang][284], "284.als");
	talk_igor_sentado(_texti[_lang][22], "I22.als");
	talk(_text[_lang][285], "285.als");
	talk_igor_sentado(_texti[_lang][23], "I23.als");
}

void DrasculaEngine::animation_7_4() {
	Negro();
	talk(_text[_lang][427], "427.als");
	FundeDelNegro(1);
	resta_objeto(8);
	resta_objeto(10);
	resta_objeto(12);
	resta_objeto(16);
	suma_objeto(17);
	flags[30] = 0;
	flags[29] = 0;
}

void DrasculaEngine::animation_1_5() {
	if (flags[0] == 0) {
		talk(_text[_lang][430], "430.als");
		talk_bj(_textbj[_lang][16], "BJ16.als");
		talk_bj(_textbj[_lang][17], "BJ17.als");
		talk_bj(_textbj[_lang][18], "BJ18.als");
		talk(_text[_lang][217], "217.als");
		talk_bj(_textbj[_lang][19], "BJ19.als");
		talk(_text[_lang][229], "229.als");
		pause(5);
		lleva_al_hare(114, 170);
		sentido_hare = 3;
		talk(_text[_lang][431], "431.als");
		talk_bj(_textbj[_lang][20], "BJ20.als");
		sentido_hare = 2;
		pause(4);
		talk(_text[_lang][438], "438.als");
		sitio_x = 120;
		sitio_y = 157;
		anda_a_objeto = 1;
		sentido_final = 1;
		empieza_andar();
		talk_bj(_textbj[_lang][21], "BJ21.als");

		for (;;) {
			if (hare_se_mueve == 0)
				break;
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		}

		sentido_hare = 1;
		talk(_text[_lang][229], "229.als");
		flags[0] = 1;
	}

	sentido_hare = 1;
	conversa("op_8.cal");
}

void DrasculaEngine::animation_2_5() {
	talk_bj(_textbj[_lang][22], "BJ22.als");
}

void DrasculaEngine::animation_3_5() {
	talk_bj(_textbj[_lang][23], "BJ23.als");
	agarra_objeto(10);
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_4_5() {
	flags[7] = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	talk(_text[_lang][228], "228.als");
	talk_lobo(_textl[_lang][1], "L1.als");
	talk_lobo(_textl[_lang][2], "L2.als");
	pause(23);
	talk(_text[_lang][229], "229.als");
	talk_lobo(_textl[_lang][3], "L3.als");
	talk_lobo(_textl[_lang][4], "L4.als");
	talk(_text[_lang][230], "230.als");
	talk_lobo(_textl[_lang][5], "L5.als");
	talk(_text[_lang][231], "231.als");
	talk_lobo(_textl[_lang][6], "L6.als");
	talk_lobo(_textl[_lang][7], "L7.als");
	pause(33);
	talk(_text[_lang][232], "232.als");
	talk_lobo(_textl[_lang][8], "L8.als");
}

void DrasculaEngine::animation_5_5(){
	int h;
	int frame = 0;
	int hueso_x[] = {1, 99, 197, 1, 99, 197, 1, 99, 197};
	int hueso_y[] = {1, 1, 1, 66, 66, 66, 131, 131, 131};
	int vuela_x[] = {1, 63, 125, 187, 249};
	int pixel_x = hare_x - 53, pixel_y = hare_y - 9;

	sin_verbo();
	resta_objeto(8);

	lleva_al_hare(hare_x - 19, hare_y + alto_hare);
	sentido_hare = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	loadPic("3an5_1.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("3an5_2.alg");
	decompressPic(dir_hare_frente, 1);

	for (frame = 0; frame < 9; frame++) {
		pause(3);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(hueso_x[frame], hueso_y[frame], pixel_x, pixel_y, 97, 64, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(pixel_x, pixel_y, pixel_x,pixel_y, 97,64, dir_zona_pantalla);
	}

	copyBackground(52, 161, 198, 81, 26, 24, dir_dibujo3, dir_zona_pantalla);
	updateScreen(198, 81, 198, 81, 26, 24, dir_zona_pantalla);

	for (frame = 0; frame < 9; frame++) {
		pause(3);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(hueso_x[frame], hueso_y[frame], pixel_x, pixel_y, 97, 64, dir_hare_frente, dir_zona_pantalla);
		updateScreen(pixel_x, pixel_y, pixel_x,pixel_y, 97, 64, dir_zona_pantalla);
	}

	flags[6] = 1;
	actualiza_datos();
	pause(12);

	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	for (h = 0; h < (200 - 18); h++)
		copyBackground(0, 53, 0, h, 320, 19, dir_hare_frente, dir_zona_pantalla);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	loadPic("101.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("3an5_3.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("3an5_4.alg");
	decompressPic(dir_hare_dch, 1);

	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo1);
	pause(9);
	for (frame = 0; frame < 5; frame++) {
		pause(3);
		copyBackground(vuela_x[frame], 1, 174, 79, 61, 109, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(174, 79, 174, 79, 61, 109, dir_zona_pantalla);
	}
	for (frame = 0; frame < 5; frame++) {
		pause(3);
		copyBackground(vuela_x[frame], 1, 174, 79, 61, 109, dir_hare_dch, dir_zona_pantalla);
		updateScreen(174, 79, 174, 79, 61, 109, dir_zona_pantalla);
	}
	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo1);

	comienza_sound("s1.als");
	fin_sound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);

	clearRoom();

	loadPic("49.alg");
	decompressPic(dir_dibujo1, MEDIA);
}

void DrasculaEngine::animation_6_5() {
	talk_lobo(_textl[_lang][9], "L9.als");
	talk(_text[_lang][234], "234.als");
}

void DrasculaEngine::animation_7_5() {
	talk_lobo(_textl[_lang][10], "L10.als");
	talk(_text[_lang][236], "236.als");
	talk_lobo(_textl[_lang][11], "L11.als");
	talk_lobo(_textl[_lang][12], "L12.als");
	talk_lobo(_textl[_lang][13], "L13.als");
	pause(34);
	talk_lobo(_textl[_lang][14], "L14.als");
}

void DrasculaEngine::animation_8_5() {
	talk_lobo(_textl[_lang][15], "L15.als");
	talk(_text[_lang][238], "238.als");
	talk_lobo(_textl[_lang][16], "L16.als");
}

void DrasculaEngine::animation_9_5() {
	flags[4] = 1;
	talk(_text[_lang][401], "401.als");
	sin_verbo();
	resta_objeto(15);
}

void DrasculaEngine::animation_10_5() {
	flags[3] = 1;
	talk(_text[_lang][401], "401.als");
	sin_verbo();
	resta_objeto(12);
}

void DrasculaEngine::animation_11_5() {
	flags[9] = 1;
	if (flags[2] == 1 && flags[3] == 1 && flags[4] == 1)
		animation_12_5();
	else {
		flags[9] = 0;
		talk(_text[_lang][33], "33.als");
	}
}

void DrasculaEngine::animation_12_5() {
	DacPalette256 palFondo1;
	DacPalette256 palFondo2;
	DacPalette256 palFondo3;

	int frame;
	const int rayo_x[] = {1, 46, 91, 136, 181, 226, 271, 181};
	const int frusky_x[] = {100, 139, 178, 217, 100, 178, 217, 139, 100, 139};
	const int elfrusky_x[] = {1, 68, 135, 1, 68, 135, 1, 68, 135, 68, 1, 135, 68, 135, 68};
	//const int humo_x[] = {1, 29, 57, 85, 113, 141, 169, 197, 225};
	int color, componente;
	char fundido;

	playMusic(26);
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(27);
	anima("rayo1.bin", 23);
	comienza_sound("s5.als");
	anima("rayo2.bin", 17);
	sentido_hare = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	hare_oscuro();

	for (color = 0; color < 255; color++)
		for (componente = 0; componente < 3; componente++) {
			palFondo1[color][componente] = palJuego[color][componente];
			palFondo2[color][componente] = palJuego[color][componente];
			palFondo3[color][componente] = palJuego[color][componente];
		}

	for (fundido = 1; fundido >= 0; fundido--) {
		for (color = 0; color < 128; color++)
			for (componente = 0; componente < 3; componente++)
				palFondo1[color][componente] = LimitaVGA(palFondo1[color][componente] - 8 + fundido);
	}

	for (fundido = 2; fundido >= 0; fundido--) {
		for (color = 0; color < 128; color++)
			for (componente = 0; componente < 3; componente++)
				palFondo2[color][componente] = LimitaVGA(palFondo2[color][componente] - 8 + fundido);
	}

	for (fundido = 3; fundido >= 0; fundido--) {
		for (color = 0; color < 128; color++)
			for (componente = 0; componente < 3; componente++)
				palFondo3[color][componente] = LimitaVGA(palFondo3[color][componente] - 8 + fundido);
	}

	loadPic("3an11_1.alg");
	decompressPic(dir_hare_fondo, 1);

	for (frame = 0; frame < 8; frame++) {
		if (frame == 2 || frame == 4 || frame == 8 || frame==10)
			setvgapalette256((byte *)&palFondo1);
		else if (frame == 1 || frame == 5 || frame == 7 || frame == 9)
			setvgapalette256((byte *)&palFondo2);
		else
			setvgapalette256((byte *)&palFondo3);

		pause(4);
		updateRoom();
		copyRect(rayo_x[frame], 1, 41, 0, 44, 44, dir_hare_fondo, dir_zona_pantalla);
		copyRect(frusky_x[frame], 113, 205, 50, 38, 86, dir_dibujo3, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	fin_sound_corte();

	for (frame = 0; frame < 15; frame++) {
		if (frame == 2 || frame == 4 || frame == 7 || frame == 9)
			setvgapalette256((byte *)&palFondo1);
		else if (frame == 1 || frame == 5)
			setvgapalette256((byte *)&palJuego);
		else
			setvgapalette256((byte *)&palFondo2);

		pause(4);
		updateRoom();
		copyRect(elfrusky_x[frame], 47, 192, 39, 66, 106, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	anima("frel.bin", 16);
	clearRoom();
	hare_claro();
	ActualizaPaleta();

	flags[1] = 1;

	animation_13_5();
	comienza_sound("s1.als");
	hipo(12);
	fin_sound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);

	lleva_al_hare(40, 169);
	lleva_al_hare(-14, 175);

	rompo = 1;
	musica_antes = musica_room;
	hare_se_ve = 1;
	clearRoom();
	sentido_hare = 1;
	hare_se_mueve = 0;
	hare_x = -1;
	obj_saliendo = 104;
	sin_verbo();
	carga_escoba("57.ald");
}

void DrasculaEngine::animation_13_5() {
	int frank_x = 199;
	int frame = 0;
	int frus_x[] = {1, 46, 91, 136, 181, 226, 271};
	int frus_y[] = {1, 1, 1, 1, 1, 1, 1, 89};
	int pos_frusky[6];

	loadPic("auxfr.alg");
	decompressPic(dir_hare_fondo, 1);

	pos_frusky[3] = 81;
	pos_frusky[4] = 44;
	pos_frusky[5] = 87;
	pos_frusky[0] = 1;
	pos_frusky[1] = 1;
	pos_frusky[2] = frank_x;
	updateRoom();
	copyRectClip(pos_frusky, dir_hare_fondo, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(15);

	playMusic(18);

	for (;;) {
		updateRoom();
		pos_frusky[0] = frus_x[frame];
		pos_frusky[1] = frus_y[frame];
		pos_frusky[2] = frank_x;
		copyRectClip( pos_frusky, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		frank_x = frank_x - 5;
		frame++;
		if (frank_x <= -45)
			break;
		if (frame == 7) {
			frame = 0;
			sentido_hare = 3;
		}
		pause(6);
	}
}

void DrasculaEngine::animation_14_5() {
	flags[11] = 1;
	comienza_sound("s3.als");
	updateRoom();
	updateScreen(0, 0, 0,0 , 320, 200, dir_zona_pantalla);
	fin_sound();
	pause(17);
	sentido_hare = 3;
	talk(_text[_lang][246],"246.als");
	lleva_al_hare(89, 160);
	flags[10] = 1;
	comienza_sound("s7.als");
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	pause(14);
	sentido_hare = 3;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	talk_solo(_textd[_lang][18], "d18.als");
	FundeAlNegro(1);
}

void DrasculaEngine::animation_15_5() {
	talk_mus(_texte[_lang][4], "E4.als");
	talk_mus(_texte[_lang][5], "E5.als");
	talk_mus(_texte[_lang][6], "E6.als");
	talk(_text[_lang][291], "291.als");
	talk_mus(_texte[_lang][7], "E7.als");
}

void DrasculaEngine::animation_16_5() {
	talk_mus(_texte[_lang][8], "E8.als");
}

void DrasculaEngine::animation_17_5() {
	talk_mus(_texte[_lang][9], "E9.als");
}

void DrasculaEngine::animation_1_6() {
	int l;

	sentido_hare = 0;
	hare_x = 103;
	hare_y = 108;
	flags[0] = 1;
	for (l = 0; l < 200; l++)
		factor_red[l] = 98;

	loadPic("auxig2.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("auxdr.alg");
	decompressPic(dir_dibujo2, 1);
	loadPic("car.alg");
	decompressPic(dir_hare_fondo, 1);
	talk_dr_dch(_textd[_lang][19], "D19.als");
	talk(_text[_lang][247], "247.als");
	talk_dr_dch(_textd[_lang][20], "d20.als");
	talk_dr_dch(_textd[_lang][21], "d21.als");
	talk(_text[_lang][248], "248.als");
	talk_dr_dch(_textd[_lang][22], "d22.als");
	talk(_text[_lang][249], "249.als");
	talk_dr_dch(_textd[_lang][23], "d23.als");
	conversa("op_11.cal");
	talk_dr_dch(_textd[_lang][26], "d26.als");

	anima("fum.bin", 15);

	talk_dr_dch(_textd[_lang][27], "d27.als");
	talk(_text[_lang][254], "254.als");
	talk_dr_dch(_textd[_lang][28], "d28.als");
	talk(_text[_lang][255], "255.als");
	talk_dr_dch(_textd[_lang][29], "d29.als");
	FundeAlNegro(1);
	clearRoom();
	loadPic("time1.alg");
	decompressPic(dir_zona_pantalla, 1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(930);
	clearRoom();
	Negro();
	hare_se_ve = 0;
	flags[0] = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(1);
	talk(_text[_lang][256], "256.als");
	talk_dr_dch(_textd[_lang][30], "d30.als");
	talk(_text[_lang][257], "257.als");
	FundeAlNegro(0);
	clearRoom();
	loadPic("time1.alg");
	decompressPic(dir_zona_pantalla,1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(900);
	clearRoom();
	Negro();
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(1);
	talk(_text[_lang][258], "258.als");
	talk_dr_dch(_textd[_lang][31], "d31.als");
	animation_5_6();
	talk_dr_dch(_textd[_lang][32], "d32.als");
	talk_igor_dch(_texti[_lang][11], "I11.als");
	sentido_igor = 3;
	talk_dr_dch(_textd[_lang][33], "d33.als");
	talk_igor_frente(_texti[_lang][12], "I12.als");
	talk_dr_dch(_textd[_lang][34], "d34.als");
	sentido_dr = 0;
	talk_dr_izq(_textd[_lang][35], "d35.als");
	clearRoom();
	carga_escoba("102.ald");
	activa_pendulo();
}

void DrasculaEngine::animation_2_6() {
	talk_dr_dch(_textd[_lang][24], "d24.als");
}

void DrasculaEngine::animation_3_6() {
	talk_dr_dch(_textd[_lang][24], "d24.als");
}

void DrasculaEngine::animation_4_6() {
	talk_dr_dch(_textd[_lang][25], "d25.als");
}

void DrasculaEngine::animation_5_6() {
	int n, pos_pen[6];

	pos_pen[0] = 1;
	pos_pen[1] = 29;
	pos_pen[2] = 204;
	pos_pen[3] = -125;
	pos_pen[4] = 18;
	pos_pen[5] = 125;

	anima("man.bin", 14);

	for (n = -125; n <= 0; n = n + 2) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		pos_pen[3] = n;
		copyRectClip(pos_pen, dir_dibujo3, dir_zona_pantalla);

		updateRefresh();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(2);
	}

	flags[3] = 1;
}

void DrasculaEngine::animation_6_6() {
	anima("rct.bin", 11);
	clearRoom();
	sin_verbo();
	resta_objeto(20);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	rompo = 1;
	obj_saliendo = 104;
	hare_x = -1;
	sin_verbo();
	carga_escoba("58.ald");
	hare_se_ve = 1;
	sentido_hare = 1;
	anima("hbp.bin", 14);

	sentido_hare = 3;
	flags[0] = 1;
	flags[1] = 0;
	flags[2] = 1;
}

void DrasculaEngine::animation_7_6() {
	flags[8] = 1;
	actualiza_datos();
}

void DrasculaEngine::animation_9_6() {
	int v_cd;

	anima("fin.bin", 14);
	playMusic(13);
	flags[5] = 1;
	anima("drf.bin", 16);
	FundeAlNegro(0);
	clearRoom();
	hare_x = -1;
	obj_saliendo = 108;
	carga_escoba("59.ald");
	strcpy(num_room, "nada.alg");
	loadPic("nota2.alg");
	decompressPic(dir_dibujo1, MEDIA);
	Negro();
	sentido_hare = 1;
	hare_x -= 21;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	pause(96);
	lleva_al_hare(116, 178);
	sentido_hare = 2;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	playMusic(9);
	clearRoom();
	loadPic("nota.alg");
	decompressPic(dir_dibujo1, COMPLETA);
	color_abc(WHITE);
	talk_solo(_textbj[_lang][24], "bj24.als");
	talk_solo(_textbj[_lang][25], "bj25.als");
	talk_solo(_textbj[_lang][26], "bj26.als");
	talk_solo(_textbj[_lang][27], "bj27.als");
	talk_solo(_textbj[_lang][28], "bj28.als");
	sentido_hare = 3;
	clearRoom();
	loadPic("96.alg");
	decompressPic(dir_hare_frente, COMPLETA);
	loadPic("nota2.alg");
	decompressPic(dir_dibujo1, MEDIA);
	talk(_text[_lang][296], "296.als");
	talk(_text[_lang][297], "297.als");
	talk(_text[_lang][298], "298.als");
	sentido_hare = 1;
	talk(_text[_lang][299], "299.als");
	talk(_text[_lang][300], "300.als");
	updateRoom();
	copyBackground(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	color_abc(LIGHT_GREEN);
	talk_solo("GOOOOOOOOOOOOOOOL", "s15.als");
	loadPic("nota2.alg");
	decompressPic(dir_dibujo1, 1);
	sentido_hare = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	talk(_text[_lang][301], "301.als");
	v_cd = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
	v_cd = v_cd + 4;
	playMusic(17);
	FundeAlNegro(1);
	clearRoom();
	fliplay("qpc.bin", 1);
	MusicFadeout();
	stopMusic();
	clearRoom();
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, v_cd * 16);
	playMusic(3);
	fliplay("crd.bin", 1);
	stopMusic();
}

void DrasculaEngine::animation_10_6() {
	comienza_sound ("s14.als");
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();
	copyBackground(164, 85, 155, 48, 113, 114, dir_dibujo3, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	talk_taber2(_textt[_lang][23], "t23.als");
	flags[7] = 1;
}

void DrasculaEngine::animation_11_6() {
	talk_taber2(_textt[_lang][10], "t10.als");
	talk(_text[_lang][268], "268.als");
	talk_taber2(_textt[_lang][11], "t11.als");
}

void DrasculaEngine::animation_12_6() {
	talk_taber2(_textt[_lang][12], "t12.als");
	talk(_text[_lang][270], "270.als");
	talk_taber2(_textt[_lang][13], "t13.als");
	talk_taber2(_textt[_lang][14], "t14.als");
}

void DrasculaEngine::animation_13_6() {
	talk_taber2(_textt[_lang][15], "t15.als");
}

void DrasculaEngine::animation_14_6() {
	talk_taber2(_textt[_lang][24], "t24.als");
	suma_objeto(21);
	flags[10] = 1;
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_15_6() {
	talk_taber2(_textt[_lang][16], "t16.als");
}

void DrasculaEngine::animation_18_6() {
	flags[6] = 1;
	sin_verbo();
	resta_objeto(21);
	anima("beb.bin", 10);
}

void DrasculaEngine::animation_19_6() {
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	copyBackground(140, 23, 161, 69, 35, 80, dir_dibujo3, dir_zona_pantalla);

	updateRefresh_pre();
	pon_hare();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(6);
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	comienza_sound("s4.als");
	pause(6);
	fin_sound();
}

void DrasculaEngine::animation_12_2() {
	loadPic("an12.alg");
	decompressPic(dir_hare_dch, 1);

	talk(_text[_lang][356], "356.als");

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	updateScreen(228, 112, 228, 112, 47, 60, dir_zona_pantalla);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianista(_textp[_lang][5], "P5.als");
	conversa("op_1.cal");

	flags[11] = 0;
	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_26_2() {
	int n, x = 0;

	loadPic("an12.alg");
	decompressPic(dir_hare_dch, 1);

	talk(_text[_lang][392], "392.als");

	updateRefresh_pre();

	copyBackground(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	updateScreen(228, 112, 228, 112, 47, 60, dir_zona_pantalla);

	pause(3);

	updateRefresh_pre();

	copyBackground(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);
	stopMusic();
	flags[11] = 1;

	talk_pianista(_textp[_lang][5], "P5.als");
	talk(_text[_lang][393], "393.als");
	talk_pianista(_textp[_lang][17], "P17.als");
	talk_pianista(_textp[_lang][18], "P18.als");
	talk_pianista(_textp[_lang][19], "P19.als");

	loadPic("an26.alg");
	decompressPic(dir_hare_dch, 1);
	for (n = 0; n < 6; n++){
		x++;
		copyBackground(x, 1, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225,113, 225,113, 50,59, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 61, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225, 113, 225, 113, 50, 59, dir_zona_pantalla);
		x = x + 50;
		if (n == 2)
			comienza_sound("s9.als");
		pause(3);
	}

	fin_sound_corte();
	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 121, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225, 113, 225, 113, 50, 59, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	agarra_objeto(11);
	resta_objeto(12);

	flags[11] = 0;
	flags[39] = 1;
	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
	musica_room = 16;
}

void DrasculaEngine::animation_11_2() {
	loadPic("an11y13.alg");
	decompressPic(dir_hare_dch, 1);

	talk(_text[_lang][352], "352.als");
	talk_tabernero(_textt[_lang][1], "T1.als");
	talk(_text[_lang][353], "353.als");
	talk_tabernero(_textt[_lang][17], "T17.als");
	talk(_text[_lang][354], "354.als");
	talk_tabernero(_textt[_lang][18], "T18.als");
	talk(_text[_lang][355], "355.als");
	pause(40);
	talk_tabernero("No, nada", "d82.als");

	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_13_2() {
	loadPic("an11y13.alg");
	decompressPic(dir_hare_frente, 1);

	if (flags[41] == 0) {
		talk(_text[_lang][103], "103.als");
		talk_borracho(_textb[_lang][4], "B4.als");
		flags[12] = 1;
		talk(_text[_lang][367], "367.als");
		talk_borracho(_textb[_lang][5], "B5.als");
		flags[12] = 1;
		talk(_text[_lang][368], "368.als");
		talk_borracho(_textb[_lang][6], "B6.als");
		talk_borracho(_textb[_lang][7], "B7.als");
		flags[41] = 1;
	}
	conversa("op_2.cal");

	loadPic("964.alg");
	decompressPic(dir_hare_frente, 1);
}

void DrasculaEngine::animation_18_2() {
	talk(_text[_lang][378], "378.als");
	talk_vbpuerta(_textvb[_lang][4], "VB4.als");
	conversa("op_3.cal");
}

void DrasculaEngine::animation_22_2() {
	talk(_text[_lang][374],"374.als");

	sentido_hare=2;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	comienza_sound("s13.als");
	fin_sound();
	sentido_hare = 1;

	talk_vbpuerta(_textvb[_lang][1], "VB1.als");
	talk(_text[_lang][375], "375.als");
	talk_vbpuerta(_textvb[_lang][2], "VB2.als");
	talk(_text[_lang][376], "376.als");
	talk_vbpuerta(_textvb[_lang][3], "VB3.als");

	flags[18] = 1;
}

void DrasculaEngine::animation_24_2() {
	if (hare_x < 178)
		lleva_al_hare(208, 136);
	sentido_hare = 3;
	updateRoom();
	pause(3);
	sentido_hare = 0;

	talk(_text[_lang][356], "356.als");

	loadPic("an24.alg");
	decompressPic(dir_hare_frente, 1);

	animation_32_2();

	flags[21] = 1;

	talk_vb(_textvb[_lang][22], "VB22.als");

	if (flags[22] == 0)
		conversa("op_4.cal");
	else
		conversa("op_5.cal");

	sal_de_la_habitacion(0);
	flags[21] = 0;
	flags[24] = 0;
	sentido_vb = 1;
	vb_x = 120;
}

void DrasculaEngine::animation_32_2() {
	int n, x = 0;

	loadPic("an32_1.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("an32_2.alg");
	decompressPic(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(x, 1, 113, 53, 65, 81, dir_dibujo3, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		pause(4);
	}

	x = 0;
	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(x, 83, 113, 53, 65, 81, dir_dibujo3, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		pause(4);
	}

	x = 0;
	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(x, 1, 113, 53, 65, 81, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		pause(4);
	}

	x = 0;
	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 83, 113, 53, 65, 81, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		if (n < 2)
			pause(4);
	}

	loadPic("aux18.alg");
	decompressPic(dir_dibujo3, 1);
}

void DrasculaEngine::animation_34_2() {
	int n, x = 0;

	sentido_hare = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	loadPic("an34_1.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an34_2.alg");
	decompressPic(dir_hare_dch, 1);

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 1, 218, 79, 83, 75, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83, 75, dir_zona_pantalla);
		x = x + 83;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 77, 218, 79, 83, 75, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83, 75, dir_zona_pantalla);
		x = x + 83;
		pause(3);
	}

	x = 0;
	comienza_sound("s8.als");

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 1, 218, 79, 83, 75,	dir_hare_dch, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83,75, dir_zona_pantalla);
		x = x + 83;
		pause(3);
	}
	fin_sound();

	pause(30);

	copyBackground(1, 77, 218, 79, 83, 75, dir_hare_dch, dir_zona_pantalla);
	updateScreen(218, 79, 218, 79, 83, 75, dir_zona_pantalla);
	pause(3);

	loadPic("994.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_36_2() {
	loadPic("an11y13.alg");
	decompressPic(dir_hare_dch, 1);

	talk(_text[_lang][404], "404.als");
	talk_tabernero(_textt[_lang][19], "T19.als");
	talk_tabernero(_textt[_lang][20], "T20.als");
	talk_tabernero(_textt[_lang][21], "T21.als");
	talk(_text[_lang][355], "355.als");
	pause(40);
	talk_tabernero("No, nada", "d82.als");

	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_7_2() {
	int n, x = 0;

	loadPic("an7_1.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an7_2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("an7_3.alg");
	decompressPic(dir_hare_frente, 1);

	if (flags[3] == 1)
		copyBackground(258, 110, 85, 44, 23, 53, dir_dibujo3, dir_dibujo1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64,	51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}
	loadPic("an7_4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an7_5.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("an7_6.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("an7_7.alg");
	decompressPic(dir_dibujo3, 1);

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1,	80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1,	80, 64, 51,73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64,	51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 2; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}
	loadPic("an7_8.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an7_9.alg");
	decompressPic(dir_hare_dch, 1);

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64,	51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73,	dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pause(3);
	}


	copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
	copyRect(1, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
	updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);

	flags[37] = 1;

	if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
		flags[38] = 1;

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("aux3.alg");
	decompressPic(dir_dibujo3, 1);
}

void DrasculaEngine::animation_5_2() {
	int n, x = 0;

	sentido_hare = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	loadPic("an5_1.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("an5_2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("an5_3.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("an5_4.alg");
	decompressPic(dir_dibujo3, 1);

	copyBackground(1, 1, 213, 66,	53,84, dir_hare_fondo, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66, 53, 84, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84,dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66, 53, 84, dir_hare_dch, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66,	53, 84, dir_hare_dch, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_dch, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66,	53, 84, dir_hare_frente, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	comienza_sound("s1.als");

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_frente, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}
	fin_sound_corte();

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66, 53, 84,	dir_dibujo3, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}

	loadPic("994.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("964.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("aux5.alg");
	decompressPic(dir_dibujo3, 1);
	flags[8] = 1;
	hare_x = hare_x - 4;
	talk_sinc(_text[_lang][46], "46.als", "4442444244244");
	sin_verbo();
}

void DrasculaEngine::animation_6_2() {
	stopMusic();
	flags[9] = 1;

	clearRoom();
	loadPic("ciego1.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("ciego2.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("ciego3.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("ciego4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("ciego5.alg");
	decompressPic(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(1);

	if (flags[4] == 1)
		talk_hacker(_textd[_lang][66], "d66.als");
	pause(6);
	talk_ciego(_textd[_lang][78], "d78.als",
				"004447222550002222755554444466222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777");
	pause(4);
	talk_hacker(_textd[_lang][67], "d67.als");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(10);

	clearRoom();

	playMusic(musica_room);
	loadPic("9.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("aux9.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	sin_verbo();

	flags[9] = 0;
}

void DrasculaEngine::animation_33_2() {
	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(_textd[_lang][56], "d56.als" );
	pause(8);

	clearRoom();
	loadPic("ciego1.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("ciego2.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("ciego3.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("ciego4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("ciego5.alg");
	decompressPic(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(10);

	talk_ciego(_textd[_lang][68], "d68.als", "44472225500022227555544444472225500022227555544444664466");
	pause(5);
	talk_hacker(_textd[_lang][57], "d57.als");
	pause(6);
	_system->delayMillis(1000);
	talk_ciego(_textd[_lang][77], "d77.als", "56665004444447222550002222755554444466555033022755555000444444444444444444444444444444");
	talk_hacker(_textd[_lang][65], "d65.als");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(14);

	clearRoom();

	playMusic(musica_room);
	loadPic("9.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("aux9.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	sin_verbo();

	flags[33] = 1;
	flags[9] = 0;
}

void DrasculaEngine::animation_1_4() {
	if (flags[21] == 0) {
		strcpy(nombre_obj[2], "igor");
		talk(_text[_lang][275], "275.als");

		updateRefresh_pre();

		copyBackground(131, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		updateScreen(199, 95, 199, 95, 50, 66, dir_zona_pantalla);

		pause(3);

		updateRefresh_pre();

		copyBackground(182, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		pon_hare();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		pause(3);
		flags[18] = 1;
		flags[20] = 1;

		talk_igor_sentado(_texti[_lang][13], "I13.als");
		talk_igor_sentado(_texti[_lang][14], "I14.als");
		talk_igor_sentado(_texti[_lang][15], "I15.als");
		flags[21] = 1;
	} else {
		talk(_text[_lang][356], "356.als");

		updateRefresh_pre();

		copyBackground(131, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		updateScreen(199, 95, 199, 95, 50, 66, dir_zona_pantalla);
		pause(2);

		updateRefresh_pre();

		copyBackground(182, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		pon_hare();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		flags[18] = 1;
		flags[20] = 1;

		talk(_text[_lang][276], "276.als");
		pause(14);
		talk_igor_sentado(_texti[_lang][6], "I6.als");
	}

	conversa("op_6.cal");
	flags[20] = 0;
	flags[18] = 0;
}

void DrasculaEngine::animation_5_4(){
	sentido_hare = 3;
	loadPic("anh_dr.alg");
	decompressPic(dir_hare_fondo, 1);
	lleva_al_hare(99, 160);
	lleva_al_hare(38, 177);
	hare_se_ve = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(800);
	anima("bio.bin", 14);
	flags[29] = 1;
	hare_x = 95;
	hare_y = 82;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	abre_puerta(2, 0);
	loadPic("auxigor.alg");
	decompressPic(dir_hare_frente, 1);
	x_igor = 100;
	y_igor = 65;
	talk_igor_frente(_texti[_lang][29], "I29.ALS");
	talk_igor_frente(_texti[_lang][30], "I30.als");
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	hare_se_ve = 1;
	FundeAlNegro(0);
	sal_de_la_habitacion(0);
}

void DrasculaEngine::animation_6_4() {
	char room[13];

	strcpy(room, num_room);
	strcpy(num_room, "26.alg");
	clearRoom();
	loadPic("26.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("aux26.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("auxigor.alg");
	decompressPic(dir_hare_frente, 1);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	update_26_pre();
	x_igor = 104;
	y_igor = 71;
	pon_igor();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(40);
	talk_igor_frente(_texti[_lang][26], "I26.als");
	strcpy(num_room, room);
	clearRoom();
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic(roomDisk);
	decompressPic(dir_dibujo3, 1);
	loadPic(num_room);
	decompressPic(dir_dibujo1, MEDIA);
	sin_verbo();
	updateRoom();
}

void DrasculaEngine::animation_8_4() {
	int frame;
	int estanteria_x[] = {1, 75, 149, 223, 1, 75, 149, 223, 149, 223, 149, 223, 149, 223};
	int estanteria_y[] = {1, 1, 1, 1, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74};

	loadPic("an_8.alg");
	decompressPic(dir_hare_frente, 1);

	for (frame = 0; frame < 14; frame++) {
		pause(2);
		copyBackground(estanteria_x[frame], estanteria_y[frame], 77, 45, 73, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(77, 45, 77, 45, 73, 72, dir_zona_pantalla);
	}

	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	abre_puerta(7, 2);
}

void DrasculaEngine::animation_9_4() {
	anima("st.bin", 14);
	FundeAlNegro(1);
}


} // End of namespace Drascula
