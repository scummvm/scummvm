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


#include "mt32emu.h"

namespace MT32Emu {

bool File::readBit16u(Bit16u *in) {
	Bit8u b[2];
	if (read(&b[0], 2) != 2)
		return false;
	*in = ((b[0] << 8) | b[1]);
	return true;
}

bool File::readBit32u(Bit32u *in) {
	Bit8u b[4];
	if (read(&b[0], 4) != 4)
		return false;
	*in = ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	return true;
}

bool File::writeBit16u(Bit16u out) {
	if (!writeBit8u((Bit8u)((out & 0xFF00) >> 8))) {
		return false;
	}
	if (!writeBit8u((Bit8u)(out & 0x00FF))) {
		return false;
	}
	return true;
}

bool File::writeBit32u(Bit32u out) {
	if (!writeBit8u((Bit8u)((out & 0xFF000000) >> 24))) {
		return false;
	}
	if (!writeBit8u((Bit8u)((out & 0x00FF0000) >> 16))) {
		return false;
	}
	if (!writeBit8u((Bit8u)((out & 0x0000FF00) >> 8))) {
		return false;
	}
	if (!writeBit8u((Bit8u)(out & 0x000000FF))) {
		return false;
	}
	return true;
}

} // End of namespace MT32Emu
