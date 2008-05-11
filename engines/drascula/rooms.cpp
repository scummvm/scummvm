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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "drascula/drascula.h"

namespace Drascula {

static const int mirar_t[3] = {100, 101, 54};
static const char mirar_v[3][14] = {"100.als", "101.als", "54.als"};
static const int poder_t[6] = {11, 109, 111, 110, 115, 116};
static const char poder_v[6][14] = {"11.als", "109.als", "111.als", "110.als", "115.als", "116.als"};

void DrasculaEngine::room_0() {
	if (num_ejec == 1) {
		if (objeto_que_lleva == LOOK)
			talk(_text[_lang][54], "54.als");
		else if (objeto_que_lleva == MOVE)
			talk(_text[_lang][19], "19.als");
		else if (objeto_que_lleva == PICK)
			talk(_text[_lang][11], "11.als");
		else if (objeto_que_lleva == OPEN)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == CLOSE)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == TALK)
			talk(_text[_lang][16], "16.als");
		else
			talk(_text[_lang][11],"11.als");
	} else if (num_ejec == 2) {
		if (objeto_que_lleva == LOOK) {
			talk(_text[_lang][mirar_t[c_mirar]], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (objeto_que_lleva == MOVE) {
			talk(_text[_lang][19], "19.als");
		} else if (objeto_que_lleva == PICK) {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else if (objeto_que_lleva == OPEN)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == CLOSE)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == TALK)
			talk(_text[_lang][16], "16.als");
		else {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	} else if (num_ejec == 3) {
		if (objeto_que_lleva == LOOK)
			talk(_text[_lang][316], "316.als");
		else if (objeto_que_lleva == MOVE)
			talk(_text[_lang][317], "317.als");
		else if (objeto_que_lleva == PICK)
			talk(_text[_lang][318], "318.als");
		else if (objeto_que_lleva == OPEN)
			talk(_text[_lang][319], "319.als");
		else if (objeto_que_lleva == CLOSE)
			talk(_text[_lang][319], "319.als");
		else if (objeto_que_lleva == TALK)
			talk(_text[_lang][320], "320.als");
		else
			talk(_text[_lang][318], "318.als");
	} else if (num_ejec == 4) {
		if (objeto_que_lleva == LOOK) {
			talk(_text[_lang][mirar_t[c_mirar]], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (objeto_que_lleva == MOVE)
			talk(_text[_lang][19], "19.als");
		else if (objeto_que_lleva == PICK) {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else if (objeto_que_lleva == OPEN)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == CLOSE)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == TALK)
			talk(_text[_lang][16], "16.als");
		else {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	} else if (num_ejec == 5) {
		if (objeto_que_lleva == LOOK) {
			talk(_text[_lang][mirar_t[c_mirar]], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (objeto_que_lleva == MOVE)
			talk(_text[_lang][19], "19.als");
		else if (objeto_que_lleva == PICK) {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else if (objeto_que_lleva == OPEN)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == CLOSE)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == TALK)
			talk(_text[_lang][16], "16.als");
		else {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	} else if (num_ejec == 6) {
		if (objeto_que_lleva == LOOK) {
			talk(_text[_lang][mirar_t[c_mirar]], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (objeto_que_lleva == MOVE)
			talk(_text[_lang][19], "19.als");
		else if (objeto_que_lleva == PICK) {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else if (objeto_que_lleva == OPEN)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == CLOSE)
			talk(_text[_lang][9], "9.als");
		else if (objeto_que_lleva == TALK)
			talk(_text[_lang][16], "16.als");
		else {
			talk(_text[_lang][poder_t[c_poder]], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	}
}

void DrasculaEngine::room_1(int fl) {
	if (objeto_que_lleva == LOOK && fl == 118) {
		talk(_text[_lang][1], "1.als");
		pause(10);
		talk(_text[_lang][2], "2.als");
	} else if (objeto_que_lleva == PICK && fl == 118)
		talk(_text[_lang][5], "5.als");
	else if (objeto_que_lleva == OPEN && fl == 118)
		talk(_text[_lang][3], "3.als");
	else if (objeto_que_lleva == CLOSE && fl == 118)
		talk(_text[_lang][4], "4.als");
	else if (objeto_que_lleva == TALK && fl == 118)
		talk(_text[_lang][6], "6.als");
	else if (objeto_que_lleva == LOOK && fl == 119)
		talk(_text[_lang][8], "8.als");
	else if (objeto_que_lleva == MOVE && fl == 119)
		talk(_text[_lang][13], "13.als");
	else if (objeto_que_lleva == CLOSE && fl == 119)
		talk(_text[_lang][10], "10.als");
	else if (objeto_que_lleva == TALK && fl == 119)
		talk(_text[_lang][12], "12.als");
	else if (objeto_que_lleva == LOOK && fl == 120 && flags[8] == 0)
		talk(_text[_lang][14], "14.als");
	else if (objeto_que_lleva == MOVE && fl == 120)
		talk(_text[_lang][13], "13.als");
	else if (objeto_que_lleva == OPEN && fl == 120)
		talk(_text[_lang][18], "18.als");
	else if (objeto_que_lleva == TALK && fl == 120)
		talk(_text[_lang][15], "15.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_3(int fl) {
	if (objeto_que_lleva == LOOK && fl == 129)
		talk(_text[_lang][21], "21.als");
	else if (objeto_que_lleva == PICK && fl == 129)
		talk(_text[_lang][5], "5.als");
	else if (objeto_que_lleva == MOVE && fl == 129)
		talk(_text[_lang][24], "24.als");
	else if (objeto_que_lleva == OPEN && fl == 129)
		talk(_text[_lang][22], "22.als");
	else if (objeto_que_lleva == CLOSE && fl == 129)
		talk(_text[_lang][10], "10.als");
	else if (objeto_que_lleva == TALK && fl == 129) {
		talk(_text[_lang][23], "23.als");
		pause(6);
		talk_sinc(_text[_lang][50], "50.als", "11111111111144432554433");
	} else if (objeto_que_lleva == LOOK && fl == 131)
		talk(_text[_lang][27], "27.als");
	else if (objeto_que_lleva == PICK && fl == 131)
		talk(_text[_lang][5], "5.als");
	else if (objeto_que_lleva == MOVE && fl == 131)
		talk(_text[_lang][24], "24.als");
	else if (objeto_que_lleva == OPEN && fl == 131)
		talk(_text[_lang][22], "22.als");
	else if (objeto_que_lleva == CLOSE && fl == 131)
		talk(_text[_lang][10], "10.als");
	else if (objeto_que_lleva == TALK && fl == 131)
		talk(_text[_lang][23], "23.als");
	else if (objeto_que_lleva == LOOK && fl == 132)
		talk(_text[_lang][28], "28.als");
	else if (objeto_que_lleva == PICK && fl == 132)
		talk(_text[_lang][5], "5.als");
	else if (objeto_que_lleva == MOVE && fl == 132)
		talk(_text[_lang][24], "24.als");
	else if (objeto_que_lleva == OPEN && fl == 132)
		talk(_text[_lang][22], "22.als");
	else if (objeto_que_lleva == CLOSE && fl == 132)
		talk(_text[_lang][10], "10.als");
	else if (objeto_que_lleva == TALK && fl == 132)
		talk(_text[_lang][23], "23.als");
	else if (objeto_que_lleva == LOOK && fl == 133)
		talk(_text[_lang][321], "321.als");
	else if (objeto_que_lleva == PICK && fl == 133)
		talk(_text[_lang][31], "31.als");
	else if (objeto_que_lleva == MOVE && fl == 133)
		talk(_text[_lang][34], "34.als");
	else if (objeto_que_lleva == OPEN && fl == 133)
		talk(_text[_lang][30], "30.als");
	else if (objeto_que_lleva == CLOSE && fl == 133)
		talk(_text[_lang][10], "10.als");
	else if (objeto_que_lleva == TALK && fl == 133) {
		talk_sinc(_text[_lang][322], "322.als", "13333334125433333333");
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(25);
		talk(_text[_lang][33], "33.als");
	} else if (objeto_que_lleva == LOOK && fl == 165) {
		talk(_text[_lang][149], "149.als");
		talk(_text[_lang][150], "150.als");
	} else if (objeto_que_lleva == PICK && fl == 165) {
		copyBackground(0, 0, 0,0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		copyRect(44, 1, hare_x, hare_y, 41, 70, dir_dibujo2, dir_zona_pantalla);
		updateRefresh();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(4);
		agarra_objeto(10);
		flags[3] = 1;
		visible[8] = 0;
	} else if (objeto_que_lleva == LOOK && fl == 166)
		talk(_text[_lang][55], "55.als");
	else if (objeto_que_lleva == PICK && fl == 166)
		talk(_text[_lang][7], "7.als");
	else if (objeto_que_lleva == 14 && fl == 166 && flags[37] == 0) {
		animation_7_2();
		agarra_objeto(8);
	} else if (objeto_que_lleva == 14 && fl == 166 && flags[37] == 1)
		talk(_text[_lang][323], "323.als");
	else if (objeto_que_lleva == LOOK && fl == 211)
		talk(_text[_lang][184], "184.als");
	else if (objeto_que_lleva == TALK && fl == 211) {
		talk(_text[_lang][185], "185.als");
		talk(_text[_lang][186], "186.als");
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_4(int fl) {
	if (objeto_que_lleva == MOVE && fl == 189 && flags[34] == 0) {
		talk(_text[_lang][327], "327.als");
		agarra_objeto(13);
		flags[34] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else if (objeto_que_lleva == LOOK && fl == 189)
		talk(_text[_lang][182], "182.als");
	else if (objeto_que_lleva == LOOK && fl == 207)
		talk(_text[_lang][175], "175.als");
	else if (objeto_que_lleva == TALK && fl == 207)
		talk(_text[_lang][176], "176.als");
	else if (objeto_que_lleva == LOOK && fl == 208)
		talk(_text[_lang][177], "177.als");
	else if (objeto_que_lleva == LOOK && fl == 209)
		talk(_text[_lang][179], "179.als");
	else if (objeto_que_lleva == LOOK && fl == 210)
		talk(_text[_lang][180], "180.als");
	else if (objeto_que_lleva == OPEN && fl == 210)
		talk(_text[_lang][181], "181.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_5(int fl) {
	if (objeto_que_lleva == LOOK && fl == 136 && flags[8]==0) talk(_text[_lang][14], "14.als");
	else if (objeto_que_lleva == MOVE && fl == 136)
		talk(_text[_lang][13], "13.als");
	else if (objeto_que_lleva == OPEN && fl == 136)
		talk(_text[_lang][18], "18.als");
	else if (objeto_que_lleva == TALK && fl == 136)
		talk(_text[_lang][15], "15.als");
	else if (objeto_que_lleva == 10 && fl == 136) {
		animation_5_2();
		resta_objeto(10);
	} else if (objeto_que_lleva == LOOK && fl == 212)
		talk(_text[_lang][187], "187.als");
	else if (objeto_que_lleva == TALK && fl == 212)
		talk(_text[_lang][188], "188.als");
	else if (objeto_que_lleva == LOOK && fl == 213)
		talk(_text[_lang][189], "189.als");
	else if (objeto_que_lleva == OPEN && fl == 213)
		talk(_text[_lang][190], "190.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_6(int fl){
	if (objeto_que_lleva == LOOK && fl==144) {
		talk(_text[_lang][41], "41.als");
		talk(_text[_lang][42], "42.als");
	} else if (objeto_que_lleva == PICK && fl == 144)
		talk(_text[_lang][43], "43.als");
	else if (objeto_que_lleva == LOOK && fl == 138)
		talk(_text[_lang][35], "35.als");
	else if (objeto_que_lleva == OPEN && fl == 138)
		abre_puerta(0, 1);
	else if (objeto_que_lleva == CLOSE && fl == 138)
		cierra_puerta(0, 1);
	else if (objeto_que_lleva == TALK && fl == 138)
		talk(_text[_lang][6], "6.als");
	else if (objeto_que_lleva == LOOK && fl == 143)
		talk(_text[_lang][37], "37.als");
	else if (objeto_que_lleva == PICK && fl == 143)
		talk(_text[_lang][7], "7.als");
	else if (objeto_que_lleva == MOVE && fl == 143)
		talk(_text[_lang][7], "7.als");
	else if (objeto_que_lleva == OPEN && fl == 143 && flags[2] == 0) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();
	copyRect(228, 102, hare_x + 5, hare_y - 1, 47, 73, dir_dibujo3, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pause(10);
	comienza_sound("s3.als");
	flags[2] = 1;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	} else if (objeto_que_lleva == CLOSE && fl == 143 && flags[2] == 1) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		flags[2] = 0;
		updateRefresh_pre();
		copyRect(228, 102, hare_x + 5, hare_y - 1, 47, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(5);
		comienza_sound("s4.als");
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fin_sound();
	} else if (objeto_que_lleva == TALK && fl == 143)
		talk(_text[_lang][38], "38.als");
	else if (objeto_que_lleva == LOOK && fl == 139)
		talk(_text[_lang][36], "36.als");
	else if (objeto_que_lleva == OPEN && fl == 139 && flags[1] == 0) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		copyRect(267, 1, hare_x - 14, hare_y - 2, 52, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(19);
		comienza_sound("s3.als");
		flags[1] = 1;
		visible[4] = 1;
		visible[2] = 0;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fin_sound();
	} else if (objeto_que_lleva == LOOK && fl == 140)
		talk(_text[_lang][147], "147.als");
	else if (objeto_que_lleva == PICK && fl == 140) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		copyRect(267, 1, hare_x - 14, hare_y - 2, 52, 73, dir_dibujo3, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(19);
		agarra_objeto(9);
		visible[4] = 0;
		flags[10] = 1;
	} else if (objeto_que_lleva == OPEN && fl == 140)
		hay_respuesta = 1;
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_7(int fl){
	if (objeto_que_lleva == LOOK && fl == 169)
		talk(_text[_lang][44], "44.als");
	else if (objeto_que_lleva == LOOK && fl == 164)
		talk(_text[_lang][35], "35.als" );
	else if (objeto_que_lleva == PICK && fl == 190) {
		agarra_objeto(17);
		flags[35] = 1;
		visible[3] = 0;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_8(int fl) {
	if (objeto_que_lleva == LOOK && fl == 147 && flags[7] == 0) {
		talk(_text[_lang][58], "58.als");
		agarra_objeto(15);
		flags[7] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else if (objeto_que_lleva == LOOK && fl == 147)
		talk(_text[_lang][59], "59.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_9(int fl){
	if (objeto_que_lleva == LOOK && fl == 150)
		talk(_text[_lang][35], "35.als");
	else if (objeto_que_lleva == TALK && fl == 150)
		talk(_text[_lang][6], "6.als");
	else if (objeto_que_lleva == LOOK && fl == 51)
		talk(_text[_lang][60], "60.als");
	else if (objeto_que_lleva == TALK && fl == 51 && flags[4] == 0)
		animation_4_2();
	else if (objeto_que_lleva == TALK && fl == 51 && flags[4] == 1)
		animation_33_2();
	else if (objeto_que_lleva == 7 && fl == 51) {
		animation_6_2();
		resta_objeto(7);
		agarra_objeto(14);}
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_12(int fl){
	if (objeto_que_lleva == LOOK && fl == 154)
		talk(_text[_lang][329], "329.als");
	else if (objeto_que_lleva == TALK && fl == 154)
		talk(_text[_lang][330], "330.als");
	else if (objeto_que_lleva == MOVE && fl == 155)
		talk(_text[_lang][48], "48.als");
	else if (objeto_que_lleva == TALK && fl == 155)
		talk(_text[_lang][331], "331.als");
	else if (objeto_que_lleva == LOOK && fl == 156)
		talk(_text[_lang][35], "35.als");
	else if (objeto_que_lleva == MOVE && fl == 156)
		talk(_text[_lang][48], "48.als");
	else if (objeto_que_lleva == TALK && fl == 156)
		talk(_text[_lang][50], "50.als");
	else if (objeto_que_lleva == OPEN && fl == 156)
		abre_puerta(16, 4);
	else if (objeto_que_lleva == CLOSE && fl == 156)
		cierra_puerta(16, 4);
	else
		hay_respuesta = 0;
}

bool DrasculaEngine::room_13(int fl) {
	if (objeto_que_lleva == LOOK && fl == 51) {
		talk(_text[_lang][411], "411.als");
		sentido_hare = 3;
		talk(_text[_lang][412], "412.als");
		strcpy(nombre_obj[1], "yoda");
	} else if (objeto_que_lleva == TALK && fl == 51)
		conversa("op_7.cal");
	else if (objeto_que_lleva == 19 && fl == 51)
		animation_1_3();
	else if (objeto_que_lleva == 9 && fl == 51) {
		animation_2_3();
		return true;
	} else
		hay_respuesta = 0;
	return false;
}

void DrasculaEngine::room_14(int fl) {
	if (objeto_que_lleva == TALK && fl == 54 && flags[39] == 0)
		animation_12_2();
	else if (objeto_que_lleva == TALK && fl == 54 && flags[39] == 1)
		talk(_text[_lang][109], "109.als");
	else if (objeto_que_lleva == 12 && fl == 54)
		animation_26_2();
	else if (objeto_que_lleva == TALK && fl == 52 && flags[5] == 0)
		animation_11_2();
	else if (objeto_que_lleva == TALK && fl == 52 && flags[5] == 1)
		animation_36_2();
	else if (objeto_que_lleva == TALK && fl == 53)
		animation_13_2();
	else if (objeto_que_lleva == LOOK && fl == 200)
		talk(_text[_lang][165], "165.als");
	else if (objeto_que_lleva == LOOK && fl == 201)
		talk(_text[_lang][166], "166.als");
	else if (objeto_que_lleva == LOOK && fl == 202)
		talk(_text[_lang][167], "167.als");
	else if (objeto_que_lleva == LOOK && fl == 203)
		talk(_text[_lang][168], "168.als");
	else if (objeto_que_lleva == PICK && fl == 203)
		talk(_text[_lang][170], "170.als");
	else if (objeto_que_lleva == MOVE && fl == 203)
		talk(_text[_lang][170], "170.als");
	else if (objeto_que_lleva == TALK && fl == 203)
		talk(_text[_lang][169], "169.als");
	else if (objeto_que_lleva == LOOK && fl == 204)
		talk(_text[_lang][171], "171.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_15(int fl) {
	if (objeto_que_lleva == TALK && fl == 188)
		talk(_text[_lang][333], "333.als");
	else if (objeto_que_lleva == LOOK && fl == 188)
		talk(_text[_lang][334], "334.als");
	else if (objeto_que_lleva == 19 && fl == 188 && flags[27] == 0)
		talk(_text[_lang][335], "335.als");
	else if (objeto_que_lleva == 19 && fl == 188 && flags[27] == 1) {
		talk(_text[_lang][336], "336.als");
		sentido_hare = 3;
		talk(_text[_lang][337], "337.als");
		talk_sinc(_text[_lang][46], "46.als", "4442444244244");
		sentido_hare = 1;
	} else if (objeto_que_lleva == 18 && fl == 188 && flags[26] == 0) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		copyRect(133, 135, hare_x + 6, hare_y, 39, 63, dir_dibujo3, dir_zona_pantalla);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		comienza_sound("s8.als");
		fin_sound();
		talk(_text[_lang][338], "338.als");
		flags[27] = 0;
		agarra_objeto(19);
		resta_objeto(18);
	} else if (objeto_que_lleva == MOVE && fl == 188 && flags[27] == 0) {
		animation_34_2();
		talk(_text[_lang][339], "339.als");
		agarra_objeto(16);
		flags[26] = 1;
		flags[27] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else if (objeto_que_lleva == LOOK && fl == 205)
		talk(_text[_lang][172], "172.als");
	else if (objeto_que_lleva == LOOK && fl == 206)
		talk(_text[_lang][173], "173.als");
	else if (objeto_que_lleva == MOVE && fl == 206)
		talk(_text[_lang][174], "174.als");
	else if (objeto_que_lleva == OPEN && fl == 206)
		talk(_text[_lang][174], "174.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_16(int fl) {
	if (objeto_que_lleva == TALK && fl == 163)
		talk(_text[_lang][6], "6.als");
	else if (objeto_que_lleva == OPEN && fl == 163)
		abre_puerta(17, 0);
	else if (objeto_que_lleva == CLOSE && fl == 163)
		cierra_puerta(17, 0);
	else if (objeto_que_lleva == LOOK && fl == 183)
		talk(_text[_lang][340], "340.als");
	else if (objeto_que_lleva == TALK && fl == 183) {
		talk(_text[_lang][341], "341.als");
		pause(10);
		talk_sinc(_text[_lang][50], "50.als", "11111111111144432554433");
		pause(3);
		talk_baul(_text[_lang][83], "d83.als");
	} else if (objeto_que_lleva == OPEN && fl == 183) {
		abre_puerta(19, NO_PUERTA);
		if (flags[20] == 0) {
			flags[20] = 1;
			sentido_hare = 3;
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			talk(_text[_lang][342], "342.als");
			agarra_objeto(22);
		}
	} else if (objeto_que_lleva == CLOSE && fl == 183)
		cierra_puerta(19, NO_PUERTA);
	else if (objeto_que_lleva == LOOK && fl == 185)
		talk(_text[_lang][37], "37.als");
	else if (objeto_que_lleva == PICK && fl == 185)
		talk(_text[_lang][7], "7.als");
	else if (objeto_que_lleva == MOVE && fl == 185)
		talk(_text[_lang][7], "7.als");
	else if (objeto_que_lleva == TALK && fl == 185)
		talk(_text[_lang][38], "38.als");
	else if (objeto_que_lleva == LOOK && fl == 187) {
		talk(_text[_lang][343], "343.als");
		sentido_hare = 3;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		talk(_text[_lang][344], "344.als");
	} else if (objeto_que_lleva == TALK && fl == 187)
		talk(_text[_lang][345], "345.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_17(int fl) {
	if (objeto_que_lleva == LOOK && fl == 177)
		talk(_text[_lang][35], "35.als");
	else if (objeto_que_lleva == TALK && fl == 177 && flags[18] == 0)
		talk(_text[_lang][6], "6.als");
	else if (objeto_que_lleva == TALK && fl == 177 && flags[18] == 1)
		animation_18_2();
	else if (objeto_que_lleva == OPEN && fl == 177 && flags[18] == 1)
		talk(_text[_lang][346], "346.als");
	else if (objeto_que_lleva == OPEN && fl == 177 && flags[14] == 0 && flags[18] == 0)
		animation_22_2();
	else if (objeto_que_lleva == OPEN && fl == 177 && flags[14] == 1)
		abre_puerta(15, 1);
	else if (objeto_que_lleva == CLOSE && fl == 177 && flags[14] == 1)
		cierra_puerta(15, 1);
	else if (objeto_que_lleva == 11 && fl == 50 && flags[22] == 0) {
		talk(_text[_lang][347], "347.als");
		flags[29] = 1;
		agarra_objeto(23);
		resta_objeto(11);
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_18(int fl) {
	if (objeto_que_lleva == TALK && fl == 55 && flags[36] == 0)
		animation_24_2();
	else if (objeto_que_lleva == TALK && fl == 55 && flags[36] == 1)
		talk(_text[_lang][109], "109.als");
	else if (objeto_que_lleva == LOOK && fl == 181)
		talk(_text[_lang][348], "348.als");
	else if (objeto_que_lleva == PICK && fl == 182) {
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		copyRect(44, 1, hare_x, hare_y, 41, 70, dir_dibujo2, dir_zona_pantalla);
		updateRefresh();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(4);
		agarra_objeto(12);
		visible[2] = 0;
		flags[28] = 1;
	} else if (objeto_que_lleva == LOOK && fl == 182)
		talk(_text[_lang][154], "154.als");
	else if (objeto_que_lleva == 8 && fl == 55 && flags[38] == 0 && flags[33] == 1)
		talk(_text[_lang][349], "349.als");
	else if (objeto_que_lleva == 13 && fl == 55 && flags[38] == 0 && flags[33] == 1)
		talk(_text[_lang][349], "349.als");
	else if (objeto_que_lleva == 15 && fl == 55 && flags[38] == 0 && flags[33] == 1)
		talk(_text[_lang][349], "349.als");
	else if (objeto_que_lleva == 16 && fl == 55 && flags[38] == 0 && flags[33] == 1)
		talk(_text[_lang][349], "349.als");
	else if (objeto_que_lleva == 17 && fl == 55 && flags[38] == 0 && flags[33] == 1)
		talk(_text[_lang][349], "349.als");
	else if (objeto_que_lleva == 8 && fl == 55 && flags[38] == 1 && flags[33] == 1)
		animation_24_2();
	else if (objeto_que_lleva == 13 && fl == 55 && flags[38] == 1 && flags[33] == 1)
		animation_24_2();
	else if (objeto_que_lleva == 15 && fl == 55 && flags[38] == 1 && flags[33] == 1)
		animation_24_2();
	else if (objeto_que_lleva == 16 && fl == 55 && flags[38] == 1 && flags[33] == 1)
		animation_24_2();
	else if (objeto_que_lleva == 17 && fl == 55 && flags[38] == 1 && flags[33] == 1)
		animation_24_2();
	else if (objeto_que_lleva == 11 && fl == 50 && flags[22] == 0) {
		sentido_hare = 3;
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		updateRefresh_pre();
		copyRect(1, 1, hare_x - 1, hare_y + 3, 42, 67, dir_dibujo2, dir_zona_pantalla);
		updateRefresh();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pause(6);
		talk(_text[_lang][347], "347.als");
		flags[29] = 1;
		agarra_objeto(23);
		resta_objeto(11);
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_19(int fl) {
	if (objeto_que_lleva == LOOK && fl == 214)
		talk(_text[_lang][191], "191.als");
	else
		hay_respuesta = 0;
}

bool DrasculaEngine::room_21(int fl) {
	if (objeto_que_lleva == OPEN && fl == 101 && flags[28] == 0)
		talk(_text[_lang][419], "419.als");
	else if (objeto_que_lleva == OPEN && fl == 101 && flags[28] == 1)
		abre_puerta(0, 1);
	else if (objeto_que_lleva == CLOSE && fl == 101)
		cierra_puerta(0, 1);
	else if(objeto_que_lleva == PICK && fl == 141) {
		agarra_objeto(19);
		visible[2] = 0;
		flags[10] = 1;
	} else if(objeto_que_lleva == 7 && fl == 101) {
		flags[28] = 1;
		abre_puerta(0, 1);
		sin_verbo();
	} else if (objeto_que_lleva == 21 && fl == 179) {
		animation_9_4();
		return true;
	} else
		hay_respuesta = 0;

	return false;
}

void DrasculaEngine::room_22(int fl) {
	if (objeto_que_lleva == PICK && fl == 140)
		talk(_text[_lang][7], "7.als");
	else if (objeto_que_lleva == 11 && fl == 140) {
		agarra_objeto(18);
		visible[1] = 0;
		flags[24] = 1;
	} else if (objeto_que_lleva == 22 && fl == 52) {
		anima("up.bin",14);
		flags[26]=1;
		comienza_sound("s1.als");
		hipo(14);
		fin_sound();
		sin_verbo();
		resta_objeto(22);
		actualiza_datos();
		sentido_hare = 3;
		talk(_text[_lang][499], "499.als");
		talk(_text[_lang][500], "500.als");
	} else if (objeto_que_lleva == LOOK && fl == 52)
		talk(_text[_lang][497], "497.als");
	else if (objeto_que_lleva == TALK && fl == 52)
		talk(_text[_lang][498], "498.als");
	else if (objeto_que_lleva == PICK && fl == 180 && flags[26] == 0)
		talk(_text[_lang][420], "420.als");
	else if (objeto_que_lleva == PICK && fl == 180 && flags[26] == 1) {
		agarra_objeto(7);
		visible[3] = 0;
		flags[27] = 1;
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_23(int fl) {
	if (objeto_que_lleva == OPEN && fl == 103) {
		abre_puerta(0, 0);
		actualiza_datos();
	} else if(objeto_que_lleva == CLOSE && fl == 103) {
		cierra_puerta(0, 0);
		actualiza_datos();
	} else if(objeto_que_lleva == OPEN && fl == 104)
		abre_puerta(1, 1);
	else if(objeto_que_lleva == CLOSE && fl == 104)
		cierra_puerta(1, 1);
	else if(objeto_que_lleva == PICK && fl == 142) {
		agarra_objeto(8);
		visible[2] = 0;
		flags[11] = 1;
		if (flags[22] == 1 && flags[14] == 1)
			flags[18] = 1;
		if (flags[18] == 1)
			animation_6_4();
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_24(int fl) {
	if (objeto_que_lleva == OPEN && fl == 105)
		abre_puerta(1, 0);
	else if (objeto_que_lleva == CLOSE && fl == 105)
		cierra_puerta(1, 0);
	else if (objeto_que_lleva == OPEN && fl == 106)
		abre_puerta(2, 1);
	else if (objeto_que_lleva == CLOSE && fl == 106)
		cierra_puerta(2, 1);
	else if (objeto_que_lleva == LOOK && fl == 151)
		talk(_text[_lang][461], "461.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_26(int fl) {
	if (objeto_que_lleva == OPEN && fl == 107 && flags[30] == 0)
		abre_puerta(2, 0);
	else if (objeto_que_lleva == OPEN && fl == 107 && flags[30] == 1)
		talk(_text[_lang][421], "421.als");
	else if (objeto_que_lleva == CLOSE && fl == 107)
		cierra_puerta(2, 0);
	else if (objeto_que_lleva == 10 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (objeto_que_lleva == 8 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (objeto_que_lleva == 12 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (objeto_que_lleva == 16 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (objeto_que_lleva == PICK && fl == 143 && flags[18] == 1) {
		lleva_al_hare(260, 180);
		agarra_objeto(10);
		visible[1] = 0;
		flags[12] = 1;
		cierra_puerta(2, 0);
		sentido_hare = 2;
		talk_igorpuerta(_texti[_lang][27], "I27.als");
		flags[30] = 1;
		talk_igorpuerta(_texti[_lang][28], "I28.als");
		lleva_al_hare(153, 180);
	} else if (objeto_que_lleva == PICK && fl == 143 && flags[18] == 0) {
		lleva_al_hare(260, 180);
		copyBackground(80, 78, 199, 94, 38, 27, dir_dibujo3, dir_zona_pantalla);
		updateScreen(199, 94, 199, 94, 38, 27, dir_zona_pantalla);
		pause(3);
		talk_igor_peluca(_texti[_lang][25], "I25.als");
		lleva_al_hare(153, 180);
	} else if (objeto_que_lleva == TALK && fl == 51)
		animation_1_4();
	else if (objeto_que_lleva == OPEN && fl == 167)
		talk(_text[_lang][467], "467.als");
	else if (objeto_que_lleva == LOOK && fl == 164)
		talk(_text[_lang][470], "470.als");
	else if (objeto_que_lleva == OPEN && fl == 164)
		talk(_text[_lang][471], "471.als");
	else if (objeto_que_lleva == LOOK && fl == 163)
		talk(_text[_lang][472], "472.als");
	else if (objeto_que_lleva == PICK && fl == 163)
		talk(_text[_lang][473], "473.als");
	else if (objeto_que_lleva == LOOK && fl == 165)
		talk(_text[_lang][474], "474.als");
	else if (objeto_que_lleva == LOOK && fl == 168)
		talk(_text[_lang][476], "476.als");
	else if (objeto_que_lleva == PICK && fl == 168)
		talk(_text[_lang][477], "477.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_27(int fl) {
	if (objeto_que_lleva == OPEN && fl == 110)
		abre_puerta(6, 1);
	else if (objeto_que_lleva == CLOSE && fl == 110)
		cierra_puerta(6, 1);
	else if (objeto_que_lleva == OPEN && fl == 116 && flags[23] == 0)
		talk(_text[_lang][419], "419.als");
	else if (objeto_que_lleva == OPEN && fl == 116 && flags[23] == 1)
		abre_puerta(5, 3);
	else if (objeto_que_lleva == 17 && fl == 116) {
		flags[23] = 1;
		abre_puerta(5,3);
		sin_verbo();
	} else if (objeto_que_lleva == LOOK && fl == 175)
		talk(_text[_lang][429], "429.als");
	else if (fl == 150)
		talk(_text[_lang][460], "460.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_29(int fl) {
	if (objeto_que_lleva == OPEN && fl == 114)
		abre_puerta(4, 1);
	else if (objeto_que_lleva == CLOSE && fl == 114)
		cierra_puerta(4, 1);
	else if (objeto_que_lleva == LOOK && fl == 152)
		talk(_text[_lang][463], "463.als");
	else if (objeto_que_lleva == OPEN && fl == 152)
		talk(_text[_lang][464], "464.als");
	else if (objeto_que_lleva == LOOK && fl == 153)
		talk(_text[_lang][465], "465.als");
	else if (objeto_que_lleva == PICK && fl == 154)
		talk(_text[_lang][466], "466.als");
	else if (objeto_que_lleva == OPEN && fl == 156)
		talk(_text[_lang][467], "467.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_30(int fl) {
	if (objeto_que_lleva == OPEN && fl == 115)
		abre_puerta(4, 0);
	else if (objeto_que_lleva == CLOSE && fl == 115)
		cierra_puerta(4, 0);
	else if (objeto_que_lleva == OPEN && fl == 144 && flags[19] == 0)
		talk(_text[_lang][422], "422.als");
	else if (objeto_que_lleva == OPEN && fl == 144 && flags[19] == 1 && flags[22] == 1)
		abre_puerta(16, 1);
	else if (objeto_que_lleva == OPEN && fl == 144 && flags[19] == 1 && flags[22] == 0) {
		abre_puerta(16, 1);
		talk(_text[_lang][423], "423.als");
		flags[22] = 1;
		agarra_objeto(12);
		if (flags[11] == 1 && flags[14] == 1)
			flags[18] = 1;
		if (flags[18] == 1)
			animation_6_4();
	} else if (objeto_que_lleva == CLOSE && fl == 144)
		cierra_puerta(16, 1);
	else if (objeto_que_lleva == 13 && fl == 144) {
		talk(_text[_lang][424], "424.als");
		flags[19] = 1;
	} else if (objeto_que_lleva == OPEN && fl == 157)
		talk(_text[_lang][468], "468.als");
	else if (objeto_que_lleva == LOOK && fl == 158)
		talk(_text[_lang][469], "469.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_31(int fl) {
	if (objeto_que_lleva == PICK && fl == 145) {
		agarra_objeto(11);
		visible[1] = 0;
		flags[13] = 1;
	} else if (objeto_que_lleva == OPEN && fl == 117)
		abre_puerta(5, 0);
	else if (objeto_que_lleva == CLOSE && fl == 117)
		cierra_puerta(5, 0);
	else if (objeto_que_lleva == LOOK && fl == 161)
		talk(_text[_lang][470], "470.als");
	else if (objeto_que_lleva == OPEN && fl == 161)
		talk(_text[_lang][471], "471.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_34(int fl) {
	if (objeto_que_lleva == MOVE && fl == 146)
		animation_8_4();
	else if (objeto_que_lleva == LOOK && fl == 146)
		talk(_text[_lang][458], "458.als");
	else if (objeto_que_lleva == PICK && fl == 146)
		talk(_text[_lang][459], "459.als");
	else if (objeto_que_lleva == OPEN && fl == 120 && flags[25] == 1)
		abre_puerta(8, 2);
	else if (objeto_que_lleva == OPEN && fl == 120 && flags[25] == 0) {
		abre_puerta(8, 2);
		sentido_hare = 3;
		talk(_text[_lang][425], "425.als");
		agarra_objeto(14);
		flags[25] = 1;
	} else if (objeto_que_lleva == CLOSE && fl == 120)
		cierra_puerta(8, 2);
	else
		hay_respuesta=0;
}

void DrasculaEngine::room_35(int fl) {
	if (objeto_que_lleva == PICK && fl == 148) {
		agarra_objeto(16);
		visible[2] = 0;
		flags[14] = 1;
	if (flags[11] == 1 && flags[22] == 1)
		flags[18] = 1;
	if (flags[18] == 1)
		animation_6_4();
	} else if (objeto_que_lleva == PICK && fl == 147) {
		talk(_text[_lang][426], "426.als");
		agarra_objeto(15);
		visible[1] = 0;
		flags[15] = 1;
		flags[17] = 1;
		actualiza_datos();
	} else if (objeto_que_lleva == PICK && fl == 149) {
		agarra_objeto(13);
		visible[3] = 0;
		flags[17] = 0;
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_44(int fl) {
	if (objeto_que_lleva == LOOK && fl == 172)
		talk(_text[_lang][428], "428.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_49(int fl){
	if (objeto_que_lleva == TALK && fl ==51)
		conversa("op_9.cal");
	else if (objeto_que_lleva == LOOK && fl == 51)
		talk(_text[_lang][132], "132.als");
	else if ((objeto_que_lleva == 8 && fl == 51) || (objeto_que_lleva == 8 && fl == 203))
		animation_5_5();
	else if (objeto_que_lleva == LOOK && fl == 200)
		talk(_text[_lang][133], "133.als");
	else if (objeto_que_lleva == TALK && fl == 200)
		talk(_text[_lang][134], "134.als");
	else if (objeto_que_lleva == LOOK && fl == 201)
		talk(_text[_lang][135], "135.als");
	else if (objeto_que_lleva == LOOK && fl == 203)
		talk(_text[_lang][137], "137.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_53(int fl) {
	if (objeto_que_lleva == PICK && fl == 120) {
		agarra_objeto(16);
		visible[3] = 0;
	} else if (objeto_que_lleva == LOOK && fl == 121)
		talk(_text[_lang][128], "128.als");
	else if (objeto_que_lleva == LOOK && fl == 209)
		talk(_text[_lang][129], "129.als");
	else if (objeto_que_lleva == MOVE && fl == 123)
		animation_11_5();
	else if (objeto_que_lleva == LOOK && fl == 52)
		talk(_text[_lang][447], "447.als");
	else if (objeto_que_lleva == TALK && fl == 52)
		talk(_text[_lang][131], "131.als");
	else if (objeto_que_lleva == 12 && fl == 52)
		animation_10_5();
	else if (objeto_que_lleva == 15 && fl == 52)
		animation_9_5();
	else if (objeto_que_lleva == 16 && fl == 121) {
		flags[2] = 1;
		sin_verbo();
		actualiza_datos();
	} else if (objeto_que_lleva == 16) {
		talk(_text[_lang][439], "439.als");
		sin_verbo();
		visible[3] = 1;
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_54(int fl) {
	if ((objeto_que_lleva == TALK && fl == 118) || (objeto_que_lleva == LOOK && fl == 118 && flags[0] == 0))
		animation_1_5();
	else if (objeto_que_lleva == LOOK && fl == 118 && flags[0]==1)
		talk(_text[_lang][124], "124.als");
	else if (objeto_que_lleva == LOOK && fl == 53)
		talk(_text[_lang][127], "127.als");
	else if (objeto_que_lleva == TALK && fl == 53 && flags[14] == 0) {
		talk(_text[_lang][288], "288.als");
		flags[12] = 1;
		pause(10);
		talk_mus(_texte[_lang][1], "E1.als");
		talk(_text[_lang][289], "289.als");
		talk_mus(_texte[_lang][2], "E2.als");
		talk_mus(_texte[_lang][3], "E3.als");
		conversa("op_10.cal");
		flags[12] = 0;
		flags[14] = 1;
	} else if (objeto_que_lleva == TALK && fl == 53 && flags[14] == 1)
		talk(_text[_lang][109], "109.als");
	else if (objeto_que_lleva == PICK && fl == 9999 && flags[13] == 0) {
		agarra_objeto(8);
		flags[13] = 1;
		talk_mus(_texte[_lang][10], "e10.als");
		actualiza_datos();
	} else if (objeto_que_lleva == OPEN && fl == 119)
		talk(_text[_lang][125], "125.als");
	else if (objeto_que_lleva == LOOK && fl == 119)
		talk(_text[_lang][126], "126.als");
	else if (objeto_que_lleva == 10 && fl == 119) {
		pause(4);
		talk(_text[_lang][436], "436.als");
		sin_verbo();
		resta_objeto(10);
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::room_55(int fl) {
	if (objeto_que_lleva == PICK && fl == 122) {
		agarra_objeto(12);
		flags[8] = 1;
		actualiza_datos();
	} else if (objeto_que_lleva == LOOK && fl == 122)
		talk(_text[_lang][138], "138.als");
	else if (objeto_que_lleva == LOOK && fl == 204)
		talk(_text[_lang][139], "139.als");
	else if (objeto_que_lleva == LOOK && fl == 205)
		talk(_text[_lang][140], "140.als");
	else if (fl == 206) {
		comienza_sound("s11.als");
		anima("det.bin", 17);
		fin_sound();
		lleva_al_hare(hare_x - 3, hare_y + alto_hare + 6);
	} else
		hay_respuesta = 0;
}

bool DrasculaEngine::room_56(int fl) {
	if (objeto_que_lleva == OPEN && fl == 124) {
		animation_14_5();
		return true;
	} else if (objeto_que_lleva == LOOK && fl == 124)
		talk(_text[_lang][450], "450.als");
	else if (objeto_que_lleva == OPEN && fl == 207)
		talk(_text[_lang][141], "141.als");
	else if (objeto_que_lleva == LOOK && fl == 208)
		talk(_text[_lang][142], "142.als");
	else
		hay_respuesta = 0;

	return false;
}

void DrasculaEngine::room_58(int fl) {
	if (objeto_que_lleva == MOVE && fl == 103)
		animation_7_6();
	else if (objeto_que_lleva == LOOK && fl == 104)
		talk(_text[_lang][454], "454.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_59(int fl) {
	if ((objeto_que_lleva == TALK && fl == 51) || (objeto_que_lleva == LOOK && fl == 51)) {
		flags[9] = 1;
		talk(_text[_lang][259], "259.als");
		talk_bj_cama(_textbj[_lang][13], "bj13.als");
		talk(_text[_lang][263], "263.als");
		talk_bj_cama(_textbj[_lang][14], "bj14.als");
		pause(40);
		talk(_text[_lang][264], "264.als");
		talk_bj_cama(_textbj[_lang][15], "BJ15.als");
		talk(_text[_lang][265], "265.als");
		flags[9] = 0;
		if (flags[11] == 0) {
			comienza_sound("s12.als");
			delay(40);
			fin_sound();
			delay(10);
			lleva_al_hare(174, 168);
			sentido_hare = 2;
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			pause(40);
			comienza_sound("s12.als");
			pause(19);
			fin_sound_corte();
			hare_se_ve = 0;
			updateRoom();
			copyRect(101, 34, hare_x - 4, hare_y - 1, 37, 70, dir_dibujo3, dir_zona_pantalla);
			copyBackground(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_ve = 1;
			clearRoom();
			loadPic("tlef0.alg");
			decompressPic(dir_dibujo1, COMPLETA);
			loadPic("tlef1.alg");
			decompressPic(dir_dibujo3, 1);
			loadPic("tlef2.alg");
			decompressPic(dir_hare_frente, 1);
			loadPic("tlef3.alg");
			decompressPic(dir_hare_fondo, 1);
			talk_htel(_text[_lang][240], "240.als");

			color_abc(VON_BRAUN);
			talk_solo(_textvb[_lang][58], "VB58.als");
			talk_htel(_text[_lang][241], "241.als");
			color_abc(VON_BRAUN);
			talk_solo(_textvb[_lang][59], "VB59.als");
			talk_htel(_text[_lang][242], "242.als");
			color_abc(VON_BRAUN);
			talk_solo(_textvb[_lang][60], "VB60.als");
			talk_htel(_text[_lang][196], "196.als");
			color_abc(VON_BRAUN);
			talk_solo(_textvb[_lang][61],"VB61.als");
			talk_htel(_text[_lang][244], "244.als");
			color_abc(VON_BRAUN);
			talk_solo(_textvb[_lang][62], "VB62.als");
			clearRoom();
			loadPic("aux59.alg");
			decompressPic(dir_dibujo3, 1);
			loadPic("96.alg");
			decompressPic(dir_hare_frente, COMPLETA);
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);
			loadPic("59.alg");
			decompressPic(dir_dibujo1, MEDIA);
			sentido_hare = 3;
			talk(_text[_lang][245], "245.als");
			sin_verbo();
			flags[11] = 1;
		}
	} else
		hay_respuesta = 0;
}

bool DrasculaEngine::room_60(int fl) {
	if (objeto_que_lleva == MOVE && fl == 112)
		animation_10_6();
	else if (objeto_que_lleva == LOOK && fl == 112)
		talk(_text[_lang][440], "440.als");
	else if (objeto_que_lleva == TALK && fl == 52) {
		talk(_text[_lang][266], "266.als");
		talk_taber2(_textt[_lang][1], "t1.als");
		conversa("op_12.cal");
		sin_verbo();
		objeto_que_lleva = 0;
	} else if (objeto_que_lleva == TALK && fl == 115)
		talk(_text[_lang][455], "455.als");
	else if (objeto_que_lleva == TALK && fl == 56)
		talk(_text[_lang][455], "455.als");
	else if (objeto_que_lleva == LOOK && fl == 114)
		talk(_text[_lang][167], "167.als");
	else if (objeto_que_lleva == LOOK && fl == 113)
		talk(_text[_lang][168], "168.als");
	else if (objeto_que_lleva == PICK && fl == 113)
		talk(_text[_lang][170], "170.als");
	else if (objeto_que_lleva == MOVE && fl == 113)
		talk(_text[_lang][170], "170.als");
	else if (objeto_que_lleva == TALK && fl == 113)
		talk(_text[_lang][169], "169.als");
	else if (objeto_que_lleva == 21 && fl == 56)
		animation_18_6();
	else if (objeto_que_lleva == 9 && fl == 56 && flags[6] == 1) {
		animation_9_6();
		return true;
	} else if (objeto_que_lleva == 9 && fl == 56 && flags[6] == 0) {
		anima("cnf.bin", 14);
		talk(_text[_lang][455], "455.als");
	} else
		hay_respuesta = 0;

	return false;
}

void DrasculaEngine::room_61(int fl) {
	if (objeto_que_lleva == LOOK && fl == 116)
		talk(_text[_lang][172], "172.als");
	else if (objeto_que_lleva == LOOK && fl == 117)
		talk(_text[_lang][173], "173.als");
	else if (objeto_que_lleva == MOVE && fl == 117)
		talk(_text[_lang][174], "174.als");
	else if (objeto_que_lleva == OPEN && fl == 117)
		talk(_text[_lang][174], "174.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_62(int fl) {
	if (objeto_que_lleva == TALK && fl == 53)
		conversa("op_13.cal");
	else if (objeto_que_lleva == TALK && fl == 52 && flags[0] == 0)
		animation_3_1();
	else if (objeto_que_lleva == TALK && fl == 52 && flags[0] == 1)
		talk(_text[_lang][109], "109.als");
	else if (objeto_que_lleva == TALK && fl == 54)
		animation_4_1();
	else if (objeto_que_lleva == LOOK && fl == 100)
		talk(_text[_lang][168], "168.als");
	else if (objeto_que_lleva == TALK && fl == 100)
		talk(_text[_lang][169], "169.als");
	else if (objeto_que_lleva == PICK && fl == 100)
		talk(_text[_lang][170], "170.als");
	else if (objeto_que_lleva == LOOK && fl == 101)
		talk(_text[_lang][171], "171.als");
	else if (objeto_que_lleva == LOOK && fl == 102)
		talk(_text[_lang][167], "167.als");
	else if (objeto_que_lleva == LOOK && fl == 103)
		talk(_text[_lang][166], "166.als");
	else hay_respuesta = 0;
}

void DrasculaEngine::room_63(int fl) {
	if (objeto_que_lleva == LOOK && fl == 110)
		talk(_text[_lang][172], "172.als");
	else if (objeto_que_lleva == LOOK && fl == 109)
		talk(_text[_lang][173], "173.als");
	else if (objeto_que_lleva == MOVE && fl == 109)
		talk(_text[_lang][174], "174.als");
	else if (objeto_que_lleva == LOOK && fl == 108)
		talk(_text[_lang][334], "334.als");
	else if (objeto_que_lleva == TALK && fl == 108)
		talk(_text[_lang][333], "333.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::room_pendulo(int fl) {
	if (objeto_que_lleva == LOOK && fl == 100)
		talk(_text[_lang][452], "452.als");
	else if (objeto_que_lleva == LOOK && fl == 101)
		talk (_text[_lang][123], "123.als");
	else if (objeto_que_lleva == PICK && fl == 101)
		agarra_objeto(20);
	else if (objeto_que_lleva == 20 && fl == 100)
		animation_6_6();
	else if (objeto_que_lleva == PICK || objeto_que_lleva == OPEN)
		talk(_text[_lang][453], "453.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::updateRefresh() {
	if (num_ejec == 1) {
		if (!strcmp(num_room, "63.alg"))
			update_63();
		else if (!strcmp(num_room, "62.alg"))
			update_62();
	} else if (num_ejec == 2) {
		if (!strcmp(num_room, "3.alg"))
			update_3();
		else if (!strcmp(num_room, "2.alg"))
			update_2();
		else if (!strcmp(num_room, "4.alg"))
			update_4();
		else if (!strcmp(num_room, "5.alg"))
			update_5();
		else if (!strcmp(num_room, "15.alg"))
			update_15();
		else if (!strcmp(num_room, "17.alg"))
			update_17();
		else if (!strcmp(num_room, "18.alg"))
			update_18();
		else if (!strcmp(num_room, "10.alg"))
			mapa();
	} else if (num_ejec == 3) {
		if (!strcmp(num_room, "20.alg"))
			update_20();
		else if (!strcmp(num_room, "13.alg"))
			update_13();
	} else if (num_ejec == 4) {
		if (!strcmp(num_room, "29.alg"))
			update_29();
		else if (!strcmp(num_room, "26.alg"))
			update_26();
		else if (!strcmp(num_room, "27.alg"))
			update_27();
		else if (!strcmp(num_room, "31.alg"))
			update_31();
		else if (!strcmp(num_room, "34.alg"))
			update_34();
		else if (!strcmp(num_room, "35.alg"))
			update_35();
	} else if (num_ejec == 5) {
		if (!strcmp(num_room, "45.alg"))
			mapa();
		else if (!strcmp(num_room, "50.alg"))
			update_50();
		else if (!strcmp(num_room, "57.alg"))
			update_57();
	} else if (num_ejec == 6) {
		if (!strcmp(num_room, "60.alg"))
			update_60();
		else if (!strcmp(num_room, "61.alg"))
			update_61();
		else if (!strcmp(num_room, "58.alg"))
			update_58();
	}
}

void DrasculaEngine::updateRefresh_pre() {
	if (num_ejec == 1) {
		if (!strcmp(num_room, "62.alg"))
			update_62_pre();
		else if (!strcmp(num_room, "16.alg"))
			pon_bj();
	} else if (num_ejec == 2) {
		if (!strcmp(num_room, "1.alg"))
			update_1_pre();
		else if (!strcmp(num_room, "3.alg"))
			update_3_pre();
		else if (!strcmp(num_room, "5.alg"))
			update_5_pre();
		else if (!strcmp(num_room, "6.alg"))
			update_6_pre();
		else if (!strcmp(num_room, "7.alg"))
			update_7_pre();
		else if (!strcmp(num_room, "9.alg"))
			update_9_pre();
		else if (!strcmp(num_room, "12.alg"))
			update_12_pre();
		else if (!strcmp(num_room, "14.alg"))
			update_14_pre();
		else if (!strcmp(num_room, "16.alg"))
			update_16_pre();
		else if (!strcmp(num_room, "17.alg"))
			update_17_pre();
		else if (!strcmp(num_room, "18.alg"))
			update_18_pre();
	} else if (num_ejec == 3) {
		// nothing
	} else if (num_ejec == 4) {
		if (!strcmp(num_room, "21.alg"))
			update_21_pre();
		else if (!strcmp(num_room, "22.alg"))
			update_22_pre();
		else if (!strcmp(num_room, "23.alg"))
			update_23_pre();
		else if (!strcmp(num_room, "24.alg"))
			update_24_pre();
		else if (!strcmp(num_room, "26.alg"))
			update_26_pre();
		else if (!strcmp(num_room, "27.alg"))
			update_27_pre();
		else if (!strcmp(num_room, "29.alg"))
			update_29_pre();
		else if (!strcmp(num_room, "30.alg"))
			update_30_pre();
		else if (!strcmp(num_room, "31.alg"))
			update_31_pre();
		else if (!strcmp(num_room, "34.alg"))
			update_34_pre();
		else if (!strcmp(num_room, "35.alg"))
			update_35_pre();
	} else if (num_ejec == 5) {
		if (!strcmp(num_room,"49.alg"))
			update_49_pre();
		else if (!strcmp(num_room,"53.alg"))
			update_53_pre();
		else if (!strcmp(num_room,"54.alg"))
			update_54_pre();
		else if (!strcmp(num_room,"56.alg"))
			update_56_pre();
	} else if (num_ejec == 6) {
		if (!strcmp(num_room, "102.alg"))
			update_pendulo();
		else if (!strcmp(num_room, "58.alg"))
			update_58_pre();
		else if (!strcmp(num_room, "59.alg"))
			update_59_pre();
		else if (!strcmp(num_room, "60.alg"))
			update_60_pre();
	}
}

void DrasculaEngine::update_1_pre() {
	int cambio_col_antes = cambio_de_color;

	if (hare_x > 98 && hare_x < 153)
		cambio_de_color = 1;
	else
		cambio_de_color = 0;

	if (cambio_col_antes != cambio_de_color && cambio_de_color == 1)
		hare_oscuro();
	if (cambio_col_antes != cambio_de_color && cambio_de_color == 0)
		hare_claro();

	if (flags[8] == 0)
		copyBackground(2, 158, 208, 67, 27, 40, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_2(){
	int pos_murci[6];
	int diferencia;
	int murci_x[] = {0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190,
					0, 48, 96, 144, 192, 240,
					30, 88, 146, 204, 262,
					88, 146, 204, 262,
					88, 146, 204, 262};

	int murci_y[] = {179, 179, 179, 179, 179, 179, 179, 179,
					158, 158, 158, 158, 158, 158, 158, 158,
					137, 137, 137, 137, 137, 137,
					115, 115, 115, 115, 115, 115,
					78, 78, 78, 78, 78,
					41, 41, 41, 41,
					4, 4, 4, 4};

	if (frame_murcielago == 41)
		frame_murcielago = 0;

	pos_murci[0] = murci_x[frame_murcielago];
	pos_murci[1] = murci_y[frame_murcielago];

	if (frame_murcielago < 22) {
		pos_murci[4] = 37;
		pos_murci[5] = 21;
	} else if (frame_murcielago > 27) {
		pos_murci[4] = 57;
		pos_murci[5] = 36;
	} else {
		pos_murci[4] = 47;
		pos_murci[5] = 22;
	}

	pos_murci[2] = 239;
	pos_murci[3] = 19;

	copyRectClip(pos_murci, dir_dibujo3, dir_zona_pantalla);
	diferencia = vez() - conta_ciego_vez;
	if (diferencia >= 6) {
		frame_murcielago++;
		conta_ciego_vez = vez();
	}

	copyRect(29, 37, 58, 114, 57, 39, dir_dibujo3, dir_zona_pantalla);
	mapa();
}

void DrasculaEngine::update_3_pre() {
	if (flags[3] == 1)
		copyBackground(258, 110, 85, 44, 23, 53, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_3() {
	if (hare_y + alto_hare < 118)
		copyRect(129, 110, 194, 36, 126, 88, dir_dibujo3, dir_zona_pantalla);
	copyRect(47, 57, 277, 143, 43, 50, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_4() {
	int cambio_col_antes = cambio_de_color;
	if (hare_x > 190)
		cambio_de_color = 1;
	else
		cambio_de_color = 0;

	if (cambio_col_antes != cambio_de_color && cambio_de_color == 1)
		hare_oscuro();
	if (cambio_col_antes != cambio_de_color && cambio_de_color == 0)
		hare_claro();
}

void DrasculaEngine::update_5_pre(){
	if (flags[8] == 0)
		copyBackground(256, 152, 208, 67, 27, 40, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_5() {
	copyRect(114, 130, 211, 87, 109, 69, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_6_pre() {
	int cambio_col_antes = cambio_de_color;

	if ((hare_x > 149 && hare_y + alto_hare > 160 && hare_x < 220 && hare_y + alto_hare < 188) ||
			(hare_x > 75 && hare_y + alto_hare > 183 && hare_x < 145))
		cambio_de_color = 0;
	else
		cambio_de_color = 1;

	if (cambio_col_antes != cambio_de_color && cambio_de_color == 1)
		hare_oscuro();
	if (cambio_col_antes != cambio_de_color && cambio_de_color == 0)
		hare_claro();

	if (flags[1] == 0)
		copyBackground(97, 117, 34, 148, 36, 31, dir_dibujo3, dir_zona_pantalla);
	if (flags[0] == 0)
		copyBackground(3, 103, 185, 69, 23, 76, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0)
		copyBackground(28, 100, 219, 72, 64, 97, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_7_pre() {
	if (flags[35] == 0)
		copyBackground(1, 72, 158, 162, 19, 12, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_9_pre() {
	int ciego_x[] = {26, 68, 110, 152, 194, 236, 278, 26, 68};
	int ciego_y[] = {51, 51, 51, 51, 51, 51, 51, 127, 127};
	int diferencia;

	copyRect(ciego_x[frame_ciego], ciego_y[frame_ciego], 122, 57, 41, 72, dir_dibujo3, dir_zona_pantalla);
	if (flags[9] == 0) {
		diferencia = vez() - conta_ciego_vez;
		if (diferencia >= 11) {
			frame_ciego++;
			conta_ciego_vez = vez();
		}
		if (frame_ciego == 9)
			frame_ciego = 0;
	} else
		frame_ciego = 3;
}

void DrasculaEngine::update_12_pre() {
	if (flags[16] == 0)
		copyBackground(1, 131, 106, 117, 55, 68, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_13() {
	if (hare_x > 55 && flags[3] == 0)
		animation_6_3();
	if (flags[1] == 0)
		copyRect(185, 110, 121, 65, 67, 88, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0)
		copyRect(185, 21, 121, 63, 67, 88, dir_dibujo3, dir_zona_pantalla);
	copyRect(3, 127, 99, 102, 181, 71, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_14_pre() {
	int velas_y[] = {158, 172, 186};
	int cirio_x[] = {14, 19, 24};
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31};
	int borracho_x[] = {1, 42, 83, 124, 165, 206, 247, 1};
	int diferencia;

	copyBackground(123, velas_y[frame_velas], 142, 14, 39, 13, dir_dibujo3, dir_zona_pantalla);
	copyBackground(cirio_x[frame_velas], 146, 311, 80, 4, 8, dir_dibujo3, dir_zona_pantalla);

	if (parpadeo == 5)
		copyBackground(1, 149, 127, 52, 9, 5, dir_dibujo3, dir_zona_pantalla);
	if (hare_x > 101 && hare_x < 155)
		copyBackground(31, 138, 178, 51, 18, 16, dir_dibujo3, dir_zona_pantalla);
	if (flags[11] == 0)
		copyBackground(pianista_x[frame_piano], 157, 245, 130, 29, 42, dir_dibujo3, dir_zona_pantalla);
	else if (flags[5] == 0)
		copyBackground(145, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	else
		copyBackground(165, 140, 229, 117, 43, 59, dir_dibujo3, dir_zona_pantalla);

	if (flags[12] == 1)
		copyBackground(borracho_x[frame_borracho], 82, 170, 50, 40, 53, dir_dibujo3, dir_zona_pantalla);
	diferencia = vez() - conta_ciego_vez;
	if (diferencia > 6) {
		if (flags[12] == 1) {
			frame_borracho++;
			if (frame_borracho == 8) {
				frame_borracho = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		parpadeo = _rnd->getRandomNumber(10);
		conta_ciego_vez = vez();
	}
}

void DrasculaEngine::update_15() {
	copyRect(1, 154, 83, 122, 131, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_16_pre() {
	if (flags[17] == 0)
		copyBackground(1, 103, 24, 72, 33, 95, dir_dibujo3, dir_zona_pantalla);
	if (flags[19] == 1)
		copyBackground(37, 151, 224, 115, 56, 47, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_17_pre() {
	if (flags[15] == 1)
		copyBackground(1, 135, 108, 65, 44, 63, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_17() {
	copyRect(48, 135, 78, 139, 80, 30, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_18_pre() {
	int diferencia;
	int ronquido_x[] = {95, 136, 95, 136, 95, 95, 95, 95, 136, 95, 95, 95, 95, 95, 95, 95};
	int ronquido_y[] = {18, 18, 56, 56, 94, 94, 94, 94, 94, 18, 18, 18, 18, 18, 18, 18};

	if (flags[21] == 0) {
		copyBackground(1, 69, 120, 58, 56, 61, dir_dibujo3, dir_zona_pantalla);
		copyBackground(ronquido_x[frame_ronquido], ronquido_y[frame_ronquido], 124, 59, 40, 37, dir_dibujo3, dir_zona_pantalla);
	} else
		pon_vb();

	diferencia = vez() - conta_ciego_vez;
	if (diferencia > 9) {
		frame_ronquido++;
		if (frame_ronquido == 16)
			frame_ronquido = 0;
		conta_ciego_vez = vez();
	}
}

void DrasculaEngine::update_18() {
	if (flags[24] == 1)
		copyRect(177, 1, 69, 29, 142, 130, dir_dibujo3, dir_zona_pantalla);
	copyRect(105, 132, 109, 108, 196, 65, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_20() {
	copyRect(1, 137, 106, 121, 213, 61, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_21_pre() {
	if (flags[0] == 1)
		copyBackground(2, 171, 84, 126, 17, 26, dir_dibujo3, dir_zona_pantalla);

	if (flags[10] == 1)
		copyBackground(20, 163, 257, 149, 14, 34, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_22_pre() {
	if (flags[24] == 1)
		copyBackground(2, 187, 107, 106, 62, 12, dir_dibujo3, dir_zona_pantalla);

	if (flags[27] == 0)
		copyBackground(32, 181, 203, 88, 13, 5, dir_dibujo3, dir_zona_pantalla);

	if (flags[26] == 0)
		copyBackground(2, 133, 137, 83, 29, 53, dir_dibujo3, dir_zona_pantalla);
	else
		copyBackground(65, 174, 109, 145, 55, 25, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_23_pre() {
	if (flags[11] == 1 && flags[0] == 0)
		copyBackground(87, 171, 237, 110, 20, 28, dir_dibujo3, dir_zona_pantalla);

	if (flags[0] == 1)
		copyBackground(29, 126, 239, 94, 57, 73, dir_dibujo3, dir_zona_pantalla);

	if (flags[1] == 1)
		copyRect(1, 135, 7, 94, 27, 64, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_24_pre() {
	if (flags[1] == 1)
		copyBackground(1, 163, 225, 124, 12, 36, dir_dibujo3, dir_zona_pantalla);

	if (flags[2] == 1)
		copyBackground(14, 153, 30, 107, 23, 46, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_26_pre() {
	int diferencia;

	if (flags[2] == 1)
		copyBackground(1, 130, 87, 44, 50, 69, dir_dibujo3, dir_zona_pantalla);

	if (flags[12] == 1)
		copyBackground(52, 177, 272, 103, 27, 22, dir_dibujo3, dir_zona_pantalla);

	if (flags[18] == 0)
		copyBackground(80, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);

	if (parpadeo == 5 && flags[18] == 0)
		copyBackground(52, 172, 226, 106, 3, 4, dir_dibujo3, dir_zona_pantalla);

	diferencia = vez() - conta_ciego_vez;
	if (diferencia >= 10) {
		parpadeo = _rnd->getRandomNumber(10);
		conta_ciego_vez = vez();
	}

	if (flags[20] == 1)
		copyBackground(182, 133, 199, 95, 50, 66, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_26() {
	if (flags[29] == 1)
		copyRect(93, 1, hare_x, hare_y, 45, 78, dir_hare_fondo, dir_zona_pantalla);

	copyRect(233, 107, 17, 102, 66, 92, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_27_pre() {
	if (flags[5] == 1)
		copyRect(1, 175, 59, 109, 17, 24, dir_dibujo3, dir_zona_pantalla);

	if (flags[6] == 1)
		copyRect(19, 177, 161, 103, 18, 22, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_27() {
	copyRect(38, 177, 103, 171, 21, 22, dir_dibujo3, dir_zona_pantalla);
	copyRect(60, 162, 228, 156, 18, 37, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_29_pre() {
	if (flags[4] == 1)
		copyBackground(12, 113, 247, 49, 41, 84, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_29() {
	copyRect(1, 180, 150, 126, 10, 17, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_30_pre() {
	if (flags[4] == 1)
		copyBackground(1, 148, 148, 66, 35, 51, dir_dibujo3, dir_zona_pantalla);

	if (flags[16] == 1)
		copyBackground(37, 173, 109, 84, 20, 26, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_31_pre() {
	if (flags[13] == 1)
		copyBackground(1, 163, 116, 41, 61, 36, dir_dibujo3, dir_zona_pantalla);

	if (flags[5] == 1)
		copyBackground(1, 78, 245, 63, 30, 84, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_31() {
	copyRect(63, 190, 223, 157, 17, 9, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_34_pre() {
	if (flags[7] == 1)
		copyBackground(99, 127, 73, 41, 79, 72, dir_dibujo3, dir_zona_pantalla);

	if (flags[8] == 1)
		copyBackground(36, 129, 153, 41, 62, 65, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_34() {
	copyRect(5, 171, 234, 126, 29, 23, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_35_pre() {
	if (flags[14] == 1)
		copyBackground(1, 86, 246, 65, 68, 87, dir_dibujo3, dir_zona_pantalla);

	if (flags[17] == 0 && flags[15] == 1)
		copyBackground(111, 150, 118, 52, 40, 23, dir_dibujo3, dir_zona_pantalla);

	if (flags[17] == 1)
		copyBackground(70, 150, 118, 52, 40, 23, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_35() {
	copyRect(1, 174, 54, 152, 195, 25, dir_dibujo3, dir_zona_pantalla);
}


void DrasculaEngine::update_49_pre() {
	if (flags[6] == 0)
		copyBackground(2, 136, 176, 81, 49, 62, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_50() {
	copyRect(4, 153, 118, 95, 67, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_53_pre() {
	if (flags[1] == 0)
		copyRect(2, 113, 205, 50, 38, 86, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0)
		copyBackground(41, 159, 27, 117, 25, 40, dir_dibujo3, dir_zona_pantalla);
	if (flags[9] == 1)
		copyBackground(67, 184, 56, 93, 32, 15, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_54_pre() {
	if (flags[5] == 1)
		copyBackground(168, 156, 187, 111, 7, 11, dir_dibujo3, dir_zona_pantalla);
	if (flags[12] == 1)
		copyBackground(16, 156, 190, 64, 18, 24, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_56_pre() {
	if (flags[10] == 0)
		copyBackground(2, 126, 42, 67, 57, 67, dir_dibujo3, dir_zona_pantalla);
	if (flags[11] == 1)
		copyBackground(60, 160, 128, 97, 103, 38, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_57() {
	copyRect(7, 113, 166, 61, 62, 82, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_58_pre() {
	if (flags[0] == 0)
		copyBackground(1, 156, 143, 120, 120, 43, dir_dibujo3, dir_zona_pantalla);
	if (flags[1] == 2)
		copyRect(252, 171, 173, 116, 25, 28, dir_dibujo3, dir_zona_pantalla);
	if (flags[1] == 0 && flags[0] == 0)
		copyRect(278, 171, 173, 116, 25, 28, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0) {
		pon_igor();
		pon_dr();
	}
	if (flags[3] == 1)
		copyRect(1, 29, 204, 0, 18, 125, dir_dibujo3, dir_zona_pantalla);
	if (flags[8] == 1)
		copyBackground(20, 60, 30, 64, 46, 95, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_58() {
	if (hare_se_ve == 1)
		copyRect(67, 139, 140, 147, 12, 16, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_59_pre() {
	if (flags[4] == 0)
		copyRect(1, 146, 65, 106, 83, 40, dir_dibujo3, dir_zona_pantalla);
	if (flags[9] == 1) {
		copyBackground(65, 103, 65, 103, 49, 38, dir_dibujo1, dir_zona_pantalla);
		copyRect(1, 105, 65, 103, 49, 38, dir_dibujo3, dir_zona_pantalla);
	}
}

void DrasculaEngine::update_60_pre() {
	int velas_y[] = {158, 172, 186};
	int diferencia;

	if (flags[5] == 0)
		pon_dr();

	copyBackground(123, velas_y[frame_velas], 142, 14, 39, 13, dir_dibujo3, dir_zona_pantalla);

	if (flag_tv == 1)
		copyBackground(114, 158, 8, 30, 8, 23, dir_dibujo3, dir_zona_pantalla);

	diferencia = vez() - conta_ciego_vez;
	parpadeo = _rnd->getRandomNumber(7);
	if (parpadeo == 5 && flag_tv == 0)
		flag_tv = 1;
	else if (parpadeo == 5 && flag_tv == 1)
		flag_tv = 0;
	if (diferencia > 6) {
		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		conta_ciego_vez = vez();
	}
}

void DrasculaEngine::update_60() {
	if (hare_y - 10 < y_dr && flags[5] == 0)
		pon_dr();
}

void DrasculaEngine::update_61() {
	copyRect(1, 154, 83, 122, 131, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_62_pre() {
	int velas_y[] = { 158, 172, 186 };
	int cirio_x[] = { 14, 19, 24 };
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31 };
	int borracho_x[] = {1, 42, 83, 124, 165, 206, 247, 1 };
	int diferencia;

	copyBackground(123, velas_y[frame_velas], 142, 14, 39, 13, dir_dibujo3, dir_zona_pantalla);
	copyBackground(cirio_x[frame_velas], 146, 311, 80, 4, 8, dir_dibujo3, dir_zona_pantalla);

	if (parpadeo == 5)
		copyBackground(1, 149, 127, 52, 9, 5, dir_dibujo3, dir_zona_pantalla);

	if (hare_x > 101 && hare_x < 155)
		copyBackground(31, 138, 178, 51, 18, 16, dir_dibujo3, dir_zona_pantalla);

	if (flags[11] == 0)
		copyBackground(pianista_x[frame_piano], 157, 245, 130, 29, 42, dir_dibujo3, dir_zona_pantalla);
	else if (flags[5] == 0)
		copyBackground(145, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	else
		copyBackground(165, 140, 229, 117, 43, 59, dir_dibujo3, dir_zona_pantalla);

	if (flags[12] == 1)
		copyBackground(borracho_x[frame_borracho], 82, 170, 50, 40, 53, dir_dibujo3, dir_zona_pantalla);

	diferencia = vez() - conta_ciego_vez;
	if (diferencia > 6) {
		if (flags[12] == 1) {
			frame_borracho++;
			if (frame_borracho == 8) {
				frame_borracho = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		parpadeo = _rnd->getRandomNumber(10);
		conta_ciego_vez = vez();
	}
}

void DrasculaEngine::update_62() {
	int borracho_x[] = { 1, 42, 83, 124, 165, 206, 247, 1 };

	copyRect(1, 1, 0, 0, 62, 142, dir_dibujo2, dir_zona_pantalla);

	if (hare_y + alto_hare < 89) {
		copyRect(205, 1, 180, 9, 82, 80, dir_dibujo3, dir_zona_pantalla);
		copyBackground(borracho_x[frame_borracho], 82, 170, 50, 40, 53, dir_dibujo3, dir_zona_pantalla);
	}
}

void DrasculaEngine::update_63() {
	copyRect(1, 154, 83, 122, 131, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::update_pendulo() {
	int pendulo_x[] = {40, 96, 152, 208, 264, 40, 96, 152, 208, 208, 152, 264, 40, 96, 152, 208, 264};
	int diferencia;

	if (frame_pen <= 4)
		dir_pendulo = dir_dibujo3;
	else if (frame_pen <= 11)
		dir_pendulo = dir_hare_dch;
	else
		dir_pendulo = dir_hare_frente;

	copyBackground(pendulo_x[frame_pen], 19, 152, 0, 55, 125, dir_pendulo, dir_zona_pantalla);

	if (flags[1] == 2)
		copyRect(18, 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);

	if (flags[1] == 0)
		copyRect(44, 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);

	diferencia = vez() - conta_ciego_vez;
	if (diferencia > 8) {
		frame_pen++;
		if (frame_pen == 17)
			frame_pen = 0;
		conta_ciego_vez = vez();
	}
}

} // End of namespace Drascula
