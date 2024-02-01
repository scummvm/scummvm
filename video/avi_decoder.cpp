/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/palette.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "video/avi_decoder.h"

// Audio Codecs
#include "audio/decoders/wave_types.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/xan_dpcm.h"

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
#define ID_INDX MKTAG('i','n','d','x')
#define ID_INDX2 MKTAG('I','N','D','X')
#define ID__PC_ MKTAG('_','P','C','_')
#define ID_PAL8 MKTAG('P','A','L','8')
#define ID_BEST MKTAG('B','E','S','T')
#define ID_SHOT MKTAG('S','H','O','T')

// Stream Types
enum {
	kStreamTypePaletteChange = MKTAG16('p', 'c'),
	kStreamTypeAudio         = MKTAG16('w', 'b')
};


AVIDecoder::AVIDecoder() :
		_frameRateOverride(0) {
	initCommon();
}

AVIDecoder::AVIDecoder(const Common::Rational &frameRateOverride) :
		_frameRateOverride(frameRateOverride) {
	initCommon();
}

AVIDecoder::~AVIDecoder() {
	close();
}

AVIDecoder::AVIAudioTrack *AVIDecoder::createAudioTrack(AVIStreamHeader sHeader, PCMWaveFormat wvInfo) {
	return new AVIAudioTrack(sHeader, wvInfo, getSoundType());
}

bool AVIDecoder::seekToFrame(uint frame) {
	if (!isSeekable())
		return false;

	// If we didn't find a video track, we can't seek by frame (of course)
	if (_videoTracks.empty())
		return false;

	AVIVideoTrack *track = static_cast<AVIVideoTrack *>(_videoTracks.front().track);
	Audio::Timestamp time = track->getFrameTime(frame);

	if (time < 0)
		return false;

	return seek(time);
}

void AVIDecoder::initCommon() {
	_decodedHeader = false;
	_foundMovieList = false;
	_movieListStart = 0;
	_movieListEnd = 0;
	_fileStream = 0;
	_videoTrackCounter = _audioTrackCounter = 0;
	_lastAddedTrack = nullptr;
	memset(&_header, 0, sizeof(_header));
	_transparencyTrack.track = nullptr;
}

bool AVIDecoder::isSeekable() const {
	// Only videos with an index can seek
	// Anyone else who wants to seek is crazy.
	return isVideoLoaded() && !_indexEntries.empty();
}

const Graphics::Surface *AVIDecoder::decodeNextFrame() {
	AVIVideoTrack *track = nullptr;
	bool isReversed = false;
	int frameNum = 0;

	// Check whether the video is playing in revese
	for (int idx = _videoTracks.size() - 1; idx >= 0; --idx) {
		track = static_cast<AVIVideoTrack *>(_videoTracks[idx].track);
		isReversed |= track->isReversed();
	}

	if (isReversed) {
		// For reverse mode we need to keep seeking to just before the
		// desired frame prior to actually decoding a frame
		frameNum = getCurFrame();
		seekIntern(track->getFrameTime(frameNum));
	}

	// Decode the next frame
	const Graphics::Surface *frame = VideoDecoder::decodeNextFrame();

	if (isReversed) {
		// In reverse mode, set next frame to be the prior frame number
		for (int idx = _videoTracks.size() - 1; idx >= 0; --idx) {
			track = static_cast<AVIVideoTrack *>(_videoTracks[idx].track);
			track->setCurFrame(frameNum - 1);
			findNextVideoTrack();
		}
	}

	return frame;
}

const Graphics::Surface *AVIDecoder::decodeNextTransparency() {
	if (!_transparencyTrack.track)
		return nullptr;

	AVIVideoTrack *track = static_cast<AVIVideoTrack *>(_transparencyTrack.track);
	return track->decodeNextFrame();
}

