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

/* Reads data from a resource file and stores the result in memory */

#include "common/util.h"
#include "sci/include/sci_memory.h"
#include "sci/include/sciresource.h"

namespace Sci {

/* DEFLATE-DCL
** Refer to the FreeSCI docs for a full description.
*/

#define HUFFMAN_LEAF 0x40000000

struct bit_read_struct {
	int length;
	int bitpos;
	int bytepos;
	byte *data;
};

#define BRANCH_SHIFT 12
#define BRANCH_NODE(pos, left, right)  ((left << BRANCH_SHIFT) | (right)),
#define LEAF_NODE(pos, value)  ((value) | HUFFMAN_LEAF),


static int length_tree[] = {
#include "treedef.1"
	0 /* We need something witout a comma at the end */
};

static int distance_tree[] = {
#include "treedef.2"
	0 /* We need something witout a comma at the end */
};

static int ascii_tree[] = {
#include "treedef.3"
	0 /* We need something witout a comma at the end */
};

#define CALLC(x) { if ((x) == -SCI_ERROR_DECOMPRESSION_OVERFLOW) return -SCI_ERROR_DECOMPRESSION_OVERFLOW; }

static inline int
getbits_msb_first(struct bit_read_struct *inp, int bits) {
	int morebytes = (bits + inp->bitpos - 1) >> 3;
	int result = 0;
	int i;

	if (inp->bytepos + morebytes >= inp->length) {
		error("read out-of-bounds with bytepos %d + morebytes %d >= length %d\n",
		        inp->bytepos, morebytes, inp->length);
		return -SCI_ERROR_DECOMPRESSION_OVERFLOW;
	}

	for (i = 0; i <= morebytes; i++)
		result |= (inp->data[inp->bytepos + i]) << (i << 3);

	result >>= inp->bitpos;
	result &= ~(~0 << bits);

	inp->bitpos += bits - (morebytes << 3);
	inp->bytepos += morebytes;

	return result;
}

static int DEBUG_DCL_INFLATE = 0; /* FIXME: Make this a define eventually */

static inline int
getbits(struct bit_read_struct *inp, int bits) {
	int morebytes = (bits + inp->bitpos - 1) >> 3;
	int result = 0;
	int i;

	if (inp->bytepos + morebytes >= inp->length) {
		error("read out-of-bounds with bytepos %d + morebytes %d >= length %d\n",
		        inp->bytepos, morebytes, inp->length);
		return -SCI_ERROR_DECOMPRESSION_OVERFLOW;
	}

	for (i = 0; i <= morebytes; i++)
		result |= (inp->data[inp->bytepos + i]) << (i << 3);

	result >>= inp->bitpos;
	result &= ~((~0) << bits);

	inp->bitpos += bits - (morebytes << 3);
	inp->bytepos += morebytes;

	if (DEBUG_DCL_INFLATE)
		error("(%d:%04x)", bits, result);

	return result;
}

static int
huffman_lookup(struct bit_read_struct *inp, int *tree) {
	int pos = 0;
	int bit;

	while (!(tree[pos] & HUFFMAN_LEAF)) {
		CALLC(bit = getbits(inp, 1));
		if (DEBUG_DCL_INFLATE)
			error("[%d]:%d->", pos, bit);
		if (bit)
			pos = tree[pos] & ~(~0 << BRANCH_SHIFT);
		else
			pos = tree[pos] >> BRANCH_SHIFT;
	}
	if (DEBUG_DCL_INFLATE)
		error("=%02x\n", tree[pos] & 0xffff);
	return tree[pos] & 0xffff;
}

#define VALUE_M(i) ((i == 0)? 7 : (VALUE_M(i - 1) + 2**i));

#define DCL_ASCII_MODE 1

static int
decrypt4_hdyn(byte *dest, int length, struct bit_read_struct *reader) {
	int mode, length_param, value, val_length, val_distance;
	int write_pos = 0;

	CALLC(mode = getbits(reader, 8));
	CALLC(length_param = getbits(reader, 8));

	if (mode == DCL_ASCII_MODE) {
		warning("DCL-INFLATE: Warning: Decompressing ASCII mode (untested)");
		/*		DEBUG_DCL_INFLATE = 1; */
	} else if (mode) {
		error("DCL-INFLATE: Error: Encountered mode %02x, expected 00 or 01\n", mode);
		return 1;
	}

	if (DEBUG_DCL_INFLATE) {
		int i;
		for (i = 0; i < reader->length; i++) {
			error("%02x ", reader->data[i]);
			if (!((i + 1) & 0x1f))
				error("\n");
		}


		error("\n---\n");
	}


	if (length_param < 3 || length_param > 6)
		error("Warning: Unexpected length_param value %d (expected in [3,6])\n", length_param);

	while (write_pos < length) {
		CALLC(value = getbits(reader, 1));

		if (value) { /* (length,distance) pair */
			CALLC(value = huffman_lookup(reader, length_tree));

			if (value < 8)
				val_length = value + 2;
			else {
				int length_bonus;

				val_length = (1 << (value - 7)) + 8;
				CALLC(length_bonus = getbits(reader, value - 7));
				val_length += length_bonus;
			}

			if (DEBUG_DCL_INFLATE)
				error(" | ");

			CALLC(value = huffman_lookup(reader, distance_tree));

			if (val_length == 2) {
				val_distance = value << 2;

				CALLC(value = getbits(reader, 2));
				val_distance |= value;
			} else {
				val_distance = value << length_param;

				CALLC(value = getbits(reader, length_param));
				val_distance |= value;
			}
			++val_distance;

			if (DEBUG_DCL_INFLATE)
				error("\nCOPY(%d from %d)\n", val_length, val_distance);

			if (val_length + write_pos > length) {
				error("DCL-INFLATE Error: Write out of bounds while copying %d bytes\n", val_length);
				return -SCI_ERROR_DECOMPRESSION_OVERFLOW;
			}

			if (write_pos < val_distance) {
				error("DCL-INFLATE Error: Attempt to copy from before beginning of input stream\n");
				return -SCI_ERROR_DECOMPRESSION_INSANE;
			}

			while (val_length) {
				int copy_length = (val_length > val_distance) ? val_distance : val_length;

				memcpy(dest + write_pos, dest + write_pos - val_distance, copy_length);

				if (DEBUG_DCL_INFLATE) {
					int i;
					for (i = 0; i < copy_length; i++)
						error("\33[32;31m%02x\33[37;37m ", dest[write_pos + i]);
					error("\n");
				}

				val_length -= copy_length;
				val_distance += copy_length;
				write_pos += copy_length;
			}

		} else { /* Copy byte verbatim */
			if (mode == DCL_ASCII_MODE) {
				CALLC(value = huffman_lookup(reader, ascii_tree));
			} else {
				CALLC(value = getbits(reader, 8));
			}

			dest[write_pos++] = value;

			if (DEBUG_DCL_INFLATE)
				error("\33[32;31m%02x \33[37;37m", value);
		}
	}

	return 0;
}

int
decrypt4(guint8* dest, guint8* src, int length, int complength) {
	struct bit_read_struct reader;

	reader.length = complength;
	reader.bitpos = 0;
	reader.bytepos = 0;
	reader.data = src;

	return -decrypt4_hdyn(dest, length, &reader);
}




void decryptinit3(void);
int decrypt3(guint8* dest, guint8* src, int length, int complength);

int decompress1(resource_t *result, Common::ReadStream &stream, int sci_version) {
	uint16 compressedLength;
	uint16 compressionMethod, result_size;
	uint8 *buffer;
	uint8 tempid;

	if (sci_version == SCI_VERSION_1_EARLY) {
		if (stream.read(&(result->id), 2) != 2)
			return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
		result->id = GUINT16_SWAP_LE_BE_CONSTANT(result->id);
#endif

		result->number = result->id & 0x07ff;
		result->type = result->id >> 11;

		if ((result->number >= sci_max_resource_nr[SCI_VERSION_1_LATE]) || (result->type > sci_invalid_resource))
			return SCI_ERROR_DECOMPRESSION_INSANE;
	} else {
		if (stream.read(&tempid, 1) != 1)
			return SCI_ERROR_IO_ERROR;

		result->id = tempid;

		result->type = result->id & 0x7f;
		if (stream.read(&(result->number), 2) != 2)
			return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
		result->number = GUINT16_SWAP_LE_BE_CONSTANT(result->number);
#endif /* WORDS_BIGENDIAN */
		if ((result->number >= sci_max_resource_nr[SCI_VERSION_1_LATE]) || (result->type > sci_invalid_resource))
			return SCI_ERROR_DECOMPRESSION_INSANE;
	}

	if ((stream.read(&compressedLength, 2) != 2) ||
	        (stream.read(&result_size, 2) != 2) ||
	        (stream.read(&compressionMethod, 2) != 2))
		return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
	compressedLength = GUINT16_SWAP_LE_BE_CONSTANT(compressedLength);
	result_size = GUINT16_SWAP_LE_BE_CONSTANT(result_size);
	compressionMethod = GUINT16_SWAP_LE_BE_CONSTANT(compressionMethod);
#endif
	result->size = result_size;

	if (result->size > SCI_MAX_RESOURCE_SIZE)
		return SCI_ERROR_RESOURCE_TOO_BIG;

	if (compressedLength > 4)
		compressedLength -= 4;
	else { /* Object has size zero (e.g. view.000 in sq3) (does this really exist?) */
		result->data = 0;
		result->status = SCI_STATUS_NOMALLOC;
		return SCI_ERROR_EMPTY_OBJECT;
	}

	buffer = (guint8*)sci_malloc(compressedLength);
	result->data = (unsigned char*)sci_malloc(result->size);

	if (stream.read(buffer, compressedLength) != compressedLength) {
		free(result->data);
		free(buffer);
		return SCI_ERROR_IO_ERROR;
	};


#ifdef _SCI_DECOMPRESS_DEBUG
	error("Resource %i.%s encrypted with method SCI1%c/%hi at %.2f%%"
	        " ratio\n",
	        result->number, sci_resource_type_suffixes[result->type],
	        early ? 'e' : 'l',
	        compressionMethod,
	        (result->size == 0) ? -1.0 :
	        (100.0 * compressedLength / result->size));
	error("  compressedLength = 0x%hx, actualLength=0x%hx\n",
	        compressedLength, result->size);
#endif

	switch (compressionMethod) {

	case 0: /* no compression */
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

	case 1: /* LZW */
		if (decrypt2(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; /* So that we know that it didn't work */
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 2: /* ??? */
		decryptinit3();
		if (decrypt3(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; /* So that we know that it didn't work */
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
			result->data = 0; /* So that we know that it didn't work */
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
			result->data = 0; /* So that we know that it didn't work */
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->data = pic_reorder(result->data, result->size);
		result->status = SCI_STATUS_ALLOCATED;
		break;

	default:
		error("Resource %s.%03hi: Compression method SCI1/%hi not "
		        "supported", sci_resource_types[result->type], result->number,
		        compressionMethod);
		free(result->data);
		result->data = 0; /* So that we know that it didn't work */
		result->status = SCI_STATUS_NOMALLOC;
		free(buffer);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	free(buffer);
	return 0;
}

} // End of namespace Sci
