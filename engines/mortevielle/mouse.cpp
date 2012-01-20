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
#include "common/rect.h"
#include "mortevielle/mouse.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

const int m_arrow = 0;
const int m_point_hand = 1;

bool int_m,
        clic;

int m_show,
        x_s,
        y_s,
        p_o_s,
        mouse_shwn;

int s_s[12][6];


void init_mouse() {
//	int i, j;
	registres reg;

	mouse_shwn = 0;
	x_s = 0;
	y_s = 0;
	p_o_s = 0;
	/*int_m:= False;*/
	clic = false;
	m_show = m_arrow;
	if ((READ_LE_UINT16(&mem[0xcc]) == 0) && (READ_LE_UINT16(&mem[0xce]) == 0))  int_m = false;
	if (int_m) {
		reg.ax = 0;
		intr(0x33, reg);
		int_m = (reg.ax == -1);
		if (int_m) {
			{
				reg.ax = 4;
				reg.cx = 0;
				reg.dx = 0;
			}
			intr(0x33, reg);
		}
	}
}

/*procedure change_mouse( t : int );
begin
  m_show:= t;
end;*/

/*procedure dessine_souris(xf,yf:int);
  var x, xx, y :int;
begin
  case m_show of
             m_arrow : begin
                         affput(2,Gd,xf,yf,0,0);
                       end;
  end;
end;  */

void hide_mouse() {
	int i, j, k, ps;;
	bool imp;

	mouse_shwn = pred(int, mouse_shwn);
	if (mouse_shwn == 0) {
		imp = odd(y_s);
		j = p_o_s;
		switch (gd) {
		case cga : {
			k = 0;
			j = ((uint)y_s >> 1) * 80 + ((uint)x_s >> 2);
			do {
				WRITE_LE_UINT16(&mem[0xb000 * 16 + j], s_s[0][k]);
				WRITE_LE_UINT16(&mem[0xb800 * 16 + j + 2], s_s[1][k]);
				WRITE_LE_UINT16(&mem[0xba00 * 16 + j], s_s[2][k]);
				WRITE_LE_UINT16(&mem[0xba00 * 16 + j + 2], s_s[3][k]);
				j = j + 80;
				k = succ(int, k);
			} while (!(k >= 5));
		}
		break;
		case ams : {
			for (i = 0; i <= 3; i ++) {
				port[0x3dd] = 1 << i;
				k = 0;
				j = p_o_s;
				do {
					if (imp) {
						WRITE_LE_UINT16(&mem[0xb800 * 16 + j], s_s[i][k]);
						j = j + 80 - 0x2000;
					} else {
						WRITE_LE_UINT16(&mem[0xb800 * 16 + j], s_s[i][k]);
						j = j + 0x2000;
					}
					imp = ! imp;
					k = succ(int, k);
				} while (!(k >= 8));
			}
		}
		break;
		case ega : {
			port[0x3c4] = 2;
			port[0x3ce] = 8;
			port[0x3cf] = 255;
			i = 0;
			do {
				port[0x3c5] = 1 << i;
				k = 0;
				j = p_o_s;
				do {
					ps = mem[0xa000 * 16 + j];
					mem[0xa000 * 16 + j] = lo(s_s[i][k]);
					ps = mem[0xa000 * 16 + j + 1];
					mem[0xa000 * 16 + j + 1] = hi(s_s[i][k]);
					j = j + 80;
					k = succ(int, k);
				} while (!(k >= 8));
				i = i + 1;
			} while (!(i == 4));
		}
		break;
		case her : {
			j = ((uint)y_s >> 1) * 80 + ((uint)x_s >> 3);
			for (i = 0; i <= 5; i ++) {
				for (k = 0; k <= 3; k ++) 
					WRITE_LE_UINT16(&mem[0xb000 * 16 + k * 0x200 + j], s_s[i][k]);
				j = j + 80;
			}
		}
		break;
		case tan : {
			j = ((uint)y_s >> 2) * 160 + ((uint)x_s >> 1);
			k = 0;
			do {
				for (i = 0; i <= 3; i ++) {
					WRITE_LE_UINT16(&mem[0xb800 * 16 + 0x200 * i + j], s_s[k][i + (k << 2)]);
					WRITE_LE_UINT16(&mem[0xb800 * 16 + 0x200 * i + j + 2], s_s[k + 3][i + (k << 2)]);
				}
				j = j + 160;
				k = succ(int, k);
			} while (!(k == 3));
		}
		break;

		}     /* case Gd */
	}
}

