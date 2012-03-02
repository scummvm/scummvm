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
	c1 = c2;
	c2 = c3;
	c3.val = 32;
	c3.code = 9;
}

void init_chariot() {
	c3.rep = 0;
	c3.freq = 0;
	c3.acc = 0;
	rot_chariot();
	rot_chariot();
}


void trait_ph() {
	const int deca[3] = {300, 30, 40};

	ptr_tcph = num_ph - 1;
	ledeb = swap(t_cph[ptr_tcph]) + deca[typlec];
	lefin = swap(t_cph[ptr_tcph + 1]) + deca[typlec];
	nb_word = lefin - ledeb;
	ptr_tcph = (uint)ledeb >> 1;
	ptr_word = 0;
	do {
		WRITE_LE_UINT16(&mem[adword + ptr_word], t_cph[ptr_tcph]);
		ptr_word = ptr_word + 2;
		ptr_tcph = ptr_tcph + 1;
	} while (ptr_tcph < (int)((uint)lefin >> 1));

	ptr_oct = 0;
	ptr_word = 0;
	init_chariot();

	do {
		rot_chariot();
		charg_car();
		trait_car();
	} while (ptr_word < nb_word);

	rot_chariot();
	trait_car();
	entroct(ord('#'));
}



void parole(int rep, int ht, int typ) {
	int savph[501];
	int tempo;

	if (g_soundOff)
		return;

	num_ph = rep;
	g_haut = ht;
	typlec = typ;
	if (typlec != 0) {
		for (int i = 0; i <= 500; ++i)
			savph[i] = t_cph[i];
		tempo = tempo_bruit;
	} else if (g_haut > 5)
		tempo = tempo_f;
	else
		tempo = tempo_m;
	addfix = (float)((tempo - addv[0])) / 256;
	cctable(tbi);
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
	g_vm->_soundManager.litph(tbi, typ, tempo);
	if (typlec != 0)
		for (int i = 0; i <= 500; ++i) {
			t_cph[i] = savph[i];
			mlec = typlec;
		}
	writepal(numpal);
}

} // End of namespace Mortevielle
