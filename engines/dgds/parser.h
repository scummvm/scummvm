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

#ifndef DGDS_PARSER_H
#define DGDS_PARSER_H

#include "common/types.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/stream.h"

namespace Dgds {

class DgdsChunkReader;
class ResourceManager;
class Decompressor;

class ParserData {};

class DgdsParser {
public:
	// FIXME: Make these private/protected
	uint32 _bytesRead;
	//Common::SeekableReadStream *_file;

public:
	DgdsParser(ResourceManager *resman, Decompressor *decompressor);
	virtual ~DgdsParser() {};
	bool parse(ParserData *data, const Common::String &filename);
	virtual bool handleChunk(DgdsChunkReader &chunk, ParserData *data) { return false; };

	static Common::HashMap<uint16, Common::String> readTags(Common::SeekableReadStream *stream);

protected:
	Common::String _filename;

private:
	ResourceManager *_resman;
	Decompressor *_decompressor;
};

class TTMParser : public DgdsParser {
public:
	TTMParser(ResourceManager *resman, Decompressor *decompressor) : DgdsParser(resman, decompressor) {}

private:
	bool handleChunk(DgdsChunkReader &chunk, ParserData *data) override;
};

class ADSParser : public DgdsParser {
public:
	ADSParser(ResourceManager *resman, Decompressor *decompressor) : DgdsParser(resman, decompressor) {}

private:
	bool handleChunk(DgdsChunkReader &chunk, ParserData *data) override;
};

} // End of namespace Dgds

#endif // DGDS_RESOURCE_H