void show_mouse() {
	int i, j, k, l;
	bool imp;

	mouse_shwn = mouse_shwn + 1;
	if (mouse_shwn != 1)  return;
	j = p_o_s;
	imp = odd(y_s);
	i = x_s & 7;
	switch (gd) {
	case cga : {
		k = 0;
		j = ((uint)y_s >> 1) * 80 + ((uint)x_s >> 2);
		do {
			s_s[0][k] = READ_LE_UINT16(&mem[0xb800 * 16 + j]);
			s_s[1][k] = READ_LE_UINT16(&mem[0xb800 * 16 + j + 2]);
			s_s[2][k] = READ_LE_UINT16(&mem[0xba00 * 16 + j]);
			s_s[3][k] = READ_LE_UINT16(&mem[0xba00 * 16 + j + 2]);
			j = j + 80;
			k = succ(int, k);
		} while (!(k >= 5));
	}
	break;
	case ams : {
		for (i = 0; i <= 3; i ++) {
			j = p_o_s;
			imp = odd(y_s);
			port[0x3de] = i;
			k = 0;
			do {
				if (imp) {
					s_s[i][k] = READ_LE_UINT16(&mem[0xb800 * 16 + j]);
					j = j + 80 - 0x2000;
				} else {
					s_s[i][k] = READ_LE_UINT16(&mem[0xb800 * 16 + j]);
					j = j + 0x2000;
				}
				imp = ! imp;
				k = succ(int, k);
			} while (!(k >= 8));
		}
	}
	break;
	case ega : {
		port[0x3ce] = 4;
		l = 0;
		do {
			port[0x3cf] = l;
			k = 0;
			j = p_o_s;
			do {
				s_s[l][k] = mem[0xa000 * 16 + j] + (mem[0xa000 * 16 + succ(int, j)] << 8);
				j = j + 80;
				k = succ(int, k);
			} while (!(k >= 8));
			l = l + 1;
		} while (!(l == 4));
	}
	break;
	case her : {
		j = ((uint)y_s >> 1) * 80 + ((uint)x_s >> 3);
		for (i = 0; i <= 5; i ++) {
			for (k = 0; k <= 3; k ++)
				s_s[i][k] = READ_LE_UINT16(&mem[0xb000 * 16 + k * 0x200 + j]);
			j = j + 80;
		}
	}
	break;
	case tan : {
		j = ((uint)y_s >> 2) * 160 + ((uint)x_s >> 1);
		k = 0;
		do {
			for (i = 0; i <= 3; i ++) {
				s_s[k][i + (k << 2)] = READ_LE_UINT16(&mem[0xb800 * 16 + 0x200 * i + j]);
				s_s[k + 3][i + (k << 2)] = READ_LE_UINT16(&mem[0xb800 * 16 + 0x200 * i + j + 2]);
			}
			j = j + 160;
			k = succ(int, k);
		} while (!(k == 3));
	}
	break;
	}    /*  case Gd   */
	affput(Common::Point(x_s, y_s), 0, 0);
}

void pos_mouse(int x, int y) {
	if (x > 314 * res)  x = 314 * res;
	else if (x < 0)  x = 0;
	if (y > 199)  y = 199;
	else if (y < 0)  y = 0;
	if ((x == x_s) && (y == y_s))  return;

	// Set the new position
	g_vm->setMousePos(Common::Point(x, y));
}

void read_pos_mouse(int &x, int &y, int &c) {
	x = g_vm->getMousePos().x;
	y = g_vm->getMousePos().y;
	c = g_vm->getMouseButtons();
}

void mov_mouse(bool &funct, char &key) {
	bool p_key;
	char in1, in2;
	int cx, cy, cd;

	// If mouse button clicked, return it
	if (g_vm->getMouseButtons() != 0) {
		clic = true;
		return;
	}

	funct = false;
	key = '\377';
	p_key = keypressed();

	while (p_key) {
		CHECK_QUIT;

		in1 = get_ch();	// input >> kbd >> in1;
		read_pos_mouse(cx, cy, cd);
		switch (toupper(in1)) {
		case '4' :
			cx = cx - 8;
			break;
		case '2' :
			cy = cy + 8;
			break;
		case '6' :
			cx = cx + 8;
			break;
		case '8' :
			cy = cy - 8;
			break;
		case '7' : {
			cy = 1;
			cx = 1;
		}
		break;
		case '1' : {
			cx = 1;
			cy = 190;
		}
		break;
		case '9' : {
			cx = 315 * res;
			cy = 1;
		}
		break;
		case '3' : {
			cy = 190;
			cx = 315 * res;
		}
		break;
		case '5' : {
			cy = 100;
			cx = 155 * res;
		}
		break;
		case ' ':
		case '\15' : {
			clic = true;
			return;
		}
		break;
		case '\33' : {
			p_key = keypressed();

			if (p_key) {
				in2 = get_ch();	// input >> kbd >> in2;

				if ((in2 >= ';') && (in2 <= 'D')) {
					funct = true;
					key = in2;
					return;
				} else {
					switch (in2) {
					case 'K' :
						cx = pred(int, cx);
						break;
					case 'P' :
						cy = succ(int, cy);
						break;
					case 'M' :
						cx = cx + 2;
						break;
					case 'H' :
						cy = pred(int, cy);
						break;
					case 'G' : {
						cx = cx - 1;
						cy = cy - 1;
					}
					break;
					case 'I' : {
						cx = cx + 1;
						cy = cy - 1;
					}
					break;
					case 'O' : {
						cx = cx - 1;
						cy = cy + 1;
					}
					break;
					case 'Q' : {
						cx = cx + 1;
						cy = cy + 1;
					}
					break;
					}    /* case */
				}
			}
		}
		break;
		case 'I' : {
			cx = res * 32;
			cy = 8;
		}
		break;
		case 'D' : {
			cx = 80 * res;
			cy = 8;
		}
		break;
		case 'A' : {
			cx = 126 * res;
			cy = 8;
		}
		break;
		case 'S' : {
			cx = 174 * res;
			cy = 8;
		}
		break;
		case 'P' : {
			cx = 222 * res;
			cy = 8;
		}
		break;
		case 'F' : {
			cx = res * 270;
			cy = 8;
		}
		break;
		case '\23' : {
			sonoff = ! sonoff;
			return;
		}
		break;
		case '\26' : {
			zuul = true;
			return;
		}
		break;
		case '\24' : {           /* ^T => mode tandy */
			funct = true;
			key = '\11';
		}
		break;
		case '\10' : {           /* ^H => mode Hercule */
			funct = true;
			key = '\7';
		}
		break;
		case '\1':
		case '\3':
		case '\5' : {
			funct = true;
			key = in1;
		}
		break;
		}
		pos_mouse(cx, cy);
		p_key = keypressed();
	}
}

} // End of namespace Mortevielle
