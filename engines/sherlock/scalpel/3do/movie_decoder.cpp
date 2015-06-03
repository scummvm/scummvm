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

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "sherlock/scalpel/3do/movie_decoder.h"
#include "image/codecs/cinepak.h"

// for Test-Code
#include "common/system.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "engines/engine.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Sherlock {

Scalpel3DOMovieDecoder::Scalpel3DOMovieDecoder()
	: _stream(0), _videoTrack(0), _audioTrack(0) {
}

Scalpel3DOMovieDecoder::~Scalpel3DOMovieDecoder() {
	close();
}

bool Scalpel3DOMovieDecoder::loadStream(Common::SeekableReadStream *stream) {
	bool   videoHeaderFound = false;
	uint32 videoSubType    = 0;
	uint32 videoTimeStamp  = 0;
	uint32 videoCodecTag   = 0;
	uint32 videoHeight     = 0;
	uint32 videoWidth      = 0;
	uint32 videoFrameRate  = 0;
	uint32 videoFrameCount = 0;
	uint32 videoFrameNr    = 0;
	uint32 audioSubType    = 0;
	uint32 audioCodecTag   = 0;
	uint32 audioChannels   = 0;
	uint32 audioSampleRate = 0;

	close();

	_stream = stream;

	// CTRL Header
	if (_stream->readUint32BE() != MKTAG('C', 'T', 'R', 'L')) {
		close();
		return false;
	}

	uint32 ctrlSize = _stream->readUint32BE() - 8;
	_stream->skip(ctrlSize);

	// Look for packets that we care about
	static const int maxPacketCheckCount = 10;
	for (int i = 0; i < maxPacketCheckCount; i++) {
		uint32 tag = _stream->readUint32BE();
		uint32 chunkSize = _stream->readUint32BE() - 8;

		// Bail out if done
		if (_stream->eos())
			break;

		uint32 dataStartOffset = _stream->pos();

		switch (tag) {
		case MKTAG('F','I','L','M'): {
			// See if this is a FILM header
			videoTimeStamp = _stream->readUint32BE();
			_stream->skip(4); // Unknown 0x00000000
			videoSubType = _stream->readUint32BE();

			switch (videoSubType) {
			case MKTAG('F', 'H', 'D', 'R'):
				// FILM header found
				if (videoHeaderFound) {
					warning("Sherlock 3DO movie: Multiple FILM headers found");
					close();
					return false;
				}
				_stream->readUint32BE();
				videoCodecTag = _stream->readUint32BE();
				videoHeight = _stream->readUint32BE();
				videoWidth = _stream->readUint32BE();
				_stream->skip(4); // time scale
				videoFrameCount = _stream->readUint32BE();
				videoHeaderFound = true;
				break;

			case MKTAG('F', 'R', 'M', 'E'):
				if (!_videoTrack) {
					if (!videoFrameNr) {
						// first frame, we expect timestamp to be 0
						assert(videoTimeStamp == 0);

					} else {
						// second frame found, we expect timestamp to be larger than 0
						assert(videoTimeStamp);
						// Framerate shouldn't be known at the moment
						assert(videoFrameRate == 0);

						if (!videoHeaderFound) {
							error("Sherlock 3DO movie: no FILM header found before FILM frame");
						}

						// 3DO clock time for movies runs at 240Hz
						videoFrameRate = 240 / videoTimeStamp;

						_videoTrack = new StreamVideoTrack(videoWidth, videoHeight, videoCodecTag, videoFrameCount, videoFrameRate);
						addTrack(_videoTrack);
					}
					videoFrameNr++;
				}
				break;

			default:
				error("Sherlock 3DO movie: Unknown subtype inside FILM packet");
				break;
			}
			break;
		}

		case MKTAG('S','N','D','S'): {
			_stream->skip(8);
			audioSubType = _stream->readUint32BE();

			switch (audioSubType) {
			case MKTAG('S', 'H', 'D', 'R'):
				// Audio header

				// Bail if we already have a track
				if (_audioTrack) {
					warning("Sherlock 3DO movie: Multiple SNDS headers found");
					close();
					return false;
				}

				// OK, this is the start of a audio stream
				_stream->readUint32BE(); // Version, always 0x00000000
				_stream->readUint32BE(); // Unknown 0x00000008 ?!
				_stream->readUint32BE(); // Unknown 0x00007500
				_stream->readUint32BE(); // Unknown 0x00004000
				_stream->readUint32BE(); // Unknown 0x00000000
				_stream->readUint32BE(); // Unknown 0x00000010
				audioSampleRate = _stream->readUint32BE();			
				audioChannels = _stream->readUint32BE();
				audioCodecTag = _stream->readUint32BE();
				_stream->readUint32BE(); // Unknown 0x00000004 compression ratio?
				_stream->readUint32BE(); // Unknown 0x00000A2C

				_audioTrack = new StreamAudioTrack(audioCodecTag, audioSampleRate, audioChannels);
				addTrack(_audioTrack);
				break;

			case MKTAG('S', 'S', 'M', 'P'):
				// Audio data
				break;
			default:
				error("Sherlock 3DO movie: Unknown subtype inside FILM packet");
				break;
			}
			break;
		}

		case MKTAG('C','T','R','L'):
		case MKTAG('F','I','L','L'):
			// Ignore but also accept CTRL + FILL packets
			break;

		default:
			warning("Unknown header inside Sherlock 3DO movie");
			close();
			return false;
		}

		if ((_videoTrack) && (_audioTrack))
			break;

		// Seek to next chunk
		_stream->seek(dataStartOffset + chunkSize);
	}

	// Bail if we didn't find video + audio
	if ((!_videoTrack) || (!_audioTrack)) {
		close();
		return false;
	}

	// Rewind back to the beginning
	_stream->seek(0);

	return true;
}

