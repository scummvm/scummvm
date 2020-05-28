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

#ifndef GLK_COMPREHEND_FILE_BUF_H
#define GLK_COMPREHEND_FILE_BUF_H

#include "common/scummsys.h"
#include "common/stream.h"

namespace Glk {
namespace Comprehend {

struct file_buf {
	uint8		*data;
	size_t		size;
	uint8		*p;

	uint8		*marked;
};

void file_buf_map(const char *filename, struct file_buf *fb);
int file_buf_map_may_fail(const char *filename, struct file_buf *fb);
void file_buf_unmap(struct file_buf *fb);
void file_buf_show_unmarked(struct file_buf *fb);

void *file_buf_data_pointer(struct file_buf *fb);
void file_buf_set_pos(struct file_buf *fb, unsigned pos);
unsigned file_buf_get_pos(struct file_buf *fb);

size_t file_buf_strlen(struct file_buf *fb, bool *eof);

void file_buf_get_data(struct file_buf *fb, void *data, size_t data_size);
void file_buf_get_u8(struct file_buf *fb, uint8 *val);
void file_buf_get_le16(struct file_buf *fb, uint16 *val);

#define file_buf_get_array(fb, type, base, array, member, size)		\
	do {								\
		uint __i;						\
		for (__i = (base); __i < (base) + (size); __i++)	\
			file_buf_get_##type(fb, &((array)[__i]).member); \
	} while (0)

#define file_buf_get_array_u8(fb, base, array, member, size) \
	file_buf_get_array(fb, u8, base, array, member, size)

#define file_buf_get_array_le16(fb, base, array, member, size) \
	file_buf_get_array(fb, le16, base, array, member, size)

void file_buf_put_skip(Common::WriteStream *fd, size_t skip);
void file_buf_put_u8(Common::WriteStream *fd, uint8 val);
void file_buf_put_le16(Common::WriteStream *fd, uint16 val);

#define file_buf_put_array(fd, type, base, array, member, size)		\
	do {								\
		int __i;						\
		for (__i = (base); __i < (base) + (size); __i++)	\
			file_buf_put_##type(fd, (array)[__i].member);	\
	} while (0)

#define file_buf_put_array_le16(fd, base, array, member, size) \
	file_buf_put_array(fd, le16, base, array, member, size)

#define file_buf_put_array_u8(fd, base, array, member, size) \
	file_buf_put_array(fd, u8, base, array, member, size)

} // namespace Comprehend
} // namespace Glk

#endif
