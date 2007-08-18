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
 * $URL: $
 * $Id: $
 */

#ifndef BACKENDS_POSIX_FILE_H
#define BACKENDS_POSIX_FILE_H

#include "backends/file/base-file.cpp"

namespace Common {

/**
 * Implements several POSIX specific file related functions used by the Common::File wrapper.
 * 
 * Parts of this class are documented in the base file class, BaseFile.
 */
class POSIXFile : public BaseFile {
public:
	POSIXFile();
	~POSIXFile();
protected:
	void _clearerr(FILE *stream);
	int _fclose(FILE *stream);
	int _feof(FILE *stream) const;
	FILE *_fopen(const char * filename, const char * mode);
	int _fread(void *buffer, size_t obj_size, size_t num, FILE *stream);
	int _fseek(FILE * stream, long int offset, int origin) const;
	long _ftell(FILE *stream) const;
	int _fwrite(const void * ptr, size_t obj_size, size_t count, FILE * stream);
};

}	// End of namespace Common

#endif	//BACKENDS_POSIX_FILE_H