bool AVIDecoder::parseNextChunk() {
	uint32 tag = _fileStream->readUint32BE();
	uint32 size = _fileStream->readUint32LE();

	if (_fileStream->eos())
		return false;

	debug(6, "Decoding tag %s", tag2str(tag));

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
	case ID_HDRL: // Header list.. what's it doing here? Probably ok to ignore?
	case ID_STRD: // Extra stream info, safe to ignore
	case ID_VEDT: // Unknown, safe to ignore
	case ID_JUNK: // Alignment bytes, should be ignored
	case ID_JUNQ: // Same as JUNK, safe to ignore
	case ID_ISFT: // Metadata, safe to ignore
	case ID_DISP: // Metadata, should be safe to ignore
	case ID_DMLH: // OpenDML extension, contains an extra total frames field, safe to ignore
	case ID_INDX: // OpenDML extension, contains another type of index
	case ID__PC_: // block in Origin Systems Xxan videos
	case ID_SHOT: // block in Origin Systems Xxan videos
	case ID_BEST: // block in Origin Systems Xxan videos
	case ID_INDX2: // block in Origin Systems Xxan videos
		skipChunk(size);
		break;
	case ID_PAL8: // PAL8 block in Origin Systems Xxan videos
		readPalette8(size);
		break;
	case ID_STRN:
		readStreamName(size);
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

	debug(7, "Found LIST of type %s", tag2str(listType));

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

	if (sHeader.streamType == ID_MIDS)
		error("Unhandled MIDI/Text stream");

	if (sHeader.streamType == ID_TXTS)
		warning("Unsupported Text stream detected");

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

		Graphics::Palette *initialPalette = nullptr;

		if (bmInfo.bitCount == 8) {
			initialPalette = new Graphics::Palette(256);

			Graphics::Palette *palette = initialPalette;
			for (uint32 i = 0; i < bmInfo.clrUsed; i++) {
				palette->data[i * 3 + 2] = _fileStream->readByte();
				palette->data[i * 3 + 1] = _fileStream->readByte();
				palette->data[i * 3] = _fileStream->readByte();
				_fileStream->readByte();
			}
		}

		AVIVideoTrack *track = new AVIVideoTrack(_header.totalFrames, sHeader, bmInfo, initialPalette);
		if (track->isValid())
			addTrack(track);
		else
			delete track;
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

		AVIAudioTrack *track = createAudioTrack(sHeader, wvInfo);
		track->createAudioStream();
		addTrack(track);
	}

	// Ensure that we're at the end of the chunk
	_fileStream->seek(startPos + strfSize);
}

void AVIDecoder::addTrack(Track *track, bool isExternal) {
	VideoDecoder::addTrack(track, isExternal);
	_lastAddedTrack = track;
}

void AVIDecoder::readStreamName(uint32 size) {
	if (!_lastAddedTrack) {
		skipChunk(size);
	} else {
		// Get in the name
		assert(size > 0 && size < 128);
		char buffer[128];
		_fileStream->read(buffer, size);
		if (size & 1)
			_fileStream->skip(1);

		// Apply it to the most recently read stream
		assert(_lastAddedTrack);
		AVIVideoTrack *vidTrack = dynamic_cast<AVIVideoTrack *>(_lastAddedTrack);
		AVIAudioTrack *audTrack = dynamic_cast<AVIAudioTrack *>(_lastAddedTrack);
		if (vidTrack)
			vidTrack->getName() = Common::String(buffer);
		else if (audTrack)
			audTrack->getName() = Common::String(buffer);
	}
}

void AVIDecoder::readPalette8(uint32 size) {
	if (size < 768) {
		warning("AVI palette8 is too small (%d, expected >= 768)", size);
		skipChunk(size);
		return;
	}

	// Should also be able to load 768 byte palette8 entries here.
	// Not supported at the moment.
	if (!_lastAddedTrack || size != 1024) {
		skipChunk(size);
	} else {
		AVIVideoTrack *vidTrack = dynamic_cast<AVIVideoTrack *>(_lastAddedTrack);
		if (vidTrack) {
			vidTrack->loadPaletteFromChunkRaw(_fileStream, 0, 256);
		} else {
			skipChunk(size);
			warning("unexpected palette8 on a non-video track");
		}
	}
}

