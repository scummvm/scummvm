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
#include "video/codecs/msvideo1.h"
#include "video/codecs/msrle.h"
#include "video/codecs/truemotion1.h"

namespace Video {

/*
static byte char2num(char c) {
	return (c >= 48 && c <= 57) ? c - 48 : 0;
}

static byte getStreamNum(uint32 tag) {
	return char2num((char)(tag >> 24)) * 16 + char2num((char)(tag >> 16));
}
*/

static uint16 getStreamType(uint32 tag) {
	return tag & 0xffff;
}

AviDecoder::AviDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) : _mixer(mixer) {
	_soundType = soundType;

	_videoCodec = NULL;
	_decodedHeader = false;
	_audStream = NULL;
	_fileStream = NULL;
	_audHandle = new Audio::SoundHandle();
	_dirtyPalette = false;
	memset(_palette, 0, sizeof(_palette));
	memset(&_wvInfo, 0, sizeof(PCMWAVEFORMAT));
	memset(&_bmInfo, 0, sizeof(BITMAPINFOHEADER));
	memset(&_vidsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_audsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_ixInfo, 0, sizeof(AVIOLDINDEX));
}

AviDecoder::~AviDecoder() {
	close();
	delete _audHandle;
}

void AviDecoder::runHandle(uint32 tag) {
	assert (_fileStream);
	if (_fileStream->eos())
		return;

	debug (3, "Decoding tag %s", tag2str(tag));

	switch (tag) {
		case ID_RIFF:
			/*_filesize = */_fileStream->readUint32LE();
			if (_fileStream->readUint32BE() != ID_AVI)
				error("RIFF file is not an AVI video");
			break;
		case ID_LIST:
			handleList();
			break;
		case ID_AVIH:
			_header.size = _fileStream->readUint32LE();
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
			//Ignore 16 bytes of reserved data
			_fileStream->skip(16);
			break;
		case ID_STRH:
			handleStreamHeader();
			break;
		case ID_STRD: // Extra stream info, safe to ignore
		case ID_VEDT: // Unknown, safe to ignore
		case ID_JUNK: // Alignment bytes, should be ignored
			{
			uint32 junkSize = _fileStream->readUint32LE();
			_fileStream->skip(junkSize + (junkSize & 1)); // Alignment
			} break;
		case ID_IDX1:
			_ixInfo.size = _fileStream->readUint32LE();
			_ixInfo.indices = new AVIOLDINDEX::Index[_ixInfo.size / 16];
			debug (0, "%d Indices", (_ixInfo.size / 16));
			for (uint32 i = 0; i < (_ixInfo.size / 16); i++) {
				_ixInfo.indices[i].id = _fileStream->readUint32BE();
				_ixInfo.indices[i].flags = _fileStream->readUint32LE();
				_ixInfo.indices[i].offset = _fileStream->readUint32LE();
				_ixInfo.indices[i].size = _fileStream->readUint32LE();
				debug (0, "Index %d == Tag \'%s\', Offset = %d, Size = %d", i, tag2str(_ixInfo.indices[i].id), _ixInfo.indices[i].offset, _ixInfo.indices[i].size);
			}
			break;
		default:
			error ("Unknown tag \'%s\' found", tag2str(tag));
	}
}

void AviDecoder::handleList() {
	uint32 listSize = _fileStream->readUint32LE() - 4; // Subtract away listType's 4 bytes
	uint32 listType = _fileStream->readUint32BE();
	uint32 curPos = _fileStream->pos();

	debug (0, "Found LIST of type %s", tag2str(listType));

	while ((_fileStream->pos() - curPos) < listSize)
		runHandle(_fileStream->readUint32BE());

	// We now have all the header data
	if (listType == ID_HDRL)
		_decodedHeader = true;
}

