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
#include "mortevielle/sprint.h"
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


const byte don[9][5] = {
	{ 0,  0,  0,  0, 0},
	{ 0,  7, 37, 22,  8},
	{0,  19, 33, 23,  7},
	{0,  31, 89, 10, 21},
	{0,  43, 25, 11,  5},
	{0,  55, 37,  5,  8},
	{0,  64, 13, 11,  2},
	{0,  62, 22, 13,  4},
	{0,  62, 25, 13,  5}
};

const byte addv[2] = {8, 8};

const byte rang[16] = {15, 14, 11, 7, 13, 12, 10, 6, 9, 5, 3, 1, 2, 4, 8, 0};

/*---------------------------------------------------------------------------*/
/*------------------------------   VARIABLES   ------------------------------*/
/*---------------------------------------------------------------------------*/

bool blo,
        bh1,
        bf1,
        bh2,
        bh4,
        bf4,
        bh5,
        bh6,
        bh8,
        bt3,
        bt7,
        bh9,

        sonoff,
        main1,
        choisi,
        test0,
        f2_all,
        imen,
        cache,
        iesc,
        col,
        syn,
        fouil,
        zuul,
        tesok,
        obpart,
        okdes,
        arret,
        anyone,
        brt,
        rect,
        rech_cfiec,
        active_menu;


int x,
        y,
        t,
        vj,
        li,
        vh,
        vm,
        jh,
        mh,
        cs,
        gd,                /*  Gd = graph device  */
        hdb,
        hfb,
        heu,
        jou,
        key,
        min,
        num,
        max,
        res,
        ment,
        haut,
        caff,
        maff,
        crep,
        ades,
        iouv,
        inei,
        ctrm,
        dobj,
        msg3,
        msg4,
        mlec,
        newgd,
        c_zzz,
        mchai,
        menup,
        ipers,
        ledeb,
        lefin,
        mpers,
        mnumo,
        xprec,
        yprec,
        perdep,
        prebru,
        typlec,
        num_ph,
        numpal,
        lderobj,
        nb_word,
        ptr_oct,
        k_tempo,
        ptr_tcph,
        ptr_word,
        color_txt;

int t_cph[6401];
byte tabdon[4001];
/*   t_dxx  : array[0..121] of int;*/
Common::String stpou;               /* donne le % d'indices */ //Translation: "Provides the % of hints"
byte is;
char mode;
Common::String al_mess,
       err_mess,
       ind_mess,
       al_mess2;

int invt[9];
int nbrep[9];
int nbrepm[9];
int disc[9];
int msg[5];
int depl[8];
Common::String inv[9];
Common::String dep[8];
Common::String act[22];
Common::String self_[7];
Common::String dis[9];
char touv[8];
sav_chaine s, s1;
byte bufcha[391];

byte lettres[7][24];

byte palher[16];

uint16 t_mot[maxti + 1];
int tay_tchar;
ind t_rec[maxtd + 1];
//file<ind> sauv_t;
//untyped_file fibyte;
byte v_lieu[8][25];
int l[108];
int tbi[256];
chariot c1, c2, c3;
float addfix;
t_pcga palsav[91];
doublet tabpal[91][17];
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
	delete tmp;
	return result;
}

/*---------------------------------------------------------------------------*/
/*------------------------------     STUBS     ------------------------------*/
/*---------------------------------------------------------------------------*/

int port[0xfff];
byte mem[65536 * 16];
int wherey;

void gotoxy(int x, int y) {
}
void textcolor(int c) {
}
void output(const Common::String &s) {
	debug(s.c_str());
}
void graphbackground(int c) {
}

void intr(int intNum, registres &regs) {
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
	// is being handled slightly differently now in the menu_up() method
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
