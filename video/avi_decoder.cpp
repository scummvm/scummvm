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
#include "image/codecs/codec.h"

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
#define ID_JUNQ MKTAG('J','U','N','Q')
#define ID_DMLH MKTAG('d','m','l','h')
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
#define ID_STRN MKTAG('s','t','r','n')

// Stream Types
enum {
	kStreamTypePaletteChange = MKTAG16('p', 'c'),
	kStreamTypeAudio         = MKTAG16('w', 'b')
};


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

AVIDecoder::AVIAudioTrack *AVIDecoder::createAudioTrack(AVIStreamHeader sHeader, PCMWaveFormat wvInfo) {
	return new AVIAudioTrack(sHeader, wvInfo, _soundType);
}

void AVIDecoder::initCommon() {
	_decodedHeader = false;
	_foundMovieList = false;
	_movieListStart = 0;
	_movieListEnd = 0;
	_fileStream = 0;
	memset(&_header, 0, sizeof(_header));
}

bool AVIDecoder::isSeekable() const {
	// Only videos with an index can seek
	// Anyone else who wants to seek is crazy.
	return isVideoLoaded() && !_indexEntries.empty();
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
	case ID_JUNQ: // Same as JUNK, safe to ignore
	case ID_ISFT: // Metadata, safe to ignore
	case ID_DISP: // Metadata, should be safe to ignore
	case ID_STRN: // Metadata, safe to ignore
	case ID_DMLH: // OpenDML extension, contains an extra total frames field, safe to ignore
		skipChunk(size);
		break;
	case ID_IDX1:
		readOldIndex(size);		
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
		_movieListEnd = _movieListStart + listSize + (listSize & 1);
		_fileStream->skip(listSize);
		return;
	case ID_HDRL: // Header List
		// Mark the header as decoded
		_decodedHeader = true;
		break;
	case ID_INFO: // Metadata
	case ID_PRMI: // Adobe Premiere metadata, safe to ignore
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

		byte *initialPalette = 0;

		if (bmInfo.bitCount == 8) {
			initialPalette = new byte[256 * 3];
			memset(initialPalette, 0, 256 * 3);

			byte *palette = initialPalette;
			for (uint32 i = 0; i < bmInfo.clrUsed; i++) {
				palette[i * 3 + 2] = _fileStream->readByte();
				palette[i * 3 + 1] = _fileStream->readByte();
				palette[i * 3] = _fileStream->readByte();
				_fileStream->readByte();
			}
		}

		addTrack(new AVIVideoTrack(_header.totalFrames, sHeader, bmInfo, initialPalette));
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

		addTrack(createAudioTrack(sHeader, wvInfo));
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

	// Check if this is a special Duck Truemotion video
	checkTruemotion1();

	return true;
}

void AVIDecoder::close() {
	VideoDecoder::close();

	delete _fileStream;
	_fileStream = 0;
	_decodedHeader = false;
	_foundMovieList = false;
	_movieListStart = 0;
	_movieListEnd = 0;

	_indexEntries.clear();
	memset(&_header, 0, sizeof(_header));
}

void AVIDecoder::readNextPacket() {
	if ((uint32)_fileStream->pos() >= _movieListEnd) {
		// Ugh, reached the end premature.
		forceVideoEnd();
		return;
	}

	uint32 nextTag = _fileStream->readUint32BE();
	uint32 size = _fileStream->readUint32LE();

	if (_fileStream->eos()) {
		// Also premature end.
		forceVideoEnd();
		return;
	}

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
		if (getStreamType(nextTag) != kStreamTypeAudio)
			error("Invalid audio track tag '%s'", tag2str(nextTag));

		assert(chunk);
		((AVIAudioTrack *)track)->queueSound(chunk);
	} else {
		AVIVideoTrack *videoTrack = (AVIVideoTrack *)track;

		if (getStreamType(nextTag) == kStreamTypePaletteChange) {
			// Palette Change
			videoTrack->loadPaletteFromChunk(chunk);
		} else {
			// Otherwise, assume it's a compressed frame
			videoTrack->decodeFrame(chunk);
		}
	}
}

bool AVIDecoder::rewind() {
	if (!VideoDecoder::rewind())
		return false;

	_fileStream->seek(_movieListStart);
	return true;
}

