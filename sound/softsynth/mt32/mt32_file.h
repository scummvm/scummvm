/* Copyright (c) 2003-2005 Various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MT32EMU_FILE_H
#define MT32EMU_FILE_H

#include "common/scummsys.h"

namespace MT32Emu {

class File {
public:
	enum OpenMode {
		OpenMode_read  = 0,
		OpenMode_write = 1
	};
	virtual ~File() {}
	virtual void close() = 0;
	virtual size_t read(void *in, size_t size) = 0;
	virtual bool readBit8u(Bit8u *in) = 0;
	virtual bool readBit16u(Bit16u *in);
	virtual bool readBit32u(Bit32u *in);
	virtual size_t write(const void *out, size_t size) = 0;
	virtual bool writeBit8u(Bit8u out) = 0;
	// Note: May write a single byte to the file before failing
	virtual bool writeBit16u(Bit16u out);
	// Note: May write some (<4) bytes to the file before failing
	virtual bool writeBit32u(Bit32u out);
	virtual bool isEOF() = 0;
};

} // End of namespace MT32Emu

#endif
