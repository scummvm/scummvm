/* Copyright (c) 2003-2004 Various contributors
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

#include <stdio.h>

#include "file.h"

namespace MT32Emu {

	bool ANSIFile::open(const char *filename, OpenMode mode) {
		const char *fmode;
		if (mode == OpenMode_read) {
			fmode = "rb";
		} else {
			fmode = "wb";
		}
		fp = fopen(filename, fmode);
		return (fp != NULL);
	}

	void ANSIFile::close() {
		fclose(fp);
	}

	int ANSIFile::readByte() {
		return fgetc(fp);
	}

	size_t ANSIFile::read(void *ptr, size_t size) {
		return fread(ptr, 1, size, fp);
	}

	bool ANSIFile::readLine(char *ptr, size_t size) {
		return fgets(ptr, (int)size, fp) != NULL;
	}

	bool ANSIFile::writeByte(unsigned char out) {
		return fputc(out, fp) != EOF;
	}

	size_t ANSIFile::write(const void *ptr, size_t size) {
		return fwrite(ptr, 1, size, fp);
	}

	bool ANSIFile::isEOF() {
		return feof(fp) != 0;
	}
}
