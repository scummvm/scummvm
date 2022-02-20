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
#include "chewy/mcga_graphics.h"
#include "chewy/mcga.h"

namespace Chewy {

#define MAXSTRING 255

McgaGraphics::McgaGraphics() {
	for (int i = 0; i < 360; i++) {
		_sines[i] = sin(i * 3.14159265359 / 180);
		_cosines[i] = cos(i * 3.14159265359 / 180);
	}
}

McgaGraphics::~McgaGraphics() {
}

void McgaGraphics::init() {
	_G(scr_w) = SCREEN_WIDTH;
	_G(scr_h) = SCREEN_HEIGHT;
	init_mcga();
}

void McgaGraphics::set_writemode(char wm) {
	_writeMode = wm;
}

void McgaGraphics::set_clip(int16 x1, int16 y1, int16 x2, int16 y2) {
	_G(clipx1) = x1;
	_G(clipx2) = x2;
	_G(clipy1) = y1;
	_G(clipy2) = y2;
}

void McgaGraphics::set_pointer(byte *ptr) {
	if (ptr) {
		_G(currentScreen) = ptr;
	} else if (_G(screenHasDefault)) {
		_G(currentScreen) = _G(screenDefaultP);
	} else {
		_G(currentScreen) = (byte *)g_screen->getPixels();
	}
}

byte *McgaGraphics::get_zeiger() {
	return get_dispoff();
}

void McgaGraphics::set_mono() {
	_mono = true;
}

void McgaGraphics::calc_mono(byte *palette, int16 startcol, int16 anz) {
	int16 k = startcol * 3;
	for (int16 i = 0; i < anz; i++) {
		uint8 r = palette[k];
		uint8 g = palette[k + 1];
		uint8 b = palette[k + 2];
		uint8 grau = (((float)r) * 0.30) + (((float)g) * 0.59) + (((float)b) * 0.11);
		palette[k] = grau;
		palette[k + 1] = grau;
		palette[k + 2] = grau;
		k += 3;
	}
}

void McgaGraphics::set_palette(byte *palette) {
	for (int16 i = 0; i < 768; i++)
		_palTable[i] = palette[i];
	if (_mono)
		calc_mono(palette, 0, 256);
	setpalette(palette);
}

void McgaGraphics::palette_save(byte *palette) {
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
	if (_mono)
		calc_mono(_palTable, c, 1);
	rastercol(c, r, g, b);
}

void McgaGraphics::einblenden(byte *palette, int16 frames) {
	if (_mono)
		calc_mono(palette, 0, 256);
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
		setpalette(_palTable);
	}
}

void McgaGraphics::aufhellen(byte *palette, int16 startcol, int16 anz, int16 stufen, int16 frames) {
	if (_mono)
		calc_mono(palette, startcol, anz);
	for (int16 j = stufen; j >= 0; j--) {
		int16 endcol = startcol + anz;
		int16 k = startcol * 3;
		for (int16 i = startcol; i < endcol; i++) {
			int16 r = _palTable[k];
			int16 g = _palTable[k + 1];
			int16 b = _palTable[k + 2];
			if ((r + 1) <= palette[k]) ++r;
			if ((g + 1) <= palette[k + 1]) ++g;
			if ((b + 1) <= palette[k + 2]) ++b;
			_palTable[k] = r;
			_palTable[k + 1] = g;
			_palTable[k + 2] = b;
			k += 3;
		}
		set_palpart(_palTable, startcol, anz);
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
		setpalette(_palTable);
	}
}

