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

#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/rational.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/memstream.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "engines/grim/debug.h"
#include "engines/grim/movie/codecs/codec48.h"
#include "engines/grim/movie/codecs/blocky8.h"
#include "engines/grim/movie/codecs/blocky16.h"
#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/codecs/vima.h"

namespace Grim {

#define ANNO_HEADER "MakeAnim animation type 'Bl16' parameters: "
#define BUFFER_SIZE 16385
#define SMUSH_SPEED 66667

bool SmushDecoder::_demo = false;

static uint16 smushDestTable[5786];

SmushDecoder::SmushDecoder() {
	_file = nullptr;

	_videoLooping = false;
	_startPos = 0;
	_frames = nullptr;

	_videoTrack = nullptr;
	_audioTrack = nullptr;
	_videoPause = false;
}

SmushDecoder::~SmushDecoder() {
	delete _videoTrack;
	delete _audioTrack;
	delete[] _frames;
}

void SmushDecoder::init() {
	_videoTrack->init();
	_audioTrack->init();
}

void SmushDecoder::initFrames() {
	delete[] _frames;
	_frames = new Frame[_videoTrack->getFrameCount()];

	int seekPos = _file->pos();
	int curFrame = -1;
	_file->seek(_startPos, SEEK_SET);
	while (curFrame < _videoTrack->getFrameCount() - 1) {
		Frame &frame = _frames[++curFrame];
		frame.frame = curFrame;
		frame.pos = _file->pos();
		frame.keyframe = false;

		uint32 tag = _file->readUint32BE();
		uint32 size;
		if (tag == MKTAG('A', 'N', 'N', 'O')) {
			size = _file->readUint32BE();
			_file->seek(size, SEEK_CUR);
			tag = _file->readUint32BE();
		}
		assert(tag == MKTAG('F', 'R', 'M', 'E'));
		size = _file->readUint32BE();

		while (size > 0) {
			uint32 subType = _file->readUint32BE();
			uint32 subSize = _file->readUint32BE();
			int32  subPos  = _file->pos();

			if (subType == MKTAG('B', 'l', '1', '6')) {
				_file->seek(18, SEEK_CUR);
				if (_file->readByte() == 0) {
					frame.keyframe = true;
				}
			}

			size -= subSize + 8 + (subSize & 1);
			_file->seek(subPos + subSize + (subSize & 1), SEEK_SET);
		}

		_file->seek(size, SEEK_CUR);
	}

	_file->seek(seekPos, SEEK_SET);
}

void SmushDecoder::close() {
	VideoDecoder::close();
	_audioTrack = nullptr;
	_videoTrack = nullptr;
	_videoLooping = false;
	_startPos = 0;
	delete[] _frames;
	_frames = nullptr;
	if (_file) {
		delete _file;
		_file = nullptr;
	}
}


bool SmushDecoder::readHeader() {
	if (!_file) {
		return false;
	}

	uint32 mainTag = _file->readUint32BE();
	uint32 pos = _file->pos();
	uint32 expectedTag = 0;
	uint32 size = _file->readUint32BE(); // file-size

	// Verify that we have the correct combination of headers.
	if (mainTag == MKTAG('A', 'N', 'I', 'M')) { // Demo
		expectedTag = MKTAG('A', 'H', 'D', 'R');
	} else if (mainTag == MKTAG('S', 'A', 'N', 'M')) { // Retail
		expectedTag = MKTAG('S', 'H', 'D', 'R');
	} else {
		error("Invalid SMUSH-header");
	}

	uint32 tag = _file->readUint32BE();
	size = _file->readUint32BE();
	pos = _file->pos();

	assert(tag == expectedTag);

	if (tag == MKTAG('A', 'H', 'D', 'R')) { // Demo
		uint32 version = _file->readUint16LE();
		uint16 nbFrames = _file->readUint16LE();
		_file->readUint16BE(); // unknown

		int width = -1;
		int height = -1;
		_videoLooping = false;
		_startPos = 0;

		_videoTrack = new SmushVideoTrack(width, height, SMUSH_SPEED, nbFrames, false);
		_videoTrack->_x = -1;
		_videoTrack->_y = -1;
		addTrack(_videoTrack);
		_file->read(_videoTrack->getPal(), 0x300);

		int audioRate = 11025;
		if (version == 2) {

			_file->readUint32LE(); // framerate
			_file->readUint32LE();
			audioRate = _file->readUint32LE();
		}

		_file->readUint32BE();
		_file->readUint32BE();
		_audioTrack = new SmushAudioTrack(getSoundType(), false, audioRate, 2);
		addTrack(_audioTrack);
		return true;

	} else if (tag == MKTAG('S', 'H', 'D', 'R')) { // Retail
		_file->readUint16LE();
		uint16 nbFrames = _file->readUint32LE();
		_file->readUint16LE();
		int width = _file->readUint16LE();
		int height = _file->readUint16LE();
		_file->readUint16LE();
		int frameRate = _file->readUint32LE();

		int16 flags = _file->readUint16LE();
		// Output information for checking out the flags
		if (Debug::isChannelEnabled(Debug::Movie | Debug::Info)) {
			warning("SMUSH Flags:");
			for (int i = 0; i < 16; i++) {
				warning(" %d", (flags & (1 << i)) != 0);
			}
		}

		_file->seek(pos + size + (size & 1), SEEK_SET);

		_videoLooping = true;
		// If the video is NOT looping, setLooping will set the speed to the proper value
		_videoTrack = new SmushVideoTrack(width, height, frameRate, nbFrames, true);
		addTrack(_videoTrack);
		return handleFramesHeader();
	}
	return false;
}

bool SmushDecoder::handleFramesHeader() {
	uint32 tag;
	int32 size;
	int pos = 0;
	int freq = 0;
	int channels = 0;

	tag = _file->readUint32BE();
	if (tag != MKTAG('F', 'L', 'H', 'D')) {
		return false;
	}
	size = _file->readUint32BE();
	byte *f_header = new byte[size];
	_file->read(f_header, size);

	do {
		if (READ_BE_UINT32(f_header + pos) == MKTAG('B', 'l', '1', '6')) {
			pos += READ_BE_UINT32(f_header + pos + 4) + 8;
		} else if (READ_BE_UINT32(f_header + pos) == MKTAG('W', 'a', 'v', 'e')) {
			freq = READ_LE_UINT32(f_header + pos + 8);
			channels = READ_LE_UINT32(f_header + pos + 12);
			pos += 20;
		} else {
			error("SmushDecoder::handleFramesHeader() unknown tag");
		}
	} while (pos < size);
	delete[] f_header;

	_audioTrack = new SmushAudioTrack(getSoundType(), true, freq, channels);
	addTrack(_audioTrack);
	return true;
}

bool SmushDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_file = stream;

