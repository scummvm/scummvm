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

#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "video/avi_decoder.h"

// Audio Codecs
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"

// Video Codecs
#include "video/codecs/cinepak.h"
#include "video/codecs/indeo3.h"
#include "video/codecs/mpeg.h"
#include "video/codecs/msvideo1.h"
#include "video/codecs/msrle.h"
#include "video/codecs/truemotion1.h"

namespace Video {

#define UNKNOWN_HEADER(a) error("Unknown header found -- \'%s\'", tag2str(a))

// IDs used throughout the AVI files
// that will be handled by this player
#define ID_RIFF MKTAG('R','I','F','F')
#define ID_AVI  MKTAG('A','V','I',' ')
#define ID_LIST MKTAG('L','I','S','T')
#define ID_HDRL MKTAG('h','d','r','l')
#define ID_AVIH MKTAG('a','v','i','h')
#define ID_STRL MKTAG('s','t','r','l')
#define ID_STRH MKTAG('s','t','r','h')
#define ID_VIDS MKTAG('v','i','d','s')
#define ID_AUDS MKTAG('a','u','d','s')
#define ID_MIDS MKTAG('m','i','d','s')
#define ID_TXTS MKTAG('t','x','t','s')
#define ID_JUNK MKTAG('J','U','N','K')
#define ID_STRF MKTAG('s','t','r','f')
#define ID_MOVI MKTAG('m','o','v','i')
#define ID_REC  MKTAG('r','e','c',' ')
#define ID_VEDT MKTAG('v','e','d','t')
#define ID_IDX1 MKTAG('i','d','x','1')
#define ID_STRD MKTAG('s','t','r','d')
#define ID_INFO MKTAG('I','N','F','O')
#define ID_ISFT MKTAG('I','S','F','T')
#define ID_DISP MKTAG('D','I','S','P')
#define ID_PRMI MKTAG('P','R','M','I')

// Codec tags
#define ID_RLE  MKTAG('R','L','E',' ')
#define ID_CRAM MKTAG('C','R','A','M')
#define ID_MSVC MKTAG('m','s','v','c')
#define ID_WHAM MKTAG('W','H','A','M')
#define ID_CVID MKTAG('c','v','i','d')
#define ID_IV32 MKTAG('i','v','3','2')
#define ID_DUCK MKTAG('D','U','C','K')
#define ID_MPG2 MKTAG('m','p','g','2')


AVIDecoder::AVIDecoder(Audio::Mixer::SoundType soundType) : _frameRateOverride(0), _soundType(soundType) {
	initCommon();
}

AVIDecoder::AVIDecoder(const Common::Rational &frameRateOverride, Audio::Mixer::SoundType soundType)
		: _frameRateOverride(frameRateOverride), _soundType(soundType) {
	initCommon();
}

AVIDecoder::~AVIDecoder() {
	close();
}

void AVIDecoder::initCommon() {
	_decodedHeader = false;
	_foundMovieList = false;
	_movieListStart = 0;
	_fileStream = 0;
	memset(&_header, 0, sizeof(_header));
}

bool AVIDecoder::parseNextChunk() {
	uint32 tag = _fileStream->readUint32BE();
	uint32 size = _fileStream->readUint32LE();

	if (_fileStream->eos())
		return false;

	debug(3, "Decoding tag %s", tag2str(tag));

	switch (tag) {
	case ID_LIST:
		handleList(size);
		break;
	case ID_AVIH:
		_header.size = size;
		_header.microSecondsPerFrame = _fileStream->readUint32LE();
		_header.maxBytesPerSecond = _fileStream->readUint32LE();
		_header.padding = _fileStream->readUint32LE();
		_header.flags = _fileStream->readUint32LE();
		_header.totalFrames = _fileStream->readUint32LE();
		_header.initialFrames = _fileStream->readUint32LE();
		_header.streams = _fileStream->readUint32LE();
		_header.bufferSize = _fileStream->readUint32LE();
		_header.width = _fileStream->readUint32LE();
		_header.height = _fileStream->readUint32LE();
		// Ignore 16 bytes of reserved data
		_fileStream->skip(16);
		break;
	case ID_STRH:
		handleStreamHeader(size);
		break;
	case ID_STRD: // Extra stream info, safe to ignore
	case ID_VEDT: // Unknown, safe to ignore
	case ID_JUNK: // Alignment bytes, should be ignored
	case ID_ISFT: // Metadata, safe to ignore
	case ID_DISP: // Metadata, should be safe to ignore
		skipChunk(size);
		break;
	case ID_IDX1:
		debug(0, "%d Indices", size / 16);
		for (uint32 i = 0; i < size / 16; i++) {
			OldIndex indexEntry;
			indexEntry.id = _fileStream->readUint32BE();
			indexEntry.flags = _fileStream->readUint32LE();
			indexEntry.offset = _fileStream->readUint32LE();
			indexEntry.size = _fileStream->readUint32LE();
			_indexEntries.push_back(indexEntry);
			debug(0, "Index %d == Tag \'%s\', Offset = %d, Size = %d", i, tag2str(indexEntry.id), indexEntry.offset, indexEntry.size);
		}
		break;
	default:
		error("Unknown tag \'%s\' found", tag2str(tag));
	}

	return true;
}

void AVIDecoder::skipChunk(uint32 size) {
	// Make sure we're aligned on a word boundary
	_fileStream->skip(size + (size & 1));
}

void AVIDecoder::handleList(uint32 listSize) {
	uint32 listType = _fileStream->readUint32BE();
	listSize -= 4; // Subtract away listType's 4 bytes
	uint32 curPos = _fileStream->pos();

	debug(0, "Found LIST of type %s", tag2str(listType));

	switch (listType) {
	case ID_MOVI: // Movie List
		// We found the movie block
		_foundMovieList = true;
		_movieListStart = curPos;
		_fileStream->skip(listSize);
		return;
	case ID_HDRL: // Header List
		// Mark the header as decoded
		_decodedHeader = true;
		break;
	case ID_INFO: // Metadata
	case ID_PRMI: // Unknown metadata, should be safe to ignore
		// Ignore metadata
		_fileStream->skip(listSize);
		return;
	case ID_STRL: // Stream list
	default:      // (Just hope we can parse it!)
		break;
	}

	while ((_fileStream->pos() - curPos) < listSize)
		parseNextChunk();
}

void AVIDecoder::handleStreamHeader(uint32 size) {
	AVIStreamHeader sHeader;
	sHeader.size = size;
	sHeader.streamType = _fileStream->readUint32BE();

	if (sHeader.streamType == ID_MIDS || sHeader.streamType == ID_TXTS)
		error("Unhandled MIDI/Text stream");

	sHeader.streamHandler = _fileStream->readUint32BE();
	sHeader.flags = _fileStream->readUint32LE();
	sHeader.priority = _fileStream->readUint16LE();
	sHeader.language = _fileStream->readUint16LE();
	sHeader.initialFrames = _fileStream->readUint32LE();
	sHeader.scale = _fileStream->readUint32LE();
	sHeader.rate = _fileStream->readUint32LE();
	sHeader.start = _fileStream->readUint32LE();
	sHeader.length = _fileStream->readUint32LE();
	sHeader.bufferSize = _fileStream->readUint32LE();
	sHeader.quality = _fileStream->readUint32LE();
	sHeader.sampleSize = _fileStream->readUint32LE();

	_fileStream->skip(sHeader.size - 48); // Skip over the remainder of the chunk (frame)

	if (_fileStream->readUint32BE() != ID_STRF)
		error("Could not find STRF tag");

	uint32 strfSize = _fileStream->readUint32LE();
	uint32 startPos = _fileStream->pos();

	if (sHeader.streamType == ID_VIDS) {
		if (_frameRateOverride != 0) {
			sHeader.rate = _frameRateOverride.getNumerator();
			sHeader.scale = _frameRateOverride.getDenominator();
		}

		BitmapInfoHeader bmInfo;
		bmInfo.size = _fileStream->readUint32LE();
		bmInfo.width = _fileStream->readUint32LE();
		bmInfo.height = _fileStream->readUint32LE();
		bmInfo.planes = _fileStream->readUint16LE();
		bmInfo.bitCount = _fileStream->readUint16LE();
		bmInfo.compression = _fileStream->readUint32BE();
		bmInfo.sizeImage = _fileStream->readUint32LE();
		bmInfo.xPelsPerMeter = _fileStream->readUint32LE();
		bmInfo.yPelsPerMeter = _fileStream->readUint32LE();
		bmInfo.clrUsed = _fileStream->readUint32LE();
		bmInfo.clrImportant = _fileStream->readUint32LE();

		if (bmInfo.clrUsed == 0)
			bmInfo.clrUsed = 256;

		if (sHeader.streamHandler == 0)
			sHeader.streamHandler = bmInfo.compression;

		AVIVideoTrack *track = new AVIVideoTrack(_header.totalFrames, sHeader, bmInfo);

		if (bmInfo.bitCount == 8) {
			byte *palette = const_cast<byte *>(track->getPalette());
			for (uint32 i = 0; i < bmInfo.clrUsed; i++) {
				palette[i * 3 + 2] = _fileStream->readByte();
				palette[i * 3 + 1] = _fileStream->readByte();
				palette[i * 3] = _fileStream->readByte();
				_fileStream->readByte();
			}

			track->markPaletteDirty();
		}

		addTrack(track);
	} else if (sHeader.streamType == ID_AUDS) {
		PCMWaveFormat wvInfo;
		wvInfo.tag = _fileStream->readUint16LE();
		wvInfo.channels = _fileStream->readUint16LE();
		wvInfo.samplesPerSec = _fileStream->readUint32LE();
		wvInfo.avgBytesPerSec = _fileStream->readUint32LE();
		wvInfo.blockAlign = _fileStream->readUint16LE();
		wvInfo.size = _fileStream->readUint16LE();

		// AVI seems to treat the sampleSize as including the second
		// channel as well, so divide for our sake.
		if (wvInfo.channels == 2)
			sHeader.sampleSize /= 2;

		addTrack(new AVIAudioTrack(sHeader, wvInfo, _soundType));
	}

	// Ensure that we're at the end of the chunk
	_fileStream->seek(startPos + strfSize);
}

bool AVIDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	uint32 riffTag = stream->readUint32BE();
	if (riffTag != ID_RIFF) {
		warning("Failed to find RIFF header");
		return false;
	}

