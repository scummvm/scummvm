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

#include "m4/graphics/gr_buff.h"
#include "m4/graphics/gr_pal.h"
#include "m4/gui/gui_vmng_core.h"
#include "m4/core/errors.h"
#include "m4/mem/memman.h"
#include "m4/core/term.h"

namespace M4 {

GrBuff::GrBuff(int32 _w, int32 _h) {
	w = _w;
	h = _h;
	x_off = y_off = 0;
	pitch = _w;
	height = _h;
	alloc_pixmap();
}

GrBuff::GrBuff(int32 _w, int32 _h, int32 _x_off, int32 _y_off, int32 _pitch, int32 _height) {
	w = _w;
	h = _h;
	x_off = _x_off;
	y_off = _y_off;
	pitch = _pitch;
	height = _height;
	alloc_pixmap();
}

GrBuff::~GrBuff() {
	if (pixmap)
		DisposeHandle(pixmap);
}

void GrBuff::lock() {
	if (!pixmap)
		return;

	HLock(pixmap);
}

void GrBuff::release() {
	if (pixmap)
		HUnLock(pixmap);
}

void GrBuff::alloc_pixmap() {
	pixmap = NewHandle(pitch * height, "pixmap");
	if (!pixmap) {
		term_message("GrBuff::alloc_pixmap(): Trying to free up %d bytes", pitch * height);
		if (MakeMem(pitch * height, "pixmap")) {
			pixmap = NewHandle(pitch * height, "pixmap");
			if (!pixmap)
				error_show(FL, 15, "pixmap h:%d w:%d bytes:%d", height, pitch, pitch * height);
		} else
			error_show(FL, 1, "GrBuff::alloc_pixmap() x, y: %d %d", pitch, height);
	}
	HLock(pixmap);
	memset(*pixmap, __BLACK, pitch * height);
	HUnLock(pixmap);
}

uint8 *GrBuff::get_pixmap() {
	if (pixmap) {
		lock();
		return (uint8 *)*pixmap;
	}
	return nullptr;
}

Buffer *GrBuff::get_buffer() {
	if (pixmap) {
		lock();
		dummy.data = (uint8 *)*pixmap;
		dummy.w = w;
		dummy.h = h;
		dummy.encoding = 0;
		dummy.stride = pitch;

		return &dummy;
	}
	return nullptr;
}

void GrBuff::refresh_video(int32 scrnX, int32 scrnY, int32 x1, int32 y1, int32 x2, int32 y2) {
	vmng_refresh_video(scrnX, scrnY, x1, y1, x2, y2, get_buffer());
}

int32 gr_buffer_free(Buffer *buf) {
	buf->w = buf->h = buf->stride = 0;

	if (buf->data != nullptr) {
		mem_free((char *)buf->data);
		buf->data = nullptr;
		return true;
	}

	error_show(FL, 'BUF!');
	return false;
}

byte *gr_buffer_pointer(Buffer *buf, int32 x, int32 y) {
	if (!buf || !buf->data || y < 0 || x < 0) {
		error_show(FL, 'BUF!', "buffer_pointer x,y = %d,%d", x, y);
		return 0;
	}

	return (byte *)(buf->data + x + (y * buf->stride));
}

const byte *gr_buffer_pointer(const Buffer *buf, int32 x, int32 y) {
	if (!buf || !buf->data || y < 0 || x < 0) {
		error_show(FL, 'BUF!', "buffer_pointer x,y = %d,%d", x, y);
		return 0;
	}

	return (byte *)(buf->data + x + (y * buf->stride));
}

int32 gr_buffer_init(Buffer *buf, const char *name, int32 w, int32 h) {
	if (buf->data)
		error_show(FL, 'BUFR', "buffer_init %s", name);

	buf->w = w;
	buf->h = h;
	buf->stride = w;

	buf->data = (uint8 *)mem_alloc(buf->stride * h, name);
	if (buf->data == nullptr)
		error_show(FL, 'OOM!', "buffer: %s - w:%d h:%d bytes:%d", name, buf->stride, h, buf->stride * h);

	memset(buf->data, 0, buf->stride * h);

	return (true);
}

bool gr_buffer_rect_copy_2(const Buffer *from, Buffer *to, int32 sx, int32 sy,
                           int32 dx, int32 dy, int32 w, int32 h) {
	// stupid check for no data
	if (!from || !to || !from->data || !to->data)
		error_show(FL, 'BUF!', "buff_rect_copy2");

	// CKB: if height is greater than source height, truncate!
	if (h > from->h)
		h = from->h;

	// if source x,y or dest x,y won't touch dest or source buffers, we're done
	if ((sx > from->w) || (sy > from->h) || (dx > to->w) || (dy > to->h))
		return true;

	// if dest request intersects dest buffer, clip dest request
	if ((dx + w) > to->w)
		w = to->w - dx;
	if ((dy + h) > to->h)
		h = to->h - dy;

	// if our dest request is too small, we're done
	if ((w < 1) || (h < 1))
		return true;

	// initialize pointers
	const byte *src = gr_buffer_pointer(from, sx, sy);
	byte *dest = gr_buffer_pointer(to, dx, dy);

	// get stride
	int32 sIncr = from->stride;
	int32 dIncr = to->stride;

	// copy one to d'other
	for (int i = 0; i < h; i++, dest += dIncr, src += sIncr)
		memmove(dest, src, w);

	return true;
}

bool gr_buffer_rect_copy(Buffer *from, Buffer *to, int32 x, int32 y, int32 w, int32 h) {
	return (gr_buffer_rect_copy_2(from, to, x, y, x, y, w, h));
}

int32 gr_buffer_rect_fill(Buffer *target, int32 x1, int32 y1, int32 w, int32 h) {
	int32 i;
	uint8 *start;
	byte color = gr_color_get_current();

	// if no data, bad.
	if (!target || !target->data)
		error_show(FL, 'BUF!', "buffer_rect_fill");

	// if nothing to fill, we're done
	if ((w < 1) || (h < 1) || (x1 > target->w) || (y1 > target->h))
		return true;

	// clip if rectangles too big
	if ((x1 + w) > target->w)
		w = target->w - x1;
	if ((y1 + h) > target->h)
		h = target->h - y1;

	if ((w < 1) || (h < 1))
		return true;

	start = target->data + y1 * target->stride + x1;
	for (i = 0; i < h; i++, start += target->stride)
		memset(start, color, w);

	return true;
}

} // namespace M4
