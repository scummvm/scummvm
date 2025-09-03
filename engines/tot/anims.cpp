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
#include "common/substream.h"

#include "tot/anims.h"
#include "tot/decoder/TotFlicDecoder.h"
#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void drawText(uint x, uint y, const Common::String &str1, const Common::String &str2, const Common::String &str3, const Common::String &str4, const Common::String &str5, byte textColor, byte borderColor) {

	littText(x, (y + 3), str1,  borderColor);
	littText(x, (y + 13), str2, borderColor);
	littText(x, (y + 23), str3, borderColor);
	littText(x, (y + 33), str4, borderColor);
	littText(x, (y + 43), str5, borderColor);
	g_engine->_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(x, (y + 1), str1,  borderColor);
	littText(x, (y + 11), str2, borderColor);
	littText(x, (y + 21), str3, borderColor);
	littText(x, (y + 31), str4, borderColor);
	littText(x, (y + 41), str5, borderColor);
	g_engine->_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(x + 1, (y + 2), str1,  borderColor);
	littText(x + 1, (y + 12), str2, borderColor);
	littText(x + 1, (y + 22), str3, borderColor);
	littText(x + 1, (y + 32), str4, borderColor);
	littText(x + 1, (y + 42), str5, borderColor);
	g_engine->_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(x - 1, (y + 2), str1,  borderColor);
	littText(x - 1, (y + 12), str2, borderColor);
	littText(x - 1, (y + 22), str3, borderColor);
	littText(x - 1, (y + 32), str4, borderColor);
	littText(x - 1, (y + 42), str5, borderColor);
	g_engine->_screen->update();
	delay(kEnforcedTextAnimDelay);
	littText(x, (y + 2), str1,  textColor);
	littText(x, (y + 12), str2, textColor);
	littText(x, (y + 22), str3, textColor);
	littText(x, (y + 32), str4, textColor);
	littText(x, (y + 42), str5, textColor);
	g_engine->_screen->update();
}

