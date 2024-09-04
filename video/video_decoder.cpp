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

#include "video/video_decoder.h"

#include "audio/audiostream.h"
#include "audio/mixer.h" // for kMaxChannelVolume

#include "common/rational.h"
#include "common/file.h"
#include "common/system.h"

namespace Video {

VideoDecoder::VideoDecoder() {
	_startTime = 0;
	_dirtyPalette = false;
	_palette = 0;
	_playbackRate = 0;
	_audioVolume = Audio::Mixer::kMaxChannelVolume;
	_audioBalance = 0;
	_soundType = Audio::Mixer::kPlainSoundType;
	_pauseLevel = 0;
	_needsUpdate = false;
	_lastTimeChange = 0;
	_endTime = 0;
	_endTimeSet = false;
	_nextVideoTrack = 0;
	_mainAudioTrack = 0;
	_canSetDither = true;
	_canSetDefaultFormat = true;
	_videoCodecAccuracy = Image::CodecAccuracy::Default;
}

void VideoDecoder::close() {
	if (isPlaying())
		stop();

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		delete *it;

	_tracks.clear();
	_internalTracks.clear();
	_externalTracks.clear();
	_dirtyPalette = false;
	_palette = 0;
	_startTime = 0;
	_audioVolume = Audio::Mixer::kMaxChannelVolume;
	_audioBalance = 0;
	_pauseLevel = 0;
	_needsUpdate = false;
	_lastTimeChange = 0;
	_endTime = 0;
	_endTimeSet = false;
	_nextVideoTrack = 0;
	_mainAudioTrack = 0;
	_canSetDither = true;
	_canSetDefaultFormat = true;
}

bool VideoDecoder::loadFile(const Common::Path &filename) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		delete file;
		return false;
	}

	bool result = loadStream(file);
	if (!result)
		delete file;
	return result;
}

bool VideoDecoder::needsUpdate() const {
	bool hasVideo = false;
	bool hasAudio = false;
	for (auto &it : _tracks) {
		switch (it->getTrackType()) {
		case Track::kTrackTypeAudio:
			hasAudio = true;
			break;
		case Track::kTrackTypeVideo:
			hasVideo = true;
			break;
		default:
			break;
		}
	}
	if (hasVideo) {
		return hasFramesLeft() && getTimeToNextFrame() == 0;
	} else if (hasAudio) {
		return !endOfVideo();
	}
	return false;
}

void VideoDecoder::delayMillis(uint msecs) {
	if (!needsUpdate())
		g_system->delayMillis(MIN<uint>(msecs, getTimeToNextFrame()));
	else
		g_system->delayMillis(1); /* This is needed to keep the mixer and timers active */
}

void VideoDecoder::pauseVideo(bool pause) {
	if (pause) {
		_pauseLevel++;

	// We can't go negative
	} else if (_pauseLevel) {
		_pauseLevel--;

	// Do nothing
	} else {
		return;
	}

	if (_pauseLevel == 1 && pause) {
		_pauseStartTime = g_system->getMillis(); // Store the starting time from pausing to keep it for later

		for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
			(*it)->pause(true);
	} else if (_pauseLevel == 0) {
		for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
			(*it)->pause(false);

		_startTime += (g_system->getMillis() - _pauseStartTime);
	}
}

void VideoDecoder::resetPauseStartTime() {
	if (isPaused())
		_pauseStartTime = g_system->getMillis();
}

void VideoDecoder::setVolume(byte volume) {
	_audioVolume = volume;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setVolume(_audioVolume);
}

void VideoDecoder::setBalance(int8 balance) {
	_audioBalance = balance;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setBalance(_audioBalance);
}

Audio::Mixer::SoundType VideoDecoder::getSoundType() const {
	return _soundType;
}

void VideoDecoder::setSoundType(Audio::Mixer::SoundType soundType) {
	_soundType = soundType;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setSoundType(_soundType);
}

