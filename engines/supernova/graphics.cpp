#include "common/algorithm.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "graphics.h"

namespace Supernova {

MSNImageDecoder::MSNImageDecoder()
	: _surface(NULL)
	, _palette(NULL)
	, _encodedImage(NULL) {
}

MSNImageDecoder::~MSNImageDecoder() {
	destroy();
}

bool MSNImageDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	
	size_t size = 0;
	size  = (stream.readUint16LE() + 0xF) >> 4;
	size |= (stream.readUint16LE() & 0xF) << 12;
	size += 0x70;    // zus_paragraph
	size *= 16;      // a paragraph is 16 bytes
	_encodedImage = new byte[size];

	_palette = new byte[717];
	g_system->getPaletteManager()->grabPalette(_palette, 16, 240);

	byte pal_diff;
	byte flag = stream.readByte();
	if (flag == 0) {
		pal_diff = 0;
		_palette[141] = 0x38;
		_palette[142] = 0x38;
		_palette[143] = 0x38;
	} else {
		pal_diff = 1;
		for (int i = flag * 3; i != 0; --i) {
			_palette[717 - i] = stream.readByte();
		}
	}
	// 18bit VGA to 24bit CLUT8
	for (size_t i = 0; i < 717; ++i) {
		_palette[i] <<= 2;
	}

	byte numSections = stream.readByte();
	for (size_t i = 0; i < kMaxSections; ++i) {
		_section[i].addressHigh = 0xff;
		_section[i].addressLow = 0xffff;
		_section[i].x2 = 0;
	}
	for (int i = 0; i < numSections; ++i) {
		_section[i].x1 = stream.readUint16LE();
		_section[i].x2 = stream.readUint16LE();
		_section[i].y1 = stream.readByte();
		_section[i].y2 = stream.readByte();
		_section[i].next = stream.readByte();
		_section[i].addressLow = stream.readUint16LE();
		_section[i].addressHigh = stream.readByte();
	}
	
	byte numClickFields = stream.readByte();
	for (int i = 0; i < numClickFields; ++i) {
		_clickField[i].x1 = stream.readUint16LE();
		_clickField[i].x2 = stream.readUint16LE();
		_clickField[i].y1 = stream.readByte();
		_clickField[i].y2 = stream.readByte();
		_clickField[i].next = stream.readByte();
	}
	
	byte zwCodes[256] = {0};
	byte numRepeat = stream.readByte();
	byte numZw = stream.readByte();
	stream.read(zwCodes, numZw);
	numZw += numRepeat;
	
	byte input = 0;
	size_t i = 0;
	// wat
	while (stream.read(&input, 1)) {
		if (input < numRepeat) {
			++input;
			byte value = stream.readByte();
			for (--value; input > 0; --input) {
				_encodedImage[i++] = value;
			}
		} else if (input < numZw) {
			input = zwCodes[input - numRepeat];
			--input;
			_encodedImage[i++] = input;
			_encodedImage[i++] = input;
		} else {
			input -= pal_diff;
			_encodedImage[i++] = input;
		}
	}
	
	return true;
}

bool MSNImageDecoder::loadSection(int section) {
	_surface = new Graphics::Surface;
	_surface->create(320, 200, g_system->getScreenFormat());
	byte *surfacePixels = static_cast<byte *>(_surface->getPixels());
	
	const uint32 kInvalidAddress = 0x00FFFFFF;
	
	size_t image = section;
	if (image < 128) {
		do {
			uint32 offset = (_section[image].addressHigh << 16) + _section[image].addressLow;
			if (offset == kInvalidAddress) {
				return false;
			}
			int width = _section[image].x2 - _section[image].x1 + 1;
			int height = _section[image].y2 - _section[image].y1 + 1;
			uint32 destAddress = 320 * _section[image].y1 + _section[image].x1;
			while (height) {
				Common::copy(_encodedImage + offset, _encodedImage + offset + width, surfacePixels + destAddress);
				offset += width;
				destAddress += 320;
				--height;
			}
			
			image = _section[image].next;
		} while (image != 0);
	} else {
		image -= 128;
		do {
			int width = _section[image].x2 - _section[image].x1 + 1;
			int height = _section[image].y2 - _section[image].y1 + 1;
			uint32 destAddress = 320 * _section[image].y1 + _section[image].x1;
			uint32 offset = (_section[image].addressHigh << 16) + _section[image].addressLow + destAddress;
			while (height) {
				Common::copy(_encodedImage + offset, _encodedImage + offset + width, surfacePixels + destAddress);
				offset += 320;
				destAddress += 320;
				--height;
			}
			
			image = _section[image].next;
		} while (image != 0);
	}
	
	return true;
}

void MSNImageDecoder::destroy() {
	if (_palette) {
		delete[] _palette;
		_palette = NULL;
	}
	if (_surface) {
		_surface->free();
		_surface = NULL;
	}
	if (_encodedImage) {
		delete[] _encodedImage;
		_encodedImage = NULL;
	}
}

}
