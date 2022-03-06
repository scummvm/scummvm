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

#include "common/memstream.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/mcga_graphics.h"
#include "chewy/mcga.h"

namespace Chewy {

McgaGraphics::McgaGraphics() {
}

McgaGraphics::~McgaGraphics() {
}

void McgaGraphics::init() {
	init_mcga();
}

void McgaGraphics::setClip(int16 x1, int16 y1, int16 x2, int16 y2) {
	_G(clipx1) = x1;
	_G(clipx2) = x2;
	_G(clipy1) = y1;
	_G(clipy2) = y2;
}

void McgaGraphics::setPointer(byte *ptr) {
	if (ptr) {
		_G(currentScreen) = ptr;
	} else {
		_G(currentScreen) = (byte *)g_screen->getPixels();
	}
}

void McgaGraphics::setPalette(byte *palette) {
	for (int16 i = 0; i < 768; i++)
		_palTable[i] = palette[i];
	set_palette(palette);
}

void McgaGraphics::savePalette(byte *palette) {
	save_palette(palette);
}

void McgaGraphics::rest_palette() {
	restore_palette();
}

void McgaGraphics::raster_col(int16 c, int16 r, int16 g, int16 b) {
	int16 index = c * 3;
	_palTable[index] = r;
	_palTable[index + 1] = g;
	_palTable[index + 2] = b;
	rastercol(c, r, g, b);
}

void McgaGraphics::einblenden(byte *palette, int16 frames) {
	for (int16 j = 63; j >= 0; j--) {
		int16 k = 0;
		for (int16 i = 0; i < 256; i++) {
			int16 r = palette[k];
			int16 g = palette[k + 1];
			int16 b = palette[k + 2];
			int16 r1 = r - j;
			int16 g1 = g - j;
			int16 b1 = b - j;
			if ((r1 > 0) && (r1 <= r))
				_palTable[k] = r1;
			if ((g1 > 0) && (r1 <= r))
				_palTable[k + 1] = g1;
			if ((b1 > 0) && (r1 <= r))
				_palTable[k + 2] = b1;
			k += 3;
		}
		set_palette(_palTable);
	}
}

void McgaGraphics::ausblenden(int16 frames) {
	for (int16 j = 0; j < 64; j++) {
		int16 k = 0;
		for (int16 i = 0; i < 256; i++) {
			int16 r = _palTable[k];
			int16 g = _palTable[k + 1];
			int16 b = _palTable[k + 2];
			if (r > 0) --r;
			if (g > 0) --g;
			if (b > 0) --b;
			_palTable[k] = r;
			_palTable[k + 1] = g;
			_palTable[k + 2] = b;
			k += 3;
		}
		set_palette(_palTable);
	}
}

void McgaGraphics::set_partialpalette(const byte *palette, int16 startCol, int16 nr) {
	int16 k = startCol * 3;
	int16 endcol = startCol + nr;
	for (int16 i = startCol; i < endcol; i++) {
		_palTable[k] = palette[k];
		_palTable[k + 1] = palette[k + 1];
		_palTable[k + 2] = palette[k + 2];
		k += 3;
	}
	setPartialPalette(_palTable, startCol, nr);
}

void McgaGraphics::cls() {
	clear_mcga();
}

void McgaGraphics::drawLine(int16 x1, int16 y1, int16 x2, int16 y2, int16 color) {
	line_mcga(x1, y1, x2, y2, color);
}

void McgaGraphics::box(int16 x1, int16 y1, int16 x2, int16 y2, int16 color) {
	line_mcga(x1, y1, x2, y1, color);
	line_mcga(x1, y2 - 1, x2, y2 - 1, color);
	line_mcga(x1, y1, x1, y2, color);
	line_mcga(x2, y1, x2, y2, color);
}

void McgaGraphics::boxFill(int16 x1, int16 y1, int16 x2, int16 y2, int16 color) {
	if (x2 == x1)x2++;
	int16 h = abs(y2 - y1);
	if (h == 0)
		h = 1;
	for (int16 i = 0; i < h; i++)
		line_mcga(x1, y1 + i, x2, y1 + i, color);
}

void McgaGraphics::pop_box(int16 x, int16 y, int16 x1, int16 y1, int16 col1, int16 col2, int16 back_col) {
	if (back_col < 255)
		boxFill(x, y, x1, y1, back_col);
	drawLine(x, y1, x1, y1, col2);
	drawLine(x1, y, x1, y1, col2);
	drawLine(x, y, x1, y, col1);
	drawLine(x, y, x, y1 + 1, col1);
}

void McgaGraphics::back2screen(byte *ptr) {
	mem2mcga(ptr);
}

void McgaGraphics::spriteSave(byte *spritePtr, int16 x, int16 y, int16 width, int16 height, int16 screenWidth) {
	if (width < 4)
		width = 4;
	if (height <= 0)
		height = 1;
	if (x < _G(clipx1)) {
		x = _G(clipx1);
		width -= (_G(clipx1) - x);
	}
	if ((x + width) > _G(clipx2) + 1)
		width = _G(clipx2) - x;
	if (y < _G(clipy1)) {
		y = _G(clipy1);
		height -= (_G(clipy1) - y);
	}
	if ((y + height) > _G(clipy2) + 1)
		height = _G(clipy2) - y;
	if (width < 1)
		width = 0;
	if (height <= 0)
		height = 0;

	spr_save_mcga(spritePtr, x, y, width, height, screenWidth);
}

void McgaGraphics::spriteSet(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	mspr_set_mcga(sptr, x, y, scrwidth);
}

void McgaGraphics::map_spr2screen(byte *sptr, int16 x, int16 y) {
	int16 br = ((int16 *)sptr)[0];
	int16 h = ((int16 *)sptr)[1];
	if ((br >= 320) || (h >= 200))
		map_spr_2screen(sptr, x, y);
}

int16 McgaGraphics::scanxy(int16 x, int16 y, int16 fcol, int16 bcol, int16 cur_col, int16 scrwidth,
                          const char *string, ...) {
	int16 i, j, stelle, stellemax, mode = 0;
	int16 disp_stelle = 0, disp_stellemax = 0, disp_akt = 0;
	int16 ret = 0;
	int16 delay_flag;
	int8 ende = 0, zeichen, zaehler, cursor_z;
	char zstring[81], z1string[81];
	int8 einfuege_cur = 0;
	unsigned char char_anf = 0, char_end = 0;
	uint16 vorzeichen = 0;
	int8 x_mode = 0;
	int8 eing = 0;
	uint16 izahl = 0;
	uint16 *intzahl = nullptr;
	int16 *intzahl1 = nullptr;
	uint32 *longzahl = nullptr;
	char *charstr = nullptr;
	uint32 luzahl;
	va_list parptr;
	va_start(parptr, string);

	KbdInfo kbInfo;
	KbdInfo *kb_old = g_events->setKbdInfo(&kbInfo);

	if ((x == -1) || (y == -1)) {
		x = _G(gcurx);
		y = _G(gcury);
	} else {
		_G(gcurx) = x;
		_G(gcury) = y;
	}

	move(x, y);
	i = 0;

	ChewyFont *font = _G(fontMgr)->getFont();

	while (!ende) {
		zeichen = string[i];
		++i;

		if ((zeichen >= font->getFirst()) && (zeichen <= font->getLast()) && (zeichen != 127)) {
			if (zeichen == '%') {
				zeichen = string[i];
				++i;
				zaehler = 0;

				if (zeichen == '[') {
					char_anf = string[i];
					++i;
					++i;
					char_end = string[i];
					++i;
					++i;
					zeichen = string[i];
					++i;
				}

				while ((zeichen >= '0') && (zeichen <= '9')) {
					zaehler *= 10;
					zaehler += (zeichen - 48);
					if (zaehler > 80)
						zaehler = 80;
					zeichen = string[i];
					++i;
				}
				if (_einfuegen == 1) {
					einfuege_cur = 125;
					cursor_z = '_' + einfuege_cur;
				} else {
					einfuege_cur = 0;
					cursor_z = '_';
				}

				switch (zeichen) {
				case 'd':
					vorzeichen = '-';
					// Fall through
				case 'u':
					if ((char_anf == 0) && (char_end == 0)) {
						char_anf = '0';
						char_end = '9';
					}
					if (!zaehler)
						zaehler = 4;
					if (zeichen == 'd') {
						mode = 4;
						intzahl1 = va_arg(parptr, int16 *);
						luzahl = intzahl1[0];
					} else {
						mode = 1;
						intzahl = va_arg(parptr, uint16 *);
						luzahl = intzahl[0];
					}
					if (luzahl != 0)
						ltoa(luzahl, zstring, 10);
					else {
						zstring[0] = '0';
						zstring[1] = 0 ;
					}
					break;
				case 'l':
					vorzeichen = '-';
					if ((char_anf == 0) && (char_end == 0))
						vorzeichen = '-';
					{
						char_anf = '0';
						char_end = '9';
					}
					mode = 2;
					if (!zaehler)
						zaehler = 9;
					longzahl = va_arg(parptr, uint32 *);
					luzahl = longzahl[0];
					if (luzahl != 0)
						ultoa(luzahl, zstring, 10);
					else {
						zstring[0] = '0';
						zstring[1] = 0 ;
					}
					break;
				case 's':
					if ((char_anf == 0) && (char_end == 0)) {
						char_anf = ' ';
						char_end = 0xdc;
					}
					mode = 3;
					vorzeichen = 0xFF;
					if (!zaehler)
						zaehler = 81;
					charstr = va_arg(parptr, char *);
					strcpy(zstring, charstr);
					break;

				default:
					break;
				}

				zeichen = string[i];
				++i;
				if (zeichen == 'x') {
					x_mode = 1;
					zeichen = string[i];
					++i;
				}
				disp_stellemax = 0;

				while ((zeichen >= '0') && (zeichen <= '9')) {
					disp_stellemax *= 10;
					disp_stellemax += (zeichen - 48);
					if (disp_stellemax > 80)
						disp_stellemax = 80;
					zeichen = string[i];
					++i;
				}
				if (!disp_stellemax)
					disp_stellemax = zaehler;

				stelle = 0;
				disp_stelle = 0;
				while (zstring[stelle] != 0) {
					++stelle;
				}
				stellemax = stelle;
				if (stelle > disp_stellemax) {
					disp_stelle = stelle - disp_stellemax;
					disp_akt = disp_stellemax;
				} else {
					disp_akt = stelle;
					disp_stelle = 0;
				}
				for (j = stelle; j < j + 1; j++) {
					while ((!kbhit()) || (eing != 0)) {
						EVENTS_UPDATE;

						eing = 0;
						move(x, y);

						for (i = disp_stelle; i <= disp_stellemax + disp_stelle; ++i) {
							if (zstring[i] != 0) {
								putz(zstring[i], fcol, bcol, scrwidth);
								vors();
							} else {
								putz(zstring[i], fcol, bcol, scrwidth);
								vors();
								break;
							}
						}
						if (_svga == ON)
							upd_scr();
						for (delay_flag = 0; (delay_flag < 10) && (!kbhit()); delay_flag++) {
							izahl = devices();

							if (izahl == Common::KEYCODE_RETURN) {
								eing = 2;
								break;
							} else if (izahl == Common::KEYCODE_ESCAPE) {
								eing = 2;
								break;
							}
						}
						const uint16 fontWidth = _G(fontMgr)->getFont()->getDataWidth();
						plot_scan_cur((x + (disp_akt)*fontWidth), _G(gcury), cur_col, 300, scrwidth, cursor_z);
						if (_svga == ON)
							upd_scr();
						for (delay_flag = 0; (delay_flag < 10) && (!kbhit()); delay_flag++) {
							izahl = devices();
							if (izahl == Common::KEYCODE_RETURN) {
								eing = 2;
								break;
							} else if (izahl == Common::KEYCODE_ESCAPE) {
								eing = 2;
								break;
							}
						}
						if (eing == 2)
							break;
					}
					if (eing < 2)
						izahl = (uint16)getch();

					if (izahl == Common::KEYCODE_RETURN) {
						ret = Common::KEYCODE_RETURN;
						ende = 1;
						break;
					}

					if (izahl == Common::KEYCODE_ESCAPE) {
						ret = Common::KEYCODE_ESCAPE;
						ende = 2;
						break;
					}

					if (izahl == Common::KEYCODE_BACKSPACE) {
						eing = 1;
						while (kbhit())
							getch();

						if (stelle > 0) {
							const uint16 fontWidth = _G(fontMgr)->getFont()->getDataWidth();
							strcpy(zstring + stelle - 1, zstring + stelle);
							plot_scan_cur((x + disp_akt * fontWidth), _G(gcury), bcol, bcol, scrwidth, cursor_z);
							if (_svga == ON)
								upd_scr();
							--stelle;
							--stellemax;
							if (disp_stelle > 0)
								--disp_stelle;
							else if (disp_akt > 0) {
								--disp_akt;
							}
						} else {
							putch(7);
						}
					}

					if (izahl == 0) {
						izahl = getch();

						if (izahl == 75) {
							while (kbhit())
								getch();
							eing = 1;
							if (stelle <= 0) {
								putch(7);
								stelle = 0;

							} else {
								if (disp_akt > 0) {
									--disp_akt;
								} else {
									if (disp_stelle > 0)
										--disp_stelle;
								}
								--stelle;
							}
						}

						if (izahl == 77) {
							eing = 1;
							while (kbhit())
								getch();
							++j;
							if (stelle >= stellemax) {
								stelle = stellemax;
								putch(7);
							} else {
								if (disp_akt < disp_stellemax) {
									++disp_akt;
								} else {
									if (disp_stelle <= (zaehler - disp_stellemax)) {
										++disp_stelle;
									}
								}
								++stelle;
							}
						}

						if (izahl == 71) {
							stelle = 0;
							disp_akt = 0;
							disp_stelle = 0;
						}

						if (izahl == 79) {
							stelle = stellemax;
							if (stellemax < disp_stellemax) {
								disp_stelle = 0;
								disp_akt = stellemax;
							} else {
								disp_akt = disp_stellemax;
								disp_stelle = stelle - disp_stellemax;
							}
						}

						if ((izahl == 83) && (stelle < stellemax)) {
							while (kbhit())
								getch();
							eing = 1;
							strcpy(zstring + stelle, zstring + stelle + 1);
							--stellemax;
						}

						if (izahl == 82) {
							g_events->delay(200);
							if (!_einfuegen) {
								_einfuegen = 1;
								einfuege_cur = 125;
								cursor_z = '_' + einfuege_cur;
							} else {
								_einfuegen = 0;
								einfuege_cur = 0;
								cursor_z = '_';
							}
						}
						izahl = 0;
					}

					if (izahl < char_anf || izahl > char_end) {
						if (vorzeichen == izahl && !stelle);
						else {
							if (x_mode == 1) {
								ende = 1;
								ret = izahl;
								break;
							}
							continue;
						}
					}
					if ((stelle == stellemax) && (stellemax >= zaehler)) {
						stellemax = zaehler;
						putch(7);

					} else {
						if (disp_akt < disp_stellemax) {
							++disp_akt;
						} else {
							if (disp_stelle <= (zaehler - disp_stellemax)) {
								++disp_stelle;
								disp_akt = disp_stellemax;
							}
						}
						if (!_einfuegen) {
							zstring[stelle] = izahl;
							zstring[stellemax + 1] = 0;
							if (stelle == stellemax) {
								++stelle;
								++stellemax;
							} else {
								++stelle;
							}
						} else {
							strcpy(z1string, zstring);
							strcpy(zstring + stelle + 1, z1string + stelle);
							zstring[stelle] = izahl;
							zstring[(int)zaehler] = 0;
							if (stellemax < zaehler) {
								++stelle;
								++stellemax;
							} else {
								++stelle;
							}
						}
					}

					if (x_mode == 1) {
						ende = 1;
						ret = izahl;
						break;
					}
				}
			}

		}
	}

	move(x, y);
	for (i = disp_stelle; i <= disp_stellemax + disp_stelle; ++i) {
		if (zstring[i] != 0) {
			putz(zstring[i], fcol, bcol, scrwidth);
			vors();
		} else {
			putz(zstring[i], fcol, bcol, scrwidth);
			vors();
			break;
		}
	}

	if (ende == 1) {
		switch (mode) {
		case 1:
			intzahl[0] = atoi(zstring);
			break;
		case 2:
			longzahl[0] = atol(zstring);
			break;
		case 3:
			strcpy(charstr, zstring);
			break;
		case 4:
			intzahl1[0] = atoi(zstring);
			break;
		default:
			break;
		}
	}

	if (_svga == ON)
		upd_scr();

	g_events->setKbdInfo(kb_old);

	va_end(parptr);
	return ret;
}

void McgaGraphics::plot_scan_cur(int16 x, int16 y, int16 fcol, int16 bcol, int16 scrwidth, char cursor_z) {
	move(x, y);
	putz(cursor_z, fcol, bcol, scrwidth);
}

void McgaGraphics::printxy(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 scrwidth, const char *string) {
	ChewyFont *font = _G(fontMgr)->getFont();
	Graphics::Surface *textSurface = font->getLine(string);
	byte *data = (byte *)textSurface->getPixels();

	for (int curX = 0; curX < textSurface->pitch; curX++) {
		for (int curY = 0; curY < textSurface->h; curY++) {
			if (curX + x < 320 && curY + y < 200) {
				byte *src = data + (curY * textSurface->pitch) + curX;
				byte *dst = (byte *)_G(currentScreen).getBasePtr(curX + x, curY + y);
				if (*src != 0xFF)
					*dst = fgCol;
				else if (bgCol < 0xFF)
					*dst = bgCol;
			}
		}
	}

	g_screen->addDirtyRect(Common::Rect(
		x, y, x + textSurface->pitch, y + textSurface->h));

	textSurface->free();
	delete textSurface;
}

void McgaGraphics::move(int16 x, int16 y) {
	_G(gcurx) = x;
	_G(gcury) = y;
}

int16 McgaGraphics::devices() {
	int16 i = 0;
	// TODO: This is wrong. 'i' should be initialized to "button status" (BX after a call to Int33h/03
	if (i > 1) {
		i = 27;
	} else if (i == 1)
		i = 13;
	else
		i = 0;

	return i;
}

void McgaGraphics::scale_set(byte *sptr, int16 x, int16 y, int16 xdiff_, int16 ydiff_, int16 scrwidth) {
	if (xdiff_ || ydiff_)
		zoom_set(sptr, x, y, xdiff_, ydiff_, scrwidth);
	else
		mspr_set_mcga(sptr, x, y, scrwidth);
}

} // namespace Chewy
