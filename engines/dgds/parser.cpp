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
#include "dgds/resource.h"
#include "dgds/parser.h"
#include "dgds/scripts.h"

namespace Dgds {

DgdsParser::DgdsParser(ResourceManager *resman) : _resman(resman), _file(nullptr), _bytesRead(0) {
}

bool DgdsParser::parse(ParserData *data, const Common::String &filename) {
	DGDS_EX _ex;

	Common::SeekableReadStream *stream = _resman->getResource(filename);

	if (!stream) {
		error("Couldn't open script file '%s'", filename.c_str());
		return false;
	}

	uint32 dot = _filename.find('.');
	if (dot != Common::String::npos) {
		_ex = MKTAG24(toupper(_filename[dot + 1]), toupper(_filename[dot + 2]), toupper(_filename[dot + 3]));
	} else {
		_ex = 0;
	}

	DgdsChunk chunk;
	while (chunk.readHeader(_file, _filename)) {
		bool stop;

		if (chunk._container) {
			chunk._stream = _file;
			stop = handleChunk(chunk, data);
		} else {
			chunk._stream = chunk.getStream(_ex, _file, &_decomp);

			stop = handleChunk(chunk, data);

			int leftover = chunk._stream->size() - chunk._stream->pos();
			chunk._stream->skip(leftover);
			delete chunk._stream;
		}
		if (stop)
			break;
	}

	delete stream;
	return true;
}

Common::HashMap<uint16, Common::String> DgdsParser::readTags(Common::SeekableReadStream *stream) {
	Common::HashMap<uint16, Common::String> tags;
	uint16 count = stream->readUint16LE();
	debug("        %u:", count);

	for (uint16 i = 0; i < count; i++) {
		uint16 idx = stream->readUint16LE();
		Common::String string = stream->readString();
		debug("        %2u: %2u, \"%s\"", i, idx, string.c_str());

		tags[idx] = string;
	}

	return tags;
}


bool TTMParser::handleChunk(DgdsChunk &chunk, ParserData *data) {
	TTMData *scriptData = (TTMData *)data;

	switch (chunk._id) {
	case ID_TTI: // Ignore containers
		break;
	case ID_TT3:
		scriptData->scr = chunk._stream->readStream(chunk._stream->size());
		break;
	case ID_TAG:
		scriptData->_tags = readTags(chunk._stream);
		break;
	case ID_VER:
	case ID_PAG: // Pages - ignore?
		chunk._stream->skip(chunk._size);
		break;
	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk._id), chunk._size, _filename.c_str());
		chunk._stream->skip(chunk._size);
		break;
	}
	return false;
}

bool ADSParser::handleChunk(DgdsChunk &chunk, ParserData *data) {
	ADSData *scriptData = (ADSData *)data;
	switch (chunk._id) {
	case EX_ADS:
	case ID_TTI: // Ignore containers
		break;
	case ID_RES: {
		uint16 count = chunk._stream->readUint16LE();

		scriptData->count = count;
		for (uint16 i = 0; i < count; i++) {
			uint16 idx = chunk._stream->readUint16LE();
			assert(idx == (i + 1));

			Common::String string = chunk._stream->readString();
			scriptData->names.push_back(string);
		}
	} break;
	case ID_SCR:
		scriptData->scr = chunk._stream->readStream(chunk._stream->size());
		break;
	case ID_TAG:
		scriptData->_tags = readTags(chunk._stream);
		break;
	case ID_VER: // Version - ignore
		chunk._stream->skip(chunk._size);
		break;
	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk._id), chunk._size, _filename.c_str());
		chunk._stream->skip(chunk._size);
		break;
	}
	return false;
}

} // End of namespace Dgds