	/* uint32 fileSize = */ stream->readUint32LE();
	uint32 riffType = stream->readUint32BE();

	if (riffType != ID_AVI) {
		warning("RIFF not an AVI file");
		return false;
	}

	_fileStream = stream;

	// Go through all chunks in the file
	while (parseNextChunk())
		;

	if (!_decodedHeader) {
		warning("Failed to parse AVI header");
		close();
		return false;
	}

	if (!_foundMovieList) {
		warning("Failed to find 'MOVI' list");
		close();
		return false;
	}

	// Seek back to the start of the MOVI list
	_fileStream->seek(_movieListStart);

	return true;
}

void AVIDecoder::close() {
	VideoDecoder::close();

	delete _fileStream;
	_fileStream = 0;
	_decodedHeader = false;
	_foundMovieList = false;
	_movieListStart = 0;

	_indexEntries.clear();
	memset(&_header, 0, sizeof(_header));
}

void AVIDecoder::readNextPacket() {
	uint32 nextTag = _fileStream->readUint32BE();
	uint32 size = _fileStream->readUint32LE();

	if (_fileStream->eos())
		return;

	if (nextTag == ID_LIST) {
		// A list of audio/video chunks
		int32 startPos = _fileStream->pos();

		if (_fileStream->readUint32BE() != ID_REC)
			error("Expected 'rec ' LIST");

		size -= 4; // subtract list type

		// Decode chunks in the list
		while (_fileStream->pos() < startPos + (int32)size)
			readNextPacket();

		return;
	} else if (nextTag == ID_JUNK || nextTag == ID_IDX1) {
		skipChunk(size);
		return;
	}

	Track *track = getTrack(getStreamIndex(nextTag));

	if (!track)
		error("Cannot get track from tag '%s'", tag2str(nextTag));

	Common::SeekableReadStream *chunk = 0;

	if (size != 0) {
		chunk = _fileStream->readStream(size);
		_fileStream->skip(size & 1);
	}

	if (track->getTrackType() == Track::kTrackTypeAudio) {
		if (getStreamType(nextTag) != MKTAG16('w', 'b'))
			error("Invalid audio track tag '%s'", tag2str(nextTag));

		assert(chunk);
		((AVIAudioTrack *)track)->queueSound(chunk);
	} else {
		AVIVideoTrack *videoTrack = (AVIVideoTrack *)track;

		if (getStreamType(nextTag) == MKTAG16('p', 'c')) {
			// Palette Change
			assert(chunk);
			byte firstEntry = chunk->readByte();
			uint16 numEntries = chunk->readByte();
			chunk->readUint16LE(); // Reserved

			// 0 entries means all colors are going to be changed
			if (numEntries == 0)
				numEntries = 256;

			byte *palette = const_cast<byte *>(videoTrack->getPalette());

			for (uint16 i = firstEntry; i < numEntries + firstEntry; i++) {
				palette[i * 3] = chunk->readByte();
				palette[i * 3 + 1] = chunk->readByte();
				palette[i * 3 + 2] = chunk->readByte();
				chunk->readByte(); // Flags that don't serve us any purpose
			}

			delete chunk;
			videoTrack->markPaletteDirty();
		} else if (getStreamType(nextTag) == MKTAG16('d', 'b')) {
			// TODO: Check if this really is uncompressed. Many videos
			// falsely put compressed data in here.
			error("Uncompressed AVI frame found");
		} else {
			// Otherwise, assume it's a compressed frame
			videoTrack->decodeFrame(chunk);
		}
	}
}