bool VideoDecoder::isVideoLoaded() const {
	return !_tracks.empty();
}

uint16 VideoDecoder::getWidth() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return ((VideoTrack *)*it)->getWidth();

	return 0;
}

uint16 VideoDecoder::getHeight() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return ((VideoTrack *)*it)->getHeight();

	return 0;
}

Graphics::PixelFormat VideoDecoder::getPixelFormat() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return ((VideoTrack *)*it)->getPixelFormat();

	return Graphics::PixelFormat();
}

const Graphics::Surface *VideoDecoder::decodeNextFrame() {
	_needsUpdate = false;
	_canSetDither = false;
	_canSetDefaultFormat = false;

	readNextPacket();

	// If we have no next video track at this point, there shouldn't be
	// any frame available for us to display.
	if (!_nextVideoTrack)
		return 0;

	const Graphics::Surface *frame = _nextVideoTrack->decodeNextFrame();

	if (_nextVideoTrack->hasDirtyPalette()) {
		_palette = _nextVideoTrack->getPalette();
		_dirtyPalette = true;
	}

	// Look for the next video track here for the next decode.
	findNextVideoTrack();

	return frame;
}

bool VideoDecoder::setReverse(bool reverse) {
	// Can only reverse video-only videos
	if (reverse && hasAudio())
		return false;

	// Attempt to make sure all the tracks are in the requested direction
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && ((VideoTrack *)*it)->isReversed() != reverse) {
			if (!((VideoTrack *)*it)->setReverse(reverse))
				return false;

			_needsUpdate = true; // force an update
		}
	}

	findNextVideoTrack();
	return true;
}

const byte *VideoDecoder::getPalette() {
	_dirtyPalette = false;
	return _palette;
}

int VideoDecoder::getCurFrame() const {
	int32 frame = -1;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			frame += ((VideoTrack *)*it)->getCurFrame() + 1;

	return frame;
}

uint32 VideoDecoder::getFrameCount() const {
	int count = 0;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			count += ((VideoTrack *)*it)->getFrameCount();

	return count;
}

uint32 VideoDecoder::getTime() const {
	if (!isPlaying())
		return _lastTimeChange.msecs();

	if (isPaused())
		return MAX<int>((_playbackRate * (_pauseStartTime - _startTime)).toInt(), 0);

	if (useAudioSync()) {
		for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
			if ((*it)->getTrackType() == Track::kTrackTypeAudio && !(*it)->endOfTrack()) {
				uint32 time = (((const AudioTrack *)*it)->getRunningTime() * _playbackRate).toInt();

				if (time != 0)
					return time + _lastTimeChange.msecs();
			}
		}
	}

	return MAX<int>((_playbackRate * (g_system->getMillis() - _startTime)).toInt(), 0);
}

uint32 VideoDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _needsUpdate || !_nextVideoTrack)
		return 0;

	uint32 currentTime = getTime();
	uint32 nextFrameStartTime = _nextVideoTrack->getNextFrameStartTime();

	if (_nextVideoTrack->isReversed()) {
		// For reversed videos, we need to handle the time difference the opposite way.
		if (nextFrameStartTime >= currentTime)
			return 0;

		return currentTime - nextFrameStartTime;
	}

	// Otherwise, handle it normally.
	if (nextFrameStartTime <= currentTime)
		return 0;

	return nextFrameStartTime - currentTime;
}

bool VideoDecoder::endOfVideo() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		const Track *track = *it;

		bool videoEndTimeReached = _endTimeSet && track->getTrackType() == Track::kTrackTypeVideo && ((const VideoTrack *)track)->getNextFrameStartTime() >= (uint)_endTime.msecs();
		bool endReached = track->endOfTrack() || (isPlaying() && videoEndTimeReached);
		if (!endReached)
			return false;
	}

	return true;
}

bool VideoDecoder::isRewindable() const {
	if (!isVideoLoaded())
		return false;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->isRewindable())
			return false;

	return true;
}

