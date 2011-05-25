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

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "video/qt_decoder.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/util.h"
#include "common/zlib.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/types.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"


#include "audio/audiostream.h"

// Audio codecs
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"

// Video codecs
#include "video/codecs/codec.h"
#include "video/codecs/cinepak.h"
#include "video/codecs/mjpeg.h"
#include "video/codecs/qdm2.h"
#include "video/codecs/qtrle.h"
#include "video/codecs/rpza.h"
#include "video/codecs/smc.h"
#include "video/codecs/cdtoons.h"


namespace Video {

////////////////////////////////////////////
// QuickTimeDecoder
////////////////////////////////////////////

QuickTimeDecoder::QuickTimeDecoder() {
	_audStream = NULL;
	_beginOffset = 0;
	_curFrame = -1;
	_startTime = _nextFrameStartTime = 0;
	_audHandle = Audio::SoundHandle();
	_numStreams = 0;
	_fd = 0;
	_scaledSurface = 0;
	_scaleFactorX = 1;
	_scaleFactorY = 1;
	_dirtyPalette = false;
	_resFork = new Common::MacResManager();
	_palette = 0;

	initParseTable();
}

QuickTimeDecoder::~QuickTimeDecoder() {
	close();
	delete _resFork;
}

uint16 QuickTimeDecoder::getWidth() const {
	if (_videoStreamIndex < 0)
		return 0;

	return (Common::Rational(_streams[_videoStreamIndex]->width) / getScaleFactorX()).toInt();
}

uint16 QuickTimeDecoder::getHeight() const {
	if (_videoStreamIndex < 0)
		return 0;

	return (Common::Rational(_streams[_videoStreamIndex]->height) / getScaleFactorY()).toInt();
}

uint32 QuickTimeDecoder::getFrameCount() const {
	if (_videoStreamIndex < 0)
		return 0;

	return _streams[_videoStreamIndex]->nb_frames;
}

Common::Rational QuickTimeDecoder::getScaleFactorX() const {
	if (_videoStreamIndex < 0)
		return 1;

	return (_scaleFactorX * _streams[_videoStreamIndex]->scaleFactorX);
}

Common::Rational QuickTimeDecoder::getScaleFactorY() const {
	if (_videoStreamIndex < 0)
		return 1;

	return (_scaleFactorY * _streams[_videoStreamIndex]->scaleFactorY);
}

uint32 QuickTimeDecoder::getFrameDuration() {
	if (_videoStreamIndex < 0)
		return 0;

	uint32 curFrameIndex = 0;
	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count; i++) {
		curFrameIndex += _streams[_videoStreamIndex]->stts_data[i].count;
		if ((uint32)_curFrame < curFrameIndex) {
			// Ok, now we have what duration this frame has.
			return _streams[_videoStreamIndex]->stts_data[i].duration;
		}
	}

	// This should never occur
	error ("Cannot find duration for frame %d", _curFrame);
	return 0;
}

Graphics::PixelFormat QuickTimeDecoder::getPixelFormat() const {
	Codec *codec = findDefaultVideoCodec();

	if (!codec)
		return Graphics::PixelFormat::createFormatCLUT8();

	return codec->getPixelFormat();
}

uint32 QuickTimeDecoder::findKeyFrame(uint32 frame) const {
	for (int i = _streams[_videoStreamIndex]->keyframe_count - 1; i >= 0; i--)
		if (_streams[_videoStreamIndex]->keyframes[i] <= frame)
				return _streams[_videoStreamIndex]->keyframes[i];
	
	// If none found, we'll assume the requested frame is a key frame
	return frame;
}

void QuickTimeDecoder::seekToFrame(uint32 frame) {
	assert(_videoStreamIndex >= 0);
	assert(frame < _streams[_videoStreamIndex]->nb_frames);

	// Stop all audio (for now)
	stopAudio();

	// Track down the keyframe
	_curFrame = findKeyFrame(frame) - 1;
	while (_curFrame < (int32)frame - 1)
		decodeNextFrame();

	// Map out the starting point
	_nextFrameStartTime = 0;
	uint32 curFrame = 0;

	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count && curFrame < frame; i++) {
		for (int32 j = 0; j < _streams[_videoStreamIndex]->stts_data[i].count && curFrame < frame; j++) {
			curFrame++;
			_nextFrameStartTime += _streams[_videoStreamIndex]->stts_data[i].duration;
		}
	}

	// Adjust the video starting point
	const Audio::Timestamp curVideoTime(0, _nextFrameStartTime, _streams[_videoStreamIndex]->time_scale);
	_startTime = g_system->getMillis() - curVideoTime.msecs();
	resetPauseStartTime();

	// Adjust the audio starting point
	if (_audioStreamIndex >= 0) {
		_audioStartOffset = curVideoTime;

		// Re-create the audio stream
		STSDEntry *entry = &_streams[_audioStreamIndex]->stsdEntries[0];
		_audStream = Audio::makeQueuingAudioStream(entry->sampleRate, entry->channels == 2);

		// First, we need to track down what audio sample we need
		Audio::Timestamp curAudioTime(0, _streams[_audioStreamIndex]->time_scale);
		uint sample = 0;
		bool done = false;
		for (int32 i = 0; i < _streams[_audioStreamIndex]->stts_count && !done; i++) {
			for (int32 j = 0; j < _streams[_audioStreamIndex]->stts_data[i].count; j++) {
				curAudioTime = curAudioTime.addFrames(_streams[_audioStreamIndex]->stts_data[i].duration);

				if (curAudioTime > curVideoTime) {
					done = true;
					break;
				}

				sample++;
			}
		}

		// Now to track down what chunk it's in
		_curAudioChunk = 0;
		uint32 totalSamples = 0;
		for (uint32 i = 0; i < _streams[_audioStreamIndex]->chunk_count; i++, _curAudioChunk++) {
			int sampleToChunkIndex = -1;

			for (uint32 j = 0; j < _streams[_audioStreamIndex]->sample_to_chunk_sz; j++)
				if (i >= _streams[_audioStreamIndex]->sample_to_chunk[j].first)
					sampleToChunkIndex = j;

			assert(sampleToChunkIndex >= 0);

			totalSamples += _streams[_audioStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;

			if (sample < totalSamples) {
				totalSamples -= _streams[_audioStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;
				break;
			}
		}
		
		// Reposition the audio stream
		readNextAudioChunk();
		if (sample != totalSamples) {
			// HACK: Skip a certain amount of samples from the stream
			// (There's got to be a better way to do this!)
			int16 *tempBuffer = new int16[sample - totalSamples];
			_audStream->readBuffer(tempBuffer, sample - totalSamples);
			delete[] tempBuffer;
			debug(3, "Skipping %d audio samples", sample - totalSamples);
		}
		
		// Restart the audio
		startAudio();
	}
}

void QuickTimeDecoder::seekToTime(Audio::Timestamp time) {
	// TODO: Audio-only seeking (or really, have QuickTime sounds)
	if (_videoStreamIndex < 0)
		error("Audio-only seeking not supported");

	// Try to find the last frame that should have been decoded
	uint32 frame = 0;
	Audio::Timestamp totalDuration(0, _streams[_videoStreamIndex]->time_scale);
	bool done = false;

	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count && !done; i++) {
		for (int32 j = 0; j < _streams[_videoStreamIndex]->stts_data[i].count; j++) {
			totalDuration = totalDuration.addFrames(_streams[_videoStreamIndex]->stts_data[i].duration);
			if (totalDuration > time) {
				done = true;
				break;
			}
			frame++;
		}
	}

	seekToFrame(frame);
}