byte AVIDecoder::getStreamIndex(uint32 tag) const {
	char string[3];
	WRITE_BE_UINT16(string, tag >> 16);
	string[2] = 0;
	return strtol(string, 0, 16);
}

AVIDecoder::AVIVideoTrack::AVIVideoTrack(int frameCount, const AVIStreamHeader &streamHeader, const BitmapInfoHeader &bitmapInfoHeader)
		: _frameCount(frameCount), _vidsHeader(streamHeader), _bmInfo(bitmapInfoHeader) {
	memset(_palette, 0, sizeof(_palette));
	_videoCodec = createCodec();
	_dirtyPalette = false;
	_lastFrame = 0;
	_curFrame = -1;
}

AVIDecoder::AVIVideoTrack::~AVIVideoTrack() {
	delete _videoCodec;
}

void AVIDecoder::AVIVideoTrack::decodeFrame(Common::SeekableReadStream *stream) {
	if (stream) {
		if (_videoCodec)
			_lastFrame = _videoCodec->decodeImage(stream);
	} else {
		// Empty frame
		_lastFrame = 0;
	}

	delete stream;
	_curFrame++;
}

Graphics::PixelFormat AVIDecoder::AVIVideoTrack::getPixelFormat() const {
	if (_videoCodec)
		return _videoCodec->getPixelFormat();

	return Graphics::PixelFormat();
}

