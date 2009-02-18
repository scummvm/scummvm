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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/include/sci_memory.h"

struct _exe_handle {
	FILE *f;
};

#include "sci/scicore/exe_dec.h"

static exe_handle_t *
raw_open(const char *filename) {
	FILE *f = sci_fopen(filename, "rb");
	exe_handle_t *handle;

	if (!f)
		return NULL;

	handle = (exe_handle_t*)sci_malloc(sizeof(exe_handle_t));
	handle->f = f;

	return handle;
}

static int
raw_read(exe_handle_t *handle, void *buf, int count) {
	return fread(buf, 1, count, handle->f);
}

static void
raw_close(exe_handle_t *handle) {
	fclose(handle->f);

	free(handle);
}

exe_decompressor_t
exe_decompressor_raw = {
	"raw",
	raw_open,
	raw_read,
	raw_close
};
