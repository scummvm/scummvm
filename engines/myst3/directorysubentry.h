#include "common/memstream.h"

class DirectorySubEntry {
	private:
		uint32 _offset;
		uint32 _size;
		uint16 _padding; // Additional data, not padding ?
		byte _face;
		byte _type;

	public:
		void readFromStream(Common::SeekableReadStream &inStream);
		void dump();
		void dumpToFile(Common::SeekableReadStream &inStream, uint16 index);
		Common::MemoryReadStream *dumpToMemory(Common::SeekableReadStream &inStream);
		uint16 getFace() { return _face; }
		uint16 getType() { return _type; }
};