Codec *QuickTimeDecoder::createCodec(uint32 codecTag, byte bitsPerPixel) {
	if (codecTag == MKTAG('c','v','i','d')) {
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this.
		return new CinepakDecoder(bitsPerPixel);
	} else if (codecTag == MKTAG('r','p','z','a')) {
		// Apple Video ("Road Pizza"): Used by some Myst videos.
		return new RPZADecoder(getWidth(), getHeight());
	} else if (codecTag == MKTAG('r','l','e',' ')) {
		// QuickTime RLE: Used by some Myst ME videos.
		return new QTRLEDecoder(getWidth(), getHeight(), bitsPerPixel);
	} else if (codecTag == MKTAG('s','m','c',' ')) {
		// Apple SMC: Used by some Myst videos.
		return new SMCDecoder(getWidth(), getHeight());
	} else if (codecTag == MKTAG('S','V','Q','1')) {
		// Sorenson Video 1: Used by some Myst ME videos.
		warning("Sorenson Video 1 not yet supported");
	} else if (codecTag == MKTAG('S','V','Q','3')) {
		// Sorenson Video 3: Used by some Myst ME videos.
		warning("Sorenson Video 3 not yet supported");
	} else if (codecTag == MKTAG('j','p','e','g')) {
		// Motion JPEG: Used by some Myst ME 10th Anniversary videos.
		return new JPEGDecoder();
	} else if (codecTag == MKTAG('Q','k','B','k')) {
		// CDToons: Used by most of the Broderbund games.
		return new CDToonsDecoder(getWidth(), getHeight());
	} else {
		warning("Unsupported codec \'%s\'", tag2str(codecTag));
	}

	return NULL;
}

void QuickTimeDecoder::startAudio() {
	if (_audStream) { // No audio/audio not supported
		updateAudioBuffer();
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_audHandle, _audStream);
	}
}

void QuickTimeDecoder::stopAudio() {
	if (_audStream) {
		g_system->getMixer()->stopHandle(_audHandle);
		_audStream = NULL; // the mixer automatically frees the stream
	}
}

void QuickTimeDecoder::pauseVideoIntern(bool pause) {
	if (_audStream)
		g_system->getMixer()->pauseHandle(_audHandle, pause);
}

Codec *QuickTimeDecoder::findDefaultVideoCodec() const {
	if (_videoStreamIndex < 0 || !_streams[_videoStreamIndex]->stsdEntryCount)
		return 0;

	return _streams[_videoStreamIndex]->stsdEntries[0].videoCodec;
}

const Graphics::Surface *QuickTimeDecoder::decodeNextFrame() {
	if (_videoStreamIndex < 0 || _curFrame >= (int32)getFrameCount() - 1)
		return 0;

	if (_startTime == 0)
		_startTime = g_system->getMillis();

	_curFrame++;
	_nextFrameStartTime += getFrameDuration();

	// Update the audio while we're at it
	updateAudioBuffer();

	// Get the next packet
	uint32 descId;
	Common::SeekableReadStream *frameData = getNextFramePacket(descId);

	if (!frameData || !descId || descId > _streams[_videoStreamIndex]->stsdEntryCount)
		return 0;

	// Find which video description entry we want
	STSDEntry *entry = &_streams[_videoStreamIndex]->stsdEntries[descId - 1];

	if (!entry->videoCodec)
		return 0;

	const Graphics::Surface *frame = entry->videoCodec->decodeImage(frameData);
	delete frameData;

	// Update the palette
	if (entry->videoCodec->containsPalette()) {
		// The codec itself contains a palette
		if (entry->videoCodec->hasDirtyPalette()) {
			_palette = entry->videoCodec->getPalette();
			_dirtyPalette = true;
		}
	} else {
		// Check if the video description has been updated
		byte *palette = entry->palette;

		if (palette != _palette) {
			_palette = palette;
			_dirtyPalette = true;
		}
	}

	return scaleSurface(frame);
}

const Graphics::Surface *QuickTimeDecoder::scaleSurface(const Graphics::Surface *frame) {
	if (getScaleFactorX() == 1 && getScaleFactorY() == 1)
		return frame;

	assert(_scaledSurface);

	for (int32 j = 0; j < _scaledSurface->h; j++)
		for (int32 k = 0; k < _scaledSurface->w; k++)
			memcpy(_scaledSurface->getBasePtr(k, j), frame->getBasePtr((k * getScaleFactorX()).toInt() , (j * getScaleFactorY()).toInt()), frame->format.bytesPerPixel);

	return _scaledSurface;
}

bool QuickTimeDecoder::endOfVideo() const {
	return (!_audStream || _audStream->endOfData()) && (!findDefaultVideoCodec() || SeekableVideoDecoder::endOfVideo());
}

uint32 QuickTimeDecoder::getElapsedTime() const {
	if (_audStream)
		return g_system->getMixer()->getSoundElapsedTime(_audHandle) + _audioStartOffset.msecs();

	return SeekableVideoDecoder::getElapsedTime();
}

