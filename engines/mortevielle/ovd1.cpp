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

#include "common/file.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/ovd1.h"
#include "mortevielle/parole.h"
#include "mortevielle/sound.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

// For ScummVM, we need to do check for file errors where we do the file access
const int ioresult = 0;

/**
 * Check for disk 2 being in the drive
 */
void dem2() {
	// Deprecated under ScummVM
}

void charpal() {
	Common::File f;		// tabdb records
	Common::File ft;	// tfxx
	Common::File fb;	// byte values
	byte b;

	if (!ft.open("fxx.mor"))
		error("Missing file - fxx.mor");
	for (int i = 0; i < 108; ++i)
		l[i] = ft.readSint16LE();
	ft.close();

	if (!f.open("plxx.mor"))
		error("Missing file - plxx.mor");
	for (int i = 0; i <= 90; ++i) {
		for (int j = 1; j <= 16; ++j) {
			tabpal[i][j].x = f.readByte();
			tabpal[i][j].y = f.readByte();
		}
	}
	f.close();
	
	if (!fb.open("cxx.mor"))
		error("Missing file - cxx.mor");

	for (int j = 0; j <= 90; ++j) {
		palcga[j].p = fb.readByte();
		for (int i = 0; i <= 15; ++i) {
			nhom &with = palcga[j].a[i];

			b = fb.readByte();
			with.n = (uint)b >> 4;
			with.hom[0] = ((uint)b >> 2) & 3;
			with.hom[1] = b & 3;
		}
	}
	palcga[10].a[9] = palcga[10].a[5];
	for (int j = 0; j <= 14; ++j) {
		tpt[j].tax = fb.readByte();
		tpt[j].tay = fb.readByte();
		for (int i = 1; i <= 20; ++i)
			for (int k = 1; k <= 20; ++k)
				tpt[j].des[i][k] = fb.readByte();
	}
	fb.close();
}

void chartex() {
	Common::File inpFile;
	Common::File ntpFile;
	char s[1410];

	g_vm->_txxFileFl = false;
	if (g_vm->getLanguage() == Common::EN_ANY) {
		warning("English version expected - Switching to DAT file");
		return;
	}

	if (!inpFile.open("TXX.INP")) {
		if (!inpFile.open("TXX.MOR")) {
			warning("Missing file - TXX.INP or .MOR - Switching to DAT file");
			return;
		}
	}

	if (!ntpFile.open("TXX.NTP")) {
		if (!ntpFile.open("TXX.IND")) {
			warning("Missing file - TXX.NTP or .IND - Switching to DAT file");
			return;
		}
	}

	if ((inpFile.size() > (maxti * 2)) || (ntpFile.size() > (maxtd * 3))) {
		warning("TXX file - Unexpected format - Switching to DAT file");
		return;
	}

	for (int i = 0; i < inpFile.size() / 2; ++i)
		t_mot[i] = inpFile.readUint16LE();

	inpFile.close();

	for (int i = 0; i < (ntpFile.size() / 3); ++i) {
		t_rec[i].indis = ntpFile.readSint16LE();
		t_rec[i].point = ntpFile.readByte();
	}

	ntpFile.close();

	g_vm->_txxFileFl = true;
}

/**
 * The original engine used this method to display a starting text screen letting the player
 * select the graphics mode to use
 */
void dialpre() {
	/* debug('o3 dialpre'); */
	crep = 998;
	int_m = true;
}

void init_lieu() {
	Common::File f_lieu;	// tab_mlieu

	/* debug('o3 init_lieu'); */
	if (!f_lieu.open("MXX.mor"))
		error("Missing file - MXX.mor");

	for (int i = 1; i < 8; ++i)
		for (int j = 0; j < 25; ++j)
			v_lieu[i][j] = f_lieu.readByte(); 

	f_lieu.close();
}


void music() {
	Common::File fic;
	int k;
	bool fin;
	char ch;
//	float x, y;


	/* debug('o3 music'); */
	if (_soundOff)
		return;

	rech_cfiec = true;
	
	if (!fic.open("mort.img"))
		error("Missing file - mort.img");

	fic.read(&mem[0x3800 * 16 + 0], 500);
	fic.read(&mem[0x47a0 * 16 + 0], 123);
	fic.close();

	g_vm->_soundManager.decodeMusic(&mem[0x3800 * 16], &mem[0x5000 * 16], 623);
	addfix = (float)((tempo_mus - addv[1])) / 256;
	cctable(tbi);

	fin = false;
	k = 0;
	do {
		fin = keypressed();
		g_vm->_soundManager.musyc(tbi, 9958 , tempo_mus);
		++k;
		fin = fin | keypressed() | (k >= 5);
	} while (!fin);
	while (keypressed())
		ch = get_ch();	// input >> kbd >> ch;
}


void charge_bruit5() {
	Common::File f;

	if (!f.open("bruit5"))
		error("Missing file - bruit5");

	f.read(&mem[adbruit5 * 16 + 0], 149 * 128);
	f.close();
}

void charge_cfiec() {
	Common::File f;

	if (!f.open("cfiec.mor"))
		error("Missing file - cfiec.mor");

	f.read(&adcfiec[0], 822 * 128);
	f.close();

	rech_cfiec = false;
}


void charge_cfiph() {
	Common::File f;

	if (!f.open("cfiph.mor"))
		error("Missing file - cfiph.mor");

	for (int i = 0; i < (f.size() / 2); ++i)
		t_cph[i] = f.readSint16LE();

	f.close();
}

void suite() {
	hirs();
	repon(7, 2035);
	caff = 51;
	taffich();
	teskbd();
	if (_newGraphicalDevice != _currGraphicalDevice)
		_currGraphicalDevice = _newGraphicalDevice;
	hirs();
	dessine(ades, 0, 0);

	Common::String cpr = "COPYRIGHT 1989 : LANKHOR";
	g_vm->_screenSurface.putxy(104 + 72 * res, 185);
	g_vm->_screenSurface.writeg(cpr, 0);
}

} // End of namespace Mortevielle
