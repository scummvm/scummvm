/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink

#include "common/endian.h"
#include "common/timer.h"
#include "common/file.h"
#include "common/events.h"
#include "common/system.h"
#include "common/mutex.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "engines/grim/movie/smush.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "engines/grim/savegame.h"

#ifdef USE_SMUSH

namespace Grim {

#define ANNO_HEADER "MakeAnim animation type 'Bl16' parameters: "
#define BUFFER_SIZE 16385

static uint16 smushDestTable[5786];
MoviePlayer *CreateSmushPlayer() {
	return new SmushPlayer();
}

void SmushPlayer::timerCallback(void *) {
	SmushPlayer *smush = static_cast<SmushPlayer *>(g_movie);
	// Use a mutex to protect against multiple threads running handleFrame
	// at the same time.
	Common::StackLock lock(smush->_frameMutex);

	if (g_grim->getGameFlags() & ADGF_DEMO)
		smush->handleFrameDemo();
	else
		smush->handleFrame();
}

SmushPlayer::SmushPlayer() {
	g_movie = this;
	_IACTpos = 0;
	_nbframes = 0;
	_file = 0;
}

SmushPlayer::~SmushPlayer() {
	deinit();
}

void SmushPlayer::init() {
	_IACTpos = 0;
	_stream = NULL;
	_frame = 0;
	_movieTime = 0;
	_videoFinished = false;
	_videoPause = false;
	_updateNeeded = false;

	assert(!_internalBuffer);
	assert(!_externalBuffer);

	if (!(g_grim->getGameFlags() & ADGF_DEMO)) {
		_internalBuffer = new byte[_width * _height * 2];
		_externalBuffer = new byte[_width * _height * 2];
		vimaInit(smushDestTable);
	}
	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, NULL);
	// Get a valid frame to draw
	timerCallback(0);
}

void SmushPlayer::deinit() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);

	if (_internalBuffer) {
		delete[] _internalBuffer;
		_internalBuffer = NULL;
	}
	if (_externalBuffer) {
		delete[] _externalBuffer;
		_externalBuffer = NULL;
	}
	if (_videoLooping && _startPos) {
		delete[] _startPos->tmpBuf;
		delete _startPos;
		_startPos = NULL;
	}
	if (_stream) {
		_stream->finish();
		_stream = NULL;
		g_system->getMixer()->stopHandle(_soundHandle);
	}
	_videoLooping = false;
	_videoFinished = true;
	_videoPause = true;
	if (g_grim->getGameFlags() & ADGF_DEMO)
		_f.close();
	else if (_file) {
		delete _file;
		_file = NULL;
	}
}

