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

#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/*---------------------------------------------------------------------------*/
/*------------------------------   CONSTANTS   ------------------------------*/
/*---------------------------------------------------------------------------*/

const byte tabdbc[18] = {7, 23, 7, 14, 13, 9, 14, 9, 5, 12, 6, 12, 13, 4, 0, 4, 5, 9};
const byte tabdph[16] = {0, 10, 2, 0, 2, 10, 3, 0, 3, 7, 5, 0, 6, 7, 7, 10};
const byte typcon[26] = {0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};
const byte intcon[26] = {1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
const byte tnocon[364] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


const byte _menuConstants[8][4] = {
	{ 7, 37, 22,  8},
	{19, 33, 23,  7},
	{31, 89, 10, 21},
	{43, 25, 11,  5},
	{55, 37,  5,  8},
	{64, 13, 11,  2},
	{62, 22, 13,  4},
	{62, 25, 13,  5}
};

const byte addv[2] = {8, 8};

const byte rang[16] = {15, 14, 11, 7, 13, 12, 10, 6, 9, 5, 3, 1, 2, 4, 8, 0};

/*---------------------------------------------------------------------------*/
/*------------------------------   VARIABLES   ------------------------------*/
/*---------------------------------------------------------------------------*/

bool g_blo,
        g_bh1,
        g_bf1,
        g_bh2,
        g_bh4,
        g_bf4,
        g_bh5,
        g_bh6,
        g_bh8,
        g_bt3,
        g_bt7,
        g_bh9,

        g_soundOff,
        g_main1,
        g_choisi,
        g_test0,
        g_f2_all,
        g_imen,
        g_cache,
        g_iesc,
        g_col,
        g_syn,
        g_fouil,
        g_zuul,
        g_tesok,
        g_obpart,
        g_okdes,
        g_arret,
        g_anyone,
        g_brt,
        g_rect,
        g_rech_cfiec;


int g_x,
        g_y,
        g_t,
        g_vj,
        g_li,
        g_vh,
        g_vm__,
        g_jh,
        g_mh,
        g_cs,
        g_currGraphicalDevice,
        g_hdb,
        g_hfb,
        g_hour,
        g_day,
        g_key,
        g_minute,
        g_num,
        g_res,
        g_ment,
        g_haut,
        g_caff,
        g_maff,
        g_crep,
        g_ades,
        g_iouv,
        g_inei,
        g_ctrm,
        g_dobj,
        g_msg3,
        g_msg4,
        g_mlec,
        g_newGraphicalDevice,
        g_c_zzz,
        g_mchai,
        g_menup,
        g_ipers,
        g_ledeb,
        g_lefin,
        g_mpers,
        g_mnumo,
        g_xprec,
        g_yprec,
        g_perdep,
        g_prebru,
        g_typlec,
        g_num_ph,
        g_numpal,
        g_nb_word,
        g_ptr_oct,
        g_k_tempo,
        g_ptr_tcph,
        g_ptr_word,
        g_color_txt;

int t_cph[6401];
byte tabdon[4001];

Common::String _hintPctMessage;               // Provides the % of hints
byte is;
byte mode;

int nbrep[9];
int nbrepm[9];
int g_msg[5];
byte touv[8];
sav_chaine g_s, s1;
byte bufcha[391];

byte lettres[7][24];

byte palher[16];

uint16 t_mot[maxti + 1];
int tay_tchar;
ind t_rec[maxtd + 1];
//file<ind> sauv_t;
//untyped_file fibyte;
byte v_lieu[8][25];
int g_l[108];
int tbi[256];
chariot c1, c2, c3;
float addfix;
t_pcga palsav[91];
Common::Point tabpal[91][17];
t_pcga palcga[91];
pattern tpt[15];

byte adcfiec[(4088 * 16) + (311 * 0x80)];

/*---------------------------------------------------------------------------*/
/*--------------------   PROCEDURES  AND  FONCTIONS   -----------------------*/
/*---------------------------------------------------------------------------*/

void hirs() {
	// Note: The original used this to set the graphics mode and clear the screen, both at
	// the start of the game, and whenever the screen need to be cleared. As such, this
	// method is deprecated in favour of clearing the screen
	debugC(1, kMortevielleCore, "TODO: hirs is deprecated in favour of ScreenSurface::clearScreen");

	g_vm->_screenSurface.clearScreen();
}

/**
 * Returns a substring of the given string
 * @param s		Source string
 * @param idx	Starting index (1 based)
 * @param size	Number of characters to return
 */
Common::String copy(const Common::String &s, int idx, size_t size) {
	// Copy the substring into a temporary buffer
	char *tmp = new char[size + 1];
	strncpy(tmp, s.c_str() + idx - 1, size);
	tmp[size] = '\0';

	Common::String result(tmp);
	delete[] tmp;
	return result;
}

/*---------------------------------------------------------------------------*/
/*------------------------------     STUBS     ------------------------------*/
/*---------------------------------------------------------------------------*/

int port[0xfff];
byte mem[65536 * 16];

void intr(int intNum, registres &regs) {
	warning("STUBBED: Call to DOS interrupt #%d", intNum);
}

/**
 * Get a random number between two values
 */
int get_random_number(int minval, int maxval) {
	return g_vm->_randomSource.getRandomNumber(maxval - minval) + minval;
}

bool keypressed() {
	return g_vm->keyPressed();
}
char get_ch() {
	return g_vm->getChar();
}

/**
 * Restore a screen sectio
 */
void s_sauv(int Gd, int y, int dy) {
	// Note: May no longer be necessary, since the backgorund restore code 
	// is being handled slightly differently now in the menuUp() method
}

void palette(int v1) {
	warning("TODO: palette");
}

/**
 * Restore a screen area
 * @remarks	Currently implemented differently in ScummVM, so this method is redundant.
 * Original assembly was present in 'charecr.asm'
 */
void s_char(int Gd, int y, int dy) {
	debugC(1, kMortevielleCore, "s_char called");
}

// (* external 'c:\mc\reusint.com'; *)
void musyc(tablint &tb, int nbseg, int att) {
	warning("TODO: musyc");
}

} // End of namespace Mortevielle