void Scalpel3DOMovieDecoder::close() {
	Video::VideoDecoder::close();

	delete _stream; _stream = 0;
	_videoTrack = 0;
}

void Scalpel3DOMovieDecoder::readNextPacket() {
	uint32 videoSubType = 0;
	uint32 videoFrameSize = 0;
	uint32 audioSubType = 0;
	uint32 audioSampleBytes = 0;
	bool gotAudio = false;
	bool gotVideo = false;

	while (!endOfVideoTracks()) {
		uint32 tag = _stream->readUint32BE();
		uint32 chunkSize = _stream->readUint32BE() - 8;
		uint32 dataStartOffset = _stream->pos();

		//warning("offset %lx - tag %lx", dataStartOffset, tag);

		if (_stream->eos())
			break;

		switch (tag) {
		case MKTAG('F','I','L','M'):
			_stream->readUint32BE(); // looks like frame * 16?
			_stream->skip(4); // Unknown
			videoSubType = _stream->readUint32BE();

			switch (videoSubType) {
			case MKTAG('F', 'H', 'D', 'R'):
				// Ignore video header
				break;

			case MKTAG('F', 'R', 'M', 'E'):
				// Have a frame!

				// If we previously found one, this is just to get the time offset of the next one
				/* uint32 frmeSize = */ _stream->readUint32BE();
				videoFrameSize = _stream->readUint32BE();
				_videoTrack->decodeFrame(_stream->readStream(videoFrameSize));
				gotVideo = true;
				break;

			default:
				error("Sherlock 3DO movie: Unknown subtype inside FILM packet");
				break;
			}
			break;

		case MKTAG('S','N','D','S'):
			_stream->skip(8);
			audioSubType = _stream->readUint32BE();

			switch (audioSubType) {
			case MKTAG('S', 'H', 'D', 'R'):
				// Ignore the audio header
				break;

			case MKTAG('S', 'S', 'M', 'P'):
				// Got audio chunk
				audioSampleBytes = _stream->readUint32BE();
				_audioTrack->queueAudio(_stream, audioSampleBytes);
				gotAudio = true;
				break;

			default:
				error("Sherlock 3DO movie: Unknown subtype inside SNDS packet");
				break;
			}
			break;

		case MKTAG('C','T','R','L'):
		case MKTAG('F','I','L','L'):
			// Ignore but also accept CTRL + FILL packets
			break;

		default:
			error("Unknown header inside Sherlock 3DO movie");
		}

		// Always seek to end of chunk
		// Sometimes not all of the chunk is filled with audio
		_stream->seek(dataStartOffset + chunkSize);

		if (gotVideo)
			return;
	}
}

Scalpel3DOMovieDecoder::StreamVideoTrack::StreamVideoTrack(uint32 width, uint32 height, uint32 codecTag, uint32 frameCount, uint32 frameRate) {
	_width = width;
	_height = height;
	_frameCount = frameCount;
	_frameRate = frameRate;
	_curFrame = -1;

	// Create the Cinepak decoder, if we're using it
	if (codecTag == MKTAG('c', 'v', 'i', 'd'))
		_codec = new Image::CinepakDecoder();
	else
		error("Unknown Sherlock 3DO stream codec tag '%s'", tag2str(codecTag));
}

Scalpel3DOMovieDecoder::StreamVideoTrack::~StreamVideoTrack() {
	delete _codec;
}

