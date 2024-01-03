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

#ifndef DGDS_PARSER_H
#define DGDS_PARSER_H

#include "dgds/decompress.h"

namespace Dgds {

class DgdsChunk;
class ResourceManager;

class ParserData {};

class DgdsParser {
public:
	// FIXME: Make these private/protected
	uint32 _bytesRead;
	Common::SeekableReadStream *_file;

	DgdsParser(ResourceManager *resman);
	bool parse(ParserData *data, const Common::String &filename);
	virtual bool handleChunk(DgdsChunk &chunk, ParserData *data) { return false; };

	static Common::HashMap<uint16, Common::String> readTags(Common::SeekableReadStream *stream);

protected:
	Common::String _filename;

private:
	ResourceManager *_resman;
	Decompressor _decomp;
};

class TTMParser : public DgdsParser {
public:
	TTMParser(ResourceManager *resman) : DgdsParser(resman) {}

private:
	bool handleChunk(DgdsChunk &chunk, ParserData *data) override;
};

class ADSParser : public DgdsParser {
public:
	ADSParser(ResourceManager *resman) : DgdsParser(resman) {}

private:
	bool handleChunk(DgdsChunk &chunk, ParserData *data) override;
};

} // End of namespace Dgds

#endif // DGDS_RESOURCE_H