bool AVIDecoder::seekIntern(const Audio::Timestamp &time) {
	// Can't seek beyond the end
	if (time > getDuration())
		return false;

	// Track down our video track.
	// We only support seeking with one video track right now.
	AVIVideoTrack *videoTrack = 0;
	int videoIndex = -1;
	uint trackID = 0;

	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++, trackID++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo) {
			if (videoTrack) {
				// Already have one
				// -> Not supported
				return false;
			}

			videoTrack = (AVIVideoTrack *)*it;
			videoIndex = trackID;
		}
	}

	// Need a video track to go forwards
	// If there isn't a video track, why would anyone be using AVI then?
	if (!videoTrack)
		return false;

	// If we seek directly to the end, just mark the tracks as over
	if (time == getDuration()) {
		videoTrack->setCurFrame(videoTrack->getFrameCount() - 1);

		for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
			if ((*it)->getTrackType() == Track::kTrackTypeAudio)
				((AVIAudioTrack *)*it)->resetStream();

		return true;
	}

	// Get the frame we should be on at this time
	uint frame = videoTrack->getFrameAtTime(time);

	// Reset any palette, if necessary
	videoTrack->useInitialPalette();

	int lastKeyFrame = -1;
	int frameIndex = -1;
	int lastRecord = -1;
	uint curFrame = 0;

	// Go through and figure out where we should be
	// If there's a palette, we need to find the palette too
	for (uint32 i = 0; i < _indexEntries.size(); i++) {
		const OldIndex &index = _indexEntries[i];

		if (index.id == ID_REC) {
			// Keep track of any records we find
			lastRecord = i;
		} else {
			if (getStreamIndex(index.id) != videoIndex)
				continue;

			uint16 streamType = getStreamType(index.id);

			if (streamType == kStreamTypePaletteChange) {
				// We need to handle any palette change we see since there's no
				// flag to tell if this is a "key" palette.
				// Decode the palette
				_fileStream->seek(_indexEntries[i].offset + 8);
				Common::SeekableReadStream *chunk = 0;

				if (_indexEntries[i].size != 0)
					chunk = _fileStream->readStream(_indexEntries[i].size);

				videoTrack->loadPaletteFromChunk(chunk);
			} else {
				// Check to see if this is a keyframe
				// The first frame has to be a keyframe
				if ((_indexEntries[i].flags & AVIIF_INDEX) || curFrame == 0)
					lastKeyFrame = i;

				// Did we find the target frame?
				if (frame == curFrame) {
					frameIndex = i;
					break;
				}

				curFrame++;
			}
		}
	}

	if (frameIndex < 0) // This shouldn't happen.
		return false;

	// Update all the audio tracks
	uint audioIndex = 0;

	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++, audioIndex++) {
		if ((*it)->getTrackType() != Track::kTrackTypeAudio)
			continue;

		AVIAudioTrack *audioTrack = (AVIAudioTrack *)*it;

		// We need to find where the start of audio should be.
		// Which is exactly 'initialFrames' audio chunks back from where
		// our found frame is.

		// Recreate the audio stream
		audioTrack->resetStream();

		uint framesNeeded = _header.initialFrames;
		uint startAudioChunk = 0;
		int startAudioSearch = (lastRecord < 0) ? (frameIndex - 1) : (lastRecord - 1);

		for (int i = startAudioSearch; i >= 0; i--) {
			if (getStreamIndex(_indexEntries[i].id) != audioIndex)
				continue;

			assert(getStreamType(_indexEntries[i].id) == kStreamTypeAudio);

			framesNeeded--;

			if (framesNeeded == 0) {
				startAudioChunk = i;
				break;
			}
		}

		// Now go forward and queue them all
		for (int i = startAudioChunk; i <= startAudioSearch; i++) {
			if (_indexEntries[i].id == ID_REC)
				continue;

			if (getStreamIndex(_indexEntries[i].id) != audioIndex)
				continue;

			assert(getStreamType(_indexEntries[i].id) == kStreamTypeAudio);

			_fileStream->seek(_indexEntries[i].offset + 8);
			Common::SeekableReadStream *chunk = _fileStream->readStream(_indexEntries[i].size);
			audioTrack->queueSound(chunk);
		}

		// Skip any audio to bring us to the right time
		audioTrack->skipAudio(time, videoTrack->getFrameTime(frame));
	}

	// Decode from keyFrame to curFrame - 1
	for (int i = lastKeyFrame; i < frameIndex; i++) {
		if (_indexEntries[i].id == ID_REC)
			continue;

		if (getStreamIndex(_indexEntries[i].id) != videoIndex)
			continue;

		uint16 streamType = getStreamType(_indexEntries[i].id);

		// Ignore palettes, they were already handled
		if (streamType == kStreamTypePaletteChange)
			continue;

		// Frame, hopefully
		_fileStream->seek(_indexEntries[i].offset + 8);
		Common::SeekableReadStream *chunk = 0;

		if (_indexEntries[i].size != 0)
			chunk = _fileStream->readStream(_indexEntries[i].size);

		videoTrack->decodeFrame(chunk);
	}

	// Seek to the right spot
	// To the beginning of the last record, or frame if that doesn't exist
	if (lastRecord >= 0)
		_fileStream->seek(_indexEntries[lastRecord].offset);
	else
		_fileStream->seek(_indexEntries[frameIndex].offset);

	videoTrack->setCurFrame((int)frame - 1);

	return true;
}