Graphics::PixelFormat Scalpel3DOMovieDecoder::StreamVideoTrack::getPixelFormat() const {
	return _codec->getPixelFormat();
}

void Scalpel3DOMovieDecoder::StreamVideoTrack::decodeFrame(Common::SeekableReadStream *stream) {
	_surface = _codec->decodeFrame(*stream);
	_curFrame++;
}

#define STREAMAUDIO_STEPSIZETABLE_MAX 88

static int16 streamAudio_stepSizeTable[STREAMAUDIO_STEPSIZETABLE_MAX + 1] = {
        7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
	   19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	   50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
	  130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	  337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	  876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	 2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	 5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int16 streamAudio_stepSizeIndex[] = {
	-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 
};

Scalpel3DOMovieDecoder::StreamAudioTrack::StreamAudioTrack(uint32 codecTag, uint32 sampleRate, uint32 channels) {
	if (channels != 1) {
		error("Sherlock 3DO stream audio is not mono");
	}
	if (codecTag != MKTAG('A','D','P','4')) {
		error("Sherlock 3DO stream audio is not using codec ADP4");
	}
	_audioStream = Audio::makeQueuingAudioStream(sampleRate, false);

	// reset ADPCM decoding
	_lastSample = 0;
	_stepIndex = 0;
}

Scalpel3DOMovieDecoder::StreamAudioTrack::~StreamAudioTrack() {
	delete _audioStream;
}

int16 Scalpel3DOMovieDecoder::StreamAudioTrack::decodeSample(uint8 dataNibble) {
	int16 currentStep = streamAudio_stepSizeTable[_stepIndex];
	int32 decodedSample = _lastSample;
	int16 delta = currentStep >> 3;

	if (dataNibble & 1)
		delta += currentStep >> 2;

	if (dataNibble & 2)
		delta += currentStep >> 1;

	if (dataNibble & 4)
		delta += currentStep;

	if (dataNibble & 8) {
		decodedSample -= delta;
	} else {
		decodedSample += delta;
	}

	_lastSample = CLIP<int32>(decodedSample, -32768, 32767);

	_stepIndex += streamAudio_stepSizeIndex[dataNibble & 0x07];
	_stepIndex = CLIP<int16>(_stepIndex, 0, STREAMAUDIO_STEPSIZETABLE_MAX);

   return _lastSample;
}

void Scalpel3DOMovieDecoder::StreamAudioTrack::queueAudio(Common::SeekableReadStream *stream, uint32 length) {
	uint32 decodedAudioSize = length * 4;
	byte  *decodedAudioBuffer = NULL;
	uint8  streamByte = 0;
	uint32 streamPos = 0;
	uint32 audioPos = 0;

	decodedAudioBuffer = (byte *)malloc(decodedAudioSize);
	assert(decodedAudioBuffer);

	for (streamPos = 0; streamPos < length; streamPos++) {
		streamByte = stream->readByte();
		WRITE_BE_UINT16(decodedAudioBuffer + audioPos, decodeSample(streamByte >> 4));
		audioPos += 2;
		WRITE_BE_UINT16(decodedAudioBuffer + audioPos, decodeSample(streamByte & 0x0F));
		audioPos += 2;
	}

	// Now the audio is loaded, so let's queue it
	_audioStream->queueBuffer(decodedAudioBuffer, decodedAudioSize, DisposeAfterUse::YES, Audio::FLAG_16BITS);
}

Audio::AudioStream *Scalpel3DOMovieDecoder::StreamAudioTrack::getAudioStream() const {
	return _audioStream;
}

// Test-code

// Code for showing a movie. Only meant for testing/debug purposes
void Scalpel3DOMoviePlay(const char *filename) {
	// HACK
	initGraphics(320, 200, false, NULL);

	Scalpel3DOMovieDecoder *videoDecoder = new Scalpel3DOMovieDecoder();

	if (!videoDecoder->loadFile(filename)) {
		// HACK
		initGraphics(320, 200, false);
		return;
	}

	bool skipVideo = false;
	//byte bytesPerPixel = videoDecoder->getPixelFormat().bytesPerPixel;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	//uint16 pitch = videoDecoder->getWidth() * bytesPerPixel;

	videoDecoder->start();

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && (!skipVideo)) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, width, height);

				if (videoDecoder->hasDirtyPalette()) {
					const byte *palette = videoDecoder->getPalette();
					g_system->getPaletteManager()->setPalette(palette, 0, 255);
				}

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE))
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	// HACK: switch back to 8bpp
	initGraphics(320, 200, false);
}


} // End of namespace Sherlock
