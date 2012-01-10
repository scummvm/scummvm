/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/rational.h"
#include "common/system.h"
#include "common/timer.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "engines/grim/debug.h"

#include "engines/grim/movie/codecs/blocky8.h"
#include "engines/grim/movie/codecs/blocky16.h"
#include "engines/grim/movie/codecs/smush_decoder.h"

#ifdef USE_SMUSH

namespace Grim {

// Prototypes to avoid depending on grim.h
void vimaInit(uint16 *destTable);
void decompressVima(const byte *src, int16 *dest, int destLen, uint16 *destTable);

#define ANNO_HEADER "MakeAnim animation type 'Bl16' parameters: "
#define BUFFER_SIZE 16385
#define SMUSH_SPEED 66667

static uint16 smushDestTable[5786];

SmushDecoder::SmushDecoder() {
	// Set colour-format statically here for SMUSH (5650), to allow for differing
	// PixelFormat in engine and renderer (and conversion from Surface there)
	// Which means 16 bpp, 565, shift of 11, 5, 0, 0 for RGBA
	_format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_nbframes = 0;
	_file = 0;
	_width = 0;
	_height = 0;
	_channels = -1;
	_freq = 22050;
	_videoLooping = false;
	_startPos = 0;
	_x = 0;
	_y = 0;
	_blocky8 = new Blocky8();
	_blocky16 = new Blocky16();
	_stream = NULL;
}

SmushDecoder::~SmushDecoder() {
	delete _blocky8;
	delete _blocky16;
}

void SmushDecoder::init() {
	_IACTpos = 0;
	_curFrame = -1;
	_videoPause = false;

	if (!_demo) {
		_surface.create(_width, _height, _format);
		vimaInit(smushDestTable);
	}
}

void SmushDecoder::close() {
	_surface.free();
	if (_stream) {
		_stream->finish();
		_stream = NULL;
		g_system->getMixer()->stopHandle(_soundHandle);
	}
	_videoLooping = false;
	_videoPause = true;
	if (_file) {
		delete _file;
		_file = NULL;
	}
}

void SmushDecoder::handleWave(const byte *src, uint32 size) {
	int16 *dst = (int16 *) malloc(size * _channels * sizeof(int16));
	decompressVima(src, dst, size * _channels * 2, smushDestTable);

	int flags = Audio::FLAG_16BITS;
	if (_channels == 2)
		flags |= Audio::FLAG_STEREO;

	if (!_stream) {
		_stream = Audio::makeQueuingAudioStream(_freq, (_channels == 2));
		g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, _stream);
	}
	if (g_system->getMixer()->isReady()) {
		_stream->queueBuffer((byte *)dst, size * _channels * 2, DisposeAfterUse::YES, flags);
	} else {
		free(dst);
	}
}

