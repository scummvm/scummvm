/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Player for Kyrandia 3 VQA movies, based on the information found at
// http://multimedia.cx/VQA_INFO.TXT
//
// The benchl.vqa movie (or whatever it is) is not supported. It does not have
// a FINF chunk.
//
// The jung2.vqa movie does not work. The offset to the first frame is strange,
// so we don't find the palette.

#include "common/stdafx.h"
#include "common/system.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "kyra/sound.h"
#include "kyra/screen.h"
#include "kyra/vqa.h"

namespace Kyra {

VQAMovie::VQAMovie(KyraEngine *vm, OSystem *system) {
	_system = system;
	_vm = vm;
	_opened = false;
	_x = _y = _drawPage = -1;
}

VQAMovie::~VQAMovie() {
	close();
}

void VQAMovie::initBuffers() {
	for (int i = 0; i < ARRAYSIZE(_buffers); i++) {
		_buffers[i].data = NULL;
		_buffers[i].size = 0;
	}
}

void *VQAMovie::allocBuffer(int num, uint32 size) {
	assert(num >= 0 && num < ARRAYSIZE(_buffers));
	assert(size > 0);

	if (size > _buffers[num].size) {
		/*
		 * We could use realloc() here, but we don't actually need the
		 * old contents of the buffer.
		 */
		free(_buffers[num].data);
		_buffers[num].data = malloc(size);
		_buffers[num].size = size;
	}

	assert(_buffers[num].data);

	return _buffers[num].data;
}

void VQAMovie::freeBuffers() {
	for (int i = 0; i < ARRAYSIZE(_buffers); i++) {
		free(_buffers[i].data);
		_buffers[i].data = NULL;
		_buffers[i].size = 0;
	}
}

uint32 VQAMovie::readTag() {
	// Some tags have to be on an even offset, so they are padded with a
	// zero byte. Skip that.

	uint32 tag = _file.readUint32BE();

	if (!(tag & 0xFF000000)) {
		tag = (tag << 8) | _file.readByte();
	}

	return tag;
}

// Chunk types ending in a 'Z' are decoded using this function.

int VQAMovie::decodeFormat80(byte *inbuf, byte *outbuf) {
	byte *src = inbuf;
	byte *dst = outbuf;

	while (1) {
		int relPos, pos;
		int count;
		byte color;
		int i;

		byte command = *src++;

		switch (command) {
		case 0x80:
			return dst - outbuf;

		case 0xFF:
			/* 11111111 <count> <pos> */
			count = src[0] | (src[1] << 8);
			pos = src[2] | (src[3] << 8);
			src += 4;
			for (i = 0; i < count; i++)
				dst[i] = outbuf[i + pos];
			break;

		case 0xFE:
			/* 11111110 <count> <color> */
			count = src[0] | (src[1] << 8);
			color = src[2];
			src += 3;
			memset(dst, color, count);
			break;

		default:
			if (command >= 0xC0) {
				/* 11 <count - 3> <pos> */
				count = (command & 0x3F) + 3;
				pos = src[0] | (src[1] << 8);
				src += 2;
				for (i = 0; i < count; i++)
					dst[i] = outbuf[pos + i];
			} else if (command >= 0x80) {
				/* 10 <count> */
				count = command & 0x3F;
				memcpy(dst, src, count);
				src += count;
			} else {
				/* 0 <count - 3> <relative pos> */
				count = ((command & 0x70) >> 4) + 3;
				relPos = ((command & 0x0F) << 8) | src[0];
				src++;
				for (i = 0; i < count; i++)
					dst[i] = dst[i - relPos];
			}
			break;
		}

		dst += count;
	}
}

inline int16 clip8BitSample(int16 sample) {
	if (sample > 255)
		return 255;
	if (sample < 0)
		return 0;
	return sample;
}

void VQAMovie::decodeSND1(byte *inbuf, uint32 insize, byte *outbuf, uint32 outsize) {
	const int8 WSTable2Bit[] = { -2, -1, 0, 1 };
	const int8 WSTable4Bit[] = {
		-9, -8, -6, -5, -4, -3, -2, -1,
		 0,  1,  2,  3,  4,  5,  6,  8
	};

	byte code;
	int8 count;
	uint16 input;

	int16 curSample = 0x80;

	while (outsize > 0) {
		input = *inbuf++ << 2;
		code = (input >> 8) & 0xff;
		count = (input & 0xff) >> 2;

		switch (code) {
		case 2:
			if (count & 0x20) {
				/* NOTE: count is signed! */
				count <<= 3;
				curSample += (count >> 3);
				*outbuf++ = curSample;
				outsize--;
			} else {
				for (; count >= 0; count--) {
					*outbuf++ = *inbuf++;
					outsize--;
				}
				curSample = *(outbuf - 1);
			}
			break;
		case 1:
			for (; count >= 0; count--) {
				code = *inbuf++;

				curSample += WSTable4Bit[code & 0x0f];
				curSample = clip8BitSample(curSample);
				*outbuf++ = curSample;

				curSample += WSTable4Bit[code >> 4];
				curSample = clip8BitSample(curSample);
				*outbuf++ = curSample;

				outsize -= 2;
			}
			break;
		case 0:
			for (; count >= 0; count--) {
				code = *inbuf++;

				curSample += WSTable2Bit[code & 0x03];
				curSample = clip8BitSample(curSample);
				*outbuf++ = curSample;

				curSample += WSTable2Bit[(code >> 2) & 0x03];
				curSample = clip8BitSample(curSample);
				*outbuf++ = curSample;

				curSample += WSTable2Bit[(code >> 4) & 0x03];
				curSample = clip8BitSample(curSample);
				*outbuf++ = curSample;

				curSample += WSTable2Bit[(code >> 6) & 0x03];
				curSample = clip8BitSample(curSample);
				*outbuf++ = curSample;

				outsize -= 4;
			}
			break;
		default:
			for (; count >= 0; count--) {
				*outbuf++ = curSample;
				outsize--;
			}
		}
	}
}

void VQAMovie::open(const char *filename) {
	debugC(9, kDebugLevelMovie, "VQAMovie::open('%s')", filename);
	close();

	if (!_file.open(filename))
		return;

	if (_file.readUint32BE() != MKID_BE('FORM')) {
		warning("VQAMovie::open: Cannot find `FORM' tag");
		return;
	}

	// For now, we ignore the size of the FORM chunk.
	_file.readUint32BE();

	if (_file.readUint32BE() != MKID_BE('WVQA')) {
		warning("WQAMovie::open: Cannot find `WVQA' tag");
		return;
	}

	bool foundHeader = false;
	bool foundFrameInfo = false;

	// The information we need is stored in two chunks: VQHD and FINF. We
	// need both of them before we can begin decoding the movie.

	while (!foundHeader || !foundFrameInfo) {
		uint32 tag = readTag();
		uint32 size = _file.readUint32BE();

		switch (tag) {
		case MKID_BE('VQHD'):	// VQA header
			_header.version     = _file.readUint16LE();
			_header.flags       = _file.readUint16LE();
			_header.numFrames   = _file.readUint16LE();
			_header.width       = _file.readUint16LE();
			_header.height      = _file.readUint16LE();
			_header.blockW      = _file.readByte();
			_header.blockH      = _file.readByte();
			_header.frameRate   = _file.readByte();
			_header.cbParts     = _file.readByte();
			_header.colors      = _file.readUint16LE();
			_header.maxBlocks   = _file.readUint16LE();
			_header.unk1        = _file.readUint32LE();
			_header.unk2        = _file.readUint16LE();
			_header.freq        = _file.readUint16LE();
			_header.channels    = _file.readByte();
			_header.bits        = _file.readByte();
			_header.unk3        = _file.readUint32LE();
			_header.unk4        = _file.readUint16LE();
			_header.maxCBFZSize = _file.readUint32LE();
			_header.unk5        = _file.readUint32LE();

			// Version 1 VQA files have some implicit defaults

			if (_header.version == 1) {
				if (_header.flags & 1) {
					if (_header.freq == 0)
						_header.freq = 22050;
					if (_header.channels == 0)
						_header.channels = 1;
					if (_header.bits == 0)
						_header.bits = 8;
				}
			}

			setX((Screen::SCREEN_W - _header.width) / 2);
			setY((Screen::SCREEN_H - _header.height) / 2);

			// HACK: I've only seen 8-bit mono audio in Kyra 3

			assert(_header.bits == 8);
			assert(_header.channels == 1);

			_frameInfo = new uint32[_header.numFrames];
			_frame = new byte[_header.width * _header.height];

			size = 0xf00 * _header.blockW * _header.blockH;
			_codeBook = new byte[size];
			_partialCodeBook = new byte[size];
			memset(_codeBook, 0, size);
			memset(_partialCodeBook, 0, size);

			_numVectorPointers = (_header.width / _header.blockW) * (_header.height * _header.blockH);
			_vectorPointers = new uint16[_numVectorPointers];
			memset(_vectorPointers, 0, _numVectorPointers * sizeof(uint16));

			_partialCodeBookSize = 0;
			_numPartialCodeBooks = 0;

			if (_header.flags & 1) {
				// A 2-second buffer ought to be enough
				_stream = Audio::makeAppendableAudioStream(_header.freq, Audio::Mixer::FLAG_UNSIGNED, 2 * _header.freq * _header.channels);
				_sound = new Audio::SoundHandle;
			} else {
				_sound = NULL;
				_stream = NULL;
			}

			foundHeader = true;
			break;

		case MKID_BE('FINF'):	// Frame info
			if (!foundHeader) {
				warning("VQAMovie::open: Found `FINF' before `VQHD'");
				return;
			}

			if (size != 4 * (uint32)_header.numFrames) {
				warning("VQAMovie::open: Expected size %d for `FINF' chunk, but got %d", 4 * _header.numFrames, size);
				return;
			}

			foundFrameInfo = true;

			for (int i = 0; i < _header.numFrames; i++) {
				_frameInfo[i] = 2 * _file.readUint32LE();
			}
			break;

		default:
			warning("VQAMovie::open: Unknown tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
			_file.seek(size, SEEK_CUR);
			break;
		}
	}

	initBuffers();

	_opened = true;
}

void VQAMovie::close() {
	debugC(9, kDebugLevelMovie, "VQAMovie::close()");
	if (_opened) {
		delete [] _frameInfo;
		delete [] _frame;
		delete [] _codeBook;
		delete [] _partialCodeBook;
		delete [] _vectorPointers;

		if (_sound) {
			_vm->_mixer->stopHandle(*_sound);
			delete _sound;
			_stream = NULL;
			_sound = NULL;
		}

		_frameInfo = NULL;
		_frame = NULL;
		_codeBook = NULL;
		_partialCodeBook = NULL;
		_vectorPointers = NULL;
		_stream = NULL;

		if (_file.isOpen())
			_file.close();

		freeBuffers();

		_opened = false;
	}
}

void VQAMovie::displayFrame(int frameNum) {
	debugC(9, kDebugLevelMovie, "VQAMovie::displayFrame(%d)", frameNum);
	if (frameNum >= _header.numFrames || !_opened)
		return;

	bool foundSound = _stream ? false : true;
	bool foundFrame = false;
	uint i;

	_file.seek(_frameInfo[frameNum] & 0x7FFFFFFF);

	while (!foundSound || !foundFrame) {
		uint32 tag = readTag();
		uint32 size = _file.readUint32BE();

		if (_file.eof()) {
			// This happens at the last frame. Apparently it has
			// no sound?
			break;
		}

		byte *inbuf, *outbuf;
		uint32 insize, outsize;
		byte *pal;
		uint32 end;

		switch (tag) {
		case MKID_BE('SND0'):	// Uncompressed sound
			foundSound = true;
			inbuf = (byte *)allocBuffer(0, size);
			_file.read(inbuf, size);
			_stream->append(inbuf, size);
			break;

		case MKID_BE('SND1'):	// Compressed sound, almost like AUD
			foundSound = true;
			outsize = _file.readUint16LE();
			insize = _file.readUint16LE();

			inbuf = (byte *)allocBuffer(0, insize);
			_file.read(inbuf, insize);

			if (insize == outsize) {
				_stream->append(inbuf, insize);
			} else {
				outbuf = (byte *)allocBuffer(1, outsize);
				decodeSND1(inbuf, insize, outbuf, outsize);
				_stream->append(outbuf, outsize);
			}
			break;

		case MKID_BE('SND2'):	// Compressed sound
			foundSound = true;
			warning("VQAMovie::displayFrame: `SND2' is not implemented");
			_file.seek(size, SEEK_CUR);
			break;

		case MKID_BE('VQFR'):
			foundFrame = true;
			end = _file.pos() + size - 8;

			while (_file.pos() < end) {
				tag = readTag();
				size = _file.readUint32BE();

				switch (tag) {
				case MKID_BE('CBF0'):	// Full codebook
					_file.read(_codeBook, size);
					break;

				case MKID_BE('CBFZ'):	// Full codebook
					inbuf = (byte *)allocBuffer(0, size);
					_file.read(inbuf, size);
					decodeFormat80(inbuf, _codeBook);
					break;

				case MKID_BE('CBP0'):	// Partial codebook
					_compressedCodeBook = false;
					_file.read(_partialCodeBook + _partialCodeBookSize, size);
					_partialCodeBookSize += size;
					_numPartialCodeBooks++;
					break;

				case MKID_BE('CBPZ'):	// Partial codebook
					_compressedCodeBook = true;
					_file.read(_partialCodeBook + _partialCodeBookSize, size);
					_partialCodeBookSize += size;
					_numPartialCodeBooks++;
					break;

				case MKID_BE('CPL0'):	// Palette
					assert(size <= 3 * 256);

					inbuf = (byte *)allocBuffer(0, size);
					pal = _palette;
					_file.read(inbuf, size);

					for (i = 0; i < size / 3; i++) {
						*pal++ = *inbuf++;
						*pal++ = *inbuf++;
						*pal++ = *inbuf++;
					}

					break;

				case MKID_BE('CPLZ'):	// Palette
					inbuf = (byte *)allocBuffer(0, size);
					outbuf = (byte *)allocBuffer(1, 3 * 256);
					pal = _palette;
					_file.read(inbuf, size);
					size = decodeFormat80(inbuf, outbuf);

					for (i = 0; i < size / 3; i++) {
						*pal++ = *outbuf++;
						*pal++ = *outbuf++;
						*pal++ = *outbuf++;
					}

					break;

				case MKID_BE('VPT0'):	// Frame data
					assert(size / 2 <= _numVectorPointers);

					for (i = 0; i < size / 2; i++)
						_vectorPointers[i] = _file.readUint16LE();
					break;

				case MKID_BE('VPTZ'):	// Frame data
					inbuf = (byte *)allocBuffer(0, size);
					outbuf = (byte *)allocBuffer(1, 2 * _numVectorPointers);

					_file.read(inbuf, size);
					size = decodeFormat80(inbuf, outbuf);

					assert(size / 2 <= _numVectorPointers);

					for (i = 0; i < size / 2; i++)
						_vectorPointers[i] = READ_LE_UINT16(outbuf + 2 * i);
					break;

				default:
					warning("VQAMovie::displayFrame: Unknown `VQFR' sub-tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
					_file.seek(size, SEEK_CUR);
					break;
				}

			}

			break;

		default:
			warning("VQAMovie::displayFrame: Unknown tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
			_file.seek(size, SEEK_CUR);
			break;
		}
	}

	// The frame has been decoded

	if (_frameInfo[frameNum] & 0x80000000) {
		_vm->screen()->setScreenPalette(_palette);
	}

	int blockPitch = _header.width / _header.blockW;

	for (int by = 0; by < _header.height / _header.blockH; by++) {
		for (int bx = 0; bx < blockPitch; bx++) {
			byte *dst = _frame + by * _header.width * _header.blockH + bx * _header.blockW;
			int val = _vectorPointers[by * blockPitch + bx];

			if ((val & 0xFF00) == 0xFF00) {
				// Solid color
				for (i = 0; i < _header.blockH; i++) {
					memset(dst, 255 - (val & 0xFF), _header.blockW);
					dst += _header.width;
				}
			} else {
				// Copy data from _vectorPointers. I'm not sure
				// why we don't use the three least significant
				// bits of 'val'.
				byte *src = &_codeBook[(val >> 3) * _header.blockW * _header.blockH];

				for (i = 0; i < _header.blockH; i++) {
					memcpy(dst, src, _header.blockW);
					src += _header.blockW;
					dst += _header.width;
				}
			}
		}
	}

	if (_numPartialCodeBooks == _header.cbParts) {
		if (_compressedCodeBook) {
			decodeFormat80(_partialCodeBook, _codeBook);
		} else {
			memcpy(_codeBook, _partialCodeBook, _partialCodeBookSize);
		}
		_numPartialCodeBooks = 0;
		_partialCodeBookSize = 0;
	}

	_vm->screen()->copyBlockToPage(_drawPage, _x, _y, _header.width, _header.height, _frame);
}

void VQAMovie::play() {
	uint32 startTick;

	if (!_opened)
		return;

	startTick = _system->getMillis();

	// First, handle any sound chunk that apears before the first frame.
	// At least in some versions, it will contain half a second of audio,
	// presumably to lessen the risk of audio underflow.
	//
	// In most movies, we will find a CMDS tag. The purpose of this is
	// currently unknown.
	//
	// In cow1_0.vqa, cow1_1.vqa, jung0.vqa, and jung1.vqa we will find a
	// VQFR tag. A frame before the first frame? Weird. It doesn't seem to
	// be needed, though.

	byte *inbuf, *outbuf;
	uint32 insize, outsize;

	if (_stream) {
		while (_file.pos() < (_frameInfo[0] & 0x7FFFFFFF)) {
			uint32 tag = readTag();
			uint32 size = _file.readUint32BE();

			if (_file.eof()) {
				warning("VQAMovie::play: Unexpected EOF");
				break;
			}

			switch (tag) {
			case MKID_BE('SND0'):	// Uncompressed sound
				inbuf = (byte *)allocBuffer(0, size);
				_file.read(inbuf, size);
				_stream->append(inbuf, size);
				break;

			case MKID_BE('SND1'):	// Compressed sound
				outsize = _file.readUint16LE();
				insize = _file.readUint16LE();

				inbuf = (byte *)allocBuffer(0, insize);
				_file.read(inbuf, insize);

				if (insize == outsize) {
					_stream->append(inbuf, insize);
				} else {
					outbuf = (byte *)allocBuffer(1, outsize);
					decodeSND1(inbuf, insize, outbuf, outsize);
					_stream->append(outbuf, outsize);
				}
				break;

			case MKID_BE('SND2'):	// Compressed sound
				warning("VQAMovie::play: `SND2' is not implemented");
				_file.seek(size, SEEK_CUR);
				break;

			default:
				warning("VQAMovie::play: Unknown tag `%c%c%c%c'", (tag >> 24) & 0xFF, (tag >> 16) & 0xFF, (tag >> 8) & 0xFF, tag & 0xFF);
				_file.seek(size, SEEK_CUR);
				break;
			}
		}
	}

	_vm->_mixer->playInputStream(Audio::Mixer::kSFXSoundType, _sound, _stream);

	for (int i = 0; i < _header.numFrames; i++) {
		displayFrame(i);

		// TODO: We ought to sync this to how much sound we've played.
		// TODO: Implement frame skipping?

		while (_system->getMillis() < startTick + (i * 1000) / _header.frameRate) {
			OSystem::Event event;

			while (_system->pollEvent(event)) {
				switch (event.type) {
				case OSystem::EVENT_KEYDOWN:
					if (event.kbd.ascii == 27)
						return;
					break;
				case OSystem::EVENT_QUIT:
					_vm->quitGame();
					return;
				default:
					break;
				}
			}

			_system->delayMillis(10);
		}

		_vm->screen()->updateScreen();
	}

	// TODO: Wait for the sound to finish?
}

} // end of namespace Kyra