bool VideoDecoder::rewind() {
	if (!isRewindable())
		return false;

	// Stop all tracks so they can be rewound
	if (isPlaying())
		stopAudio();

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->rewind())
			return false;

	// Now that we've rewound, start all tracks again
	if (isPlaying())
		startAudio();

	_lastTimeChange = 0;
	_startTime = g_system->getMillis();
	resetPauseStartTime();
	findNextVideoTrack();
	return true;
}

bool VideoDecoder::isSeekable() const {
	if (!isVideoLoaded())
		return false;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->isSeekable())
			return false;

	return true;
}

bool VideoDecoder::seek(const Audio::Timestamp &time) {
	if (!isSeekable())
		return false;

	// Stop all tracks so they can be seek'ed
	if (isPlaying())
		stopAudio();

	// Do the actual seeking
	if (!seekIntern(time))
		return false;

	// Seek any external track too
	for (TrackListIterator it = _externalTracks.begin(); it != _externalTracks.end(); it++)
		if (!(*it)->seek(time))
			return false;

	_lastTimeChange = time;

	// Now that we've seek'ed, start all tracks again
	// Also reset our start time
	if (isPlaying()) {
		startAudio();
		_startTime = g_system->getMillis() - (time.msecs() / _playbackRate).toInt();
	}

	resetPauseStartTime();
	findNextVideoTrack();
	_needsUpdate = true;
	return true;
}

bool VideoDecoder::seekToFrame(uint frame) {
	if (!isSeekable())
		return false;

	VideoTrack *track = 0;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo) {
			// We only allow seeking by frame when one video track
			// is present
			if (track)
				return false;

			track = (VideoTrack *)*it;
		}
	}

	// If we didn't find a video track, we can't seek by frame (of course)
	if (!track)
		return false;

	Audio::Timestamp time = track->getFrameTime(frame);

	if (time < 0)
		return false;

	return seek(time);
}

void VideoDecoder::start() {
	if (!isPlaying())
		setRate(1);
}

void VideoDecoder::stop() {
	if (!isPlaying())
		return;

	// Stop audio here so we don't have it affect getTime()
	stopAudio();

	// Keep the time marked down in case we start up again
	// We do this before _playbackRate is set so we don't get
	// _lastTimeChange returned, but before _pauseLevel is
	// reset.
	_lastTimeChange = getTime();

	_playbackRate = 0;
	_startTime = 0;
	_palette = 0;
	_dirtyPalette = false;
	_needsUpdate = false;

	// Also reset the pause state.
	_pauseLevel = 0;

	// Reset the pause state of the tracks too
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		(*it)->pause(false);
}

void VideoDecoder::setRate(const Common::Rational &rate) {
	if (!isVideoLoaded() || _playbackRate == rate)
		return;

	if (rate == 0) {
		stop();
		return;
	}

	Common::Rational targetRate = rate;

	if (hasAudio()) {
		setAudioRate(targetRate);
	}

	// Attempt to set the reverse
	if (!setReverse(rate < 0)) {
		assert(rate < 0); // We shouldn't fail for forward.
		warning("Cannot set custom rate to backwards");
		setReverse(false);
		targetRate = 1;

		if (_playbackRate == targetRate)
			return;
	}

	if (_playbackRate != 0)
		_lastTimeChange = getTime();

	_playbackRate = targetRate;
	_startTime = g_system->getMillis();

	// Adjust start time if we've seek'ed to something besides zero time
	if (_lastTimeChange != 0)
		_startTime -= (_lastTimeChange.msecs() / _playbackRate).toInt();

	startAudio();
}

bool VideoDecoder::isPlaying() const {
	return _playbackRate != 0;
}

Audio::Timestamp VideoDecoder::getDuration() const {
	Audio::Timestamp maxDuration(0, 1000);

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		Audio::Timestamp duration = (*it)->getDuration();

		if (duration > maxDuration)
			maxDuration = duration;
	}

	return maxDuration;
}

