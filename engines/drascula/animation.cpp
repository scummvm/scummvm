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
#include "drascula/texts.h"

namespace Drascula {

static const int interf_x[] ={ 1, 65, 129, 193, 1, 65, 129 };
static const int interf_y[] ={ 51, 51, 51, 51, 83, 83, 83 };

void DrasculaEngine::animation_1_1() {
	int l, l2, p;
	int pos_pixel[6];

	while (term_int == 0) {
		playmusic(29);
		fliplay("logoddm.bin", 9);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(600);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		delay(340);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		playmusic(26);
		delay(500);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fliplay("logoalc.bin", 8);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("cielo.alg");
		descomprime_dibujo(dir_zona_pantalla, 256);
		Negro();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(2);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(900);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_abc(ROJO);
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
		borra_pantalla();
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
		lee_dibujos("cielo2.alg");
		descomprime_dibujo(dir_zona_pantalla, 256);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		FundeAlNegro(1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();

		lee_dibujos("96.alg");
		descomprime_dibujo(dir_hare_frente, COMPLETA);
		lee_dibujos("103.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		lee_dibujos("104.alg");
		descomprime_dibujo(dir_dibujo3, 1);
		lee_dibujos("aux104.alg");
		descomprime_dibujo(dir_dibujo2, 1);

		playmusic(4);
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
				pausa(3);
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

		habla_dr_grande(TEXTD1, "D1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		borra_pantalla();

		lee_dibujos("100.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		lee_dibujos("auxigor.alg");
		descomprime_dibujo(dir_hare_frente, 1);
		lee_dibujos("auxdr.alg");
		descomprime_dibujo(dir_hare_fondo, 1);
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
		habla_igor_dch(TEXTI8, "I8.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD2, "d2.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD3, "d3.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		color_solo = ROJO;
		lee_dibujos("plan1.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(10);
		habla_solo(TEXTD4,"d4.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("plan1.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_solo(TEXTD5, "d5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("plan2.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(20);
		habla_solo(TEXTD6, "d6.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("plan3.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(20);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_solo(TEXTD7, "d7.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("plan3.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_solo(TEXTD8, "d8.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("100.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		MusicFadeout();
		stopmusic();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI9, "I9.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD9, "d9.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI10, "I10.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		playmusic(11);
		habla_dr_izq(TEXTD10, "d10.als");
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
		borra_pantalla();
		Negro();
		playmusic(23);
		FundeDelNegro(0);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 1;
		habla_igor_dch(TEXTI1, "I1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_dch(TEXTD11, "d11.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(1);
		sentido_dr = 0;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD12, "d12.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(1);
		sentido_dr = 1;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_igor_dch(TEXTI2, "I2.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(13);
		habla_dr_dch(TEXTD13,"d13.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(1);
		sentido_dr = 0;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD14, "d14.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI3, "I3.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD15, "d15.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI4, "I4.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD16, "d16.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI5, "I5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_igor = 3;
		habla_dr_izq(TEXTD17, "d17.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(18);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_frente(TEXTI6, "I6.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		FundeAlNegro(0);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();

		playmusic(2);
		pausa(5);
		fliplay("intro.bin", 12);
		term_int = 1;
	}
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::habla_dr_grande(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;
	int x_habla[4] = {47, 93, 139, 185};
	int cara;
	int l = 0;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(ROJO);

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
	copyBackground(x_habla[cara], 1, 171, 68, 45, 48, dir_dibujo2, dir_zona_pantalla);
	l++;
	if (l == 7)
		l = 0;

	if (con_voces == 0)
		centra_texto(dicho, 191, 69);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;

	if (key != 0)
		ctvd_stop();
	buffer_teclado();
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

		lee_dibujos("an11y13.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		habla_tabernero(TEXTT22, "T22.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		lee_dibujos("97.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		pausa(4);
		comienza_sound("s1.als");
		hipo(18);
		fin_sound();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		borra_pantalla();
		stopmusic();
		corta_musica = 1;
		memset(dir_zona_pantalla, 0, 64000);
		color_solo = BLANCO;
		pausa(80);

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_solo(TEXTBJ1, "BJ1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("bj.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		Negro();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = AMARILLO;
		habla_solo(TEXT214, "214.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();

		lee_dibujos("16.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("auxbj.alg");
		descomprime_dibujo(dir_dibujo3, 1);
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

		lee_dibujos("97g.alg");
		descomprime_dibujo(dir_hare_dch, 1);
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

		habla_bj(TEXTBJ2, "BJ2.als");
		hablar(TEXT215, "215.als");
		habla_bj(TEXTBJ3, "BJ3.als");
		hablar(TEXT216, "216.als");
		habla_bj(TEXTBJ4, "BJ4.als");
		habla_bj(TEXTBJ5, "BJ5.als");
		habla_bj(TEXTBJ6, "BJ6.als");
		hablar(TEXT217, "217.als");
		habla_bj(TEXTBJ7, "BJ7.als");
		hablar(TEXT218, "218.als");
		habla_bj(TEXTBJ8, "BJ8.als");
		hablar(TEXT219, "219.als");
		habla_bj(TEXTBJ9, "BJ9.als");
		hablar(TEXT220, "220.als");
		hablar(TEXT221, "221.als");
		habla_bj(TEXTBJ10, "BJ10.als");
		hablar(TEXT222, "222.als");
		anima("gaf.bin", 15);
		anima("bjb.bin", 14);
		playmusic(9);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("97.alg");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		refresca_pantalla();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(120);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_solo(TEXT223, "223.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = BLANCO;
		refresca_pantalla();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(110);
		habla_solo(TEXTBJ11, "BJ11.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		refresca_pantalla();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(118);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lleva_al_hare(132, 97 + alto_hare);
		pausa(60);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		hablar(TEXT224, "224.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_bj(TEXTBJ12, "BJ12.als");
		lleva_al_hare(157, 98 + alto_hare);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("bes.bin", 16);
		playmusic(11);
		anima("rap.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 3;
		strcpy(num_room, "no_bj.alg");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(8);
		refresca_pantalla();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		hablar(TEXT225, "225.als");
		pausa(76);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		refresca_pantalla();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		hablar(TEXT226, "226.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		refresca_pantalla();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(30);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		hablar(TEXT227,"227.als");
		FundeAlNegro(0);
		break;
	}
}

void DrasculaEngine::animation_3_1() {
	lee_dibujos("an11y13.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT192, "192.als");
	habla_tabernero(TEXTT1, "t1.als");
	hablar(TEXT193, "193.als");
	habla_tabernero(TEXTT2, "t2.als");
	hablar(TEXT194, "194.als");
	habla_tabernero(TEXTT3, "t3.als");
	hablar(TEXT195, "195.als");
	habla_tabernero(TEXTT4, "t4.als");
	hablar(TEXT196, "196.als");
	habla_tabernero(TEXTT5, "t5.als");
	habla_tabernero(TEXTT6, "t6.als");
	hablar(TEXT197, "197.als");
	habla_tabernero(TEXTT7, "t7.als");
	hablar(TEXT198, "198.als");
	habla_tabernero(TEXTT8, "t8.als");
	hablar(TEXT199, "199.als");
	habla_tabernero(TEXTT9, "t9.als");
	hablar(TEXT200, "200.als");
	hablar(TEXT201, "201.als");
	hablar(TEXT202, "202.als");

	flags[0] = 1;

	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
}

void DrasculaEngine::animation_4_1() {
	lee_dibujos("an12.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT205,"205.als");

	actualiza_refresco_antes();

	copyBackground(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	updateScreen(228,112, 228,112, 47,60, dir_zona_pantalla);

	pausa(3);

	actualiza_refresco_antes();

	copyBackground(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);
	stopmusic();
	flags[11] = 1;

	habla_pianista(TEXTP1, "p1.als");
	hablar(TEXT206, "206.als");
	habla_pianista(TEXTP2, "p2.als");
	hablar(TEXT207, "207.als");
	habla_pianista(TEXTP3, "p3.als");
	hablar(TEXT208, "208.als");
	habla_pianista(TEXTP4, "p4.als");
	hablar(TEXT209, "209.als");

	flags[11] = 0;
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
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
	actualiza_refresco();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	lee_dibujos("an2_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an2_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	copyBackground(1, 1, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		updateScreen(201,87, 201,87, 50,52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 55, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++){
		x++;
		copyBackground(x, 109, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;
	comienza_sound("s2.als");

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		updateScreen(201,87, 201,87, 50,52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 55, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}
	x = 0;

	for (n = 0; n < 2; n++) {
		x++;
		copyBackground(x, 109, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		updateScreen(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	fin_sound();

	pausa (4);

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
	stopmusic();
	flags[9] = 1;

	pausa(12);
	hablar(TEXTD56, "d56.als");
	pausa(8);

	borra_pantalla();
	lee_dibujos("ciego1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("ciego2.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("ciego3.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("ciego4.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("ciego5.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(10);

	habla_ciego(TEXTD68, "d68.als", "44472225500022227555544444664447222550002222755554444466");
	pausa(5);
	habla_hacker(TEXTD57, "d57.als");
	pausa(6);
	habla_ciego(TEXTD69,"d69.als","444722255000222275555444446655033336666664464402256555005504450005446");
	pausa(4);
	habla_hacker(TEXTD58,"d58.als");
	habla_ciego(TEXTD70,"d70.als", "4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046");
	delay(14);
	habla_hacker(TEXTD59,"d59.als");
	habla_ciego(TEXTD71,"d71.als", "550330227556444744446660004446655544444722255000222275555444446644444");
	habla_hacker(TEXTD60,"d60.als");
	habla_ciego(TEXTD72,"d72.als", "55033022755644455550444744400044504447222550002222755554444466000");
	habla_hacker(TEXTD61,"d61.als");
	habla_ciego(TEXTD73,"d73.als", "55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446");
	habla_hacker(TEXTD62,"d62.als");
	habla_ciego(TEXTD74,"d74.als", "55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666");
	habla_hacker(TEXTD63,"d63.als");
	habla_ciego(TEXTD75,"d75.als", "44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555");
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	_system->delayMillis(1);
	habla_hacker(TEXTD64, "d64.als");
	habla_ciego(TEXTD76, "d76.als", "5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(14);

	borra_pantalla();

	playmusic(musica_room);
	lee_dibujos("9.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("aux9.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sin_verbo();

	flags[9] = 0;
	flags[4] = 1;
}

void DrasculaEngine::animation_8_2() {
	habla_pianista(TEXTP6, "P6.als");
	hablar(TEXT358, "358.als");
	habla_pianista(TEXTP7, "P7.als");
	habla_pianista(TEXTP8, "P8.als");
}

void DrasculaEngine::animation_9_2() {
	habla_pianista(TEXTP9, "P9.als");
	habla_pianista(TEXTP10, "P10.als");
	habla_pianista(TEXTP11, "P11.als");
}

void DrasculaEngine::animation_10_2() {
	habla_pianista(TEXTP12, "P12.als");
	hablar(TEXT361, "361.als");
	pausa(40);
	habla_pianista(TEXTP13, "P13.als");
	hablar(TEXT362, "362.als");
	habla_pianista(TEXTP14, "P14.als");
	hablar(TEXT363, "363.als");
	habla_pianista(TEXTP15, "P15.als");
	hablar(TEXT364, "364.als");
	habla_pianista(TEXTP16, "P16.als");
}

void DrasculaEngine::animation_14_2() {
	int n, pos_cabina[6];
	int l = 0;

	lee_dibujos("an14_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an14_1.alg");

	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = -160;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;

	for (n = -160; n <= 0; n = n + 5 + l) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		actualiza_refresco_antes();
		pon_hare();
		pon_vb();
		pos_cabina[3] = n;
		copyRectClip(pos_cabina, dir_hare_fondo, dir_zona_pantalla);
		actualiza_refresco();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		l = l + 1;
	}

	flags[24] = 1;

	descomprime_dibujo(dir_dibujo1, 1);

	comienza_sound("s7.als");
	hipo(15);

	fin_sound();

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_15_2() {
	habla_borracho(TEXTB8, "B8.als");
	pausa(7);
	habla_borracho(TEXTB9, "B9.als");
	habla_borracho(TEXTB10, "B10.als");
	habla_borracho(TEXTB11, "B11.als");
}

void DrasculaEngine::animation_16_2() {
	int l;

	habla_borracho(TEXTB12, "B12.als");
	hablar(TEXT371, "371.als");

	borra_pantalla();

	playmusic(32);
	int key = getscan();
	if (key != 0)
		goto asco;

	color_abc(VERDE_OSCURO);

	lee_dibujos("his1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(HIS1, 180, 180);
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

	borra_pantalla();
	lee_dibujos("his2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(HIS2, 180, 180);
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

	borra_pantalla();
	lee_dibujos("his3.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(HIS3, 180, 180);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);

	borra_pantalla();
	lee_dibujos("his4_1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("his4_2.alg");
	descomprime_dibujo(dir_dibujo3, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo3, dir_zona_pantalla);
	centra_texto(HIS1, 180, 180);
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

	pausa(5);
	FundeAlNegro(2);
	borra_pantalla();

asco:
	lee_dibujos(pantalla_disco);
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);
	Negro();
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	if (musica_room != 0)
		playmusic(musica_room);
	else
		stopmusic();
}

void DrasculaEngine::animation_17_2() {
	habla_borracho(TEXTB13, "B13.als");
	habla_borracho(TEXTB14, "B14.als");
	flags[40] = 1;
}

void DrasculaEngine::animation_19_2() {
	habla_vbpuerta(TEXTVB5, "VB5.als");
}

void DrasculaEngine::animation_20_2() {
	habla_vbpuerta(TEXTVB7, "VB7.als");
	habla_vbpuerta(TEXTVB8, "VB8.als");
	hablar(TEXT383, "383.als");
	habla_vbpuerta(TEXTVB9, "VB9.als");
	hablar(TEXT384, "384.als");
	habla_vbpuerta(TEXTVB10, "VB10.als");
	hablar(TEXT385, "385.als");
	habla_vbpuerta(TEXTVB11, "VB11.als");
	if (flags[23] == 0) {
		hablar(TEXT350, "350.als");
		habla_vbpuerta(TEXTVB57, "VB57.als");
	} else {
		hablar(TEXT386, "386.als");
		habla_vbpuerta(TEXTVB12, "VB12.als");
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
	habla_vbpuerta(TEXTVB6, "VB6.als");
}

void DrasculaEngine::animation_23_2() {
	lee_dibujos("an24.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	flags[21] = 1;

	if (flags[25] == 0) {
		habla_vb(TEXTVB13, "VB13.als");
		habla_vb(TEXTVB14, "VB14.als");
		pausa(10);
		hablar(TEXT387, "387.als");
	}

	habla_vb(TEXTVB15, "VB15.als");
	lleva_vb(42);
	sentido_vb = 1;
	habla_vb(TEXTVB16, "VB16.als");
	sentido_vb = 2;
	lleva_al_hare(157, 147);
	lleva_al_hare(131, 149);
	sentido_hare = 0;
	animation_14_2();
	if (flags[25] == 0)
		habla_vb(TEXTVB17, "VB17.als");
	pausa(8);
	sentido_vb = 1;
	habla_vb(TEXTVB18, "VB18.als");

	if (flags[29] == 0)
		animation_23_anexo();
	else
		animation_23_anexo2();

	sentido_vb = 2;
	animation_25_2();
	lleva_vb(99);

	if (flags[29] == 0) {
		habla_vb(TEXTVB19, "VB19.als");
		if (flags[25] == 0) {
			habla_vb(TEXTVB20,"VB20.als");
			if (resta_objeto(7) == 0)
				flags[30] = 1;
			if (resta_objeto(18) == 0)
				flags[31] = 1;
			if (resta_objeto(19) == 0)
				flags[32] = 1;
		}
		habla_vb(TEXTVB21, "VB21.als");
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

	lee_dibujos("an23.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 34; n++) {
		copyRect(p_x, p_y, p_x, p_y, 36, 74, dir_dibujo1, dir_zona_pantalla);
		copyRect(x[n], y[n], p_x, p_y, 36, 74, dir_hare_fondo, dir_zona_pantalla);
		actualiza_refresco();
		updateScreen(p_x, p_y, p_x, p_y, 36, 74, dir_zona_pantalla);
		pausa(5);
	}

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_23_anexo2() {
	int n, p_x = hare_x + 4, p_y = hare_y;
	int x[] = {1, 35, 69, 103, 137, 171, 205, 239, 273, 1, 35, 69, 103, 137};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 73, 73, 73, 73, 73};

	pausa(50);

	lee_dibujos("an23_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 14; n++) {
		copyRect(p_x, p_y, p_x, p_y, 33, 71, dir_dibujo1, dir_zona_pantalla);
		copyRect(x[n], y[n], p_x, p_y, 33, 71, dir_hare_fondo, dir_zona_pantalla);
		actualiza_refresco();
		updateScreen(p_x,p_y, p_x,p_y, 33,71, dir_zona_pantalla);
		pausa(5);
	}

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo,1);
}

void DrasculaEngine::animation_25_2() {
	int n, pos_cabina[6];

	lee_dibujos("an14_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("18.alg");
	descomprime_dibujo(dir_dibujo1, 1);

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

		actualiza_refresco_antes();
		pon_hare();
		pon_vb();

		pos_cabina[3] = n;

		copyRectClip(pos_cabina, dir_hare_fondo, dir_zona_pantalla);

		actualiza_refresco();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	fin_sound();

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animation_27_2() {
	flags[22] = 1;

	sin_verbo();
	resta_objeto(23);
	suma_objeto(11);

	habla_vb(TEXTVB23, "VB23.als");
	habla_vb(TEXTVB24, "VB24.als");
	if (flags[30] == 1)
		suma_objeto(7);
	if (flags[31] == 1)
		suma_objeto(18);
	if (flags[32] == 1)
		suma_objeto(19);
	habla_vb(TEXTVB25,"VB25.als");
	habla_vb(TEXTVB26,"VB26.als");
}

void DrasculaEngine::animation_28_2() {
	habla_vb(TEXTVB27, "VB27.als");
	habla_vb(TEXTVB28, "VB28.als");
	habla_vb(TEXTVB29, "VB29.als");
	habla_vb(TEXTVB30, "VB30.als");
}

void DrasculaEngine::animation_29_2() {
	if (flags[33] == 0) {
		habla_vb(TEXTVB32, "VB32.als");
		hablar(TEXT398, "398.als");
		habla_vb(TEXTVB33, "VB33.als");
		hablar(TEXT399, "399.als");
		habla_vb(TEXTVB34, "VB34.als");
		habla_vb(TEXTVB35, "VB35.als");
		hablar(TEXT400, "400.als");
		habla_vb(TEXTVB36, "VB36.als");
		habla_vb(TEXTVB37, "VB37.als");
		hablar(TEXT386, "386.als");
		habla_vb(TEXTVB38, "VB38.als");
		habla_vb(TEXTVB39, "VB39.als");
		hablar(TEXT401, "401.als");
		habla_vb(TEXTVB40, "VB40.als");
		habla_vb(TEXTVB41, "VB41.als");
		flags[33] = 1;
	} else
		habla_vb(TEXTVB43, "VB43.als");

	hablar(TEXT402, "402.als");
	habla_vb(TEXTVB42, "VB42.als");

	if (flags[38] == 0) {
		hablar(TEXT403, "403.als");
		rompo_y_salgo = 1;
	} else
		hablar(TEXT386, "386.als");
}

void DrasculaEngine::animation_30_2() {
	habla_vb(TEXTVB31, "VB31.als");
	hablar(TEXT396, "396.als");
}

void DrasculaEngine::animation_31_2() {
	habla_vb(TEXTVB44, "VB44.als");
	lleva_vb(-50);
	pausa(15);
	lleva_al_hare(159, 140);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sentido_hare = 2;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(78);
	sentido_hare = 0;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(22);
	hablar(TEXT406, "406.als");
	lleva_vb(98);
	habla_vb(TEXTVB45, "VB45.als");
	habla_vb(TEXTVB46, "VB46.als");
	habla_vb(TEXTVB47, "VB47.als");
	hablar(TEXT407, "407.als");
	habla_vb(TEXTVB48, "VB48.als");
	habla_vb(TEXTVB49, "VB49.als");
	hablar(TEXT408, "408.als");
	habla_vb(TEXTVB50, "VB50.als");
	habla_vb(TEXTVB51, "VB51.als");
	hablar(TEXT409, "409.als");
	habla_vb(TEXTVB52, "VB52.als");
	habla_vb(TEXTVB53, "VB53.als");
	pausa(12);
	habla_vb(TEXTVB54, "VB54.als");
	habla_vb(TEXTVB55, "VB55.als");
	hablar(TEXT410, "410.als");
	habla_vb(TEXTVB56, "VB56.als");

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

	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("an35_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an35_2.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 70, 90, 46, 80, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(70,90, 70,90, 46,80,dir_zona_pantalla);
		x = x + 46;
		pausa(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 82, 70, 90, 46, 80, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(70, 90, 70, 90, 46, 80, dir_zona_pantalla);
		x = x + 46;
		pausa(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 70, 90, 46, 80, dir_hare_frente, dir_zona_pantalla);
		updateScreen(70, 90, 70, 90, 46, 80, dir_zona_pantalla);

		x = x + 46;

		pausa(3);
	}

	x = 0;
	for (n = 0; n < 2; n++) {
		x++;
		copyBackground(x, 82, 70, 90, 46, 80, dir_hare_frente, dir_zona_pantalla);
		updateScreen(70, 90, 70,90, 46, 80,dir_zona_pantalla);
		x = x + 46;
		pausa(3);
	}

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(19);

	comienza_sound("s1.als");
	hipo_sin_nadie(18);
	fin_sound();

	pausa(10);

	FundeAlNegro(2);
}

void DrasculaEngine::animation_1_3() {
	hablar(TEXT413, "413.als");
	grr();
	pausa(50);
	hablar(TEXT414, "414.als");
}

void DrasculaEngine::animation_2_3() {
	flags[0] = 1;
	playmusic(13);
	animation_3_3();
	playmusic(13);
	animation_4_3();
	flags[1] = 1;
	refresca_pantalla();
	updateScreen(120, 0, 120, 0, 200, 200, dir_zona_pantalla);
	animation_5_3();
	flags[0] = 0;
	flags[1] = 1;

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	lleva_al_hare(332, 127);
}

void DrasculaEngine::animation_3_3() {
	int n, x = 0;
	int px = hare_x - 20, py = hare_y - 1;

	lee_dibujos("an2y_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an2y_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an2y_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}
}

void DrasculaEngine::animation_4_3() {
	int n, x = 0;
	int px = 120, py = 63;

	lee_dibujos("any_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("any_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("any_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++){
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, px, py, 77, 89, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 91, px, py, 77, 89, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, px, py, 77, 89, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 91, px, py, 77, 89, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, px, py, 77, 89, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 91, px, py, 77, 89, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}
}

void DrasculaEngine::animation_5_3() {
	int n, x = 0;
	int px = hare_x - 20, py = hare_y - 1;

	lee_dibujos("an3y_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an3y_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an3y_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		updateScreen(px,py, px,py, 71,72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 2, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}
}

void DrasculaEngine::animation_6_3() {
	int frame = 0, px = 112, py = 62;
	int yoda_x[] = { 3 ,82, 161, 240, 3, 82 };
	int yoda_y[] = { 3, 3, 3, 3, 94, 94 };

	hare_se_mueve = 0;
	flags[3] = 1;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	flags[1] = 0;

	lee_dibujos("an4y.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (frame = 0; frame < 6; frame++) {
		pausa(3);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(yoda_x[frame], yoda_y[frame], px, py,	78, 90,	dir_hare_frente, dir_zona_pantalla);
		updateScreen(px, py, px, py, 78, 90, dir_zona_pantalla);
	}

	flags[2] = 1;

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::animation_rayo() {
	lee_dibujos("anr_1.alg");
	descomprime_dibujo(dir_hare_frente, MEDIA);
	lee_dibujos("anr_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("anr_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("anr_4.alg");
	descomprime_dibujo(dir_dibujo1, 1);
	lee_dibujos("anr_5.alg");
	descomprime_dibujo(dir_dibujo3, 1);

	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_frente);

	pausa(50);

	comienza_sound("s5.als");

	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_dch);
	pausa(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_fondo);
	pausa(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo1);
	pausa(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_fondo);
	pausa(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo3);
	pausa(3);
	updateScreen(0, 0, 0, 0, 320, 200, dir_hare_frente);
	fin_sound();
}

void DrasculaEngine::animation_2_4() {
	habla_igor_sentado(TEXTI16, "I16.als");
	hablar(TEXT278, "278.als");
	habla_igor_sentado(TEXTI17, "I17.als");
	hablar(TEXT279, "279.als");
	habla_igor_sentado(TEXTI18, "I18.als");
}

void DrasculaEngine::animation_3_4() {
	habla_igor_sentado(TEXTI19, "I19.als");
	habla_igor_sentado(TEXTI20, "I20.als");
	hablar(TEXT281, "281.als");
}

void DrasculaEngine::animation_4_4() {
	hablar(TEXT287, "287.als");
	habla_igor_sentado(TEXTI21, "I21.als");
	hablar(TEXT284, "284.als");
	habla_igor_sentado(TEXTI22, "I22.als");
	hablar(TEXT285, "285.als");
	habla_igor_sentado(TEXTI23, "I23.als");
}

void DrasculaEngine::animation_7_4() {
	Negro();
	hablar(TEXT427, "427.als");
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
		hablar(TEXT430, "430.als");
		habla_bj(TEXTBJ16, "BJ16.als");
		habla_bj(TEXTBJ17, "BJ17.als");
		habla_bj(TEXTBJ18, "BJ18.als");
		hablar(TEXT217, "217.als");
		habla_bj(TEXTBJ19, "BJ19.als");
		hablar(TEXT229, "229.als");
		pausa(5);
		lleva_al_hare(114, 170);
		sentido_hare = 3;
		hablar(TEXT431, "431.als");
		habla_bj(TEXTBJ20, "BJ20.als");
		sentido_hare = 2;
		pausa(4);
		hablar(TEXT438, "438.als");
		sitio_x = 120;
		sitio_y = 157;
		anda_a_objeto = 1;
		sentido_final = 1;
		empieza_andar();
		habla_bj(TEXTBJ21, "BJ21.als");

		for (;;) {
			if (hare_se_mueve == 0)
				break;
			refresca_pantalla();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		}

		sentido_hare = 1;
		hablar(TEXT229, "229.als");
		flags[0] = 1;
	}

	sentido_hare = 1;
	conversa("op_8.cal");
}

void DrasculaEngine::animation_2_5() {
	habla_bj(TEXTBJ22, "BJ22.als");
}

void DrasculaEngine::animation_3_5() {
	habla_bj(TEXTBJ23, "BJ23.als");
	agarra_objeto(10);
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_4_5() {
	flags[7] = 1;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	hablar(TEXT228, "228.als");
	habla_lobo(TEXTL1, "L1.als");
	habla_lobo(TEXTL2, "L2.als");
	pausa(23);
	hablar(TEXT229, "229.als");
	habla_lobo(TEXTL3, "L3.als");
	habla_lobo(TEXTL4, "L4.als");
	hablar(TEXT230, "230.als");
	habla_lobo(TEXTL5, "L5.als");
	hablar(TEXT231, "231.als");
	habla_lobo(TEXTL6, "L6.als");
	habla_lobo(TEXTL7, "L7.als");
	pausa(33);
	hablar(TEXT232, "232.als");
	habla_lobo(TEXTL8, "L8.als");
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
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("3an5_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("3an5_2.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (frame = 0; frame < 9; frame++) {
		pausa(3);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(hueso_x[frame], hueso_y[frame], pixel_x, pixel_y, 97, 64, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(pixel_x, pixel_y, pixel_x,pixel_y, 97,64, dir_zona_pantalla);
	}

	copyBackground(52, 161, 198, 81, 26, 24, dir_dibujo3, dir_zona_pantalla);
	updateScreen(198, 81, 198, 81, 26, 24, dir_zona_pantalla);

	for (frame = 0; frame < 9; frame++) {
		pausa(3);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(hueso_x[frame], hueso_y[frame], pixel_x, pixel_y, 97, 64, dir_hare_frente, dir_zona_pantalla);
		updateScreen(pixel_x, pixel_y, pixel_x,pixel_y, 97, 64, dir_zona_pantalla);
	}

	flags[6] = 1;
	actualiza_datos();
	pausa(12);

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	for (h = 0; h < (200 - 18); h++)
		copyBackground(0, 53, 0, h, 320, 19, dir_hare_frente, dir_zona_pantalla);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("101.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("3an5_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("3an5_4.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo1);
	pausa(9);
	for (frame = 0; frame < 5; frame++) {
		pausa(3);
		copyBackground(vuela_x[frame], 1, 174, 79, 61, 109, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(174, 79, 174, 79, 61, 109, dir_zona_pantalla);
	}
	for (frame = 0; frame < 5; frame++) {
		pausa(3);
		copyBackground(vuela_x[frame], 1, 174, 79, 61, 109, dir_hare_dch, dir_zona_pantalla);
		updateScreen(174, 79, 174, 79, 61, 109, dir_zona_pantalla);
	}
	updateScreen(0, 0, 0, 0, 320, 200, dir_dibujo1);

	comienza_sound("s1.als");
	fin_sound();

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	borra_pantalla();

	lee_dibujos("49.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
}

void DrasculaEngine::animation_6_5() {
	habla_lobo(TEXTL9, "L9.als");
	hablar(TEXT234, "234.als");
}

void DrasculaEngine::animation_7_5() {
	habla_lobo(TEXTL10, "L10.als");
	hablar(TEXT236, "236.als");
	habla_lobo(TEXTL11, "L11.als");
	habla_lobo(TEXTL12, "L12.als");
	habla_lobo(TEXTL13, "L13.als");
	pausa(34);
	habla_lobo(TEXTL14, "L14.als");
}

void DrasculaEngine::animation_8_5() {
	habla_lobo(TEXTL15, "L15.als");
	hablar(TEXT238, "238.als");
	habla_lobo(TEXTL16, "L16.als");
}

void DrasculaEngine::animation_9_5() {
	flags[4] = 1;
	hablar(TEXT401, "401.als");
	sin_verbo();
	resta_objeto(15);
}

void DrasculaEngine::animation_10_5() {
	flags[3] = 1;
	hablar(TEXT401, "401.als");
	sin_verbo();
	resta_objeto(12);
}

void DrasculaEngine::animation_11_5() {
	flags[9] = 1;
	if (flags[2] == 1 && flags[3] == 1 && flags[4] == 1)
		animation_12_5();
	else {
		flags[9] = 0;
		hablar(TEXT33, "33.als");
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

	playmusic(26);
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(27);
	anima("rayo1.bin", 23);
	comienza_sound("s5.als");
	anima("rayo2.bin", 17);
	sentido_hare = 1;
	refresca_pantalla();
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

	lee_dibujos("3an11_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (frame = 0; frame < 8; frame++) {
		if (frame == 2 || frame == 4 || frame == 8 || frame==10)
			setvgapalette256((byte *)&palFondo1);
		else if (frame == 1 || frame == 5 || frame == 7 || frame == 9)
			setvgapalette256((byte *)&palFondo2);
		else
			setvgapalette256((byte *)&palFondo3);

		pausa(4);
		refresca_pantalla();
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

		pausa(4);
		refresca_pantalla();
		copyRect(elfrusky_x[frame], 47, 192, 39, 66, 106, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}

	anima("frel.bin", 16);
	borra_pantalla();
	hare_claro();
	ActualizaPaleta();

	flags[1] = 1;

	animation_13_5();
	comienza_sound("s1.als");
	hipo(12);
	fin_sound();

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	lleva_al_hare(40, 169);
	lleva_al_hare(-14, 175);

	rompo = 1;
	musica_antes = musica_room;
	hare_se_ve = 1;
	borra_pantalla();
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

	lee_dibujos("auxfr.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	pos_frusky[3] = 81;
	pos_frusky[4] = 44;
	pos_frusky[5] = 87;
	pos_frusky[0] = 1;
	pos_frusky[1] = 1;
	pos_frusky[2] = frank_x;
	refresca_pantalla();
	copyRectClip(pos_frusky, dir_hare_fondo, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(15);

	playmusic(18);

	for (;;) {
		refresca_pantalla();
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
		pausa(6);
	}
}

void DrasculaEngine::animation_14_5() {
	flags[11] = 1;
	comienza_sound("s3.als");
	refresca_pantalla();
	updateScreen(0, 0, 0,0 , 320, 200, dir_zona_pantalla);
	fin_sound();
	pausa(17);
	sentido_hare = 3;
	hablar(TEXT246,"246.als");
	lleva_al_hare(89, 160);
	flags[10] = 1;
	comienza_sound("s7.als");
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	pausa(14);
	sentido_hare = 3;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	habla_solo(TEXTD18, "d18.als");
	FundeAlNegro(1);
}

void DrasculaEngine::animation_15_5() {
	habla_mus(TEXTE4, "E4.als");
	habla_mus(TEXTE5, "E5.als");
	habla_mus(TEXTE6, "E6.als");
	hablar(TEXT291, "291.als");
	habla_mus(TEXTE7, "E7.als");
}

void DrasculaEngine::animation_16_5() {
	habla_mus(TEXTE8, "E8.als");
}

void DrasculaEngine::animation_17_5() {
	habla_mus(TEXTE9, "E9.als");
}

void DrasculaEngine::animation_1_6() {
	int l;

	sentido_hare = 0;
	hare_x = 103;
	hare_y = 108;
	flags[0] = 1;
	for (l = 0; l < 200; l++)
		factor_red[l] = 98;

	lee_dibujos("auxig2.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("auxdr.alg");
	descomprime_dibujo(dir_dibujo2, 1);
	lee_dibujos("car.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	habla_dr_dch(TEXTD19, "D19.als");
	hablar(TEXT247, "247.als");
	habla_dr_dch(TEXTD20, "d20.als");
	habla_dr_dch(TEXTD21, "d21.als");
	hablar(TEXT248, "248.als");
	habla_dr_dch(TEXTD22, "d22.als");
	hablar(TEXT249, "249.als");
	habla_dr_dch(TEXTD23, "d23.als");
	conversa("op_11.cal");
	habla_dr_dch(TEXTD26, "d26.als");

	anima("fum.bin", 15);

	habla_dr_dch(TEXTD27, "d27.als");
	hablar(TEXT254, "254.als");
	habla_dr_dch(TEXTD28, "d28.als");
	hablar(TEXT255, "255.als");
	habla_dr_dch(TEXTD29, "d29.als");
	FundeAlNegro(1);
	borra_pantalla();
	lee_dibujos("time1.alg");
	descomprime_dibujo(dir_zona_pantalla, 1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(930);
	borra_pantalla();
	Negro();
	hare_se_ve = 0;
	flags[0] = 0;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(1);
	hablar(TEXT256, "256.als");
	habla_dr_dch(TEXTD30, "d30.als");
	hablar(TEXT257, "257.als");
	FundeAlNegro(0);
	borra_pantalla();
	lee_dibujos("time1.alg");
	descomprime_dibujo(dir_zona_pantalla,1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(900);
	borra_pantalla();
	Negro();
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(1);
	hablar(TEXT258, "258.als");
	habla_dr_dch(TEXTD31, "d31.als");
	animation_5_6();
	habla_dr_dch(TEXTD32, "d32.als");
	habla_igor_dch(TEXTI11, "I11.als");
	sentido_igor = 3;
	habla_dr_dch(TEXTD33, "d33.als");
	habla_igor_frente(TEXTI12, "I12.als");
	habla_dr_dch(TEXTD34, "d34.als");
	sentido_dr = 0;
	habla_dr_izq(TEXTD35, "d35.als");
	borra_pantalla();
	carga_escoba("102.ald");
	activa_pendulo();
}

void DrasculaEngine::animation_2_6() {
	habla_dr_dch(TEXTD24, "d24.als");
}

void DrasculaEngine::animation_3_6() {
	habla_dr_dch(TEXTD24, "d24.als");
}

void DrasculaEngine::animation_4_6() {
	habla_dr_dch(TEXTD25, "d25.als");
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
		actualiza_refresco_antes();
		pos_pen[3] = n;
		copyRectClip(pos_pen, dir_dibujo3, dir_zona_pantalla);

		actualiza_refresco();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(2);
	}

	flags[3] = 1;
}

void DrasculaEngine::animation_6_6() {
	anima("rct.bin", 11);
	borra_pantalla();
	sin_verbo();
	resta_objeto(20);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
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
	playmusic(13);
	flags[5] = 1;
	anima("drf.bin", 16);
	FundeAlNegro(0);
	borra_pantalla();
	hare_x = -1;
	obj_saliendo = 108;
	carga_escoba("59.ald");
	strcpy(num_room, "nada.alg");
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	Negro();
	sentido_hare = 1;
	hare_x -= 21;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	pausa(96);
	lleva_al_hare(116, 178);
	sentido_hare = 2;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	playmusic(9);
	borra_pantalla();
	lee_dibujos("nota.alg");
	descomprime_dibujo(dir_dibujo1, COMPLETA);
	color_abc(BLANCO);
	habla_solo(TEXTBJ24, "bj24.als");
	habla_solo(TEXTBJ25, "bj25.als");
	habla_solo(TEXTBJ26, "bj26.als");
	habla_solo(TEXTBJ27, "bj27.als");
	habla_solo(TEXTBJ28, "bj28.als");
	sentido_hare = 3;
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	hablar(TEXT296, "296.als");
	hablar(TEXT297, "297.als");
	hablar(TEXT298, "298.als");
	sentido_hare = 1;
	hablar(TEXT299, "299.als");
	hablar(TEXT300, "300.als");
	refresca_pantalla();
	copyBackground(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	color_abc(VERDE_CLARO);
	habla_solo("GOOOOOOOOOOOOOOOL", "s15.als");
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, 1);
	sentido_hare = 0;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	hablar(TEXT301, "301.als");
	v_cd = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
	v_cd = v_cd + 4;
	playmusic(17);
	FundeAlNegro(1);
	borra_pantalla();
	fliplay("qpc.bin", 1);
	MusicFadeout();
	stopmusic();
	borra_pantalla();
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, v_cd * 16);
	playmusic(3);
	fliplay("crd.bin", 1);
	stopmusic();
}

void DrasculaEngine::animation_10_6() {
	comienza_sound ("s14.als");
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();
	copyBackground(164, 85, 155, 48, 113, 114, dir_dibujo3, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	habla_taber2(TEXTT23, "t23.als");
	flags[7] = 1;
}

void DrasculaEngine::animation_11_6() {
	habla_taber2(TEXTT10, "t10.als");
	hablar(TEXT268, "268.als");
	habla_taber2(TEXTT11, "t11.als");
}

void DrasculaEngine::animation_12_6() {
	habla_taber2(TEXTT12, "t12.als");
	hablar(TEXT270, "270.als");
	habla_taber2(TEXTT13, "t13.als");
	habla_taber2(TEXTT14, "t14.als");
}

void DrasculaEngine::animation_13_6() {
	habla_taber2(TEXTT15, "t15.als");
}

void DrasculaEngine::animation_14_6() {
	habla_taber2(TEXTT24, "t24.als");
	suma_objeto(21);
	flags[10] = 1;
	rompo_y_salgo = 1;
}

void DrasculaEngine::animation_15_6() {
	habla_taber2(TEXTT16, "t16.als");
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

	actualiza_refresco_antes();
	pon_hare();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(6);
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	comienza_sound("s4.als");
	pausa(6);
	fin_sound();
}

void DrasculaEngine::animation_12_2() {
	lee_dibujos("an12.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT356, "356.als");

	actualiza_refresco_antes();

	copyBackground(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	updateScreen(228, 112, 228, 112, 47, 60, dir_zona_pantalla);

	pausa(3);

	actualiza_refresco_antes();

	copyBackground(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);
	stopmusic();
	flags[11] = 1;

	habla_pianista(TEXTP5, "P5.als");
	conversa("op_1.cal");

	flags[11] = 0;
	lee_dibujos("974.alg");
	descomprime_dibujo(dir_hare_dch, 1);
}

void DrasculaEngine::animation_26_2() {
	int n, x = 0;

	lee_dibujos("an12.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT392, "392.als");

	actualiza_refresco_antes();

	copyBackground(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	updateScreen(228, 112, 228, 112, 47, 60, dir_zona_pantalla);

	pausa(3);

	actualiza_refresco_antes();

	copyBackground(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);
	stopmusic();
	flags[11] = 1;

	habla_pianista(TEXTP5, "P5.als");
	hablar(TEXT393, "393.als");
	habla_pianista(TEXTP17, "P17.als");
	habla_pianista(TEXTP18, "P18.als");
	habla_pianista(TEXTP19, "P19.als");

	lee_dibujos("an26.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	for (n = 0; n < 6; n++){
		x++;
		copyBackground(x, 1, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225,113, 225,113, 50,59, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 61, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225, 113, 225, 113, 50, 59, dir_zona_pantalla);
		x = x + 50;
		if (n == 2)
			comienza_sound("s9.als");
		pausa(3);
	}

	fin_sound_corte();
	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 121, 225, 113, 50, 59, dir_hare_dch, dir_zona_pantalla);
		updateScreen(225, 113, 225, 113, 50, 59, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	agarra_objeto(11);
	resta_objeto(12);

	flags[11] = 0;
	flags[39] = 1;
	lee_dibujos("974.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	musica_room = 16;
}

void DrasculaEngine::animation_11_2() {
	lee_dibujos("an11y13.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT352, "352.als");
	habla_tabernero(TEXTT1, "T1.als");
	hablar(TEXT353, "353.als");
	habla_tabernero(TEXTT17, "T17.als");
	hablar(TEXT354, "354.als");
	habla_tabernero(TEXTT18, "T18.als");
	hablar(TEXT355, "355.als");
	pausa(40);
	habla_tabernero("No, nada", "d82.als");

	lee_dibujos("974.alg");
	descomprime_dibujo(dir_hare_dch, 1);
}

void DrasculaEngine::animation_13_2() {
	lee_dibujos("an11y13.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	if (flags[41] == 0) {
		hablar(TEXT103, "103.als");
		habla_borracho(TEXTB4, "B4.als");
		flags[12] = 1;
		hablar(TEXT367, "367.als");
		habla_borracho(TEXTB5, "B5.als");
		flags[12] = 1;
		hablar(TEXT368, "368.als");
		habla_borracho(TEXTB6, "B6.als");
		habla_borracho(TEXTB7, "B7.als");
		flags[41] = 1;
	}
	conversa("op_2.cal");

	lee_dibujos("964.alg");
	descomprime_dibujo(dir_hare_frente, 1);
}

void DrasculaEngine::animation_18_2() {
	hablar(TEXT378, "378.als");
	habla_vbpuerta(TEXTVB4, "VB4.als");
	conversa("op_3.cal");
}

void DrasculaEngine::animation_22_2() {
	hablar(TEXT374,"374.als");

	sentido_hare=2;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	comienza_sound("s13.als");
	fin_sound();
	sentido_hare = 1;

	habla_vbpuerta(TEXTVB1, "VB1.als");
	hablar(TEXT375, "375.als");
	habla_vbpuerta(TEXTVB2, "VB2.als");
	hablar(TEXT376, "376.als");
	habla_vbpuerta(TEXTVB3, "VB3.als");

	flags[18] = 1;
}

void DrasculaEngine::animation_24_2() {
	if (hare_x < 178)
		lleva_al_hare(208, 136);
	sentido_hare = 3;
	refresca_pantalla();
	pausa(3);
	sentido_hare = 0;

	hablar(TEXT356, "356.als");

	lee_dibujos("an24.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	animation_32_2();

	flags[21] = 1;

	habla_vb(TEXTVB22, "VB22.als");

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

	lee_dibujos("an32_1.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("an32_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(x, 1, 113, 53, 65, 81, dir_dibujo3, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		pausa(4);
	}

	x = 0;
	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(x, 83, 113, 53, 65, 81, dir_dibujo3, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		pausa(4);
	}

	x = 0;
	for (n = 0; n < 4; n++) {
		x++;
		copyBackground(x, 1, 113, 53, 65, 81, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		pausa(4);
	}

	x = 0;
	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 83, 113, 53, 65, 81, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(113, 53, 113, 53, 65, 81, dir_zona_pantalla);
		x = x + 65;
		if (n < 2)
			pausa(4);
	}

	lee_dibujos("aux18.alg");
	descomprime_dibujo(dir_dibujo3, 1);
}

void DrasculaEngine::animation_34_2() {
	int n, x = 0;

	sentido_hare = 1;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("an34_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an34_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 1, 218, 79, 83, 75, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83, 75, dir_zona_pantalla);
		x = x + 83;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 77, 218, 79, 83, 75, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83, 75, dir_zona_pantalla);
		x = x + 83;
		pausa(3);
	}

	x = 0;
	comienza_sound("s8.als");

	for (n = 0; n < 3; n++) {
		x++;
		copyBackground(x, 1, 218, 79, 83, 75,	dir_hare_dch, dir_zona_pantalla);
		updateScreen(218, 79, 218, 79, 83,75, dir_zona_pantalla);
		x = x + 83;
		pausa(3);
	}
	fin_sound();

	pausa(30);

	copyBackground(1, 77, 218, 79, 83, 75, dir_hare_dch, dir_zona_pantalla);
	updateScreen(218, 79, 218, 79, 83, 75, dir_zona_pantalla);
	pausa(3);

	lee_dibujos("994.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("974.alg");
	descomprime_dibujo(dir_hare_dch, 1);
}

void DrasculaEngine::animation_36_2() {
	lee_dibujos("an11y13.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT404, "404.als");
	habla_tabernero(TEXTT19, "T19.als");
	habla_tabernero(TEXTT20, "T20.als");
	habla_tabernero(TEXTT21, "T21.als");
	hablar(TEXT355, "355.als");
	pausa(40);
	habla_tabernero("No, nada", "d82.als");

	lee_dibujos("974.alg");
	descomprime_dibujo(dir_hare_dch, 1);
}

void DrasculaEngine::animation_7_2() {
	int n, x = 0;

	lee_dibujos("an7_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an7_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an7_3.alg");
	descomprime_dibujo(dir_hare_frente, 1);

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
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64,	51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}
	lee_dibujos("an7_4.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an7_5.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an7_6.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an7_7.alg");
	descomprime_dibujo(dir_dibujo3, 1);

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1,	80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1,	80, 64, 51,73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64,	51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_frente, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 2; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}
	lee_dibujos("an7_8.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an7_9.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64,	51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 75, 80, 64, 51, 73, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
		copyRect(x, 1, 80, 64, 51, 73,	dir_hare_dch, dir_zona_pantalla);
		updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);
		x = x + 51;
		pausa(3);
	}


	copyBackground(80, 64, 80, 64, 51, 73, dir_dibujo1, dir_zona_pantalla);
	copyRect(1, 75, 80, 64, 51, 73, dir_hare_dch, dir_zona_pantalla);
	updateScreen(80, 64, 80, 64, 51, 73, dir_zona_pantalla);

	flags[37] = 1;

	if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
		flags[38] = 1;

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("aux3.alg");
	descomprime_dibujo(dir_dibujo3, 1);
}

void DrasculaEngine::animation_5_2() {
	int n, x = 0;

	sentido_hare = 0;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("an5_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an5_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an5_3.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an5_4.alg");
	descomprime_dibujo(dir_dibujo3, 1);

	copyBackground(1, 1, 213, 66,	53,84, dir_hare_fondo, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66, 53, 84, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84,dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_fondo, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66, 53, 84, dir_hare_dch, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66,	53, 84, dir_hare_dch, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_dch, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66,	53, 84, dir_hare_frente, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	comienza_sound("s1.als");

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 86, 213, 66, 53, 84, dir_hare_frente, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}
	fin_sound_corte();

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		copyBackground(x, 1, 213, 66, 53, 84,	dir_dibujo3, dir_zona_pantalla);
		updateScreen(213, 66, 213, 66, 53, 84, dir_zona_pantalla);
		x = x + 52;
		pausa(3);
	}

	lee_dibujos("994.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("974.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("964.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("aux5.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	flags[8] = 1;
	hare_x = hare_x - 4;
	hablar_sinc(TEXT46, "46.als", "4442444244244");
	sin_verbo();
}

void DrasculaEngine::animation_6_2() {
	stopmusic();
	flags[9] = 1;

	borra_pantalla();
	lee_dibujos("ciego1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("ciego2.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("ciego3.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("ciego4.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("ciego5.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(1);

	if (flags[4] == 1)
		habla_hacker(TEXTD66, "d66.als");
	pausa(6);
	habla_ciego(TEXTD78, "d78.als",
				"004447222550002222755554444466222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777");
	pausa(4);
	habla_hacker(TEXTD67, "d67.als");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(10);

	borra_pantalla();

	playmusic(musica_room);
	lee_dibujos("9.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("aux9.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sin_verbo();

	flags[9] = 0;
}

void DrasculaEngine::animation_33_2() {
	stopmusic();
	flags[9] = 1;

	pausa(12);
	hablar(TEXTD56, "d56.als" );
	pausa(8);

	borra_pantalla();
	lee_dibujos("ciego1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("ciego2.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("ciego3.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("ciego4.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("ciego5.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(10);

	habla_ciego(TEXTD68, "d68.als", "44472225500022227555544444472225500022227555544444664466");
	pausa(5);
	habla_hacker(TEXTD57, "d57.als");
	pausa(6);
	_system->delayMillis(1000);
	habla_ciego(TEXTD77, "d77.als", "56665004444447222550002222755554444466555033022755555000444444444444444444444444444444");
	habla_hacker(TEXTD65, "d65.als");

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(14);

	borra_pantalla();

	playmusic(musica_room);
	lee_dibujos("9.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("aux9.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sin_verbo();

	flags[33] = 1;
	flags[9] = 0;
}

void DrasculaEngine::animation_1_4() {
	if (flags[21] == 0) {
		strcpy(nombre_obj[2], "igor");
		hablar(TEXT275, "275.als");

		actualiza_refresco_antes();

		copyBackground(131, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		updateScreen(199, 95, 199, 95, 50, 66, dir_zona_pantalla);

		pausa(3);

		actualiza_refresco_antes();

		copyBackground(182, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		pon_hare();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		pausa(3);
		flags[18] = 1;
		flags[20] = 1;

		habla_igor_sentado(TEXTI13, "I13.als");
		habla_igor_sentado(TEXTI14, "I14.als");
		habla_igor_sentado(TEXTI15, "I15.als");
		flags[21] = 1;
	} else {
		hablar(TEXT356, "356.als");

		actualiza_refresco_antes();

		copyBackground(131, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		updateScreen(199, 95, 199, 95, 50, 66, dir_zona_pantalla);
		pausa(2);

		actualiza_refresco_antes();

		copyBackground(182, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
		pon_hare();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		flags[18] = 1;
		flags[20] = 1;

		hablar(TEXT276, "276.als");
		pausa(14);
		habla_igor_sentado(TEXTI6, "I6.als");
	}

	conversa("op_6.cal");
	flags[20] = 0;
	flags[18] = 0;
}

void DrasculaEngine::animation_5_4(){
	sentido_hare = 3;
	lee_dibujos("anh_dr.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lleva_al_hare(99, 160);
	lleva_al_hare(38, 177);
	hare_se_ve = 0;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(800);
	anima("bio.bin", 14);
	flags[29] = 1;
	hare_x = 95;
	hare_y = 82;
	refresca_pantalla();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	abre_puerta(2, 0);
	lee_dibujos("auxigor.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	x_igor = 100;
	y_igor = 65;
	habla_igor_frente(TEXTI29, "I29.ALS");
	habla_igor_frente(TEXTI30, "I30.als");
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	hare_se_ve = 1;
	FundeAlNegro(0);
	sal_de_la_habitacion(0);
}

void DrasculaEngine::animation_6_4() {
	char room[13];

	strcpy(room, num_room);
	strcpy(num_room, "26.alg");
	borra_pantalla();
	lee_dibujos("26.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("aux26.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("auxigor.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	refresca_26_antes();
	x_igor = 104;
	y_igor = 71;
	pon_igor();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(40);
	habla_igor_frente(TEXTI26, "I26.als");
	strcpy(num_room, room);
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos(pantalla_disco);
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);
	sin_verbo();
	refresca_pantalla();
}

void DrasculaEngine::animation_8_4() {
	int frame;
	int estanteria_x[] = {1, 75, 149, 223, 1, 75, 149, 223, 149, 223, 149, 223, 149, 223};
	int estanteria_y[] = {1, 1, 1, 1, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74};

	lee_dibujos("an_8.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (frame = 0; frame < 14; frame++) {
		pausa(2);
		copyBackground(estanteria_x[frame], estanteria_y[frame], 77, 45, 73, 72, dir_hare_frente, dir_zona_pantalla);
		updateScreen(77, 45, 77, 45, 73, 72, dir_zona_pantalla);
	}

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	abre_puerta(7, 2);
}

void DrasculaEngine::animation_9_4() {
	anima("st.bin", 14);
	FundeAlNegro(1);
}


} // End of namespace Drascula