uint32 QuickTimeDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _curFrame < 0)
		return 0;

	// Convert from the QuickTime rate base to 1000
	uint32 nextFrameStartTime = _nextFrameStartTime * 1000 / _streams[_videoStreamIndex]->time_scale;
	uint32 elapsedTime = getElapsedTime();

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

bool QuickTimeDecoder::loadFile(const Common::String &filename) {
	if (!_resFork->open(filename) || !_resFork->hasDataFork())
		return false;

	_foundMOOV = false;
	_numStreams = 0;
	_videoStreamIndex = _audioStreamIndex = -1;
	_startTime = 0;

	MOVatom atom = { 0, 0, 0xffffffff };

	if (_resFork->hasResFork()) {
		// Search for a 'moov' resource
		Common::MacResIDArray idArray = _resFork->getResIDArray(MKTAG('m','o','o','v'));

		if (!idArray.empty())
			_fd = _resFork->getResource(MKTAG('m','o','o','v'), idArray[0]);

		if (_fd) {
			atom.size = _fd->size();
			if (readDefault(atom) < 0 || !_foundMOOV)
				return false;
		}
		delete _fd;

		atom.type = 0;
		atom.offset = 0;
		atom.size = 0xffffffff;
	}

	_fd = _resFork->getDataFork();

	if (readDefault(atom) < 0 || !_foundMOOV)
		return false;

	init();
	return true;
}

bool QuickTimeDecoder::loadStream(Common::SeekableReadStream *stream) {
	_fd = stream;
	_foundMOOV = false;
	_numStreams = 0;
	_videoStreamIndex = _audioStreamIndex = -1;
	_startTime = 0;

	MOVatom atom = { 0, 0, 0xffffffff };

	if (readDefault(atom) < 0 || !_foundMOOV) {
		_fd = 0;
		return false;
	}

	init();
	return true;
}

void QuickTimeDecoder::init() {
	// Remove non-Video/Audio streams
	for (uint32 i = 0; i < _numStreams;) {
		if (_streams[i]->codec_type == CODEC_TYPE_MOV_OTHER) {
			delete _streams[i];
			for (uint32 j = i + 1; j < _numStreams; j++)
				_streams[j - 1] = _streams[j];
			_numStreams--;
		} else
			i++;
	}

	// Adjust time/duration
	for (uint32 i = 0; i < _numStreams; i++) {
		MOVStreamContext *sc = _streams[i];

		if (!sc->time_rate)
			sc->time_rate = 1;

		if (!sc->time_scale)
			sc->time_scale = _timeScale;

		sc->duration /= sc->time_rate;

		if (sc->codec_type == CODEC_TYPE_VIDEO && _videoStreamIndex < 0)
			_videoStreamIndex = i;
		else if (sc->codec_type == CODEC_TYPE_AUDIO && _audioStreamIndex < 0)
			_audioStreamIndex = i;
	}

	// Initialize audio, if present
	if (_audioStreamIndex >= 0) {
		STSDEntry *entry = &_streams[_audioStreamIndex]->stsdEntries[0];

		if (checkAudioCodecSupport(entry->codecTag)) {
			_audStream = Audio::makeQueuingAudioStream(entry->sampleRate, entry->channels == 2);
			_curAudioChunk = 0;

			// Make sure the bits per sample transfers to the sample size
			if (entry->codecTag == MKTAG('r','a','w',' ') || entry->codecTag == MKTAG('t','w','o','s'))
				_streams[_audioStreamIndex]->sample_size = (entry->bitsPerSample / 8) * entry->channels;

			startAudio();
		}

		_audioStartOffset = Audio::Timestamp(0);
	}

	// Initialize video, if present
	if (_videoStreamIndex >= 0) {
		for (uint32 i = 0; i < _streams[_videoStreamIndex]->stsdEntryCount; i++) {
			STSDEntry *entry = &_streams[_videoStreamIndex]->stsdEntries[i];
			entry->videoCodec = createCodec(entry->codecTag, entry->bitsPerSample & 0x1F);
		}

		if (getScaleFactorX() != 1 || getScaleFactorY() != 1) {
			// We have to initialize the scaled surface
			_scaledSurface = new Graphics::Surface();
			_scaledSurface->create(getWidth(), getHeight(), getPixelFormat());
		}
	}
}

void QuickTimeDecoder::initParseTable() {
	static const ParseTable p[] = {
		{ &QuickTimeDecoder::readDefault, MKTAG('d','i','n','f') },
		{ &QuickTimeDecoder::readLeaf,    MKTAG('d','r','e','f') },
		{ &QuickTimeDecoder::readDefault, MKTAG('e','d','t','s') },
		{ &QuickTimeDecoder::readELST,    MKTAG('e','l','s','t') },
		{ &QuickTimeDecoder::readHDLR,    MKTAG('h','d','l','r') },
		{ &QuickTimeDecoder::readDefault, MKTAG('m','d','a','t') },
		{ &QuickTimeDecoder::readMDHD,    MKTAG('m','d','h','d') },
		{ &QuickTimeDecoder::readDefault, MKTAG('m','d','i','a') },
		{ &QuickTimeDecoder::readDefault, MKTAG('m','i','n','f') },
		{ &QuickTimeDecoder::readMOOV,    MKTAG('m','o','o','v') },
		{ &QuickTimeDecoder::readMVHD,    MKTAG('m','v','h','d') },
		{ &QuickTimeDecoder::readLeaf,    MKTAG('s','m','h','d') },
		{ &QuickTimeDecoder::readDefault, MKTAG('s','t','b','l') },
		{ &QuickTimeDecoder::readSTCO,    MKTAG('s','t','c','o') },
		{ &QuickTimeDecoder::readSTSC,    MKTAG('s','t','s','c') },
		{ &QuickTimeDecoder::readSTSD,    MKTAG('s','t','s','d') },
		{ &QuickTimeDecoder::readSTSS,    MKTAG('s','t','s','s') },
		{ &QuickTimeDecoder::readSTSZ,    MKTAG('s','t','s','z') },
		{ &QuickTimeDecoder::readSTTS,    MKTAG('s','t','t','s') },
		{ &QuickTimeDecoder::readTKHD,    MKTAG('t','k','h','d') },
		{ &QuickTimeDecoder::readTRAK,    MKTAG('t','r','a','k') },
		{ &QuickTimeDecoder::readLeaf,    MKTAG('u','d','t','a') },
		{ &QuickTimeDecoder::readLeaf,    MKTAG('v','m','h','d') },
		{ &QuickTimeDecoder::readCMOV,    MKTAG('c','m','o','v') },
		{ &QuickTimeDecoder::readWAVE,    MKTAG('w','a','v','e') },
		{ 0, 0 }
	};

	_parseTable = p;
}

