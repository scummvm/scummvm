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

#include "sci/scicore/exe.h"
#include "sci/scicore/exe_dec.h"

extern exe_decompressor_t exe_decompressor_lzexe;
extern exe_decompressor_t exe_decompressor_raw;

exe_decompressor_t *exe_decompressors[] = {
	&exe_decompressor_lzexe,
	&exe_decompressor_raw,
	NULL
};

struct _exe_file {
	struct _exe_decompressor *decompressor;
	struct _exe_handle *handle;
};

exe_file_t *
exe_open(const char *filename) {
	int i = 0;
	exe_decompressor_t *dec;

	while ((dec = exe_decompressors[i])) {
		exe_handle_t *handle = dec->open(filename);

		if (handle) {
			exe_file_t *file = (exe_file_t*)sci_malloc(sizeof(exe_file_t));

			sciprintf("Scanning '%s' with decompressor '%s'\n",
			          filename, dec->name);

			file->handle = handle;
			file->decompressor = dec;
			return file;
		}

		i++;
	}

	return NULL;
}

int
exe_read(exe_file_t *file, void *buf, int count) {
	return file->decompressor->read(file->handle, buf, count);
}

void
exe_close(exe_file_t *file) {
	file->decompressor->close(file->handle);

	free(file);
}
