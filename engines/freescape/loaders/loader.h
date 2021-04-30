#ifndef __Phantasma___Loader__
#define __Phantasma___Loader__

#include "common/array.h"

namespace Freescape {

class StreamLoader {
private:
	Common::Array<uint8>::size_type bytePointer;
	Common::Array<uint8> binary;
	uint8 readMaskByte1;
	uint8 readMaskByte2;

public:
	StreamLoader(Common::Array<uint8> &_binary) {
		binary = _binary;
		bytePointer = 0;
		readMaskByte1 = 0;
		readMaskByte2 = 0;
	}

	uint8 get8() {
		if (!eof()) {
			uint8 sourceByte = binary[bytePointer];

			if (bytePointer & 1)
				sourceByte ^= readMaskByte2;
			else
				sourceByte ^= readMaskByte1;

			bytePointer++;
			return sourceByte;
		} else
			error("eof");

		return 0;
	}

	uint16 get16() {
		uint16 result = (uint16)(get8() << 8);
		result |= get8();
		return result;
	}

	uint16 rget16() {
		uint16 result = (uint16)(get8());
		result |= (get8() << 8);
		return result;
	}

	uint32 get32() {
		uint32 result = (uint32)(get16() << 16);
		result |= get16();
		return result;
	}

	bool eof() {
		return bytePointer >= binary.size();
	}

	void alignPointer() {
		if (bytePointer & 1)
			bytePointer++;
	}

	void skipBytes(Common::Array<uint8>::size_type numberOfBytes) {
		bytePointer += numberOfBytes;
	}

	Common::Array<uint8> *nextBytes(Common::Array<uint8>::size_type numberOfBytes) {
		Common::Array<uint8> *returnBuffer(new Common::Array<uint8>);
		//debug("skiping %d bytes", numberOfBytes);

		while (numberOfBytes--)
			returnBuffer->push_back(get8());

		return returnBuffer;
	}

	Common::Array<uint8>::size_type getFileOffset() {
		return bytePointer;
	}

	void setFileOffset(Common::Array<uint8>::size_type newOffset) {
		bytePointer = newOffset;
	}

	void setReadMask(uint8 byte1, uint8 byte2) {
		readMaskByte1 = byte1;
		readMaskByte2 = byte2;
	}
};

} // namespace Freescape

#endif