int QuickTimeDecoder::readDefault(MOVatom atom) {
	uint32 total_size = 0;
	MOVatom a;
	int err = 0;

	a.offset = atom.offset;

	while(((total_size + 8) < atom.size) && !_fd->eos() && _fd->pos() < _fd->size() && !err) {
		a.size = atom.size;
		a.type = 0;

		if (atom.size >= 8) {
			a.size = _fd->readUint32BE();
			a.type = _fd->readUint32BE();

			// Some QuickTime videos with resource forks have mdat chunks
			// that are of size 0. Adjust it so it's the correct size.
			if (a.type == MKTAG('m','d','a','t') && a.size == 0)
				a.size = _fd->size();
		}

		total_size += 8;
		a.offset += 8;
		debug(4, "type: %08x  %.4s  sz: %x %x %x", a.type, tag2str(a.type), a.size, atom.size, total_size);

		if (a.size == 1) { // 64 bit extended size
			warning("64 bit extended size is not supported in QuickTime");
			return -1;
		}

		if (a.size == 0) {
			a.size = atom.size - total_size;
			if (a.size <= 8)
				break;
		}

		uint32 i = 0;

		for (; _parseTable[i].type != 0 && _parseTable[i].type != a.type; i++)
			// empty;

		if (a.size < 8)
			break;

		a.size -= 8;

		if (_parseTable[i].type == 0) { // skip leaf atoms data
			debug(0, ">>> Skipped [%s]", tag2str(a.type));

			_fd->seek(a.size, SEEK_CUR);
		} else {
			uint32 start_pos = _fd->pos();
			err = (this->*_parseTable[i].func)(a);

			uint32 left = a.size - _fd->pos() + start_pos;

			if (left > 0) // skip garbage at atom end
				_fd->seek(left, SEEK_CUR);
		}

		a.offset += a.size;
		total_size += a.size;
	}

	if (!err && total_size < atom.size)
		_fd->seek(atom.size - total_size, SEEK_SET);

	return err;
}

int QuickTimeDecoder::readLeaf(MOVatom atom) {
	if (atom.size > 1)
		_fd->seek(atom.size, SEEK_SET);

	return 0;
}

int QuickTimeDecoder::readMOOV(MOVatom atom) {
	if (readDefault(atom) < 0)
		return -1;

	// We parsed the 'moov' atom, so we don't need anything else
	_foundMOOV = true;
	return 1;
}

int QuickTimeDecoder::readCMOV(MOVatom atom) {
#ifdef USE_ZLIB
	// Read in the dcom atom
	_fd->readUint32BE();
	if (_fd->readUint32BE() != MKTAG('d','c','o','m'))
		return -1;
	if (_fd->readUint32BE() != MKTAG('z','l','i','b')) {
		warning("Unknown cmov compression type");
		return -1;
	}

	// Read in the cmvd atom
	uint32 compressedSize = _fd->readUint32BE() - 12;
	if (_fd->readUint32BE() != MKTAG('c','m','v','d'))
		return -1;
	uint32 uncompressedSize = _fd->readUint32BE();

	// Read in data
	byte *compressedData = (byte *)malloc(compressedSize);
	_fd->read(compressedData, compressedSize);

	// Create uncompressed stream
	byte *uncompressedData = (byte *)malloc(uncompressedSize);

	// Uncompress the data
	unsigned long dstLen = uncompressedSize;
	if (!Common::uncompress(uncompressedData, &dstLen, compressedData, compressedSize)) {
		warning ("Could not uncompress cmov chunk");
		free(compressedData);
		free(uncompressedData);
		return -1;
	}

	// Load data into a new MemoryReadStream and assign _fd to be that
	Common::SeekableReadStream *oldStream = _fd;
	_fd = new Common::MemoryReadStream(uncompressedData, uncompressedSize, DisposeAfterUse::YES);

	// Read the contents of the uncompressed data
	MOVatom a = { MKTAG('m','o','o','v'), 0, uncompressedSize };
	int err = readDefault(a);

	// Assign the file handle back to the original handle
	free(compressedData);
	delete _fd;
	_fd = oldStream;

	return err;
#else
	warning ("zlib not found, cannot read QuickTime cmov atom");
	return -1;
#endif
}