bool VideoDecoder::seekIntern(const Audio::Timestamp &time) {
	for (TrackList::iterator it = _internalTracks.begin(); it != _internalTracks.end(); it++)
		if (!(*it)->seek(time))
			return false;

	return true;
}

bool VideoDecoder::setDitheringPalette(const byte *palette) {
	// If a frame was already decoded, we can't set it now.
	if (!_canSetDither)
		return false;

	bool result = false;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && ((VideoTrack *)*it)->canDither()) {
			((VideoTrack *)*it)->setDither(palette);
			result = true;
		}
	}

	return result;
}

bool VideoDecoder::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	// If a frame was already decoded, we can't set it now.
	if (!_canSetDefaultFormat)
		return false;

	bool result = false;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo) {
			if (((VideoTrack *)*it)->setOutputPixelFormat(format))
				result = true;
		}
	}

	return result;
}

void VideoDecoder::setVideoCodecAccuracy(Image::CodecAccuracy accuracy) {
	_videoCodecAccuracy = accuracy;

	for (Track *track : _tracks) {
		if (track->getTrackType() == Track::kTrackTypeVideo)
			static_cast<VideoTrack *>(track)->setCodecAccuracy(accuracy);
	}
}

VideoDecoder::Track::Track() {
	_paused = false;
}

bool VideoDecoder::Track::isRewindable() const {
	return isSeekable();
}

bool VideoDecoder::Track::rewind() {
	return seek(Audio::Timestamp(0, 1000));
}

void VideoDecoder::Track::pause(bool shouldPause) {
	_paused = shouldPause;
	pauseIntern(shouldPause);
}

Audio::Timestamp VideoDecoder::Track::getDuration() const {
	return Audio::Timestamp(0, 1000);
}

bool VideoDecoder::VideoTrack::endOfTrack() const {
	return getCurFrame() >= (getFrameCount() - 1);
}

Audio::Timestamp VideoDecoder::VideoTrack::getFrameTime(uint frame) const {
	// Default implementation: Return an invalid (negative) number
	return Audio::Timestamp().addFrames(-1);
}

uint32 VideoDecoder::FixedRateVideoTrack::getNextFrameStartTime() const {
	if (endOfTrack() || getCurFrame() < 0)
		return 0;

	return getFrameTime(getCurFrame() + 1).msecs();
}

Audio::Timestamp VideoDecoder::FixedRateVideoTrack::getFrameTime(uint frame) const {
	// Try to get as accurate as possible, considering we have a fractional frame rate
	// (which Audio::Timestamp doesn't support).
	Common::Rational frameRate = getFrameRate();

	// Try to keep it in terms of the frame rate, if the frame rate is a whole
	// number.
	if (frameRate.getDenominator() == 1)
		return Audio::Timestamp(0, frame, frameRate.toInt());

	// Convert as best as possible
	Common::Rational time = frameRate.getInverse() * frame;
	return Audio::Timestamp(0, time.getNumerator(), time.getDenominator());
}

uint VideoDecoder::FixedRateVideoTrack::getFrameAtTime(const Audio::Timestamp &time) const {
	Common::Rational frameRate = getFrameRate();

	// Easy conversion
	if (frameRate == time.framerate())
		return time.totalNumberOfFrames();

	// Create the rational based on the time first to hopefully cancel out
	// *something* when multiplying by the frameRate (which can be large in
	// some AVI videos).
	return (Common::Rational(time.totalNumberOfFrames(), time.framerate()) * frameRate).toInt();
}

Audio::Timestamp VideoDecoder::FixedRateVideoTrack::getDuration() const {
	return getFrameTime(getFrameCount());
}

VideoDecoder::AudioTrack::AudioTrack(Audio::Mixer::SoundType soundType) :
		_volume(Audio::Mixer::kMaxChannelVolume),
		_soundType(soundType),
		_rate(0),
		_balance(0),
		_muted(false) {
}

