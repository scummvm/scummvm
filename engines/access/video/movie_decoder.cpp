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

#include "access/access.h"
#include "access/video/movie_decoder.h"

// for Test-Code
#include "common/system.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "engines/engine.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Access {

AccessVIDMovieDecoder::AccessVIDMovieDecoder()
	: _stream(0), _videoTrack(0), _audioTrack(0) {
	_streamSeekOffset = 0;
	_streamVideoIndex = 0;
	_streamAudioIndex = 0;
}

AccessVIDMovieDecoder::~AccessVIDMovieDecoder() {
	close();
}

bool AccessVIDMovieDecoder::loadStream(Common::SeekableReadStream *stream) {
	uint32 videoCodecTag   = 0;
	uint32 videoHeight     = 0;
	uint32 videoWidth      = 0;
	uint16 regularDelay    = 0;
	uint32 audioSampleRate = 0;

	close();

	_stream = stream;
	_streamSeekOffset = 15; // offset of first chunk
	_streamVideoIndex = 0;
	_streamAudioIndex = 0;

	// read header
	//  ID [dword] "VID"
	//  ?? [byte]
	//  ?? [word]
	//  width [word]
	//  height [word]
	//  regular delay between frames (60 per second) [word]
	//  ?? [word]

	videoCodecTag = _stream->readUint32BE();
	if (videoCodecTag != MKTAG('V','I','D',0x00)) {
		warning("AccessVIDMoviePlay: bad codec tag, not a video file?");
		close();
		return false;
	}
	_stream->skip(3);
	videoWidth = _stream->readUint16LE();
	videoHeight = _stream->readUint16LE();
	regularDelay = _stream->readUint16LE();
	_stream->skip(2);

	if (!regularDelay) {
		warning("AccessVIDMoviePlay: delay between frames is zero?");
		close();
		return false;
	}

	// create video track
	_videoTrack = new StreamVideoTrack(videoWidth, videoHeight, regularDelay);
	addTrack(_videoTrack);

	//warning("width %d, height %d", videoWidth, videoHeight);

	// Look through the first few packets
	static const int maxPacketCheckCount = 10;

	for (int i = 0; i < maxPacketCheckCount; i++) {
		byte chunkId = _stream->readByte();

		// Bail out if done
		if (_stream->eos())
			break;

		// Bail also in case end of file chunk was found
		if (chunkId == kVIDMovieChunkId_EndOfFile)
			break;

		uint32 chunkStartOffset = _stream->pos();
		//warning("data chunk at %x", chunkStartOffset);

		switch (chunkId) {
		case kVIDMovieChunkId_FullFrame:
		case kVIDMovieChunkId_FullFrameCompressed:
		case kVIDMovieChunkId_PartialFrameCompressed:
		case kVIDMovieChunkId_FullFrameCompressedFill: {
			if (!_videoTrack->skipOverFrame(_stream, chunkId)) {
				close();
				return false;
			}
			break;
		}

		case kVIDMovieChunkId_Palette: {
			if (!_videoTrack->skipOverPalette(_stream)) {
				close();
				return false;
			}
			break;
		}

		case kVIDMovieChunkId_AudioFirstChunk:
		case kVIDMovieChunkId_Audio: {
			// sync [word]
			// sampling rate [byte]
			// size of audio data [word]
			// sample data [] (mono, 8-bit, unsigned)
			//
			// Only first chunk has sync + sampling rate
			if (chunkId == kVIDMovieChunkId_AudioFirstChunk) {
				byte soundblasterRate;

				_stream->skip(2); // skip over sync
				soundblasterRate = _stream->readByte();
				audioSampleRate = 1000000 / (256 - soundblasterRate);

				_audioTrack = new StreamAudioTrack(audioSampleRate, getSoundType());
				addTrack(_audioTrack);

				_stream->seek(chunkStartOffset); // seek back
			}

			if (!_audioTrack) {
				warning("AccessVIDMoviePlay: regular audio chunk, before audio chunk w/ header");
				close();
				return false;
			}
			if (!_audioTrack->skipOverAudio(_stream, chunkId)) {
				close();
				return false;
			}
			break;
		}

		default:
			warning("AccessVIDMoviePlay: Unknown chunk-id '%x' inside VID movie", chunkId);
			close();
			return false;
		}

		// Remember this chunk inside our cache
		IndexCacheEntry indexCacheEntry;

		indexCacheEntry.chunkId = chunkId;
		indexCacheEntry.offset  = chunkStartOffset;

		_indexCacheTable.push_back(indexCacheEntry);

		// Got an audio chunk now? -> exit b/c we are done
		if (audioSampleRate)
			break;
	}

	// Remember offset of latest not-indexed-yet chunk
	_streamSeekOffset = _stream->pos();

	// If sample rate was found, create an audio track
	if (audioSampleRate) {
		_audioTrack = new StreamAudioTrack(audioSampleRate, getSoundType());
		addTrack(_audioTrack);
	}

	// Rewind back to the beginning right to the first chunk
	_stream->seek(15);

	return true;
}

