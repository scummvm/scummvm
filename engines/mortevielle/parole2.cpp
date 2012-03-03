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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/endian.h"
#include "mortevielle/level15.h"
#include "mortevielle/parole2.h"
#include "mortevielle/parole.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/sound.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void rot_chariot() {
	g_c1 = g_c2;
	g_c2 = g_c3;
	g_c3._val = 32;
	g_c3._code = 9;
}

void init_chariot() {
	g_c3._rep = 0;
	g_c3._freq = 0;
	g_c3._acc = 0;
	rot_chariot();
	rot_chariot();
}


void trait_ph() {
	const int deca[3] = {300, 30, 40};

	g_ptr_tcph = g_num_ph - 1;
	g_ledeb = swap(g_t_cph[g_ptr_tcph]) + deca[g_typlec];
	g_lefin = swap(g_t_cph[g_ptr_tcph + 1]) + deca[g_typlec];
	g_nb_word = g_lefin - g_ledeb;
	g_ptr_tcph = (uint)g_ledeb >> 1;
	g_ptr_word = 0;
	do {
		WRITE_LE_UINT16(&g_mem[adword + g_ptr_word], g_t_cph[g_ptr_tcph]);
		g_ptr_word += 2;
		++g_ptr_tcph;
	} while (g_ptr_tcph < (int)((uint)g_lefin >> 1));

	g_ptr_oct = 0;
	g_ptr_word = 0;
	init_chariot();

	do {
		rot_chariot();
		charg_car();
		trait_car();
	} while (g_ptr_word < g_nb_word);

	rot_chariot();
	trait_car();
	entroct(ord('#'));
}



void parole(int rep, int ht, int typ) {
	int savph[501];
	int tempo;

	if (g_soundOff)
		return;

	g_num_ph = rep;
	g_haut = ht;
	g_typlec = typ;
	if (g_typlec != 0) {
		for (int i = 0; i <= 500; ++i)
			savph[i] = g_t_cph[i];
		tempo = kTempoNoise;
	} else if (g_haut > 5)
		tempo = kTempoF;
	else
		tempo = kTempoM;
	g_addfix = (float)((tempo - g_addv[0])) / 256;
	cctable(g_tbi);
	switch (typ) {
	case 1:
		charge_bruit();
		/*if zuul then zzuul(adbruit,0,1095);*/
		regenbruit();
		break;
	case 2:
		charge_son();
		charge_phbruit();
		break;
	default:
		break;
	}
	trait_ph();
	g_vm->_soundManager.litph(g_tbi, typ, tempo);
	if (g_typlec != 0)
		for (int i = 0; i <= 500; ++i) {
			g_t_cph[i] = savph[i];
			g_mlec = g_typlec;
		}
	writepal(g_numpal);
}

} // End of namespace Mortevielle
