#include "startrek/filestream.h"

namespace StarTrek {

FileStream::FileStream(Common::SeekableReadStream *stream, bool bigEndian) : Common::SeekableReadStreamEndian(bigEndian) {
	_stream = stream;
	_bigEndian = bigEndian;
}

FileStream::~FileStream() {
	delete _stream;
}

// ReadStream functions

bool FileStream::eos() const {
	return _stream->eos();
}

uint32 FileStream::read(void* dataPtr, uint32 dataSize) {
	return _stream->read(dataPtr, dataSize);
}

// SeekableReadStream functions

int32 FileStream::pos() const {
	return _stream->pos();
}

int32 FileStream::size() const {
	return _stream->size();
}

bool FileStream::seek(int32 offset, int whence) {
	return _stream->seek(offset, whence);
}

}
