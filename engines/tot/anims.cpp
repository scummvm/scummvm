/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"

#include "tot/anims.h"
#include "tot/decoder/TotFlicDecoder.h"
#include "tot/graphics.h"
#include "tot/playanim.h"
#include "tot/texts.h"
#include "tot/tot.h"

namespace Tot {

const int sizefrase = 320 * 30 + 4;

int32 posflicfile;
uint numerovuelta, posrelfli;
byte *punterofondofrase = (byte *)malloc(sizefrase);
byte framecontador;
bool primeravuelta;

void drawText(uint xfrase, uint yfrase, Common::String str1, Common::String str2, Common::String str3, Common::String str4, Common::String str5, byte colorfrase, byte colorborde) {

	outtextxy(xfrase, (yfrase + 3), str1, colorborde);
	outtextxy(xfrase, (yfrase + 13), str2, colorborde);
	outtextxy(xfrase, (yfrase + 23), str3, colorborde);
	outtextxy(xfrase, (yfrase + 33), str4, colorborde);
	outtextxy(xfrase, (yfrase + 43), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase, (yfrase + 1), str1, colorborde);
	outtextxy(xfrase, (yfrase + 11), str2, colorborde);
	outtextxy(xfrase, (yfrase + 21), str3, colorborde);
	outtextxy(xfrase, (yfrase + 31), str4, colorborde);
	outtextxy(xfrase, (yfrase + 41), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase + 1, (yfrase + 2), str1, colorborde);
	outtextxy(xfrase + 1, (yfrase + 12), str2, colorborde);
	outtextxy(xfrase + 1, (yfrase + 22), str3, colorborde);
	outtextxy(xfrase + 1, (yfrase + 32), str4, colorborde);
	outtextxy(xfrase + 1, (yfrase + 42), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase - 1, (yfrase + 2), str1, colorborde);
	outtextxy(xfrase - 1, (yfrase + 12), str2, colorborde);
	outtextxy(xfrase - 1, (yfrase + 22), str3, colorborde);
	outtextxy(xfrase - 1, (yfrase + 32), str4, colorborde);
	outtextxy(xfrase - 1, (yfrase + 42), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	outtextxy(xfrase, (yfrase + 2), str1, colorfrase);
	outtextxy(xfrase, (yfrase + 12), str2, colorfrase);
	outtextxy(xfrase, (yfrase + 22), str3, colorfrase);
	outtextxy(xfrase, (yfrase + 32), str4, colorfrase);
	outtextxy(xfrase, (yfrase + 42), str5, colorfrase);
	g_engine->_screen->update();
}

void removeText(uint xfrase1, uint yfrase1, uint xfrase2, uint yfrase2, byte colorrelleno) {

	for (int j = yfrase1; j < yfrase2 + 1; j++) {
		for (int i = xfrase1; i < xfrase2 + 1; i++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = 0;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(xfrase1, yfrase1, xfrase2, yfrase2));
}

void handleFlcEvent(byte numero_del_evento) {
	switch (numero_del_evento) {
	case 0:
		if (contadorpc > 103)
			showError(274);
		break;
	case 1:
		if (framecontador == 3)

			switch (numerovuelta) {
			case 2:
				drawText(80, 0,
					animMessages_ES[0],
					animMessages_ES[1],
					animMessages_ES[2],
					animMessages_ES[3],
					animMessages_ES[4],
					253, 0);
				break;
			case 13:
				removeText(80, 0, 319, 53, 0);
				break;
			case 14:
				drawText(80, 0,
					animMessages_ES[5],
					animMessages_ES[6],
					animMessages_ES[7],
					animMessages_ES[8],
					animMessages_ES[9],
					253, 0);
				break;
			case 25:
				removeText(80, 0, 319, 53, 0);
				break;
			case 26:
				drawText(80, 0,
					animMessages_ES[10],
					animMessages_ES[11],
					animMessages_ES[12],
					animMessages_ES[13],
					animMessages_ES[14],
					253, 0);
				break;
			case 35:
				removeText(80, 0, 319, 53, 0);
				break;
			case 36:
				drawText(2, 100,
					animMessages_ES[15],
					animMessages_ES[16],
					animMessages_ES[17],
					animMessages_ES[18],
					animMessages_ES[19],
					255, 0);
				break;
			case 47:
				removeText(2, 100, 134, 199, 0);
				break;
			case 48:
				drawText(2, 100,
					animMessages_ES[20],
					animMessages_ES[21],
					animMessages_ES[22],
					animMessages_ES[23],
					animMessages_ES[24],
					255, 0);
				break;
			case 59:
				removeText(2, 100, 134, 199, 0);
				break;
			case 60:
				drawText(80, 0,
					animMessages_ES[25],
					animMessages_ES[26],
					animMessages_ES[27],
					animMessages_ES[28],
					animMessages_ES[29],
					253, 0);
				break;
			case 63:
				drawText(2, 100,
					animMessages_ES[30],
					animMessages_ES[31],
					animMessages_ES[32],
					animMessages_ES[33],
					animMessages_ES[34],
					255, 0);
				break;
			case 75:
				removeText(2, 100, 135, 199, 0);
				break;
			case 76:
				removeText(80, 0, 319, 53, 0);
				break;
			case 77:
				drawText(2, 100,
					animMessages_ES[35],
					animMessages_ES[36],
					animMessages_ES[37],
					animMessages_ES[38],
					animMessages_ES[39],
					255, 0);
				break;
			case 89:
				removeText(2, 100, 135, 199, 0);
				break;
			case 90:
				drawText(2, 100,
					animMessages_ES[40],
					animMessages_ES[41],
					animMessages_ES[42],
					animMessages_ES[43],
					animMessages_ES[44],
					 255, 0);
				break;
			case 102:
				removeText(2, 100, 135, 199, 0);
				break;
			case 103:
				drawText(80, 0,
					animMessages_ES[45],
					animMessages_ES[46],
					animMessages_ES[47],
					animMessages_ES[48],
					animMessages_ES[49],
					253, 0);
				break;
			case 120:
				removeText(80, 0, 319, 53, 0);
				break;
			case 121:
				drawText(80, 0,
					animMessages_ES[50],
					animMessages_ES[51],
					animMessages_ES[52],
					animMessages_ES[53],
					animMessages_ES[54],
					253, 0);
				break;
			case 125:
				drawText(2, 100,
					animMessages_ES[55],
					animMessages_ES[56],
					animMessages_ES[57],
					animMessages_ES[58],
					animMessages_ES[59],
					255, 0);
				break;
			case 135: {
				removeText(80, 0, 319, 53, 0);
				removeText(2, 100, 135, 199, 0);
			} break;
			}
		break;
	case 2:
		switch (framecontador) {
		case 1: {
			removeText(2, 100, 135, 199, 0);
			removeText(80, 0, 319, 53, 0);
		} break;
		case 7:
			playVoc("MANDO", 142001, 11469);
			break;
		case 20:
			drawText(2, 100,
				animMessages_ES[60],
				animMessages_ES[61],
				animMessages_ES[62],
				animMessages_ES[63],
				animMessages_ES[64],
				255, 0);
			break;
		case 58: {
			removeText(2, 100, 135, 199, 0);
			drawText(2, 100,
				animMessages_ES[65],
				animMessages_ES[66],
				animMessages_ES[67],
				animMessages_ES[68],
				animMessages_ES[69],
				255, 0);
		} break;
		case 74: {
			delay(1500);
			removeText(2, 100, 135, 199, 0);
			drawText(2, 100,
				animMessages_ES[70],
				animMessages_ES[71],
				animMessages_ES[72],
				animMessages_ES[73],
				animMessages_ES[74],
				255, 0);
			delay(4000);
			removeText(2, 100, 135, 199, 0);
			drawText(80, 0,
				animMessages_ES[75],
				animMessages_ES[76],
				animMessages_ES[77],
				animMessages_ES[78],
				animMessages_ES[79],
				253, 0);
			delay(7000);
			removeText(80, 0, 319, 53, 0);
			drawText(80, 0,
				animMessages_ES[80],
				animMessages_ES[81],
				animMessages_ES[82],
				animMessages_ES[83],
				animMessages_ES[84],
				253, 0);
			delay(7000);
			removeText(80, 0, 319, 53, 0);
			drawText(80, 0,
				animMessages_ES[85],
				animMessages_ES[86],
				animMessages_ES[87],
				animMessages_ES[88],
				animMessages_ES[89],
				253, 0);
			delay(7000);
			removeText(80, 0, 319, 53, 0);
			drawText(2, 100,
				animMessages_ES[90],
				animMessages_ES[91],
				animMessages_ES[92],
				animMessages_ES[93],
				animMessages_ES[94],
				255, 0);
			delay(6000);
			removeText(2, 100, 135, 199, 0);
			drawText(2, 100,
				animMessages_ES[95],
				animMessages_ES[96],
				animMessages_ES[97],
				animMessages_ES[98],
				animMessages_ES[99],
				255, 0);
			delay(6000);
		} break;
		}
		break;
	case 3:
		switch (framecontador) {
		case 15:
			playVoc("FRENAZO", 165322, 15073);
			break;
		case 26:
			delay(1000);
			break;
		case 43:
			playVoc("PORTAZO", 434988, 932);
			break;
		case 60:
			getImg(0, 0, 319, 29, punterofondofrase);
			break;
		}
		break;
	case 4:
		if (framecontador == 3)
			playVoc("TIMBRAZO", 423775, 11213);
		break;
	case 5:
		if ((numerovuelta == 1) && (framecontador == 2)) {

			delay(2000);
			drawText(5, 1,
				animMessages_ES[100],
				animMessages_ES[101],
				animMessages_ES[102],
				animMessages_ES[103],
				animMessages_ES[104],
				255, 249);
			delay(3500);
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[105],
				animMessages_ES[106],
				animMessages_ES[107],
				animMessages_ES[108],
				animMessages_ES[109],
				255, 0);
		}
		break;
	case 6:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[110],
				animMessages_ES[111],
				animMessages_ES[112],
				animMessages_ES[113],
				animMessages_ES[114],
				255, 249);
		}
		break;
	case 7:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[115],
				animMessages_ES[116],
				animMessages_ES[117],
				animMessages_ES[118],
				animMessages_ES[119],
				255, 0);
		}
		break;
	case 8:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[120],
				animMessages_ES[121],
				animMessages_ES[122],
				animMessages_ES[123],
				animMessages_ES[124],
				255, 249);
		}
		break;
	case 9:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[125],
				animMessages_ES[126],
				animMessages_ES[127],
				animMessages_ES[128],
				animMessages_ES[129],
				255, 0);
		}
		break;
	case 10:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[130],
				animMessages_ES[131],
				animMessages_ES[132],
				animMessages_ES[133],
				animMessages_ES[134],
				255, 249);
		}
		break;
	case 11:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[135],
				animMessages_ES[136],
				animMessages_ES[137],
				animMessages_ES[138],
				animMessages_ES[139],
				 255, 0);
		}
		break;
	case 12:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[140],
				animMessages_ES[141],
				animMessages_ES[142],
				animMessages_ES[143],
				animMessages_ES[144],
				255, 249);
		}
		break;
	case 13:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[145],
				animMessages_ES[146],
				animMessages_ES[147],
				animMessages_ES[148],
				animMessages_ES[149],
				255, 0);
		}
		break;
	case 14:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[150],
				animMessages_ES[151],
				animMessages_ES[152],
				animMessages_ES[153],
				animMessages_ES[154],
				255, 249);
		}
		break;
	case 15:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[155],
				animMessages_ES[156],
				animMessages_ES[157],
				animMessages_ES[158],
				animMessages_ES[159],
				255, 0);
		}
		break;
	case 16:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[160],
				animMessages_ES[161],
				animMessages_ES[162],
				animMessages_ES[163],
				animMessages_ES[164],
				255, 249);
		}
		break;
	case 17:
		switch (framecontador) {
		case 1:
			putImg(0, 0, punterofondofrase);
			break;
		case 17:
			delay(500);
			break;
		case 18:
			playVoc("ACELERON", 30200, 42398);
			break;
		}
		break;
	case 18:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				animMessages_ES[165],
				animMessages_ES[166],
				animMessages_ES[167],
				animMessages_ES[168],
				animMessages_ES[169],
				255, 0);
		}
		break;
	case 19:
		if (framecontador == 1)
			drawText(5, 121,
				animMessages_ES[170],
				animMessages_ES[171],
				animMessages_ES[172],
				animMessages_ES[173],
				animMessages_ES[174],
			 	253, 249);
		break;
	case 20:
		switch (numerovuelta) {
		case 1:
			switch (framecontador) {
			case 1:
				getImg(0, 0, 319, 29, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[175],
					animMessages_ES[176],
					animMessages_ES[177],
					animMessages_ES[178],
					animMessages_ES[179],
					255, 0);
				break;
			}
			break;
		case 3:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[180],
					animMessages_ES[181],
					animMessages_ES[182],
					animMessages_ES[183],
					animMessages_ES[184],
					230, 249);
				break;
			}
			break;
		case 6:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[185],
					animMessages_ES[186],
					animMessages_ES[187],
					animMessages_ES[188],
					animMessages_ES[189],
					230, 249);
				break;
			}
			break;
		case 9:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[190],
					animMessages_ES[191],
					animMessages_ES[192],
					animMessages_ES[193],
					animMessages_ES[194],
					230, 249);
				break;
			}
			break;
		case 12:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[195],
					animMessages_ES[196],
					animMessages_ES[197],
					animMessages_ES[198],
					animMessages_ES[199],
					230, 249);
				break;
			}
			break;
		case 15:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[200],
					animMessages_ES[201],
					animMessages_ES[202],
					animMessages_ES[203],
					animMessages_ES[204],
					230, 249);
				break;
			}
			break;
		case 18:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[205],
					animMessages_ES[206],
					animMessages_ES[207],
					animMessages_ES[208],
					animMessages_ES[209],
					230, 249);
				break;
			}
			break;
		case 21:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[210],
					animMessages_ES[211],
					animMessages_ES[212],
					animMessages_ES[213],
					animMessages_ES[214],
					230, 249);
				break;
			}
			break;
		case 24:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[215],
					animMessages_ES[216],
					animMessages_ES[217],
					animMessages_ES[218],
					animMessages_ES[219],
					230, 249);
				break;
			}
			break;
		case 27:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[220],
					animMessages_ES[221],
					animMessages_ES[222],
					animMessages_ES[223],
					animMessages_ES[224],
					230, 249);
				break;
			}
			break;
		case 30:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					animMessages_ES[225],
					animMessages_ES[226],
					animMessages_ES[227],
					animMessages_ES[228],
					animMessages_ES[229],
					230, 249);
				break;
			}
			break;
		case 33:
			if (framecontador == 17)
				putImg(0, 0, punterofondofrase);
			break;
		}
		break;
	case 21:
		switch (framecontador) {
		case 1:
			playVoc("TRIDEN", 409405, 14370);
			break;
		case 5:
			playVoc("PUFF", 191183, 18001);
			break;
		}
		break;
	case 22:
		if (framecontador == 24)
			playVoc("PUFF", 191183, 18001);
		break;
	case 23:
		switch (framecontador) {
		case 8:
			playVoc("AFILAR", 0, 6433);
			break;
		case 18:
			pitavocmem();
			break;
		}
		break;
	case 24:
		if (framecontador == 8)
			playVoc("DECAPITA", 354269, 1509);
		break;
	case 25:
		if (framecontador == 97)
			playVoc("PUFF2", 209184, 14514);
		break;
	case 26:
		switch (numerovuelta) {
		case 1:
			switch (framecontador) {
			case 2:
				getImg(0, 0, 319, 29, punterofondofrase);
				break;
			case 3:
				drawText(65, 1,
					animMessages_ES[230],
					animMessages_ES[231],
					animMessages_ES[232],
					animMessages_ES[233],
					animMessages_ES[234],
					253, 249);
				break;
			}
			break;
		case 2:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1,
					animMessages_ES[235],
					animMessages_ES[236],
					animMessages_ES[237],
					animMessages_ES[238],
					animMessages_ES[239],
					253, 249);
				break;
			}
			break;
		case 5:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1,
					animMessages_ES[240],
					animMessages_ES[241],
					animMessages_ES[242],
					animMessages_ES[243],
					animMessages_ES[244],
					253, 249);
				break;
			}
			break;
		case 8:
			switch (framecontador) {
			case 1:
				putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1,
					animMessages_ES[245],
					animMessages_ES[246],
					animMessages_ES[247],
					animMessages_ES[248],
					animMessages_ES[249],
					253, 249);
				break;
			}
			break;
		}
		break;
	case 27:
		if (framecontador == 148)
			playVoc("DECAPITA", 354269, 1509);
		break;
	}
}