bool AVIDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	uint32 riffTag = stream->readUint32BE();
	if (riffTag != ID_RIFF) {
		warning("Failed to find RIFF header");
		return false;
	}

	int32 fileSize = stream->readUint32LE();
	uint32 riffType = stream->readUint32BE();

	if (riffType != ID_AVI) {
		warning("RIFF not an AVI file");
		return false;
	}

	_fileStream = stream;

	// Go through all chunks in the file
	while (_fileStream->pos() < fileSize && parseNextChunk())
		;

	if (_decodedHeader) {
		// Ensure there's at least a supported video track
		_decodedHeader = findNextVideoTrack() != nullptr;
	}

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

	// Create the status entries
	uint32 index = 0;
	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++, index++) {
		TrackStatus status;
		status.track = *it;
		status.index = index;
		status.chunkSearchOffset = _movieListStart;

		if ((*it)->getTrackType() == Track::kTrackTypeAudio) {
			_audioTracks.push_back(status);
		} else if (_videoTracks.empty()) {
			_videoTracks.push_back(status);
		} else {
			// Secondary video track. For now we assume it will always be a
			// transparency information track
			status.chunkSearchOffset = getVideoTrackOffset(index);
			assert(!_transparencyTrack.track);
			assert(status.chunkSearchOffset != 0);

			// Copy the track status information into the transparency track field
			_transparencyTrack = status;
		}
	}

	// If there is a transparency track, remove it from the video decoder's track list.
	// This is to stop it being included in calls like getFrameCount
	if (_transparencyTrack.track)
		eraseTrack(_transparencyTrack.track);

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

	_videoTracks.clear();
	_audioTracks.clear();

	delete _transparencyTrack.track;
	_transparencyTrack.track = nullptr;
}

void AVIDecoder::readNextPacket() {
	// Shouldn't get this unless called on a non-open video
	if (_videoTracks.empty())
		return;

	// Handle the video first
	for (uint idx = 0; idx < _videoTracks.size(); ++idx)
		handleNextPacket(_videoTracks[idx]);

	// Handle any transparency track
	if (_transparencyTrack.track)
		handleNextPacket(_transparencyTrack);

	// Handle audio tracks next
	for (uint idx = 0; idx < _audioTracks.size(); ++idx)
		handleNextPacket(_audioTracks[idx]);
}

void AVIDecoder::handleNextPacket(TrackStatus &status) {
	// If there's no more to search, bail out
	if (status.chunkSearchOffset + 8 >= _movieListEnd) {
		if (status.track->getTrackType() == Track::kTrackTypeVideo) {
			// Horrible AVI video has a premature end
			// Force the frame to be the last frame
			debug(7, "Forcing end of AVI video");
			((AVIVideoTrack *)status.track)->forceTrackEnd();
		}

		return;
	}

	// See if audio needs to be buffered and break out if not
	if (status.track->getTrackType() == Track::kTrackTypeAudio && !shouldQueueAudio(status))
		return;

	// Seek to where we shall start searching
	_fileStream->seek(status.chunkSearchOffset);
	bool isReversed = false;
	AVIVideoTrack *videoTrack = nullptr;

	for (;;) {
		// If there's no more to search, bail out
		if ((uint32)_fileStream->pos() + 8 >= _movieListEnd) {
			if (status.track->getTrackType() == Track::kTrackTypeVideo) {
				// Horrible AVI video has a premature end
				// Force the frame to be the last frame
				debug(7, "Forcing end of AVI video");
				((AVIVideoTrack *)status.track)->forceTrackEnd();
			}

			break;
		}

		uint32 nextTag = _fileStream->readUint32BE();
		uint32 size = _fileStream->readUint32LE();

		if (nextTag == ID_LIST) {
			// A list of audio/video chunks
			if (_fileStream->readUint32BE() != ID_REC)
				error("Expected 'rec ' LIST");

			continue;
		} else if (nextTag == ID_JUNK || nextTag == ID_IDX1) {
			skipChunk(size);
			continue;
		}

		// Only accept chunks for this stream
		uint32 streamIndex = getStreamIndex(nextTag);
		if (streamIndex != status.index) {
			skipChunk(size);
			continue;
		}

		Common::SeekableReadStream *chunk = 0;

		if (size != 0) {
			chunk = _fileStream->readStream(size);
			_fileStream->skip(size & 1);
		}

		if (status.track->getTrackType() == Track::kTrackTypeAudio) {
			if (getStreamType(nextTag) != kStreamTypeAudio)
				error("Invalid audio track tag '%s'", tag2str(nextTag));

			assert(chunk);
			((AVIAudioTrack *)status.track)->queueSound(chunk);

			// Break out if we have enough audio
			if (!shouldQueueAudio(status))
				break;
		} else {
			videoTrack = (AVIVideoTrack *)status.track;
			isReversed = videoTrack->isReversed();

			if (getStreamType(nextTag) == kStreamTypePaletteChange) {
				// Palette Change
				videoTrack->loadPaletteFromChunk(chunk);
			} else {
				// Otherwise, assume it's a compressed frame
				videoTrack->decodeFrame(chunk);
				break;
			}
		}
	}

	if (!isReversed) {
		// Start us off in this position next time
		status.chunkSearchOffset = _fileStream->pos();
	}
}

