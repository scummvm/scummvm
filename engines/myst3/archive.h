#include "engines/myst3/directoryentry.h"
#include "common/stream.h"
#include "common/array.h"

class Archive {
	private:
		Common::Array<DirectoryEntry> _directory;
		void _decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream);

	public:
		void readFromStream(Common::SeekableReadStream &inStream);
		void dumpDirectory();
		void dumpToFiles(Common::SeekableReadStream &inStream);
};
