#include "engines/myst3/directoryentry.h"
#include "common/debug.h"

void DirectoryEntry::readFromStream(Common::SeekableReadStream &inStream) {
	_index = inStream.readUint16LE();
	_unk = inStream.readByte();
	byte subItemCount = inStream.readByte();
	
	// End of directory marker ?
	if (_unk != 0) {
		subItemCount = 0;
	}

	_subentries.clear();
	for (uint i = 0; i < subItemCount ; i++) {
		DirectorySubEntry subEntry;
		subEntry.readFromStream(inStream);
		_subentries.push_back(subEntry);
	}
}

void DirectoryEntry::dump() {
	debug("index : %d unk: %d subitems : %d", _index, _unk, _subentries.size());

	for (uint i = 0; i < _subentries.size(); i++) {
		_subentries[i].dump();
	}
}

bool DirectoryEntry::hasSubEntries() {
	return !_subentries.empty();
}

void DirectoryEntry::dumpToFiles(Common::SeekableReadStream &inStream) {
	for (uint i = 0; i < _subentries.size(); i++) {
		_subentries[i].dumpToFile(inStream, _index);
	}
}