void McgaGraphics::abblenden(int16 startcol, int16 anz, int16 stufen, int16 frames) {
	int16 endcol = startcol + anz;
	for (int16 j = 0; j < stufen; j++) {
		int16 k = startcol * 3;
		for (int16 i = startcol; i < endcol; i++) {
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
		set_palpart(_palTable, startcol, anz);
	}
}

void McgaGraphics::set_teilpalette(const byte *palette, int16 startcol, int16 anz) {
	int16 k = startcol * 3;
	int16 endcol = startcol + anz;
	for (int16 i = startcol; i < endcol; i++) {
		_palTable[k] = palette[k];
		_palTable[k + 1] = palette[k + 1];
		_palTable[k + 2] = palette[k + 2];
		k += 3;
	}
	if (_mono)
		calc_mono(_palTable, startcol, anz);
	set_palpart(_palTable, startcol, anz);
}

void McgaGraphics::cls() {
	clear_mcga();
}

void McgaGraphics::punkt(int16 xpos, int16 ypos, int16 farbn) {
	setpixel_mcga(xpos, ypos, farbn);
}

uint8 McgaGraphics::get_pixel(int16 xpos, int16 ypos) {
	uint8 pix = getpix(xpos, ypos);

	return pix;
}

void McgaGraphics::linie(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	line_mcga(x1, y1, x2, y2, farbe);
}

void McgaGraphics::box(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	line_mcga(x1, y1, x2, y1, farbe);
	line_mcga(x1, y2 - 1, x2, y2 - 1, farbe);
	line_mcga(x1, y1, x1, y2, farbe);
	line_mcga(x2, y1, x2, y2, farbe);
}

void McgaGraphics::box_fill(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	if (x2 == x1)x2++;
	int16 h = abs(y2 - y1);
	if (h == 0)
		h = 1;
	for (int16 i = 0; i < h; i++)
		line_mcga(x1, y1 + i, x2, y1 + i, farbe);
}

void McgaGraphics::pop_box(int16 x, int16 y, int16 x1, int16 y1, int16 col1, int16 col2, int16 back_col) {
	if (back_col < 255)
		box_fill(x, y, x1, y1, back_col);
	linie(x, y1, x1, y1, col2);
	linie(x1, y, x1, y1, col2);
	linie(x, y, x1, y, col1);
	linie(x, y, x, y1 + 1, col1);
}

void McgaGraphics::kreis(int16 x, int16 y, int16 r, int16 farbe) {
	int16 b = 0, alt = 0;
	for (int16 w = 0; w <= 91; w++) {
		int16 a = (int16)(_sines[w] * ((float)r * 0.85));
		if ((a - alt) > 1) {
			int16 diff = a - alt;
			for (int16 i = 0; i <= diff; i++) {
				setpixel_mcga(x - b, (y - (alt + i)), farbe);
				setpixel_mcga(x + b, (y - (alt + i)), farbe);
				setpixel_mcga(x - b, (y + (alt + i)), farbe);
				setpixel_mcga(x + b, (y + (alt + i)), farbe);
			}
		}
		b = (int16)(_cosines[w] * (float)r);
		setpixel_mcga(x - b, y - a, farbe);
		setpixel_mcga(x + b, y - a, farbe);
		setpixel_mcga(x - b, y + a, farbe);
		setpixel_mcga(x + b, y + a, farbe);
		alt = a;
	}
}

void McgaGraphics::fkreis(int16 x, int16 y, int16 r, int16 farbe) {
	int16 b = 0, alt = 0, i = 0, diff;
	for (int16 w = 0; w <= 90; w++) {
		int16 a = (int16)(_sines[w] * ((float)r * 0.85));
		if ((a - alt) > 1) {
			diff = a - alt;
			for (i = 0; i < diff; i++) {
				line_mcga(x - b, (y - (alt + i)), x + b, (y - (alt + i)), farbe);
				line_mcga(x - b, (y + (alt + i)), x + b, (y + (alt + i)), farbe);
			}
		}
		b = (int16)(_cosines[w] * ((float)r));
		line_mcga(x - b, (y - (alt + i)), x + b, (y - (alt + i)), farbe);
		line_mcga(x - b, (y + (alt + i)), x + b, (y + (alt + i)), farbe);
		alt = a;
	}
}

void McgaGraphics::back2screen(byte *ptr) {
	mem2mcga(ptr);
}

void McgaGraphics::back2back(byte *ptr1, byte *ptr2) {
	mem2mem(ptr1, ptr2);
}

void McgaGraphics::back2back_maskiert(byte *ptr1, byte *ptr2, int16 maske) {
	mem2mem_masked(ptr1, ptr2, maske);
}

void McgaGraphics::screen2back(byte *ptr) {
	mcga2mem(ptr);
}

void McgaGraphics::sprite_save(byte *sptr, int16 x, int16 y, int16 breite, int16 hoehe, int16 scrwidth) {
	if (breite < 4)
		breite = 4;
	if (hoehe <= 0)
		hoehe = 1;
	if (x < _G(clipx1)) {
		x = _G(clipx1);
		breite -= (_G(clipx1) - x);
	}
	if ((x + breite) > _G(clipx2) + 1)
		breite = _G(clipx2) - x;
	if (y < _G(clipy1)) {
		y = _G(clipy1);
		hoehe -= (_G(clipy1) - y);
	}
	if ((y + hoehe) > _G(clipy2) + 1)
		hoehe = _G(clipy2) - y;
	if (breite < 1)
		breite = 0;
	if (hoehe <= 0)
		hoehe = 0;

	spr_save_mcga(sptr, x, y, breite, hoehe, scrwidth);
}

void McgaGraphics::sprite_set(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	mspr_set_mcga(sptr, x, y, scrwidth);
}

void McgaGraphics::blockcopy(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	spr_set_mcga(sptr, x, y, scrwidth);
}

void McgaGraphics::map_spr2screen(byte *sptr, int16 x, int16 y) {
	int16 br = ((int16 *)sptr)[0];
	int16 h = ((int16 *)sptr)[1];
	if ((br >= 320) || (h >= 200))
		map_spr_2screen(sptr, x, y);
}

void McgaGraphics::set_fontadr(byte *adr) {
	TffHeader *tff = (TffHeader *)adr;

	setfont(adr + sizeof(TffHeader), tff->width, tff->height,
	        tff->first, tff->last);
	_G(fvorx) = tff->width;
	_G(fvory) = 0;
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

	while (!ende) {
		zeichen = string[i];
		++i;

		if ((zeichen >= _G(fontFirst)) && (zeichen <= _G(fontLast)) && (zeichen != 127)) {
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
						plot_scan_cur((x + (disp_akt)*_G(fvorx)), _G(gcury), cur_col, 300, scrwidth, cursor_z);
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
							strcpy(zstring + stelle - 1, zstring + stelle);
							plot_scan_cur((x + disp_akt * _G(fvorx)), _G(gcury), bcol, bcol, scrwidth, cursor_z);
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

void McgaGraphics::printxy(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 scrwidth, const char *string, ...) {
	int16 i = 0, k = 0, l;
	char zstring[35];
	int16 diff;
	char *tempptr;
	va_list parptr;
	va_start(parptr, string);
	_crlfx = x;
	_crlfy = y + _fontH + 2;
	_G(gcurx) = x;
	_G(gcury) = y;
	i = 0;

	unsigned char nextChar;
	do {
		nextChar = (unsigned char)string[i];
		++i;
		if ((nextChar < 32) || (nextChar == 127)) {
			switch (nextChar) {
			case 8:
				_G(gcurx) -= _G(fvorx);
				_G(gcury) -= _G(fvory);
				putz(32, fgCol, bgCol, scrwidth);
				break;

			case 10:
				_G(gcury) = _crlfy;
				_G(gcurx) = _crlfx;
				_crlfx = _G(gcurx);
				_crlfy = _G(gcury) + _fontH + 2;
				break;

			case 13:
				_G(gcurx) = _crlfx;
				break;

			case 127 :
				putz(32, fgCol, bgCol, scrwidth);
				break;

			default :
				if (nextChar >= _G(fontFirst))
					putz(nextChar, fgCol, bgCol, scrwidth);
				break;
			}
		} else if ((nextChar >= _G(fontFirst)) && (nextChar <= _G(fontLast))) {
			if (nextChar != '%') {
				putz(nextChar, fgCol, bgCol, scrwidth);
				vors();
			} else {
				nextChar = string[i];
				int16 count = 0;
				if ((nextChar >= 0x30) && (nextChar <= 0x39))
					count = atoi(string + i);
				while ((nextChar >= 0x30) && (nextChar <= 0x39)) {
					++i;
					nextChar = string[i];
				}
				++i;
				switch (nextChar) {
				case '%':
					putz(nextChar, fgCol, bgCol, scrwidth);
					vors();
					break;

				case 'd':
				case 'u':
					if (nextChar == 'd') {
						int16 izahl = va_arg(parptr, int);
						itoa(izahl, zstring, 10);
					} else {
						uint16 uzahl = va_arg(parptr, unsigned int);
						itoa(uzahl, zstring, 10);
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, fgCol, bgCol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], fgCol, bgCol, scrwidth);
						vors();
						++k;
					}
					break;

				case 'l':
					if (string[i] != 'u') {
						long lzahl = va_arg(parptr, long);
						ltoa(lzahl, zstring, 10);
					} else {
						uint32 luzahl = va_arg(parptr, uint32);
						ultoa(luzahl, zstring, 10);
						++i;
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, fgCol, bgCol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], fgCol, bgCol, scrwidth);
						vors();
						++k;
					}
					break;

				case 's':
					tempptr = va_arg(parptr, char *);
					if (!count) {
						while (*tempptr != 0) {
							putz(*tempptr, fgCol, bgCol, scrwidth);
							++tempptr;
							vors();
						}
					} else {
						for (l = 0; l < count; l++) {
							putz(*tempptr, fgCol, bgCol, scrwidth);
							++tempptr;
							vors();
						}
					}
					break;

				default:
					break;
				}
			}
		}
	} while ((i < MAXSTRING) && (nextChar != 0));
	va_end(parptr);
}

