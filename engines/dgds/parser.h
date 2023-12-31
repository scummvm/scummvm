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

namespace Dgds {

class DgdsChunk;
class Decompressor;
class DgdsScriptData;

class DgdsParser {
public:
	Common::String _filename;
	Common::SeekableReadStream &_file;
	uint32 _bytesRead;

	DgdsParser(Common::SeekableReadStream &file, const Common::String &filename);
	void parse(DgdsScriptData *data, Decompressor *decompressor);
	virtual bool callback(DgdsChunk &chunk, DgdsScriptData *data) { return false; }

	static Common::HashMap<uint16, Common::String> readTags(Common::SeekableReadStream *stream);
};

class TTMParser : public DgdsParser {
public:
	TTMParser(Common::SeekableReadStream &file, const Common::String &filename) : DgdsParser(file, filename) {}

private:
	bool callback(DgdsChunk &chunk, DgdsScriptData *data);
};

class ADSParser : public DgdsParser {
public:
	ADSParser(Common::SeekableReadStream &file, const Common::String &filename) : DgdsParser(file, filename) {}

private:
	bool callback(DgdsChunk &chunk, DgdsScriptData *data);
};

} // End of namespace Dgds

#endif // DGDS_RESOURCE_H
