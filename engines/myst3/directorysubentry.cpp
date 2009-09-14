#include "engines/myst3/directorysubentry.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/file.h"

void DirectorySubEntry::readFromStream(Common::SeekableReadStream &inStream) {
	_offset = inStream.readUint32LE();
	_size = inStream.readUint32LE();
	_padding = inStream.readUint16LE();
	_face = inStream.readByte();
	_type = inStream.readByte();

	dump();

	inStream.skip(_padding * sizeof(uint32));
}

void DirectorySubEntry::dump() {
	debug("offset : %x size: %d padding : %d face : %d type : %d", _offset, _size, _padding, _face, _type);
}

void DirectorySubEntry::dumpToFile(Common::SeekableReadStream &inStream, uint16 index) {
	char fileName[255];
	
	switch (_type) {
		case 0:
		case 5:
			sprintf(fileName, "dump/%d-%d.jpg", index, _face);
			break;
		case 1:
			sprintf(fileName, "dump/%d-%d.mask", index, _face);
			break;
		case 8:
			sprintf(fileName, "dump/%d.bik", index);
			break;
		default:
			sprintf(fileName, "dump/%d-%d.%d", index, _face, _type);
			break;
	}
	
	
	debug("Extracted %s", fileName);
	
	Common::DumpFile outFile;
	outFile.open(fileName);
	
	inStream.seek(_offset);
	
	uint8 *buf = new uint8[_size];
	
	inStream.read(buf, _size);
	outFile.write(buf, _size);
	
	delete[] buf;
	
	outFile.close();
}
