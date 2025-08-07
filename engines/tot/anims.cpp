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

const int sizefrase = 320 * 70 + 4;

int32 posflicfile;
uint numerovuelta, posrelfli;
byte *punterofondofrase = (byte *)malloc(sizefrase);
byte framecontador;
bool primeravuelta;

void drawText(uint xfrase, uint yfrase, Common::String str1, Common::String str2, Common::String str3, Common::String str4, Common::String str5, byte colorfrase, byte colorborde) {

	littText(xfrase, (yfrase + 3), str1, colorborde);
	littText(xfrase, (yfrase + 13), str2, colorborde);
	littText(xfrase, (yfrase + 23), str3, colorborde);
	littText(xfrase, (yfrase + 33), str4, colorborde);
	littText(xfrase, (yfrase + 43), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(xfrase, (yfrase + 1), str1, colorborde);
	littText(xfrase, (yfrase + 11), str2, colorborde);
	littText(xfrase, (yfrase + 21), str3, colorborde);
	littText(xfrase, (yfrase + 31), str4, colorborde);
	littText(xfrase, (yfrase + 41), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(xfrase + 1, (yfrase + 2), str1, colorborde);
	littText(xfrase + 1, (yfrase + 12), str2, colorborde);
	littText(xfrase + 1, (yfrase + 22), str3, colorborde);
	littText(xfrase + 1, (yfrase + 32), str4, colorborde);
	littText(xfrase + 1, (yfrase + 42), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(xfrase - 1, (yfrase + 2), str1, colorborde);
	littText(xfrase - 1, (yfrase + 12), str2, colorborde);
	littText(xfrase - 1, (yfrase + 22), str3, colorborde);
	littText(xfrase - 1, (yfrase + 32), str4, colorborde);
	littText(xfrase - 1, (yfrase + 42), str5, colorborde);
	g_engine->_screen->update();
	delay(enforcedTextAnimDelay);
	littText(xfrase, (yfrase + 2), str1, colorfrase);
	littText(xfrase, (yfrase + 12), str2, colorfrase);
	littText(xfrase, (yfrase + 22), str3, colorfrase);
	littText(xfrase, (yfrase + 32), str4, colorfrase);
	littText(xfrase, (yfrase + 42), str5, colorfrase);
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

void drawTvText(Common::String str1, Common::String str2, Common::String str3, Common::String str4, Common::String str5) {
	drawText(80, 0, str1, str2, str3, str4, str5, 253, 0);
}

void clearTvText() {
	removeText(80, 0, 319, 53, 0);
}

void drawCharacterText(Common::String str1, Common::String str2, Common::String str3, Common::String str4, Common::String str5) {
	drawText(2, 100,str1, str2, str3, str4, str5, 255, 0);
}
void clearCharacterText() {
	removeText(2, 100, 134, 199, 0);
}

void handleFlcEvent(byte numero_del_evento) {

	const char *const *messages = g_engine->_lang == Common::ES_ESP ? animMessages[0] : animMessages[1];
	bool isSpanish = g_engine->_lang == Common::ES_ESP;
	bool isEnglish = !isSpanish;
	switch (numero_del_evento) {
	case 0:
		if (contadorpc > 103)
			showError(274);
		break;
	case 1:
		if (framecontador == 3)

			switch (numerovuelta) {
			case 2:
				drawTvText(messages[0], messages[1], messages[2], messages[3], messages[4]);
				break;
			case 13:
				clearTvText();
				break;
			case 14:
				drawTvText(messages[5], messages[6], messages[7], messages[8], messages[9]);
				break;
			case 25:
				clearTvText();
				break;
			case 26:
				drawTvText(messages[10], messages[11], messages[12], messages[13], messages[14]);
				break;
			case 35:
				clearTvText();
				break;
			case 36:
				drawCharacterText(messages[15], messages[16], messages[17], messages[18], messages[19]);
				break;
			// English only
			case 40:
				if(isEnglish){
					clearCharacterText();
				}
				break;
			case 41:
				if(isEnglish) {
					drawCharacterText(messages[250], messages[251], messages[252], messages[253], messages[254]);
				}
				break;
			case 47:
				clearCharacterText();
				break;
			case 48:
				drawCharacterText(messages[20], messages[21], messages[22], messages[23], messages[24]);
				break;
			case 59:
				clearCharacterText();
				break;
			case 60:
				drawTvText(messages[25], messages[26], messages[27], messages[28], messages[29]);
				break;
			case 63:
				drawCharacterText(messages[30], messages[31], messages[32], messages[33], messages[34]);
				break;
			case 70:
				if(isEnglish) {
					clearTvText();
				}
				break;
			case 71:
				if(isEnglish) {
					drawTvText(messages[255], messages[256], messages[257], messages[258], messages[259]);
				}
				break;
			case 73:
				if(isEnglish) {
					clearCharacterText();
				}
			case 74:
				if(isEnglish) {
					drawCharacterText(
						messages[35],
						messages[36],
						messages[37],
						messages[38],
						messages[39]
					);
				}
			case 75:
				if(isSpanish) {
					clearCharacterText();
				}
				break;
			case 76:
				if(isSpanish) {
					clearTvText();
				}
				break;
			case 77:
				if(isSpanish) {
					drawCharacterText(messages[35], messages[36], messages[37], messages[38], messages[39]);
				}
				break;

			case 80:
				if(isEnglish) {
					clearTvText();
				}
				break;
			case 82:
				if(isEnglish) {
					clearCharacterText();
				}
				break;
			case 83:
				if(isEnglish) {
					drawCharacterText(messages[260], messages[261], messages[262], messages[263], messages[264]);
				}
				break;
			case 89:
				if(isSpanish) {
					clearCharacterText();
				}
				break;
			case 90:
				if(isSpanish) {
					drawCharacterText(messages[40], messages[41], messages[42], messages[43], messages[44]);
				}
				else {
					clearCharacterText();
				}
				break;
			case 91:
				if(isEnglish) {
					drawCharacterText(messages[40], messages[41], messages[42], messages[43], messages[44]);
				}
				break;

			case 102:
				clearCharacterText();
				break;
			case 103:
				drawTvText(messages[45], messages[46], messages[47], messages[48], messages[49]);
				break;
			case 120:
				clearTvText();
				break;
			case 121:
				drawTvText(messages[50], messages[51], messages[52], messages[53], messages[54]);
				break;
			case 125:
				drawCharacterText(messages[55], messages[56], messages[57], messages[58], messages[59]);
				break;
			case 135: {
				clearTvText();
				clearCharacterText();
			} break;
			}
		break;
	case 2:
		switch (framecontador) {
		case 1: {
			clearCharacterText();
			clearTvText();
		} break;
		case 7:
			g_engine->_sound->playVoc("MANDO", 142001, 11469);
			break;
		case 20:
			drawCharacterText(messages[60], messages[61], messages[62], messages[63], messages[64]);
			break;
		case 58: {
			clearCharacterText();
			drawCharacterText(messages[65], messages[66], messages[67], messages[68], messages[69]);
		} break;
		case 74: {
			delay(1500);
			clearCharacterText();
			drawCharacterText(messages[70], messages[71], messages[72], messages[73], messages[74]);

			delay(4000);
			clearCharacterText();
			drawTvText(messages[75], messages[76], messages[77], messages[78], messages[79]);

			delay(7000);
			clearTvText();
			drawTvText(messages[80], messages[81], messages[82], messages[83], messages[84]);

			if(isEnglish) {
				delay(5000);
				clearTvText();
				drawTvText(messages[265], messages[266], messages[267], messages[268], messages[269]);
			}

			delay(7000);
			clearTvText();
			drawTvText(messages[85], messages[86], messages[87], messages[88], messages[89]);

			delay(7000);
			clearTvText();
			drawCharacterText(messages[90], messages[91], messages[92], messages[93], messages[94]);

			delay(6000);
			clearCharacterText();
			drawCharacterText(messages[95], messages[96], messages[97], messages[98], messages[99]);

			if(isEnglish) {
				delay(6000);
				clearCharacterText();
				drawCharacterText(messages[95], messages[96], messages[97], messages[98], messages[99]);
			}

			delay(6000);
		} break;
		}
		break;
	case 3:
		switch (framecontador) {
		case 15:
			g_engine->_sound->playVoc("FRENAZO", 165322, 15073);
			break;
		case 26:
			delay(1000);
			break;
		case 43:
			g_engine->_sound->playVoc("PORTAZO", 434988, 932);
			break;
		case 60:
			g_engine->_graphics->getImg(0, 0, 319, 29, punterofondofrase);
			break;
		}
		break;
	case 4:
		if (framecontador == 3)
			g_engine->_sound->playVoc("TIMBRAZO", 423775, 11213);
		break;
	case 5:
		if ((numerovuelta == 1) && (framecontador == 2)) {

			delay(2000);
			drawText(5, 1,
				messages[100],
				messages[101],
				messages[102],
				messages[103],
				messages[104],
				255, 249);
			delay(3500);
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[105],
				messages[106],
				messages[107],
				messages[108],
				messages[109],
				255, 0);
		}
		break;
	case 6:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[110],
				messages[111],
				messages[112],
				messages[113],
				messages[114],
				255, 249);
		}
		else if ((numerovuelta == 5) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[275],
				messages[276],
				messages[277],
				messages[278],
				messages[279],
				255, 249);
		}
		break;
	case 7:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[115],
				messages[116],
				messages[117],
				messages[118],
				messages[119],
				255, 0);
		}
		break;
	case 8:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[120],
				messages[121],
				messages[122],
				messages[123],
				messages[124],
				255, 249);
		}
		else if ((numerovuelta == 5) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[280],
				messages[281],
				messages[282],
				messages[283],
				messages[284],
				255, 249);
		}
		break;
	case 9:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[125],
				messages[126],
				messages[127],
				messages[128],
				messages[129],
				255, 0);
		}
		break;
	case 10:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[130],
				messages[131],
				messages[132],
				messages[133],
				messages[134],
				255, 249);
		}
		break;
	case 11:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[135],
				messages[136],
				messages[137],
				messages[138],
				messages[139],
				 255, 0);
		}
		break;
	case 12:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[140],
				messages[141],
				messages[142],
				messages[143],
				messages[144],
				255, 249);
		}
		break;
	case 13:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[145],
				messages[146],
				messages[147],
				messages[148],
				messages[149],
				255, 0);
		}
		break;
	case 14:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[150],
				messages[151],
				messages[152],
				messages[153],
				messages[154],
				255, 249);
		}
		else if ((numerovuelta == 5) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[285],
				messages[286],
				messages[287],
				messages[288],
				messages[289],
				255, 249);
		}
		break;
	case 15:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[155],
				messages[156],
				messages[157],
				messages[158],
				messages[159],
				255, 0);
		}
		break;
	case 16:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[160],
				messages[161],
				messages[162],
				messages[163],
				messages[164],
				255, 249);
		}
		break;
	case 17:
		switch (framecontador) {
		case 1:
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			break;
		case 17:
			delay(500);
			break;
		case 18:
			g_engine->_sound->playVoc("ACELERON", 30200, 42398);
			break;
		}
		break;
	case 18:
		if ((numerovuelta == 1) && (framecontador == 3)) {
			g_engine->_graphics->putImg(0, 0, punterofondofrase);
			drawText(5, 1,
				messages[165],
				messages[166],
				messages[167],
				messages[168],
				messages[169],
				255, 0);
		}
		break;
	case 19:
		if (framecontador == 1)
			drawText(5, 121,
				messages[170],
				messages[171],
				messages[172],
				messages[173],
				messages[174],
			 	253, 249);
		break;
	case 20:
		switch (numerovuelta) {
		case 1:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->getImg(0, 0, 319, 69, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[175],
					messages[176],
					messages[177],
					messages[178],
					messages[179],
					255, 0);
				break;
			}
			break;
		case 3:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[180],
					messages[181],
					messages[182],
					messages[183],
					messages[184],
					230, 249);
				break;
			}
			break;
		case 6:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[185],
					messages[186],
					messages[187],
					messages[188],
					messages[189],
					230, 249);
				break;
			}
			break;
		case 9:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[190],
					messages[191],
					messages[192],
					messages[193],
					messages[194],
					230, 249);
				break;
			}
			break;
		case 12:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[195],
					messages[196],
					messages[197],
					messages[198],
					messages[199],
					230, 249);
				break;
			}
			break;
		case 15:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[200],
					messages[201],
					messages[202],
					messages[203],
					messages[204],
					230, 249);
				break;
			}
			break;
		case 18:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[205],
					messages[206],
					messages[207],
					messages[208],
					messages[209],
					230, 249);
				break;
			}
			break;
		case 21:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[210],
					messages[211],
					messages[212],
					messages[213],
					messages[214],
					230, 249);
				break;
			}
			break;
		case 24:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[215],
					messages[216],
					messages[217],
					messages[218],
					messages[219],
					230, 249);
				break;
			}
			break;
		case 27:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[220],
					messages[221],
					messages[222],
					messages[223],
					messages[224],
					230, 249);
				break;
			}
			break;
		case 30:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 3:
				drawText(15, 1,
					messages[225],
					messages[226],
					messages[227],
					messages[228],
					messages[229],
					230, 249);
				break;
			}
			break;
		case 33:
			if (framecontador == 17)
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
			break;
		}
		break;
	case 21:
		switch (framecontador) {
		case 1:
			g_engine->_sound->playVoc("TRIDEN", 409405, 14370);
			break;
		case 5:
			g_engine->_sound->playVoc("PUFF", 191183, 18001);
			break;
		}
		break;
	case 22:
		if (framecontador == 24)
			g_engine->_sound->playVoc("PUFF", 191183, 18001);
		break;
	case 23:
		switch (framecontador) {
		case 8:
			g_engine->_sound->playVoc("AFILAR", 0, 6433);
			break;
		case 18:
			g_engine->_sound->playVoc();
			break;
		}
		break;
	case 24:
		if (framecontador == 8)
			g_engine->_sound->playVoc("DECAPITA", 354269, 1509);
		break;
	case 25:
		if (framecontador == 97)
			g_engine->_sound->playVoc("PUFF2", 209184, 14514);
		break;
	case 26:
		switch (numerovuelta) {
		case 1:
			switch (framecontador) {
			case 2:
				g_engine->_graphics->getImg(0, 0, 319, 69, punterofondofrase);
				break;
			case 3:
				drawText(65, 1,
					messages[230],
					messages[231],
					messages[232],
					messages[233],
					messages[234],
					253, 249);
				break;
			}
			break;
		case 2:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1,
					messages[235],
					messages[236],
					messages[237],
					messages[238],
					messages[239],
					253, 249);
				break;
			}
			break;
		case 5:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1,
					messages[240],
					messages[241],
					messages[242],
					messages[243],
					messages[244],
					253, 249);
				break;
			}
			break;
		case 8:
			switch (framecontador) {
			case 1:
				g_engine->_graphics->putImg(0, 0, punterofondofrase);
				break;
			case 2:
				drawText(65, 1,
					messages[245],
					messages[246],
					messages[247],
					messages[248],
					messages[249],
					253, 249);
				break;
			}
			break;
		}
		break;
	case 27:
		if (framecontador == 148)
			g_engine->_sound->playVoc("DECAPITA", 354269, 1509);
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
		exitProcedure(salidaflis, permitesalida);
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