int QuickTimeDecoder::readMVHD(MOVatom atom) {
	byte version = _fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	if (version == 1) {
		warning("QuickTime version 1");
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	_timeScale = _fd->readUint32BE(); // time scale
	debug(0, "time scale = %i\n", _timeScale);

	// duration
	_duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE();
	_fd->readUint32BE(); // preferred scale

	_fd->readUint16BE(); // preferred volume

	_fd->seek(10, SEEK_CUR); // reserved

	// We only need two values from the movie display matrix. Most of the values are just
	// skipped. xMod and yMod are 16:16 fixed point numbers, the last part of the 3x3 matrix
	// is 2:30.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	_scaleFactorX = Common::Rational(0x10000, xMod);
	_scaleFactorY = Common::Rational(0x10000, yMod);

	_scaleFactorX.debugPrint(1, "readMVHD(): scaleFactorX =");
	_scaleFactorY.debugPrint(1, "readMVHD(): scaleFactorY =");

	_fd->readUint32BE(); // preview time
	_fd->readUint32BE(); // preview duration
	_fd->readUint32BE(); // poster time
	_fd->readUint32BE(); // selection time
	_fd->readUint32BE(); // selection duration
	_fd->readUint32BE(); // current time
	_fd->readUint32BE(); // next track ID

	return 0;
}

int QuickTimeDecoder::readTRAK(MOVatom atom) {
	MOVStreamContext *sc = new MOVStreamContext();

	if (!sc)
		return -1;

	sc->codec_type = CODEC_TYPE_MOV_OTHER;
	sc->start_time = 0; // XXX: check
	_streams[_numStreams++] = sc;

	return readDefault(atom);
}

int QuickTimeDecoder::readTKHD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	byte version = _fd->readByte();

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags
	//
	//MOV_TRACK_ENABLED 0x0001
	//MOV_TRACK_IN_MOVIE 0x0002
	//MOV_TRACK_IN_PREVIEW 0x0004
	//MOV_TRACK_IN_POSTER 0x0008
	//

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	/* st->id = */_fd->readUint32BE(); // track id (NOT 0 !)
	_fd->readUint32BE(); // reserved
	//st->start_time = 0; // check
	(version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // highlevel (considering edits) duration in movie timebase
	_fd->readUint32BE(); // reserved
	_fd->readUint32BE(); // reserved

	_fd->readUint16BE(); // layer
	_fd->readUint16BE(); // alternate group
	_fd->readUint16BE(); // volume
	_fd->readUint16BE(); // reserved

	// We only need the two values from the displacement matrix for a track.
	// See readMVHD() for more information.
	uint32 xMod = _fd->readUint32BE();
	_fd->skip(12);
	uint32 yMod = _fd->readUint32BE();
	_fd->skip(16);

	st->scaleFactorX = Common::Rational(0x10000, xMod);
	st->scaleFactorY = Common::Rational(0x10000, yMod);

	st->scaleFactorX.debugPrint(1, "readTKHD(): scaleFactorX =");
	st->scaleFactorY.debugPrint(1, "readTKHD(): scaleFactorY =");

	// these are fixed-point, 16:16
	// uint32 tkWidth = _fd->readUint32BE() >> 16; // track width
	// uint32 tkHeight = _fd->readUint32BE() >> 16; // track height

	return 0;
}

// edit list atom
int QuickTimeDecoder::readELST(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->editCount = _fd->readUint32BE();
	st->editList = new EditListEntry[st->editCount];

	debug(2, "Track %d edit list count: %d", _numStreams - 1, st->editCount);

	for (uint32 i = 0; i < st->editCount; i++){
		st->editList[i].trackDuration = _fd->readUint32BE();
		st->editList[i].mediaTime = _fd->readSint32BE();
		st->editList[i].mediaRate = Common::Rational(_fd->readUint32BE(), 0x10000);
		debugN(3, "\tDuration = %d, Media Time = %d, ", st->editList[i].trackDuration, st->editList[i].mediaTime);
		st->editList[i].mediaRate.debugPrint(3, "Media Rate =");
	}

	if (st->editCount != 1)
		warning("Multiple edit list entries. Things may go awry");

	return 0;
}

int QuickTimeDecoder::readHDLR(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	// component type
	uint32 ctype = _fd->readUint32LE();
	uint32 type = _fd->readUint32BE(); // component subtype

	debug(0, "ctype= %s (0x%08lx)", tag2str(ctype), (long)ctype);
	debug(0, "stype= %s", tag2str(type));

	if(ctype == MKTAG('m','h','l','r')) // MOV
		debug(0, "MOV detected");
	else if(ctype == 0) {
		warning("MP4 streams are not supported");
		return -1;
	}

	if (type == MKTAG('v','i','d','e'))
		st->codec_type = CODEC_TYPE_VIDEO;
	else if (type == MKTAG('s','o','u','n'))
		st->codec_type = CODEC_TYPE_AUDIO;

	_fd->readUint32BE(); // component manufacture
	_fd->readUint32BE(); // component flags
	_fd->readUint32BE(); // component flags mask

	if (atom.size <= 24)
		return 0; // nothing left to read

	// .mov: PASCAL string
	byte len = _fd->readByte();
	_fd->seek(len, SEEK_CUR);

	_fd->seek(atom.size - (_fd->pos() - atom.offset), SEEK_CUR);

	return 0;
}

int QuickTimeDecoder::readMDHD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	byte version = _fd->readByte();

	if (version > 1)
		return 1; // unsupported

	_fd->readByte(); _fd->readByte();
	_fd->readByte(); // flags

	if (version == 1) {
		_fd->readUint32BE(); _fd->readUint32BE();
		_fd->readUint32BE(); _fd->readUint32BE();
	} else {
		_fd->readUint32BE(); // creation time
		_fd->readUint32BE(); // modification time
	}

	st->time_scale = _fd->readUint32BE();
	st->duration = (version == 1) ? (_fd->readUint32BE(), _fd->readUint32BE()) : _fd->readUint32BE(); // duration

	_fd->readUint16BE(); // language
	_fd->readUint16BE(); // quality

	return 0;
}

