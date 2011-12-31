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

#include "common/str.h"
#include "common/textconsole.h"
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

const char *fic[3] = {
	"", 
	" Sauvegarde",
	 " Chargement"
};

const byte addv[2] = {8, 8};

const char recom[] = " Recommence  ";

const char f3[] = "F3: Encore";
const char f8[] = "F8: Suite";

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
        perdu,
        col,
        syn,
        fouil,
        zuul,
        tesok,
        obpart,
        okdes,
        solu,
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
        xwhere,
        ywhere,
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
Common::String stpou;                               /* donne le % d'indices */
byte is;
char mode;
Common::String al_mess,
       err_mess,
       ind_mess,
       al_mess2;

int invt[8];
int nbrep[8];
int nbrepm[8];
int disc[8];
int msg[5];
int depl[7];
Common::String inv[9];
Common::String dep[8];
Common::String act[22];
Common::String self_[6];
Common::String dis[9];
char touv[7];
sav_chaine s, s1;
byte bufcha[391];

byte lettres[7][24];

byte palher[16];

int t_mot[maxti + 1];
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
doublet tabpal[91][16];
t_pcga palcga[91];
pattern tpt[15];

/*---------------------------------------------------------------------------*/
/*--------------------   PROCEDURES  AND  FONCTIONS   -----------------------*/
/*---------------------------------------------------------------------------*/

void hirs() {
	const byte tandy[14] = {113, 80, 90, 14, 63, 6, 50, 56, 2, 3, 6, 7, 0, 0};
	const byte herc[13] = {50, 40, 41, 9, 103, 3, 100, 100, 2, 3, 0, 0, 0};
	int i, j;

	switch (gd) {
	case cga : {
		graphcolormode;
		graphbackground(0);
		palette(1);
		res = 1;
	}
	break;
	case ams : {
		hires;
		/* 
		inline_((float)(0xb8) / 6 / 0 / //  =>  mov ax,6
		        0xcd / 0x10);			//  =>  int 16
		port[0x3d9] = 15;
		port[0x3df] = 0;
		port[0x3dd] = 15;
		*/
		res = 2;
	}
	break;
	case ega : {
		inline_((float)(0xb8) / 14 / 0 /     /*  MOV AX, 14   ; mode video 14 = 640*200 16 couleurs */
		        0xcd / 0x10);        /*  INT 16  */
		res = 2;
	}
	break;
	case her : {
		port[0x3bf] = 3;
		port[0x3b8] = 2;
		for (i = 0; i <= 12; i ++) {
			port[0x3b4] = i;
			port[0x3b5] = herc[i];
		}
		inline_((float)(0xfc) / 0xb9 / 0 / 0x80 / 0xb8 / 0 / 0xb0 / 0x8e / 0xc0 / 0x31 / 0xff / 0x31 / 0xc0 / 0xf3 / 0xab);
		port[0x3b8] = 10;
		res = 2;
	}
	break;
	case tan : {
		port[0x3d8] = 0;
		port[0x3da] = 3;
		port[0x3de] = 0x14;
		for (i = 0; i <= 13; i ++) {
			port[0x3d4] = i;
			port[0x3d5] = tandy[i];
		}
		port[0x3da] = 2;
		port[0x3de] = 0;
		port[0x3d8] = port[0x3d8] | (11 & 0xef);
		port[0x3dd] = port[0x3dd] | 1;
		port[0x3df] = port[0x3df] | 0xc0;
		box(0, gd, 0, 0, 640, 200, 255);
		res = 1;
	}
	break;
	}
}

/* procedure affput(Chx,Gd,x,y,coul,char:int); external 'c:\mc\divaf.com'; */
void affput(int Chx, int Gd, int x, int y, int coul, int char_) {
	warning("TODO: Implement affput");
}

void affcar(int gd, int x, int y, int coul, int chr) {
	if (res == 1)  affput(1, gd, ((uint)x >> 1), y, coul, chr);
	else affput(1, gd, x, y, coul, chr);
}

void putpix(int gd, int x, int y, int coul) {
	affput(0, gd, x, y, coul, 0);
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
byte mem[0xffff];
int memw[0xffff];
int wherey;

void gotoxy(int x, int y) {
}
void textcolor(int c) {
}
void output(const Common::String &s) {
}
void intr(int intNum, registres &regs) {
}

void palette(int v1) {
	warning("TODO: s_sauv");
}

// (* external 'c:\mc\charecr.com'; *)
void s_char(int Gd, int y, int dy) {
		warning("TODO: s_char");
}

// (* external 'c:\mc\sauvecr.com'; *)
void s_sauv(int Gd, int y, int dy) {
	warning("TODO: s_sauv");
}

// (* external 'c:\mc\boite.com'; *)
void box(int c, int Gd, int xo, int yo, int xi, int yi, int patt) {
	warning("TODO: box method not yet implemented");
}

// (* external 'c:\mc\zuul.com'; *)
void zzuul(int ad, int seg, int tai) {
	warning("TODO: zzuul");
}

// (* external 'c:\mc\decomp.com'; *)
void decomp(int seg, int dep) {
	warning("TODO: decomp");
}

// (* external 'c:\mc\affich.com'; *)
void afff(int Gd, int seg, int dep, int x, int y) {
	warning("TODO: afff");
}
// (* external 'c:\mc\reusint.com'; *)
void musyc(tablint &tb, int nbseg, int att) {
	warning("TODO: musyc");
}

} // End of namespace Mortevielle
