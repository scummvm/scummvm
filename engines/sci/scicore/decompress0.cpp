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

/* Reads data from a resource file and stores the result in memory.
** This is for SCI version 0 style compression.
*/

#include "common/stream.h"
#include "common/endian.h"

#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"

namespace Sci {

//#define _SCI_DECOMPRESS_DEBUG

// 9-12 bit LZW encoding
int decrypt1(uint8 *dest, uint8 *src, int length, int complength) {
	// Doesn't do length checking yet
	/* Theory: Considering the input as a bit stream, we get a series of
	** 9 bit elements in the beginning. Every one of them is a 'token'
	** and either represents a literal (if < 0x100), or a link to a previous
	** token (tokens start at 0x102, because 0x101 is the end-of-stream
	** indicator and 0x100 is used to reset the bit stream decoder).
	** If it's a link, the indicated token and the character following it are
	** placed into the output stream. Note that the 'indicated token' may
	** very well consist of a link-token-plus-literal construct again, so
	** it's possible to represent strings longer than 2 recursively.
	** If the maximum number of tokens has been reached, the bit length is
	** increased by one, up to a maximum of 12 bits.
	** This implementation remembers the position each token was print to in
	** the output array, and the length of this token. This method should
	** be faster than the recursive approach.
	*/

	uint16 bitlen = 9; // no. of bits to read (max. 12)
	uint16 bitmask = 0x01ff;
	uint16 bitctr = 0; // current bit position
	uint16 bytectr = 0; // current byte position
	uint16 token; // The last received value
	uint16 maxtoken = 0x200; // The biggest token

	uint16 tokenlist[4096]; // pointers to dest[]
	uint16 tokenlengthlist[4096]; // char length of each token
	uint16 tokenctr = 0x102; // no. of registered tokens (starts here)

	uint16 tokenlastlength = 0;

	uint16 destctr = 0;

	while (bytectr < complength) {

		uint32 tokenmaker = src[bytectr++] >> bitctr;
		if (bytectr < complength)
			tokenmaker |= (src[bytectr] << (8 - bitctr));
		if (bytectr + 1 < complength)
			tokenmaker |= (src[bytectr+1] << (16 - bitctr));

		token = tokenmaker & bitmask;

		bitctr += bitlen - 8;

		while (bitctr >= 8) {
			bitctr -= 8;
			bytectr++;
		}

		if (token == 0x101)
			return 0; // terminator
		if (token == 0x100) { // reset command
			maxtoken = 0x200;
			bitlen = 9;
			bitmask = 0x01ff;
			tokenctr = 0x0102;
		} else {
			{
				int i;

				if (token > 0xff) {
					if (token >= tokenctr) {
#ifdef _SCI_DECOMPRESS_DEBUG
						fprintf(stderr, "decrypt1: Bad token %x!\n", token);
#endif
						// Well this is really bad
						// May be it should throw something like SCI_ERROR_DECOMPRESSION_INSANE
					} else {
						tokenlastlength = tokenlengthlist[token] + 1;
						if (destctr + tokenlastlength > length) {
#ifdef _SCI_DECOMPRESS_DEBUG
							// For me this seems a normal situation, It's necessary to handle it
							printf("decrypt1: Trying to write beyond the end of array(len=%d, destctr=%d, tok_len=%d)!\n",
							       length, destctr, tokenlastlength);
#endif
							i = 0;
							for (; destctr < length; destctr++) {
								dest[destctr++] = dest [tokenlist[token] + i];
								i++;
							}
						} else
							for (i = 0; i < tokenlastlength; i++) {
								dest[destctr++] = dest[tokenlist[token] + i];
							}
					}
				} else {
					tokenlastlength = 1;
					if (destctr >= length) {
#ifdef _SCI_DECOMPRESS_DEBUG
						printf("decrypt1: Try to write single byte beyond end of array!\n");
#endif
					} else
						dest[destctr++] = (byte)token;
				}

			}

			if (tokenctr == maxtoken) {
				if (bitlen < 12) {
					bitlen++;
					bitmask <<= 1;
					bitmask |= 1;
					maxtoken <<= 1;
				} else
					continue; // no further tokens allowed
			}

			tokenlist[tokenctr] = destctr - tokenlastlength;
			tokenlengthlist[tokenctr++] = tokenlastlength;
		}
	}

	return 0;
}

// Huffman-style token encoding
/***************************************************************************/
/* This code was taken from Carl Muckenhoupt's sde.c, with some minor      */
/* modifications.                                                          */
/***************************************************************************/

// decrypt2 helper function
int16 getc2(uint8 *node, uint8 *src, uint16 *bytectr, uint16 *bitctr, int complength) {
	uint16 next;

	while (node[1] != 0) {
		int16 value = (src[*bytectr] << (*bitctr));
		(*bitctr)++;
		if (*bitctr == 8) {
			(*bitctr) = 0;
			(*bytectr)++;
		}

		if (value & 0x80) {
			next = node[1] & 0x0f; // low 4 bits
			if (next == 0) {
				uint16 result = (src[*bytectr] << (*bitctr));

				if (++(*bytectr) > complength)
					return -1;
				else if (*bytectr < complength)
					result |= src[*bytectr] >> (8 - (*bitctr));

				result &= 0x0ff;
				return (result | 0x100);
			}
		} else {
			next = node[1] >> 4;  // high 4 bits
		}
		node += next << 1;
	}

	return (int16)READ_LE_UINT16(node);
}

// Huffman token decryptor
int decrypt2(uint8* dest, uint8* src, int length, int complength) {
	// no complength checking atm */
	uint8 numnodes, terminator;
	uint8 *nodes;
	int16 c;
	uint16 bitctr = 0, bytectr;

	numnodes = src[0];
	terminator = src[1];
	bytectr = 2 + (numnodes << 1);
	nodes = src + 2;

	while (((c = getc2(nodes, src, &bytectr, &bitctr, complength)) != (0x0100 | terminator)) && (c >= 0)) {
		if (length-- == 0)
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;

		*dest = (uint8)c;
		dest++;
	}

	return (c == -1) ? SCI_ERROR_DECOMPRESSION_OVERFLOW : 0;

}

// Carl Muckenhoupt's decompression code ends here

int sci0_get_compression_method(Common::ReadStream &stream) {
	uint16 compressionMethod;

	stream.readUint16LE();
	stream.readUint16LE();
	stream.readUint16LE();
	compressionMethod = stream.readUint16LE();
	if (stream.err())
		return SCI_ERROR_IO_ERROR;

	return compressionMethod;
}

int decompress0(resource_t *result, Common::ReadStream &stream, int sci_version) {
	uint16 compressedLength;
	uint16 compressionMethod;
	uint8 *buffer;

	result->id = stream.readUint16LE();
	if (stream.err())
		return SCI_ERROR_IO_ERROR;

	result->number = result->id & 0x07ff;
	result->type = result->id >> 11;

	if ((result->number > sci_max_resource_nr[sci_version]) || (result->type > sci_invalid_resource))
		return SCI_ERROR_DECOMPRESSION_INSANE;

	compressedLength = stream.readUint16LE();
	result->size = stream.readUint16LE();
	compressionMethod = stream.readUint16LE();
	if (stream.err())
		return SCI_ERROR_IO_ERROR;

	if (result->size > SCI_MAX_RESOURCE_SIZE)
		return SCI_ERROR_RESOURCE_TOO_BIG;
	// With SCI0, this simply cannot happen.

	if (compressedLength > 4)
		compressedLength -= 4;
	else { // Object has size zero (e.g. view.000 in sq3) (does this really exist?)
		result->data = 0;
		result->status = SCI_STATUS_NOMALLOC;
		return SCI_ERROR_EMPTY_OBJECT;
	}

	buffer = (uint8*)sci_malloc(compressedLength);
	result->data = (unsigned char *)sci_malloc(result->size);

	if (stream.read(buffer, compressedLength) != compressedLength) {
		free(result->data);
		free(buffer);
		return SCI_ERROR_IO_ERROR;
	};


#ifdef _SCI_DECOMPRESS_DEBUG
	fprintf(stderr, "Resource %s.%03hi encrypted with method %hi at %.2f%%"
	        " ratio\n",
	        sci_resource_types[result->type], result->number, compressionMethod,
	        (result->size == 0) ? -1.0 :
	        (100.0 * compressedLength / result->size));
	fprintf(stderr, "  compressedLength = 0x%hx, actualLength=0x%hx\n",
	        compressedLength, result->size);
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

	case 1: // LZW compression
		if (decrypt1(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 2: // Some sort of Huffman encoding
		if (decrypt2(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	default:
		fprintf(stderr, "Resource %s.%03hi: Compression method %hi not "
		        "supported!\n", sci_resource_types[result->type], result->number,
		        compressionMethod);
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
