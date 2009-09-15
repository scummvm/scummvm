#include "engines/myst3/directorysubentry.h"
#include "common/stream.h"
#include "common/array.h"

class DirectoryEntry {
	private:
		uint16 _index;
		uint8 _unk;
		Common::Array<DirectorySubEntry> _subentries;

	public:
		void readFromStream(Common::SeekableReadStream &inStream);
		void dump();
		void dumpToFiles(Common::SeekableReadStream &inStream);
		Common::MemoryReadStream *dumpToMemory(Common::SeekableReadStream &inStream, uint16 face, uint16 type);
		bool hasSubEntries();
		uint16 getIndex() { return _index; }
};