bool AVIDecoder::shouldQueueAudio(TrackStatus& status) {
	// Sanity check:
	if (status.track->getTrackType() != Track::kTrackTypeAudio)
		return false;

	// If video is done, make sure that the rest of the audio is queued
	// (I guess this is also really a sanity check)
	AVIVideoTrack *videoTrack = (AVIVideoTrack *)_videoTracks[0].track;
	if (videoTrack->endOfTrack())
		return true;

	// Being three frames ahead should be enough for any video.
	return ((AVIAudioTrack *)status.track)->getCurChunk() < (uint32)(videoTrack->getCurFrame() + 3);
}

bool AVIDecoder::rewind() {
	if (!VideoDecoder::rewind())
		return false;

	for (uint32 i = 0; i < _videoTracks.size(); i++)
		_videoTracks[i].chunkSearchOffset = getVideoTrackOffset(_videoTracks[i].index);

	for (uint32 i = 0; i < _audioTracks.size(); i++)
		_audioTracks[i].chunkSearchOffset = _movieListStart;

	return true;
}

uint AVIDecoder::getVideoTrackOffset(uint trackIndex, uint frameNumber) {
	if (trackIndex == _videoTracks.front().index && frameNumber == 0)
		return _movieListStart;

	OldIndex *entry = _indexEntries.find(trackIndex, frameNumber);
	assert(entry);
	return entry->offset;
}

