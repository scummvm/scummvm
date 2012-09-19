/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

// Player for Kyrandia 3 VQA movies, based on the information found at
// http://multimedia.cx/VQA_INFO.TXT
//
// The benchl.vqa movie (or whatever it is) is not supported. It does not have
// a FINF chunk.
//
// The jung2.vqa movie does work, but only thanks to a grotesque hack.

#include "kyra/kyra_v1.h"
#include "kyra/vqa.h"
#include "kyra/screen.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "common/system.h"
#include "common/events.h"

#include "graphics/surface.h"

namespace Kyra {

static uint32 readTag(Common::SeekableReadStream *stream) {
	// Some tags have to be on an even offset, so they are padded with a
	// zero byte. Skip that.

	uint32 tag = stream->readUint32BE();

	if (stream->eos())
		return 0;

	if (!(tag & 0xFF000000)) {
		tag = (tag << 8) | stream->readByte();
	}

	return tag;
}

// -----------------------------------------------------------------------

VqaDecoder::VqaDecoder() {
}

VqaDecoder::~VqaDecoder() {
	close();
}

bool VqaDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	if (stream->readUint32BE() != MKTAG('F','O','R','M')) {
		warning("VqaDecoder::loadStream(): Cannot find `FORM' tag");
		return false;
	}

	// Ignore the size of the FORM chunk. We're only interested in its
	// children.
	stream->readUint32BE();

	if (stream->readUint32BE() != MKTAG('W','V','Q','A')) {
		warning("VqaDecoder::loadStream(): Cannot find `WVQA' tag");
		return false;
	}

	VqaVideoTrack *videoTrack = new VqaDecoder::VqaVideoTrack(stream);
	addTrack(videoTrack);

	// We want to find both a VQHD chunk containing the header, and a FINF
	// chunk containing the frame offsets.

	bool foundVQHD = false;
	bool foundFINF = false;

	VqaAudioTrack *audioTrack = NULL;

	// The information we need is stored in two chunks: VQHD and FINF. We
	// need both of them before we can begin decoding the movie.

	while (!foundVQHD || !foundFINF) {
		uint32 tag = readTag(stream);
		uint32 size = stream->readUint32BE();

		switch (tag) {
		case MKTAG('V','Q','H','D'):
			videoTrack->handleVQHD();
			if (videoTrack->hasSound()) {
				audioTrack = new VqaAudioTrack(stream, videoTrack->getAudioFreq());
				videoTrack->setAudioTrack(audioTrack);
				addTrack(audioTrack);
			}
			foundVQHD = true;
			break;
		case MKTAG('F','I','N','F'):
			if (!foundVQHD) {
				warning("VqaDecoder::loadStream(): Found `FINF' before `VQHD'");
				return false;
			}
			if (size != 4 * getFrameCount()) {
				warning("VqaDecoder::loadStream(): Expected size %d for `FINF' chunk, but got %u", 4 * getFrameCount(), size);
				return false;
			}
			videoTrack->handleFINF();
			foundFINF = true;
			break;
		default:
			warning("VqaDecoder::loadStream(): Unknown tag `%s'", tag2str(tag));
			stream->seek(size, SEEK_CUR);
			break;
		}
	}

	return true;
}

// -----------------------------------------------------------------------

VqaDecoder::VqaAudioTrack::VqaAudioTrack(Common::SeekableReadStream *stream, int freq) {
	_fileStream = stream;
	_audioStream = Audio::makeQueuingAudioStream(freq, false);
}

VqaDecoder::VqaAudioTrack::~VqaAudioTrack() {
	delete _audioStream;
}

Audio::AudioStream *VqaDecoder::VqaAudioTrack::getAudioStream() const {
	return _audioStream;
}

