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

#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/events.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "graphics/video/avi_player.h"

// Codecs
#include "graphics/video/msvideo1.h"

namespace Graphics {

AVIPlayer::AVIPlayer(OSystem* syst) : _system(syst) {
	_videoCodec = NULL;
	_palette = NULL;
	_decodedHeader = false;
	_filesize = 0;
	_curFrame = 0;
	_audStream = NULL;
	_dirtyPalette = false;
	_stream = NULL;
	_audHandle = new Audio::SoundHandle();
	memset(&_wvInfo, 0, sizeof(PCMWAVEFORMAT));
	memset(&_bmInfo, 0, sizeof(BITMAPINFOHEADER));
	memset(&_vidsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_audsHeader, 0, sizeof(AVIStreamHeader));
	memset(&_ixInfo, 0, sizeof(AVIOLDINDEX));
}

AVIPlayer::~AVIPlayer() {
	close();
	delete _audHandle;
}
	
void AVIPlayer::runHandle(uint32 tag) {
	assert (_stream);
	if (_stream->eos())
		return;

	debug (3, "Decoding tag %s", tag2str(tag));
		
	switch (tag) {
		case ID_RIFF:
			_filesize = _stream->readUint32LE();
			assert(_stream->readUint32BE() == ID_AVI);
			break;
		case ID_LIST:
			handleList();
			break;
		case ID_AVIH:
			_header.size = _stream->readUint32LE();
			_header.microSecondsPerFrame = _stream->readUint32LE();
			_header.maxBytesPerSecond = _stream->readUint32LE();
			_header.padding = _stream->readUint32LE();
			_header.flags = _stream->readUint32LE();
			_header.totalFrames = _stream->readUint32LE();
			_header.initialFrames = _stream->readUint32LE();
			_header.streams = _stream->readUint32LE();
			_header.bufferSize = _stream->readUint32LE();
			_header.width = _stream->readUint32LE();
			_header.height = _stream->readUint32LE();
			//Ignore 16 bytes of reserved data
			_stream->skip(16);
			break;
		case ID_STRH:
			handleStreamHeader();
			break;
		case ID_STRD: // Extra stream info, safe to ignore
		case ID_VEDT: // Unknown, safe to ignore
		case ID_JUNK: // Alignment bytes, should be ignored
			{
			uint32 junkSize = _stream->readUint32LE();
			_stream->skip(junkSize + (junkSize & 1)); // Alignment
			} break;
		case ID_IDX1:
			_ixInfo.size = _stream->readUint32LE();
			_ixInfo.indices = new AVIOLDINDEX::Index[_ixInfo.size / 16];
			debug (0, "%d Indices", (_ixInfo.size / 16));
			for (uint32 i = 0; i < (_ixInfo.size / 16); i++) {
				_ixInfo.indices[i].id = _stream->readUint32BE();
				_ixInfo.indices[i].flags = _stream->readUint32LE();
				_ixInfo.indices[i].offset = _stream->readUint32LE();
				_ixInfo.indices[i].size = _stream->readUint32LE();
				debug (0, "Index %d == Tag \'%s\', Offset = %d, Size = %d", i, tag2str(_ixInfo.indices[i].id), _ixInfo.indices[i].offset, _ixInfo.indices[i].size);
			}
			break;
		default: error ("Unknown tag \'%s\' found", tag2str(tag));
	}
}
	
void AVIPlayer::handleList() {
	uint32 listSize = _stream->readUint32LE() - 4; // Subtract away listType's 4 bytes
	uint32 listType = _stream->readUint32BE();
	uint32 curPos = _stream->pos();
	
	debug (0, "Found LIST of type %s", tag2str(listType));
	
	while ((_stream->pos() - curPos) < listSize)
		runHandle(_stream->readUint32BE());
	
	// We now have all the header data
	if (listType == ID_HDRL)
		_decodedHeader = true;
}
	
void AVIPlayer::handleStreamHeader() {
	AVIStreamHeader sHeader;	
	sHeader.size = _stream->readUint32LE();
	sHeader.streamType = _stream->readUint32BE();
	if (sHeader.streamType == ID_MIDS || sHeader.streamType == ID_TXTS)
		error ("Unhandled MIDI/Text stream");
	sHeader.streamHandler = _stream->readUint32BE();
	sHeader.flags = _stream->readUint32LE();
	sHeader.priority = _stream->readUint16LE();
	sHeader.language = _stream->readUint16LE();
	sHeader.initialFrames = _stream->readUint32LE();
	sHeader.scale = _stream->readUint32LE();
	sHeader.rate = _stream->readUint32LE();
	sHeader.start = _stream->readUint32LE();
	sHeader.length = _stream->readUint32LE();
	sHeader.bufferSize = _stream->readUint32LE();
	sHeader.quality = _stream->readUint32LE();
	sHeader.sampleSize = _stream->readUint32LE();
	sHeader.frame.left = _stream->readSint16LE();
	sHeader.frame.top = _stream->readSint16LE();
	sHeader.frame.right = _stream->readSint16LE();
	sHeader.frame.bottom = _stream->readSint16LE();
		
	assert (_stream->readUint32BE() == ID_STRF);
	/* uint32 strfSize = */ _stream->readUint32LE();
	
	if (sHeader.streamType == ID_VIDS) {
		_vidsHeader = sHeader;
		
		_bmInfo.size = _stream->readUint32LE();
		_bmInfo.width = _stream->readUint32LE();
		assert (_header.width == _bmInfo.width);
		_bmInfo.height = _stream->readUint32LE();
		assert (_header.height == _bmInfo.height);	
		_bmInfo.planes = _stream->readUint16LE();
		_bmInfo.bitCount = _stream->readUint16LE();
		_bmInfo.compression = _stream->readUint32BE();
		_bmInfo.sizeImage = _stream->readUint32LE();
		_bmInfo.xPelsPerMeter = _stream->readUint32LE();
		_bmInfo.yPelsPerMeter = _stream->readUint32LE();
		_bmInfo.clrUsed = _stream->readUint32LE();
		_bmInfo.clrImportant = _stream->readUint32LE();
		
		if (_bmInfo.bitCount == 8) {
			if (_bmInfo.clrUsed == 0)
				_bmInfo.clrUsed = 256;
			
			_palette = (byte *)malloc(256 * 4);
			
			for (uint32 i = 0; i < _bmInfo.clrUsed; i++) {
				_palette[i * 4 + 2] = _stream->readByte();
				_palette[i * 4 + 1] = _stream->readByte();
				_palette[i * 4] = _stream->readByte();
				_palette[i * 4 + 3] = _stream->readByte();
			}
			
			// Assign the palette to be dirty
			_dirtyPalette = true;
		}
	} else if (sHeader.streamType == ID_AUDS) {
		_audsHeader = sHeader;
		
		_wvInfo.tag = _stream->readUint16LE();
		_wvInfo.channels = _stream->readUint16LE();
		_wvInfo.samplesPerSec = _stream->readUint32LE();
		_wvInfo.avgBytesPerSec = _stream->readUint32LE();
		_wvInfo.blockAlign = _stream->readUint16LE();
		_wvInfo.size = _stream->readUint16LE();
	}
}

bool AVIPlayer::open(Common::String filename) {
	Common::File *file = new Common::File();

	if (!file->open(filename.c_str()))
		return false;
		
	open(file);
	
	return true;
}

void AVIPlayer::open(Common::SeekableReadStream *stream) {
	close();

	assert(stream);
	_stream = stream;
	
	_decodedHeader = false;
	_curFrame = 0;
	
	// Read chunks until we have decoded the header
	while (!_decodedHeader)
		runHandle(_stream->readUint32BE());

	uint32 nextTag = _stream->readUint32BE();
	
	// Throw out any JUNK section
	if (nextTag == ID_JUNK) {
		runHandle(ID_JUNK);
		nextTag = _stream->readUint32BE();
	}
	
	// Ignore the 'movi' LIST
	if (nextTag == ID_LIST) {
		_stream->readUint32BE(); // Skip size
		if (_stream->readUint32BE() != ID_MOVI)
			error ("Expected 'movi' LIST");
	} else
		error ("Expected 'movi' LIST");
		
	// Now, create the codec
	_videoCodec = createCodec();
	
	// Initialize the video stuff too
	_audStream = createAudioStream();
	if (_audStream)
		_system->getMixer()->playInputStream(Audio::Mixer::kPlainSoundType, _audHandle, _audStream);
			
	debug (0, "Frames = %d, Dimensions = %d x %d", _header.totalFrames, _header.width, _header.height);
	debug (0, "Frame Rate = %d", getFrameRate());
	if (_header.flags & AVIF_ISINTERLEAVED)
		debug (0, "Sound Rate = %d", AUDIO_RATE);
	debug (0, "Video Codec = \'%s\'", tag2str(_vidsHeader.streamHandler));
}

void AVIPlayer::close() {
	delete _stream;
		
	// Deinitialize sound
	_system->getMixer()->stopHandle(*_audHandle);
		
	if (_palette) {
		free(_palette);
		_palette = NULL;
	}
	
	_decodedHeader = false;
	_filesize = 0;
	
	delete _videoCodec;
	delete[] _ixInfo.indices;
}
	
Surface *AVIPlayer::getNextFrame() {
	uint32 nextTag = _stream->readUint32BE();
		
	if (nextTag == ID_LIST) {
		// A list of audio/video chunks
		uint32 listSize = _stream->readUint32LE() - 4;
		int32 startPos = _stream->pos();

		if (_stream->readUint32BE() != ID_REC)
			error ("Expected 'rec ' LIST");
		
		// Decode chunks in the list and see if we get a frame
		Surface *frame = NULL;
		while (_stream->pos() < startPos + (int32)listSize) {
			Surface *temp = getNextFrame();
			if (temp)
				frame = temp;
		}

		return frame;
	} else if (getStreamType(nextTag) == 'wb') {
		// Audio Chunk
		uint32 chunkSize = _stream->readUint32LE();
		byte *data = new byte[chunkSize];
		_stream->read(data, chunkSize);
		_audStream->queueBuffer(data, chunkSize);
		_stream->skip(chunkSize & 1); // Alignment
	} else if (getStreamType(nextTag) == 'dc' || getStreamType(nextTag) == 'id') {
		// Compressed Frame
		_curFrame++;
		uint32 chunkSize = _stream->readUint32LE();

		if (chunkSize == 0) // Keep last frame on screen
			return NULL;
		
		Common::SeekableReadStream *frameData = _stream->readStream(chunkSize);
		Graphics::Surface *surface = _videoCodec->decodeImage(frameData);
		delete frameData;
		_stream->skip(chunkSize & 1); // Alignment
		return surface;
	} else if (getStreamType(nextTag) == 'pc') {
		// Palette Change
		_stream->readUint32LE(); // Chunk size, not needed here
		byte firstEntry = _stream->readByte();
		uint16 numEntries = _stream->readByte();
		_stream->readUint16LE(); // Reserved
		
		// 0 entries means all colors are going to be changed
		if (numEntries == 0)
			numEntries = 256;
			
		for (uint16 i = firstEntry; i < numEntries + firstEntry; i++) {
			_palette[i * 4] = _stream->readByte();
			_palette[i * 4 + 1] = _stream->readByte();
			_palette[i * 4 + 2] = _stream->readByte();
			_stream->readByte(); // Flags that don't serve us any purpose
		}
		
		// Mark the palette as dirty
		_dirtyPalette = true;

		// No alignment necessary. It's always even.
	} else if (nextTag == ID_JUNK) {
		runHandle(ID_JUNK);
	} else
		error ("Tag = \'%s\'", tag2str(nextTag));

	return NULL;
}

Codec *AVIPlayer::createCodec() {
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

Audio::AppendableAudioStream *AVIPlayer::createAudioStream() {
	if (_wvInfo.tag == WAVE_FORMAT_PCM)
		return Audio::makeAppendableAudioStream(AUDIO_RATE, Audio::Mixer::FLAG_UNSIGNED|Audio::Mixer::FLAG_AUTOFREE);
	
	if (_wvInfo.tag != 0) // No sound
		warning ("Unsupported AVI audio format %d", _wvInfo.tag);

	return NULL;
}
	
byte AVIPlayer::char2num(char c) {
	return (c >= 48 && c <= 57) ? c - 48 : 0;
}
	
byte AVIPlayer::getStreamNum(uint32 tag) {
	return char2num((char)(tag >> 24)) * 16 + char2num((char)(tag >> 16));
}

uint16 AVIPlayer::getStreamType(uint32 tag) {
	return tag & 0xffff;
}

} // End of namespace JMP
