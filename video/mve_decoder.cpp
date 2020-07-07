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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "video/mve_decoder.h"

#include "audio/decoders/raw.h"

#include "common/endian.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/events.h"

#include "graphics/surface.h"
#include "graphics/palette.h"

namespace Video {

MveDecoder::MveDecoder()
	: _done(false),
	  _s(nullptr),
	  _dirtyPalette(false),
	  _skipMapSize(0),
	  _skipMap(nullptr),
	  _decodingMapSize(0),
	  _decodingMap(nullptr),
	  _frameNumber(-1),
	  _frameSize(0),
	  _frameData(nullptr),
	  _audioTrack(0),
	  _audioStream(nullptr)
{
	for (int i = 0; i < 0x300; ++i)
		_palette[i] = 0;
}

MveDecoder::~MveDecoder() {
	close();
	delete _audioStream;
	delete[] _frameData;
	delete[] _decodingMap;
	delete[] _skipMap;
}

static const char signature[] = "Interplay MVE File\x1A";

bool MveDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	byte signature_buffer[sizeof(signature)];
	stream->read(signature_buffer, sizeof(signature_buffer));
	if (memcmp(signature_buffer, signature, sizeof(signature))) {
		warning("MveDecoder::loadStream(): attempted to load non-MVE data");
		return false;
	}
	_s = stream;

	uint16 h1 = _s->readUint16LE();
	uint16 h2 = _s->readUint16LE();
	uint16 h3 = _s->readUint16LE();

	assert(h1 == 0x001a);
	assert(h2 == 0x0100);
	assert(h3 == 0x1133);

	readPacketHeader();
	while (!_done && _packetKind < 3) {
		readNextPacket();
	}

	return true;
}

void MveDecoder::setAudioTrack(int track) {
	assert(track >= 0 && track < 16);
	_audioTrack= track;
}

void MveDecoder::applyPalette(PaletteManager *paletteManager) {
	paletteManager->setPalette(_palette + 3 * _palStart, _palStart, _palCount);
}

void MveDecoder::copyBlock_8bit(Graphics::Surface &dst, Common::MemoryReadStream &s, int block) {
	int x = (block % _widthInBlocks) * 8;
	int y = (block / _widthInBlocks) * 8;

	byte *p = (byte*)dst.getBasePtr(x, y);

	for (int i = 0; i != 8; ++i) {
		s.read(p, 8);
		p += dst.pitch;
	}
}

void MveDecoder::copyBlock_16bit(Graphics::Surface &dst, Common::MemoryReadStream &s, int block) {
	int x = (block % _widthInBlocks) * 8;
	int y = (block / _widthInBlocks) * 8;

	byte *p = (byte*)dst.getBasePtr(x, y);

	for (int i = 0; i != 8; ++i) {
		for (int j = 0; j != 8; ++j) {
			WRITE_UINT16(p+2*j, s.readUint16LE());
		}
		p += dst.pitch;
	}
}

void MveDecoder::copyBlock(Graphics::Surface &dst, Graphics::Surface &src, int block, int offset) {
	int dx = (block % _widthInBlocks) * 8;
	int dy = (block / _widthInBlocks) * 8;

	int sx = dx + offset % _width;
	int sy = dy + offset / _width;

	byte *dp = (byte*)dst.getBasePtr(dx, dy);
	byte *sp = (byte*)src.getBasePtr(sx, sy);

	for (int i = 0; i != 8; ++i) {
		memmove(dp, sp, !_trueColor ? 8 : 16);
		dp += dst.pitch;
		sp += src.pitch;
	}
}

void MveDecoder::copyBlock(Graphics::Surface &dst, Graphics::Surface &src, int dx, int dy, int off_x, int off_y) {
	int sx = dx + off_x;
	int sy = dy + off_y;

	byte *dp = (byte*)dst.getBasePtr(dx, dy);
	byte *sp = (byte*)src.getBasePtr(sx, sy);

	for (int i = 0; i != 8; ++i) {
		memmove(dp, sp, !_trueColor ? 8 : 16);
		dp += dst.pitch;
		sp += src.pitch;
	}
}

void MveDecoder::decodeFormat6() {
	_decodingMapSize = _widthInBlocks * _heightInBlocks * 2;
	_decodingMap     = _frameData + 14;

	Common::MemoryReadStream opStream = Common::MemoryReadStream(_decodingMap, _decodingMapSize);
	Common::MemoryReadStream frameStream = Common::MemoryReadStream(_frameData + _decodingMapSize + 14, _frameSize);

	// Pass 1
	opStream.seek(0);
	for (int b = 0; b != _widthInBlocks * _heightInBlocks; ++b) {
		uint16 op = opStream.readUint16LE();
		if (op == 0) {
			if (!_trueColor) {
				copyBlock_8bit(_decodeSurface0, frameStream, b);
			} else {
				copyBlock_16bit(_decodeSurface0, frameStream, b);
			}
		}
	}

	// Pass 2
	opStream.seek(0);
	for (int b = 0; b != _widthInBlocks * _heightInBlocks; ++b) {
		uint16 op = opStream.readUint16LE();
		if (op != 0) {
			Graphics::Surface &src = (op & 0x8000) ? _decodeSurface1 : _decodeSurface0;
			int offset = int(op & 0x7fff) - 0x4000;
			copyBlock(_decodeSurface0, src, b, offset);
		}
	}

	// Pass 3
	for (int b = 0; b != _widthInBlocks * _heightInBlocks; ++b) {
		copyBlock(_frameSurface, _decodeSurface0, b);
	}

	Graphics::Surface t = _decodeSurface0;
	_decodeSurface0 = _decodeSurface1;
	_decodeSurface1 = t;

	_decodingMap = nullptr;
}