byte AVIDecoder::getStreamIndex(uint32 tag) const {
	char string[3];
	WRITE_BE_UINT16(string, tag >> 16);
	string[2] = 0;
	return strtol(string, 0, 16);
}

void AVIDecoder::readOldIndex(uint32 size) {
	uint32 entryCount = size / 16;

	debug(0, "Old Index: %d entries", entryCount);

	if (entryCount == 0)
		return;

	// Read the first index separately
	OldIndex firstEntry;
	firstEntry.id = _fileStream->readUint32BE();
	firstEntry.flags = _fileStream->readUint32LE();
	firstEntry.offset = _fileStream->readUint32LE(); 
	firstEntry.size = _fileStream->readUint32LE();

	// Check if the offset is already absolute
	// If it's absolute, the offset will equal the start of the movie list
	bool isAbsolute = firstEntry.offset == _movieListStart;

	debug(1, "Old index is %s", isAbsolute ? "absolute" : "relative");

	if (!isAbsolute)
		firstEntry.offset += _movieListStart - 4;

	debug(0, "Index 0: Tag '%s', Offset = %d, Size = %d (Flags = %d)", tag2str(firstEntry.id), firstEntry.offset, firstEntry.size, firstEntry.flags);
	_indexEntries.push_back(firstEntry);

	for (uint32 i = 1; i < entryCount; i++) {
		OldIndex indexEntry;
		indexEntry.id = _fileStream->readUint32BE();
		indexEntry.flags = _fileStream->readUint32LE();
		indexEntry.offset = _fileStream->readUint32LE();
		indexEntry.size = _fileStream->readUint32LE();

		// Adjust to absolute, if necessary
		if (!isAbsolute)
			indexEntry.offset += _movieListStart - 4;

		_indexEntries.push_back(indexEntry);
		debug(0, "Index %d: Tag '%s', Offset = %d, Size = %d (Flags = %d)", i, tag2str(indexEntry.id), indexEntry.offset, indexEntry.size, indexEntry.flags);
	}
}

void AVIDecoder::forceVideoEnd() {
	// Horrible AVI video has a premature end
	// Force the frame to be the last frame
	debug(0, "Forcing end of AVI video");

	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			((AVIVideoTrack *)*it)->forceTrackEnd();
}

void AVIDecoder::checkTruemotion1() {
	AVIVideoTrack *track = 0;

	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo) {
			if (track) {
				// Multiple tracks; isn't going to be truemotion 1
				return;
			}

			track = (AVIVideoTrack *)*it;
		}
	}

	// No track found?
	if (!track)
		return;

	// Ignore non-truemotion tracks
	if (!track->isTruemotion1())
		return;

	// Search for a non-empty frame
	const Graphics::Surface *frame = 0;
	for (int i = 0; i < 10 && !frame; i++)
		frame = decodeNextFrame();

	if (!frame) {
		// Probably shouldn't happen
		rewind();
		return;
	}

	// Fill in the width/height based on the frame's width/height
	_header.width = frame->w;
	_header.height = frame->h;
	track->forceDimensions(frame->w, frame->h);

	// Rewind us back to the beginning
	rewind();
}

VideoDecoder::AudioTrack *AVIDecoder::getAudioTrack(int index) {
	// AVI audio track indexes are relative to the first track
	Track *track = getTrack(index);

	if (!track || track->getTrackType() != Track::kTrackTypeAudio)
		return 0;

	return (AudioTrack *)track;
}

AVIDecoder::AVIVideoTrack::AVIVideoTrack(int frameCount, const AVIStreamHeader &streamHeader, const BitmapInfoHeader &bitmapInfoHeader, byte *initialPalette)
		: _frameCount(frameCount), _vidsHeader(streamHeader), _bmInfo(bitmapInfoHeader), _initialPalette(initialPalette) {
	_videoCodec = createCodec();
	_lastFrame = 0;
	_curFrame = -1;

	useInitialPalette();
}

