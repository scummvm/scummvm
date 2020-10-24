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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/platform.h"
#include "dgds/includes.h"
#include "dgds/movies.h"
#include "dgds/resource.h"
#include "dgds/parser.h"

namespace Dgds {

DgdsParser::DgdsParser(Common::SeekableReadStream &file, const char *filename) : _file(file) {
	Common::strlcpy(_filename, filename, sizeof(_filename));
	bytesRead = 0;
}

void DgdsParser::parse(void *data, Decompressor *decompressor) {
	const char *dot;
	DGDS_EX _ex;

	if ((dot = strrchr(_filename, '.'))) {
		_ex = MKTAG24(toupper(dot[1]), toupper(dot[2]), toupper(dot[3]));
	} else {
		_ex = 0;
	}

	DgdsChunk chunk;
	while (chunk.readHeader(*this)) {
		bool stop;

		//chunk._stream = 0;
		if (chunk.container) {
			chunk._stream = &_file;
			stop = callback(chunk, data);
		} else {
			chunk._stream = chunk.isPacked(_ex) ? chunk.decodeStream(*this, decompressor) : chunk.readStream(*this);

			stop = callback(chunk, data);

			int leftover = chunk._stream->size() - chunk._stream->pos();
			chunk._stream->skip(leftover);
			delete chunk._stream;
		}
		if (stop)
			break;
	}
}

bool TTMParser::callback(DgdsChunk &chunk, void *data) {
	TTMData *scriptData = (TTMData *)data;

	switch (chunk._id) {
	case ID_TT3:
		scriptData->scr = chunk._stream->readStream(chunk._stream->size());
		break;
	case ID_VER:
		chunk._stream->skip(chunk._size);
		break;
	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk._id), chunk._size, _filename);
		chunk._stream->skip(chunk._size);
		break;
	}
	return false;
}

bool ADSParser::callback(DgdsChunk &chunk, void *data) {
	ADSData *scriptData = (ADSData *)data;
	switch (chunk._id) {
	case EX_ADS:
		break;
	case ID_RES: {
		uint16 count = chunk._stream->readUint16LE();
		char **strs = new char *[count];
		assert(strs);

		scriptData->count = count;
		for (uint16 i = 0; i < count; i++) {
			Common::String string;
			byte c = 0;
			uint16 idx;

			idx = chunk._stream->readUint16LE();
			assert(idx == (i + 1));

			while ((c = chunk._stream->readByte()))
				string += c;

			strs[i] = new char[string.size() + 1];
			strcpy(strs[i], string.c_str());
		}
		scriptData->names = strs;
	} break;
	case ID_SCR:
		scriptData->scr = chunk._stream->readStream(chunk._stream->size());
		break;
	case ID_VER:
		// These exist in Willy Beamish
		chunk._stream->skip(chunk._size);
		break;
	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk._id), chunk._size, _filename);
		chunk._stream->skip(chunk._size);
		break;
	}
	return false;
}

} // End of namespace Dgds
