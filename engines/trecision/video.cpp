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

void NightlongVideoDecoder::muteTrack(uint track, bool mute) {
	// FIXME: In the Amiga version, there's only one audio track
	// for each video, so we silently ignore calls to mute the
	// second audio track. Is this correct?

	Track *t = getTrack(track);
	if (t && t->getTrackType() == Track::kTrackTypeAudio) {
		((AudioTrack *)t)->setMute(mute);
	}
}

void NightlongVideoDecoder::setMute(bool mute) {
	for (TrackList::iterator it = getTrackListBegin(); it != getTrackListEnd(); ++it) {
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setMute(mute);
	}
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

// ----------------------------------------------------------------------------

NightlongAmigaDecoder::AmigaVideoTrack::AmigaVideoTrack(const Common::String &fileName) {
	memset(_palette, 0, sizeof(_palette));

	Common::File *stream = new Common::File();
	stream->open(fileName);

	if (!stream->isOpen())
		return;

	_curFrame = 0;
	_frameCount = 10; // TODO: Anything > 1 to keep playing till the audio is done

	// TODO: some videos have more than 256 entries
	/*uint16 palEntries = stream->readUint16LE();
	stream->skip(2); // unknown
	for (uint16 i = 0; i < palEntries; i++) {
		_palette[i * 3] = stream->readByte();
		_palette[i * 3 + 1] = stream->readByte();
		_palette[i * 3 + 2] = stream->readByte();
		stream->skip(1); // unused alpha channel
	}*/

	delete stream;
}

uint16 NightlongAmigaDecoder::AmigaVideoTrack::getWidth() const {
	// TODO
	return 0;
}

uint16 NightlongAmigaDecoder::AmigaVideoTrack::getHeight() const {
	// TODO
	return 0;
}

Graphics::PixelFormat NightlongAmigaDecoder::AmigaVideoTrack::getPixelFormat() const {
	// TODO
	return g_system->getScreenFormat();
}

uint32 NightlongAmigaDecoder::AmigaVideoTrack::getNextFrameStartTime() const {
	// TODO
	return 0;
}

const Graphics::Surface *NightlongAmigaDecoder::AmigaVideoTrack::decodeNextFrame() {
	// TODO
	return nullptr;
}

NightlongAmigaDecoder::AmigaAudioTrack::AmigaAudioTrack(const Common::String &fileName) :
	AudioTrack(Audio::Mixer::SoundType::kSFXSoundType) {
	Common::File *stream = new Common::File();
	stream->open(fileName);
	_audioStream = Audio::makeRawStream(stream, 11025, 0, DisposeAfterUse::YES);
}

void NightlongAmigaDecoder::readNextPacket() {
	AmigaVideoTrack *videoTrack = (AmigaVideoTrack *)getTrack(0);

	if (videoTrack->endOfTrack())
		return;

	// TODO
}

bool NightlongAmigaDecoder::loadStream(Common::SeekableReadStream *stream) {
	Common::File *file = dynamic_cast<Common::File *>(stream);
	if (!file)
		return false;
	Common::String fileName = file->getName();
	addTrack(new AmigaVideoTrack(fileName));
	if (Common::File::exists("a" + fileName))
		addTrack(new AmigaAudioTrack("a" + fileName));

	return true;
}

bool NightlongAmigaDecoder::forceSeekToFrame(uint frame) {
	// TODO
	return false;
}

const Common::Rect *NightlongAmigaDecoder::getNextDirtyRect() {
	// TODO
	return &_lastDirtyRect;
}

bool NightlongAmigaDecoder::endOfFrames() const {
	//return getCurFrame() >= (int32)getFrameCount() - 1;
	return true;
}

} // namespace Trecision
