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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "audio/decoders/raw.h"

#include "trecision/actor.h"
#include "trecision/animtype.h"
#include "trecision/defines.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

NightlongVideoDecoder::NightlongVideoDecoder(bool isAmiga) {
	_isAmiga = isAmiga;
	_smkDecoder = !isAmiga ? new NightlongSmackerDecoder() : nullptr;
	_mixer = g_system->getMixer();
}

NightlongVideoDecoder::~NightlongVideoDecoder() {
	delete _smkDecoder;

	if (_mixer->isSoundHandleActive(_amigaSoundHandle))
		_mixer->stopHandle(_amigaSoundHandle);
}

bool NightlongVideoDecoder::loadFile(const Common::Path &filename) {
	if (!_isAmiga)
		return _smkDecoder->loadFile(filename);
	else {
		// TODO: Amiga video format

		// Load the video's audio track
		Common::File *stream = new Common::File();
		Common::String file = filename.toString();
		stream->open("a" + file);

		if (stream->isOpen()) {
			Audio::SeekableAudioStream *sound = Audio::makeRawStream(stream, 11025, 0, DisposeAfterUse::YES);

			_mixer->playStream(
				Audio::Mixer::kSFXSoundType,
				&_amigaSoundHandle,
				sound);

			return true;
		} else {
			delete stream;
			return false;
		}	
	}
}

bool NightlongVideoDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (!_isAmiga)
		return _smkDecoder->loadStream(stream);
	else
		return false;	// TODO: Amiga videos
}

void NightlongVideoDecoder::muteTrack(uint track, bool mute) {
	if (!_isAmiga)
		_smkDecoder->muteTrack(track, mute);
	// TODO: Amiga videos
}

void NightlongVideoDecoder::setMute(bool mute) {
	if (!_isAmiga)
		_smkDecoder->setMute(mute);
	// TODO: Amiga videos
}

bool NightlongVideoDecoder::forceSeekToFrame(uint frame) {
	if (!_isAmiga)
		return _smkDecoder->forceSeekToFrame(frame);
	else
		return false;	// TODO: Amiga videos
}

bool NightlongVideoDecoder::endOfFrames() const {
	if (!_isAmiga)
		return _smkDecoder->endOfFrames();
	else
		return !_mixer->isSoundHandleActive(_amigaSoundHandle);	// HACK, since we only play the audio for now
}

int NightlongVideoDecoder::getCurFrame() const {
	if (!_isAmiga)
		return _smkDecoder->getCurFrame();
	else
		return 0;	// TODO: Amiga videos
}

uint16 NightlongVideoDecoder::getWidth() const {
	if (!_isAmiga)
		return _smkDecoder->getWidth();
	else
		return 0;	// TODO: Amiga videos
}

uint16 NightlongVideoDecoder::getHeight() const {
	if (!_isAmiga)
		return _smkDecoder->getHeight();
	else
		return 0;	// TODO: Amiga videos
}

const Graphics::Surface *NightlongVideoDecoder::decodeNextFrame() {
	if (!_isAmiga)
		return _smkDecoder->decodeNextFrame();
	else
		return nullptr;	// TODO: Amiga videos
}

uint32 NightlongVideoDecoder::getFrameCount() const {
	if (!_isAmiga)
		return _smkDecoder->getFrameCount();
	else
		return 10;	// TODO: Amiga videos. Anything > 1 to keep playing till the audio is done
}

const byte *NightlongVideoDecoder::getPalette() {
	if (!_isAmiga)
		return _smkDecoder->getPalette();
	else
		return nullptr;	// TODO: Amiga videos
}

void NightlongVideoDecoder::start() {
	if (!_isAmiga)
		_smkDecoder->start();
	// TODO: Amiga videos
}

void NightlongVideoDecoder::rewind() {
	if (!_isAmiga)
		_smkDecoder->rewind();
	// TODO: Amiga videos
}

bool NightlongVideoDecoder::needsUpdate() const {
	if (!_isAmiga)
		return _smkDecoder->needsUpdate();
	else
		return false;	// TODO: Amiga videos
}

void NightlongVideoDecoder::setEndFrame(uint frame) {
	if (!_isAmiga)
		_smkDecoder->setEndFrame(frame);
	// TODO: Amiga videos
}

bool NightlongVideoDecoder::endOfVideo() const {
	if (!_isAmiga)
		return _smkDecoder->endOfVideo();
	else
		return false;	// TODO: Amiga videos
}

const Common::Rect *NightlongVideoDecoder::getNextDirtyRect() {
	if (!_isAmiga)
		return _smkDecoder->getNextDirtyRect();
	else
		return nullptr;	// TODO: Amiga videos
}

bool NightlongSmackerDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (!SmackerDecoder::loadStream(stream))
		return false;

	// Map audio tracks to sound types
	for (uint32 i = 0; i < 8; i++) {
		Track *t = getTrack(i);
		if (t && t->getTrackType() == Track::kTrackTypeAudio) {
			AudioTrack *audio = (AudioTrack *)t;
			audio->setMute(false);
			audio->setSoundType(i == 7 ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType);
		}
	}
	return true;
}

void NightlongSmackerDecoder::muteTrack(uint track, bool mute) {
	Track *t = getTrack(track);
	if (t && t->getTrackType() == Track::kTrackTypeAudio) {
		((AudioTrack *)t)->setMute(mute);
	}
}

void NightlongSmackerDecoder::setMute(bool mute) {
	for (TrackList::iterator it = getTrackListBegin(); it != getTrackListEnd(); ++it) {
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setMute(mute);
	}
}

bool NightlongSmackerDecoder::forceSeekToFrame(uint frame) {
	const uint seekFrame = MAX<uint>(frame - 10, 0);

	if (!isVideoLoaded())
		return true;

	if (seekFrame >= getFrameCount())
		return false;

	if (!rewind())
		return false;

	stopAudio();
	SmackerVideoTrack *videoTrack = (SmackerVideoTrack *)getTrack(0);
	uint32 startPos = _fileStream->pos();
	uint32 offset = 0;
	for (uint32 i = 0; i < seekFrame; i++) {
		videoTrack->increaseCurFrame();
		// Frames with palette data contain palette entries which use
		// the previous palette as their base. Therefore, we need to
		// parse all palette entries up to the requested frame
		if (_frameTypes[videoTrack->getCurFrame()] & 1) {
			_fileStream->seek(startPos + offset, SEEK_SET);
			videoTrack->unpackPalette(_fileStream);
		}
		offset += _frameSizes[i] & ~3;
	}

	if (!_fileStream->seek(startPos + offset, SEEK_SET))
		return false;

	while (getCurFrame() < (int)frame) {
		decodeNextFrame();
	}

	_lastTimeChange = videoTrack->getFrameTime(frame);
	_startTime = g_system->getMillis() - (_lastTimeChange.msecs() / getRate()).toInt();
	startAudio();

	return true;
}

// TODO: Background videos only loop smoothly like this,
// possibly an audio track bug?
bool NightlongSmackerDecoder::endOfFrames() const {
	return getCurFrame() >= (int32)getFrameCount() - 1;
}

} // namespace Trecision
