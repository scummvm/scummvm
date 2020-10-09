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

#ifndef GLK_QUETZAL
#define GLK_QUETZAL

#include "common/array.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "engines/savestate.h"
#include "glk/blorb.h"
#include "glk/glk_types.h"

namespace Glk {

enum QueztalTag {
	ID_IFSF = MKTAG('I', 'F', 'S', 'F'),
	ID_IFZS = MKTAG('I', 'F', 'Z', 'S'),
	ID_IFhd = MKTAG('I', 'F', 'h', 'd'),
	ID_UMem = MKTAG('U', 'M', 'e', 'm'),
	ID_CMem = MKTAG('C', 'M', 'e', 'm'),
	ID_Stks = MKTAG('S', 't', 'k', 's'),
	ID_SCVM = MKTAG('S', 'C', 'V', 'M')
};

class QuetzalBase {
public:
	static uint32 getInterpreterTag(InterpreterType interpType);
};

/**
 * Quetzal save file reader
 */
class QuetzalReader : public QuetzalBase {
	struct Chunk {
		uint32 _id;
		size_t _offset, _size;
	};
public:
	/**
	 * Iterator for the chunks list
	 */
	struct Iterator {
	private:
		Common::SeekableReadStream *_stream;
		Common::Array<Chunk> &_chunks;
		int _index;
	public:
		/**
		 * Constructor
		 */
		Iterator(Common::SeekableReadStream *stream, Common::Array<Chunk> &chunks, int index) :
			_stream(stream), _chunks(chunks), _index(index) {}

		/**
		 * Deference
		 */
		Chunk &operator*() const { return _chunks[_index]; }

		/**
		 * Incrementer
		 */
		Iterator &operator++() {
			++_index;
			return *this;
		}
		
		/**
		 * Decrementer
		 */
		Iterator &operator--() {
			--_index;
			return *this;
		}

		/**
		 * Equality test
		 */
		bool operator==(const Iterator &rhs) { return _index == rhs._index; }

		/**
		 * Inequality test
		 */
		bool operator!=(const Iterator &rhs) { return _index != rhs._index; }

		/**
		 * Get a read stream for the contents of a chunk
		 */
		Common::SeekableReadStream *getStream() {
			_stream->seek(_chunks[_index]._offset);
			return (_chunks[_index]._size == 0) ?
				new Common::MemoryReadStream((byte *)malloc(0), 0, DisposeAfterUse::YES) :
				_stream->readStream(_chunks[_index]._size);
		}
	};
private:
	Common::SeekableReadStream *_stream;
	Common::Array<Chunk> _chunks;
public:
	/**
	 * Constructor
	 */
	QuetzalReader() : _stream(nullptr) {}

	/**
	 * Clear
	 */
	void clear();

	/**
	 * Opens a Quetzal file for access
	 */
	bool open(Common::SeekableReadStream *stream, uint32 formType = 0);

	/**
	 * Return an iterator for the beginning of the chunks list
	 */
	Iterator begin() { return Iterator(_stream, _chunks, 0); }

	/**
	 * Return an iterator for the beginning of the chunks list
	 */
	Iterator end() { return Iterator(_stream, _chunks, _chunks.size()); }

	/**
	 * Loads a Quetzal save and extracts it's description from an ANNO chunk
	 */
	static bool getSavegameDescription(Common::SeekableReadStream *rs, Common::String &saveName);

	/**
	 * Loads a Quetzal save and extract's it's description and meta info
	 */
	static bool getSavegameMetaInfo(Common::SeekableReadStream *rs, SaveStateDescriptor &ssd);

	/**
	 * Support method for reading a string from a stream
	 */
	static Common::String readString(Common::ReadStream *src);
};

/**
 * Quetzal save file writer
 */
class QuetzalWriter : public QuetzalBase {
	/**
	 * Chunk entry
	 */
	struct Chunk {
		uint32 _id;
		Common::MemoryWriteStreamDynamic _stream;

		/**
		 * Constructor
		 */
		Chunk() : _id(0), _stream(DisposeAfterUse::YES) {}

		/**
		 * Constructor
		 */
		Chunk(uint32 id) : _id(id), _stream(DisposeAfterUse::YES) {}
	};
private:
	Common::Array<Chunk> _chunks;

	/**
	 * Add chunks common to all Glk savegames
	 */
	void addCommonChunks(const Common::String &saveName);
public:
	/**
	 * Clear
	 */
	void clear() { _chunks.clear(); }

	/**
	 * Add a chunk
	 */
	Common::WriteStream &add(uint32 chunkId);

	/**
	 * Save the added chunks to file
	 */
	void save(Common::WriteStream *out, const Common::String &saveName, uint32 formType = ID_IFSF);
};

} // End of namespace Glk

#endif
