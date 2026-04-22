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

#include "common/system.h"
#include "common/savefile.h"
#include "common/util.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/room.h"

namespace MADS {
namespace MADSV2 {

word pattern_control_value = 0x181d;
word pattern_initial_value = 0xb78e;
int  auto_pattern = true;
int buffer_restore_keep_flag = false;

static word accum;                    // Pattern accumulator
static Buffer buffer_preserve_conventional;

char buffer_disk_filename[8] = "$MPOP.$";
byte buffer_accum = 0;
byte buffer_tracking[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


bool buffer_init(Buffer *buf, word x, word y) {
	buf->data = (byte *)mem_get((long)x * y);
	if (buf->data == NULL)
		return false;

	buf->x = x;
	buf->y = y;
	return true;
}

bool buffer_init_name(Buffer *buf, word x, word y, const char *block_name) {
	buf->data = (byte *)mem_get_name((long)x * y, block_name);
	if (buf->data == NULL)
		return false;

	buf->x = x;
	buf->y = y;
	return true;
}

bool buffer_free(Buffer *buf) {
	bool flag = false;

	if (buf->data != NULL) {
		mem_free(buf->data);
		flag = true;
	}

	buf->data = NULL;
	buf->x = 0;
	buf->y = 0;

	return flag;
}

bool buffer_fill(Buffer target, byte value) {
	return buffer_rect_fill(target, 0, 0, target.x, target.y, value);
}

bool buffer_rect_copy(Buffer from, Buffer unto,
	int ul_x, int ul_y, int size_x, int size_y) {
	bool result;

	result = ((from.data != NULL) && (unto.data != NULL));

	if (result && size_x > 0 && size_y > 0) {
		byte *from_ptr = buffer_pointer(&from, ul_x, ul_y);
		byte *unto_ptr = buffer_pointer(&unto, ul_x, ul_y);
		int   from_wrap = from.x - size_x;
		int   unto_wrap = unto.x - size_x;

		for (int row = 0; row < size_y; row++) {
			memcpy(unto_ptr, from_ptr, size_x);
			from_ptr += from_wrap + size_x;   // advance by full source stride
			unto_ptr += unto_wrap + size_x;   // advance by full dest stride
		}
	}

	return result;
}

bool buffer_rect_fill(Buffer target, int ul_x, int  ul_y, int  size_x, int  size_y, byte value) {
	bool result;

	if (buffer_conform(&target, &ul_x, &ul_y, &size_x, &size_y))
		return false;

	result = (target.data != NULL);

	if (result && size_x > 0 && size_y > 0) {
		byte *target_ptr = buffer_pointer(&target, ul_x, ul_y);
		int   target_wrap = target.x - size_x;

		for (int row = 0; row < size_y; row++) {
			memset(target_ptr, value, size_x);
			target_ptr += target_wrap + size_x;
		}
	}

	return result;
}

bool buffer_rect_copy_2(Buffer from, Buffer unto,
		int from_x, int from_y, int unto_x, int unto_y, int size_x, int size_y) {
	bool result;

	result = ((from.data != NULL) && (unto.data != NULL));

	if (result && size_x > 0 && size_y > 0)
	{
		byte *from_ptr = buffer_pointer(&from, from_x, from_y);
		byte *unto_ptr = buffer_pointer(&unto, unto_x, unto_y);
		int   from_wrap = from.x - size_x;
		int   unto_wrap = unto.x - size_x;

		for (int row = 0; row < size_y; row++)
		{
			memcpy(unto_ptr, from_ptr, size_x);
			from_ptr += from_wrap + size_x;
			unto_ptr += unto_wrap + size_x;
		}
	}

	return result;
}

void buffer_put_pixel(Buffer buf, word x, word y, byte c) {
	buf.data[y * buf.x + x] = c;
}

byte buffer_get_pixel(Buffer buf, word x, word y) {
	return buf.data[y * buf.x + x];
}

void buffer_hline(Buffer buf, word x1, word x2, word y, byte color) {
	byte *ptr = buf.data + (y * buf.x) + x1;
	int   count = x2 - x1 + 1;

	memset(ptr, color, count);
}

void buffer_vline(Buffer buf, word x, word y1, word y2, byte color) {
	byte *ptr = buf.data + (y1 * buf.x) + x;
	int   count = y2 - y1 + 1;

	for (int i = 0; i < count; i++, ptr += buf.x)
		*ptr = color;
}

void buffer_draw_box(Buffer buf, word x1, word y1, word x2, word y2, byte color) {
	int tmp;

	if (x1 > x2) {
		tmp = x1; x1 = x2; x2 = tmp;
	}
	if (y1 > y2) {
		tmp = y1; y1 = y2; y2 = tmp;
	}

	buffer_hline(buf, x1, x2, y1, color); buffer_hline(buf, x1, x2, y2, color);
	buffer_vline(buf, x1, y1, y2, color); buffer_vline(buf, x2, y1, y2, color);
}

void buffer_hline_xor(Buffer buf, int x1, int x2, int y) {
	if (x1 > x2)
	{
		int tmp = x1;
		x1 = x2;
		x2 = tmp;
	}

	int   len = x2 - x1 + 1;
	byte *ptr = buf.data + y * buf.x + x1;

	for (int i = 0; i < len; i++)
		ptr[i] ^= 0xFF;
}

void buffer_vline_xor(Buffer buf, int x, int y1, int y2) {
	if (y1 > y2)
	{
		int tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	int   len = y2 - y1 + 1;
	byte *ptr = buf.data + y1 * buf.x + x;

	for (int i = 0; i < len; i++, ptr += buf.x)
		*ptr ^= 0xFF;
}

void buffer_draw_crosshair(Buffer buf, int x, int y) {
	buffer_hline_xor(buf, 0, buf.x - 1, y);
	buffer_vline_xor(buf, x, 0, buf.y - 1);
}

void buffer_draw_box_xor(Buffer buf, int x1, int y1, int x2, int y2) {
	int tmp;

	if (x1 > x2) {
		tmp = x1; x1 = x2; x2 = tmp;
	}
	if (y1 > y2) {
		tmp = y1; y1 = y2; y2 = tmp;
	}

	buffer_hline_xor(buf, x1, x2, y1);
	buffer_hline_xor(buf, x1, x2, y2);
	buffer_vline_xor(buf, x1, y1 + 1, y2 - 1);
	buffer_vline_xor(buf, x2, y1 + 1, y2 - 1);
}

int buffer_get_delta_bounds(Buffer buf1, Buffer buf2, int newcol,
	word *xl, word *xh, word *yl, word *yh) {
	if (buf1.x != buf2.x || buf1.y != buf2.y)
		return -1;

	word xxl = buf1.x, xxh = 0;
	word yyl = buf1.y, yyh = 0;

	byte *p1 = buf1.data;
	byte *p2 = buf2.data;

	for (word runy = 0; runy < buf1.y; runy++) {
		for (word runx = 0; runx < buf1.x; runx++, p1++, p2++) {
			if (*p1 != *p2) {
				if (newcol == 0)
					*p1 = 0;

				if (runx > xxh) xxh = runx;
				if (runx < xxl) xxl = runx;
				if (runy > yyh) yyh = runy;
				if (runy < yyl) yyl = runy;
			}
		}
	}

	*xl = xxl;
	*xh = xxh;
	*yl = yyl;
	*yh = yyh;

	return 0;
}

byte *buffer_pointer(Buffer *buf, int x, int y) {
	return buf->data + y * buf->x + x;
}

bool buffer_conform(Buffer *buffer, int *x, int *y, int *xs, int *ys) {
	if (*x < 0)
	{
		*xs += *x;
		*x = 0;
	}
	if (*y < 0)
	{
		*ys += *y;
		*y = 0;
	}

	int temp_x = MIN(*x + *xs - 1, buffer->x - 1);
	int temp_y = MIN(*y + *ys - 1, buffer->y - 1);

	*xs = (temp_x - *x) + 1;
	*ys = (temp_y - *y) + 1;

	return ((*xs <= 0) || (*ys <= 0));
}

int buffer_inter_merge_2(Buffer from, Buffer unto,
	int from_x, int from_y,
	int unto_x, int unto_y,
	int size_x, int size_y) {
	int result;

	result = ((from.data != NULL) && (unto.data != NULL));

	if (result && size_x > 0 && size_y > 0) {
		byte *from_ptr = buffer_pointer(&from, from_x, from_y);
		byte *unto_ptr = buffer_pointer(&unto, unto_x, unto_y);
		int   from_wrap = from.x - size_x;
		int   unto_wrap = unto.x - size_x;

		for (int row = 0; row < size_y; row++) {
			for (int col = 0; col < size_x; col++) {
				byte dst = unto_ptr[col];
				if (dst >= 8 && dst <= 15)
					unto_ptr[col] = from_ptr[col];
			}

			from_ptr += from_wrap + size_x;
			unto_ptr += unto_wrap + size_x;
		}
	}

	return result;
}

void buffer_line(Buffer target, int x1, int y1, int x2, int y2, int color) {
	int delta_x = abs(x2 - x1);
	int delta_y = abs(y2 - y1);
	int x_inc = (x2 >= x1) ? 1 : -1;
	int y_inc = (y2 >= y1) ? target.x : -target.x;

	byte *ptr = target.data + y1 * target.x + x1;

	int x_count = delta_x + 1;
	int y_count = delta_y + 1;

	int lineAccum = (delta_x >= delta_y) ? delta_y : delta_x;

	for (int col = 0; col < x_count; col++)
	{
		lineAccum += y_count;
		*ptr = (byte)color;

		while (lineAccum >= x_count)
		{
			lineAccum -= x_count;
			*ptr = (byte)color;
			ptr += y_inc;
		}

		ptr += x_inc;
	}
}

void buffer_line_xor(Buffer target, int x1, int y1, int x2, int y2) {
	int delta_x = abs(x2 - x1);
	int delta_y = abs(y2 - y1);
	int x_inc = (x2 >= x1) ? 1 : -1;
	int y_inc = (y2 >= y1) ? target.x : -target.x;

	byte *ptr = target.data + y1 * target.x + x1;

	int x_count = delta_x + 1;
	int y_count = delta_y + 1;

	int lineAccum = (delta_x >= delta_y) ? delta_y : delta_x;

	for (int col = 0; col < x_count; col++) {
		lineAccum += y_count;
		bool written = false;
		*ptr ^= 0xFF;

		while (lineAccum >= x_count) {
			lineAccum -= x_count;
			if (!written) {
				*ptr ^= 0xFF;
				written = true;
			}
			ptr += y_inc;
		}

		ptr += x_inc;
	}
}

int buffer_legal(const Buffer &walk, int orig_wrap,
	int x1, int y1, int x2, int y2) {
	word legality = LEGAL;
	word currently_illegal = false;

	if (walk.data == NULL)                                return legality;
	if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0)          return legality;
	if (x1 >= orig_wrap || x2 >= orig_wrap)              return legality;
	if (y1 >= walk.y || y2 >= walk.y)                 return legality;

	int delta_y = y2 - y1;
	int y_sign = walk.x;
	if (delta_y < 0) {
		delta_y = -delta_y; y_sign = -y_sign;
	}

	int delta_x = x2 - x1;
	int x_sign = 1;
	int dAccum = 0;

	if (delta_x < 0) {
		delta_x = -delta_x;
		x_sign = -1;
		dAccum = MIN(delta_x, delta_y);
	}

	int x_count = delta_x + 1;
	int y_count = delta_y + 1;

	byte *ptr = walk.data + y1 * walk.x + (x1 / 8);
	uint bit_pos = 8 - (x1 % 8);  // cl: 1=MSB side, 8=LSB side

	for (int col = x_count; col > 0; col--) {
		dAccum += y_count;

		bool blocked = ((*ptr >> bit_pos) & 1) != 0;

		if (blocked) {
			if (!currently_illegal) {
				currently_illegal = true;
				legality -= ILLEGAL;
				if (legality == 0) return legality;
			}
		} else {
			currently_illegal = false;
		}

		while (dAccum >= x_count) {
			dAccum -= x_count;
			blocked = ((*ptr >> bit_pos) & 1) != 0;
			if (blocked) {
				if (!currently_illegal) {
					currently_illegal = true;
					legality -= ILLEGAL;
					if (legality == 0) return legality;
				}
			} else {
				currently_illegal = false;
			}

			ptr += y_sign;
		}

		// Advance one pixel in X
		uint new_cl = ((bit_pos - x_sign - 1) & 7) + 1;
		if ((bit_pos - x_sign - 1) & ~7)
			// ie. bit_pos < 0 or > 7
			ptr += x_sign;
		bit_pos = new_cl;
	}

	return legality;
}

/**
 * Advance the pattern accumulator one step.
 */
static void pattern_math(void) {
	word bx = accum;
	accum += pattern_control_value;
	bx = (bx >> 9) | (bx << 7);  // ror 9
	accum ^= bx;
	bx = (bx >> 3) | (bx << 13);  // ror 3
	accum += bx;
}

static void scale_coord(int *coord, int *size, int boundary, int less_than) {
	int condition;
	int difference;

	if (less_than) {
		difference = boundary - *coord;
		condition = *coord < boundary;
	} else {
		condition = *coord > (boundary - 1);
		difference = *coord - (boundary - 1);
	}

	if (condition) {
		*size = MAX(0, *size - difference);
		if (less_than)
			*coord += difference;
		else
			*coord -= difference;
	}
}

word buffer_rect_fill_pattern(Buffer target, int ul_x, int ul_y, int size_x, int size_y,
		int base_x, int base_y, int base_xs, byte value1, byte value2,
		word start_accum, word note_line) {
	int result;
	int target_wrap;
	int x2, base_x2;
	int y2;
	int base_ys = 0;
	int line_wrap;
	word noted_accum = 0;
	word line_count = 0;

	x2 = ul_x + size_x - 1;
	y2 = ul_y + size_y - 1;

	scale_coord(&ul_x, &size_x, 0, true);
	scale_coord(&x2, &size_x, target.x, false);
	scale_coord(&ul_y, &size_y, 0, true);
	scale_coord(&y2, &size_y, target.y, false);

	x2 = base_x + base_xs - 1;
	y2 = base_y + base_ys;

	scale_coord(&base_x, &base_xs, 0, true);
	scale_coord(&x2, &base_xs, target.x, false);
	scale_coord(&base_y, &base_ys, 0, true);
	scale_coord(&y2, &base_ys, target.y, false);

	if (!size_x || !size_y || !base_xs)
		goto done;

	if (auto_pattern) {
		pattern_control_value = 0x181d;
		pattern_initial_value = 0xb78e;

		if (((base_xs >= 71) && (base_xs <= 73)) ||
			((base_xs >= 83) && (base_xs <= 87)) ||
			((base_xs >= 107) && (base_xs <= 110)) ||
			((base_xs >= 139) && (base_xs <= 148)) ||
			((base_xs >= 171) && (base_xs <= 173)) ||
			((base_xs >= 212) && (base_xs <= 220)) ||
			(base_xs == 268)) {
			pattern_control_value = 0x6164;
			pattern_initial_value = 0xdd00;
		}

		if (((base_xs >= 105) && (base_xs <= 106)) ||
			((base_xs >= 120) && (base_xs <= 124))) {
			pattern_control_value = 0x5124;
			pattern_initial_value = 0xe1e4;
		}

		if (((base_xs >= 207) && (base_xs <= 211)) ||
			(base_xs == 184)) {
			pattern_control_value = 0x90d6;
			pattern_initial_value = 0x89e0;
		}

		if (((base_xs >= 256) && (base_xs <= 260)) ||
			((base_xs >= 280) && (base_xs <= 291)) ||
			(base_xs == 161)) {
			pattern_control_value = 0x6d90;
			pattern_initial_value = 0xecd7;
		}
	}

	target_wrap = target.x - size_x;
	result = (target.data != NULL);

	if (result) {
		accum = pattern_initial_value;

		// Wind the accumulator forward to account for rows above ul_y
		for (int row = 0; row < (ul_y - base_y); row++)
			for (int col = 0; col < base_xs; col++)
				pattern_math();

		// Wind the accumulator forward to account for columns left of ul_x
		for (int col = 0; col < (ul_x - base_x); col++)
			pattern_math();

		x2 = ul_x + size_x - 1;
		base_x2 = base_x + base_xs - 1;

		line_wrap = (base_x2 - x2) + (ul_x - base_x);

		if (start_accum)
			accum = start_accum;

		byte *target_ptr = buffer_pointer(&target, ul_x, ul_y);

		for (int row = 0; row < size_y; row++)
		{
			if (line_count == note_line)
				noted_accum = accum;

			line_count++;

			for (int col = 0; col < size_x; col++)
			{
				pattern_math();
				*target_ptr++ = (accum & 16) ? value2 : value1;
			}

			target_ptr += target_wrap;

			// Wind accumulator past the columns not being drawn
			for (int col = 0; col < line_wrap; col++)
				pattern_math();
		}
	}

done:
	return noted_accum;
}

bool buffer_rect_fill_swap(Buffer target,
	int  ul_x, int  ul_y,
	int  size_x, int size_y,
	byte value1, byte value2) {
	bool result = (target.data != NULL);

	if (result && size_x > 0 && size_y > 0) {
		byte *target_ptr = buffer_pointer(&target, ul_x, ul_y);
		int   target_wrap = target.x - size_x;

		for (int row = 0; row < size_y; row++)
		{
			for (int col = 0; col < size_x; col++)
			{
				if (target_ptr[col] == value1)
					target_ptr[col] = value2;
			}
			target_ptr += target_wrap + size_x;
		}
	}

	return result;
}

void buffer_peel_horiz(Buffer *target, int peel) {
	if (!peel) return;

	int peel_sign = (peel > 0) ? 1 : -1;
	int peel_val = abs(peel);

	if (peel_val > MAX_PEEL_VALUE) return;

	byte  temp_buf[MAX_PEEL_VALUE];
	byte *scan = target->data;
	int   x = target->x;
	int   y = target->y;

	for (int row = 0; row < y; row++)
	{
		if (peel_sign > 0) {
			// Save the first peel_val bytes, shift row left, append saved bytes at end
			memcpy(temp_buf, scan, peel_val);
			memmove(scan, scan + peel_val, x - peel_val);
			memcpy(scan + x - peel_val, temp_buf, peel_val);
		} else {
			// Save the last peel_val bytes, shift row right, prepend saved bytes at start
			memcpy(temp_buf, scan + x - peel_val, peel_val);
			memmove(scan + peel_val, scan, x - peel_val);
			memcpy(scan, temp_buf, peel_val);
		}

		scan += x;
	}
}

void buffer_peel_vert(Buffer *target, int peel, byte *work_memory, long work_size) {
	if (!peel)
		return;

	byte *scan = target->data;
	int x = target->x;
	int y = target->y;

	int peel_sign = (peel > 0) ? 1 : -1;
	int peel_val = abs(peel);
	int peel_memory = peel_val * x;

	byte *work_area = NULL;

	if (work_memory == NULL) {
		work_area = (byte *)mem_get_name(peel_memory, "$PEEL");
		if (work_area == NULL) return;
	} else {
		work_area = work_memory;
		if ((long)peel_memory > work_size) return;
	}

	if (peel_sign > 0) {
		// Positive peel: rotate buffer upward by peel_val rows.
		// Save the bottom peel_val rows, shift everything down,
		// then place the saved rows at the top.
		byte *deep_scan = buffer_pointer(target, 0, (y - 1) - (peel_val - 1));

		memcpy(work_area, deep_scan, peel_memory);

		for (int count = 0; count < (y - peel_val); count++)
		{
			byte *from = buffer_pointer(target, 0, (y - 1) - (count + peel_val));
			byte *unto = from + peel_memory;
			memcpy(unto, from, x);
		}

		memcpy(scan, work_area, peel_memory);
	} else {
		// Negative peel: rotate buffer downward by peel_val rows.
		// Save the top peel_val rows, shift everything up,
		// then place the saved rows at the bottom.
		byte *deep_scan = buffer_pointer(target, 0, (y - 1) - (peel_val - 1));

		memcpy(work_area, scan, peel_memory);

		byte *unto = scan;
		byte *from = scan + peel_memory;

		for (int count = 0; count < (y - peel_val); count++) {
			memcpy(unto, from, x);
			unto += x;
			from += x;
		}

		memcpy(deep_scan, work_area, peel_memory);
	}

	if (work_area != NULL && work_memory == NULL)
		mem_free(work_area);
}

int buffer_preserve(Buffer *source, int flags, int source_ems_handle, int x, int y, int xs, int ys) {
	int preserve_handle = BUFFER_NOT_PRESERVED;
	int disk_number;

	if (buffer_conform(source, &x, &y, &xs, &ys)) {
		goto done;
	}

	// Try to preserve in conventional memory, if requested
	if (flags == BUFFER_ATTEMPT_CONVENTIONAL) {
		buffer_init_name(&buffer_preserve_conventional, xs, ys, "$preserv");
		if (buffer_preserve_conventional.data != NULL) {
			buffer_rect_copy_2(*source, buffer_preserve_conventional,
				x, y, 0, 0, xs, ys);
			preserve_handle = BUFFER_PRESERVED_CONVENTIONAL;
			goto done;
		}
	}

	// Try to preserve in EMS memory
	preserve_handle = buffer_to_ems(source, flags, source_ems_handle, x, y, xs, ys);
	if (preserve_handle >= 0) {
		goto done;
	}

	// Try to preserve on disk
	if (flags != BUFFER_PRESERVE_RAM) {
		disk_number = buffer_to_disk(source, x, y, xs, ys);
		if (disk_number >= 0) {
			preserve_handle = BUFFER_PRESERVED_DISK - disk_number;
			goto done;
		}
	}

	preserve_handle = BUFFER_NOT_PRESERVED;

done:
	return (preserve_handle);
}

void buffer_restore(Buffer *source, int preserve_handle, int target_ems_handle, int x, int y, int xs, int ys) {
	if (buffer_conform(source, &x, &y, &xs, &ys)) {
		goto done;
	}

	switch (preserve_handle) {
	case BUFFER_PRESERVED_CONVENTIONAL:
		buffer_rect_copy_2(buffer_preserve_conventional, *source,
			0, 0, x, y, xs, ys);
		if (!buffer_restore_keep_flag) buffer_free(&buffer_preserve_conventional);
		break;

	case BUFFER_PRESERVED_DISK:
	case BUFFER_PRESERVED_DISK - 1:
	case BUFFER_PRESERVED_DISK - 2:
	case BUFFER_PRESERVED_DISK - 3:
	case BUFFER_PRESERVED_DISK - 4:
	case BUFFER_PRESERVED_DISK - 5:
	case BUFFER_PRESERVED_DISK - 6:
	case BUFFER_PRESERVED_DISK - 7:
	case BUFFER_PRESERVED_DISK - 8:
	case BUFFER_PRESERVED_DISK - 9:
		buffer_from_disk(source, neg(preserve_handle - BUFFER_PRESERVED_DISK), buffer_restore_keep_flag, x, y, xs, ys);
		break;

	case BUFFER_NOT_PRESERVED:
		break;

	default:
		if (buffer_restore_keep_flag) preserve_handle &= ~BUFFER_CREATED_PAGE_HANDLE;
		buffer_from_ems(source, preserve_handle, target_ems_handle, x, y, xs, ys);
		break;
	}

done:
	;
}

bool buffer_to_ems(Buffer *source, int page_handle, int source_ems_handle,
		int x, int y, int xs, int ys) {
	int special_page_handle = 0;
	Buffer ems_buffer = { video_y, video_x, NULL };

	if (page_handle < 0) {
		page_handle = ems_get_page_handle(4);
		special_page_handle = BUFFER_CREATED_PAGE_HANDLE;
	}
	if (page_handle < 0) {
		goto done;
	}

	if (source_ems_handle < 0) {
		ems_map_buffer(page_handle);
		ems_buffer.data = ems_page[0];

		buffer_rect_copy_2(*source, ems_buffer, x, y, x, y, xs, ys);
	} else {
		ems_buffer_to_buffer(source_ems_handle, page_handle);
	}

	page_handle |= special_page_handle;

	ems_unmap_all();

done:
	return page_handle;
}

bool buffer_from_ems(Buffer *source, int page_handle, int target_ems_handle,
		int x, int y, int xs, int ys) {
	Buffer ems_buffer = { video_y, video_x, NULL };
	int special_page_handle;

	special_page_handle = page_handle & BUFFER_CREATED_PAGE_HANDLE;

	page_handle &= ~(BUFFER_CREATED_PAGE_HANDLE);

	if (target_ems_handle < 0) {
		ems_map_buffer(page_handle);
		ems_buffer.data = ems_page[0];

		buffer_rect_copy_2(ems_buffer, *source, x, y, x, y, xs, ys);
	} else {
		ems_buffer_to_buffer(page_handle, target_ems_handle);
	}

	if (special_page_handle) ems_free_page_handle(page_handle);

	ems_unmap_all();

	return page_handle;
}

bool buffer_rect_translate(Buffer from, Buffer unto, int from_x, int from_y,
		int unto_x, int unto_y, int size_x, int size_y, byte *table) {
	int from_wrap, unto_wrap;
	int row_count;
	int result;
	byte *from_ptr;
	byte *unto_ptr;

	from_wrap = from.x - size_x;
	unto_wrap = unto.x - size_x;

	result = ((from.data != NULL) && (unto.data != NULL));
	if (result) {
		if (size_y == 0 || size_x == 0)
			return result;

		from_ptr = buffer_pointer(&from, from_x, from_y);
		unto_ptr = buffer_pointer(&unto, unto_x, unto_y);

		for (row_count = size_y; row_count > 0; row_count--) {
			for (int col = 0; col < size_x; col++) {
				*unto_ptr++ = table[*from_ptr++];
			}
			from_ptr += from_wrap;
			unto_ptr += unto_wrap;
		}
	}

	return result;
}

int buffer_to_disk(Buffer *source, int x, int y, int xs, int ys) {
	int buffer_id = -1;
	Common::OutSaveFile *handle = nullptr;
	byte *scan;
	int count;
	int error_count = 0;
	int a_ok = false;
	char file_name[40];

	do {
		buffer_accum = (byte)((buffer_accum + 1) % 10);

		Common::strcpy_s(file_name, buffer_disk_filename);
		env_catint(file_name, buffer_accum, 1);

		if (!buffer_tracking[buffer_accum]) {
			a_ok = true;
		} else {
			error_count++;
			if (error_count > 10) goto done;
		}
	} while (!a_ok);

	buffer_tracking[buffer_accum] = true;

	handle = g_system->getSavefileManager()->openForSaving(file_name, false);
	if (handle == nullptr)
		goto done;

	for (count = 0; count < ys; count++) {
		scan = buffer_pointer(source, x, y + count);
		if (!fileio_fwrite_f(scan, xs, 1, handle)) goto done;
	}

	buffer_id = buffer_accum;

done:
	delete handle;
	return buffer_id;
}

void buffer_from_disk(Buffer *source, int buffer_id, int keep_flag, int x, int y, int xs, int ys) {
	Common::InSaveFile *handle = nullptr;
	byte *scan;
	int count;
	char file_name[40];

	Common::strcpy_s(file_name, buffer_disk_filename);
	env_catint(file_name, buffer_id, 1);

	handle = g_system->getSavefileManager()->openForLoading(file_name);
	if (handle == NULL) goto done;

	for (count = 0; count < ys; count++) {
		scan = buffer_pointer(source, x, y + count);
		if (!fileio_fread_f(scan, xs, 1, handle)) goto done;
	}

done:
	if (handle != NULL) {
		delete handle;
		if (!keep_flag) {
			buffer_tracking[buffer_id] = 0;
			remove(file_name);
		}
	}
}

} // namespace MADSV2
} // namespace MADS