	// Load the video
	if (!readHeader()) {
		warning("Failure loading SMUSH-file");
		return false;
	}

	_startPos = _file->pos();

	init();
	return true;
}

const Graphics::Surface *SmushDecoder::decodeNextFrame() {
	handleFrame();

	// We might be interested in getting the last frame even after the video ends:
	if (endOfVideo()) {
		return _videoTrack->decodeNextFrame();
	}
	return VideoDecoder::decodeNextFrame();
}

void SmushDecoder::setLooping(bool l) {
	_videoLooping = l;

	if (!_videoLooping) {
		_videoTrack->setMsPerFrame(SMUSH_SPEED);
	}
}

void SmushDecoder::handleFrame() {
	uint32 tag;
	int32 size;

	if (isPaused()) {
		return;
	}

	if (_videoTrack->endOfTrack()) { // Looping is handled outside, by rewinding the video.
		_audioTrack->stop(); // HACK: Avoids the movie playing past the last frame
		//  pauseVideo(true);
		return;
	}

	tag = _file->readUint32BE();
	size = _file->readUint32BE();
	if (tag == MKTAG('A', 'N', 'N', 'O')) {
		char *anno;
		byte *data;

		data = new byte[size];
		_file->read(data, size);
		anno = (char *)data;
		if (strncmp(anno, ANNO_HEADER, sizeof(ANNO_HEADER) - 1) == 0) {
			//char *annoData = anno + sizeof(ANNO_HEADER);

			// Examples:
			//  Water streaming around boat from Manny's balcony
			//  MakeAnim animation type 'Bl16' parameters: 10000;12000;100;1;0;0;0;0;25;0;
			//  Water in front of the Blue Casket
			//  MakeAnim animation type 'Bl16' parameters: 20000;25000;100;1;0;0;0;0;25;0;
			//  Scrimshaw exterior:
			//  MakeAnim animation type 'Bl16' parameters: 6000;8000;100;0;0;0;0;0;2;0;
			//  Lola engine room (loops a limited number of times?):
			//  MakeAnim animation type 'Bl16' parameters: 6000;8000;90;1;0;0;0;0;2;0;
			Debug::debug(Debug::Movie, "Announcement data: %s\n", anno);
			// It looks like the announcement data is actually for setting some of the
			// header parameters, not for any looping purpose
		} else {
			Debug::debug(Debug::Movie, "Announcement header not understood: %s\n", anno);
		}
		delete[] anno;
		tag = _file->readUint32BE();
		size = _file->readUint32BE();
	}

	assert(tag == MKTAG('F', 'R', 'M', 'E'));
	handleFRME(_file, size);

	_videoTrack->finishFrame();
}

