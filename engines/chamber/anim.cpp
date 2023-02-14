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

#include "common/system.h"

#include "chamber/chamber.h"

#include "chamber/common.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"
#include "chamber/room.h"
#include "chamber/sound.h"

namespace Chamber {


byte *anima_end_ofs;

byte last_anim_y = 0;
byte last_anim_x = 0;
byte anim_shift_y = 0;
byte anim_shift_x = 0;
byte last_anim_height;
byte last_anim_width;
byte anim_cycle;
byte anim_flags;
byte anim_use_dot_effect;
uint16 anim_draw_delay;
byte dot_effect_step;
uint16 dot_effect_delay;

extern uint16 cpu_speed_delay;

extern byte *seekToEntry(byte *bank, uint16 num, byte **end);
extern void loadLutinSprite(uint16 lutidx);

void getScratchBuffer(byte mode) {
	byte *buffer = scratch_mem2;
	uint16 offs = 0;
	if (mode & 0x80)
		offs += 3200;
	if (mode & 0x40)
		offs += 1600;
	lutin_mem = buffer + offs;
}

void animLoadSprite(byte **panim) {
	byte mode;
	byte index;
	mode = *((*panim)++);
	index = *((*panim)++);
	getScratchBuffer(mode);
	loadLutinSprite(index);
}

void clipSprite(byte *x, byte *y, byte *sprw, byte *sprh, byte **sprite, int8 dx, int8 dy) {
	if (anim_flags == 7)
		return;
	if (anim_flags & 4) {
		if (anim_cycle == 0)
			return;
		if (anim_flags & 2) {
			*sprh = anim_cycle;
			if (anim_cycle >= dy)
				anim_cycle -= dy;
			else
				anim_cycle = 0;
		} else if (anim_flags & 1) {
			*sprw = anim_cycle;
			anim_cycle--;
		} else {
			*x -= dx;
			*sprite += (*sprw - anim_cycle) * 2;
			*sprw = anim_cycle;
			anim_cycle--;
		}
	} else if (anim_flags & 2) {
		if (*sprw == anim_cycle) {
			anim_cycle = 0;
		} else if (anim_flags & 1) {
			*sprw = anim_cycle;
			anim_cycle++;
		} else {
			*x -= dx;
			*sprite += (*sprw - anim_cycle) * 2;
			*sprw = anim_cycle;
			anim_cycle++;
		}
	}
}

void copyScreenBlockWithDotEffect(byte *source, byte x, byte y, byte width, byte height, byte *target) {
	uint16 offs;
	uint16 xx = x * 4;
	uint16 ww = width * 4;
	uint16 cur_image_end = ww * height;

	for (offs = 0; offs != cur_image_end;) {
		byte mask = 0xC0 >> (((xx + offs % ww) % 4) * 2);
		uint16 ofs = cga_CalcXY(xx + offs % ww, y + offs / ww);

		target[ofs] = (target[ofs] & ~mask) | (source[ofs] & mask);

		if (dot_effect_delay / 4 != 0) {
			uint16 i;
			for (i = 0; i < dot_effect_delay / 4; i++) ; /*TODO: weak delay*/
		}

		offs += dot_effect_step;
		if (offs > cur_image_end)
			offs -= cur_image_end;
	}
}

void animDrawSprite(byte x, byte y, byte sprw, byte sprh, byte *pixels, uint16 pitch) {
	uint16 delay;
	byte ex, ey, updx, updy, updw, updh;
	uint16 ofs = cga_CalcXY_p(x, y);
	cga_BackupImage(backbuffer, ofs, sprw, sprh, sprit_load_buffer);
	cga_BlitSprite(pixels, pitch, sprw, sprh, backbuffer, ofs);
	ex = x + sprw;
	ey = y + sprh;
	if (last_anim_height != 0) {
		if (last_anim_x + last_anim_width > ex)
			ex = last_anim_x + last_anim_width;

		if (last_anim_y + last_anim_height > ey)
			ey = last_anim_y + last_anim_height;

		updx = (x > last_anim_x) ? last_anim_x : x;
		updy = (y > last_anim_y) ? last_anim_y : y;
	} else {
		updx = x;
		updy = y;
	}
	updw = ex - updx;
	updh = ey - updy;
	ofs = cga_CalcXY_p(updx, updy);
	/*TODO looks like here was some code before*/
	for (delay = 0; delay < anim_draw_delay; delay++) {
		g_system->delayMillis(1000 / 16 / 25);
	}
	waitVBlank();

	if (anim_use_dot_effect)
		copyScreenBlockWithDotEffect(backbuffer, updx, updy, updw, updh, frontbuffer);
	else {
		cga_CopyScreenBlock(backbuffer, updw, updh, frontbuffer, ofs);
	}
	cga_RestoreImage(sprit_load_buffer, backbuffer);

	last_anim_x = x;
	last_anim_y = y;
	last_anim_width = sprw;
	last_anim_height = sprh;

	anim_shift_x = anim_shift_y = 0;
}

void animUndrawSprite(void) {
	cga_CopyScreenBlock(backbuffer, last_anim_width, last_anim_height, CGA_SCREENBUFFER, cga_CalcXY_p(last_anim_x, last_anim_y));
	last_anim_height = 0;
}

void playAnimCore(byte **panim) {
	byte mode;
	uint16 count, count2;
	byte *pframe;
	mode = *((*panim)++);
	anim_flags = mode & 7;
	count = mode >> 3;

	while (count--) {
		pframe = *panim;
		mode = *pframe++;
		anim_draw_delay = ((mode & ~7) >> 3) << 1;
		dot_effect_step = (mode & ~7) >> 3;
		dot_effect_delay = 500;
		count2 = mode & 7;
		while (count2--) {
			byte *sprite;
			byte sprw, sprh;
			byte x, y;
			int8 dx, dy;
			uint16 pitch;
			mode = *pframe++;
			getScratchBuffer(mode);
			dy = mode & 7;
			dx = (mode >> 3) & 7;

			dx = (dx & 1) ? -(dx & ~1) : dx;
			dx /= 2;
			dy = (dy & 1) ? -(dy & ~1) : dy;

			x = last_anim_x + dx + anim_shift_x;
			y = last_anim_y + dy + anim_shift_y;

			sprite = lutin_mem;
			sprw = *sprite++;
			sprh = *sprite++;

			pitch = sprw * 2;
			clipSprite(&x, &y, &sprw, &sprh, &sprite, dx, dy);
			animDrawSprite(x, y, sprw, sprh, sprite, pitch);

			if (anim_flags & 4) {
				if (anim_cycle == 0) {
					animUndrawSprite();
					goto end;
				}
			} else if (anim_flags & 2) {
				if (anim_cycle == 0) {
					goto end;
				}
			}
		}
	}
end:
	;
	mode = *((*panim)++);
	*panim += mode & 7;
}

void anim1(byte **panim) {
	anim_cycle = 0xFF;
	anim_use_dot_effect = 0;
	playAnimCore(panim);
}

void anim2(byte **panim) {
	anim_cycle = 1;
	anim_use_dot_effect = 0;
	playAnimCore(panim);
}

void anim3(byte **panim) {
	anim_cycle = 1;
	anim_use_dot_effect = 0;
	playAnimCore(panim);
}

void anim4(byte **panim) {
	anim_cycle = last_anim_width - 1;
	anim_use_dot_effect = 0;
	playAnimCore(panim);
}

void anim5(byte **panim) {
	anim_cycle = last_anim_width - 1;
	anim_use_dot_effect = 0;
	playAnimCore(panim);
}

void anim6(byte **panim) {
	anim_cycle = last_anim_height;
	anim_use_dot_effect = 0;
	playAnimCore(panim);
}

void anim7(byte **panim) {
	anim_cycle = 0xFF;
	anim_use_dot_effect = 1;
	playAnimCore(panim);
}

typedef void (*animhandler_t)(byte **panim);

animhandler_t anim_handlers[] = {
	animLoadSprite,
	anim1,
	anim2,
	anim3,
	anim4,
	anim5,
	anim6,
	anim7
};

void playAnim(byte index, byte x, byte y) {
	byte sound;
	byte *panim;

	last_anim_width = 0;
	last_anim_height = 0;
	last_anim_x = x;
	last_anim_y = y;

	panim = seekToEntry(anima_data, index - 1, &anima_end_ofs);
	while (panim != anima_end_ofs) {
		byte mode = *panim;
		switch (mode) {
		case 0xFE:  /*set shift*/
			panim++;
			anim_shift_x = *panim++;
			anim_shift_y = *panim++;
			break;
		case 0xFD:  /*play sfx*/
			panim++;
			sound = *panim++;
			panim++;    /*unused*/
			playSound(sound);
			break;
		case 0xFC:  /*nothing*/
			panim++;
			break;
		default:
			anim_handlers[mode & 7](&panim);
		}
	}
}

} // End of namespace Chamber
