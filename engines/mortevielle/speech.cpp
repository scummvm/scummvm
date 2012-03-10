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
#include "common/file.h"
#include "mortevielle/speech.h"
#include "mortevielle/sound.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mor.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void spfrac(int wor) {
	g_c3._rep = (uint)wor >> 12;
	if ((g_typlec == 0) && (g_c3._code != 9))
		if (((g_c3._code > 4) && (g_c3._val != 20) && (g_c3._rep != 3) && (g_c3._rep != 6) && (g_c3._rep != 9)) ||
				((g_c3._code < 5) && ((g_c3._val != 19) && (g_c3._val != 22) && (g_c3._rep != 4) && (g_c3._rep != 9)))) {
			++g_c3._rep;
		}

	g_c3._freq = ((uint)wor >> 6) & 7;
	g_c3._acc = ((uint)wor >> 9) & 7;
}

void charg_car(int &currWordNumb) {
	int wor, int_;

	wor = swap(READ_LE_UINT16(&g_mem[adword + currWordNumb]));
	int_ = wor & 0x3f;

	if ((int_ >= 0) && (int_ <= 13)) {
		g_c3._val = int_;
		g_c3._code = 5;
	} else if ((int_ >= 14) && (int_ <= 21)) {
		g_c3._val = int_;
		g_c3._code = 6;
	} else if ((int_ >= 22) && (int_ <= 47)) {
		int_ = int_ - 22;
		g_c3._val = int_;
		g_c3._code = g_typcon[int_];
	} else if ((int_ >= 48) && (int_ <= 56)) {
		g_c3._val = int_ - 22;
		g_c3._code = 4;
	} else {
		switch (int_) {
		case 60:
			g_c3._val = 32;  /*  " "  */
			g_c3._code = 9;
			break;
		case 61:
			g_c3._val = 46;  /*  "."  */
			g_c3._code = 9;
			break;
		case 62:
			g_c3._val = 35;  /*  "#"  */
			g_c3._code = 9;
		default:
			break;
		}
	}

	spfrac(wor);
	currWordNumb += 2;
}


void entroct(byte o) {
	g_mem[adtroct * 16 + g_ptr_oct] = o;
	++g_ptr_oct;
}

void veracf(byte b) {
	;
}

void cctable(tablint &t) {
	float tb[257];

	tb[0] = 0;
	for (int k = 0; k <= 255; ++k) {
		tb[k + 1] = g_addfix + tb[k];
		t[255 - k] = abs((int)tb[k] + 1);
	}
}

void regenbruit() {
	int i = offsetb3 + 8590;
	int j = 0;
	do {
		g_t_cph[j] = READ_LE_UINT16(&g_mem[adbruit3 + i]);
		i += 2;
		++j;
	} while (i < offsetb3 + 8790);
}

void charge_son() {
	Common::File f;

	if (!f.open("sonmus.mor"))
		error("Missing file - sonmus.mor");
	
	f.read(&g_mem[0x7414 * 16 + 0], 273);

	g_vm->_soundManager.decodeMusic(&g_mem[0x7414 * 16], &g_mem[adson * 16], 273);
	f.close();
}

void charge_phbruit() {
	Common::File f;

	if (!f.open("phbrui.mor"))
		error("Missing file - phbrui.mor");

	for (int i = 1; i <= 3; ++i)
		g_t_cph[i] = f.readSint16LE();

	f.close();
}

void charge_bruit() {
	Common::File f;
	int i;

	if (!f.open("bruits"))               //Translation: "noise"
		error("Missing file - bruits");

	f.read(&g_mem[adbruit * 16 + 0], 250);
	for (i = 0; i <= 19013; ++i)
		g_mem[adbruit * 16 + 32000 + i] = g_mem[adbruit5 + i];
	f.read(&g_mem[adbruit1 * 16 + offsetb1], 149);

	f.close();
}

