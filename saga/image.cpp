/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    SAGA Image resource management routines

 Notes: 
*/

#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "game_mod.h"

/*
 * Begin module
\*--------------------------------------------------------------------------*/
#include "image_mod.h"
#include "image.h"

namespace Saga {

static int granulate(int value, int granularity)
{

	int remainder;

	if (value == 0)
		return 0;

	if (granularity == 0)
		return 0;

	remainder = value % granularity;

	if (remainder == 0) {
		return value;
	} else {
		return (granularity - remainder + value);
	}

}

int
IMG_DecodeBGImage(const uchar * image_data,
    size_t image_size,
    uchar ** output_buf, size_t * output_buf_len, int *w, int *h)
{

	R_IMAGE_HEADER hdr;
	int modex_height;

	const uchar *RLE_data_ptr;
	size_t RLE_data_len;

	uchar *decode_buf;
	size_t decode_buf_len;

	uchar *out_buf;
	size_t out_buf_len;

	const uchar *read_p = image_data;

	if (image_size <= SAGA_IMAGE_DATA_OFFSET) {
		/* Image size is way too small */
		return R_FAILURE;
	}

	hdr.width = ys_read_u16_le(read_p, &read_p);
	hdr.height = ys_read_u16_le(read_p, &read_p);
	hdr.unknown4 = ys_read_u16_le(read_p, &read_p);
	hdr.unknown6 = ys_read_u16_le(read_p, &read_p);

	RLE_data_ptr = image_data + SAGA_IMAGE_DATA_OFFSET;
	RLE_data_len = image_size - SAGA_IMAGE_DATA_OFFSET;

	modex_height = granulate(hdr.height, 4);

	decode_buf_len = hdr.width * modex_height;
	decode_buf = (uchar *)malloc(decode_buf_len);

	out_buf_len = hdr.width * hdr.height;
	out_buf = (uchar *)malloc(out_buf_len);

	if (DecodeBGImageRLE(RLE_data_ptr,
		RLE_data_len, decode_buf, decode_buf_len) != R_SUCCESS) {

		free(decode_buf);
		free(out_buf);

		return R_FAILURE;
	}

	UnbankBGImage(out_buf, decode_buf, hdr.width, hdr.height);

	/* For some reason bg images in IHNM are upside down
	 * \*------------------------------------------------------------- */
	if (GAME_GetGameType() == R_GAMETYPE_IHNM) {

		FlipImage(out_buf, hdr.width, hdr.height);
	}

	free(decode_buf);

	*output_buf_len = out_buf_len;
	*output_buf = out_buf;

	*w = hdr.width;
	*h = hdr.height;

	return R_SUCCESS;
}

int
DecodeBGImageRLE(const uchar * inbuf,
    size_t inbuf_len, uchar * outbuf, size_t outbuf_len)
{

	const uchar *inbuf_ptr;
	uchar *outbuf_ptr;
	size_t inbuf_remain;

	const uchar *inbuf_end;
	uchar *outbuf_end;
	size_t outbuf_remain;

	uchar mark_byte;
	int test_byte;

	uint runcount;

	uchar bitfield;
	uchar bitfield_byte1;
	uchar bitfield_byte2;

	uchar *backtrack_ptr;
	int backtrack_amount;

	uint c, b;

	int decode_err = 0;

	inbuf_ptr = inbuf;
	inbuf_remain = inbuf_len;

	outbuf_ptr = outbuf;
	outbuf_remain = outbuf_len;

	inbuf_end = (inbuf + inbuf_len) - 1;
	outbuf_end = (outbuf + outbuf_len) - 1;

	memset(outbuf, 0, outbuf_len);

	while ((inbuf_remain > 1) && (outbuf_remain > 0) && !decode_err) {

		if ((inbuf_ptr > inbuf_end) || (outbuf_ptr > outbuf_end)) {
			return R_FAILURE;
		}

		mark_byte = *inbuf_ptr++;
		inbuf_remain--;

		test_byte = mark_byte & 0xC0;	/* Mask all but two high order bits */

		switch (test_byte) {

		case 0xC0:	/* 1100 0000 */

			/* Uncompressed run follows: Max runlength 63 */
			runcount = mark_byte & 0x3f;

			if ((inbuf_remain < runcount) ||
			    (outbuf_remain < runcount)) {
				return R_FAILURE;
			}

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr++;
			}

			inbuf_remain -= runcount;
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x80:	/* 1000 0000 */

			/* Compressed run follows: Max runlength 63 */
			runcount = (mark_byte & 0x3f) + 3;

			if (!inbuf_remain || (outbuf_remain < runcount)) {
				return R_FAILURE;
			}

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr;
			}

			inbuf_ptr++;
			inbuf_remain--;
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x40:	/* 0100 0000 */

			/* Repeat decoded sequence from output stream: 
			 * Max runlength 10 */

			runcount = ((mark_byte >> 3) & 0x07U) + 3;

			backtrack_amount = *inbuf_ptr;

			if (!inbuf_remain ||
			    (backtrack_amount > (outbuf_ptr - outbuf)) ||
			    (runcount > outbuf_remain)) {
				return R_FAILURE;
			}

			inbuf_ptr++;
			inbuf_remain--;

			backtrack_ptr = outbuf_ptr - backtrack_amount;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *backtrack_ptr++;
			}

			outbuf_remain -= runcount;
			continue;

			break;

