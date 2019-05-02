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

#ifndef PINK_ARCHIVE_H
#define PINK_ARCHIVE_H

#include "common/hash-str.h"
#include "common/str-array.h"
#include "common/stream.h"

namespace Common {

class File;

}

namespace Pink {

class Object;

class Archive {
public:
	Archive(Common::SeekableReadStream *stream);
	Archive(Common::WriteStream *stream);

	void mapObject(Object *obj);

	byte readByte() { return _readStream->readByte(); }
	uint32 readDWORD() { return _readStream->readUint32LE(); }
	uint16 readWORD() { return _readStream->readUint16LE(); }

	Common::String readString();
	void skipString();

	Object *readObject();

	void write(const void *dataPtr, uint32 dataSize) { _writeStream->write(dataPtr, dataSize); }
	void writeByte(byte val) { _writeStream->writeByte(val); }
	void writeDWORD(uint32 val) { _writeStream->writeUint32LE(val); }
	void writeWORD(uint16 val) { _writeStream->writeUint16LE(val); }

	void writeString(const Common::String &string);

private:
	uint findObjectId(const char *name);

	Object *parseObject(bool &isCopyReturned);

	Common::Array<Object *> _objectMap;
	Common::Array<uint> _objectIdMap;
	Common::SeekableReadStream *_readStream;
	Common::WriteStream *_writeStream;
};

} // End of namespace Pink

#endif