void McgaGraphics::speed_printxy(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 scrwidth, const char *string) {
	char zeichen;
	_G(gcurx) = x;
	_G(gcury) = y;
	int16 i = 0;
	do {
		zeichen = string[i];
		++i;
		if ((zeichen >= _G(fontFirst)) && (zeichen <= _G(fontLast))) {
			putz(zeichen, fgCol, bgCol, scrwidth);
			vors();
		}
	} while ((i < MAXSTRING) && (zeichen != 0));
}

void McgaGraphics::print(int16 fgCol, int16 bgCol, int16 scrwidth, const char *string, ...) {
	int16 k = 0, l;
	char zeichen, zstring[35];
	char *tempptr;
	int16 diff;
	va_list parptr;
	va_start(parptr, string);
	_crlfx = _G(gcurx);
	_crlfy = _G(gcury) + _fontH + 2;
	int16 i = 0;
	do {
		zeichen = string[i];
		++i;
		if ((zeichen > 0 && zeichen < 32) || (zeichen == 127)) {
			switch (zeichen) {
			case 8:
				_G(gcurx) -= _G(fvorx);
				_G(gcury) -= _G(fvory);
				putz(32, fgCol, bgCol, scrwidth);
				break;

			case 10:
				_G(gcury) = _crlfy;
				_G(gcurx) = _crlfx;
				_crlfx = _G(gcurx);
				_crlfy = _G(gcury) + _fontH + 2;
				break;

			case 13:
				_G(gcurx) = _crlfx;
				break;

			case 127 :
				putz(32, fgCol, bgCol, scrwidth);
				break;

			default :
				if (zeichen >= _G(fontFirst)) {
					putz(zeichen, fgCol, bgCol, scrwidth);
					vors();
				}
				break;
			}
		} else if ((zeichen >= _G(fontFirst)) && (zeichen <= _G(fontLast)) && (zeichen != 0)) {
			if (zeichen != '%') {
				putz(zeichen, fgCol, bgCol, scrwidth);
				vors();
			} else {
				zeichen = string[i];
				int16 count = 0;
				if ((zeichen >= 0x30) && (zeichen <= 0x39))
					count = atoi(string + i);
				while ((zeichen >= 0x30) && (zeichen <= 0x39)) {
					++i;
					zeichen = string[i];
				}
				++i;
				switch (zeichen) {
				case '%':
					putz(zeichen, fgCol, bgCol, scrwidth);
					vors();
					break;

				case 'd':
				case 'u':
					if (zeichen == 'd') {
						int16 izahl = va_arg(parptr, int);
						itoa(izahl, zstring, 10);
					} else {
						uint16 uzahl = va_arg(parptr, unsigned int);
						itoa(uzahl, zstring, 10);
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, fgCol, bgCol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], fgCol, bgCol, scrwidth);
						vors();
						++k;
					}
					break;

				case 'l':
					if (string[i] != 'u') {
						long lzahl = va_arg(parptr, long);
						ltoa(lzahl, zstring, 10);
					} else {
						uint32 luzahl = va_arg(parptr, unsigned long);
						ultoa(luzahl, zstring, 10);
						++i;
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, fgCol, bgCol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], fgCol, bgCol, scrwidth);
						vors();
						++k;
					}
					break;

				case 's':
					tempptr = va_arg(parptr, char *);
					if (!count) {
						while (*tempptr != 0) {
							putz(*tempptr, fgCol, bgCol, scrwidth);
							++tempptr;
							vors();
						}
					} else {
						for (l = 0; l < count; l++) {
							putz(*tempptr, fgCol, bgCol, scrwidth);
							++tempptr;
							vors();
						}
					}

					break;

				default:
					break;
				}
			}
		}
	} while ((i < MAXSTRING) && (zeichen != 0));

	va_end(parptr);
}