AVIDecoder::AVIVideoTrack::~AVIVideoTrack() {
	delete _videoCodec;
	delete[] _initialPalette;
}

void AVIDecoder::AVIVideoTrack::decodeFrame(Common::SeekableReadStream *stream) {
	if (stream) {
		if (_videoCodec)
			_lastFrame = _videoCodec->decodeFrame(*stream);
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

void AVIDecoder::AVIVideoTrack::loadPaletteFromChunk(Common::SeekableReadStream *chunk) {
	assert(chunk);
	byte firstEntry = chunk->readByte();
	uint16 numEntries = chunk->readByte();
	chunk->readUint16LE(); // Reserved

	// 0 entries means all colors are going to be changed
	if (numEntries == 0)
		numEntries = 256;

	for (uint16 i = firstEntry; i < numEntries + firstEntry; i++) {
		_palette[i * 3] = chunk->readByte();
		_palette[i * 3 + 1] = chunk->readByte();
		_palette[i * 3 + 2] = chunk->readByte();
		chunk->readByte(); // Flags that don't serve us any purpose
	}

	delete chunk;
	_dirtyPalette = true;
}

void AVIDecoder::AVIVideoTrack::useInitialPalette() {
	_dirtyPalette = false;

	if (_initialPalette) {
		memcpy(_palette, _initialPalette, sizeof(_palette));
		_dirtyPalette = true;
	}
}

bool AVIDecoder::AVIVideoTrack::isTruemotion1() const {
	return _bmInfo.compression == MKTAG('D', 'U', 'C', 'K') || _bmInfo.compression == MKTAG('d', 'u', 'c', 'k');
}

void AVIDecoder::AVIVideoTrack::forceDimensions(uint16 width, uint16 height) {
	_bmInfo.width = width;
	_bmInfo.height = height;
}

bool AVIDecoder::AVIVideoTrack::rewind() {
	_curFrame = -1;

	useInitialPalette();

	delete _videoCodec;
	_videoCodec = createCodec();
	_lastFrame = 0;
	return true;
}

Image::Codec *AVIDecoder::AVIVideoTrack::createCodec() {
	return Image::createBitmapCodec(_bmInfo.compression, _bmInfo.width, _bmInfo.height, _bmInfo.bitCount);
}

void AVIDecoder::AVIVideoTrack::forceTrackEnd() {
	_curFrame = _frameCount - 1;
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
		} else if (_wvInfo.tag == kWaveFormatMP3) {
			warning("AVI: MP3 audio stream is not supported");
		}
	} else {
		delete stream;
	}
}

void AVIDecoder::AVIAudioTrack::skipAudio(const Audio::Timestamp &time, const Audio::Timestamp &frameTime) {
	Audio::Timestamp timeDiff = time.convertToFramerate(_wvInfo.samplesPerSec) - frameTime.convertToFramerate(_wvInfo.samplesPerSec);
	int skipFrames = timeDiff.totalNumberOfFrames();

	if (skipFrames <= 0)
		return;

	if (_audStream->isStereo())
		skipFrames *= 2;

	int16 *tempBuffer = new int16[skipFrames];
	_audStream->readBuffer(tempBuffer, skipFrames);
	delete[] tempBuffer;
}

void AVIDecoder::AVIAudioTrack::resetStream() {
	delete _audStream;
	_audStream = createAudioStream();
}

bool AVIDecoder::AVIAudioTrack::rewind() {
	resetStream();
	return true;
}

Audio::AudioStream *AVIDecoder::AVIAudioTrack::getAudioStream() const {
	return _audStream;
}

Audio::QueuingAudioStream *AVIDecoder::AVIAudioTrack::createAudioStream() {
	if (_wvInfo.tag == kWaveFormatPCM || _wvInfo.tag == kWaveFormatMSADPCM || _wvInfo.tag == kWaveFormatMSIMAADPCM || _wvInfo.tag == kWaveFormatDK3 || _wvInfo.tag == kWaveFormatMP3)
		return Audio::makeQueuingAudioStream(_wvInfo.samplesPerSec, _wvInfo.channels == 2);
	else if (_wvInfo.tag != kWaveFormatNone) // No sound
		warning("Unsupported AVI audio format %d", _wvInfo.tag);

	return 0;
}

} // End of namespace Video
