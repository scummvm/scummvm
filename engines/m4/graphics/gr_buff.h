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

#ifndef M4_GRAPHICS_GR_BUFF_H
#define M4_GRAPHICS_GR_BUFF_H

#include "m4/m4_types.h"
#include "m4/mem/reloc.h"

namespace M4 {

class GrBuff {
protected:
	void alloc_pixmap();

	Buffer dummy;
	MemHandle pixmap;
public:
	int32 w, h, x_off, y_off, pitch, height;

public:
	GrBuff(int32 _w, int32 _h);
	GrBuff(int32 _w, int32 _h, int32 _x_off, int32 _y_off, int32 _pitch, int32 _height);
	virtual ~GrBuff();

	/**
	 * Get buffer pointer
	 * @remarks		Get_buffer will return the whole pixmap, not the
	 * subrectangle specified via x_off, y_off, w, h, and pitch.
	 * get_buffer will lock the pixmap, unlock it after use, SVP.
	 * DO NOT FREE THE RETURNED BUFFER!
	 */
	Buffer *get_buffer();

	uint8 *get_pixmap();
	void clear();

	void lock();
	void release();
	void refresh_video(int32 scrnX, int32 scrnY, int32 x1, int32 y1, int32 x2, int32 y2);
};

int32 gr_buffer_free(Buffer *buf);
byte *gr_buffer_pointer(Buffer *buf, int32 x, int32 y);
const byte *gr_buffer_pointer(const Buffer *buf, int32 x, int32 y);
int32 gr_buffer_init(Buffer *buf, const char *name, int32 w, int32 h);

/**
 * Copies a "rectangular" buffer area from "from" to "unto".  Size
 * of copied rectangle is determined by "size_x, size_y".  Upper left
 * corner in source buffer is indicated by "from_x, from_y", and
 * "unto_x, unto_y" determines upper left corner in destination
 * buffer (if upper left corner coordinates are the same in both
 * buffers, buf_rect_copy() can be used instead).
 * @returns		Returns true if successful.
 */
bool gr_buffer_rect_copy_2(const Buffer *from, Buffer *to, int32 sx, int32 sy,
	int32 dx, int32 dy, int32 w, int32 h);

/**
 * Copies a "rectangular" buffer area from "from" to "unto".  Size
 * of copied rectangle is determined by "size_x, size_y".  Upper left
 * corner in BOTH buffers is indicated by "ul_x, ul_y".  (To copy
 * using separate corner coordinates in each buffer, use
 * buf_rect_copy_2 ().
 * @returns		Returns true if successful.
 */
bool gr_buffer_rect_copy(Buffer *from, Buffer *to, int32 x, int32 y, int32 w, int32 h);

/**
 * Fills a rectangular buffer area with the specified byte value.
 * Upper left corner is determined by "ul_x, ul_y", and sizes are
 * determined by "size_x, size_y."
 * @returns		Returns true if successful; false if buffer invalid.
 */
int32 gr_buffer_rect_fill(Buffer *target, int32 x1, int32 y1, int32 w, int32 h);

void GrBuff_Show(void *s, void *r, void *b, int32 destX, int32 destY);

} // namespace M4

#endif