void AviDecoder::handleStreamHeader() {
	AVIStreamHeader sHeader;
	sHeader.size = _fileStream->readUint32LE();
	sHeader.streamType = _fileStream->readUint32BE();
	if (sHeader.streamType == ID_MIDS || sHeader.streamType == ID_TXTS)
		error ("Unhandled MIDI/Text stream");
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
		_vidsHeader = sHeader;

		_bmInfo.size = _fileStream->readUint32LE();
		_bmInfo.width = _fileStream->readUint32LE();
		assert (_header.width == _bmInfo.width);
		_bmInfo.height = _fileStream->readUint32LE();
		assert (_header.height == _bmInfo.height);
		_bmInfo.planes = _fileStream->readUint16LE();
		_bmInfo.bitCount = _fileStream->readUint16LE();
		_bmInfo.compression = _fileStream->readUint32BE();
		_bmInfo.sizeImage = _fileStream->readUint32LE();
		_bmInfo.xPelsPerMeter = _fileStream->readUint32LE();
		_bmInfo.yPelsPerMeter = _fileStream->readUint32LE();
		_bmInfo.clrUsed = _fileStream->readUint32LE();
		_bmInfo.clrImportant = _fileStream->readUint32LE();

		if (_bmInfo.bitCount == 8) {
			if (_bmInfo.clrUsed == 0)
				_bmInfo.clrUsed = 256;

			for (uint32 i = 0; i < _bmInfo.clrUsed; i++) {
				_palette[i * 3 + 2] = _fileStream->readByte();
				_palette[i * 3 + 1] = _fileStream->readByte();
				_palette[i * 3] = _fileStream->readByte();
				_fileStream->readByte();
			}

			_dirtyPalette = true;
		}

		if (!_vidsHeader.streamHandler)
			_vidsHeader.streamHandler = _bmInfo.compression;
	} else if (sHeader.streamType == ID_AUDS) {
		_audsHeader = sHeader;

		_wvInfo.tag = _fileStream->readUint16LE();
		_wvInfo.channels = _fileStream->readUint16LE();
		_wvInfo.samplesPerSec = _fileStream->readUint32LE();
		_wvInfo.avgBytesPerSec = _fileStream->readUint32LE();
		_wvInfo.blockAlign = _fileStream->readUint16LE();
		_wvInfo.size = _fileStream->readUint16LE();

		// AVI seems to treat the sampleSize as including the second
		// channel as well, so divide for our sake.
		if (_wvInfo.channels == 2)
			_audsHeader.sampleSize /= 2;
	}

	// Ensure that we're at the end of the chunk
	_fileStream->seek(startPos + strfSize);
}

bool AviDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_fileStream = stream;
	_decodedHeader = false;

	// Read chunks until we have decoded the header
	while (!_decodedHeader)
		runHandle(_fileStream->readUint32BE());

	uint32 nextTag = _fileStream->readUint32BE();

	// Throw out any JUNK section
	if (nextTag == ID_JUNK) {
		runHandle(ID_JUNK);
		nextTag = _fileStream->readUint32BE();
	}

	// Ignore the 'movi' LIST
	if (nextTag == ID_LIST) {
		_fileStream->readUint32BE(); // Skip size
		if (_fileStream->readUint32BE() != ID_MOVI)
			error ("Expected 'movi' LIST");
	} else
		error ("Expected 'movi' LIST");

	// Now, create the codec
	_videoCodec = createCodec();

	// Initialize the video stuff too
	_audStream = createAudioStream();
	if (_audStream)
		_mixer->playStream(_soundType, _audHandle, _audStream);

	debug (0, "Frames = %d, Dimensions = %d x %d", _header.totalFrames, _header.width, _header.height);
	debug (0, "Frame Rate = %d", _vidsHeader.rate / _vidsHeader.scale);
	if (_wvInfo.samplesPerSec != 0)
		debug (0, "Sound Rate = %d", _wvInfo.samplesPerSec);
	debug (0, "Video Codec = \'%s\'", tag2str(_vidsHeader.streamHandler));

	if (!_videoCodec)
		return false;

	return true;
}

void AviDecoder::close() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;

	// Deinitialize sound
	_mixer->stopHandle(*_audHandle);
	_audStream = 0;

	_decodedHeader = false;

	delete _videoCodec;
	_videoCodec = 0;

	delete[] _ixInfo.indices;
	_ixInfo.indices = 0;

	memset(_palette, 0, sizeof(_palette));
	memset(&_wvInfo, 0, sizeof(PCMWAVEFORMAT));
	memset(&_bmInfo, 0, sizeof(BITMAPINFOHEADER));
	memset(&_vidsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_audsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_ixInfo, 0, sizeof(AVIOLDINDEX));

	reset();
}

uint32 AviDecoder::getElapsedTime() const {
	if (_audStream)
		return _mixer->getSoundElapsedTime(*_audHandle);

	return FixedRateVideoDecoder::getElapsedTime();
}

