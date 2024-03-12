/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "kyra/graphics/vqa.h"
#include "kyra/graphics/screen.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "common/system.h"
#include "common/events.h"

#include "graphics/paletteman.h"
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

VQADecoder::VQADecoder() : _fileStream(nullptr), _frameInfo(nullptr) {
	memset(&_header, 0, sizeof(_header));
}

VQADecoder::~VQADecoder() {
	close();
	delete[] _frameInfo;
}

bool VQADecoder::loadStream(Common::SeekableReadStream *stream) {
	close();
	_fileStream = stream;

	if (_fileStream->readUint32BE() != MKTAG('F','O','R','M')) {
		warning("VQADecoder::loadStream(): Cannot find `FORM' tag");
		return false;
	}

	// Ignore the size of the FORM chunk. We're only interested in its
	// children.
	_fileStream->readUint32BE();

	if (_fileStream->readUint32BE() != MKTAG('W','V','Q','A')) {
		warning("VQADecoder::loadStream(): Cannot find `WVQA' tag");
		return false;
	}

	// We want to find both a VQHD chunk containing the header, and a FINF
	// chunk containing the frame offsets.

	bool foundVQHD = false;
	bool foundFINF = false;

	VQAAudioTrack *audioTrack = nullptr;

	// The information we need is stored in two chunks: VQHD and FINF. We
	// need both of them before we can begin decoding the movie.

	while (!foundVQHD || !foundFINF) {
		uint32 tag = readTag(stream);
		uint32 size = _fileStream->readUint32BE();

		switch (tag) {
		case MKTAG('V','Q','H','D'):
			handleVQHD(_fileStream);
			if (_header.flags & 1) {
				audioTrack = new VQAAudioTrack(&_header, getSoundType());
				addTrack(audioTrack);
			}
			foundVQHD = true;
			break;
		case MKTAG('F','I','N','F'):
			if (!foundVQHD) {
				warning("VQADecoder::loadStream(): Found `FINF' before `VQHD'");
				return false;
			}
			if (size != 4 * getFrameCount()) {
				warning("VQADecoder::loadStream(): Expected size %d for `FINF' chunk, but got %u", 4 * getFrameCount(), size);
				return false;
			}
			handleFINF(_fileStream);
			foundFINF = true;
			break;
		default:
			warning("VQADecoder::loadStream(): Unknown tag `%s'", tag2str(tag));
			_fileStream->seek(size, SEEK_CUR);
			break;
		}
	}

	return true;
}