void SmushDecoder::handleFRME(Common::SeekableReadStream *stream, uint32 size) {
	int blockSize = size;

	byte *block = new byte[size];
	stream->read(block, size);

	Common::MemoryReadStream *memStream = new Common::MemoryReadStream(block, size, DisposeAfterUse::NO);
	while (size > 0) {
		uint32 subType = memStream->readUint32BE();
		uint32 subSize = memStream->readUint32BE();
		uint32 subPos = memStream->pos();

		switch (subType) {
			// Retail only:
		case MKTAG('B', 'l', '1', '6'):
			_videoTrack->handleBlocky16(memStream, subSize);
			break;
		case MKTAG('W', 'a', 'v', 'e'):
			_audioTrack->handleVIMA(memStream, blockSize);
			break;
			// Demo only:
		case MKTAG('F', 'O', 'B', 'J'):
			_videoTrack->handleFrameObject(memStream, subSize);
			break;
		case MKTAG('I', 'A', 'C', 'T'):
			_audioTrack->handleIACT(memStream, subSize);
			break;
		case MKTAG('X', 'P', 'A', 'L'):
			_videoTrack->handleDeltaPalette(memStream, subSize);
			break;
		default:
			Debug::error(Debug::Movie, "SmushDecoder::handleFrame() unknown tag");
		}
		size -= subSize + 8 + (subSize & 1);
		memStream->seek(subPos + subSize + (subSize & 1), SEEK_SET);
	}
	delete memStream;
	delete[] block;
}

bool SmushDecoder::rewind() {
	return seekToFrame(0);
}

bool SmushDecoder::seekIntern(const Audio::Timestamp &time) {
	int32 wantedFrame = (uint32)((time.msecs() / 1000.0f) * _videoTrack->getFrameRate().toDouble());
	if (wantedFrame != 0) {
		Debug::debug(Debug::Movie, "Seek to time: %d, frame: %d", time.msecs(), wantedFrame);
		Debug::debug(Debug::Movie, "Current frame: %d", _videoTrack->getCurFrame());
	}

	if (wantedFrame > _videoTrack->getFrameCount()) {
		return false;
	}

	if (!_frames) {
		initFrames();
	}

	// Track down the keyframe
	int keyframe = 0;
	for (int i = wantedFrame; i >= 0; --i) {
		if (_frames[i].keyframe) {
			keyframe = i;
			break;
		}
	}
	_videoTrack->setFrameStart(keyframe);

	// VIMA frames are 50 frames ahead of time, so we have to make sure we have 50 frames
	// of audio before the wantedFrame. Here we use 51 to have a bit of safe margin
	if (wantedFrame - keyframe < 51) {
		keyframe = wantedFrame - 51;
	}
	if (keyframe < 0) {
		keyframe = 0;
	}

	_file->seek(_frames[keyframe].pos, SEEK_SET);
	_videoTrack->setCurFrame(keyframe - 1);

	while (_videoTrack->getCurFrame() < wantedFrame - 1) {
		decodeNextFrame();
	}

	// As said, VIMA is 50 frames ahead of time. Every frame it pushes 1470 samples, and 50 * 1470 = 73500.
	// The first frame, instead of 1470, it pushes 73500 samples to have this 50-frames-time.
	// So if we have used frame 0 as keyframe we can remove safely time * rate samples, and we will
	// still have the 50 frames margin. If we have used a later frame as keyframe we don't have the 73500
	// samples pushed the first frame, so we have to be careful not to remove too much data,
	// otherwise the audio will start at a later point. (72030 == 73500 - 1470)
	int offset = (keyframe == 0 ? 0 : 72030);

	// Skip decoded audio between the keyframe and the target frame
	Audio::Timestamp delay = 0;
	if (_videoTrack->getCurFrame() > 0) {
		delay = _videoTrack->getFrameTime(_videoTrack->getCurFrame());
	}
	if (keyframe > 0) {
		delay = delay - _videoTrack->getFrameTime(keyframe);
	}

	int32 sampleCount = (delay.msecs() / 1000.f) * _audioTrack->getRate() - offset;
	_audioTrack->skipSamples(sampleCount);

	VideoDecoder::seekIntern(time);
	return true;
}

