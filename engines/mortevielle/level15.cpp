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

#include "common/system.h"
#include "common/file.h"
#include "mortevielle/graphics.h"
#include "mortevielle/level15.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/* NIVEAU 15 */
void copcha() {
	int i = acha;
	do {
		tabdon[i] = tabdon[i + 390];
		i = succ(int, i);
	} while (!(i == acha + 390));
}

bool dans_rect(rectangle r) {
	int x, y, c;

	bool dans_rect_result;
	getMousePos(x, y, c);
	if ((x > r.x1) &&
	        (x < r.x2) &&
	        (y > r.y1) &&
	        (y < r.y2))
		dans_rect_result = true;
	else
		dans_rect_result = false;

	return dans_rect_result;
}

void outbloc(int n, pattern p, t_nhom pal) {
	int i, j, ad;

	ad = n * 404 + 0xd700;

	WRITE_LE_UINT16(&mem[0x6000 * 16 + ad], p.tax);
	WRITE_LE_UINT16(&mem[0x6000 * 16 + ad + 2], p.tay);
	ad = ad + 4;
	for (i = 1; i <= p.tax; i ++)
		for (j = 1; j <= p.tay; j ++)
			mem[0x6000 * 16 + ad + pred(int, j)*p.tax + pred(int, i)] = pal[n].hom[p.des[i][j]];
}

void writepal(int n) {
	int i;
	t_nhom pal;

	switch (gd) {
	case tan:
	case ega:
	case ams :
		for (i = 1; i <= 16; i++) {
			mem[0x7000 * 16 + 2 * i] = tabpal[n][i].x;
			mem[0x7000 * 16 + succ(int, 2 * i)] = tabpal[n][i].y;
		}
		break;
	case cga : {
		warning("TODO: If this code is needed, resolve the incompatible types");
//		pal = palcga[n].a;
		if (n < 89)
			palette(palcga[n].p);
		for (i = 0; i <= 15; i++)
			outbloc(i, tpt[pal[i].n], pal);
	}
	break;
	}
}


void pictout(int seg, int dep, int x, int y) {
	GfxSurface surface;
	surface.decode(&mem[seg * 16 + dep]);

	if (gd == her) {
		mem[0x7000 * 16 + 2] = 0;
		mem[0x7000 * 16 + 32] = 15;
	}
	if ((caff != 51) && (READ_LE_UINT16(&mem[0x7000 * 16 + 0x4138]) > 0x100))
		WRITE_LE_UINT16(&mem[0x7000 * 16 + 0x4138], 0x100);
	g_vm->_screenSurface.drawPicture(surface, x, y);
}

void sauvecr(int y, int dy) {
	hideMouse();
	s_sauv(gd, y, dy);
	showMouse();
}

void charecr(int y, int dy) {
	hideMouse();
	s_char(gd, y, dy);
	showMouse();
}

void adzon() {
	Common::File f;

	if (!f.open("don.mor"))
		error("Missing file - don.mor");
	f.read(tabdon, 7 * 256);
	f.close();

	if (!f.open("bmor.mor"))
		error("Missing file - bmor.mor");
	f.read(&tabdon[fleche], 1 * 1916);
	f.close();

	if (!f.open("dec.mor"))
		error("Missing file - dec.mor");
	f.read(&mem[0x73a2 * 16 + 0], 1 * 1664);
	f.close();
}

/**
 * Returns the offset within the compressed image data resource of the desired image
 */
int animof(int ouf, int num) {
	int nani, aux;

	int animof_result;
	nani = mem[adani * 16 + 1];
	aux = num;
	if (ouf != 1)
		aux += nani;
	animof_result = (nani << 2) + 2 + READ_BE_UINT16(&mem[adani * 16 + (aux << 1)]);

	return animof_result;
}

} // End of namespace Mortevielle
