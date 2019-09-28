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

#include "glk/adrift/scare.h"
#include "glk/adrift/sxprotos.h"
#include "common/textconsole.h"

namespace Glk {
namespace Adrift {

/*
 * Structure for representing a fake game save/restore file.  Used to catch
 * a serialized gamestate, and return it later on restore.  For now we allow
 * only one of these to exist.
 */
struct sx_scr_stream_t {
	sc_byte *data;
	sc_int length;
	sc_bool is_open;
	sc_bool is_writable;
};
static sx_scr_stream_t scr_serialization_stream = {NULL, 0, FALSE, FALSE};


/*
 * file_open_file_callback()
 * file_read_file_callback()
 * file_write_file_callback()
 * file_close_file_callback()
 *
 * Fake a single gamestate save/restore file.  Used to satisfy requests from
 * the script to serialize and restore a gamestate.  Only one "file" can
 * exist, meaning that a script must restore a saved game before trying to
 * save another.
 */
void *file_open_file_callback(sc_bool is_save) {
	sx_scr_stream_t *const stream = &scr_serialization_stream;

	/* Detect any problems due to scripting limitations. */
	if (stream->is_open) {
		error("File open error: %s",
		                "stream is in use (script limitation)");
		return NULL;
	} else if (is_save && stream->data) {
		error("File open error: %s",
		                "stream has not been read (script limitation)");
		return NULL;
	}

	/*
	 * Set up the stream for the requested mode.  Act as if no such file if
	 * no data available for a read-only open.
	 */
	if (is_save) {
		stream->data = NULL;
		stream->length = 0;
	} else if (!stream->data)
		return NULL;

	stream->is_open = TRUE;
	stream->is_writable = is_save;
	return stream;
}

sc_int file_read_file_callback(void *opaque, sc_byte *buffer, sc_int length) {
	sx_scr_stream_t *const stream = (sx_scr_stream_t *)opaque;
	sc_int bytes;
	assert(opaque && buffer && length > 0);

	/* Detect any problems with the callback parameters. */
	if (stream != &scr_serialization_stream) {
		error("File read error: %s", "stream is invalid");
		return 0;
	} else if (!stream->is_open) {
		error("File read error: %s", "stream is not open");
		return 0;
	} else if (stream->is_writable) {
		error("File read error: %s", "stream is not open for read");
		return 0;
	}

	/* Read and remove the first block of data (or all if less than length). */
	bytes = (stream->length < length) ? stream->length : length;
	memcpy(buffer, stream->data, bytes);
	memmove(stream->data, stream->data + bytes, stream->length - bytes);
	stream->length -= bytes;
	return bytes;
}

void file_write_file_callback(void *opaque, const sc_byte *buffer, sc_int length) {
	sx_scr_stream_t *const stream = (sx_scr_stream_t *)opaque;
	assert(opaque && buffer && length > 0);

	/* Detect any problems with the callback parameters. */
	if (stream != &scr_serialization_stream) {
		error("File write error: %s", "stream is invalid");
		return;
	} else if (!stream->is_open) {
		error("File write error: %s", "stream is not open");
		return;
	} else if (!stream->is_writable) {
		error("File write error: %s", "stream is not open for write");
		return;
	}

	/* Reallocate, then add this block of data to the buffer. */
	stream->data = (sc_byte *)sx_realloc(stream->data, stream->length + length);
	memcpy(stream->data + stream->length, buffer, length);
	stream->length += length;
}

void file_close_file_callback(void *opaque) {
	sx_scr_stream_t *const stream = (sx_scr_stream_t *)opaque;
	assert(opaque);

	/* Detect any problems with the callback parameters. */
	if (stream != &scr_serialization_stream) {
		error("File close error: %s", "stream is invalid");
		return;
	} else if (!stream->is_open) {
		error("File close error: %s", "stream is not open");
		return;
	}

	/*
	 * If closing after a read, free allocations, and return the stream to
	 * its empty state; if after write, leave the data for the later read.
	 */
	if (!stream->is_writable) {
		sx_free(stream->data);
		stream->data = NULL;
		stream->length = 0;
	}
	stream->is_writable = FALSE;
	stream->is_open = FALSE;
}


/*
 * file_cleanup()
 *
 * Free any pending allocations and clean up on completion of a script.
 */
void file_cleanup(void) {
	sx_scr_stream_t *const stream = &scr_serialization_stream;

	sx_free(stream->data);
	stream->data = NULL;
	stream->length = 0;
	stream->is_writable = FALSE;
	stream->is_open = FALSE;
}

} // End of namespace Adrift
} // End of namespace Glk