bool VideoDecoder::AudioTrack::endOfTrack() const {
	Audio::AudioStream *stream = getAudioStream();
	return !stream || !g_system->getMixer()->isSoundHandleActive(_handle) || stream->endOfData();
}

void VideoDecoder::AudioTrack::setVolume(byte volume) {
	_volume = volume;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, _muted ? 0 : _volume);
}

void VideoDecoder::AudioTrack::setRate(uint32 rate) {
	_rate = rate;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelRate(_handle, _rate);
}

void VideoDecoder::AudioTrack::setRate(Common::Rational rate) {
	Audio::AudioStream *stream = getAudioStream();
	assert(stream);

	// Convert rational rate to audio rate
	uint32 convertedRate = (stream->getRate() * rate).toInt();

	setRate(convertedRate);
}

void VideoDecoder::AudioTrack::setBalance(int8 balance) {
	_balance = balance;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelBalance(_handle, _balance);
}

void VideoDecoder::AudioTrack::start() {
	stop();

	Audio::AudioStream *stream = getAudioStream();
	assert(stream);

	g_system->getMixer()->playStream(_soundType, &_handle, stream, -1, _muted ? 0 : getVolume(), getBalance(), DisposeAfterUse::NO);

	// Set rate of audio
	if (_rate != 0)
		g_system->getMixer()->setChannelRate(_handle, _rate);

	// Pause the audio again if we're still paused
	if (isPaused())
		g_system->getMixer()->pauseHandle(_handle, true);
}

void VideoDecoder::AudioTrack::stop() {
	g_system->getMixer()->stopHandle(_handle);
}

void VideoDecoder::AudioTrack::start(const Audio::Timestamp &limit) {
	stop();

	Audio::AudioStream *stream = getAudioStream();
	assert(stream);

	stream = Audio::makeLimitingAudioStream(stream, limit, DisposeAfterUse::NO);

	g_system->getMixer()->playStream(_soundType, &_handle, stream, -1, _muted ? 0 : getVolume(), getBalance(), DisposeAfterUse::YES);

	// Set rate of audio
	if (_rate != 0)
		g_system->getMixer()->setChannelRate(_handle, _rate);

	// Pause the audio again if we're still paused
	if (isPaused())
		g_system->getMixer()->pauseHandle(_handle, true);
}

uint32 VideoDecoder::AudioTrack::getRunningTime() const {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		return g_system->getMixer()->getSoundElapsedTime(_handle);

	return 0;
}

void VideoDecoder::AudioTrack::setMute(bool mute) {
	// Update the mute settings, if required
	if (_muted != mute) {
		_muted = mute;

		if (g_system->getMixer()->isSoundHandleActive(_handle))
			g_system->getMixer()->setChannelVolume(_handle, mute ? 0 : _volume);
	}
}

void VideoDecoder::AudioTrack::pauseIntern(bool shouldPause) {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->pauseHandle(_handle, shouldPause);
}

Audio::AudioStream *VideoDecoder::RewindableAudioTrack::getAudioStream() const {
	return getRewindableAudioStream();
}

bool VideoDecoder::RewindableAudioTrack::rewind() {
	Audio::RewindableAudioStream *stream = getRewindableAudioStream();
	assert(stream);
	return stream->rewind();
}

Audio::Timestamp VideoDecoder::SeekableAudioTrack::getDuration() const {
	Audio::SeekableAudioStream *stream = getSeekableAudioStream();
	assert(stream);
	return stream->getLength();
}

Audio::AudioStream *VideoDecoder::SeekableAudioTrack::getAudioStream() const {
	return getSeekableAudioStream();
}

bool VideoDecoder::SeekableAudioTrack::seek(const Audio::Timestamp &time) {
	Audio::SeekableAudioStream *stream = getSeekableAudioStream();
	assert(stream);
	return stream->seek(time);
}