bool AVIDecoder::seekIntern(const Audio::Timestamp &time) {
	uint frame;

	// Can't seek beyond the end
	if (time > getDuration())
		return false;

	// Get our video
	AVIVideoTrack *videoTrack = (AVIVideoTrack *)_videoTracks[0].track;
	uint32 videoIndex = _videoTracks[0].index;

	if (time == getDuration()) {
		videoTrack->setCurFrame(videoTrack->getFrameCount() - 1);

		if (!videoTrack->isReversed()) {
			// Since we're at the end, just mark the tracks as over
			for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
				if ((*it)->getTrackType() == Track::kTrackTypeAudio)
					((AVIAudioTrack *)*it)->resetStream();

			return true;
		}

		frame = videoTrack->getFrameCount() - 1;
	} else {
		// Get the frame we should be on at this time
		frame = videoTrack->getFrameAtTime(time);
	}

	// Reset any palette, if necessary
	videoTrack->useInitialPalette();

	int lastKeyFrame = -1;
	int frameIndex = -1;
	uint curFrame = 0;

	// Go through and figure out where we should be
	// If there's a palette, we need to find the palette too
	for (uint32 i = 0; i < _indexEntries.size(); i++) {
		const OldIndex &index = _indexEntries[i];

		// We don't care about RECs
		if (index.id == ID_REC)
			continue;

		// We're only looking at entries for this track
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

	if (frameIndex < 0) // This shouldn't happen.
		return false;

	// Update all the audio tracks
	for (uint32 i = 0; i < _audioTracks.size(); i++) {
		AVIAudioTrack *audioTrack = (AVIAudioTrack *)_audioTracks[i].track;

		// Recreate the audio stream
		audioTrack->resetStream();

		// Set the chunk index for the track
		audioTrack->setCurChunk(frame);

		uint32 chunksFound = 0;
		for (uint32 j = 0; j < _indexEntries.size(); j++) {
			const OldIndex &index = _indexEntries[j];

			// Continue ignoring RECs
			if (index.id == ID_REC)
				continue;

			if (getStreamIndex(index.id) == _audioTracks[i].index) {
				if (chunksFound == frame) {
					_fileStream->seek(index.offset + 8);
					Common::SeekableReadStream *audioChunk = _fileStream->readStream(index.size);
					audioTrack->queueSound(audioChunk);
					_audioTracks[i].chunkSearchOffset = (j == _indexEntries.size() - 1) ? _movieListEnd : _indexEntries[j + 1].offset;
					break;
				}

				chunksFound++;
			}
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

	// Update any transparency track if present
	if (_transparencyTrack.track)
		seekTransparencyFrame(frame);

	// Set the video track's frame
	videoTrack->setCurFrame(frame - 1);

	// Set the video track's search offset to the right spot
	_videoTracks[0].chunkSearchOffset = _indexEntries[frameIndex].offset;
	return true;
}

void AVIDecoder::seekTransparencyFrame(int frame) {
	TrackStatus &status = _transparencyTrack;
	AVIVideoTrack *transTrack = static_cast<AVIVideoTrack *>(status.track);

	// Find the index entry for the frame
	int indexFrame = frame;
	OldIndex *entry = nullptr;
	do {
		entry = _indexEntries.find(status.index, indexFrame);
	} while (!entry && indexFrame-- > 0);
	assert(entry);

	// Set it's frame number
	transTrack->setCurFrame(indexFrame - 1);

	// Read in the frame
	Common::SeekableReadStream *chunk = nullptr;
	_fileStream->seek(entry->offset + 8);
	status.chunkSearchOffset = entry->offset;

	if (entry->size != 0)
		chunk = _fileStream->readStream(entry->size);
	transTrack->decodeFrame(chunk);

	if (indexFrame < (int)frame) {
		while (status.chunkSearchOffset < _movieListEnd && indexFrame++ < (int)frame) {
			// There was no index entry for the desired frame, so an earlier one was decoded.
			// We now have to sequentially skip frames until we get to the desired frame
			_fileStream->readUint32BE();
			uint32 size = _fileStream->readUint32LE() - 8;
			_fileStream->skip(size & 1);
			status.chunkSearchOffset = _fileStream->pos();
		}
	}

	transTrack->setCurFrame(frame - 1);
}

byte AVIDecoder::getStreamIndex(uint32 tag) {
	char string[3];
	WRITE_BE_UINT16(string, tag >> 16);
	string[2] = 0;
	return strtol(string, 0, 16);
}

void AVIDecoder::readOldIndex(uint32 size) {
	uint32 entryCount = size / 16;

	debug(7, "Old Index: %d entries", entryCount);

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

	debug(6, "Old index is %s", isAbsolute ? "absolute" : "relative");

	if (!isAbsolute)
		firstEntry.offset += _movieListStart - 4;

	debug(7, "Index 0: Tag '%s', Offset = %d, Size = %d (Flags = %d)", tag2str(firstEntry.id), firstEntry.offset, firstEntry.size, firstEntry.flags);
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
		debug(7, "Index %d: Tag '%s', Offset = %d, Size = %d (Flags = %d)", i, tag2str(indexEntry.id), indexEntry.offset, indexEntry.size, indexEntry.flags);
	}
}

void AVIDecoder::checkTruemotion1() {
	// If we got here from loadStream(), we know the track is valid
	assert(!_videoTracks.empty());

	TrackStatus &status = _videoTracks[0];
	AVIVideoTrack *track = (AVIVideoTrack *)status.track;

	// Ignore non-truemotion tracks
	if (!track->isTruemotion1())
		return;

	// Read the next video packet
	handleNextPacket(status);

	const Graphics::Surface *frame = track->decodeNextFrame();
	if (!frame) {
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

AVIDecoder::AVIVideoTrack::AVIVideoTrack(int frameCount, const AVIStreamHeader &streamHeader, const BitmapInfoHeader &bitmapInfoHeader, Graphics::Palette *initialPalette)
		: _frameCount(frameCount), _vidsHeader(streamHeader), _bmInfo(bitmapInfoHeader), _initialPalette(initialPalette) {
	_videoCodec = createCodec();
	_lastFrame = 0;
	_curFrame = -1;
	_reversed = false;
	_palette = new Graphics::Palette(256);

	useInitialPalette();
}

AVIDecoder::AVIVideoTrack::~AVIVideoTrack() {
	delete _videoCodec;
	delete _initialPalette;
	delete _palette;
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

	if (!_reversed) {
		_curFrame++;
	} else {
		_curFrame--;
	}
}

Graphics::PixelFormat AVIDecoder::AVIVideoTrack::getPixelFormat() const {
	if (_videoCodec)
		return _videoCodec->getPixelFormat();

	return Graphics::PixelFormat();
}

bool AVIDecoder::AVIVideoTrack::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (_videoCodec)
		return _videoCodec->setOutputPixelFormat(format);

	return false;
}

void AVIDecoder::AVIVideoTrack::loadPaletteFromChunkRaw(Common::SeekableReadStream *chunk, int firstEntry, int numEntries) {
	assert(chunk);
	assert(firstEntry >= 0);
	assert(numEntries > 0);
	for (uint16 i = firstEntry; i < numEntries + firstEntry; i++) {
		_palette->data[i * 3] = chunk->readByte();
		_palette->data[i * 3 + 1] = chunk->readByte();
		_palette->data[i * 3 + 2] = chunk->readByte();
		chunk->readByte(); // Flags that don't serve us any purpose
	}
	_dirtyPalette = true;
}

void AVIDecoder::AVIVideoTrack::loadPaletteFromChunk(Common::SeekableReadStream *chunk) {
	assert(chunk);
	byte firstEntry = chunk->readByte();
	uint16 numEntries = chunk->readByte();
	chunk->readUint16LE(); // Reserved

	// 0 entries means all colors are going to be changed
	if (numEntries == 0)
		numEntries = 256;

	loadPaletteFromChunkRaw(chunk, firstEntry, numEntries);

	delete chunk;
}


void AVIDecoder::AVIVideoTrack::useInitialPalette() {
	_dirtyPalette = false;

	if (_initialPalette) {
		_palette->set(*_initialPalette, 0, 256);
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
	return Image::createBitmapCodec(_bmInfo.compression, _vidsHeader.streamHandler, _bmInfo.width,
									_bmInfo.height, _bmInfo.bitCount);
}

void AVIDecoder::AVIVideoTrack::forceTrackEnd() {
	_curFrame = _frameCount - 1;
}

const byte *AVIDecoder::AVIVideoTrack::getPalette() const {
	if (_videoCodec && _videoCodec->containsPalette())
		return _videoCodec->getPalette();

	_dirtyPalette = false;
	return _palette->data;
}

bool AVIDecoder::AVIVideoTrack::hasDirtyPalette() const {
	if (_videoCodec && _videoCodec->containsPalette())
		return _videoCodec->hasDirtyPalette();

	return _dirtyPalette;
}

bool AVIDecoder::AVIVideoTrack::setReverse(bool reverse) {
	if (isRewindable()) {
		// Track is rewindable, so reversing is allowed
		_reversed = reverse;
		return true;
	}

	return !reverse;
}

bool AVIDecoder::AVIVideoTrack::endOfTrack() const {
	if (_reversed)
		return _curFrame < -1;

	return _curFrame >= (getFrameCount() - 1);
}

bool AVIDecoder::AVIVideoTrack::canDither() const {
	return _videoCodec && _videoCodec->canDither(Image::Codec::kDitherTypeVFW);
}

void AVIDecoder::AVIVideoTrack::setDither(const byte *palette) {
	assert(_videoCodec);
	_videoCodec->setDither(Image::Codec::kDitherTypeVFW, palette);
}

AVIDecoder::AVIAudioTrack::AVIAudioTrack(const AVIStreamHeader &streamHeader, const PCMWaveFormat &waveFormat, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType),
		_audsHeader(streamHeader),
		_wvInfo(waveFormat),
		_audioStream(0),
		_packetStream(0),
		_curChunk(0) {
}

AVIDecoder::AVIAudioTrack::~AVIAudioTrack() {
	delete _audioStream;
}

void AVIDecoder::AVIAudioTrack::queueSound(Common::SeekableReadStream *stream) {
	if (_packetStream)
		_packetStream->queuePacket(stream);
	else
		delete stream;

	_curChunk++;
}

void AVIDecoder::AVIAudioTrack::skipAudio(const Audio::Timestamp &time, const Audio::Timestamp &frameTime) {
	Audio::Timestamp timeDiff = time.convertToFramerate(_wvInfo.samplesPerSec) - frameTime.convertToFramerate(_wvInfo.samplesPerSec);
	int skipFrames = timeDiff.totalNumberOfFrames();

	if (skipFrames <= 0)
		return;

	Audio::AudioStream *audioStream = getAudioStream();
	if (!audioStream)
		return;

	if (audioStream->isStereo())
		skipFrames *= 2;

	int16 *tempBuffer = new int16[skipFrames];
	audioStream->readBuffer(tempBuffer, skipFrames);
	delete[] tempBuffer;
}

void AVIDecoder::AVIAudioTrack::resetStream() {
	delete _audioStream;
	createAudioStream();
	_curChunk = 0;
}

bool AVIDecoder::AVIAudioTrack::rewind() {
	resetStream();
	return true;
}

void AVIDecoder::AVIAudioTrack::createAudioStream() {
	_packetStream = 0;

	switch (_wvInfo.tag) {
	case Audio::kWaveFormatPCM: {
		byte flags = 0;
		if (_audsHeader.sampleSize == 2)
			flags |= Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
		else
			flags |= Audio::FLAG_UNSIGNED;

		if (_wvInfo.channels == 2)
			flags |= Audio::FLAG_STEREO;

		_packetStream = Audio::makePacketizedRawStream(_wvInfo.samplesPerSec, flags);
		break;
	}
	case Audio::kWaveFormatXanDPCM:
		_packetStream = new Audio::XanDPCMStream(_wvInfo.samplesPerSec, _wvInfo.channels);
		break;
	case Audio::kWaveFormatMSADPCM:
		_packetStream = Audio::makePacketizedADPCMStream(Audio::kADPCMMS, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign);
		break;
	case Audio::kWaveFormatMSIMAADPCM:
		_packetStream = Audio::makePacketizedADPCMStream(Audio::kADPCMMSIma, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign);
		break;
	case Audio::kWaveFormatDK3:
		_packetStream = Audio::makePacketizedADPCMStream(Audio::kADPCMDK3, _wvInfo.samplesPerSec, _wvInfo.channels, _wvInfo.blockAlign);
		break;
	case Audio::kWaveFormatMP3:
#ifdef USE_MAD
		_packetStream = Audio::makePacketizedMP3Stream(_wvInfo.channels, _wvInfo.samplesPerSec);
#else
		warning("AVI MP3 stream found, but no libmad support compiled in");
#endif
		break;
	case Audio::kWaveFormatNone:
		break;
	default:
		warning("Unsupported AVI audio format %d", _wvInfo.tag);
		break;
	}

	if (_packetStream)
		_audioStream = _packetStream;
	else
		_audioStream = Audio::makeNullAudioStream();
}

AVIDecoder::TrackStatus::TrackStatus() : track(0), chunkSearchOffset(0) {
}

AVIDecoder::OldIndex *AVIDecoder::IndexEntries::find(uint index, uint frameNumber) {
	for (uint idx = 0, frameCtr = 0; idx < size(); ++idx) {
		if ((*this)[idx].id != ID_REC &&
				AVIDecoder::getStreamIndex((*this)[idx].id) == index) {
			if (frameCtr++ == frameNumber)
				return &(*this)[idx];
		}
	}

	return nullptr;
}

} // End of namespace Video
