#include "engines/myst3/archive.h"
#include "common/debug.h"

void Archive::_decryptHeader(Common::SeekableReadStream &inStream, Common::WriteStream &outStream) {
	static const uint32 addKey = 0x3C6EF35F;
	static const uint32 multKey = 0x0019660D;

	inStream.seek(0);
	uint32 encryptedSize = inStream.readUint32LE();
	uint32 decryptedSize = encryptedSize ^ addKey;
	
	inStream.seek(0);
	uint32 currentKey = 0;

	for (uint i = 0; i < decryptedSize; i++) {
		currentKey += addKey;
		outStream.writeUint32LE(inStream.readUint32LE() ^ currentKey);
		currentKey *= multKey;
	}
}

void Archive::readFromStream(Common::SeekableReadStream &inStream) {
	Common::MemoryWriteStreamDynamic buf(true);
	_decryptHeader(inStream, buf);
	
	Common::MemoryReadStream directory(buf.getData(), buf.size());
	directory.skip(sizeof(uint32));
	
	while (directory.pos() < directory.size()) {
		DirectoryEntry entry;
		entry.readFromStream(directory);
		if (entry.hasSubEntries()) {
			_directory.push_back(entry);
		}
	}
}

void Archive::dumpDirectory() {
	for (uint i = 0; i < _directory.size(); i++) {
		_directory[i].dump();
	}
}

void Archive::dumpToFiles(Common::SeekableReadStream &inStream) {
	for (uint i = 0; i < _directory.size(); i++) {
		_directory[i].dumpToFiles(inStream);
	}
}