VideoDecoder::StreamFileAudioTrack::StreamFileAudioTrack(Audio::Mixer::SoundType soundType) :
		SeekableAudioTrack(soundType) {
	_stream = 0;
}

VideoDecoder::StreamFileAudioTrack::StreamFileAudioTrack(Audio::SeekableAudioStream *stream, Audio::Mixer::SoundType soundType) :
		SeekableAudioTrack(soundType) {
	_stream = stream;
}

VideoDecoder::StreamFileAudioTrack::~StreamFileAudioTrack() {
	delete _stream;
}

bool VideoDecoder::StreamFileAudioTrack::loadFromFile(const Common::Path &baseName) {
	// TODO: Make sure the stream isn't being played
	delete _stream;
	_stream = Audio::SeekableAudioStream::openStreamFile(baseName);
	return _stream != 0;
}

void VideoDecoder::addTrack(Track *track, bool isExternal) {
	_tracks.push_back(track);

	if (isExternal)
		_externalTracks.push_back(track);
	else
		_internalTracks.push_back(track);

	if (track->getTrackType() == Track::kTrackTypeAudio) {
		// Update volume settings if it's an audio track
		((AudioTrack *)track)->setVolume(_audioVolume);
		((AudioTrack *)track)->setBalance(_audioBalance);

		if (!isExternal && supportsAudioTrackSwitching()) {
			if (_mainAudioTrack) {
				// The main audio track has already been found
				((AudioTrack *)track)->setMute(true);
			} else {
				// First audio track found -> now the main one
				_mainAudioTrack = (AudioTrack *)track;
				_mainAudioTrack->setMute(false);
			}
		}
	} else if (track->getTrackType() == Track::kTrackTypeVideo) {
		// If this track has a better time, update _nextVideoTrack
		if (!_nextVideoTrack || ((VideoTrack *)track)->getNextFrameStartTime() < _nextVideoTrack->getNextFrameStartTime())
			_nextVideoTrack = (VideoTrack *)track;
	}

	// Keep the track paused if we're paused
	if (isPaused())
		track->pause(true);

	// Start the track if we're playing
	if (isPlaying() && track->getTrackType() == Track::kTrackTypeAudio)
		((AudioTrack *)track)->start();
}

bool VideoDecoder::addStreamTrack(Audio::SeekableAudioStream *stream) {
	// Only allow adding external tracks if a video is already loaded
	if (!isVideoLoaded())
		return false;

	StreamFileAudioTrack *track = new StreamFileAudioTrack(stream, getSoundType());
	addTrack(track, true);
	return true;
}

bool VideoDecoder::addStreamFileTrack(const Common::Path &baseName) {
	// Only allow adding external tracks if a video is already loaded
	if (!isVideoLoaded())
		return false;

	StreamFileAudioTrack *track = new StreamFileAudioTrack(getSoundType());

	bool result = track->loadFromFile(baseName);

	if (result)
		addTrack(track, true);
	else
		delete track;

	return result;
}

bool VideoDecoder::setAudioTrack(int index) {
	if (!supportsAudioTrackSwitching())
		return false;

	AudioTrack *audioTrack = getAudioTrack(index);

	if (!audioTrack)
		return false;

	if (_mainAudioTrack == audioTrack)
		return true;

	_mainAudioTrack->setMute(true);
	audioTrack->setMute(false);
	_mainAudioTrack = audioTrack;
	return true;
}