void MveDecoder::decodeFormat10() {
	MveSkipStream skipStream = MveSkipStream(_skipMap, _skipMapSize);
	Common::MemoryReadStream opStream = Common::MemoryReadStream(_decodingMap, _decodingMapSize);
	Common::MemoryReadStream frameStream = Common::MemoryReadStream(_frameData + 14, _frameSize - 14);

	// Pass 1
	opStream.seek(0);
	skipStream.reset();
	for (int b = 0; b != _widthInBlocks * _heightInBlocks; ++b) {
		if (skipStream.skip()) continue;
		uint16 op = opStream.readUint16LE();
		if (op == 0) {
			if (!_trueColor) {
				copyBlock_8bit(_decodeSurface0, frameStream, b);
			} else {
				copyBlock_16bit(_decodeSurface0, frameStream, b);
			}
		}
	}

	// Pass 2
	opStream.seek(0);
	skipStream.reset();
	for (int b = 0; b != _widthInBlocks * _heightInBlocks; ++b) {
		if (skipStream.skip()) continue;
		uint16 op = opStream.readUint16LE();
		if (op != 0) {
			Graphics::Surface &src = (op & 0x8000) ? _decodeSurface1 : _decodeSurface0;
			int offset = int(op & 0x7fff) - 0x4000;
			copyBlock(_decodeSurface0, src, b, offset);
		}
	}

	// Pass 3
	skipStream.reset();
	for (int b = 0; b != _widthInBlocks * _heightInBlocks; ++b) {
		if (skipStream.skip()) continue;
		copyBlock(_frameSurface, _decodeSurface0, b);
	}

	Graphics::Surface t = _decodeSurface0;
	_decodeSurface0 = _decodeSurface1;
	_decodeSurface1 = t;
}

void MveDecoder::readPacketHeader() {
	_packetLen  = _s->readUint16LE();
	_packetKind = _s->readUint16LE();

	/*
	switch (_packetKind) {
		case 0:
			warning("initialize audio");
			break;
		case 1:
			warning("audio");
			break;
		case 2:
			warning("initialize video");
			break;
		case 3:
			warning("video");
			break;
		case 4:
			warning("shutdown");
			break;
		case 5:
			warning("end chunk");
			break;
	}
	*/
}

