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

#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/util.h"
#include "common/algorithm.h"
#include "common/file.h"

namespace Glk {
namespace Comprehend {

int file_buf_map_may_fail(const char *filename, struct file_buf *fb)
{
	Common::File f;

	memset(fb, 0, sizeof(*fb));

	if (!f.open(filename))
		return -1;

	fb->data = (uint8 *)xmalloc(f.size());
	fb->size = f.size();
	f.read(fb->data, fb->size);
	f.close();

	fb->p = fb->data;

	// FIXME - remove
	fb->marked = (uint8 *)malloc(fb->size);
	memset(fb->marked, 0, fb->size);

	return 0;
}

void file_buf_map(const char *filename, struct file_buf *fb)
{
	int err;

	err = file_buf_map_may_fail(filename, fb);
	if (err)
		fatal_strerror(errno, "Cannot open file '%s'", filename);
}

void file_buf_unmap(struct file_buf *fb)
{
	free(fb->marked);
	free(fb->data);
}

void file_buf_set_pos(struct file_buf *fb, unsigned pos)
{
	if (pos > fb->size)
		error("Bad position set in file (%x > %x)",
			    pos, fb->size);

	fb->p = fb->data + pos;
}

unsigned file_buf_get_pos(struct file_buf *fb)
{
	return fb->p - fb->data;
}

void *file_buf_data_pointer(struct file_buf *fb)
{
	return fb->p;
}

size_t file_buf_strlen(struct file_buf *fb, bool *eof)
{
	uint8 *end;

	if (eof)
		*eof = false;

	end = (uint8 *)memchr(fb->p, '\0', fb->size - file_buf_get_pos(fb));
	if (!end) {
		/* No null terminator - string is remaining length */
		if (eof)
			*eof = true;
		return fb->size - file_buf_get_pos(fb);
	}

	return end - fb->p;
}

void file_buf_get_data(struct file_buf *fb, void *data, size_t data_size)
{
	if (file_buf_get_pos(fb) + data_size > fb->size)
		error("Not enough data in file (%x + %x > %x)",
			    file_buf_get_pos(fb), data_size, fb->size);

	if (data)
		memcpy(data, fb->p, data_size);

	/* Mark this region of the file as read */
	memset(fb->marked + file_buf_get_pos(fb), '?', data_size);

	fb->p += data_size;
}

void file_buf_get_u8(struct file_buf *fb, uint8 *val)
{
	file_buf_get_data(fb, val, sizeof(*val));
}

void file_buf_get_le16(struct file_buf *fb, uint16 *val)
{
	file_buf_get_data(fb, val, sizeof(*val));
	*val = READ_LE_UINT16(val);
}

/*
 * Debugging function to show regions of a file that have not been read.
 */
void file_buf_show_unmarked(struct file_buf *fb)
{
	int i, start = -1;

	for (i = 0; i < (int)fb->size; i++) {
		if (!fb->marked[i] && start == -1)
			start = i;

		if ((fb->marked[i] || i == (int)fb->size - 1) && start != -1) {
			warning("%.4x - %.4x unmarked (%d bytes)\n", 
			       start, i - 1, i - start);
			start = -1;
		}
	}
}

void file_buf_put_u8(Common::WriteStream *fd, uint8 val)
{
	fd->writeByte(val);
}

void file_buf_put_le16(Common::WriteStream *fd, uint16 val) {
	fd->writeUint16LE(val);
}

void file_buf_put_skip(Common::WriteStream *fd, size_t skip) {
	for (uint i = 0; i < skip; i++)
		file_buf_put_u8(fd, 0);
}

} // namespace Comprehend
} // namespace Glk
