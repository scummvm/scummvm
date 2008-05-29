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
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(600);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		delay(340);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		playMusic(26);
		delay(500);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		fliplay("logoalc.bin", 8);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("cielo.alg");
		decompressPic(dir_zona_pantalla, 256);
		black();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fadeFromBlack(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(900);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		color_abc(RED);
		centerText(_textmisc[_lang][1], 160, 100);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1000);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1200);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		fliplay("scrollb.bin", 9);

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		playSound("s5.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (anima("scr2.bin", 17))
			break;
		stopSound_corte();
		if (anima("scr3.bin", 17))
			break;
		loadPic("cielo2.alg");
		decompressPic(dir_zona_pantalla, 256);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		fadeToBlack(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		loadPic("96.alg");
		decompressPic(dir_hare_frente, COMPLETE_PAL);
		loadPic("103.alg");
		decompressPic(dir_dibujo1, HALF_PAL);
		loadPic("104.alg");
		decompressPic(dir_dibujo3, 1);
		loadPic("aux104.alg");
		decompressPic(dir_dibujo2, 1);

		playMusic(4);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		delay(400);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		for (l2 = 0; l2 < 3; l2++)
			for (l = 0; l < 7; l++) {
				copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
				copyBackground(interf_x[l], interf_y[l], 156, 45, 63, 31, dir_dibujo2, dir_zona_pantalla);
				updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
				if (getScan() == Common::KEYCODE_ESCAPE) {
					term_int = 1;
					break;
				}
				pause(3);
			}
			if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
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
			if (getScan() == Common::KEYCODE_ESCAPE) {
				term_int = 1;
				break;
			}
		}
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);

		talk_dr_grande(_textd[_lang][1], "D1.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		clearRoom();

		loadPic("100.alg");
		decompressPic(dir_dibujo1, HALF_PAL);
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
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_igor_dch(8);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_dr_izq(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(3);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (anima("lib.bin", 16))
			break;
		if (anima("lib2.bin", 16))
			break;
		clearRoom();
		color_solo = RED;
		loadPic("plan1.alg");
		decompressPic(dir_zona_pantalla, HALF_PAL);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(10);
		talk_solo(_textd[_lang][4],"d4.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("plan1.alg");
		decompressPic(dir_zona_pantalla, HALF_PAL);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_solo(_textd[_lang][5], "d5.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (anima("lib2.bin", 16))
			break;
		clearRoom();
		loadPic("plan2.alg");
		decompressPic(dir_zona_pantalla, HALF_PAL);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(20);
		talk_solo(_textd[_lang][6], "d6.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (anima("lib2.bin", 16))
			break;
		clearRoom();
		loadPic("plan3.alg");
		decompressPic(dir_zona_pantalla, HALF_PAL);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(20);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_textd[_lang][7], "d7.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("plan3.alg");
		decompressPic(dir_zona_pantalla, HALF_PAL);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_solo(_textd[_lang][8], "d8.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("100.alg");
		decompressPic(dir_dibujo1, HALF_PAL);
		MusicFadeout();
		stopMusic();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(9);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(9);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(10);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		playMusic(11);
		talk_dr_izq(10);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (anima("rayo1.bin", 16))
			break;
		playSound("s5.als");
		if (anima("rayo2.bin", 15))
			break;
		if (anima("frel2.bin", 16))
			break;
		if (anima("frel.bin", 16))
			break;
		if (anima("frel.bin", 16))
			break;
		stopSound_corte();
		clearRoom();
		black();
		playMusic(23);
		fadeFromBlack(0);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 1;
		talk_igor_dch(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_dch(11);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(1);
		sentido_dr = 0;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_dr_izq(12);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(1);
		sentido_dr = 1;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_igor_dch(2);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(13);
		talk_dr_dch(13);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(1);
		sentido_dr = 0;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		placeIgor();
		placeDrascula();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk_dr_izq(14);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(3);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(15);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(4);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_dr_izq(16);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_dch(5);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_igor = 3;
		talk_dr_izq(17);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(18);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_igor_frente(_texti[_lang][6], "I6.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		fadeToBlack(0);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		playMusic(2);
		pause(5);
		fliplay("intro.bin", 12);
		term_int = 1;
	}
	clearRoom();
	loadPic("96.alg");
	decompressPic(dir_hare_frente, COMPLETE_PAL);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::talk_dr_grande(const char *said, const char *filename) {
	int x_talk[4] = {47, 93, 139, 185};
	int cara;
	int l = 0;
	int length = strlen(said);

	_rnd->setSeed((unsigned int)_system->getMillis() / 2);

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

	if (withVoices == 0)
		centerText(said, 191, 69);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(3);

	byte key = getScan();
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
		length -= 2;
		if (length > 0)
			goto bucless;
	}
}

void DrasculaEngine::animation_2_1() {
	int l;

	lleva_al_hare(231, 91);
	hare_se_ve = 0;

	term_int = 0;

	for (;;) {
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		if (anima("ag.bin", 14))
			break;

		if (_lang == kSpanish)
			dir_texto = dir_hare_frente;

		loadPic("an11y13.alg");
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		talk_tabernero(22);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		if (_lang == kSpanish)
			dir_texto = dir_hare_dch;

		loadPic("97.alg");
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		pause(4);
		playSound("s1.als");
		hiccup(18);
		stopSound();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		clearRoom();
		stopMusic();
		musicStopped = 1;
		memset(dir_zona_pantalla, 0, 64000);
		color_solo = WHITE;
		pause(80);

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_textbj[_lang][1], "BJ1.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();
		loadPic("bj.alg");
		decompressPic(dir_zona_pantalla, HALF_PAL);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		black();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fadeFromBlack(1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = YELLOW;
		talk_solo(_text[_lang][214], "214.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		clearRoom();

		loadPic("16.alg");
		decompressPic(dir_dibujo1, HALF_PAL);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		loadPic("auxbj.alg");
		decompressPic(dir_dibujo3, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		roomNumber = 16;

		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
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
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		loadPic("97g.alg");
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;

		if (anima("lev.bin", 15))
			break;

		lleva_al_hare(100 + ancho_hare / 2, 99 + alto_hare);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		hare_x = 100;
		hare_y = 95;

		talk_bj(2);
		talk(215);
		talk_bj(3);
		talk(216);
		talk_bj(4);
		talk_bj(5);
		talk_bj(6);
		talk(217);
		talk_bj(7);
		talk(218);
		talk_bj(8);
		talk(219);
		talk_bj(9);
		talk(220);
		talk(221);
		talk_bj(10);
		talk(222);
		if (anima("gaf.bin", 15))
			break;
		if (anima("bjb.bin", 14))
			break;
		playMusic(9);
		loadPic("97.alg");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		decompressPic(dir_hare_dch, 1);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(120);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_solo(_text[_lang][223], "223.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = WHITE;
		updateRoom();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(110);
		talk_solo(_textbj[_lang][11], "BJ11.als");
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(118);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		lleva_al_hare(132, 97 + alto_hare);
		pause(60);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk(224);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk_bj(12);
		lleva_al_hare(157, 98 + alto_hare);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		if (anima("bes.bin", 16))
			break;
		playMusic(11);
		if (anima("rap.bin", 16))
			break;
		sentido_hare = 3;
		// The room number was originally changed here to "no_bj.alg",
		// which doesn't exist. In reality, this was just a hack to
		// set the room number to a non-existant one, so that BJ does
		// not appear again when the room is refreshed after the
		// animation where Count Drascula abducts her. We set the
		// room number to -1 for the same purpose
		// Also check animation_9_6(), where the same hack was used by
		// the original
		roomNumber = -1;
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		pause(8);
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk(225);
		pause(76);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk(226);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(30);
		if ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE))
			break;
		talk(227);
		fadeToBlack(0);
		break;
	}
}

void DrasculaEngine::animation_3_1() {
	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

	loadPic("an11y13.alg");
	decompressPic(dir_hare_dch, 1);

	talk(192);
	talk_tabernero(1);
	talk(193);
	talk_tabernero(2);
	talk(194);
	talk_tabernero(3);
	talk(195);
	talk_tabernero(4);
	talk(196);
	talk_tabernero(5);
	talk_tabernero(6);
	talk(197);
	talk_tabernero(7);
	talk(198);
	talk_tabernero(8);
	talk(199);
	talk_tabernero(9);
	talk(200);
	talk(201);
	talk(202);

	flags[0] = 1;

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_4_1() {
	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

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

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

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
	playSound("s2.als");

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

	stopSound();

	pause (4);

	playSound("s1.als");
	hipo_sin_nadie(12);
	stopSound();
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
	loadPic("blind1.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("blind2.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("blind3.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("blind4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("blind5.alg");
	decompressPic(dir_hare_frente, 1);

	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(10);

	talk_blind(_textd[_lang][68], "d68.als", _textd1[_lang][68 - TEXTD_START]);
	pause(5);
	talk_hacker(_textd[_lang][57], "d57.als");
	pause(6);
	talk_blind(_textd[_lang][69],"d69.als", _textd1[_lang][69 - TEXTD_START]);
	pause(4);
	talk_hacker(_textd[_lang][58],"d58.als");
	talk_blind(_textd[_lang][70],"d70.als", _textd1[_lang][70 - TEXTD_START]);
	delay(14);
	talk_hacker(_textd[_lang][59],"d59.als");
	talk_blind(_textd[_lang][71],"d71.als", _textd1[_lang][71 - TEXTD_START]);
	talk_hacker(_textd[_lang][60],"d60.als");
	talk_blind(_textd[_lang][72],"d72.als", _textd1[_lang][72 - TEXTD_START]);
	talk_hacker(_textd[_lang][61],"d61.als");
	talk_blind(_textd[_lang][73],"d73.als", _textd1[_lang][73 - TEXTD_START]);
	talk_hacker(_textd[_lang][62],"d62.als");
	talk_blind(_textd[_lang][74],"d74.als", _textd1[_lang][74 - TEXTD_START]);
	talk_hacker(_textd[_lang][63],"d63.als");
	talk_blind(_textd[_lang][75],"d75.als", _textd1[_lang][75 - TEXTD_START]);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	_system->delayMillis(1000);
	talk_hacker(_textd[_lang][64], "d64.als");
	talk_blind(_textd[_lang][76], "d76.als", _textd1[_lang][76 - TEXTD_START]);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(14);

	clearRoom();

	playMusic(roomMusic);
	loadPic("9.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("aux9.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	withoutVerb();

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

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

	playSound("s7.als");
	hiccup(15);

	stopSound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_15_2() {
	talk_drunk(_textb[_lang][8], "B8.als");
	pause(7);
	talk_drunk(_textb[_lang][9], "B9.als");
	talk_drunk(_textb[_lang][10], "B10.als");
	talk_drunk(_textb[_lang][11], "B11.als");
}

void DrasculaEngine::animation_16_2() {
	int l;

	talk_drunk(_textb[_lang][12], "B12.als");
	talk(_text[_lang][371], "371.als");

	clearRoom();

	if (_lang == kSpanish)
		playMusic(30);
	else
		playMusic(32);

	int key = getScan();
	if (key != 0)
		goto asco;

	if (_lang != kSpanish)
		color_abc(DARK_GREEN);

	loadPic("his1.alg");
	decompressPic(dir_dibujo1, HALF_PAL);

	if (_lang == kSpanish)
		black();

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][1], 180, 180);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (_lang == kSpanish)
		fadeFromBlack(1);

	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(3000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	fadeToBlack(1);
	key = getScan();
	if (key != 0)
		goto asco;

	clearRoom();
	loadPic("his2.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][2], 180, 180);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(3000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	fadeToBlack(1);
	key = getScan();
	if (key != 0)
		goto asco;

	clearRoom();
	loadPic("his3.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][3], 180, 180);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(3000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	fadeToBlack(1);

	clearRoom();
	loadPic("his4_1.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("his4_2.alg");
	decompressPic(dir_dibujo3, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo3, dir_zona_pantalla);

	if (_lang != kSpanish)
		centerText(_texthis[_lang][1], 180, 180);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getScan();
	if (key != 0)
		goto asco;

	if (_lang == kSpanish)
		_system->delayMillis(2000);
	else
		_system->delayMillis(4000);

	key = getScan();
	if (key != 0)
		goto asco;

	for (l = 1; l < 200; l++) {
		copyBackground(0, 0, 0, l, 320, 200 - l, dir_dibujo3, dir_zona_pantalla);
		copyBackground(0, 200 - l, 0, 0, 320, l, dir_dibujo1, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		key = getScan();
		if (key != 0)
			goto asco;
	}

	pause(5);
	fadeToBlack(2);
	clearRoom();

asco:
	loadPic(roomDisk);
	decompressPic(dir_dibujo3, 1);
	char rm[20];
	sprintf(rm, "%i.alg", roomNumber);
	loadPic(rm);
	decompressPic(dir_dibujo1, HALF_PAL);
	black();
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fadeFromBlack(0);
	if (roomMusic != 0)
		playMusic(roomMusic);
	else
		stopMusic();
}

void DrasculaEngine::animation_17_2() {
	talk_drunk(_textb[_lang][13], "B13.als");
	talk_drunk(_textb[_lang][14], "B14.als");
	flags[40] = 1;
}

void DrasculaEngine::animation_19_2() {
	talk_vbpuerta(5);
}

void DrasculaEngine::animation_20_2() {
	talk_vbpuerta(7);
	talk_vbpuerta(8);
	talk(383);
	talk_vbpuerta(9);
	talk(384);
	talk_vbpuerta(10);
	talk(385);
	talk_vbpuerta(11);
	if (flags[23] == 0) {
		talk(350);
		talk_vbpuerta(57);
	} else {
		talk(386);
		talk_vbpuerta(12);
		flags[18] = 0;
		flags[14] = 1;
		openDoor(15, 1);
		exitRoom(1);
		animation_23_2();
		exitRoom(0);
		flags[21] = 0;
		flags[24] = 0;
		sentido_vb = 1;
		vb_x = 120;

		rompo_y_salgo = 1;
	}
}

void DrasculaEngine::animation_21_2() {
	talk_vbpuerta(6);
}

void DrasculaEngine::animation_23_2() {
	loadPic("an24.alg");
	decompressPic(dir_hare_frente, 1);

	flags[21] = 1;

	if (flags[25] == 0) {
		talk_vb(13);
		talk_vb(14);
		pause(10);
		talk(387);
	}

	talk_vb(15);
	lleva_vb(42);
	sentido_vb = 1;
	talk_vb(16);
	sentido_vb = 2;
	lleva_al_hare(157, 147);
	lleva_al_hare(131, 149);
	sentido_hare = 0;
	animation_14_2();
	if (flags[25] == 0)
		talk_vb(17);
	pause(8);
	sentido_vb = 1;
	talk_vb(18);

	if (flags[29] == 0)
		animation_23_anexo();
	else
		animation_23_anexo2();

	sentido_vb = 2;
	animation_25_2();
	lleva_vb(99);

	if (flags[29] == 0) {
		talk_vb(19);
		if (flags[25] == 0) {
			talk_vb(20);
			if (removeObject(7) == 0)
				flags[30] = 1;
			if (removeObject(18) == 0)
				flags[31] = 1;
			if (removeObject(19) == 0)
				flags[32] = 1;
		}
		talk_vb(21);
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

	playSound("s6.als");

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

	stopSound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_27_2() {
	flags[22] = 1;

	withoutVerb();
	removeObject(23);
	addObject(11);

	talk_vb(23);
	talk_vb(24);
	if (flags[30] == 1)
		addObject(7);
	if (flags[31] == 1)
		addObject(18);
	if (flags[32] == 1)
		addObject(19);
	talk_vb(25);
	talk_vb(26);
}

void DrasculaEngine::animation_28_2() {
	talk_vb(27);
	talk_vb(28);
	talk_vb(29);
	talk_vb(30);
}

void DrasculaEngine::animation_29_2() {
	if (flags[33] == 0) {
		talk_vb(32);
		talk(398);
		talk_vb(33);
		talk(399);
		talk_vb(34);
		talk_vb(35);
		talk(400);
		talk_vb(36);
		talk_vb(37);
		talk(386);
		talk_vb(38);
		talk_vb(39);
		talk(401);
		talk_vb(40);
		talk_vb(41);
		flags[33] = 1;
	} else
		talk_vb(43);

	talk(402);
	talk_vb(42);

	if (flags[38] == 0) {
		talk(403);
		rompo_y_salgo = 1;
	} else
		talk(386);
}

void DrasculaEngine::animation_30_2() {
	talk_vb(31);
	talk(396);
}

void DrasculaEngine::animation_31_2() {
	talk_vb(44);
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
	talk(406);
	lleva_vb(98);
	talk_vb(45);
	talk_vb(46);
	talk_vb(47);
	talk(407);
	talk_vb(48);
	talk_vb(49);
	talk(408);
	talk_vb(50);
	talk_vb(51);
	talk(409);
	talk_vb(52);
	talk_vb(53);
	pause(12);
	talk_vb(54);
	talk_vb(55);
	talk(410);
	talk_vb(56);

	rompo_y_salgo = 1;

	flags[38] = 0;
	flags[36] = 1;
	withoutVerb();
	removeObject(8);
	removeObject(13);
	removeObject(15);
	removeObject(16);
	removeObject(17);
	addObject(20);
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

	playSound("s1.als");
	hipo_sin_nadie(18);
	stopSound();

	pause(10);

	fadeToBlack(2);
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
	decompressPic(dir_hare_frente, HALF_PAL);
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

	playSound("s5.als");

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
	stopSound();
}

void DrasculaEngine::animation_2_4() {
	talk_igor_seated(_texti[_lang][16], "I16.als");
	talk(_text[_lang][278], "278.als");
	talk_igor_seated(_texti[_lang][17], "I17.als");
	talk(_text[_lang][279], "279.als");
	talk_igor_seated(_texti[_lang][18], "I18.als");
}

void DrasculaEngine::animation_3_4() {
	talk_igor_seated(_texti[_lang][19], "I19.als");
	talk_igor_seated(_texti[_lang][20], "I20.als");
	talk(_text[_lang][281], "281.als");
}

void DrasculaEngine::animation_4_4() {
	talk(_text[_lang][287], "287.als");
	talk_igor_seated(_texti[_lang][21], "I21.als");
	talk(_text[_lang][284], "284.als");
	talk_igor_seated(_texti[_lang][22], "I22.als");
	talk(_text[_lang][285], "285.als");
	talk_igor_seated(_texti[_lang][23], "I23.als");
}

void DrasculaEngine::animation_7_4() {
	black();
	talk(_text[_lang][427], "427.als");
	fadeFromBlack(1);
	removeObject(8);
	removeObject(10);
	removeObject(12);
	removeObject(16);
	addObject(17);
	flags[30] = 0;
	flags[29] = 0;
}

void DrasculaEngine::animation_1_5() {
	if (flags[0] == 0) {
		talk(_text[_lang][430], "430.als");
		talk_bj(16);
		talk_bj(17);
		talk_bj(18);
		talk(217);
		talk_bj(19);
		talk(229);
		pause(5);
		lleva_al_hare(114, 170);
		sentido_hare = 3;
		talk(431);
		talk_bj(20);
		sentido_hare = 2;
		pause(4);
		talk(438);
		sitio_x = 120;
		sitio_y = 157;
		walkToObject = 1;
		sentido_final = 1;
		startWalking();
		talk_bj(21);

		for (;;) {
			if (hare_se_mueve == 0)
				break;
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		}

		sentido_hare = 1;
		talk(229);
		flags[0] = 1;
	}

	sentido_hare = 1;
	converse("op_8.cal");
}

void DrasculaEngine::animation_2_5() {
	talk_bj(22);
}

void DrasculaEngine::animation_3_5() {
	talk_bj(23);
	pickObject(10);
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_4_5() {
	flags[7] = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	talk(_text[_lang][228], "228.als");
	talk_wolf(_textl[_lang][1], "L1.als");
	talk_wolf(_textl[_lang][2], "L2.als");
	pause(23);
	talk(_text[_lang][229], "229.als");
	talk_wolf(_textl[_lang][3], "L3.als");
	talk_wolf(_textl[_lang][4], "L4.als");
	talk(_text[_lang][230], "230.als");
	talk_wolf(_textl[_lang][5], "L5.als");
	talk(_text[_lang][231], "231.als");
	talk_wolf(_textl[_lang][6], "L6.als");
	talk_wolf(_textl[_lang][7], "L7.als");
	pause(33);
	talk(_text[_lang][232], "232.als");
	talk_wolf(_textl[_lang][8], "L8.als");
}

void DrasculaEngine::animation_5_5(){
	int h;
	int frame = 0;
	int hueso_x[] = {1, 99, 197, 1, 99, 197, 1, 99, 197};
	int hueso_y[] = {1, 1, 1, 66, 66, 66, 131, 131, 131};
	int vuela_x[] = {1, 63, 125, 187, 249};
	int pixel_x = hare_x - 53, pixel_y = hare_y - 9;

	withoutVerb();
	removeObject(8);

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
	updateData();
	pause(12);

	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	for (h = 0; h < (200 - 18); h++)
		copyBackground(0, 53, 0, h, 320, 19, dir_hare_frente, dir_zona_pantalla);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	loadPic("101.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
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

	playSound("s1.als");
	stopSound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);

	clearRoom();

	loadPic("49.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
}

void DrasculaEngine::animation_6_5() {
	talk_wolf(_textl[_lang][9], "L9.als");
	talk(_text[_lang][234], "234.als");
}

void DrasculaEngine::animation_7_5() {
	talk_wolf(_textl[_lang][10], "L10.als");
	talk(_text[_lang][236], "236.als");
	talk_wolf(_textl[_lang][11], "L11.als");
	talk_wolf(_textl[_lang][12], "L12.als");
	talk_wolf(_textl[_lang][13], "L13.als");
	pause(34);
	talk_wolf(_textl[_lang][14], "L14.als");
}

void DrasculaEngine::animation_8_5() {
	talk_wolf(_textl[_lang][15], "L15.als");
	talk(_text[_lang][238], "238.als");
	talk_wolf(_textl[_lang][16], "L16.als");
}

void DrasculaEngine::animation_9_5() {
	flags[4] = 1;
	talk(_text[_lang][401], "401.als");
	withoutVerb();
	removeObject(15);
}

void DrasculaEngine::animation_10_5() {
	flags[3] = 1;
	talk(_text[_lang][401], "401.als");
	withoutVerb();
	removeObject(12);
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
	playSound("s5.als");
	anima("rayo2.bin", 17);
	sentido_hare = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	hare_oscuro();

	for (color = 0; color < 255; color++)
		for (componente = 0; componente < 3; componente++) {
			palFondo1[color][componente] = gamePalette[color][componente];
			palFondo2[color][componente] = gamePalette[color][componente];
			palFondo3[color][componente] = gamePalette[color][componente];
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
			setPalette((byte *)&palFondo1);
		else if (frame == 1 || frame == 5 || frame == 7 || frame == 9)
			setPalette((byte *)&palFondo2);
		else
			setPalette((byte *)&palFondo3);

		pause(4);
		updateRoom();
		copyRect(rayo_x[frame], 1, 41, 0, 44, 44, dir_hare_fondo, dir_zona_pantalla);
		copyRect(frusky_x[frame], 113, 205, 50, 38, 86, dir_dibujo3, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	stopSound_corte();

	for (frame = 0; frame < 15; frame++) {
		if (frame == 2 || frame == 4 || frame == 7 || frame == 9)
			setPalette((byte *)&palFondo1);
		else if (frame == 1 || frame == 5)
			setPalette((byte *)&gamePalette);
		else
			setPalette((byte *)&palFondo2);

		pause(4);
		updateRoom();
		copyRect(elfrusky_x[frame], 47, 192, 39, 66, 106, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	anima("frel.bin", 16);
	clearRoom();
	hare_claro();
	updatePalette();

	flags[1] = 1;

	animation_13_5();
	playSound("s1.als");
	hiccup(12);
	stopSound();

	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);

	lleva_al_hare(40, 169);
	lleva_al_hare(-14, 175);

	rompo = 1;
	previousMusic = roomMusic;
	hare_se_ve = 1;
	clearRoom();
	sentido_hare = 1;
	hare_se_mueve = 0;
	hare_x = -1;
	objExit = 104;
	withoutVerb();
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
	playSound("s3.als");
	updateRoom();
	updateScreen(0, 0, 0,0 , 320, 200, dir_zona_pantalla);
	stopSound();
	pause(17);
	sentido_hare = 3;
	talk(_text[_lang][246],"246.als");
	lleva_al_hare(89, 160);
	flags[10] = 1;
	playSound("s7.als");
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	stopSound();
	pause(14);
	sentido_hare = 3;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	talk_solo(_textd[_lang][18], "d18.als");
	fadeToBlack(1);
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
	talk_dr_dch(19);
	talk(247);
	talk_dr_dch(20);
	talk_dr_dch(21);
	talk(248);
	talk_dr_dch(22);
	talk(249);
	talk_dr_dch(23);
	converse("op_11.cal");
	talk_dr_dch(26);

	anima("fum.bin", 15);

	talk_dr_dch(27);
	talk(254);
	talk_dr_dch(28);
	talk(255);
	talk_dr_dch(29);
	fadeToBlack(1);
	clearRoom();
	loadPic("time1.alg");
	decompressPic(dir_zona_pantalla, 1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(930);
	clearRoom();
	black();
	hare_se_ve = 0;
	flags[0] = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fadeFromBlack(1);
	talk(256);
	talk_dr_dch(30);
	talk(257);
	fadeToBlack(0);
	clearRoom();
	loadPic("time1.alg");
	decompressPic(dir_zona_pantalla,1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(900);
	clearRoom();
	black();
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fadeFromBlack(1);
	talk(258);
	talk_dr_dch(31);
	animation_5_6();
	talk_dr_dch(32);
	talk_igor_dch(11);
	sentido_igor = 3;
	talk_dr_dch(33);
	talk_igor_frente(_texti[_lang][12], "I12.als");
	talk_dr_dch(34);
	sentido_dr = 0;
	talk_dr_izq(35);

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	clearRoom();
	carga_escoba("102.ald");
	activatePendulum();
}

void DrasculaEngine::animation_2_6() {
	talk_dr_dch(24);
}

void DrasculaEngine::animation_3_6() {
	talk_dr_dch(24);
}

void DrasculaEngine::animation_4_6() {
	talk_dr_dch(25);
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
	withoutVerb();
	removeObject(20);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	rompo = 1;
	objExit = 104;
	hare_x = -1;
	withoutVerb();
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
	updateData();
}

void DrasculaEngine::animation_9_6() {
	int v_cd;

	anima("fin.bin", 14);
	playMusic(13);
	flags[5] = 1;
	anima("drf.bin", 16);
	fadeToBlack(0);
	clearRoom();
	hare_x = -1;
	objExit = 108;
	carga_escoba("59.ald");
	// The room number was originally changed here to "nada.alg",
	// which is a non-existant file. In reality, this was just a
	// hack to set the room number to a non-existant one, so that
	// room sprites do not appear again when the room is refreshed.
	// We set the room number to -1 for the same purpose.
	// Also check animation_2_1(), where the same hack was used
	// by the original
	roomNumber = -1;
	loadPic("nota2.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	black();
	sentido_hare = 1;
	hare_x -= 21;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fadeFromBlack(0);
	pause(96);
	lleva_al_hare(116, 178);
	sentido_hare = 2;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	playMusic(9);
	clearRoom();
	loadPic("nota.alg");
	decompressPic(dir_dibujo1, COMPLETE_PAL);
	color_abc(WHITE);
	talk_solo(_textbj[_lang][24], "bj24.als");
	talk_solo(_textbj[_lang][25], "bj25.als");
	talk_solo(_textbj[_lang][26], "bj26.als");
	talk_solo(_textbj[_lang][27], "bj27.als");
	talk_solo(_textbj[_lang][28], "bj28.als");
	sentido_hare = 3;
	clearRoom();
	loadPic("96.alg");
	decompressPic(dir_hare_frente, COMPLETE_PAL);
	loadPic("nota2.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
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
	fadeToBlack(1);
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
	playSound ("s14.als");
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();
	copyBackground(164, 85, 155, 48, 113, 114, dir_dibujo3, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	stopSound();
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
	addObject(21);
	flags[10] = 1;
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_15_6() {
	talk_taber2(_textt[_lang][16], "t16.als");
}

void DrasculaEngine::animation_18_6() {
	flags[6] = 1;
	withoutVerb();
	removeObject(21);
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
	playSound("s4.als");
	pause(6);
	stopSound();
}

void DrasculaEngine::animation_12_2() {
	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

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
	converse("op_1.cal");

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	flags[11] = 0;
	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_26_2() {
	int n, x = 0;

	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

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
			playSound("s9.als");
		pause(3);
	}

	stopSound_corte();
	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 121, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225, 113, 225, 113, 50, 59, dir_zona_pantalla);
		x = x + 50;
		pause(3);
	}

	pickObject(11);
	removeObject(12);

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	flags[11] = 0;
	flags[39] = 1;
	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
	roomMusic = 16;
}

void DrasculaEngine::animation_11_2() {
	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

	loadPic("an11y13.alg");
	decompressPic(dir_hare_dch, 1);

	talk(352);
	talk_tabernero(1);
	talk(353);
	talk_tabernero(17);
	talk(354);
	talk_tabernero(18);
	talk(355);
	pause(40);
	talk_tabernero("No, nada", "d82.als");

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	loadPic("974.alg");
	decompressPic(dir_hare_dch, 1);
}

void DrasculaEngine::animation_13_2() {
	loadPic("an11y13.alg");
	decompressPic(dir_hare_frente, 1);

	if (flags[41] == 0) {
		talk(_text[_lang][103], "103.als");
		talk_drunk(_textb[_lang][4], "B4.als");
		flags[12] = 1;
		talk(_text[_lang][367], "367.als");
		talk_drunk(_textb[_lang][5], "B5.als");
		flags[12] = 1;
		talk(_text[_lang][368], "368.als");
		talk_drunk(_textb[_lang][6], "B6.als");
		talk_drunk(_textb[_lang][7], "B7.als");
		flags[41] = 1;
	}
	converse("op_2.cal");

	loadPic("964.alg");
	decompressPic(dir_hare_frente, 1);
}

void DrasculaEngine::animation_18_2() {
	talk(378);
	talk_vbpuerta(4);
	converse("op_3.cal");
}

void DrasculaEngine::animation_22_2() {
	talk(_text[_lang][374],"374.als");

	sentido_hare=2;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	playSound("s13.als");
	stopSound();
	sentido_hare = 1;

	talk_vbpuerta(1);
	talk(375);
	talk_vbpuerta(2);
	talk(376);
	talk_vbpuerta(3);

	flags[18] = 1;
}

void DrasculaEngine::animation_24_2() {
	if (hare_x < 178)
		lleva_al_hare(208, 136);
	sentido_hare = 3;
	updateRoom();
	pause(3);
	sentido_hare = 0;

	talk(356);

	loadPic("an24.alg");
	decompressPic(dir_hare_frente, 1);

	animation_32_2();

	flags[21] = 1;

	talk_vb(22);

	if (flags[22] == 0)
		converse("op_4.cal");
	else
		converse("op_5.cal");

	exitRoom(0);
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
	playSound("s8.als");

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 1, 218, 79, 83, 75,	dir_hare_dch, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83,75, dir_zona_pantalla);
		x = x + 83;
		pause(3);
	}
	stopSound();

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
	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

	loadPic("an11y13.alg");
	decompressPic(dir_hare_dch, 1);

	talk(404);
	talk_tabernero(19);
	talk_tabernero(20);
	talk_tabernero(21);
	talk(355);
	pause(40);
	talk_tabernero("No, nada", "d82.als");

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

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

	playSound("s1.als");

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_frente, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pause(3);
	}
	stopSound_corte();

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
	withoutVerb();
}

void DrasculaEngine::animation_6_2() {
	stopMusic();
	flags[9] = 1;

	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

	clearRoom();
	loadPic("blind1.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("blind2.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("blind3.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("blind4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("blind5.alg");
	decompressPic(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(1);

	if (flags[4] == 1)
		talk_hacker(_textd[_lang][66], "d66.als");
	pause(6);
	talk_blind(_textd[_lang][78], "d78.als", _textd1[_lang][78 - TEXTD_START]);
	pause(4);
	talk_hacker(_textd[_lang][67], "d67.als");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(10);

	clearRoom();

	playMusic(roomMusic);
	loadPic("9.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("aux9.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	withoutVerb();

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	flags[9] = 0;
}

void DrasculaEngine::animation_33_2() {
	stopMusic();
	flags[9] = 1;

	pause(12);
	talk(_textd[_lang][56], "d56.als" );
	pause(8);

	clearRoom();
	loadPic("blind1.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("blind2.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("blind3.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("blind4.alg");
	decompressPic(dir_hare_fondo, 1);
	loadPic("blind5.alg");
	decompressPic(dir_hare_frente, 1);

	if (_lang == kSpanish)
		dir_texto = dir_hare_frente;

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(10);

	talk_blind(_textd[_lang][68], "d68.als", _textd1[_lang][68 - TEXTD_START]);
	pause(5);
	talk_hacker(_textd[_lang][57], "d57.als");
	pause(6);
	_system->delayMillis(1000);
	talk_blind(_textd[_lang][77], "d77.als", _textd1[_lang][77 - TEXTD_START]);
	talk_hacker(_textd[_lang][65], "d65.als");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pause(14);

	clearRoom();

	playMusic(roomMusic);
	loadPic("9.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("aux9.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic("97.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	withoutVerb();

	if (_lang == kSpanish)
		dir_texto = dir_hare_dch;

	flags[33] = 1;
	flags[9] = 0;
}

void DrasculaEngine::animation_1_4() {
	if (flags[21] == 0) {
		strcpy(objName[2], "igor");
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

		talk_igor_seated(_texti[_lang][13], "I13.als");
		talk_igor_seated(_texti[_lang][14], "I14.als");
		talk_igor_seated(_texti[_lang][15], "I15.als");
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
		talk_igor_seated(_texti[_lang][6], "I6.als");
	}

	converse("op_6.cal");
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
	openDoor(2, 0);
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
	fadeToBlack(0);
	exitRoom(0);
}

void DrasculaEngine::animation_6_4() {
	int prevRoom = roomNumber;

	roomNumber = 26;
	clearRoom();
	loadPic("26.alg");
	decompressPic(dir_dibujo1, HALF_PAL);
	loadPic("aux26.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("auxigor.alg");
	decompressPic(dir_hare_frente, 1);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	update_26_pre();
	x_igor = 104;
	y_igor = 71;
	placeIgor();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(40);
	talk_igor_frente(_texti[_lang][26], "I26.als");
	roomNumber = prevRoom;
	clearRoom();
	loadPic("96.alg");
	decompressPic(dir_hare_frente, 1);
	loadPic(roomDisk);
	decompressPic(dir_dibujo3, 1);
	char rm[20];
	sprintf(rm, "%i.alg", roomNumber);
	loadPic(rm);
	decompressPic(dir_dibujo1, HALF_PAL);
	withoutVerb();
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
	openDoor(7, 2);
}

void DrasculaEngine::animation_9_4() {
	anima("st.bin", 14);
	fadeToBlack(1);
}


} // End of namespace Drascula
