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
 * $URL$
 * $Id$
 *
 */

#include "common/archive.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/events.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "graphics/video/avi_decoder.h"

// Codecs
#include "graphics/video/codecs/msvideo1.h"

namespace Graphics {

AviDecoder::AviDecoder(Audio::Mixer *mixer) : _mixer(mixer) {
	_videoCodec = NULL;
	_decodedHeader = false;
	_audStream = NULL;
	_fileStream = NULL;
	_audHandle = new Audio::SoundHandle();
	memset(&_wvInfo, 0, sizeof(PCMWAVEFORMAT));
	memset(&_bmInfo, 0, sizeof(BITMAPINFOHEADER));
	memset(&_vidsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_audsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_ixInfo, 0, sizeof(AVIOLDINDEX));
}

AviDecoder::~AviDecoder() {
	closeFile();
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
	sHeader.frame.left = _fileStream->readSint16LE();
	sHeader.frame.top = _fileStream->readSint16LE();
	sHeader.frame.right = _fileStream->readSint16LE();
	sHeader.frame.bottom = _fileStream->readSint16LE();
		
	if (_fileStream->readUint32BE() != ID_STRF)
		error("Could not find STRF tag");
	/* uint32 strfSize = */ _fileStream->readUint32LE();
	
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
				/*_palette[i * 4 + 3] = */_fileStream->readByte();
			}
			
			setPalette(_palette);
		}
	} else if (sHeader.streamType == ID_AUDS) {
		_audsHeader = sHeader;
		
		_wvInfo.tag = _fileStream->readUint16LE();
		_wvInfo.channels = _fileStream->readUint16LE();
		_wvInfo.samplesPerSec = _fileStream->readUint32LE();
		_wvInfo.avgBytesPerSec = _fileStream->readUint32LE();
		_wvInfo.blockAlign = _fileStream->readUint16LE();
		_wvInfo.size = _fileStream->readUint16LE();
	}
}

bool AviDecoder::loadFile(const char *fileName) {
	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;
	
	_decodedHeader = false;
	// Seek to the first frame
	_videoInfo.currentFrame = 0;
	

	// Read chunks until we have decoded the header
	while (!_decodedHeader)
		runHandle(_fileStream->readUint32BE());
		
	_videoFrameBuffer = new byte[_header.width * _header.height];
	memset(_videoFrameBuffer, 0, _header.width * _header.height);

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
		_mixer->playInputStream(Audio::Mixer::kPlainSoundType, _audHandle, _audStream);
			
	debug (0, "Frames = %d, Dimensions = %d x %d", _header.totalFrames, _header.width, _header.height);
	debug (0, "Frame Rate = %d", getFrameRate());
	if (_header.flags & AVIF_ISINTERLEAVED)
		debug (0, "Sound Rate = %d", AUDIO_RATE);
	debug (0, "Video Codec = \'%s\'", tag2str(_vidsHeader.streamHandler));

	_videoInfo.firstframeOffset = _fileStream->pos();
	_videoInfo.width = _header.width;
	_videoInfo.height = _header.height;
	_videoInfo.frameCount = _header.totalFrames;
	// Our frameDelay is calculated in 1/100 ms, so we convert it here
	_videoInfo.frameDelay = _header.microSecondsPerFrame / 10;

	if (!_videoCodec)
		return false;

	return true;
}

void AviDecoder::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;

	delete[] _videoFrameBuffer;
	_videoFrameBuffer = 0;
		
	// Deinitialize sound
	_mixer->stopHandle(*_audHandle);
		
	_decodedHeader = false;
	
	delete _videoCodec;
	_videoCodec = 0;

	delete[] _ixInfo.indices;
	_ixInfo.indices = 0;
}

