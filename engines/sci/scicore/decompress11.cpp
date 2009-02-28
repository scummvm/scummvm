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

#include "common/stream.h"

#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"

namespace Sci {

#define DDEBUG if (0) printf

void decryptinit3();
int decrypt3(uint8* dest, uint8* src, int length, int complength);
int decrypt4(uint8* dest, uint8* src, int length, int complength);

int decompress11(Resource *result, Common::ReadStream &stream, int sci_version) {
	uint16 compressedLength;
	uint16 compressionMethod;
	uint8 *buffer;

	DDEBUG("d1");

	result->id = stream.readByte();
	if (stream.err())
		return SCI_ERROR_IO_ERROR;

	uint16 type = result->id & 0x7f;
	if (type > kResourceTypeInvalid)
		return SCI_ERROR_DECOMPRESSION_INSANE;
	result->type = (ResourceType)type;

	result->number = stream.readUint16LE();
	compressedLength = stream.readUint16LE();
	result->size = stream.readUint16LE();
	compressionMethod = stream.readUint16LE();
	if (stream.err())
		return SCI_ERROR_IO_ERROR;

	//if ((result->size < 0) || (compressedLength < 0))
	//	return SCI_ERROR_DECOMPRESSION_INSANE;
	// This return will never happen in SCI0 or SCI1 (does it have any use?)

	if (result->size > SCI_MAX_RESOURCE_SIZE)
		return SCI_ERROR_RESOURCE_TOO_BIG;

	if (compressedLength > 0)
		compressedLength -= 0;
	else { // Object has size zero (e.g. view.000 in sq3) (does this really exist?)
		result->data = 0;
		result->status = SCI_STATUS_NOMALLOC;
		return SCI_ERROR_EMPTY_OBJECT;
	}

	buffer = (uint8*)sci_malloc(compressedLength);
	result->data = (unsigned char*)sci_malloc(result->size);

	if (stream.read(buffer, compressedLength) != compressedLength) {
		free(result->data);
		free(buffer);
		return SCI_ERROR_IO_ERROR;
	};

	if (!(compressedLength & 1)) { // Align
		stream.readByte();
	}

#ifdef _SCI_DECOMPRESS_DEBUG
	fprintf(stderr, "Resource %i.%s encrypted with method SCI1.1/%hi at %.2f%%"
	        " ratio\n",
	        result->number, getResourceTypeSuffix(result->type),
	        compressionMethod,
	        (result->size == 0) ? -1.0 :
	        (100.0 * compressedLength / result->size));
	fprintf(stderr, "  compressedLength = 0x%hx, actualLength=0x%hx\n",
	        compressedLength, result->size);
#endif

	DDEBUG("/%d[%d]", compressionMethod, result->size);

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

	case 18:
	case 19:
	case 20:
		if (decrypt4(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; // So that we know that it didn't work
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 3:
	case 4: // NYI
		fprintf(stderr, "Resource %d.%s: Warning: compression type #%d not yet implemented\n",
		        result->number, getResourceTypeSuffix(result->type), compressionMethod);
		free(result->data);
		result->data = NULL;
		result->status = SCI_STATUS_NOMALLOC;
		break;

	default:
		fprintf(stderr, "Resource %d.%s: Compression method SCI1/%hi not "
		        "supported!\n", result->number, getResourceTypeSuffix(result->type),
		        compressionMethod);
		free(result->data);
		result->data = NULL; // So that we know that it didn't work
		result->status = SCI_STATUS_NOMALLOC;
		free(buffer);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	free(buffer);

	return 0;
}

} // End of namespace Sci
