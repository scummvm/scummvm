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

#include "titanic/compression.h"
#include "common/textconsole.h"

namespace Titanic {

CompressionData::CompressionData() {
	_commandNum = 0;
	_field4 = 0;
	_field8 = 0;
	_fieldC = 0;
	_field10 = 0;
	_field14 = 0;
}

/*------------------------------------------------------------------------*/

Compression::Compression() {
	_srcPtr = nullptr;
	_srcCount = 0;
	_field8 = 0;
	_destPtr = nullptr;
	_destCount = 0;
	_field14 = 0;
	_field18 = 0;
	_compressionData = nullptr;
	_createFn = nullptr;
	_destroyFn = nullptr;
	_field28 = 0;
	_field2C = 0;
	_field30 = 0;
	_field34 = 0;
}

Compression::~Compression() {
	close();
}

void Compression::initDecompress(const char *version, int v) {
	if (!version || *version != '1')
		error("Bad version");
	
	_field18 = 0;
	if (!_createFn) {
		_createFn = &Compression::createMethod;
		_field28 = 0;
	}

	if (!_destroyFn) {
		_destroyFn = &Compression::destroyMethod;
	}

	_compressionData = (this->*_createFn)(_field28, 1);
	_compressionData->_field14 = 0;
	_compressionData->_fieldC = 0;
	if (v < 0) {
		v = -v;
		_compressionData->_fieldC = 1;
	}

	if (v < 8 || v > 15)
		error("Bad parameter");

	_compressionData->_field10 = v;
	_compressionData->_field14 = sub1(_compressionData->_fieldC ? nullptr : &Compression::method3, 1 << v);

	if (_compressionData->_field14)
		sub2();
	else
		close();
}

void Compression::initCompress(const char *version, int v) {
	error("TODO");
}


int Compression::sub1(Method3Fn fn, int v) {
	return 0;
}

void Compression::close() {
	if (_destroyFn)
		(this->*_destroyFn)(_compressionData);
}

CompressionData *Compression::createMethod(int v1, int v2) {
	return new CompressionData();
}

void Compression::destroyMethod(CompressionData *ptr) {
	delete ptr;
}

int Compression::compress(int v) {
	return 0;
}

int Compression::decompress(size_t count) {
/*
	if (!_compressionData || !_srcPtr || !count)
		// Needed fields aren't set
		return -2;

	int result = -5;
	do {
		int ebx = 5;

		switch (_compressionData->_commandNum) {
		case 0:
			if (_compressionData->_field4)
				return result;

		default:
			return -2;
		}
	}*/
	return -2;
}

} // End of namespace Titanic
