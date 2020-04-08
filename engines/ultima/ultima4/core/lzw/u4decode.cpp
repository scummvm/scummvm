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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/core/lzw/lzw.h"
#include "ultima/ultima4/core/lzw/u4decode.h"

namespace Ultima {
namespace Ultima4 {
namespace LZW {

/*
 * Loads a file, decompresses it (from memory to memory), and writes the decompressed data to another file
 * Returns:
 * -1 if there was an error
 * the decompressed file length, on success
 */
long decompress_u4_file(Common::SeekableReadStream *in, long filesize, void **out) {
	byte *compressed_mem, *decompressed_mem;
	long compressed_filesize, decompressed_filesize;
	long errorCode;

	/* size of the compressed input file */
	compressed_filesize = filesize;

	/* input file should be longer than 0 bytes */
	if (compressed_filesize == 0)
		return (-1);

	/* check if the input file is _not_ a valid LZW-compressed file */
	if (!mightBeValidCompressedFile(in))
		return (-1);

	/* load compressed file into compressed_mem[] */
	compressed_mem = (byte *) malloc(compressed_filesize);
	in->read(compressed_mem, compressed_filesize);

	/*
	 * determine decompressed file size
	 * if lzw_get_decompressed_size() can't determine the decompressed size (i.e. the compressed
	 * data is corrupt), it returns -1
	 */
	decompressed_filesize = lzwGetDecompressedSize(compressed_mem, compressed_filesize);

	if (decompressed_filesize <= 0) {
		return (-1);
	}

	/* decompress file from compressed_mem[] into decompressed_mem[] */
	decompressed_mem = (byte *) malloc(decompressed_filesize);

	/* testing: clear destination mem */
	memset(decompressed_mem, 0, decompressed_filesize);

	errorCode = lzwDecompress(compressed_mem, decompressed_mem, compressed_filesize);

	free(compressed_mem);

	*out = decompressed_mem;

	return (errorCode);
}

long decompress_u4_memory(void *in, long inlen, void **out) {
	byte *compressed_mem, *decompressed_mem;
	long compressed_filesize, decompressed_filesize;
	long errorCode;

	/* size of the compressed input */
	compressed_filesize = inlen;

	/* input file should be longer than 0 bytes */
	if (compressed_filesize == 0)
		return (-1);

	compressed_mem = (byte *) in;

	/*
	 * determine decompressed data size
	 * if lzw_get_decompressed_size() can't determine the decompressed size (i.e. the compressed
	 * data is corrupt), it returns -1
	 */
	decompressed_filesize = lzwGetDecompressedSize(compressed_mem, compressed_filesize);

	if (decompressed_filesize <= 0) {
		return (-1);
	}

	/* decompress file from compressed_mem[] into decompressed_mem[] */
	decompressed_mem = (byte *) malloc(decompressed_filesize);

	/* testing: clear destination mem */
	memset(decompressed_mem, 0, decompressed_filesize);

	errorCode = lzwDecompress(compressed_mem, decompressed_mem, compressed_filesize);

	*out = decompressed_mem;

	return (errorCode);
}

/*
 * Returns the size of a file, and moves the file pointer to the beginning.
 * The file must already be open when this function is called.
 */
long getFilesize(Common::SeekableReadStream *input_file) {
	return input_file->size();
}

/*
 * If the input file is a valid LZW-compressed file, the upper 4 bits of
 * the first byte must be 0, because the first codeword is always a root.
 */
byte mightBeValidCompressedFile(Common::SeekableReadStream *input_file) {
	byte firstByte;
	byte c1, c2, c3;   /* booleans */
	long input_filesize;

	/*  check if the input file has a valid size             */
	/*  the compressed file is made up of 12-bit codewords,  */
	/*  so there are either 0 or 4 bits of wasted space      */
	input_filesize = getFilesize(input_file);
	c1 = (input_filesize * 8) % 12 == 0;
	c2 = (input_filesize * 8 - 4) % 12 == 0;

	// read first byte, and then reset back file pointer
	input_file->seek(0);
	firstByte = input_file->readByte();
	input_file->seek(0);
	c3 = (firstByte >> 4) == 0;

	// check if upper 4 bits are 0
	return ((c1 || c2) && c3);
}

} // End of namespace LZW
} // End of namespace Ultima4
} // End of namespace Ultima
