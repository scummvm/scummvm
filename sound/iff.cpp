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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sound/iff.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Audio {


void A8SVXDecoder::readVHDR(Common::IFFChunk &chunk) {
	_header.oneShotHiSamples = chunk.readUint32BE();
	_header.repeatHiSamples = chunk.readUint32BE();
	_header.samplesPerHiCycle = chunk.readUint32BE();
	_header.samplesPerSec = chunk.readUint16BE();
	_header.octaves = chunk.readByte();
	_header.compression = chunk.readByte();
	_header.volume = chunk.readUint32BE();
}

void A8SVXDecoder::readBODY(Common::IFFChunk &chunk) {

	switch (_header.compression) {
	case 0:
		_dataSize = chunk.size;
		_data = (byte*)malloc(_dataSize);
		chunk.read(_data, _dataSize);
		break;

	case 1:
		warning("compressed IFF audio is not supported");
		break;
	}

}


A8SVXDecoder::A8SVXDecoder(Common::ReadStream &input, Voice8Header &header, byte *&data, uint32 &dataSize) :
	IFFParser(input), _header(header), _data(data), _dataSize(dataSize) {
	if (_typeId != ID_8SVX)
		error("unknown audio format");
}

void A8SVXDecoder::decode() {

	Common::IFFChunk *chunk;

	while ((chunk = nextChunk()) != 0) {
		switch (chunk->id) {
		case ID_VHDR:
			readVHDR(*chunk);
			break;

		case ID_BODY:
			readBODY(*chunk);
			break;
		}
	}
}

}
