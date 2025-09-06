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

#include "tot/statics.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

// Enforces a small delay when text reveal is supposed to happen
const int kEnforcedTextAnimDelay = 0;

void showError(int code) {
	const char *const *messages = (isLanguageSpanish()) ? errorMessages[0] : errorMessages[1];
	switch (code) {
	case 1:
		error("%s", messages[0]);
		break;
	case 2:
		error("%s", messages[1]);
		break;
	case 3:
		error("%s", messages[2]);
		break;
	case 4:
		error("%s", messages[3]);
		break;
	case 5:
		error("%s", messages[4]);
		break;
	case 6:
		error("%s", messages[5]);
		break;
	case 12:
		error("%s", messages[6]);
		break;
	case 13:
	case 14:
		error(messages[7], code);
		break;
	case 15:
		error("%s", messages[8]);
		break;
	case 16:
		error("%s", messages[9]);
		break;
	case 17:
		error("%s", messages[10]);
		break;
	case 18:
		error("%s", messages[11]);
		break;
	case 100:
		error("%s", messages[12]);
		break;
	case 101:
		error("%s", messages[13]);
		break;
	case 102:
		error("%s", messages[14]);
		break;
	case 103:
		error("%s", messages[15]);
		break;
	case 104:
		error("%s", messages[16]);
		break;
	case 105:
		error("%s", messages[17]);
		break;
	case 106:
		error("%s", messages[18]);
		break;
	case 150:
		error("%s", messages[19]);
		break;
	case 151:
		error("%s", messages[20]);
		break;
	case 152:
		error("%s", messages[21]);
		break;
	case 153:
		error("%s", messages[22]);
		break;
	case 154:
		error("%s", messages[23]);
		break;
	case 155:
		error(messages[24], code);
		break;
	case 156:
		error("%s", messages[25]);
		break;
	case 157:
		error("%s", messages[26]);
		break;
	case 158:
		error("%s", messages[27]);
		break;
	case 159:
		error("%s", messages[28]);
		break;
	case 160:
		error("%s", messages[29]);
		break;
	case 161:
		error("%s", messages[30]);
		break;
	case 162:
		error("%s", messages[31]);
		break;
	case 200:
		error("%s", messages[32]);
		break;
	case 201:
		error("%s", messages[33]);
		break;
	case 202:
		error("%s", messages[34]);
		break;
	case 203:
		error("%s", messages[35]);
		break;
	case 204:
		error("%s", messages[36]);
		break;
	case 205:
		error("%s", messages[37]);
		break;
	case 206:
		error("%s", messages[38]);
		break;
	case 207:
		error("%s", messages[39]);
		break;
	case 208:
		error("%s", messages[40]);
		break;
	case 209:
		error("%s", messages[41]);
		break;
	case 210:
		error("%s", messages[42]);
		break;
	case 211:
		error("%s", messages[43]);
		break;
	case 212:
		error("%s", messages[44]);
		break;
	case 213:
		error("%s", messages[45]);
		break;
	case 214:
		error("%s", messages[46]);
		break;
	case 215:
		error("%s", messages[47]);
		break;
	case 216:
		error("%s", messages[48]);
		break;
	case 250:
		error("%s", messages[49]);
		break;
	case 251:
		error("%s", messages[50]);
		break;
	case 252:
		error("%s", messages[51]);
		break;
	case 253:
		error("%s", messages[52]);
		break;
	case 254:
		error("%s", messages[53]);
		break;
	case 255:
		error("%s", messages[54]);
		break;
	case 256:
		error("%s", messages[55]);
		break;
	case 257:
		error("%s", messages[56]);
		break;
	case 258:
		error("%s", messages[57]);
		break;
	case 259:
		error("%s", messages[58]);
		break;
	case 260:
		error("%s", messages[59]);
		break;
	case 261:
		error("%s", messages[60]);
		break;
	case 262:
		error("%s", messages[61]);
		break;
	case 263:
		error("%s", messages[62]);
		break;
	case 264:
		error("%s", messages[63]);
		break;
	case 265:
		error("%s", messages[64]);
		break;
	case 266:
		error("%s", messages[65]);
		break;
	case 267:
		error("%s", messages[66]);
		break;
	case 268:
		error("%s", messages[67]);
		break;
	case 269:
		error("%s", messages[68]);
		break;
	case 270:
		error("%s", messages[69]);
		break;
	case 271:
		error("%s", messages[70]);
		break;
	case 272:
		error("%s", messages[71]);
		break;
	case 273:
		error("%s", messages[72]);
		break;
	case 274:
		error("%s", messages[73]);
		break;
	case 275:
		error("%s", messages[74]);
		break;
	case 276:
		error("%s", messages[75]);
		break;
	case 277:
		error("%s", messages[76]);
		break;
	case 278:
		error("%s", messages[77]);
		break;
	case 279:
		error("%s", messages[78]);
		break;
	case 280:
		error("%s", messages[79]);
		break;
	case 281:
		error("%s", messages[80]);
		break;
	case 282:
		error("%s", messages[81]);
		break;
	case 299:
		error("%s", messages[82]);
		break;
	case 300:
		error("%s", messages[83]);
		break;
	case 301:
		error("%s", messages[84]);
		break;
	case 302:
		error("%s", messages[85]);
		break;
	case 303:
		error("%s", messages[86]);
		break;
	case 304:
		error("%s", messages[87]);
		break;
	case 305:
		error("%s", messages[88]);
		break;
	case 306:
		error("%s", messages[89]);
		break;
	case 307:
		error("%s", messages[90]);
		break;
	case 308:
		error("%s", messages[91]);
		break;
	case 309:
		error("%s", messages[92]);
		break;
	case 310:
		error("%s", messages[93]);
		break;
	case 311:
		error("%s", messages[94]);
		break;
	case 312:
		error("%s", messages[95]);
		break;
	case 313:
		error("%s", messages[96]);
		break;
	case 314:
		error("%s", messages[97]);
		break;
	case 315:
		error("%s", messages[98]);
		break;
	case 316:
		error("%s", messages[99]);
		break;
	case 317:
		error("%s", messages[100]);
		break;
	case 318:
		error("%s", messages[101]);
		break;
	case 319:
		error("%s", messages[102]);
		break;
	case 320:
		error("%s", messages[103]);
		break;
	case 321:
		error("%s", messages[104]);
		break;
	case 322:
		error("%s", messages[105]);
		break;
	case 323:
		error("%s", messages[106]);
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
	if (e.type == Common::EVENT_KEYDOWN) {
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

int getRandom(int range) { return g_engine->getRandomNumber(range - 1); }

Common::String getObjectName(int idx) {
	return g_engine->_lang == Common::ES_ESP ? hardcodedTexts_ES[idx] : hardcodedTexts_EN[idx];
}

Common::KeyCode hotKeyFor(HOTKEYS hotkey) {
	const Common::KeyCode *selectedHotkeys = (g_engine->_lang == Common::ES_ESP)? hotkeys[0]: hotkeys[1];
	return selectedHotkeys[hotkey];
};

Common::String getActionLineText(int idx) {
	return g_engine->_lang == Common::ES_ESP ? actionLine_ES[idx] : actionLine_EN[idx];
}

const char *const *getFullScreenMessagesByCurrentLanguage() {
	return (g_engine->_lang == Common::ES_ESP) ? fullScreenMessages[0] : fullScreenMessages[1];
}

const char *const *getAnimMessagesByCurrentLanguage() {
	return (g_engine->_lang == Common::ES_ESP) ? animMessages[0] : animMessages[1];
}

const char *const *getHardcodedTextsByCurrentLanguage() {
	return (g_engine->_lang == Common::ES_ESP) ? hardcodedTexts_ES : hardcodedTexts_EN;
}

const int32 *getOffsetsByCurrentLanguage() {
	return (g_engine->_lang == Common::ES_ESP) ? flcOffsets[0] : flcOffsets[1];
}


} // End of namespace Tot