void drawFlc(uint flicx, uint flicy, int32 posicionfli, uint loop,
			 byte veloc, byte numevento, bool palcompleta, bool permitesalida,
			 bool doscientos, bool &salidaflis);

static void exitProcedure(bool &salir_bucle, bool &permitesalida) {

	salir_bucle = false;
	if (permitesalida) {
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN || (e.type == Common::EVENT_LBUTTONUP)) {
				salir_bucle = true;
				debug("Exiting from exitProcedure!");
			}
		}
	}
}

static fliheader readHeader(Common::File *file) {
	fliheader headerfile;

	headerfile.size = file->readSint32LE();
	headerfile.magic = file->readSint16LE();
	headerfile.frames = file->readSint16LE();
	headerfile.width = file->readSint16LE();
	headerfile.heigth = file->readSint16LE();
	headerfile.depth = file->readSint16LE();
	headerfile.flags = file->readSint16LE();
	headerfile.speed = file->readSint32LE();
	headerfile.reserved1 = file->readSint16LE();
	headerfile.created = file->readSint32LE();
	file->read(headerfile.creator, 4);
	headerfile.updated = file->readSint32LE();
	file->read(headerfile.updator, 4);
	headerfile.aspectx = file->readSint16LE();
	headerfile.aspecty = file->readSint16LE();
	// Note: probably shouldnt just harcode 2 here
	file->read(headerfile.reserved2, 19 * 2);
	headerfile.ofsframe1 = file->readSint32LE();
	headerfile.ofsframe2 = file->readSint32LE();
	// Note: probably shouldnt just harcode 2 here
	file->read(headerfile.reserved2, 20 * 2);
	return headerfile;
}