void SmushDecoder::handleFrame() {
	uint32 tag;
	int32 size;
	int pos = 0;

	if (_videoLooping && _curFrame == _nbframes - 1) {
		_file->seek(_startPos, SEEK_SET);
		_curFrame = -1;
	}

	if (_curFrame == -1)
		_startTime = g_system->getMillis();

	if (_videoPause)
		return;

	if (endOfVideo()) { // Looping is handled outside, by rewinding the video.
		_videoPause = true;
		return;
	}

	tag = _file->readUint32BE();
	if (tag == MKTAG('A','N','N','O')) {
		char *anno;
		byte *data;

		size = _file->readUint32BE();
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
	}

	assert(tag == MKTAG('F','R','M','E'));
	size = _file->readUint32BE();
	byte *frame = new byte[size];
	_file->read(frame, size);

	do {
		if (READ_BE_UINT32(frame + pos) == MKTAG('B','l','1','6')) {
			_blocky16->decode((byte *)_surface.pixels, frame + pos + 8);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (READ_BE_UINT32(frame + pos) == MKTAG('W','a','v','e')) {
			int decompressed_size = READ_BE_UINT32(frame + pos + 8);
			if (decompressed_size < 0)
				handleWave(frame + pos + 8 + 4 + 8, READ_BE_UINT32(frame + pos + 8 + 8));
			else
				handleWave(frame + pos + 8 + 4, decompressed_size);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else {
			Debug::error(Debug::Movie, "SmushDecoder::handleFrame() unknown tag");
		}
	} while (pos < size);
	delete[] frame;

	++_curFrame;
}

static byte delta_color(byte org_color, int16 delta_color) {
	int t = (org_color * 129 + delta_color) / 128;
	return CLIP(t, 0, 255);
}

void SmushDecoder::handleDeltaPalette(byte *src, int32 size) {
	if (size == 0x300 * 3 + 4) {
		for (int i = 0; i < 0x300; i++)
			_deltaPal[i] = READ_LE_UINT16(src + (i * 2) + 4);
		memcpy(_pal, src + 0x600 + 4, 0x300);
	} else if (size == 6) {
		for (int i = 0; i < 0x300; i++)
			_pal[i] = delta_color(_pal[i], _deltaPal[i]);
	} else {
		error("SmushDecoder::handleDeltaPalette() Wrong size for DeltaPalette");
	}
}

void SmushDecoder::handleIACT(const byte *src, int32 size) {
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
				byte *output_data = new byte[4096];
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

				if (!_stream) {
					_stream = Audio::makeQueuingAudioStream(22050, true);
					g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _stream);
				}
				_stream->queueBuffer(output_data, 0x1000, DisposeAfterUse::YES, Audio::FLAG_STEREO | Audio::FLAG_16BITS);

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
}

void SmushDecoder::handleFrameDemo() {
	uint32 tag;
	int32 size;
	int pos = 0;

	if (_videoPause)
		return;

	if (endOfVideo()) {
		_videoPause = true;
		return;
	}

	if (_curFrame == -1)
		_startTime = g_system->getMillis();

	tag = _file->readUint32BE();
	assert(tag == MKTAG('F','R','M','E'));
	size = _file->readUint32BE();
	byte *frame = new byte[size];
	_file->read(frame, size);

	do {
		if (READ_BE_UINT32(frame + pos) == MKTAG('F','O','B','J')) {
			_x = READ_LE_UINT16(frame + pos + 10);
			_y = READ_LE_UINT16(frame + pos + 12);
			int width = READ_LE_UINT16(frame + pos + 14);
			int height = READ_LE_UINT16(frame + pos + 16);
			if (width != _width || height != _height) {
				_width = width;
				_height = height;
				_surface.create(_width, _height, _format);
				_blocky8->init(_width, _height);
			}
			_blocky8->decode((byte *)_surface.pixels, frame + pos + 8 + 14);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (READ_BE_UINT32(frame + pos) == MKTAG('I','A','C','T')) {
			handleIACT(frame + pos + 8, READ_BE_UINT32(frame + pos + 4));
			int offset = READ_BE_UINT32(frame + pos + 4) + 8;
			if (offset & 1)
				offset += 1;
			pos += offset;
		} else if (READ_BE_UINT32(frame + pos) == MKTAG('X','P','A','L')) {
			handleDeltaPalette(frame + pos + 8, READ_BE_UINT32(frame + pos + 4));
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else {
			error("SmushDecoder::handleFrame() unknown tag");
		}
	} while (pos < size);
	delete[] frame;

	Graphics::Surface conversion;
	conversion.create(0, 0, _format); // Avoid issues with copyFrom, by creating an empty surface.
	conversion.copyFrom(_surface);

	uint16 *d = (uint16 *)_surface.pixels;
	for (int l = 0; l < _width * _height; l++) {
		int index = ((byte *)conversion.pixels)[l];
		d[l] = ((_pal[(index * 3) + 0] & 0xF8) << 8) | ((_pal[(index * 3) + 1] & 0xFC) << 3) | (_pal[(index * 3) + 2] >> 3);
	}
	conversion.free();

	_curFrame++;
}

void SmushDecoder::handleFramesHeader() {
	uint32 tag;
	int32 size;
	int pos = 0;

	tag = _file->readUint32BE();
	assert(tag == MKTAG('F','L','H','D'));
	size = _file->readUint32BE();
	byte *f_header = new byte[size];
	_file->read(f_header, size);

	do {
		if (READ_BE_UINT32(f_header + pos) == MKTAG('B','l','1','6')) {
			pos += READ_BE_UINT32(f_header + pos + 4) + 8;
		} else if (READ_BE_UINT32(f_header + pos) == MKTAG('W','a','v','e')) {
			_freq = READ_LE_UINT32(f_header + pos + 8);
			_channels = READ_LE_UINT32(f_header + pos + 12);
			pos += 20;
		} else {
			error("SmushDecoder::handleFramesHeader() unknown tag");
		}
	} while (pos < size);
	delete[] f_header;
}

bool SmushDecoder::setupAnimDemo() {
	uint32 tag;
	int32 size;

	tag = _file->readUint32BE();
	assert(tag == MKTAG('A','N','I','M'));
	size = _file->readUint32BE();

	tag = _file->readUint32BE();
	assert(tag == MKTAG('A','H','D','R'));
	size = _file->readUint32BE();

	_file->readUint16BE(); // version
	_nbframes = _file->readUint16LE();
	_file->readUint16BE(); // unknown

	for (int l = 0; l < 0x300; l++) {
		_pal[l] = _file->readByte();
	}
	_file->readUint32BE();
	_file->readUint32BE();
	_file->readUint32BE();
	_file->readUint32BE();
	_file->readUint32BE();

	_x = -1;
	_y = -1;
	_width = -1;
	_height = -1;
	_videoLooping = false;
	_startPos = 0;

	setMsPerFrame(SMUSH_SPEED);
	return true;
}

bool SmushDecoder::setupAnim() {
	uint32 tag;
	int32 size;
	int16 flags;

	if (!_file)
		return false;

	tag = _file->readUint32BE();
	assert(tag == MKTAG('S','A','N','M'));
	size = _file->readUint32BE();

	tag = _file->readUint32BE();
	assert(tag == MKTAG('S','H','D','R'));
	size = _file->readUint32BE();
	byte *s_header = new byte[size];
	_file->read(s_header, size);
	_nbframes = READ_LE_UINT32(s_header + 2);
	int width = READ_LE_UINT16(s_header + 8);
	int height = READ_LE_UINT16(s_header + 10);
	if (_width != width || _height != height) {
		_blocky16->init(width, height);
	}

	_width = width;
	_height = height;

	// If the video is NOT looping, setLooping will set the speed to the proper value
	setMsPerFrame(READ_LE_UINT32(s_header + 14));

	flags = READ_LE_UINT16(s_header + 18);
	// Output information for checking out the flags
	if (Debug::isChannelEnabled(Debug::Movie | Debug::Info)) {
		warning("SMUSH Flags:");
		for (int i = 0; i < 16; i++)
			warning(" %d", (flags & (1 << i)) != 0);
		//printf("\n");
	}
	_videoLooping = true;

	delete[] s_header;

	return true;
}

bool SmushDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_file = stream;

	// Load the video
	if (_demo) {
		if (!setupAnimDemo())
			return false;
	} else {
		if (!setupAnim())
			return false;

		handleFramesHeader();
	}

	_startPos = _file->pos();

	init();
	if (!_demo)
		_surface.create(_width, _height, _format);
	return true;
}

const Graphics::Surface *SmushDecoder::decodeNextFrame() {
	if (_demo)
		handleFrameDemo();
	else
		handleFrame();

	return &_surface;
}

void SmushDecoder::setLooping(bool l) {
	_videoLooping = l;

	if (!_videoLooping)
		setMsPerFrame(SMUSH_SPEED);
}

void SmushDecoder::pauseVideoIntern(bool p) {
	g_system->getMixer()->pauseHandle(_soundHandle, p);
}

uint32 SmushDecoder::getFrameCount() const {
	return _nbframes;
}

void SmushDecoder::setMsPerFrame(int ms) {
	_frameRate = Common::Rational(1000000, ms);
}

void SmushDecoder::seekToTime(Audio::Timestamp time) { // FIXME: This will be off by a second or two right now.
	int32 wantedFrame = (uint32) ((time.msecs() / 1000.0f) * getFrameRate().toDouble());
	warning("Seek to time: %d, frame: %d", time.msecs(), wantedFrame);
	warning("Current frame: %d", _curFrame);
	uint32 tag;
	int32 size;

	if (_stream)
		_stream->finish();

	if (wantedFrame > _nbframes)
		return;

	if (wantedFrame < _curFrame) {
		_file->seek(_startPos, SEEK_SET);
	}

	_videoPause = true;
	_startTime = g_system->getMillis() - time.msecs(); // This won't be correct, as we should round off to the frame-start.

	while(_curFrame < wantedFrame) {
		tag = _file->readUint32BE();
		if (tag == MKTAG('A','N','N','O')) {
			size = _file->readUint32BE();
			_file->seek(size, SEEK_CUR);
			tag = _file->readUint32BE();

		}
		assert(tag == MKTAG('F','R','M','E'));
		size = _file->readUint32BE();
		_file->seek(size, SEEK_CUR);
		_curFrame++;
	}

	warning("Seek complete");
	_videoPause = false;
}

uint32 SmushDecoder::getDuration() const {
	return (uint32) (getFrameCount() / getFrameRate().toDouble());
}

uint32 SmushDecoder::getTimeToNextFrame() const {
	if (endOfVideo()) { //handle looping
		uint32 elapsedTime = getElapsedTime();

		Common::Rational beginTime = (_curFrame + 1) * 1000;
		beginTime /= getFrameRate();
		uint32 nextFrameStartTime = beginTime.toInt();

		// If the time that the next frame should be shown has past
		// the frame should be shown ASAP.
		if (nextFrameStartTime <= elapsedTime)
			return 0;

		return nextFrameStartTime - elapsedTime;
	} else {
		return FixedRateVideoDecoder::getTimeToNextFrame();
	}
}

} // end of namespace Grim

#endif // USE_SMUSH
