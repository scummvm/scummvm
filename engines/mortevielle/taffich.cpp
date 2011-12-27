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

#include "mortevielle/taffich.h"
#include "common/str.h"

namespace Mortevielle {

void chardes(Common::String nom, float passe, int long_) {
	int i, p, l;
	byte b;
	untyped_file f;

	/* debug('chardes'); */
	assign(f, nom);
	/*$i-*/
	reset(f);
	testfi();
	p = 0;
	while (passe > 127) {
		p = p + 1;
		passe = passe - 128;
	}
	if (p != 0)  seek(f, p);
	p = trunc(passe);
	l = long_ + p;
	i = 0;
	while (l > 0) {
		blockread(f, mem[0x6000 + i], 1);
		testfi();
		l = l - 128;
		i = i + 128;
	}
	close(f);
	for (i = p; i <= long_ + p; i ++) mem[0x7000 + i - p] = mem[0x6000 + i];
	/*$i+*/
}

void charani(Common::String nom, float passe, int long_) {
	int i, p, l;
	byte b;
	untyped_file f;

	/* debug('charani'); */
	assign(f, nom);
	/*$i-*/
	reset(f);
	testfi();
	p = 0;
	while (passe > 127) {
		passe = passe - 128;
		p = p + 1;
	}
	if (p != 0)  seek(f, p);
	p = trunc(passe);
	l = long_ + p;
	i = 0;
	while (l > 0) {
		blockread(f, mem[0x6000 + i], 1);
		testfi();
		l = l - 128;
		i = i + 128;
	}
	close(f);
	for (i = p; i <= long_ + p; i ++) mem[0x7314 + i - p] = mem[0x6000 + i];
	/*$i+*/
}

void taffich() {
	const array<136, 140, byte> tran1
	= {{ 121, 121, 138, 139, 120 }};
	const array<153, 161, byte> tran2
	= {{
			150, 150, 152, 152, 100,
			110, 159, 100, 100
		}
	};
	int i, m, a, b, cx, handle,
	        npal;
	float lgt, lhandle;
	Common::String nom;
	int palh, k, j;
	int alllum[16];


	a = caff;
	if (set::of(range(153, 161), eos).has(a))  a = tran2[a];
	else if (set::of(range(136, 140), eos).has(a))  a = tran1[a];
	b = a;
	if (maff == a)  return;
	if (a == 16) {
		s.pourc[9] = '*';
		s.teauto[42] = '*';
	}
	if (a == 20) {
		s.teauto[39] = '*';
		if (s.teauto[36] == '*') {
			s.pourc[3] = '*';
			s.teauto[38] = '*';
		}
	}
	if (a == 24)  s.teauto[37] = '*';
	if (a == 30)  s.teauto[9] = '*';
	if (a == 31) {
		s.pourc[4] = '*';
		s.teauto[35] = '*';
	}
	if (a == 118)  s.teauto[41] = '*';
	if (a == 143)  s.pourc[1] = '*';
	if (a == 150)  s.teauto[34] = '*';
	if (a == 151)  s.pourc[2] = '*';
	okdes = true;
	hide_mouse();
	lgt = 0;
	if ((a != 50) && (a != 51)) {
		m = a + 2000;
		if ((m > 2001) && (m < 2010))  m = 2001;
		if (m == 2011)  m = 2010;
		if (a == 32)  m = 2034;
		if ((a == 17) && (maff == 14))  m = 2018;
		if (a > 99)
			if ((is == 1) || (is == 0))  m = 2031;
			else m = 2032;
		if (((a > 69) && (a < 80)) || (a == 30) || (a == 31) || (a == 144)
		        || (a == 147) || (a == 149))  m = 2030;
		if (((a < 27) && (((maff > 69) && (! s.ipre)) || (maff > 99)))
		        || ((maff > 29) && (maff < 33)))  m = 2033;
		messint(m);
		maff = a;
		if (a == 159)  a = 86;
		else if (a > 140)  a = a - 67;
		else if (a > 137)  a = a - 66;
		else if (a > 99)  a = a - 64;
		else if (a > 69)  a = a - 42;
		else if (a > 29)  a = a - 5;
		else if (a == 26)  a = 24;
		else if (a > 18)  a = a - 1;
		npal = a;
		for (cx = 0; cx <= (a - 1); cx ++) lgt = lgt + l[cx];
		handle = l[a];
		nom = "DXX.mor";
	} else {
		nom = "DZZ.mor";
		handle = l[87];
		if (a == 51) {
			lgt = handle;
			handle = l[88];
		}
		maff = a;
		npal = a + 37;
	}
	chardes(nom, lgt, handle);
	if (gd == her) {
		for (i = 0; i <= 15; i ++) {
			palh = memw[0x7000 + (succ(int, i) << 1)];
			alllum[i] = (palh & 15) + (((cardinal)palh >> 12) & 15) + (((cardinal)palh >> 8) & 15);
		}
		for (i = 0; i <= 15; i ++) {
			k = 0;
			for (j = 0; j <= 15; j ++) if (alllum[j] > alllum[k])  k = j;
			mem[0x7000 + 2 + (k << 1)] = rang[i];
			alllum[k] = -1;
		}
	}
	numpal = npal;
	writepal(npal);

	if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26) || (b == 50)) {
		lgt = 0;
		if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26)) {
			if (b == 26)  b = 18;
			else if (b == 24)  b = 17;
			else if (b > 15)  b = b - 1;
			for (cx = 0; cx <= (b - 1); cx ++) lgt = lgt + l[cx + 89];
			handle = l[b + 89];
			nom = "AXX.mor";
		} else if (b == 50) {
			nom = "AZZ.mor";
			handle = 1260;
		}
		charani(nom, lgt, handle);
	}
	show_mouse();
	if ((a < 27) && ((maff < 27) || (s.mlieu == 15)) && (msg[4] != entrer)) {
		if ((a == 13) || (a == 14))  person();
		else if (! blo)  t11(s.mlieu, cx);
		mpers =  0;
	}
}

/*    begin
      for i:=0 to 15 do
         begin
           PalH:=memw[$7000:succ(i) shl 1];
           Alllum[i]:=PalH and 15 + PalH shr 4 and 15 + PalH shr 8 and 15;
         end;
      for i:=0 to 15 do
         begin
           k:=0;
           for j:=0 to 15 do if Alllum[j]>Alllum[k] then k:=j;
           mem[$7000:2+k shl 1]:= Rang[i];
           Alllum[k]:=-1;
         end;
    end;*/

} // End of namespace Mortevielle