void McgaGraphics::printnxy(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 menge,
                           int16 scrwidth, const char *string, ...) {
	int16 i = 0, k = 0, l;
	char zstring[35];
	char *tempptr;
	int16 diff;
	va_list parptr;
	va_start(parptr, string);
	_G(gcurx) = x;
	_G(gcury) = y;
	_crlfx = _G(gcurx);
	_crlfy = y + _fontH + 2;
	for (i = 0; i < menge;) {
		char zeichen = string[i];
		++i;
		if ((zeichen < 32) || (zeichen == 127)) {
			switch (zeichen) {
			case 8:
				_G(gcurx) -= _G(fvorx);
				_G(gcury) -= _G(fvory);
				putz(32, fgCol, bgCol, scrwidth);
				break;

			case 10:
				_G(gcury) = _crlfy;
				_G(gcurx) = _crlfx;
				_crlfx = _G(gcurx);
				_crlfy = _G(gcury) + _fontH + 2;
				break;

			case 13:
				_G(gcurx) = _crlfx;
				break;

			case 127 :
				putz(32, fgCol, bgCol, scrwidth);
				break;

			default :
				if (zeichen >= _G(fontFirst))
					putz(zeichen, fgCol, bgCol, scrwidth);
				break;
			}
		} else if ((zeichen >= _G(fontFirst)) && (zeichen <= _G(fontLast))) {
			if (zeichen != '%') {
				putz(zeichen, fgCol, bgCol, scrwidth);
				vors();
			} else {
				zeichen = string[i];
				int16 count = 0;
				if ((zeichen >= 0x30) && (zeichen <= 0x39))
					count = atoi(string + i);
				while ((zeichen >= 0x30) && (zeichen <= 0x39)) {
					++i;
					zeichen = string[i];
				}
				++i;
				switch (zeichen) {
				case '%':
					putz(zeichen, fgCol, bgCol, scrwidth);
					vors();
					break;

				case 'd':
				case 'u':
					if (zeichen == 'd') {
						int16 izahl = va_arg(parptr, int);
						itoa(izahl, zstring, 10);
					} else {
						uint16 uzahl = va_arg(parptr, unsigned int);
						itoa(uzahl, zstring, 10);
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, fgCol, bgCol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], fgCol, bgCol, scrwidth);
						vors();
						++k;
					}
					break;

				case 'l':
					if (string[i] != 'u') {
						long lzahl = va_arg(parptr, long);
						ltoa(lzahl, zstring, 10);
					} else {
						uint32 luzahl = va_arg(parptr, uint32);
						ultoa(luzahl, zstring, 10);
						++i;
					} k
					    = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, fgCol, bgCol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], fgCol, bgCol, scrwidth);
						vors();
						++k;
					}
					break;

				case 's':
					tempptr = va_arg(parptr, char *);
					if (!count) {
						while (*tempptr != 0) {
							putz(*tempptr, fgCol, bgCol, scrwidth);
							++tempptr;
							vors();
						}
					}
					else {
						for (l = 0; l < count; l++) {
							putz(*tempptr, fgCol, bgCol, scrwidth);
							++tempptr;
							vors();
						}
					}

					break;

				default:
					break;
				}
			}
		}
	}
	va_end(parptr);
}

