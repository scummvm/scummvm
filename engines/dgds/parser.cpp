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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/platform.h"
#include "dgds/includes.h"
#include "dgds/resource.h"
#include "dgds/parser.h"
#include "dgds/scripts.h"
#include "dgds/ttm.h"
#include "dgds/ads.h"

namespace Dgds {

DgdsParser::DgdsParser(ResourceManager *resman, Decompressor *decompressor) :
_resman(resman), _decompressor(decompressor), _bytesRead(0) {
}

bool DgdsParser::parse(ParserData *data, const Common::String &filename) {
	DGDS_EX ex;

	_filename = filename;
	Common::SeekableReadStream *resStream = _resman->getResource(filename);

	if (!resStream) {
		error("Couldn't open script file '%s'", filename.c_str());
		return false;
	}

	uint32 dot = _filename.find('.');
	if (dot != Common::String::npos) {
		ex = MKTAG24(toupper(_filename[dot + 1]), toupper(_filename[dot + 2]), toupper(_filename[dot + 3]));
	} else {
		ex = 0;
	}

	DgdsChunkReader chunk(resStream);
	while (chunk.readNextHeader(ex, _filename)) {
		bool stop;

		if (chunk.isContainer()) {
			stop = handleChunk(chunk, data);
		} else {
			chunk.readContent(_decompressor);
			stop = handleChunk(chunk, data);
		}

		if (stop)
			break;
	}

	delete resStream;
	return true;
}

Common::HashMap<uint16, Common::String> DgdsParser::readTags(Common::SeekableReadStream *stream) {
	Common::HashMap<uint16, Common::String> tags;
	uint16 count = stream->readUint16LE();
	debug(1, "        %u tags:", count);

	for (uint16 i = 0; i < count; i++) {
		uint16 idx = stream->readUint16LE();
		const Common::String tagVal = stream->readString();
		debug(1, "        %2u: %2u, \"%s\"", i, idx, tagVal.c_str());
		// TODO: How to handle when these IDs overlap? (eg, see
		// BBEDROOM.TTM in RotD)
		if (!tags.contains(idx))
			tags[idx] = tagVal;
	}

	return tags;
}


bool TTMParser::handleChunk(DgdsChunkReader &chunk, ParserData *data) {
	TTMEnviro *scriptData = static_cast<TTMEnviro *>(data);

	switch (chunk.getId()) {
	case ID_TTI: // Ignore containers
		break;
	case ID_TT3:
		// Make a memory read stream from the chunk data
		scriptData->scr = chunk.makeMemoryStream();
		break;
	case ID_TAG:
		scriptData->_tags = readTags(chunk.getContent());
		break;
	case ID_VER: // Version - ignore
		break;
	case ID_PAG:
		if (chunk.getSize() != 2) {
			warning("unspected PAG chunk size %d in %s", chunk.getSize(), _filename.c_str());
			break;
		}
		scriptData->_totalFrames = chunk.getContent()->readUint16LE();
		scriptData->_frameOffsets.resize(scriptData->_totalFrames + 1, -1);
		break;
	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk.getId()), chunk.getSize(), _filename.c_str());
		//chunk._contentStream->skip(chunk._size);
		break;
	}
	return false;
}


bool ADSParser::handleChunk(DgdsChunkReader &chunk, ParserData *data) {
	ADSData *scriptData = static_cast<ADSData *>(data);
	Common::SeekableReadStream *chunkStream = chunk.getContent();
	switch (chunk.getId()) {
	case EX_ADS:
	case ID_TTI: // Ignore containers
		break;
	case ID_RES: {
		uint16 count = chunkStream->readUint16LE();

		for (uint16 i = 0; i < count; i++) {
			uint16 idx = chunkStream->readUint16LE();
			//assert(idx == (i + 1)); // not true in Willy Beamish.
			if (idx > scriptData->_scriptNames.size())
				scriptData->_scriptNames.resize(idx);

			scriptData->_scriptNames[idx - 1] = chunkStream->readString();
		}
	} break;
	case ID_SCR:
		scriptData->scr = chunk.makeMemoryStream();
		break;
	case ID_TAG:
		scriptData->_tags = readTags(chunkStream);
		break;
	case ID_VER: // Version - ignore
		break;
	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk.getId()), chunk.getSize(), _filename.c_str());
		break;
	}
	return false;
}

} // End of namespace Dgds