void VqaDecoder::VqaAudioTrack::handleSND0() {
	uint32 size = _fileStream->readUint32BE();
	byte *buf = (byte *)malloc(size);
	_fileStream->read(buf, size);
	_audioStream->queueBuffer(buf, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
}

void VqaDecoder::VqaAudioTrack::handleSND1() {
	_fileStream->readUint32BE();
	uint16 outsize = _fileStream->readUint16LE();
	uint16 insize = _fileStream->readUint16LE();
	byte *inbuf = (byte *)malloc(insize);

	_fileStream->read(inbuf, insize);

	if (insize == outsize) {
		_audioStream->queueBuffer(inbuf, insize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	} else {
		const int8 WSTable2Bit[] = { -2, -1, 0, 1 };
		const int8 WSTable4Bit[] = {
			-9, -8, -6, -5, -4, -3, -2, -1,
			 0,  1,  2,  3,  4,  5,  6,  8
		};

		byte *outbuf = (byte *)malloc(outsize);
		byte *in = inbuf;
		byte *out = outbuf;
		int16 curSample = 0x80;
		uint16 bytesLeft = outsize;

		while (bytesLeft > 0) {
			uint16 input = *in++ << 2;
			byte code = (input >> 8) & 0xFF;
			int8 count = (input & 0xFF) >> 2;
			int i;

			switch (code) {
			case 2:
				if (count & 0x20) {
					/* NOTE: count is signed! */
					count <<= 3;
					curSample += (count >> 3);
					*out++ = curSample;
					bytesLeft--;
				} else {
					for (; count >= 0; count--) {
						*out++ = *in++;
						bytesLeft--;
					}
					curSample = *(out - 1);
				}
				break;
			case 1:
				for (; count >= 0; count--) {
					code = *in++;

					for (i = 0; i < 2; i++) {
						curSample += WSTable4Bit[code & 0x0F];
						curSample = CLIP<int16>(curSample, 0, 255);
						code >>= 4;
						*out++ = curSample;
					}

					bytesLeft -= 2;
				}
				break;
			case 0:
				for (; count >= 0; count--) {
					code = *in++;

					for (i = 0; i < 4; i++) {
						curSample += WSTable2Bit[code & 0x03];
						curSample = CLIP<int16>(curSample, 0, 255);
						code >>= 2;
						*out++ = curSample;
					}

					bytesLeft -= 4;
				}
				break;
			default:
				for (; count >= 0; count--) {
					*out++ = curSample;
					bytesLeft--;
				}
				break;
			}
		}
		_audioStream->queueBuffer(outbuf, outsize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
		free(inbuf);
	}
}

void VqaDecoder::VqaAudioTrack::handleSND2() {
	uint32 size = _fileStream->readUint32BE();
	warning("VqaDecoder::VqaAudioTrack::handleSND2(): `SND2' is not implemented");
	_fileStream->seek(size, SEEK_CUR);
}

// -----------------------------------------------------------------------

VqaDecoder::VqaVideoTrack::VqaVideoTrack(Common::SeekableReadStream *stream) {
	_fileStream = stream;
	_surface = new Graphics::Surface();
	memset(_palette, 0, sizeof(_palette));
	_dirtyPalette = false;
	_audioTrack = NULL;

	_curFrame = -1;

	memset(&_header, 0, sizeof(_header));
	_frameInfo = NULL;
	_codeBookSize = 0;
	_compressedCodeBook = false;
	_codeBook = NULL;
	_partialCodeBookSize = 0;
	_numPartialCodeBooks = 0;
	_partialCodeBook = NULL;
	_numVectorPointers = 0;
	_vectorPointers = NULL;
}

VqaDecoder::VqaVideoTrack::~VqaVideoTrack() {
	delete _surface;
	delete[] _frameInfo;
	delete[] _codeBook;
	delete[] _partialCodeBook;
	delete[] _vectorPointers;
	// The audio track gets deleted by VqaDecoder.
}

uint16 VqaDecoder::VqaVideoTrack::getWidth() const {
	return _header.width;
}

uint16 VqaDecoder::VqaVideoTrack::getHeight() const {
	return _header.height;
}

Graphics::PixelFormat VqaDecoder::VqaVideoTrack::getPixelFormat() const {
	return _surface->format;
}

int VqaDecoder::VqaVideoTrack::getCurFrame() const {
	return _curFrame;
}

int VqaDecoder::VqaVideoTrack::getFrameCount() const {
	return _header.numFrames;
}

Common::Rational VqaDecoder::VqaVideoTrack::getFrameRate() const {
	return _header.frameRate;
}

bool VqaDecoder::VqaVideoTrack::hasSound() const {
	return (_header.flags & 1) != 0;
}

int VqaDecoder::VqaVideoTrack::getAudioFreq() const {
	return _header.freq;
}

bool VqaDecoder::VqaVideoTrack::hasDirtyPalette() const {
	return _dirtyPalette;
}

const byte *VqaDecoder::VqaVideoTrack::getPalette() const {
	_dirtyPalette = false;
	return _palette;
}

void VqaDecoder::VqaVideoTrack::setAudioTrack(VqaAudioTrack *audioTrack) {
	_audioTrack = audioTrack;
}

const Graphics::Surface *VqaDecoder::VqaVideoTrack::decodeNextFrame() {
	// Stop if reading the tag is enough to put us ahead of the next frame
	int32 end = (_frameInfo[_curFrame + 1] & 0x7FFFFFFF) - 7;

	// At this point, we probably only need to adjust for the offset in the
	// stream to be even. But we may as well do this to really make sure
	// we have the correct offset.
	if (_curFrame >= 0) {
		_fileStream->seek(_frameInfo[_curFrame] & 0x7FFFFFFF);
	}

	bool hasFrame = false;

	while (!_fileStream->eos() && _fileStream->pos() < end) {
		uint32 tag = readTag(_fileStream);
		uint32 size;

		switch (tag) {
		case MKTAG('S','N','D','0'):	// Uncompressed sound
			assert(_audioTrack);
			_audioTrack->handleSND0();
			break;
		case MKTAG('S','N','D','1'):	// Compressed sound, almost like AUD
			assert(_audioTrack);
			_audioTrack->handleSND1();
			break;
		case MKTAG('S','N','D','2'):	// Compressed sound
			assert(_audioTrack);
			_audioTrack->handleSND2();
			break;
		case MKTAG('V','Q','F','R'):
			handleVQFR();
			hasFrame = true;
			break;
		case MKTAG('C','M','D','S'):
			// The purpose of this is unknown, but it's known to
			// exist so don't warn about it.
			size = _fileStream->readUint32BE();
			_fileStream->seek(size, SEEK_CUR);
			break;
		default:
			warning("VqaDecoder::VqaVideoTrack::decodeNextFrame(): Unknown tag `%s'", tag2str(tag));
			size = _fileStream->readUint32BE();
			_fileStream->seek(size, SEEK_CUR);
			break;
		}
	}

	if (hasFrame) {
		if (_frameInfo[_curFrame] & 0x80000000) {
			_dirtyPalette = true;
		}

		int blockPitch = _header.width / _header.blockW;

		for (int by = 0; by < _header.height / _header.blockH; by++) {
			for (int bx = 0; bx < blockPitch; bx++) {
				byte *dst = (byte *)_surface->getBasePtr(bx * _header.blockW, by * _header.blockH);
				int val = _vectorPointers[by * blockPitch + bx];
				int i;

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
				Screen::decodeFrame4(_partialCodeBook, _codeBook, _codeBookSize);
			} else {
				memcpy(_codeBook, _partialCodeBook, _partialCodeBookSize);
			}
			_numPartialCodeBooks = 0;
			_partialCodeBookSize = 0;
		}
	}

	_curFrame++;
	return _surface;
}

void VqaDecoder::VqaVideoTrack::handleVQHD() {
	_header.version     = _fileStream->readUint16LE();
	_header.flags       = _fileStream->readUint16LE();
	_header.numFrames   = _fileStream->readUint16LE();
	_header.width       = _fileStream->readUint16LE();
	_header.height      = _fileStream->readUint16LE();
	_header.blockW      = _fileStream->readByte();
	_header.blockH      = _fileStream->readByte();
	_header.frameRate   = _fileStream->readByte();
	_header.cbParts     = _fileStream->readByte();
	_header.colors      = _fileStream->readUint16LE();
	_header.maxBlocks   = _fileStream->readUint16LE();
	_header.unk1        = _fileStream->readUint32LE();
	_header.unk2        = _fileStream->readUint16LE();
	_header.freq        = _fileStream->readUint16LE();
	_header.channels    = _fileStream->readByte();
	_header.bits        = _fileStream->readByte();
	_header.unk3        = _fileStream->readUint32LE();
	_header.unk4        = _fileStream->readUint16LE();
	_header.maxCBFZSize = _fileStream->readUint32LE();
	_header.unk5        = _fileStream->readUint32LE();

	_surface->create(_header.width, _header.height, Graphics::PixelFormat::createFormatCLUT8());

	// Kyrandia 3 uses version 1 VQA files, and is the only known game to
	// do so. This version of the format has some implicit default values.

	if (_header.version == 1) {
		if (_header.freq == 0)
			_header.freq = 22050;
		if (_header.channels == 0)
			_header.channels = 1;
		if (_header.bits == 0)
			_header.bits = 8;
	}

	_frameInfo = new uint32[_header.numFrames + 1];

	_codeBookSize = 0xF00 * _header.blockW * _header.blockH;
	_codeBook = new byte[_codeBookSize];
	_partialCodeBook = new byte[_codeBookSize];
	memset(_codeBook, 0, _codeBookSize);
	memset(_partialCodeBook, 0, _codeBookSize);

	_numVectorPointers = (_header.width / _header.blockW) * (_header.height * _header.blockH);
	_vectorPointers = new uint16[_numVectorPointers];
	memset(_vectorPointers, 0, _numVectorPointers * sizeof(uint16));

	_partialCodeBookSize = 0;
	_numPartialCodeBooks = 0;

	if (hasSound()) {
		// Kyrandia 3 uses 8-bit sound, and so far testing indicates
		// that it's all mono.
		//
		// This is good, because it means we won't have to worry about
		// the confusing parts of the VQA spec, where 8- and 16-bit
		// data have different signedness and stereo sample layout
		// varies between different games.

		assert(_header.bits == 8);
		assert(_header.channels == 1);
	}
}

void VqaDecoder::VqaVideoTrack::handleFINF() {
	for (int i = 0; i < _header.numFrames; i++) {
		_frameInfo[i] = 2 * _fileStream->readUint32LE();
	}

	// HACK: This flag is set in jung2.vqa, and its purpose - if it has
	// one - is currently unknown. It can't be a general purpose flag,
	// because in large movies the frame offset can be large enough to
	// set this flag, though of course never for the first frame.
	//
	// At least in my copy of Kyrandia 3, _frameInfo[0] is 0x81000098, and
	// the desired index is 0x4716. So the value should be 0x80004716, but
	// I don't want to hard-code it. Instead, scan the file for the offset
	// to the first VQFR chunk.

	if (_frameInfo[0] & 0x01000000) {
		uint32 oldPos = _fileStream->pos();

		while (1) {
			uint32 scanTag = readTag(_fileStream);
			uint32 scanSize = _fileStream->readUint32BE();

			if (_fileStream->eos())
				break;

			if (scanTag == MKTAG('V','Q','F','R')) {
				_frameInfo[0] = (_fileStream->pos() - 8) | 0x80000000;
				break;
			}

			_fileStream->seek(scanSize, SEEK_CUR);
		}

		_fileStream->seek(oldPos);
	}

	_frameInfo[_header.numFrames] = 0x7FFFFFFF;
}

void VqaDecoder::VqaVideoTrack::handleVQFR() {
	uint32 size = _fileStream->readUint32BE();
	int32 end = _fileStream->pos() + size - 8;
	byte *inbuf;

	while (_fileStream->pos() < end) {
		uint32 tag = readTag(_fileStream);
		uint32 i;
		size = _fileStream->readUint32BE();
		
		switch (tag) {
		case MKTAG('C','B','F','0'):	// Full codebook
			_fileStream->read(_codeBook, size);
			break;
		case MKTAG('C','B','F','Z'):	// Full codebook
			inbuf = (byte *)malloc(size);
			_fileStream->read(inbuf, size);
			Screen::decodeFrame4(inbuf, _codeBook, _codeBookSize);
			free(inbuf);
			break;
		case MKTAG('C','B','P','0'):	// Partial codebook
			_compressedCodeBook = false;
			_fileStream->read(_partialCodeBook + _partialCodeBookSize, size);
			_partialCodeBookSize += size;
			_numPartialCodeBooks++;
			break;
		case MKTAG('C','B','P','Z'):	// Partial codebook
			_compressedCodeBook = true;
			_fileStream->read(_partialCodeBook + _partialCodeBookSize, size);
			_partialCodeBookSize += size;
			_numPartialCodeBooks++;
			break;
		case MKTAG('C','P','L','0'):	// Palette
			assert(size <= 3 * 256);
			_fileStream->read(_palette, size);
			break;
		case MKTAG('C','P','L','Z'):	// Palette
			inbuf = (byte *)malloc(size);
			_fileStream->read(inbuf, size);
			Screen::decodeFrame4(inbuf, _palette, 3 * 256);
			free(inbuf);
			break;
		case MKTAG('V','P','T','0'):	// Frame data
			assert(size / 2 <= _numVectorPointers);
			for (i = 0; i < size / 2; i++)
				_vectorPointers[i] = _fileStream->readUint16LE();
			break;
		case MKTAG('V','P','T','Z'):	// Frame data
			inbuf = (byte *)malloc(size);
			_fileStream->read(inbuf, size);
			size = Screen::decodeFrame4(inbuf, (uint8 *)_vectorPointers, 2 * _numVectorPointers);
			for (i = 0; i < size / 2; i++)
				_vectorPointers[i] = TO_LE_16(_vectorPointers[i]);
			free(inbuf);
			break;
		default:
			warning("VqaDecoder::VqaVideoTrack::handleVQFR(): Unknown `VQFR' sub-tag `%s'", tag2str(tag));
			_fileStream->seek(size, SEEK_CUR);
			break;
		}
	}
}

// -----------------------------------------------------------------------

VqaMovie::VqaMovie(KyraEngine_v1 *vm, OSystem *system) {
	_system = system;
	_vm = vm;
	_screen = _vm->screen();
	_decoder = new VqaDecoder();
	_drawPage = -1;
}

VqaMovie::~VqaMovie() {
	close();
	delete _decoder;
}

void VqaMovie::setDrawPage(int page) {
	_drawPage = page;
}

bool VqaMovie::open(const char *filename) {
	if (_file.open(filename)) {
		return true;
	}
	return false;
}

void VqaMovie::close() {
	if (_file.isOpen()) {
		_file.close();
	}
}

void VqaMovie::play() {
	if (_decoder->loadStream(&_file)) {
		Common::EventManager *eventMan = _vm->getEventManager();
		int width = _decoder->getWidth();
		int height = _decoder->getHeight();
		int x = (Screen::SCREEN_W - width) / 2;
		int y = (Screen::SCREEN_H - height) / 2;

		_decoder->start();

		// Note that decoding starts at frame -1. That's because there
		// is usually sound data before the first frame, probably to
		// avoid sound underflow.

		while (!_decoder->endOfVideo()) {
			Common::Event event;
			while (eventMan->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
						return;
					break;
				case Common::EVENT_RTL:
				case Common::EVENT_QUIT:
					return;
				default:
					break;
				}
			}

			if (_decoder->needsUpdate()) {
				const Graphics::Surface *surface = _decoder->decodeNextFrame();
				if (_decoder->hasDirtyPalette()) {
					memcpy(_screen->getPalette(0).getData(), _decoder->getPalette(), 3 * 256);
					_screen->setScreenPalette(_screen->getPalette(0));
				}

				_screen->copyBlockToPage(_drawPage, surface->pitch, x, y, width, height, (const byte *)surface->getBasePtr(0, 0));
			}

			_screen->updateScreen();
			_system->delayMillis(10);
		}
	}
}

} // End of namespace Kyra
