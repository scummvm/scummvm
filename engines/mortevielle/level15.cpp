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
		g_tabdon[i] = g_tabdon[i + 390];
		++i;
	} while (i != acha + 390);
}

/**
 * Engine function : Is mouse in a given rect?
 * @remarks	Originally called 'dans_rect'
 */
bool isMouseIn(rectangle r) {
	int x, y, c;

	getMousePos(x, y, c);
	if ((x > r._x1) && (x < r._x2) && (y > r._y1) && (y < r._y2))
		return true;

	return false;
}

void outbloc(int n, pattern p, nhom *pal) {
	int ad = n * 404 + 0xd700;

	WRITE_LE_UINT16(&g_mem[0x6000 * 16 + ad], p._tax);
	WRITE_LE_UINT16(&g_mem[0x6000 * 16 + ad + 2], p._tay);
	ad += 4;
	for (int i = 1; i <= p._tax; ++i) {
		for (int j = 1; j <= p._tay; ++j)
			g_mem[(0x6000 * 16) + ad + (j - 1) * p._tax + i - 1] = pal[n]._hom[p._des[i][j]];
	}
}

void writepal(int n) {
	switch (g_vm->_currGraphicalDevice) {
	case MODE_TANDY:
	case MODE_EGA:
	case MODE_AMSTRAD1512:
		for (int i = 1; i <= 16; ++i) {
			g_mem[(0x7000 * 16) + (2 * i)] = g_tabpal[n][i].x;
			g_mem[(0x7000 * 16) + (2 * i) + 1] = g_tabpal[n][i].y;
		}
		break;
	case MODE_CGA: {
		warning("TODO: If this code is needed, resolve the incompatible types");
		nhom pal[16];
		for (int i = 0; i < 16; ++i) {
			pal[i] = g_palcga[n]._a[i];
		}
//		nhom pal[16] = palcga[n]._a;
		if (n < 89)
			palette(g_palcga[n]._p);
		
		for (int i = 0; i <= 15; ++i)
			outbloc(i, g_tpt[pal[i]._id], pal);
		}
		break;
	default:
		break;
	}
}


void pictout(int seg, int dep, int x, int y) {
	GfxSurface surface;
	surface.decode(&g_mem[seg * 16 + dep]);

	if (g_vm->_currGraphicalDevice == MODE_HERCULES) {
		g_mem[0x7000 * 16 + 2] = 0;
		g_mem[0x7000 * 16 + 32] = 15;
	}

	if ((g_caff != 51) && (READ_LE_UINT16(&g_mem[0x7000 * 16 + 0x4138]) > 0x100))
		WRITE_LE_UINT16(&g_mem[0x7000 * 16 + 0x4138], 0x100);

	g_vm->_screenSurface.drawPicture(surface, x, y);
}

void sauvecr(int y, int dy) {
	hideMouse();
	s_sauv(g_vm->_currGraphicalDevice, y, dy);
	showMouse();
}

void charecr(int y, int dy) {
	hideMouse();
	s_char(g_vm->_currGraphicalDevice, y, dy);
	showMouse();
}

void adzon() {
	Common::File f;

	if (!f.open("don.mor"))
		error("Missing file - don.mor");

	f.read(g_tabdon, 7 * 256);
	f.close();

	if (!f.open("bmor.mor"))
		error("Missing file - bmor.mor");

	f.read(&g_tabdon[fleche], 1 * 1916);
	f.close();

	if (!f.open("dec.mor"))
		error("Missing file - dec.mor");

	f.read(&g_mem[0x73a2 * 16 + 0], 1 * 1664);
	f.close();
}

/**
 * Returns the offset within the compressed image data resource of the desired image
 */
int animof(int ouf, int num) {
	int nani = g_mem[adani * 16 + 1];
	int aux = num;
	if (ouf != 1)
		aux += nani;

	int animof_result = (nani << 2) + 2 + READ_BE_UINT16(&g_mem[adani * 16 + (aux << 1)]);

	return animof_result;
}

} // End of namespace Mortevielle