void MveDecoder::readNextPacket() {
	bool frameDone = false;
	while (!_done && !frameDone) {
		uint16 opLen = _s->readUint16LE();
		uint16 opKind = _s->readUint16BE();

		switch (opKind) {
			case 0x0000:
			{
				_done = true;
				assert(opLen == 0);
				break;
			}
			case 0x0100:
			{
				assert(opLen == 0);
				readPacketHeader();
				break;
			}
			case 0x0200: // create timer
			{
				assert(opLen == 6);
				uint32 rate = _s->readUint32LE();
				uint16 subdiv = _s->readUint16LE();
				_frameRate = Common::Rational(1000000, rate * subdiv);
				break;
			}
			case 0x0300: // init audio
			{
				assert(opLen == 8);
				/*uint16 unk =*/ _s->readUint16LE();
				uint16 flags = _s->readUint16LE();
				uint16 sampleRate = _s->readUint16LE();
				/*uint16 bufLen =*/ _s->readUint16LE();

				/*
				warning("\t\tAudio: %dHz %s %s",
					sampleRate,
					(flags & 1) == 0 ? "mono" : "stereo",
					(flags & 2) == 0 ? "8-bit" : "16-bit"
				);
				*/

				assert((flags & 1) == 0);
				assert((flags & 2) == 0);

				_audioStream = Audio::makeQueuingAudioStream(sampleRate, (flags & 2) != 0);
				addTrack(new MveAudioTrack(this));

				break;
			}
			case 0x0400: // send audio
			{
				assert(opLen == 0);
				break;
			}
			case 0x0502: // init video buffers
			{
				assert(opLen == 8);

				uint16 width = _s->readUint16LE();
				uint16 height = _s->readUint16LE();
				/*uint16 count =*/ _s->readUint16LE();
				uint16 trueColor = _s->readUint16LE();

				_widthInBlocks = width;
				_heightInBlocks = height;

				_width = 8 * width;
				_height = 8 * height;

				_trueColor = !!trueColor;

				if (!_trueColor) {
					_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
				} else {
					_pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
				}

				_decodeSurface0.create(_width, _height, _pixelFormat);
				_decodeSurface0.fillRect(Common::Rect(_width, _height), 0);

				_decodeSurface1.create(_width, _height, _pixelFormat);
				_decodeSurface1.fillRect(Common::Rect(_width, _height), 0);

				_frameSurface.create(_width, _height, _pixelFormat);
				_frameSurface.fillRect(Common::Rect(_width, _height), 0);

				addTrack(new MveVideoTrack(this));

				break;
			}
			case 0x0600:
			{
				delete[] _frameData;
				_frameData = new byte[opLen];
				_frameSize = opLen;
				_s->read(_frameData, _frameSize);

				decodeFormat6();

				break;
			}
			case 0x0701: // send video
			{
				assert(opLen == 6);
				uint16 palStart = _s->readUint16LE();
				uint16 palCount = _s->readUint16LE();
				uint16 unk = _s->readUint16LE();
				(void)unk;

				if (palStart || palCount) {
					_palStart = palStart;
					_palCount = palCount;
				}

				_frameNumber += 1;
				frameDone = true;

				break;
			}
			case 0x0800: // audio frame
			{
				/*uint16 seq =*/ _s->readUint16LE();
				uint16 mask = _s->readUint16LE();
				uint16 len = _s->readUint16LE();

				assert(opLen == len + 6);
				assert(_audioStream);

				if (mask & (1 << _audioTrack)) {
					byte *audioFrame = new byte[len];
					_s->read(audioFrame, len);
					_audioStream->queueBuffer(audioFrame, len, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
				} else {
					_s->skip(len);
				}

				break;
			}
			case 0x0900: // audio frame (silent)
			{
				assert(opLen == 6);
				/*uint16 seq =*/ _s->readUint16LE();
				/*uint16 mask =*/ _s->readUint16LE();
				/*uint16 len =*/ _s->readUint16LE();

				break;
			}
			case 0x0a00: // set video mode
			{
				assert(opLen == 6);
				/*uint16 width =*/ _s->readUint16LE();
				/*uint16 height =*/ _s->readUint16LE();
				/*uint16 flags =*/ _s->readUint16LE();

				break;
			}
			case 0x0c00:
			{
				uint16 palStart = _s->readUint16LE();
				uint16 palCount = _s->readUint16LE();

				assert(opLen >= 3 * palCount + 2);

				for (int i = palStart; i < palStart + palCount; ++i) {
					byte r = _s->readByte();
					byte g = _s->readByte();
					byte b = _s->readByte();

					_palette[3*i+0] = (r << 2) | (r >> 4);
					_palette[3*i+1] = (g << 2) | (g >> 4);
					_palette[3*i+2] = (b << 2) | (b >> 4);
				}
				if (palCount & 1) {
					_s->skip(1);
				}

				_dirtyPalette = true;
				_palStart = palStart;
				_palCount = palCount;

				break;
			}
			case 0x0e00:
			{
				// TODO: Preallocate or keep existing buffer
				delete[] _skipMap;
				_skipMap = new byte[opLen];
				_skipMapSize = opLen;
				_s->read(_skipMap, _skipMapSize);
				break;
			}
			case 0x0f00:
			{
				// TODO: Preallocate or keep existing buffer
				delete[] _decodingMap;
				_decodingMap = new byte[opLen];
				_decodingMapSize = opLen;
				_s->read(_decodingMap, _decodingMapSize);
				break;
			}
			case 0x1000:
			{
				// TODO: Preallocate or keep existing buffer
				delete[] _frameData;
				_frameData = new byte[opLen];
				_frameSize = opLen;
				_s->read(_frameData, _frameSize);

				decodeFormat10();

				break;
			}
			default:
				error("Unknown opcode %04x", opKind);
				_s->skip(opLen);
				break;
		}
	}
}

MveDecoder::MveVideoTrack::MveVideoTrack(MveDecoder *decoder) : _decoder(decoder) {
}

bool MveDecoder::MveVideoTrack::endOfTrack() const {
	return _decoder->_done;
}

uint16 MveDecoder::MveVideoTrack::getWidth() const {
	return _decoder->getWidth();
}

uint16 MveDecoder::MveVideoTrack::getHeight() const {
	return _decoder->getHeight();
}

Graphics::PixelFormat MveDecoder::MveVideoTrack::getPixelFormat() const {
	return _decoder->getPixelFormat();
}

int MveDecoder::MveVideoTrack::getCurFrame() const {
	return _decoder->_frameNumber;
}

const Graphics::Surface *MveDecoder::MveVideoTrack::decodeNextFrame() {
	return &_decoder->_frameSurface;
}

const byte *MveDecoder::MveVideoTrack::getPalette() const {
	return _decoder->_palette;
}

bool MveDecoder::MveVideoTrack::hasDirtyPalette() const {
	return _decoder->_dirtyPalette;
}

Common::Rational MveDecoder::MveVideoTrack::getFrameRate() const {
	return _decoder->getFrameRate();
}

MveDecoder::MveAudioTrack::MveAudioTrack(MveDecoder *decoder) :
	AudioTrack(Audio::Mixer::kPlainSoundType),
	_decoder(decoder)
{
}

Audio::AudioStream *MveDecoder::MveAudioTrack::getAudioStream() const {
	return _decoder->_audioStream;
}

} // End of namespace Video