int QuickTimeDecoder::readSTSD(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->stsdEntryCount = _fd->readUint32BE();
	st->stsdEntries = new STSDEntry[st->stsdEntryCount];

	for (uint32 i = 0; i < st->stsdEntryCount; i++) { // Parsing Sample description table
		STSDEntry *entry = &st->stsdEntries[i];

		MOVatom a = { 0, 0, 0 };
		uint32 start_pos = _fd->pos();
		int size = _fd->readUint32BE(); // size
		uint32 format = _fd->readUint32BE(); // data format

		_fd->readUint32BE(); // reserved
		_fd->readUint16BE(); // reserved
		_fd->readUint16BE(); // index

		debug(0, "size=%d 4CC= %s codec_type=%d", size, tag2str(format), st->codec_type);

		entry->codecTag = format;

		if (st->codec_type == CODEC_TYPE_VIDEO) {
			debug(0, "Video Codec FourCC: \'%s\'", tag2str(format));

			_fd->readUint16BE(); // version
			_fd->readUint16BE(); // revision level
			_fd->readUint32BE(); // vendor
			_fd->readUint32BE(); // temporal quality
			_fd->readUint32BE(); // spacial quality

			uint16 width = _fd->readUint16BE(); // width
			uint16 height = _fd->readUint16BE(); // height

			// The width is most likely invalid for entries after the first one
			// so only set the overall width if it is not zero here.
			if (width)
				st->width = width;

			if (height)
				st->height = height;

			_fd->readUint32BE(); // horiz resolution
			_fd->readUint32BE(); // vert resolution
			_fd->readUint32BE(); // data size, always 0
			_fd->readUint16BE(); // frames per samples

			byte codec_name[32];
			_fd->read(codec_name, 32); // codec name, pascal string (FIXME: true for mp4?)
			if (codec_name[0] <= 31) {
				memcpy(entry->codecName, &codec_name[1], codec_name[0]);
				entry->codecName[codec_name[0]] = 0;
			}

			entry->bitsPerSample = _fd->readUint16BE(); // depth
			entry->colorTableId = _fd->readUint16BE(); // colortable id

			// figure out the palette situation
			byte colorDepth = entry->bitsPerSample & 0x1F;
			bool colorGreyscale = (entry->bitsPerSample & 0x20) != 0;

			debug(0, "color depth: %d", colorDepth);

			// if the depth is 2, 4, or 8 bpp, file is palettized
			if (colorDepth == 2 || colorDepth == 4 || colorDepth == 8) {
				// Initialize the palette
				entry->palette = new byte[256 * 3];
				memset(entry->palette, 0, 256 * 3);

				if (colorGreyscale) {
					debug(0, "Greyscale palette");

					// compute the greyscale palette
					uint16 colorCount = 1 << colorDepth;
					int16 colorIndex = 255;
					byte colorDec = 256 / (colorCount - 1);
					for (byte j = 0; j < colorCount; j++) {
						entry->palette[j * 3] = entry->palette[j * 3 + 1] = entry->palette[j * 3 + 2] = colorIndex;
						colorIndex -= colorDec;
						if (colorIndex < 0)
							colorIndex = 0;
					}
				} else if (entry->colorTableId & 0x08) {
					// if flag bit 3 is set, use the default palette
					//uint16 colorCount = 1 << colorDepth;

					warning("Predefined palette! %dbpp", colorDepth);
				} else {
					debug(0, "Palette from file");

					// load the palette from the file
					uint32 colorStart = _fd->readUint32BE();
					/* uint16 colorCount = */ _fd->readUint16BE();
					uint16 colorEnd = _fd->readUint16BE();
					for (uint32 j = colorStart; j <= colorEnd; j++) {
						// each R, G, or B component is 16 bits;
						// only use the top 8 bits; skip alpha bytes
						// up front
						_fd->readByte();
						_fd->readByte();
						entry->palette[j * 3] = _fd->readByte();
						_fd->readByte();
						entry->palette[j * 3 + 1] = _fd->readByte();
						_fd->readByte();
						entry->palette[j * 3 + 2] = _fd->readByte();
						_fd->readByte();
					}
				}
			}
		} else if (st->codec_type == CODEC_TYPE_AUDIO) {
			debug(0, "Audio Codec FourCC: \'%s\'", tag2str(format));

			uint16 stsdVersion = _fd->readUint16BE();
			_fd->readUint16BE(); // revision level
			_fd->readUint32BE(); // vendor

			entry->channels = _fd->readUint16BE();			 // channel count
			entry->bitsPerSample = _fd->readUint16BE();	  // sample size

			_fd->readUint16BE(); // compression id = 0
			_fd->readUint16BE(); // packet size = 0

			entry->sampleRate = (_fd->readUint32BE() >> 16);

			debug(0, "stsd version =%d", stsdVersion);
			if (stsdVersion == 0) {
				// Not used, except in special cases. See below.
				entry->samplesPerFrame = entry->bytesPerFrame = 0;
			} else if (stsdVersion == 1) {
				// Read QT version 1 fields. In version 0 these dont exist.
				entry->samplesPerFrame = _fd->readUint32BE();
				debug(0, "stsd samples_per_frame =%d",entry->samplesPerFrame);
				_fd->readUint32BE(); // bytes per packet
				entry->bytesPerFrame = _fd->readUint32BE();
				debug(0, "stsd bytes_per_frame =%d", entry->bytesPerFrame);
				_fd->readUint32BE(); // bytes per sample
			} else {
				warning("Unsupported QuickTime STSD audio version %d", stsdVersion);
				return 1;
			}

			// Version 0 videos (such as the Riven ones) don't have this set,
			// but we need it later on. Add it in here.
			if (format == MKTAG('i','m','a','4')) {
				entry->samplesPerFrame = 64;
				entry->bytesPerFrame = 34 * entry->channels;
			}

			if (entry->sampleRate == 0 && st->time_scale > 1)
				entry->sampleRate = st->time_scale;
		} else {
			// other codec type, just skip (rtp, mp4s, tmcd ...)
			_fd->seek(size - (_fd->pos() - start_pos), SEEK_CUR);
		}

		// this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...)
		a.size = size - (_fd->pos() - start_pos);
		if (a.size > 8)
			readDefault(a);
		else if (a.size > 0)
			_fd->seek(a.size, SEEK_CUR);
	}

	return 0;
}

int QuickTimeDecoder::readSTSC(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->sample_to_chunk_sz = _fd->readUint32BE();

	debug(0, "track[%i].stsc.entries = %i", _numStreams - 1, st->sample_to_chunk_sz);

	st->sample_to_chunk = new MOVstsc[st->sample_to_chunk_sz];

	if (!st->sample_to_chunk)
		return -1;

	for (uint32 i = 0; i < st->sample_to_chunk_sz; i++) {
		st->sample_to_chunk[i].first = _fd->readUint32BE() - 1;
		st->sample_to_chunk[i].count = _fd->readUint32BE();
		st->sample_to_chunk[i].id = _fd->readUint32BE();
		//warning("Sample to Chunk[%d]: First = %d, Count = %d", i, st->sample_to_chunk[i].first, st->sample_to_chunk[i].count);
	}

	return 0;
}

int QuickTimeDecoder::readSTSS(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->keyframe_count = _fd->readUint32BE();

	debug(0, "keyframe_count = %d", st->keyframe_count);

	st->keyframes = new uint32[st->keyframe_count];

	if (!st->keyframes)
		return -1;

	for (uint32 i = 0; i < st->keyframe_count; i++) {
		st->keyframes[i] = _fd->readUint32BE() - 1; // Adjust here, the frames are based on 1
		debug(6, "keyframes[%d] = %d", i, st->keyframes[i]);

	}
	return 0;
}

int QuickTimeDecoder::readSTSZ(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->sample_size = _fd->readUint32BE();
	st->sample_count = _fd->readUint32BE();

	debug(5, "sample_size = %d sample_count = %d", st->sample_size, st->sample_count);

	if (st->sample_size)
		return 0; // there isn't any table following

	st->sample_sizes = new uint32[st->sample_count];

	if (!st->sample_sizes)
		return -1;

	for(uint32 i = 0; i < st->sample_count; i++) {
		st->sample_sizes[i] = _fd->readUint32BE();
		debug(6, "sample_sizes[%d] = %d", i, st->sample_sizes[i]);
	}

	return 0;
}

