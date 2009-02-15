/***************************************************************************
 exe.c Copyright (C) 2005 Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include <sci_memory.h>

#include "exe.h"
#include "exe_dec.h"

extern exe_decompressor_t exe_decompressor_lzexe;
extern exe_decompressor_t exe_decompressor_raw;

exe_decompressor_t *exe_decompressors[] = {
	&exe_decompressor_lzexe,
	&exe_decompressor_raw,
	NULL
};

struct _exe_file
{
	struct _exe_decompressor *decompressor;
	struct _exe_handle *handle;
};

exe_file_t *
exe_open(const char *filename)
{
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
exe_read(exe_file_t *file, void *buf, int count)
{
	return file->decompressor->read(file->handle, buf, count);
}

void
exe_close(exe_file_t *file)
{
	file->decompressor->close(file->handle);

	sci_free(file);
}
