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

#ifndef DIRECTOR_ARCHIVE_H
#define DIRECTOR_ARCHIVE_H

#include "common/file.h"
#include "common/substream.h"

namespace Common {
class MacResManager;
}

namespace Director {

// Completely ripped off of Mohawk's Archive code

struct Resource {
	uint32 index;
	uint32 offset;
	uint32 size;
	uint32 castId;
	uint32 tag;
	Common::String name;
	Common::Array<Resource> children;
};

class Archive {
public:
	Archive();
	virtual ~Archive();

	virtual bool openFile(const Common::String &fileName);
	virtual bool openStream(Common::SeekableReadStream *stream, uint32 offset = 0) = 0;
	virtual void close();

	Common::String getFileName() const { return _fileName; }

	bool isOpen() const { return _stream != 0; }

	bool hasResource(uint32 tag, int id) const;
	bool hasResource(uint32 tag, const Common::String &resName) const;
	virtual Common::SeekableSubReadStreamEndian *getResource(uint32 tag, uint16 id);
	virtual Resource getResourceDetail(uint32 tag, uint16 id);
	uint32 getOffset(uint32 tag, uint16 id) const;
	uint16 findResourceID(uint32 tag, const Common::String &resName) const;
	Common::String getName(uint32 tag, uint16 id) const;

	Common::Array<uint32> getResourceTypeList() const;
	Common::Array<uint16> getResourceIDList(uint32 type) const;
	bool _isBigEndian;
	static uint32 convertTagToUppercase(uint32 tag);

protected:
	Common::SeekableReadStream *_stream;
	typedef Common::HashMap<uint16, Resource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> TypeMap;
	TypeMap _types;

	Common::String _fileName;
};

class MacArchive : public Archive {
public:
	MacArchive();
	~MacArchive();

	void close();
	bool openFile(const Common::String &fileName);
	bool openStream(Common::SeekableReadStream *stream, uint32 startOffset = 0);
	Common::SeekableSubReadStreamEndian *getResource(uint32 tag, uint16 id);

private:
	Common::MacResManager *_resFork;
};

class RIFFArchive : public Archive {
public:
	RIFFArchive() : Archive() { _startOffset = 0; }
	~RIFFArchive() {}

	bool openStream(Common::SeekableReadStream *stream, uint32 startOffset = 0);
	Common::SeekableSubReadStreamEndian *getResource(uint32 tag, uint16 id);

	uint32 _startOffset;
};

class RIFXArchive : public Archive {
public:
	RIFXArchive() : Archive(){ _isBigEndian = true; }
	~RIFXArchive() {}

	bool openStream(Common::SeekableReadStream *stream, uint32 startOffset = 0);
	Common::SeekableSubReadStreamEndian *getResource(uint32 tag, uint16 id);
	Resource getResourceDetail(uint32 tag, uint16 id);
};

} // End of namespace Director

#endif