void VQADecoder::handleVQHD(Common::SeekableReadStream *stream) {
	_header.version     = stream->readUint16LE();
	_header.flags       = stream->readUint16LE();
	_header.numFrames   = stream->readUint16LE();
	_header.width       = stream->readUint16LE();
	_header.height      = stream->readUint16LE();
	_header.blockW      = stream->readByte();
	_header.blockH      = stream->readByte();
	_header.frameRate   = stream->readByte();
	_header.cbParts     = stream->readByte();
	_header.colors      = stream->readUint16LE();
	_header.maxBlocks   = stream->readUint16LE();
	_header.unk1        = stream->readUint32LE();
	_header.unk2        = stream->readUint16LE();
	_header.freq        = stream->readUint16LE();
	_header.channels    = stream->readByte();
	_header.bits        = stream->readByte();
	_header.unk3        = stream->readUint32LE();
	_header.unk4        = stream->readUint16LE();
	_header.maxCBFZSize = stream->readUint32LE();
	_header.unk5        = stream->readUint32LE();

	_frameInfo = new uint32[_header.numFrames + 1];

	VQAVideoTrack *videoTrack = new VQAVideoTrack(&_header);
	addTrack(videoTrack);

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

	if (_header.flags & 1) {
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

void VQADecoder::handleFINF(Common::SeekableReadStream *stream) {
	for (int i = 0; i < _header.numFrames; i++) {
		_frameInfo[i] = 2 * stream->readUint32LE();
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
		uint32 oldPos = stream->pos();

		while (1) {
			uint32 scanTag = readTag(stream);
			uint32 scanSize = stream->readUint32BE();

			if (stream->eos())
				break;

			if (scanTag == MKTAG('V','Q','F','R')) {
				_frameInfo[0] = (stream->pos() - 8) | 0x80000000;
				break;
			}

			stream->seek(scanSize, SEEK_CUR);
		}

		stream->seek(oldPos);
	}

	_frameInfo[_header.numFrames] = 0x7FFFFFFF;
}

void VQADecoder::readNextPacket() {
	VQAVideoTrack *videoTrack = (VQAVideoTrack *)getTrack(0);
	VQAAudioTrack *audioTrack = (VQAAudioTrack *)getTrack(1);

	assert(videoTrack);

	int curFrame = videoTrack->getCurFrame();

	// Stop if reading the tag is enough to put us ahead of the next frame
	int32 end = (_frameInfo[curFrame + 1] & 0x7FFFFFFF) - 7;

	// At this point, we probably only need to adjust for the offset in the
	// stream to be even. But we may as well do this to really make sure
	// we have the correct offset.
	if (curFrame >= 0) {
		_fileStream->seek(_frameInfo[curFrame] & 0x7FFFFFFF);
		if (_frameInfo[curFrame] & 0x80000000) {
			videoTrack->setHasDirtyPalette();
		}
	}

	while (!_fileStream->eos() && _fileStream->pos() < end) {
		uint32 tag = readTag(_fileStream);
		uint32 size;

		switch (tag) {
		case MKTAG('S','N','D','0'):	// Uncompressed sound
			assert(audioTrack);
			audioTrack->handleSND0(_fileStream);
			break;
		case MKTAG('S','N','D','1'):	// Compressed sound, almost like AUD
			assert(audioTrack);
			audioTrack->handleSND1(_fileStream);
			break;
		case MKTAG('S','N','D','2'):	// Compressed sound
			assert(audioTrack);
			audioTrack->handleSND2(_fileStream);
			break;
		case MKTAG('V','Q','F','R'):
			videoTrack->handleVQFR(_fileStream);
			break;
		case MKTAG('C','M','D','S'):
			// The purpose of this is unknown, but it's known to
			// exist so don't warn about it.
			size = _fileStream->readUint32BE();
			_fileStream->seek(size, SEEK_CUR);
			break;
		default:
			warning("VQADecoder::readNextPacket(): Unknown tag `%s'", tag2str(tag));
			size = _fileStream->readUint32BE();
			_fileStream->seek(size, SEEK_CUR);
			break;
		}
	}
}

// -----------------------------------------------------------------------

VQADecoder::VQAAudioTrack::VQAAudioTrack(const VQAHeader *header, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType) {
	_audioStream = Audio::makeQueuingAudioStream(header->freq, false);
}

VQADecoder::VQAAudioTrack::~VQAAudioTrack() {
	delete _audioStream;
}

Audio::AudioStream *VQADecoder::VQAAudioTrack::getAudioStream() const {
	return _audioStream;
}

void VQADecoder::VQAAudioTrack::handleSND0(Common::SeekableReadStream *stream) {
	uint32 size = stream->readUint32BE();
	byte *buf = (byte *)malloc(size);
	stream->read(buf, size);
	_audioStream->queueBuffer(buf, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
}

void VQADecoder::VQAAudioTrack::handleSND1(Common::SeekableReadStream *stream) {
	stream->readUint32BE();
	uint16 outsize = stream->readUint16LE();
	uint16 insize = stream->readUint16LE();
	byte *inbuf = (byte *)malloc(insize);

	stream->read(inbuf, insize);

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

void VQADecoder::VQAAudioTrack::handleSND2(Common::SeekableReadStream *stream) {
	uint32 size = stream->readUint32BE();
	warning("VQADecoder::VQAAudioTrack::handleSND2(): `SND2' is not implemented");
	stream->seek(size, SEEK_CUR);
}

// -----------------------------------------------------------------------

VQADecoder::VQAVideoTrack::VQAVideoTrack(const VQAHeader *header) {
	memset(_palette, 0, sizeof(_palette));
	_dirtyPalette = false;

	_width = header->width;
	_height = header->height;
	_blockW = header->blockW;
	_blockH = header->blockH;
	_cbParts = header->cbParts;

	_newFrame = false;

	_curFrame = -1;
	_frameCount = header->numFrames;
	_frameRate = header->frameRate;

	_codeBookSize = 0xF00 * header->blockW * header->blockH;
	_compressedCodeBook = false;
	_codeBook = new byte[_codeBookSize]();
	_partialCodeBookSize = 0;
	_numPartialCodeBooks = 0;
	_partialCodeBook = new byte[_codeBookSize]();
	_numVectorPointers = (header->width / header->blockW) * (header->height * header->blockH);
	_vectorPointers = new uint16[_numVectorPointers]();

	_surface = new Graphics::Surface();
	_surface->create(header->width, header->height, Graphics::PixelFormat::createFormatCLUT8());
}

VQADecoder::VQAVideoTrack::~VQAVideoTrack() {
	_surface->free();
	delete _surface;
	delete[] _codeBook;
	delete[] _partialCodeBook;
	delete[] _vectorPointers;
}

uint16 VQADecoder::VQAVideoTrack::getWidth() const {
	return _width;
}

uint16 VQADecoder::VQAVideoTrack::getHeight() const {
	return _height;
}

Graphics::PixelFormat VQADecoder::VQAVideoTrack::getPixelFormat() const {
	return _surface->format;
}

int VQADecoder::VQAVideoTrack::getCurFrame() const {
	return _curFrame;
}

int VQADecoder::VQAVideoTrack::getFrameCount() const {
	return _frameCount;
}

Common::Rational VQADecoder::VQAVideoTrack::getFrameRate() const {
	return _frameRate;
}

void VQADecoder::VQAVideoTrack::setHasDirtyPalette() {
	_dirtyPalette = true;
}

bool VQADecoder::VQAVideoTrack::hasDirtyPalette() const {
	return _dirtyPalette;
}

const byte *VQADecoder::VQAVideoTrack::getPalette() const {
	_dirtyPalette = false;
	return _palette;
}

const Graphics::Surface *VQADecoder::VQAVideoTrack::decodeNextFrame() {
	if (_newFrame) {
		_newFrame = false;

		int blockPitch = _width / _blockW;

		for (int by = 0; by < _height / _blockH; by++) {
			for (int bx = 0; bx < blockPitch; bx++) {
				byte *dst = (byte *)_surface->getBasePtr(bx * _blockW, by * _blockH);
				int val = _vectorPointers[by * blockPitch + bx];
				int i;

				if ((val & 0xFF00) == 0xFF00) {
					// Solid color
					for (i = 0; i < _blockH; i++) {
						memset(dst, 255 - (val & 0xFF), _blockW);
						dst += _width;
					}
				} else {
					// Copy data from _vectorPointers. I'm not sure
					// why we don't use the three least significant
					// bits of 'val'.
					byte *src = &_codeBook[(val >> 3) * _blockW * _blockH];

					for (i = 0; i < _blockH; i++) {
						memcpy(dst, src, _blockW);
						src += _blockW;
						dst += _width;
					}
				}
			}
		}

		if (_numPartialCodeBooks == _cbParts) {
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

void VQADecoder::VQAVideoTrack::handleVQFR(Common::SeekableReadStream *stream) {
	uint32 size = stream->readUint32BE();
	int32 end = stream->pos() + size - 8;
	byte *inbuf;

	_newFrame = true;

	while (stream->pos() < end) {
		uint32 tag = readTag(stream);
		uint32 i;
		size = stream->readUint32BE();

		switch (tag) {
		case MKTAG('C','B','F','0'):	// Full codebook
			stream->read(_codeBook, size);
			break;
		case MKTAG('C','B','F','Z'):	// Full codebook
			inbuf = (byte *)malloc(size);
			stream->read(inbuf, size);
			Screen::decodeFrame4(inbuf, _codeBook, _codeBookSize);
			free(inbuf);
			break;
		case MKTAG('C','B','P','0'):	// Partial codebook
			_compressedCodeBook = false;
			stream->read(_partialCodeBook + _partialCodeBookSize, size);
			_partialCodeBookSize += size;
			_numPartialCodeBooks++;
			break;
		case MKTAG('C','B','P','Z'):	// Partial codebook
			_compressedCodeBook = true;
			stream->read(_partialCodeBook + _partialCodeBookSize, size);
			_partialCodeBookSize += size;
			_numPartialCodeBooks++;
			break;
		case MKTAG('C','P','L','0'):	// Palette
			assert(size <= 3 * 256);
			stream->read(_palette, size);
			break;
		case MKTAG('C','P','L','Z'):	// Palette
			inbuf = (byte *)malloc(size);
			stream->read(inbuf, size);
			Screen::decodeFrame4(inbuf, _palette, 3 * 256);
			free(inbuf);
			break;
		case MKTAG('V','P','T','0'):	// Frame data
			assert(size / 2 <= _numVectorPointers);
			for (i = 0; i < size / 2; i++)
				_vectorPointers[i] = stream->readUint16LE();
			break;
		case MKTAG('V','P','T','Z'):	// Frame data
			inbuf = (byte *)malloc(size);
			stream->read(inbuf, size);
			size = Screen::decodeFrame4(inbuf, (uint8 *)_vectorPointers, 2 * _numVectorPointers);
			for (i = 0; i < size / 2; i++)
				_vectorPointers[i] = TO_LE_16(_vectorPointers[i]);
			free(inbuf);
			break;
		default:
			warning("VQADecoder::VQAVideoTrack::handleVQFR(): Unknown `VQFR' sub-tag `%s'", tag2str(tag));
			stream->seek(size, SEEK_CUR);
			break;
		}
	}
}

// -----------------------------------------------------------------------

VQAMovie::VQAMovie(KyraEngine_v1 *vm, OSystem *system) {
	_system = system;
	_vm = vm;
	_screen = _vm->screen();
	_decoder = new VQADecoder();
}

VQAMovie::~VQAMovie() {
	close();
	delete _decoder;
}

bool VQAMovie::open(const char *filename) {
	if (_file.open(filename)) {
		return true;
	}
	return false;
}

void VQAMovie::close() {
	if (_file.isOpen()) {
		_file.close();
	}
}

void VQAMovie::play() {
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
				case Common::EVENT_RETURN_TO_LAUNCHER:
				case Common::EVENT_QUIT:
					return;
				default:
					break;
				}
			}

			if (_decoder->needsUpdate()) {
				const Graphics::Surface *surface = _decoder->decodeNextFrame();
				if (_decoder->hasDirtyPalette()) {
					const byte *decoderPalette = _decoder->getPalette();
					byte systemPalette[256 * 3];
					for (int i = 0; i < ARRAYSIZE(systemPalette); i++) {
						systemPalette[i] = (decoderPalette[i] * 0xFF) / 0x3F;
					}
					_system->getPaletteManager()->setPalette(systemPalette, 0, 256);
				}

				_system->copyRectToScreen((const byte *)surface->getBasePtr(0, 0), surface->pitch, x, y, width, height);
			}

			_screen->updateBackendScreen(true);
			_system->delayMillis(10);
		}
	}
}

} // End of namespace Kyra
