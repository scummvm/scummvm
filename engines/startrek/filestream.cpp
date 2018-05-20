#include "startrek/filestream.h"

namespace StarTrek {

FileStream::FileStream(Common::SeekableReadStream *stream, bool bigEndian) {
	_bigEndian = bigEndian;

	_pos = 0;
	_size = stream->size();
	_data = new byte[_size];
	stream->read(_data, _size);
	delete stream;
}

FileStream::~FileStream() {
	delete[] _data;
}

// ReadStream functions

uint32 FileStream::read(void* dataPtr, uint32 dataSize) {
	if (_pos + dataSize > (uint32)size())
		dataSize = size() - _pos;
	memcpy(dataPtr, _data + _pos, dataSize);
	_pos += dataSize;
	return dataSize;
}

byte FileStream::readByte() {
	assert(_pos + 1 <= size());
	return _data[_pos++];
}

uint16 FileStream::readUint16() {
	assert(_pos + 2 <= size());
	uint16 w;
	if (_bigEndian)
		w = _data[_pos + 1] | (_data[_pos] << 8);
	else
		w = _data[_pos] | (_data[_pos + 1] << 8);
	_pos += 2;
	return w;
}

// SeekableReadStream functions

int32 FileStream::pos() const {
	return _pos;
}

int32 FileStream::size() const {
	return _size;
}

bool FileStream::seek(int32 offset, int whence) {
	assert(whence == SEEK_SET);
	_pos = offset;
	return true;
}

}