void AccessVIDMovieDecoder::close() {
	Video::VideoDecoder::close();

	delete _stream; _stream = 0;
	_videoTrack = 0;

	_indexCacheTable.clear();
}

// We try to at least decode 1 frame
// and also try to get at least 0.5 seconds of audio queued up
void AccessVIDMovieDecoder::readNextPacket() {
	uint32 currentMovieTime = getTime();
	uint32 wantedAudioQueued  = currentMovieTime + 500; // always try to be 0.500 seconds in front of movie time

	uint32 streamIndex = 0;
	IndexCacheEntry indexEntry;
	bool currentlySeeking = false;

	bool videoDone     = false;
	bool audioDone     = false;

	// Seek to smallest stream offset
	if ((_streamVideoIndex <= _streamAudioIndex) || (!_audioTrack)) {
		streamIndex = _streamVideoIndex;
	} else {
		streamIndex = _streamAudioIndex;
	}

	if (_audioTrack) {
		if (wantedAudioQueued <= _audioTrack->getTotalAudioQueued()) {
			// already got enough audio queued up
			audioDone = true;
		}
	} else {
		// no audio track, audio is always done
		audioDone = true;
	}

	while (1) {
		// Check, if stream-index is already cached
		if (streamIndex < _indexCacheTable.size()) {
			indexEntry.chunkId = _indexCacheTable[streamIndex].chunkId;
			indexEntry.offset  = _indexCacheTable[streamIndex].offset;
			currentlySeeking = false;

		} else {
			// read from file
			_stream->seek(_streamSeekOffset);
			indexEntry.chunkId = _stream->readByte();
			indexEntry.offset  = _stream->pos();
			currentlySeeking = true;

			// and store that as well
			_indexCacheTable.push_back(indexEntry);
		}

		// end of stream -> exit
		if (_stream->eos())
			break;

		// end of file chunk -> exit
		if (indexEntry.chunkId == kVIDMovieChunkId_EndOfFile)
			break;

//		warning("chunk %x", indexEntry.chunkId);

		switch (indexEntry.chunkId) {
		case kVIDMovieChunkId_FullFrame:
		case kVIDMovieChunkId_FullFrameCompressed:
		case kVIDMovieChunkId_PartialFrameCompressed:
		case kVIDMovieChunkId_FullFrameCompressedFill: {
			if ((_streamVideoIndex <= streamIndex) && (!videoDone)) {
				// We are at an index, that is still relevant for video decoding
				// and we are not done with video yet
				if (!currentlySeeking) {
					// seek to stream position in case we used the cache
					_stream->seek(indexEntry.offset);
				}
				//warning("video decode chunk %x at %lx", indexEntry.chunkId, _stream->pos());
				_videoTrack->decodeFrame(_stream, indexEntry.chunkId);
				videoDone = true;
				_streamVideoIndex = streamIndex + 1;
			} else {
				if (currentlySeeking) {
					// currently seeking, so we have to skip the frame bytes manually
					_videoTrack->skipOverFrame(_stream, indexEntry.chunkId);
				}
			}
			break;
		}

		case kVIDMovieChunkId_Palette: {
			if ((_streamVideoIndex <= streamIndex) && (!videoDone)) {
				// We are at an index, that is still relevant for video decoding
				// and we are not done with video yet
				if (!currentlySeeking) {
					// seek to stream position in case we used the cache
					_stream->seek(indexEntry.offset);
				}
				_videoTrack->decodePalette(_stream);
				_streamVideoIndex = streamIndex + 1;
			} else {
				if (currentlySeeking) {
					// currently seeking, so we have to skip the frame bytes manually
					_videoTrack->skipOverPalette(_stream);
				}
			}
			break;
		}

		case kVIDMovieChunkId_AudioFirstChunk:
		case kVIDMovieChunkId_Audio: {
			if ((_streamAudioIndex <= streamIndex) && (!audioDone)) {
				// We are at an index that is still relevant for audio decoding
				if (!currentlySeeking) {
					// seek to stream position in case we used the cache
					_stream->seek(indexEntry.offset);
				}
				_audioTrack->queueAudio(_stream, indexEntry.chunkId);
				_streamAudioIndex = streamIndex + 1;

				if (wantedAudioQueued <= _audioTrack->getTotalAudioQueued()) {
					// Got enough audio
					audioDone = true;
				}
			} else {
				if (!_audioTrack) {
					error("AccessVIDMoviePlay: audio chunks found without audio track active");
				}
				if (currentlySeeking) {
					// currently seeking, so we have to skip the audio bytes manually
					_audioTrack->skipOverAudio(_stream, indexEntry.chunkId);
				}
			}
			break;
		}

		default:
			error("AccessVIDMoviePlay: Unknown chunk-id '%x' inside VID movie", indexEntry.chunkId);
		}

		if (currentlySeeking) {
			// remember currently stream offset in case we are seeking
			_streamSeekOffset = _stream->pos();
		}

		// go to next index
		streamIndex++;

		if ((videoDone) && (audioDone)) {
			return;
		}
	}

	if (!videoDone) {
		// no more video frames? set end of video track
		_videoTrack->setEndOfTrack();
	}
}

