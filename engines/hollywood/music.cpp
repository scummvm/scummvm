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

#include "hollywood/music.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

static const char *const kIntroMusicArchiveName = "RESOURCE.M09";
static const uint16 kIntroMusicCueId = 0x000d;
static const uint kMusicCueTableSize = 0x190;
static const int kMusicSampleRate = 11025;

MusicPlayer::MusicPlayer() {
}

MusicPlayer::~MusicPlayer() {
	stop();
}

bool MusicPlayer::playIntroMusic() {
	stop();

	const Common::Path fileName(kIntroMusicArchiveName);
	uint32 start = 0;
	uint32 size = 0;
	if (!readCueSpan(fileName, kIntroMusicCueId, start, size))
		return false;

	Common::File *file = new Common::File();
	if (!file->open(fileName)) {
		warning("Failed to open %s", kIntroMusicArchiveName);
		delete file;
		return false;
	}

	Common::SeekableReadStream *sampleStream = new Common::SeekableSubReadStream(file, start, start + size, DisposeAfterUse::YES);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(sampleStream, kMusicSampleRate,
		Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Failed to create raw stream for %s cue %u", kIntroMusicArchiveName, kIntroMusicCueId);
		delete sampleStream;
		return false;
	}

	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, audioStream,
		-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);

	debugC(1, kDebugResources, "Started intro music %s cue %u: offset=%u size=%u",
		kIntroMusicArchiveName, kIntroMusicCueId, start, size);
	return true;
}

void MusicPlayer::stop() {
	if (isPlaying())
		g_system->getMixer()->stopHandle(_musicHandle);
}

bool MusicPlayer::isPlaying() const {
	return g_system && g_system->getMixer() && g_system->getMixer()->isSoundHandleActive(_musicHandle);
}

bool MusicPlayer::readCueSpan(const Common::Path &fileName, uint16 cueId, uint32 &start, uint32 &size) const {
	if ((cueId + 1) * 4 >= kMusicCueTableSize) {
		warning("Invalid %s cue id %u", kIntroMusicArchiveName, cueId);
		return false;
	}

	Common::File file;
	if (!file.open(fileName)) {
		warning("Failed to open %s", kIntroMusicArchiveName);
		return false;
	}

	if (file.size() < (int32)kMusicCueTableSize) {
		warning("%s is too small for the cue table", kIntroMusicArchiveName);
		return false;
	}

	file.seek(cueId * 4);
	start = file.readUint32LE();
	const uint32 end = file.readUint32LE();

	if (start >= end || end > (uint32)file.size()) {
		warning("Invalid %s cue %u span: start=%u end=%u fileSize=%u",
			kIntroMusicArchiveName, cueId, start, end, (uint)file.size());
		return false;
	}

	size = end - start;
	if (size & 1)
		size--;

	if (size == 0) {
		warning("%s cue %u is empty", kIntroMusicArchiveName, cueId);
		return false;
	}

	return true;
}

} // End of namespace Hollywood
