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

#include "mortevielle/keyboard.h"
#include "mortevielle/var_mor.h"
#include "mortevielle/asm.h"

namespace Mortevielle {

char readkey1() {
	char c = get_ch();

	return c;
}

int testou() {
	char ch = get_ch();

	switch (ch)  {
	case '\23' :
		g_soundOff = !g_soundOff;
		break;
	case '\26' :
		if ((g_c_zzz == 1) || (g_c_zzz == 2)) {
			zzuul(&g_adcfiec[161 * 16], ((822 * 128) - (161 * 16)) / 64);
			++g_c_zzz;

			return 61;
		}
		break;
	case '\33' :
		if (keypressed())
			ch = get_ch();
		break;
	default:
		break;
	}

	return ord(ch);
}

void teskbd() {
	if (keypressed())
		testou();
}

} // End of namespace Mortevielle