AccessVIDMovieDecoder::StreamVideoTrack::StreamVideoTrack(uint32 width, uint32 height, uint16 regularFrameDelay) {
	_width = width;
	_height = height;
	_regularFrameDelay = regularFrameDelay;
	_curFrame = -1;
	_nextFrameStartTime = 0;
	_endOfTrack = false;
	_dirtyPalette = false;

	memset(&_palette, 0, sizeof(_palette));

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());
}

AccessVIDMovieDecoder::StreamVideoTrack::~StreamVideoTrack() {
	delete _surface;
}

bool AccessVIDMovieDecoder::StreamVideoTrack::endOfTrack() const {
	return _endOfTrack;
}

Graphics::PixelFormat AccessVIDMovieDecoder::StreamVideoTrack::getPixelFormat() const {
	return _surface->format;
}

void AccessVIDMovieDecoder::StreamVideoTrack::decodeFrame(Common::SeekableReadStream *stream, byte chunkId) {
	byte *framePixelsPtr = (byte *)_surface->getPixels();
	byte *pixelsPtr = framePixelsPtr;
	byte rleByte = 0;
	uint16 additionalDelay = 0;
	int32 expectedPixels = 0;

	switch (chunkId) {
	case kVIDMovieChunkId_FullFrame: {
		// Full frame is:
		//  data [width * height]
		additionalDelay = stream->readUint16LE();
		stream->read(framePixelsPtr, _width * _height);
		break;
	}

	case kVIDMovieChunkId_FullFrameCompressed:
	case kVIDMovieChunkId_PartialFrameCompressed: {
		// Skip manually over compressed data
		// Full frame compressed is:
		//  additional delay [word]
		//  REPEAT:
		//   RLE [byte]
		//   RLE upper bit set: skip over RLE & 0x7F pixels
		//   RLE upper bit not set: draw RLE amount of pixels (those pixels follow right after RLE byte)
		//
		// Partial frame compressed is:
		//  sync [word]
		//  horizontal start position [word]
		//  REPEAT:
		//   see full frame compressed
		uint16 horizontalStartPosition = 0;

		additionalDelay = stream->readUint16LE();

		if (chunkId == kVIDMovieChunkId_PartialFrameCompressed) {
			horizontalStartPosition = stream->readUint16LE();
			if (horizontalStartPosition >= _height) {
				error("AccessVIDMoviePlay: starting position larger than height during partial frame compressed, data corrupt?");
				return;
			}
		}

		expectedPixels = _width * (_height - horizontalStartPosition);

		// adjust frame destination pointer
		pixelsPtr += (horizontalStartPosition * _width);

		while (expectedPixels >= 0) {
			rleByte = stream->readByte();
			if (!rleByte) // NUL means end of stream
				break;

			if (rleByte & 0x80) {
				rleByte = rleByte & 0x7F;
				expectedPixels -= rleByte;
			} else {
				// skip over pixels
				expectedPixels -= rleByte;
				stream->read(pixelsPtr, rleByte); // read pixel data into frame
			}
			pixelsPtr += rleByte;
		}
		// expectedPixels may be positive here in case stream got terminated with a NUL
		if (expectedPixels < 0) {
			error("AccessVIDMoviePlay: pixel count mismatch during full/partial frame compressed, data corrupt?");
		}
		break;
	}

	case kVIDMovieChunkId_FullFrameCompressedFill: {
		// Full frame compressed fill is:
		//  additional delay [word]
		//  REPEAT:
		//   RLE [byte]
		//   RLE upper bit set: draw RLE amount (& 0x7F) of pixels with specified color (color byte follows after RLE byte)
		//   RLE upper bit not set: draw RLE amount of pixels (those pixels follow right after RLE byte)
		additionalDelay = stream->readUint16LE();
		expectedPixels = _width * _height;

		while (expectedPixels > 0) {
			rleByte = stream->readByte();

			if (rleByte & 0x80) {
				rleByte = rleByte & 0x7F;
				expectedPixels -= rleByte;

				byte fillColor = stream->readByte();
				memset(pixelsPtr, fillColor, rleByte);
			} else {
				// skip over pixels
				expectedPixels -= rleByte;
				stream->read(pixelsPtr, rleByte); // read pixel data into frame
			}
			pixelsPtr += rleByte;
		}
		if (expectedPixels < 0) {
			error("AccessVIDMoviePlay: pixel count mismatch during full frame compressed fill, data corrupt?");
		}
		break;
	}
	default:
		assert(0);
		break;
	}

	_curFrame++;

	// TODO: not sure, if additionalDelay is supposed to affect the follow-up frame or the current frame
	// the videos, that I found, don't have it set
	uint32 currentFrameStartTime = getNextFrameStartTime();
	uint32 nextFrameStartTime = (_regularFrameDelay * _curFrame) * 1000 / 60;
	if (additionalDelay) {
		nextFrameStartTime += additionalDelay * 1000 / 60;
	}
	assert(currentFrameStartTime <= nextFrameStartTime);
	setNextFrameStartTime(nextFrameStartTime);
}

