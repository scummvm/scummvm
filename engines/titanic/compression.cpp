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
	_errorMessage = nullptr;
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
	
	_errorMessage = nullptr;
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
	if (!_compressionData || !_srcPtr || !count)
		// Needed fields aren't set
		return -2;

	int result = -5;
	int ebx = 5;
	uint v;

	for (;;) {
		switch (_compressionData->_commandNum) {
		case 0:
			if (!_srcCount)
				return result;

			result = 0;
			--_srcCount;
			++_field8;
			_compressionData->_field4 = *_srcPtr++;

			if ((_compressionData->_field4 & 0xf) == 8) {
				_compressionData->_commandNum = 13;
				_compressionData->_field4 = ebx;
				_errorMessage = "unknown compression method";
			} else {
				if ((_compressionData->_field4 / 16 + 8) > _compressionData->_field10) {
					_compressionData->_commandNum = 13;
					_compressionData->_field4 = ebx;
					_errorMessage = "invalid window size";
				} else {
					_compressionData->_commandNum = 1;
				}
			}
			break;

		case 1:
			if (!_srcCount)
				return result;

			result = 0;
			--_srcCount;
			++_field8;
			v = *_srcPtr++;
			if ((_compressionData->_field4 * 256 + v) % 31) {
				_compressionData->_commandNum = 13;
				_compressionData->_field4 = ebx;
				ebx = 5;
				_errorMessage = "incorrect header check";
			} else if (!(v & 0x20)) {
				_compressionData->_commandNum = 7;
				ebx = 5;
			} else {
				_compressionData->_commandNum = 2;
				ebx = 5;
			}
			break;

		case 2:
			if (!_srcCount)
				return result;

			result = 0;
			--_srcCount;
			++_field8;
			_compressionData->_field8 = (uint)*_srcPtr++ << 24;
			_compressionData->_commandNum = 3;
			break;

		case 3:
			if (!_srcCount)
				return result;

			result = 0;
			--_srcCount;
			++_field8;
			_compressionData->_field8 += (uint)*_srcPtr++ << 16;
			_compressionData->_commandNum = 4;
			break;

		case 4:
			if (!_srcCount)
				return result;

			result = 0;
			--_srcCount;
			++_field8;
			_compressionData->_field8 = (uint)*_srcPtr++ << 8;
			_compressionData->_commandNum = ebx;
			break;

		case 5:
			if (!_srcCount)
				return result;

			--_srcCount;
			++_field8;
			_compressionData->_field8 += *_srcPtr++;
			_compressionData->_commandNum = ebx;
			_field30 = _compressionData->_field8;
			_compressionData->_commandNum = 6;
			return 2;

		case 6:
			_compressionData->_commandNum = 13;
			_compressionData->_field4 = 0;
			_errorMessage = "need dictionary";
			return -2;

		case 7:
			error("TODO");
			break;

		case 8:
			if (!_srcCount)
				return result;

			--_srcCount;
			++_field8;
			_compressionData->_field8 += *_srcPtr++ << 24;
			_compressionData->_commandNum = 9;
			break;

		case 9:
			if (!_srcCount)
				return result;

			--_srcCount;
			++_field8;
			_compressionData->_field8 += *_srcPtr++ << 16;
			_compressionData->_commandNum = 10;
			break;

		case 10:
			if (!_srcCount)
				return result;

			--_srcCount;
			++_field8;
			_compressionData->_field8 += *_srcPtr++ << 8;
			_compressionData->_commandNum = 11;
			break;

		case 11:
			if (!_srcCount)
				return result;

			--_srcCount;
			++_field8;
			_compressionData->_field8 += *_srcPtr++ << 8;

			if (_compressionData->_field4 == _compressionData->_field8) {
				_compressionData->_commandNum = 12;
			} else {
				_compressionData->_commandNum = 13;
				_compressionData->_field4 = ebx;
				_errorMessage = "incorrect data check";				
			}
			break;

		case 12:
			return 1;

		case 13:
			return -3;

		default:
			return -2;
		}
	}
}

} // End of namespace Titanic