void trait_car() {
	byte d3;
	int d2, i;

	switch (g_c2._code) {
	case 9:
		if (g_c2._val != ord('#'))
			for (i = 0; i <= g_c2._rep; ++i)
				entroct(g_c2._val);
		break;
	case 5:
	case 6:
		if (g_c2._code == 6)
			d3 = g_tabdph[(g_c2._val - 14) << 1];
		else
			d3 = null;
		if (g_c1._code >= 5) {
			veracf(g_c2._acc);
			if (g_c1._code == 9) {
				entroct(4);
				if (d3 == null)
					entroct(g_c2._val);
				else
					entroct(d3);
				entroct(22);
			}
		}

		switch (g_c2._rep) {
		case 0:
			entroct(0);
			entroct(g_c2._val);
			if (d3 == null)
				if (g_c3._code == 9)
					entroct(2);
				else
					entroct(4);
			else if (g_c3._code == 9)
				entroct(0);
			else
				entroct(1);
			break;
		case 4:
		case 5:
		case 6:
			if (g_c2._rep != 4) {
				i = g_c2._rep - 5;
				do {
					--i;
					entroct(0);
					if (d3 == null)
						entroct(g_c2._val);
					else
						entroct(d3);
					entroct(3);
				} while (i >= 0);
			}
			if (d3 == null) {
				entroct(4);
				entroct(g_c2._val);
				entroct(0);
			} else {
				entroct(0);
				entroct(g_c2._val);
				entroct(3);
			}
			break;
		case 7:
		case 8:
		case 9:
			if (g_c2._rep != 7) {
				i = g_c2._rep - 8;
				do {
					--i;
					entroct(0);
					if (d3 == null)
						entroct(g_c2._val);
					else
						entroct(d3);
					entroct(3);
				} while (i >= 0);
			}
			if (d3 == null) {
				entroct(0);
				entroct(g_c2._val);
				entroct(2);
			} else {
				entroct(0);
				entroct(g_c2._val);
				entroct(0);
			}
			break;
		case 1:
		case 2:
		case 3:
			if (g_c2._rep != 1) {
				i = g_c2._rep - 2;
				do {
					--i;
					entroct(0);
					if (d3 == null)
						entroct(g_c2._val);
					else
						entroct(d3);
					entroct(3);
				} while (i >= 0);
			}
			entroct(0);
			entroct(g_c2._val);
			if (g_c3._code == 9)
				entroct(0);
			else
				entroct(1);
			break;
		default:
			break;
		}     //  switch  c2.rep
		break;

	case 2:
	case 3:
		d3 = g_c2._code + 5; //  7 ou 8  => Corresponding vowel
		if (g_c1._code > 4) {
			veracf(g_c2._acc);
			if (g_c1._code == 9) {
				entroct(4);
				entroct(d3);
				entroct(22);
			}
		}
		i = g_c2._rep;
		assert(i >= 0);
		if (i != 0) {
			do {
				--i;
				entroct(0);
				entroct(d3);
				entroct(3);
			} while (i > 0);
		}
		veracf(g_c3._acc);
		if (g_c3._code == 6) {
			entroct(4);
			entroct(g_tabdph[(g_c3._val - 14) << 1]);
			entroct(g_c2._val);
		} else {
			entroct(4);
			if (g_c3._val == 4)
				entroct(3);
			else
				entroct(g_c3._val);
			entroct(g_c2._val);
		}
		break;
	case 0:
	case 1: 
		veracf(g_c2._acc);
		switch (g_c3._code) {
		case 2:
			d2 = 7;
			break;
		case 3:
			d2 = 8;
			break;
		case 6:
			d2 = g_tabdph[(g_c3._val - 14) << 1];
			break;
		case 5:
			d2 = g_c3._val;
			break;
		default:
			d2 = 10;
			break;
		}       //  switch  c3._code
		d2 = (d2 * 26) + g_c2._val;
		if (g_tnocon[d2] == 0)
			d3 = 2;
		else
			d3 = 6;
		if (g_c2._rep >= 5) {
			g_c2._rep = g_c2._rep - 5;
			d3 = 8 - d3;       // Swap 2 and 6
		}
		if (g_c2._code == 0) {
			i = g_c2._rep;
			if (i != 0) {
				do {
					--i;
					entroct(d3);
					entroct(g_c2._val);
					entroct(3);
				} while (i > 0);
			}
			entroct(d3);
			entroct(g_c2._val);
			entroct(4);
		} else {
			entroct(d3);
			entroct(g_c2._val);
			entroct(3);
			i = g_c2._rep;
			if (i != 0) {
				do {
					--i;
					entroct(d3);
					entroct(g_c2._val);
					entroct(4);
				} while (i > 0);
			}
		}
		if (g_c3._code == 9) {
			entroct(d3);
			entroct(g_c2._val);
			entroct(5);
		} else if ((g_c3._code != 0) && (g_c3._code != 1) && (g_c3._code != 4)) {
			veracf(g_c3._acc);
			switch (g_c3._code) {
			case 3:
				d2 = 8;
				break;
			case 6:
				d2 = g_tabdph[(g_c3._val - 14) << 1];
				break;
			case 5:
				d2 = g_c3._val;
				break;
			default:
				d2 = 7;
				break;
			}     //  switch c3._code
			if (d2 == 4)
				d2 = 3;

			if (g_intcon[g_c2._val] != 0)
				++g_c2._val;

			if ((g_c2._val == 17) || (g_c2._val == 18))
				g_c2._val = 16;

			entroct(4);
			entroct(d2);
			entroct(g_c2._val);
		}
	
		break;
	case 4:
		veracf(g_c2._acc);
		i = g_c2._rep;
		if (i != 0) {
			do {
				--i;
				entroct(2);
				entroct(g_c2._val);
				entroct(3);
			} while (i > 0);
		}
		entroct(2);
		entroct(g_c2._val);
		entroct(4);
		if (g_c3._code == 9) {
			entroct(2);
			entroct(g_c2._val);
			entroct(5);
		} else if ((g_c3._code != 0) && (g_c3._code != 1) && (g_c3._code != 4)) {
			veracf(g_c3._acc);
			switch (g_c3._code) {
			case 3:
				d2 = 8;
				break;
			case 6:
				d2 = g_tabdph[(g_c3._val - 14) << 1];
				break;
			case 5:
				d2 = g_c3._val;
				break;
			default:
				d2 = 7;
				break;
			}     //  switch c3._code

			if (d2 == 4)
				d2 = 3;

			if (g_intcon[g_c2._val] != 0)
				++g_c2._val;

			entroct(4);
			entroct(d2);
			entroct(g_tabdbc[((g_c2._val - 26) << 1) + 1]);
		}
	
		break;
	default:
		break;
	}     // switch c2.code
}

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

	int ptr_tcph = g_num_ph - 1;
	int startPos = swap(g_t_cph[ptr_tcph]) + deca[g_typlec];
	int endPos = swap(g_t_cph[ptr_tcph + 1]) + deca[g_typlec];
	int wordCount = endPos - startPos;
	for (int i = (uint)startPos >> 1, currWord = 0; i < (int)((uint)endPos >> 1); i++, currWord += 2)
		WRITE_LE_UINT16(&g_mem[adword + currWord], g_t_cph[i]);

	g_ptr_oct = 0;
	int currWord = 0;
	init_chariot();

	do {
		rot_chariot();
		charg_car(currWord);
		trait_car();
	} while (currWord < wordCount);

	rot_chariot();
	trait_car();
	entroct(ord('#'));
}



void startSpeech(int rep, int ht, int typ) {
	int savph[501];
	int tempo;

	if (g_vm->_soundOff)
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