SmushDecoder::SmushVideoTrack::SmushVideoTrack(int width, int height, int fps, int numFrames, bool is16Bit) {
	if (!is16Bit) { // Demo
		_format = Graphics::PixelFormat::createFormatCLUT8();
		_codec48 = new Codec48Decoder();
		_blocky8 = new Blocky8();
		_blocky16 = nullptr;
	} else {
		_format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		_codec48 = nullptr;
		_blocky8 = nullptr;
		_blocky16 = new Blocky16();
		_blocky16->init(width, height);
	}
	_width = width;
	_height = height;
	_nbframes = numFrames;
	_is16Bit = is16Bit;
	_x = 0;
	_y = 0;
	setMsPerFrame(fps);
	_curFrame = 0;
	for (int i = 0; i < 0x300; i++) {
		_palette[i] = 0;
		_deltaPal[i] = 0;
		_dirtyPalette = false;
	}
	_frameStart = 0;
}

SmushDecoder::SmushVideoTrack::~SmushVideoTrack() {
	delete _codec48;
	delete _blocky8;
	delete _blocky16;
	_surface.free();
}

void SmushDecoder::SmushVideoTrack::init() {
	_curFrame = -1;
	_frameStart = -1;
	if (_is16Bit) { // Retail only
		_surface.create(_width, _height, _format);
	}
}

void SmushDecoder::SmushVideoTrack::finishFrame() {
	_curFrame++;
}

void SmushDecoder::SmushVideoTrack::setFrameStart(int frame) {
	_frameStart = frame - 1;
}

void SmushDecoder::SmushVideoTrack::handleBlocky16(Common::SeekableReadStream *stream, uint32 size) {
	if (_curFrame < _frameStart) {
		return;
	}

	assert(_is16Bit);
	byte *ptr = new byte[size];
	stream->read(ptr, size);

	_blocky16->decode((byte *)_surface.getPixels(), ptr);
	delete[] ptr;
}

void SmushDecoder::SmushVideoTrack::handleFrameObject(Common::SeekableReadStream *stream, uint32 size) {
	if (_curFrame < _frameStart) {
		return;
	}

	assert(!_is16Bit);
	assert(size >= 14);
	byte codec = stream->readByte();
	assert(codec == 47 || codec == 48);
	/* byte codecParam = */ stream->readByte();
	_x = stream->readSint16LE();
	_y = stream->readSint16LE();
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	if (width != _width || height != _height) {
		_width = width;
		_height = height;
		_surface.create(_width, _height, _format);
		_codec48->init(_width, _height);
		_blocky8->init(_width, _height);
	}
	stream->readUint16LE();
	stream->readUint16LE();

	size -= 14;
	byte *ptr = new byte[size];
	stream->read(ptr, size);

	if (codec == 47) {
		_blocky8->decode((byte *)_surface.getPixels(), ptr);
	} else if (codec == 48) {
		_codec48->decode((byte *)_surface.getPixels(), ptr);
	}
	delete[] ptr;
}

static byte delta_color(byte org_color, int16 delta_color) {
	int t = (org_color * 129 + delta_color) / 128;
	return CLIP(t, 0, 255);
}

void SmushDecoder::SmushVideoTrack::handleDeltaPalette(Common::SeekableReadStream *stream, int32 size) {
	if (size == 0x300 * 3 + 4) {
		stream->seek(4, SEEK_CUR);
		for (int i = 0; i < 0x300; i++) {
			_deltaPal[i] = stream->readUint16LE();
		}
		stream->read(_palette, 0x300);
		_dirtyPalette = true;
	} else if (size == 6) {
		for (int i = 0; i < 0x300; i++) {
			_palette[i] = delta_color(_palette[i], _deltaPal[i]);
		}
		_dirtyPalette = true;
	} else {
		error("SmushDecoder::handleDeltaPalette() Wrong size for DeltaPalette");
	}
}

Graphics::Surface *SmushDecoder::SmushVideoTrack::decodeNextFrame() {
	return &_surface;
}

void SmushDecoder::SmushVideoTrack::setMsPerFrame(int ms) {
	_frameRate = Common::Rational(1000000, ms);
}
SmushDecoder::SmushAudioTrack::SmushAudioTrack(Audio::Mixer::SoundType soundType, bool isVima, int freq, int channels) :
	AudioTrack(soundType) {
	_isVima = isVima;
	_channels = channels;
	_freq = freq;
	_queueStream = Audio::makeQueuingAudioStream(_freq, (_channels == 2));
	_IACTpos = 0;
}

SmushDecoder::SmushAudioTrack::~SmushAudioTrack() {
	delete _queueStream;
}

void SmushDecoder::SmushAudioTrack::init() {
	_IACTpos = 0;

	if (_isVima) {
		vimaInit(smushDestTable);
	}
}