void SmushPlayer::handleWave(const byte *src, uint32 size) {
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

void SmushPlayer::handleFrame() {
	uint32 tag;
	int32 size;
	int pos = 0;

	if (_videoPause)
		return;

	if (_videoFinished) {
		_videoPause = true;
		return;
	}

	if (_frame == _nbframes) {
		// If the video has been looping and was previously on the last
		// frame then reset the frame number and the movie time, this
		// needs to occur at the beginning so the last frame has time to
		// render appropriately
		_frame = 1;
		_movieTime = 0;
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
			if (gDebugLevel == DEBUG_SMUSH || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
				debug("Announcement data: %s\n", anno);
			// It looks like the announcement data is actually for setting some of the
			// header parameters, not for any looping purpose
		} else {
			if (gDebugLevel == DEBUG_SMUSH || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
				debug("Announcement header not understood: %s\n", anno);
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
			_blocky16.decode(_internalBuffer, frame + pos + 8);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (READ_BE_UINT32(frame + pos) == MKTAG('W','a','v','e')) {
			int decompressed_size = READ_BE_UINT32(frame + pos + 8);
			if (decompressed_size < 0)
				handleWave(frame + pos + 8 + 4 + 8, READ_BE_UINT32(frame + pos + 8 + 8));
			else
				handleWave(frame + pos + 8 + 4, decompressed_size);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (gDebugLevel == DEBUG_SMUSH || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL) {
			error("SmushPlayer::handleFrame() unknown tag");
		}
	} while (pos < size);
	delete[] frame;

	memcpy(_externalBuffer, _internalBuffer, _width * _height * 2);
	_updateNeeded = true;

	_frame++;
	_movieTime += _speed / 1000.f;
	if (_frame == _nbframes) {
		// If we're not supposed to loop (or looping fails) then end the video
		if (!_videoLooping || !_file->seek(_startPos->filePos, SEEK_SET)) {
			_videoFinished = true;
			g_grim->setMode(ENGINE_MODE_NORMAL);
			return;
		}
	}
}

static byte delta_color(byte org_color, int16 delta_color) {
	int t = (org_color * 129 + delta_color) / 128;
	return CLIP(t, 0, 255);
}

void SmushPlayer::handleDeltaPalette(byte *src, int32 size) {
	if (size == 0x300 * 3 + 4) {
		for (int i = 0; i < 0x300; i++)
			_deltaPal[i] = READ_LE_UINT16(src + (i * 2) + 4);
		memcpy(_pal, src + 0x600 + 4, 0x300);
	} else if (size == 6) {
		for (int i = 0; i < 0x300; i++)
			_pal[i] = delta_color(_pal[i], _deltaPal[i]);
	} else {
		error("SmushPlayer::handleDeltaPalette() Wrong size for DeltaPalette");
	}
}

void SmushPlayer::handleIACT(const byte *src, int32 size) {
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

void SmushPlayer::handleFrameDemo() {
	uint32 tag;
	int32 size;
	int pos = 0;

	if (_videoPause)
		return;

	if (_videoFinished) {
		_videoPause = true;
		return;
	}

	tag = _f.readUint32BE();
	assert(tag == MKTAG('F','R','M','E'));
	size = _f.readUint32BE();
	byte *frame = new byte[size];
	_f.read(frame, size);

	do {
		if (READ_BE_UINT32(frame + pos) == MKTAG('F','O','B','J')) {
			_x = READ_LE_UINT16(frame + pos + 10);
			_y = READ_LE_UINT16(frame + pos + 12);
			int width = READ_LE_UINT16(frame + pos + 14);
			int height = READ_LE_UINT16(frame + pos + 16);
			if (width != _width || height != _height) {
				delete[] _internalBuffer;
				delete[] _externalBuffer;
				_width = width;
				_height = height;
				_internalBuffer = new byte[_width * _height];
				_externalBuffer = new byte[_width * _height * 2];
				_blocky8.init(_width, _height);
			}
			_blocky8.decode(_internalBuffer, frame + pos + 8 + 14);
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
			error("SmushPlayer::handleFrame() unknown tag");
		}
	} while (pos < size);
	delete[] frame;

	uint16 *d = (uint16 *)_externalBuffer;
	for (int l = 0; l < _width * _height; l++) {
		int index = _internalBuffer[l];
		d[l] = ((_pal[(index * 3) + 0] & 0xF8) << 8) | ((_pal[(index * 3) + 1] & 0xFC) << 3) | (_pal[(index * 3) + 2] >> 3);
	}

	_updateNeeded = true;

	_frame++;
	_movieTime += _speed / 1000.f;
	if (_frame == _nbframes) {
		_videoFinished = true;
		g_grim->setMode(ENGINE_MODE_NORMAL);
		return;
	}
}

void SmushPlayer::handleFramesHeader() {
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
			error("SmushPlayer::handleFramesHeader() unknown tag");
		}
	} while (pos < size);
	delete[] f_header;
}

bool SmushPlayer::setupAnimDemo(const char *file) {
	uint32 tag;
	int32 size;

	if (!_f.open(file))
		return false;

	tag = _f.readUint32BE();
	assert(tag == MKTAG('A','N','I','M'));
	size = _f.readUint32BE();

	tag = _f.readUint32BE();
	assert(tag == MKTAG('A','H','D','R'));
	size = _f.readUint32BE();

	_f.readUint16BE(); // version
	_nbframes = _f.readUint16LE();
	_f.readUint16BE(); // unknown

	for (int l = 0; l < 0x300; l++) {
		_pal[l] = _f.readByte();
	}
	_f.readUint32BE();
	_f.readUint32BE();
	_f.readUint32BE();
	_f.readUint32BE();
	_f.readUint32BE();

	_x = -1;
	_y = -1;
	_width = -1;
	_height = -1;
	_videoLooping = false;
	_startPos = NULL;
	_speed = 66667;

	return true;
}

bool SmushPlayer::setupAnim(const char *file, bool looping, int x, int y) {
	uint32 tag;
	int32 size;
	int16 flags;

	_file = wrapCompressedReadStream(g_resourceloader->openNewSubStreamFile(file));
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
		_blocky16.init(width, height);
	}

	_x = x;
	_y = y;
	_width = width;
	_height = height;

	if (looping)
		_speed = READ_LE_UINT32(s_header + 14);
	else
		_speed = 66667;
	flags = READ_LE_UINT16(s_header + 18);
	// Output information for checking out the flags
	if (gDebugLevel == DEBUG_SMUSH || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL) {
		printf("SMUSH Flags:");
		for (int i = 0; i < 16; i++)
			printf(" %d", (flags & (1 << i)) != 0);
		printf("\n");
	}

	_videoLooping = looping;
	_startPos = NULL; // Set later
	delete[] s_header;

	return true;
}

void SmushPlayer::stop() {
	deinit();
	g_grim->setMode(ENGINE_MODE_NORMAL);
}

bool SmushPlayer::play(const char *filename, bool looping, int x, int y) {
	deinit();
	_fname = filename;

	if (gDebugLevel == DEBUG_SMUSH)
		printf("Playing video '%s'.\n", filename);

	// Load the video
	if (g_grim->getGameFlags() & ADGF_DEMO) {
		if (!setupAnimDemo(filename))
			return false;
	} else {
		if (!setupAnim(filename, looping, x, y))
			return false;

		handleFramesHeader();
	}

	if (_videoLooping) {
		_startPos = new SavePos;
		_startPos->tmpBuf = NULL;
		_startPos->filePos = _file->pos();
	}

	init();

	return true;
}

void SmushPlayer::saveState(SaveGame *state) {
	state->beginSection('SMUS');

	state->writeString(_fname);

	state->writeLESint32(_frame);
	state->writeFloat(_movieTime);
	state->writeLESint32(_videoFinished);
	state->writeLESint32(_videoLooping);

	state->writeLESint32(_x);
	state->writeLESint32(_y);

	state->endSection();
}

void SmushPlayer::restoreState(SaveGame *state) {
	state->beginSection('SMUS');

	_fname = state->readString();

	int32 frame = state->readLESint32();
	float movieTime = state->readFloat();
	bool videoFinished = state->readLESint32();
	bool videoLooping = state->readLESint32();

	int x = state->readLESint32();
	int y = state->readLESint32();

	if (!videoFinished) {
		play(_fname.c_str(), videoLooping, x, y);
	}
	_frame = frame;
	_movieTime = movieTime;

	state->endSection();
}

} // end of namespace Grim


#endif // USE_SMUSH