void McgaGraphics::printcharxy(int16 x, int16 y, char zeichen, int16 fgCol, int16 bgCol, int16 scrwidth) {
	_crlfx = x;
	_crlfy = y + _fontH + 2;
	if ((zeichen < 32) || (zeichen == 127)) {
		switch (zeichen) {
		case 8:
			x -= _G(fvorx);
			y -= _G(fvory);
			putcxy(x, y, 32, fgCol, bgCol, scrwidth);
			break;

		case 10:
			_G(gcury) = _crlfy;
			_G(gcurx) = _crlfx;
			_crlfx = _G(gcurx);
			_crlfy = _G(gcury) + _fontH + 2;
			break;

		case 13:
			_G(gcurx) = _crlfx;
			_G(gcury) = _crlfy;
			break;

		case 127:
			putcxy(x, y, 32, fgCol, bgCol, scrwidth);
			break;

		default:
			break;
		}
	} else if ((zeichen >= _G(fontFirst)) && (zeichen <= _G(fontLast)) && (zeichen != 127)) {
		putcxy(x, y, zeichen, fgCol, bgCol, scrwidth);
		vors();
	}
}

void McgaGraphics::printchar(char zeichen, int16 fgCol, int16 bgCol, int16 scrwidth) {
	_crlfx = _G(gcurx);
	_crlfy = _G(gcury) + _fontH + 2;
	if ((zeichen < 32) || (zeichen == 127)) {
		switch (zeichen) {
		case 8:
			_G(gcurx) -= _G(fvorx);
			_G(gcury) -= _G(fvory);
			putz(32, fgCol, bgCol, scrwidth);
			break;

		case 10:
			_G(gcury) = _crlfy;
			_G(gcurx) = _crlfx;
			_crlfx = _G(gcurx);
			_crlfy = _G(gcury) + _fontH + 2;
			break;

		case 13:
			_G(gcurx) = _crlfx;
			_G(gcury) = _crlfy;
			break;

		case 127:
			putz(32, fgCol, bgCol, scrwidth);
			break;

		default:
			break;
		}
	} else if ((zeichen >= _G(fontFirst)) && (zeichen <= _G(fontLast)) && (zeichen != 127)) {
		putz(zeichen, fgCol, bgCol, scrwidth);
		vors();
	}
}

