#pragma once

namespace Cryo {

class BitReaderBase {
public:
	unsigned char *_data;   //NB! never override this - used by decompressor
	unsigned int _queue;
	unsigned int _queueLeft;
public:
	BitReaderBase(void *data, unsigned int dataSize = 0) {
		_data = static_cast<unsigned char *>(data);
		_queue = _queueLeft = 0;
	}

	unsigned char GetBit() {
		return 0;   // to be overriden
	}
};

// used to decompress HSQ files
class BitReader16 : BitReaderBase {
public:
	unsigned char GetBit() {
		if (!_queueLeft) {
			_queue = (_data[1] << 8) | _data[0];
			_data += 2;
			_queueLeft += 16;
		}
		unsigned char bit = _queue & 1;
		_queue >>= 1;
		_queueLeft--;
		return bit;
	}
};

// used by HNM decoder
class BitReader32 : BitReaderBase {
public:
	unsigned char GetBit() {
		if (!_queueLeft) {
			_queue = (_data[3] << 24) | (_data[2] << 16) | (_data[1] << 8) | _data[0];
			_data += 4;
			_queueLeft += 32;
		}
		unsigned char bit = (_queue >> (_queueLeft - 1)) & 1;
		_queueLeft--;
		return bit;
	}
};

template <class BitReader>
class LempelZivBase : BitReader {
public:
	LempelZivBase(void *input, unsigned int inputSize) : BitReader(input, inputSize)

		unsigned int UnpackBuffer(void *output, unsigned int maxOutputSize) {
		unsigned char *out = static_cast<unsigned char *>(output);
		for (;;) {
			if (GetBit()) {
				*out++ = *_data++;
			} else {
				int length, offset;
				if (GetBit()) {
					length = *_data & 7;
					offset = ((_data[1] << 8) | _data[0]) >> 3;
					_data += 2;
					offset -= 8192;
					if (!length)
						length = *_data++;
					if (!length)
						break;
				} else {
					length = GetBit() * 2 + GetBit();
					offset = *(_data++) - 256;
				}
				length += 2;
				while (length--) {
					*out = *(out + offset);
					out++;
				}
			}
		}
		return out - static_cast<unsigned char *>(output);
	}
};

}
