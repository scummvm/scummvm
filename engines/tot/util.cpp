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
#include "common/textconsole.h"
#include "graphics/paletteman.h"

#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

// Enforces a small delay when text reveal is supposed to happen
const int kEnforcedTextAnimDelay = 0;

void showError(int code) {
	const char *const *messages = (isLanguageSpanish()) ? errorMessages[0] : errorMessages[1];

	switch (code) {
	case 1:
		error(messages[0]);
		break;
	case 2:
		error(messages[1]);
		break;
	case 3:
		error(messages[2]);
		break;
	case 4:
		error(messages[3]);
		break;
	case 5:
		error(messages[4]);
		break;
	case 6:
		error(messages[5]);
		break;
	case 12:
		error(messages[6]);
		break;
	case 13:
	case 14:
		error(messages[7],
			  code);
		break;
	case 15:
		error(messages[8]);
		break;
	case 16:
		error(messages[9]);
		break;
	case 17:
		error(messages[10]);
		break;
	case 18:
		error(messages[11]);
		break;
	case 100:
		error(messages[12]);
		break;
	case 101:
		error(messages[13]);
		break;
	case 102:
		error(messages[14]);
		break;
	case 103:
		error(messages[15]);
		break;
	case 104:
		error(messages[16]);
		break;
	case 105:
		error(messages[17]);
		break;
	case 106:
		error(messages[18]);
		break;
	case 150:
		error(messages[19]);
		break;
	case 151:
		error(messages[20]);
		break;
	case 152:
		error(messages[21]);
		break;
	case 153:
		error(messages[22]);
		break;
	case 154:
		error(messages[23]);
		break;
	case 155:
		error(messages[24], code);
		break;
	case 156:
		error(messages[25]);
		break;
	case 157:
		error(messages[26]);
		break;
	case 158:
		error(messages[27]);
		break;
	case 159:
		error(messages[28]);
		break;
	case 160:
		error(messages[29]);
		break;
	case 161:
		error(messages[30]);
		break;
	case 162:
		error(messages[31]);
		break;
	case 200:
		error(messages[32]);
		break;
	case 201:
		error(messages[33]);
		break;
	case 202:
		error(messages[34]);
		break;
	case 203:
		error(messages[35]);
		break;
	case 204:
		error(messages[36]);
		break;
	case 205:
		error(messages[37]);
		break;
	case 206:
		error(messages[38]);
		break;
	case 207:
		error(messages[39]);
		break;
	case 208:
		error(messages[40]);
		break;
	case 209:
		error(messages[41]);
		break;
	case 210:
		error(messages[42]);
		break;
	case 211:
		error(messages[43]);
		break;
	case 212:
		error(messages[44]);
		break;
	case 213:
		error(messages[45]);
		break;
	case 214:
		error(messages[46]);
		break;
	case 215:
		error(messages[47]);
		break;
	case 216:
		error(messages[48]);
		break;
	case 250:
		error(messages[49]);
		break;
	case 251:
		error(messages[50]);
		break;
	case 252:
		error(messages[51]);
		break;
	case 253:
		error(messages[52]);
		break;
	case 254:
		error(messages[53]);
		break;
	case 255:
		error(messages[54]);
		break;
	case 256:
		error(messages[55]);
		break;
	case 257:
		error(messages[56]);
		break;
	case 258:
		error(messages[57]);
		break;
	case 259:
		error(messages[58]);
		break;
	case 260:
		error(messages[59]);
		break;
	case 261:
		error(messages[60]);
		break;
	case 262:
		error(messages[61]);
		break;
	case 263:
		error(messages[62]);
		break;
	case 264:
		error(messages[63]);
		break;
	case 265:
		error(messages[64]);
		break;
	case 266:
		error(messages[65]);
		break;
	case 267:
		error(messages[66]);
		break;
	case 268:
		error(messages[67]);
		break;
	case 269:
		error(messages[68]);
		break;
	case 270:
		error(messages[69]);
		break;
	case 271:
		error(messages[70]);
		break;
	case 272:
		error(messages[71]);
		break;
	case 273:
		error(messages[72]);
		break;
	case 274:
		error(messages[73]);
		break;
	case 275:
		error(messages[74]);
		break;
	case 276:
		error(messages[75]);
		break;
	case 277:
		error(messages[76]);
		break;
	case 278:
		error(messages[77]);
		break;
	case 279:
		error(messages[78]);
		break;
	case 280:
		error(messages[79]);
		break;
	case 281:
		error(messages[80]);
		break;
	case 282:
		error(messages[81]);
		break;
	case 299:
		error(messages[82]);
		break;
	case 300:
		error(messages[83]);
		break;
	case 301:
		error(messages[84]);
		break;
	case 302:
		error(messages[85]);
		break;
	case 303:
		error(messages[86]);
		break;
	case 304:
		error(messages[87]);
		break;
	case 305:
		error(messages[88]);
		break;
	case 306:
		error(messages[89]);
		break;
	case 307:
		error(messages[90]);
		break;
	case 308:
		error(messages[91]);
		break;
	case 309:
		error(messages[92]);
		break;
	case 310:
		error(messages[93]);
		break;
	case 311:
		error(messages[94]);
		break;
	case 312:
		error(messages[95]);
		break;
	case 313:
		error(messages[96]);
		break;
	case 314:
		error(messages[97]);
		break;
	case 315:
		error(messages[98]);
		break;
	case 316:
		error(messages[99]);
		break;
	case 317:
		error(messages[100]);
		break;
	case 318:
		error(messages[101]);
		break;
	case 319:
		error(messages[102]);
		break;
	case 320:
		error(messages[103]);
		break;
	case 321:
		error(messages[104]);
		break;
	case 322:
		error(messages[105]);
		break;
	case 323:
		error(messages[106]);
		break;
	default: {
		if (code >= 7 && code <= 11) {
			error(messages[107], code);
		} else if (code >= 19 && code <= 99) {
			error(messages[108], code);
		} else if (code >= 107 && code <= 149) {
			error(messages[109],
				  code);
		} else if (code >= 163 && code <= 199) {
			error(messages[110], code);
		} else if (code >= 217 && code <= 249) {
			error(messages[111], code);
		} else if (code >= 283 && code <= 298) {
			error(messages[112], code);
		} else {
			error(messages[113], code);
		}
	}
	}
}