uint VideoDecoder::getAudioTrackCount() const {
	uint count = 0;

	for (TrackList::const_iterator it = _internalTracks.begin(); it != _internalTracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			count++;

	return count;
}

void VideoDecoder::setEndTime(const Audio::Timestamp &endTime) {
	Audio::Timestamp startTime = 0;

	if (isPlaying()) {
		startTime = getTime();
		stopAudio();
	}

	_endTime = endTime;
	_endTimeSet = true;

	if (startTime > endTime)
		return;

	if (isPlaying()) {
		// We'll assume the audio track is going to start up at the same time it just was
		// and therefore not do any seeking.
		// Might want to set it anyway if we're seekable.
		startAudioLimit(_endTime.msecs() - startTime.msecs());
		_lastTimeChange = startTime;
	}
}

void VideoDecoder::setEndFrame(uint frame) {
	VideoTrack *track = 0;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo) {
			// We only allow this when one video track is present
			if (track)
				return;

			track = (VideoTrack *)*it;
		}
	}

	// If we didn't find a video track, we can't set the final frame (of course)
	if (!track)
		return;

	Audio::Timestamp time = track->getFrameTime(frame + 1);

	if (time < 0)
		return;

	setEndTime(time);
}

void VideoDecoder::resetStartTime() {
	if (_nextVideoTrack) {
		Audio::Timestamp curTime = _nextVideoTrack->getFrameTime(_nextVideoTrack->getCurFrame());
		if (isPlaying()) {
			_startTime = g_system->getMillis() - (curTime.msecs() / _playbackRate).toInt();
		}
	}
}

VideoDecoder::Track *VideoDecoder::getTrack(uint track) {
	if (track >= _internalTracks.size())
		return 0;

	return _internalTracks[track];
}

const VideoDecoder::Track *VideoDecoder::getTrack(uint track) const {
	if (track >= _internalTracks.size())
		return 0;

	return _internalTracks[track];
}

bool VideoDecoder::endOfVideoTracks() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && !(*it)->endOfTrack())
			return false;

	return true;
}

VideoDecoder::VideoTrack *VideoDecoder::findNextVideoTrack() {
	_nextVideoTrack = 0;
	uint32 bestTime = 0xFFFFFFFF;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && !(*it)->endOfTrack()) {
			VideoTrack *track = (VideoTrack *)*it;
			uint32 time = track->getNextFrameStartTime();

			if (time < bestTime) {
				bestTime = time;
				_nextVideoTrack = track;
			}
		}
	}

	return _nextVideoTrack;
}

void VideoDecoder::startAudio() {
	if (_endTimeSet) {
		// HACK: Timestamp's subtraction asserts out when subtracting two times
		// with different rates.
		startAudioLimit(_endTime - _lastTimeChange.convertToFramerate(_endTime.framerate()));
		return;
	}

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->start();
}

void VideoDecoder::stopAudio() {
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->stop();
}

void VideoDecoder::setAudioRate(Common::Rational rate) {
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio) {
			((AudioTrack *)*it)->setRate(rate);
		}
}

void VideoDecoder::startAudioLimit(const Audio::Timestamp &limit) {
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->start(limit);
}

bool VideoDecoder::hasFramesLeft() const {
	// This is similar to endOfVideo(), except it doesn't take Audio into account (and returns true if not the end of the video)
	// This is only used for needsUpdate() atm so that setEndTime() works properly
	// And unlike endOfVideoTracks(), this takes into account _endTime
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() != Track::kTrackTypeVideo)
			continue;

		const VideoTrack *track = (const VideoTrack *)*it;

		bool videoEndTimeReached = _endTimeSet && track->getNextFrameStartTime() >= (uint)_endTime.msecs();
		bool endReached = track->endOfTrack() || (isPlaying() && videoEndTimeReached);
		if (!endReached)
			return true;
	}

	return false;
}

bool VideoDecoder::hasAudio() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			return true;

	return false;
}

void VideoDecoder::eraseTrack(Track *track) {
	for (uint idx = 0; idx < _externalTracks.size(); ++idx) {
		if (_externalTracks[idx] == track)
			_externalTracks.remove_at(idx);
	}

	for (uint idx = 0; idx < _internalTracks.size(); ++idx) {
		if (_internalTracks[idx] == track)
			_internalTracks.remove_at(idx);
	}

	for (uint idx = 0; idx < _tracks.size(); ++idx) {
		if (_tracks[idx] == track)
			_tracks.remove_at(idx);
	}
}

} // End of namespace Video