Surface *AviDecoder::getNextFrame() {
	uint32 nextTag = _fileStream->readUint32BE();

	if (_fileStream->eos())
		return NULL;

	if (nextTag == ID_LIST) {
		// A list of audio/video chunks
		uint32 listSize = _fileStream->readUint32LE() - 4;
		int32 startPos = _fileStream->pos();

		if (_fileStream->readUint32BE() != ID_REC)
			error ("Expected 'rec ' LIST");
		
		// Decode chunks in the list and see if we get a frame
		Surface *frame = NULL;
		while (_fileStream->pos() < startPos + (int32)listSize) {
			Surface *temp = getNextFrame();
			if (temp)
				frame = temp;
		}

		return frame;
	} else if (getStreamType(nextTag) == 'wb') {
		// Audio Chunk
		uint32 chunkSize = _fileStream->readUint32LE();
		byte *data = new byte[chunkSize];
		_fileStream->read(data, chunkSize);
		_audStream->queueBuffer(data, chunkSize);
		_fileStream->skip(chunkSize & 1); // Alignment
	} else if (getStreamType(nextTag) == 'dc' || getStreamType(nextTag) == 'id' || getStreamType(nextTag) == 'AM') {		
		// Compressed Frame
		_videoInfo.currentFrame++;
		uint32 chunkSize = _fileStream->readUint32LE();

		if (chunkSize == 0) // Keep last frame on screen
			return NULL;
		
		Common::SeekableReadStream *frameData = _fileStream->readStream(chunkSize);
		Graphics::Surface *surface = _videoCodec->decodeImage(frameData);
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
		
		setPalette(_palette);

		// No alignment necessary. It's always even.
	} else if (nextTag == ID_JUNK) {
		runHandle(ID_JUNK);
	} else if (nextTag == ID_IDX1) {
		runHandle(ID_IDX1);
	} else
		error ("Tag = \'%s\', %d", tag2str(nextTag), _fileStream->pos());

	return NULL;
}

bool AviDecoder::decodeNextFrame() {	
	if (_videoInfo.currentFrame == 0)
		_videoInfo.startTime = g_system->getMillis();

	Surface *surface = NULL;
	
	uint32 curFrame = _videoInfo.currentFrame;

	while (!surface && _videoInfo.currentFrame < _videoInfo.frameCount && !_fileStream->eos())
		surface = getNextFrame();
	
	if (curFrame == _videoInfo.currentFrame) {
		warning("No video frame found");
		_videoInfo.currentFrame++;
	}

	if (surface)
		memcpy(_videoFrameBuffer, surface->pixels, _header.width * _header.height);

	return _videoInfo.currentFrame < _videoInfo.frameCount;
}

int32 AviDecoder::getAudioLag() {
	if (!_fileStream)
		return 0;

	int32 frameDelay = getFrameDelay();
	int32 videoTime = _videoInfo.currentFrame * frameDelay;
	int32 audioTime;

	if (!_audStream) {
		/* No audio.
		   Calculate the lag by how much time has gone by since the first frame
		   and how much time *should* have passed.
		*/

		audioTime = (g_system->getMillis() - _videoInfo.startTime) * 100;
	} else
		audioTime = (((int32)_mixer->getSoundElapsedTime(*_audHandle)) * 100);

	return videoTime - audioTime;
}

Codec *AviDecoder::createCodec() {
	switch (_vidsHeader.streamHandler) {
		case ID_CRAM:
		case ID_MSVC:
		case ID_WHAM:
			return new MSVideo1Decoder(_bmInfo.width, _bmInfo.height, _bmInfo.bitCount);
		default:
			warning ("Unknown/Unhandled compression format \'%s\'", tag2str(_vidsHeader.streamHandler));
	}
	
	return NULL;
}

Audio::AppendableAudioStream *AviDecoder::createAudioStream() {
	if (_wvInfo.tag == AVI_WAVE_FORMAT_PCM)
		return Audio::makeAppendableAudioStream(AUDIO_RATE, Audio::Mixer::FLAG_UNSIGNED|Audio::Mixer::FLAG_AUTOFREE);
	
	if (_wvInfo.tag != 0) // No sound
		warning ("Unsupported AVI audio format %d", _wvInfo.tag);

	return NULL;
}
	
byte AviDecoder::char2num(char c) {
	return (c >= 48 && c <= 57) ? c - 48 : 0;
}
	
byte AviDecoder::getStreamNum(uint32 tag) {
	return char2num((char)(tag >> 24)) * 16 + char2num((char)(tag >> 16));
}

uint16 AviDecoder::getStreamType(uint32 tag) {
	return tag & 0xffff;
}

} // End of namespace Graphics
