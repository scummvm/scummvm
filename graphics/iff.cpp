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
 * $URL$
 * $Id$
 */

#include "graphics/iff.h"
#include "graphics/surface.h"

#include "common/util.h"


namespace Common {

// this really belongs to iff_container.cpp, but we don't want
// to put only this in a source file
char *ID2string(Common::IFF_ID id) {
	static char str[] = "abcd";

	str[0] = (char)(id >> 24 & 0xff);
	str[1] = (char)(id >> 16 & 0xff);
	str[2] = (char)(id >>  8 & 0xff);
	str[3] = (char)(id >>  0 & 0xff);

	return str;
}

}


namespace Graphics {


void fillBMHD(BMHD &bitmapHeader, Common::ReadStream &stream) {

	bitmapHeader.width = stream.readUint16BE();
	bitmapHeader.height = stream.readUint16BE();
	bitmapHeader.x = stream.readUint16BE();
	bitmapHeader.y = stream.readUint16BE();
	bitmapHeader.depth = stream.readByte();
	bitmapHeader.masking = stream.readByte();
	bitmapHeader.pack = stream.readByte();
	bitmapHeader.flags = stream.readByte();
	bitmapHeader.transparentColor = stream.readUint16BE();
	bitmapHeader.xAspect = stream.readByte();
	bitmapHeader.yAspect = stream.readByte();
	bitmapHeader.pageWidth = stream.readUint16BE();
	bitmapHeader.pageHeight = stream.readUint16BE();

}


ILBMDecoder::ILBMDecoder(Common::ReadStream &input, Surface &surface, byte *&colors) : IFFParser(input), _surface(&surface), _colors(&colors) {
	if (_typeId != ID_ILBM)
		error("unsupported IFF subtype '%s'", Common::ID2string(_typeId));
}

void ILBMDecoder::decode() {

	Common::IFFChunk *chunk;
	while ((chunk = nextChunk()) != 0) {
		switch (chunk->id) {
		case ID_BMHD:
			readBMHD(*chunk);
			break;

		case ID_CMAP:
			readCMAP(*chunk);
			break;

		case ID_BODY:
			readBODY(*chunk);
			break;
		}
	}

	return;
}

void ILBMDecoder::readBMHD(Common::IFFChunk &chunk) {

	fillBMHD(_bitmapHeader, chunk);

	_colorCount = 1 << _bitmapHeader.depth;
	*_colors = (byte*)malloc(sizeof(**_colors) * _colorCount * 3);
	_surface->create(_bitmapHeader.width, _bitmapHeader.height, 1);

}

void ILBMDecoder::readCMAP(Common::IFFChunk &chunk) {
	if (*_colors == NULL) {
		error("wrong input chunk sequence");
	}
	for (uint32 i = 0; i < _colorCount; i++) {
		(*_colors)[i * 3 + 0] = chunk.readByte();
		(*_colors)[i * 3 + 1] = chunk.readByte();
		(*_colors)[i * 3 + 2] = chunk.readByte();
	}
}

void ILBMDecoder::readBODY(Common::IFFChunk& chunk) {

	switch (_bitmapHeader.pack) {
	case 0:
		error("unpacked ILBM files are not supported");
		break;

	case 1: {
		uint32 scanWidth = _bitmapHeader.width >> 3;
		byte *scan = (byte*)malloc(scanWidth);
		byte *out = (byte*)_surface->pixels;

		PackBitsReadStream stream(chunk);

		for (uint32 i = 0; i < _bitmapHeader.height; i++) {

			for (uint32 j = 0; j < _bitmapHeader.depth; j++) {
				stream.read(scan, scanWidth);
				fillPlane(out, scan, scanWidth, j);
			}

			out += _bitmapHeader.width;
		}
		free(scan);
		break;
	}

	}
}

void ILBMDecoder::fillPlane(byte *out, byte* buf, uint32 width, uint32 plane) {

	byte src, idx, set;
	byte mask = 1 << plane;

	for (uint32 j = 0; j < _bitmapHeader.width; j++) {
		src = buf[j >> 3];
		idx = 7 - (j & 7);
		set = src & (1 << idx);

		if (set)
			out[j] |= mask;
	}

}




PBMDecoder::PBMDecoder(Common::ReadStream &input, Surface &surface, byte *&colors) : IFFParser(input), _surface(&surface), _colors(&colors) {
	if (_typeId != ID_PBM)
		error("unsupported IFF subtype '%s'", Common::ID2string(_typeId));
}

void PBMDecoder::decode() {

	Common::IFFChunk *chunk;
	while ((chunk = nextChunk()) != 0) {
		switch (chunk->id) {
		case ID_BMHD:
			readBMHD(*chunk);
			break;

		case ID_CMAP:
			readCMAP(*chunk);
			break;

		case ID_BODY:
			readBODY(*chunk);
			break;
		}
	}

	return;
}

void PBMDecoder::readBMHD(Common::IFFChunk &chunk) {

	fillBMHD(_bitmapHeader, chunk);

	_colorCount = 1 << _bitmapHeader.depth;
	*_colors = (byte*)malloc(sizeof(**_colors) * _colorCount * 3);
	_surface->create(_bitmapHeader.width, _bitmapHeader.height, 1);

}

void PBMDecoder::readCMAP(Common::IFFChunk &chunk) {
	if (*_colors == NULL) {
		error("wrong input chunk sequence");
	}
	for (uint32 i = 0; i < _colorCount; i++) {
		(*_colors)[i * 3 + 0] = chunk.readByte();
		(*_colors)[i * 3 + 1] = chunk.readByte();
		(*_colors)[i * 3 + 2] = chunk.readByte();
	}
}

void PBMDecoder::readBODY(Common::IFFChunk& chunk) {

	uint si = 0;

	switch (_bitmapHeader.pack) {
	case 0:
		while (!chunk.hasReadAll()) {
			((byte*)_surface->pixels)[si++] = chunk.readByte();
		}
		break;

	case 1: {
		PackBitsReadStream stream(chunk);
		stream.read((byte*)_surface->pixels, _surface->w * _surface->h);
		break;
	}

	}
}






PackBitsReadStream::PackBitsReadStream(Common::ReadStream &input) : _input(&input), _wStoragePos(_storage), _rStoragePos(_storage) {
}

PackBitsReadStream::~PackBitsReadStream() {
}

bool PackBitsReadStream::eos() const {
	//FIXME: eos definition needs to be changed in parallaction engine
	// which is the only place where this class is used
	return _input->eos() && (_rStoragePos == _wStoragePos);
}

uint32 PackBitsReadStream::read(void *dataPtr, uint32 dataSize) {
	_out = (byte*)dataPtr;
	_outEnd = _out + dataSize;

	feed();
	unpack();
	return _fed + _unpacked;
}

void PackBitsReadStream::store(byte b) {
	if (_out < _outEnd) {
		*_out++ = b;
		_unpacked++;
		_wStoragePos = _storage;
	} else {
		assert(_wStoragePos < _storage + 257);
		*_wStoragePos++ = b;
	}

	_rStoragePos = _storage;
}

void PackBitsReadStream::feed() {
	_fed = 0;

	int len = MIN(_wStoragePos - _rStoragePos, _outEnd - _out);
	if (len == 0) return;

	for (int i = 0; i < len; i++)
		*_out++ = *_rStoragePos++;

	_fed = len;
}

void PackBitsReadStream::unpack() {
	byte byteRun;
	byte idx;

	uint32 i, j;
	_unpacked = 0;

	while (_out < _outEnd && !_input->eos()) {
		byteRun = _input->readByte();
		//FIXME: eos definition needs to be changed in parallaction engine
		// which is the only place where this class is used
		//if (_input->eos()) break;
		if (byteRun <= 127) {
			i = byteRun + 1;
			for (j = 0; j < i; j++) {
				idx = _input->readByte();
				store(idx);
			}
		} else if (byteRun != 128) {
			i = (256 - byteRun) + 1;
			idx = _input->readByte();
			for (j = 0; j < i; j++) {
				store(idx);
			}
		}
	}
}


void decodePBM(Common::ReadStream &input, Surface &surface, byte *&colors) {
	PBMDecoder decoder(input, surface, colors);
	decoder.decode();
}

}
