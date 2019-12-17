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

#include "nuvieDefs.h"

#include "NuvieIOFile.h"

namespace Ultima {
namespace Ultima6 {

NuvieIOFile::NuvieIOFile(): NuvieIO() {
	fp = NULL;
}

NuvieIOFile::~NuvieIOFile() {
	close();
}

bool NuvieIOFile::openWithMode(const char *filename, const char *mode) {
	if (fp) //We already have a file open lets bail.
		return false;

	fp = fopen(filename, mode);

	if (fp == NULL) {
		DEBUG(0, LEVEL_ERROR, "Failed opening '%s'\n", filename);
		return false;
	}

	size = get_filesize();
	pos = 0;

	return true;
}

void NuvieIOFile::close() {
	if (fp != NULL)
		fclose(fp);

	fp = NULL;

	NuvieIO::close();
}

uint32 NuvieIOFile::get_filesize() {
	uint32 file_length;

	fseek(fp, 0, SEEK_END);
	file_length = (uint32)ftell(fp);
	fseek(fp, pos, SEEK_SET);

	return file_length;
}

void NuvieIOFile::seek(uint32 new_pos) {
	if (fp && new_pos <= size) {
		fseek(fp, new_pos, SEEK_SET);
		pos = new_pos;
	}

	return;
}

//NuvieIOFileRead methods
//

bool NuvieIOFileRead::open(const char *filename) {
	return openWithMode(filename, "rb");
}


uint8 NuvieIOFileRead::read1() {
	if (pos >= size)
		return 0;

	pos++;

	return (fgetc(fp));
}

uint16 NuvieIOFileRead::read2() {
	unsigned char b0, b1;

	if (pos > size - 2)
		return 0;

	pos += 2;

	b0 = fgetc(fp);
	b1 = fgetc(fp);

	return (b0 + (b1 << 8));
}

uint32 NuvieIOFileRead::read4() {
	unsigned char b0, b1, b2, b3;
	if (pos > size - 4)
		return 0;

	pos += 4;

	b0 = fgetc(fp);
	b1 = fgetc(fp);
	b2 = fgetc(fp);
	b3 = fgetc(fp);

	return (b0 + (b1 << 8) + (b2 << 16) + (b3 << 24));
}

bool NuvieIOFileRead::readToBuf(unsigned char *buf, uint32 buf_size) {
	if (pos + buf_size > size)
		return false;

	fread(buf, 1, buf_size, fp); // FIX for partial read.

	pos += buf_size;

	return true;
}



// NuvieIOFileWrite
//

bool NuvieIOFileWrite::open(const char *filename) {
	return openWithMode(filename, "wb");
}

bool NuvieIOFileWrite::write1(uint8 src) {
	if (fp == NULL) // || pos >= size-1)
		return false;

	fputc(src, fp);

	pos++;

	if (pos > size)
		size = pos;

	return true;
}

bool NuvieIOFileWrite::write2(uint16 src) {
	if (fp == NULL) // || pos >= size-2)
		return false;

	fputc((uint8)(src & 0xff), fp);
	fputc((uint8)((src >> 8) & 0xff), fp);

	pos += 2;

	if (pos > size)
		size = pos;

	return true;
}

bool NuvieIOFileWrite::write4(uint32 src) {
	if (fp == NULL) // || pos >= size-4)
		return false;

	fputc((uint8)(src & 0xff), fp);
	fputc((uint8)((src >> 8) & 0xff), fp);
	fputc((uint8)((src >> 16) & 0xff), fp);
	fputc((uint8)((src >> 24) & 0xff), fp);

	pos += 4;

	if (pos > size)
		size = pos;

	return true;
}

uint32 NuvieIOFileWrite::writeBuf(const unsigned char *src, uint32 src_size) {
	if (fp == NULL) // || pos + src_size > size)
		return (0);

	pos += src_size;

	if (pos > size)
		size = pos;

	return (fwrite(src, sizeof(unsigned char), src_size, fp));
}

uint32 NuvieIOFileWrite::write(NuvieIO *src) {
	return 0;
}

} // End of namespace Ultima6
} // End of namespace Ultima
