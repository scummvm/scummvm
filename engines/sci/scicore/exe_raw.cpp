/***************************************************************************
 exe_raw.c Copyright (C) 2005 Walter van Niftrik


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

	sci_free(handle);
}

exe_decompressor_t
exe_decompressor_raw = {
	"raw",
	raw_open,
	raw_read,
	raw_close
};