static uint32 ff_gcd(uint32 a, uint32 b) {
	return b ? ff_gcd(b, a % b) : a;
}

int QuickTimeDecoder::readSTTS(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];
	uint32 duration = 0;
	uint32 total_sample_count = 0;

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->stts_count = _fd->readUint32BE();
	st->stts_data = new MOVstts[st->stts_count];

	debug(0, "track[%i].stts.entries = %i", _numStreams - 1, st->stts_count);

	st->time_rate = 0;

	for (int32 i = 0; i < st->stts_count; i++) {
		int sample_duration;
		int sample_count;

		sample_count = _fd->readUint32BE();
		sample_duration = _fd->readUint32BE();
		st->stts_data[i].count = sample_count;
		st->stts_data[i].duration = sample_duration;

		st->time_rate = ff_gcd(st->time_rate, sample_duration);

		debug(0, "sample_count=%d, sample_duration=%d", sample_count, sample_duration);

		duration += sample_duration * sample_count;
		total_sample_count += sample_count;
	}

	st->nb_frames = total_sample_count;

	if (duration)
		st->duration = duration;

	return 0;
}

int QuickTimeDecoder::readSTCO(MOVatom atom) {
	MOVStreamContext *st = _streams[_numStreams - 1];

	_fd->readByte(); // version
	_fd->readByte(); _fd->readByte(); _fd->readByte(); // flags

	st->chunk_count = _fd->readUint32BE();
	st->chunk_offsets = new uint32[st->chunk_count];

	if (!st->chunk_offsets)
		return -1;

	for (uint32 i = 0; i < st->chunk_count; i++) {
		// WORKAROUND/HACK: The offsets in Riven videos (ones inside the Mohawk archives themselves)
		// have offsets relative to the archive and not the video. This is quite nasty. We subtract
		// the initial offset of the stream to get the correct value inside of the stream.
		st->chunk_offsets[i] = _fd->readUint32BE() - _beginOffset;
	}

	return 0;
}

int QuickTimeDecoder::readWAVE(MOVatom atom) {
	if (_numStreams < 1)
		return 0;

	MOVStreamContext *st = _streams[_numStreams - 1];

	if (atom.size > (1 << 30))
		return -1;

	if (st->stsdEntries[0].codecTag == MKTAG('Q','D','M','2')) // Read extradata for QDM2
		st->extradata = _fd->readStream(atom.size - 8);
	else if (atom.size > 8)
		return readDefault(atom);
	else
		_fd->skip(atom.size);

	return 0;
}

void QuickTimeDecoder::close() {
	stopAudio();

	for (uint32 i = 0; i < _numStreams; i++)
		delete _streams[i];

	delete _fd;
	_fd = 0;

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = 0;
	}

	// The audio stream is deleted automatically
	_audStream = NULL;

	SeekableVideoDecoder::reset();
}

Common::SeekableReadStream *QuickTimeDecoder::getNextFramePacket(uint32 &descId) {
	if (_videoStreamIndex < 0)
		return NULL;

	// First, we have to track down which chunk holds the sample and which sample in the chunk contains the frame we are looking for.
	int32 totalSampleCount = 0;
	int32 sampleInChunk = 0;
	int32 actualChunk = -1;

	for (uint32 i = 0; i < _streams[_videoStreamIndex]->chunk_count; i++) {
		int32 sampleToChunkIndex = -1;

		for (uint32 j = 0; j < _streams[_videoStreamIndex]->sample_to_chunk_sz; j++)
			if (i >= _streams[_videoStreamIndex]->sample_to_chunk[j].first)
				sampleToChunkIndex = j;

		if (sampleToChunkIndex < 0)
			error("This chunk (%d) is imaginary", sampleToChunkIndex);

		totalSampleCount += _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;

		if (totalSampleCount > getCurFrame()) {
			actualChunk = i;
			descId = _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].id;
			sampleInChunk = _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].count - totalSampleCount + getCurFrame();
			break;
		}
	}

	if (actualChunk < 0) {
		warning ("Could not find data for frame %d", getCurFrame());
		return NULL;
	}

	// Next seek to that frame
	_fd->seek(_streams[_videoStreamIndex]->chunk_offsets[actualChunk]);

	// Then, if the chunk holds more than one frame, seek to where the frame we want is located
	for (int32 i = getCurFrame() - sampleInChunk; i < getCurFrame(); i++) {
		if (_streams[_videoStreamIndex]->sample_size != 0)
			_fd->skip(_streams[_videoStreamIndex]->sample_size);
		else
			_fd->skip(_streams[_videoStreamIndex]->sample_sizes[i]);
	}

	// Finally, read in the raw data for the frame
	//printf ("Frame Data[%d]: Offset = %d, Size = %d\n", getCurFrame(), _fd->pos(), _streams[_videoStreamIndex]->sample_sizes[getCurFrame()]);

	if (_streams[_videoStreamIndex]->sample_size != 0)
		return _fd->readStream(_streams[_videoStreamIndex]->sample_size);

	return _fd->readStream(_streams[_videoStreamIndex]->sample_sizes[getCurFrame()]);
}

bool QuickTimeDecoder::checkAudioCodecSupport(uint32 tag) {
	// Check if the codec is a supported codec
	if (tag == MKTAG('t','w','o','s') || tag == MKTAG('r','a','w',' ') || tag == MKTAG('i','m','a','4'))
		return true;

#ifdef VIDEO_CODECS_QDM2_H
	if (tag == MKTAG('Q','D','M','2'))
		return true;
#endif

	warning("Audio Codec Not Supported: \'%s\'", tag2str(tag));

	return false;
}

