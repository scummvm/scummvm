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

#ifndef TITANIC_COMPRESSION_H
#define TITANIC_COMPRESSION_H

#include "common/scummsys.h"

namespace Titanic {

class Compression;
class CompressionData;

typedef CompressionData *(Compression::*CompressionCreateFn)(int v1, int v2);
typedef void(Compression::*CompressionDestroyFn)(CompressionData *ptr);
typedef void(Compression::*Method3Fn)();

class CompressionData {
public:
	int _commandNum;
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
public:
	CompressionData();
};

class Compression {
private:
	CompressionData *createMethod(int v1, int v2);

	void destroyMethod(CompressionData *ptr);

	void method3() {
		// TODO
	}

	int sub1(Method3Fn fn, int v);

	void sub2() {}
public:
	byte *_srcPtr;
	int _srcCount;
	int _field8;
	byte *_destPtr;
	int _destCount;
	int _field14;
	const char *_errorMessage;
	CompressionData *_compressionData;
	CompressionCreateFn _createFn;
	CompressionDestroyFn _destroyFn;
	int _field28;
	int _field2C;
	int _field30;
	int _field34;
public:
	Compression();
	~Compression();

	/**
	 * Initialize for decompression
	 */
	void initDecompress(const char *version = "1.0.4", int v = 15);

	/**
	 * Initialize for compression
	 */
	void initCompress(const char *version = "1.0.4", int v = -1);

	void close();

	/**
	 * Compress data
	 */
	int compress(int v);

	/**
	 * Decompress data
	 */
	int decompress(size_t count);
};

} // End of namespace Titanic

#endif /* TITANIC_COMPRESSION_H */
