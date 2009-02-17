/***************************************************************************
 exe_lzexe.c Copyright (C) 2005 Walter van Niftrik


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

/* Based on public domain code by Mitugu Kurizono. */

#include "sci/include/sci_memory.h"
#include "sci/scicore/exe_dec.h"

/* Macro to interpret two sequential bytes as an unsigned integer. */
#define UINT16(A) ((*((A) + 1) << 8) + *(A))

/* The amount of most recent data (in bytes) that we need to keep in the
** buffer. lzexe compression is based on copying chunks of previous data to
** form new data.
*/
#define LZEXE_WINDOW 8192

/* Buffer size. */
#define LZEXE_BUFFER_SIZE (LZEXE_WINDOW + 4096)

/* Maximum amount of data (in bytes) that can be in the buffer at the start
** of the decompression loop. The maximum amount of data that can be added
** to the buffer during a single step of the loop is 256 bytes.
*/
#define LZEXE_BUFFER_MAX (LZEXE_BUFFER_SIZE - 256)

struct _exe_handle {
	FILE *f;

	/* Output buffer. */
	guint8 buffer[LZEXE_BUFFER_SIZE];
	guint8 *bufptr;

	/* Bit buffer. Bits [0..count) still contain unprocessed data. */
	int buf;
	int count;

	/* End of data flag. */
	int eod;
};

static int
lzexe_read_uint16(FILE *f, int *value) {
	int data;

	if ((*value = fgetc(f)) == EOF)
		return 0;

	if ((data = fgetc(f)) == EOF)
		return 0;

	*value |= data << 8;
	return 1;
}

static int
lzexe_read_uint8(FILE *f, int *value) {
	if ((*value = fgetc(f)) == EOF)
		return 0;

	return 1;
}

static int
lzexe_init(exe_handle_t *handle, FILE *f) {
	handle->f = f;
	handle->bufptr = handle->buffer;
	handle->eod = 0;

	if (!lzexe_read_uint16(handle->f, &handle->buf))
		return 0;

	handle->count = 16;
	return 1;
}

static int
lzexe_get_bit(exe_handle_t *handle, int *bit) {
	*bit = handle->buf & 1;

	if (--handle->count == 0) {
		if (!lzexe_read_uint16(handle->f, &handle->buf))
			return 0;
		handle->count = 16;
	} else
		handle->buf >>= 1;

	return 1;
}

static int
lzexe_decompress(exe_handle_t *handle) {
	while (!handle->eod
	        && handle->bufptr - handle->buffer <= LZEXE_BUFFER_MAX) {
		int bit;
		int len, span;

		if (!lzexe_get_bit(handle, &bit))
			return 0;

		if (bit) {
			/* 1: copy byte verbatim. */

			int data;

			if (!lzexe_read_uint8(handle->f, &data))
				return 0;

			*handle->bufptr++ = data;

			continue;
		}

		if (!lzexe_get_bit(handle, &bit))
			return 0;

		if (!bit) {
			/* 00: copy small block. */

			/* Next two bits indicate block length - 2. */
			if (!lzexe_get_bit(handle, &bit))
				return 0;

			len = bit << 1;

			if (!lzexe_get_bit(handle, &bit))
				return 0;

			len |= bit;
			len += 2;

			/* Read span byte. This forms the low byte of a
			** negative two's compliment value.
			*/
			if (!lzexe_read_uint8(handle->f, &span))
				return 0;

			/* Convert to negative integer. */
			span -= 256;
		} else {
			/* 01: copy large block. */
			int data;

			/* Read low byte of span value. */
			if (!lzexe_read_uint8(handle->f, &span))
				return 0;

			/* Read next byte. Bits [7..3] contain bits [12..8]
			** of span value. Bits [2..0] contain block length -
			** 2.
			*/
			if (!lzexe_read_uint8(handle->f, &data))
				return 0;
			span |= (data & 0xf8) << 5;
			/* Convert to negative integer. */
			span -= 8192;

			len = (data & 7) + 2;

			if (len == 2) {
				/* Next byte is block length value - 1. */
				if (!lzexe_read_uint8(handle->f, &len))
					return 0;

				if (len == 0) {
					/* End of data reached. */
					handle->eod = 1;
					break;
				}

				if (len == 1)
					/* Segment change marker. */
					continue;

				len++;
			}
		}

		assert(handle->bufptr + span >= handle->buffer);

		/* Copy block. */
		while (len-- > 0) {
			*handle->bufptr = *(handle->bufptr + span);
			handle->bufptr++;
		}
	}

	return 1;
}

static exe_handle_t *
lzexe_open(const char *filename) {
	exe_handle_t *handle;
	guint8 head[0x20];
	guint8 size[2];
	off_t fpos;

	FILE *f = sci_fopen(filename, "rb");

	if (!f)
		return NULL;

	/* Read exe header plus possible lzexe signature. */
	if (fread(head, 1, 0x20, f) != 0x20)
		return NULL;

	/* Verify "MZ" signature, header size == 2 paragraphs and number of
	** overlays == 0.
	*/
	if (UINT16(head) != 0x5a4d || UINT16(head + 8) != 2
	        || UINT16(head + 0x1a) != 0)
		return NULL;

	/* Verify that first relocation item offset is 0x1c. */
	if (UINT16(head + 0x18) != 0x1c)
		return NULL;

	/* Look for lzexe signature. */
	if (memcmp(head + 0x1c, "LZ09", 4)
	        && memcmp(head + 0x1c, "LZ91", 4)) {
		return NULL;
	}

	/* Calculate code segment offset in exe file. */
	fpos = (UINT16(head + 0x16) + UINT16(head + 8)) << 4;
	/* Seek to offset 8 of info table at start of code segment. */
	if (fseek(f, fpos + 8, SEEK_SET) == -1)
		return NULL;

	/* Read size of compressed data in paragraphs. */
	if (fread(size, 1, 2, f) != 2)
		return NULL;

	/* Move file pointer to start of compressed data. */
	fpos -= UINT16(size) << 4;
	if (fseek(f, fpos, SEEK_SET) == -1)
		return NULL;

	handle = (exe_handle_t*)sci_malloc(sizeof(exe_handle_t));

	if (!lzexe_init(handle, f)) {
		free(handle);
		return NULL;
	}

	return handle;
}

static int
lzexe_read(exe_handle_t *handle, void *buf, int count) {
	int done = 0;

	while (done != count) {
		int size, copy, i;
		int left = count - done;

		if (!lzexe_decompress(handle))
			return done;

		/* Total amount of bytes in buffer. */
		size = handle->bufptr - handle->buffer;

		/* If we're not at end of data we need to maintain the
		** window.
		*/
		if (!handle->eod)
			copy = size - LZEXE_WINDOW;
		else {
			if (size == 0)
				/* No data left. */
				return done;

			copy = size;
		}

		/* Do not copy more than requested. */
		if (copy > left)
			copy = left;

		memcpy((char *) buf + done, handle->buffer, copy);

		/* Move remaining data to start of buffer. */
		for (i = copy; i < size; i++)
			handle->buffer[i - copy] = handle->buffer[i];

		handle->bufptr -= copy;
		done += copy;
	}

	return done;
}

static void
lzexe_close(exe_handle_t *handle) {
	fclose(handle->f);

	free(handle);
}

exe_decompressor_t
exe_decompressor_lzexe = {
	"lzexe",
	lzexe_open,
	lzexe_read,
	lzexe_close
};
