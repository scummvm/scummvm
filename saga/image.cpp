/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// SAGA Image resource management routines
#include "saga/saga.h"

#include "saga/stream.h"

namespace Saga {

static int granulate(int value, int granularity) {
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

int SagaEngine::decodeBGImage(const byte *image_data, size_t image_size,
				  byte **output_buf, size_t *output_buf_len, int *w, int *h, bool flip) {
	ImageHeader hdr;
	int modex_height;
	const byte *RLE_data_ptr;
	size_t RLE_data_len;
	byte *decode_buf;
	size_t decode_buf_len;
	byte *out_buf;
	size_t out_buf_len;

	if (image_size <= SAGA_IMAGE_DATA_OFFSET) {
		error("decodeBGImage() Image size is way too small (%d)", image_size);
	}

	MemoryReadStreamEndian readS(image_data, image_size, isBigEndian());

	hdr.width = readS.readUint16();
	hdr.height = readS.readUint16();
	// The next four bytes of the image header aren't used.
	readS.readUint16();
	readS.readUint16();

	RLE_data_ptr = image_data + SAGA_IMAGE_DATA_OFFSET;
	RLE_data_len = image_size - SAGA_IMAGE_DATA_OFFSET;

	modex_height = granulate(hdr.height, 4);

	decode_buf_len = hdr.width * modex_height;
	decode_buf = (byte *)malloc(decode_buf_len);

	out_buf_len = hdr.width * hdr.height;
	out_buf = (byte *)malloc(out_buf_len);

	if (decodeBGImageRLE(RLE_data_ptr,
		RLE_data_len, decode_buf, decode_buf_len) != SUCCESS) {
		free(decode_buf);
		free(out_buf);
		return FAILURE;
	}

	unbankBGImage(out_buf, decode_buf, hdr.width, hdr.height);

	// For some reason bg images in IHNM are upside down
	if (getGameType() == GType_IHNM && !flip) {
		flipImage(out_buf, hdr.width, hdr.height);
	}

	free(decode_buf);

	*output_buf_len = out_buf_len;
	*output_buf = out_buf;

	*w = hdr.width;
	*h = hdr.height;

	return SUCCESS;
}

int SagaEngine::decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len) {
	const byte *inbuf_ptr;
	byte *outbuf_ptr;
	uint32 inbuf_remain;

	const byte *inbuf_end;
	byte *outbuf_end;
	uint32 outbuf_remain;

	byte mark_byte;
	int test_byte;

	uint32 runcount;

	byte bitfield;
	byte bitfield_byte1;
	byte bitfield_byte2;

	byte *backtrack_ptr;
	int backtrack_amount;

	uint16 c, b;

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
			return FAILURE;
		}

		mark_byte = *inbuf_ptr++;
		inbuf_remain--;

		test_byte = mark_byte & 0xC0; // Mask all but two high order bits

		switch (test_byte) {
		case 0xC0: // 1100 0000
			// Uncompressed run follows: Max runlength 63
			runcount = mark_byte & 0x3f;
			if ((inbuf_remain < runcount) || (outbuf_remain < runcount)) {
				return FAILURE;
			}

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr++;
			}

			inbuf_remain -= runcount;
			outbuf_remain -= runcount;
			continue;
			break;
		case 0x80: // 1000 0000
			// Compressed run follows: Max runlength 63
			runcount = (mark_byte & 0x3f) + 3;
			if (!inbuf_remain || (outbuf_remain < runcount)) {
				return FAILURE;
			}

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr;
			}

			inbuf_ptr++;
			inbuf_remain--;
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x40: // 0100 0000
			// Repeat decoded sequence from output stream:
			// Max runlength 10

			runcount = ((mark_byte >> 3) & 0x07U) + 3;
			backtrack_amount = *inbuf_ptr;

			if (!inbuf_remain || (backtrack_amount > (outbuf_ptr - outbuf)) || (runcount > outbuf_remain)) {
				return FAILURE;
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
		default: // 0000 0000
			break;
		}

		// Mask all but the third and fourth highest order bits
		test_byte = mark_byte & 0x30;

		switch (test_byte) {

		case 0x30: // 0011 0000
			// Bitfield compression
			runcount = (mark_byte & 0x0F) + 1;

			if ((inbuf_remain < (runcount + 2)) || (outbuf_remain < (runcount * 8))) {
				return FAILURE;
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
		case 0x20: // 0010 0000
			// Uncompressed run follows
			runcount = ((mark_byte & 0x0F) << 8) + *inbuf_ptr;
			if ((inbuf_remain < (runcount + 1)) || (outbuf_remain < runcount)) {
				return FAILURE;
			}

			inbuf_ptr++;

			for (c = 0; c < runcount; c++) {
				*outbuf_ptr++ = *inbuf_ptr++;
			}

			inbuf_remain -= (runcount + 1);
			outbuf_remain -= runcount;
			continue;

			break;

		case 0x10: // 0001 0000
			// Repeat decoded sequence from output stream
			backtrack_amount = ((mark_byte & 0x0F) << 8) + *inbuf_ptr;
			if (inbuf_remain < 2) {
				return FAILURE;
			}

			inbuf_ptr++;
			runcount = *inbuf_ptr++;

			if ((backtrack_amount > (outbuf_ptr - outbuf)) || (outbuf_remain < runcount)) {
				return FAILURE;
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
			return FAILURE;
			break;
		}
	}

	return SUCCESS;
}

int SagaEngine::flipImage(byte *img_buf, int columns, int scanlines) {
	int line;
	byte *tmp_scan;

	byte *flip_p1;
	byte *flip_p2;

	int flipcount = scanlines / 2;

	tmp_scan = (byte *)malloc(columns);
	if (tmp_scan == NULL) {
		return FAILURE;
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

	return SUCCESS;
}

int SagaEngine::unbankBGImage(byte *dst_buf, const byte *src_buf, int columns, int scanlines) {
	int x, y;
	int temp;
	int quadruple_rows;
	int remain_rows;
	int rowjump_src;
	int rowjump_dest;
	const byte *src_p;
	byte *dst_p;
	const byte *srcptr1, *srcptr2, *srcptr3, *srcptr4;
	byte *dstptr1, *dstptr2, *dstptr3, *dstptr4;

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

	// Unbank groups of 4 first
	for (y = 0; y < quadruple_rows; y += 4) {
		for (x = 0; x < columns; x++) {
			temp = x * 4;
			dstptr1[x] = srcptr1[temp];
			dstptr2[x] = srcptr2[temp];
			dstptr3[x] = srcptr3[temp];
			dstptr4[x] = srcptr4[temp];
		}

		// This is to avoid generating invalid pointers -
		// usually innocuous, but undefined
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

	// Unbank rows remaining
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
	return SUCCESS;
}

const byte *SagaEngine::getImagePal(const byte *image_data, size_t image_size) {
	if (image_size <= SAGA_IMAGE_HEADER_LEN) {
		return NULL;
	}

	return image_data + SAGA_IMAGE_HEADER_LEN;
}

} // End of namespace Saga