		default:	/* 0000 0000 */
			break;
		}

		/* Mask all but the third and fourth highest order bits */
		test_byte = mark_byte & 0x30;

		switch (test_byte) {

		case 0x30:	/* 0011 0000 */
			/* Bitfield compression */

			runcount = (mark_byte & 0x0F) + 1;

			if ((inbuf_remain < (runcount + 2)) ||
			    (outbuf_remain < (runcount * 8))) {
				return R_FAILURE;
			}

			bitfield_byte1 = *inbuf_ptr++;
			bitfield_byte2 = *inbuf_ptr++;

			for (c = 0; c < runcount; c++) {

				bitfield = *inbuf_ptr;

				for (b = 0; b < 8; b++) {

					if (bitfield & 0x80) {
						*outbuf_ptr = bitfield_byte2;
					} else {
						*outbuf_ptr = bitfield_byte1;
					}

					bitfield <<= 1;

					outbuf_ptr++;
				}
				inbuf_ptr++;
			}

			inbuf_remain -= (runcount + 2);
			outbuf_remain -= (runcount * 8);
			continue;

			break;

		case 0x20:	/* 0010 0000 */
			/* Uncompressed run follows */

			runcount = ((mark_byte & 0x0F) << 8) + *inbuf_ptr;

			if ((inbuf_remain < (runcount + 1)) ||
			    (outbuf_remain < runcount)) {
				return R_FAILURE;
			}

			inbuf_ptr++;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr++;
			}

			inbuf_remain -= (runcount + 1);
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x10:	/* 0001 0000 */
			/* Repeat decoded sequence from output stream */

			backtrack_amount =
			    ((mark_byte & 0x0F) << 8) + *inbuf_ptr;

			if (inbuf_remain < 2) {
				return R_FAILURE;
			}

			inbuf_ptr++;
			runcount = *inbuf_ptr++;

			if ((backtrack_amount > (outbuf_ptr - outbuf)) ||
			    (outbuf_remain < runcount)) {
				return R_FAILURE;
			}

			backtrack_ptr = outbuf_ptr - backtrack_amount;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *backtrack_ptr++;
			}

			inbuf_remain -= 2;
			outbuf_remain -= runcount;
			continue;

			break;

		default:
			return R_FAILURE;
			break;

		}

	}			/* end while */

	return R_SUCCESS;
}

int FlipImage(uchar * img_buf, int columns, int scanlines)
{

	int line;
	uchar *tmp_scan;

	uchar *flip_p1;
	uchar *flip_p2;

	int flipcount = scanlines / 2;

	tmp_scan = (uchar *)malloc(columns);
	if (tmp_scan == NULL) {
		return R_FAILURE;
	}

	flip_p1 = img_buf;
	flip_p2 = img_buf + (columns * (scanlines - 1));

	for (line = 0; line < flipcount; line++) {

		memcpy(tmp_scan, flip_p1, columns);
		memcpy(flip_p1, flip_p2, columns);
		memcpy(flip_p2, tmp_scan, columns);

		flip_p1 += columns;
		flip_p2 -= columns;
	}

	free(tmp_scan);

	return R_SUCCESS;
}

int
UnbankBGImage(uchar * dst_buf,
    const uchar * src_buf, int columns, int scanlines)
{

	int x, y;
	int temp;

	int quadruple_rows;
	int remain_rows;

	int rowjump_src;
	int rowjump_dest;

	const uchar *src_p;
	uchar *dst_p;

	const uchar *srcptr1, *srcptr2, *srcptr3, *srcptr4;
	uchar *dstptr1, *dstptr2, *dstptr3, *dstptr4;

	quadruple_rows = scanlines - (scanlines % 4);
	remain_rows = scanlines - quadruple_rows;

	assert(scanlines > 0);

	src_p = src_buf;
	dst_p = dst_buf + columns;

	srcptr1 = src_p;
	srcptr2 = src_p + 1;
	srcptr3 = src_p + 2;
	srcptr4 = src_p + 3;

	dstptr1 = dst_buf;
	dstptr2 = dst_buf + columns;
	dstptr3 = dst_buf + columns * 2;
	dstptr4 = dst_buf + columns * 3;

	rowjump_src = columns * 4;
	rowjump_dest = columns * 4;

	/* Unbank groups of 4 first */
	for (y = 0; y < quadruple_rows; y += 4) {

		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
			dstptr3[x] = srcptr3[temp];
			dstptr4[x] = srcptr4[temp];
		}

		/* This is to avoid generating invalid pointers - 
		 * usually innocuous, but undefined */
		if (y < quadruple_rows - 4) {

			dstptr1 += rowjump_dest;
			dstptr2 += rowjump_dest;
			dstptr3 += rowjump_dest;
			dstptr4 += rowjump_dest;
			srcptr1 += rowjump_src;
			srcptr2 += rowjump_src;
			srcptr3 += rowjump_src;
			srcptr4 += rowjump_src;
		}

	}

	/* Unbank rows remaining */
	switch (remain_rows) {

	case 1:
		dstptr1 += rowjump_dest;
		srcptr1 += rowjump_src;

		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
		}
		break;

	case 2:
		dstptr1 += rowjump_dest;
		dstptr2 += rowjump_dest;
		srcptr1 += rowjump_src;
		srcptr2 += rowjump_src;

		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
		}
		break;

	case 3:
		dstptr1 += rowjump_dest;
		dstptr2 += rowjump_dest;
		dstptr3 += rowjump_dest;
		srcptr1 += rowjump_src;
		srcptr2 += rowjump_src;
		srcptr3 += rowjump_src;

		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
			dstptr3[x] = srcptr3[temp];
		}
		break;

	default:
		break;
	}

	return R_SUCCESS;
}

const uchar *IMG_GetImagePal(const uchar * image_data, size_t image_size)
{
	if (image_size <= SAGA_IMAGE_HEADER_LEN) {
		return NULL;
	}

	return image_data + SAGA_IMAGE_HEADER_LEN;
}

} // End of namespace Saga