bool AccessVIDMovieDecoder::StreamVideoTrack::skipOverFrame(Common::SeekableReadStream *stream, byte chunkId) {
	byte rleByte = 0;
	int32 expectedPixels = 0;

	switch (chunkId) {
	case kVIDMovieChunkId_FullFrame: {
		// Full frame is:
		//  additional delay [word]
		//  data [width * height]
		stream->skip(2);
		stream->skip(_width * _height);
		break;
	}

	case kVIDMovieChunkId_FullFrameCompressed:
	case kVIDMovieChunkId_PartialFrameCompressed: {
		// Skip manually over compressed data
		// Full frame compressed is:
		//  additional delay [word]
		//  REPEAT:
		//   RLE [byte]
		//   RLE upper bit set: skip over RLE & 0x7F pixels
		//   RLE upper bit not set: draw RLE amount of pixels (those pixels follow right after RLE byte)
		//
		// Partial frame compressed is:
		//  sync [word]
		//  horizontal start position [word]
		//  REPEAT:
		//   see full frame compressed
		uint16 horizontalStartPosition = 0;

		stream->skip(2);

		if (chunkId == kVIDMovieChunkId_PartialFrameCompressed) {
			horizontalStartPosition = stream->readUint16LE();
			if (horizontalStartPosition >= _height) {
				warning("AccessVIDMoviePlay: starting position larger than height during partial frame compressed, data corrupt?");
				return false;
			}
		}

		expectedPixels = _width * (_height - horizontalStartPosition);

		while (expectedPixels >= 0) {
			rleByte = stream->readByte();
			if (!rleByte) // NUL means end of stream
				break;

			if (rleByte & 0x80) {
				expectedPixels -= rleByte & 0x7F;
			} else {
				// skip over pixels
				expectedPixels -= rleByte;
				stream->skip(rleByte); // skip over pixel data
			}
		}
		// expectedPixels may be positive here in case stream got terminated with a NUL
		if (expectedPixels < 0) {
			warning("AccessVIDMoviePlay: pixel count mismatch during full/partial frame compressed, data corrupt?");
			return false;
		}
		break;
	}

	case kVIDMovieChunkId_FullFrameCompressedFill: {
		// Full frame compressed fill is:
		//  additional delay [word]
		//  REPEAT:
		//   RLE [byte]
		//   RLE upper bit set: draw RLE amount (& 0x7F) of pixels with specified color (color byte follows after RLE byte)
		//   RLE upper bit not set: draw RLE amount of pixels (those pixels follow right after RLE byte)
		stream->skip(2);
		expectedPixels = _width * _height;

		while (expectedPixels > 0) {
			rleByte = stream->readByte();

			if (rleByte & 0x80) {
				expectedPixels -= rleByte & 0x7F;
				stream->skip(1);
			} else {
				// skip over pixels
				expectedPixels -= rleByte;
				stream->skip(rleByte); // skip over pixel data
			}
		}
		if (expectedPixels < 0) {
			warning("AccessVIDMoviePlay: pixel count mismatch during full frame compressed fill, data corrupt?");
			return false;
		}
		break;
	}
	default:
		assert(0);
		break;
	}
	return true;
}