void removeText(uint xTextLine1, uint yTextLine1, uint xTextLine2, uint yTextLine2, byte fillColor) {

	for (int j = yTextLine1; j < yTextLine2 + 1; j++) {
		for (int i = xTextLine1; i < xTextLine2 + 1; i++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = 0;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(xTextLine1, yTextLine1, xTextLine2, yTextLine2));
}

void drawTvText(const Common::String &str1, const Common::String &str2, const Common::String &str3, const Common::String &str4, const Common::String &str5) {
	drawText(80, 0, str1, str2, str3, str4, str5, 253, 0);
}

void clearTvText() {
	removeText(80, 0, 319, 53, 0);
}

void drawCharacterText(const Common::String &str1, const Common::String &str2, const Common::String &str3, const Common::String &str4, const Common::String &str5) {
	drawText(2, 100, str1, str2, str3, str4, str5, 255, 0);
}
void clearCharacterText() {
	removeText(2, 100, 134, 199, 0);
}

void handleFlcEvent(byte eventNumber, uint loopNumber, byte frameCount) {

	const char *const *messages = getAnimMessagesByCurrentLanguage();
	bool isSpanish = isLanguageSpanish();
	bool isEnglish = !isSpanish;
	switch (eventNumber) {
	case 0:
		if (g_engine->_cpCounter > 103)
			showError(274);
		break;
	case 1:
		if (frameCount == 3)

			switch (loopNumber) {
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
				if (isEnglish) {
					clearCharacterText();
				}
				break;
			case 41:
				if (isEnglish) {
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
				if (isEnglish) {
					clearTvText();
				}
				break;
			case 71:
				if (isEnglish) {
					drawTvText(messages[255], messages[256], messages[257], messages[258], messages[259]);
				}
				break;
			case 73:
				if (isEnglish) {
					clearCharacterText();
				}
			case 74:
				if (isEnglish) {
					drawCharacterText(
						messages[35],
						messages[36],
						messages[37],
						messages[38],
						messages[39]);
				}
			case 75:
				if (isSpanish) {
					clearCharacterText();
				}
				break;
			case 76:
				if (isSpanish) {
					clearTvText();
				}
				break;
			case 77:
				if (isSpanish) {
					drawCharacterText(messages[35], messages[36], messages[37], messages[38], messages[39]);
				}
				break;

			case 80:
				if (isEnglish) {
					clearTvText();
				}
				break;
			case 82:
				if (isEnglish) {
					clearCharacterText();
				}
				break;
			case 83:
				if (isEnglish) {
					drawCharacterText(messages[260], messages[261], messages[262], messages[263], messages[264]);
				}
				break;
			case 89:
				if (isSpanish) {
					clearCharacterText();
				}
				break;
			case 90:
				if (isSpanish) {
					drawCharacterText(messages[40], messages[41], messages[42], messages[43], messages[44]);
				} else {
					clearCharacterText();
				}
				break;
			case 91:
				if (isEnglish) {
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
		switch (frameCount) {
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

			if (isEnglish) {
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

			if (isEnglish) {
				delay(6000);
				clearCharacterText();
				drawCharacterText(messages[95], messages[96], messages[97], messages[98], messages[99]);
			}

			delay(6000);
		} break;
		}
		break;
	case 3:
		switch (frameCount) {
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
			g_engine->_graphics->getImg(0, 0, 319, 29, g_engine->_graphics->_textAreaBackground);
			break;
		}
		break;
	case 4:
		if (frameCount == 3)
			g_engine->_sound->playVoc("TIMBRAZO", 423775, 11213);
		break;
	case 5:
		if ((loopNumber == 1) && (frameCount == 2)) {

			delay(2000);
			drawText(5, 1,
					 messages[100],
					 messages[101],
					 messages[102],
					 messages[103],
					 messages[104],
					 255, 249);
			delay(3500);
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
			drawText(5, 1,
					 messages[110],
					 messages[111],
					 messages[112],
					 messages[113],
					 messages[114],
					 255, 249);
		} else if ((loopNumber == 5) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
			drawText(5, 1,
					 messages[120],
					 messages[121],
					 messages[122],
					 messages[123],
					 messages[124],
					 255, 249);
		} else if ((loopNumber == 5) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
			drawText(5, 1,
					 messages[150],
					 messages[151],
					 messages[152],
					 messages[153],
					 messages[154],
					 255, 249);
		} else if ((loopNumber == 5) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		switch (frameCount) {
		case 1:
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if ((loopNumber == 1) && (frameCount == 3)) {
			g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if (frameCount == 1)
			drawText(5, 121,
					 messages[170],
					 messages[171],
					 messages[172],
					 messages[173],
					 messages[174],
					 253, 249);
		break;
	case 20:
		switch (loopNumber) {
		case 1:
			switch (frameCount) {
			case 1:
				g_engine->_graphics->getImg(0, 0, 319, 69, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			if (frameCount == 17)
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
			break;
		}
		break;
	case 21:
		switch (frameCount) {
		case 1:
			g_engine->_sound->playVoc("TRIDEN", 409405, 14370);
			break;
		case 5:
			g_engine->_sound->playVoc("PUFF", 191183, 18001);
			break;
		}
		break;
	case 22:
		if (frameCount == 24)
			g_engine->_sound->playVoc("PUFF", 191183, 18001);
		break;
	case 23:
		switch (frameCount) {
		case 8:
			g_engine->_sound->playVoc("AFILAR", 0, 6433);
			break;
		case 18:
			g_engine->_sound->playVoc();
			break;
		}
		break;
	case 24:
		if (frameCount == 8)
			g_engine->_sound->playVoc("DECAPITA", 354269, 1509);
		break;
	case 25:
		if (frameCount == 97)
			g_engine->_sound->playVoc("PUFF2", 209184, 14514);
		break;
	case 26:
		switch (loopNumber) {
		case 1:
			switch (frameCount) {
			case 2:
				g_engine->_graphics->getImg(0, 0, 319, 69, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
			switch (frameCount) {
			case 1:
				g_engine->_graphics->putImg(0, 0, g_engine->_graphics->_textAreaBackground);
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
		if (frameCount == 148)
			g_engine->_sound->playVoc("DECAPITA", 354269, 1509);
		break;
	}
}

void drawFlc(uint x, uint y, int32 fliOffset, uint loop,
			 byte speed, byte eventNumber, bool fullPalette, bool isSkipAllowed,
			 bool doscientos, bool &salidaflis);

static void exitProcedure(bool &exitLoop, bool isSkipAllowed) {
	exitLoop = false;
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		changeGameSpeed(e);
		if (isSkipAllowed && (e.type == Common::EVENT_KEYDOWN || (e.type == Common::EVENT_LBUTTONUP))) {
			exitLoop = true;
		}
	}
}

static FliHeader readHeader(Common::File *file) {
	FliHeader headerfile;

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
	file->read(headerfile.reserved2, 19 * 2);
	headerfile.ofsframe1 = file->readSint32LE();
	headerfile.ofsframe2 = file->readSint32LE();
	file->read(headerfile.reserved2, 20 * 2);
	return headerfile;
}

void blit(TotFlicDecoder *flic, const Graphics::Surface *src, Common::Rect bounds) {
	Graphics::Surface dest = g_engine->_screen->getSubArea(bounds);
	flic->copyDirtyRectsToBuffer((byte *)dest.getPixels(), dest.pitch);
	g_engine->_screen->addDirtyRect(bounds);
	g_engine->_screen->update();
}

void drawFlc(
	uint x,
	uint y,
	int32 offset,
	uint loop,
	byte speed,
	byte eventNumber,
	bool fullPalette,
	bool isSkipAllowed,
	bool limitPaletteTo200,
	bool &exitAnim) {

	uint loopNumber = 0;
	byte frameCount = 0;

	Common::File animationsFile;
	Common::String fileName;
	if (loop == 60000)
		fileName = "OBJGIRO.DAT";
	else
		fileName = "FILMS.DAT";

	if (!animationsFile.open(Common::Path(fileName))) {
		showError(272);
	}
	animationsFile.seek(offset, SEEK_SET);
	// Need to read header to get the total size of the FLIC file.
	FliHeader header = readHeader(&animationsFile);
	Common::SeekableSubReadStream *thisFlic = new Common::SeekableSubReadStream(
		&animationsFile,
		offset,
		offset + header.size);

	TotFlicDecoder *flic = new TotFlicDecoder();

	flic->loadStream(thisFlic);
	if(speed == 9) {
		debug("Playing at half speed!");
	}
	flic->start();
	bool skipFrame = false;
	do {
		exitProcedure(exitAnim, isSkipAllowed);
		loopNumber++;
		do {
			g_engine->_chrono->updateChrono();
			exitProcedure(exitAnim, isSkipAllowed);
			if (exitAnim) {
				goto Lexit_proc;
			}

			if (g_engine->_chrono->_gameTick) {
				// Make sure we also update the palette animations! Esp. for part 2
				if (g_engine->_currentRoomData != nullptr && !g_engine->_shouldQuitGame) {
					g_engine->_graphics->advancePaletteAnim();
				}
				if(speed == 9) {
					skipFrame = !skipFrame;
				}
				handleFlcEvent(eventNumber, loopNumber, frameCount);
				if (!skipFrame) {
					const Graphics::Surface *frame = flic->decodeNextFrame();
					if (frame) {
						frameCount++;
						Common::Rect boundingBox = Common::Rect(x, y, x + flic->getWidth() + 1, y + flic->getHeight() + 1);
						blit(flic, frame, boundingBox);
						if (flic->hasDirtyPalette()) {

							const byte *fliPalette = (const byte *)flic->getPalette();
							byte *palette = (byte *)malloc(768);
							Common::copy(fliPalette, fliPalette + 768, palette);
							// game fixes background to 0
							palette[0] = 0;
							palette[1] = 0;
							palette[2] = 0;
							if (fullPalette) {
								g_engine->_graphics->fadePalettes(g_engine->_graphics->getPalette(), palette);
								g_engine->_graphics->copyPalette(palette, g_engine->_graphics->_pal);
							} else if (limitPaletteTo200) {
								g_engine->_graphics->setPalette(palette, 0, 200);
								for (int i = 0; i <= 200; i++) {
									if (g_engine->_gamePart == 2 && !g_engine->_shouldQuitGame && (i == 131 || i == 134 || i == 143 || i == 187)) {
										continue;
									}
									g_engine->_graphics->_pal[i * 3 + 0] = palette[i * 3 + 0];
									g_engine->_graphics->_pal[i * 3 + 1] = palette[i * 3 + 1];
									g_engine->_graphics->_pal[i * 3 + 2] = palette[i * 3 + 2];
								}

							} else {
								g_engine->_graphics->setPalette(palette);
								g_engine->_graphics->copyPalette(palette, g_engine->_graphics->_pal);
							}
						}

						g_engine->_chrono->_gameTick = false;
					} else {
						break;
					}
				}
			}
			g_system->delayMillis(10);
		} while (!flic->endOfVideo() && !g_engine->shouldQuit());

		if (flic->endOfVideo()) {
			if (flic->isRewindable()) {
				flic->rewind();
			}
			frameCount = 0;
		}
	} while (loopNumber <= loop && !g_engine->shouldQuit());
	flic->stop();
Lexit_proc:
	animationsFile.close();
}

void clearAnims() {
	free(g_engine->_graphics->_textAreaBackground);
}
} // End of namespace Tot
