/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qdengine/qd_precomp.h"


namespace QDEngine {

XBuffer::XBuffer() {
	makeFree_ = 0;
	buffer_ = NULL;
	alloc(XB_DEFSIZE);
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	automaticRealloc_ = true;
}

XBuffer::XBuffer(unsigned int sz, bool automatic_realloc_) {
	makeFree_ = 0;
	buffer_ = NULL;
	alloc(sz);
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	automaticRealloc_ = automatic_realloc_;
}

void XBuffer::alloc(unsigned int sz) {
	free();
	buffer_ = (char*)malloc(size_ = sz);
	offset_ = 0L;
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	makeFree_ = 1;
	if (size_)
		*buffer_ = 0;
}

XBuffer::XBuffer(void* p, int sz) {
	buffer_ = (char*)p;
	size_ = sz;
	offset_ = 0;
	radix_ = XB_DEFRADIX;
	digits_ = XB_DEFDIGITS;
	makeFree_ = 0;
	automaticRealloc_ = 0;
}

void XBuffer::free() {
	if (makeFree_ && buffer_) {
		::free(buffer_);
		buffer_ = NULL;
	}
}

void XBuffer::fill(char fc) {
	memset(buffer_, fc, size_);
}

void XBuffer::set(int off, int mode) {
	switch (mode) {
	case XB_BEG:
		offset_ = off;
		break;
	case XB_CUR:
		offset_ += off;
		break;
	case XB_END:
		offset_ = size_ - off;
		break;
	}
}

void XBuffer::init() {
	offset_ = 0;
	if (size())
		*buffer_ = 0;
}

unsigned int XBuffer::read(void* s, unsigned int len) {
	memcpy(s, buffer_ + offset_, len);
	offset_ += len;
	return len;
}

void XBuffer::handleOutOfSize() {
	if (automaticRealloc_) {
		buffer_ = (char*)realloc(buffer_, size_ *= 2);
		if (!buffer_) {
			error("Out of XBuffer (low of system memory)");
		}
	} else {
		error("Out of XBuffer");
	}
}

unsigned int XBuffer::write(const void* s, unsigned int len, bool bin_flag) {
	if (bin_flag)
		while (offset_ + len > size_)
			handleOutOfSize();
	else
		while (offset_ + len >= size_)
			handleOutOfSize();

	memcpy(buffer_ + offset_, s, len);
	offset_ += len;

	if (!bin_flag)
		buffer_[offset_] = '\0';

	return len;
}

XBuffer& XBuffer::operator< (const char* v) {
	if (v)
		write(v, strlen(v), 0);
	return *this;
}

XBuffer& XBuffer::operator> (char* v) {
	if (v)
		read(v, strlen(buffer_ + offset_) + 1);
	return *this;
}

const int size = 10000;
static XBuffer buf4AssertsBuffer(size);
XBuffer &assertsBuffer() {
	if (buf4AssertsBuffer.tell() > size * 0.8) {
		buf4AssertsBuffer.init();
		buf4AssertsBuffer < "assertBuffer overflow\r\n";
	}
	return buf4AssertsBuffer;
}
} // namespace QDEngine