const Graphics::Surface *AviDecoder::decodeNextFrame() {
	uint32 nextTag = _fileStream->readUint32BE();

	if (_fileStream->eos())
		return NULL;

	if (_curFrame == -1)
		_startTime = g_system->getMillis();

	if (nextTag == ID_LIST) {
		// A list of audio/video chunks
		uint32 listSize = _fileStream->readUint32LE() - 4;
		int32 startPos = _fileStream->pos();

		if (_fileStream->readUint32BE() != ID_REC)
			error ("Expected 'rec ' LIST");

		// Decode chunks in the list and see if we get a frame
		const Graphics::Surface *frame = NULL;
		while (_fileStream->pos() < startPos + (int32)listSize) {
			const Graphics::Surface *temp = decodeNextFrame();
			if (temp)
				frame = temp;
		}

		return frame;
	} else if (getStreamType(nextTag) == 'wb') {
		// Audio Chunk
		uint32 chunkSize = _fileStream->readUint32LE();
		queueAudioBuffer(chunkSize);
		_fileStream->skip(chunkSize & 1); // Alignment
	} else if (getStreamType(nextTag) == 'dc' || getStreamType(nextTag) == 'id' ||
	           getStreamType(nextTag) == 'AM' || getStreamType(nextTag) == '32' ||
			   getStreamType(nextTag) == 'iv') {
		// Compressed Frame
		_curFrame++;
		uint32 chunkSize = _fileStream->readUint32LE();

		if (chunkSize == 0) // Keep last frame on screen
			return NULL;

		Common::SeekableReadStream *frameData = _fileStream->readStream(chunkSize);
		const Graphics::Surface *surface = _videoCodec->decodeImage(frameData);
		delete frameData;
		_fileStream->skip(chunkSize & 1); // Alignment
		return surface;
	} else if (getStreamType(nextTag) == 'pc') {
		// Palette Change
		_fileStream->readUint32LE(); // Chunk size, not needed here
		byte firstEntry = _fileStream->readByte();
		uint16 numEntries = _fileStream->readByte();
		_fileStream->readUint16LE(); // Reserved

		// 0 entries means all colors are going to be changed
		if (numEntries == 0)
			numEntries = 256;

		for (uint16 i = firstEntry; i < numEntries + firstEntry; i++) {
			_palette[i * 3] = _fileStream->readByte();
			_palette[i * 3 + 1] = _fileStream->readByte();
			_palette[i * 3 + 2] = _fileStream->readByte();
			_fileStream->readByte(); // Flags that don't serve us any purpose
		}

		_dirtyPalette = true;

		// No alignment necessary. It's always even.
	} else if (nextTag == ID_JUNK) {
		runHandle(ID_JUNK);
	} else if (nextTag == ID_IDX1) {
		runHandle(ID_IDX1);
	} else
		error("Tag = \'%s\', %d", tag2str(nextTag), _fileStream->pos());

	return NULL;
}

Codec *AviDecoder::createCodec() {
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
		default:
			warning ("Unknown/Unhandled compression format \'%s\'", tag2str(_vidsHeader.streamHandler));
	}

	return NULL;
}

Graphics::PixelFormat AviDecoder::getPixelFormat() const {
	assert(_videoCodec);
	return _videoCodec->getPixelFormat();
}

Audio::QueuingAudioStream *AviDecoder::createAudioStream() {
	if (_wvInfo.tag == kWaveFormatPCM || _wvInfo.tag == kWaveFormatDK3)
		return Audio::makeQueuingAudioStream(_wvInfo.samplesPerSec, _wvInfo.channels == 2);
	else if (_wvInfo.tag != kWaveFormatNone) // No sound
		warning("Unsupported AVI audio format %d", _wvInfo.tag);

	return NULL;
}

void AviDecoder::queueAudioBuffer(uint32 chunkSize) {
	// Return if we haven't created the queue (unsupported audio format)
	if (!_audStream) {
		_fileStream->skip(chunkSize);
		return;
	}

	Common::SeekableReadStream *stream = _fileStream->readStream(chunkSize);

	if (_wvInfo.tag == kWaveFormatPCM) {
		byte flags = 0;
		if (_audsHeader.sampleSize == 2)
			flags |= Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
		else
			flags |= Audio::FLAG_UNSIGNED;

		if (_wvInfo.channels == 2)
			flags |= Audio::FLAG_STEREO;

		_audStream->queueAudioStream(Audio::makeRawStream(stream, _wvInfo.samplesPerSec, flags, DisposeAfterUse::YES), DisposeAfterUse::YES);
	} else if (_wvInfo.tag == kWaveFormatDK3) {
		_audStream->queueAudioStream(Audio::makeADPCMStream(stream, DisposeAfterUse::YES, chunkSize, Audio::kADPCMDK3, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign), DisposeAfterUse::YES);
	}
}

} // End of namespace Video
