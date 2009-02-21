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

// Reads data from a resource file and stores the result in memory

#include "common/util.h"
#include "sci/include/sci_memory.h"
#include "sci/include/sciresource.h"

namespace Sci {

// The following code was originally created by Carl Muckenhoupt for his
// SCI decoder. It has been ported to the FreeSCI environment by Sergey Lapin.

// TODO: Clean up, re-organize, improve speed-wise */

struct tokenlist {
	guint8 data;
	gint16 next;
} tokens[0x1004];

static gint8 stak[0x1014] = {0};
static gint8 lastchar = 0;
static gint16 stakptr = 0;
static guint16 s_numbits, s_bitstring, lastbits, decryptstart;
static gint16 curtoken, endtoken;

uint32 gbits(int numbits,  guint8 * data, int dlen);

void decryptinit3() {
	int i;

	lastchar = lastbits = s_bitstring = stakptr = 0;
	s_numbits = 9;
	curtoken = 0x102;
	endtoken = 0x1ff;
	decryptstart = 0;
	gbits(0, 0, 0);

	for (i = 0;i < 0x1004;i++) {
		tokens[i].next = 0;
		tokens[i].data = 0;
	}
}

int decrypt3(guint8 *dest, guint8 *src, int length, int complength) {
	static gint16 token;
	while (length != 0) {
		switch (decryptstart) {
		case 0:
		case 1:
			s_bitstring = gbits(s_numbits, src, complength);
			if (s_bitstring == 0x101) { // found end-of-data signal
				decryptstart = 4;
				return 0;
			}
			if (decryptstart == 0) { // first char
				decryptstart = 1;
				lastbits = s_bitstring;
				*(dest++) = lastchar = (s_bitstring & 0xff);
				if (--length != 0)
					continue;
				return 0;
			}
			if (s_bitstring == 0x100) { // start-over signal
				s_numbits = 9;
				endtoken = 0x1ff;
				curtoken = 0x102;
				decryptstart = 0;
				continue;
			}
			token = s_bitstring;
			if (token >= curtoken) { // index past current point
				token = lastbits;
				stak[stakptr++] = lastchar;
			}
			while ((token > 0xff) && (token < 0x1004)) { // follow links back in data
				stak[stakptr++] = tokens[token].data;
				token = tokens[token].next;
			}
			lastchar = stak[stakptr++] = token & 0xff;
		case 2:
			while (stakptr > 0) { // put stack in buffer
				*(dest++) = stak[--stakptr];
				length--;
				if (length == 0) {
					decryptstart = 2;
					return 0;
				}
			}
			decryptstart = 1;
			if (curtoken <= endtoken) { // put token into record
				tokens[curtoken].data = lastchar;
				tokens[curtoken].next = lastbits;
				curtoken++;
				if (curtoken == endtoken && s_numbits != 12) {
					s_numbits++;
					endtoken <<= 1;
					endtoken++;
				}
			}
			lastbits = s_bitstring;
			continue; // When are "break" and "continue" synonymous?
		case 4:
			return 0;
		}
	}

	return 0;
}

guint32 gbits(int numbits,  guint8 * data, int dlen) {
	int place; // indicates location within byte
	guint32 bitstring;
	static guint32 whichbit = 0;
	int i;

	if (numbits == 0) {
		whichbit = 0;
		return 0;
	}

	place = whichbit >> 3;
	bitstring = 0;
	for (i = (numbits >> 3) + 1;i >= 0;i--) {
		if (i + place < dlen)
			bitstring |= data[place+i] << (8 * (2 - i));
	}
	//bitstring = data[place + 2] | (long)(data[place + 1]) << 8 | (long)(data[place]) << 16;
	bitstring >>= 24 - (whichbit & 7) - numbits;
	bitstring &= (0xffffffff >> (32 - numbits));
	// Okay, so this could be made faster with a table lookup.
	// It doesn't matter. It's fast enough as it is.
	whichbit += numbits;

	return bitstring;
}

// Carl Muckenhoupt's code ends here

enum {
	PIC_OP_SET_COLOR = 0xf0,
	PIC_OP_DISABLE_VISUAL = 0xf1,
	PIC_OP_SET_PRIORITY = 0xf2,
	PIC_OP_DISABLE_PRIORITY = 0xf3,
	PIC_OP_SHORT_PATTERNS = 0xf4,
	PIC_OP_MEDIUM_LINES = 0xf5,
	PIC_OP_LONG_LINES = 0xf6,
	PIC_OP_SHORT_LINES = 0xf7,
	PIC_OP_FILL = 0xf8,
	PIC_OP_SET_PATTERN = 0xf9,
	PIC_OP_ABSOLUTE_PATTERN = 0xfa,
	PIC_OP_SET_CONTROL = 0xfb,
	PIC_OP_DISABLE_CONTROL = 0xfc,
	PIC_OP_MEDIUM_PATTERNS = 0xfd,
	PIC_OP_OPX = 0xfe,
	PIC_OP_TERMINATE = 0xff
};

enum {
	PIC_OPX_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_EMBEDDED_VIEW = 1,
	PIC_OPX_SET_PALETTE = 2,
	PIC_OPX_PRIORITY_TABLE_EQDIST = 3,
	PIC_OPX_PRIORITY_TABLE_EXPLICIT = 4
};

#define PAL_SIZE 1284
#define CEL_HEADER_SIZE 7
#define EXTRA_MAGIC_SIZE 15

static void decode_rle(byte **rledata, byte **pixeldata, byte *outbuffer, int size) {
	int pos = 0;
	char nextbyte;
	byte *rd = *rledata;
	byte *ob = outbuffer;
	byte *pd = *pixeldata;

	while (pos < size) {
		nextbyte = *(rd++);
		*(ob++) = nextbyte;
		pos ++;
		switch (nextbyte&0xC0) {
		case 0x40 :
		case 0x00 :
			memcpy(ob, pd, nextbyte);
			pd += nextbyte;
			ob += nextbyte;
			pos += nextbyte;
			break;
		case 0xC0 :
			break;
		case 0x80 :
			nextbyte = *(pd++);
			*(ob++) = nextbyte;
			pos ++;
			break;
		}
	}

	*rledata = rd;
	*pixeldata = pd;
}

/*
 * Does the same this as above, only to determine the length of the compressed
 * source data.
 *
 * Yes, this is inefficient.
 */
static int rle_size(byte *rledata, int dsize) {
	int pos = 0;
	char nextbyte;
	int size = 0;

	while (pos < dsize) {
		nextbyte = *(rledata++);
		pos ++;
		size ++;

		switch (nextbyte&0xC0) {
		case 0x40 :
		case 0x00 :
			pos += nextbyte;
			break;
		case 0xC0 :
			break;
		case 0x80 :
			pos ++;
			break;
		}
	}

	return size;
}

byte *pic_reorder(byte *inbuffer, int dsize) {
	byte *reorderBuffer;
	int view_size;
	int view_start;
	int cdata_size;
	int i;
	byte *seeker = inbuffer;
	byte *writer;
	char viewdata[CEL_HEADER_SIZE];
	byte *cdata, *cdata_start;

	writer = reorderBuffer = (byte *) malloc(dsize);

	*(writer++) = PIC_OP_OPX;
	*(writer++) = PIC_OPX_SET_PALETTE;

	for (i = 0;i < 256;i++) // Palette translation map
		*(writer++) = i;

	putInt16(writer, 0); // Palette stamp
	writer += 2;
	putInt16(writer, 0);
	writer += 2;

	view_size = getUInt16(seeker);
	seeker += 2;
	view_start = getUInt16(seeker);
	seeker += 2;
	cdata_size = getUInt16(seeker);
	seeker += 2;

	memcpy(viewdata, seeker, sizeof(viewdata));
	seeker += sizeof(viewdata);

	memcpy(writer, seeker, 4 * 256); // Palette
	seeker += 4 * 256;
	writer += 4 * 256;

	if (view_start != PAL_SIZE + 2) { // +2 for the opcode
		memcpy(writer, seeker, view_start - PAL_SIZE - 2);
		seeker += view_start - PAL_SIZE - 2;
		writer += view_start - PAL_SIZE - 2;
	}

	if (dsize != view_start + EXTRA_MAGIC_SIZE + view_size) {
		memcpy(reorderBuffer + view_size + view_start + EXTRA_MAGIC_SIZE, seeker,
		       dsize - view_size - view_start - EXTRA_MAGIC_SIZE);
		seeker += dsize - view_size - view_start - EXTRA_MAGIC_SIZE;
	}

	cdata_start = cdata = (byte *)malloc(cdata_size);
	memcpy(cdata, seeker, cdata_size);
	seeker += cdata_size;

	writer = reorderBuffer + view_start;
	*(writer++) = PIC_OP_OPX;
	*(writer++) = PIC_OPX_EMBEDDED_VIEW;
	*(writer++) = 0;
	*(writer++) = 0;
	*(writer++) = 0;
	putInt16(writer, view_size + 8);
	writer += 2;

	memcpy(writer, viewdata, sizeof(viewdata));
	writer += sizeof(viewdata);

	*(writer++) = 0;

	decode_rle(&seeker, &cdata, writer, view_size);

	free(cdata_start);
	free(inbuffer);

	return reorderBuffer;
}

#define VIEW_HEADER_COLORS_8BIT 0x80

static void build_cel_headers(byte **seeker, byte **writer, int celindex, int *cc_lengths, int max) {
	int c, w;

	for (c = 0;c < max;c++) {
		w = getUInt16(*seeker);
		putInt16(*writer, w);
		*seeker += 2;
		*writer += 2;
		w = getUInt16(*seeker);
		putInt16(*writer, w);
		*seeker += 2;
		*writer += 2;
		w = getUInt16(*seeker);
		putInt16(*writer, w);
		*seeker += 2;
		*writer += 2;
		w = *((*seeker)++);
		putInt16(*writer, w); // Zero extension
		*writer += 2;

		*writer += cc_lengths[celindex];
		celindex ++;
	}
}

byte *view_reorder(byte *inbuffer, int dsize) {
	byte *cellengths;
	int loopheaders;
	int lh_present;
	int lh_mask;
	int pal_offset;
	int cel_total;
	int unknown;
	byte *seeker = inbuffer;
	char celcounts[100];
	byte *outbuffer = (byte *)malloc(dsize);
	byte *writer = outbuffer;
	byte *lh_ptr;
	byte *rle_ptr, *pix_ptr;
	int l, lb, c, celindex, lh_last = -1;
	int chptr;
	int w;
	int *cc_lengths;
	byte **cc_pos;

	// Parse the main header
	cellengths = inbuffer + getUInt16(seeker) + 2;
	seeker += 2;
	loopheaders = *(seeker++);
	lh_present = *(seeker++);
	lh_mask = getUInt16(seeker);
	seeker += 2;
	unknown = getUInt16(seeker);
	seeker += 2;
	pal_offset = getUInt16(seeker);
	seeker += 2;
	cel_total = getUInt16(seeker);
	seeker += 2;

	cc_pos = (byte **)malloc(sizeof(byte *) * cel_total);
	cc_lengths = (int *)malloc(sizeof(int) * cel_total);

	for (c = 0;c < cel_total;c++)
		cc_lengths[c] = getUInt16(cellengths + 2 * c);

	*(writer++) = loopheaders;
	*(writer++) = VIEW_HEADER_COLORS_8BIT;
	putInt16(writer, lh_mask);
	writer += 2;
	putInt16(writer, unknown);
	writer += 2;
	putInt16(writer, pal_offset);
	writer += 2;

	lh_ptr = writer;
	writer += 2 * loopheaders; // Make room for the loop offset table

	pix_ptr = writer;

	memcpy(celcounts, seeker, lh_present);
	seeker += lh_present;

	lb = 1;
	celindex = 0;

	rle_ptr = pix_ptr = cellengths + (2 * cel_total);
	w = 0;

	for (l = 0;l < loopheaders;l++) {
		if (lh_mask & lb) { // The loop is _not_ present
			if (lh_last == -1) {
				error("Error: While reordering view: Loop not present, but can't re-use last loop");
				lh_last = 0;
			}
			putInt16(lh_ptr, lh_last);
			lh_ptr += 2;
		} else {
			lh_last = writer - outbuffer;
			putInt16(lh_ptr, lh_last);
			lh_ptr += 2;
			putInt16(writer, celcounts[w]);
			writer += 2;
			putInt16(writer, 0);
			writer += 2;

			// Now, build the cel offset table
			chptr = (writer - outbuffer) + (2 * celcounts[w]);

			for (c = 0; c < celcounts[w]; c++) {
				putInt16(writer, chptr);
				writer += 2;
				cc_pos[celindex + c] = outbuffer + chptr;
				chptr += 8 + getUInt16(cellengths + 2 * (celindex + c));
			}

			build_cel_headers(&seeker, &writer, celindex, cc_lengths, celcounts[w]);

			celindex += celcounts[w];
			w++;
		}

		lb = lb << 1;
	}

	if (celindex < cel_total) {
		error("View decompression generated too few (%d / %d) headers", celindex, cel_total);
		return NULL;
	}

	// Figure out where the pixel data begins.
	for (c = 0;c < cel_total;c++)
		pix_ptr += rle_size(pix_ptr, cc_lengths[c]);

	rle_ptr = cellengths + (2 * cel_total);
	for (c = 0;c < cel_total;c++)
		decode_rle(&rle_ptr, &pix_ptr, cc_pos[c] + 8, cc_lengths[c]);

	*(writer++) = 'P';
	*(writer++) = 'A';
	*(writer++) = 'L';

	for (c = 0;c < 256;c++)
		*(writer++) = c;

	seeker -= 4; // The missing four. Don't ask why.
	memcpy(writer, seeker, 4 * 256 + 4);

	free(cc_pos);
	free(cc_lengths);
	free(inbuffer);

	return outbuffer;
}

int decompress01(resource_t *result, Common::ReadStream &stream, int sci_version) {
	uint16 compressedLength, result_size;
	uint16 compressionMethod;
	uint8 *buffer;

	if (stream.read(&(result->id), 2) != 2)
		return SCI_ERROR_IO_ERROR;

#ifdef SCUMM_BIG_ENDIAN
	result->id = GUINT16_SWAP_LE_BE_CONSTANT(result->id);
#endif

	result->number = result->id & 0x07ff;
	result->type = result->id >> 11;

	if ((result->number > sci_max_resource_nr[sci_version] || (result->type > sci_invalid_resource)))
		return SCI_ERROR_DECOMPRESSION_INSANE;

	if ((stream.read(&compressedLength, 2) != 2) || (stream.read(&result_size, 2) != 2) || (stream.read(&compressionMethod, 2) != 2))
		return SCI_ERROR_IO_ERROR;

#ifdef SCUMM_BIG_ENDIAN
	compressedLength = GUINT16_SWAP_LE_BE_CONSTANT(compressedLength);
	result_size = GUINT16_SWAP_LE_BE_CONSTANT(result_size);
	compressionMethod = GUINT16_SWAP_LE_BE_CONSTANT(compressionMethod);
#endif
	result->size = result_size;

	//if ((result->size < 0) || (compressedLength < 0))
	//	return SCI_ERROR_DECOMPRESSION_INSANE;
	// This return will never happen in SCI0 or SCI1 (does it have any use?)

	if (result->size > SCI_MAX_RESOURCE_SIZE)
		return SCI_ERROR_RESOURCE_TOO_BIG;

	if (compressedLength > 4)
		compressedLength -= 4;
	else { // Object has size zero (e.g. view.000 in sq3) (does this really exist?)
		result->data = 0;
		result->status = SCI_STATUS_NOMALLOC;
		return SCI_ERROR_EMPTY_OBJECT;
	}

	buffer = (guint8 *)sci_malloc(compressedLength);
	result->data = (unsigned char *)sci_malloc(result->size);

	if (stream.read(buffer, compressedLength) != compressedLength) {
		free(result->data);
		free(buffer);
		return SCI_ERROR_IO_ERROR;
	};


#ifdef _SCI_DECOMPRESS_DEBUG
	error("Resource %s.%03hi encrypted with method SCI01/%hi at %.2f%% ratio\n", sci_resource_types[result->type],
			result->number, compressionMethod, (result->size == 0) ? -1.0 : (100.0 * compressedLength / result->size));
	error("  compressedLength = 0x%hx, actualLength=0x%hx\n", compressedLength, result->size);
#endif

	switch (compressionMethod) {

	case 0: // no compression
		if (result->size != compressedLength) {
			free(result->data);
			result->data = NULL;
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		memcpy(result->data, buffer, compressedLength);
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 1: // Some huffman encoding
		if (decrypt2(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 2: // ???
		decryptinit3();
		if (decrypt3(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 3:
		decryptinit3();
		if (decrypt3(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->data = view_reorder(result->data, result->size);
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 4:
		decryptinit3();
		if (decrypt3(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->data = pic_reorder(result->data, result->size);
		result->status = SCI_STATUS_ALLOCATED;
		break;

	default:
		error("Resource %s.%03hi: Compression method SCI1/%hi not supported", sci_resource_types[result->type],
				result->number, compressionMethod);
		free(result->data);
		result->data = 0; // So that we know that it didn't work
		result->status = SCI_STATUS_NOMALLOC;
		free(buffer);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	free(buffer);
	return 0;
}

} // End of namespace Sci