void SmushDecoder::SmushAudioTrack::handleVIMA(Common::SeekableReadStream *stream, uint32 size) {
	if (size < 8)
		return;
	int decompressedSize = stream->readUint32BE();
	if (decompressedSize == MKTAG('P', 'S', 'A', 'D')) {
		decompressedSize = stream->readUint32BE();
		if (decompressedSize > (int)size - 8)
			decompressedSize = size - 8;
		if (decompressedSize < 10)
			return;
		stream->skip(10);
		decompressedSize -= 10;
		byte *src = (byte *)malloc(decompressedSize);
		stream->read(src, decompressedSize);

		int flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
		if (_channels == 2) {
			flags |= Audio::FLAG_STEREO;
		}

		if (!_queueStream) {
			_queueStream = Audio::makeQueuingAudioStream(_freq, (_channels == 2));
		}
		_queueStream->queueBuffer(src, decompressedSize, DisposeAfterUse::YES, flags);

		return;
	}
	if (decompressedSize < 0) {
		stream->readUint32BE();
		decompressedSize = stream->readUint32BE();
	}

	byte *src = new byte[size];
	stream->read(src, size);

	// this will be deleted using free() by the stream, so allocate it using malloc().
	int16 *dst = (int16 *)malloc(decompressedSize * _channels * 2);
	decompressVima(src, dst, decompressedSize * _channels * 2, smushDestTable, true);

	int flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
	if (_channels == 2) {
		flags |= Audio::FLAG_STEREO;
	}

	if (!_queueStream) {
		_queueStream = Audio::makeQueuingAudioStream(_freq, (_channels == 2));
	}
	_queueStream->queueBuffer((byte *)dst, decompressedSize * _channels * 2, DisposeAfterUse::YES, flags);
	delete[] src;
}

void SmushDecoder::SmushAudioTrack::handleIACT(Common::SeekableReadStream *stream, int32 size) {
	byte *src = new byte[size];
	stream->read(src, size);

	int32 bsize = size - 18;
	const byte *d_src = src + 18;

	while (bsize > 0) {
		if (_IACTpos >= 2) {
			int32 len = READ_BE_UINT16(_IACToutput) + 2;
			len -= _IACTpos;
			if (len > bsize) {
				memcpy(_IACToutput + _IACTpos, d_src, bsize);
				_IACTpos += bsize;
				bsize = 0;
			} else {
				// this will be deleted using free() by the stream, so allocate it using malloc().
				byte *output_data = (byte *)malloc(4096);
				memcpy(_IACToutput + _IACTpos, d_src, len);
				byte *dst = output_data;
				byte *d_src2 = _IACToutput;
				d_src2 += 2;
				int32 count = 1024;
				byte variable1 = *d_src2++;
				byte variable2 = variable1 / 16;
				variable1 &= 0x0f;
				do {
					byte value;
					value = *(d_src2++);
					if (value == 0x80) {
						*dst++ = *d_src2++;
						*dst++ = *d_src2++;
					} else {
						int16 val = (int8)value << variable2;
						*dst++ = val >> 8;
						*dst++ = (byte)(val);
					}
					value = *(d_src2++);
					if (value == 0x80) {
						*dst++ = *d_src2++;
						*dst++ = *d_src2++;
					} else {
						int16 val = (int8)value << variable1;
						*dst++ = val >> 8;
						*dst++ = (byte)(val);
					}
				} while (--count);

				if (!_queueStream) {
					_queueStream = Audio::makeQueuingAudioStream(22050, true);
				}
				_queueStream->queueBuffer(output_data, 0x1000, DisposeAfterUse::YES, Audio::FLAG_STEREO | Audio::FLAG_16BITS);

				bsize -= len;
				d_src += len;
				_IACTpos = 0;
			}
		} else {
			if (bsize > 1 && _IACTpos == 0) {
				*(_IACToutput + 0) = *d_src++;
				_IACTpos = 1;
				bsize--;
			}
			*(_IACToutput + _IACTpos) = *d_src++;
			_IACTpos++;
			bsize--;
		}
	}
	delete[] src;
}

bool SmushDecoder::SmushAudioTrack::seek(const Audio::Timestamp &time) {
	return true;
}

void SmushDecoder::SmushAudioTrack::skipSamples(int sampleCount) {
	if (sampleCount <= 0)
		return;

	if (_queueStream->isStereo())
		sampleCount *= 2;

	int16 *tempBuffer = new int16[sampleCount];
	_queueStream->readBuffer(tempBuffer, sampleCount);
	delete[] tempBuffer;
}

} // end of namespace Grim
