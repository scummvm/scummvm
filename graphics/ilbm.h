/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#ifndef GRAPHICS_ILBM_H
#define GRAPHICS_ILBM_H

namespace Graphics {

void decodeILBM(Common::ReadStream &input, Surface &surface, byte *&colors);

typedef uint32 IFF_ID;

struct Chunk {
	IFF_ID id;
	uint32 size;
	uint32 bytesRead;
	Common::ReadStream *_input;

	Chunk(Common::ReadStream *input): _input(input) {
		size = bytesRead = 0;
	}

	void incBytesRead(uint32 inc) {
		bytesRead += inc;
		if (bytesRead > size) {
			error("Chunk overead");
		}
	}

	void readHeader() {
		id = _input->readUint32BE();
		size = _input->readUint32BE();
		bytesRead = 0;
	}

	bool eos() {
		return (size - bytesRead) == 0;
	}

	void feed() {
		if (size % 2) {
			size++;
		}
		while (!_input->eos() && !eos()) {
			readByte();
		}
	}

	byte readByte() {
		incBytesRead(1);
		return _input->readByte();
	}

	int8 readSByte() {
		incBytesRead(1);
		return _input->readSByte();
	}

	uint16 readUint16() {
		incBytesRead(2);
		return _input->readUint16BE();
	}

	uint32 readUint32() {
		incBytesRead(4);
		return _input->readUint32BE();
	}

	int16 readSint16() {
		return (int16)readUint16();
	}

	int32 readSint32() {
		return (int32)readUint32();
	}
};

struct BMHD {
	uint16 width, height;
	uint16 x, y;
	byte depth;
	byte masking;
	byte pack;
	byte flags;
	uint16 transparentColor;
	byte xAspect, yAspect;
	uint16 pageWidth, pageHeight;

	BMHD() {
		memset(this, 0, sizeof(*this));
	}
};

class IFFDecoder {
public:
	IFFDecoder(Common::ReadStream &input);
	virtual ~IFFDecoder() {}

	virtual void decode(Surface &surface, byte *&colors);

protected:
	Chunk 	_formChunk;
	Chunk 	_chunk;

	IFF_ID 	_typeId;
	BMHD 	_bitmapHeader;
	uint32 	_colorCount;

	Surface *_surface;
	byte    *_colors;

	virtual bool isTypeSupported(IFF_ID type) = 0;
	virtual void readBODY() = 0;

	virtual void readBMHD();
	virtual void readCMAP();
};

class PBMDecoder : public IFFDecoder {
public:
	PBMDecoder(Common::ReadStream &input) : IFFDecoder(input) {}
protected:
	bool isTypeSupported(IFF_ID type);
	void readBody();
};

}	// End of namespace Graphics

#endif