void blit(const Graphics::Surface *src, Common::Rect bounds) {
	int16 height = bounds.bottom - bounds.top;
	int16 width = bounds.right - bounds.left;
	Graphics::Surface dest = g_engine->_screen->getSubArea(bounds);

	for (int i = 0; i < height - 1; i++) {
		for (int j = 0; j < width - 1; j++) {
			*((byte *)dest.getBasePtr(j, i)) = *((byte *)src->getBasePtr(j, i));
		}
	}
	g_engine->_screen->addDirtyRect(bounds);
	g_engine->_screen->update();
}

static void loadFlc(
	uint &loop,
	bool &permitesalida,
	bool &salidaflis,
	byte &numevento,
	bool &palcompleta,
	bool &doscientos,
	byte &veloc,
	uint &flicx,
	uint &flicy) {

	framecontador = 0;
	numerovuelta = 0;

	Common::File animationsFile;
	Common::String fileName;
	if (loop == 60000)
		fileName = "OBJGIRO.DAT";
	else
		fileName = "FILMS.DAT";

	if (!animationsFile.open(Common::Path(fileName))) {
		showError(272);
	}
	animationsFile.seek(posflicfile, SEEK_SET);
	// Need to read header to get the total size of the FLIC file.
	fliheader header = readHeader(&animationsFile);
	Common::SeekableSubReadStream *thisFlic = new Common::SeekableSubReadStream(
		&animationsFile,
		posflicfile,
		posflicfile + header.size);

	TotFlicDecoder flic = TotFlicDecoder();

	flic.loadStream(thisFlic);
	flic.start();

	do {
		g_engine->_chrono->updateChrono();
		numerovuelta++;
		do {
			g_engine->_chrono->updateChrono();
			exitProcedure(salidaflis, permitesalida);
			if (salidaflis) {
				goto Lsalir_proc;
			}
			if (tocapintar) {
				framecontador++;
				handleFlcEvent(numevento);
				const Graphics::Surface *frame = flic.decodeNextFrame();
				if (frame) {
					Common::Rect boundingBox = Common::Rect(flicx, flicy, flicx + flic.getWidth() + 1, flicy + flic.getHeight() + 1);
					blit(frame, boundingBox);
					if (flic.hasDirtyPalette()) {
						byte *palette = (byte *)flic.getPalette();
						// game fixes background to 0
						palette[0] = 0;
						palette[1] = 0;
						palette[2] = 0;
						if (palcompleta) {
							changePalette(g_engine->_graphics->getPalette(), palette);
							copyPalette(palette, pal);
						} else if (doscientos) {
							g_engine->_graphics->setPalette(palette, 200);
							for (int i = 0; i <= 200; i++) {
								if(gamePart == 2 && !salirdeljuego && (i == 131 || i == 134 || i == 143 || i == 187)) {
									continue;
								}
								pal[i * 3 + 0] = palette[i * 3 + 0];
								pal[i * 3 + 1] = palette[i * 3 + 1];
								pal[i * 3 + 2] = palette[i * 3 + 2];
							}

						} else {
							g_engine->_graphics->setPalette(palette);
							copyPalette(palette, pal);
						}
					}
					// Make sure we also update the palette animations! Esp. for part 2
					if (currentRoomData != NULL && (currentRoomData->paletteAnimationFlag) && (saltospal >= 4) && !salirdeljuego) {
						saltospal = 0;
						if (movidapaleta > 6)
							movidapaleta = 0;
						else movidapaleta += 1;
						updatePalette(movidapaleta);
					} else saltospal += 1;

					tocapintar = false;
				} else {
					break;
				}
			}
			g_system->delayMillis(10);
		} while (!flic.endOfVideo() && !g_engine->shouldQuit());

		if (flic.endOfVideo()) {
			if (flic.isRewindable()) {
				flic.rewind();
			}
			framecontador = 0;
			primeravuelta = false;
		}
		g_system->delayMillis(10);
	} while (numerovuelta <= loop && !g_engine->shouldQuit());
	flic.stop();
Lsalir_proc:
	animationsFile.close();
}

void drawFlc(
	uint flicx,
	uint flicy,
	int32 posicionfli,
	uint loop,
	byte veloc,
	byte numevento,
	bool palcompleta,
	bool permitesalida,
	bool doscientos,
	bool &salidaflis) {

	primeravuelta = true;
	posflicfile = posicionfli;
	posrelfli = flicx + flicy * 320;
	loadFlc(loop, permitesalida, salidaflis, numevento, palcompleta, doscientos, veloc, flicx, flicy);
	debug("Salida flis: %d", salidaflis);
}

void clearAnims() {
	free(punterofondofrase);
}
} // End of namespace Tot