void McgaGraphics::set_vorschub(int16 x, int16 y) {
	if (_G(fvorx) != -255)
		_G(fvorx) = x;
	if (_G(fvory) != -255)
		_G(fvory) = y;
}

void McgaGraphics::get_fontinfo(int16 *vorx, int16 *vory, int16 *fntbr, int16 *fnth) {
	*vorx = _G(fvorx);
	*vory = _G(fvory);
	*fntbr = _fontBr;
	*fnth = _fontH;
}

void McgaGraphics::vorschub() {
	vors();
}

void McgaGraphics::move(int16 x, int16 y) {
	_G(gcurx) = x;
	_G(gcury) = y;
}

void McgaGraphics::init_mausmode(MouseInfo *mInfo) {
	_mausMode = true;
	_mInfo = mInfo;
}

void McgaGraphics::exit_mausmode() {
	_mausMode = false;
	_mInfo = nullptr;
}

int16 McgaGraphics::devices() {
	int16 i = 0;
	if (_mausMode != false) {
		// TODO: This is wrong. 'i' should be initialized to "button status" (BX after a call to Int33h/03
		if (i > 1) {
			i = 27;
		} else if (i == 1)
			i = 13;
		else
			i = 0;
	}

	return i;
}

int16 McgaGraphics::check_stellen_anz(char *zstring, int16 *pos, int16 stellen) {
	int16 diff;
	int16 k = 0;
	while (zstring[k] != 0)
		++k;

	if (k >= stellen) {
		while ((k) && (stellen)) {
			--k;
			--stellen;
		}
		diff = 0;
	} else {
		diff = stellen - k;
		k = 0;
	}
	*pos = k;

	return diff;
}

void McgaGraphics::scale_set(byte *sptr, int16 x, int16 y, int16 xdiff_, int16 ydiff_, int16 scrwidth) {
	if (xdiff_ || ydiff_)
		zoom_set(sptr, x, y, xdiff_, ydiff_, scrwidth);
	else
		mspr_set_mcga(sptr, x, y, scrwidth);
}

void McgaGraphics::update_screen() {
	if (_svga == ON)
		upd_scr();
}

void McgaGraphics::ltoa(long N, char *str, int base) {
	sprintf(str, "%ld", N);
}

void McgaGraphics::ultoa(uint32 N, char *str, int base) {
	sprintf(str, "%u", N);
}

void McgaGraphics::itoa(int N, char *str, int base) {
	sprintf(str, "%d", N);
}

} // namespace Chewy