void delay(uint16 ms) {
	g_engine->_chrono->delay(ms);
}

/*
Allows you to modify palette entries for the IBM
8514 and the VGA drivers.
*/
void setRGBPalette(int color, int r, int g, int b) {
	// Take current palette
	byte palbuf[768];
	g_system->getPaletteManager()->grabPalette(palbuf, 0, 256);

	palbuf[color * 3 + 0] = r << 2;
	palbuf[color * 3 + 1] = g << 2;
	palbuf[color * 3 + 2] = b << 2;

	g_system->getPaletteManager()->setPalette(palbuf, 0, 256);
}

void littText(int x, int y, const Common::String &text, byte color, Graphics::TextAlign align) {
	littText(x, y, text.c_str(), color, align);
}

void littText(int x, int y, const char *text, byte color, Graphics::TextAlign align) {
	bool yAligned = (align == Graphics::kTextAlignCenter) ? true : false;
	x = (align == Graphics::kTextAlignCenter) ? 0 : x;
	y = y + 2;
	g_engine->_graphics->littText(text, x, y, color, align, yAligned);
}

void euroText(int x, int y, const Common::String &text, byte color, Graphics::TextAlign align) {
	euroText(x, y, text.c_str(), color, align);
}

void euroText(int x, int y, const char *text, byte color, Graphics::TextAlign align) {
	bool yAligned = (align == Graphics::kTextAlignCenter) ? true : false;
	x = (align == Graphics::kTextAlignCenter) ? 0 : x;
	y = y + 2;
	g_engine->_graphics->euroText(text, x, y, color, align, yAligned);
}

void biosText(int x, int y, const Common::String &text, byte color) {
	g_engine->_graphics->biosText(text, x, y, color);
}

uint imagesize(int x, int y, int x2, int y2) {
	int w = x2 - x + 1;
	int h = y2 - y + 1;
	return 4 + (w * h);
}

void rectangle(int x1, int y1, int x2, int y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y1, color);
	g_engine->_screen->drawLine(x2, y1, x2, y2, color);
	g_engine->_screen->drawLine(x1, y2, x2, y2, color);
	g_engine->_screen->drawLine(x1, y1, x1, y2, color);
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void bar(int x1, int y1, int x2, int y2, byte color) {
	x2 = x2 + 1;
	y2 = y2 + 1;
	for (int i = x1; i < x2; i++) {
		for (int j = y1; j < y2; j++) {
			*((byte *)g_engine->_screen->getBasePtr(i, j)) = color;
		}
	}
	g_engine->_screen->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void putpixel(int x, int y, byte color) {
	g_engine->_screen->setPixel(x, y, color);
}

void line(uint x1, uint y1, uint x2, uint y2, byte color) {
	g_engine->_screen->drawLine(x1, y1, x2, y2, color);
}

void changeGameSpeed(Common::Event e) {
	if (e.type == Common::EVENT_KEYUP) {
		if (e.kbd.hasFlags(Common::KBD_CTRL)) {
			if (e.kbd.keycode == Common::KEYCODE_f) {
				g_engine->_chrono->changeSpeed();
			}
		}
	}
}

void emptyLoop() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (!g_engine->_chrono->_gameTick && !g_engine->shouldQuit());
}

void emptyLoop2() {
	do {
		g_engine->_chrono->updateChrono();
		g_engine->_screen->update();
	} while (!g_engine->_chrono->_gameTickHalfSpeed);
}

void waitForKey() {
	bool waitForKey = false;
	Common::Event e;
	debug("Waiting for key!");
	while (!waitForKey && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_KEYDOWN) {
				waitForKey = true;
			}
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
}

uint Random(uint range) { return g_engine->getRandomNumber(range); }
int Random(int range) { return g_engine->getRandomNumber(range - 1); }

} // End of namespace Tot