Audio::AudioStream *QuickTimeDecoder::createAudioStream(Common::SeekableReadStream *stream) {
	if (!stream || _audioStreamIndex < 0)
		return NULL;

	STSDEntry *entry = &_streams[_audioStreamIndex]->stsdEntries[0];

	if (entry->codecTag == MKTAG('t','w','o','s') || entry->codecTag == MKTAG('r','a','w',' ')) {
		// Fortunately, most of the audio used in Myst videos is raw...
		uint16 flags = 0;
		if (entry->codecTag == MKTAG('r','a','w',' '))
			flags |= Audio::FLAG_UNSIGNED;
		if (entry->channels == 2)
			flags |= Audio::FLAG_STEREO;
		if (entry->bitsPerSample == 16)
			flags |= Audio::FLAG_16BITS;
		uint32 dataSize = stream->size();
		byte *data = (byte *)malloc(dataSize);
		stream->read(data, dataSize);
		delete stream;
		return Audio::makeRawStream(data, dataSize, entry->sampleRate, flags);
	} else if (entry->codecTag == MKTAG('i','m','a','4')) {
		// Riven uses this codec (as do some Myst ME videos)
		return Audio::makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), Audio::kADPCMApple, entry->sampleRate, entry->channels, 34);
#ifdef VIDEO_CODECS_QDM2_H
	} else if (entry->codecTag == MKTAG('Q','D','M','2')) {
		// Several Myst ME videos use this codec
		return makeQDM2Stream(stream, _streams[_audioStreamIndex]->extradata);
#endif
	}

	error("Unsupported audio codec");

	return NULL;
}

uint32 QuickTimeDecoder::getAudioChunkSampleCount(uint chunk) {
	if (_audioStreamIndex < 0)
		return 0;

	uint32 sampleCount = 0;

	for (uint32 j = 0; j < _streams[_audioStreamIndex]->sample_to_chunk_sz; j++)
		if (chunk >= _streams[_audioStreamIndex]->sample_to_chunk[j].first)
			sampleCount = _streams[_audioStreamIndex]->sample_to_chunk[j].count;

	return sampleCount;
}

void QuickTimeDecoder::readNextAudioChunk() {
	STSDEntry *entry = &_streams[_audioStreamIndex]->stsdEntries[0];
	Common::MemoryWriteStreamDynamic *wStream = new Common::MemoryWriteStreamDynamic();

	_fd->seek(_streams[_audioStreamIndex]->chunk_offsets[_curAudioChunk]);

	// First, we have to get the sample count
	uint32 sampleCount = getAudioChunkSampleCount(_curAudioChunk);
	assert(sampleCount);

	// Then calculate the right sizes
	while (sampleCount > 0) {
		uint32 samples = 0, size = 0;

		if (entry->samplesPerFrame >= 160) {
			samples = entry->samplesPerFrame;
			size = entry->bytesPerFrame;
		} else if (entry->samplesPerFrame > 1) {
			samples = MIN<uint32>((1024 / entry->samplesPerFrame) * entry->samplesPerFrame, sampleCount);
			size = (samples / entry->samplesPerFrame) * entry->bytesPerFrame;
		} else {
			samples = MIN<uint32>(1024, sampleCount);
			size = samples * _streams[_audioStreamIndex]->sample_size;
		}

		// Now, we read in the data for this data and output it
		byte *data = (byte *)malloc(size);
		_fd->read(data, size);
		wStream->write(data, size);
		free(data);
		sampleCount -= samples;
	}

	// Now queue the buffer
	_audStream->queueAudioStream(createAudioStream(new Common::MemoryReadStream(wStream->getData(), wStream->size(), DisposeAfterUse::YES)));
	delete wStream;

	_curAudioChunk++;
}

void QuickTimeDecoder::updateAudioBuffer() {
	if (!_audStream)
		return;

	uint32 numberOfChunksNeeded = 0;

	if (_curFrame == (int32)_streams[_videoStreamIndex]->nb_frames - 1) {
		// If we're on the last frame, make sure all audio remaining is buffered
		numberOfChunksNeeded = _streams[_audioStreamIndex]->chunk_count;
	} else {
		STSDEntry *entry = &_streams[_audioStreamIndex]->stsdEntries[0];

		// Calculate the amount of chunks we need in memory until the next frame
		uint32 timeToNextFrame = getTimeToNextFrame();
		uint32 timeFilled = 0;
		uint32 curAudioChunk = _curAudioChunk - _audStream->numQueuedStreams();

		for (; timeFilled < timeToNextFrame && curAudioChunk < _streams[_audioStreamIndex]->chunk_count; numberOfChunksNeeded++, curAudioChunk++) {
			uint32 sampleCount = getAudioChunkSampleCount(curAudioChunk);
			assert(sampleCount);

			timeFilled += sampleCount * 1000 / entry->sampleRate;
		}

		// Add a couple extra to ensure we don't underrun
		numberOfChunksNeeded += 3;
	}

	// Keep three streams in buffer so that if/when the first two end, it goes right into the next
	while (_audStream->numQueuedStreams() < numberOfChunksNeeded && _curAudioChunk < _streams[_audioStreamIndex]->chunk_count)
		readNextAudioChunk();
}

QuickTimeDecoder::STSDEntry::STSDEntry() {
	codecTag = 0;
	bitsPerSample = 0;
	memset(codecName, 0, 32);
	colorTableId = 0;
	palette = 0;
	videoCodec = 0;
	channels = 0;
	sampleRate = 0;
	samplesPerFrame = 0;
	bytesPerFrame = 0;
}

QuickTimeDecoder::STSDEntry::~STSDEntry() {
	delete[] palette;
	delete videoCodec;
}

QuickTimeDecoder::MOVStreamContext::MOVStreamContext() {
	chunk_count = 0;
	chunk_offsets = 0;
	stts_count = 0;
	stts_data = 0;
	sample_to_chunk_sz = 0;
	sample_to_chunk = 0;
	sample_size = 0;
	sample_count = 0;
	sample_sizes = 0;
	keyframe_count = 0;
	keyframes = 0;
	time_scale = 0;
	time_rate = 0;
	width = 0;
	height = 0;
	codec_type = CODEC_TYPE_MOV_OTHER;
	stsdEntryCount = 0;
	stsdEntries = 0;
	editCount = 0;
	editList = 0;
	extradata = 0;
	nb_frames = 0;
	duration = 0;
	start_time = 0;
}

QuickTimeDecoder::MOVStreamContext::~MOVStreamContext() {
	delete[] chunk_offsets;
	delete[] stts_data;
	delete[] sample_to_chunk;
	delete[] sample_sizes;
	delete[] keyframes;
	delete[] stsdEntries;
	delete[] editList;
	delete extradata;
}

} // End of namespace Video