bool AccessVIDMovieDecoder::StreamVideoTrack::skipOverPalette(Common::SeekableReadStream *stream) {
	stream->skip(0x300); // 3 bytes per color, 256 colors
	return true;
}

void AccessVIDMovieDecoder::StreamVideoTrack::decodePalette(Common::SeekableReadStream *stream) {
	byte red, green, blue;
	assert(stream);

	// VID files use a 6-bit palette and not a 8-bit one, we change it to 8-bit
	for (uint16 curColor = 0; curColor < 256; curColor++) {
		red = stream->readByte() & 0x3F;
		green = stream->readByte() & 0x3F;
		blue = stream->readByte() & 0x3F;
		_palette[curColor * 3] = (red << 2) | (red >> 4);
		_palette[curColor * 3 + 1] = (green << 2) | (green >> 4);
		_palette[curColor * 3 + 2] = (blue << 2) | (blue >> 4);
	}

	_dirtyPalette = true;
}

const byte *AccessVIDMovieDecoder::StreamVideoTrack::getPalette() const {
	_dirtyPalette = false;
	return _palette;
}

bool AccessVIDMovieDecoder::StreamVideoTrack::hasDirtyPalette() const {
	return _dirtyPalette;
}

AccessVIDMovieDecoder::StreamAudioTrack::StreamAudioTrack(uint32 sampleRate, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType) {
	_totalAudioQueued = 0; // currently 0 milliseconds queued

	_sampleRate  = sampleRate;
	_stereo = false; // always mono

	_audioStream = Audio::makeQueuingAudioStream(sampleRate, _stereo);
}

AccessVIDMovieDecoder::StreamAudioTrack::~StreamAudioTrack() {
	delete _audioStream;
}

void AccessVIDMovieDecoder::StreamAudioTrack::queueAudio(Common::SeekableReadStream *stream, byte chunkId) {
	Common::SeekableReadStream *rawAudioStream = 0;
	Audio::RewindableAudioStream *audioStream = 0;
	uint32 audioLengthMSecs = 0;

	if (chunkId == kVIDMovieChunkId_AudioFirstChunk) {
		stream->skip(3); // skip over additional delay + sample rate
	}

	uint32 audioSize = stream->readUint16LE();

	// Read the specified chunk into memory
	rawAudioStream = stream->readStream(audioSize);
	audioLengthMSecs = audioSize * 1000 / _sampleRate; // 1 byte == 1 8-bit sample

	audioStream = Audio::makeRawStream(rawAudioStream, _sampleRate, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::YES);
	if (audioStream) {
		_totalAudioQueued += audioLengthMSecs;
		_audioStream->queueAudioStream(audioStream, DisposeAfterUse::YES);
	} else {
		// in case there was an error
		delete rawAudioStream;
	}
}

bool AccessVIDMovieDecoder::StreamAudioTrack::skipOverAudio(Common::SeekableReadStream *stream, byte chunkId) {
	if (chunkId == kVIDMovieChunkId_AudioFirstChunk) {
		stream->skip(3); // skip over additional delay + sample rate
	}
	uint32 audioSize = stream->readUint16LE();
	stream->skip(audioSize);
	return true;
}

Audio::AudioStream *AccessVIDMovieDecoder::StreamAudioTrack::getAudioStream() const {
	return _audioStream;
}

bool AccessEngine::playMovie(const Common::String &filename, const Common::Point &pos) {
	AccessVIDMovieDecoder *videoDecoder = new AccessVIDMovieDecoder();

	Common::Point framePos(pos.x, pos.y);

	if (!videoDecoder->loadFile(filename)) {
		warning("AccessVIDMoviePlay: could not open '%s'", filename.c_str());
		return false;
	}

	bool skipVideo = false;

	_events->clearEvents();
	videoDecoder->start();

	while (!shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				_screen->blitFrom(*frame);

				if (videoDecoder->hasDirtyPalette()) {
					const byte *palette = videoDecoder->getPalette();
					g_system->getPaletteManager()->setPalette(palette, 0, 256);
				}

				_screen->update();
			}
		}

		_events->pollEventsAndWait();

		Common::KeyState keyState;
		if (_events->getKey(keyState)) {
			if (keyState.keycode == Common::KEYCODE_ESCAPE)
				skipVideo = true;
		}
	}

	videoDecoder->close();
	delete videoDecoder;

	return !skipVideo;
}

} // End of namespace Access
