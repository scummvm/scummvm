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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "chewy/ngshext.h"
#include "chewy/effect.h"
#include "chewy/defines.h"
#include "chewy/global.h"

namespace Chewy {

effect::effect() {
}
effect::~effect() {
}

void effect::rnd_blende(byte *rnd_speicher, byte *sram_speicher,
		byte *screen, byte *palette, int16 col, int16 skip_line) {
	int16 *rnd_zeiger;
	int16 i, x, y;
	byte *sp;
	sp = (byte *)calloc(8 * 8 + 4, 1);
	if (!modul) {
		rnd_zeiger = (int16 *)rnd_speicher;
		if (col < 256) {
			for (i = 0; i < 1002; i++) {
				y = (rnd_zeiger[i] / 40) * 8;
				if (rnd_zeiger[i] > 39)
					x = (rnd_zeiger[i] - (40 * (y / 8))) * 8;
				else
					x = rnd_zeiger[i] * 8;
				out->skip_line(skip_line);
				out->box_fill(x, y, x + 8, y + 8, col);
			}
			out->set_palette(palette);
		}
		for (i = 0; i < 1000; i++) {
			y = (rnd_zeiger[i] / 40) * 8;
			if (rnd_zeiger[i] > 39)
				x = (rnd_zeiger[i] - (40 * (y / 8))) * 8;
			else
				x = rnd_zeiger[i] * 8;
			out->skip_line(skip_line);
			out->setze_zeiger(sram_speicher);
			out->sprite_save(sp, x, y, 8, 8, 0);
			out->setze_zeiger(screen);
			out->sprite_set(sp, x, y, 0);
		}
		free(sp);
	}
}

void effect::blende1(byte *sram_speicher, byte *screen,
		byte *palette, int16 frames, uint8 mode, int16 col) {

	int16 i;
	int16 x, y, x1, y1;
	byte *sp;
	sp = (byte *)calloc(8 * 8 + 4, 1);
	if (!modul) {
		i = 0;
		if (col < 256) {
			for (i = 0; i < 13; i++) {
				for (x = i; x < 39 - i; x++)
					out->box_fill(x * 8, i * 8, x * 8 + 8, i * 8 + 8, col);
				for (y = i; y < 24 - i; y++)
					out->box_fill(x * 8, y * 8, x * 8 + 8, y * 8 + 8, col);
				for (x1 = 39 - i; x1 > i; x1--)
					out->box_fill(x1 * 8, y * 8, x1 * 8 + 8, y * 8 + 8, col);
				for (y1 = 24 - i; y1 >= i; y1--)
					out->box_fill(x1 * 8, y1 * 8, x1 * 8 + 8, y1 * 8 + 8, col);
				out->skip_line(frames);
			}
			out->set_palette(palette);
		}
		switch ((int16)mode) {

		case 0:
			for (i = 13; i >= 0; i--) {
				for (x = i; x < 39 - i; x++) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x * 8, i * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x * 8, i * 8, 0);
				}
				for (y = i; y < 24 - i; y++) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x * 8, y * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x * 8, y * 8, 0);
				}
				for (x1 = 39 - i; x1 > i; x1--) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x1 * 8, y * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x1 * 8, y * 8, 0);
				}
				for (y1 = 24 - i; y1 > i; y1--) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x1 * 8, y1 * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x1 * 8, y1 * 8, 0);
				}
				out->skip_line(frames);
			}
			break;

		case 1:
			for (i = 0; i < 13; i++) {
				for (x = i; x < 39 - i; x++) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x * 8, i * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x * 8, i * 8, 0);
				}
				for (y = i; y < 24 - i; y++) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x * 8, y * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x * 8, y * 8, 0);
				}
				for (x1 = 39 - i; x1 > i; x1--) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x1 * 8, y * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x1 * 8, y * 8, 0);
				}
				for (y1 = 24 - i; y1 > i; y1--) {
					out->setze_zeiger(sram_speicher);
					out->sprite_save(sp, x1 * 8, y1 * 8, 8, 8, 0);
					out->setze_zeiger(screen);
					out->sprite_set(sp, x1 * 8, y1 * 8, 0);
				}
				out->skip_line(frames);
			}
			break;

		}
		free(sp);
	}
}

void effect::border(byte *workpage_, int16 lines, uint8 mode, int16 farbe) {
	if (mode) {
		for (int i = 0, x = 0; i < 20; ++i, x += 8) {
			out->setze_zeiger(workpage_ + 4);
			out->box_fill(152 - x, 0, 152 - x + 8, 200, farbe);
			out->box_fill(x + 160, 0, x + 168, 200, farbe);
			out->back2screen(workpage_);
			out->skip_line(lines);
		}
	} else {
		for (int i = 0, x = 0; i < 20; ++i, x += 8) {
			out->setze_zeiger(workpage_ + 4);
			out->box_fill(x, 0, x + 8, 200, farbe);
			out->box_fill(312 - x, 0, 31 - x + 8, 200, farbe);
			out->back2screen(workpage_);
			out->skip_line(lines);
		}
	}
}

void effect::spr_blende(byte *workpage_, int16 lines, bool mode, int16 col) {
	int i, y;

	if (mode) {
		for (i = 0, y = 0; i < 20; ++i, y += 8) {
			out->setze_zeiger(workpage_ + 4);
			out->box_fill(0, 92 - y, 320, 92 - y + 8, col);
			out->box_fill(0, 100 + y, 320, 108 + y, col);
			out->back2screen(workpage_);
			out->skip_line(lines);
		}
	} else {
		for (i = 0; i < 20; ++i) {
			out->setze_zeiger(workpage_ + 4);
			out->box_fill(0, i, 320, i + 8, col);
			out->box_fill(0, 192 - i, 320, 200 - i, col);
			out->back2screen(workpage_);
			out->skip_line(lines);
		}
	}
}

} // namespace Chewy