Codec *AVIDecoder::AVIVideoTrack::createCodec() {
	switch (_vidsHeader.streamHandler) {
	case ID_CRAM:
	case ID_MSVC:
	case ID_WHAM:
		return new MSVideo1Decoder(_bmInfo.width, _bmInfo.height, _bmInfo.bitCount);
	case ID_RLE:
		return new MSRLEDecoder(_bmInfo.width, _bmInfo.height, _bmInfo.bitCount);
	case ID_CVID:
		return new CinepakDecoder(_bmInfo.bitCount);
	case ID_IV32:
		return new Indeo3Decoder(_bmInfo.width, _bmInfo.height);
#ifdef VIDEO_CODECS_TRUEMOTION1_H
	case ID_DUCK:
		return new TrueMotion1Decoder(_bmInfo.width, _bmInfo.height);
#endif
#ifdef USE_MPEG2
	case ID_MPG2:
		return new MPEGDecoder();
#endif
	default:
		warning("Unknown/Unhandled compression format \'%s\'", tag2str(_vidsHeader.streamHandler));
	}

	return 0;
}

AVIDecoder::AVIAudioTrack::AVIAudioTrack(const AVIStreamHeader &streamHeader, const PCMWaveFormat &waveFormat, Audio::Mixer::SoundType soundType)
		: _audsHeader(streamHeader), _wvInfo(waveFormat), _soundType(soundType) {
	_audStream = createAudioStream();
}

AVIDecoder::AVIAudioTrack::~AVIAudioTrack() {
	delete _audStream;
}

void AVIDecoder::AVIAudioTrack::queueSound(Common::SeekableReadStream *stream) {
	if (_audStream) {
		if (_wvInfo.tag == kWaveFormatPCM) {
			byte flags = 0;
			if (_audsHeader.sampleSize == 2)
				flags |= Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
			else
				flags |= Audio::FLAG_UNSIGNED;

			if (_wvInfo.channels == 2)
				flags |= Audio::FLAG_STEREO;

			_audStream->queueAudioStream(Audio::makeRawStream(stream, _wvInfo.samplesPerSec, flags, DisposeAfterUse::YES), DisposeAfterUse::YES);
		} else if (_wvInfo.tag == kWaveFormatMSADPCM) {
			_audStream->queueAudioStream(Audio::makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), Audio::kADPCMMS, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign), DisposeAfterUse::YES);
		} else if (_wvInfo.tag == kWaveFormatMSIMAADPCM) {
			_audStream->queueAudioStream(Audio::makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), Audio::kADPCMMSIma, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign), DisposeAfterUse::YES);
		} else if (_wvInfo.tag == kWaveFormatDK3) {
			_audStream->queueAudioStream(Audio::makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), Audio::kADPCMDK3, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign), DisposeAfterUse::YES);
		}
	} else {
		delete stream;
	}
}

Audio::AudioStream *AVIDecoder::AVIAudioTrack::getAudioStream() const {
	return _audStream;
}

Audio::QueuingAudioStream *AVIDecoder::AVIAudioTrack::createAudioStream() {
	if (_wvInfo.tag == kWaveFormatPCM || _wvInfo.tag == kWaveFormatMSADPCM || _wvInfo.tag == kWaveFormatMSIMAADPCM || _wvInfo.tag == kWaveFormatDK3)
		return Audio::makeQueuingAudioStream(_wvInfo.samplesPerSec, _wvInfo.channels == 2);
	else if (_wvInfo.tag != kWaveFormatNone) // No sound
		warning("Unsupported AVI audio format %d", _wvInfo.tag);

	return 0;
}

} // End of namespace Video
